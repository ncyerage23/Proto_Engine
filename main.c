//Starting file for Proto Engine
//compile command btw: gcc main.c sector_math.c -o poop -lSDL2

/*The plan:
    Hey! I got it to work! The buffer switch took like a half hour tops! I'm killin it!
    So, I think this is the plan. Now that I'm on a new branch, let's slightly restart and 
    make it all work a bit better. For starters, I gotta cull and not draw things off the screen
    without just dropping those things altogether. This should be a bit of a challenge, but hey!
    Look at me now!

    From there, if it still crashes and all that, I think I need to figure out why. Use gdb and
    valgrind and figure out how to use them to their fullest. Then, yeah. Just improve the code. 

    After that, I guess its time for floors and ceilings, and the map editor. I'll probably need
    to restart then a second time, just to map out how that'll all work. Textures for walls, floors,
    ceilings, and the offsets between different floors and ceilings all need to be tracked, I think. Damn.

    But yeah. this is going great. Once I get the rendering and shit all worked out for this, I guess
    I could start trying to do sprites, "decals" and whatever other game objects. Either sprites or decals
    seem the easiest. I do wanna get to voxel objects as well. That should be fun. 

    One thing I want to keep in mind with the rewrites is to increase abstraction. That'll help the code make
    a lot more sense and all that. I think I'll keep everything in C for this engine and the dungeon crawler, 
    but later I'm sure c++ would be the move. Lua (or python) integration is definitely gonna be a big thing
    later, so I should really read up on that. I'm thinking this version is in python, and then whatever later
    versions of the engine I'll learn lua. Unless python really is that slow, or lua really is that easy. Time will tell. 
*/

/*The current plan:
    Idk what's going wrong here, honestly. Clipping doesn't seem to be the issue. Idk. I guess I should just start
    using gdb and/or valgrind, but idk how to do that because macOS seems to be a little picky about that sort of thing.

    So yeah. My two big things I gotta do now are: figuring out debugging / fixing the crashing problem, and rebuilding 
    the code with all my new knowledge. lol. I think I should start adding more abstraction to the files as well. Like,
    this file is pushing 500 lines of code, and it's mostly a mess. Maybe make the drawing stuff on a separate file, 
    and then have this one just deal with the game logic and that kind of thing? That's probably a good idea. 
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

#define BLUE                0xFFFF0000
#define RED                 0xFF0000FF
#define GREEN               0xFF00FF00
#define WHITE               0xFFFFFFFF
#define BLACK               0xFF000000

#define sect_color(id)      ( (id) ? BLUE : RED )


//Main controller for like everything in the game
//holds all the data and stuff. Also calls the functions
static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture * texture;
    uint32_t *pixels;
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


//converts world pos to camera pos, but not really? It really just finds relative pos from the cam.
//make more of these
static inline vect wpos_to_cam(vect p) {
    const vect u = { p.x - pcam.pos.x, p.y - pcam.pos.y };
    return (vect) {
        u.x * pcam.cos - u.y * pcam.sin,
        u.x * pcam.sin + u.y * pcam.cos,
    };
}


void draw_line(int x, int y_lo, int y_hi, uint32_t color) {
    for (int y = y_lo; y <= y_hi; y++) {
        control.pixels[y * SCREEN_WIDTH + x] = color;
    }
}

//attempt at filled walls, works poorly
void render_sector(int sect_id) {
    if (sect_id < 0 || sect_id >= control.sectors.n || control.sectors.rendered[sect_id] == 1) {
        return;
    }

    control.sectors.rendered[sect_id] = 1;
    sector_t* sect = &control.sectors.arr[sect_id];

    //make this an array later
    int next_sector = -1;

    //helpful values
    int hh = (SCREEN_HEIGHT/2), hw = (SCREEN_WIDTH/2);

    //z values for walls
    float wz1_floor = pcam.zpos - sect->zfloor;
    float wz1_ceil = pcam.zpos - sect->zceil;
    float wz2_floor = pcam.zpos - sect->zfloor;
    float wz2_ceil = pcam.zpos - sect->zceil;

    vect p1, p2, w1, w2;
    for (int i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        wall_t* wall = &control.walls.arr[i];
        w1 = wpos_to_cam(wall->p1);
        w2 = wpos_to_cam(wall->p2);

        //means wall is behind camera, drop  (need more clipping stuff)
        if (w1.y <= 0 || w2.y <= 0) continue;

        //coefficient for screen pos calculations   
        //ig i should store these somewhere else? idek how I'd do that. 
        float inv_wy1 = FOV_SCALE / w1.y;
        float inv_wy2 = FOV_SCALE / w2.y;

        //convert world x to screen x
        w1.x = w1.x * inv_wy1 + hw;
        w2.x = w2.x * inv_wy2 + hw;

        if (w1.x > SCREEN_WIDTH && w2.x > SCREEN_WIDTH) continue;
        if (w1.x < 0 && w2.x < 0) continue;

        //convert world y to screen y
        float wy1_bottom = wz1_floor * inv_wy1 + hh;
        float wy2_bottom = wz2_floor * inv_wy2 + hh;
        float wy1_top = wz1_ceil * inv_wy1 + hh;
        float wy2_top = wz2_ceil * inv_wy2 + hh;

        if (wall->portal == -1) {
            //vectors for floor and ceiling lines
            vect floor = normal_vect( ( (vect){w2.x - w1.x, wy2_bottom - wy1_bottom} ) );
            vect ceil = normal_vect( ( (vect){w2.x - w1.x, wy2_top - wy1_top} ) );

            //scale for finding top and bottom coords of line
            float scale_floor = (floor.y / floor.x);
            float scale_ceil = (ceil.y / ceil.x);

            for( int x = 0; x < w2.x - w1.x; x += 1 ) {
                int x_val = w1.x + x;
                int bottom = wy1_bottom + (scale_floor * x);
                int top = wy1_top + (scale_ceil * x);
                //printf("%d, %d, %d\n", x_val, top, bottom);
                draw_line(x_val, bottom, top, sect_color(sect_id));
            }

        } 
        else next_sector = control.walls.arr[i].portal;
        
    }
    
    if (next_sector != -1 && control.sectors.rendered[next_sector] != 1) {
        render_sector(next_sector);
    }
}


void render_sector2(int sect_id) {
    if (sect_id < 0 || sect_id >= control.sectors.n || control.sectors.rendered[sect_id] == 1) {
        return;
    }

    control.sectors.rendered[sect_id] = 1;
    sector_t* sect = &control.sectors.arr[sect_id];

    //make this an array later
    int next_sector = -1;

    //helpful values
    int hh = (SCREEN_HEIGHT/2), hw = (SCREEN_WIDTH/2);

    //z values for walls
    float wz1_floor = pcam.zpos - sect->zfloor;
    float wz1_ceil = pcam.zpos - sect->zceil;
    float wz2_floor = pcam.zpos - sect->zfloor;
    float wz2_ceil = pcam.zpos - sect->zceil;

    vect p1, p2, w1, w2;
    for (int i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        wall_t* wall = &control.walls.arr[i];
        w1 = wpos_to_cam(wall->p1);
        w2 = wpos_to_cam(wall->p2);

        //means wall is behind camera, drop  (need more clipping stuff)
        if (w1.y <= 0 || w2.y <= 0) continue;

        //coefficient for screen pos calculations   
        //ig i should store these somewhere else? idek how I'd do that. 
        float inv_wy1 = FOV_SCALE / w1.y;
        float inv_wy2 = FOV_SCALE / w2.y;

        //convert world x to screen x
        w1.x = w1.x * inv_wy1 + hw;
        w2.x = w2.x * inv_wy2 + hw;

        if (w1.x > SCREEN_WIDTH && w2.x > SCREEN_WIDTH) continue;
        if (w1.x < 0 && w2.x < 0) continue;

        //convert world y to screen y
        float wy1_bottom = wz1_floor * inv_wy1 + hh;
        float wy2_bottom = wz2_floor * inv_wy2 + hh;
        float wy1_top = wz1_ceil * inv_wy1 + hh;
        float wy2_top = wz2_ceil * inv_wy2 + hh;

        if (wall->portal == -1) {
            //printf("wall coords: %.2f, %.2f\nscreen coords: %.2f, %.2f\t%.2f, %.2f\n\n", wall->p1.x, wall->p1.y, w1.x, wy1_bottom, w2.x, wy2_bottom);

            vect floor = normal_vect( ( (vect){w2.x - w1.x, wy2_bottom - wy1_bottom} ) );
            vect ceil = normal_vect( ( (vect){w2.x - w1.x, wy2_top - wy1_top} ) );

            float scale_floor = (floor.y / floor.x);
            float scale_ceil = (ceil.y / ceil.x);

            for (int x = 0; x < w2.x - w1.x; x++) {
                int x_val = w1.x + x;
                int bottom = wy1_bottom + (scale_floor * x);
                int top = wy1_top + (scale_ceil * x);
                if (x_val >= 0 && x_val <= SCREEN_WIDTH && wy1_top >= 0 && wy1_bottom < SCREEN_HEIGHT) {
                    draw_line(x_val, top, bottom, sect_color(sect_id));
                }
            }

        } else {
            next_sector = wall->portal;
        }
    }

    if (next_sector != -1 && control.sectors.rendered[next_sector] != 1) {
        render_sector2(next_sector);
    }

}


void render() {
    memset(control.pixels, 0, sizeof(uint32_t) * SCREEN_HEIGHT * SCREEN_WIDTH);     //clears framebuffer
    memset(control.y_hi, SCREEN_HEIGHT - 1, sizeof(int) * SCREEN_WIDTH);            //clears hi pixel drawn
    memset(control.y_lo, 0, sizeof(int) * SCREEN_WIDTH);                            //clears low pixel drawn
    memset(control.sectors.rendered, 0, sizeof(int) * control.sectors.n);           //resets all sectors to not-rendered

    render_sector2(pcam.sector);

    //Render sprites and stuff here

    SDL_UpdateTexture(control.texture, NULL, control.pixels, SCREEN_WIDTH * sizeof(uint32_t));
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

    control.texture = SDL_CreateTexture(control.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!control.texture) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(control.renderer);
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
    if (control.pixels)         free(control.pixels);
    if (control.texture)        SDL_DestroyTexture(control.texture);
    if (control.renderer)       SDL_DestroyRenderer(control.renderer);
    if (control.window)         SDL_DestroyWindow(control.window);
    SDL_Quit();
}


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
            control.sectors.rendered = (int*)malloc( sizeof(int) * sect_count );
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

    control.pixels = (uint32_t*)malloc(sizeof(uint32_t) * SCREEN_HEIGHT * SCREEN_WIDTH);
    if (!control.pixels) return 1;
    memset(control.pixels, 0, sizeof(uint32_t) * SCREEN_HEIGHT * SCREEN_WIDTH);

    pcam.cos = cos(pcam.angle);
    pcam.sin = sin(pcam.angle);

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

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        float dx = 0;

        handle_keys(keystate);

        // Paints the screen black
        SDL_SetRenderDrawColor(control.renderer, 0, 0, 0, 255);
        SDL_RenderClear(control.renderer);

        render();
        
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




