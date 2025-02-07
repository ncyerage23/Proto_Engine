//Header file for sector stuff

#ifndef SECTOR_H
#define SECTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int id;
    int first_wall;
    int num_walls;
    float zfloor, zceil;
} sector_t;







#endif