/*
	This file is part of cave9.

	cave9 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cave9 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cave9.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef game_h_included
#define game_h_included

#include "vec.h"

#define FPS 30
#define SECTOR_COUNT 32
#define SEGMENT_COUNT 64
#define SEGMENT_LEN 2.0
#define SHIP_RADIUS 1.0

#define GRAVITY 9.8
#define THRUST (GRAVITY*2)
#define VELOCITY 30.0

#define MAX_CAVE_RADIUS (SHIP_RADIUS*30)
#define MIN_CAVE_RADIUS (SHIP_RADIUS*5)
#define MIN_CAVE_RADIUS_DEPTH 10000

#define MAX_VEL_X GRAVITY
#define MAX_VEL_Y (GRAVITY*4)
#define MAX_VEL_Z VELOCITY

#define MONOLITH_DEPTH  (1*1)
#define MONOLITH_WIDTH  (2*2)
#define MONOLITH_HEIGHT (3*3)

typedef struct {
	float radius;
	Vec3 pos, vel, lookAt;
	bool lefton, righton;
	float dist;  // distance to cave wall
	float start;
} Ship;

typedef struct {
	Vec3 segs[SEGMENT_COUNT][SECTOR_COUNT];
	GLuint gl_list[SEGMENT_COUNT];
	GLuint gl_wire_list[SEGMENT_COUNT];
	int i;  // circular array index

	float monolith_x;
	float monolith_y;
	float monolith_yaw;
} Cave;

void cave_gen(Cave*, Ship* digger);
void cave_init(Cave*, Ship* digger);
void ship_init(Ship*, float radius);
void ship_move(Ship*, float dt);
void digger_control(Ship*);
float collision(Cave*, Ship*);

#endif

// vim600:fdm=syntax:fdn=1:
