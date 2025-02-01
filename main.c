//Starting file for Proto Engine

#include <stdlib.h>
#include <stdio.h>

#include "render_objects.c"

#define SCREEN_WIDTH    850
#define SCREEN_HEIGHT   650
#define FOV_SCALE       SCREEN_WIDTH / 2

#define MOVE_SPEED      5.0
#define ROT_SPEED       0.01



//Main controller for like everything in the game
//holds all the data and stuff. Also calls the functions
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




int init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    control.window = SDL_CreateWindow(
        "No idea!", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    if (!control.window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    control.renderer = SDL_CreateRenderer(control.window, -1, SDL_RENDERER_ACCELERATED);
    if (!control.renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(control.window);
        SDL_Quit();
        return 1;
    }

    control.quit = 0;
    return 0;
}



int main() {
    if (init()) {
        printf("Init failed.");
        return 1;
    }
    
    SDL_Event e;
    while (!control.quit) {

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                control.quit = 1;
            }
        }

        // Paints the screen black
        SDL_SetRenderDrawColor(control.renderer, 0, 0, 0, 255);
        SDL_RenderClear(control.renderer);

        SDL_SetRenderDrawColor(control.renderer, 255, 0, 0, 255);
        SDL_Rect rect = { 200, 150, 400, 300 };
        SDL_RenderFillRect(control.renderer, &rect);


        // Present the renderer (show the content on the window)
        SDL_RenderPresent(control.renderer);
    }

    SDL_DestroyRenderer(control.renderer);
    SDL_DestroyWindow(control.window);
    SDL_Quit();
    return 0;
}




