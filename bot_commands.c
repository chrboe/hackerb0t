#include "bot_commands.h"
#define is_mod(x) (strcmp(x->usertype, "mod") == 0 || strcmp(x->channel, x->username) == 0)


array_t* channels_lurked = NULL;

char* ping_handler(hackerbot_command_args* args)
{
    args->channel = "angrymasturbator";
    return "pong";
}

char* saymyname_handler(hackerbot_command_args* args)
{
    return args->username;
}

char* title_handler(hackerbot_command_args* args)
{
    cJSON* root = send_twitch_api_request("/kraken/streams/hackerc0w", twitch_token);

    char* response  = "There was an error while fulfilling your request BibleThump";
    if(root)
    {
        cJSON* stream = cJSON_GetObjectItem(root, "stream");
        if(!stream) goto end;
        cJSON* channel = cJSON_GetObjectItem(stream, "channel");
        if(!channel) goto end;
        response = cJSON_GetObjectItem(channel, "status")->valuestring;
        cJSON_Delete(root);
    }
    end:
    return response;
}

char* die_handler(hackerbot_command_args* args)
{
    if(!is_mod(args))
        return "This command can only be used by mods!";

    running = 0;
    return "Farewell, cruel world BibleThump";
}

char* suggest_handler(hackerbot_command_args* args)
{
    if(!is_mod(args))
        return "This command can only be used by mods!";

    FILE* f = fopen("suggestions.txt", "a");
    if(!f)
        return "There was an error while fulfilling your request BibleThump";
    fwrite(args->params->array[0], sizeof(char), strlen(args->params->array[0]), f);
    fwrite("\r\n", sizeof(char), 2, f);
    fclose(f);
    return "Your suggestion was successfully added.";
}

char* uptime_handler(hackerbot_command_args* args)
{            
    cJSON* root = send_twitch_api_request("/kraken/streams/hackerc0w", twitch_token);
    //printf("%s\n", cJSON_Print(root));
    cJSON* stream = cJSON_GetObjectItem(root, "stream");
    if(!stream)
        return "This channel is currently not online.";

    char* date_string = cJSON_GetObjectItem(stream, "created_at")->valuestring;
    int year, month, day, hour, minute, second;
    char* retbuf = NULL;
    if(sscanf(date_string, "%4u%*c%2u%*c%2u%*c%2u%*c%2u%*c%2u", &year, &month, &day, &hour, &minute, &second) != EOF)
    {
        struct tm parsed_time;
        struct tm time_now;
        struct tm* ti;

        time_t rawTime;
        time(&rawTime);
        ti = localtime(&rawTime);
        memcpy(&parsed_time, ti, sizeof(*ti));
        ti = gmtime(&rawTime);
        memcpy(&time_now, ti, sizeof(*ti));

        parsed_time.tm_year = year - 1900;
        parsed_time.tm_mon = month - 1;
        parsed_time.tm_mday = day;
        parsed_time.tm_hour = hour; /* twitch sends stuff in gmt-1 (?) */
        parsed_time.tm_min = minute;
        parsed_time.tm_sec = second;

        char* diff = datetime_difference(&parsed_time, &time_now);
        char* diff_format = "This channel has been live for %s.";
        retbuf = calloc(strlen(diff_format) - 2 + strlen(diff) + 1, sizeof(char));
        sprintf(retbuf, diff_format, diff);
        free(diff);
    }
    // TODO: THIS LEAKS MEMORY!!
    return retbuf;
}

char* playlist_handler(hackerbot_command_args* args)
{
    return "https://open.spotify.com/user/hackercow/playlist/5XPzaJ9Djfm47soHJ52CSQ";
}

char* botsnack_handler(hackerbot_command_args* args)
{
    return "Mmmmmh, delicious Kreygasm";
}

