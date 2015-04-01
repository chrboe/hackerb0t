
#include <stdio.h>
#include "array.h"
#include "socket.h"
#include "irc.h"
#include "cJSON.h"
#include "openssl/ssl.h"


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
        else if(strcmp(bot_command, "!twitchapi") == 0)
        {
            char* request_format = "GET /kraken HTTP/1.1\r\nConnection: close\r\nHost: api.twitch.tv\r\nAccept: application/vnd.twitchtv.v3+json\r\nAuthorization: OAuth %s\r\n\r\n";
            int request_format_len = strlen(request_format) -2;
            char* request = calloc(request_format_len + token_len + 1, sizeof(char));
            sprintf(request, request_format, token);
            SOCKET api_sock = get_connect_socket("api.twitch.tv", "443");

            SSL_load_error_strings();
            SSL_library_init();
            SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_client_method());
            SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);

            SSL* conn = SSL_new(ssl_ctx);
            SSL_set_connect_state(conn);
            SSL_connect(conn);
            SSL_set_verify(conn, SSL_VERIFY_NONE, NULL);

            long l = 0;
            ioctlsocket(api_sock, FIONBIO, &l);

            //SSL_set_fd(conn, api_sock);
            BIO *sbio = NULL;
            sbio = BIO_new_socket(api_sock, BIO_NOCLOSE);
            SSL_set_bio(conn, sbio, sbio);


            int send_len = strlen(request);

            int total = 0;        // how many bytes we've sent
            int bytesleft = send_len; // how many we have left to send
            int res;

            //printf("sending %s\n",request);
            char* request_m = calloc(send_len+1, sizeof(char));
            memcpy(request_m, request, send_len);
            res = SSL_write(conn, request_m, bytesleft);
            if (res < 0)
            {
                char errorstr[512];
                int error = SSL_get_error(conn, res);
                printf("send failed with error: ");
                switch(error)
                {
                case SSL_ERROR_NONE:
                    printf("SSL_ERROR_NONE (what?)\n");
                    break;
                case SSL_ERROR_ZERO_RETURN:
                    printf("SSL_ERROR_ZERO_RETURN\n");
                    break;
                case SSL_ERROR_WANT_READ:
                    printf("SSL_ERROR_WANT_READ\n");
                    break;
                case SSL_ERROR_WANT_WRITE:
                    printf("SSL_ERROR_WANT_WRITE\n");
                    break;
                case SSL_ERROR_WANT_CONNECT:
                    printf("SSL_ERROR_WANT_CONNECT\n");
                    break;
                case SSL_ERROR_WANT_ACCEPT:
                    printf("SSL_ERROR_WANT_ACCEPT\n");
                    break;
                case SSL_ERROR_WANT_X509_LOOKUP:
                    printf("SSL_ERROR_WANT_X509_LOOKUP\n");
                    break;
                case SSL_ERROR_SYSCALL:
                    printf("SSL_ERROR_SYSCALL\n");
                    break;
                case SSL_ERROR_SSL:
                    printf("SSL_ERROR_SSL\n");
                    break;
                }
                printf("(%s)\n", ERR_error_string(ERR_get_error(), NULL));
                closesocket(api_sock);
                WSACleanup();
                return 1;
            }
            free(request_m);

            printf("sending finished\n");

            int result = 0;
            char* buf = calloc(512, sizeof(char));
            int recvbuflen = 512;
            char* fullbuf = NULL;
            int fullbufsize = 0;
            do
            {
                printf("reading\n");
                result = SSL_read(conn, buf, recvbuflen);
                if(result < 0)
                {
                    char errorstr[512];
                    int error = SSL_get_error(conn, res);
                    printf("ssl recv failed with error: ");
                    switch(error)
                    {
                    case SSL_ERROR_NONE:
                        printf("SSL_ERROR_NONE (what?)\n");
                        break;
                    case SSL_ERROR_ZERO_RETURN:
                        printf("SSL_ERROR_ZERO_RETURN\n");
                        break;
                    case SSL_ERROR_WANT_READ:
                        printf("SSL_ERROR_WANT_READ\n");
                        break;
                    case SSL_ERROR_WANT_WRITE:
                        printf("SSL_ERROR_WANT_WRITE\n");
                        break;
                    case SSL_ERROR_WANT_CONNECT:
                        printf("SSL_ERROR_WANT_CONNECT\n");
                        break;
                    case SSL_ERROR_WANT_ACCEPT:
                        printf("SSL_ERROR_WANT_ACCEPT\n");
                        break;
                    case SSL_ERROR_WANT_X509_LOOKUP:
                        printf("SSL_ERROR_WANT_X509_LOOKUP\n");
                        break;
                    case SSL_ERROR_SYSCALL:
                        printf("SSL_ERROR_SYSCALL\n");
                        break;
                    case SSL_ERROR_SSL:
                        printf("SSL_ERROR_SSL\n");
                        break;
                    }
                }
                fullbuf = realloc(fullbuf, fullbufsize + result);
                memcpy(fullbuf + fullbufsize, buf, result);
                fullbufsize += result;
            } while(result > 0);
            fullbuf = realloc(fullbuf, fullbufsize +1);
            fullbuf[fullbufsize] = '\0';
            printf("received from api: %s\n", fullbuf);

            free(buf);
            SSL_shutdown(conn);
            SSL_free(conn);
            closesocket(api_sock);

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
        return INVALID_SOCKET;
    }

    SOCKET connect_socket = get_connect_socket(HOST, PORT);

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

    char* pass_buf_format = "PASS oauth:%s\r\nNICK HackerB0t\r\nJOIN #hackerc0w\r\nCAP REQ :twitch.tv/tags\r\n";
    int formatlen = strlen(pass_buf_format)-2;

    int buflen = formatlen + input_file_size;
    char* pass_buf = calloc(buflen, sizeof(char));
    sprintf(pass_buf, pass_buf_format, token);
    //printf("auth: %s\n", pass_buf);
    // authentication
    send_msg(connect_socket, pass_buf, &buflen);

    int recvlen = 0;
    while(1)
    {
        char* recv_buf = recv_irc_msg(connect_socket, &recvlen);
        if(!recv_buf) break;
        handle_irc(connect_socket, recv_buf, recvlen);
        free(recv_buf);
    }
    closesocket(connect_socket);
    free(token);

    return 0;
}
