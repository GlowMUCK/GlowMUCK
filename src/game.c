#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#ifndef WIN95
# include <sys/wait.h>
#endif

#include "color.h"
#include "db.h"
#include "props.h"
#include "tune.h"
#include "interface.h"
#include "match.h"
#include "msgparse.h"
#include "strings.h"
#include "externs.h"

/* declarations */
static const char *dumpfile = 0;
static int epoch = 0;
static int dumpcount = 0;
FILE   *input_file;
FILE   *delta_infile;
FILE   *delta_outfile;
char   *in_filename;

void 
do_dump(dbref player, const char *newfile)
{
    char    buf[BUFFER_LEN];

    if (Mage(player)) {
	if (*newfile && Man(player)) {
	    if (dumpfile) free( (void *)dumpfile );
	    dumpfile = alloc_string(newfile);
	    sprintf(buf, CINFO "Dumping to file %s...", dumpfile);
	} else {
	    sprintf(buf, CINFO "Dumping to file %s...", dumpfile);
	}
	anotify(player, buf);
	dump_db_now();
	anotify(player, CINFO "Done.");
    } else {
	anotify(player, CFAIL NOPERM_MESG);
    }
}

#ifdef DELTADUMPS
void 
do_delta(dbref player)
{
    if (Mage(player)) {
	anotify(player, CINFO "Dumping deltas...");
	delta_dump_now();
	anotify(player, CINFO "Done.");
    } else
	anotify(player, CFAIL NOPERM_MESG);
}
#endif

void 
do_shutdown(dbref player, const char *msg)
{
    if (Wiz(player)) {
      if( *msg == '\0' || strcmp(msg, tp_muckname))
      {
	notify(player, "Usage: @shutdown muckname" );
    	return;
      }
	log_status("SHUT: by %s\n", unparse_object(player, player));
	shutdown_flag = 1;
	restart_flag = 0;
    } else {
	anotify(player, CFAIL NOPERM_MESG);
	log_status("SHAM: Shutdown by %s\n", unparse_object(player, player));
    }
}

void 
do_restart(dbref player, const char *msg)
{
    if (Mage(player)) {
      if( *msg == '\0' || strcmp(msg, tp_muckname))
      {
	notify(player, "Usage: @restart muckname" );
    	return;
      }
	log_status("REST: by %s\n", unparse_object(player, player));
	shutdown_flag = 1;
	restart_flag = 1;
    } else {
	anotify(player, CFAIL NOPERM_MESG);
	log_status("SHAM: Restart by %s\n", unparse_object(player, player));
    }
}


#ifdef DISKBASE
extern int propcache_hits;
extern int propcache_misses;
#endif

static void 
dump_database_internal(void)
{
    char    tmpfile[512];
    FILE   *f;
    int     copies, decay, dcount;

    if (tp_dbdump_warning)
	wall_and_flush(tp_dumping_mesg);

    /* nuke our predecessor - seems better to leave it */
/*  sprintf(tmpfile, "%s.#%d#", dumpfile, epoch - 1);
    if(unlink(tmpfile))
	perror(tmpfile);
*/

    sprintf(tmpfile, "%s.#%d#", dumpfile, epoch);

    if ((f = fopen(tmpfile, "wb")) != NULL) {
	db_write(f);
	fclose(f);
#ifdef DISKBASE
# ifdef FLUSHCHANGED
       fclose(input_file);
# endif
#endif

	if(tp_dump_copies > 0) {
	    char fromfile[512], destfile[512];

	    /* Rename the old db instead of deleting it, move the set up */
	    copies = (tp_dump_copies <= MAX_DUMP_COPIES) ? tp_dump_copies : MAX_DUMP_COPIES;

	    /* Calculate exponential decay limit */
	    /* db.new   -> db.new.1 every time */
	    /* db.new.1 -> db.new.2 every other */
	    /* db.new.2 -> db.new.3 every fourth */
	    /* db.new.3 -> db.new.4 every eighth, etc */
	    if(tp_dump_copies_decay) {
		if(dumpcount <= 0) {
		    copies = 1; /* Only update one copy first dump */
		} else {
		    dcount = dumpcount;
		    for(decay = 1; decay < copies; decay++) {
			if(dcount & 1) {
			    copies = decay;
			    break;
			}
			dcount >>= 1;
		    }
		}
	    }

	    for(; copies > 1; copies--) {
		sprintf(fromfile, "%s.%d", dumpfile, copies - 1);
		sprintf(destfile, "%s.%d", dumpfile, copies);
#ifdef WIN95
		unlink(destfile);
#endif
		rename(fromfile, destfile);
	    }
	    sprintf(destfile, "%s.1", dumpfile);
#ifdef WIN95
	    unlink(destfile); /* glow.new.1 */
#endif
	    rename(dumpfile, destfile); /* glow.new -> glow.new.1 */

	} else {
#ifdef WIN95
	    /* Windows 95 rename() differs from Unix rename() */
	    /* Unix overwrites existing files, Win95 will not */
	    if (unlink(dumpfile))
		perror(dumpfile);
#endif
	}

	if (rename(tmpfile, dumpfile) < 0)
	    perror(tmpfile);

#ifdef DISKBASE

#ifdef FLUSHCHANGED
	free((void *)in_filename);
	in_filename = string_dup(dumpfile);
	if ((input_file = fopen(in_filename, "rb")) == NULL)
	    perror(dumpfile);

#ifdef DELTADUMPS
	fclose(delta_outfile);
	if ((delta_outfile = fopen(DELTAFILE_NAME, "wb")) == NULL)
	    perror(DELTAFILE_NAME);

	fclose(delta_infile);
	if ((delta_infile = fopen(DELTAFILE_NAME, "rb")) == NULL)
	    perror(DELTAFILE_NAME);
#endif
#endif

#endif

    } else {
	perror(tmpfile);
    }

    /* Write out the macros */

    /* nuke the previous macro file -- why? it's not there unless a crash */
/*  sprintf(tmpfile, "%s.#%d#", MACRO_FILE, epoch - 1);
    if(unlink(tmpfile))
	perror(tmpfile);
*/

    sprintf(tmpfile, "%s.#%d#", MACRO_FILE, epoch);

    if ((f = fopen(tmpfile, "wb")) != NULL) {
	macrodump(macrotop, f);
	fclose(f);
#ifdef WIN95
/* Windows 95 rename() differs from Unix rename() */
/* Unix overwrites existing files, Win95 will not */
	if (unlink(MACRO_FILE))
	    perror(MACRO_FILE);
#endif
	if (rename(tmpfile, MACRO_FILE) < 0)
	    perror(tmpfile);
    } else {
	perror(tmpfile);
    }

    if (tp_dbdump_warning)
	wall_and_flush(tp_dumpdone_mesg);
#ifdef DISKBASE
    propcache_hits = 0L;
    propcache_misses = 1L;
#endif

    if (tp_periodic_program_purge)
	free_unused_programs();
#ifdef DISKBASE
    dispose_all_oldprops();
#endif

    dumpcount++;
}

void 
panic(const char *message)
{
    char    panicfile[2048];
    FILE   *f;

    log_status("PANIC: %s\n", message);
    fprintf(stderr, "PANIC: %s\n", message);

    /* shut down interface */
    emergency_shutdown();

    /* dump panic file */
    sprintf(panicfile, "%s.PANIC", dumpfile);
    if ((f = fopen(panicfile, "wb")) == NULL) {
	perror("CANNOT OPEN PANIC FILE, YOU LOSE");

#ifdef NOCOREDUMP
	_exit(135);
#else				/* !NOCOREDUMP */
#ifdef SIGIOT
	signal(SIGIOT, SIG_DFL);
#endif
	abort();
#endif				/* NOCOREDUMP */
    } else {
	log_status("DUMP: %s\n", panicfile);
	fprintf(stderr, "DUMP: %s\n", panicfile);
	db_write(f);
	fclose(f);
	log_status("DUMP: %s (done)\n", panicfile);
	fprintf(stderr, "DUMP: %s (done)\n", panicfile);
	if(unlink(DELTAFILE_NAME))
	    perror(DELTAFILE_NAME);
    }

    /* Write out the macros */
    sprintf(panicfile, "%s.PANIC", MACRO_FILE);
    if ((f = fopen(panicfile, "wb")) != NULL) {
	macrodump(macrotop, f);
	fclose(f);
    } else {
	perror("CANNOT OPEN MACRO PANIC FILE, YOU LOSE");
#ifdef NOCOREDUMP
	_exit(135);
#else				/* !NOCOREDUMP */
# ifdef SIGIOT
	signal(SIGIOT, SIG_DFL);
# endif
	abort();
#endif				/* NOCOREDUMP */
    }

#ifdef NOCOREDUMP
    _exit(136);
#else				/* !NOCOREDUMP */
#ifdef SIGIOT
    signal(SIGIOT, SIG_DFL);
#endif
    abort();
#endif				/* NOCOREDUMP */
}

void 
dump_database(void)
{
    epoch++;

    log_status("DUMP: %s.#%d# (%d)\n", dumpfile, epoch, dumpcount);
    dump_database_internal();
    log_status("DUMP: %s.#%d# (done)\n", dumpfile, epoch);
}

time_t last_monolithic_time = 0;

/*
 * Named "fork_and_dump()" mostly for historical reasons...
 */
void fork_and_dump(void)
{
    epoch++;

    if(tp_save_glow_flags)
	do_glowflags(NOTHING, "save", "all");

    last_monolithic_time = current_systime;
    log_status("DUMP: %s.#%d# (%d)\n", dumpfile, epoch, dumpcount);

    dump_database_internal();
    time(&current_systime);
    host_save();
}

#ifdef DELTADUMPS
extern int deltas_count;

int
time_for_monolithic(void)
{
    dbref i;
    int count = 0;
    int a, b;

    if (!last_monolithic_time)
	last_monolithic_time = current_systime;
    if (current_systime - last_monolithic_time >=
	    (tp_monolithic_interval - tp_dump_warntime)
    ) {
	return 1;
    }

    for (i = 0; i < db_top; i++)
	if (FLAGS(i) & (SAVED_DELTA | OBJECT_CHANGED)) count++;
    if (((count * 100) / db_top)  > tp_max_delta_objs) {
	return 1;
    }

    fseek(delta_infile, 0L, 2);
    a = ftell(delta_infile);
    fseek(input_file, 0L, 2);
    b = ftell(input_file);
    if (a >= b) {
	return 1;
    }
    return 0;
}
#endif

void
dump_warning(void)
{
    if (tp_dbdump_warning) {
#ifdef DELTADUMPS
	if (time_for_monolithic()) {
	    wall_and_flush(tp_dumpwarn_mesg);
	} else {
	    if (tp_deltadump_warning) {
		wall_and_flush(tp_deltawarn_mesg);
	    }
	}
#else
	wall_and_flush(tp_dumpwarn_mesg);
#endif
    }
}

#ifdef DELTADUMPS
void
dump_deltas(void)
{
    if (time_for_monolithic()) {
	fork_and_dump();
	deltas_count = 0;
	return;
    }

    epoch++;
    log_status("DELT: %s.#%d#\n", dumpfile, epoch);

    if (tp_deltadump_warning)
	wall_and_flush(tp_dumpdeltas_mesg);

    db_write_deltas(delta_outfile);

    if (tp_deltadump_warning)
	wall_and_flush(tp_dumpdone_mesg);
#ifdef DISKBASE
    propcache_hits = 0L;
    propcache_misses = 1L;
#endif
    host_save();
}
#endif

extern short db_conversion_flag;

int 
init_game(const char *infile, const char *outfile)
{
    FILE   *f;

    if ((f = fopen(MACRO_FILE, "rb")) == NULL)
	log_status("INIT: Macro storage file %s is tweaked.\n", MACRO_FILE);
    else {
	macroload(f);
	fclose(f);
    }

    in_filename = (char *)string_dup(infile);
    if ((input_file = fopen(infile, "rb")) == NULL)
	return -1;

#ifdef DELTADUMPS
    if ((delta_outfile = fopen(DELTAFILE_NAME, "wb")) == NULL)
	return -1;

    if ((delta_infile = fopen(DELTAFILE_NAME, "rb")) == NULL)
	return -1;
#endif

    db_free();
    init_primitives();		 /* init muf compiler */
#ifdef MPI
    mesg_init();		       /* init mpi interpreter */
#endif
    SRANDOM(getpid());		 /* init random number generator */
    tune_load_parmsfile(NOTHING);      /* load @tune parms from file */

    /* ok, read the db in */
    log_status("LOAD: %s\n", infile);
    fprintf(stderr, "LOAD: %s\n", infile);
    if (db_read(input_file) < 0)
	return -1;
    log_status("LOAD: %s (done)\n", infile);
    fprintf(stderr, "LOAD: %s (done)\n", infile);

#ifndef DISKBASE
    /* everything ok */
    fclose(input_file);
#endif

    /* set up dumper */
    if (dumpfile)
	free((void *) dumpfile);
    dumpfile = alloc_string(outfile);

    if (!db_conversion_flag) {
	/* initialize the ~sys/startuptime property */
	add_property((dbref)0, "~sys/startuptime", NULL,
		     (int)time((time_t *) NULL));
	add_property((dbref)0, "~sys/maxpennies", NULL, tp_max_pennies);
	add_property((dbref)0, "~sys/dumpinterval", NULL, tp_dump_interval);
	add_property((dbref)0, "~sys/max_connects", NULL, 0);
    }

    return 0;
}


extern short wizonly_mode;
void
do_restrict(dbref player, const char *arg)
{
    if (!Arch(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (!strcmp(arg, "on")) {
	wizonly_mode = 1;
	anotify(player, CSUCC "Login access is now restricted to " NAMEWIZ "s only.");
    } else if (!strcmp(arg, "off")) {
	wizonly_mode = 0;
	anotify(player, CSUCC "Login access is now unrestricted.");
    } else {
        anotify_fmt(player, CNOTE "Wizard-only connection mode is currently %s.",
	    wizonly_mode ? "on" : "off"
	);
    }
}


extern short localhost_mode;
void
do_localhost(dbref player, const char *arg)
{
    if (!Arch(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (!strcmp(arg, "on")) {
	localhost_mode = 1;
	anotify(player, CSUCC "Login access is now restricted to localhost.");
    } else if (!strcmp(arg, "off")) {
	localhost_mode = 0;
	anotify(player, CSUCC "Login access is now unrestricted.");
    } else {
        anotify_fmt(player, CNOTE "Localhost connection mode is currently %s.",
	    localhost_mode ? "on" : "off"
	);
    }
}


/* use this only in process_command */
#define Matched(string) { if(!string_prefix((string), command)) goto bad; }


int
try_alias(dbref player, const char *command)
{
    const char *format, *arr[10], *marr[10], *args;
    char cbuf[BUFFER_LEN], *p, nbuf[BUFFER_LEN];
    int i = 0, c, len;

    if(!tp_alias_support)
	return 0;
    
    /* process_command() does player and *command error checking */
    if(!*command) return 0;

    /* Stupidity-proof the system */
    if(!string_compare(command, "alias")) return 0;

    c = strlen(PROP_ALIASDIR "/");
    strcpy(cbuf, PROP_ALIASDIR "/");

    while(command[i] && !isspace(command[i]))
	cbuf[c++] = command[i++];
    cbuf[c] = '\0';

    /* Args points to first char after first space after command */
    if(command[i]) args = command + i + 1; else args = "";

    format = get_property_class(player, cbuf);
#ifdef COMPRESS
    if(format && *format) format = uncompress(format);
#endif /* COMPRESS */

    if(tp_global_aliases && ((!format) || (!*format))) {
	/* Try for global alias on environment room, usually #0 */
	format = get_property_class(EnvRoom, cbuf);
#ifdef COMPRESS
	if(format && *format) format = uncompress(format);
#endif /* COMPRESS */
    }

    if((!format) || (!*format))
	return 0;

    /* We have a format string, parse $0-$9,$*,$$ */
    strcpy(cbuf, command);

    p = cbuf;
    while(isspace(*p)) p++;
    for(i = 0, p = cbuf; i < 10; i++) {
	if(*p) {
	    arr[i] = p;
	    while(*p && (!isspace(*p))) p++;
	    if(*p) *(p++) = '\0';
	    while(isspace(*p)) p++;
	    marr[i] = (p - cbuf) + command;
	} else marr[i] = arr[i] = "";
    }

    c = i = 0;
    while(format[i] != '\0') {
	if(format[i] == '$') {
	    if(format[++i] == '*') {
		if(((len = strlen(args)) + c) >= MAX_COMMAND_LEN) break;
		strcpy(nbuf + c, args);
		c += len; i++;
	    } else if(format[i] == '-') {
		if((format[++i] >= '0') && (format[i] <= '9')) {
		    if(((len = strlen(marr[format[i]-'0'])) + c) >= MAX_COMMAND_LEN) break;
		    strcpy(nbuf + c, marr[format[i]-'0']);
		    c += len; i++;
		} else nbuf[c++] = format[i++];
	    } else if((format[i] >= '0') && (format[i] <= '9')) {
		if(((len = strlen(arr[format[i]-'0'])) + c) >= MAX_COMMAND_LEN) break;
		strcpy(nbuf + c, arr[format[i]-'0']);
		c += len; i++;
	    } else nbuf[c++] = format[i++];
	} else nbuf[c++] = format[i++];
    }
    nbuf[c] = '\0';
    if(!nbuf[0]) return 0;

    alias_level++;
    process_command(player, nbuf, 1);
    alias_level--;
    return 1;
}

int force_level = 0;
int alias_level = 0;

const char *
split_after_equal(char *command)
{
    char *pname;

    pname = command;
    while(*pname && (*pname != '='))
	pname++;
    if(*pname == '=') {
	*(pname++) = '\0';
	while(*pname == ' ')
	    pname++;
	return pname;
    }
    return "";
}

void 
process_command(dbref player, const char *command, int alias)
{
    char   *arg1;
    char   *arg2;
    const char *full_command, *commandline=command;
    char   *p;			/* utility */
    char    pbuf[BUFFER_LEN];
    char    xbuf[BUFFER_LEN];
    char    ybuf[BUFFER_LEN];
    dbref   loc;

    if (command == 0) abort();

    /* robustify player */
    if ((!OkObj(player)) ||
	    (Typeof(player) != TYPE_PLAYER && Typeof(player) != TYPE_THING)) {
	log_status("process_command: bad player %d\n", player);
	return;
    }

    loc = DBFETCH(player)->location;

    if (    (tp_log_commands
#ifdef MUD
	    || (MUDDER(player) && tp_log_mud_commands)
#endif
	    || (tp_log_guests && Guest(OWNER(player)))
	    || (tp_log_wizards && Mage(OWNER(player)))
	    || (FLAG2(player) & F2SUSPECT)
	) && ((tp_log_interactive) ||
	    (!( FLAGS(player) & (INTERACTIVE | READMODE) ))
	)
    ) {
	if(tp_log_with_names && OkObj(player) && OkObj(getloc(player))) {
	    log_command("%s(%d)->%s(%d)@%s(%d):%s%s%s\n",
		NAME(OWNER(player)), (int) OWNER(player),
		NAME(player), (int) player,
		NAME(getloc(player)), (int) getloc(player),
		Mage(OWNER(player)) ? " [WIZ]" :
		  ( Guest(OWNER(player)) ? " [GST]" :
		    ( (FLAG2(player) & F2SUSPECT) ? " [SUS]" :
#ifdef MUD
		    ( MUDDER(player) ? " [MUD]" : "")
#else
		    ""
#endif
		  ) ) ,
		alias ? " (alias) " :
		((FLAGS(player) & INTERACTIVE) ? "*" : " "),
		command
	    );
	} else {
	    log_command("%6d->%6d@%6d:%s%s%s\n",
		(int) OWNER(player),
		(int) player,
		(int) getloc(player),
		Mage(OWNER(player)) ? " [WIZ]" :
		  ( Guest(OWNER(player)) ? " [GST]" :
		    ( (FLAG2(player) & F2SUSPECT) ? " [SUS]" :
#ifdef MUD
		    ( MUDDER(player) ? " [MUD]" : "")
#else
		    ""
#endif
		  ) ) ,
		alias ? " (alias) " :
		((FLAGS(player) & INTERACTIVE) ? "*" : " "),
		command
	    );
	}
    }

    if (FLAGS(player) & INTERACTIVE) {
	interactive(player, command);
	return;
    }
    /* eat leading whitespace */
    while (*command && isspace(*command))
	command++;

    /* check for single-character commands */
    if (*command == SAY_TOKEN || *command == SAY_TOKEN2) {
	sprintf(pbuf, SAY_COMMAND " %s", command + 1);
	command = &pbuf[0];
    } else if (*command == POSE_TOKEN || *command == POSE_TOKEN2) {
	sprintf(pbuf, POSE_COMMAND " %s", command + 1);
	command = &pbuf[0];
    } else if (tp_dash_tokens && (*command == DASH_TOKEN)) {
	sprintf(pbuf, DASH_COMMAND " %s", command + 1);
	command = &pbuf[0];
    } else if (tp_chat_tokens && (*command == CHAT_TOKEN)) {
	sprintf(pbuf, CHAT_COMMAND " %s", command + 1);
	command = &pbuf[0];
    } else if (tp_chat_tokens && (*command == CHATPOSE_TOKEN)) {
	sprintf(pbuf, CHATPOSE_COMMAND " %s", command + 1);
	command = &pbuf[0];
    }

    /* if player is a wizard, and uses overide token to start line...*/
    /* ... then do NOT run actions, but run the command they specify. */

    if (!( *command == OVERIDE_TOKEN && TMage(player) )) {
	if((!alias) && (!alias_level) && try_alias(player, command))
	    return;

#ifdef PATH
	if(!can_move(player, command, tp_path_mlevel + 1)) {
	    dbref who, whonext, pathdest;
	    char pathname[BUFFER_LEN];
	    int fullmatch = 0;

	    pathdest = match_path(DBFETCH(player)->location, command, pathname, &fullmatch);
	    if(OkObj(pathdest)) { /* path found */

		move_path(player, pathname, command, fullmatch);

		/* lead/follow loop for paths */
		if(Typeof(pathdest) == TYPE_ROOM) {
		    who = DBFETCH(loc)->contents;
		    while(who != NOTHING) {
			 whonext = DBFETCH(who)->next;
			if( (getleader(who) == player) &&
			    (DBFETCH(who)->location == loc)
			) {
			    anotify_fmt(who, CCYAN "You follow %s.", NAME(player));
			    move_path(who, pathname, command, fullmatch);
			}
			who = whonext;
		    }
		}
		return;
	    }
	}
#endif /* PATH */

	if( can_move(player, command, 0) ) {
	    do_move(player, command, 0); /* command is exact match for exit */
	    *match_args = 0;
	    *match_cmdname = 0;	
	    return;
	}
    }

	if (*command == OVERIDE_TOKEN && TMage(player))
	    command++;

	full_command = strcpy(xbuf, command);
	for (; *full_command && !isspace(*full_command); full_command++);
	if (*full_command) full_command++;

	/* find arg1 -- move over command word */
	command = strcpy(ybuf, command);
	for (arg1 = (char *) command; *arg1 && !isspace(*arg1); arg1++);
	/* truncate command */
	if (*arg1) *arg1++ = '\0';

	/* move over spaces */
	while (*arg1 && isspace(*arg1)) arg1++;

	/* find end of arg1, start of arg2 */
	for (arg2 = arg1; *arg2 && *arg2 != ARG_DELIMITER; arg2++);

	/* truncate arg1 */
	for (p = arg2 - 1; p >= arg1 && isspace(*p); p--) *p = '\0';

	/* go past delimiter if present */
	if (*arg2) *arg2++ = '\0';
	while (*arg2 && isspace(*arg2)) arg2++;

	/* remember command for programs, why is this done twice in FB? */
	strcpy(match_cmdname, command);
	strcpy(match_args, full_command);

	switch (command[0]) {
	    case '@':
		switch (command[1]) {
		    case 'a':
		    case 'A':
			/* @action, @attach */
			switch (command[2]) {
			    case 'c':
			    case 'C':
				Matched("@action");
				do_action(player, arg1, arg2);
				break;
			    case 'l':
			    case 'L':
				Matched("@alias");
				do_alias(player, arg1, arg2);
				break;
			    case 'r':
			    case 'R':
				if (string_compare(command, "@armageddon"))
				    goto bad;
				do_armageddon(player, full_command);
				break;
			    case 't':
			    case 'T':
				Matched("@attach");
				do_attach(player, arg1, arg2);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'b':
		    case 'B':
			Matched("@bootme");
			do_boot(player, arg1);
			break;
		    case 'c':
		    case 'C':
			/* chown, contents, create */
			switch (command[2]) {
			    case 'h':
			    case 'H':
				switch (command[3]) {
				    case 'e':
				    case 'E':
					Matched("@check");
					do_check(player, full_command);
					break;
				    case 'l':
				    case 'L':
					Matched("@chlock");
					do_chlock(player, arg1, arg2);
					break;
				    case 'o':
				    case 'O':
					Matched("@chown");
					do_chown(player, arg1, arg2);
					break;
				    default:
					goto bad;
				}
				break;
			    case 'o':
			    case 'O':
			      if(command[3] == 'l' || command[3] == 'L') {
				    Matched("@colorset");
				    do_colorset(player, arg1, arg2);
				    break;
			      } else {
				switch (command[4]) {
				    case 'l':
				    case 'L':
					Matched("@conlock");
					do_conlock(player, arg1, arg2);
					break;
				    case 't':
				    case 'T':
					Matched("@contents");
					do_contents(player, arg1, arg2);
					break;
				    default:
					goto bad;
				}
			      }
			      break;
			    case 'r':
			    case 'R':
				if (string_compare(command, "@credits")) {
				    Matched("@create");
				    do_create(player, arg1, arg2);
				} else {
				    do_credits(player);
				}
				break;
			    default:
				goto bad;
			}
			break;
		    case 'd':
		    case 'D':
			/* describe, dequeue, dig, or dump */
			switch (command[2]) {
			    case 'b':
			    case 'B':
				Matched("@dbginfo");
				do_serverdebug(player, arg1, arg2);
				break;
			    case 'e':
			    case 'E':
#ifdef DELTADUMPS
				if(command[3] == 'l' || command[3] == 'L') {
				    Matched("@delta");
				    do_delta(player);
				} else
#endif /* DELTADUMPS */
				{
				    Matched("@describe");
				    do_describe(player, arg1, arg2);
				}
				break;
			    case 'i':
			    case 'I':
				Matched("@dig");
				do_dig(player, arg1, arg2);
				break;
#ifdef DELTADUMPS
			    case 'l':
			    case 'L':
				Matched("@dlt");
				do_delta(player);
				break;
#endif /* DELTADUMPS */
			    case 'o':
			    case 'O':
			        if(command[3] == 's' || command[3] == 'S') {
				    Matched("@dosinfo");
				    do_dos(player, arg1, arg2);
			        } else {
				    Matched("@doing");
				    if (!tp_who_doing) goto bad;
				    do_doing(player, arg1, arg2);
				}
				break;
			    case 'r':
			    case 'R':
				Matched("@drop");
				do_drop_message(player, arg1, arg2);
				break;
			    case 'u':
			    case 'U':
				Matched("@dump");
				do_dump(player, full_command);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'e':
		    case 'E':
			switch (command[2]) {
			    case 'd':
			    case 'D':
				Matched("@edit");
				do_edit(player, arg1);
				break;
			    case 'n':
			    case 'N':
				Matched("@entrances");
				do_entrances(player, arg1, arg2);
				break;
			    case 'x':
			    case 'X':
				Matched("@examine");
				sane_dump_object(player, arg1);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'f':
		    case 'F':
			/* fail, find, force, or frob */
			switch (command[2]) {
			    case 'a':
			    case 'A':
				Matched("@fail");
				do_fail(player, arg1, arg2);
				break;
			    case 'i':
			    case 'I':
				if(command[3] == 'x' || command[3] == 'X') {
				    Matched("@fixwizbits");
				    do_fixw(player, full_command);
				} else {
				    Matched("@find");
				    do_find(player, arg1, arg2);
				}
				break;
			    case 'l':
			    case 'L':
				Matched("@flock");
				do_flock(player, arg1, arg2);
				break;
			    case 'o':
			    case 'O':
				Matched("@force");
				do_force(player, arg1, arg2);
				break;
			    case 'r':
			    case 'R':
				if (!string_compare(command, "@frob")) {
				    do_frob(player, arg1, arg2, 1);
				} else {
				    Matched("@free");
				    do_jail(player, full_command, 1);
				}
				break;
			    default:
				goto bad;
			}
			break;
		    case 'g':
		    case 'G':
			switch (command[2]) {
			    case 'a':
			    case 'A':
				Matched("@gag");
				do_gag(player, full_command);
				break;
			    case 'e':
			    case 'E':
				if(command[3] == 't' || command[3] == 'T') {
				    if (string_compare(command, "@getpw"))
					goto bad;
				    do_getpw(player, full_command);
				} else {
				    Matched("@gender");
				    do_sex(player, arg1, arg2);
				}
			    	break;
			    case 'l':
			    case 'L':
				Matched("@glowflags");
				do_glowflags(player, arg1, arg2);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'h':
		    case 'H':
			switch (command[2]) {
#ifdef PATH
			    case 'i':
			    case 'I':
			    	Matched("@hide");
			    	do_path_setprop(player, arg1, "d", "yes");
			    	break;
#endif /* PATH */
			    case 'o':
			    case 'O':
				if(command[3] == 's' || command[3] == 'S') {
				    Matched("@hostcache");
				    do_hostcache(player, arg1, arg2);
				} else {
				    Matched("@hopper");
				    do_hopper(player, arg1);
				}
			    	break;
			    default:
				goto bad;
			}
			break;
		    case 'i':
		    case 'I':
			switch (command[2]) {
			    case 'd':
			    case 'D':
				Matched("@idescribe");
				do_idescribe(player, arg1, arg2);
				break;
			    case 'g':
			    case 'G':
				Matched("@ignore");
				do_gag(player, full_command);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'j':
		    case 'J':
			switch (command[2]) {
			    case 'a':
			    case 'A':
				Matched("@jail");
				do_jail(player, full_command, 0);
				break;
#ifdef PATH
			    case 'u':
			    case 'U':
				Matched("@junk");
				do_path_junk(player, full_command);
				break;
#endif /* PATH */
			    default:
				goto bad;
			}
			break;
		    case 'k':
		    case 'K':
			Matched("@kill");
			do_dequeue(player, arg1);
			break;
		    case 'l':
		    case 'L':
			/* lock or link */
			switch (command[2]) {
			    case 'i':
			    case 'I':
				switch (command[3]) {
				    case 'n':
				    case 'N':
					if (string_compare(command, "@link")) {
					    Matched("@linewrap");
					    do_linewrap(player, full_command);
					} else {
					    do_link(player, arg1, arg2);
					}
					break;
				    case 's':
				    case 'S':
					Matched("@list");
					match_and_list(player, arg1, arg2);
					break;
				    default:
					goto bad;
				}
				break;
			    case 'o':
			    case 'O':
			        if(command[3]) {
				    switch (command[4]) {
					case 'a':
					case 'A':
					    Matched("@localhost");
					    do_localhost(player, arg1);
					    break;
					case 'k':
					case 'K':
					    Matched("@lock");
					    do_lock(player, arg1, arg2);
					    break;
					default:
					    goto bad;
				    }
				} else goto bad;
				break;
			    default:
				goto bad;
			}
			break;
		    case 'm':
		    case 'M':
		        switch (command[2]) {
			    case 'e':
			    case 'E':
			        Matched("@memory");
			        do_memory(player);
			        break;
			    case 'p':
			    case 'P':
			        Matched("@mpitops");
			        do_mpi_topprofs(player, arg1);
			        break;
			    case 'u':
			    case 'U':
			        Matched("@muftops");
			        do_muf_topprofs(player, arg1);
			        break;
			    default:
			        goto bad;
			}
			break;
		    case 'n':
		    case 'N':
			/* @name or @newpassword */
			switch (command[2]) {
			    case 'a':
			    case 'A':
				Matched("@name");
				do_name(player, arg1, arg2);
				break;
			    case 'e':
			    case 'E':
				if (string_compare(command, "@newpassword"))
				    goto bad;
				do_newpassword(player, arg1, arg2);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'o':
		    case 'O':
			switch (command[2]) {
			    case 'd':
			    case 'D':
				Matched("@odrop");
				do_odrop(player, arg1, arg2);
				break;
			    case 'e':
			    case 'E':
				Matched("@oecho");
				do_oecho(player, arg1, arg2);
				break;
			    case 'f':
			    case 'F':
				Matched("@ofail");
				do_ofail(player, arg1, arg2);
				break;
#ifdef MUD
			    case 'n':
			    case 'N':
				Matched("@oname");
				do_oname(player, arg1, arg2);
				break;
#endif /* MUD */
			    case 'p':
			    case 'P':
				Matched("@open");
				do_open(player, arg1, arg2);
				break;
			    case 's':
			    case 'S':
				Matched("@osuccess");
				do_osuccess(player, arg1, arg2);
				break;
			    case 'w':
			    case 'W':
				Matched("@owned");
				do_owned(player, arg1, arg2);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'p':
		    case 'P':
			switch (command[2]) {
			    case 'a':
			    case 'A':
#ifdef PATH
				if (string_prefix("@path", command)) {
				    Matched("@path");
				    do_path(player, arg1, arg2, split_after_equal(arg2));
				} else
#endif /* PATH */
				{
				    Matched("@password");
				    do_password(player, arg1, arg2);
				}
				break;
			    case 'c':
			    case 'C':
				Matched("@pcreate");
				do_pcreate(player, arg1, arg2);
				break;
			    case 'e':
			    case 'E':
				if (string_prefix("@pecho", command)) {
				    Matched("@pecho");
				    do_pecho(player, arg1, arg2);
				} else {
				    Matched("@permissions");
				    do_check(player, full_command);
				}
				break;
			    case 'o':
			    case 'O':
				Matched("@position");
				do_position(player, arg1, arg2);
				break;
			    case 'r':
			    case 'R':
				if (string_prefix("@program", command)) {
				    Matched("@program");
				    do_prog(player, arg1);
				} else if (string_prefix("@proginfo", command)) {
				    Matched("@proginfo");
				    do_proginfo(player, arg1);
				} else {
				    Matched("@propset");
				    do_propset(player, arg1, arg2);
				}
				break;
			    case 's':
			    case 'S':
				Matched("@ps");
				list_events(player);
				break;
			    case 'u':
			    case 'U':
				Matched("@purge");
				do_purge(player, arg1, arg2);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'q':
		    case 'Q':
			Matched("@quota");
			do_quota(player, arg1);
			break;
		    case 'r':
		    case 'R':
			switch (command[3]) {
			    case 'c':
			    case 'C':
				Matched("@recycle");
				do_recycle(player, arg1);
				break;
#ifdef PATH
			    case 'l':
			    case 'L':
				Matched("@relink");
				do_path(player, arg1, arg2, "");
				break;
#endif /* PATH */
			    case 's':
			    case 'S':
				if (!strcmp(command, "@restart")) {
				    do_restart(player, full_command);
				} else if (!strcmp(command, "@restrict")) {
				    do_restrict(player, arg1);
				} else {
				    goto bad;
				}
				break;
			    default:
				goto bad;
			}
			break;
		    case 's':
		    case 'S':
			/* set, shutdown, success */
			switch (command[2]) {
			    case 'a':
			    case 'A':
				if (!strcmp(command, "@sanity")) {
				    sanity(player);
				} else if (!strcmp(command, "@sanchange")) {
				    sanechange(player, full_command);
				} else if (!strcmp(command, "@sanfix")) {
				    sanfix(player);
				} else {
				    goto bad;
				}
				break;
			    case 'e':
			    case 'E':
				if (!string_compare(command, "@sex")) {
				    do_sex(player, arg1, arg2);
				    break;
				}
				if(strlen(command) <= 4) {
				    Matched("@set");
				    do_set(player, arg1, arg2);
				} else {
				    if(command[4] == 'c' || command[4] == 'C') {
					Matched("@setcontents");
					do_setcontents(player, arg1, arg2);
				    } else {
					Matched("@setquota");
					do_setquota(player, arg1, arg2);
				    }
				}
				break;
			    case 'h':
			    case 'H':
				if (!string_compare(command, "@shout")) {
				    do_wall(player, full_command);
				} else
#ifdef PATH
				if (!string_compare(command, "@show")) {
				    do_path_setprop(player, arg1, "d", "");
				} else
#endif /* PATH */
				if (!string_compare(command, "@shutdown")) {
				    do_shutdown(player, arg1);
				} else {
				    goto bad;
				}
				break;
			    case 'o':
			    case 'O':
				if (string_compare(command, "@soul"))
				    goto bad;

				do_frob(player, arg1, arg2, 1);
				break;
			    case 'p':
			    case 'P':
				Matched("@species");
				do_species(player, arg1, arg2);
				break;
			    case 't':
			    case 'T':
				Matched("@stats");
				do_stats(player, arg1);
				break;
			    case 'u':
			    case 'U':
				Matched("@success");
				do_success(player, arg1, arg2);
				break;
			    case 'w':
			    case 'W':
				Matched("@sweep");
				do_sweep(player, arg1);
				break;
			    default:
				goto bad;
			}
			break;
		    case 't':
		    case 'T':
			switch (command[2]) {
			    case 'e':
			    case 'E':
				Matched("@teleport");
				do_teleport(player, arg1, arg2);
				break;
#ifdef MUD
			    case 'i':
			    case 'I':
				Matched("@title");
				do_oname(player, arg1, arg2);
				break;
#endif /* MUD */
			    case 'o':
			    case 'O':
				if (!string_compare(command, "@toad")) {
				    do_frob(player, arg1, arg2, 0);
				} else {
				    Matched("@tops");
				    do_all_topprofs(player, arg1);
				}
				break;
			    case 'r':
			    case 'R':
				Matched("@trace");
				do_trace(player, arg1, atoi(arg2));
				break;
			    case 'u':
			    case 'U':
				Matched("@tune");
				do_tune(player, arg1, arg2);
				break;
			    default:
				goto bad;
			}
			break;
		    case 'u':
		    case 'U':
			switch (command[2]) {
			    case 'n':
			    case 'N':
				switch (command[3]) {
				    case 'c':
				    case 'C':
					    Matched("@uncompile");
					    do_uncompile(player);
					    break;
				    case 'g':
				    case 'G':
					    Matched("@ungag");
					    do_ungag(player, full_command);
					    break;
				    case 'i':
				    case 'I':
					    Matched("@unignore");
					    do_ungag(player, full_command);
					    break;
				    case 'l':
				    case 'L':
					if (string_prefix(command, "@unli")) {
					    Matched("@unlink");
					    do_unlink(player, arg1);
					} else {
					    Matched("@unlock");
					    do_unlock(player, arg1);
					}
					break;
				    default:
					goto bad;
				}
				break;
#ifndef NO_USAGE_COMMAND
			    case 's':
			    case 'S':
				Matched("@usage");
				do_usage(player);
				break;
#endif /* NO_USAGE_COMMAND */
			    default:
				goto bad;
			}
			break;
		    case 'v':
		    case 'V':
			Matched("@version");
			anotify(player, CRED VERSION CWHITE " -- " CAQUA GLOWVER );
			anotify_fmt(player, CINFO "Options: %s", servopts);
			break;
		    case 'w':
		    case 'W':
			switch (command[2]) {
			    case 'a':
			    case 'A':
				if(string_compare(command, "@wall"))
				    goto bad;
				do_wall(player, full_command);
				break;
			    case 'r':
			    case 'R':
				Matched("@wrap");
				do_linewrap(player, arg1);
				break;
			    default:
				Matched("@when");
				do_examine(player, arg1, arg2, 1);
			}
			break;
		    default:
			goto bad;
		}
		break;
	    case 'a':
	    case 'A':
		switch (command[1]) {
		    case 't':
		    case 'T':
			Matched("attack");
			do_kill(player, arg1, atoi(arg2));
			break;
		    default:
			Matched("alias");
			do_alias(player, arg1, arg2);
		}
		break;
	    case 'c':
	    case 'C':
		switch (command[1]) {
#ifdef MUD
		    case 'a':
		    case 'A':
			Matched("cast");
			do_cast(player, arg1, arg2);
			break;
#endif /* MUD */

		    case 'o':
		    case 'O':
			Matched("color");
			do_set(player, "me",
			    ((arg1[0] != 'y') && (arg1[0] != 'Y') && strcasecmp(arg1, "on"))
			    ? "!C"
			    : "C"
			);
			break;

		    default:
			Matched("chat");
			if(full_command[0] == ':')
			    do_pose(player, full_command + 1);
			else
			    do_say(player, full_command);
		}
		break;
	    case 'd':
	    case 'D':
		switch (command[1]) {
		    case 'b':
		    case 'B':
			Matched("dboot");
			do_dboot(player, arg1);
			break;
		    case 'f':
		    case 'F':
			Matched("dforce");
			do_dforce(player, arg1, arg2);
			break;
		    case 'i':
		    case 'I':
			Matched("dinfo");
			do_dinfo(player, arg1);
			break;
		    case 'w':
		    case 'W':
			Matched("dwall");
			do_dwall(player, arg1, arg2);
			break;
		    default:
			Matched("drop");
			do_drop(player, arg1, arg2);
		}
		break;
	    case 'e':
	    case 'E':
		switch (command[1]) {
		    case 'm':
		    case 'M':
			Matched("emote");
			do_pose(player, full_command);
			break;
		    default:
			Matched("examine");
			do_examine(player, arg1, arg2, 0);
		}
		break;
	    case 'f':
	    case 'F':
		Matched("follow");
		do_follow(player, full_command);
		break;
	    case 'g':
	    case 'G':
		/* get, give, go, or gripe */
		switch (command[1]) {
		    case 'i':
		    case 'I':
			Matched("give");
			do_give(player, arg1, atoi(arg2));
			break;
		    case 'o':
		    case 'O':
			Matched("goto");
			do_move(player, arg1, 0);
			break;
		    case 'r':
		    case 'R':
			if (string_compare(command, "gripe"))
			    goto bad;
			do_gripe(player, full_command);
			break;
		    default:
			Matched("get");
			do_get(player, arg1, arg2);
		}
		break;
	    case 'h':
	    case 'H':
		switch (command[1]) {
		    case 'a':
		    case 'A':
			Matched("hand");
			do_drop(player, arg1, arg2);
			break;
		    default:
			Matched("help");
			do_help(player, arg1, arg2);
		}
		break;
	    case 'i':
	    case 'I':
		switch (command[1]) {
#ifdef ROLEPLAY
		    case 'c':
		    case 'C':
			Matched("ic");
			do_set(player, *arg1 ? arg1 : "me", "ic");
			break;
#endif /* ROLEPLAY */
		    default:
			if (string_compare(command, "info")) {
			    Matched("inventory");
			    do_inventory(player);
			} else {
			    Matched("info");
			    do_info(player, arg1, arg2);
			}
		}
		break;
	    case 'k':
	    case 'K':
		Matched("kill");
		do_kill(player, arg1, atoi(arg2));
		break;
	    case 'l':
	    case 'L':
		switch (command[1]) {
		    case 'e':
		    case 'E':
			if (string_prefix("lead", command)) {
			    Matched("lead");
			    do_lead(player, full_command, 0);
			    break;
			} else {
			    Matched("leave");
			    do_leave(player);
			}
			break;
		    case 'o':
		    case 'O':
		    default:
			Matched("look");
			do_look_at(player, arg1, arg2);
		}
		break;
	    case 'm':
	    case 'M':
		switch (command[1]) {
		    case 'a':
		    case 'A':
			Matched("man");
			do_man(player, arg1, arg2);
			break;
		    case 'o':
		    case 'O':
			switch (command[2]) {
			    case 'v':
			    case 'V':
				Matched("move");
				do_move(player, arg1, 0);
				break;
			    case 'r':
			    case 'R':
				Matched("more");
				do_more(player, full_command);
				break;
			    default:
				Matched("motd");
				do_motd(player, full_command);
			}
			break;
		    default:
			Matched("mpi");
			do_mpihelp(player, arg1, arg2);
		}
		break;
	    case 'n':
	    case 'N':
		if(command[1] == 'e' || command[1] == 'E'){
		    Matched("news");
		    do_news(player, arg1, arg2);
		} else {
		    Matched("note");
		    do_note(player, full_command);
		}
		break;
	    case 'o':
	    case 'O':
		switch (command[1]) {
#ifdef MUD
		    case 'f':
		    case 'F':
			/* offer */
			Matched("offer");
			do_offer(player, full_command);
			break;
#endif /* MUD */
#ifdef ROLEPLAY
		    case 'o':
		    case 'O':
			Matched("ooc");
			do_set(player, *arg1 ? arg1 : "me", "!ic");
			break;
#endif /* ROLEPLAY */
		    default:
		    	goto bad;
		}
		break;
	    case 'p':
	    case 'P':
		switch (command[1]) {
		    case 'o':
		    case 'O':
			Matched("pose");
			do_pose(player, full_command);
			break;
		    case 'u':
		    case 'U':
			Matched("put");
			do_drop(player, arg1, arg2);
			break;
		    default:
			Matched("page");
			do_page(player, arg1, arg2);
		}
		break;
	    case 'r':
	    case 'R':
		switch (command[1]) {
		    case 'e':
		    case 'E':
			switch (command[2]) {
			    case 'a':
			    case 'A':
				Matched("read");	/* undocumented alias for look */
				do_look_at(player, arg1, arg2);
				break;
			    case 'q':
			    case 'Q':
				Matched("request");
				request(player, NULL, commandline);
				break;
			    default:
				Matched("release");
				do_lead(player, full_command, 1);
			}
			break;
		    default:
			Matched("rob");
			do_rob(player, arg1);
		}
		break;
	    case 's':
	    case 'S':
		switch (command[1]) {
		    case 'c':
		    case 'C':
			if( command[2] == 'a' || command[2] == 'A' ) {
			    Matched("scan");
			    do_sweep(player, arg1);
			} else {
			    Matched("score");
			    do_score(player, 1);
			}
			break;
		    default:
			Matched("say");
			do_say(player, full_command);
		}
		break;
	    case 't':
	    case 'T':
		switch (command[1]) {
		    case 'h':
		    case 'H':
			Matched("throw");
			do_drop(player, arg1, arg2);
			break;
		    default:
			Matched("take");
			do_get(player, arg1, arg2);
		}
		break;
	    case 'w':
	    case 'W':
		switch (command[1]){
		    case 'c':
		    case 'C':
		    	Matched("wc");
		    	do_wizchat(player, full_command);
		    	break;
		    case 'i':
		    case 'I':
		    	Matched("wizchat");
		    	do_wizchat(player, arg1);
		    	break;
		    default:
			Matched("whisper");
			do_whisper(player, arg1, arg2);
		}
		break;
	    default:
	bad:
		anotify_fmt(player, CINFO "%s", tp_huh_mesg);
		if (tp_log_failed_commands &&
			!controls(player, DBFETCH(player)->location)) {
		    log_status(" HUH: %s(%d) in %s(%d)[%s]: %s %s\n",
		       NAME(player), player, NAME(DBFETCH(player)->location),
			       DBFETCH(player)->location,
			     NAME(OWNER(DBFETCH(player)->location)), command,
			       full_command);
		}
		break;
	}

}

#undef Matched


