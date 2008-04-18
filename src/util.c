/* This file is released in public domain. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char* find_file (const char* basename, const char* paths[])
{
	static char path[FILENAME_MAX];

	for (int i = 0;  paths[i] != NULL;  ++i) {
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

// vim600:fdm=syntax:fdn=1:
