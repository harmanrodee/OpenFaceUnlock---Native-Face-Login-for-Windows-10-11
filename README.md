# OpenFaceUnlock — Native Face Login for Windows 10/11

<img src="faceunlock.ico" width="128" align="right" alt="OpenFaceUnlock Icon"/>

**An open-source, purely native C++ and C# based Windows face recognition login utility. It integrates directly into the Windows Lock Screen via a custom Credential Provider DLL (`FaceUnlock.dll`), runs entirely locally using ONNX Runtime (YuNet + ArcFace), and allows you to audit the source code.**

Keywords: *windows face unlock, windows face login, face recognition windows, webcam login windows, windows hello alternative, credential provider face, arcface windows, yunet windows.*

| | |
|---|---|
| Platform | Windows 10 / 11 x64 |
| Languages| C++ (Core Engine & Credential Provider), C# .NET (Manager UI) |
| License  | MIT |
| Models   | YuNet (Face Detection & Landmarks) + ArcFace (512-d Facial Embeddings) |
| Version  | 0.9.0 |

---

## Architecture

OpenFaceUnlock consists of three cooperating subprojects working in harmony:

```
[ LogonUI.exe (SYSTEM) ]
       │
       ▼ (Loads COM DLL)
┌─────────────────────────────────────────────────────────────┐
│  FaceUnlock.dll (Credential Provider C++)                   │
│  - Displays tile on Lock Screen                             │
│  - Spawns FaceUnlock.exe with HIGH_PRIORITY_CLASS           │
│  - Listens on Named Pipe: \\.\pipe\FaceUnlockAuth           │
│  - Decrypts credential blob received via named pipe         │
└──────────────────────────────┬──────────────────────────────┘
                               │ Named Pipe Status Packets + Encrypted Credential Blob
                               ▼
┌─────────────────────────────────────────────────────────────┐
│  FaceUnlock.exe (C++ Native Engine)                         │
│  - OpenCV DirectShow VideoCapture                           │
│  - YuNet face detection & 5-point landmark alignment        │
│  - ArcFace 512-dimensional feature embedding extraction     │
│  - Cosine similarity matching vs embedding.dat              │
│  - Reads encrypted credentials.dat & sends to pipe on match │
│  - OS Power Throttling disabled for fast battery startup    │
└─────────────────────────────────────────────────────────────┘
                               ▲ Spawns with --mode register
                               │ & embeds window via Win32 SetParent
┌──────────────────────────────┴──────────────────────────────┐
│  FaceUnlockManager.exe (C# WinForms UI)                     │
│  - Encrypts credentials via DPAPI (LocalMachine Scope)       │
│  - Sets ACL permissions on C:\ProgramData\FaceUnlock        │
│  - Registers FaceUnlock.dll into Windows Registry           │
└──────────────────────────────┴──────────────────────────────┘
```

| Component | Language | Role |
|-----------|----------|------|
| **Credential Provider** | C++ | COM DLL loaded by `LogonUI.exe`. Displays the face unlock tile, spawns `FaceUnlock.exe`, listens for authentication status packets over a named pipe (`\\.\pipe\FaceUnlockAuth`), decrypts credential payloads, and automatically logs the user in upon success. |
| **Face Engine** | C++ | Native C++ executable. Handles camera acquisition, YuNet face detection, ArcFace embedding calculation, cosine similarity matching, and reads `credentials.dat` to transmit to the Credential Provider over named pipes upon face match. |
| **Manager UI** | C# | WinForms UI for user setup. Validates Windows credentials using `PrincipalContext`, encrypts credentials using DPAPI, manages registry keys, and embeds `FaceUnlock.exe` in registration mode. |

---

## How It Works Under the Hood

