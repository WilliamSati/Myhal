#include "createSocket.h"

SOCKET createClientSocket() {

    //initialize the socket
    int iResult;
    WSADATA wsaData;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return INVALID_SOCKET;
    }

    // Create a SOCKET for connecting to server
    SOCKET ConnectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return INVALID_SOCKET;
    }

    return ConnectSocket;
}