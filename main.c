#define _WIN32_WINNT 0x0501

#include <winsock2.h>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "token.h"
#include "array.h"
#include "socket.h"
#include "irc.h"

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

void process_line(SOCKET socket, char* msg)
{
    // V3:
    //@color=#0000FF;emotes=;subscriber=0;turbo=0;user_type= :bomb_mask!bomb_mask@bomb_mask.tmi.twitch.tv PRIVMSG #bomb_mask :Yo thanks
    //:nickname!username@nickname.tmi.twitch.tv PRIVMSG #channel :message that was sent

    //printf("process start\n");
    //printf("line: %s\n", msg);
    char* tags = strtok(msg, " ");

    //printf("tags read\n");

    array_t tags_arr;
    int r = parse_tags(tags, &tags_arr);
    //printf("array parsed\n");
    char* prefix = strtok(NULL, " ");
    //printf("prefix: %s\n", prefix);

    char* cmd = strtok(NULL, " ");
    //printf("prefix and cmd read\n");
    //printf("stuff parsed\n");
    if(strcmp(cmd, "PRIVMSG") == 0)
    {
        //printf("private message\n");
        char* username = calloc(10, sizeof(char));
        char* channel = strtok(NULL, " ");
        char* bot_command = strtok(NULL, " ");
        sscanf(bot_command, ":%s", bot_command);

        array_t params;
        init_array(&params, 1);
        char* temp_tok = NULL;
        while(temp_tok = strtok(NULL, " "))
        {
            insert_array(&params, temp_tok);
        }
        int res = sscanf(prefix, "%*[^!]!%[^@]@%*", username);
        //int res = sscanf(msg, "%*[^ ] PRIVMSG #%[^ ] :%99c%*c[^\r\n]", username, channel, message);
        //printf("%s, %s, %s", username, channel, message);

        //printf("%s: %s\n", username, bot_command);

        char* usertype = get_value(&tags_arr, "user_type");
        //printf("usertype: %s\n", usertype);

        if(strcmp(bot_command, "!ping") == 0)
        {
            //printf("sending !ping\n");
            if(params.used == 0)
                send_irc_message(socket, "pong", username);
            else
                send_irc_message(socket, "!ping doesn't accept any parameters", username);
        }
        else if(strcmp(bot_command, "!saymyname") == 0)
        {
            //printf("savmyname\n");
            if(params.used == 0)
                send_irc_message(socket, username, username);
            else
                send_irc_message(socket, "!saymyname doesn't accept any parameters", username);
        }
        else if(strcmp(bot_command, "!suggest") == 0)
        {
            if(strcmp(usertype, "mod") != 0 && strcmp(channel+1, username) != 0)
            {
                send_irc_message(socket, "This command can only be used by mods!", username);
            }
            else
            {
                //printf("suggest\n");
                if(params.used == 0)
                {
                    //printf("suggest usage\n");
                    send_irc_message(socket, "Usage: !suggest [suggestion]", username);
                }
                else
                {
                    char* suggest_text = params.array[0];
                    int msg_len = 0;
                    for(int i = 1; i< params.used; i++)
                    {
                        msg_len = strlen(suggest_text);
                        char* temp_buf = calloc(msg_len + 1 + strlen(params.array[i]) + 1, sizeof(char));
                        memcpy(temp_buf, suggest_text, msg_len);
                        memcpy(temp_buf + msg_len, " ", 1);
                        memcpy(temp_buf + msg_len + 1, params.array[i], strlen(params.array[i]) + 1);

                        free(suggest_text);
                        suggest_text = calloc(msg_len + 1 + strlen(params.array[i]) + 1, sizeof(char));
                        memcpy(suggest_text, temp_buf, msg_len + 1 + strlen(params.array[i]) + 1);
                        free(temp_buf);
                    }
                    /*
                    int suggest_text_len = strlen(suggest_text);
                    suggest_text = realloc(suggest_text, suggest_text_len + 1);
                    suggest_text[suggest_text_len+1] = '\0';
                    suggest_text[suggest_text_len] = '\n';
                    */
                    //printf("%s suggested %s", username, suggest_text);

                    FILE* f = fopen("suggestions.txt", "a");
                    fwrite(suggest_text, sizeof(char), strlen(suggest_text), f);
                    fclose(f);

                    send_irc_message(socket, "Your suggestion was successfully added.", username);

                }
            }
        }
        free(msg);
        free_array(&tags_arr);
        free_array(&params);
        //printf("line: %s res: %d\n", msg, res        free(username);
        free(channel);
        free(bot_command);
    }
    else if(strcmp(cmd, "PING") == 0)
    {
        //printf("ping\n");
        int len = strlen("PONG");
        send_msg(socket, "PONG tmi.twitch.tv", &len);
    }
}


int main(int argc, char** argv)
{
    WSADATA wsadata;
    int res = WSAStartup(MAKEWORD(2,2), &wsadata);
    if(res != 0)
    {
        printf("there was an error initializing winsock: %d\n", WSAGetLastError());
        return 1;
    }

    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;
    SOCKET connect_socket = INVALID_SOCKET;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    res = getaddrinfo(HOST, PORT, &hints, &result);
    if(res != 0)
    {
        printf("there was an error resolving the address: %d\n", WSAGetLastError());
        return 1;
    }

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connect_socket == INVALID_SOCKET)
        {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
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
        return 1;
    }

    char* pass_buf = "PASS " AUTH_TOKEN "\r\nNICK HackerB0t\r\nJOIN #hackerc0w\r\nCAP REQ :twitch.tv/tags\r\n";
    int buflen = strlen(pass_buf);

    // authentication
    send_msg(connect_socket, pass_buf, &buflen);

    int recvlen = 0;
    while(1)
    {
        char* recv_buf = recv_msg(connect_socket, &recvlen);
        handle_irc(connect_socket, recv_buf, recvlen);
        free(recv_buf);
    }
    closesocket(connect_socket);

    return 0;
}
