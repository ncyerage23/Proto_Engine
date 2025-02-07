//Header for walls and things

#ifndef WALL_H
#define WALL_H

#include "vector.h"

#define wall_intersect(w1, w2)      ( vector_intersect( w1.p1, w1.p2, w2.p1, w2.p2 ) )

typedef struct {
    v2 p1, p2;
    int portal;
} wall_t;







#endif