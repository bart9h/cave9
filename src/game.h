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

#define SECTOR_COUNT 32
#define SEGMENT_COUNT 64

enum DisplayMode
{
	DISPLAYMODE_NORMAL,
	DISPLAYMODE_MINIMAP,
	DISPLAYMODE_COUNT
};

typedef struct  Ship_struct
{
	float radius;
	Vec3 pos, vel, lookAt;
	bool lefton, righton;
	float dist;  // distance to cave wall
	Vec3 repulsion;  // normal to collision
	float start;
} Ship;

typedef struct  Cave_struct
{
	Vec3 segs[SEGMENT_COUNT][SECTOR_COUNT];
	GLuint gl_list[DISPLAYMODE_COUNT][SEGMENT_COUNT];
	int i;  // circular array index

	float monolith_x;
	float monolith_y;
	float monolith_yaw;
} Cave;

typedef struct Game_struct
{
	Cave cave;
	Ship player;
	Ship digger;
	Score score;

	int mode;
	bool monoliths;
} Game;

enum GameMode
{
	ONE_BUTTON = 1,
	TWO_BUTTONS = 2
};

typedef struct Args_struct
{
	int width;
	int height;
	int bpp;
	int fullscreen;
	int highres;
	int antialiasing;
	int monoliths;
	int start;
	int cockpit;
	int game_mode;
} Args;

void game_init (Game* game, Args* args);
void cave_gen (Cave*, Ship* digger);
void cave_init (Cave*, Ship* digger, int game_mode);
void ship_init (Ship*, float radius);
void ship_move (Ship*, float dt);
void digger_control (Ship*, int game_mode);
float collision (Cave*, Ship*);
bool game_nocheat(Game *game);
int game_score (Game *game);
void game_score_update (Game *game);
float ship_hit (Ship*);

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
