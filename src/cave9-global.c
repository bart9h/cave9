
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#define GLOBAL_SCORE
#include "score-common.h"

int record(const char *filename, unsigned int score)
{
	FILE *fp = fopen(filename, "w");
	if(fp == NULL) {
		fprintf(stderr,"writing '%s': %s\n", 
				filename, strerror(errno));
		return 1;
	}
	//printf("hiscore %u @ %d\n", score, time(NULL));
	fprintf(fp, "%u\n", score);
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("usage: %s <hiscore-path> [<port=%d>]\n", 
				argv[0], GLOBAL_SCORE_PORT);
		return 1;
	}

	const char *filename = argv[1];
	int port = argc < 3 ? GLOBAL_SCORE_PORT : atoi(argv[2]);

	// read previous hiscore
	unsigned int hiscore = 0;
	FILE *fp = fopen(filename, "r");
	if(fp != NULL) {
		fscanf(fp, "%u", &hiscore);
		fclose(fp);
	} else {
		fprintf(stderr,"reading '%s': %s\n", 
				filename, strerror(errno));
		if(record(filename, hiscore))
			return 1;
	}

	// init udp listening socket
	int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in local;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(port);

	if (bind(sock,(struct sockaddr *)&local, sizeof(struct sockaddr)) < 0) {
		perror("bind");
		return 1;
	}

	for(;;) {

		// receive hiscore request
		struct sockaddr_in remote;
		socklen_t remote_len;
		char buf[GLOBAL_SCORE_LEN];
		int size = recvfrom(sock, (void *)buf, sizeof(buf)-1, 0, 
				(struct sockaddr *)&remote, &remote_len);
		if (size < 0)
			perror("recvfrom");
		else if(size > 0 && size < sizeof(buf)) {
			buf[size] = '\0';
			int score = strtoul(buf, NULL, 10);

			// write if hiscore
			if(score > hiscore) {
				hiscore = score;
				if(record(filename, hiscore))
					return 1;
			}

			// respond hiscore query
			size = snprintf(buf, sizeof(buf), "%u\n", hiscore);
			if(size < 0) {
				perror("snprintf");
			} else if(size >= sizeof(buf)) {
				fprintf(stderr, "message size (%d) too small to write hiscore %u\n",
						sizeof(buf), hiscore);
			} else {
				size = sendto(sock, (void *)buf, sizeof(buf), 0, 
					(struct sockaddr *)&remote, remote_len);
				if(size < 0)
					perror("sendto");
			}
		}

	}

	return 0;
}
