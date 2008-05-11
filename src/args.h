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

#ifndef args_h_included
#define args_h_included

#define ARG_STR_MAX 256

typedef struct Args_struct
{
	int width;
	int height;
	int bpp;
	int fullscreen;
	int highres;
	int antialiasing;
	int monoliths;
	int start;
	int cockpit;
	int game_mode;
	int nosound;
	int noshake;
	int stalactites;
	int autopilot;
	int aidtrack;
	int arabic;
#ifdef USE_SDLNET
	int port;
	char server[ARG_STR_MAX];
#endif
} Args;

#endif

// vim600:fdm=syntax:fdn=1:
