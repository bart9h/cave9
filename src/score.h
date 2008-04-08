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

#include <SDL_net.h>

#define SCORE_FILE "data/cave9.hi"
#define GLOBAL_SCORE_PORT 31559
#define GLOBAL_SCORE_HOST "cave9.9hells.org"
#define GLOBAL_SCORE_LEN 16
#define GLOBAL_SCORE_WAIT 666

typedef struct Score_struct
{
	int session_score;
	int local_score;
	int global_score;

	UDPsocket udp_sock;
	UDPpacket* udp_pkt;
} Score;

void score_init (Score* score);
void score_finish (Score* score);
void score_net_update (Score* score);

#endif

// vim600:fdm=syntax:fdn=1:
