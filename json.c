#include "json.h"

json_object* create_object()
{
    json_object* result = calloc(1, sizeof(*result));
    return result;
}

void free_object(json_object* object)
{
    free(object);
}

json_object* json_parse(char* json)
{
    json_object* current = NULL;
    json_parse_mode current_mode = NONE;
    char* tmp = NULL;
    int tmp_len = 0;
    for(;*json;json++)
    {
        if(*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')
            continue;

        if(*json == '{')
        {
            if(current_mode == PARSING_VALUE)
            {
                /* "object" value */

            }
            else
            {
                /* initial bracket */
                current = create_object();
                current_mode = PARSING_STRING;
            }
        }
        else if(*json == '}')
        {

        }
        else if(*json == '[')
        {

        }
        else if(*json == '}')
        {

        }
        else if(*json == ',')
        {

        }
        else if(*json == ':')
        {
            if(current_mode != PARSING_STRING)
                goto error;
            current_mode = PARSING_VALUE;
        }
        else if(*json == '"')
        {
            if(!current)
                goto error;

            if(!current->string)
            {
                if(current_mode == PARSING_STRING)
                {

                }
                else if(current_mode == PARSING_VALUE)
                {

                }
            }
            else
            {
                if(current_mode == PARSING_STRING)
                {
                    current->string = realloc(current->string, tmp_len+1);
                    current->string[tmp_len] = '\0';
                }
                else if(current_mode == PARSING_VALUE)
                {
                }
                free(tmp);
                tmp = NULL;
                tmp_len = 0;
            }
        }
        else
        {
            current->string = realloc(current->string, tmp_len+1);
            memcpy(current->string + tmp_len, json, 1);
            tmp_len++;
        }
    }
    return current;
    error:
        free(current);
        current = NULL;
    return current;
}

void json_print(json_object o)
{

}
