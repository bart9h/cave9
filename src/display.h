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

#define  ICON_FILE             "icon.png"
#define  WALL_TEXTURE_FILE     "rocky.jpg"
#define  OUTSIDE_TEXTURE_FILE  "grass.jpg"
#define  FONT_FILE             "trashco.ttf"

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
	GLuint outside_texture_id;
	GLuint wall_texture_id;
	GLuint hud_id;
	GLuint msg_id;
	TTF_Font* font;
	Vec3 cam, target;
	GLfloat near_plane, far_plane;

	GLuint gl_list[DISPLAYMODE_COUNT][SEGMENT_COUNT];
	GLuint list_start[DISPLAYMODE_COUNT];
	GLuint ship_list;

	bool cockpit;
	bool shaking;
} Display;

void viewport (Display*, GLsizei w, GLsizei h, GLsizei bpp, bool fullscreen, int aa);
void display_init (Display*, Args*);
void display_message (Display*, Game*, const char* buf);
void display_frame (Display*, Game*);

#endif

// vim600:fdm=syntax:fdn=1:
