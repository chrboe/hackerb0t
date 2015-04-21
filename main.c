#include "bot.h"

#define HOST "irc.twitch.tv"
#define PORT "6667"

char* get_value(array_t* arr, char* key)
{
    if(!arr) return NULL;
    int key_length = strlen(key);
    for(int i = 0; i < arr->used; i++)
    {
        if(strncmp(key, arr->array[i], key_length) == 0)
        {
            return strstr(arr->array[i], "=")+1;
        }
    }
}

char* read_token_from_file(char* file)
{
    long input_file_size;

    FILE* f = fopen(file, "r");

    if(!f) return NULL;

    fseek(f, 0, SEEK_END);
    input_file_size = ftell(f);
    rewind(f);

    char* token = calloc((input_file_size + 1), (sizeof(char)));
    fread(token, sizeof(char), input_file_size, f);
    fclose(f);
    token[input_file_size] = 0;
    return token;
}


int main(int argc, char** argv)
{
    WSADATA wsadata;
    int res = WSAStartup(MAKEWORD(2,2), &wsadata);
    if(res != 0)
    {
        printf("there was an error initializing winsock: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }
    SOCKET connect_socket = get_connect_socket(HOST, PORT);

    char* l_twitch_token = read_token_from_file("twitch.token");
    char* l_spotify_token = read_token_from_file("spotify.token");

    if(!l_twitch_token)
    {
        printf("Your Twitch authentification token could not be read.\nMake sure there is a file called \"twitch.token\" with the token in it\n");
        return 1;
    }
    if(!l_spotify_token)
    {
        printf("Your Spotify authentification token could not be read.\nMake sure there is a file called \"spotify.token\" with the token in it\n");
        return 1;
    }

    register_commands();

    hackerbot_start_bot(connect_socket, l_twitch_token, l_spotify_token);

    closesocket(connect_socket);
    free(l_twitch_token);
    free(l_spotify_token);

    return 0;
}
