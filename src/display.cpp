
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "display.h"

#define AA //anti-aliasing

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
	display->screen = SDL_SetVideoMode(w, h, bpp, 
			SDL_HWSURFACE|SDL_OPENGLBLIT|SDL_RESIZABLE|SDL_FULLSCREEN);
	if(display->screen == NULL) 
		goto error;
	//printf("%dx%dx%d\n", display->screen->w, display->screen->h, display->screen->format->BitsPerPixel);

#ifdef AA
	int arg;
	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLEBUFFERS, &arg );
	//printf("SDL_GL_MULTISAMPLEBUFFERS %d\n", arg);
	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &arg );
	//printf("SDL_GL_MULTISAMPLESAMPLES %d\n", arg);
#endif

	SDL_WM_SetCaption("cave9 -- 9hells.org", "cave9");
	SDL_ShowCursor(SDL_DISABLE);

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

	glPolygonMode( GL_FRONT, GL_FILL );
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

#ifdef TEXTURE
	glEnable(GL_TEXTURE_2D);
#endif

#if 1
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	{
		glFogi(GL_FOG_MODE, GL_LINEAR);
		GLfloat fog_color[] = {0,0,0,1};
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_START, display->near_plane);
		glFogf(GL_FOG_END, display->far_plane);
		glEnable(GL_FOG);
	}
#else
	glShadeModel(GL_FLAT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
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

void cave_model(Display *display, Cave *cave)
{
	cave->ymin = FLT_MAX;
	cave->ymax = FLT_MIN;

//	glEnable(GL_BLEND);
	for( int i = 0; i < SEGMENT_COUNT-1; ++i ) {
		int i0 = (cave->i + i)%SEGMENT_COUNT;

		if( cave->gl_list[i0] == 0 ) {
			int id = cave->gl_list[i0] = i0 + display->list_start;

			glNewList( id, GL_COMPILE_AND_EXECUTE );

			int i1 = (i0 + 1)%SEGMENT_COUNT;
#ifdef TEXTURE
				glBindTexture(GL_TEXTURE_2D, display->texture_id);
#endif
				glBegin(GL_QUAD_STRIP);
			for( int k = 0; k <= SECTOR_COUNT; ++k ) {

				int k0 = k%SECTOR_COUNT;

#ifdef TEXTURE
					glTexCoord2f( (float)i0/SEGMENT_COUNT, (float)k/SECTOR_COUNT);
#else
					glColor3f((float)i0/SEGMENT_COUNT, 1-(float)i0/SEGMENT_COUNT, (float)k0/SECTOR_COUNT);
#endif
				glVertex3fv(cave->segs[i0][k0]);

#ifdef TEXTURE
					glTexCoord2f( ((float)i0+1)/SEGMENT_COUNT, (float)k/SECTOR_COUNT);
#else
					glColor3f((float)i1/SEGMENT_COUNT, 1-(float)i1/SEGMENT_COUNT, (float)k0/SECTOR_COUNT);
#endif
				glVertex3fv(cave->segs[i1][k0]);
			}
			glEnd();

			glEndList();
		} else {
			glCallList( cave->gl_list[i0] );
		}


		if(cave->seg_y[i][0] < cave->ymin)
			cave->ymin = cave->seg_y[i][0];
		if(cave->seg_y[i][1] > cave->ymax)
			cave->ymax = cave->seg_y[i][1];
	}
//	glDisable(GL_BLEND);

}

void ship_model(Ship *ship)
{
}

void render_text(Display *display, const char *text, float x, float y)
{
	if(text == NULL || text[0] == '\0')
		return;
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
#define HUD_TEXT_MAX 80
	char buf[HUD_TEXT_MAX];
	float wow_factor = 20.0;
	snprintf(buf, HUD_TEXT_MAX, "collision %.1f  velocity %.2fKm/h  score %.1f",
			player->dist, wow_factor*LEN(player->vel), player->pos[2]);

#ifdef USE_TTF
	render_text(display, buf, .5, .95);
#else
	static Uint32 last_hud_print = 0;
	Uint32 now = SDL_GetTicks();
	if( now - last_hud_print >= 200 ) {
		last_hud_print = now;
		printf("\r%s", buf);
		fflush(stdout);
	}
#endif
}

char display_message_buf[256];
void display_message(Display *display, Cave *cave, Ship *player, const char *buf)
{
	strncpy(display_message_buf, buf, sizeof(display_message_buf)-1);
	display_message_buf[sizeof(display_message_buf)-1] = '\0';
#ifdef USE_TTF
	display_frame(display, cave, player);
#else
	printf("\n%s\n", display_message_buf);
#endif
}

void display_start_frame(Display *display, Ship *player)
{
	display->rect_n = 0;
	COPY(display->cam, player->pos);
	ADD2(display->target, player->pos, player->vel);
	display->target[1]=display->target[1]*.5+player->pos[1]*.5;
	display->target[2]+=10;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		display->cam[0], display->cam[1], display->cam[2],
		display->target[0], display->target[1], display->target[2],
		0,1,0
	);
}

void display_end_frame(Display *display)
{
	glFinish();

	SDL_UpdateRects(display->screen, display->rect_n, display->rect); // only update 2D

	SDL_GL_SwapBuffers(); // update 3d
}

void display_frame(Display *display, Cave *cave, Ship *player)
{
	int hit = player->dist <= 1;
	glClearColor( (hit?1:0), 0, 0, 0);
	display_start_frame(display, player);
	if(!hit) // avoid drawing the cave from outside
	cave_model(display, cave);
	ship_model(player);
	display_hud(display, player);
	display_minimap(display, cave, player);
	render_text(display, display_message_buf, .5, .5);
	display_end_frame(display);
}

void display_init(Display *display)
{
	
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL_Init(): %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	display->rect_n = 0;
	display->near_plane = SHIP_RADIUS/2.; // was EPSILON;
	display->far_plane = SEGMENT_COUNT * SEGMENT_LEN;
	SET(display->cam,0,0,0);
	SET(display->target,0,0,1);

	viewport(display,1024,768,16);
	display->list_start = glGenLists( SEGMENT_COUNT );

#ifdef USE_TTF
	if(TTF_Init() != 0) {
		fprintf(stderr, "TTF_Init(): %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);

	char* font_filename = "font.pcf";
	int font_size = 16;
	display->font = TTF_OpenFont(font_filename, font_size); // FIXME path
	if(display->font == NULL) {
		fprintf(stderr, "TTF_OpenFont(%s): %s\n", font_filename, TTF_GetError());
		exit(1);
	}
#endif

	display->minimap = SDL_CreateRGBSurface( SDL_SWSURFACE,
			SEGMENT_COUNT*2, SEGMENT_COUNT*2,
			display->screen->format->BitsPerPixel,
			0, 0, 0, 0);
	assert( display->minimap != NULL );

#ifdef TEXTURE
	char* texture_filename = "t4096.jpg";

    glGenTextures(1, &display->texture_id);
    glBindTexture(GL_TEXTURE_2D, display->texture_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_Surface *texture = IMG_Load(texture_filename);
	if(texture == NULL) {
		fprintf(stderr, "IMG_Load(%s): %s\n", texture_filename, IMG_GetError());
		exit(1);
	}

	GLenum err = gluBuild2DMipmaps(GL_TEXTURE_2D, 
			GL_RGB, texture->w, texture->h, 
			GL_RGB, GL_UNSIGNED_BYTE,texture->pixels);
	if(err) {
		fprintf(stderr, "gluBuild2DMipmaps(): %s\n", gluErrorString(err));
		exit(1);
	}

	SDL_FreeSurface(texture);
#endif

}

void display_minimap(Display *display, Cave *cave, Ship *player)
{
	SDL_FillRect( display->minimap, 0, 0 );

	// cave
	int i;
	for( i = 0; i < SEGMENT_COUNT; ++i ) {
		int i0 = ((i+cave->i) % SEGMENT_COUNT);
		float y1 = cave->segs[i0][1*SECTOR_COUNT/4][1];
		float y0 = cave->segs[i0][3*SECTOR_COUNT/4][1];
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
