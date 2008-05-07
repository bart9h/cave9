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
#include <math.h>
#include <float.h>
#include <assert.h>
#include "display.h"
#include "util.h"

const float shake_hit = 3.0;
const float shake_vel = 0.2;
const float shake_velZ = 0.08;
const float shake_thrust = 0.14;

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

static void display_world_transform (Display* display, Ship* player)
{
	COPY(display->cam, player->pos);

	if (display->shaking) {
		float hit = ship_hit(player);
		Vec3 shake = {
			shake_hit    * RAND * player->radius * hit + 
			shake_vel    * RAND * player->radius * player->vel[0] / MAX_VEL_X +
			shake_thrust * RAND * player->radius * player->lefton + 
			shake_thrust * RAND * player->radius * player->righton +
			shake_velZ   * RAND * player->radius * player->vel[2] / MAX_VEL_Z,
					 
			shake_hit    * RAND * player->radius * hit + 
			shake_vel    * RAND * player->radius * player->vel[1] / MAX_VEL_Y +
			shake_velZ   * RAND * player->radius * player->vel[2] / MAX_VEL_Z,
					 
			shake_hit    * RAND * player->radius * hit +
			shake_vel    * RAND * player->radius * player->vel[2] / MAX_VEL_Z
		};
		ADD(display->cam, shake);
		ADDSCALE(display->cam, player->repulsion, hit * player->radius * 2);
	}

	ADD2(display->target, player->pos, player->lookAt);
	//display->target[1]=display->target[1]*.5+player->pos[1]*.5;
	//display->target[2]+=10;
	gluLookAt(
		display->cam[0], display->cam[1], display->cam[2],
		display->target[0], display->target[1], display->target[2],
		0,1,0
	);
}

static void cave_model (Display* display, Cave* cave, int mode)
{

	for (int i = 0; i < SEGMENT_COUNT-1; ++i) {
		if (display->aidtrack  &&  mode == DISPLAYMODE_NORMAL && !(i&1)) {
			glColor4f(0.5,0.5,1,1);
			glBegin(GL_LINE_STRIP);

#define SIZE 0.1
			glVertex3f(cave->centers[i][0]-SIZE,cave->centers[i][1]-SIZE,cave->centers[i][2]);
			glVertex3f(cave->centers[i][0]+SIZE,cave->centers[i][1]+SIZE,cave->centers[i][2]);
			glVertex3fv(cave->centers[i]);
			glVertex3f(cave->centers[i][0]+SIZE,cave->centers[i][1]-SIZE,cave->centers[i][2]);
			glVertex3f(cave->centers[i][0]-SIZE,cave->centers[i][1]+SIZE,cave->centers[i][2]);

			glEnd();
		}

		int i0 = (cave->i + i)%SEGMENT_COUNT;

		if(cave->dirty[i0]) {
			for (int mode = 0; mode < DISPLAYMODE_COUNT; ++mode) {
				if (glIsList (display->gl_list[mode][i0]))
					glDeleteLists (display->gl_list[mode][i0], 1);
				display->gl_list[mode][i0] = 0;
			}
			cave->dirty[i0] = false;
		}

		if (display->gl_list[mode][i0] == 0) {
			int id = display->gl_list[mode][i0] = i0 + display->list_start[mode];

			glNewList (id, GL_COMPILE);

			int i1 = (i0 + 1)%SEGMENT_COUNT;
			if (mode == DISPLAYMODE_NORMAL) {
				glBindTexture (GL_TEXTURE_2D,
#ifdef OUTSIDE_TEXTURE_FILE
						cave->segs[0][0][2] < ROOM_LEN/2
						? display->outside_texture_id : 
#endif
						display->wall_texture_id
				);
			}

			glBegin (GL_QUAD_STRIP);
			for (int k = 0; k <= SECTOR_COUNT; ++k) {

				int k0 = k%SECTOR_COUNT;

#if TEXTURE_BOUNDARY_DEBUG
				if (mode == DISPLAYMODE_NORMAL) {
					if(i0==0||i1==0||k==3*SECTOR_COUNT/4)
						glColor4f(1, 0, 0, 0.5);
					else
						glColor4f(1, 1, 1, 0.5);
				}
#else
				//glColor4f(1, .8, .7, 0.5);
				glColor4f(1, 1, 1, 0.5);
#endif

				if (mode == DISPLAYMODE_NORMAL) {
					glTexCoord2f(
#ifndef NO_STRETCH_FIX
							cave->segs[i0][k0][2]/SEGMENT_LEN/SEGMENT_COUNT, 
#else
							(float)(cave->i+i)/SEGMENT_COUNT,
#endif
							(float)k/SECTOR_COUNT);
				} else if (mode == DISPLAYMODE_MINIMAP) {
					glColor4f(
							(float)i0/SEGMENT_COUNT,
							1-(float)i0/SEGMENT_COUNT,
							(float)k0/SECTOR_COUNT,
							0.5);
				}
				glVertex3fv(cave->segs[i0][k0]);

				if (mode == DISPLAYMODE_NORMAL) {
					glTexCoord2f(
#ifndef NO_STRETCH_FIX
							cave->segs[i1][k0][2]/SEGMENT_LEN/SEGMENT_COUNT, 
#else
							((float)(cave->i+i+1))/SEGMENT_COUNT,
#endif
							(float)k/SECTOR_COUNT);
				} else if (mode == DISPLAYMODE_MINIMAP) {
					glColor4f(
							(float)i1/SEGMENT_COUNT,
							1-(float)i1/SEGMENT_COUNT,
							(float)k0/SECTOR_COUNT,
							0.5);
				}
				glVertex3fv(cave->segs[i1][k0]);
			}
			glEnd();

			glEndList();
		}

		if (mode == DISPLAYMODE_NORMAL) {
			glEnable  (GL_DEPTH_TEST);
			glDisable (GL_BLEND);
			glEnable  (GL_TEXTURE_2D);
		} else {
			glDisable (GL_DEPTH_TEST);
			glEnable  (GL_BLEND);
			glDisable (GL_TEXTURE_2D);
		}

		glCallList (display->gl_list[mode][i0]);
	}

}

