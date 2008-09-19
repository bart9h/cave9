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

#ifdef GLOBAL_SCORE

static void score_net_finish (Score* score)
{
	if(score->udp_pkt != NULL) {
		SDLNet_FreePacket(score->udp_pkt);
		score->udp_pkt = NULL;
	}

	if(score->udp_sock != NULL) {
		SDLNet_UDP_Close(score->udp_sock);
		score->udp_sock = NULL;
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
	score->udp_sock = NULL;
	score->udp_pkt = NULL;
	if (SDLNet_ResolveHost (&addr, server, port) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost('%s',%d): %s\n", 
				server, port, SDLNet_GetError());
	} else {
		score->udp_sock = SDLNet_UDP_Open(0);
		if(score->udp_sock == NULL) {
			fprintf(stderr, "SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
			score_net_finish (score);
		} else {
			if(SDLNet_UDP_Bind(score->udp_sock, 0, &addr) == -1) {
				fprintf(stderr, "SDLNet_UDP_Bind(): %s\n", SDLNet_GetError());
				score_net_finish (score);
			} else {
				score->udp_pkt = SDLNet_AllocPacket (GLOBAL_SCORE_LEN);
				if(score->udp_pkt == NULL) {
					fprintf(stderr, "SDLNet_AllocPacket(%d): %s\n", 
							GLOBAL_SCORE_LEN, SDLNet_GetError());
					score_net_finish (score);
				} else {
					printf("Score Global init UDP port %d\n", port);
				}
			}
		}
	}
}

static void score_net_update (Score* score)
{
	if (score->udp_sock == NULL) {
		return;
	}

	printf("Score Global send %d\n", score->global);
	snprintf ((char*)score->udp_pkt->data,GLOBAL_SCORE_LEN, "%d\n", score->global);
	score->udp_pkt->len = GLOBAL_SCORE_LEN;
	if (SDLNet_UDP_Send (score->udp_sock, 0, score->udp_pkt) == 1)
	{
		SDL_Delay (GLOBAL_SCORE_WAIT); // XXX only wait GLOBAL_SCORE_WAIT for hiscores
		int n = SDLNet_UDP_Recv (score->udp_sock, score->udp_pkt);
		if (n == 1) {
			score->udp_pkt->data[GLOBAL_SCORE_LEN-1] = '\0'; // XXX safeguard
			sscanf ((char*)score->udp_pkt->data, "%d", &score->global);
			printf("Score Global recv %d\n", score->global);
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

void score_init (Score* score, Args* args, int caveseed, int monstal)
{
	if (args == NULL)
		return;

	memset (score, 0, sizeof(Score));

	char cave9_home[FILENAME_MAX];
	char* home = getenv("HOME");
	if (home != NULL) {
		sprintf (cave9_home, "%s/.cave9", home);
		mkdir (
			cave9_home
#ifndef _WIN32 // XXX no mode on win32
			, 0755
#endif
		);
	}
	else {
		fprintf (stderr,
			"HOME environment variable not set, using '%s' to save\n",
			bin_path);

		strncpy(cave9_home, bin_path, FILENAME_MAX-1);
		cave9_home[FILENAME_MAX-1] = '\0';
	}
	size_t len = strlen(cave9_home) + strlen("/") + strlen(SCORE_FILE) + 1;
	score->filename = malloc (len);
	snprintf (score->filename, len, "%s/%s", cave9_home, SCORE_FILE);
	assert (score->filename != NULL);

	const char* paths[] = { "", ".", "~/.cave9", NULL };
	const char* filename = find_file (SCORE_FILE, paths, false);
	
	score->local = 0;
	score->caveseed = caveseed;
	score->monstal = monstal;

	if(filename != NULL) {
		FILE* fp = fopen (filename, "r");
		if (fp != NULL) {
			int fseed = -1;
			int fmonstal = 0;
			int fscore = 0;
			while(fseed != score->caveseed && fmonstal != score->monstal && !feof(fp))
			{
				fscanf (fp, "%11d%2d%11d ", &fseed, &fmonstal, &fscore);
			}
			if(fseed == caveseed && fmonstal == monstal)
				score->local = fscore;
			fclose (fp);
		}
	}

#ifdef GLOBAL_SCORE
	score_net_init (score, args->server, args->port);
#endif
}

void score_finish (Score* score)
{
#ifdef GLOBAL_SCORE
	score_net_finish (score);
#endif
	free (score->filename);
	score->filename = NULL;
}

void score_update (Score* score, int new_score, bool is_global)
{
	if (new_score > score->session)
		score->session = new_score;

	if (is_global) {

		if (new_score > score->local) {
			score->local = new_score;
			FILE* fp = fopen (score->filename, "r+");
			bool readwrite = true;
			if (fp == NULL)
			{
				// file didn't exist yet, we need to create an empty one
				fp = fopen (score->filename, "w");
				readwrite = false;
			}
			if (fp == NULL) {
				perror ("failed to open score file");
			} else {
				int fseed = -1;
				int fmonstal = 0;
				int fscore = 0;
				bool found = false;
				if (readwrite)
				{
					while (!feof(fp) && !found)
					{
						fscanf (fp, "%11d%2d%11d ", &fseed, &fmonstal, &fscore);
						if (fseed == score->caveseed && fmonstal == score->monstal)
						{
							if (fscore <= score->local)
							{
								fseek(fp, -12, SEEK_CUR);
								fprintf(fp, "%11d", score->local);
							}
							found = true;
						}
					}
				}
				if (!found || !readwrite)
				{
					fprintf(fp, "%11d%2d%11d\n", score->caveseed, score->monstal, score->local);
				}
				fclose (fp);
			}
		}

		int new_global = MAX(new_score, score->local);
		if (new_global > score->global) {
			score->global = new_global;
#ifdef GLOBAL_SCORE
			score_net_update (score);
#endif
		}
	}
}

// vim600:fdm=syntax:fdn=1:
