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
static inline float point_side( vect p, vect v1, vect v2 ) {
    float out = cross_vect( sub_vect(v2, v1), sub_vect(p, v1) );
    return out;
}


int test() {
    vect v1 = {5.0, 10.0};
    vect v2 = {10.0, 5.0};

    vect p = {12.0, 8.0};

    int out = point_side(p, v1, v2);
    printf("and its...%d\n", out);
    return 1;
}