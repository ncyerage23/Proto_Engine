//Header file for sector stuff

#ifndef SECTOR_H
#define SECTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "wall.h"
#include "vector.h"

typedef struct {
    int id;
    int first_wall;
    int num_walls;
    float zfloor, zceil;
} sector_t;


//finds the center of a given sector
static inline v2 sect_center(sector_t* sect, wall_t* walls);

//checks if p is inside the sector
int in_sector(v2 p, sector_t* sect, wall_t* walls);




#endif