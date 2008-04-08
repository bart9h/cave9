#include "score.h"

void score_net_finish (Score* score)
{
	if(score->udp_pkt != NULL) {
		SDLNet_FreePacket(score->udp_pkt);
		score->udp_pkt = NULL;
	}
	if(score->udp_sock != 0) {
		SDLNet_UDP_Close(score->udp_sock);
		score->udp_sock = 0;
	}
}

void score_net_init (Score* score)
{
	if(SDLNet_Init()==-1)
	{
		fprintf(stderr, "SDLNet_Init(): %s\n",SDLNet_GetError());
		exit(1);
	}
	atexit(SDLNet_Quit);

	IPaddress addr;
	score->udp_sock = 0;
	score->udp_pkt = NULL;
	if(SDLNet_ResolveHost(&addr,GLOBAL_SCORE_HOST, GLOBAL_SCORE_PORT) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost(): %s\n", SDLNet_GetError());
	} else {
		score->udp_sock=SDLNet_UDP_Open(0);
		if(score->udp_sock == 0) {
			fprintf(stderr, "SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
			score_net_finish(score);
		} else {
			if(SDLNet_UDP_Bind(score->udp_sock, 0, &addr) == -1) {
				fprintf(stderr, "SDLNet_UDP_Bind(): %s\n", SDLNet_GetError());
				score_net_finish(score);
			} else {
				score->udp_pkt = SDLNet_AllocPacket(GLOBAL_SCORE_LEN);
				if(score->udp_pkt != NULL) {
					memset (score->udp_pkt->data, 0, GLOBAL_SCORE_LEN);
				}
				else {
					score_net_finish(score);
				}
			}
		}
	}
}

void score_net_update (Score* score)
{
	if(score->udp_sock == 0)
		return;

	snprintf((char*)score->udp_pkt->data,GLOBAL_SCORE_LEN,"%d",score->global_score);
	score->udp_pkt->len = GLOBAL_SCORE_LEN;
	if(SDLNet_UDP_Send(score->udp_sock,0,score->udp_pkt) == 0) {
		fprintf(stderr, "SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
	} else {
		SDL_Delay(GLOBAL_SCORE_WAIT); // XXX only wait 666ms for hiscores
		if(SDLNet_UDP_Recv(score->udp_sock,score->udp_pkt) == 0) {
			fprintf(stderr, "SDLNet_UDP_Recv(%s,%d): %s\n",
					GLOBAL_SCORE_HOST, GLOBAL_SCORE_PORT, SDLNet_GetError());
		} else {
			sscanf((char*)score->udp_pkt->data,"%d",&score->global_score);
		}
	}
}

void score_init (Score* score)
{
	score->local_score = 0;
	score->session_score = 0;
	score->global_score = 0;

	FILE* fp = fopen(SCORE_FILE, "r");
	if (fp == NULL) {
		perror ("failed to open score file");
	} else {
		fscanf (fp, "%d", &score->local_score);
		fclose (fp);
	}

	score_net_init (score);
}

void score_finish (Score* score)
{
	score_net_finish (score);
}

