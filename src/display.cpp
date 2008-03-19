
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

void viewport(Display* display, GLsizei w, GLsizei h, GLsizei bpp, 
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
		glFogf(GL_FOG_END, display->far_plane);
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
	fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
	exit(1);
}

void display_world_transform(Display* display, Ship* player)
{
	COPY(display->cam, player->pos);
	ADD2(display->target, player->pos, player->lookAt);
	//display->target[1]=display->target[1]*.5+player->pos[1]*.5;
	//display->target[2]+=10;
	gluLookAt(
		display->cam[0], display->cam[1], display->cam[2],
		display->target[0], display->target[1], display->target[2],
		0,1,0
	);
}

void cave_model(Display* display, Cave* cave, int wire)
{
	for( int i = 0; i < SEGMENT_COUNT-1; ++i ) {
		int i0 = (cave->i + i)%SEGMENT_COUNT;

		if( (wire ? cave->gl_wire_list : cave->gl_list)[i0] == 0 ) {
			int id = wire ?
				(cave->gl_wire_list[i0] = i0 + display->wire_list_start) :
				(cave->gl_list[i0]      = i0 + display->list_start) ;

			glNewList( id, GL_COMPILE );

			int i1 = (i0 + 1)%SEGMENT_COUNT;
			if(!wire)
				glBindTexture(GL_TEXTURE_2D, display->texture_id);
			glBegin(GL_QUAD_STRIP);
			for( int k = 0; k <= SECTOR_COUNT; ++k ) {

				int k0 = k%SECTOR_COUNT;

				if(!wire) {
					if(i0==0||i1==0||k==3*SECTOR_COUNT/4) 
						glColor4f(1, 0, 0, 0.5); 
					else 
						glColor4f(1, 1, 1, 0.5);
				}

				if(!wire) {
					glTexCoord2f( 
							(float)(cave->i+i)/SEGMENT_COUNT, 
							(float)k/SECTOR_COUNT);
				} else {
					glColor4f(
							(float)i0/SEGMENT_COUNT, 
							1-(float)i0/SEGMENT_COUNT, 
							(float)k0/SECTOR_COUNT, 
							0.5);
				}
				glVertex3fv(cave->segs[i0][k0]);

				if(!wire) {
					glTexCoord2f( 
							((float)(cave->i+i+1))/SEGMENT_COUNT, 
							(float)k/SECTOR_COUNT);
				} else {
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

		if(wire) {
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		} else {
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);
		}

		glCallList( (wire ? cave->gl_wire_list : cave->gl_list)[i0] );
	}

}

void monolith_model(Display* display, Cave* cave, Ship* player)
{
	if(!display->monoliths)
		return;

	glColor3f(.2,.2,.2);

	float w = MONOLITH_WIDTH/2;
	float h = MONOLITH_HEIGHT/2;
	float d = MONOLITH_DEPTH;

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

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

void ship_model(Display* display, Ship* ship)
{
	if(!display->cockpit)
		return;

	if(ship->dist <= 0)
		return;

	float alpha = (1-MIN(1,(ship->pos[2]/MIN_CAVE_RADIUS_DEPTH)))/8.;
	if(alpha == 0)
		return;

	float alert_dist = ship->radius*10;
	float white = ship->dist <= 0 || ship->dist > alert_dist ? 1 : 
		1-(alert_dist - ship->dist)/alert_dist;

	float f =1.8;

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

void render_text(Display* display, GLuint id, const char* text, 
		float x, float y, float w, float h,
		float r, float g, float b)
{
	if(text == NULL || text[0] == '\0')
		return;
	SDL_Color color = {0xff,0xff,0xff,0xff};
	SDL_Surface* label = TTF_RenderText_Blended(display->font, text, color);
	assert(label != NULL);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, id);
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

void display_hud(Display* display, Ship* player)
{
	if(player->dist == FLT_MAX)
		return;

	float max_vel[3] = { MAX_VEL_X, MAX_VEL_Y, MAX_VEL_Z };
	float vel = MIN(1,
			log(1+LEN(player->vel)-MAX_VEL_Z) /
			log(1+LEN(max_vel)-MAX_VEL_Z));
	char gauge[11];
	int i = int(vel*10);
	memset(gauge,'/',i);
	gauge[i] = '\0';

	int score = (int)player->pos[2];

#define HUD_TEXT_MAX 80
	char buf[HUD_TEXT_MAX];
	if(player->dist > 0) {
		snprintf(buf, HUD_TEXT_MAX, "velocity %-10s  score %9d",
			gauge, score
		);
	} else {
		if(score > display->session_score) 
			display->session_score = score;
		if(player->start) {
			snprintf(buf, HUD_TEXT_MAX, "velocity %s  score %d (%d) - %d",
				gauge, score,
				display->session_score, 
				(int)player->start
			);
		} else {
			if(score > display->local_score) {
				display->local_score = score;
				FILE* fp = fopen(SCORE_FILE, "w");
				if(fp == NULL) {
					perror("failed to open score file");
				} else {
					fprintf(fp, "%d", display->local_score);
					fclose(fp);
				}
			}
			if(score > display->global_score) {
				display->global_score = score;
				display_net_update(display);
			}
			snprintf(buf, HUD_TEXT_MAX, "velocity %s  score %d (%d/%d/%d)",
				gauge, score,
				display->session_score, 
				display->local_score, 
				display->global_score
			);
		}
	}

	float white = player->dist <= 0 ? 1 : 1-vel;
	render_text(display, display->hud_id, buf, .5,.9,1,.2, 1,white,white);
}

char display_message_buf[256];
void display_message(Display* display, Cave* cave, Ship* player, const char* buf)
{
	strncpy(display_message_buf, buf, sizeof(display_message_buf)-1);
	display_message_buf[sizeof(display_message_buf)-1] = '\0';
	display_frame(display, cave, player);
}

void display_start_frame(Display* display, float r, float g, float b)
{
	glClearColor(r,g,b,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display_end_frame(Display* display)
{
	glFinish();

	SDL_GL_SwapBuffers();
}

void display_frame(Display* display, Cave* cave, Ship* player)
{
	int hit = player->dist <= SHIP_RADIUS*1.1;

	display_start_frame(display, hit,0,0);

	if(!hit) { // avoid drawing the cave from outside
		glPushMatrix();
			display_world_transform(display, player);
			cave_model(display, cave, 0);
			monolith_model(display, cave, player);
		glPopMatrix();
	}

	ship_model(display, player);
	display_minimap(display, cave, player);
	display_hud(display, player);
	render_text(display, display->msg_id, display_message_buf, .5,.5,1,.25, 1,1,1);

	display_end_frame(display);
}

GLuint display_make_ship_list()
{
	/* Magic Numbers: It is possible to create a dodecahedron by attaching two pentagons 
	 * to each face of a cube. The coordinates of the points are:
	 * (+-x,0, z); (+-1, 1, 1); (0, z, x )
	 * where x = 0.61803398875 and z = 1.61803398875.
	 */

	const double x = 0.61803398875;
	const double z = 1.61803398875;
	const double n1 = 0.525731112119;
	const double n2 = 0.850650808354;

	GLuint ship_list = glGenLists( SEGMENT_COUNT );
	glNewList( ship_list, GL_COMPILE );
		glBegin( GL_LINE_LOOP );
			glNormal3d(  0, n1, n2 );
			glVertex3d(  0,  z,  x );
			glVertex3d( -1,  1,  1 );
			glVertex3d( -x,  0,  z );
			glVertex3d(  x,  0,  z );
			glVertex3d(  1,  1,  1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d(  0, n1, -n2 );
			glVertex3d(  0,  z,  -x );
			glVertex3d(  1,  1,  -1 );
			glVertex3d(  x,  0,  -z );
			glVertex3d( -x,  0,  -z );
			glVertex3d( -1,  1,  -1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d(  0, -n1, n2 );
			glVertex3d(  0,  -z,  x );
			glVertex3d(  1,  -1,  1 );
			glVertex3d(  x,   0,  z );
			glVertex3d( -x,   0,  z );
			glVertex3d( -1,  -1,  1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d(  0, -n1, -n2 );
			glVertex3d(  0,  -z,  -x );
			glVertex3d( -1,  -1,  -1 );
			glVertex3d( -x,   0,  -z );
			glVertex3d(  x,   0,  -z );
			glVertex3d(  1,  -1,  -1 );
		glEnd();

		glBegin( GL_LINE_LOOP );
			glNormal3d( n2,  0, n1 );
			glVertex3d(  x,  0,  z );
			glVertex3d(  1, -1,  1 );
			glVertex3d(  z, -x,  0 );
			glVertex3d(  z,  x,  0 );
			glVertex3d(  1,  1,  1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d( -n2,  0, n1 );
			glVertex3d(  -x,  0,  z );
			glVertex3d(  -1,  1,  1 );
			glVertex3d(  -z,  x,  0 );
			glVertex3d(  -z, -x,  0 );
			glVertex3d(  -1, -1,  1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d( n2,  0, -n1 );
			glVertex3d(  x,  0,  -z );
			glVertex3d(  1,  1,  -1 );
			glVertex3d(  z,  x,   0 );
			glVertex3d(  z, -x,   0 );
			glVertex3d(  1, -1,  -1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d( -n2,  0, -n1 );
			glVertex3d(  -x,  0,  -z );
			glVertex3d(  -1, -1,  -1 );
			glVertex3d(  -z, -x,   0 );
			glVertex3d(  -z,  x,   0 );
			glVertex3d(  -1,  1,  -1 );
		glEnd();

		glBegin( GL_LINE_LOOP );
			glNormal3d( n1, n2,  0 );
			glVertex3d(  z,  x,  0 );
			glVertex3d(  1,  1, -1 );
			glVertex3d(  0,  z, -x );
			glVertex3d(  0,  z,  x );
			glVertex3d(  1,  1,  1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d( n1, -n2,  0 );
			glVertex3d(  z,  -x,  0 );
			glVertex3d(  1,  -1,  1 );
			glVertex3d(  0,  -z,  x );
			glVertex3d(  0,  -z, -x );
			glVertex3d(  1,  -1, -1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d( -n1, n2,  0 );
			glVertex3d(  -z,  x,  0 );
			glVertex3d(  -1,  1,  1 );
			glVertex3d(   0,  z,  x );
			glVertex3d(   0,  z, -x );
			glVertex3d(  -1,  1, -1 );
		glEnd();
		glBegin( GL_LINE_LOOP );
			glNormal3d( -n1, -n2,  0 );
			glVertex3d(  -z,  -x,  0 );
			glVertex3d(  -1,  -1, -1 );
			glVertex3d(   0,  -z, -x );
			glVertex3d(   0,  -z,  x );
			glVertex3d(  -1,  -1,  1 );
		glEnd();

	glEndList();

	return ship_list;
}

void display_init(Display* display, Args* args)
{

	memset(display, 0, sizeof(Display));

	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL_Init(): %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	display->near_plane = MIN(SEGMENT_LEN,SHIP_RADIUS)/4.; // was EPSILON;
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
	display->wire_list_start = glGenLists( SEGMENT_COUNT );

	if(TTF_Init() != 0) {
		fprintf(stderr, "TTF_Init(): %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);

	char* font_filename = FONT_FILE;
	int font_size = args->antialiasing ? 96 : 48;
	display->font = TTF_OpenFont(font_filename, font_size); // FIXME path
	if(display->font == NULL) {
		fprintf(stderr, "TTF_OpenFont(%s): %s\n", font_filename, TTF_GetError());
		exit(1);
	}

    glGenTextures(1, &display->hud_id);
    glBindTexture(GL_TEXTURE_2D, display->hud_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glGenTextures(1, &display->msg_id);
    glBindTexture(GL_TEXTURE_2D, display->msg_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	display_start_frame(display, 0,0,0);
	render_text(display, display->msg_id, "loading cave9", .5,.5,1,.25, 1,1,1);
	display_end_frame(display);

	char* texture_filename = TEXTURE_FILE;

    glGenTextures(1, &display->texture_id);
    glBindTexture(GL_TEXTURE_2D, display->texture_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_Surface* texture = IMG_Load(texture_filename);
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

	display->ship_list = display_make_ship_list();

	display->monoliths = args->monoliths;
	display->cockpit = args->cockpit;

	display->session_score = 0;

	display->local_score = 0;
	FILE* fp = fopen(SCORE_FILE, "r");
	if(fp == NULL) {
		perror("failed to open score file");
	} else {
		fscanf(fp, "%d", &display->local_score);
		fclose(fp);
	}

	display->global_score = 0;

	if(SDLNet_Init()==-1)
	{
		fprintf(stderr, "SDLNet_Init(): %s\n",SDLNet_GetError());
		exit(1);
	}
	atexit(SDLNet_Quit);

	IPaddress addr;
	display->udp_sock = 0;
	display->udp_pkt = NULL;
	if(SDLNet_ResolveHost(&addr,GLOBAL_SCORE_HOST, GLOBAL_SCORE_PORT) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost(): %s\n", SDLNet_GetError());
	} else {
		display->udp_sock=SDLNet_UDP_Open(0);
		if(display->udp_sock == 0) {
			fprintf(stderr, "SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
			display_net_finish(display);
		} else {
			if(SDLNet_UDP_Bind(display->udp_sock, 0, &addr) == -1) {
				fprintf(stderr, "SDLNet_UDP_Bind(): %s\n", SDLNet_GetError());
				display_net_finish(display);
			} else {
				display->udp_pkt = SDLNet_AllocPacket(GLOBAL_SCORE_LEN);
				if(display->udp_pkt == NULL) {
					display_net_finish(display);
				}
			}
		}
	}

}

void display_net_update(Display* display)
{
	if(display->udp_sock == 0)
		return;

	snprintf((char*)display->udp_pkt->data,GLOBAL_SCORE_LEN,"%d",display->global_score);
	display->udp_pkt->len = GLOBAL_SCORE_LEN;
	if(SDLNet_UDP_Send(display->udp_sock,0,display->udp_pkt) == 0) {
		fprintf(stderr, "SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
	} else {
		SDL_Delay(666); // XXX only wait 666ms for hiscores
		if(SDLNet_UDP_Recv(display->udp_sock,display->udp_pkt) == 0) {
			fprintf(stderr, "SDLNet_UDP_Recv(%s,%d): %s\n", 
					GLOBAL_SCORE_HOST, GLOBAL_SCORE_PORT, SDLNet_GetError());
		} else {
			sscanf((char*)display->udp_pkt->data,"%d",&display->global_score);
		}
	}
}

void display_net_finish(Display* display)
{
	if(display->udp_pkt != NULL){ 
		SDLNet_FreePacket(display->udp_pkt);
		display->udp_pkt = NULL;
	}
	if(display->udp_sock != 0) {
		SDLNet_UDP_Close(display->udp_sock);
		display->udp_sock = 0;
	}
}

void display_minimap(Display* display, Cave* cave, Ship* player)
{
	glPushMatrix();
		glScalef(.0065,.003,.001);
		glRotatef(-90,0,1,0);
		glTranslatef(
				-player->pos[0]-1000, // XXX hardcoded
				-player->pos[1]-100,
				-player->pos[2]-(SEGMENT_COUNT-1)*SEGMENT_LEN/2);
		cave_model(display, cave, 1);
	glPopMatrix();

}

// vim600:fdm=syntax:fdn=1:
