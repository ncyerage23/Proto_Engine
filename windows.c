//Stuff for the sdl2 interface and all that / rendering
//Yeah, this is where I render, right? 

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "render_objects.c"


//Main controller for like everything in the game?
//holds all the data
static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int quit;

    struct { sector_t arr[32]; size_t n; } sectors;
    struct { wall_t arr[32]; size_t n; } walls;

    struct {
        vector_t pos;
        float zpos;
        float angle;
        int sector;
    } camera;

} control;