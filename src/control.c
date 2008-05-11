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

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "display.h"
#include "game.h"
#include "audio.h"

const char* data_paths[] =
{
	"./data",
	"~/.cave9/data",
	"/usr/local/share/cave9",
	"/usr/share/cave9",
	NULL
};

typedef struct Input_struct
{
	bool pressed[SDLK_LAST];
	enum { WELCOME, PLAY, PAUSE, GAMEOVER, QUIT } state;
} Input;

static void control (Display* display, Audio* audio, Game* game, Input* input)
{
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
			case SDLK_ESCAPE:
			case SDLK_q:
				input->state = QUIT;
				break;
			case SDLK_f:
				if(input->state == PLAY)  {
					input->state = PAUSE;
					display_message(display, game, "paused");
				}
				SDL_WM_ToggleFullScreen(display->screen);
				break;
			case SDLK_p:
			case SDLK_PAUSE:
			case SDLK_SPACE:
				if(input->state == WELCOME 
				|| input->state == PAUSE
				|| input->state == GAMEOVER) {
					if(input->state == GAMEOVER)
						game_init (game, NULL);
					display_message (display, game, "");
					input->state = PLAY;
					audio_start (audio, &game->player);
				}
				else {
					if(input->state == PLAY)  {
						input->state = PAUSE;
						display_message (display, game, "paused");
					}
					audio_stop (audio);
				}
				break;
			case SDLK_RETURN:
				if(SDL_GetModState() & KMOD_ALT)
					SDL_WM_ToggleFullScreen(display->screen);
				break;
			default:
				break;
			}
		case SDL_KEYUP:
			input->pressed[event.key.keysym.sym] = (event.type == SDL_KEYDOWN);
			break;
		case SDL_QUIT:
			input->state = QUIT;
			break;
		case SDL_VIDEORESIZE:
			{
				int aa;
				SDL_GL_GetAttribute (SDL_GL_MULTISAMPLESAMPLES, &aa);
				viewport (display, event.resize.w, event.resize.h, 0, 
						display->screen->flags & SDL_FULLSCREEN, aa);
			}
			break;
		case SDL_VIDEOEXPOSE:
			display_frame (display, game);
			break;
		default:
			break;
		}
	}
}

static void player_control (Ship* player, Input* input, int game_mode)
{
#define K(k) (input->pressed[k])
	bool up    = K(SDLK_DOWN)  || K(SDLK_UP);
	bool left  = K(SDLK_LEFT)  || K(SDLK_LSHIFT) || K(SDLK_LCTRL);
	bool right = K(SDLK_RIGHT) || K(SDLK_RSHIFT) || K(SDLK_RCTRL);

	if (game_mode == ONE_BUTTON) {
		player->lefton = player->righton = left || up || right;
	}
	else {
		player->lefton  = left  || (up && !right);
		player->righton = right || (up && !left);
	}
}

