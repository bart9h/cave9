
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "vec.h"
#include "game.h"

#define GRAVITY 9.8
#define THRUST (GRAVITY * 2)
float velocity = 30.0;

void cave_gen(Cave *cave, Ship *digger)
{
	// check if the digger advanced to the next segment
	int i = (cave->i-1+SEGMENT_COUNT)%SEGMENT_COUNT;
	if( digger->pos[2] > 1 && digger->pos[2] - SEGMENT_LEN < cave->segs[i][0][2] )
		return;

	// invalidate GL list for this segment
	cave->gl_list[cave->i] = 0;

	// generate new segment
	cave->seg_y[i][0] = FLT_MAX; // ymin
	cave->seg_y[i][1] = FLT_MIN; // ymax
	for( i = 0; i < SECTOR_COUNT; ++i ) {
		float a = i*M_PI*2/SECTOR_COUNT;
		float r = digger->radius;
		SET(cave->segs[cave->i][i],
			digger->pos[0] + r*cos(a),
			digger->pos[1] + r*sin(a),
			((int)(digger->pos[2]/SEGMENT_LEN))*SEGMENT_LEN
		);

		if(cave->segs[cave->i][i][1] < cave->seg_y[cave->i][0])
			cave->seg_y[cave->i][0] = cave->segs[cave->i][i][1];
		if(cave->segs[cave->i][i][1] > cave->seg_y[cave->i][1])
			cave->seg_y[cave->i][1] = cave->segs[cave->i][i][1];
	}

	// increment segment circular pointer
	cave->i ++;
	if( cave->i >= SEGMENT_COUNT )
		cave->i = 0;
}

void cave_init(Cave *cave, Ship *digger)
{
	cave->i = 0;
	do {
		digger_control(digger);
		ship_move(digger, 1./FPS);
		cave_gen(cave, digger);
	}
	while(cave->i != 0);
}

void ship_init(Ship* ship, float radius)
{
	SET(ship->pos,0,0,0);
	SET(ship->vel,0,0,velocity);
	ship->radius = radius;
}

void ship_move(Ship *ship, float dt)
{
#if 1
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
#else
	if(ship->lefton)
		ship->vel[1] += THRUST*dt;

	ship->vel[1] -= GRAVITY*dt;

	ship->pos[1] += ship->vel[1]*dt;
#endif
}

void digger_control(Ship *ship)
{
#if 0
	ship->lefton  = rand()%2 ? true : false;
	ship->righton = rand()%2 ? true : false;
#else
	static bool cave_change = true;
	static float repeat = 0;

	float skill_factor = 200;
	float skill = log((ship->pos[2]+skill_factor)/skill_factor);
	if(repeat * ship->vel[1] < 0)
		repeat = 0;
	repeat += ship->vel[1];

	float velocity_change_rate = .01;
	float repeat_change_rate = .0001;
	float h_factor = 10;
	int dir = ship->lefton || ship->righton ? -1: 1;
	if(
		(RAND < fabs(velocity_change_rate*(h_factor*ship->vel[0]+ship->vel[1])/skill) && ship->vel[1]*dir < 0)
		|| (RAND < fabs(repeat_change_rate*repeat * skill) && repeat*dir < 0)
	) {
		if(RAND < .5) 
			ship->lefton  = !ship->lefton;
		else
			ship->righton  = !ship->righton;
	}
	if(RAND < fabs(velocity_change_rate*(h_factor*ship->vel[0]+ship->vel[1])/skill))
		ship->righton = ship->lefton;

	float max_cave_height = 10+30/(1+skill);
	float min_cave_height = max_cave_height/skill;

	float cave_change_rate = .2;
	if(RAND > cave_change_rate/skill)
		 cave_change = !cave_change;

	if(ship->radius < min_cave_height) {
		cave_change = !cave_change;
		ship->radius = min_cave_height;
	}

	if(ship->radius > max_cave_height) {
		cave_change = !cave_change;
		ship->radius = max_cave_height;
	}

	if(cave_change)
		ship->radius += RAND>.5?-1:1;
#endif
	//printf("l(%d), r(%d)\n", ship->lefton, ship->righton);
}

static float X(Cave *cave, int i, float xn, float yn, int k0, int k1)
{
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

float collision(Cave *cave, Ship *ship)
{
	float min = FLT_MAX;

	// This method counts the number of intersections of a semi-line
	// starting from the point being checked against the poligon,
	// that in this case is the segment of the cave.
	// If the number of intersections is odd, the point is inside.

	// In fact we'll check four points around the center of the ship
	// (to simulate a diamond-shaped bounding box).

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
		//printf("intersection_count[%d] = %d\n", i, intersection_count[i]);
		if(intersection_count[i] % 2 == 0) {
			return ship->dist = 0;
		}
	}

	ship->dist = min - 2*ship->radius;
	return min;
}

// vim600:fdm=syntax:fdn=1:

