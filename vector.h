//Vector stuff & things

#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Vector stuff!
    cross product 2d:
        if > 0, v_1 is counterclockwise from vect; vect is to the right when moving along v_1
        if < 0, v_1 is clockwise from vect; vect is to the left when moving along v_1
        if = 0, vectors are parallel!
    
    dot product:
        equals |v_1||vect|cos(theta) where theta is the angle between?
        that's how the vect_angle dude works
        if v_1 dot vect = 0, the vectors are perpendicular
    
    projection:
        the projection of v_1 onto vect. Like the length of v_1 but pointing the direction of vect

    Also did the intersection.
    Before I continue imma have to test these. rip. 

    NV is a null vector, not a zero vector. By the way. 
*/

#define vector(x, y)            ( (v2){ x, y } )
#define NV                      ( (v2){ NAN, NAN } )
#define add_vect(v_1, v_2)      ( (v2){ v_1.x + v_2.x, v_1.y + v_2.y } )
#define sub_vect(v_1, v_2)      ( (v2){ v_1.x - v_2.x, v_1.y - v_2.y } )
#define mult_vect(v, c)         ( (v2){ v.x * c, v.y * c } )
#define div_vect(v, c)          ( (v2){ v.x / c, v.y / c} )
#define normal_vect(v)          ( div_vect(v, vect_len(v)) )
#define dot_vect(v_1, v_2)      ( v_1.x * v_2.x + v_1.y * v_2.y )
#define cross_vect(v_1, v_2)    ( v_1.x * v_2.y - v_1.y * v_2.x )
#define vect_len(v)             ( sqrtf(v.x * v.x + v.y * v.y) )
#define vect_angle(v_1, v_2)    ( acosf( dot_vect(v_1, v_2) / ( vect_len(v_1) * vect_len(v_2) ) ) )
#define proj_vect(v_1, v_2)     ( mult_vect( vect, ( dot_vect(v_1, v_2) / dot_vect(v_2, v_2) ) ) )
#define invert_vect(v)          ( (v2){ v.x * -1, v.y * -1 } )
#define scale_vect(v, x)        ( (v2){ x, v.y * (x / v.x)} )

typedef struct {
    float x, y;
} v2;

//rotates a vector by angle a
v2 rotate_vect(v2 v, float a);

//checks if two vectors intersect and returns that vector
v2 vector_intersect( v2 v_1, v2 v_2, v2 v_3, v2 v_4 );

//checks what side p is on from the directed vector (v1 -> vect)
int point_side( v2 p, v2 v_1, v2 v_2 );


#endif