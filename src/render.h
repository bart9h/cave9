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

#include "display.h"

typedef struct Render_struct
{
	Display display;

	GLuint wall_texture_id;
#ifdef OUTSIDE_TEXTURE_FILE
	GLuint outside_texture_id;
#endif
	GLuint hud_id;
	GLuint msg_id;
	TTF_Font *font;
#ifdef FONT_MENU_FILE
	TTF_Font *font_menu;
#endif
	Vec3 cam, target;

	GLuint gl_list[DISPLAYMODE_COUNT][SEGMENT_COUNT];
	GLuint list_start[DISPLAYMODE_COUNT];
	GLuint ship_list;

	bool cockpit;
	bool shaking;
	bool aidtrack;
	bool roman;

	float gauge;
} Render;

void render_init (Render*, Args*);
void render_frame (Render*, Game*);
void render_message (Render*, Game*, const char* buf);

// vim600:fdm=syntax:fdn=1:
