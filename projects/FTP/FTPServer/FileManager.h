#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

class FileManager {
public:
	FileManager(std::wstring directoryPath){
		mDirectoryPath = directoryPath;
		FetchFileList();
	}

	LONGLONG GetFileSize(const std::wstring& fileName) {
		std::wstring fullPath = mDirectoryPath + L"\\" + fileName;

		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (!GetFileAttributesExW(fullPath.c_str(), GetFileExInfoStandard, &fad)) {
			return -1;
		}

		LARGE_INTEGER size;
		size.LowPart = fad.nFileSizeLow;
		size.HighPart = fad.nFileSizeHigh;

		return size.QuadPart;
	}

	bool FetchFileList() {
		fileList.clear();
		std::wstring searchPath = mDirectoryPath + L"\\*";

		WIN32_FIND_DATAW fileData;
		HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fileData);

		if (hFind == INVALID_HANDLE_VALUE) {
			return false;
		}

		do {
			if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				fileList.push_back(fileData.cFileName);
			}
		} while (FindNextFileW(hFind, &fileData));

		FindClose(hFind);
		return true;
	}

	bool GetSerializedFileListForNet(std::vector<char>& outData) {
		if (fileList.empty()) return false;

		// 1. 모든 파일명을 하나의 wstring으로 결합 (구분자 '|')
		std::wstring combined;
		for (size_t i = 0; i < fileList.size(); ++i) {
			combined += fileList[i];
			if (i < fileList.size() - 1) combined += L"|";
		}

		// 2. UTF-8로 변환 시 필요한 버퍼 크기 계산
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, combined.c_str(), (int)combined.size(), NULL, 0, NULL, NULL);
		if (sizeNeeded <= 0) return false;

		// 3. 출력 벡터 크기 조정 및 변환
		outData.resize(sizeNeeded);
		WideCharToMultiByte(CP_UTF8, 0, combined.c_str(), (int)combined.size(), outData.data(), sizeNeeded, NULL, NULL);

		return true;
	}

	std::unique_ptr<char[]> ReadWithFixedBuffer(const std::wstring& fileName, LONGLONG offset, DWORD bufferSize, DWORD& outBytesRead) {
		std::wstring fullPath = mDirectoryPath + L"\\" + fileName;
		outBytesRead = 0;

		HANDLE hFile = CreateFileW(
			fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
		);

		if (hFile == INVALID_HANDLE_VALUE) return nullptr;

		// 포인터 이동
		LARGE_INTEGER liOffset;
		liOffset.QuadPart = offset;
		SetFilePointerEx(hFile, liOffset, NULL, FILE_BEGIN);

		// 고정 크기 버퍼 할당 (서버 IO 효율성을 위해 heap 할당)
		auto buffer = std::make_unique<char[]>(bufferSize);

		// 읽기 수행
		if (!ReadFile(hFile, buffer.get(), bufferSize, &outBytesRead, NULL)) {
			outBytesRead = 0;
			buffer.reset();
		}

		CloseHandle(hFile);
		return buffer; // 스마트 포인터를 반환하여 메모리 누수 방지
	}
private:
	std::vector<std::wstring> fileList;
	std::wstring mDirectoryPath;
};