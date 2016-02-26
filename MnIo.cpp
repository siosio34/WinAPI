#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <Winerror.h>
#include <stdint.h>
#include <Strsafe.h>
using namespace std;

int main()
{
	wchar_t *buf = NULL;
	uint32_t buflen = 0;

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
	wchar_t file_name2[260];

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

	free(buf); 
	buf = NULL;

	HANDLE file_handle = CreateFile(
		(LPCWSTR)file_name,
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);

	if (INVALID_HANDLE_VALUE == file_handle)
	{
		printf("err, CreateFile(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	// check file size

	LARGE_INTEGER fileSize;
	if (TRUE != GetFileSizeEx(file_handle, &fileSize))
	{
		printf("err, GetFileSizeEx(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	_ASSERTE(fileSize.HighPart == 0);
	if (fileSize.HighPart > 0)
	{
		printf("file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
			fileSize.QuadPart);
		CloseHandle(file_handle);
		return false;
	}

	DWORD file_size = (DWORD)fileSize.QuadPart;

	HANDLE file_map = CreateFileMapping( //파일이름 1 객체 맵핑
		file_handle,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL
		);

	if (NULL == file_map) // 매핑이 안됬을시
	{
		printf("err, CreateFileMapping(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	PCHAR file_view = (PCHAR)MapViewOfFile( //매핑한걸 file_view로 전달
		file_map,
		FILE_MAP_READ,
		0,
		0,
		0
		);

	if (file_view == NULL) // 매핑한걸 못읽었을시
	{
		printf("err, MapViewOfFile(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_map);
		CloseHandle(file_handle);
		return false;
	}

	wchar_t strUnicode[256] = { 0, };
	char mulcode[256] = { 0, };

	// utf-8 -> unicode -> multibyte
	MultiByteToWideChar(CP_UTF8, 0, file_view, -1, strUnicode, 256);
	WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, mulcode, 256, NULL, NULL);
	
	printf("%s", mulcode);

	UnmapViewOfFile(file_view); // 매핑해제
	CloseHandle(file_map);
	CloseHandle(file_handle);

}
