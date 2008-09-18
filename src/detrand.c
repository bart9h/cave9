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
#include "detrand.h"

int randval;
int randseed;

float detrand()
{
	// taken from http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
	randval = (randval<<13) ^ randval;
	return ( 1.0 - ( (randval++ * (randval * randval * randseed * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

void detsrand(int seed)
{
	randseed = seed;
}

