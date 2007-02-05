#ifndef display_h_included
#define display_h_included

#include <SDL.h>
#include <SDL_ttf.h>
#include "game.h"

typedef struct {
	SDL_Surface *screen;
	SDL_Surface *minimap;
	TTF_Font *font;
	SDL_Rect rect[16];
	int rect_n;
	Vec3 cam, target;
	GLfloat near_plane, far_plane;
} Display;

void viewport(Display *display, GLsizei w, GLsizei h, GLsizei bpp);
void cave_model(Cave *cave);
void ship_model(Ship *ship);
void render_hud(Display*, Ship *player);
void display_init(Display* display);
void display_start_frame(Display *display, Ship *player);
void display_end_frame(Display *display);
void display_minimap(Display *display, Cave *cave, Ship *player);
void display_hud(Display *display, Ship *player);
void display_message(Display *display, Cave *cave, Ship *player, const char *buf);
void display_frame(Display *display, Cave *cave, Ship *player);

#endif

