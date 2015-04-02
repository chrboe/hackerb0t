#include "bot.h"

int running = 1;
char* token = NULL;
int token_len = 0;
void process_line(SOCKET socket, char* msg)
{
    // V3:
    //@color=#0000FF;emotes=;subscriber=0;turbo=0;user_type= :bomb_mask!bomb_mask@bomb_mask.tmi.twitch.tv PRIVMSG #bomb_mask :Yo thanks
    //:nickname!username@nickname.tmi.twitch.tv PRIVMSG #channel :message that was sent

    char* tags = strtok(msg, " ");
    array_t tags_arr;
    int r = parse_tags(tags, &tags_arr);
    char* prefix = strtok(NULL, " ");

    char* cmd = strtok(NULL, " ");
    //printf("prefix and cmd read\n");
    //printf("stuff parsed\n");
    if(strcmp(cmd, "PRIVMSG") == 0)
    {
        //printf("private message\n");
        char* username = calloc(32, sizeof(char));
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
        else if(strcmp(bot_command, "!twitchapi") == 0)
        {
            if(strcmp(usertype, "mod") != 0 && strcmp(channel+1, username) != 0)
            {
                send_irc_message(socket, "This command can only be used by mods!", username);
            }
            char* request_format = "GET /kraken/streams/hackerc0w HTTP/1.1\r\nConnection: close\r\nHost: api.twitch.tv\r\nAccept: application/vnd.twitchtv.v3+json\r\nAuthorization: OAuth %s\r\n\r\n";
            int request_format_len = strlen(request_format) -2;
            char* request = calloc(request_format_len + token_len + 1, sizeof(char));
            sprintf(request, request_format, token);
            char* buffer = NULL;
            send_twitch_api_request(request, &buffer);
            printf("request done\n");
            if(!buffer)
            {
                printf("no buffer :(\n");
                send_irc_message(socket, "There was an error while fulfilling your request BibleThump", username);
            }
            free(buffer);

            //printf("json: %s\n", json_text);
            //send_irc_message(socket, buffer, username);
            free(request);
        }
        else if(strcmp(bot_command, "!title") == 0)
        {
            char* request_format = "GET /kraken/streams/hackerc0w HTTP/1.1\r\nConnection: close\r\nHost: api.twitch.tv\r\nAccept: application/vnd.twitchtv.v3+json\r\nAuthorization: OAuth %s\r\n\r\n";
            int request_format_len = strlen(request_format) -2;
            char* request = calloc(request_format_len + token_len + 1, sizeof(char));
            sprintf(request, request_format, token);
            char* buffer = NULL;
            send_twitch_api_request(request, &buffer);
            //printf("request done\n");
            if(!buffer)
            {
                send_irc_message(socket, "There was an error while fulfilling your request BibleThump", username);
            }
            else
            {
                //printf("raw json:\n%s\n", buffer);

                cJSON* root = cJSON_Parse(strstr(buffer, "\r\n\r\n"));
                char* title = cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(root, "stream"), "channel"), "status")->valuestring;
                //printf("json: %s\n", json_text);
                send_irc_message(socket, title, username);
                cJSON_Delete(root);
                free(request);
            }
            free(buffer);
        }
        else if(strcmp(bot_command, "!die") == 0)
        {
            if(strcmp(usertype, "mod") != 0 && strcmp(channel+1, username) != 0)
            {
                send_irc_message(socket, "This command can only be used by mods!", username);
            }
            send_irc_message(socket, "Farewell, cruel world BibleThump", username);
            running = 0;
        }
        else if(strcmp(bot_command, "!suggest") == 0)
        {
            if(strcmp(usertype, "mod") != 0 && strcmp(channel+1, username) != 0)
            {
                send_irc_message(socket, "This command can only be used by mods!", username);
            }
            else
            {
                if(params.used == 0)
                {
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
                    free(suggest_text);

                }
            }
        }
        free_array(&tags_arr);
        free_array(&params);
        free(username);
    }
    else if(strcmp(cmd, "PING") == 0)
    {
        //printf("ping\n");
        int len = strlen("PONG");
        send_msg(socket, "PONG tmi.twitch.tv", &len);
    }
}


void start_bot(SOCKET connect_socket, char* token)
{
    token_len = strlen(token);
    char* pass_buf_format = "PASS oauth:%s\r\nNICK HackerB0t\r\nJOIN #hackerc0w\r\nCAP REQ :twitch.tv/tags\r\n";
    int formatlen = strlen(pass_buf_format)-2;

    int buflen = formatlen + token_len;
    char* pass_buf = calloc(buflen+1, sizeof(char));
    sprintf(pass_buf, pass_buf_format, token);

    send_msg(connect_socket, pass_buf, &buflen);

    int recvlen = 0;

    printf("Joined channel and ready.\n");

    while(running)
    {
        char* recv_buf = recv_irc_msg(connect_socket, &recvlen);
        //printf("%s\n\n\n", recv_buf);
        if(!recv_buf) break;
        handle_irc(connect_socket, recv_buf, recvlen);
        free(recv_buf);
    }
    free(pass_buf);
}
