//Rendering stuff

#include "render.h"

//Here's the plan, I guess. First, let's figure out an optimal way to do all this
//pixel buffering stuff with the lock texture. Just so things are a little better organized. 
//Then, lets start figuring out the drawing. Simply at first, then we can build up from there.
//make sure everything works correctly before we move forward. 
//keep doing that work (and reading up on it) until I have something that doesn't crash lol. 
//then figure out floors/ceilings, and build the good editor. Idk. It's really just gonna be the first part for a while.

//maybe make the frame struct static w/in this file? just like state? idk. 
//plan is just copy jdh's code and apply it to mine (after I clean up the structure here)
//yeah, just copy the math stuff and figure out how it works...Actually figure out how it works tho

//idk, he seems to be way past me, idk. I'll figure it out though. 
//I copied everything from jdh. :( not what I should be doing. it doesn't even work lol. 
//I do sort of understand more of what I'm doing tho...maybe? idk. I need to do an actual deep dive. 
//go through it and understand how it works. Essentially recreate it by myself in another method. 
//And try to avoid all of his macros, make my own in their respective files. 
//learn from this and some other code and stuff. idk. 

#define PI 3.14159265359f
#define TAU (2.0f * PI)
#define PI_2 (PI / 2.0f)
#define PI_4 (PI / 4.0f)
#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_d) ((_d) * (180.0f / PI))
#define EYE_Z 1.65f
#define HFOV DEG2RAD(90.0f)
#define VFOV 0.5f
#define ZNEAR 0.0001f
#define ZFAR  128.0f
#define min(_a, _b) ({ __typeof__(_a) __a = (_a), __b = (_b); __a < __b ? __a : __b; })
#define max(_a, _b) ({ __typeof__(_a) __a = (_a), __b = (_b); __a > __b ? __a : __b; })
#define clamp(_x, _mi, _ma) (min(max(_x, _mi), _ma))
#define ifnan(_x, _alt) ({ __typeof__(_x) __x = (_x); isnan(__x) ? (_alt) : __x; })


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


static inline v2 wpos_to_cam(frame_t* fr, v2 p) {
    const v2 u = sub_vect( p, pcam->pos );
    return (v2) {
        u.x * cos(pcam->angle) - u.y * sin(pcam->angle),
        u.x * sin(pcam->angle) + u.y * cos(pcam->angle),
    };
}

static inline float normalize_angle(float a) {
    return a - (TAU * floor((a + PI) / TAU));
}


// see: https://en.wikipedia.org/wiki/Line–line_intersection
// compute intersection of two line segments, returns (NAN, NAN) if there is
// no intersection.
static inline v2 intersect_segs(v2 a0, v2 a1, v2 b0, v2 b1) {
    const float d =
        ((a0.x - a1.x) * (b0.y - b1.y))
            - ((a0.y - a1.y) * (b0.x - b1.x));

    if (fabsf(d) < 0.000001f) { return (v2) { NAN, NAN }; }

    const float
        t = (((a0.x - b0.x) * (b0.y - b1.y))
                - ((a0.y - b0.y) * (b0.x - b1.x))) / d,
        u = (((a0.x - b0.x) * (a0.y - a1.y))
                - ((a0.y - b0.y) * (a0.x - a1.x))) / d;
    return (t >= 0 && t <= 1 && u >= 0 && u <= 1) ?
        ((v2) {
            a0.x + (t * (a1.x - a0.x)),
            a0.y + (t * (a1.y - a0.y)) })
        : ((v2) { NAN, NAN });
}


//more jdh code
// rotate vector v by angle a
static inline v2 rotate(v2 v, float a) {
    return (v2) {
        (v.x * cos(a)) - (v.y * sin(a)),
        (v.x * sin(a)) + (v.y * cos(a)),
    };
}

