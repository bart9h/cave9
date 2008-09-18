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

#include <SDL_image.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "display.h"
#include "util.h"

void viewport (Display* display, GLsizei w, GLsizei h, GLsizei bpp,
		bool fullscreen, int aa)
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

	SDL_WM_SetIcon(display->icon, NULL);

	int flags = SDL_HWSURFACE|SDL_OPENGLBLIT|SDL_RESIZABLE;
	if(fullscreen)
		flags |= SDL_FULLSCREEN;
	display->screen = SDL_SetVideoMode(w, h, bpp, flags);
	if(display->screen == NULL) {
		fprintf(stderr, "SDL_SetVideoMode(%d,%d,%d,%d): %s\n", 
				w, h, bpp, flags, SDL_GetError());
		goto error;
	}

	bpp = display->screen->format->BitsPerPixel;
	printf("Display %dx%d-%dbit\n", 
			display->screen->w, display->screen->h, 
			display->screen->format->BitsPerPixel);

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

	if(aa) {
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
	} else {
		glShadeModel(GL_FLAT);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	}

	glLineWidth(16);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	{
		glFogi(GL_FOG_MODE, GL_LINEAR);
		GLfloat fog_color[] = {0,0,0,1};
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_START, display->near_plane);
		glFogf(GL_FOG_END, 128);
		glEnable(GL_FOG);
	}

	if(aa) {
#ifdef GL_ARB_multisample
		glEnable(GL_MULTISAMPLE_ARB);
#endif
		glHint(GL_MULTISAMPLE_FILTER_HINT_NV,GL_NICEST);
	}

	return;
	error:
	fprintf(stderr, ": %s\n", SDL_GetError());
	exit(1);
}

void display_text_box (Display* display, GLuint* id, 
		TTF_Font *font, const char* text,
		float x, float y, float w, float h,
		float r, float g, float b, float a)
{
	if(text == NULL || text[0] == '\0')
		return;

	if(*id == 0) {
		glGenTextures(1, id);
		glBindTexture(GL_TEXTURE_2D, *id);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	} else {
		glBindTexture(GL_TEXTURE_2D, *id);
	}

	SDL_Color color = {0xff,0xff,0xff,0xff};
	SDL_Surface* label = TTF_RenderText_Blended(font, text, color);
	if(label == NULL)
		return;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	// FIXME dont rebuild the texture when it didnt changed
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, label->w, label->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, label->pixels);

	SDL_FreeSurface(label);

	glPushMatrix();
		glColor4f(r,g,b,a);
		glTranslatef(0,0,-2.65); // XXX magic number
		glBegin(GL_QUAD_STRIP);
			glTexCoord2f(0,1);  glVertex3f(1-x*2+w,1-y*2-h,0);
			glTexCoord2f(0,0);  glVertex3f(1-x*2+w,1-y*2+h,0);

			glTexCoord2f(1,1);  glVertex3f(1-x*2-w,1-y*2-h,0);
			glTexCoord2f(1,0);  glVertex3f(1-x*2-w,1-y*2+h,0);
		glEnd();
	glPopMatrix();
}

void display_text (Display* display, GLuint* id, 
		TTF_Font *font, const char* text,
		float x, float y, float scale,
		float r, float g, float b, float a)
{
	int w, h;
	if (TTF_SizeText (font, text, &w, &h) == 0) {
		float s = scale*.01;
		display_text_box (display, id, font, text, x+w*s/2, y-h*s/2, w*s, h*s, r,g,b,a);
	}
	else {
		fprintf (stderr,
				"Error getting TTF size of string \"%s\":\n%s\n",
				text,
				TTF_GetError()
		);
	}
}

void display_start_frame (float r, float g, float b)
{
	glClearColor(r,g,b,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display_end_frame()
{
	glFinish();

	SDL_GL_SwapBuffers();
}

GLuint load_texture (const char* filename)
{
	GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	filename = FIND (filename);
	SDL_Surface* texture = IMG_Load (filename);
	if(texture == NULL) {
		fprintf(stderr, "IMG_Load(%s): %s\n", filename, IMG_GetError());
		exit(1);
	}

	if(texture->format->BitsPerPixel != 24) {
		fprintf(stderr, "texture '%s' format must be 24bits\n", filename);
		exit(1);
	}

	GLenum err = gluBuild2DMipmaps(GL_TEXTURE_2D,
			GL_RGB, texture->w, texture->h,
			GL_RGB, GL_UNSIGNED_BYTE,texture->pixels);
	if(err) {
		fprintf(stderr, "gluBuild2DMipmaps(): %s\n", gluErrorString(err));
		exit(1);
	}

	fprintf(stderr, "Display Texture '%s' %dx%d-24bit\n", 
			filename, texture->w, texture->h);

	SDL_FreeSurface(texture);
	return id;
}

TTF_Font *load_font (const char* filename, int size)
{
	filename = FIND (filename);
	TTF_Font *font = TTF_OpenFont(filename, size);
	if(font == NULL) {
		fprintf(stderr, "TTF_OpenFont(%s,%d): %s\n", filename, size, TTF_GetError());
		exit(1);
	}
	return font;
}

void display_init (Display* display, GLfloat near_plane, GLfloat far_plane, Args* args)
{
	memset(display, 0, sizeof(Display));
	display->near_plane = near_plane;
	display->far_plane  = far_plane;

#ifndef _WIN32
	// avoid crash on EeePC's Xandros
	if (NULL == SDL_getenv ("SDL_VIDEO_X11_WMCLASS"))
		SDL_putenv ("SDL_VIDEO_X11_WMCLASS=cave9");
#endif

	Uint32 flags = SDL_INIT_VIDEO;
	if (!args->nosound)
		flags |= SDL_INIT_AUDIO;
	if (SDL_Init (flags) != 0) {
		fprintf (stderr, "SDL_Init(): %s\n", SDL_GetError());
		exit (1);
	}
	atexit(SDL_Quit);

	const char* icon_file = FIND (ICON_FILE);
	display->icon = IMG_Load(icon_file);
	if(display->icon == NULL) {
		fprintf(stderr, "IMG_Load(%s): %s\n", icon_file, IMG_GetError());
		exit(1);
	}

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
		w = BASE_W;
		h = BASE_H;
#endif
		f = 1;
	}
	viewport(display, w, h, args->bpp, f, args->antialiasing);

	if(TTF_Init() != 0) {
		fprintf(stderr, "TTF_Init(): %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);
}

// vim600:fdm=syntax:fdn=1:
