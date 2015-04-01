#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#define _WIN32_WINNT 0x0501

#include <winsock2.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

/*
    max length for an IRC message is defined
    to be 512 bytes per RFC,
*/
#define BUFLEN 512

char* recv_irc_msg(SOCKET s, int* received);
char* recv_msg(SOCKET s);
int send_msg(SOCKET s, char *buf, int *len);
SOCKET get_connect_socket(char* host, char* port);

#endif // SOCKET_H_INCLUDED
