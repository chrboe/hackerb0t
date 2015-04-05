#ifndef HTTP_UTILS_H_INCLUDED
#define HTTP_UTILS_H_INCLUDED

#include <stdlib.h>

char* strip_chunked(char* buffer);
int check_for_chunked(char* buffer);

#endif // HTTP_UTILS_H_INCLUDED
