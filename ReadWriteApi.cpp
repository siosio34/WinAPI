#pragma once
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <Winerror.h>
#include <stdint.h>
#include <Strsafe.h>
#include <tchar.h>
#include <locale.h>
#include <Stringapiset.h>
using namespace std;


int main()
{
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	DWORD dwPtr;
	DWORD dwFileSize;

	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}
	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	wchar_t file_name[260]; // 원본
	wchar_t file_name2[260]; // 복사할 파일이름

	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf)))
	{
		printf("err, can not create file name");
		free(buf);
		return false;
	}

	HANDLE file_handle = CreateFile(
		(LPCWSTR)file_name,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		CREATE_ALWAYS,
		0,
		NULL
		);

	if (INVALID_HANDLE_VALUE == file_handle) // 파일이 생성되지 않을경우
	{
		printf("err, CreateFile(%ws) failed, gle = %u", file_name, GetLastError());
		return false;
	}

	// check file size (4기가가 넘어가면 안됨)

	LARGE_INTEGER fileSize;
	if (TRUE != GetFileSizeEx(file_handle, &fileSize)) // 파일 사이즈가 측정이 불가능할 경우
	{
		printf("err, GetFileSizeEx(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}
	_ASSERTE(fileSize.HighPart == 0);
	if (fileSize.HighPart > 0) // 4기가 넘어갈경우
	{
		printf("file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
			fileSize.QuadPart);
		CloseHandle(file_handle);
		return false;
	}

	// 복사할 파일 생성
	if (!SUCCEEDED(StringCbPrintfW(
		file_name2,
		sizeof(file_name2),
		L"%ws\\bob2.txt",
		buf)))
	{
		printf("err, can not create file name");
		free(buf);
		return false;
	}


	wchar_t *uni;
	uni = new wchar_t;
	uni = L"노용환 멘토님 만세~ I can give my world ";

	char *strUtf8;
	strUtf8 = new char;

	int i = 0;

	int nLen = WideCharToMultiByte(CP_UTF8, 0, uni, lstrlenW(uni), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, uni, lstrlenW(uni), strUtf8, nLen, NULL, NULL); // 메모장에쓸때는 유니코드에서 (UTF-8)로 변환후 저장을 해야 안깨짐.

	WriteFile(file_handle, strUtf8, strlen(strUtf8), &dwPtr, NULL); // 파일이름 1에 노용환 멘토님 만세를 사용한다.
	CloseHandle(file_handle);

	CopyFile(file_name, file_name2, 1); // 파일 1의 내용을 파일 2에 복사한다.

	file_handle = CreateFile( // 파일 2의 핸들을 얻음.
		(LPCWSTR)file_name2,
		GENERIC_ALL,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	char temp[256] = { 0, };

	dwFileSize = GetFileSize(file_handle, NULL);
	ReadFile(file_handle, temp, dwFileSize, &dwPtr, NULL); // 파일 2의 내용을 읽어 temp 에 저장한다.
	CloseHandle(file_handle); // 핸들을 제대로 닫아주지않으면 오류가 발생함.

	wchar_t strUni[256] = { 0, };

	// utf-8 -> unicode
	nLen = MultiByteToWideChar(CP_UTF8, 0, temp, strlen(temp), NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, temp, strlen(temp), strUni, nLen);

	// unicode -> multibyte 콘솔창에 출력할때는 멀티바이트
	char strMultibyte2[256] = { 0, };
	nLen = WideCharToMultiByte(CP_ACP, 0, strUni, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strUni, -1, strMultibyte2, nLen, NULL, NULL);

	printf("%s", strMultibyte2);
	buf = NULL;

	DeleteFileW(file_name);
	DeleteFileW(file_name2);
	
	return 0;

}