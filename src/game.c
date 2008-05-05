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

float cave_len (Cave *cave)
{
	int head = cave->i;
	int tail = (head - 1 + SEGMENT_COUNT) % SEGMENT_COUNT;
	return cave->segs[tail][0][2] - cave->segs[head][0][2];
}

void cave_gen (Cave* cave, Digger* digger)
{
	Ship *ship = SHIP(digger);
	// check if the ship advanced to the next segment
	int i = (cave->i - 1 + SEGMENT_COUNT) % SEGMENT_COUNT;
	if (ship->pos[2] > ship->start+1  &&  ship->pos[2]-SEGMENT_LEN < cave->segs[i][0][2])
		return;

	// invalidate GL list for this segment
	for (int mode = 0; mode < DISPLAYMODE_COUNT; ++mode) {
		if (glIsList (cave->gl_list[mode][cave->i]))
			glDeleteLists (cave->gl_list[mode][cave->i], 1);
		cave->gl_list[mode][cave->i] = 0;
	}

	// generate new segment
	for( i = 0; i < SECTOR_COUNT; ++i ) {
		float a = M_PI_2+(i-1)*M_PI*2/SECTOR_COUNT;
		float r = ship->radius;

		float cos_a = cos(a);
		float sin_a = sin(a);

		float mult_x = (cos_a > 0)? digger->y_top_radius : digger->y_bottom_radius;
		float mult_y = (sin_a > 0)? digger->x_left_radius: digger->x_right_radius;

		// clamp the multipliers to [1 .. 2]
		mult_x = (3 + sin(mult_x))/2;
		mult_y = (3 + sin(mult_y))/2;

		SET(cave->segs[cave->i][i],
			ship->pos[0] + (r * mult_x * cos_a) + RAND,
			ship->pos[1] + (r * mult_y * sin_a) + RAND,
			ship->pos[2]
		);
	}
	COPY (cave->centers[cave->i], ship->pos);

	// increment segment circular pointer
	cave->i = (cave->i + 1) % SEGMENT_COUNT;

	// place monolith on rooms
	if (ship->pos[2] > cave->monolith_pos[2]+ROOM_SPACING)
	{
		cave->monolith_pos[0] = ship->pos[0];
		cave->monolith_pos[1] = ship->pos[1];
		cave->monolith_pos[2] = ROOM_LEN/2 + ROOM_SPACING*(int)(ship->pos[2]/ROOM_SPACING);
		cave->monolith_yaw = atan2 (ship->vel[0], ship->vel[2]);
	}
}

static void cave_init (Cave* cave, Digger* digger, int game_mode)
{
	Ship *ship = SHIP(digger);
	memset (cave, 0, sizeof(Cave));
	cave->i = 0;
	do {
		digger_control(digger, game_mode);
		ship_move(ship, 1./FPS);
		cave_gen(cave, digger);
	}
	while(cave->i != 0);
}

static void ship_init (Ship* ship, float radius)
{
	SET (ship->pos, 0,0,ship->start);
	SET (ship->vel, 0,0,VELOCITY);
	SET (ship->lookAt, 0,0,VELOCITY);
	ship->radius = radius;
	ship->dist = FLT_MAX;
	SET(ship->repulsion,0,1,0);
	ship->lefton = ship->righton = false;
}

static void digger_init(Digger *digger, float radius)
{
	ship_init(SHIP(digger), radius);

	digger->x_left_radius = 0.0;
	digger->x_right_radius = 0.0;
	digger->y_top_radius = 0.0;
	digger->y_bottom_radius = 0.0;
}

void game_init (Game* game, Args* args)
{
	if (args != NULL) {
		game->mode = args->game_mode;
		game->monoliths = args->monoliths;
		game->player.start = game->digger.ship.start = (float)args->start;
	}

	ship_init (&game->player, SHIP_RADIUS);
	digger_init (&game->digger, MAX_CAVE_RADIUS);
	cave_init (&game->cave, &game->digger, game->mode);
	score_init (&game->score, args);
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

void digger_control (Digger* digger, int game_mode)
{
	Ship *ship = SHIP(digger);

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

		if (game_mode == ONE_BUTTON)
			ship->lefton = ship->righton = (ship->lefton | ship->righton);
	}

	float scale = 1-MIN(1,log(1+ship->pos[2])/log(1+MIN_CAVE_RADIUS_DEPTH));
	ship->radius = MIN_CAVE_RADIUS+(MAX_CAVE_RADIUS-MIN_CAVE_RADIUS)*scale+RAND;

	float z = ship->pos[2] - ship->start;

	if (z < ROOM_LEN/2) { // outside
#if 1
		ship->radius *= (ROOM_MUL-1) + 1/(1e-9+2*z/ROOM_LEN);
#else
		//FIXME: why does the first room begins larger then others?
		ship->radius *= ROOM_MUL;
#endif
	}
	else { // rooms
		int r = ((int)z)%ROOM_SPACING;
		if (r < ROOM_LEN)
			ship->radius *= 1+(ROOM_MUL-1)*(cos(M_PI*(r-ROOM_LEN/2)/ROOM_LEN));
	}

	if (z < .33*SEGMENT_COUNT*SEGMENT_LEN)
		ship->lefton = ship->righton = false;

	digger->x_right_radius += RAND - 0.5;
	digger->y_top_radius += RAND - 0.5;
	digger->x_left_radius += RAND - 0.5;
	digger->y_bottom_radius += RAND - 0.5;
}

void autopilot (Game* game, float dt)
{
	Vec3 d;
	SUB2 (d, game->player.pos, game->cave.centers[(game->cave.i+2)%SEGMENT_COUNT]);

	bool* R = &game->player.righton;
	bool* L = &game->player.lefton;

	if (fabsf(d[1]) > fabsf(d[0])) {
		*R = *L = (d[1] < 0)  &&  (game->player.vel[1] < VELOCITY/8);
	}
	else {
		*R = d[0] < 0  &&  (game->player.vel[0] < VELOCITY/8);
		*L = !*R;
	}
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
	return x;
}

float collision (Cave* cave, Ship* ship)
{
	float min = FLT_MAX;
	Vec3 center = {0,0,0};

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

		ADD(center,cave->segs[i][i0]);

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

	SCALE(center,SECTOR_COUNT);
	SUB2(ship->repulsion, center, ship->pos);
	ship->repulsion[2] = 0;
	NORM(ship->repulsion);

	for(i = 0; i < 4; ++i) {
		if(intersection_count[i] % 2 == 0) {
			return ship->dist = 0;  // hit
		}
	}

	ship->dist = min - 2*ship->radius;
	return min;  // miss
}

bool game_nocheat (Game *game)
{
	return (game->player.start == 0);
}

int game_score (Game *game)
{
	return game->player.pos[2] / (game->mode==ONE_BUTTON?2:1);
}

void game_score_update (Game *game)
{
	score_update (&game->score, game_score(game), game_nocheat(game));
}

float ship_hit (Ship *ship)
{
	return 1-CLAMP(ship->dist / (2*SHIP_RADIUS),0,1);
}

// vim600:fdm=syntax:fdn=1:
