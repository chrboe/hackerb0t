#include "bot.h"

int running = 1;
hackerbot_command** registered_commands = NULL;
int command_count = 0;

void process_line(SOCKET socket, char* msg)
{
    // V3:
    //@color=#0000FF;emotes=;subscriber=0;turbo=0;user_type= :bomb_mask!bomb_mask@bomb_mask.tmi.twitch.tv PRIVMSG #bomb_mask :Yo thanks
    //:nickname!username@nickname.tmi.twitch.tv PRIVMSG #channel :message that was sent

    if(strncmp(msg, "PING", 4) == 0)
    {
        printf("\n\n\n###########PING###########\n\n\n");
        int len = strlen("PONG");
        send_msg(socket, "PONG tmi.twitch.tv", &len);
        return;
    }

    /* see if message contains tags */
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
        if(bot_command[0] != '!')
            goto cleanup;

        array_t params;
        init_array(&params, 1);
        char* temp_tok = NULL;
        while(temp_tok = strtok(NULL, " \r\n"))
        {
            insert_array(&params, temp_tok);
        }
        int res = sscanf(prefix, "%*[^!]!%[^@]@%*", username);

        char* usertype = get_value(&tags_arr, "user_type");
        //printf("usertype: %s\n", usertype);

        for(int i = 0; i < command_count; i++)
        {
            if(strcmp(registered_commands[i]->name, bot_command+1) == 0)
            {
                char* response = NULL;
                hackerbot_command_args args = {username, usertype, channel+1, NULL};
                if(registered_commands[i]->argcount == 0)
                {
                    response = registered_commands[i]->function(&args);
                }
                else
                {
                    if(params.used < registered_commands[i]->argcount)
                        response = registered_commands[i]->usage;
                    /* 
                        if the user supplies too many arguments, we will just merge all the 
                        "unnecessarry" ones into one string to make the last argument 
                    */
                    else if(params.used > registered_commands[i]->argcount)
                    {
                        /* ...-1 is perfectly fine because we never hit this code path if argcount == 0 */
                        char* merged = strdup(params.array[registered_commands[i]->argcount-1]);
                        int msg_len = 0;
                        for(int j = registered_commands[i]->argcount; j < params.used; j++)
                        {
                            msg_len = strlen(merged);
                            char* temp_buf = calloc(msg_len + 1 + strlen(params.array[j]) + 1, sizeof(char));
                            memcpy(temp_buf, merged, msg_len);
                            memcpy(temp_buf + msg_len, " ", 1);
                            memcpy(temp_buf + msg_len + 1, params.array[j], strlen(params.array[j]) + 1);

                            free(merged);
                            merged = calloc(msg_len + 1 + strlen(params.array[j]) + 1, sizeof(char));
                            memcpy(merged, temp_buf, msg_len + 1 + strlen(params.array[j]) + 1);
                            free(temp_buf);
                        }

                        int used = params.used;
                        for(int j = registered_commands[i]->argcount; j < used; j++)
                            remove_from_array(&params);

                        params.array[registered_commands[i]->argcount-1] = merged;

                        args.params = &params;
                        response = registered_commands[i]->function(&args);
                        free(merged);
                    }
                    else
                    {
                        args.params = &params;
                        response = registered_commands[i]->function(&args);
                    }
                }
                if(!response)
                    response = "There was an error while processing your request BibleThump";

                send_irc_message(socket, response, username);
                goto cleanup;
            }
        }

        send_irc_message(socket, "I don't know that command", username);

        goto cleanup;

        if(strcmp(bot_command, "!spotify") == 0)
        {
            if(params.used == 0)
            {
            }
            if(strcmp(params.array[0], "search") == 0)
            {
                //send_spotify_api_request("/v1/search?q=can%27t+touch+this&type=track", spotify_token);
                send_irc_message(socket, "This command isn't implemented yet BibleThump", username);
                goto cleanup;
            }
            else if(strcmp(params.array[0], "add") == 0)
            {
                send_irc_message(socket, "This command isn't implemented yet BibleThump", username);
                goto cleanup;
            }

            send_usage:
                send_irc_message(socket, "Usage: !spotify [search|add]", username);
                goto cleanup;

        }
        else if(strcmp(bot_command, "!playlist") == 0)
        {
            send_irc_message(socket, "https://open.spotify.com/user/hackercow/playlist/5XPzaJ9Djfm47soHJ52CSQ", username);
        }
        else if(strcmp(bot_command,"!botsnack") == 0)
        {
            send_irc_message(socket, "Mmmmm, delicious Kreygasm", username);
        }
        else
        {
            send_irc_message(socket, "I don't know that command", username);
        }
        cleanup:
        free_array(&tags_arr);
        free_array(&params);
        free(username);
    }
}

void hackerbot_register_command(hackerbot_command* command)
{
    if(!registered_commands)
    {
        registered_commands = calloc(1, sizeof(hackerbot_command**));
    }
    registered_commands[command_count] = calloc(1, sizeof(*command));
    //memcpy(registered_commands + command_count, command, sizeof(hackerbot_command*));
    *registered_commands[command_count] = *command;
    command_count++;
}

void hackerbot_start_bot(SOCKET connect_socket, char* p_twitch_token, char* p_spotify_token)
{
    twitch_token = p_twitch_token;
    spotify_token = p_spotify_token;

    int token_len = strlen(twitch_token);
    char* pass_buf_format = "PASS oauth:%s\r\nNICK HackerB0t\r\nJOIN #hackerc0w\r\nCAP REQ :twitch.tv/tags\r\n";
    int formatlen = strlen(pass_buf_format)-2;

    int buflen = formatlen + token_len;
    char* pass_buf = calloc(buflen+1, sizeof(char));
    sprintf(pass_buf, pass_buf_format, twitch_token);

    send_msg(connect_socket, pass_buf, &buflen);

    int recvlen = 0;

    printf("Joined channel and ready.\n");
    printf("%d commands registered.\n", command_count);

    while(running)
    {
        char* recv_buf = recv_irc_msg(connect_socket, &recvlen);
        //printf("%s\n\n\n", recv_buf);
        if(!recv_buf) break;
        handle_irc(connect_socket, recv_buf, recvlen, twitch_token);
        free(recv_buf);
    }
    free(pass_buf);
    for(int i = 0; i < command_count; i++) free(registered_commands[i]);
    free(registered_commands);
}
