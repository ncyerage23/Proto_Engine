//Starting file for Proto Engine

//compile command btw: gcc main.c sector_math.c -o poop -lSDL2
//gotta get to doing a makefile at some point

/*The plan:
    Make it draw the full walls w/ the draw buffer stuff
    Understand the current code and fix it for what I wanna do. Sectors are being weird rn.
        For some reason, it thinks two of the walls are in sector 1 when they're in sector 0, 
        but only when I can see sector 1? Weird. It's especially weird cause it's only those two walls

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

    I think this is going pretty well, although there's a lot of bugs. I'm gonna do everything super inefficient right now, except for drawing bc
    that's gonna cut framerate. Things that will be inefficient are all the typing stuff and searching through sectors. I think I'll just always
    search through every single sector for right now. 

    holy shit it actually works so well now. well mostly. I'm still happy. Despite wasting a large amount of time on this. I need to do actual work. 
    hey tho, good job. Nvm, my computer's about to die. What a lame. 
*/


#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "sector_math.h"

#define SCREEN_WIDTH    1200
#define SCREEN_HEIGHT   700
#define FOV_SCALE       SCREEN_WIDTH / 2

#define MOVE_SPEED          5.0
#define ROT_SPEED           0.01
#define pcam                control.camera
#define set_color(id)       ( SDL_SetRenderDrawColor(control.renderer, (id) ? 255 : 0, 0, (id) ? 0 : 255, 255) )
#define SET_TRIG            pcam.cos = cos(pcam.angle); pcam.sin = sin(pcam.angle);


//Main controller for like everything in the game
//holds all the data and stuff. Also calls the functions
static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int quit;

    struct { sector_t *arr; size_t n; int *rendered; } sectors;
    struct { wall_t *arr; size_t n; } walls;

    int y_lo[SCREEN_WIDTH], y_hi[SCREEN_WIDTH];

    struct {
        vect pos;
        float zpos;
        float angle;
        int sector;
        double cos;
        double sin;
    } camera;

} control;


void handle_keys(const Uint8 *keystate) {
    float dx = sin(pcam.angle) * 2.5, dy = cos(pcam.angle) * 2.5;

    float posx = pcam.pos.x, posy = pcam.pos.y;

    if (keystate[SDL_SCANCODE_W]) { posx += dx; posy += dy; }
    if (keystate[SDL_SCANCODE_S]) { posx -= dx; posy -= dy; }
    if (keystate[SDL_SCANCODE_A]) { posx -= dy; posy += dx; }
    if (keystate[SDL_SCANCODE_D]) { posx += dy; posy -= dx; }

    if (keystate[SDL_SCANCODE_LEFT]) { pcam.angle -= 0.03; SET_TRIG; }
    if (keystate[SDL_SCANCODE_RIGHT]) { pcam.angle += 0.03; SET_TRIG; }

    if (keystate[SDL_SCANCODE_SPACE]) pcam.zpos += 1.0;
    if (keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT] ) pcam.zpos -= 1.0;


    //checks outer wall collision and stuff (pretty slow)
    if ( (posx != pcam.pos.x && posy != pcam.pos.y) ) {
        int new_sect = -1;
        for (int i = 0; i < control.sectors.n; i++) {
            if (in_sector(((vect){posx, posy}), &control.sectors.arr[i], control.walls.arr)) {
                new_sect = i;
            }
        }

        if (new_sect != -1) {
            pcam.pos = (vect){posx, posy};
            pcam.sector = new_sect;
            pcam.zpos = control.sectors.arr[new_sect].zfloor + 20.0;
        }
    }
}


void render_sector(int sect_id) {
    if (sect_id < 0 || sect_id >= control.sectors.n || control.sectors.rendered[sect_id] == 1) {
        return;
    }

    set_color(sect_id);
    control.sectors.rendered[sect_id] = 1;

    int i = control.sectors.arr[sect_id].first_wall;
    int max_wall = i + control.sectors.arr[sect_id].num_walls;
    int sect_z_floor = control.sectors.arr[sect_id].zfloor;
    int sect_z_ceil = control.sectors.arr[sect_id].zceil;

    double cs = cos((double)control.camera.angle);
    double sn = sin((double)control.camera.angle);

    int next_sector = -1;

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

        SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1, (int)wx2, (int)wy2);
        SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1_top, (int)wx2, (int)wy2_top);

        SDL_RenderDrawLine(control.renderer, (int)wx1, (int)wy1, (int)wx1, (int)wy1_top);
        SDL_RenderDrawLine(control.renderer, (int)wx2, (int)wy2, (int)wx2, (int)wy2_top);

        if (control.walls.arr[i].portal != -1) {
            next_sector = control.walls.arr[i].portal;
        }
    }
    
    if (next_sector != -1 && control.sectors.rendered[next_sector] != 1) {
        render_sector(next_sector);
    }
}


//revised render_sector, actually figure it out tho? 
void render_sector2(int sect_id) {
    if (sect_id < 0 || sect_id >= control.sectors.n || control.sectors.rendered[sect_id] == 1) {
        return;
    } 

    set_color(sect_id);
    control.sectors.rendered[sect_id] = 1;
    float zfloor = control.sectors.arr->zfloor, zceil = control.sectors.arr->zceil;

    //gonna need to make this an array later
    int next_sector = -1;

    vect p1, p2, w1, w2;
    for (int i = 0; i < control.sectors.arr[sect_id].num_walls; i++) {
        p1 = sub_vect(p1, pcam.pos);
        p2 = sub_vect(p2, pcam.pos);

        w1.x = p1.x * pcam.cos - p1.y * pcam.sin; w1.y = p1.y * pcam.cos + p1.x * pcam.sin;
        w2.x = p2.x * pcam.cos - p2.y * pcam.sin; w2.y = p2.y * pcam.cos + p2.x * pcam.sin;

        float wz1_floor = pcam.zpos - zfloor, wz2_floor = pcam.zpos - zfloor;
        float wz1_ceil = pcam.zpos - zceil, wz2_ceil = pcam.zpos - zceil;

        //this may be the clipping thing
        if (w1.y <= 0 || w2.y <= 0) {
            continue;
        }
    }

}


//First, imma fix up render_sector1 so it actually works w/o those weird errors and stuff.
//then streamline it with the macros and vector math I already made.
void render_sector3(int sect_id) {
    //Things I need to do first:
    //  Understand the current algorithms (mostly for world x and y to screen x and y), prob read some articles. 
    //  Then, yeah. Make render_sector3 a streamlined version of 1

    //render_sector4's algorithm:
    //  get the sector, check if it's already been drawn (maybe make an array for that?) Or if its too far away (make a function for sector center)
    //  maybe make a function or macro to convert world coords to screen coords?? That would be nice.
    //  iterate through the walls. Figure out the coordinates using the algorithms, check if it should be drawn. If not, chuck it. 
    //  If it's on the screen but needs clipped, modify the coords and whatnot for clipping. 
    //  Then, draw the walls that need drawn in that sector (if its not a portal)
    //  If the wall's a portal, recursively call the function to render that sector. 
    //  I think, instead of immediately calling it when the portal is found, do it at the end. Imma try that right now.
    //  there's probably more, but I'll get to it after I do render3
    
}

void clear_rendered() {
    for (int i = 0; i < control.sectors.n; i++) {
        control.sectors.rendered[i] = 0;
    }
}

//idk what else I need here, maybe just use the above one here? idk yet. 
void render() {
    clear_rendered();
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
            control.sectors.rendered = (int*)malloc(  sizeof(int) * sect_count );
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




