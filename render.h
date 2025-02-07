//Header file for rendering stuff

#ifndef RENDER_H
#define RENDER_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define BLUE                0xFFFF0000
#define RED                 0xFF0000FF
#define GREEN               0xFF00FF00
#define WHITE               0xFFFFFFFF
#define BLACK               0xFF000000

typedef struct {
    int width, height;
    uint32_t* pixels;
    int* y_hi;
    int* y_lo;
} frame_t;

frame_t* frame_create(int width, int height);
void frame_destroy(frame_t* fr);


#endif