static void monolith_model (Display* display, Game* game)
{
	if (!game->monoliths)
		return;

	glColor3f(.2,.2,.2);

	float w = MONOLITH_WIDTH/2;
	float h = MONOLITH_HEIGHT/2;
	float d = MONOLITH_DEPTH;

	glEnable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);

	glPushMatrix();

		glTranslatef (game->cave.monolith_pos[0], game->cave.monolith_pos[1], game->cave.monolith_pos[2]);
		glRotatef (game->cave.monolith_yaw,   1, 0, 0);

		glBegin (GL_QUAD_STRIP);
			glVertex3f (+w, -h, d);  glVertex3f (-w, -h, d);
			glVertex3f (+w, -h, 0);  glVertex3f (-w, -h, 0);
			glVertex3f (+w, +h, 0);  glVertex3f (-w, +h, 0);
			glVertex3f (+w, +h, d);  glVertex3f (-w, +h, d);
		glEnd();

		glBegin (GL_QUADS);
			glVertex3f (-w, -h, d);  glVertex3f (-w, +h, d);
			glVertex3f (-w, +h, 0);  glVertex3f (-w, -h, 0);

			glVertex3f (+w, +h, d);  glVertex3f (+w, -h, d);
			glVertex3f (+w, -h, 0);  glVertex3f (+w, +h, 0);
		glEnd();

	glPopMatrix();
}

static void ship_model(Display* display, Ship* ship)
{
	if(!display->cockpit)
		return;

	//if(ship->dist <= 0) return;

	float alpha = (1-MIN(1,(ship->pos[2]/MIN_CAVE_RADIUS_DEPTH)))/8.;
	if(alpha == 0)
		return;

	float alert_dist = ship->radius*10;
	float white = ship->dist <= 0 || ship->dist > alert_dist ? 1 :
		1-(alert_dist - ship->dist)/alert_dist;

	float f = 1.8;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glColor4f(1,white,white,alpha);
	glPushMatrix();
		glTranslatef(0,0,-SHIP_RADIUS*f);
		glCallList( display->ship_list );
	glPopMatrix();

	glPushMatrix();
		display_world_transform(display, ship);
		glTranslatef(
				ship->pos[0],
				ship->pos[1],
				ship->pos[2]+SHIP_RADIUS*f
		);
		glCallList( display->ship_list );
	glPopMatrix();
}