### 1. Startup Warmup Optimization (`FaceUnlockWarmup`)
To ensure near-instant startup when reaching the Windows lock screen, the installer registers a Scheduled Task (`FaceUnlockWarmup`) set to run at System Startup under the `SYSTEM` account:
- Executable: `FaceUnlock.exe --mode warmup`
- Settings: Highest Privileges (`RunLevel Highest`), allowed to run on battery power.
- Purpose: Preloads ONNX Runtime DLLs, system libraries, and model graphs directly into RAM cache during system boot. Once cached in memory, `FaceUnlock.exe` exits immediately. When you navigate to the lock screen, subsequent launches of `FaceUnlock.exe` start instantly.

### 2. Windows Credential Provider Registration
When you set up OpenFaceUnlock via `FaceUnlockManager.exe`, `InstallationManager.cs` writes the following keys into the Windows Registry:
* **Credential Provider Registration**:
  `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{B84B5619-5995-4A1E-9302-F580D593DFA6}` &rarr; `Default = "FaceUnlockProvider"`
* **COM InprocServer32 Registration**:
  `HKCR\CLSID\{B84B5619-5995-4A1E-9302-F580D593DFA6}\InprocServer32` &rarr; `Default = "FaceUnlock.dll"`, `ThreadingModel = "Apartment"`
* **Default System Provider Policy**:
  `HKLM\SOFTWARE\Policies\Microsoft\Windows\System` &rarr; `DefaultCredentialProvider = "{B84B5619-5995-4A1E-9302-F580D593DFA6}"`

### 3. Lock Screen Authentication Flow
1. **Tile Selection**: When the lock screen appears, `LogonUI.exe` loads `FaceUnlock.dll`.
2. **Pipe Server**: `CSampleCredential.cpp` starts a background thread running `NamedPipeServer()` listening on `\\.\pipe\FaceUnlockAuth`.
3. **Process Spawn**: `CSampleCredential.cpp` terminates any zombie processes and calls `CreateProcess` to launch `FaceUnlock.exe --mode login` with `HIGH_PRIORITY_CLASS` and `CREATE_NO_WINDOW`.
4. **Camera & AI Processing**:
   - `FaceUnlock.exe` disables Windows Power Throttling (`PROCESS_POWER_THROTTLING_EXECUTION_SPEED`) for high-speed execution on battery.
   - OpenCV opens the webcam via DirectShow (`CAP_DSHOW`).
   - YuNet detects faces and locates 5 facial landmarks (eyes, nose, mouth corners).
   - OpenCV performs affine transformations to crop and align the face to 112x112.
   - ArcFace calculates a 512-dimensional vector embedding.
   - Cosine distance is evaluated against the stored vector in `C:\ProgramData\FaceUnlock\embedding.dat`.
5. **Credential Retrieval & IPC Signaling**: Upon a successful face match, `FaceUnlock.exe` reads `C:\ProgramData\FaceUnlock\credentials.dat` and transmits the encrypted blob along with `STATE_SUCCESS` over `\\.\pipe\FaceUnlockAuth`.
6. **Auto Logon**: On receiving `STATE_SUCCESS`, `CredentialOP.cpp` in `FaceUnlock.dll` calls `DecryptCredentialBlob()` using `CryptUnprotectData` (`CRYPTPROTECT_LOCAL_MACHINE`), extracts the stored username/password JSON, and passes them to `LogonUI.exe` to unlock Windows.

### 4. Face Enrollment & Credential Storage
- `FaceUnlockManager` secures `C:\ProgramData\FaceUnlock` using Win32 ACLs (`System` & `Administrators` = Full Control, `Users` = Read & Execute, directory hidden).
- User credentials (username & password) `PIN not supported` are validated against Active Directory / Local SAM using `PrincipalContext.ValidateCredentials()`, serialized to JSON, and encrypted using Windows DPAPI (`DataProtectionScope.LocalMachine`). `NOTE: use your actual Microsoft Account Password or Local Account Password if you are using local account`
- During face enrollment, `RegisterFaceForm.cs` spawns `FaceUnlock.exe --mode register` and uses Win32 `SetParent` and `SetWindowLong` to seamlessly embed the C++ preview window (`FaceUnlockRegister`) directly into the WinForms control.

