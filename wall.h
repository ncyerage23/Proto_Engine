//Header for walls and things

#ifndef WALL_H
#define WALL_H

#include "vector.h"

typedef struct {
    vector_t p1, p2;
    int portal;
} wall_t;

//do I need this for all of them? not really
wall_t* wall_create(vector_t p1, vector_t p2, int portal);
void wall_destroy(wall_t* wall);


typedef struct {
    wall_t* arr;
    int n;
    int* rendered;
} wall_list;

wall_list* wallList_create(int wall_count);
void wallList_destroy(wall_list* list);

int add_wall(wall_list* list, vector_t p1, vector_t p2, int portal);

#endif