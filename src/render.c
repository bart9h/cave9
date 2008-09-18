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
#include <math.h>
#include <float.h>
#include "render.h"
#include "util.h"

const float shake_hit = 3.0;
const float shake_vel = 0.2;
const float shake_velZ = 0.08;
const float shake_thrust = 0.14;

static Vec3 huemap[SEGMENT_COUNT];

static GLuint render_make_ship_list()
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

void render_init (Render* render, Args* args)
{
	memset (render, 0, sizeof(Render));
	Display* display = &render->display;

	GLfloat near_plane = MIN(SEGMENT_LEN,SHIP_RADIUS)/16.; // was EPSILON;
	GLfloat  far_plane = SEGMENT_COUNT * SEGMENT_LEN;
	display_init (display, near_plane, far_plane, args);

	SET(render->cam, 0,0,0);
	SET(render->target, 0,0,1);

	for (int mode = 0; mode < DISPLAYMODE_COUNT; ++mode)
		render->list_start[mode] = glGenLists (SEGMENT_COUNT);

	render->hud_id = 0;
	render->msg_id = 0;

	float scale = MIN(
			display->screen->w/(float)BASE_W, 
			display->screen->h/(float)BASE_H
		) * args->antialiasing ? 2 : 1;

	render->font      = load_font(FONT_FILE,      48*scale);

	display_start_frame (0,0,0);
	display_text_box(display, &render->msg_id, render->font, 
			"loading cave9", .5,.5,1,.25, .75,.25,.25,1);
	display_end_frame();

#ifdef FONT_MENU_FILE
	render->font_menu = load_font(FONT_MENU_FILE, 22*scale);
#endif

	render->wall_texture_id    = load_texture (WALL_TEXTURE_FILE);
#ifdef OUTSIDE_TEXTURE_FILE
	render->outside_texture_id = load_texture (OUTSIDE_TEXTURE_FILE);
#endif

	render->ship_list = render_make_ship_list();

	render->cockpit = args->cockpit;
	render->shaking = !args->noshake;
	render->aidtrack = args->aidtrack;
	render->roman = args->roman;

	render->lighting = args->lighting;

	{ // huemap for velocity gauge
		int base = 1;
		for (int h = 0; h < SEGMENT_COUNT; ++h) {
			int hue = (int)(720-2*360/3-360/3*
					log(h+base) / log(SEGMENT_COUNT+base)
					)%360;
			PIX_HSV_TO_RGB_COMMON(
				hue,1,1,
				huemap[h][0],
				huemap[h][1],
				huemap[h][2]);
		}
	}
}

static char render_message_buf[256];
void render_message (Render* render, Game* game, const char* buf)
{
	strncpy (render_message_buf, buf, sizeof(render_message_buf)-1);
	render_message_buf[sizeof(render_message_buf)-1] = '\0';
	render_frame (render, game);
}

static void render_world_transform (Render* render, Ship* player)
{
	COPY(render->cam, player->pos);

	if (render->shaking) {
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
		ADD(render->cam, shake);
		ADDSCALE(render->cam, player->repulsion, hit * player->radius * 2);
	}

	ADD2(render->target, player->pos, player->lookAt);
	//render->target[1]=render->target[1]*.5+player->pos[1]*.5;
	//render->target[2]+=10;

	GLfloat lightpos[] = {render->cam[0], render->cam[1], render->cam[2] + 1, 1.0f};
	glLightfv(GL_LIGHT1, GL_POSITION, lightpos);

	gluLookAt(
		render->cam[0], render->cam[1], render->cam[2],
		render->target[0], render->target[1], render->target[2],
			sin(player->roll), cos(player->roll), 0
	);
}

static void cave_model (Render* render, Cave* cave, int mode)
{

	for (int i = 0; i < SEGMENT_COUNT-1; ++i) {

		// aid bread-crumb track
		if (render->aidtrack  &&  mode == DISPLAYMODE_NORMAL && !(i&1)) {
			glDisable(GL_LIGHTING);
			glColor4f(0.5,0.5,1,1);
			glBegin(GL_LINE_STRIP);

			#define CRUMB_SIZE 0.1
			glVertex3f(cave->centers[i][0]-CRUMB_SIZE,cave->centers[i][1]-CRUMB_SIZE,cave->centers[i][2]);
			glVertex3f(cave->centers[i][0]+CRUMB_SIZE,cave->centers[i][1]+CRUMB_SIZE,cave->centers[i][2]);
			glVertex3fv(cave->centers[i]);
			glVertex3f(cave->centers[i][0]+CRUMB_SIZE,cave->centers[i][1]-CRUMB_SIZE,cave->centers[i][2]);
			glVertex3f(cave->centers[i][0]-CRUMB_SIZE,cave->centers[i][1]+CRUMB_SIZE,cave->centers[i][2]);

			glEnd();
		}

		if (render->lighting)
			glEnable(GL_LIGHTING);

		int i0 = (cave->i + i)%SEGMENT_COUNT;

		if (cave->dirty[i0]) {
			for (int mode = 0; mode < DISPLAYMODE_COUNT; ++mode) {
				if (glIsList (render->gl_list[mode][i0]))
					glDeleteLists (render->gl_list[mode][i0], 1);
				render->gl_list[mode][i0] = 0;
			}
			cave->dirty[i0] = false;
		}

		if (render->gl_list[mode][i0] == 0) {
			int id = render->gl_list[mode][i0] = i0 + render->list_start[mode];

			glNewList (id, GL_COMPILE);

			int i1 = (i0 + 1)%SEGMENT_COUNT;
			if (mode == DISPLAYMODE_NORMAL) {
				glBindTexture (GL_TEXTURE_2D,
#ifdef OUTSIDE_TEXTURE_FILE
						cave->segs[0][0][2] < ROOM_LEN/2
						? render->outside_texture_id : 
#endif
						render->wall_texture_id
				);

				glColor4f (1, 1, 1, 0.5);
			}

			glBegin (GL_QUAD_STRIP);
			for (int k = 0; k <= SECTOR_COUNT; ++k) {

				int k0 = k%SECTOR_COUNT;

				if (mode == DISPLAYMODE_NORMAL) {
					glTexCoord2f(
							cave->segs[i0][k0][2]/SEGMENT_LEN/SEGMENT_COUNT, 
							(float)k/SECTOR_COUNT);
				}

				GLfloat thenormal[] = {0, 0, 0};
				SUB2(thenormal, cave->centers[i0], cave->segs[i0][k0]);
				NORM(thenormal);
				glNormal3fv(thenormal);

				glVertex3fv(cave->segs[i0][k0]);

				if (mode == DISPLAYMODE_NORMAL) {
					glTexCoord2f(
							cave->segs[i1][k0][2]/SEGMENT_LEN/SEGMENT_COUNT, 
							(float)k/SECTOR_COUNT);
				}

				SUB2(thenormal, cave->centers[i1], cave->segs[i1][k0]);
				NORM(thenormal);
				glNormal3fv(thenormal);

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
			glDisable(GL_LIGHTING);
		}

		if (mode == DISPLAYMODE_MINIMAP) {
// apparently pow is a lot more complicated than what we need here.
#define FASTPOW6(a) a * a * a * a * a * a
			float alpha = .12f - .12f * FASTPOW6(((SEGMENT_COUNT / 2.0f - i) / SEGMENT_COUNT * 2.0f));

			if(i > render->gauge * SEGMENT_COUNT)
				glColor4f (1, 1, 1, alpha);
			else
				glColor4f (
				           huemap[i][0],
				           huemap[i][1],
				           huemap[i][2], alpha);
		}
		glCallList (render->gl_list[mode][i0]);
	}

}

static void monolith_model (Render* render, Game* game)
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
	if (render->lighting)
		glEnable (GL_LIGHTING);

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

static void ship_model(Render* render, Ship* ship)
{
	if(!render->cockpit)
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
	glDisable(GL_LIGHTING);

	glColor4f(1,white,white,alpha);
	glPushMatrix();
		glTranslatef(0,0,-SHIP_RADIUS*f);
		glCallList( render->ship_list );
	glPopMatrix();

	glPushMatrix();
		render_world_transform(render, ship);
		glTranslatef(
				ship->pos[0],
				ship->pos[1],
				ship->pos[2]+SHIP_RADIUS*f
		);
		glCallList( render->ship_list );
	glPopMatrix();
}

static void render_hud (Render* render, Game* game)
{
	if(game->player.dist == FLT_MAX)
		return;

#define HUD_TEXT_MAX 80
	char buf[HUD_TEXT_MAX];

#ifdef FONT_MENU_FILE
	TTF_Font* font = render->font_menu;
#else
	TTF_Font* font = render->font;
#endif

	void (*number) (char *, unsigned int) = render->roman ? roman : arabic;

	char score[NUMBER_STR_MAX];
	char session[NUMBER_STR_MAX];
	number (score, game_score(game));
	number (session, game->score.session);

	if (game->player.dist > 0) {
		snprintf (buf, HUD_TEXT_MAX, "%s", score);

		char session[NUMBER_STR_MAX];
		number (session,game->score.session);

		display_text (&render->display, &render->hud_id, font, buf, 
				0.25,1, .25, 1,1,1,.5);

		render->gauge = ship_speed(&game->player);

	}
	else if (game->player.dist == -1) {

		snprintf (buf, HUD_TEXT_MAX, "score %s", score);

		if (game->score.session != game_score(game)) {
			strcat (buf, "  session ");
			strcat (buf, " ");  //WTF? ttf bug?
			strcat (buf, session);
		}

		if (game_nocheat(game)) {

			if (game->score.session != game->score.local) {
				char local[NUMBER_STR_MAX];
				number (local, game->score.local);
				strcat (buf, "  local ");
				strcat (buf, local);
			}

			if (game->score.local != game->score.global) {
				char global[NUMBER_STR_MAX];
				number (global, game->score.global);
				strcat (buf, "  global ");
				strcat (buf, global);
			}
		}
		else {

			if (game->score.session != game->score.local) {
				char local[NUMBER_STR_MAX];
				number (local, game->score.local);
				strcat (buf, "  local ");
				strcat (buf, local);
			}

			char start[NUMBER_STR_MAX];
			number (start, game->player.start);
			strcat (buf, "  starting at ");
			strcat (buf, start);
		}

		display_text_box (&render->display, &render->hud_id, font, buf, 
			.5,.85, 1,.1, 1,1,1,1);
	}

}

static void render_minimap (Render* render, Game* game)
{
	float len = cave_len(&game->cave);
	glPushMatrix();
		glScalef(.0065,.003,.001);
		glRotatef(-90,0,1,0);
		glTranslatef(
				-game->player.pos[0]-len*8, // XXX hardcoded
				-game->player.pos[1]-MAX_CAVE_RADIUS*3,
				-game->player.pos[2]-len/2);
		cave_model (render, &game->cave, true);

		glColor4f(1,1,1,0.05);
		glTranslatef (game->player.pos[0],game->player.pos[1],game->player.pos[2]);
		glCallList( render->ship_list );
	glPopMatrix();
}

void render_frame (Render* render, Game* game)
{
	display_start_frame (0,0,0);

	if (game != NULL) {
		float hit = ship_hit(&game->player);
		if(hit < .9) { // avoid drawing the cave from outside
			glPushMatrix();
				render_world_transform (render, &game->player);
				cave_model (render, &game->cave, DISPLAYMODE_NORMAL);
				monolith_model (render, game);
			glPopMatrix();
		}

		if(hit) {
			glDisable (GL_DEPTH_TEST);
			glEnable  (GL_BLEND);
			glDisable (GL_TEXTURE_2D);
			glDisable (GL_LIGHTING);

			glColor4f(1,0,0,hit/2.);
			glBegin (GL_QUADS);
			glVertex3f(-1,-1,-1);
			glVertex3f(+1,-1,-1);
			glVertex3f(+1,+1,-1);
			glVertex3f(-1,+1,-1);
			glEnd();
		}

		ship_model (render, &game->player);
		render_minimap (render, game);
		render_hud (render, game);
	}

	display_text_box (&render->display, &render->msg_id, 
			render->font, render_message_buf, 
			.5,.5,1,.25, 1,1,1,1);

	display_end_frame();
}

// vim600:fdm=syntax:fdn=1:
