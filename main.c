//Starting file for Proto Engine

//compile command btw: gcc main.c sector_math.c -o poop -lSDL2
//gotta get to doing a makefile at some point

/*The plan:
    understand the current code and streamline it
    Figure out switching sectors
    Draw vertically for each bit of the line (like the real thing) instead of wireframes (add color to each sector)
    Figure out the drawing buffer so we don't overdraw things (not the comp sci word (unless it is), just drawing over itself)
    Eventually, level editor and wall textures. And collision detection on walls and stuff.

    Then, when all of the basics are done, I just need to clean everything up and ensure that it's easy to use and well structured. 
    Make it modular, and feel relatively easy to approach. It doesn't have to be crazy well made, but it'd be nice. 

    From there, it's all gameplay. Copy the code to the dungeon crawler project. Implement enemy sprites and basic combat, 
    map out how the dungeons will work and build the rooms. Eventually build the dungeon generator, which will be annoying, but whatever. 
    But, yeah. Just run with it, and make a simple but fun dungeon crawler game. 

    One of the fun things will be writing the script for the dungeon generator, since I want it to essentially be infinite. All the 
    weapons and enemies need to scale as you go and stuff. It should be really fun, but, yeah I gotta figure that out. 

    One thing I haven't approached yet is sprite animation. I think I can figure it out, but it may be smart to build a data structure
    around the frames of animation. I like that idea. 

    Should I try python or lua embedding for the ai enemy scripting? Perhaps. Honestly, that might be the move, but we'll see when I get there. 
    It'd just be a little easier to do, to be honest. 

    I also gotta implement timers and that sort of stuff. Making the lua/python libraries for all of this is gonna be weird. But it will be super
    helpful. I mean like, this is exactly what I plan to do later, so yeah. 
*/


#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "sector_math.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600
#define FOV_SCALE       SCREEN_WIDTH / 2

#define MOVE_SPEED      5.0
#define ROT_SPEED       0.01
#define pcam            control.camera


//Main controller for like everything in the game
//holds all the data and stuff. Also calls the functions
static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int quit;

    struct { sector_t *arr; size_t n; } sectors;
    struct { wall_t *arr; size_t n; } walls;

    struct {
        vect pos;
        float zpos;
        float angle;
        int sector;
    } camera;

} control;


//may also be wrong
void handle_keys(const Uint8 *keystate) {
    float dx = sin(pcam.angle) * 5.0, dy = cos(pcam.angle) * 5.0;

    if (keystate[SDL_SCANCODE_W]) { pcam.pos.x += dx; pcam.pos.y += dy; }
    if (keystate[SDL_SCANCODE_S]) { pcam.pos.x -= dx; pcam.pos.y -= dy; }
    if (keystate[SDL_SCANCODE_A]) { pcam.pos.x -= dy; pcam.pos.y += dx; }
    if (keystate[SDL_SCANCODE_D]) { pcam.pos.x += dy; pcam.pos.y -= dx; }

    if (keystate[SDL_SCANCODE_LEFT]) { pcam.angle -= 0.05; }
    if (keystate[SDL_SCANCODE_RIGHT]) { pcam.angle += 0.05; }

}


//This currently works quite well, which is amazing. 
void render_sector(int sect_id) {
    if (sect_id < 0 || sect_id >= control.sectors.n) {
        return;
        //also check if it was already rendered, or is too far? idk yet
    }

    SDL_SetRenderDrawColor(control.renderer, 255, 0, 0, 255);

    int i = control.sectors.arr[sect_id].first_wall;
    int max_wall = i + control.sectors.arr[sect_id].num_walls;
    int sect_z_floor = control.sectors.arr[sect_id].zfloor;
    int sect_z_ceil = control.sectors.arr[sect_id].zceil;

    double cs = cos((double)control.camera.angle);
    double sn = sin((double)control.camera.angle);

    for (i = control.sectors.arr[sect_id].first_wall; i < max_wall; i++) {
        vect p1 = control.walls.arr[i].p1;
        vect p2 = control.walls.arr[i].p2;

        float x1 = p1.x - pcam.pos.x, y1 = p1.y - pcam.pos.y;
        float x2 = p2.x - pcam.pos.x, y2 = p2.y - pcam.pos.y;

        float wx1 = x1 * cs - y1 * sn, wy1 = y1 * cs + x1 * sn;
        float wx2 = x2 * cs - y2 * sn, wy2 = y2 * cs + x2 * sn;
        float wz1_floor = pcam.zpos - sect_z_floor, wz2_floor = pcam.zpos - sect_z_floor;
        float wz1_ceil = pcam.zpos - sect_z_ceil, wz2_ceil = pcam.zpos - sect_z_ceil;

        if (wy1 <= 0 || wy2 <= 0) {
            continue;
        }

        float temp_wy1 = wy1, temp_wy2 = wy2;
        wx1 = wx1 * FOV_SCALE / wy1 + (SCREEN_WIDTH/2); wy1 = wz1_floor * FOV_SCALE / temp_wy1 + (SCREEN_HEIGHT/2);
        wx2 = wx2 * FOV_SCALE / wy2 + (SCREEN_WIDTH/2); wy2 = wz2_floor * FOV_SCALE / temp_wy2 + (SCREEN_HEIGHT/2);

        float wy1_top = wz1_ceil * FOV_SCALE / temp_wy1 + (SCREEN_HEIGHT / 2);
        float wy2_top = wz2_ceil * FOV_SCALE / temp_wy2 + (SCREEN_HEIGHT / 2);

        if (control.walls.arr[i].portal != -1) {
            int delta_zfloor = control.sectors.arr[sect_id].zfloor - control.sectors.arr[control.walls.arr[i].portal].zfloor;
            int delta_zceil = control.sectors.arr[sect_id].zceil - control.sectors.arr[control.walls.arr[i].portal].zceil;

            float wall_len = 60.0;
            float rel_wall_len = vect_len( sub_vect( ((vect){wx1, wy1}), ((vect){wx1, wy1_top}) ));
            float m = delta_zfloor / wall_len;
            m *= rel_wall_len;
            float n = delta_zceil / wall_len;
            n *= rel_wall_len;

            if (delta_zfloor < 0) {
                SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1, (int)wx2, (int)wy2);
                SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1 + m, (int)wx2, (int)wy2 + m);
            } 
            
            if (delta_zceil < 0) {
                SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1_top + n, (int)wx2, (int)wy2_top + n);
                SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1_top, (int)wx2, (int)wy2_top);
            }
            

            render_sector(control.walls.arr[i].portal);
            
        } else {
            SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1, (int)wx2, (int)wy2);
            SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1_top, (int)wx2, (int)wy2_top);

            SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1, (int)wx1, (int)wy1_top);
            SDL_RenderDrawLine(control.renderer, (int)wx2, (int)wy2, (int)wx2, (int)wy2_top);
        }
        

    }
}


//idk what else I need here, maybe just use the above one here? idk yet. 
void render() {
    render_sector(control.camera.sector);
}


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

    control.sectors.n = 0;
    control.walls.n = 0;
    control.quit = 0;
    return 0;
}

void close() {
    if (control.sectors.arr)    free(control.sectors.arr);
    if (control.walls.arr)      free(control.walls.arr);
    if (control.renderer)       SDL_DestroyRenderer(control.renderer);
    if (control.window)         SDL_DestroyWindow(control.window);
    SDL_Quit();
}


//getting somewhere, I think. Quit copying tho, learn it yourself. it's all so weird tho lol. 
static int read_file(const char* path) {
    control.sectors.n = 1;

    FILE* f = fopen(path, "r");
    if (!f) { close(); return 1; }

    int sect_count, wall_count;

    char line[40];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue; 

        if (sscanf(line, "SECT %d", &sect_count) == 1) {
            control.sectors.arr = (sector_t*)malloc( sizeof(sector_t) * sect_count );
            if (!control.sectors.arr) { fclose(f); close(); return 1; }

            for (int i = 0; i < sect_count; i++) {
                if (!fgets(line, sizeof(line), f)) break;
                //(id, start_wall, num_walls, zfloor, zceil)
                sector_t* sect = &control.sectors.arr[i];
                control.sectors.n++;
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

    
    SDL_Event e;
    while (!control.quit) {

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                control.quit = 1;
            }
        }

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        float dx = 0;

        handle_keys(keystate);

        // Paints the screen black
        SDL_SetRenderDrawColor(control.renderer, 0, 0, 0, 255);
        SDL_RenderClear(control.renderer);

        render();


        // Present the renderer (show the content on the window)
        SDL_RenderPresent(control.renderer);
    }

    close();
    return 0;
}