static void args_init (Args* args, int argc, char* argv[])
{
	args->width = 640;
	args->height = 480;
	args->bpp = 0;
	args->fullscreen = 0;
	args->highres = 0;
	args->antialiasing = 0;
	args->monoliths = 0;
	args->start = 0;
	args->cockpit = 0;
	args->game_mode = TWO_BUTTONS;
	args->nosound = 0;
	args->noshake = 0;
	args->stalactites = 0;
	args->autopilot = 0;
	args->aidtrack = 0;
	args->arabic = 0;
#ifdef USE_SDLNET
	args->port = GLOBAL_SCORE_PORT;
# ifdef NET_DEFAULT_ENABLED
	snprintf (args->server, ARG_STR_MAX, "%s", GAME_SCORE_HOST);
# else
	args->server[0] = '\0';
# endif
#endif
	int help_called = 0;

	struct {
		char* short_name;
		char* long_name;
		bool  has_arg;
		bool  is_experimental;
		int*  val_num;
		char* val_str;
	} options[] = {
		//                        arg?    exp?    int-value            string-value
		{ "-h", "--help",         false,  false,  &help_called,        NULL         },
		{ "-g", "--game_mode",    true,   false,  &args->game_mode,    NULL         },
		{ "-W", "--width",        true,   false,  &args->width,        NULL         },
		{ "-H", "--height",       true,   false,  &args->height,       NULL         },
		{ "-B", "--bpp",          true,   false,  &args->bpp,          NULL         },
		{ "-F", "--fullscreen",   false,  false,  &args->fullscreen,   NULL         },
		{ "-R", "--highres",      false,  false,  &args->highres,      NULL         },
		{ "-A", "--antialiasing", true,   false,  &args->antialiasing, NULL         },
		{ "-S", "--start",        true,   false,  &args->start,        NULL         },
		{ "-C", "--cockpit",      false,  false,  &args->cockpit,      NULL         },
		{ "-N", "--nosound",      false,  false,  &args->nosound,      NULL         },
		{ "-K", "--noshake",      false,  false,  &args->noshake,      NULL         },
		{ "-M", "--monoliths",    false,  true,   &args->monoliths,    NULL         },
		{ "",   "--stalactites",  false,  true,   &args->stalactites,  NULL         },
		{ "-a", "--autopilot",    false,  true,   &args->autopilot,    NULL         },
		{ "-T", "--aidtrack",     false,  true,   &args->aidtrack,     NULL         },
		{ "",   "--arabic",       false,  false,  &args->arabic,       NULL         },
#ifdef USE_SDLNET                         
		{ "-s", "--server",       true,   true,   NULL,                args->server },
		{ "-p", "--port",         true,   true,   &args->port,         NULL         },
#endif
		{ 0, 0, 0, 0, 0, 0 }
	};

	for (int i = 1;  i < argc;  ++i) {
		for (int opt = 0; ; ++opt) {
			if (options[opt].long_name == NULL) {
				fprintf(stderr, "invalid argument %s\n", argv[i]);
				help_called = 1;
				break;
			}

			assert (options[opt].long_name != NULL);
			if ((options[opt].short_name != NULL && strcmp (argv[i], options[opt].short_name) == 0)
					|| (strcmp (argv[i], options[opt].long_name) == 0))
			{
				if (options[opt].has_arg) {
					if (++i == argc) {
						fprintf(stderr, "argument required for %s\n", argv[i-1]);
						exit(1);
					}

					if (options[opt].val_num != NULL)
						*(options[opt].val_num) = atoi(argv[i]);
					else if (options[opt].val_str != NULL)
						snprintf (options[opt].val_str, ARG_STR_MAX, "%s", argv[i]);
					else
						assert (0=="has_arg && !(val_num||val_str)");
				}
				else {
					assert (options[opt].val_num != NULL);
					*(options[opt].val_num) = 1;
				}
				break;
			}
		}
	}

	if (help_called) {
		printf ("command-line options:\n");
		for (int opt = 0;  options[opt].long_name != NULL;  ++opt) {
			if (options[opt].short_name[0])
				printf ("%2s  or", options[opt].short_name);
			else
				printf ("      ");
			printf ("  %s", options[opt].long_name);
			if (options[opt].has_arg) {
				if (options[opt].val_num != NULL)
					printf ("  <num>");
				else if (options[opt].val_str != NULL)
					printf ("  <str>");
			}
			if (options[opt].is_experimental)
				printf ("    (experimental)");
			printf ("\n");
		}
		exit(1);
	}
}

int main_control (int argc, char* argv[])
{
	Args args;
	Display display;
	Audio audio;
	Input input;
	memset (input.pressed, 0, sizeof(input.pressed));

	srand(time(NULL));

	Game game;

	args_init (&args, argc, argv);
	display_init (&display, &args);
	audio_init (&audio, !args.nosound);

	game_init (&game, &args);

	input.state = WELCOME;
	display_message (&display, &game, "welcome! use arrows/space");

	float dt = 1./FPS;
	while (input.state != QUIT) {
		int t0 = SDL_GetTicks();

		control (&display, &audio, &game, &input);

		switch (input.state) {
		case PLAY:
			digger_control (&game.digger, game.mode);
			ship_move (SHIP(&game.digger), dt);
			
			if (args.autopilot)
				autopilot (&game, dt);
			else
				player_control (&game.player, &input, game.mode);
			ship_move (&game.player, dt);
			
#ifndef NO_STRETCH_FIX
			game.player.pos[2] = 
				0.7 * (game.player.pos[2]) +
				0.3 * (game.digger.ship.pos[2] - cave_len(&game.cave));
			// XXX fix player position in case digger moves differently
#endif

			if (collision (&game.cave, &game.player) <= 0) {
				input.state = GAMEOVER;
				display_message (&display, &game, "gameover.  [press space]");
				audio_stop (&audio);
				game_score_update (&game);
			}
			cave_gen (&game.cave, &game.digger);

			display_frame (&display, &game);
			break;

		case WELCOME:
		case PAUSE:
		case GAMEOVER:
		case QUIT:
			break;
		}

		int t1 = SDL_GetTicks();
		SDL_Delay( MAX( 1, 1000/FPS-(t1-t0) ) );

		dt = (SDL_GetTicks()-t0)/1000.;
	}

	//display_message (&display, &game, "bye.");
	score_finish (&game.score);

	return 0;
}

// vim600:fdm=syntax:fdn=1:
