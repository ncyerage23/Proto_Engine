//Header file for rendering stuff

#ifndef RENDER_H
#define RENDER_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "sector.h"
#include "wall.h"
#include "vector.h"

#define BLUE                0xFFFF0000
#define RED                 0xFF0000FF
#define GREEN               0xFF00FF00
#define WHITE               0xFFFFFFFF
#define BLACK               0xFF000000


typedef struct {
    v2 pos;
    float zpos;
    float angle;
    int sector;
} camera_t;


typedef struct {
    int width, height;
    //uint32_t* pixels;
    int* y_hi;
    int* y_lo;
    camera_t* cam;

    struct { sector_t* arr; int n; int* rendered; } *sectors;
    struct { wall_t* arr; int n; } *walls;
} frame_t;


frame_t* frame_create(int width, int height, void* sectors, void* walls, camera_t* cam);
void frame_destroy(frame_t* fr);


void reset_stuff(frame_t* fr, uint32_t* pixels);

void draw_line(frame_t* fr, uint32_t* pixels, int x, int top, int bottom, uint32_t color);
static inline v2 wpos_to_cam(frame_t* fr, v2 p);

void render_sector(frame_t* fr, uint32_t* pixels, int sect_id, int depth);
void render(frame_t* fr, uint32_t* pixels);

#endif