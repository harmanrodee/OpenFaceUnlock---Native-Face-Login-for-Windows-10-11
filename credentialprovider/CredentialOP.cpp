#include "CredentialOP.h"
#include "json.hpp"

#include <windows.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>

#pragma comment(lib,"Crypt32.lib")

using json = nlohmann::json;

bool DecryptCredentialBlob(
    const std::vector<BYTE>& blob,
    wchar_t* username,
    size_t usernameSize,
    wchar_t* password,
    size_t passwordSize)
{



    if (blob.empty())
    {
        return false;
    }


    DATA_BLOB encryptedBlob{};
    encryptedBlob.cbData = (DWORD)blob.size();
    encryptedBlob.pbData = const_cast<BYTE*>(blob.data());

    DATA_BLOB decryptedBlob{};


    BOOL success = CryptUnprotectData(
        &encryptedBlob,
        NULL,
        NULL,
        NULL,
        NULL,
        CRYPTPROTECT_LOCAL_MACHINE,
        &decryptedBlob);

    if (!success)
    {
        DWORD err = GetLastError();
        return false;
    }


    std::string jsonString(
        reinterpret_cast<char*>(decryptedBlob.pbData),
        decryptedBlob.cbData);

    try
    {
        json credjson = json::parse(jsonString);


        std::string username_str = credjson["username"];
        std::string password_str = credjson["password"];


        int result1 = MultiByteToWideChar(
            CP_UTF8,
            0,
            username_str.c_str(),
            -1,
            username,
            (int)usernameSize);


        if (result1 == 0)
        {

            LocalFree(decryptedBlob.pbData);
            return false;
        }

        int result2 = MultiByteToWideChar(
            CP_UTF8,
            0,
            password_str.c_str(),
            -1,
            password,
            (int)passwordSize);


        if (result2 == 0)
        {

            LocalFree(decryptedBlob.pbData);
            return false;
        }

    }
    catch (const std::exception& ex)
    {

        LocalFree(decryptedBlob.pbData);
        return false;
    }

    LocalFree(decryptedBlob.pbData);

    return true;
}
