
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
			case SDLK_RETURN:
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
			viewport(display, event.resize.w, event.resize.h, 0);
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

int main(int argc, char *argv[])
{
	Display display;
	Input input;
	memset( input.pressed, 0, sizeof(input.pressed) );

	srand(time(NULL));

	Cave cave;
	Ship digger;
	Ship player;

	display_init(&display);

	game_init(&display, &cave, &digger, &player);
	input.state = Input::WELCOME;
	display_message(&display, &cave, &player, "welcome!  left+right for control.  [press space]");

	float dt = 0;
	while(input.state != Input::QUIT) {
		int t0 = SDL_GetTicks();
		display.rect_n = 0;

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
