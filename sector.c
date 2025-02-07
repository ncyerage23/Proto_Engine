//Func declarations for sectors

#include "sector.h"

v2 sect_center(sector_t* sect, wall_t* walls) {
    v2 center = {0.0, 0.0};
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

int in_sector(v2 p, sector_t* sect, wall_t* walls) {
    for (size_t i = sect->first_wall; i < sect->first_wall + sect->num_walls; i++) {
        const wall_t wall = walls[i];

        if (point_side(p, wall.p1, wall.p2) > 0) {
            return 0;
        }
    }
    return 1;
}