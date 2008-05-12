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

#ifndef display_h_included
#define display_h_included

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "game.h"

#define BASE_W 1024
#define BASE_H 768

#define  ICON_FILE             "icon.png"
#define  WALL_TEXTURE_FILE     "wall.jpg"
//#define  OUTSIDE_TEXTURE_FILE  "outside.jpg"
#define  FONT_FILE             "hud.ttf"
//#define  FONT_MENU_FILE        "menu.ttf"

enum DisplayMode
{
	DISPLAYMODE_NORMAL,
	DISPLAYMODE_MINIMAP,
	DISPLAYMODE_COUNT
};

typedef struct Display_struct
{
	SDL_Surface* icon;
	SDL_Surface* screen;
	GLfloat near_plane, far_plane;
} Display;

void display_init (Display*, GLfloat near_plane, GLfloat far_plane, Args*);
void viewport (Display*, GLsizei w, GLsizei h, GLsizei bpp, bool fullscreen, int aa);

void display_text_box (Display* display, GLuint *id, 
		TTF_Font *font, const char* text,
		float x, float y, float w, float h,
		float r, float g, float b, float a);

void display_text (Display* display, GLuint *id, 
		TTF_Font *font, const char* text,
		float x, float y, float scale,
		float r, float g, float b, float a);

void display_start_frame (float r, float g, float b);
void display_end_frame();

GLuint load_texture (const char* filename);

TTF_Font *load_font (const char* filename, int size);

#endif

// vim600:fdm=syntax:fdn=1:
