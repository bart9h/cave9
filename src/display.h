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
#include "game.h"

#include "config.h"

#define TEXTURE_FILE DATA_DIR "cave9.jpg"
#define FONT_FILE DATA_DIR "cave9.ttf"
#define AUDIO_FILE DATA_DIR "cave9.wav"

typedef struct Audio_struct
{
	int enabled;
	SDL_AudioSpec fmt;
	Ship *ship;
	signed short *data;
	unsigned size;
	unsigned index;
	float low_index;
	float left;
	float right;
} Audio;

typedef struct Display_struct
{
	SDL_Surface* screen;
	GLuint texture_id;
	GLuint hud_id;
	GLuint msg_id;
	TTF_Font* font;
	Vec3 cam, target;
	GLfloat near_plane, far_plane;

	GLuint list_start;
	GLuint wire_list_start;
	GLuint ship_list;

	int cockpit;

	UDPsocket udp_sock;
	UDPpacket* udp_pkt;

	Audio audio;
} Display;

void audio_mix(void *data, Uint8 *stream, int len);
void audio_start(Display *display, Ship *ship);
void audio_stop(Display *display);

void viewport (Display*, GLsizei w, GLsizei h, GLsizei bpp, bool fullscreen, int aa);
void cave_model (Display*, Cave*, bool wire);
void ship_model (Display*, Ship*);
void render_hud (Display*, Ship* player);
void display_init (Display*, Args*);
void display_start_frame (Display* display, float r, float g, float b);
void display_end_frame (Display*);
void display_minimap (Display*, Game*);
void display_hud (Display*, Game*);
void display_message (Display*, Game*, const char* buf);
void display_frame (Display*, Game*);

#endif

// vim600:fdm=syntax:fdn=1:
