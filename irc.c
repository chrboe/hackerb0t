#include "irc.h"


int irc_msg_len(char* msg, size_t maxlen)
{
    if(msg == NULL) return -1;
    size_t len = 0;
    while(msg[len] != '\r')
    {
        if(len >= maxlen)
            return -1;
        len++;
    }
    return len+2;
}

char* temp = NULL;
int temp_len = 0;

char* fetch_complete_line(char** msg, size_t* msg_len, size_t* line_len)
{
    int len = irc_msg_len(*msg, *msg_len);
    if(len == -1)
    {
        return NULL;
    }

    if(temp)
    {
        len += temp_len;
        char* tmp_cpy = calloc((*msg_len) + temp_len, sizeof(char));
        memcpy(tmp_cpy, temp, temp_len);
        memcpy(tmp_cpy + temp_len, *msg, *msg_len);
        *msg_len += temp_len;
        free(*msg);
        *msg = calloc(*msg_len, sizeof(char));
        memcpy(*msg, tmp_cpy, *msg_len);
        free(tmp_cpy);
        free(temp);
        temp = NULL;
        temp_len = 0;
    }

    *line_len = len;

    char* retbuf = calloc(len, sizeof(char));
    memcpy(retbuf, *msg, len);

    if(len == *msg_len)
    {
        *msg_len = 0;
        free(*msg);
        *msg = NULL;
        return retbuf;
    }

    char* remaining_buf = calloc((*msg_len) - len, sizeof(char));
    memcpy(remaining_buf, *msg + len, (*msg_len) - len);

    int linelength_remaining = irc_msg_len(remaining_buf, (*msg_len) - len-1);

    if(linelength_remaining == -1)
    {
        free(temp);
        temp = calloc((*msg_len) - len, sizeof(char));
        temp_len = (*msg_len) - len;
        memcpy(temp, *msg, (*msg_len) - len);
        *msg_len = 0;
        free(*msg);
        *msg = NULL;
    }
    else
    {
        *msg_len -= len;
        free(*msg);
        *msg = calloc(*msg_len, sizeof(char));
        memcpy(*msg, remaining_buf, *msg_len);
    }


    free(remaining_buf);
    return retbuf;
}

int parse_tags(char* tags, array_t* arr)
{
    // ['@' <tags> <SPACE>] [':' <prefix> <SPACE> ] <command> <params> <crlf>
    if(!tags || tags[0] != '@')
        return 0;

    init_array(arr, 1);

    char* current = NULL;
    int current_size = 0;
    tags++;
    while(*tags)
    {
        //printf("loop: %s\n", tags);
        if(*tags == ';')
        {
            current = realloc(current, current_size +1);
            current[current_size] = '\0';
            insert_array(arr, current);
            free(current);
            current = NULL;
            current_size = 0;
        }
        else
        {
            /* we don't add a null byte here because we do it above */
            char* temp = calloc(current_size+1, sizeof(char));
            memcpy(temp, current, current_size);

            free(current);
            current = calloc(current_size + 1, sizeof(char));
            memcpy(current, temp, current_size);
            free(temp);

            memcpy(current + current_size, tags, 1);
            current_size++;
        }
        tags++;
    }
    current = realloc(current, current_size +1);
    current[current_size] = '\0';
    insert_array(arr, current);
    free(current);
    current = NULL;
    current_size = 0;
    return 1;

}

void send_irc_message(SOCKET socket, char* msg, char* username, char* channel)
{
    char* wrapper = NULL;
    if(username)
    {
        wrapper = ":hackerb0t!hackerb0t@hackerb0t.tmi.twitch.tv PRIVMSG #%s :[%s] %s\r\n";
        int wrapper_length = strlen(wrapper) -6; // -6 because 3 %s
        int sendlen = wrapper_length + strlen(msg) + strlen(username) + strlen(channel);
        char* msgtosend = calloc(sendlen, sizeof(char));
        sprintf(msgtosend, wrapper, channel, username, msg);
        send_msg(socket, msgtosend, &sendlen);
        free(msgtosend);
    }
    else
    {
        wrapper = ":hackerb0t!hackerb0t@hackerb0t.tmi.twitch.tv PRIVMSG #%s :%s\r\n";
        int wrapper_length = strlen(wrapper) -4; // -4 because 2 %s
        int sendlen = wrapper_length + strlen(msg) + strlen(channel);
        char* msgtosend = calloc(sendlen, sizeof(char));
        sprintf(msgtosend, wrapper, channel, msg);
        send_msg(socket, msgtosend, &sendlen);
        free(msgtosend);
    }

    //char* wrapper = "PRIVMSG #hackerc0w :[@%s] %s\r\n";
}

void send_raw_irc_message(SOCKET socket, char* command)
{
    int sendlen = strlen(command);
    send_msg(socket, command, &sendlen);
}

void handle_irc(SOCKET socket, char* msg, int recvlen, char* token)
{
    /*
    - fetch one complete line from the buffer
    - process this line
    - if there are no more complete lines, return
    - else, fetch next complete line
    */

    char* line = NULL;
    size_t line_len = 0;
    size_t msg_len = recvlen;
    do
    {
        line = fetch_complete_line(&msg, &msg_len, &line_len);
        if(!line)
            break;
        line = realloc(line, line_len + 1);
        line[line_len] = '\0';

        process_line(socket, line, token);


        free(line);
    } while(line);
}
