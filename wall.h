//Header for walls and things

#ifndef WALL_H
#define WALL_H

#include "vector.h"

//should I add wall id? nah.
typedef struct {
    vector_t p1, p2;
    int portal;
} wall_t;





#endif