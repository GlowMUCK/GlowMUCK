/*
 * Copyright (c) 1990 Chelsea Dyerman
 * University of California, Berkeley (XCF)
 *
 */

#include "config.h"
#include "local.h"

#include <stdio.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>

#include "strings.h"
#include "interface.h"
#include "externs.h"

/* cks: these are varargs routines. We are assuming ANSI C. We could at least
   USE ANSI C varargs features, no? Sigh. */

void 
log2file(char *myfilename, char *format,...)
{
    va_list args;
    FILE   *fp;

    va_start(args, format);

    if ((fp = fopen(myfilename, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", myfilename);
	vfprintf(stderr, format, args);
    } else {
	vfprintf(fp, format, args);
	fprintf(fp, "\n");
	fclose(fp);
    }
    va_end(args);
}

void 
log2filetime(char *myfilename, char *format,...)
{
    char wall[BUFFER_LEN];
    va_list args;
    FILE   *fp;
    time_t  lt;
    char buf[40];

    lt = current_systime;
    va_start(args, format);

    *buf = '\0';
    if ((fp = fopen(myfilename, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", LOG_STATUS);
	fprintf(stderr, "%.16s: ", ctime(&lt));
	vsprintf(wall, format, args);
	fprintf(stderr, "%s", wall);
    } else {
	format_time(buf, 32, "%c\0", localtime(&lt));
	fprintf(fp, "%.32s: ", buf);
	vsprintf(wall, format, args);
	fprintf(fp, "%s", wall);
	fclose(fp);
    }
    va_end(args);
}

void 
log_status(char *format,...)
{
    char wall[BUFFER_LEN];
    va_list args;
    FILE   *fp;
    time_t  lt;
    char buf[40];

    lt = current_systime;
    va_start(args, format);

    *buf = '\0';
    if ((fp = fopen(LOG_STATUS, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", LOG_STATUS);
	fprintf(stderr, "%.16s: ", ctime(&lt));
	vsprintf(wall, format, args);
	fprintf(stderr, "%s", wall);
    } else {
	format_time(buf, 32, "%c\0", localtime(&lt));
	fprintf(fp, "%.32s: ", buf);
	vsprintf(wall, format, args);
	fprintf(fp, "%s", wall);
	fclose(fp);
    }
    va_end(args);
    wall_status(wall);
}

#ifdef HTTPD
void 
log_http(char *format,...)
{
    char wall[BUFFER_LEN];
    va_list args;
    FILE   *fp;
    time_t  lt;
    char buf[40];

    lt = current_systime;
    va_start(args, format);

    *buf = '\0';
    if ((fp = fopen(LOG_HTTP, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", LOG_STATUS);
	fprintf(stderr, "%.16s: ", ctime(&lt));
	vsprintf(wall, format, args);
	fprintf(stderr, "%s", wall);
    } else {
	format_time(buf, 32, "%c\0", localtime(&lt));
	fprintf(fp, "%.32s: ", buf);
	vsprintf(wall, format, args);
	fprintf(fp, "%s", wall);
	fclose(fp);
    }
    va_end(args);
}
#endif /* HTTPD */

void 
log_conc(char *format,...)
{
    va_list args;
    FILE   *conclog;

    va_start(args, format);
    if ((conclog = fopen(LOG_CONC, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", LOG_CONC);
	vfprintf(stderr, format, args);
    } else {
	vfprintf(conclog, format, args);
	fclose(conclog);
    }
    va_end(args);
}

void 
log_muf(char *format,...)
{
    va_list args;
    FILE   *muflog;

    va_start(args, format);
    if ((muflog = fopen(LOG_MUF, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", LOG_MUF);
	vfprintf(stderr, format, args);
    } else {
	vfprintf(muflog, format, args);
	fclose(muflog);
    }
    va_end(args);
}

void 
log_gripe(char *format,...)
{
    va_list args;
    FILE   *fp;
    time_t  lt;
    char buf[40];

    va_start(args, format);
    lt = current_systime;

    *buf = '\0';
    if ((fp = fopen(LOG_GRIPE, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", LOG_GRIPE);
	fprintf(stderr, "%.16s: ", ctime(&lt));
	vfprintf(stderr, format, args);
    } else {
	format_time(buf, 32, "%c\0", localtime(&lt));
	fprintf(fp, "%.32s: ", buf);
	vfprintf(fp, format, args);
	fclose(fp);
    }
    va_end(args);
}

void 
log_command(char *format,...)
{
    va_list args;
    char buf[40];
    FILE   *fp;
    time_t  lt;

    va_start(args, format);
    lt = current_systime;

    *buf = '\0';
    if ((fp = fopen(COMMAND_LOG, "ab")) == NULL) {
	fprintf(stderr, "Unable to open %s!\n", COMMAND_LOG);
	vfprintf(stderr, format, args);
    } else {
	format_time(buf, 32, "%c\0", localtime(&lt));
	fprintf(fp, "%.32s: ", buf);
	vfprintf(fp, format, args);
	fclose(fp);
    }
    va_end(args);
}

void 
notify_fmt(dbref player, char *format,...)
{
    va_list args;
    char    bufr[BUFFER_LEN];

    va_start(args, format);
    vsprintf(bufr, format, args);
    notify(player, bufr);
    va_end(args);
}

void 
anotify_fmt(dbref player, char *format,...)
{
    va_list args;
    char    bufr[BUFFER_LEN];

    va_start(args, format);
    vsprintf(bufr, format, args);
    anotify(player, bufr);
    va_end(args);
}


