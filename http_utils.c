#include "http_utils.h"

char* strip_chunked(char* buffer)
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

int check_for_chunked(char* buffer)
{
    char* key = "Transfer-Encoding: chunked";
    char* temp_buf = strdup(buffer);
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
