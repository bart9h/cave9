
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "vec.h"
#include "display.h"
#include "game.h"

typedef struct {
	bool pressed[SDLK_LAST];
	enum {WELCOME, PLAY, PAUSE, GAMEOVER, QUIT} state;
} Input;

void game_init(Display *display, Cave *cave, Ship *digger, Ship *player)
{
	ship_init(player, SHIP_RADIUS);
	ship_init(digger, SHIP_RADIUS*20);
	cave_init(cave,digger);
	display_message(display, cave, player, "");
}

void control(Display *display, Cave *cave, Ship *digger, Ship *player, Input *input)
{
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
			case SDLK_ESCAPE:
			case SDLK_q:
				input->state = Input::QUIT;
				break;
			case SDLK_f:
				SDL_WM_ToggleFullScreen(display->screen);
				break;
			case SDLK_p:
			case SDLK_PAUSE:
			case SDLK_SPACE:
				if(input->state == Input::WELCOME 
				|| input->state == Input::PAUSE
				|| input->state == Input::GAMEOVER) {
					if(input->state == Input::GAMEOVER)
						game_init(display, cave, digger, player);
					else
						display_message(display, cave, player, "");
					input->state = Input::PLAY;
				}
				else if(input->state == Input::PLAY)  {
					input->state = Input::PAUSE;
					display_message(display, cave, player, "paused");
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
			input->state = Input::QUIT;
			break;
		case SDL_VIDEORESIZE:
			viewport(display, event.resize.w, event.resize.h, 0, display->screen->flags & SDL_FULLSCREEN);
			break;
		case SDL_VIDEOEXPOSE:
			display_frame(display, cave, player);
			break;
		default:
			break;
		}
	}
}

void player_control(Ship *player, Input *input)
{
	player->lefton  = input->pressed[SDLK_LEFT];
	player->righton = input->pressed[SDLK_RIGHT];
}

void args_init(Args *args, int argc, char *argv[])
{
	args->width = 640;
	args->height = 480;
	args->bpp = 0;
	args->fullscreen = 0;
	args->highres = 0;
	int help_called = 0;

	struct {
		bool has_arg;
		int *val;
		char *short_name;
		char *long_name;
	} options[] = {
		{ 0, &help_called, "-h", "--help" },
		{ 1, &args->width, "-W", "--width" },
		{ 1, &args->height, "-H", "--height" },
		{ 1, &args->bpp, "-B", "--bpp" },
		{ 0, &args->fullscreen, "-F", "--fullscreen" },
		{ 0, &args->highres, "-R", "--highres" },
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

int main(int argc, char *argv[])
{
	Args args;
	Display display;
	Input input;
	memset( input.pressed, 0, sizeof(input.pressed) );

	srand(time(NULL));

	Cave cave;
	Ship digger;
	Ship player;

	args_init(&args, argc, argv);
	display_init(&display, &args);

	game_init(&display, &cave, &digger, &player);
	input.state = Input::WELCOME;
	display_message(&display, &cave, &player, "welcome!  left+right for control.  [press space]");

	float dt = 0;
	while(input.state != Input::QUIT) {
		int t0 = SDL_GetTicks();

		control(&display, &cave, &digger, &player, &input);

		switch(input.state) {
		case Input::PLAY:
			player_control(&player, &input);
			ship_move(&player, dt);
			digger_control(&digger);
			ship_move(&digger, dt);
			if(collision(&cave, &player) <= 0) {
				display_message(&display, &cave, &player, "gameover.  [press space]");
				input.state = Input::GAMEOVER;
			}
			cave_gen(&cave, &digger);

			display_frame(&display, &cave, &player);
			break;
		case Input::WELCOME:
		case Input::PAUSE:
		case Input::GAMEOVER:
		case Input::QUIT:
			break;
		}

		int t1 = SDL_GetTicks();
		SDL_Delay( MAX( 1, 1000/FPS-(t1-t0) ) );

		dt = (SDL_GetTicks()-t0)/1000.;
		//dt = 1./FPS;
	}

	display_message(&display, &cave, &player, "bye.");

	return 0;
}

// vim600:fdm=syntax:fdn=1:
