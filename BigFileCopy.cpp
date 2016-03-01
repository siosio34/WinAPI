#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <Winerror.h>
#include <Strsafe.h>
using namespace std;

typedef struct map_context
{
	HANDLE  handle;
	LARGE_INTEGER size;
	HANDLE  map;
	PBYTE  view;

}*pmap_context;

bool is_file_existsW(_In_ const wchar_t* file_path)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ((NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH))) return false;

	WIN32_FILE_ATTRIBUTE_DATA info = { 0 };

	if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &info) == 0)
		return false;
	else
		return true;

}

void close_map_context(_In_ pmap_context ctx)
{
	if (NULL != ctx)
	{
		if (NULL != ctx->view) UnmapViewOfFile(ctx->view);
		if (NULL != ctx->map) CloseHandle(ctx->map);
		if (INVALID_HANDLE_VALUE != ctx->handle) CloseHandle(ctx->handle);
		free(ctx); ctx = NULL;
	}
}

bool create_very_big_file()
{
	// current directory 를 구한다.
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);

	if (0 == buflen)
	{
		printf("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ test.txt 파일명 생성
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob4.mp3",
		buf)))
	{
		printf("err ] can not create file name");
		free(buf);
		return false;
	}

	free(buf); buf = NULL;



	_ASSERTE(NULL != file_name);
	HANDLE file_handle = CreateFile(
		file_name,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if (INVALID_HANDLE_VALUE == file_handle)
	{
		printf("err ] CreateFile( %ws ) failed. gle = %u", file_name, GetLastError());
		return false;
	}

	int temp = 5;
	LARGE_INTEGER file_size = { 0 };
	uint32_t u32;
	LONGLONG li;

	do
	{
		if (temp >= 4)
		{
			u32 = (uint32_t)((1024 * 1024 * 1024) * 3);
			temp = temp - 3;
			li = (LONGLONG)(u32);
			file_size.QuadPart = li;
			if (!SetFilePointerEx(file_handle, file_size, NULL, FILE_CURRENT))
			{
				printf("err ] SetFilePointerEx() failed. gle = %u", GetLastError());
				CloseHandle(file_handle);
				return false;
			}
			SetEndOfFile(file_handle);

		}
		else
		{
			u32 = ((1024 * 1024 * 1024) * temp);
			li = (LONGLONG)(u32);
			file_size.QuadPart = li;
			if (!SetFilePointerEx(file_handle, file_size, NULL, FILE_CURRENT))
			{
				printf("err ] SetFilePointerEx() failed. gle = %u", GetLastError());

				//CloseHandle(file_handle);
				return false;
			}
			SetEndOfFile(file_handle);
			temp = -1;
		}
		//SetEndOfFile(file_handle);

	} while (temp >= 0);

	CloseHandle(file_handle);
	return true;
}

bool MnIOCopy()
{
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	LARGE_INTEGER fileSize;

	pmap_context ctx = (pmap_context)malloc(sizeof(map_context));
	RtlZeroMemory(ctx, sizeof(map_context));

	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	DWORD dwBytesInBlock = 1000 * sinf.dwAllocationGranularity;

	DWORD dwFileSizeHigh; // Big File 사이즈 구하기
	__int64 qwFileSize = GetFileSize(ctx->handle, &dwFileSizeHigh);
	qwFileSize += (((__int64)dwFileSizeHigh) << 32);
	__int64 qwFileOffset = 0;

	bool ret = false;

	if (0 == buflen)
	{
		printf("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ test.txt 파일명 생성
	wchar_t file_path[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_path,
		sizeof(file_path),
		L"%ws\\bob4.mp3",
		buf)))
	{
		printf("err ] can not create file name");
		free(buf);
		return false;
	}

	_ASSERTE(NULL != file_path);
	if (NULL == file_path) return false;



#pragma warning(disable: 4127)

	do
	{
		ctx->handle = CreateFile(
			(LPCWSTR)file_path,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (INVALID_HANDLE_VALUE == ctx->handle)
		{
			printf("err ] CreateFile( %ws ) failed. gle = %u", file_path, GetLastError());
			break;
		}

		DWORD dwFileSizeHigh; // Big File 사이즈 구하기
		__int64 qwFileSize = GetFileSize(ctx->handle, &dwFileSizeHigh);
		qwFileSize += (((__int64)dwFileSizeHigh) << 32);
		__int64 qwFileOffset = 0;

		ctx->map = CreateFileMapping(
			ctx->handle,
			NULL,
			PAGE_READWRITE,
			0,
			0,
			NULL
			);

		if (NULL == ctx->map)
		{
			printf("err ] CreateFileMapping( %ws ) failed. gle = %u", file_path, GetLastError());
			break;
		}

		CloseHandle(ctx->handle); // 핸들을 다 사용했으므로 닫아준다.

		pmap_context ctx2 = (pmap_context)malloc(sizeof(map_context));
		RtlZeroMemory(ctx2, sizeof(map_context));

		wchar_t file_name2[260];
		if (!SUCCEEDED(StringCbPrintfW(
			file_name2,
			sizeof(file_name2),
			L"%ws\\bob43.mp3",
			buf)))
		{
			printf("err ] can not create file name");
			free(buf);
			return false;
		}

		free(buf); buf = NULL;

		_ASSERTE(NULL != file_name2);
		if (NULL == file_name2) return false;


		ctx2->handle = CreateFile(
			(LPCWSTR)file_name2,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
			NULL
			);

		if (INVALID_HANDLE_VALUE == ctx2->handle)
		{
			printf("err ] CreateFile( %ws ) failed. gle = %u", file_name2, GetLastError());
			return false;
		}

		ctx2->size.QuadPart = qwFileSize;
		ctx2->map = CreateFileMapping(
			ctx2->handle,
			NULL,
			PAGE_READWRITE,
			ctx2->size.HighPart,
			ctx2->size.LowPart,
			NULL
			);
		
		if (NULL == ctx2->map)
		{
			printf("err ] CreateFileMapping( %ws ) failed. gle = %u", file_name2, GetLastError());
			return false;
		}

		DWORD bytes_written;
		while (qwFileSize > 0)
		{
			if (qwFileSize < dwBytesInBlock)
				dwBytesInBlock = qwFileSize;

			ctx->view = (PBYTE)MapViewOfFile(ctx->map, FILE_MAP_READ,
				(DWORD)(qwFileOffset >> 32),  // 상위 오프셋
				(DWORD)(qwFileOffset & 0xFFFFFFFF), // 하위 오프셋
				dwBytesInBlock);

			ctx2->view = (PBYTE)MapViewOfFile(ctx2->map, FILE_MAP_WRITE,
				(DWORD)(qwFileOffset >> 32),  // 상위 오프셋
				(DWORD)(qwFileOffset & 0xFFFFFFFF), // 하위 오프셋
				dwBytesInBlock);

			memcpy(ctx2->view, ctx->view, dwBytesInBlock);


			//WriteFile(ctx2->handle, ctx->view, dwBytesInBlock, &bytes_written, NULL);
			UnmapViewOfFile(ctx->view);
			UnmapViewOfFile(ctx2->view);

			qwFileOffset += dwBytesInBlock;
			qwFileSize -= dwBytesInBlock;
		}
		
		ret = true;

#pragma warning(default: 4127)

	} while (false);

	if (!ret) close_map_context(ctx);

}



bool file_save_Readfile()
{
	// current directory 를 구한다.
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);


	if (0 == buflen)
	{
		printf("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ test.txt 파일명 생성
	wchar_t src_file[260];
	wchar_t dst_file[260];
	if (!SUCCEEDED(StringCbPrintfW(
		src_file,
		sizeof(src_file),
		L"%ws\\bob4.mp3",
		buf)))
	{
		printf("err ] can not create file name");
		free(buf);
		return false;
	}

	if (!SUCCEEDED(StringCbPrintfW(
		dst_file,
		sizeof(dst_file),
		L"%ws\\bob42.mp3",
		buf)))
	{
		printf("err ] can not create file name");
		free(buf);
		return false;
	}

	free(buf); buf = NULL;

	_ASSERTE(NULL != src_file);
	_ASSERTE(NULL != dst_file);
	if (NULL == src_file || NULL == dst_file) return false;

	if (!is_file_existsW(src_file))
	{
		printf("err ] no src file = %ws", src_file);
		return false;
	}

	HANDLE src_handle = CreateFileW(
		src_file,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (INVALID_HANDLE_VALUE == src_handle)
	{
		printf("err ] CreateFile( %ws ) failed. gle = %u", src_file, GetLastError());
		return false;
	}

	// open dst file with WRITE mode
	HANDLE dst_handle = CreateFileW(
		dst_file,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if (INVALID_HANDLE_VALUE == dst_handle)
	{
		printf("err ] CreateFile( %ws ) failed. gle = %u", dst_file, GetLastError());

		CloseHandle(src_handle);
		return false;
	}

	// file copy
	bool ret = false;
	char _buf[1000000] = { 0 };
	DWORD bytes_written = 0;
	DWORD bytes_read = 0;
	bool bResult;

	do
	{
		// read from src
		bResult = ReadFile(src_handle, _buf, sizeof(_buf), &bytes_read, NULL);

		if (!bResult)
		{
			printf("err ] ReadFile( src_handle ) failed. gle = %u", GetLastError());
			break;
		}

		// write to dst
		if (!WriteFile(dst_handle, _buf, bytes_read, &bytes_written, NULL))
		{
			printf("err ] WriteFile( dst_handle ) failed. gle = %u", GetLastError());
			break;
		}

		if (bResult && bytes_read == 0)
			break;

	} while (true);

	CloseHandle(src_handle);
	CloseHandle(dst_handle);
	return ret;
}


int main()
{

	//create_very_big_file(); - 5GB 파일 데이터 생성
	//file_save_Readfile(); - ReadFile API로 5GB 파일 데이터 읽기 /복사
	//create_map_context(); - MNIO 로 5GB 파일 데이터 읽기

	MnIOCopy(); // Mnio Api
	file_save_Readfile(); // 많이 느립니다. Read, Write Api

	cout << " 끄읕 " << endl;
	return 1;
}