static void render_text_box (Display* display, GLuint *id, 
		TTF_Font *font, const char* text,
		float x, float y, float w, float h,
		float r, float g, float b)
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

	gluBuild2DMipmaps(GL_TEXTURE_2D,
			GL_RGBA, label->w, label->h,
			GL_RGBA, GL_UNSIGNED_BYTE, label->pixels);

	SDL_FreeSurface(label);

	glPushMatrix();
		glColor3f(r,g,b);
		glTranslatef(0,0,-2.65); // XXX magic number
		glBegin(GL_QUAD_STRIP);
			glTexCoord2f(0,1);  glVertex3f(1-x*2+w,1-y*2-h,.5);
			glTexCoord2f(0,0);  glVertex3f(1-x*2+w,1-y*2+h,0);

			glTexCoord2f(1,1);  glVertex3f(1-x*2-w,1-y*2-h,.5);
			glTexCoord2f(1,0);  glVertex3f(1-x*2-w,1-y*2+h,0);
		glEnd();
	glPopMatrix();
}

static void render_text (Display* display, GLuint *id, 
		TTF_Font *font, const char* text,
		float x, float y, float scale,
		float r, float g, float b)
{
	int w, h;
	if (TTF_SizeText (font, text, &w, &h) == 0) {
		float s = scale*.01;
		render_text_box (display, id, font, text, x+w*s/2, y-h*s/2, w*s, h*s, r, g, b);
	}
	else {
		fprintf (stderr,
				"Error getting TTF size of string \"%s\":\n%s\n",
				text,
				TTF_GetError()
		);
	}
}

static void display_hud (Display* display, Game* game)
{
	if(game->player.dist == FLT_MAX)
		return;

#define HUD_TEXT_MAX 80
	char buf[HUD_TEXT_MAX];

#ifdef FONT_MENU_FILE
	TTF_Font* font = display->font_menu;
#else
	TTF_Font* font = display->font;
#endif

	int score = game_score(game);

	if (game->player.dist > 0) { // FIXME display hiscore before dead

		float max_vel[3] = { MAX_VEL_X, MAX_VEL_Y, MAX_VEL_Z };
		float vel = MIN(1,
				log(1+MAX(0,LEN(game->player.vel)-MAX_VEL_Z)) /
				log(1+MAX(0,LEN(max_vel)-MAX_VEL_Z)));
		float white = game->player.dist <= 0 ? 1 : 1-vel;

#ifdef ROMAN_SCORE
		snprintf (buf, HUD_TEXT_MAX, "SCORE  %s", roman(score));
#else
		snprintf (buf, HUD_TEXT_MAX, "SCORE  %d", score);
#endif

		render_text (display, &display->hud_id, font,
				buf, 
				.6,.95, .1, 1,white,white);

		char gauge[] = "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\";
		int n = MIN(strlen(gauge), (int)(vel*20));
		gauge[n] = '\0';
		snprintf (buf, HUD_TEXT_MAX, "velocity  %s", gauge);

		render_text (display, &display->hud_id, font,
				buf, 
				.1,.95, .1, 1,white,white);
	} else {
		if (game_nocheat(game)) {
			snprintf(buf, HUD_TEXT_MAX, "SCORE %d (%d, %d, %d)",
				score,
				game->score.session,
				game->score.local,
				game->score.global
			);
		}
		else {
			snprintf (buf, HUD_TEXT_MAX, "SCORE %d (%d) - %d",
				score,
				game->score.session,
				(int)game->player.start
			);
		}

		render_text_box (display, &display->hud_id, font,
				buf, 
				.5,.85,1,.2, 1,1,1);
	}

}

