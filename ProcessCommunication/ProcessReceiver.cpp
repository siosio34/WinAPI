#include <iostream>
#include <thread>
#include <strsafe.h>
#include <Windows.h>
#include <string>
#include <stdio.h>
using namespace std;

struct ForIPCCommunication {
	char *view;
	bool Direction;
};
void WaitChat(int &_CheckDirection)
{
	bool check = true;
	string ChatContent;
	while (check) {
		cin >> ChatContent;
		_CheckDirection = 1;
	}
}

void CheckChatContent(int &_CheckDirection, ForIPCCommunication *_view)
{
	bool check = true;
	while (check)
	if (_CheckDirection == 0) {
		fprintf(stdout, "%s \n", _view);
		UnmapViewOfFile(_view);
		break;
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

	// MapViewOfFile() ȣ�� -> ���� ��ü �����͸� ���´�.
	ForIPCCommunication* view = (ForIPCCommunication*)MapViewOfFile(
		map,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		1024
		);

	if (view == NULL)
	{
		fwprintf(stderr, L"MapViewOfFile() failed.\n");
		CloseHandle(map);
		return -1;
	}










}