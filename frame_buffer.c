//All the stuff for drawing the screen

/*
    This is a decent start, but I think I should
    go back to the drawing board, and figure out the architecture. 
    I mostly have an idea, but not yet exactly.

    I want game logic in main.c, and rendering in frame_buffer, right? 
    How's that gonna work with sectors and things? Idk. 

    Because both rendering and game logic need to know the sector data. 
    I guess just have main pass it to each frame_buffer function?? Idk. 

    This should only involve drawing a frame. Like, that's what this one does. 
    Do everything with the header files for now, then implement. I think.
    
    Yeah. Mess around with how everything is structured for now, until I 
    get something that feels clean. 
*/

#include "sector_math.h"

typedef struct {
    uint32_t *pixels;
    int width;
    int height;
    int *y_lo;
    int *y_hi;
} frbuff;


frbuff* frbuff_create(int width, int height) {
    frbuff* out = (frbuff*)malloc( sizeof(frbuff) );

    out->width = width;
    out->height = height;

    out->pixels = (uint32_t*)malloc( sizeof(uint32_t) * width * height * 4);
    out->y_lo = (int*)malloc( sizeof(int) * width);
    out->y_hi = (int*)malloc( sizeof(int) * width);

    return out;
}

void frbuff_destroy(frbuff *fr) {
    free(fr->pixels);
    free(fr->y_lo);
    free(fr->y_hi);
}



