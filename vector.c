//Function declarations for vector math

#include "vector.h"

static inline v2 rotate_vect(v2 v, float a) {
    return (v2){ (v.x * cos(a)) - (v.y * sin(a)), (v.x * sin(a) + v.y * cos(a)) };
}


static inline v2 vector_intersect( v2 v_1, v2 v_2, v2 v_3, v2 v_4 ) {
    const float den = (v_1.x - v_2.x) * (v_3.y - v_4.y) - (v_1.y - v_2.y) * (v_3.x - v_4.x);
    if (den <= 0.0) { return NV; } 

    const float t = (v_1.x - v_3.x) * (v_3.y - v_4.y) - (v_1.y - v_3.y) * (v_3.x - v_4.x);
    const float u = -1 * ( (v_1.x - v_2.x) * (v_1.y - v_3.y) - (v_1.y - v_2.y) * (v_1.x - v_3.x) );

    if ( 0.0 <= t && t <= 1 && 0.0 <= u && u <= 1 ) {
        return (v2){ v_1.x + t * (v_2.x - v_1.x), v_1.y + t * (v_2.y - v_1.y) };
    } else {
        return NV;
    }
}


//greater than 0, to the left
//less, to the right
//equal, it's on the line
//maybe just return the value, no ifs. Yeah. 
static inline int point_side( v2 p, v2 v_1, v2 v_2 ) {
    float out = cross_vect( sub_vect(v_2, v_1), sub_vect(p, v_1) );
    return (out > 0) - (out < 0);
}

