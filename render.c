//Rendering stuff

#include "render.h"

//So, I got it to draw. Now, I need to figure out what's making it crash. that's all. 
//I wasn't handling keys and stuff, lol. Good looking. 
//I don't know what the issue is, but we'll get there. 
//idk what's happening anymore, but it's sort of getting somewhere??? Who mfin knows.
//I think I just actually learn everything about what I'm doing
//this sucks

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

    out->pixels = (uint32_t*)malloc( sizeof(uint32_t) * width * height * 4);
    if (!out->pixels) {
        free(out);
        return NULL;
    }

    out->y_hi = (int*)malloc( sizeof(int) * width );
    out->y_lo = (int*)malloc( sizeof(int) * width );
    if ( !(out->y_hi && out->y_lo) ) {
        frame_destroy(out);
        return NULL;
    }

    memset(out->y_hi, 0, width * sizeof(int));
    memset(out->y_lo, 0, width * sizeof(int));
    memset(out->pixels, BLACK, sizeof(uint32_t) * width * height * 4);

    out->cam = cam;
    out->sectors = sectors;
    out->walls = walls;


    return out;
}

void frame_destroy(frame_t* fr) {
    if (!fr) return;
    if (fr->pixels) free(fr->pixels);
    if (fr->y_hi) free(fr->y_hi);
    if (fr->y_lo) free(fr->y_lo);
    free(fr);
}


void reset_stuff(frame_t* fr) {
    memset(fr->y_hi, 0, fr->width * sizeof(int));
    memset(fr->y_lo, 0, fr->width * sizeof(int));
    memset(fr->pixels, BLACK, sizeof(uint32_t) * fr->width * fr->height * 4);
    memset(fr->sectors->rendered, 0, sizeof(int) * fr->width);
}

void draw_line(frame_t* fr, int x, int top, int bottom, uint32_t color) {
    if (x < 0 || x >= SCREEN_WIDTH) return;

    if (top < 0) top = 0;
    if (bottom >= SCREEN_HEIGHT) bottom = SCREEN_HEIGHT;

    for (int y = top; y <= bottom; y++) { fr->pixels[y * fr->width + x] = color; }
}


static inline v2 wpos_to_cam(frame_t* fr, v2 p) {
    //const v2 u = { p.x - fr.cam->pos->x, p.y - pcam->pos->y };
    const v2 u = sub_vect( p, pcam->pos );
    return (v2) {
        u.x * cos(pcam->angle) - u.y * sin(pcam->angle),
        u.x * sin(pcam->angle) + u.y * cos(pcam->angle),
    };
}

//don't work. idk why. probably all of its dumb. gonna have to rewrite the whole thing ig. Idk. 
//really don't feel like it tho. 
void render_sector(frame_t* fr, int sect_id) {
    if (sect_id < 0 || sect_id >= fr->sectors->n || fr->sectors->rendered[sect_id] == 1) return;
    
    fr->sectors->rendered[sect_id] = 1;
    sector_t* sect = &fr->sectors->arr[sect_id];

    struct { int arr[MAX_PORTALS]; int n; } portals;
    portals.n = 0;
    int hh = (SCREEN_HEIGHT/2), hw = (SCREEN_WIDTH/2);

    float wz1_floor = pcam->zpos - sect->zfloor;
    float wz1_ceil = pcam->zpos - sect->zceil;
    float wz2_floor = pcam->zpos - sect->zfloor;
    float wz2_ceil = pcam->zpos - sect->zceil;

    v2 w1, w2;
    for (int i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        wall_t* wall = &fr->walls->arr[i];
        w1 = wpos_to_cam(fr, wall->p1);
        w2 = wpos_to_cam(fr, wall->p2);

        if (w1.y <= 0 || w2.y <= 0) continue;

        float inv_wy1 = FOV_SCALE / w1.y;
        float inv_wy2 = FOV_SCALE / w2.y;

        //convert world x to screen x
        w1.x = w1.x * inv_wy1 + hw;
        w2.x = w2.x * inv_wy2 + hw;

        //idk if these are right
        if (w1.x > SCREEN_WIDTH && w2.x > SCREEN_WIDTH) continue;
        if (w1.x < 0 && w2.x < 0) continue;

        //convert world y to screen y
        float wy1_bottom = wz1_floor * inv_wy1 + hh;
        float wy2_bottom = wz2_floor * inv_wy2 + hh;
        float wy1_top = wz1_ceil * inv_wy1 + hh;
        float wy2_top = wz2_ceil * inv_wy2 + hh;

        if (wall->portal == -1) {
            v2 floor = normal_vect( ( (v2){w2.x - w1.x, wy2_bottom - wy1_bottom} ) );
            v2 ceil = normal_vect( ( (v2){w2.x - w1.x, wy2_top - wy1_top} ) );

            //scale for finding top and bottom coords of line
            float scale_floor = (floor.y / floor.x);
            float scale_ceil = (ceil.y / ceil.x);

            for( int x = 0; x < w2.x - w1.x; x += 1 ) {
                int x_val = w1.x + x;
                int bottom = wy1_bottom + (scale_floor * x);
                int top = wy1_top + (scale_ceil * x);

                draw_line(fr, x_val, top, bottom, RED);
            }
        } else {
            portals.arr[portals.n] = wall->portal;
            portals.n++;
        }
    }

    if (portals.n != 0) {
        for (int i = 0; i < portals.n; i++) {
            render_sector(fr, portals.arr[i]);
        }
    }

}

void render(frame_t* fr) {
    reset_stuff(fr);
    
    render_sector(fr, 0);
}