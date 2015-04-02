#include "bot.h"


#define HOST "irc.twitch.tv"
#define PORT "6667"

char* token;
int token_len;

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

void read_token_from_file()
{
    long input_file_size;

    FILE* f = fopen("token.txt", "r");
    fseek(f, 0, SEEK_END);
    input_file_size = ftell(f);
    rewind(f);

    token = calloc((input_file_size + 1), (sizeof(char)));
    fread(token, sizeof(char), input_file_size, f);
    fclose(f);
    token[input_file_size] = 0;
    token_len = input_file_size;
}


int main(int argc, char** argv)
{
    /*
    if(strcmp(usertype, "mod") != 0 && strcmp(channel+1, username) != 0)
    {
        send_irc_message(socket, "This command can only be used by mods!", username);
    }
    */
    WSADATA wsadata;
    int res = WSAStartup(MAKEWORD(2,2), &wsadata);
    if(res != 0)
    {
        printf("there was an error initializing winsock: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }
    SOCKET connect_socket = get_connect_socket(HOST, PORT);

    read_token_from_file();
    if(!token)
    {
        printf("Your authentification token could not be read.\nMake sure there is a file called \"token.txt\" with the token in it\n");
        return 1;
    }
    //printf("token: %s\n", token);
    start_bot(connect_socket, token);

    closesocket(connect_socket);
    free(token);

    return 0;
}
