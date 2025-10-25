#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"
#define SERVER_ADDR "127.0.0.1" //IPv4»Ø»·µØÖ·

void GetCurrentTimeString(char* buffer, size_t bufferSize) {
	SYSTEMTIME st;
	GetLocalTime(&st);

	sprintf_s(buffer, bufferSize, "%02d:%02d:%02d.%03d",
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}


int __cdecl main(int argc, char **argv) 
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	const char *sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	const unsigned int timeBufferLen = 64;
	char timeBuffer[timeBufferLen];

	/*
	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}
	*/

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("WSAStartup failed with error: %d\n", iResult);
		_getch();
		return 1;
	}

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(SERVER_ADDR, DEFAULT_PORT, &hints, &result);
	if ( iResult != 0 ) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		_getch();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			_getch();
			return 1;
		}

		// Connect to server.
		iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("socket failed with error: SOCKET_ERROR\n");
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("Unable to connect to server!\n");
		WSACleanup();
		_getch();
		return 1;
	}

	// Send an initial buffer
	iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
	if (iResult == SOCKET_ERROR) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		_getch();
		return 1;
	}

	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
		printf("%s\t", timeBuffer);
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		_getch();
		return 1;
	}

	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("Bytes received: %d\n", iResult);
		}
		else if (iResult == 0) {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("Connection closed\n");
		}
		else {
			GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
			printf("%s\t", timeBuffer);
			printf("recv failed with error: %d\n", WSAGetLastError());
		}

	} while( iResult > 0 );

	// cleanup
	GetCurrentTimeString(timeBuffer, sizeof(timeBuffer));
	printf("%s\t", timeBuffer);
	printf("Client closed\n");
	closesocket(ConnectSocket);
	WSACleanup();
	_getch();
	return 0;
}