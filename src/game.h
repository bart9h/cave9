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