static inline int screen_angle_to_x(float angle) {
    return
        ((int) (1200 / 2))
            * (1.0f - tan(((angle + (HFOV / 2.0)) / HFOV) * PI_2 - PI_4));
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
    //his makes a queue in render and runs through it for each in the queue, instead of doing recursion.
    //tbh, I'm not sure which is better. They're both pretty alright. Maybe mine for recursion depth? idk

    //jdh code:
    const v2
        zdl = rotate(((v2) { 0.0f, 1.0f }), +(HFOV / 2.0f)),
        zdr = rotate(((v2) { 0.0f, 1.0f }), -(HFOV / 2.0f)),
        znl = (v2) { zdl.x * ZNEAR, zdl.y * ZNEAR },
        znr = (v2) { zdr.x * ZNEAR, zdr.y * ZNEAR },
        zfl = (v2) { zdl.x * ZFAR, zdl.y * ZFAR },
        zfr = (v2) { zdr.x * ZFAR, zdr.y * ZFAR };
    
    for (int i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        wall_t* wall = &fr->walls->arr[i];
        
        // translate relative to player and rotate points around player's view
        const v2 
            op1 = wpos_to_cam(fr, wall->p1), 
            op2 = wpos_to_cam(fr, wall->p2);

        // wall clipped pos
        v2 cp1 = op1, cp2 = op2;

        // both are negative -> wall is entirely behind player
        if (cp1.y <= 0 && cp2.y <= 0) continue;

        // angle-clip against view frustum
        float   
            ap1 = normalize_angle(atan2(cp1.y, cp1.x) - PI_2), 
            ap2 = normalize_angle(atan2(cp2.y, cp2.x) - PI_2);

        // clip against view frustum if both angles are not clearly within
        // HFOV
        if (cp1.y < ZNEAR           //yeah this is all clipping
            || cp2.y < ZNEAR
            || ap1 > +(HFOV / 2)
            || ap2 < -(HFOV / 2)) {
            const v2
                il = intersect_segs(cp1, cp2, znl, zfl),
                ir = intersect_segs(cp1, cp2, znr, zfr);

            // recompute angles if points change
            if (!isnan(il.x)) {
                cp1 = il;
                ap1 = normalize_angle(atan2(cp1.y, cp1.x) - PI_2);
            }

            if (!isnan(ir.x)) {
                cp2 = ir;
                ap2 = normalize_angle(atan2(cp2.y, cp2.x) - PI_2);      //gotta figure out what this means lol
            }
        }


        if (ap1 < ap2) {
            continue;
        }

        if ((ap1 < -(HFOV / 2) && ap2 < -(HFOV / 2))        //not a clue
            || (ap1 > +(HFOV / 2) && ap2 > +(HFOV / 2))) {
            continue;
        }

        // // "true" xs before portal clamping
         const int
             tx1 = screen_angle_to_x(ap1),              //this finds the x val, this is an important bit
             tx2 = screen_angle_to_x(ap2);
        // // bounds check against portal window
        // if (tx0 > entry.x1) { continue; }            //cause yeah. If the x isn't in bounds (this is for the sector window), it's not drawn
        // if (tx1 < entry.x0) { continue; }            //wait! I get this! each sector is given a x range to draw based on portal coords

        // draw_line(fr, pixels, tx1, 100, 200, RED);
        // draw_line(fr, pixels, tx2, 100, 200, RED);

        const int
                x1 = clamp(tx1, 0, 1200-1),         //I think this is clipping? probably cut this and add it back later. idk what it is really
                x2 = clamp(tx2, 0, 1200-1);

        const float
                z_floor = sect->zfloor,         //z level stuff & things
                z_ceil = sect->zceil,
                nz_floor =
                    wall->portal ? fr->sectors->arr[wall->portal].zfloor : 0,
                nz_ceil =
                    wall->portal ? fr->sectors->arr[wall->portal].zceil : 0;
        
        const float
            sy0 = ifnan((VFOV * SCREEN_HEIGHT) / cp1.y, 1e10),      //not sure what this does
            sy1 = ifnan((VFOV * SCREEN_HEIGHT) / cp2.y, 1e10);

        const int
            yf1  = (SCREEN_HEIGHT / 2) + (int) (( z_floor - pcam->zpos) * sy0),         //this seems to be the math to convert world to screen pos. 
            yc1  = (SCREEN_HEIGHT / 2) + (int) (( z_ceil  - pcam->zpos) * sy0),         //make sure I remember this shit
            yf2  = (SCREEN_HEIGHT / 2) + (int) (( z_floor - pcam->zpos) * sy1),
            yc2  = (SCREEN_HEIGHT / 2) + (int) (( z_ceil  - pcam->zpos) * sy1),
            nyf1 = (SCREEN_HEIGHT / 2) + (int) ((nz_floor - pcam->zpos) * sy0),
            nyc1 = (SCREEN_HEIGHT / 2) + (int) ((nz_ceil  - pcam->zpos) * sy0),
            nyf2 = (SCREEN_HEIGHT / 2) + (int) ((nz_floor - pcam->zpos) * sy1),
            nyc2 = (SCREEN_HEIGHT / 2) + (int) ((nz_ceil  - pcam->zpos) * sy1),
            txd = tx2 - tx1,
            yfd = yf2 - yf1,
            ycd = yc2 - yc1,
            nyfd = nyf2 - nyf1,
            nycd = nyc2 - nyc1;

        for (int x = x1; x <= x2; x++) {
            // calculate progress along x-axis via tx{0,1} so that walls
            // which are partially cut off due to portal edges still have
            // proper heights
            const float xp = ifnan((x - tx1) / (float) txd, 0);

            // get y coordinates for this x
            const int
                tyf = (int) (xp * yfd) + yf1,
                tyc = (int) (xp * ycd) + yc1,
                yf = clamp(tyf, fr->y_lo[x], fr->y_hi[x]),      //i think this is for clipping?
                yc = clamp(tyc, fr->y_lo[x], fr->y_hi[x]);

            //printf("%d, %d, %d\n", x, tyf, tyc);
            draw_line(fr, pixels, x, tyf, tyc, RED);

        }

    }
}


void render(frame_t* fr, uint32_t* pixels) {
    //clears buffers & stuff for new frame
    memset(fr->y_hi, 0, fr->width * sizeof(int));
    memset(fr->y_lo, 0, fr->width * sizeof(int));
    memset(pixels, BLACK, sizeof(uint32_t) * fr->width * fr->height);
    memset(fr->sectors->rendered, 0, sizeof(int) * fr->width);

    render_sector(fr, pixels, fr->cam->sector, 0);
}