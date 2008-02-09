#ifndef display_h_included
#define display_h_included

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include "game.h"

#define TEXTURE_FILE "cave9.jpg"
#define FONT_FILE "cave9.ttf"
#define SCORE_FILE "cave9.hi"
#define GLOBAL_SCORE_PORT 31559
#define GLOBAL_SCORE_HOST "cave9.9hells.org"
#define GLOBAL_SCORE_LEN 16
#define GLOBAL_SCORE_WAIT 666

typedef struct {
	SDL_Surface* screen;
	GLuint texture_id;
	GLuint hud_id;
	GLuint msg_id;
	TTF_Font* font;
	Vec3 cam, target;
	GLfloat near_plane, far_plane;
	
	GLuint list_start;
	GLuint wire_list_start;
	GLuint ship_list;
	
	int monoliths;
	int cockpit;
	
	int session_score;
	int local_score;
	int global_score;

	UDPsocket udp_sock;
	UDPpacket* udp_pkt;
} Display;

typedef struct {
	int width;
	int height;
	int bpp;
	int fullscreen;
	int highres;
	int antialiasing;
	int monoliths;
	int start;
	int cockpit;
} Args;

void viewport(Display*, GLsizei w, GLsizei h, GLsizei bpp, bool fullscreen, int aa);
void cave_model(Display*, Cave*, int wire);
void ship_model(Display*, Ship*);
void render_hud(Display*, Ship* player);
void display_init(Display*, Args*);
void display_start_frame(Display*, Ship* player);
void display_end_frame(Display*);
void display_minimap(Display*, Cave*, Ship* player);
void display_hud(Display*, Ship* player);
void display_message(Display*, Cave*, Ship* player, const char* buf);
void display_frame(Display*, Cave*, Ship* player);
void display_net_update(Display*);
void display_net_finish(Display*);

#endif

