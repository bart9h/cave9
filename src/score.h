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

#ifndef score_h_included
#define score_h_included

#include <stdbool.h>
#include "args.h"

#ifdef GLOBAL_SCORE
#include <SDL_net.h>
#endif

#include "score-common.h"

typedef struct Score_struct
{
	int current;
	int session;
	int local;
	int global;
	char* filename;

	int caveseed; // the seed used to generate the cave - 0 for random
	int monstal;  // monoliths? * 2 + stalactites

#ifdef GLOBAL_SCORE
	UDPsocket udp_sock;
	UDPpacket* udp_pkt;
#endif
} Score;

void score_init (Score*, Args*, int caveseed, int monstal);
void score_finish (Score*);
void score_update (Score*, int new_score, bool is_global);

#endif

// vim600:fdm=syntax:fdn=1:
