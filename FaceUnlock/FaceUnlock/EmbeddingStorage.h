#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

struct CredentialData
{
    std::wstring username;
    std::wstring password;
};

bool LoadCredentials(CredentialData& credentials);

bool ReadCredFile(std::vector<BYTE>& bytes);

bool loadEmbedding(std::vector<float>& embedding);

bool saveEmbedding(std::vector<float> embedding);