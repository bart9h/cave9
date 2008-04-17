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
#include "display.h"
#include "game.h"
#include "audio.h"

typedef struct Input_struct
{
	bool pressed[SDLK_LAST];
	enum { WELCOME, PLAY, PAUSE, GAMEOVER, QUIT } state;
} Input;

void control (Display* display, Audio* audio, Game* game, Input* input)
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

void player_control (Ship* player, Input* input, int game_mode)
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

void args_init (Args* args, int argc, char* argv[])
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
	audio_init (&audio);
	game_init (&game, &args);

	input.state = WELCOME;
	display_message (&display, &game, "welcome!  left+right for control.  [press space]");

	float dt = 0;
	while (input.state != QUIT) {
		int t0 = SDL_GetTicks();

		control (&display, &audio, &game, &input);

		switch (input.state) {
		case PLAY:
			digger_control (&game.digger, game.mode);
			ship_move (&game.digger, dt);
			
			player_control (&game.player, &input, game.mode);
			ship_move (&game.player, dt);
			
#ifndef NO_STRETCH_FIX
			game.player.pos[2] = 
				0.7 * (game.player.pos[2]) +
				0.3 * (game.digger.pos[2] - cave_len(&game.cave)); 
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
		//dt = 1./FPS;
	}

	display_message (&display, &game, "bye.");
	score_finish (&game.score);

	return 0;
}

// vim600:fdm=syntax:fdn=1:
