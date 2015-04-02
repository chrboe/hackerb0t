#ifndef BOT_H_INCLUDED
#define BOT_H_INCLUDED

#include "socket.h"
#include <stdio.h>
#include "array.h"
#include "socket.h"
#include "irc.h"
#include "cJSON.h"

extern int running;
void process_line(SOCKET socket, char* msg);

#endif // BOT_H_INCLUDED