char* lurk_handler(hackerbot_command_args* args)
{
    if(!is_mod(args))
        return "This command can only be used by mods!";

    char* format = "JOIN #%s\r\n";
    int msglen = strlen(format) -2;
    char* msgtosend = calloc(msglen+1, sizeof(char));
    sprintf(msgtosend, format, args->params->array[0]);
    send_raw_irc_message(args->socket, msgtosend);
    free(msgtosend);

    int recvlen = 0;
    char* recv_buf = recv_irc_msg(args->socket, &recvlen);
    printf("RESPONSE: %s\n", recv_buf);

    char* success_format = "Successfully joined channel #%s";
    int success_len = strlen(success_format) -2;
    char* success = calloc(success_len+1, sizeof(char));
    sprintf(success, success_format, args->params->array[0]);
    return success;
    // TODO: MEMORY LEAK
}

char* unlurk_handler(hackerbot_command_args* args)
{
    if(!is_mod(args))
        return "This command can only be used by mods!";
    char* format = "PART #%s\r\n";
    int msglen = strlen(format) -2 + strlen(args->params->array[0]);
    char* msgtosend = calloc(msglen+1, sizeof(char));
    sprintf(msgtosend, format, args->params->array[0]);
    send_raw_irc_message(args->socket, msgtosend);
    free(msgtosend);

    char* success_format = "Successfully left channel #%s";
    int success_len = strlen(success_format) -2 + strlen(args->params->array[0]);
    char* success = calloc(success_len+1, sizeof(char));
    sprintf(success, success_format, args->params->array[0]);
    return success;
    // TODO: MEMORY LEAK
}

char* sendto_handler(hackerbot_command_args* args)
{
    char* remote_format = "[From %s] %s";
    int remote_len = strlen(remote_format) - 4 + strlen(args->channel) + strlen(args->params->array[1]);
    char* remote = calloc(remote_len, sizeof(char));
    sprintf(remote, remote_format, args->channel, args->params->array[1]);
    send_irc_message(args->socket, remote, NULL, args->params->array[0]);
    free(remote);

    char* success_format = "Successfully sent message to %s";
    int success_len = strlen(success_format) -2 + strlen(args->params->array[0]);
    char* success = calloc(success_len+1, sizeof(char));
    sprintf(success, success_format, args->params->array[0]);
    return success;
}

void register_commands()
{
    hackerbot_command ping_cmd =        {"ping",        0, ping_handler,        "!ping"};                       hackerbot_register_command(&ping_cmd);
    hackerbot_command saymyname_cmd =   {"saymyname",   0, saymyname_handler,   "!saymyname"};                  hackerbot_register_command(&saymyname_cmd);
    hackerbot_command title_cmd =       {"title",       0, title_handler,       "!title"};                      hackerbot_register_command(&title_cmd);
    hackerbot_command die_cmd =         {"die",         0, die_handler,         "!die"};                        hackerbot_register_command(&die_cmd);
    hackerbot_command suggest_cmd =     {"suggest",     1, suggest_handler,     "!suggest [suggestion]"};       hackerbot_register_command(&suggest_cmd);
    hackerbot_command uptime_cmd =      {"uptime",      0, uptime_handler,      "!uptime"};                     hackerbot_register_command(&uptime_cmd);
    hackerbot_command playlist_cmd =    {"playlist",    0, playlist_handler,    "!playlist"};                   hackerbot_register_command(&playlist_cmd);
    hackerbot_command botsnack_cmd =    {"botsnack",    0, botsnack_handler,    "!botsnack"};                   hackerbot_register_command(&botsnack_cmd);
    hackerbot_command lurk_cmd =        {"lurk",        1, lurk_handler,        "!lurk [channel]"};             hackerbot_register_command(&lurk_cmd);
    hackerbot_command unlurk_cmd =      {"unlurk",      1, unlurk_handler,      "!unlurk [channel]"};           hackerbot_register_command(&unlurk_cmd);
    hackerbot_command sendto_cmd =      {"sendto",      2, sendto_handler,      "!sendto [channel] [message]"}; hackerbot_register_command(&sendto_cmd);
}
