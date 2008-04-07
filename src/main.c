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
#include "vec.h"
#include "display.h"
#include "game.h"

typedef struct Input_struct
{
	bool pressed[SDLK_LAST];
	enum {WELCOME, PLAY, PAUSE, GAMEOVER, QUIT} state;
} Input;

void game_init (Display* display, Game* game, Args* args)
{
	if (args != NULL) {
		game->mode = args->game_mode;
		game->player.start = game->digger.start = (float)args->start;
	}
	ship_init (&game->player, SHIP_RADIUS);
	ship_init (&game->digger, MAX_CAVE_RADIUS);
	cave_init (&game->cave, &game->digger, game->mode);
	display_message (display, game, "");
}

void control (Display* display, Game* game, Input* input)
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
						game_init (display, game, NULL);
					else
						display_message (display, game, "");
					input->state = PLAY;
				}
				else if(input->state == PLAY)  {
					input->state = PAUSE;
					display_message (display, game, "paused");
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

void player_control (Ship* player, Input* input, int game_mode)
{
	if (game_mode == ONE_BUTTON) {

		player->lefton  =
		player->righton =
			input->pressed[SDLK_DOWN]    ||
			input->pressed[SDLK_UP];
	}
	else {

		player->lefton  =
			input->pressed[SDLK_LEFT]    ||
			input->pressed[SDLK_LSHIFT]  ||
			input->pressed[SDLK_LCTRL];

		player->righton =
			input->pressed[SDLK_RIGHT]   ||
			input->pressed[SDLK_RSHIFT]  ||
			input->pressed[SDLK_RCTRL];
	}
}

void args_init (Args* args, int argc, char* argv[])
{
	args->width = 640;
	args->height = 480;
	args->bpp = 0;
	args->fullscreen = 0;
	args->highres = 0;
	args->antialiasing = 2;
	args->monoliths = 0;
	args->start = 0;
	args->cockpit = 0;
	args->game_mode = TWO_BUTTONS;
	int help_called = 0;

	struct {
		bool has_arg;
		int* val;
		char* short_name;
		char* long_name;
	} options[] = {
		{ 0, &help_called, "-h", "--help" },
		{ 1, &args->game_mode, "-g", "--game_mode" },
		{ 1, &args->width, "-W", "--width" },
		{ 1, &args->height, "-H", "--height" },
		{ 1, &args->bpp, "-B", "--bpp" },
		{ 0, &args->fullscreen, "-F", "--fullscreen" },
		{ 0, &args->highres, "-R", "--highres" },
		{ 1, &args->antialiasing, "-A", "--antialiasing" },
		{ 0, &args->monoliths, "-M", "--monoliths" },
		{ 1, &args->start, "-S", "--start" },
		{ 0, &args->cockpit, "-C", "--cockpit" },
		{ 0, NULL, NULL, NULL }
	};

	for(int i = 1; i < argc; ++i) {
		for(int opt = 0; ; ++opt) {
			if(options[opt].val == NULL) {
				fprintf(stderr, "invalid argument %s\n", argv[i]);
				help_called = 1;
				break;
			}
			if(!strcmp(argv[i], options[opt].short_name) || !strcmp(argv[i], options[opt].long_name)) {
				int value = 1;
				if(options[opt].has_arg) {
					if(++i == argc) {
						fprintf(stderr, "argument required for %s\n", argv[i-1]);
						exit(1);
					}
					value = atoi(argv[i]);
				}
				*(options[opt].val) = value;
				break;
			}
		}
	}

	if(help_called) {
		printf("command-line options:\n");
		for(int opt = 0; options[opt].val; ++opt) {
			printf("%s  or  %s", options[opt].short_name, options[opt].long_name);
			if(options[opt].has_arg)
				printf("  <num>");
			printf("\n");
		}
		exit(1);
	}
}

int main (int argc, char* argv[])
{
	Args args;
	Display display;
	Input input;
	memset (input.pressed, 0, sizeof(input.pressed));

	srand(time(NULL));

	Game game;

	args_init (&args, argc, argv);
	display_init (&display, &args);

	game_init (&display, &game, &args);
	input.state = WELCOME;
	display_message (&display, &game, "welcome!  left+right for control.  [press space]");

	float dt = 0;
	while (input.state != QUIT) {
		int t0 = SDL_GetTicks();

		control (&display, &game, &input);

		switch (input.state) {
		case PLAY:
			player_control (&game.player, &input, args.game_mode);
			ship_move (&game.player, dt);
			digger_control (&game.digger, args.game_mode);
			ship_move (&game.digger, dt);
			if (collision (&game.cave, &game.player) <= 0) {
				display_message (&display, &game, "gameover.  [press space]");
				input.state = GAMEOVER;
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
		//dt = 1./FPS;
	}

	display_net_finish (&display);
	display_message (&display, &game, "bye.");

	return 0;
}

// vim600:fdm=syntax:fdn=1:
