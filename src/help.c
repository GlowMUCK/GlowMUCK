#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include "color.h"
#include "db.h"
#include "tune.h"
#include "mpi.h"
#include "props.h"
#include "interface.h"
#include "externs.h"

/* commands for giving help */

/*
 * Ok, directory stuff IS a bit ugly.
 */
#if defined(DIRENT) || defined(_POSIX_VERSION)
# include <dirent.h>
# define NLENGTH(dirent) (strlen((dirent)->d_name))
#else /* not (DIRENT or _POSIX_VERSION) */
# define dirent direct
# define NLENGTH(dirent) ((dirent)->d_namlen)
# ifdef SYSNDIR
#  include <sys/ndir.h>
# endif /* SYSNDIR */
# ifdef SYSDIR
#  include <sys/dir.h>
# endif /* SYSDIR */
# ifdef NDIR
#  include <ndir.h>
# endif /* NDIR */
#endif /* not (DIRENT or _POSIX_VERSION) */

#if defined(DIRENT) || defined(_POSIX_VERSION) || defined(SYSNDIR) || defined(SYSDIR) || defined(NDIR)
# define DIR_AVALIBLE
#endif

int
spit_file_segment(dbref player, const char *filename, const char *seg, int linenums)
{
    FILE   *f;
    char    buf[BUFFER_LEN];
    char    buf2[BUFFER_LEN];
    char    segbuf[BUFFER_LEN];
    char   *p;
    int     startline, endline, currline, lines;

    startline = endline = currline = lines = 0;
    if (seg && *seg) {
	strcpy(segbuf, seg);
	for (p = segbuf; isdigit(*p); p++);
	if (*p) {
	    *p++ = '\0';
	    startline = atoi(segbuf);
	    while (*p && !isdigit(*p)) p++;
	    if (*p) endline = atoi(p);
	} else {
	    endline = startline = atoi(segbuf);
	}
    }
    if ((f = fopen(filename, "rb")) == NULL) {
	sprintf(buf, CINFO "%s is missing.  Management has been notified.",
		filename);
	anotify(player, buf);
	fputs("spit_file:", stderr);
	perror(filename);
    } else {
	while (fgets(buf, sizeof buf, f)) {
	    for (p = buf; *p; p++)
		if (*p == '\n') {
		    *p = '\0';
		    break;
		}
	    currline++;
	    if ((!startline || (currline >= startline)) &&
		    (!endline || (currline <= endline))) {
		lines++;
		if (*buf) {
		    if(linenums) {
			notify_fmt(player, "%2d: %s", currline, buf);
		    } else if (tp_parse_help_as_mpi) {
			do_parse_mesg(player, 0, buf, "FILE", buf2, 1);
			notify(player, buf2);
		    } else {
			notify(player, buf);
		    }
		} else {
		    notify(player, "  ");
		}
	    }
	}
	fclose(f);
    }
    return lines;
}

void
remove_file_line( const char *filename, int line )
{
  FILE *of;
  FILE *nf;
  char tmpname[BUFFER_LEN];
  char *p;
  int currline = 0;
  char buf[BUFFER_LEN];

  sprintf(tmpname, "%s.tmp", filename);
  
  if ((of = fopen(filename, "rb"))) {
   nf = fopen(tmpname, "w");
    while(fgets(buf, sizeof buf, of)) {
      for (p = buf; *p; p++)
	if (*p == '\n') {
	  *p = '\0';
	  break;
	}
      currline++;
      if (currline != line) {
	fprintf(nf, buf);
      }
    }
    fclose(nf);
    fclose(of);
  }
  unlink(filename);
  rename(tmpname, filename);
}


void 
get_file_line( const char *filename, char *retbuf, int line )
{
    FILE   *f;
    char    buf[BUFFER_LEN];
    char   *p;
    int     currline=0;

    retbuf[0]='\0';
    if ((f = fopen(filename, "rb"))) {
	while (fgets(buf, sizeof buf, f)) {
	    for (p = buf; *p; p++)
		if (*p == '\n') {
		    *p = '\0';
		    break;
		}
	    currline++;
	    if (currline == line){
		if (*buf) {
		    strcpy( retbuf, buf );
		    break;
		}
	    }
	}
	fclose(f);
    }
}

void 
spit_file(dbref player, const char *filename)
{
    spit_file_segment(player, filename, "", 0);
}


void 
index_file(dbref player, const char *onwhat, const char *file, int look)
{
    FILE   *f;
    char    buf[BUFFER_LEN];
    char    topic[BUFFER_LEN];
    char   *p;
    unsigned int     arglen;
    unsigned int found;
    unsigned int     len;

    if(Mage(OWNER(player)) && (onwhat[0] == '!')) {
	onwhat++;
	look = 0; /* No muf/action looking */
    }

    if(tp_db_help_first && look && can_move(player, onwhat, tp_help_mlevel + 1)) {
	do_look_at(player, onwhat, "");
	return;
    }

    *topic = '\0';
    strcpy(topic, onwhat);
    if (*onwhat) {
	strcat(topic, "|");
    }

    if ((f = fopen(file, "rb")) == NULL) {
	sprintf(buf, CINFO
		"%s is missing.  Management has been notified.", file);
	anotify(player, buf);
	fprintf(stderr, "help: No file %s!\n", file);
    } else {
	arglen = strlen(topic);
	if (*topic && (arglen > 1)) {
	    do {
		do {
		    if (!(fgets(buf, sizeof buf, f))) {
			if((!tp_db_help_first) && look && can_move(player, onwhat, tp_help_mlevel + 1)) {
			    do_look_at(player, onwhat, "");
			} else {
			    sprintf(buf, CINFO "There is no help for \"%s\"",
				onwhat);
			    anotify(player, buf);
			}
			fclose(f);
			return;
		    }
		} while (*buf != '~');
		do {
		    if (!(fgets(buf, sizeof buf, f))) {
			if(look && can_move(player, onwhat, tp_help_mlevel + 1)) {
			    do_look_at(player, onwhat, "");
			} else {
			    sprintf(buf, CINFO "There is no help for \"%s\"",
				onwhat);
			    anotify(player, buf);
			    if(tp_log_failed_commands)
				log_status(
				    "NHLP: %s(%d) in %s(%d): %s\n",
				    NAME(player), player,
				    NAME(DBFETCH(player)->location),
				    DBFETCH(player)->location,
				    onwhat
				);
			}
			fclose(f);
			return;
		    }
		} while (*buf == '~');
		p = buf;
		found = 0;
		len = 0;
		while((buf[len] >= ' ') && (len < ((sizeof buf) - 2)))
		    len++;
		buf[len++] = '|' ;
		buf[len  ] = '\0';
		while (*p && !found) {
		    if (strncasecmp(p, topic, arglen)) {
			while (*p && (*p != '|')) p++;
			if (*p) p++;
		    } else {
			found = 1;
		    }
		}
	    } while (!found);
	}
	while (fgets(buf, sizeof buf, f)) {
	    if (*buf == '~')
		break;
	    for (p = buf; *p; p++)
		if (*p == '\n') {
		    *p = '\0';
		    break;
		}
	    if (*buf) {
		notify(player, buf);
	    } else {
		notify(player, "  ");
	    }
	}
	fclose(f);
    }
}


int 
show_subfile(dbref player, const char *dir, const char *topic, const char *seg,
	     int partial)
{
    char   buf[256];
    const char *am;
    struct stat st;
#ifdef DIR_AVALIBLE
    DIR		*df;
    struct dirent *dp;
#endif 

    if (!topic || !*topic) return 0;

    if ((*topic == '.') || (*topic == '~') || (index(topic, '/'))) {
	return 0;
    }
    if (strlen(topic) > 63) return 0;


#ifdef DIR_AVALIBLE
    /* TO DO: (1) exact match, or (2) partial match, but unique */
    *buf = 0;

    if ((df = (DIR *) opendir(dir)))
    {
	while ((dp = readdir(df)))
	{
	    if ((partial  && string_prefix(dp->d_name, topic)) ||
		(!partial && !string_compare(dp->d_name, topic))
		)
	    {
		sprintf(buf, "%s/%s", dir, dp->d_name);
		break;
	    }
	}
	closedir(df);
    }
    
    if (!*buf)
    {
	return 0; /* no such file or directory */
    }
#else /* !DIR_AVALIBLE */
    sprintf(buf, "%s/%s", dir, topic);
#endif /* !DIR_AVALIBLE */

    if (stat(buf, &st)) {
	return 0;
    } else {
	if(spit_file_segment(player, buf, seg, 0) >= 30) {
	    am = get_property_class(player, PROP_MORE);
#ifdef COMPRESS
	    if(am && *am) am = uncompress(am);
#endif
	    if(am && *am) { /* has 'more' prompt on */
		notify(player,
"** If you turned the more prompt on just to read this, type 'more off' now." 
		);
	    } else {
		if(player != tp_www_user) {
		    notify(player,
"** If this just spammed past you, try doing 'more 20' to use the more pager."
		    );
		    notify(player,
"   If pressing Return doesn't go to the next page, just type 'more'."
		    );
		}
	    }
	}
	return 1;
    }
}


void 
do_man(dbref player, const char *topic, const char *seg)
{
    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (show_subfile(player, MAN_DIR, topic, seg, FALSE)) return;
    index_file(player, topic, MAN_FILE, 0);
}


void 
do_mpihelp(dbref player, const char *topic, const char *seg)
{
    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (show_subfile(player, MPI_DIR, topic, seg, FALSE)) return;
    index_file(player, topic, MPI_FILE, 0);
}


void 
do_help(dbref player, const char *topic, const char *seg)
{
    if (show_subfile(player, HELP_DIR, topic, seg, FALSE)) return;
    index_file(player, topic, HELP_FILE, 1);
}


void 
do_sysparm(dbref player, const char *topic)
{
    if(Guest(player))
	return;

    index_file(player, topic, SYSPARM_HELP_FILE, 0);
}


void 
do_news(dbref player, const char *topic, const char *seg)
{
    if (show_subfile(player, NEWS_DIR, topic, seg, FALSE)) return;
    index_file(player, topic, NEWS_FILE, 0);
}


void 
add_motd_text_fmt(const char *text)
{
    char    buf[80];
    const char *p = text;
    int     count = 0; /* Was 2 for double indent */

    /* buf[0] = buf[1] = ' '; */
    while (*p) {
	while (*p && (count < 68))
	    buf[count++] = *p++;
	while (*p && !isspace(*p) && (count < 76))
	    buf[count++] = *p++;
	buf[count] = '\0';
	log2file(MOTD_FILE, "  %s", buf);
	while (*p && isspace(*p))
	    p++;
	count = 0;
    }
}


void 
do_motd(dbref player, const char *text)
{
    const char *am;
    char buf[BUFFER_LEN];
    time_t  lt;

    if (!Guest(OWNER(player)) && !string_compare(text, "pause")) {
	am = get_property_class(player, PROP_MOTDPAUSE);
#ifdef COMPRESS
	if(am && *am) am = uncompress(am);
#endif
	if(am && *am) { /* Has prop, clear it */
	    remove_property(player, PROP_MOTDPAUSE);
	    anotify(player, CSUCC "MOTD pause enabled.");
	} else { /* Add prop */
	    add_property(player, PROP_MOTDPAUSE, "yes", 0);
	    anotify(player, CSUCC "MOTD pause disabled.");
	    clear_prompt(player);
	}
	return;
    }
    if (!*text || !Mage(OWNER(player))) {
	spit_file(player, MOTD_FILE);
	return;
    }
    if (!string_compare(text, "clear")) {
	if(unlink(MOTD_FILE))
	    perror(MOTD_FILE);
	log2file(MOTD_FILE, MARK "%s's Messages of the Day . . .\n ",
		tp_muckname);	
	anotify(player, CSUCC "MOTD cleared.");
	return;
    }
    lt = current_systime;
    if(tp_date_motd && tp_author_motd) {
	sprintf(buf, "%s - %s, %.16s", text, NAME(player), ctime(&lt));
	add_motd_text_fmt(buf);
    } else if(tp_author_motd) {
	sprintf(buf, "%s - %s", text, NAME(player));
	add_motd_text_fmt(buf);
    } else if(tp_date_motd) {
	sprintf(buf, "%s - %.16s", text, ctime(&lt));
	add_motd_text_fmt(buf);
    } else {
	add_motd_text_fmt(text);
    }
    log2file(MOTD_FILE, " ", buf);
    anotify(player, CSUCC "MOTD updated.");
}


void 
do_note(dbref player, const char *text)
{
    char buf[BUFFER_LEN];
    time_t  lt;

    if ((player>0)&&(!*text || !Mage(OWNER(player)))) {
	spit_file(player, NOTE_FILE);
	anotify(player, CINFO "Done.");
	return;
    }
    if (!string_compare(text, "clear")) {
	if(unlink(NOTE_FILE))
	    perror(NOTE_FILE);
	log2file(NOTE_FILE, "Character Request Information\n ");
	if(player>0) anotify(player, CSUCC "Note file cleared.");
	return;
    }
    lt = current_systime;
    sprintf(buf, "%.16s: %s", ctime(&lt), text);
    log2file(NOTE_FILE, buf);
    if(player>0) anotify(player, CSUCC "Character request notes updated.");
}


void 
do_info(dbref player, const char *topic, const char *seg)
{
#ifdef DIR_AVALIBLE
    DIR		*df;
    struct dirent *dp;
#endif
    int		f;
    int		cols;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (*topic) {
	if (!show_subfile(player, INFO_DIR, topic, seg, TRUE))
	{
	    anotify(player, CINFO NO_INFO_MSG);
	}
    } else {
#ifdef DIR_AVALIBLE
	char buf[BUFFER_LEN];
	/* buf = (char *) calloc(1, 80); */
	(void) strcpy(buf, "    ");
	f = 0;
	cols = 0;
	if ((df = (DIR *) opendir(INFO_DIR))) 
	{
	    while ((dp = readdir(df)))
	    {
		
		if (*(dp->d_name) != '.') 
		{
		    if (!f)
			anotify(player, CINFO "Available information files are:");
		    if ((cols++ > 2) || 
			((strlen(buf) + strlen(dp->d_name)) > 63)) 
		    {
			notify(player, buf);
			(void) strcpy(buf, "    ");
			cols = 0;
		    }
		    (void) strcat(strcat(buf, dp->d_name), " ");
		    f = strlen(buf);
		    while ((f % 20) != 4)
			buf[f++] = ' ';
		    buf[f] = '\0';
		}
	    }
	    closedir(df);
	}
	if (f)
	    notify(player, buf);
	else
	    anotify(player, CINFO "No information files are available.");
	/* free(buf); */
#else /* !DIR_AVALIBLE */
	anotify(player, CINFO "Type 'info index' for a list of files.");
#endif /* !DIR_AVALIBLE */
    }
}
