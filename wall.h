//Header for walls and things

#ifndef WALL_H
#define WALL_H

#include "vector.h"

//should I add wall id? nah.
typedef struct {
    vector_t p1, p2;
    int portal;
} wall_t;



typedef struct {
    wall_t* arr;
    int n;
    int* rendered;
} wall_list;

wall_list* wallList_create(int wall_count);
void wallList_destroy(wall_list* list);


#endif