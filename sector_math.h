#ifndef SECTOR_MATH_H
#define SECTOR_MATH_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//color rgb values and stuff
typedef struct {
    int r, g, b, a;
} color_t; 



/* Vector stuff!
    cross product 2d:
        if > 0, v1 is counterclockwise from v2; v2 is to the right when moving along v1
        if < 0, v1 is clockwise from v2; v2 is to the left when moving along v1
        if = 0, vectors are parallel!
    
    dot product:
        equals |v1||v2|cos(theta) where theta is the angle between?
        that's how the vect_angle dude works
        if v1 dot v2 = 0, the vectors are perpendicular
    
    projection:
        the projection of v1 onto v2. Like the length of v1 but pointing the direction of v2

    Also did the intersection.
    Before I continue imma have to test these. rip. 

    NV is a null vector, not a zero vector. By the way. 
*/

#define vector(x, y)            ( (vector_t){ x, y } )
#define NV                      ( (vector_t){ NAN, NAN } )
#define add_vect(v1, v2)        ( (vector_t){ v1.x + v2.x, v1.y + v2.y } )
#define sub_vect(v1, v2)        ( (vector_t){ v1.x - v2.x, v1.y - v2.y } )
#define mult_vect(v, c)         ( (vector_t){ v.x * c, v.y * c } )
#define dot_vect(v1, v2)        ( v1.x * v2.x + v1.y * v2.y )
#define cross_vect(v1, v2)      ( v1.x * v2.y - v1.y * v2.x )
#define vect_len(v)             ( sqrtf(v.x * v.x + v.y * v.y) )
#define vect_angle(v1, v2)      ( acosf( dot_vect(v1, v2) / ( vect_len(v1) * vect_len(v2) ) ) )
#define proj_vect(v1, v2)       ( mult_vect( v2, ( dot_vect(v1, v2) / dot_vect(v2, v2) ) ) )
#define invert_vect(v)          ( (vector_t){ v.x * -1, v.y * -1 } )

typedef struct {
    float x, y;
} vector_t;

//rotates a vector
static inline vector_t rotate_vect(vector_t v, float a);

//checks if two vectors intersect and returns that vector
static inline vector_t vector_intersect( vector_t v1, vector_t v2, vector_t v3, vector_t v4 );

//checks what side p is on from the directed vector (v1 -> v2)
static inline int point_side( vector_t p, vector_t v1, vector_t v2 );

int test();


//walls
#define wall_intersect(w1, w2)      ( vector_intersect( w1.p1, w1.p2, w2.p1, w2.p2 ) )
typedef struct {
    vector_t p1, p2;
    int portal;
} wall_t;


//sectors
typedef struct {
    int id;
    int first_wall, num_walls;
    float zfloor, zceil;
} sector_t;




#endif