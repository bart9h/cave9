#ifndef game_h_included
#define game_h_included

#include "vec.h"

#define FPS 30
#define SECTOR_COUNT 32
#define SEGMENT_COUNT 64
#define SEGMENT_LEN 2.0
#define SHIP_RADIUS 1.0

#define MONOLITH_DEPTH  (1*1)
#define MONOLITH_WIDTH  (2*2)
#define MONOLITH_HEIGHT (3*3)

typedef struct {
	float radius;
	Vec3 pos, vel;
	bool lefton, righton;
	float dist;
} Ship;

typedef struct {
	Vec3 segs[SEGMENT_COUNT][SECTOR_COUNT];
	GLuint gl_list[SEGMENT_COUNT];
	int i; // circular array index

	float monolith_x;
	float monolith_y;
	float monolith_yaw;

} Cave;

void cave_gen(Cave *cave, Ship *digger);
void cave_init(Cave *cave, Ship *digger);
void ship_init(Ship* ship, float radius);
void ship_move(Ship *ship, float dt);
void digger_control(Ship *ship);
float collision(Cave *, Ship *);

#endif

