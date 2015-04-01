#include "socket.h"

char* recv_msg(SOCKET s, int* received)
{
    if(s == INVALID_SOCKET) return NULL;

    int res;
    char* recvbuf = calloc(BUFLEN, sizeof(char));
    int recvbuflen = BUFLEN;

    res = recv(s, recvbuf, recvbuflen, 0);
    if (res > 0)
    {

        //printf("Bytes received: %d\n", res);
        *received = res;
        //printf("%s\n", recvbuf);

        return recvbuf;
    }

    printf("recv failed with error: %d\n", WSAGetLastError());
    return NULL;
}


int send_msg(SOCKET s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    int res;

    while(total < *len) {
        //printf("sending %s\n",buf+total);
        n = send(s, buf+total, bytesleft, 0);
        if (res == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(s);
            WSACleanup();
            return 1;
        }
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}
