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

bool create_big_file()
{
	HANDLE hFile;
	DWORD dwCurrentFilePosition;

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

	wchar_t file_name[260];

	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\test.txt",
		buf)))
	{
		printf("err, can not create file name");
		free(buf);
		return false;
	}

	HANDLE file_handle = CreateFile(
		(LPCWSTR)file_name,
		GENERIC_WRITE,
		NULL,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	uint32_t u32 = (1024 * 1024 * 1024) * 2;
	LONGLONG li = (LONGLONG)u32;
	LARGE_INTEGER size;
	size.QuadPart = li;

	SetFilePointerEx(file_handle, size, 0, FILE_BEGIN);
	SetEndOfFile(file_handle);
	CloseHandle(file_handle);

}
int main()
{
	
	create_big_file();
	return 0;
}