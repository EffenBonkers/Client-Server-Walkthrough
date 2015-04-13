// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define DEFAULT_PORT    "27015"
#define BUFFERLEN       256

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
    int iResult;
    struct addrinfo *ptr = NULL, *result = NULL, hints;
    SOCKET ConnectionSocket = INVALID_SOCKET;
    char *sendbuf = "This is a test.\n";
    char rcvbuf[BUFFERLEN];

    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    
    ptr = result;

    ConnectionSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (ConnectionSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = connect(ConnectionSocket, ptr->ai_addr, ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectionSocket);
        ConnectionSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ConnectionSocket == INVALID_SOCKET) {
        printf("Unable to connect to server.\n");
        WSACleanup();
        return 1;
    }

    do {
        cin.getline(sendbuf, BUFFERLEN);

        if (strcmp(sendbuf, 'exit') == 0)
            break;

        iResult = send(ConnectionSocket, sendbuf, strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(ConnectionSocket);
            WSACleanup();
            return 1;
        }
    } while (1);

    

    return 0;
}

