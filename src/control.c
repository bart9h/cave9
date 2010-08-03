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
#include "render.h"
#include "game.h"
#include "audio.h"
#include "util.h"
#include "version.h"

typedef struct Input_struct
{
	bool pressed[SDLK_LAST];
	enum { WELCOME, PLAY, PAUSE, GAMEOVER, QUIT } state;
} Input;

static void pause (Render* render, Audio* audio, Game* game, Input* input)
{
	if (input->state == PLAY) {
		input->state = PAUSE;
		render_message (render, game, "PAUSED");
		audio_stop (audio);
	}
}

static void control (Render* render, Audio* audio, Game* game, Input* input, Args* args)
{
	SDL_Event event;

	while (SDL_PollEvent (&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
			case SDLK_q:
				input->state = QUIT;
				break;
			case SDLK_f:
				pause (render, audio, game, input);
				SDL_WM_ToggleFullScreen (render->display.screen);
				break;
			case SDLK_p:
			case SDLK_PAUSE:
			case SDLK_SPACE:
				if(input->state == WELCOME 
				|| input->state == PAUSE
				|| input->state == GAMEOVER) {
					if(input->state == GAMEOVER)
						game_init (game, args);
					render_message (render, game, "");
					input->state = PLAY;
					audio_start (audio, &game->player);
				}
				else {
					pause (render, audio, game, input);
				}
				break;
			case SDLK_RETURN:
				if (SDL_GetModState() & KMOD_ALT)
					SDL_WM_ToggleFullScreen (render->display.screen);
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
				viewport (&render->display, event.resize.w, event.resize.h, 0, 
						render->display.screen->flags & SDL_FULLSCREEN, aa, render->lighting);
			}
			break;
		case SDL_VIDEOEXPOSE:
			render_frame (render, game);
			break;
		default:
			break;
		}
	}
}

