
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

void viewport(Display *display, GLsizei w, GLsizei h, GLsizei bpp, 
		bool fullscreen, int aa=0)
{
	// video mode
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, bpp/3 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, bpp/3 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bpp/3 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	if(aa) {
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, aa );
	}

	int flags = SDL_HWSURFACE|SDL_OPENGLBLIT|SDL_RESIZABLE;
	if(fullscreen)
		flags |= SDL_FULLSCREEN;
	display->screen = SDL_SetVideoMode(w, h, bpp, flags);
	if(display->screen == NULL) 
		goto error;

	bpp = display->screen->format->BitsPerPixel;
	//printf("%dx%dx%d\n", display->screen->w, display->screen->h, display->screen->format->BitsPerPixel);

#if 0
	if(aa) {
		int arg;
		SDL_GL_GetAttribute( SDL_GL_MULTISAMPLEBUFFERS, &arg );
		printf("SDL_GL_MULTISAMPLEBUFFERS %d\n", arg);
		SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &arg );
		printf("SDL_GL_MULTISAMPLESAMPLES %d\n", arg);
	}
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
#ifdef GL_ARB_multisample
	glEnable(GL_MULTISAMPLE_ARB);
#endif
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV,GL_NICEST);
#endif

	return;
	error:
	fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
	exit(1);
}

void cave_model(Display *display, Cave *cave)
{
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
				if(i0==0||i1==0||k==3*SECTOR_COUNT/4) 
					glColor3f(1, 0, 0); 
				else 
					glColor3f(1, 1, 1);
#endif

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
	}

}

void monolith_model(Display *display, Cave *cave, Ship *player)
{
	if(!display->monoliths)
		return;

	glColor3f(.2,.2,.2);

	float w = MONOLITH_WIDTH/2;
	float h = MONOLITH_HEIGHT/2;
	float d = MONOLITH_DEPTH;

	glPushMatrix();

		glTranslatef( cave->monolith_x, cave->monolith_y, cave->segs[0][0][2] );
		glRotatef( cave->monolith_yaw,   1, 0, 0 );

		glBegin( GL_QUAD_STRIP );
			glVertex3f( +w, -h, d );  glVertex3f( -w, -h, d );
			glVertex3f( +w, -h, 0 );  glVertex3f( -w, -h, 0 );
			glVertex3f( +w, +h, 0 );  glVertex3f( -w, +h, 0 );
			glVertex3f( +w, +h, d );  glVertex3f( -w, +h, d );
		glEnd();

		glBegin( GL_QUADS );
			glVertex3f( -w, -h, d );  glVertex3f( -w, +h, d );
			glVertex3f( -w, +h, 0 );  glVertex3f( -w, -h, 0 );
									 
			glVertex3f( +w, +h, d );  glVertex3f( +w, -h, d );
			glVertex3f( +w, -h, 0 );  glVertex3f( +w, +h, 0 );
		glEnd();

	glPopMatrix();
}

void ship_model(Ship *ship)
{
	// TODO use call-list
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
		glTranslatef(ship->pos[0],ship->pos[1],ship->pos[2]);
		glBegin(GL_QUADS);
			glColor4f(1,1,1,.25); 
				glVertex3f(+.75,+.01,SHIP_RADIUS*3);
				glVertex3f(+.75,-.01,SHIP_RADIUS*3);
				glVertex3f(-.75,-.01,SHIP_RADIUS*3);
				glVertex3f(-.75,+.01,SHIP_RADIUS*3);
		glEnd();
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void render_text(Display *display, GLuint id, const char *text, 
		float x, float y, float w, float h,
		float r, float g, float b)
{
	if(text == NULL || text[0] == '\0')
		return;
#ifdef USE_TTF
	SDL_Color color = {0xff,0xff,0xff,0xff};
	SDL_Surface *label = TTF_RenderText_Blended(display->font, text, color);
	assert(label != NULL);

    glBindTexture(GL_TEXTURE_2D, id);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 
			GL_RGBA, label->w, label->h, 
			GL_RGBA, GL_UNSIGNED_BYTE, label->pixels);

	SDL_FreeSurface(label);

	glPushMatrix();
		glColor3f(r,g,b);
		glTranslatef(0,0,-3); // XXX magic number
		glBegin(GL_QUAD_STRIP);
			glTexCoord2f(0,1);  glVertex3f(1-x*2+w,1-y*2-h,.5);
			glTexCoord2f(0,0);  glVertex3f(1-x*2+w,1-y*2+h,0);

			glTexCoord2f(1,1);  glVertex3f(1-x*2-w,1-y*2-h,.5);
			glTexCoord2f(1,0);  glVertex3f(1-x*2-w,1-y*2+h,0);
		glEnd();
	glPopMatrix();

#endif
}

void display_hud(Display *display, Ship *player)
{
	if(player->dist == FLT_MAX)
		return;
#define HUD_TEXT_MAX 80
	char buf[HUD_TEXT_MAX];
	snprintf(buf, HUD_TEXT_MAX, " collision %4.1f  velocity %5.2f  score %9.0f ",
			player->dist, LEN(player->vel), player->pos[2]);

#ifdef USE_TTF
	float alert_dist = player->radius*10;
	float c = player->dist <= 0 || player->dist > alert_dist ? 1 : 
		1-(alert_dist - player->dist)/alert_dist;
	render_text(display, display->hud_id, buf, .5,.9,.8,.1, 1,c,c);
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

void display_start_frame(Display *display, float r, float g, float b)
{
	glClearColor(r,g,b,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display_world_transform(Display *display, Ship *player)
{
	COPY(display->cam, player->pos);
	ADD2(display->target, player->pos, player->vel);
	//display->target[1]=display->target[1]*.5+player->pos[1]*.5;
	//display->target[2]+=10;
	gluLookAt(
		display->cam[0], display->cam[1], display->cam[2],
		display->target[0], display->target[1], display->target[2],
		0,1,0
	);
}

void display_end_frame(Display *display)
{
	glFinish();

	SDL_GL_SwapBuffers();
}

void display_frame(Display *display, Cave *cave, Ship *player)
{
	int hit = player->dist <= SHIP_RADIUS*1.1;

	display_start_frame(display, hit,0,0);

	if(!hit) { // avoid drawing the cave from outside
		glPushMatrix();
			display_world_transform(display, player);
			cave_model(display, cave);
			monolith_model(display, cave, player);
		glPopMatrix();
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();
			display_world_transform(display, player);
			ship_model(player);
		glPopMatrix();

		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
		display_minimap(display, cave, player);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		display_hud(display, player);
		render_text(display, display->msg_id, display_message_buf, .5,.5,.8,.1, 1,1,1);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	display_end_frame(display);
}

void display_init(Display *display, Args *args)
{

	memset(display, 0, sizeof(Display));

	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL_Init(): %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	display->near_plane = SHIP_RADIUS/2.; // was EPSILON;
	display->far_plane = SEGMENT_COUNT * SEGMENT_LEN;
	SET(display->cam, 0,0,0);
	SET(display->target, 0,0,1);

	int w = args->width;
	int h = args->height;
	int f = args->fullscreen;
	if(args->highres) {
#if SDL_VERSION_ATLEAST(1,2,11)
		const SDL_VideoInfo* info = SDL_GetVideoInfo();
		assert(info != NULL);
		w = info->current_w;
		h = info->current_h;
#else
		w = 1024;
		h = 768;
#endif
		f = 1;
	}
	viewport(display, w, h, args->bpp, f, args->antialiasing);
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

    glGenTextures(1, &display->hud_id);
    glBindTexture(GL_TEXTURE_2D, display->hud_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &display->msg_id);
    glBindTexture(GL_TEXTURE_2D, display->msg_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#ifdef TEXTURE
	char* texture_filename = "texture.jpg";

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

	display->monoliths = args->monoliths;
}

void display_minimap(Display *display, Cave *cave, Ship *player)
{
	glPushMatrix();
		glScalef(.005,.003,.001);
		glRotatef(-90,0,1,0);
		glTranslatef(
				-player->pos[0]-1000, // XXX hardcoded
				-player->pos[1]-100,
				-player->pos[2]-(SEGMENT_COUNT-1)*SEGMENT_LEN/2);
		cave_model(display, cave);
	glPopMatrix();

}

// vim600:fdm=syntax:fdn=1:
