#include "precompiled.h"

#define MAX_USERS		32

#define SERVER_PORT		666

#define MAX_LEN_NAME	32
#define MAX_LEN_MESSAGE	256

int g_iClients;

typedef struct client_info_s
{
	int index;
	char name[MAX_LEN_NAME];
	char message[MAX_LEN_MESSAGE];
	SOCKET client_desc;

	bool auth;
} CLIENT;

CLIENT ClientData[MAX_USERS];

int main()
{
	printf("************************************\n");
	printf("\tTCP SERVER STARTED\n");
	printf("************************************\n");
	
	g_iClients = 0;

	WSADATA wsData;

	if(WSAStartup(0x0202, &wsData) == SOCKET_ERROR)
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
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(server_desc, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		printf("* [Error] Socket was not binding.\n");
		printf("* [Error] Error: %d\n", WSAGetLastError());

		closesocket(server_desc);

		WSACleanup();

		system("pause");

		return 1;
	}

	printf("* Server is binding\n");

	if (listen(server_desc, MAX_USERS))
	{
		printf("* [Error] Socket was not binding.\n");
		printf("* [Error] Error: %d\n", WSAGetLastError());

		closesocket(server_desc);

		WSACleanup();

		system("pause");

		return 1;
	}

	printf("* Server is ready.\n");

	SOCKET client_desc;

	sockaddr_in client_addr;
	int client_addr_size = sizeof(client_addr);

	char client_ip[16] = { 0 };
	ULONG address;

	HOSTENT *host = nullptr;

	while ((client_desc = accept(server_desc, (sockaddr *)&client_addr, &client_addr_size)))
	{
		strcpy(client_ip, inet_ntoa(client_addr.sin_addr));

		address = inet_addr(client_ip);

		host = gethostbyaddr((char *)&address, sizeof(address), AF_INET);

		ClientData[g_iClients].index = g_iClients;
		ClientData[g_iClients].name[0] = '\0';
		ClientData[g_iClients].message[0] = '\0';
		ClientData[g_iClients].client_desc = client_desc;
		ClientData[g_iClients].auth = false;

		HANDLE hClient;
		hClient = CreateThread(NULL, NULL, ListenClient, &ClientData[g_iClients++], NULL, NULL);
	}

	if(WSACleanup())
	{
		printf("* Library \'Windows Sockets\' could not unload.\n");
	}
	else
	{
		printf("* Library \'Windows Sockets\' is unload.\n");
	}

	closesocket(server_desc);

	WSACleanup();

	system("pause");

	return 0;
}

DWORD WINAPI ListenClient(LPVOID lpClientData)
{
	CLIENT *client = (CLIENT *)lpClientData;

	int size = 0;

	char message[MAX_LEN_MESSAGE] = { 0 };

	send(client->client_desc, "[SERVER] Connected to chat. What's you name: ", strlen("[SERVER] Connected to chat. What's you name: "), 0);

	if ((size = recv(client->client_desc, message, sizeof(message) - 1, 0)) != SOCKET_ERROR)
	{
		if (client->name[0] == '\0')
		{
			message[size] = 0;

			strcpy(client->name, message);

			client->auth = true;

			sprintf(message, "[SERVER] Hello, %s. Have fun!\n", client->name);

			send(client->client_desc, message, strlen(message), 0);

			printf("* %s is connected!\n", client->name);
		}
	}
	else
	{
		closesocket(client->client_desc);

		memset(&client, 0, sizeof(CLIENT));

		g_iClients--;

		return 0;
	}

	while(true)
	{
		if ((size = recv(client->client_desc, message, sizeof(message) - 1, 0)) == SOCKET_ERROR)
		{
			printf("* %s has disconnected.\n", client->name);
			
			closesocket(client->client_desc);
			
			memset(client, 0, sizeof(CLIENT));

			g_iClients--;

			break;
		}

		message[size] = 0;

		printf("%s is write \'%s\'\n", client->name, message);

		if(g_iClients > 1)
		{
			HANDLE hClient;

			for(int i = 0; i < g_iClients; i++)
			{
				if(i == client->index || !ClientData[i].auth)
				{
					continue;
				}

				sprintf(ClientData[i].message, "%s: %s\n", client->name, message);
				ClientData[i].message[strlen(client->name) + strlen(message) + 3] = 0;

				hClient = CreateThread(NULL, NULL, WriteClient, &ClientData[i], NULL, NULL);
			}
		}
	}

	return 0;
}

DWORD WINAPI WriteClient(LPVOID lpClientData)
{
	CLIENT client = *(CLIENT *)lpClientData;

	send(client.client_desc, client.message, strlen(client.message), 0);

	return 0;
}