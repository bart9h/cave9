#ifndef game_h_included
#define game_h_included

#include "vec.h"

#define FPS 30
#define SECTOR_COUNT 30
#define SEGMENT_COUNT 60
#define SEGMENT_LEN 3.0
#define SHIP_RADIUS 1.0

typedef struct {
	// triangle strip
	float radius;
	Vec3 pos, vel;
	bool lefton, righton;
	float dist;
} Ship;

typedef struct {
	Vec3 segs[SEGMENT_COUNT][SECTOR_COUNT];
	GLuint gl_list[SEGMENT_COUNT];
	int i; // indice do array circular
	float ymin, ymax; // aux, for minimap
} Cave;

void cave_gen(Cave *cave, Ship *digger);
void cave_init(Cave *cave, Ship *digger);
void ship_init(Ship* ship, float radius);
void ship_move(Ship *ship, float dt);
void digger_control(Ship *ship);
float collision(Cave *, Ship *);

#endif