static void player_control (Ship* player, Input* input, int game_mode)
{
#define K(k) (input->pressed[SDLK_##k])
	bool up    = K(DOWN)  || K(UP) || K(w) || K(s);;
	bool left  = K(LEFT)  || K(LSHIFT) || K(LCTRL) || K(a);
	bool right = K(RIGHT) || K(RSHIFT) || K(RCTRL) || K(d);

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
	args->roman = 0;
	args->lighting = 0;
	args->caveseed = 0;
#ifdef GLOBAL_SCORE
	args->port = GLOBAL_SCORE_PORT;
# ifndef NET_DEFAULT_DISABLED
	snprintf (args->server, ARG_STR_MAX, "%s", GLOBAL_SCORE_HOST);
# else
	args->server[0] = '\0';
# endif
#endif
	int help_called = 0;
	int version_called = 0;

	struct Option {
		char* short_name;
		char* long_name;
		bool  has_arg;
		bool  is_experimental;
		int*  val_num;
		char* val_str;
	} options[] = {
		//                        arg?    exp?    int-value            string-value
		{ "-h", "--help",         false,  false,  &help_called,        NULL         },
		{ "-v", "--version",      false,  false,  &version_called,     NULL         },
		{ "-g", "--game_mode",    true,   false,  &args->game_mode,    NULL         },
		{ "-W", "--width",        true,   false,  &args->width,        NULL         },
		{ "-H", "--height",       true,   false,  &args->height,       NULL         },
		{ "-B", "--bpp",          true,   false,  &args->bpp,          NULL         },
		{ "-F", "--fullscreen",   false,  false,  &args->fullscreen,   NULL         },
		{ "-R", "--highres",      false,  false,  &args->highres,      NULL         },
		{ "-A", "--antialiasing", true,   false,  &args->antialiasing, NULL         },
		{ "-S", "--start",        true,   false,  &args->start,        NULL         },
		{ "-N", "--nosound",      false,  false,  &args->nosound,      NULL         },
		{ "-K", "--noshake",      false,  false,  &args->noshake,      NULL         },
		{ "",   "--cockpit",      false,  true,   &args->cockpit,      NULL         },
		{ "",   "--monoliths",    false,  true,   &args->monoliths,    NULL         },
		{ "",   "--stalactites",  false,  true,   &args->stalactites,  NULL         },
		{ "",   "--autopilot",    false,  true,   &args->autopilot,    NULL         },
		{ "",   "--aidtrack",     false,  true,   &args->aidtrack,     NULL         },
		{ "",   "--roman",        false,  false,  &args->roman,        NULL         },
		{ "",   "--lighting",     false,  false,  &args->lighting,     NULL         },
		{ "-L", "--level",        true,   false,  &args->caveseed,     NULL         },
#ifdef GLOBAL_SCORE                         
		{ "-s", "--server",       true,   true,   NULL,                args->server },
		{ "-p", "--port",         true,   true,   &args->port,         NULL         },
#endif
		{ 0, 0, 0, 0, 0, 0 }
	};

	for (int i = 1;  i < argc;  ++i) {
		for (struct Option* opt = options;  ; ++opt) {
			if (opt->long_name == NULL) {
				fprintf(stderr, "invalid argument %s\n", argv[i]);
				help_called = 1;
				break;
			}

			if ((opt->short_name != NULL && strcmp (argv[i], opt->short_name) == 0)
					|| (strcmp (argv[i], opt->long_name) == 0))
			{
				if (opt->has_arg) {
					if (++i == argc) {
						fprintf(stderr, "argument required for %s\n", argv[i-1]);
						exit(1);
					}

					if (opt->val_num != NULL)
						*(opt->val_num) = atoi(argv[i]);
					else if (opt->val_str != NULL)
						snprintf (opt->val_str, ARG_STR_MAX, "%s", argv[i]);
					else
						assert (0=="has_arg && !(val_num||val_str)");
				}
				else {
					assert (opt->val_num != NULL);
					*(opt->val_num) = 1;
				}
				break;
			}
		}
	}

	if (help_called) {

		unsigned max_len = 0;
		for (struct Option* opt = options;  opt->long_name != NULL;  ++opt) {
			unsigned len = strlen (opt->long_name);
			if (max_len < len)
				max_len = len;
		}

		for (struct Option* opt = options;  opt->long_name != NULL;  ++opt) {
			if (opt->short_name[0])
				printf ("%2s  or", opt->short_name);
			else
				printf ("      ");
			printf ("  %-*s", max_len, opt->long_name);
			if (opt->has_arg) {
				if (opt->val_num != NULL)
					printf ("  <num>");
				else if (opt->val_str != NULL)
					printf ("  <str>");
			}
			if (opt->is_experimental) {
				if (!opt->has_arg)
					printf ("       ");
				printf ("  (experimental)");
			}
			printf ("\n");
		}
		exit (1);
	}

	if (version_called) {
		printf ("cave9 version %s  (data pack version %s)\n", CODE_VERSION, DATA_VERSION);
		exit (0);
	}
}

int main_control (int argc, char* argv[])
{
	Args args;
	Render render;
	Audio audio;
	Input input;
	memset (input.pressed, 0, sizeof(input.pressed));

	find_init(argv[0]);

	srand (time(NULL));

	Game game;

	args_init (&args, argc, argv);
	render_init (&render, &args);
	audio_init (&audio, !args.nosound);

	game_init (&game, &args);

	input.state = WELCOME;
	render_message (&render, &game, "WELCOME! use arrows and space");

	float dt = 1./FPS;
	while (input.state != QUIT) {
		int t0 = SDL_GetTicks();

		control (&render, &audio, &game, &input, &args);

		switch (input.state) {
		case PLAY:
			if(game.digger.ship.pos[2] <= game.player.pos[2] + cave_len(&game.cave))
			{
				digger_control (&game.digger, game.mode);
				cave_gen (&game.cave, &game.digger);
				ship_move (SHIP(&game.digger), 0.05);
			}

			if (args.autopilot)
				autopilot (&game, dt);
			else
				player_control (&game.player, &input, game.mode);
			ship_move (&game.player, dt);

			if (collision (&game.cave, &game.player) <= 0) {
				input.state = GAMEOVER;
				game.player.dist = -1;
				game.player.lefton =
				game.player.righton = 0;
				render_message (&render, &game, "GAMEOVER. press space");
				game_score_update (&game);
				SDL_Delay(500); audio_stop (&audio); // time to listen hit sound
			}

			render_frame (&render, &game);
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

	audio_stop(&audio);
	//render_message (&render, &game, "bye.");
	score_finish (&game.score);

	return 0;
}

// vim600:fdm=syntax:fdn=1:
