#include <iostream>
#include <thread>
#include <strsafe.h>
#include <Windows.h>
#include <string>
#include <stdio.h>
using namespace std;

struct ForIPCCommunication {
	char view[1024];
	int Direction;
};

void WaitChat(struct ForIPCCommunication *SharedMemory)
{
	bool check = true;
	string ChatContent;
	while (check) {
		cin >> ChatContent;
		StringCbPrintfA(SharedMemory->view, 1024, "%s", ChatContent.c_str());
		SharedMemory->Direction = 0;
	}
}

void CheckChatContent(struct ForIPCCommunication *SharedMemory)
{
	bool check = true;
	while (check) {
		if (SharedMemory->Direction == 1) {

			fprintf(stdout, "%s \n", SharedMemory->view);
			SharedMemory->Direction = -1;
			//UnmapViewOfFile(SharedMemory->view);

		}
	}
}

int main()
{
	int CheckDirection = 0;
	HANDLE map = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		1024,
		L"bob4_mil"
		);

	if (map == NULL)
	{
		fwprintf(stderr, L"CreateFileMapping() failed.\n");
		return -1;
	}

	// MapViewOfFile() 호출 -> 섹션 객체 포인터를 얻어온다.
	ForIPCCommunication* Shared = (ForIPCCommunication*)MapViewOfFile(
		map,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		1024
		);

	Shared->Direction = -1;

	if (Shared == NULL)
	{
		fwprintf(stderr, L"MapViewOfFile() failed.\n");
		CloseHandle(map);
		return -1;
	}

	std::thread t1(WaitChat, Shared);
	std::thread t2(CheckChatContent, Shared);

	t1.join();
	t2.join();

	return 0;
}

