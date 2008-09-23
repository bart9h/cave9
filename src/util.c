/* This file is released in public domain. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "util.h"

char bin_path[FILENAME_MAX];
void find_init (const char* argv0)
{
	char *p = strrchr(argv0, '/');
	if(p == NULL) {
		bin_path[0] = '.';
		bin_path[1] = '\0';
	} else {
		int len = p - argv0;
		int max = FILENAME_MAX-1;
		if(len > max) {
			fprintf (stderr, "ERROR: call path (%d) "
				"longer than maximum allowed search path (%d)\n", 
				len, max);
			exit(1);
		}
		memcpy(bin_path, argv0, len);
		bin_path[len] = '\0';
	}
}

const char* find_file (const char* basename, 
		const char* paths[], 
		bool required)
{
	static char path[FILENAME_MAX];

	for (int i = 0; paths[i];  ++i) {

		if (strncmp (paths[i], "/", 1) == 0) { // root
			snprintf (path, FILENAME_MAX, "%s/%s", paths[i], basename);
		}
		else if (strncmp (paths[i], "~/", 2) == 0) { // home
		   	if(getenv("HOME"))
				snprintf (path, FILENAME_MAX, "%s/%s/%s", getenv("HOME"), paths[i]+2, basename);
			else
				continue;
		}
		else if (strncmp (paths[i], ".", 1) == 0) { // cwd
			snprintf (path, FILENAME_MAX, "%s/%s", paths[i], basename);
		}
		else { // bin
			snprintf (path, FILENAME_MAX, "%s/%s/%s", bin_path, paths[i], basename);
		}

		if (access (path, R_OK) == 0)
			return path;
	}

	if (required) {
		fprintf (stderr, "ERROR: Could not find file \"%s\" on any of:\n", basename);
		for (int j = 0;  paths[j] != NULL;  ++j)
			fprintf (stderr, "       %s\n", paths[j]);
		exit(1);
	}

	return NULL;
}

void arabic (char* buf, unsigned int n)
{
	const char thousand_separator = ' ';

	if (snprintf (buf, NUMBER_STR_MAX, "%u", n) >= NUMBER_STR_MAX) {
		fprintf (stderr, "truncated number %u\n", n);
		return;
	}

	int from = strlen (buf);
	int to = from + (from-1)/3;

	while (from != to) {
		from -= 3;
		to -= 3;
		memmove (&buf[to], &buf[from], 3);
		buf[--to] = thousand_separator;
	}
}

void roman (char* buf, unsigned int n)
{
	n /= 10;  // score scale

	static const struct {
		char* str;
		unsigned int val;
	} symbols[] = {
		{ "M",  1000 },
		{ "CM",  900 },
		{ "D",   500 },
		{ "CD",  400 },
		{ "C",   100 },
		{ "XC",   90 },
		{ "L",    50 },
		{ "XL",   40 },
		{ "X",    10 },
		{ "IX",    9 },
		{ "V",     5 },
		{ "IV",    4 },
		{ "I",     1 },
		{ NULL,    0 }
	};

	buf[0] = '\0';

	for (unsigned int i = 0;  symbols[i].val > 0;  ++i) {
		while (n >= symbols[i].val) {
			assert (strlen(buf) + strlen(symbols[i].str) < NUMBER_STR_MAX);
			strcat (buf, symbols[i].str);
			n -= symbols[i].val;
		}
	}

	assert (n == 0);
}

// vim600:fdm=syntax:fdn=1:
