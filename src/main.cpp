
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "vec.h"
#include "display.h"
#include "game.h"

typedef struct {
	bool pressed[SDLK_LAST];
	enum {WELCOME, INIT, PLAY, PAUSE, GAMEOVER, QUIT} state;
} Input;

void control(Display *display, Input *input)
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
				if(input->state == Input::PLAY)  {
					input->state = Input::PAUSE;
					display_message(display, "paused");
				}
				else if(input->state == Input::PAUSE)
					input->state = Input::PLAY;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
				if(input->state == Input::WELCOME || input->state == Input::GAMEOVER)
					input->state = Input::INIT;
				else if(input->state == Input::PAUSE)
					input->state = Input::PLAY;
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
			//TODO
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
	input.state = Input::WELCOME;
	display_message(&display, "welcome.  [press space]");

	float dt = 0;
	while(input.state != Input::QUIT) {
		int t0 = SDL_GetTicks();
		display.rect_n = 0;

		control(&display, &input);

		switch(input.state) {
		case Input::INIT:
			ship_init(&player, 1);
			ship_init(&digger, 12);
			cave_init(&cave,&digger);
			input.state = Input::PLAY;
			break;
		case Input::PLAY:
			display_start_frame(&display, &player);

			player_control(&player, &input);
			ship_move(&player, dt);
			digger_control(&digger);
			ship_move(&digger, dt);
			if(collision(&cave, &player) <= 0) {
				display_message(&display, "gameover.  [press space]");
				input.state = Input::GAMEOVER;
			}
			cave_gen(&cave, &digger);

			cave_model(&cave);
			ship_model(&player);
			display_end_frame(&display, &player);
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

	return 0;
}

// vim600:fdm=syntax:fdn=1:
