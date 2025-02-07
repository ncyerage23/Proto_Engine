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



typedef struct {
    sector_t* arr;
    int n;
    int* rendered;
} sector_list;

sector_list* sectList_create(int sect_count);
void sectList_destroy(sector_list* list);




#endif