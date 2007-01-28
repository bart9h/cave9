#ifndef display_h_included
#define display_h_included

#include <SDL.h>
#include <SDL_ttf.h>
#include "game.h"

typedef struct {
	SDL_Surface *screen;
	TTF_Font *font;
	SDL_Rect rect[16];
	int rect_n;
	Vec3 cam, target;
	GLfloat near, far;
} Display;

void viewport(Display *display, GLsizei w, GLsizei h, GLsizei bpp);
void cave_model(Cave *cave);
void ship_model(Ship *ship);
void render_hud(Display*, Ship *player);
void display_init(Display* display);
void display_start_frame(Display *display, Ship *player);

#endif

