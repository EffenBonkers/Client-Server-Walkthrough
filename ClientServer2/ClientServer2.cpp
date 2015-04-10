// ClientServer2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT "27015"



int _tmain(int argc, _TCHAR* argv[])
{
	int iResult;
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	// Initialize Winsock (Use version 2.2)
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup Failed: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;			// IPV4
	hints.ai_socktype = SOCK_STREAM;	// Stream Socket (other options include: Datagram and Raw sockets)
	hints.ai_protocol = IPPROTO_TCP;	// TCP protocol
	hints.ai_flags = AI_PASSIVE;		// allows returned address to be used in call to bind
										// when nodename parameter in call to getaddrinfo is null, the IP
										// address portion of the socket address structure is set to INADDR_ANY

	/* ------ getaddrinfo function -------- 
	 * The function provides protocol-independent translation from ANSI host name to an address.
	 * Parameters:
	 * [in, optional]	PCSTR			pNodeName		host name or numeric host address string (i.e. IP Address)
	 * [in, optional]	PCSTR			pServiceName	service name or port number
	 * [in, optional]	const ADDRINFOA	*pHints			addrinfo structure that provides 'hints' about the type of socket supported
	 * [out]			PADDRINFOA		*ppResult		linked list of addrinfo structures with information about the host
	 */
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo Failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	

	return 0;
}

