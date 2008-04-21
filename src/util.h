/* This file is released in public domain. */

/* warning: returns a fixed buffer, not reentrant */
/* paths[] must be NULL-terminated */
const char* find_file (const char* basename, const char* paths[], bool required);

