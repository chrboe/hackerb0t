#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

typedef enum {NULL, STRING, INT, OBJECT, ARRAY, TRUE, FALSE} json_object_type;
typedef enum {NONE,PARSING_STRING, PARSING_VALUE} json_parse_mode;

typedef struct json_object json_object;
typedef struct json_object
{
    char* string;
    json_object* child;
    json_object* next;
    json_object* previous;
    char* value_string;
    int value_int;
    json_object_type type;
} json_object;

#endif // JSON_H_INCLUDED
