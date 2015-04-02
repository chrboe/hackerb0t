#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <stdlib.h>
#include <string.h>

typedef struct {
  char** array;
  size_t used;
  size_t size;
} array_t;

void init_array(array_t* a, size_t initialSize);
void insert_array(array_t* a, char* element);
void free_array(array_t* a);

#endif // ARRAY_H_INCLUDED
