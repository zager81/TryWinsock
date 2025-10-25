#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"

void GetCurrentTimeString(char* buffer, size_t bufferSize) {
	SYSTEMTIME st;
	GetLocalTime(&st);

	sprintf_s(buffer, bufferSize, "%02d:%02d:%02d.%03d",
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

int __cdecl main(void) 
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	const unsigned int timeBufferLen = 64;
	char timeBuffer[timeBufferLen];

	// 1. Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("WSAStartup failed with error: %d\n", iResult);
		_getch();
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// 2. Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if ( iResult != 0 ) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		_getch();
		return 1;
	}

	// 3. Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		_getch();
		return 1;
	}

	// 4. Setup the TCP listening socket
	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		_getch();
		return 1;
	}

	freeaddrinfo(result);

	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("Listening...\n");
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		_getch();
		return 1;
	}

	// 5. Accept a client socket
	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("Accepting...\n");
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		_getch();
		return 1;
	}

	// 6. No longer need server socket
	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("ListenSocket closed\n", WSAGetLastError());
	closesocket(ListenSocket);

	// 7. Receive until the peer shuts down the connection
	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("Receiving...\n");
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
			if (iSendResult == SOCKET_ERROR) {
				GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
				printf("%s\t", timeBuffer);
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				_getch();
				return 1;
			}
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0) {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("Connection closing...\n");
		}
		else  {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			_getch();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("Shutting down...\n");
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		_getch();
		return 1;
	}

	// cleanup
	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("Server closed\n");
	closesocket(ClientSocket);
	WSACleanup();
	_getch();
	return 0;
}