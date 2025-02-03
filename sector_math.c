//Objects for the game thingy
//I should make a header file for this, probably. 

#include "sector_math.h"


//vector functions

static inline vect rotate_vect(vect v, float a) {
    return (vect){ (v.x * cos(a)) - (v.y * sin(a)), (v.x * sin(a) + v.y * cos(a)) };
}

static inline vect vector_intersect( vect v1, vect v2, vect v3, vect v4 ) {
    const float den = (v1.x - v2.x) * (v3.y - v4.y) - (v1.y - v2.y) * (v3.x - v4.x);
    if (den <= 0.0) { return NV; } 

    const float t = (v1.x - v3.x) * (v3.y - v4.y) - (v1.y - v3.y) * (v3.x - v4.x);
    const float u = -1 * ( (v1.x - v2.x) * (v1.y - v3.y) - (v1.y - v2.y) * (v1.x - v3.x) );

    if ( 0.0 <= t && t <= 1 && 0.0 <= u && u <= 1 ) {
        return (vect){ v1.x + t * (v2.x - v1.x), v1.y + t * (v2.y - v1.y) };
    } else {
        return NV;
    }
}

//greater than 0, to the left
//less, to the right
//equal, it's on the line
//maybe just return the value, no ifs. Yeah. 
static inline int point_side( vect p, vect v1, vect v2 ) {
    float out = cross_vect( sub_vect(v2, v1), sub_vect(p, v1) );
    return (out > 0) - (out < 0);
}


//wall stuff?


//sector stuff

//I can't get the walls tho!
static inline vect sect_center(sector_t* sect, wall_t* walls) {
    vect center = {0.0, 0.0};
    int total_points = 0;

    for (int i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        center.x += walls[i].p1.x + walls[i].p2.x;
        center.y += walls[i].p1.y + walls[i].p2.y;
        total_points += 2;
    }

    if (total_points > 0) {
        center.x /= total_points;
        center.y /= total_points;
    }

    return center;
}

int in_sector(vect p, sector_t* sect, wall_t* walls) {
    for (size_t i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        const wall_t wall = walls[i];

        if (point_side(p, wall.p1, wall.p2) > 0) {
            return 0;
        }
    }
    return 1;
}