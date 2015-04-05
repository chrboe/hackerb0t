#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <time.h>

char* datetime_difference(struct tm* first, struct tm* second);
int numberwidth(int num);

#endif // UTILS_H_INCLUDED