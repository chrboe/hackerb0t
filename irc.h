#ifndef IRC_H_INCLUDED
#define IRC_H_INCLUDED

#include "array.h"
#include "socket.h"

int irc_msg_len(char* msg, size_t maxlen);
char* fetch_complete_line(char** msg, size_t* msg_len, size_t* line_len);
int parse_tags(char* tags, array_t* arr);
void send_irc_message(SOCKET socket, char* msg, char* username);
void handle_irc(SOCKET socket, char* msg, int recvlen);


#endif // IRC_H_INCLUDED
