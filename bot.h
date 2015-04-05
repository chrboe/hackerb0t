#ifndef BOT_H_INCLUDED
#define BOT_H_INCLUDED

#include "socket.h"
#include <stdio.h>
#include "array.h"
#include "socket.h"
#include "irc.h"
#include "cJSON.h"
#include "tokens.h"


typedef struct
{
    char* name;
    int argcount;
    char* (*function)(array_t* args);
    char* usage;
} hackerbot_command;

typedef struct
{
    char* username;
    char* usertype;
    char* channel;
    array_t* params;
} hackerbot_command_args;

extern int running;
void process_line(SOCKET socket, char* msg);
void hackerbot_register_command(hackerbot_command* cmd);


#endif // BOT_H_INCLUDED
