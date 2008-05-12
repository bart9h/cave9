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

#ifdef _WIN32
# undef __STRICT_ANSI__ // where did it come from? we're running C99 // for string.h: strdup
#endif
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include "score.h"
#include "util.h"

#ifdef _WIN32
#include <io.h> // mkdir
#endif

#ifdef USE_SDLNET

static void score_net_finish (Score* score)
{
	if(score->udp_pkt != NULL) {
		SDLNet_FreePacket(score->udp_pkt);
		score->udp_pkt = NULL;
	}

	if(score->udp_sock != 0) {
		SDLNet_UDP_Close(score->udp_sock);
		score->udp_sock = 0;
	}
}

static void score_net_init (Score* score, const char* server, int port)
{
	if (server == NULL  ||  server[0] == '\0')
		return;

	if (SDLNet_Init() == -1) {
		fprintf (stderr, "SDLNet_Init(): %s\n", SDLNet_GetError());
		exit(1);
	}
	atexit (SDLNet_Quit);

	IPaddress addr;
	score->udp_sock = 0;
	score->udp_pkt = NULL;
	if (SDLNet_ResolveHost (&addr, server, port) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost(): %s\n", SDLNet_GetError());
	} else {
		score->udp_sock = SDLNet_UDP_Open(0);
		if(score->udp_sock == 0) {
			fprintf(stderr, "SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
			score_net_finish (score);
		} else {
			if(SDLNet_UDP_Bind(score->udp_sock, 0, &addr) == -1) {
				fprintf(stderr, "SDLNet_UDP_Bind(): %s\n", SDLNet_GetError());
				score_net_finish (score);
			} else {
				score->udp_pkt = SDLNet_AllocPacket (GLOBAL_SCORE_LEN);
				if(score->udp_pkt != NULL) {
					memset (score->udp_pkt->data, 0, GLOBAL_SCORE_LEN);
				}
				else {
					score_net_finish (score);
				}
			}
		}
	}
}

static void score_net_update (Score* score)
{
	if (score->udp_sock == 0)
		return;

	snprintf ((char*)score->udp_pkt->data,GLOBAL_SCORE_LEN, "%d", score->global);
	score->udp_pkt->len = GLOBAL_SCORE_LEN;
	if (SDLNet_UDP_Send (score->udp_sock, 0, score->udp_pkt) == 1)
	{
		SDL_Delay (GLOBAL_SCORE_WAIT); // XXX only wait GLOBAL_SCORE_WAIT for hiscores
		int n = SDLNet_UDP_Recv (score->udp_sock, score->udp_pkt);
		if (n == 1) {
			sscanf ((char*)score->udp_pkt->data, "%d", &score->global);
		}
		else if (n < 0) {
			fprintf (stderr, "SDLNet_UDP_Recv(%s,%d): %s\n",
					GLOBAL_SCORE_HOST, GLOBAL_SCORE_PORT, SDLNet_GetError());
		}
	}
	else {
		fprintf (stderr, "SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
	}
}

#endif

void score_init (Score* score, Args* args)
{
	memset (score, 0, sizeof(Score));

	char cave9_home[FILENAME_MAX] = ".";
	char* home = getenv("HOME");
	if (home != NULL) {
		sprintf (cave9_home, "%s/.cave9", home);
		mkdir (
			cave9_home
#ifndef _WIN32 // XXX no mode on win32
			, 0755
#endif
		);

		size_t len = strlen(cave9_home) + strlen("/") + strlen(SCORE_FILE) + 1;
		score->filename = malloc (len);
		snprintf (score->filename, len, "%s/%s", cave9_home, SCORE_FILE);
	}
	else {
		fprintf (stderr,
			"HOME environment variable not set, using current dir to save %s\n",
			SCORE_FILE);
		score->filename = strdup (SCORE_FILE);
	}
	assert (score->filename != NULL);

	const char* paths[] = { cave9_home, ".", NULL };
	const char* filename = find_file (SCORE_FILE, paths, false);
	FILE* fp = fopen (filename, "r");
	if (fp != NULL) {
		if (fscanf (fp, "%d", &score->local) != 1)
			score->local = 0;
		fclose (fp);
	}

#ifdef USE_SDLNET
	if (args != NULL)
		score_net_init (score, args->server, args->port);
#endif
}

void score_finish (Score* score)
{
#ifdef USE_SDLNET
	score_net_finish (score);
#endif
	free (score->filename);
	memset (score, 0, sizeof(Score));
}

void score_update (Score* score, int new_score, bool is_global)
{
	if (new_score > score->session)
		score->session = new_score;

	if (is_global) {

		if (new_score > score->local) {
			score->local = new_score;
			FILE* fp = fopen (score->filename, "w");
			if (fp == NULL) {
				perror ("failed to open score file");
			} else {
				fprintf (fp, "%d\n", score->local);
				fclose (fp);
			}
		}

		int new_global = MAX(new_score, score->local);
		if (new_global > score->global) {
			score->global = new_global;
#ifdef USE_SDLNET
			score_net_update (score);
#endif
		}
	}
}

// vim600:fdm=syntax:fdn=1:
