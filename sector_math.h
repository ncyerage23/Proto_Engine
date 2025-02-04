#ifndef SECTOR_MATH_H
#define SECTOR_MATH_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//color rgb values and stuff
#define WHITE   ( (color_t){255, 255, 255, 255} )
#define BLACK   ( (color_t){0, 0, 0, 255} )
#define RED     ( (color_t){255, 0, 0, 255} )
#define GREEN   ( (color_t){0, 255, 0, 255} )
#define BLUE    ( (color_t){0, 0, 255, 255} )

typedef struct {
    int r, g, b, a;
} color_t; 



/* Vector stuff!
    cross product 2d:
        if > 0, v1 is counterclockwise from vect; vect is to the right when moving along v1
        if < 0, v1 is clockwise from vect; vect is to the left when moving along v1
        if = 0, vectors are parallel!
    
    dot product:
        equals |v1||vect|cos(theta) where theta is the angle between?
        that's how the vect_angle dude works
        if v1 dot vect = 0, the vectors are perpendicular
    
    projection:
        the projection of v1 onto vect. Like the length of v1 but pointing the direction of vect

    Also did the intersection.
    Before I continue imma have to test these. rip. 

    NV is a null vector, not a zero vector. By the way. 
*/

#define vector(x, y)            ( (vect){ x, y } )
#define NV                      ( (vect){ NAN, NAN } )
#define add_vect(v1, v2)        ( (vect){ v1.x + v2.x, v1.y + v2.y } )
#define sub_vect(v1, v2)        ( (vect){ v1.x - v2.x, v1.y - v2.y } )
#define mult_vect(v, c)         ( (vect){ v.x * c, v.y * c } )
#define div_vect(v, c)          ( (vect){ v.x / c, v.y / c} )
#define normal_vect(v)          ( div_vect(v, vect_len(v)) )
#define dot_vect(v1, v2)        ( v1.x * v2.x + v1.y * v2.y )
#define cross_vect(v1, v2)      ( v1.x * v2.y - v1.y * v2.x )
#define vect_len(v)             ( sqrtf(v.x * v.x + v.y * v.y) )
#define vect_angle(v1, v2)      ( acosf( dot_vect(v1, vect) / ( vect_len(v1) * vect_len(v2) ) ) )
#define proj_vect(v1, v2)       ( mult_vect( vect, ( dot_vect(v1, v2) / dot_vect(v2, v2) ) ) )
#define invert_vect(v)          ( (vect){ v.x * -1, v.y * -1 } )

typedef struct {
    float x, y;
} vect;

//rotates a vector
static inline vect rotate_vect(vect v, float a);

//checks if two vectors intersect and returns that vector
static inline vect vector_intersect( vect v1, vect v2, vect v3, vect v4 );

//checks what side p is on from the directed vector (v1 -> vect)
static inline int point_side( vect p, vect v1, vect vect );

//will delete l8r
int test();


//walls
#define wall_intersect(w1, w2)      ( vector_intersect( w1.p1, w1.p2, w2.p1, w2.p2 ) )
typedef struct {
    vect p1, p2;
    int portal;
} wall_t;

//what other wall functions do I need? Idek. 


//sectors
typedef struct {
    int id;
    int first_wall, num_walls;
    float zfloor, zceil;
} sector_t;

static inline vect sect_center(sector_t* sect, wall_t* walls);

int in_sector(vect p, sector_t* sect, wall_t* walls);



#endif