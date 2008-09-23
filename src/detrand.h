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

#ifndef detrand_h_included
#define detrand_h_included

#include <stdlib.h>

extern int randval;
extern int randseed;

float detrand();
void detsrand (int seed);

#define DRAND (detrand() / 2.0 + 0.5)
#define DRAND_BIG (int)(DRAND * RAND_MAX)

#endif
