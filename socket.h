#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <winsock2.h>

/*
    max length for an IRC message is defined
    to be 512 bytes per RFC,
*/
#define BUFLEN 512

char* recv_msg(SOCKET s, int* received);
int send_msg(SOCKET s, char *buf, int *len);

#endif // SOCKET_H_INCLUDED
