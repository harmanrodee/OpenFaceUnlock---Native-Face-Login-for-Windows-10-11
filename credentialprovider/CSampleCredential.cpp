//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#include <fstream>
#include <string>
#include <Windows.h>
#include <iostream>
#include <vector>
#define WIN32_NO_STATUS
#define MAX_CREDENTIAL_BLOB_SIZE 4096
#endif
#include <unknwn.h>
#include <TlHelp32.h>
#include "CSampleCredential.h"
#include "guid.h"
#include "CredentialOP.h"

//Face Authentication globals
bool g_faceAuthenticated = false;
static wchar_t g_faceUsername[128] = L"";
static wchar_t g_facePassword[128] = L"";
static std::vector<BYTE> g_credentialBlob;
static std::wstring g_faceStatus = L"";
static CSampleCredential* g_activeCredential = nullptr;
static HANDLE g_hFaceUnlockProcess = NULL;
struct AuthPacket {
    int state = 0;
    bool authenticated;
    DWORD credentialBlobSize;
    BYTE credentialBlob[MAX_CREDENTIAL_BLOB_SIZE];
};

enum FaceUnlockState
{
    STATE_IDLE = 0,
    STATE_STARTING = 1,
    STATE_LOADING_MODELS = 2,
    STATE_OPENING_CAMERA = 3,
    STATE_DETECTING_FACE = 4,
    STATE_RECOGNIZING_FACE = 5,
    STATE_AUTH_SUCCESS = 6,
    STATE_AUTH_FAILED = 7,
    STATE_ERROR_CAMERA = 8,
    STATE_ERROR_FEED = 9,
    STATE_ERROR_MODEL = 10,
    STATE_ERROR_EMBEDDING = 11,
    STATE_ERROR_CRED = 12
};

void HandlePacket(const AuthPacket& packet)
{
    switch (packet.state)
    {
    case STATE_STARTING:
        g_faceStatus = L"Starting Face Unlock";
        break;

    case STATE_OPENING_CAMERA:
        g_faceStatus = L"Opening camera";
        break;

    case STATE_LOADING_MODELS:
        g_faceStatus = L"Loading models";
        break;

    case STATE_RECOGNIZING_FACE:
        g_faceStatus = L"Recognizing face";
        break;

    case STATE_AUTH_SUCCESS:
    {
        g_faceStatus = L"Face recognized";
        g_faceAuthenticated = packet.authenticated;
        g_credentialBlob.assign(packet.credentialBlob, packet.credentialBlob + packet.credentialBlobSize);
        bool success = DecryptCredentialBlob(g_credentialBlob, g_faceUsername, 128, g_facePassword, 128);

        if (!success) {
            g_faceAuthenticated = false;
        }

        if (g_activeCredential)
            g_activeCredential->NotifyWindowsCredentialChanged();
        break;
    }

    case STATE_AUTH_FAILED:
        g_faceStatus = L"Face not recognized";
        break;

    case STATE_ERROR_CAMERA:
        g_faceStatus = L"Unable to access camera";
        break;

    case STATE_ERROR_MODEL:
        g_faceStatus = L"Error: Unable To access models";
        break;

    case STATE_ERROR_EMBEDDING:
        g_faceStatus = L"No Face Registered";
        break;

    case STATE_ERROR_CRED:
        g_faceStatus = L"Password Not Saved, Update Password !";
        break;
    }

    if (g_activeCredential)
        g_activeCredential->UpdateStatusText(g_faceStatus.c_str());
}

