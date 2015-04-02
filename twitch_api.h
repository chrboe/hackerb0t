#ifndef TWITCH_API_H_INCLUDED
#define TWITCH_API_H_INCLUDED

#include "socket.h"

void send_twitch_api_request(char* request, char** buffer);

#endif // TWITCH_API_H_INCLUDED
