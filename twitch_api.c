#include "twitch_api.h"


cJSON* send_twitch_api_request(char* url, char* token)
{
    char* request_format = "GET %s HTTP/1.1\r\nConnection: close\r\nHost: api.twitch.tv\r\nAccept: application/vnd.twitchtv.v3+json\r\nAuthorization: OAuth %s\r\n\r\n";
    int request_format_len = strlen(request_format) - 2 - 2; // one -2 for every %s
    int token_len = strlen(token);
    int url_len = strlen(url);
    char* request = calloc(request_format_len + url_len + token_len + 1, sizeof(char));
    sprintf(request, request_format, url, token);

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
    free(request);

    //printf("sending finished\n");

    int result = 0;
    char* buf = calloc(512, sizeof(char));
    int recvbuflen = 512;
    int fullbufsize = 0;
    char* buffer = NULL;
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
        buffer = realloc(buffer, fullbufsize + result+1);
        memcpy(buffer + fullbufsize, buf, result);
        free(buf);
        buf = calloc(512, sizeof(char));
        fullbufsize += result;
    } while(result > 0);
    buffer = realloc(buffer, fullbufsize +1);
    memset(buffer + fullbufsize, 0, 1);
    if(check_for_chunked(buffer))
    {
        buffer = strip_chunked(buffer); // TODO: should probably make use of transfer-encoding: chunked
    }

    cJSON* root = cJSON_Parse(strstr(buffer, "\r\n\r\n"));

    free(buf);
    free(buffer);
    SSL_shutdown(conn);
    SSL_free(conn);
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    ERR_remove_state(0);
    EVP_cleanup();
    closesocket(api_sock);
    return root;
}
