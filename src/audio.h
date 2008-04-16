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

#ifndef audio_h_included
#define audio_h_included

#include "game.h"
#include "vec.h"

typedef struct Audio_struct
{
	bool enabled;
	SDL_AudioSpec fmt;
	Ship* ship;
	signed short* data;
	unsigned size;
	unsigned index;
	float low_index;
	float left;
	float right;
} Audio;

void audio_init (Audio*);
void audio_start (Audio*, Ship*);
void audio_stop (Audio*);

#define AUDIO_FILE "data/cave9.wav"

#endif

// vim600:fdm=syntax:fdn=1:
