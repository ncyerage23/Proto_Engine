//Starting file for Proto Engine
//compile command btw: gcc main.c render.c sector.c wall.c vector.c -o poop -lSDL2

/* The Plan:
    So, I'm starting from scratch! Yay! We'll see how well it goes, lol. 
    Idk what to do from here. Lol. I gotta read through my old stuff. 

    So, I guess, figure out vector first, then sector and wall. Basically
    just get all the errors out of main.c, then get to work on render.  
*/


#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#include "render.h"
#include "sector.h"
#include "vector.h"
#include "wall.h"

#define SCREEN_WIDTH    1200
#define SCREEN_HEIGHT   700
#define FOV_SCALE       600

#define MOVE_SPEED          5.0
#define ROT_SPEED           0.05


static struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int quit;

    sector_list* sectors;
    wall_list* walls;
    frame_t* fr;

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
        "Proto Engine!", 
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

    control.texture = SDL_CreateTexture(control.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!control.texture) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(control.renderer);
        SDL_DestroyWindow(control.window);
        SDL_Quit();
        return 1;
    }

    
    
    control.quit = 0;
    return 0;
}

void close() {
    if (control.sectors)        free(control.sectors);
    if (control.walls)          free(control.walls);
    if (control.fr)             free(control.fr);
    if (control.texture)        SDL_DestroyTexture(control.texture);
    if (control.renderer)       SDL_DestroyRenderer(control.renderer);
    if (control.window)         SDL_DestroyWindow(control.window);
    SDL_Quit();
}

int read_file(const char* path) {

    FILE* f = fopen(path, "r");
    if (!f) { close(); return 1; }

    int sect_count, wall_count;

    char line[40];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue; 

        if (sscanf(line, "SECT %d", &sect_count) == 1) {
            control.sectors = sectList_create(sect_count);
            if (!&control.sectors) { fclose(f); close(); return 1; }

            for (int i = 0; i < sect_count; i++) {
                if (!fgets(line, sizeof(line), f)) break;
                //(id, start_wall, num_walls, zfloor, zceil)
                sector_t* sect = &control.sectors->arr[i];
                sscanf(line, "%d %d %d %f %f", &sect->id, &sect->first_wall, &sect->num_walls, &sect->zfloor, &sect->zceil);
            }
            //control.sectors.n = sect_count;
        }

        if (sscanf(line, "WALL %d", &wall_count) == 1) {
            control.walls = wallList_create(wall_count);
            if (!&control.walls) { fclose(f); close(); return 1; }

            for (int j = 0; j < wall_count; j++) {
                if (!fgets(line, sizeof(line), f)) break;
                //(id, p1x, p1y, p2x, p2y, portal)
                wall_t* wall = &control.walls->arr[j];
                sscanf(line, "%f %f %f %f %d", &wall->p1.x, &wall->p1.y, &wall->p2.x, &wall->p2.y, &wall->portal);
            }
            //control.walls.n = wall_count;
        }

        if (strncmp(line, "CAM", 3) == 0) {
            if (!fgets(line, sizeof(line), f)) break;
            //(px, py, zpos, angle, sector)
            sscanf(line, "%f %f %f %d", &control.camera.pos.x, &control.camera.pos.y, &control.camera.zpos, &control.camera.sector);
        }

    }

    fclose(f);
    return 0;
}


int main() {
    if (init()) {
        printf("Init failed.");
        return 1;
    }

    if (read_file("./map.txt")) {
        printf("File read failure.");
        close();
        return 1;
    }

    int lastTime = SDL_GetTicks();
    int frames = 0;
    int fps = 0;
    int totalFrames = 0;
    int totalTime = 0;
    
    SDL_Event e;
    while (0) {

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                control.quit = 1;
            }
        }

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);




        
        SDL_RenderCopy(control.renderer, control.texture, NULL, NULL);
        SDL_RenderPresent(control.renderer);

        frames++;
        int currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1000) {
            fps = frames;
            totalFrames += frames;
            totalTime += (currentTime - lastTime);
            frames = 0;
            lastTime = currentTime;
        }
    }

    if (totalTime > 0) {
        float avgFps = (totalFrames * 1000.0) / totalTime;
        printf("Average FPS: %.2f\n", avgFps);
    }

    close();
    return 0;
}




