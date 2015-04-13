#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT	"27000"
#define MAXCONNECTIONS	10
#define DEFAULT_BUFLEN	512
#define STRLEN			256

DWORD WINAPI Connect(LPVOID lpParam);

SOCKET ListenSocket;
int iConn;

int _tmain(int argc, _TCHAR* argv[])
{
    int iResult;
    WSADATA wsaData;
    DWORD threadConnectionID[MAXCONNECTIONS];
    HANDLE hThreadConnection[MAXCONNECTIONS];
	char IPAddress[STRLEN];
    DWORD AddrLen;
	ListenSocket = INVALID_SOCKET;
	iConn = 0;

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    // Initialize Winsock (Use version 2.2)
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup Failed: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;          // IPV4
    hints.ai_socktype = SOCK_STREAM;    // Stream Socket (other options include: Datagram and Raw sockets)
    hints.ai_protocol = IPPROTO_TCP;    // TCP protocol
    hints.ai_flags = AI_PASSIVE;        // allows returned address to be used in call to bind
    // when nodename parameter in call to getaddrinfo is null, the IP
    // address portion of the socket address structure is set to INADDR_ANY

    /* ------ getaddrinfo function (Convenience Function) --------
    * The function provides protocol-independent translation from ANSI host name to an address.
    * -> [in, optional]    PCSTR           pNodeName       host name or numeric host address string (i.e. IP Address)
    * -> [in, optional]    PCSTR           pServiceName    service name or port number
    * -> [in, optional]    const ADDRINFOA *pHints         addrinfo structure that provides 'hints' about the type of socket supported
    * -> [out]             PADDRINFOA      *ppResult       linked list of addrinfo structures with information about the host
    * <-                   int             ErrorCode       Success returns zero. Failure returns nonzero. (Error codes online)
    *
    * REMARKS: This is essentially a convenience function that automatically sets up the address information passed when creating a socket.
    *          Manually this can be accomplished with the following.
    *
    *          ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    *          sockaddr_in result;
    *          result.sin_family = AF_INET;
    *          result.sin_addr.s_addr = inet_addr("127.0.0.1");
    *          result.sin_port = htons(27015);
    *          bind(ListenSocket, (SOCKADDR *) &result, sizeof(result));
    */
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo Failed: %d\n", iResult);
        WSACleanup();
        return 1;
	}

    /* ------- socket function ----------------------
    * The function creates a socket that is bound to a specific transport service provider.
    * -> [in]  int     af          address family (i.e. AF_INET=IPV4, AF_INET6=IPV6)
    * -> [in]  int     type        type of socket (i.e. SOCK_STREAM=TCP, SOCK_DGRAM=UDP)
    * -> [in]  int     protocol    protocol (i.e. IPPROTO_TCP=TCP, IPPROTO_UDP=UDP) used in conjuction with type
    * <-       SOCKET  socket      returns socket descriptor on no error, otherwise INVALID_SOCKET, where error code
    can be obtained from WSAGetLastError()
    */
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }


    printf("Binding server to %s\n", inet_ntoa(((sockaddr_in *)result->ai_addr)->sin_addr));

    //Setup TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    //done with address info so let's release it
    freeaddrinfo(result);

	printf("Server running on %s", DEFAULT_PORT);
    while (1) {
        if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
            printf("Listen failed with error: %ld\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
		if (iConn < MAXCONNECTIONS) {
			hThreadConnection[iConn] = CreateThread(
				NULL,
				0,
				Connect,
				NULL,
				0,
				&threadConnectionID[iConn]
			);

			if (hThreadConnection[iConn] == NULL) {
				printf("Failed to create a connection thread.");
				return 1;
			}

			iConn++;
		}
    }

    return 0;
}

DWORD WINAPI Connect(LPVOID lpParam)
{
	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	SOCKET ClientSocket = INVALID_SOCKET;

	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("Accept Failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			printf("Data received: %s\n", recvbuf);

			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
			}
			printf("Bytes Sent: %d\n", iSendResult);
		} else if (iResult == 0)
			printf("Connection closing....\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	closesocket(ClientSocket);
	iConn--;
	return 0;
}

