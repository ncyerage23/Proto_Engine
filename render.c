//Rendering stuff

#include "render.h"

//Here's the plan, I guess. First, let's figure out an optimal way to do all this
//pixel buffering stuff with the lock texture. Just so things are a little better organized. 
//Then, lets start figuring out the drawing. Simply at first, then we can build up from there.
//make sure everything works correctly before we move forward. 
//keep doing that work (and reading up on it) until I have something that doesn't crash lol. 
//then figure out floors/ceilings, and build the good editor. Idk. It's really just gonna be the first part for a while.

#define MAX_PORTALS     5
#define SCREEN_HEIGHT   fr->height
#define SCREEN_WIDTH    fr->width
#define pcam            fr->cam
#define FOV_SCALE       600         //change this l8r, idk


frame_t* frame_create(int width, int height, void* sectors, void* walls, camera_t* cam) {
    frame_t* out = (frame_t*)malloc( sizeof(frame_t) );
    if (!out) return NULL;

    out->width = width;
    out->height = height;

    out->y_hi = (int*)malloc( sizeof(int) * width );
    out->y_lo = (int*)malloc( sizeof(int) * width );
    if ( !(out->y_hi && out->y_lo) ) {
        frame_destroy(out);
        return NULL;
    }

    memset(out->y_hi, 0, width * sizeof(int));
    memset(out->y_lo, 0, width * sizeof(int));

    out->cam = cam;
    out->sectors = sectors;
    out->walls = walls;


    return out;
}

void frame_destroy(frame_t* fr) {
    if (!fr) return;
    if (fr->y_hi) free(fr->y_hi);
    if (fr->y_lo) free(fr->y_lo);
    free(fr);
}


void draw_line(frame_t* fr, uint32_t* pixels, int x, int top, int bottom, uint32_t color) {
    if (x < 0 || x >= SCREEN_WIDTH) return;

    if (top < 0) top = 0;
    if (bottom >= SCREEN_HEIGHT) bottom = SCREEN_HEIGHT;

    for (int y = top; y <= bottom; y++) { pixels[y * fr->width + x] = color; }
}

void draw_point(frame_t* fr, uint32_t* pixels, int x, int y, uint32_t color) {
    for (int i = x-1; i <= x+1; i++) {
        for (int j = y-1; j<=y+1; j++) {
            pixels[j * SCREEN_WIDTH + i] = color;
        }
    }
}


static inline v2 wpos_to_cam(frame_t* fr, v2 p) {
    const v2 u = sub_vect( p, pcam->pos );
    return (v2) {
        u.x * cos(pcam->angle) - u.y * sin(pcam->angle),
        u.x * sin(pcam->angle) + u.y * cos(pcam->angle),
    };
}

// i guess just figure the math out again, and do it correctly this time. 
// dude idek what could be going wrong. this sucks.
// idk what I have to do to fix this, but i should really do it. I can't do anything else until I do, lol. 
void render_sector(frame_t* fr, uint32_t* pixels, int sect_id, int depth) {
    if (sect_id < 0 || sect_id >= fr->sectors->n) return;
    if (fr->sectors->rendered[sect_id] == 1) return;
    if (depth >= 5) return;

    fr->sectors->rendered[sect_id] = 1;
    sector_t* sect = &fr->sectors->arr[sect_id];

    //maybe make a stack/queue for next to render? yeah, def do that. it'll be easier than what I have
    
    for (int i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        wall_t* wall = &fr->walls->arr[i];
        draw_point(fr, pixels, wall->p1.x, wall->p1.y, (sect_id) ? BLUE : RED);
        draw_point(fr, pixels, wall->p2.x, wall->p2.y, (sect_id) ? BLUE : RED);


        if (wall->portal != -1) {
            if (fr->sectors->rendered[wall->portal] == 0) {
                render_sector(fr, pixels, wall->portal, depth + 1);
            }
        }
    }
}


void render(frame_t* fr, uint32_t* pixels) {
    //clears buffers & stuff for new frame
    memset(fr->y_hi, 0, fr->width * sizeof(int));
    memset(fr->y_lo, 0, fr->width * sizeof(int));
    memset(pixels, BLACK, sizeof(uint32_t) * fr->width * fr->height);
    memset(fr->sectors->rendered, 0, sizeof(int) * fr->width);

    draw_point(fr, pixels, pcam->pos.x, pcam->pos.y, WHITE);
    render_sector(fr, pixels, fr->cam->sector, 0);
}