//Rendering stuff

#include "render.h"

//so, now all I gotta do is like. The other stuff. I really don't feel like it thoooooo
//idk later then. Shouldn't be like, too too bad. 

frame_t* frame_create(int width, int height, void* sectors, void* walls, camera_t* cam) {
    frame_t* out = (frame_t*)malloc( sizeof(frame_t) );
    if (!out) return NULL;

    out->width = width;
    out->height = height;

    out->pixels = (uint32_t*)malloc( sizeof(uint32_t) * width * height * 4);
    if (!out->pixels) {
        free(out);
        return NULL;
    }

    out->y_hi = (int*)malloc( sizeof(int) * width );
    out->y_lo = (int*)malloc( sizeof(int) * width );
    if ( !(out->y_hi && out->y_lo) ) {
        frame_destroy(out);
        return NULL;
    }

    memset(out->y_hi, 0, width * sizeof(int));
    memset(out->y_lo, 0, width * sizeof(int));
    memset(out->pixels, BLACK, sizeof(uint32_t) * width * height * 4);

    out->cam = cam;
    out->sectors = sectors;
    out->walls = walls;


    return out;
}

void frame_destroy(frame_t* fr) {
    if (!fr) return;
    if (fr->pixels) free(fr->pixels);
    if (fr->y_hi) free(fr->y_hi);
    if (fr->y_lo) free(fr->y_lo);
    free(fr);
}


void reset_stuff(frame_t* fr) {
    memset(fr->y_hi, 0, fr->width * sizeof(int));
    memset(fr->y_lo, 0, fr->width * sizeof(int));
    memset(fr->pixels, BLACK, sizeof(uint32_t) * fr->width * fr->height * 4);
    memset(fr->sectors->rendered, 0, sizeof(int) * fr->width);
}

void draw_line(frame_t* fr, int x, int top, int bottom, uint32_t color) {
    if (x < 0 || x >= fr->width) return;

    if (top < 0) top = 0;
    if (bottom >= fr->height) bottom = fr->height;

    for (int y = top; y <= bottom; y++) { fr->pixels[y * fr->width + x] = color; }
}

void render(frame_t* fr) {
    reset_stuff(fr);
    //draw_line(fr, 20, 10, 400, BLUE);

}