static char display_message_buf[256];
void display_message (Display* display, Game* game, const char* buf)
{
	strncpy (display_message_buf, buf, sizeof(display_message_buf)-1);
	display_message_buf[sizeof(display_message_buf)-1] = '\0';
	display_frame (display, game);
}

static void display_start_frame(Display* display, float r, float g, float b)
{
	glClearColor(r,g,b,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void display_end_frame(Display* display)
{
	glFinish();

	SDL_GL_SwapBuffers();
}

static void display_minimap (Display* display, Game* game)
{
	float len = cave_len(&game->cave);
	glPushMatrix();
		glScalef(.0065,.003,.001);
		glRotatef(-90,0,1,0);
		glTranslatef(
				-game->player.pos[0]-len*8, // XXX hardcoded
				-game->player.pos[1]-MAX_CAVE_RADIUS*3,
				-game->player.pos[2]-len/2);
		cave_model (display, &game->cave, true);

		glColor4f(1,1,1,0.05);
		glTranslatef (game->player.pos[0],game->player.pos[1],game->player.pos[2]);
		glCallList( display->ship_list );
	glPopMatrix();
}

void display_frame (Display* display, Game* game)
{
	display_start_frame (display, 0,0,0);

	if (game != NULL) {
		float hit = ship_hit(&game->player);
		if(hit < .9) { // avoid drawing the cave from outside
			glPushMatrix();
				if (game->player.lefton && !game->player.righton)
				{
					glRotatef(M_PI_2, 0, 0, 1);
				} else if (game->player.righton && !game->player.lefton)
				{
					glRotatef(-M_PI_2, 0, 0, 1);
				}
				display_world_transform (display, &game->player);
				cave_model (display, &game->cave, DISPLAYMODE_NORMAL);
				monolith_model (display, game);
			glPopMatrix();
		}

		if(hit) {
			glDisable (GL_DEPTH_TEST);
			glEnable  (GL_BLEND);
			glDisable (GL_TEXTURE_2D);

			glColor4f(1,0,0,hit/2.);
			glBegin (GL_QUADS);
			glVertex3f(-1,-1,-1);
			glVertex3f(+1,-1,-1);
			glVertex3f(+1,+1,-1);
			glVertex3f(-1,+1,-1);
			glEnd();
		}

		ship_model (display, &game->player);
		display_minimap (display, game);
		display_hud (display, game);
	}

	render_text_box (display, &display->msg_id, 
			display->font, display_message_buf, 
			.5,.5,1,.25, 1,1,1);

	display_end_frame(display);
}

static GLuint display_make_ship_list()
{
	/* Magic Numbers: It is possible to create a dodecahedron by attaching two pentagons
	 * to each face of a cube. The coordinates of the points are:
	 * (+-x,0, z); (+-1, 1, 1); (0, z, x )
	 * where x = 0.61803398875 and z = 1.61803398875.
	 */
	const float x = 0.61803398875;
	const float z = 1.61803398875;
	const float a = 0.525731112119;
	const float b = 0.850650808354;
	const float p[12][6][3] = {
		{ {  0,  a,  b }, {  0,  z,  x }, { -1,  1,  1 }, { -x,  0,  z }, {  x,  0,  z }, {  1,  1,  1 } },
		{ {  0,  a, -b }, {  0,  z, -x }, {  1,  1, -1 }, {  x,  0, -z }, { -x,  0, -z }, { -1,  1, -1 } },
		{ {  0, -a,  b }, {  0, -z,  x }, {  1, -1,  1 }, {  x,  0,  z }, { -x,  0,  z }, { -1, -1,  1 } },
		{ {  0, -a, -b }, {  0, -z, -x }, { -1, -1, -1 }, { -x,  0, -z }, {  x,  0, -z }, {  1, -1, -1 } },

		{ {  b,  0,  a }, {  x,  0,  z }, {  1, -1,  1 }, {  z, -x,  0 }, {  z,  x,  0 }, {  1,  1,  1 } },
		{ { -b,  0,  a }, { -x,  0,  z }, { -1,  1,  1 }, { -z,  x,  0 }, { -z, -x,  0 }, { -1, -1,  1 } },
		{ {  b,  0, -a }, {  x,  0, -z }, {  1,  1, -1 }, {  z,  x,  0 }, {  z, -x,  0 }, {  1, -1, -1 } },
		{ { -b,  0, -a }, { -x,  0, -z }, { -1, -1, -1 }, { -z, -x,  0 }, { -z,  x,  0 }, { -1,  1, -1 } },

		{ {  a,  b,  0 }, {  z,  x,  0 }, {  1,  1, -1 }, {  0,  z, -x }, {  0,  z,  x }, {  1,  1,  1 } },
		{ {  a, -b,  0 }, {  z, -x,  0 }, {  1, -1,  1 }, {  0, -z,  x }, {  0, -z, -x }, {  1, -1, -1 } },
		{ { -a,  b,  0 }, { -z,  x,  0 }, { -1,  1,  1 }, {  0,  z,  x }, {  0,  z, -x }, { -1,  1, -1 } },
		{ { -a, -b,  0 }, { -z, -x,  0 }, { -1, -1, -1 }, {  0, -z, -x }, {  0, -z,  x }, { -1, -1,  1 } }
	};

	GLuint ship_list = glGenLists (SEGMENT_COUNT);
	glNewList (ship_list, GL_COMPILE);
	for (int j = 0;  j < 12;  j++) {
		glBegin (GL_LINE_LOOP);
			glNormal3fv (p[j][0]);
			for (int i = 1;  i < 6;  i++)
				glVertex3fv (p[j][i]);
		glEnd();
	}
	glEndList();

	return ship_list;
}

static GLuint load_texture (const char* filename)
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

	GLenum err = gluBuild2DMipmaps(GL_TEXTURE_2D,
			GL_RGB, texture->w, texture->h,
			GL_RGB, GL_UNSIGNED_BYTE,texture->pixels);
	if(err) {
		fprintf(stderr, "gluBuild2DMipmaps(): %s\n", gluErrorString(err));
		exit(1);
	}

	SDL_FreeSurface(texture);
	return id;
}

static TTF_Font *load_font (const char* filename, int size)
{
	filename = FIND (filename);
	TTF_Font *font = TTF_OpenFont(filename, size);
	if(font == NULL) {
		fprintf(stderr, "TTF_OpenFont(%s,%d): %s\n", filename, size, TTF_GetError());
		exit(1);
	}
	return font;
}

void display_init (Display* display, Args* args)
{
	memset(display, 0, sizeof(Display));

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

	display->near_plane = MIN(SEGMENT_LEN,SHIP_RADIUS)/16.; // was EPSILON;
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
		w = BASE_W;
		h = BASE_H;
#endif
		f = 1;
	}
	viewport(display, w, h, args->bpp, f, args->antialiasing);

	for (int mode = 0; mode < DISPLAYMODE_COUNT; ++mode)
		display->list_start[mode] = glGenLists (SEGMENT_COUNT);

	if(TTF_Init() != 0) {
		fprintf(stderr, "TTF_Init(): %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);

    display->hud_id = 0;
    display->msg_id = 0;

	float scale = MIN(
			display->screen->w/(float)BASE_W, 
			display->screen->h/(float)BASE_H
		) * args->antialiasing ? 2 : 1;

	display->font      = load_font(FONT_FILE,      48*scale);

	display_start_frame(display, 0,0,0);
	render_text_box(display, &display->msg_id, display->font, 
			"loading cave9", .5,.5,1,.25, .75,.25,.25);
	display_end_frame(display);

#ifdef FONT_MENU_FILE
	display->font_menu = load_font(FONT_MENU_FILE, 22*scale);
#endif

	display->wall_texture_id    = load_texture (WALL_TEXTURE_FILE);
#ifdef OUTSIDE_TEXTURE_FILE
	display->outside_texture_id = load_texture (OUTSIDE_TEXTURE_FILE);
#endif

	display->ship_list = display_make_ship_list();

	display->cockpit = args->cockpit;
	display->shaking = !args->noshake;
	display->aidtrack = args->aidtrack;
}

// vim600:fdm=syntax:fdn=1:
