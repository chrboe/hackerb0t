#ifndef TWITCH_API_H_INCLUDED
#define TWITCH_API_H_INCLUDED

#include "socket.h"
#include "cJSON.h"
#include "http_utils.h"

cJSON* send_twitch_api_request(char* url, char* token);

#endif // TWITCH_API_H_INCLUDED
