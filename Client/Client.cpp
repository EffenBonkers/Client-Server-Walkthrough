// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define DEFAULT_PORT    "27015"
#define BUFFERLEN       256
#define DATASIZE        100

#pragma comment(lib, "ws2_32.lib")

void print_data(unsigned int *data, int n);


int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
    int iResult;
    struct addrinfo *ptr = NULL, *result = NULL, hints;
    SOCKET ConnectionSocket = INVALID_SOCKET;

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

    //create random data for transmission
    unsigned int data[DATASIZE];
    srand(time(NULL));
    for (int idx = 0; idx < DATASIZE; idx++) {
        data[idx] = rand() % INT_MAX;
    }

    printf("----------------------------------------------------\n");
    printf("SENT DATA\n");
    printf("----------------------------------------------------\n");
    print_data(data, DATASIZE);

    iResult = send(ConnectionSocket, (char *) data, sizeof(unsigned int) * DATASIZE, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectionSocket);
        WSACleanup();
        return 1;
    }

    //now receive the response from the server
    unsigned int rcv_data[DATASIZE];
    iResult = recv(ConnectionSocket, (char *)rcv_data, sizeof(int) * DATASIZE, 0);
    if (iResult == SOCKET_ERROR) {
        printf("Receive failed: %d\n", WSAGetLastError());
        closesocket(ConnectionSocket);
        WSACleanup();
        return 1;
    }

    printf("----------------------------------------------------\n");
    printf("RECEIVED DATA\n");
    printf("----------------------------------------------------\n");
    print_data(rcv_data, DATASIZE);

    //validate response
    bool data_invalid = false;
    for (int idx = 0; idx < DATASIZE; idx++) {
        if (data[idx] != rcv_data[idx])
            data_invalid = true;
    }

    printf("----------------------------------------------------\n");
    printf("VALIDATING DATA\n");
    printf("----------------------------------------------------\n");
    if (data_invalid == true)
        printf("(-) Data invalid.\n");
    else
        printf("(+) Data Valid.\n");


    //send one last transmission of 0 bytes to ensure server closes connection
    iResult = send(ConnectionSocket, (char *) rcv_data, 0, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectionSocket);
        WSACleanup();
        return 1;
    }

    iResult = shutdown(ConnectionSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectionSocket);
        WSACleanup();
        return 1;
    }
    closesocket(ConnectionSocket);
    WSACleanup();

    return 0;
}

void print_data(unsigned int *data, int n)
{
    int col_length = 10;
    int col_idx = 0;

    for (int idx = 0; idx < n; idx++) {
        printf("%d ", data[idx]);
        col_idx++;
        if (col_idx == col_length) {
            printf("\n");
            col_idx = 0;
        }
    }
}