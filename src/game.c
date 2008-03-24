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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include "vec.h"
#include "game.h"

Ship* nearest_digger (Ship* player, Ship* diggers[2])
{
	#define D(a,b)\
	((a->pos[0]-b->pos[0])*(a->pos[0]-b->pos[0])\
	+(a->pos[1]-b->pos[1])*(a->pos[1]-b->pos[1]))
	return diggers[ (D(player,diggers[0]) < D(player,diggers[1])) ? 0 : 1 ];
}

void cave_gen (Cave* cave, Ship* player, Ship* diggers[2])
{
	// diggers must be synchronized  (same Z)
	float digZ = diggers[0]->pos[2];
	assert (digZ == diggers[1]->pos[2]);

	// check if the digger advanced to the next segment
	int i = (cave->i - 1 + SEGMENT_COUNT) % SEGMENT_COUNT;
	if (digZ > diggers[0]->start+1  &&  digZ < cave->segs[i][0][2])
		return;

	// invalidate GL list for this segment
	if(glIsList(cave->gl_list[cave->i]))
		glDeleteLists(cave->gl_list[cave->i], 1);
	cave->gl_list[cave->i] = 0;

	if(glIsList(cave->gl_wire_list[cave->i]))
		glDeleteLists(cave->gl_wire_list[cave->i], 1);
	cave->gl_wire_list[cave->i] = 0;

	// generate new segment
	{
		Vec3 d0 = diggers[0]->pos;
		Vec3 d1 = diggers[1]->pos;
		float r0 = diggers[0]->radius;
		float r1 = diggers[1]->radius;

		float int0x, int0y, int1x, inty1;
		int n = circle_circle_intersection (
				d0[0], d0[1], r0,
				d1[0], d1[1], r1,
				&int0x, &int0y, &int1x, &int1y
		);
		if (n == 2) {
			//TODO
		}
		else {
			Vec3 pl = player->pos;
			Ship* dig = diggers[
				(n == 1) ?
				// circles inside each other, pick bigger one
				( (diggers[0]->radius > diggers[1]->radius) ? 0 : 1 ) :
				// no intersection, check which one has the player ship
				( ((pl[0]-d0[0])*(pl[0]-d0[0])+(pl[1]-d0[1])*(pl[1]-d0[1])) < r0*r0 ? 0 : 1 )
			];

			for( i = 0; i < SECTOR_COUNT; ++i ) {
				float a = M_PI_2+(i-1)*M_PI*2/SECTOR_COUNT;
				float r = dig->radius;
				SET(cave->segs[cave->i][i],
					dig->pos[0] + r*cos(a) + RAND,
					dig->pos[1] + r*sin(a) + RAND,
					((int)(digZ/SEGMENT_LEN))*SEGMENT_LEN
				);
			}
		}
	}

	// increment segment circular pointer
	cave->i ++;
	if( cave->i >= SEGMENT_COUNT ) {
		cave->i = 0;

		// place monolith
		cave->monolith_x = digger->pos[0] + (2*RAND-1)*(digger->radius-MONOLITH_WIDTH);
		cave->monolith_y = digger->pos[1] + (2*RAND-1)*(digger->radius-MONOLITH_HEIGHT);
		cave->monolith_yaw = atan2(digger->vel[0], digger->vel[2]);
	}
}

void cave_init (Cave* cave, Ship* diggers[2])
{
	cave->i = 0;
	do {
		digger_control(digger);
		ship_move(digger, 1./FPS);
		cave_gen(cave, digger);
	}
	while(cave->i != 0);
}

void ship_init (Ship* ship, float radius)
{
	SET(ship->pos,0,0,ship->start);
	SET(ship->vel,0,0,VELOCITY);
	SET(ship->lookAt,0,0,VELOCITY);
	ship->radius = radius;
	ship->dist = FLT_MAX;
}

void ship_move (Ship* ship, float dt)
{
	float a = THRUST*dt;

	if(ship->lefton) {
		Vec3 leftup = {-a,a/2,0};
		ADD(ship->vel, leftup);
	}

	if(ship->righton) {
		Vec3 rightup = {+a,a/2,0};
		ADD(ship->vel, rightup);
	}

	ship->vel[1] -= GRAVITY*dt;

	ADDSCALE(ship->pos, ship->vel, dt);

	{
		float smoothness = 0.2;
		Vec3 d;
		SUB2 (d, ship->vel, ship->lookAt);
		ADDSCALE (ship->lookAt, d, smoothness);
	}
}

void digger_control (Ship* ship)
{
	float twist_factor = 500;
	float noise = .1;
	float twist = 1 - 1/(1 + ship->pos[2]/twist_factor);
	float max_vel[3] = { 
		MAX_VEL_X * twist, 
		MAX_VEL_Y * twist, 
		MAX_VEL_Z
	};

	if( 
			ship->vel[1] >  max_vel[1] || 
			ship->vel[1] < -max_vel[1] || 
			ship->vel[0] >  max_vel[0] ||
			ship->vel[0] < -max_vel[0] ||
			RAND < twist*noise
		) 
	{
		if(RAND>twist/2)
			ship->lefton = RAND<twist*noise ? rand()%2 :
				ship->vel[1] < 0 || ship->vel[0] > +max_vel[0]; 

		if(RAND>twist/2)
			ship->righton = RAND<twist*noise ? rand()%2 :
				ship->vel[1] < 0 || ship->vel[0] < -max_vel[0];
	}

	float scale = 1-MIN(1,log(1+ship->pos[2])/log(1+MIN_CAVE_RADIUS_DEPTH));
	ship->radius = MIN_CAVE_RADIUS+(MAX_CAVE_RADIUS-MIN_CAVE_RADIUS)*scale+RAND;
}


static float X (Cave* cave, int i, float xn, float yn, int k0, int k1)
{// used by collision()

	float x1 = cave->segs[i][k0][0];
	float y1 = cave->segs[i][k0][1];
	float x2 = cave->segs[i][k1][0];
	float y2 = cave->segs[i][k1][1];
	float t = (yn - y2)/(y1 - y2);
	if( t < 0 || t > 1 )
		return 0;
	float x = (x1 - xn)*t + (x2 - xn)*(1 - t);
	//printf("t(%f), x(%f)\n", t, x);
	return x;
}

float collision (Cave* cave, Ship* ship)
{
	float min = FLT_MAX;

	// This method counts the number of intersections of a semi-line
	// starting from the point being checked against the poligon,
	// that in this case is the segment of the cave.
	// If the number of intersections is odd, the point is inside.

	// In fact we'll check four points around the center of the ship
	// (to simulate a diamond-shaped bounding box).

	// The return value is the distance from the ship to the cave.

	int intersection_count[4];
	memset(intersection_count, 0, sizeof(intersection_count));

	int i = cave->i;
	int k;
	for( k = 0; k < SECTOR_COUNT; ++k ) {
		int i0 = (k+0)%SECTOR_COUNT;
		int i1 = (k+1)%SECTOR_COUNT;

		Vec3 dist;
		SUB2(dist, ship->pos, cave->segs[i][i0]);
		float len = LEN(dist);
		if(len < min)
			min = len;

		// optimize
		if(cave->segs[i][i0][0] < ship->pos[0]-ship->radius &&
				cave->segs[i][i1][0] < ship->pos[0]-ship->radius)
			continue;
		if(cave->segs[i][i0][1] > ship->pos[1]+ship->radius &&
				cave->segs[i][i1][1] > ship->pos[1]+ship->radius)
			continue;
		if(cave->segs[i][i0][1] < ship->pos[1]-ship->radius &&
				cave->segs[i][i1][1] < ship->pos[1]-ship->radius)
			continue;

		if(X(cave, i, ship->pos[0] - ship->radius, ship->pos[1], i0, i1) > 0)
			++intersection_count[0];

		if(X(cave, i, ship->pos[0], ship->pos[1] + ship->radius, i0, i1) > 0)
			++intersection_count[1];

		if(X(cave, i, ship->pos[0] + ship->radius, ship->pos[1], i0, i1) > 0)
			++intersection_count[2];

		if(X(cave, i, ship->pos[0], ship->pos[1] - ship->radius, i0, i1) > 0)
			++intersection_count[3];
	}

	for(i = 0; i < 4; ++i) {
		if(intersection_count[i] % 2 == 0) {
			return ship->dist = 0;  // hit
		}
	}

	ship->dist = min - 2*ship->radius;
	return min;  // miss
}

// vim600:fdm=syntax:fdn=2:
