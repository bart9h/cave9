/* This file is released in public domain. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

const char* find_file (const char* basename, const char* paths[], bool required)
{
	static char path[FILENAME_MAX];

	for (int i = 0;  ;  ++i) {
		if (paths[i] == NULL) {
			if (required) {
				fprintf (stderr, "ERROR: Could not find file \"%s\" on any of:\n", basename);
				for (int j = 0;  paths[j] != NULL;  ++j)
					fprintf (stderr, "       %s\n", paths[j]);
				exit(1);
			}
			else break;
		}
		if (strncmp (paths[i], "~/", 2) != 0)
			snprintf (path, FILENAME_MAX, "%s/%s", paths[i], basename);
		else if (getenv("HOME"))
			snprintf (path, FILENAME_MAX, "%s/%s/%s", getenv("HOME"), paths[i]+2, basename);
		else continue;
		if (access (path, R_OK) == 0)
			break;
	}

	return path;
}

const char* roman (unsigned int n)
{
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

#define ROMAN_STR_MAX 80
	static char str[ROMAN_STR_MAX];
	str[0] = '\0';

	for (unsigned int i = 0;  symbols[i].val > 0;  ++i) {
		while (n >= symbols[i].val) {
			assert (strlen(str) + strlen(symbols[i].str) < ROMAN_STR_MAX);
			strcat (str, symbols[i].str);
			n -= symbols[i].val;
		}
	}

	assert (n == 0);

	return str;
}

// vim600:fdm=syntax:fdn=1:
