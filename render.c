//Rendering stuff

#include "render.h"

frame_t* frame_create(int width, int height) {
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

    memset(out->y_hi, 0, width * sizeof(int));
    memset(out->y_lo, 0, width * sizeof(int));
    memset(out->pixels, BLACK, sizeof(uint32_t) * width * height * 4);

    return out;
}

void frame_destroy(frame_t* fr) {
    if (!fr) return;
    if (fr->pixels) free(fr->pixels);
    if (fr->y_hi) free(fr->y_hi);
    if (fr->y_lo) free(fr->y_lo);
    free(fr);
}