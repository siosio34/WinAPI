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

	wchar_t file_name[260]; // ����
	wchar_t file_name2[260]; // ������ �����̸�

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

	if (INVALID_HANDLE_VALUE == file_handle) // ������ �������� �������
	{
		printf("err, CreateFile(%ws) failed, gle = %u", file_name, GetLastError());
		return false;
	}

	// check file size (4�Ⱑ�� �Ѿ�� �ȵ�)

	LARGE_INTEGER fileSize;
	if (TRUE != GetFileSizeEx(file_handle, &fileSize)) // ���� ����� ������ �Ұ����� ���
	{
		printf("err, GetFileSizeEx(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}
	_ASSERTE(fileSize.HighPart == 0);
	if (fileSize.HighPart > 0) // 4�Ⱑ �Ѿ���
	{
		printf("file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
			fileSize.QuadPart);
		CloseHandle(file_handle);
		return false;
	}

	// ������ ���� ����
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
	uni = L"���ȯ ����� ����~ I can give my world ";

	char *strUtf8;
	strUtf8 = new char;

	int i = 0;

	int nLen = WideCharToMultiByte(CP_UTF8, 0, uni, lstrlenW(uni), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, uni, lstrlenW(uni), strUtf8, nLen, NULL, NULL); // �޸��忡������ �����ڵ忡�� (UTF-8)�� ��ȯ�� ������ �ؾ� �ȱ���.

	WriteFile(file_handle, strUtf8, strlen(strUtf8), &dwPtr, NULL); // �����̸� 1�� ���ȯ ����� ������ ����Ѵ�.
	CloseHandle(file_handle);

	CopyFile(file_name, file_name2, 1); // ���� 1�� ������ ���� 2�� �����Ѵ�.

	file_handle = CreateFile( // ���� 2�� �ڵ��� ����.
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
	ReadFile(file_handle, temp, dwFileSize, &dwPtr, NULL); // ���� 2�� ������ �о� temp �� �����Ѵ�.
	CloseHandle(file_handle); // �ڵ��� ����� �ݾ����������� ������ �߻���.

	wchar_t strUni[256] = { 0, };

	// utf-8 -> unicode
	nLen = MultiByteToWideChar(CP_UTF8, 0, temp, strlen(temp), NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, temp, strlen(temp), strUni, nLen);

	// unicode -> multibyte �ܼ�â�� ����Ҷ��� ��Ƽ����Ʈ
	char strMultibyte2[256] = { 0, };
	nLen = WideCharToMultiByte(CP_ACP, 0, strUni, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strUni, -1, strMultibyte2, nLen, NULL, NULL);

	printf("%s", strMultibyte2);
	buf = NULL;

	DeleteFileW(file_name);
	DeleteFileW(file_name2);
	
	return 0;

}