---

## Manual Build & Installation (For Developers)

### Prerequisites
- Visual Studio 2022 (C++ Desktop Development & .NET Desktop Development)
- OpenCV C++ SDK
- ONNX Runtime C++ SDK

### 1. Build Solution
1. Open and build `FaceUnlockManager.csproj` (Output: `FaceUnlockManager.exe`).
2. Open and build `FaceUnlock.sln` (Output: `FaceUnlock.exe`).
3. Open and build `SampleV2CredentialProvider.vcxproj` (Output: `FaceUnlock.dll`).

### 2. File Placement
Place the following files together in your installation directory (e.g., `C:\Program Files\FaceUnlock` or directory specified in `HKLM\SOFTWARE\FaceUnlock\InstallDir`):
- `FaceUnlock.dll`
- `FaceUnlock.exe`
- `FaceUnlockManager.exe`
- Model files (`yunet.onnx`, `arcface.onnx`)

### 3. Registry Installation
Run `FaceUnlockManager.exe` as Administrator to automatically apply registry settings, or execute the equivalent manual registry commands in an **Administrator Command Prompt**:
```cmd
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{B84B5619-5995-4A1E-9302-F580D593DFA6}" /ve /t REG_SZ /d "FaceUnlockProvider" /f
reg add "HKCR\CLSID\{B84B5619-5995-4A1E-9302-F580D593DFA6}" /ve /t REG_SZ /d "FaceUnlockProvider" /f
reg add "HKCR\CLSID\{B84B5619-5995-4A1E-9302-F580D593DFA6}\InprocServer32" /ve /t REG_SZ /d "FaceUnlock.dll" /f
reg add "HKCR\CLSID\{B84B5619-5995-4A1E-9302-F580D593DFA6}\InprocServer32" /v "ThreadingModel" /t REG_SZ /d "Apartment" /f
reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v "DefaultCredentialProvider" /t REG_SZ /d "{B84B5619-5995-4A1E-9302-F580D593DFA6}" /f
```

### 4. Scheduled Task Registration (Startup Warmup)
To enable fast model loading, register the `FaceUnlockWarmup` scheduled task in an **Administrator PowerShell**:
```powershell
Register-ScheduledTask -TaskName 'FaceUnlockWarmup' `
  -Trigger (New-ScheduledTaskTrigger -AtStartup) `
  -Action (New-ScheduledTaskAction -Execute 'C:\Program Files\FaceUnlock\FaceUnlock.exe' -Argument '--mode warmup') `
  -User 'SYSTEM' `
  -Settings (New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries) `
  -RunLevel Highest -Force
```

---

## Installer Notice

An automated Inno Setup installer package is available under the **Releases** section. 
> **SmartScreen Warning:** Because this installer is currently self-signed/unsigned, Windows SmartScreen may present an **"Unknown Publisher"** dialog. You can proceed by selecting **More info &rarr; Run anyway**.

---

## Credits & Open Source References

* **[OpenCV](https://github.com/opencv/opencv):** Frame capture & geometric affine face alignment.
* **[ONNX Runtime](https://github.com/microsoft/onnxruntime):** Cross-platform machine learning inferencing engine.
* **[YuNet](https://github.com/ShiqiYu/libfacedetection):** Real-time 5-landmark face detection model.
* **[ArcFace](https://github.com/deepinsight/insightface):** High-accuracy 512-d facial feature embedding model.
* **[nlohmann/json](https://github.com/nlohmann/json):** C++ JSON parsing library.
* **[Microsoft Credential Provider Samples](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/CredentialProvider):** Base COM `ICredentialProvider` implementation.

---
*License: GNU AFFERO GENERAL PUBLIC LICENSE*
