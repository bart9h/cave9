#ifndef display_h_included
#define display_h_included

#include <SDL.h>
#include <SDL_ttf.h>
#include "game.h"

#define TEXTURE //cave texture

typedef struct {
	SDL_Surface *screen;
#ifdef TEXTURE
	GLuint texture_id;
#endif
	GLuint hud_id;
	GLuint msg_id;
	TTF_Font *font;
	Vec3 cam, target;
	GLfloat near_plane, far_plane;
	GLuint list_start;
} Display;

typedef struct {
	int width;
	int height;
	int bpp;
	int fullscreen;
	int highres;
} Args;

void viewport(Display *display, GLsizei w, GLsizei h, GLsizei bpp, bool fullscreen);
void cave_model(Display *display, Cave *cave);
void ship_model(Ship *ship);
void render_hud(Display*, Ship *player);
void display_init(Display* display, Args* args);
void display_start_frame(Display *display, Ship *player);
void display_end_frame(Display *display);
void display_minimap(Display *display, Cave *cave, Ship *player);
void display_hud(Display *display, Ship *player);
void display_message(Display *display, Cave *cave, Ship *player, const char *buf);
void display_frame(Display *display, Cave *cave, Ship *player);

#endif