static void NamedPipeServer() {
    while (true)
    {
        HANDLE hPipe = CreateNamedPipe(
            L"\\\\.\\pipe\\FaceUnlockAuth",
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,
            1024,
            1024,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE)
            return;

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ?
            TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);


        if (connected)
        {
            AuthPacket packet{};
            DWORD bytesRead = 0;

            BOOL success = ReadFile(
                hPipe,
                &packet,
                sizeof(packet),
                &bytesRead,
                NULL
            );
    
            if (success)
            {
                HandlePacket(packet);
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
}

DWORD WINAPI PipeServerThread(LPVOID lpParam) {
    NamedPipeServer();
    return 0;
};

std::wstring getFaceUnlockDir() {
    HKEY hkey;
    wchar_t buffer[MAX_PATH];
    DWORD buffer_size = sizeof(buffer);

    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\FaceUnlock", 0, KEY_READ, &hkey);

    if (result != ERROR_SUCCESS) {
        return L"";
    }

    result = RegQueryValueEx(hkey, L"InstallDir", NULL, NULL, (LPBYTE)buffer, &buffer_size);

    RegCloseKey(hkey);

    if (result != ERROR_SUCCESS) {
        return L"";
    }

    return std::wstring(buffer);
}

static void KillZombieFaceUnlockProcesses() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnap, &pe32)) {
            do {
                if (_wcsicmp(pe32.szExeFile, L"FaceUnlock.exe") == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcess != NULL) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32NextW(hSnap, &pe32));
        }
        CloseHandle(hSnap);
    }
}

bool IsFaceUnlockRunning()
{
    if (g_hFaceUnlockProcess == NULL) {
        return false;
    }

    DWORD exitCode = 0;
    if (GetExitCodeProcess(g_hFaceUnlockProcess, &exitCode)) {
        if (exitCode == STILL_ACTIVE) {
            return true;
        }
    }

    CloseHandle(g_hFaceUnlockProcess);
    g_hFaceUnlockProcess = NULL;
    return false;
}

void CSampleCredential::StartFaceUnlockProcess() {
    if (IsFaceUnlockRunning()) {
        return;
    }
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    std::wstring install_dir = getFaceUnlockDir();

    if (!install_dir.empty()) {
        KillZombieFaceUnlockProcesses();
        std::wstring exe_path = install_dir + L"\\FaceUnlock.exe";
        g_faceStatus = L"Opening camera...";
        std::wstring commandLine = L"--mode login";
        BOOL success = CreateProcess(
            exe_path.c_str(),
            &commandLine[0],
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW | HIGH_PRIORITY_CLASS,
            NULL,
            install_dir.c_str(),
            &si,
            &pi
        );

        if (success) {
            //CloseHandle(pi.hProcess);
            g_hFaceUnlockProcess = pi.hProcess;
            CloseHandle(pi.hThread);
        }
    }
}

void CSampleCredential::UpdateStatusText(PCWSTR newText) {
    if (_rgFieldStrings[SFI_LOGONSTATUS_TEXT]) {
        CoTaskMemFree(_rgFieldStrings[SFI_LOGONSTATUS_TEXT]);
        _rgFieldStrings[SFI_LOGONSTATUS_TEXT] = nullptr;
    }

    SHStrDupW(newText, &_rgFieldStrings[SFI_LOGONSTATUS_TEXT]);

    if (_pCredProvCredentialEvents) {
        _pCredProvCredentialEvents->SetFieldString(this, SFI_LOGONSTATUS_TEXT, _rgFieldStrings[SFI_LOGONSTATUS_TEXT]);
    }

}

CSampleCredential::CSampleCredential():
    _cRef(1),
    _pCredProvCredentialEvents(nullptr),
    _providerEvents(nullptr),
    _providerContext(0),
    _pszUserSid(nullptr),
    _pszQualifiedUserName(nullptr),
    _fIsLocalUser(false),
    _fChecked(false),
    _fShowControls(false),
    _dwComboIndex(0)
{
    DllAddRef();

    ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
    ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
    ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
}

CSampleCredential::~CSampleCredential()
{
    if (g_hFaceUnlockProcess != NULL)
    {
        TerminateProcess(g_hFaceUnlockProcess, 0);
        CloseHandle(g_hFaceUnlockProcess);
        g_hFaceUnlockProcess = NULL;
    }
    if (_rgFieldStrings[SFI_PASSWORD])
    {
        size_t lenPassword = wcslen(_rgFieldStrings[SFI_PASSWORD]);
        SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));
    }
    for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
    {
        CoTaskMemFree(_rgFieldStrings[i]);
        CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
    }
    CoTaskMemFree(_pszUserSid);
    CoTaskMemFree(_pszQualifiedUserName);
    DllRelease();
}


