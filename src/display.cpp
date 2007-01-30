
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "display.h"

//#define AA //anti-aliasing

void viewport(Display *display, GLsizei w, GLsizei h, GLsizei bpp)
{
	if( bpp == 0 )
		bpp = display->screen->format->BitsPerPixel;

	// video mode
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, bpp/3 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, bpp/3 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bpp/3 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
#ifdef AA
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );
#endif
	display->screen = SDL_SetVideoMode(w, h, bpp, SDL_HWSURFACE|SDL_OPENGLBLIT|SDL_RESIZABLE);
	if(display->screen == NULL) goto error;
	//printf("%dx%dx%d\n", display->screen->w, display->screen->h, display->screen->format->BitsPerPixel);

#ifdef AA
	int arg;
	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLEBUFFERS, &arg );
	//printf("SDL_GL_MULTISAMPLEBUFFERS %d\n", arg);
	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &arg );
	//printf("SDL_GL_MULTISAMPLESAMPLES %d\n", arg);
#endif

	// projection
	glViewport(0,0,w,h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,-w/(GLfloat)h,display->near_plane,display->far_plane);


	// settings
	glClearColor(0,0,0,0);
	glClearDepth(1);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);
	//glShadeModel(GL_FLAT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if 1
	{
		glFogi(GL_FOG_MODE, GL_LINEAR);
		GLfloat fog_color[] = {0,0,0,1};
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_START, display->near_plane);
		glFogf(GL_FOG_END, display->far_plane);
		glEnable(GL_FOG);
	}
#endif

#ifdef AA
#ifndef GL_ARB_multisample
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV,GL_NICEST);
#endif
	glEnable(GL_MULTISAMPLE_ARB);
#endif

	return;
	error:
	fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
	exit(1);
}

void cave_model(Cave *cave)
{
	cave->ymin = FLT_MAX;
	cave->ymax = FLT_MIN;

	int i, j;
	glEnable(GL_BLEND);
	for( j = 0; j < CAVE_DEPTH-1; ++j ) {
		int j0 = (cave->i + j)%CAVE_DEPTH;
		int j1 = (j0 + 1)%CAVE_DEPTH;
		glBegin(GL_TRIANGLE_STRIP);
		for( i = 0; i <= N_SEGS; ++i ) {

			int i0 = i%N_SEGS;

			glColor4f(.4, .6*i0/N_SEGS, .9*j1/CAVE_DEPTH, 1);
			glVertex3fv(cave->segs[j0][i0]);

			glColor4f(.6, .6*i0/N_SEGS, .9*(1-j0/CAVE_DEPTH), 1);
			glVertex3fv(cave->segs[j1][i0]);

			if(cave->segs[j][i0][1] < cave->ymin)
				cave->ymin = cave->segs[j][i0][1];
			if(cave->segs[j][i0][1] > cave->ymax)
				cave->ymax = cave->segs[j][i0][1];
		}
		glEnd();
	}
	glDisable(GL_BLEND);

}

void ship_model(Ship *ship)
{
}

void render_text(Display *display, const char *text, float x, float y)
{
#ifdef USE_TTF
	SDL_Color color = {0xff,0xff,0xff,0xff};
	SDL_Surface *label = TTF_RenderText_Blended(display->font, text, color);
	assert(label != NULL);

	display->rect[display->rect_n].w = label->w;
	display->rect[display->rect_n].h = label->h;
	display->rect[display->rect_n].x = (int)(x*display->screen->w - .5*label->w);
	display->rect[display->rect_n].y = (int)(y*display->screen->h - .5*label->h);

	SDL_FillRect(display->screen, &display->rect[display->rect_n],
		SDL_MapRGBA(display->screen->format, 0x00,0x00,0x80,0xff));
	SDL_BlitSurface(label, NULL, display->screen, &display->rect[display->rect_n]);

	SDL_FreeSurface(label);
	++display->rect_n;
#endif
}

void display_hud(Display *display, Ship *player)
{
	char buf[80];
	sprintf(buf, "collision %.1f  velocity %.3fKm/s  score %.1f",
			player->dist, LEN(player->vel), player->pos[2]);
	render_text(display, buf, .5, .95);
}

void display_message(Display *display, const char *msg)
{
	render_text(display, msg, .5, .5);
	SDL_UpdateRects(display->screen, display->rect_n, display->rect); // only update 2D
	SDL_GL_SwapBuffers(); // update geral
}

void display_start_frame(Display *display, Ship *player)
{
	COPY(display->cam, player->pos);
	SET(display->target, player->pos[0], player->pos[1], player->pos[2]+1);
	//ADD2(display->target, player->pos, player->vel);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#if 1
	gluLookAt(
		display->cam[0], display->cam[1], display->cam[2],
		display->target[0], display->target[1], display->target[2],
		0,1,0
	);
#else // de lado
	gluLookAt(
		display->cam[0], display->cam[1], display->cam[2],
		display->target[0], display->target[1], display->target[2],
		0,1,0
	);
#endif
}

void display_end_frame(Display *display, Ship *player)
{
	glFinish();

	SDL_UpdateRects(display->screen, display->rect_n, display->rect); // only update 2D

	SDL_GL_SwapBuffers(); // update 3d
}

void display_init(Display *display)
{
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	display->rect_n = 0;
	display->near_plane = EPSILON;
	display->far_plane = CAVE_DEPTH * SEGMENT_LEN;
	SET(display->cam,0,0,0);
	SET(display->target,0,0,1);

	viewport(display,640,480,16);

#ifdef USE_TTF
	if(TTF_Init() != 0) {
		fprintf(stderr, "TTF_Init(): %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);

	char* font_filename = "font.ttf";
	int font_size = 16;
	display->font = TTF_OpenFont(font_filename, font_size); // FIXME path
	if(display->font == NULL) {
		fprintf(stderr, "TTF_OpenFont(%s): %s\n", font_filename, TTF_GetError());
		exit(1);
	}
#endif

	display->minimap = SDL_CreateRGBSurface( SDL_SWSURFACE,
			CAVE_DEPTH*2, CAVE_DEPTH*2,
			display->screen->format->BitsPerPixel,
			0, 0, 0, 0);
	assert( display->minimap != NULL );
}

void display_minimap(Display *display, Cave *cave, Ship *player)
{
	SDL_FillRect( display->minimap, 0, 0 );

	// cave
	int i;
	for( i = 0; i < CAVE_DEPTH; ++i ) {
		int j = ((i+cave->i) % CAVE_DEPTH);
		float y1 = cave->segs[j][1*N_SEGS/4][1];
		float y0 = cave->segs[j][3*N_SEGS/4][1];
		SDL_Rect rect;
		rect.x = i*2;
		rect.w = 2-1;
		rect.y = (int)(cave->ymax - y1);
		rect.h = (int)(y1 - y0);
		SDL_FillRect( display->minimap, &rect, 0xffffffff );
	}

	{ // player ship
		SDL_Rect rect;
		rect.x = (int)(2*(player->pos[2] - cave->segs[cave->i][0][2]));
		rect.x = 2; //HACK: smooth. TODO: properly smooth cave and ship.
		rect.w = 2;
		rect.y = (int)(cave->ymax - player->pos[1]);
		rect.h = 2;
		SDL_FillRect( display->minimap, &rect, 0 );
	}


	// blit minimap to screen
	SDL_Rect rect;
	rect.x = 0;
	rect.w = display->minimap->w;
	rect.y = 0;
	rect.h = (int)(cave->ymax - cave->ymin);

	SDL_Rect *d_rect = &display->rect[display->rect_n++];
	d_rect->x = display->screen->w - display->minimap->w;
	d_rect->w = display->minimap->w;
	d_rect->y = 0;
	d_rect->h = (int)(cave->ymax - cave->ymin);
	SDL_BlitSurface(display->minimap, &rect, display->screen, d_rect);
}

// vim600:fdm=syntax:fdn=1:
