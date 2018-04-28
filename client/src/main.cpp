#include "precompiled.h"

#define SERVER_IP		"127.0.0.1"
#define SERVER_PORT		666

#define THREAD_WRITE	0
#define THREAD_LISTEN	1

#define MAX_LEN_NAME	32
#define MAX_LEN_MESSAGE	256

int main()
{
	printf("************************************\n");
	printf("\tTCP CLIENT STARTED\n");
	printf("************************************\n");

	WSADATA wsData;

	if (WSAStartup(0x0202, &wsData) == SOCKET_ERROR)
	{
		printf("* [Error] Library \'Windows Sockets\' could not load.\n");
		printf("* [Error] Error: %d\n", WSAGetLastError());

		WSACleanup();

		system("pause");

		return 1;
	}

	printf("* Library \'Windows Sockets\' is loaded.\n");

	SOCKET server_desc;

	if ((server_desc = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("* [Error] Socket was not created.\n");
		printf("* [Error] Error: %d\n", WSAGetLastError());

		WSACleanup();

		system("pause");

		return 1;
	}

	printf("* Socket is was created.\n");

	sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	if (connect(server_desc, (sockaddr *)&server_addr, sizeof(server_addr)))
	{
		printf("* [Error] Client could not connect.\n");
		printf("* [Error] Error: %d\n", WSAGetLastError());

		WSACleanup();

		system("pause");

		return 1;
	}

	printf("* Connection with \'%s\' successfully.\n", SERVER_IP);

	HANDLE hThread[2];

	hThread[THREAD_WRITE] = CreateThread(NULL, NULL, WriteToServer, &server_desc, NULL, NULL);
	hThread[THREAD_LISTEN] = CreateThread(NULL, NULL, ListenServer, &server_desc, NULL, NULL);

	switch (WaitForMultipleObjects(2, hThread, false, INFINITE) - WAIT_OBJECT_0)
	{
		case THREAD_WRITE:
		{
			TerminateThread(hThread[THREAD_WRITE], 0);

			printf("* You has disconnected.\n");

			break;
		}
		case THREAD_LISTEN:
		{
			TerminateThread(hThread[THREAD_LISTEN], 0);

			printf("* The server shut down.\n");

			break;
		}
	}

	closesocket(server_desc);

	WSACleanup();

	system("pause");

	return 0;
}

DWORD WINAPI ListenServer(LPVOID lpServerDesc)
{
	SOCKET server_desc = *(SOCKET *)lpServerDesc;

	int size = 0;

	char buffer[MAX_LEN_MESSAGE] = { 0 };

	while(true)
	{
		if ((size = recv(server_desc, &buffer[0], sizeof(buffer) - 1, 0)) != SOCKET_ERROR)
		{
			buffer[size] = 0;

			printf("%s", buffer);
		}
		else
		{
			break;
		}
	}

	return 0;
}

DWORD WINAPI WriteToServer(LPVOID lpServerDesc)
{
	SOCKET server_desc = *(SOCKET *)lpServerDesc;

	char message[MAX_LEN_MESSAGE] = { 0 };

	while(true)
	{
		do
		{
			fflush(stdin);
			gets(message);
		} while (strlen(message) <= 0);

		if (message[0] == 'e' && message[3] == 't')
		{
			break;
		}

		send(server_desc, message, strlen(message), 0);
	}

	return 0;
}