void CSampleCredential::SetProviderEvents(
    ICredentialProviderEvents* events,
    UINT_PTR context)
{
    _providerEvents = events;
    _providerContext = context;
}

void CSampleCredential::NotifyWindowsCredentialChanged()
{

    if (_providerEvents)
    {
        HRESULT hr = _providerEvents->CredentialsChanged(_providerContext);
    }

}

// Initializes one credential with the field information passed in.
// Set the value of the SFI_LARGE_TEXT field to pwzUsername.
HRESULT CSampleCredential::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
                                      _In_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR const *rgcpfd,
                                      _In_ FIELD_STATE_PAIR const *rgfsp,
                                      _In_ ICredentialProviderUser *pcpUser)
{
    g_activeCredential = this;
    g_faceStatus = L"Starting FaceUnlock...";
    HRESULT hr = S_OK;
    /*STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;*/

    HANDLE hThread = CreateThread(
        NULL,
        0,
        PipeServerThread,
        NULL,
        0,
        NULL
    );

    if (hThread != NULL) {
        CloseHandle(hThread);
    }

    StartFaceUnlockProcess();

    //std::wstring install_dir = getFaceUnlockDir();

    //if (!install_dir.empty()) {
    //    KillZombieFaceUnlockProcesses();
    //    std::wstring exe_path = install_dir + L"\\FaceUnlock.exe";
    //    g_faceStatus = L"Opening camera...";
    //    std::wstring commandLine = L"--mode login";
    //    BOOL success = CreateProcess(
    //        exe_path.c_str(),
    //        &commandLine[0],
    //        NULL,
    //        NULL,
    //        FALSE,
    //        CREATE_NO_WINDOW | HIGH_PRIORITY_CLASS,
    //        NULL,
    //        install_dir.c_str(),
    //        &si,
    //        &pi
    //    );

    //    if (success) {
    //        //CloseHandle(pi.hProcess);
    //        g_hFaceUnlockProcess = pi.hProcess;
    //        CloseHandle(pi.hThread);
    //    }
    //}
    _cpus = cpus;

    GUID guidProvider;
    pcpUser->GetProviderID(&guidProvider);
    _fIsLocalUser = (guidProvider == Identity_LocalUserProvider);

    // Copy the field descriptors for each field. This is useful if you want to vary the field
    // descriptors based on what Usage scenario the credential was created for.
    for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
    {
        _rgFieldStatePairs[i] = rgfsp[i];
        hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
    }

    // Initialize the String value of all the fields.
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Sample Credential", &_rgFieldStrings[SFI_LABEL]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Sample Credential Provider", &_rgFieldStrings[SFI_LARGE_TEXT]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Edit Text", &_rgFieldStrings[SFI_EDIT_TEXT]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Submit", &_rgFieldStrings[SFI_SUBMIT_BUTTON]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Checkbox", &_rgFieldStrings[SFI_CHECKBOX]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Combobox", &_rgFieldStrings[SFI_COMBOBOX]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Launch helper window", &_rgFieldStrings[SFI_LAUNCHWINDOW_LINK]);
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Hide additional controls", &_rgFieldStrings[SFI_HIDECONTROLS_LINK]);
    }
    if (SUCCEEDED(hr))
    {
        hr = pcpUser->GetStringValue(PKEY_Identity_QualifiedUserName, &_pszQualifiedUserName);
    }
    if (SUCCEEDED(hr))
    {
        PWSTR pszUserName;
        pcpUser->GetStringValue(PKEY_Identity_UserName, &pszUserName);
        if (pszUserName != nullptr)
        {
            wchar_t szString[256];
            StringCchPrintf(szString, ARRAYSIZE(szString), L"User Name: %s", pszUserName);
            hr = SHStrDupW(szString, &_rgFieldStrings[SFI_FULLNAME_TEXT]);
            CoTaskMemFree(pszUserName);
        }
        else
        {
            hr =  SHStrDupW(L"User Name is NULL", &_rgFieldStrings[SFI_FULLNAME_TEXT]);
        }
    }
    if (SUCCEEDED(hr))
    {
        PWSTR pszDisplayName;
        pcpUser->GetStringValue(PKEY_Identity_DisplayName, &pszDisplayName);
        if (pszDisplayName != nullptr)
        {
            wchar_t szString[256];
            StringCchPrintf(szString, ARRAYSIZE(szString), L"Display Name: %s", pszDisplayName);
            hr = SHStrDupW(szString, &_rgFieldStrings[SFI_DISPLAYNAME_TEXT]);
            CoTaskMemFree(pszDisplayName);
        }
        else
        {
            hr = SHStrDupW(L"Display Name is NULL", &_rgFieldStrings[SFI_DISPLAYNAME_TEXT]);
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Initializing...", &_rgFieldStrings[SFI_LOGONSTATUS_TEXT]);
    }

    if (SUCCEEDED(hr))
    {
        hr = pcpUser->GetSid(&_pszUserSid);
    }

    return hr;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of anything.
HRESULT CSampleCredential::Advise(_In_ ICredentialProviderCredentialEvents *pcpce)
{
    if (_pCredProvCredentialEvents != nullptr)
    {
        _pCredProvCredentialEvents->Release();
    }
    return pcpce->QueryInterface(IID_PPV_ARGS(&_pCredProvCredentialEvents));
}

// LogonUI calls this to tell us to release the callback.
HRESULT CSampleCredential::UnAdvise()
{
    if (_pCredProvCredentialEvents)
    {
        _pCredProvCredentialEvents->Release();
    }
    _pCredProvCredentialEvents = nullptr;
    return S_OK;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the
// field definitions. But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT CSampleCredential::SetSelected(_Out_ BOOL *pbAutoLogon)
{
    StartFaceUnlockProcess();
    if (!g_faceStatus.empty())
    {
        UpdateStatusText(g_faceStatus.c_str());
    }
    static int count = 0;
    count++;

    if (g_faceAuthenticated)
    {
        *pbAutoLogon = TRUE;
    }
    else
    {
        *pbAutoLogon = FALSE;
    }

    return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT CSampleCredential::SetDeselected()
{
    HRESULT hr = S_OK;
    if (_rgFieldStrings[SFI_PASSWORD])
    {
        size_t lenPassword = wcslen(_rgFieldStrings[SFI_PASSWORD]);
        SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));

        CoTaskMemFree(_rgFieldStrings[SFI_PASSWORD]);
        hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);

        if (SUCCEEDED(hr) && _pCredProvCredentialEvents)
        {
            _pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
        }
    }

    return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information
// to display the tile.
HRESULT CSampleCredential::GetFieldState(DWORD dwFieldID,
                                         _Out_ CREDENTIAL_PROVIDER_FIELD_STATE *pcpfs,
                                         _Out_ CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE *pcpfis)
{
    HRESULT hr;

    // Validate our parameters.
    if ((dwFieldID < ARRAYSIZE(_rgFieldStatePairs)))
    {
        *pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
        *pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID
HRESULT CSampleCredential::GetStringValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ PWSTR *ppwsz)
{
    HRESULT hr;
    *ppwsz = nullptr;

    // Check to make sure dwFieldID is a legitimate index
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors))
    {
        // Make a copy of the string and return that. The caller
        // is responsible for freeing it.
        hr = SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Get the image to show in the user tile
HRESULT CSampleCredential::GetBitmapValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ HBITMAP *phbmp)
{
    HRESULT hr;
    *phbmp = nullptr;

    if ((SFI_TILEIMAGE == dwFieldID))
    {
        HBITMAP hbmp = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_TILE_IMAGE));
        if (hbmp != nullptr)
        {
            hr = S_OK;
            *phbmp = hbmp;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets pdwAdjacentTo to the index of the field the submit button should be
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT CSampleCredential::GetSubmitButtonValue(DWORD dwFieldID, _Out_ DWORD *pdwAdjacentTo)
{
    HRESULT hr;

    if (SFI_SUBMIT_BUTTON == dwFieldID)
    {
        // pdwAdjacentTo is a pointer to the fieldID you want the submit button to
        // appear next to.
        *pdwAdjacentTo = SFI_PASSWORD;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field
HRESULT CSampleCredential::SetStringValue(DWORD dwFieldID, _In_ PCWSTR pwz)
{
    HRESULT hr;

    // Validate parameters.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
        (CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft ||
        CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        PWSTR *ppwszStored = &_rgFieldStrings[dwFieldID];
        CoTaskMemFree(*ppwszStored);
        hr = SHStrDupW(pwz, ppwszStored);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Returns whether a checkbox is checked or not as well as its label.
HRESULT CSampleCredential::GetCheckboxValue(DWORD dwFieldID, _Out_ BOOL *pbChecked, _Outptr_result_nullonfailure_ PWSTR *ppwszLabel)
{
    HRESULT hr;
    *ppwszLabel = nullptr;

    // Validate parameters.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
        (CPFT_CHECKBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        *pbChecked = _fChecked;
        hr = SHStrDupW(_rgFieldStrings[SFI_CHECKBOX], ppwszLabel);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets whether the specified checkbox is checked or not.
HRESULT CSampleCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
    HRESULT hr;

    // Validate parameters.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
        (CPFT_CHECKBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        _fChecked = bChecked;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the
// currently selected item (pdwSelectedItem).
HRESULT CSampleCredential::GetComboBoxValueCount(DWORD dwFieldID, _Out_ DWORD *pcItems, _Deref_out_range_(<, *pcItems) _Out_ DWORD *pdwSelectedItem)
{
    HRESULT hr;
    *pcItems = 0;
    *pdwSelectedItem = 0;

    // Validate parameters.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
        (CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        *pcItems = ARRAYSIZE(s_rgComboBoxStrings);
        *pdwSelectedItem = 0;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
HRESULT CSampleCredential::GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, _Outptr_result_nullonfailure_ PWSTR *ppwszItem)
{
    HRESULT hr;
    *ppwszItem = nullptr;

    // Validate parameters.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
        (CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        hr = SHStrDupW(s_rgComboBoxStrings[dwItem], ppwszItem);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Called when the user changes the selected item in the combobox.
HRESULT CSampleCredential::SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem)
{
    HRESULT hr;

    // Validate parameters.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
        (CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        _dwComboIndex = dwSelectedItem;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Called when the user clicks a command link.
HRESULT CSampleCredential::CommandLinkClicked(DWORD dwFieldID)
{
    HRESULT hr = S_OK;

    CREDENTIAL_PROVIDER_FIELD_STATE cpfsShow = CPFS_HIDDEN;

    // Validate parameter.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) &&
        (CPFT_COMMAND_LINK == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        HWND hwndOwner = nullptr;
        switch (dwFieldID)
        {
        case SFI_LAUNCHWINDOW_LINK:
            if (_pCredProvCredentialEvents)
            {
                _pCredProvCredentialEvents->OnCreatingWindow(&hwndOwner);
            }

            // Pop a messagebox indicating the click.
            ::MessageBox(hwndOwner, L"Command link clicked", L"Click!", 0);
            break;
        case SFI_HIDECONTROLS_LINK:
            _pCredProvCredentialEvents->BeginFieldUpdates();
            cpfsShow = _fShowControls ? CPFS_DISPLAY_IN_SELECTED_TILE : CPFS_HIDDEN;
            _pCredProvCredentialEvents->SetFieldState(nullptr, SFI_FULLNAME_TEXT, cpfsShow);
            _pCredProvCredentialEvents->SetFieldState(nullptr, SFI_DISPLAYNAME_TEXT, cpfsShow);
            _pCredProvCredentialEvents->SetFieldState(nullptr, SFI_LOGONSTATUS_TEXT, cpfsShow);
            _pCredProvCredentialEvents->SetFieldState(nullptr, SFI_CHECKBOX, cpfsShow);
            _pCredProvCredentialEvents->SetFieldState(nullptr, SFI_EDIT_TEXT, cpfsShow);
            _pCredProvCredentialEvents->SetFieldState(nullptr, SFI_COMBOBOX, cpfsShow);
            _pCredProvCredentialEvents->SetFieldString(nullptr, SFI_HIDECONTROLS_LINK, _fShowControls? L"Hide additional controls" : L"Show additional controls");
            _pCredProvCredentialEvents->EndFieldUpdates();
            _fShowControls = !_fShowControls;
            break;
        default:
            hr = E_INVALIDARG;
        }

    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Collect the username and password into a serialized credential for the correct usage scenario
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials
// back to the system to log on.
HRESULT CSampleCredential::GetSerialization(
    _Out_ CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    _Out_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
    _Outptr_result_maybenull_ PWSTR* ppwszOptionalStatusText,
    _Out_ CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    HRESULT hr = E_UNEXPECTED;
    *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    *ppwszOptionalStatusText = nullptr;
    *pcpsiOptionalStatusIcon = CPSI_NONE;
    ZeroMemory(pcpcs, sizeof(*pcpcs));

    // 1. Determine which password to use (UI textbox vs Decrypted Face Unlock Password)
    PWSTR passwordToUse = _rgFieldStrings[SFI_PASSWORD];
    if (g_faceAuthenticated) {
        passwordToUse = g_facePassword;
    }

    if (_fIsLocalUser)
    {
        // ================= LOCAL USER PATH =================
        PWSTR pwzProtectedPassword;
        hr = ProtectIfNecessaryAndCopyPassword(passwordToUse, _cpus, &pwzProtectedPassword);
        if (SUCCEEDED(hr))
        {
            PWSTR pszDomain;
            PWSTR pszUsername;
            hr = SplitDomainAndUsername(_pszQualifiedUserName, &pszDomain, &pszUsername);
            if (SUCCEEDED(hr))
            {
                KERB_INTERACTIVE_UNLOCK_LOGON kiul;
                hr = KerbInteractiveUnlockLogonInit(pszDomain, pszUsername, pwzProtectedPassword, _cpus, &kiul);
                if (SUCCEEDED(hr))
                {
                    hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);
                    if (SUCCEEDED(hr))
                    {
                        ULONG ulAuthPackage;
                        hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
                        if (SUCCEEDED(hr))
                        {
                            pcpcs->ulAuthenticationPackage = ulAuthPackage;
                            pcpcs->clsidCredentialProvider = CLSID_CSample;
                            *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;

                            // Reset state
                            g_faceAuthenticated = false;
                            g_faceUsername[0] = L'\0';
                            g_facePassword[0] = L'\0';
                        }
                    }
                }
                CoTaskMemFree(pszDomain);
                CoTaskMemFree(pszUsername);
            }
            CoTaskMemFree(pwzProtectedPassword);
        }
    }
    else
    {
        // ================= MICROSOFT ACCOUNT / NON-LOCAL PATH =================
        DWORD dwAuthFlags = CRED_PACK_PROTECTED_CREDENTIALS | CRED_PACK_ID_PROVIDER_CREDENTIALS;

        // Pass 'passwordToUse' instead of hardcoded _rgFieldStrings[SFI_PASSWORD]
        if (!CredPackAuthenticationBuffer(dwAuthFlags, _pszQualifiedUserName, passwordToUse, nullptr, &pcpcs->cbSerialization) &&
            (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
        {
            pcpcs->rgbSerialization = static_cast<byte*>(CoTaskMemAlloc(pcpcs->cbSerialization));
            if (pcpcs->rgbSerialization != nullptr)
            {
                hr = S_OK;

                // Retrieve the authentication buffer with decrypted face password
                if (CredPackAuthenticationBuffer(dwAuthFlags, _pszQualifiedUserName, passwordToUse, pcpcs->rgbSerialization, &pcpcs->cbSerialization))
                {
                    ULONG ulAuthPackage;
                    hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
                    if (SUCCEEDED(hr))
                    {
                        pcpcs->ulAuthenticationPackage = ulAuthPackage;
                        pcpcs->clsidCredentialProvider = CLSID_CSample;

                        *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;

                        // Reset state for MSA path as well
                        g_faceAuthenticated = false;
                        g_faceUsername[0] = L'\0';
                        g_facePassword[0] = L'\0';
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    if (SUCCEEDED(hr))
                    {
                        hr = E_FAIL;
                    }
                }

                if (FAILED(hr))
                {
                    CoTaskMemFree(pcpcs->rgbSerialization);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

struct REPORT_RESULT_STATUS_INFO
{
    NTSTATUS ntsStatus;
    NTSTATUS ntsSubstatus;
    PWSTR     pwzMessage;
    CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
{
    { STATUS_LOGON_FAILURE, STATUS_SUCCESS, L"Incorrect password or username.", CPSI_ERROR, },
    { STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
};

// ReportResult is completely optional.  Its purpose is to allow a credential to customize the string
// and the icon displayed in the case of a logon failure.  For example, we have chosen to
// customize the error shown in the case of bad username/password and in the case of the account
// being disabled.
HRESULT CSampleCredential::ReportResult(NTSTATUS ntsStatus,
                                        NTSTATUS ntsSubstatus,
                                        _Outptr_result_maybenull_ PWSTR *ppwszOptionalStatusText,
                                        _Out_ CREDENTIAL_PROVIDER_STATUS_ICON *pcpsiOptionalStatusIcon)
{
    *ppwszOptionalStatusText = nullptr;
    *pcpsiOptionalStatusIcon = CPSI_NONE;

    DWORD dwStatusInfo = (DWORD)-1;

    // Look for a match on status and substatus.
    for (DWORD i = 0; i < ARRAYSIZE(s_rgLogonStatusInfo); i++)
    {
        if (s_rgLogonStatusInfo[i].ntsStatus == ntsStatus && s_rgLogonStatusInfo[i].ntsSubstatus == ntsSubstatus)
        {
            dwStatusInfo = i;
            break;
        }
    }

    if ((DWORD)-1 != dwStatusInfo)
    {
        if (SUCCEEDED(SHStrDupW(s_rgLogonStatusInfo[dwStatusInfo].pwzMessage, ppwszOptionalStatusText)))
        {
            *pcpsiOptionalStatusIcon = s_rgLogonStatusInfo[dwStatusInfo].cpsi;
        }
    }

    // If we failed the logon, try to erase the password field.
    if (FAILED(HRESULT_FROM_NT(ntsStatus)))
    {
        if (_pCredProvCredentialEvents)
        {
            _pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
        }
    }

    // Since nullptr is a valid value for *ppwszOptionalStatusText and *pcpsiOptionalStatusIcon
    // this function can't fail.
    return S_OK;
}

// Gets the SID of the user corresponding to the credential.
HRESULT CSampleCredential::GetUserSid(_Outptr_result_nullonfailure_ PWSTR *ppszSid)
{
    *ppszSid = nullptr;
    HRESULT hr = E_UNEXPECTED;
    if (_pszUserSid != nullptr)
    {
        hr = SHStrDupW(_pszUserSid, ppszSid);
    }
    // Return S_FALSE with a null SID in ppszSid for the
    // credential to be associated with an empty user tile.

    return hr;
}

// GetFieldOptions to enable the password reveal button and touch keyboard auto-invoke in the password field.
HRESULT CSampleCredential::GetFieldOptions(DWORD dwFieldID,
                                           _Out_ CREDENTIAL_PROVIDER_CREDENTIAL_FIELD_OPTIONS *pcpcfo)
{
    *pcpcfo = CPCFO_NONE;

    if (dwFieldID == SFI_PASSWORD)
    {
        *pcpcfo = CPCFO_ENABLE_PASSWORD_REVEAL;
    }
    else if (dwFieldID == SFI_TILEIMAGE)
    {
        *pcpcfo = CPCFO_ENABLE_TOUCH_KEYBOARD_AUTO_INVOKE;
    }

    return S_OK;
}
