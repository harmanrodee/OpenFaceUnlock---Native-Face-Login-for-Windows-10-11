#pragma once

#include <vector>
#include <windows.h>

bool DecryptCredentialBlob(
    const std::vector<BYTE>& blob,
    wchar_t* username,
    size_t usernameSize,
    wchar_t* password,
    size_t passwordSize);
