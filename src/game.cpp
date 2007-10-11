
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
	if(glIsList(cave->gl_list[cave->i]))
		glDeleteLists(cave->gl_list[cave->i], 1);
	cave->gl_list[cave->i] = 0;

	// generate new segment
	for( i = 0; i < SECTOR_COUNT; ++i ) {
		float a = M_PI_2+(i-1)*M_PI*2/SECTOR_COUNT;
		float r = digger->radius;
		SET(cave->segs[cave->i][i],
			digger->pos[0] + r*cos(a),
			digger->pos[1] + r*sin(a),
			((int)(digger->pos[2]/SEGMENT_LEN))*SEGMENT_LEN
		);
	}

	// increment segment circular pointer
	cave->i ++;
	if( cave->i >= SEGMENT_COUNT ) {
		cave->i = 0;

		// place monolith
		cave->monolith_x = digger->pos[0] + (2*RAND-1)*(digger->radius-MONOLITH_WIDTH);
		cave->monolith_y = digger->pos[1] + (2*RAND-1)*(digger->radius-MONOLITH_HEIGHT);
		cave->monolith_yaw   = atan2(digger->vel[0], digger->vel[2]);
	}
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
	ship->dist = FLT_MAX;
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
	float start = 0;
	float depth = 10;
	float noise = .1;
	float skill = 1-1/(1+ (start+ship->pos[2])/depth );
	float max_vel[3] = { 
		GRAVITY*skill, 
		GRAVITY*skill*4, 
		velocity 
	};

	if( 
			ship->vel[1] >  max_vel[1] || 
			ship->vel[1] < -max_vel[1] || 
			ship->vel[0] >  max_vel[0] ||
			ship->vel[0] < -max_vel[0] ||
			RAND<skill*noise
		) 
	{
		if(RAND>skill/2)
			ship->lefton = RAND<skill*noise ? rand()%2 :
				ship->vel[1] < 0 || ship->vel[0] > +max_vel[0]; 

		if(RAND>skill/2)
			ship->righton = RAND<skill*noise ? rand()%2 :
				ship->vel[1] < 0 || ship->vel[0] < -max_vel[0];
	}

	float min_radius = SHIP_RADIUS*5;
	float max_radius = SHIP_RADIUS*30;
	float factor = 10000;
	float scale = log(1+(1-skill)*factor)/log(1+depth*factor);
	ship->radius = MAX(min_radius, max_radius*scale+RAND);
}

static float X(Cave *cave, int i, float xn, float yn, int k0, int k1)
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

