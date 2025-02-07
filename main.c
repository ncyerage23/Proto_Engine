//Starting file for Proto Engine
//compile command btw: gcc main.c render.c vector.c wall.c sector.c -o poop -lSDL2
//can't do this ^ until I actually make c files lol

/* The Plan:
    Alright, well I copied over all of my old stuff & things, which is nice. 
    Unfortunately, they don't seem to like me very much. Well, here's the new plan. 

    Let's rework main and render until I can just get a stable window going. From there, 
    idk. Start tryna draw some lines and shit. Later, we can try out the weird stuff. 

    HEY! IT WORKED! Lol. It does work, but I jumped the gun a little lol. 
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

    struct { sector_t* arr; int n; int* rendered; } sectors;
    struct { wall_t* arr; int n; } walls;

    frame_t* fr;
    camera_t* cam;

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
    if (control.sectors.arr)        free(control.sectors.arr);
    if (control.walls.arr)          free(control.walls.arr);
    if (control.fr)                 frame_destroy(control.fr);
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
            control.sectors.arr = (sector_t*)malloc( sizeof(sector_t) * sect_count );
            control.sectors.rendered = (int*)malloc( sizeof(int) * SCREEN_WIDTH );
            if (!control.sectors.arr) { fclose(f); close(); return 1; }

            for (int i = 0; i < sect_count; i++) {
                if (!fgets(line, sizeof(line), f)) break;
                //(id, start_wall, num_walls, zfloor, zceil)
                sector_t* sect = &control.sectors.arr[i];
                sscanf(line, "%d %d %d %f %f", &sect->id, &sect->first_wall, &sect->num_walls, &sect->zfloor, &sect->zceil);
            }
            control.sectors.n = sect_count;
        }

        if (sscanf(line, "WALL %d", &wall_count) == 1) {
            control.walls.arr = (wall_t*)malloc( sizeof(wall_t) * wall_count );
            if (!control.walls.arr) { fclose(f); close(); return 1; }

            for (int j = 0; j < wall_count; j++) {
                if (!fgets(line, sizeof(line), f)) break;
                //(id, p1x, p1y, p2x, p2y, portal)
                wall_t* wall = &control.walls.arr[j];
                sscanf(line, "%f %f %f %f %d", &wall->p1.x, &wall->p1.y, &wall->p2.x, &wall->p2.y, &wall->portal);
            }
            control.walls.n = wall_count;
        }

        if (strncmp(line, "CAM", 3) == 0) {
            if (!fgets(line, sizeof(line), f)) break;
            //(px, py, zpos, angle, sector)
            control.cam = (camera_t*)malloc(sizeof(camera_t));
            sscanf(line, "%f %f %f %d", &control.cam->pos.x, &control.cam->pos.y, &control.cam->zpos, &control.cam->sector);
        }

    }

    //creating renderer
    control.fr = frame_create(SCREEN_WIDTH, SCREEN_HEIGHT, &control.sectors, &control.walls, control.cam);
    if (!control.fr) {
        printf("yo, sum went wrong\n");
        return 1;
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
    while (!control.quit) {

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                control.quit = 1;
            }
        }

        //const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        render(control.fr);

        SDL_UpdateTexture(control.texture, NULL, control.fr->pixels, control.fr->width * sizeof(uint32_t));
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




