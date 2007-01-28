
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "vec.h"
#include "display.h"
#include "game.h"

typedef struct {
	bool pressed[SDLK_LAST];
	bool run;
	bool live;
	bool paused;
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
				input->run = 0;
				break;
			case SDLK_f:
				SDL_WM_ToggleFullScreen(display->screen);
				break;
			case SDLK_p:
			case SDLK_PAUSE:
				input->paused = !input->paused;
				break;
			default:
				break;
			}
		case SDL_KEYUP:
			input->pressed[event.key.keysym.sym] 
				= (event.type == SDL_KEYDOWN);
			break;
		case SDL_QUIT:
			input->run = 0;
			break;
		case SDL_VIDEORESIZE:
			viewport(display, event.resize.w, event.resize.h, 0);
			break;
		}
	}
}

void player_control(Ship *player, Input *input)
{
	player->lefton  = (input->pressed[SDLK_UP] || input->pressed[SDLK_LEFT]) ? 1 : 0;
	player->righton = (input->pressed[SDLK_UP] || input->pressed[SDLK_RIGHT]) ? 1 : 0;
}

int main(int argc, char *argv[])
{
	Display display;
	Input input;
	input.run = 1;
	input.live = 1;
	input.paused = 0;
	memset( input.pressed, 0, sizeof(input.pressed) );

	srand(time(NULL));

	Cave cave;
	Ship digger;
	Ship player;

	display_init(&display);
	ship_init(&player, 1);
	ship_init(&digger, 12);
	cave_init(&cave,&digger);

	float dt = 0;
	while(input.run) {
		int t0 = SDL_GetTicks();
		display.rect_n = 0;

		control(&display, &input);

		if(!input.paused) {

			display_start_frame(&display, &player);

			if(input.live) {
				player_control(&player, &input);
				ship_move(&player, dt);
				digger_control(&digger);
				ship_move(&digger, dt);
				if(colision(&cave, &player) <= 0)
					input.live = 0;
				cave_gen(&cave, &digger);

				cave_model(&cave);
				ship_model(&player);
			} else {
				//render_gameover();
			}

			glFinish();

			render_hud(&display, &player);
			SDL_UpdateRects(display.screen, display.rect_n, display.rect); // only update 2D

			SDL_GL_SwapBuffers(); // update 3d
		}

		int t1 = SDL_GetTicks();
		SDL_Delay( MAX( 1, 1000/FPS-(t1-t0) ) );

		dt = (SDL_GetTicks()-t0)/1000.;
	}

	return 0;
}

// vim600:fdm=syntax:fdn=1:
