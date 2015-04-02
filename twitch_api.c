#include "twitch_api.h"

char* strtok_single (char * str, char const * delims)
{
  static char  * src = NULL;

  char  *  p,  * ret = 0;

  if (str != NULL)
    src = str;

  if (src == NULL)
    return NULL;


  if ((p = strpbrk (src, delims)) != NULL) {
    *p  = 0;
    ret = src;
    src = ++p;
  }

  return ret;
}

static char* strip_chunked(char* buffer)
{
    char* dup_buffer = strdup(buffer);
    char* body = strstr(dup_buffer, "\r\n\r\n")+4;
    free(buffer);
    int result_len = body-dup_buffer;
    char* result_buffer = calloc(result_len, sizeof(char));
    memcpy(result_buffer, dup_buffer, result_len);

    char* line = strtok(body, "");
    int found = 0;
    int firsttime = 1;
    char* temp_line = NULL;
    while(line = strtok(firsttime ? body : NULL, "\r\n"))
    {
        int line_len = strlen(line);
        firsttime = 0;
        found=0;
        for(int i = 0;i<line_len;i++)
        {
            if(!(isxdigit(line[i]) || line[i]=='\r' || line[i]=='\n'))
            {
                found=0;
                break;
            }
            else
                found=1;
        }
        if(found)
        {
            int len = strtol(line, NULL, 16);
            //printf("next chunk length: %d\n", len);
            if(len > 0)
            {
                result_buffer = realloc(result_buffer, result_len+len+2);
                memcpy(result_buffer + result_len, dup_buffer + result_len + line_len + 2, len+2); // +2 because \r\n
                result_len += len + 2;
            }
        }
    }
    free(dup_buffer);

    result_buffer = realloc(result_buffer, result_len + 2);
    result_buffer[result_len] = '\0';
    return result_buffer;
}

int check_for_chunked(char** buffer)
{
    char* key = "Transfer-Encoding: chunked";
    char* temp_buf = strdup(*buffer);
    char* body = strstr(temp_buf, "\r\n\r\n");
    int header_len = body - temp_buf;
    temp_buf[header_len] = '\0';
    char* header_line = strtok(temp_buf, "\r\n");
    while(header_line = strtok(NULL, "\r\n"))
    {
        if(strncmp(header_line, key, strlen(key)) == 0)
            break;
    }
    free(temp_buf);
    int res = header_line != NULL;
    return res;
}

void send_twitch_api_request(char* request, char** buffer)
{
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

    //printf("sending finished\n");

    int result = 0;
    char* buf = calloc(512, sizeof(char));
    int recvbuflen = 512;
    int fullbufsize = 0;
    *buffer = NULL;
    do
    {
        //printf("reading\n");
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
        *buffer = realloc(*buffer, fullbufsize + result+1);
        memcpy(*buffer + fullbufsize, buf, result);
        free(buf);
        buf = calloc(512, sizeof(char));
        fullbufsize += result;
    } while(result > 0);
    *buffer = realloc(*buffer, fullbufsize +1);
    memset((*buffer) + fullbufsize, 0, 1);
    //(*buffer)[fullbufsize+1] = '\0';
    //printf("received from api: %s\n", *buffer);
    if(check_for_chunked(buffer))
    {
        *buffer = strip_chunked(*buffer); // TODO: should probably make use of transfer-encoding: chunked
    }

    free(buf);
    SSL_shutdown(conn);
    SSL_free(conn);
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    ERR_remove_state(0);
    EVP_cleanup();
    closesocket(api_sock);
}
