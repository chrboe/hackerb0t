#include "socket.h"

char* recv_irc_msg(SOCKET s, int* received)
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

    printf("recv irc failed with error: %d\n", WSAGetLastError());
    WSACleanup();
    return NULL;
}

char* recv_msg(SOCKET s)
{
    char buf[BUFLEN+1];
    memset(buf, 0, sizeof(buf));
    int start = 0;
    char* content;
    int tmpres;
    while((tmpres = recv(s, buf, BUFLEN, 0)) > 0)
    {
        printf("receiving...\n");
        if(start == 0)
        {
            /* Under certain conditions this will not work.
            * If the \r\n\r\n part is splitted into two messages
            * it will fail to detect the beginning of HTML content
            */
            content = strstr(buf, "\r\n\r\n");
            if(content != NULL)
            {
                start = 1;
                content += 4;
            }
        }
        else
        {
            content = buf;
        }
        if(start)
        {
            printf("received content: %s\n", content);
        }
        memset(buf, 0, tmpres);
    }
    return content;
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

SOCKET get_connect_socket(char* host, char* port)
{

    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;
    SOCKET connect_socket = INVALID_SOCKET;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int res = getaddrinfo(host, port, &hints, &result);
    if(res != 0)
    {
        printf("there was an error resolving the address: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connect_socket == INVALID_SOCKET)
        {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return INVALID_SOCKET;
        }
        res = connect( connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (res == SOCKET_ERROR)
        {
            closesocket(connect_socket);
            connect_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (connect_socket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return INVALID_SOCKET;
    }
    return connect_socket;
}
