#include "EmbeddingStorage.h"
#include <Windows.h>
#include <wincrypt.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <ShlObj.h>
#include <filesystem>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Crypt32.lib")

static const char HEADER[4] = { 'F', 'A', 'C', 'E' };
static const std::string embeddingPath = "C:\\ProgramData\\FaceUnlock\\";
static const std::string dataPath = "C:\\ProgramData\\FaceUnlock";

bool ReadCredFile(std::vector<BYTE>& bytes) {
	std::wstring path = L"C:\\ProgramData\\FaceUnlock\\credentials.dat";
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		std::cout << "cred file not found" << std::endl;
		std::ofstream log3("C:\\poc\\readcredfunc.txt", std::ios::app);
		log3 << "cred file not located " << std::endl;
		log3.close();
		std::wofstream log(L"C:\\poc\\path_log.txt", std::ios::app);

		log << L"Path = " << path << std::endl;

		log.close();
		return false;
	}
	file.seekg(0, std::ios::end);

	size_t size = (size_t)file.tellg();

	file.seekg(0);

	bytes.resize(size);

	file.read(reinterpret_cast<char*>(bytes.data()), size);

	return true;
}

bool saveEmbedding(std::vector<float> embedding, std::string type)
{
	DATA_BLOB inputBlob;
	DATA_BLOB outputBlob;

	inputBlob.pbData =
		reinterpret_cast<BYTE*>(embedding.data());

	inputBlob.cbData =
		static_cast<DWORD>(embedding.size() * sizeof(float));

	if (!CryptProtectData(
		&inputBlob,
		L"FaceUnlockEmbedding",
		NULL,
		NULL,
		NULL,
		CRYPTPROTECT_LOCAL_MACHINE,
		&outputBlob))
	{
		std::cout << "Encryption failed\n";
		return false;
	}

	std::filesystem::create_directories(dataPath);
	std::ofstream file(embeddingPath + "embedding_" + type + ".dat", std::ios::binary);

	if (!file.is_open())
	{
		LocalFree(outputBlob.pbData);
		return false;
	}

	file.write(HEADER, sizeof(HEADER));

	DWORD blobSize = outputBlob.cbData;

	file.write(
		reinterpret_cast<char*>(&blobSize),
		sizeof(blobSize)
	);

	file.write(
		reinterpret_cast<char*>(outputBlob.pbData),
		blobSize
	);

	LocalFree(outputBlob.pbData);

	return true;
}

//BUG: is loading 4 embeddings
//so far known that the for loop is only running 2 times
bool loadEmbedding(std::vector <std::vector<float>>& embeddings){
	std::cout << "Loaded embeddings{before}: " << embeddings.size() << std::endl;
	std::vector<std::string> files = { "embedding_main.dat", "embedding_alt.dat" };
	for (int i = 0; i < files.size(); i++) {
		std::cout << "Loading embedding file: " << files[i] << std::endl;
		std::ifstream file(embeddingPath + files[i], std::ios::binary);

		if (!file.is_open())
		{
			continue;
		}

		char fileHeader[sizeof(HEADER)];

		file.read(fileHeader, sizeof(HEADER));

		if (memcmp(fileHeader, HEADER, sizeof(HEADER)) != 0)
		{
			std::cout << "Invalid file\n";
			return false;
		}

		DWORD blobSize;

		file.read(
			reinterpret_cast<char*>(&blobSize),
			sizeof(blobSize)
		);

		std::vector<BYTE> encrypted(blobSize);

		file.read(
			reinterpret_cast<char*>(encrypted.data()),
			blobSize
		);

		DATA_BLOB inputBlob;
		DATA_BLOB outputBlob;

		inputBlob.pbData = encrypted.data();
		inputBlob.cbData = blobSize;

		if (!CryptUnprotectData(
			&inputBlob,
			NULL,
			NULL,
			NULL,
			NULL,
			CRYPTPROTECT_LOCAL_MACHINE,
			&outputBlob))
		{
			std::cout << "Decryption failed\n";
			return false;
		}

		std::vector<float> embedding( 512 );

		memcpy(
			embedding.data(),
			outputBlob.pbData,
			outputBlob.cbData
		);

		embeddings.push_back(embedding);

		LocalFree(outputBlob.pbData);
	}

	std::cout << "Loaded embeddings {after}: " << embeddings.size() << std::endl;
	return true;
}