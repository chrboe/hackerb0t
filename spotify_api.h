#ifndef SPOTIFY_API_H_INCLUDED
#define SPOTIFY_API_H_INCLUDED

#include "socket.h"
#include "cJSON.h"
#include "http_utils.h"

cJSON* send_spotify_api_request(char* url, char* token);

#endif // SPOTIFY_API_H_INCLUDED
