#include "copyright.h"
#include "config.h"

#include <stdio.h>

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef alloc_string
#undef string_dup

extern const char * old_uncompress(const char *s);

char *in_filename;
FILE *infile;

char *
string_dup(const char *s)
{
    char *p;
    p = (char *)malloc(strlen(s) + 1);
    if (!p) return p;
    strcpy(p, s);
    return p;
}

int 
main(int argc, char **argv)
{
    char    buf[16384];

    if (argc > 2) {
	fprintf(stderr, "Usage: %s [infile]\n", argv[0]);
	return 1;
    }

    if (argc < 2) {
	infile = stdin;
    } else {
	in_filename = (char *)string_dup(argv[1]);
	if ((infile = fopen(in_filename, "rb")) == NULL) {
	    fprintf(stderr, "%s: unable to open input file.\n", argv[0]);
	    return 1;
	}
    }

    while (fgets(buf, sizeof(buf), infile)) {
	buf[sizeof(buf) - 1] = '\0';
	fputs((char *)old_uncompress(buf), stdout);
    }
    exit(0);
    return 0;
}
