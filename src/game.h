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

#include "score.h"
#include "vec.h"
#include "args.h"

#define SECTOR_COUNT 32
#define SEGMENT_COUNT 64

#define ROOM_SPACING 1000
#define ROOM_START (2*ROOM_SPACING)
#define ROOM_LEN 100.0
#define ROOM_MUL 3.0

typedef struct  Ship_struct
{
	float radius;
	Vec3 pos, vel, lookAt;
	bool lefton, righton;
	float dist;  // distance to cave wall
	Vec3 repulsion;  // normal to collision
	float start;
} Ship;

typedef struct Digger_struct
{
	Ship ship; // parent class

	float x_right_radius;
	float x_left_radius;
	float y_top_radius;
	float y_bottom_radius;
} Digger;

#define SHIP(digger) (&((digger)->ship))

typedef struct  Cave_struct
{
	Vec3 segs[SEGMENT_COUNT][SECTOR_COUNT];
	bool dirty[SEGMENT_COUNT];
	Vec3 centers[SEGMENT_COUNT];
	int i;  // circular array index

	Vec3  monolith_pos;
	float monolith_yaw;
} Cave;

typedef struct Game_struct
{
	Cave cave;
	Ship player;
	Digger digger;
	Score score;

	int mode;
	bool monoliths;
} Game;

enum GameMode
{
	ONE_BUTTON = 1,
	TWO_BUTTONS = 2
};

void game_init (Game* game, Args* args);
float cave_len (Cave*);
void cave_gen (Cave*, Digger* digger);
void ship_move (Ship*, float dt);
void digger_control (Digger*, int game_mode);
void autopilot (Game*, float dt);
float collision (Cave*, Ship*);
bool game_nocheat(Game *game);
int game_score (Game *game);
void game_score_update (Game *game);
float ship_hit (Ship*);

extern const char* data_paths[];
#define FIND(f) find_file(f,data_paths,true)

#define FPS 30
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

#endif

// vim600:fdm=syntax:fdn=1:
