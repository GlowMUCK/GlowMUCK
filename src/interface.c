#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <sys/types.h>
#ifdef WIN95
# include <fcntl.h>
# include <ctype.h>
# include <string.h>
#else
# include <sys/file.h>
# include <sys/ioctl.h>
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
# include <fcntl.h>
# include <sys/errno.h>
# include <ctype.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
#endif

#include "strings.h"
#include "color.h"
#include "db.h"
#include "interface.h"
#include "tune.h"
#include "props.h"
#include "match.h"
#include "msgparse.h"
#include "mpi.h"
#include "interp.h"
#include "reg.h"
#include "externs.h"

/* Cynbe stuff added to help decode corefiles:
#define CRT_LAST_COMMAND_MAX 65535
char crt_last_command[ CRT_LAST_COMMAND_MAX ];
int  crt_last_len;
int  crt_last_player;
*/
int  crt_connect_count = 0;

extern int errno;
int     shutdown_flag = 0;
int     restart_flag = 0;
int	dump_flag = 0;

static const char *create_fail = "That player exists, you need a password, or you picked a bad name.\r\n";
static const char *connect_fail = "Incorrect login.\r\n";

static const char *flushed_message = "<Flushed>\r\n";

int resolver_sock[2];

struct descriptor_data	*descriptor_list = 0;

static int numsocks = 0;
static int sock[MAX_LISTEN_PORTS];
static int ndescriptors = 0;
extern void fork_and_dump(void);

#ifdef RWHO
extern int rwhocli_setup(const char *server, const char *serverpw, const char *myname, const char *comment);
extern int rwhocli_shutdown(void);
extern int rwhocli_pingalive(void);
extern int rwhocli_userlogin(const char *uid, const char *name,time_t tim);
extern int rwhocli_userlogout(const char *uid);
#endif

void    process_commands(void);
void    shovechars(int portc, int* portv, int wwwport);
void    shutdownsock(struct descriptor_data * d);
struct descriptor_data *initializesock(int s, const char *hostname, int port, int hostaddr, int ctype);
void    make_nonblocking(int s);
void    freeqs(struct descriptor_data * d);
void    welcome_user(struct descriptor_data * d);
void    help_user(struct descriptor_data * d);
void    check_connect(struct descriptor_data * d, const char *msg);
void    close_sockets(const char *msg);
const char *addrout(int, unsigned short, int);
void    dump_users(struct descriptor_data * d, char *user);
struct descriptor_data *new_connection(int sock, int port, int ctype);
void    parse_connect(const char *msg, char *command, char *user, char *pass);
void    set_userstring(char **userstring, const char *command);
int     do_command(struct descriptor_data * d, char *command);
char   *strsave(const char *s);
int     make_socket(int);
int     queue_string(struct descriptor_data *, const char *);
int     queue_write(struct descriptor_data *, const char *, int);
int     process_output(struct descriptor_data * d);
int     process_input(struct descriptor_data * d);
void    announce_idle(struct descriptor_data * d);
void    announce_unidle(struct descriptor_data * d);
void    announce_connect(dbref);
void    announce_disconnect(struct descriptor_data * d);
char   *time_format_1(time_t);
char   *time_format_2(time_t);
void    init_descriptor_lookup(void);
void    init_descr_count_lookup(void);
void    remember_player_descr(dbref player, int);
void    update_desc_count_table(void);
void    forget_player_descr(dbref player, int);
int     remember_descriptor(struct descriptor_data *);
void    forget_descriptor(struct descriptor_data *);
struct  descriptor_data* descrdata_by_descr(int descr);
struct  descriptor_data* descrdata_by_index(int index);
struct  descriptor_data* lookup_descriptor(int);
int     online(dbref player);
int     online_init(void);
dbref   online_next(int *ptr);
int 	max_open_files(void);
const char *clean_doing( const char *doing );

#ifdef SPAWN_HOST_RESOLVER
void kill_resolver(void);
#endif

#ifdef SPAWN_HOST_RESOLVER
void spawn_resolver(void);
void resolve_hostnames(void);
#endif

#define MALLOC(result, type, number) do {   \
				       if (!((result) = (type *) malloc ((number) * sizeof (type)))) \
				       panic("Out of memory"); \
				     } while (0)

#define FREE(x) (free((void *) x))

#ifndef BOOLEXP_DEBUGGING

#ifdef DISKBASE
extern FILE *input_file;
extern FILE *delta_infile;
extern FILE *delta_outfile;

#endif

short db_conversion_flag = 0;
short db_decompression_flag = 0;
short wizonly_mode = 0;
short localhost_mode = 0;
unsigned long listen_address = INADDR_ANY;

time_t sel_prof_start_time;
long sel_prof_idle_sec;
long sel_prof_idle_usec;
unsigned long sel_prof_idle_use;

void
show_program_usage(char *prog)
{
    fprintf(stderr, "Usage: %s [<options>] @<address> infile dumpfile [portnum [portnum ...]]\n", prog);
    fprintf(stderr, "            @<address>     only listen on specified IP address\n");
    fprintf(stderr, "            -convert       load db, save in current format, and quit.\n");
    fprintf(stderr, "            -decompress    when saving db, save in uncompressed format.\n");
    fprintf(stderr, "            -nosanity      don't do db sanity checks at startup time.\n");
    fprintf(stderr, "            -insanity      load db, then enter interactive sanity editor.\n");
    fprintf(stderr, "            -sanfix        attempt to auto-fix a corrupt db after loading.\n");
    fprintf(stderr, "            -wizonly       only allow " NAMEWIZ "s to login.\n");
    fprintf(stderr, "            -localhost     only allow localhost users to login.\n");
    fprintf(stderr, "            -help          display this message.\n");
    exit(1);
}


extern int sanity_violated;

int 
main(int argc, char **argv)
{
  FILE *ffd;
  char *infile_name = NULL;
    char *outfile_name = NULL;
    int i, plain_argnum, nomore_options;
    int sanity_skip;
    int sanity_interactive;
    int sanity_autofix;
    int portcount = 0;
    int whatport[MAX_LISTEN_PORTS];
    int wwwport = 0;

#ifdef DETACH
    int   fd;
#endif

#ifdef WIN95
    WSADATA wsaData; 
    if(WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
	perror("WSAStartup failed.");
	exit(0);
	return 0;
    }
#endif

    if (argc < 3) {
	show_program_usage(*argv);
    }

    time(&current_systime);
    init_descriptor_lookup();
    init_descr_count_lookup();

    plain_argnum = 0;
    nomore_options = 0;
    sanity_skip = 0;
    sanity_interactive = 0;
    sanity_autofix = 0;
    for (i = 1; i < argc; i++) {
	if (!nomore_options && argv[i][0] == '@') {
	    nomore_options = 1;
	    listen_address = htonl( str2ip( argv[i] + 1 ) );
	    if(listen_address == -1) {
		fprintf(stderr, "Invalid listen address.\n");
		exit(3);
	    }
	} else if (!nomore_options && argv[i][0] == '-') {
	    if (!strcmp(argv[i], "-convert")) {
		db_conversion_flag = 1;
	    } else if (!strcmp(argv[i], "-decompress")) {
		db_decompression_flag = 1;
	    } else if (!strcmp(argv[i], "-nosanity")) {
		sanity_skip = 1;
	    } else if (!strcmp(argv[i], "-insanity")) {
		sanity_interactive = 1;
	    } else if (!strcmp(argv[i], "-wizonly")) {
		wizonly_mode = 1;
	    } else if (!strcmp(argv[i], "-localhost")) {
		localhost_mode = 1;
	    } else if (!strcmp(argv[i], "-sanfix")) {
		sanity_autofix = 1;
	    } else if (!strcmp(argv[i], "--")) {
		nomore_options = 1;
	    } else {
		show_program_usage(*argv);
	    }
	} else {
	    nomore_options = 1;
	    plain_argnum++;
	    switch (plain_argnum) {
	      case 1:
		infile_name = argv[i];
		break;
	      case 2:
		outfile_name = argv[i];
		break;
	      default:
		whatport[portcount] = atoi(argv[i]);
		if (portcount >= MAX_LISTEN_PORTS) {
		    show_program_usage(*argv);
		}
		if (whatport[portcount] < 1 || whatport[portcount] > 65535) {
		    show_program_usage(*argv);
		}
		portcount++;
		break;
	    }
	}
    }

    if (plain_argnum < 2) {
	show_program_usage(*argv);
    } else if (plain_argnum == 2) {
	whatport[portcount++] = TINYPORT;
    }

#ifdef DISKBASE
    if (!strcmp(infile_name, outfile_name)) {
	fprintf(stderr, "Output file must be different from the input file.\n");
	exit(3);
    }
#endif


    if (!sanity_interactive) {

#ifdef DETACH
	/* Go into the background */
	fclose(stdin); fclose(stdout); fclose(stderr);
	if (fork() != 0) exit(0);
#endif

	/* save the PID for future use */
	if ((ffd = fopen(PID_FILE,"wb")) != NULL)
	{
	    fprintf(ffd, "%d\n", getpid());
	    fclose(ffd);
	}

	log_status("INIT: %s starting.\n", VERSION);

#ifdef DETACH
	/* Detach from the TTY, log whatever output we have... */
	freopen(LOG_ERR_FILE,"a",stderr);
	setbuf(stderr, NULL);
	freopen(LOG_FILE,"a",stdout);
	setbuf(stdout, NULL);

	/* Disassociate from Process Group */
# ifdef SYS_POSIX
	setsid();
# else
#  if defined(SYSV) || defined(LINUX)
	setpgrp(); /* System V's way */
#  else
	setpgrp(0, getpid()); /* BSDism */
#  endif  /* SYSV */

#  ifdef  TIOCNOTTY  /* we can force this, POSIX / BSD */
	if ( (fd = open("/dev/tty", O_RDWR)) >= 0)
	{
	    ioctl(fd, TIOCNOTTY, (char *)0); /* lose controll TTY */
	    close(fd);
	}
#  endif /* TIOCNOTTY */
# endif /* !SYS_POSIX */
#endif /* DETACH */

	if (!db_conversion_flag) {
#ifdef SPAWN_HOST_RESOLVER
	    spawn_resolver();
#endif
	    host_init();
	}

    }

    sel_prof_start_time = time(NULL); /* Set useful starting time */
    sel_prof_idle_sec = 0;
    sel_prof_idle_usec = 0;
    sel_prof_idle_use = 0;
    if (init_game(infile_name, outfile_name) < 0) {
	fprintf(stderr, "Couldn't load %s!\n", infile_name);
	exit(2);
    }

    if (!sanity_interactive && !db_conversion_flag) {
#if !defined(WIN95) || !defined(DEBUG)
	set_signals();
#endif

	if (!sanity_skip) {
	    sanity(AMBIGUOUS);
	    if (sanity_violated) {
		wizonly_mode = 1;
		if (sanity_autofix) {
		    sanfix(AMBIGUOUS);
		}
	    }
	}

#ifdef HTTPD
	if((portcount < MAX_LISTEN_PORTS) &&
	   (tp_www_port > 0) &&
	   (tp_www_port <= 65535) &&
	   (!localhost_mode) &&
	   (!wizonly_mode)
	) {
	    whatport[portcount++] = wwwport = tp_www_port;
	}
#endif /* HTTPD */

	/* go do it */
	shovechars(portcount, whatport, wwwport);

	if (restart_flag) {
	    char buf[BUFFER_LEN + 6];
	    sprintf(buf, "\r\n%.512s\r\n\r\n", tp_restart_mesg);
	    close_sockets(buf);
	} else {
	    char buf[BUFFER_LEN + 6];
	    sprintf(buf, "\r\n%.512s\r\n\r\n", tp_shutdown_mesg);
	    close_sockets(buf);
	}

	do_dequeue((dbref) 1, "all");

#ifdef RWHO
	if (tp_rwho)
	    rwhocli_shutdown();
#endif
	host_shutdown();
      }

    if (sanity_interactive) {
	san_main();
    } else {
	dump_database();
	tune_save_parmsfile();

#ifdef SPAWN_HOST_RESOLVER
	kill_resolver();
#endif

#ifdef WIN95
	WSACleanup();
#endif

#ifdef MALLOC_PROFILING
	db_free();
	free_old_macros();
	purge_all_free_frames();
	purge_mfns();
#endif

#ifdef DISKBASE
	fclose(input_file);
#endif
#ifdef DELTADUMPS
	fclose(delta_infile);
	fclose(delta_outfile);
	if(unlink(DELTAFILE_NAME))
	    perror(DELTAFILE_NAME);
#endif

#ifdef MALLOC_PROFILING
	CrT_summarize_to_file("malloc_log", "Shutdown");
#endif

	if (restart_flag) {
	    char* argbuf[MAX_LISTEN_PORTS + 3];
	    int socknum;
	    int argnum = 0;

	    argbuf[argnum++] = "restart";
	    for (socknum = 0; socknum < numsocks; socknum++, argnum++) {
		if(whatport[socknum] != wwwport) {
		    argbuf[argnum] = (char *)malloc(16);
		    sprintf(argbuf[argnum], "%d", whatport[socknum]);
		}
	    }
	    argbuf[argnum] = NULL;
	    execv("restart", argbuf);
	}
    }

    exit(0);
    return 0;
}

#endif				/* BOOLEXP_DEBUGGING */



void
notify_descriptor(int descr, const char *msg)
{
   char *ptr1;
   const char *ptr2;
   char buf[BUFFER_LEN + 2];
   struct descriptor_data *d;

   for (d = descriptor_list; d && (d->descriptor != descr); d = d->next);

    ptr2 = msg;
    while (ptr2 && *ptr2) {
	ptr1 = buf;
	while (ptr2 && *ptr2 && *ptr2 != '\r')
	    *(ptr1++) = *(ptr2++);
	*(ptr1++) = '\r';
	*(ptr1++) = '\n';
	*(ptr1++) = '\0';
	if (*ptr2 == '\r')
	    ptr2++;
   }
   queue_string(d, buf);
   process_output(d);   
}

int 
notify_nolisten(dbref player, const char *msg, int isprivate)
{
    int     retval = 0;
    char    buf[BUFFER_LEN + 2];
    char    buf2[BUFFER_LEN + 2];
    struct descriptor_data *d;
    int firstpass = 1;
    char *ptr1;
    const char *ptr2;
    dbref ref;
    int di;
    int *darr;
    int dcount;

#ifdef COMPRESS
    msg = uncompress(msg);
#endif				/* COMPRESS */

    ptr2 = msg;
    while (ptr2 && *ptr2) {
	ptr1 = buf;
	while (ptr2 && *ptr2 && *ptr2 != '\r')
	    *(ptr1++) = *(ptr2++);
	*(ptr1++) = '\r';
	*(ptr1++) = '\n';
	*(ptr1++) = '\0';
	if (*ptr2 == '\r')
	    ptr2++;

	if (Typeof(player) == TYPE_PLAYER) {
	    darr = DBFETCH(player)->sp.player.descrs;
	    dcount = DBFETCH(player)->sp.player.descr_count;
	    if (!darr)
		dcount = 0;
	} else {
	    dcount = 0;
	    darr = NULL;
	}

        for (di = 0; di < dcount; di++) {
	    d = descrdata_by_index(darr[di]);
	    if(d)
		queue_string(d, buf);
	    if (firstpass) retval++;
	}

	if ( tp_zombies && (!(FLAGS(player)&QUELL)) ) {
	    if ((Typeof(player) == TYPE_THING) && (FLAGS(player) & ZOMBIE) &&
		!(FLAGS(OWNER(player)) & ZOMBIE) /* &&
		( !(FLAGS(player) & DARK) || Mage(OWNER(player)) ) */
	    ) {
		ref = getloc(player);
		if (Mage(OWNER(player)) || ref == NOTHING ||
			Typeof(ref) != TYPE_ROOM || !(FLAGS(ref) & ZOMBIE)) {
		    if (isprivate || getloc(player) != getloc(OWNER(player))) {
			char pbuf[BUFFER_LEN];
			const char *prefix;

			prefix = GETPECHO(player);
			if (prefix && *prefix) {
			    char ch = *match_args;

#ifdef COMPRESS
			    prefix = uncompress(prefix);
#endif
			    *match_args = '\0';
			    prefix = do_parse_mesg(player, player, prefix,
			    		"(@Pecho)", pbuf, MPI_ISPRIVATE
			    	     );
			    *match_args = ch;
			}
			if (!prefix || !*prefix) {
			    prefix = NAME(player);
			    sprintf(buf2, "%.512s> %.*s", prefix,
				(int)(BUFFER_LEN - (strlen(prefix) + 3)), buf);
			} else {
			    sprintf(buf2, "%.512s %.*s", prefix,
				(int)(BUFFER_LEN - (strlen(prefix) + 2)), buf);
			}

                        if (Typeof(OWNER(player)) == TYPE_PLAYER) {
                            darr   = DBFETCH(OWNER(player))->sp.player.descrs;
                            dcount = DBFETCH(OWNER(player))->sp.player.descr_count;
                            if (!darr)
                                dcount = 0;
                        } else {
                            dcount = 0;
                            darr = NULL;
                        }

                        for (di = 0; di < dcount; di++) {
			    d = descrdata_by_index(darr[di]);
			    if(d)
				queue_string(d, buf2);
                            if (firstpass)
				retval++;
                        }
		    }
		}
	    }
	}
	firstpass = 0;
    }
    return retval;
}

int 
notify_from_echo(dbref from, dbref player, const char *msg, int isprivate)
{
    return notify_listeners(from, NOTHING, player, getloc(from), msg, isprivate);
}

int 
notify_from(dbref from, dbref player, const char *msg)
{
    return notify_from_echo(from, player, msg, 1);
}

int 
notify(dbref player, const char *msg)
{
    return notify_from_echo(player, player, msg, 1);
}

void do_linewrap(dbref player, const char *arg) {
    int size = atoi(arg), lval;

    if(!OkObj(player)) return;
    if( Typeof(player) != TYPE_PLAYER ) return;

    if(!arg || !*arg) {
	lval = DBFETCH(player)->sp.player.linewrap;
	if(lval > 0) {
	    anotify_fmt(player, CINFO "Your linewrap is set to %d columns.", lval);
	} else {
	    anotify(player, CNOTE "'@wrap 80' will set your linewrap to 80 columns.");
	    anotify(player, CINFO "Your linewrap is off.");
	}
	return;
    }

    if(size <= 0) {
	DBFETCH(player)->sp.player.linewrap = 0;
	if(!Guest(player)) {
	    ts_modifyobject(player);
	    remove_property(player, PROP_LINEWRAP);
	}
	anotify(player, CINFO "Linewrap turned off.");
    } else {
	DBFETCH(player)->sp.player.linewrap = size;
	/* We let guests set linewrap but not save it when they log out. */
	if(!Guest(player)) {
	    ts_modifyobject(player);
	    add_property(player, PROP_LINEWRAP, arg, 0);
	}
	anotify_fmt(player, CINFO "Linewrap set to %d columns.", size);
    }
}

void do_more(dbref player, const char *arg) {
    int size = atoi(arg), lval;

    if(!OkObj(player)) return;
    if( Typeof(player) != TYPE_PLAYER ) return;

    lval = clear_prompt(player);
    if(!arg || !*arg) {
	/* If we cleared a page with no argument, return */
	if(lval)
	    return;
	lval = DBFETCH(player)->sp.player.more;
	if(lval > 0) {
	    anotify(player, CNOTE "'more off' will turn more paging off.");
	    anotify_fmt(player, CINFO "Your more paging is set to every %d lines.", lval);
	} else {
	    anotify(player, CNOTE "'more 20' will set your more paging to every 20 lines.");
	    anotify(player, CINFO "Your more paging is off.");
	}
	return;
    }

    if(size <= 0) {
	DBFETCH(player)->sp.player.more = 0;
	if(!Guest(player)) remove_property(player, PROP_MORE);
	anotify(player, CINFO "More paging turned off.");
    } else {
	DBFETCH(player)->sp.player.more = size;
	/* We let guests set page length but not save it when they log out. */
	if(!Guest(player)) add_property(player, PROP_MORE, arg, 0);
	anotify_fmt(player, CINFO "More paging set to every %d lines.", size);
    }
}


int 
ansi_notify_nolisten(dbref player, const char *msg, int isprivate, int parseansi)
{
    char    buf[BUFFER_LEN + 2];

    /* ansi() checks that ansi lookup table != NULL */
    ansi(buf, msg, DBFETCH(OWNER(player))->sp.player.ansi,
	((parseansi != 2) || tp_server_ansi) &&
	(Typeof(OWNER(player)) == TYPE_PLAYER) &&
	(FLAGS(OWNER(player)) & CHOWN_OK),
	parseansi
    );

    return notify_nolisten(player, buf, isprivate);
}


int 
ansi_notify_from_echo(dbref from, dbref player, const char *msg, int isprivate, int parseansi)
{
    return ansi_notify_listeners(from, NOTHING, player, getloc(from), msg, isprivate, parseansi);
}


int 
ansi_notify_from(dbref from, dbref player, const char *msg, int parseansi)
{
    return ansi_notify_from_echo(from, player, msg, 1, parseansi);
}


int 
ansi_notify(dbref player, const char *msg, int parseansi)
{
    return ansi_notify_from_echo(player, player, msg, 1, parseansi);
}


struct timeval 
timeval_sub(struct timeval now, struct timeval then)
{
    now.tv_sec -= then.tv_sec;
    now.tv_usec -= then.tv_usec;
    if (now.tv_usec < 0) {
	now.tv_usec += 1000000;
	now.tv_sec--;
    }
    return now;
}

int 
msec_diff(struct timeval now, struct timeval then)
{
    return ((now.tv_sec - then.tv_sec) * 1000
	    + (now.tv_usec - then.tv_usec) / 1000);
}

struct timeval 
msec_add(struct timeval t, int x)
{
    t.tv_sec += x / 1000;
    t.tv_usec += (x % 1000) * 1000;
    if (t.tv_usec >= 1000000) {
	t.tv_sec += t.tv_usec / 1000000;
	t.tv_usec = t.tv_usec % 1000000;
    }
    return t;
}

struct timeval 
update_quotas(struct timeval last, struct timeval current)
{
    int     nslices;
    int     cmds_per_time;
    struct descriptor_data *d;

    nslices = msec_diff(current, last) / tp_command_time_msec;

    if (nslices > 0) {
	for (d = descriptor_list; d; d = d->next) {
	    if (d->connected) {
		cmds_per_time = ((FLAGS(d->player) & INTERACTIVE)
			      ? (tp_commands_per_time * 8) : tp_commands_per_time);
	    } else {
		cmds_per_time = tp_commands_per_time;
	    }
	    d->quota += cmds_per_time * nslices;
	    if (d->quota > tp_command_burst_size)
		d->quota = tp_command_burst_size;
	}
    }
    return msec_add(last, nslices * tp_command_time_msec);
}

/*
 * long max_open_files()
 *
 * This returns the max number of files you may have open
 * as a long, and if it can use setrlimit() to increase it,
 * it will do so.
 *
 * Becuse there is no way to just "know" if get/setrlimit is
 * around, since its defs are in <sys/resource.h>, you need to
 * define USE_RLIMIT in config.h to attempt it.
 *
 * Otherwise it trys to use sysconf() (POSIX.1) or getdtablesize()
 * to get what is avalible to you.
 */
#ifdef HAVE_RESOURCE_H
# include <sys/resource.h>
#endif

#if defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE)
# define USE_RLIMIT
#endif

int 	max_open_files(void)
{
#if defined(_SC_OPEN_MAX) && !defined(USE_RLIMIT) /* Use POSIX.1 method, sysconf() */
/*
 * POSIX.1 code.
 */
    return sysconf(_SC_OPEN_MAX);
#else /* !POSIX */
# if defined(USE_RLIMIT) && (defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE))
#  ifndef RLIMIT_NOFILE
#   define RLIMIT_NOFILE RLIMIT_OFILE /* We Be BSD! */
#  endif /* !RLIMIT_NOFILE */
/*
 * get/setrlimit() code.
 */
    struct rlimit file_limit;
    
    getrlimit(RLIMIT_NOFILE, &file_limit);		/* Whats the limit? */

    if (file_limit.rlim_cur < file_limit.rlim_max)	/* if not at max... */
    {
	file_limit.rlim_cur = file_limit.rlim_max;	/* ...set to max. */
	setrlimit(RLIMIT_NOFILE, &file_limit);
	
	getrlimit(RLIMIT_NOFILE, &file_limit);		/* See what we got. */
    }	

    return file_limit.rlim_cur;

# else /* !RLIMIT */
/*
 * Don't know what else to do, try getdtablesize().
 * email other bright ideas to me. :) (whitefire)
 */
    return getdtablesize();
# endif /* !RLIMIT */
#endif /* !POSIX */
}

void 
goodbye_user(struct descriptor_data * d)
{
    int len;
    
    writesocket(d->descriptor, "\r\n", 2);
    writesocket(d->descriptor, tp_leave_mesg, len = strlen(tp_leave_mesg));
    writesocket(d->descriptor, "\r\n\r\n", 4);
    tp_write_bytes += (6 + len);
}

void
idleboot_user(struct descriptor_data * d)
{
    int len;

    writesocket(d->descriptor, "\r\n", 2);
    writesocket(d->descriptor, tp_idle_mesg, len = strlen(tp_idle_mesg));
    writesocket(d->descriptor, "\r\n\r\n", 4);
    tp_write_bytes += (6 + len);
    d->booted=1;
}

static int con_players_max = 0;  /* one of Cynbe's good ideas. */
static int con_players_curr = 0;  /* for playermax checks. */
extern void purge_free_frames(void);

time_t current_systime = 0;

void 
shovechars(int portc, int* portv, int wwwport)
{
    fd_set  input_set, output_set;
    time_t  now, tmptq;
    struct timeval last_slice, current_time;
    struct timeval next_slice;
    struct timeval timeout, slice_timeout;
    int     maxd, cnt;
    struct descriptor_data *d, *dnext;
    struct descriptor_data *newd;
    int     avail_descriptors;
    struct timeval sel_in, sel_out;
    int socknum;
    int openfiles_max;
    int ctype;

    for (maxd = 0, socknum = 0; socknum < portc; socknum++) {
	sock[socknum] = make_socket(portv[socknum]);
	maxd = sock[socknum] + 1;
	numsocks++;
    }
    gettimeofday(&last_slice, NULL);

    openfiles_max =  max_open_files();
    printf("Max FDs = %d\n", openfiles_max);

    avail_descriptors = openfiles_max - 5;

    while (shutdown_flag == 0) {
	gettimeofday(&current_time, NULL);
	last_slice = update_quotas(last_slice, current_time);

	next_muckevent();
	process_commands();

	if(dump_flag != 0) {
	    dump_db_now();
	    dump_flag = 0;
	}

	time(&now);
	for (d = descriptor_list; d; d = dnext) {
	    dnext = d->next;
	    if (d->booted) {
#ifdef HTTPD
 #ifdef HTTPDELAY
		if(d->httpdata) {
		    queue_string(d, d->httpdata);
		    free((void *)d->httpdata);
		    d->httpdata = NULL;
		}
 #endif /* HTTPDELAY */
#endif /* HTTPD */
		process_output(d);

		/* Boot '3' means close after all output has been sent, */
		/* gives HTTP requests up to a minute to send their info. */
		if(d->booted && (
		       (d->booted < 3) ||
		       ((now - d->last_time) > 60)
		     )
		) {
		    if (d->booted == 2) {
			goodbye_user(d);
		    }
		    d->booted = 0;
		    d->moreline = 0;
		    d->prompted = 0;
		    process_output(d);
		    shutdownsock(d);
		}
	    }
	}
	purge_free_frames();
	untouchprops_incremental(1);

	if (shutdown_flag)
	    break;
	timeout.tv_sec = 4;
	timeout.tv_usec = 0;
	next_slice = msec_add(last_slice, tp_command_time_msec);
	slice_timeout = timeval_sub(next_slice, current_time);

	FD_ZERO(&output_set);
	FD_ZERO(&input_set);
	if (ndescriptors < avail_descriptors) {
	    for (socknum = 0; socknum < numsocks; socknum++) {
		FD_SET(sock[socknum], &input_set);
	    }
	}
	time(&now);
	for (d = descriptor_list; d; d = d->next) {
	    if (d->input.lines > 100)
		timeout = slice_timeout;
	    else if (d->resolved || (d->type != CT_MUCK) || (!tp_validate_hostname) || (!tp_hostnames))
		FD_SET(d->descriptor, &input_set);
	    if ((d->output.head && !d->prompted) ||
		/* Check to close HTTP if no commands in input queue */
		((d->booted == 3) && (!d->raw_input) && (!d->input.lines))
	    )
		FD_SET(d->descriptor, &output_set);
	}
#ifdef SPAWN_HOST_RESOLVER
	FD_SET(resolver_sock[1], &input_set);
#endif

	tmptq = next_muckevent_time();
	if ((tmptq >= 0L) && (timeout.tv_sec > tmptq)) {
	    timeout.tv_sec = tmptq + (tp_pause_min / 1000);
	    timeout.tv_usec = (tp_pause_min % 1000) * 1000L;
	}
	gettimeofday(&sel_in,NULL);
	if (select(maxd, &input_set, &output_set,
		   (fd_set *) 0, &timeout) < 0) {
	    if (errnosocket != EINTR) {
		perror("select");
		return;
	    }
	} else {
	    time(&current_systime);
	    gettimeofday(&sel_out, NULL);
	    sel_out.tv_usec -= sel_in.tv_usec;
	    sel_out.tv_sec -= sel_in.tv_sec;
	    if (sel_out.tv_usec < 0) {
		sel_out.tv_usec += 1000000;
		sel_out.tv_sec -= 1;
	    }
	    sel_prof_idle_sec += sel_out.tv_sec;
	    sel_prof_idle_usec += sel_out.tv_usec;
	    if (sel_prof_idle_usec >= 1000000) {
		sel_prof_idle_usec -= 1000000;
		sel_prof_idle_sec += 1;
	    }
	    sel_prof_idle_use++;
	    time(&now);
	    for (socknum = 0; socknum < numsocks; socknum++) {

		ctype =
#ifdef HTTPD
		    (portv[socknum] == wwwport) ? CT_HTML :
#endif
		CT_MUCK;

		if (FD_ISSET(sock[socknum], &input_set)) {
		    if (!(newd = new_connection(sock[socknum], portv[socknum], ctype))) {
			if ((newd < 0) && errnosocket
#ifndef WIN95
				    && errnosocket != EINTR
				    && errnosocket != EMFILE
				    && errnosocket != ENFILE
#endif /* WIN95 */
			) {
			    log_status("NCER: errno %d type %d\n", errnosocket, ctype);
			}
		    } else {
			if (newd->descriptor >= maxd)
			    maxd = newd->descriptor + 1;
			boot_welcome_site_idlest(newd->hostaddr);
			if(str2ip(newd->hostname) != newd->hostaddr) {
			    newd->resolved = 1;
			    welcome_user(newd);
			} else if ((newd->type == CT_MUCK) && tp_validate_hostname && tp_hostnames) {
			    if(tp_validate_warning) queue_string(newd,
				"Finding your hostname, please wait.\r\n"
			    );
			} else
			    welcome_user(newd);
		    }
		}
	    }

#ifdef SPAWN_HOST_RESOLVER
	    if (FD_ISSET(resolver_sock[1], &input_set)) {
		resolve_hostnames();
	    }
#endif
	    for (cnt = 0, d = descriptor_list; d; d = dnext) {
		dnext = d->next;
		if (FD_ISSET(d->descriptor, &input_set)) {
		    if(	!d->connected || !(FLAG2(d->player) & F2IDLE) ) {
			d->last_time = now;
			if(d->idle && tp_show_idlers && d->connected) {
			    announce_unidle(d);
			}
			d->idle = 0;
		    }
		    if (!process_input(d)) {
			/* log_status("TEST: d%d b%d Input closed, booting.\n", d->descriptor, d->booted); */
			d->booted = 1;
		    }
		}
		if (FD_ISSET(d->descriptor, &output_set)) {
		    /*	Ok, I'm having trouble with the logic of this...
			If an HTTP request comes in we need to hold the
			socket open until all data is flushed out, we can
			check this by looking at the outputd FD_ISSET.
			To make sure it's really safe we check output after
			everything has been written through again.  Also don't
			close until all input is in, which gets read in from
			the input_set and stored in either raw_input or input.head
			depending how far it got.  We check here that nothing
			new was added to raw_input or input.head again.  We
			want to check in the FD_SET part to keep the processor
			from spinning it's wheels unnecessarily.

			boot = 3 -> Close after output is all out and flushed
		    */
		    if((d->booted == 3) && (!d->output_size) &&
			(!d->raw_input) && (!d->input.lines)
		    ) {
			/* log_status("TEST: Set d%d b%d boot=1 CORRECT\n", d->descriptor, d->booted); */
			d->booted = 1;
		    }
		    if (!process_output(d)) {
			d->booted = 1;
		    }
		}
		if (d->connected) {
		    cnt++;
		    if (tp_idleboot&&((now - d->last_time) > tp_maxidle)&&
			    !TMage(d->player)
		    ) {
			idleboot_user(d);
		    } else if(!d->idle && (now - d->last_time) > tp_idletime) {
			d->idle = 1;
			if(tp_show_idlers) {
			    announce_idle(d);
			}
		    }
		} else {
		    /* On connect screen */
		    if ((now - d->connected_at) > tp_connect_wait) {
			d->booted = 1;
		    }
		}
	    }
	    if (cnt > con_players_max) {
		add_property((dbref)0, "~sys/max_connects", NULL, cnt);
		con_players_max = cnt;
	    }
	    con_players_curr = cnt;
	}
    }
    (void) time(&now);
    /* add_property((dbref)0, "~sys/lastdumptime", NULL, (int)now); */
    add_property((dbref)0, "~sys/shutdowntime", NULL, (int)now);
}


void 
wall_and_flush(const char *msg)
{
    struct descriptor_data *d;
    char    buf[BUFFER_LEN + 2];

#ifdef COMPRESS
    msg = uncompress(msg);
#endif				/* COMPRESS */

    if (!msg || !*msg) return;
    strcpy(buf, msg);
    /* strcat(buf, "\r\n"); Done in queue_ansi */

    for (d = descriptor_list; d; d = d->next) {
	queue_ansi(d, buf);
	if (!process_output(d)) {
	    d->booted = 1;
	}
    }
}

void 
wall_status(const char *msg)
{
    struct descriptor_data *d;
    char buf[BUFFER_LEN + 2];
    int pos=0;

#ifdef COMPRESS
    msg = uncompress(msg);
#endif				/* COMPRESS */

    strcpy(buf, "[$] ");
    strcat(buf, msg);

    while(buf[pos]) {
	if((buf[pos] == '\n') || !isprint(buf[pos])) {
	    buf[pos] = '\0';
	    break;
	}
	pos++;
    }

    strcat(buf, "\r\n");

    for (d = descriptor_list; d; d = d->next) {
	/* Only logwall to Mages+ if set LogWall, only Boys can set the flag */
	if ( d->connected && (FLAG2(d->player)&F2LOGWALL) && TMage(d->player)) {
	    queue_string(d, buf);
	    if (!process_output(d)) {
		d->booted = 1;
	    }
	}
    }
}

#ifdef MUD
void 
wall_mud(const char *msg)
{
    struct descriptor_data *d;
    char    buf[BUFFER_LEN + 2];

#ifdef COMPRESS
    msg = uncompress(msg);
#endif				/* COMPRESS */

    strcpy(buf, msg);
    strcat(buf, "\r\n");

    for (d = descriptor_list; d; d = d->next) {
	if ( d->connected && /* (d->player >= 0) && */ MUDDER(d->player) ) {
	    queue_string(d, buf);
	    if (!process_output(d)) {
		d->booted = 1;
	    }
	}
    }
}
#endif /* MUD */

void 
wall_arches(const char *msg)
{
    struct descriptor_data *d;
    char    buf[BUFFER_LEN + 2];

#ifdef COMPRESS
    msg = uncompress(msg);
#endif				/* COMPRESS */

    strcpy(buf, msg);
    strcat(buf, "\r\n");

    for (d = descriptor_list; d; d = d->next) {
	if ( d->connected && /* (d->player >= 0) && */ Wiz(d->player)) {
	    queue_string(d, buf);
	    if (!process_output(d)) {
		d->booted = 1;
	    }
	}
    }
}


void 
wall_wizards(const char *msg)
{
    struct descriptor_data *d;
    char    buf[BUFFER_LEN + 2];

#ifdef COMPRESS
    msg = uncompress(msg);
#endif				/* COMPRESS */

    strcpy(buf, msg);
    strcat(buf, "\r\n");

    for (d = descriptor_list; d; d = d->next) {
	if ( d->connected && /* (d->player >= 0) && */ Mage(d->player)) {
	    queue_string(d, buf);
	    if (!process_output(d)) {
		d->booted = 1;
	    }
	}
    }
}


void 
show_wizards(dbref player)
{
    struct descriptor_data *d;

    anotify(player, CINFO "WizChatters:");
    for (d = descriptor_list; d; d = d->next)
	if ( d->connected && /* (d->player >= 0) && */ Mage(d->player))
	    notify(player, NAME(d->player));
}



void
flush_user_output(dbref player)
{
    int di;
    int* darr;
    int dcount;
    struct descriptor_data *d;
 
    if (Typeof(player) == TYPE_PLAYER) {
        darr = DBFETCH(player)->sp.player.descrs;
        dcount = DBFETCH(player)->sp.player.descr_count;
        if (!darr)
            dcount = 0;
    } else {
        dcount = 0;
        darr = NULL;
    }

    for (di = 0; di < dcount; di++) {
        d = descrdata_by_index(darr[di]);
        if (d && !process_output(d))
            d->booted = 1;
    }
}   


void 
wall_all(const char *msg)
{
    struct descriptor_data *d;
    char    buf[BUFFER_LEN + 2];

#ifdef COMPRESS
    msg = uncompress(msg);
#endif				/* COMPRESS */

    strcpy(buf, msg);
    strcat(buf, "\r\n");

    for (d = descriptor_list; d; d = d->next) {
	queue_string(d, buf);
	if (!process_output(d))
	    d->booted = 1;
    }
}

struct descriptor_data *
new_connection(int sock, int port, int ctype)
{
    int    newsock;
    struct sockaddr_in addr;
    int    addr_len;
    int    lma; /* accepts in past minute */
    time_t lt;
    char   hostname[128];
    
    lt = current_systime;

    addr_len = sizeof(addr);
    newsock = accept(sock, (struct sockaddr *) & addr, (int *) &addr_len);
    if (newsock < 0) {
	return 0;
    } else {
	strcpy(hostname, host_as_hex(ntohl(addr.sin_addr.s_addr)));

	lma = host_touch_last_min_accepts(ntohl(addr.sin_addr.s_addr));

	/* Clear the lma value for localhost and web connections */
	if((port == tp_www_port) ||
	    (ntohl(addr.sin_addr.s_addr) == 0x7f000001) ||
	    (	(listen_address != INADDR_ANY) &&
		(addr.sin_addr.s_addr == listen_address)
	    )
	) lma = 0;

        if((tp_max_site_lma > 1) && (lma >= tp_max_site_lma)) {
            /* Boot now, ask questions later */
	    shutdown(newsock, 2);
	    closesocket(newsock);

	    if(lma == tp_max_site_lma) { /* Set ban for later */
		char sbuf[128], pbuf[128];

		sprintf(sbuf, REG_SITE "/%s x", host_as_hex(ntohl(addr.sin_addr.s_addr) & ~0xff));
		sprintf(pbuf, "DOS: Too many ACPTS/minute on %.24s", ctime(&lt));
		add_property((dbref)0, sbuf, pbuf, 0);
		log_status( "*DOS: %2d %s(%d) C#%d *** BLOCKED *** (Too many ACPTs/minute)\n", newsock,
		    hostname, ntohs(addr.sin_port),
		    ++crt_connect_count
		);
	    }
	    return 0;
        }

	if ((reg_site_is_blocked(ntohl(addr.sin_addr.s_addr)) == TRUE) &&
	    (ntohl(addr.sin_addr.s_addr) != 0x7f000001) &&
	    (	(listen_address == INADDR_ANY) ||
		(addr.sin_addr.s_addr != listen_address)
	    )
	) {
	    log_status( "*BLK: %2d %s(%d) C#%d\n", newsock,
		hostname, ntohs(addr.sin_port),
		++crt_connect_count
	    );
	    shutdown(newsock, 2);
	    closesocket(newsock);
	    return 0;
	}

        /* Don't start resolving until now as this could be a DOS attack */
	/* resolver spins it's wheels when you give it 10k+ requests...  */
	strcpy(hostname, addrout(addr.sin_addr.s_addr, addr.sin_port, port));

	if(/* 1 || */ ctype != CT_HTML) /* Ignore HTTP */
	  log_status( "ACPT: %2d %s(%d) %s C#%d %s\n", newsock,
	    hostname, ntohs(addr.sin_port),
	    host_as_hex(ntohl(addr.sin_addr.s_addr)),
	    ++crt_connect_count,
	    ctype==CT_MUCK   ? "muck"   : (
	    ctype==CT_HTML   ? "html"   : (
	    ctype==CT_PUEBLO ? "pueblo" : (
	    "unknown" )))
	  );
	return initializesock( newsock,
	    hostname, ntohs(addr.sin_port),
	    ntohl(addr.sin_addr.s_addr), ctype
	);
    }
}


#ifdef SPAWN_HOST_RESOLVER

void
kill_resolver()
{
    int i;
    pid_t p;

    write(resolver_sock[1], "QUIT\n", 5);
    p = wait(&i);
}



void
spawn_resolver()
{
    socketpair(AF_UNIX, SOCK_STREAM, 0, resolver_sock);
    make_nonblocking(resolver_sock[1]);
    if (!fork()) {
#ifdef DUPBUGFIX
	dup2(resolver_sock[0], STDIN_FILENO);
	dup2(resolver_sock[0], STDOUT_FILENO);
#else
	close(0);
	close(1);
	dup(resolver_sock[0]);
	dup(resolver_sock[0]);
#endif
	execl("./resolver", "resolver", NULL);
	perror("resolver execlp");
	_exit(1);
    }
}


void
resolve_hostnames()
{
    char buf[BUFFER_LEN], *oldname;
    char *ptr, *ptr2, *ptr3, *hostip, *port, *hostname, *username, *tempptr;
    struct descriptor_data *d;
    int got, dc, iport;
    int ipnum;

    got = read(resolver_sock[1], buf, sizeof(buf));
    if (got < 0) return;
    if (got == sizeof(buf)) {
	got--;
	while(got>0 && buf[got] != '\n') buf[got--] = '\0';
    }
    ptr = buf;
    dc = 0;
    do {
	for(ptr2 = ptr; *ptr && *ptr != '\n' && dc < got; ptr++,dc++);
	if (*ptr) { *ptr++ = '\0'; dc++; }
	if (*ptr2) {
	    ptr3 = index(ptr2, ':');
	    if (!ptr3) return;
	   hostip = ptr2;
	   port = index(ptr2, '(');
	   if (!port) return;
	   tempptr = index(port, ')');
	   if (!tempptr) return;
	   *tempptr = '\0';
	   hostname = ptr3;
	   username = index(ptr3, '(');
	   if (!username) return;
	   tempptr = index(username, ')');
	   if (!tempptr) return;
	   *tempptr = '\0';
	    if (*port&&*hostname&&*username) {
		*port++ = '\0';
		*hostname++ = '\0';
		*username++ = '\0';
		if( (ipnum = str2ip(hostip)) != -1 ) {
		    /* we got a new name, update? */
		    if(!( oldname = host_fetch(ipnum) ))
			host_add(ipnum, hostname, (time_t) 0);
		    else {
			if(strcmp(oldname, hostname)) {
			    log_status( "*RES: %s to %s\n", oldname, hostname );
			    host_del(ipnum);
			    host_add(ipnum, hostname, host_lastreq(ipnum));
			}
		    }
		} else {
		    log_status( "*BUG: resolve_hostnames bad ipstr %s\n", hostip );
		}
		iport = atoi(port);
		for (d = descriptor_list; d; d = d->next) {
		    if((d->hostaddr == ipnum) && (iport == d->port)) {
			FREE(d->hostname);
			FREE(d->username);
			d->hostname = strsave(hostname);
			d->username = strsave(username);
			if((!d->resolved) && (d->type == CT_MUCK) && tp_validate_hostname && tp_hostnames) {
			    if(tp_validate_warning) queue_string(d,
				"Done finding your hostname, continue.\r\n"
			    );
			    if(!d->connected)
				welcome_user(d);
			}
			d->resolved = 1;
		    }
		}
	    }
	}
    } while (dc < got && *ptr);
}

#endif


/*  addrout -- Translate address 'a' from int to text.		   */
/* 2004-03-18 ide/fm: I changed the servport param to 'int'  */
/* since that is what is being passed to it, and changed the */
/* sprintf accordingly.                                      */

const char *
addrout(int a, unsigned short prt, int servport) 
{
    static char buf[128];
    char *host;
    struct in_addr addr;
    
    addr.s_addr = a;
    prt = ntohs(prt);
    a = ntohl(a);
	
#ifndef SPAWN_HOST_RESOLVER
    if (tp_hostnames) {

	static int secs_lost = 0;

	if( (host = host_fetch(a)) ) {
	    sprintf(buf, "%.127s", host);
	    return buf;
	}

	/* One day the nameserver Qwest uses decided to start */
	/* doing halfminute lags, locking up the entire muck  */
	/* that long on every connect.  This is intended to   */
	/* prevent that, reduces average lag due to nameserver*/
	/* to 1 sec/call, simply by not calling nameserver if */
	/* it's in a slow mood *grin*. If the nameserver lags */
	/* consistently, a hostname cache ala OJ's tinymuck2.3*/
	/* would make more sense:			      */
	if (secs_lost) {
	    secs_lost--;
	} else {
	    time_t gethost_start = current_systime;
	    struct hostent *he   = gethostbyaddr(((char *)&addr), 
						 sizeof(addr), AF_INET);
	    time_t gethost_stop  = current_systime;
	    time_t lag	   = gethost_stop - gethost_start;
	    if (lag > 10) {
		secs_lost = lag;
#if	    MIN_SECS_TO_LOG
		if (lag >= CFG_MIN_SECS_TO_LOG) {
		    log_status( "GHNR: secs %3d\n", lag );
		}
#endif

	    }
	    if (he) {
		char *p;
		sprintf(buf, "%.127s", he->h_name);

		/* Windows 95 Winsock tends to return garbage for hosts */
		p = buf;
		while(*p) {
		    if(*p < ' ') /* Takes care of > 127 too from signed char */
			break;
		    p++;
		}
		if(*p == '\0') /* Only return string if no invalid chars */
		{
		    host_add(a, buf, (time_t) 0);
		    return buf;
		}
	    }
	}
    }
#endif /* no SPAWN_HOST_RESOLVER */

#ifdef SPAWN_HOST_RESOLVER
    sprintf(buf, "%d.%d.%d.%d(%u)%u\n",
	(a >> 24) & 0xff,
	(a >> 16) & 0xff,
	(a >> 8)  & 0xff,
	 a	& 0xff,
	prt, servport
    );
    if (tp_hostnames) {
	write(resolver_sock[1], buf, strlen(buf));
    }
#endif
    if( (host = host_fetch(a)) )
	sprintf(buf, "%.127s", host);
    else
	sprintf(buf, "%.127s", host_as_hex(a));

    return buf;
}


void 
clearstrings(struct descriptor_data * d)
{
    if (d->output_prefix) {
	FREE(d->output_prefix);
	d->output_prefix = 0;
    }
    if (d->output_suffix) {
	FREE(d->output_suffix);
	d->output_suffix = 0;
    }
}

void 
shutdownsock(struct descriptor_data * d)
{
    if (/* 1 || */ d->type != CT_HTML) { /* Ignore HTTP */
      if (d->connected) {
	log_status("DISC: %2d %s %s(%s) %s, %d cmds\n",
		d->descriptor, unparse_object(d->player, d->player),
		d->hostname, d->username,
		host_as_hex(d->hostaddr), d->commands);
	announce_disconnect(d);
      } else {
	log_status("DISC: %2d %s(%s) %s, %d cmds (never connected)\n",
		d->descriptor, d->hostname, d->username,
		host_as_hex(d->hostaddr), d->commands);
      }
    }

    if(d->connected && OkObj(d->player) && (Typeof(d->player) == TYPE_PLAYER)) {
	forget_player_descr(d->player, d->descriptor);
	d->connected = 0;
	d->player = NOTHING;
	update_desc_count_table();
    }
    clearstrings(d);
    shutdown(d->descriptor, 2);
    closesocket(d->descriptor);
    forget_descriptor(d);
    freeqs(d);
    *d->prev = d->next;
    if (d->next)
	d->next->prev = d->prev;
    if (d->hostname)
	free((void *)d->hostname);
    if (d->username)
	free((void *)d->username);
#ifdef HTTPD
 #ifdef HTTPDELAY
    if (d->httpdata)
	free((void *)d->httpdata);
 #endif /* HTTPDELAY */
#endif /* HTTPD */
    FREE(d);
    ndescriptors--;
}

struct descriptor_data *
initializesock(int s, const char *hostname, int port, int hostaddr, int ctype)
{
    struct descriptor_data *d;
    char buf[128];

    ndescriptors++;
    MALLOC(d, struct descriptor_data, 1);
    d->descriptor = s;
    d->connected = 0;
    d->player = 0;
    d->booted = 0;
    d->fails = 0;
    d->connected_at = current_systime;
    make_nonblocking(s);
    d->output_prefix = 0;
    d->output_suffix = 0;
    d->output_size = 0;
    d->output.lines = 0;
    d->output.head = 0;
    d->output.tail = &d->output.head;
    d->hostaddr = hostaddr;
    d->input.lines = 0;
    d->input.head = 0;
    d->input.tail = &d->input.head;
    d->raw_input = 0;
    d->raw_input_at = 0;
    d->quota = tp_command_burst_size;
    d->commands = 0;
    d->last_time = d->connected_at;
    d->last_fail = 0;
    d->port = port;
    d->hostname = alloc_string(hostname);
    sprintf(buf, "%d", port);
    d->username = alloc_string(buf);
    d->commands = 0;
    d->linelen = 0;
    d->moreline = 0;
    d->prompted = 0;
    d->type = ctype;
    d->idle = 0;
    d->resolved = 0;
#ifdef HTTPD
 #ifdef HTTPDELAY
    d->httpdata = NULL;
 #endif /* HTTPDELAY */
#endif /* HTTPD */
    if (descriptor_list)
	descriptor_list->prev = &d->next;
    d->next = descriptor_list;
    d->prev = &descriptor_list;
    descriptor_list = d;
    if(remember_descriptor(d) < 0)
	d->booted = 1; /* Drop the connection ASAP */

    return d;
}

int 
make_socket(int port)
{
    int     s;
    struct sockaddr_in server;
    int     opt;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("creating stream socket");
	exit(3);
    }
    opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
		   (char *) &opt, sizeof(opt)) < 0) {
	perror("setsockopt");
	exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = listen_address;

// This is mainly to avoid a compiler warning in MSVC...ide/fm 2004-03-18
#if INCL_WINSOCK_API_PROTOTYPES
    server.sin_port = htons((u_short) port);
#else
    server.sin_port = htons(port);
#endif // INCL_WINSOCK_API_PROTOTYPES
    if (bind(s, (struct sockaddr *) & server, sizeof(server))) {
	perror("binding stream socket");
	closesocket(s);
	exit(4);
    }
    listen(s, 5);
    return s;
}

struct text_block *
make_text_block(const char *s, int n)
{
    struct text_block *p;

    MALLOC(p, struct text_block, 1);
    MALLOC(p->buf, char, n);
    bcopy(s, p->buf, n);
    p->nchars = n;
    p->start = p->buf;
    p->nxt = 0;
    return p;
}

void 
free_text_block(struct text_block * t)
{
    FREE(t->buf);
    FREE((char *) t);
}

void 
add_to_queue(struct text_queue * q, const char *b, int n)
{
    struct text_block *p;

    if (n == 0)
	return;

    p = make_text_block(b, n);
    p->nxt = 0;
    *q->tail = p;
    q->tail = &p->nxt;
    q->lines++;
}

int 
flush_queue(struct text_queue * q, int n)
{
    struct text_block *p;
    int     really_flushed = 0;

    n += strlen(flushed_message);

    while (n > 0 && (p = q->head)) {
	n -= p->nchars;
	really_flushed += p->nchars;
	q->head = p->nxt;
	q->lines--;
	free_text_block(p);
    }
    p = make_text_block(flushed_message, strlen(flushed_message));
    p->nxt = q->head;
    q->head = p;
    q->lines++;
    if (!p->nxt)
	q->tail = &p->nxt;
    really_flushed -= p->nchars;
    return really_flushed;
}

int 
queue_write(struct descriptor_data * d, const char *b, int n)
{
    int     space;

    space = tp_max_output - d->output_size - n;
    if (space < 0)
	d->output_size -= flush_queue(&d->output, -space);
    add_to_queue(&d->output, b, n);
    d->output_size += n;
    return n;
}

int queue_ansi(struct descriptor_data *d, const char *s)
{
    char buf[BUFFER_LEN];

    ansi(buf, s,
	(d->connected) ? DBFETCH(d->player)->sp.player.ansi : NULL,
	tp_server_ansi && (d->connected) &&
	(FLAGS(d->player) & CHOWN_OK),
	2
    );
    strcat(buf, "\r\n");
    return queue_string(d, buf);
}

const char *wrap(char *buf, dbref player, const char *s)
{
    int lw = 0, col = 0, i = 0, ansi = 0, li = 0, lc = 0, split = 0;
    const char *ls = NULL;
    
    if( (!buf) || (!s) || (!OkObj(player))) return s;
    if( Typeof(player) != TYPE_PLAYER ) return s;
    if( (lw = DBFETCH(player)->sp.player.linewrap) <= 0) return s;

    while(*s) {
	if(ansi && isalpha(*s))
	    ansi = 2;

#ifdef WRAP_STRIPLEAD
	if(split && (!col) && (!ansi))
	    while(*s == ' ')
		s++; /* striplead of next line */
#endif /* WRAP_STRIPLEAD */

	switch(*s) {
	    case '\033':
		ansi = 1;
		break;
	    case '\n':
	    case '\r':
		split = col = ansi = li = lc = 0;
		ls = NULL; /* Clear last known space and end pointers */
		break;
	    default:
		/* Ignore nonprintables like ^G, etc */
	    	if((!ansi) && (*s >= ' ')) {
		    if((*s == ' ') /* (*(s + 1) >= ' ') */ ) {
			ls = s + 1;
			li = i;
			lc = col;
		    }
		    col++;
		}
	}

	/* We do this little stupidity so we don't cut ansi strings and
	   don't put an extra linefeed after an ANSI command with no text
	   after it
	*/
	if((!ansi) && (*s >= ' ') && (col > lw)) {
	    if(ls && (*s != ' ') /* && (lc >= (lw / 2)) */) {
		/* printf("p=%02d col=%02d lc=%02d li=%02d \"%s\" <- \"%s\"\n", player, col, lc, li, s, ls); */
		/* Backtrack to last space */
		i = li;
		s = ls;
	    }
	    buf[i++] = '\r';
	    buf[i++] = '\n';
	    col = li = lc = 0;
	    split = 1;
	    ls = NULL;
	} else {
	    buf[i++] = *(s++);

	    /* Reenable column counting after ansi string */
	    if(ansi == 2)
		ansi = 0;
	}
    }

    buf[i] = '\0';
    return buf;
}

int 
queue_string(struct descriptor_data * d, const char *s)
{
    char buf[BUFFER_LEN*3]; /* worst case: 1 col, 1 char + \r\n */

    buf[0] = '\0';
    if(d && (d->type != CT_HTML) && s) wrap(buf, d->player, s);
    if(*buf) s = buf;

    return queue_write(d, s, strlen(s));
}

int 
process_output(struct descriptor_data * d)
{
    struct text_block **qp, *cur;
    int     cnt, ml = 0;

    /* drastic, but this may give us crash test data */
    if (!d || !d->descriptor) {
	fprintf(stderr, "process_output: bad descriptor or connect struct!\n");
	abort();
    }

    if (d->output.lines == 0) {
	return 1;
    }

    /* More pager, hold output if past page length */
    if(d->prompted) {
	if(d->output_size > tp_max_output) {
	    /* Flush output, stupid spammer */
	    flush_queue(&d->output, 0);
	    d->moreline = 0;
	    d->prompted = 0;
	}
	return 1;
    }

    if ((d->connected) && (d->type != CT_HTML) && OkObj(d->player) )
	ml = (ml = DBFETCH(d->player)->sp.player.more) > 0 ? ml : 0;

    if((ml > 0) && (d->moreline >= ml)) { /* No space man! */
	if(d->output_size > 0) {
	    if(d->moreline == ml) {
	        int len = 0;
	        
		if(d->connected && OkObj(d->player) &&
		   (FLAGS(d->player) & (INTERACTIVE | READMODE))
		)
		    len = writesocket(d->descriptor, IMORE_MESSAGE, strlen(IMORE_MESSAGE));
		else
		    len = writesocket(d->descriptor, MORE_MESSAGE, strlen(MORE_MESSAGE));
		if(len > 0)
		    tp_write_bytes += len;
		d->prompted = 1;
	    }
	}
	return 1;
    }

    for ( qp = &d->output.head; (cur = *qp); ) {
	const char *np, *end;
	int nok = 0;

	if(ml>0) { /* More pager */
	    end = cur->start + cur->nchars;
	    for(np = cur->start; np < end; np++) {
		if(*np == '\n') {
		    d->moreline++;
		    if(d->moreline >= ml) { /* Stop the presses! */
			nok = (np - cur->start) + 1; /* # chars to let thru */
			break;
		    }
		}
	    }
	}

	cnt = writesocket(d->descriptor, cur->start, (nok>0) ? nok : cur->nchars);
        if(cnt > 0)
	    tp_write_bytes += cnt;
	if (cnt < 0) {
#ifdef DEBUGPROCESS
	    fprintf(stderr, "process_output: write failed errno %d\n",
		errnosocket);
#endif
	    if (errnosocket == EWOULDBLOCK)
		return 1;
	    return 0;
	}
	d->output_size -= cnt;
	if (cnt == cur->nchars) {
	    d->output.lines--;
	    if (!cur->nxt) {
		d->output.tail = qp;
		d->output.lines = 0;
	    }
	    *qp = cur->nxt;
	    free_text_block(cur);
	    if(nok>0) break; else continue;	/* do not adv ptr */
	}
	cur->nchars -= cnt;
	cur->start += cnt;
	break;
    }
    return 1;
}

void 
make_nonblocking(int s)
{
#if !defined(O_NONBLOCK) || defined(ULTRIX)	/* POSIX ME HARDER */
# ifdef FNDELAY 	/* SUN OS */
#  define O_NONBLOCK FNDELAY 
# else
#  ifdef O_NDELAY 	/* SyseVil */
#   define O_NONBLOCK O_NDELAY
#  endif /* O_NDELAY */
# endif /* FNDELAY */
#endif

#ifdef WIN95
	int turnon = 1;
	if (ioctl(s, FIONBIO, &turnon) != 0) {
#else
    if (fcntl(s, F_SETFL, O_NONBLOCK) == -1) {
#endif
	perror("make_nonblocking: fcntl");
	panic("O_NONBLOCK fcntl failed");
    }
}

void 
freeqs(struct descriptor_data * d)
{
    struct text_block *cur, *next;

    cur = d->output.head;
    while (cur) {
	next = cur->nxt;
	free_text_block(cur);
	cur = next;
    }
    d->output.lines = 0;
    d->output.head = 0;
    d->output.tail = &d->output.head;

    cur = d->input.head;
    while (cur) {
	next = cur->nxt;
	free_text_block(cur);
	cur = next;
    }
    d->input.lines = 0;
    d->input.head = 0;
    d->input.tail = &d->input.head;

    if (d->raw_input)
	FREE(d->raw_input);
    d->raw_input = 0;
    d->raw_input_at = 0;
}

char   *
strsave(const char *s)
{
    char   *p;

    MALLOC(p, char, strlen(s) + 1);

    if (p)
	strcpy(p, s);
    return p;
}

void 
save_command(struct descriptor_data * d, const char *command)
{
    if(d->connected && OkObj(d->player))
	FLAG2(d->player) &= ~F2READBLANKLINE;
    if (d->connected && !string_compare((char *) command, BREAK_COMMAND)) {
	if (dequeue_prog(d->player, 2))
	    anotify(d->player, CINFO "Foreground program aborted.");
	DBFETCH(d->player)->sp.player.block = 0;
	if (!(FLAGS(d->player) & INTERACTIVE))
	    return;
    }
    add_to_queue(&d->input, command, strlen(command) + 1);
}

int 
process_input(struct descriptor_data * d)
{
    char    buf[MAX_COMMAND_LEN * 2];
    int     got;
    char   *p, *pend, *q, *qend;

    got = readsocket(d->descriptor, buf, sizeof buf);
    if(got > 0)
	tp_read_bytes += got;
    if (got <= 0) {
#ifdef DEBUGPROCESS
	fprintf(stderr, "process_input: read failed errno %d\n",
	    errnosocket);
#endif
	return 0;
    }
    if (!d->raw_input) {
	MALLOC(d->raw_input, char, MAX_COMMAND_LEN);
	d->raw_input_at = d->raw_input;
    }
    p = d->raw_input_at;
    pend = d->raw_input + MAX_COMMAND_LEN - 1;
    for (q = buf, qend = buf + got; q < qend; q++) {
	if (*q == '\n') {
	    *p = '\0';
	    if (p > d->raw_input) {
		/* Clear more line count if command received and not paging */
		if((!d->prompted) && d->moreline)
		    d->moreline = 0;
		save_command(d, d->raw_input);
	    } else {
		/* BARE NEWLINE! */
		/* log_status("TEST: d%d b%d *BARE NEWLINE*\n", d->descriptor, d->booted); */
		/* The more pager has priority, then readblankline */
		if(d->type == CT_MUCK) {
		    if(d->prompted) {
			d->moreline = 0;
			d->prompted = 0; /* Clear more prompt */
		    } else if(
			d->connected && OkObj(d->player) &&
			(FLAGS(d->player) & INTERACTIVE) &&
			(FLAGS(d->player) & READMODE) &&
			(FLAG2(d->player) & F2READBLANKLINE)
		    ) {
			save_command(d, " "); /* Note that a return was pressed */
		    }
		}
#ifdef HTTPD
		else if(d->type == CT_HTML) {
 #ifdef HTTPDELAY
		    /* If we get a bare return send httpdata now and end it */
		    if(d->httpdata) {
			queue_string(d, d->httpdata);
			free((void *)d->httpdata);
			d->httpdata = NULL;
			d->booted = 1;
		    }
 #endif /* HTTPDELAY */
		}
#endif /* HTTPD */
	    }
	    p = d->raw_input;
	} else if (p < pend && isascii(*q)) {
	    if (isprint(*q)) {
		*p++ = *q;
	    } else if (*q == '\t') {
		*p++ = ' ';
	    } else if (*q == 8 || *q == 127) {
		/* if BS or DEL, delete last character */
		if (p > d->raw_input) p--;
	    }
	}
    }
    if (p > d->raw_input) {
	d->raw_input_at = p;
    } else {
	FREE(d->raw_input);
	d->raw_input = 0;
	d->raw_input_at = 0;
    }
    return 1;
}

void 
set_userstring(char **userstring, const char *command)
{
    if (*userstring) {
	FREE(*userstring);
	*userstring = 0;
    }
    while (*command && isascii(*command) && isspace(*command))
	command++;
    if (*command)
	*userstring = strsave(command);
}

void 
process_commands(void)
{
    int     nprocessed;
    struct descriptor_data *d, *dnext;
    struct text_block *t;

    do {
	nprocessed = 0;
	for (d = descriptor_list; d; d = dnext) {
	    dnext = d->next;
	    if (d->quota > 0 && (t = d->input.head)
	       && (!(d->connected && DBFETCH(d->player)->sp.player.block))) {
		d->quota--;
		nprocessed++;
		if (!do_command(d, t->start)) d->booted = 2;
		/* start former else block */
		d->input.head = t->nxt;
		d->input.lines--;
		if (!d->input.head) {
		    d->input.tail = &d->input.head;
		    d->input.lines = 0;
		}
		free_text_block(t);
		/* end former else block */
	    }
	}
    } while (nprocessed > 0);
}

int 
do_command(struct descriptor_data * d, char *command)
{
    if (d->connected)
	ts_lastuseobject(d->player);

    (d->commands)++;
    if (!strcmp(command, QUIT_COMMAND)) {
	return 0;
    } else if( !strncmp(command, "!WHO", sizeof("!WHO") - 1)) {
	if(!d->connected) log_status("!WHO: %2d %s(%s) %s '%s' %d cmds\n",
	    d->descriptor, d->hostname, d->username,
	    host_as_hex(d->hostaddr), command, d->commands);
	if ((!d->connected) && (reg_site_is_barred(d->hostaddr) == TRUE)) {
		queue_string(d, "Lots of people are on now.\r\n");
	} else if (tp_secure_who && (!d->connected || !TMage(d->player))) {
		queue_string(d, "Connect and find out!\r\n");
	} else {
		dump_users(d, command + sizeof("!WHO") - 1);
	}
    } else if (!strncmp(command, WHO_COMMAND, sizeof(WHO_COMMAND) - 1)) {
	char buf[BUFFER_LEN];

	if(!d->connected) log_status(" WHO: %2d %s(%s) %s '%s' %d cmds\n",
	    d->descriptor, d->hostname, d->username,
	    host_as_hex(d->hostaddr), command, d->commands);

	if (d->output_prefix) {
	    queue_string(d, d->output_prefix);
	    queue_write(d, "\r\n", 2);
	}
	strcpy(buf, "@");
	strcat(buf, WHO_COMMAND);
	strcat(buf, " ");
	strcat(buf, command + sizeof(WHO_COMMAND) - 1);
	if (!d->connected || (FLAGS(d->player) & INTERACTIVE)) {
	    if (tp_secure_who || (reg_site_is_barred(d->hostaddr) == TRUE)) {
		queue_string(d,"Lots of people are on now.\r\n");
	    } else {
		dump_users(d, command + sizeof(WHO_COMMAND) - 1);
	    }
	} else {
	    if (can_move(d->player, buf, LM1 + 1)) {
		do_move(d->player, buf, LM1 + 1);
	    } else {
		dump_users(d, command + sizeof(WHO_COMMAND) - 1);
	    }
	}
	if (d->output_suffix) {
	    queue_string(d, d->output_suffix);
	    queue_write(d, "\r\n", 2);
	}
    } else if (!strncmp(command, PREFIX_COMMAND, sizeof(PREFIX_COMMAND) - 1)) {
	set_userstring(&d->output_prefix, command + sizeof(PREFIX_COMMAND) - 1);
    } else if (!strncmp(command, SUFFIX_COMMAND, sizeof(SUFFIX_COMMAND) - 1)) {
	set_userstring(&d->output_suffix, command + sizeof(SUFFIX_COMMAND) - 1);
    } else if (
	d->connected &&	OkObj(d->player) &&
	(!strncmp(command, MORE_COMMAND, sizeof(MORE_COMMAND) - 1))
    ) {
	do_more(d->player, command + sizeof(MORE_COMMAND) - 1);
#ifndef NO_CONNECT_HOOK
    } else if (
	d->connected &&	OkObj(d->player) &&
	(!strncmp(command, CONNECT_COMMAND, sizeof(CONNECT_COMMAND) - 1))
    ) {
	char *password;
	dbref player;

	command += sizeof(CONNECT_COMMAND);
	while(*command == ' ')
	    command++;
	password = command;
	while(*password > ' ')
	    password++;
	if(*password == ' ')
	    *(password++) = '\0';

	player = lookup_player(command);
	
	if((!OkObj(player)) || (!*password) ||
	    (!DBFETCH(player)->sp.player.password) ||
	    (!*DBFETCH(player)->sp.player.password) ||
	    strcmp(password, DBFETCH(player)->sp.player.password)
	) {
	    queue_string(d, "Login incorrect.\r\n");
	} else {
	    if(!dset_user(d, player))
		queue_string(d, "Failed.\r\n");
	}
#endif /* !NO_CONNECT_HOOK */
    } else if (tp_pueblo_support && !strncmp(command, PUEBLO_COMMAND, sizeof(PUEBLO_COMMAND) - 1)) {
	d->type = CT_PUEBLO;
	if(d->connected && OkObj(d->player))
	    FLAG2(d->player) |= F2PUEBLO;
	queue_string(d, tp_pueblo_message);
	queue_string(d, "\r\n");
    } else {
	if (d->connected) {
	    if (d->output_prefix) {
		queue_string(d, d->output_prefix);
		queue_write(d, "\r\n", 2);
	    }
	    process_command(d->player, command, 0);
	    if (d->output_suffix) {
		queue_string(d, d->output_suffix);
		queue_write(d, "\r\n", 2);
	    }
	} else {
	    if(tp_unix_login && (d->type == CT_MUCK)) {
		if(strchr(command, ' ')) {

		    d->player = 0;
		    check_connect(d, command);

		} else if(!d->player) { /* It's a name! */

		    d->player = lookup_player(command);
		    queue_string(d, "Password: " TELNET_ECHO_OFF);

		} else { /* It's a password! */
		    char buf[BUFFER_LEN];

		    sprintf(buf, "connect %.64s %.64s",
			OkObj(d->player) ? NAME(d->player) : "<unknown>",
			command
		    );
		    d->player = 0;
		    check_connect(d, buf);
		}
	    } else check_connect(d, command);
	}
    }
    return 1;
}

void 
interact_warn(dbref player)
{
    if (FLAGS(player) & INTERACTIVE) {
	char    buf[BUFFER_LEN];

	sprintf(buf, CINFO "***  %s  ***",
		(FLAGS(player) & READMODE) ?
		"You are currently using a program.  Use \"" BREAK_COMMAND
		"\" to return to a more reasonable state of control." :
		(DBFETCH(player)->sp.player.insert_mode ?
		 "You are currently inserting MUF program text.  Use \".\" to return to the editor, then \"quit\" if you wish to return to your regularly scheduled Muck universe." :
		 "You are currently using the MUF program editor."));
	anotify(player, buf);
    }
}

#ifdef HTTPD
void
httpd_unknown(struct descriptor_data *d, const char *name) {
    /* First try custom error on www_root, else print default message */

    if(httpd_get_lsedit(d, tp_www_root, "http/404") <= 0) {
	queue_string(d,
	    "<HTML>\r\n<HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY>\r\n"
	    "<H1>404 Not Found</H1>\r\n"
	    "The requested URL "
	);
	queue_string(d, name);
	queue_string(d,
	    " was not found on this server.<P>\r\n</BODY></HTML>\r\n"
	);
    }
    d->booted = 3;
}

int
httpd_get_lsedit(struct descriptor_data *d, dbref what, const char *prop) {
    const char *m = NULL;
    int   lines = 0;
    char  buf[BUFFER_LEN];

    if(!OkObj(what))
	return 0;
    
    if (*prop && (Prop_Hidden(prop) || Prop_Private(prop)))
	return 0;

    while((*prop == '/') || (*prop == ' '))
	prop++;

    while ( (lines < 256) && (!lines || (m && *m)) ) {
	if(*prop)
	    sprintf(buf, WWWDIR "/%.512s#/%d", prop, ++lines);
	else
	    sprintf(buf, WWWDIR "#/%d", ++lines);
	m = get_property_class(what, buf);
#ifdef COMPRESS
	if(m && *m) m = uncompress(m);
#endif
	if( m && *m ) {
	    sprintf(buf, "%.512s\r\n", m);
	    queue_string(d, buf);
	}
    }

    return lines - 1;
}

void
httpd_get(struct descriptor_data *d, const char *name, const char *http) {
    const char *prop = NULL, *m = NULL, **dit = infotext;
    dbref   what, prog;
    int lines = 0, cold = CT_MUCK;
    char    buf[BUFFER_LEN], *argstr;

    while(*name == '/')
	name++;

#ifdef HTTPDELAY
    if( d->httpdata ) {
	free((void *)d->httpdata);
	d->httpdata = NULL;
    }
#endif /* HTTPDELAY */

    if (*name == '@') {
	name++;
	if( string_prefix("credits", name) ) {
	    queue_string(d, "<HTML>\r\n<HEAD><TITLE>TinyMuck Credits</TITLE></HEAD>\r\n");
	    queue_string(d, "<H1>TinyMuck Credits</H1>\r\n");
	    queue_string(d, "<BODY><PRE>\r\n");
	    while(*dit) {
		unparse_ansi(buf, *dit++, 2);
		queue_string(d, buf);
		queue_string(d, "\r\n");
	    }
	    queue_string(d, "</PRE><hr><p><A HREF=\"/\">Go back.</A><p>\r\n");
	    queue_string(d, "</BODY></HTML>\r\n");
	    d->booted = 3;
	} else if ( string_prefix("help", name) ) {
	    queue_string(d, "<HTML>\r\n<HEAD><TITLE>Connection Help</TITLE></HEAD>\r\n");
	    queue_string(d, "<BODY><PRE>\r\n");
		cold = d->type;
		d->type = CT_MUCK;
		help_user(d);
		d->type = cold;
	    queue_string(d, "</PRE><hr><p><A HREF=\"");
	    queue_string(d, tp_html_parent_link);
	    queue_string(d, "\">Go back.</A><p>\r\n");
	    queue_string(d, "</BODY></HTML>\r\n");
	    d->booted = 3;
	} else if ( string_prefix("who", name) ) {
	    queue_string(d, "<HTML>\r\n<HEAD><TITLE>Who's on now?</TITLE></HEAD>\r\n");
	    queue_string(d, "<H1>Who's on now?</H1>\r\n");
	    queue_string(d, "<BODY><PRE>\r\n");
	    if (tp_secure_who)
		queue_string(d, "Connect and find out!\r\n");
	    else
		dump_users(d, "");
	    queue_string(d, "</PRE><hr><p><A HREF=\"");
	    queue_string(d, tp_html_parent_link);
	    queue_string(d, "\">Go back.</A><p>\r\n");
	    queue_string(d, "</BODY></HTML>\r\n");
	    d->booted = 3;
	} else if ( string_prefix("welcome", name) ) {
	    queue_string(d, "<HTML>\r\n<HEAD><TITLE>Welcome!</TITLE></HEAD>\r\n");
	    queue_string(d, "<H1>Welcome!</H1>\r\n");
	    queue_string(d, "<BODY><PRE>\r\n");
		cold = d->type;
		d->type = CT_MUCK;
		welcome_user(d);
		d->type = cold;
	    queue_string(d, "</PRE><hr><p><A HREF=\"");
	    queue_string(d, tp_html_parent_link);
	    queue_string(d, "\">Go back.</A><p>\r\n");
	    queue_string(d, "</BODY></HTML>\r\n");
	    d->booted = 3;
	} else {
	    httpd_unknown(d, name);
	}
	return;
    } else if (tp_www_player_pages && (*name == '~')) { 
	prop = ++name;
	while (*prop && (*prop != '?') && (*prop != '/') && ((prop-name) < (BUFFER_LEN - 1))) {
            prop++;
	    buf[prop-name] = (*prop);
        }
	buf[prop-name] = '\0';
	if (*buf) {
	    what = lookup_player(buf);
	    
	    if(OkObj(what) &&
	       (tp_restricted_www) &&
	       (!(FLAG2(what) & F2WWW))
	    ) {
		/* This player isn't allowed to have a web page */
		httpd_unknown(d, name);
		return;
	    }
	} else
	    what = NOTHING;
	while (*prop == '/') prop++;
    } else {
	prop = name;
	what = tp_www_root;
    }
    if ( !OkObj(what) ) {
	httpd_unknown(d, name);
	return;
    }
    /* First check if there is an lsedit style list here */

    lines = httpd_get_lsedit(d, what, prop);
    if ( lines < 1 ) {
	/* There was no lsedit list, try a relocation */
	if(*prop && ((strlen(prop) + strlen(WWWDIR)) < (BUFFER_LEN - 1))) {
	    sprintf(buf, WWWDIR "/%s", prop);
	} else
	    strcpy(buf, WWWDIR);

	argstr = buf;
	while((*argstr) && (*argstr != '?'))
	    argstr++;

	if(*argstr)
	    (*(argstr++)) = '\0';

	m = get_property_class(what, buf);
#ifdef COMPRESS
	if(m && *m) m = uncompress(m);
#endif
	if(m && *m) {
	    if( (*m == '#') &&
		OkObj(tp_www_user) &&
		(Typeof(tp_www_user) == TYPE_PLAYER) &&
		(!d->connected) &&
		((prog = atoi(m + 1)) > 0) &&
		(OkObj(prog)) &&
		(Typeof(prog) == TYPE_PROGRAM) &&
		(FLAGS(prog) & LINK_OK) &&
		((!tp_restricted_www) || (FLAG2(prog) & F2WWW))
	    ) {
		/* This is a dbref -> CGI script */
		int flags, flag2;
		    
		/* Make sure our user is set up right */
		flags = FLAGS(tp_www_user);
		flag2 = FLAG2(tp_www_user);
		    
		FLAGS(tp_www_user) = TYPE_PLAYER;
		FLAG2(tp_www_user) = F2GUEST;

		d->connected = 1;
		d->player = tp_www_user;
		update_desc_count_table();
		remember_player_descr(d->player, d->descriptor);
		    
		sprintf(match_cmdname,
		    "(Web) %d %d %.512s",
		    d->descriptor,
		    d->hostaddr,
		    buf
		);
		strcpy(match_args, argstr);
		    
		/* We appear to have a valid program */
		interp(tp_www_user, what,
		    prog, tp_www_user, PREEMPT, STD_HARDUID, 0);

		match_cmdname[0] = '\0';
		match_args[0] = '\0';

		forget_player_descr(d->player, d->descriptor);
		d->connected = 0;
		d->player = NOTHING;
		update_desc_count_table();

		FLAGS(tp_www_user) = flags;
		FLAG2(tp_www_user) = flag2;

	    } else if( (*m == '&') && (m[1]) &&
		OkObj(tp_www_user) &&
		(Typeof(tp_www_user) == TYPE_PLAYER) &&
		(!d->connected)
	    ) {
		/* This is an MPI -> CGI script */
		int flags, flag2;
		char buf2[BUFFER_LEN];
		    
		/* Make sure our user is set up right */
		flags = FLAGS(tp_www_user);
		flag2 = FLAG2(tp_www_user);
		    
		FLAGS(tp_www_user) = TYPE_PLAYER;
		FLAG2(tp_www_user) = F2GUEST | F2MPI;
		    
		d->connected = 1;
		d->player = tp_www_user;
		update_desc_count_table();
		remember_player_descr(d->player, d->descriptor);
		    
		sprintf(match_cmdname,
		    "(Web) %d %d %.512s",
		    d->descriptor,
		    d->hostaddr,
		    buf
		);
		strcpy(match_args, argstr);
		    
		argstr = do_parse_mesg(tp_www_user, what, m + 1, "(Web)",
		    buf2, MPI_ISPRIVATE | MPI_ISLOCK);
		if(argstr && *argstr)
		    notify(tp_www_user, argstr);

		match_cmdname[0] = '\0';
		match_args[0] = '\0';

		forget_player_descr(d->player, d->descriptor);
		d->connected = 0;
		d->player = NOTHING;
		update_desc_count_table();

		FLAGS(tp_www_user) = flags;
		FLAG2(tp_www_user) = flag2;

	    } else {
#ifdef HTTPDELAY
		sprintf(buf, "HTTP/1.0 302 Found\r\n"
		    "Status: 302 Found\r\n"
		    "Location: %.70s\r\n"
		    "Content-type: text/html\r\n\r\n"
		    "<HTML>\r\n<HEAD><TITLE>302 Found</TITLE></HEAD>\r\n"
		    "<H1>302 Found</H1>\r\n"
		    "Your browser doesn't seem to support redirection.<P>\r\n"
		    "Try clicking <A HREF=\"%.70s\">HERE</A>.\r\n</HTML>\r\n",
		    m, m
		);
		d->httpdata = string_dup(buf);
#else
		queue_string(d, "HTTP/1.0 302 Found\r\n"
		    "Status: 302 Found\r\n"
		    "Location: "
		);
		queue_string(d, m);
		queue_string(d, "\r\nContent-type: text/html\r\n\r\n"
		    "<HTML>\r\n<HEAD><TITLE>302 Found</TITLE></HEAD>\r\n"
		    "<H1>302 Found</H1>\r\n"
		    "Your browser doesn't seem to support redirection.<P>\r\n"
		    "Try clicking <A HREF=\""
		);
		queue_string(d, m);
		queue_string(d, "\">HERE</A>.\r\n</HTML>\r\n");
#endif /* HTTPDELAY */
	    }
	    if (!d->booted)
		d->booted = 3;
	    /* if (!*http)
		d->booted = 3; */
	} else
	    httpd_unknown(d, prop);

    } else {
	process_output(d);
	d->booted = 3;
    }
}
#endif /* HTTPD */

void 
check_connect(struct descriptor_data * d, const char *msg)
{
    char    command[80];
    char    user[80];
    char    password[80];
    const   char *why;
#ifdef HTTPD
    char    *ndst, *nsrc;
#endif /* HTTPD */
    dbref   player;
    const char *spw, *ppw;
    int quiet=0;
    time_t now;

    if(d->connected)
	return;
    
    now = time(NULL);

    if( tp_log_connects )
	log2filetime(CONNECT_LOG, "%2d: %s\r\n", d->descriptor, msg );

    if(*msg == '-') {
	msg++;
	quiet=1;
    }

    if(tp_unix_login && (d->type == CT_MUCK))
	queue_string(d, "\r\n" TELNET_ECHO_ON);

    parse_connect(msg, command, user, password);

    if (!strncmp(command, "cr", 2)) {
	if (!tp_registration) {
	    if (localhost_mode || wizonly_mode || (tp_playermax && con_players_curr >= tp_playermax_limit)) {
		if (localhost_mode || wizonly_mode) {
		    queue_string(d, "Sorry, but ");
		    queue_string(d, tp_muckname);
		    queue_string(d, " is in maintenance mode currently, and only " NAMEWIZ "s are allowed to connect.  Try again later.");
		} else {
		    queue_string(d, tp_playermax_bootmesg);
		}
		queue_string(d, "\r\n");
		d->booted = 1;
		log_status("XMCR: %2d %s pw '%s' %s(%s) %s\n",
		    d->descriptor, user, password,
		    d->hostname, d->username,
		    host_as_hex(d->hostaddr));
		return;
	    } else if( reg_site_can_request(d->hostaddr) != TRUE ) {
		char buf[BUFFER_LEN];

		sprintf(buf, CFG_REG_BLK, "creations", tp_register_mesg);
		queue_string(d, buf);
		return;
	    } else if(
		current_systime < (domain_lastreq(d->hostaddr) + tp_registration_wait)
	    ) {
		char buf[BUFFER_LEN];

		queue_string(d, "Wow, business is booming!  "
		  "We recently received another creation from someone\r\nnear you.  "
		  "Please come back in "
		);
		sprintf(buf, "%d", (int)
		    (((domain_lastreq(d->hostaddr) + tp_registration_wait - current_systime)
		    / 60) + 1)
		);
		queue_string(d, buf);
		queue_string(d, " minutes and try your request again.\r\n");
		return;
	    } else {
		player = create_player(user, password);
		if (player == NOTHING) {
		    queue_string(d, create_fail);
		    log_status("FCRE: %2d %s pw '%s' %s(%s) %s\n",
			d->descriptor, user, password,
			d->hostname, d->username,
			host_as_hex(d->hostaddr));
		    return;
		} else {
		    add_property(player, PROP_ID, d->hostname, 0 );
		    host_touch_lastreq(d->hostaddr);
		    log_status("CRE8: %2d %s %s(%s) %s\n",
			d->descriptor, NAME(player),
			d->hostname, d->username,
			host_as_hex(d->hostaddr));
		    strcpy(command, "connect");
		}
	    }
	} else {
	    char buf[ 1024 ];
	    sprintf( buf, CFG_REG_CRE, tp_register_mesg );
	    queue_string( d, buf );
	    log_status("FCRE: %2d %s pw '%s' %s(%s) %s\n",
		d->descriptor, user, password,
		d->hostname, d->username,
		host_as_hex(d->hostaddr));
	    return;
	}
    }

#ifdef HTTPD
    if (d->type == CT_HTML) {
	/* log_status("TEST: HTTP b%d '%s' '%s' '%s'\n", d->booted, command, user, password); */
	if(d->booted > 0)
	    return;
	if (!strcmp(command, "GET")) {
	    char buf[BUFFER_LEN];

	    /* Attempt to get more of the address for CGI */
	    /* The full message should be less than MAX_COMMAND_LEN */
	    /* which is 1/4 of BUFFER_LEN, thus no checking here.   */
	    nsrc = (char *) msg;
	    while(nsrc[0] > ' ') /* Run through GET */
		nsrc++;
	    while(nsrc[0] == ' ') /* Past GET */
		nsrc++;
	    ndst = nsrc;
	    while(ndst[0] > ' ') /* Run through address */
		ndst++;
	    strncpy(buf, nsrc, ndst - nsrc); /* Copy full address */
	    buf[ndst - nsrc] = '\0';

	    /* Strip useless /'s and other characters */
	    nsrc = ndst = buf;
	    while(*nsrc) {
		if((nsrc > ndst) && (*ndst != '/') && (*nsrc == '/'))
		    *(ndst++) = *(nsrc++);
		while((*nsrc) && (
		   (*nsrc == '\\') || (*nsrc == ':') ||
		   (*nsrc <  ' ' ) || (*nsrc >  '~')
		)) nsrc++;
		if(*nsrc)
		    *(ndst++) = *(nsrc++);
	    }
	    /* Strip trailing / if it's there */
	    if((ndst > buf) && ((*(ndst - 1)) == '/'))
		ndst--;
	    *ndst = '\0';
	    if(tp_log_http)
	      log_http(" GET: '%s' '%s' %s(%s) %s\n",
		buf, password, d->hostname, d->username,
		host_as_hex(d->hostaddr)
	      );
	    httpd_get(d, buf, password);
	} else if (index(msg, ':')) {
		/* Ignore http request fields */
	} else {
	    if(tp_log_http)
	      log_http("BAD: '%s' '%s' %s(%s) %s\n",
		user, password, d->hostname, d->username,
		host_as_hex(d->hostaddr)
	      );
	    queue_string(d,
		"<HTML>\r\n<HEAD><TITLE>400 Bad Request</TITLE></HEAD><BODY>\r\n"
		"<H1>400 Bad Request</H1>\r\n"
		"You sent a query this server doesn't understand.<P>\r\n"
		"Reason: Unknown method.<P>\r\n"
		"</BODY></HTML>\r\n"
	    );
	    d->booted = 3;
	}
    } else
#endif /* HTTPD */
    if (!strncasecmp(command, "co", 2)) {
	player = connect_player(user, password);
	if((now - d->last_fail) < tp_fail_wait) {
	    char buf[ 1024 ];
	    int wait;
	    wait = tp_fail_wait - (now - d->last_fail);
	    sprintf( buf, "\r\nPlease wait %d second%s before attempting another connection.\r\n",
		wait, (wait==1) ? "" : "s"
	    );
	    queue_string( d, buf );
	    log_status("FAST: %2d %s pw '%s' %s(%s) %s\n",
		d->descriptor, user, password,
		d->hostname, d->username,
		host_as_hex(d->hostaddr));
	} else if (player == NOTHING) {
	    d->fails++;
	    d->last_fail = now;
	    queue_string(d, connect_fail);
	    if( d->fails >= tp_fail_retries ) d->booted = 1;
	    log_status("FAIL: %2d %s pw '%s' %s(%s) %s\n",
		d->descriptor, user, password,
		d->hostname, d->username,
		host_as_hex(d->hostaddr));

	} else if ( (why=reg_user_is_suspended( player )) ) {
	    queue_string(d,"\r\n" );
	    queue_string(d,"You are temporarily suspended: " );
	    queue_string(d,why );
	    queue_string(d,"\r\n" );
	    queue_string(d,"Please contact " );
	    queue_string(d, tp_register_mesg );
	    queue_string(d," for assistance if needed.\r\n" );
	    log_status("*LOK: %2d %s %s(%s) %s %s\n",
		d->descriptor, unparse_object(player, player),
		d->hostname, d->username,
		host_as_hex(d->hostaddr), why);
	    d->booted = 1;
	} else if (reg_user_is_barred( d->hostaddr, player ) == TRUE) {
	    char buf[ 1024 ];
	    sprintf( buf, CFG_REG_MSG, tp_register_mesg );
	    queue_string( d, buf );
	    log_status("*BAN: %2d %s %s(%s) %s\n",
		d->descriptor, unparse_object(player, player),
		d->hostname, d->username,
		host_as_hex(d->hostaddr));
	    d->booted = 1;
	} else if (tp_mortals_need_id_prop && !TMage(player) && !get_property_class(player, PROP_ID)) {
	    /* Player has no @/id property set */
	    char buf[ 1024 ];
	    sprintf( buf, CFG_NO_ID, tp_register_mesg );
	    queue_string( d, buf );
	    log_status("NOID: %2d %s %s(%s) %s\n",
		d->descriptor, unparse_object(player, player),
		d->hostname, d->username,
		host_as_hex(d->hostaddr));
	    d->booted = 1;
	} else if ((wizonly_mode ||
		 (tp_playermax && con_players_curr >= tp_playermax_limit)) &&
		!TMage(player)
	    ) {
		if (wizonly_mode) {
		    queue_string(d, "Sorry, but the game is in maintenance mode currently, and only " NAMEWIZ "s are allowed to connect.  Try again later.");
		} else {
		    queue_string(d, tp_playermax_bootmesg);
		}
		queue_string(d, "\r\n");
		d->booted = 1;
	} else if (
	    localhost_mode &&
	    (d->hostaddr != 0x7f000001) &&
	    ( (listen_address == INADDR_ANY) ||
	      (d->hostaddr != ntohl(listen_address))
	    )
	) {
		queue_string(d, "Sorry, but the game is in maintenance mode currently, and only " NAMEWIZ "s are allowed to connect.  Try again later.\r\n");
		d->booted = 1;
	} else {
	    const char *am;

	    log_status("CONN: %2d %s %s(%s) %s\n",
		d->descriptor, unparse_object(player, player),
		d->hostname, d->username,
		host_as_hex(d->hostaddr));
	    d->connected = 1;
	    /* This looks goofy in WHO if one person sits on the login screen */
	    /* d->connected_at = current_systime; */
	    d->player = player;
	    update_desc_count_table();
	    remember_player_descr(player, d->descriptor);
	    if(d->type == CT_PUEBLO)
		FLAG2(d->player) |= F2PUEBLO;
	    else
		FLAG2(d->player) &= ~F2PUEBLO;

	    /* cks: someone has to initialize this somewhere. */
	    DBFETCH(d->player)->sp.player.block = 0;

	    /* Send motd unless user does '-connect' */
	    if(!quiet) spit_file(player, MOTD_FILE);

	    if ( TWiz(player) ) {
		char buf[ 128 ];
		int count = hop_count();
		if(count > 0) {
		  sprintf( buf, CNOTE MARK
		    "There %s %d registration%s in the hopper.",
		    (count==1)?"is":"are", count, (count==1)?"":"s" );
		  anotify( player, buf );
		}
	    }

	    interact_warn(player);
	    if (sanity_violated && TMage(player)) {
		anotify(player, CNOTE MARK CFAIL "WARNING!  "
		    CNOTE "The DB appears to be corrupt!"
		);
	    }

	    if(Guest(player)) {
		FLAGS(player) &= ~CHOWN_OK;
		if(tp_online_registration) {
		    anotify(player, CNOTE MARK "You can request your own character while online.");
		    anotify(player, CNOTE "    Type 'request' to see what to do.");
		}
		anotify(player, CNOTE MARK "ANSI color is available by typing 'color on'.");
	    }

	    spw = get_property_class(player, PROP_PW);
#ifdef COMPRESS
	    if(spw && *spw) spw = uncompress(spw);
#endif
	    ppw = DBFETCH(player)->sp.player.password;
	    if (spw && *spw && ppw && *ppw && !strcmp(spw, ppw)) {
		anotify(player, CNOTE MARK CFAIL "WARNING!  " CNOTE
		    "Your password should be changed from the one sent to you."
		);
		anotify_fmt(player, CNOTE "    To change it, type:  "
		    CSUCC "@password %s=SomeNewPassword", password
		);
	    }

	    if(tp_pause_after_motd) {
		am = get_property_class(player, PROP_MOTDPAUSE);
#ifdef COMPRESS
		if(am && *am) am = uncompress(am);
#endif
		if(!am || !*am) {
		    int len = 0;
		    if (!process_output(d)) d->booted = 1;
		    len = writesocket(d->descriptor, MOTD_MESSAGE, strlen(MOTD_MESSAGE));
		    if(len > 0)
			tp_write_bytes += len;
		    d->prompted = 1;
		}
	    }

	    announce_connect(player);

	    /* Boot any extra connections past max_player_logins */
	    /* Do it here because the old connection gets spammed anyway */
	    if(tp_max_player_logins > 0)
		if(online(player) > tp_max_player_logins)
		    boot_idlest(player, 0);

	}
    } else if (!strncasecmp(command, "h", 1) ) { /* Connection Help */
	log_status("HELP: %2d %s(%s) %s %d cmds\n",
	    d->descriptor, d->hostname, d->username,
	    host_as_hex(d->hostaddr), d->commands);
	help_user(d);
    } else if (!strncasecmp(command, "n", 1) ) { /* Connection Notes */
	log_status("NOTE: %2d %s(%s) %s %d cmds\n",
	    d->descriptor, d->hostname, d->username,
	    host_as_hex(d->hostaddr), d->commands);
	if(desc_file(0, d, NOTE_FILE))
	    queue_string(d, "The request note file is empty.\r\n");
    } else if (!strncasecmp(command, "r", 1) ) {
	log_status("RQST: %2d %s(%s) %s %d cmds\n",
	    d->descriptor, d->hostname, d->username,
	    host_as_hex(d->hostaddr), d->commands);
	if( request(-1, d, msg) ) d->booted = 1;
    } else {
	log_status("TYPO: %2d %s(%s) %s '%s' %d cmds\n",
	    d->descriptor, d->hostname, d->username,
	    host_as_hex(d->hostaddr), command, d->commands);
	welcome_user(d);
    }

    if(tp_unix_login && (d->type == CT_MUCK) && (!d->connected) && (!d->player) && (!d->booted))
	queue_string(d, "\r\nlogin: ");
}

void 
parse_connect(const char *msg, char *command, char *user, char *pass)
{
    int cnt;
    char   *p;

    while (*msg && isascii(*msg) && isspace(*msg)) /* space */
	msg++;
    p = command;
    cnt = 0;
    while (*msg && isascii(*msg) && !isspace(*msg) && (++cnt < 80))
	*p++ = *msg++; 
    *p = '\0'; 
    cnt = 0;
    while (*msg && isascii(*msg) && isspace(*msg)) /* space */
	msg++;
    p = user;
    cnt = 0;
    while (*msg && isascii(*msg) && !isspace(*msg) && (++cnt < 80))
	*p++ = *msg++;
    *p = '\0';
    while (*msg && isascii(*msg) && isspace(*msg)) /* space */
	msg++;
    p = pass;
    cnt = 0;
    while (*msg && isascii(*msg) /* && !isspace(*msg) */ && (++cnt < 80))
	*p++ = *msg++;
    *p = '\0';
}


int 
boot_off(dbref player)
{
    struct descriptor_data *d;
    struct descriptor_data *last = NULL;

    for (d = descriptor_list; d; d = d->next)
	if (d->connected && d->player == player && !d->booted)
	    last = d;

    if (last) {
	process_output(last);
	last->booted = 1;
	/* shutdownsock(last); */
	return 1;
    }
    return 0;
}

int 
boot_welcome_site_idlest(int site)
{
    struct descriptor_data *d, *mostidle = NULL;
    time_t mostidletime, now;
    int count;

    if(tp_max_site_welcomes < 1)
	return 0;

    site &= ~0xff; /* Mask lowest 256 addresses */
    
    now = current_systime;
    mostidletime = now;

    for (count = 0, d = descriptor_list; d; d = d->next)
	if ((!d->connected) && ((d->hostaddr & ~0xff) == site) && !d->booted) {
	    count++;
	    if(d->last_time <= mostidletime) {
		mostidletime = d->last_time;
		mostidle = d;
	    }
	}

    if((count <= 1) || (count <= tp_max_site_welcomes) || !mostidle)
	return 0;

    idleboot_user(mostidle);
    return 1;
}

int 
boot_idlest(dbref player, int keep_only_one)
{
    struct descriptor_data *d, *mostidle = NULL, *leastidle = NULL;
    time_t mostidletime, leastidletime, now;
    
    now = current_systime;
    mostidletime = now;
    leastidletime = 0;

    for (d = descriptor_list; d; d = d->next)
	if (d->connected && (d->player == player) && !d->booted) {
	    if(d->last_time <= mostidletime) {
		mostidletime = d->last_time;
		mostidle = d;
	    }
	    if(d->last_time > leastidletime) {
		leastidletime = d->last_time;
		leastidle = d;
	    }
	}

    if((!mostidle) || (!leastidle) || (mostidle == leastidle))
	/* Found none or just one */
	return 0;

    if(!keep_only_one) {
	idleboot_user(mostidle);
	return 1;
    }

    for (d = descriptor_list; d; d = d->next)
	if (d->connected && (d->player == player) && (d != leastidle) && !d->booted)
	    idleboot_user(d);

    return 1;
}

void 
boot_player_off(dbref player)
{
    int di;
    int *darr;
    int dcount;
    struct descriptor_data *d;

    if (Typeof(player) == TYPE_PLAYER) {
        darr = DBFETCH(player)->sp.player.descrs;
        dcount = DBFETCH(player)->sp.player.descr_count;
        if (!darr)
            dcount = 0;
    } else {
        dcount = 0;
        darr = NULL;
    }
  
    /* We need to be a tad more brutal as this player may be getting @toaded */
    for (di = 0; di < dcount; di++) {
        d = descrdata_by_index(darr[di]);
	if(d) {
	    forget_player_descr(d->player, d->descriptor);
	    d->connected = 0;
	    d->player = NOTHING;
            if (!d->booted)
		d->booted = 1;
        }
    }
    update_desc_count_table();
}


void
close_sockets(const char *msg)
{
    struct descriptor_data *d, *dnext;
    int socknum, len;

    for (d = descriptor_list; d; d = dnext) {
	dnext = d->next;
	len = writesocket(d->descriptor, msg, strlen(msg));
	if(len > 0)
	    tp_write_bytes += len;
	clearstrings(d);			/** added to clean up **/
	if (shutdown(d->descriptor, 2) < 0)
	    perror("shutdown");
	closesocket(d->descriptor);
	freeqs(d);				/****/
	*d->prev = d->next;			/****/
	if (d->next)				/****/
	    d->next->prev = d->prev;		/****/
	if (d->hostname)			/****/
	    free((void *)d->hostname);	  	/****/
	if (d->username)			/****/
	    free((void *)d->username);	  	/****/
	FREE(d);				/****/
	ndescriptors--;				/****/
    }
    for (socknum = 0; socknum < numsocks; socknum++) {
	closesocket(sock[socknum]);
    }
}

struct descriptor_data *
get_descr(int descr, dbref player)
{
    struct descriptor_data *next=descriptor_list;

    while(next) {
	if ( (OkObj(player) && (next->player == player)) ||
	     ((descr  > 0) && (next->descriptor == descr)) ) return next;
	next = next->next;
    }
    return NULL;
}

void
do_dinfo(dbref player, const char *arg)
{
    struct descriptor_data *d;
    int who, descr;
    char *ctype = NULL;
    time_t now;

    if (!Mage(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if(!*arg) {
	anotify(player, CINFO "Usage: dinfo descriptor  or  dinfo name");
	return;
    }
    (void) time(&now);
    if(strcmp(arg, "me"))
	who = lookup_player(arg);
    else who = player;
    descr = atoi(arg);
    if (!( d = get_descr(descr, who))) {
	anotify(player, CINFO "Invalid descriptor or user not online.");
	return;
    }

    switch(d->type) {
	case CT_MUCK:
		ctype = "muck"; break;
	case CT_HTML:
		ctype = "html"; break;
	default:
		ctype = "unknown";
    }

    anotify_fmt(player, "%s" CAQUA " descr " CYELLOW "%d" CBLUE " (%s)",
	d->connected ? ansi_unparse_object(player, d->player) :
	CSUCC "<Connecting>",
	d->descriptor, ctype
    );

    if(Wiz(player))
	anotify_fmt(player, CAQUA "Host: " CCYAN "%s" CBLUE "@" CCYAN "%s"
		CAQUA "  LMA: " CCYAN "%d %s",
		d->username, d->hostname, host_last_min_accepts(d->hostaddr),
		d->resolved ? CBLUE "(resolved)" : ""
	);
    else
	anotify_fmt(player, CAQUA "Host: " CCYAN "%s",
		hostname_domain(d->hostname));

    if(Wiz(player))
	anotify_fmt(player, CAQUA "IP: " CCYAN "%s" CYELLOW "(%d) " CNAVY "%X",
		host_as_hex(d->hostaddr), d->port, d->hostaddr);

    anotify_fmt(player, CVIOLET "Online: " CPURPLE "%s  " CBROWN "Idle: "
	CYELLOW "%s, %s  " CCRIMSON "Commands: " CRED "%d  "
	CFOREST "Output: " CGREEN "%d",
	time_format_1(now - d->connected_at),
	d->idle ? "yes" : "no",
	time_format_2(now - d->last_time), d->commands, d->output_size
    );

    if (d->connected)
	anotify_fmt(player, CAQUA "Location: %s",
	    ansi_unparse_object(player, DBFETCH(d->player)->location));
}

void
do_dwall(dbref player, const char *name, const char *msg)
{
    struct descriptor_data *d;
    int who, descr;
    char buf[BUFFER_LEN];

    if (!Mage(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    switch(msg[0]) {
	case ':':
	case ';':
		sprintf(buf, MARK "%s %s\r\n", NAME(player), msg+1);
		break;
	case '@':
		sprintf(buf, MARK "%s\r\n", msg+1);
		break;
	case '!':
		sprintf(buf, "%s\r\n", msg+1);
		break;
	case '\0':
	case '#':
  		notify(player, "DWall Help");
  		notify(player, "~~~~~~~");
  		notify(player, "dw player=msg -- tell player 'msg'");
  		notify(player, "dw 14=message -- tell ds 14 'message'");
  		notify(player, "dw 14=:yerfs  -- pose 'yerfs' to ds 14");
		notify(player, "dw 14=@boo    -- spoof 'boo' to ds 14");
		notify(player, "dw 14=!boo    -- same as @ with no 'mark'");
		notify(player, "dw 14=#       -- this help list");
		notify(player, "Use WHO or WHO! to find ds numbers for players online.");
		return;
	default:
		sprintf(buf, MARK "%s tells you, \"%s\"\r\n", NAME(player), msg);
		break;
    }

    if(strcmp(name, "me"))
	who = lookup_player(name);
    else who = player;
    descr = atoi(name);
    if (!( d = get_descr(descr, who))) {
	anotify(player, CINFO "Invalid descriptor or user not online.");
	return;
    }
    queue_string(d, buf);
    if(!process_output(d)) d->booted = 1;
    anotify_fmt(player, CSUCC "Message sent to descriptor %d.", d->descriptor);
}

void
do_dboot(dbref player, const char *name)
{
    struct descriptor_data *d;
    int who, descr;

    if (!Wiz(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if(!*name) {
	anotify(player, CINFO "Usage: dboot descriptor  or  dboot name");
    }
    if(strcmp(name, "me"))
	who = lookup_player(name);
    else who = player;
    descr = atoi(name);
    if (!( d = get_descr(descr, who))) {
	anotify(player, CINFO "Invalid descriptor or user not online.");
	return;
    }

    d->booted = 1;
    anotify(player, CSUCC "Booted.");
}

void
do_dforce(dbref player, const char *name, const char *arg)
{
    struct descriptor_data *d;
    int descr;

    if (!Wiz(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if((!*name) || (!*arg)) {
	anotify(player, CINFO "Usage: dforce descriptor = command");
    }
    descr = atoi(name);
    if (!( d = get_descr(descr, NOTHING))) {
	anotify(player, CINFO "Invalid descriptor.");
	return;
    }

    if(d->connected && OkObj(d->player))
    {
	/* This descriptor is connected, use @force */
	anotify(player, CFAIL "Player is connected, use @force.");
    } else {
	/* This descriptor isn't connected, need to slip the command in */
	save_command(d, arg);
	anotify(player, CSUCC "Forced.");
    }
}

void
do_armageddon(dbref player, const char *msg)
{
    char buf[BUFFER_LEN];
    if (!Arch(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	log_status("SHAM: Armageddon by %s\n", unparse_object(player, player));
	return;
    }
    if( *msg == '\0' || strcmp(msg, tp_muckname))
    {
	anotify(player, CINFO "Usage: @armageddon muckname" );
	notify(player, "***WARNING*** All data since last save will be lost!" );
    	return;
    }
    sprintf(buf, "\r\nImmediate shutdown by %s.\r\n", NAME(player));
    log_status("DDAY: %s: %s\n",
	    unparse_object(MAN, player), msg);
    fprintf(stderr, "DDAY: %s(%d)\n",
	    NAME(player), player);
    close_sockets(buf);

#ifdef SPAWN_HOST_RESOLVER
    kill_resolver();
#endif

    exit(1);
}

int
request( dbref player, struct descriptor_data *d, const char *msg )
{
    char    command[80];
    char    user[80];
    char    password[80];
    char    email[80];
    char    firstname[80];
    char    lastname[80];
    char    fullname[161];
    char    *jerk;

    if( !d )
	if(!( d = get_descr(0, player) )) return 0;

    if( (reg_site_can_request(d->hostaddr) != TRUE) &&
	!((player >= 0) && Mage(player))
    ) {
	char buf[BUFFER_LEN];

	sprintf(buf, CFG_REG_BLK, "requests", tp_register_mesg);
	queue_string(d, buf);
	return 0;
    }

    if ((player >= 0) && !Guest(player) && !Mage(player)) {
	anotify(player, CFAIL "Only guests can request new characters.");
	return 0;
    }

    /* Wizzes don't have to wait, everyone else does */
    if( tp_online_registration ) {
    if( ((player >= 0) && (WLevel(player) >= LMAGE)) ||
	(current_systime > (domain_lastreq(d->hostaddr) + tp_registration_wait))
    ) {
	parse_connect(msg, command, user, password);
	parse_connect(password, email, firstname, lastname);
	if(!*user) {
	    queue_string(d, "To request a character type:\r\n\r\n"
		"   request <char name> <e-mail> <your name>   (Don't type the <> signs)\r\n\r\n"
		"<char name> is the name you'd like for your character\r\n"
		"   <e-mail> is your email address, ie: artie@muq.org\r\n"
		"<your name> is your first and last name in real life\r\n"
	    );
	    return 0;
	}
	if (!ok_player_name(user)) {
	    queue_string(d,
		"Sorry, that name is invalid or in use.  Try another?\r\n"
	    );
	} else if (!strchr(email,'@') || !strchr(email,'.')) {
	    queue_string(d,
		"That doesn't look like an email address.  Type just 'request' for help.\r\n"
	    );
	} else if ( *firstname == '\0' ) {
	    queue_string(d,
		"You forgot your name.  Type just 'request' for help.\r\n"
	    );
	} else if ( *lastname == '\0' ) {
	    queue_string(d,
		"You forgot your last name.\r\n"
	    );
	} else if ( strchr(firstname,'\'')
		 || strchr(lastname,'\'')
		 || strchr(email,'\'')
		 || strchr(user,'\'')  ) {
	    queue_string(d,
		"Please don't use single quotes in names.\r\n"
	    );
	} else if ( strchr(firstname,'`')
		 || strchr(lastname,'`')
		 || strchr(email,'`')
		 || strchr(user,'`')  ) {
	    queue_string(d,
		"Please don't use backquotes in names.\r\n"
	    );
	} else if ( strchr(firstname,'\"')
		 || strchr(lastname,'\"')
		 || strchr(email,'\"')
		 || strchr(user,'\"')  ) {
	    queue_string(d,
		"Please don't use quotes in names.\r\n"
	    );
	} else if ( strchr(firstname,'\\')
		 || strchr(lastname,'\\')
		 || strchr(email,'\\')
		 || strchr(user,'\\')  ) {
	    queue_string(d,
		"Please don't use backslashes in names.\r\n"
	    );
	} else if ( strchr(email,'<')
		 || strchr(email,'>')
		 || strchr(email,'/')
		 || strchr(email,';')
		 || strchr(email,'!')
		 || strchr(email,'|')
		 || strchr(email,'%')
		 || strchr(email,'&')
		 || strchr(email,':')  ) {
	    queue_string(d,
		"There are unacceptable characters in your email address.\r\n"
	    );
	} else {
	    jerk = reg_email_is_a_jerk(email) ? "##JERK## " : "";

#ifdef SPAWN_HOST_RESOLVER
	    resolve_hostnames(); /* See if we can get a real site name */
#endif

	    log2filetime( LOG_HOPPER, "'%s' %s'%s' '%s %s' %s(%s) %s\r\n",
		user, jerk, email, firstname, lastname,
		d->hostname, d->username, host_as_hex(d->hostaddr)
	    );

	    log_status(
		"*REG: %2d '%s' %s'%s' '%s %s' %s(%s) %s\n",
		d->descriptor, user, jerk, email, firstname, lastname,
		d->hostname, d->username, host_as_hex(d->hostaddr)
	    );

	    if( tp_fast_registration && !(*jerk) ) {
		sprintf(fullname, "%s %s", firstname, lastname);
		email_newbie(user, email, fullname);
		queue_string(d, "Your request has been processed.\r\n"
		    "Your player's password has been e-mailed to you.\r\n"
		);
		wall_arches( MARK
		    "New request filed in newbie hopper and processed." );
	    } else {
		char buf[80];
		sprintf(buf, "%s is filed and waiting for approval.", user);
		do_note(0, buf);

		queue_string(d, "Your request has been filed.\r\n"
		    "Turnaround is generally less than 24 hours.\r\n"
		    "Your player's password will be sent to you via e-mail.\r\n"
		);
		wall_arches( MARK "New request filed in newbie hopper." );
		
	    }
	    host_touch_lastreq(d->hostaddr);
	    return 1;
	}
    } else {
	char buf[BUFFER_LEN];

	queue_string(d, "Wow, business is booming!  "
	  "We recently received another request from someone\r\nnear you.  "
	  "Please come back in "
	);
	sprintf(buf, "%d", (int)
	    (((domain_lastreq(d->hostaddr) + tp_registration_wait - current_systime)
	    / 60) + 1)
	);
	queue_string(d, buf);
	queue_string(d, " minutes and try your request again.\r\n");
      }
    } else {
	char buf[BUFFER_LEN];

	sprintf(buf, CFG_NO_OLR, tp_register_mesg);
	queue_string(d, buf);
    }
    return 0;
}

void 
emergency_shutdown(void)
{
    close_sockets("\r\nEep!  Emergency shutdown.  Be back soon!");

#ifdef SPAWN_HOST_RESOLVER
    kill_resolver();
#endif

}

void 
dump_users(struct descriptor_data * e, char *user)
{
    struct descriptor_data *d;
    int     wizard, players, showidle = 0, idlecount = 0;
    time_t  now;
    char    buf[BUFFER_LEN], tbuf[BUFFER_LEN];
    char    pbuf[64];
    dbref   loc;
    const char *p;

    wizard = e->connected && Mage(e->player);

    while (*user && isspace(*user)) user++;

#ifdef MORTWHO
    if( (*user == '!') || (*user == '*') ) { user++; } else { wizard = 0; }
#else
    if( (*user == '!') || (*user == '*') ) { user++; wizard = 0; }
#endif

    if( *user == '+' ) { user++; showidle = 1; } else showidle = 0;

    if(!tp_show_idlers || wizard)
	showidle = 1;

    while (*user && isspace(*user)) user++;

    if (!*user)
	user = NULL;
    else
	showidle = 1;

    (void) time(&now);
    if (wizard) {
	queue_ansi(e,
		"DS "
		CFOREST "Player Name                 " PLAYER_NAME_SPACE
		CAQUA "Room     " 
		CVIOLET "On For" 
		CBROWN " Idle"
		CCRIMSON " Cmds"
		CNAVY " Host"
	);
    } else {
	if (tp_who_doing) {
	    p = get_property_class((dbref)0, "~who/poll");
#ifdef COMPRESS
	    p = uncompress(p);
#endif
	    if((!p) || (!*p))
		p = "Doing...";
	} else p = "";

#ifdef ROLEPLAY
	if(tp_rp_who_lists_ic && tp_who_doing)
	    sprintf( buf,
		CFOREST "Player Name           " PLAYER_NAME_SPACE
		CVIOLET "On For "
		CBROWN "Idle  "
		CCRIMSON "IC  "
		CAQUA "%-.*s",
		40 - (int) sizeof(PLAYER_NAME_SPACE),
		clean_doing(p)
	    );
	else
#endif
	sprintf( buf,
	    CFOREST "Player Name           " PLAYER_NAME_SPACE
	    CVIOLET "On For "
	    CBROWN "Idle  "
	    CAQUA "%-.*s",
	    44 - (int) sizeof(PLAYER_NAME_SPACE),
	    clean_doing(p)
	);
	queue_ansi(e, buf);
    }

    d = descriptor_list;
    players = 0;
    while (d) {
	if(d->connected)
	    players++;
	if(d->idle)
	    idlecount++;
	if (!user || (d->connected && string_prefix(NAME(d->player), user))) {
	    if (d->connected && (!OkObj(d->player))) {
		sprintf( buf, "<\?\?\?> desc %d #%d", d->descriptor, d->player );
	    } else if (wizard) {
		if(d->connected)
		    sprintf(pbuf, "%s", unparse_object(MAN, d->player));
		else
		    sprintf(pbuf, "<connecting>");

		sprintf(pbuf, "%.*s", PLAYER_NAME_LIMIT + 10, tct(pbuf,tbuf));

		loc = (d->connected) ? DBFETCH(d->player)->location : -1;

		sprintf(buf, "%-3d%s%-*s %s%5d%c %s%9s%s%s%4s%s%5d %s%s",
			d->descriptor % 1000,  
			CGREEN, PLAYER_NAME_LIMIT + 10, pbuf,
			CCYAN, loc % 100000,
			((loc>=0)&&(FLAGS(loc)&JUMP_OK))?'J':' ',
			CPURPLE,	time_format_1(now - d->connected_at),
			d->connected ? ((FLAGS(d->player) & INTERACTIVE) ? "*" : " ")
				: " ",
			CYELLOW,	time_format_2(now - d->last_time),
			CRED, d->commands % 100000,
			CBLUE, tct(hostname_domain(d->hostname), tbuf)
		);
	    } else if (d->connected) {
#ifdef ROLEPLAY
		if (tp_rp_who_lists_ic && tp_who_doing) {
		    sprintf(buf, "%s%-*s %s%10s %s%4s%s %s %s%-.*s",
			CGREEN,	PLAYER_NAME_LIMIT + 1, tct(NAME(d->player),pbuf),
			CPURPLE, time_format_1(now - d->connected_at),
			CYELLOW, time_format_2(now - d->last_time),
			(FLAGS(d->player) & INTERACTIVE) ? "*" : " ",
			(FLAG2(d->player)&F2IC) ? CGREEN "Yes" : CRED "No ",
			CCYAN, 40 - (int) sizeof(PLAYER_NAME_SPACE),
			GETDOING(d->player) ? clean_doing(
#ifdef COMPRESS
			    tct(uncompress(GETDOING(d->player)),tbuf)
#else
			    tct(GETDOING(d->player),tbuf)
#endif
			    ) : ""
		    );
		} else
#endif
		       if (tp_who_doing) {
		    sprintf(buf, "%s%-*s %s%10s %s%4s%s %s%-.*s",
			CGREEN,	PLAYER_NAME_LIMIT + 1, tct(NAME(d->player),pbuf),
			CPURPLE, time_format_1(now - d->connected_at),
			CYELLOW, time_format_2(now - d->last_time),
			((FLAGS(d->player) & INTERACTIVE) ? "*" : " "),
			CCYAN, 44 - (int) sizeof(PLAYER_NAME_SPACE),
			GETDOING(d->player) ? clean_doing(
#ifdef COMPRESS
			    tct(uncompress(GETDOING(d->player)),tbuf)
#else
			    tct(GETDOING(d->player),tbuf)
#endif
			    ) : ""
		    );
		} else {
		    sprintf(buf, "%s%-*s %s%10s %s%4s%s",
			CGREEN, PLAYER_NAME_LIMIT + 1, tct(NAME(d->player), pbuf),
			CPURPLE, time_format_1(now - d->connected_at),
			CYELLOW, time_format_2(now - d->last_time),
			((FLAGS(d->player) & INTERACTIVE) ? "*" : " ")
		    );
		}
	    }
	    if( d->connected || wizard ) {
		if( showidle || !d->idle )
		    queue_ansi(e, buf);
	    }
	}
	d = d->next;
    }

    if(!showidle && (idlecount > 0)) {
	if( (p = get_property_class((dbref)0, "~who/idle")) ) {
#ifdef COMPRESS
	    p = uncompress(p);
#endif
	}
	sprintf(buf, CYELLOW "%s:" CGREEN, (p && *p) ? p : "Online but idle");
	for(d = descriptor_list; d; d = d->next) {
	    if(!d->connected || !d->idle)
		continue;
	    strcat(buf, " ");
	    strcat(buf, NAME(d->player));
	    if(strlen(buf) >= (BUFFER_LEN - (PLAYER_NAME_LIMIT + 1)))
		break;
	}
	queue_ansi(e, buf);
    }

    sprintf(buf, "%s%d player%s %s connected.  %s(%d active, %d idle, Max was %d)",
	CBLUE, players,
	(players == 1) ? "" : "s",
	(players == 1) ? "is" : "are",
	CYELLOW,	players - idlecount, idlecount,
	con_players_max
    );
    queue_ansi(e, buf);
}

const char *clean_doing( const char *doing )
{
    int found = 0, foundafter = 0;
    const char *c = NULL, *start = NULL;

    if( !tp_doing_blocks_ads || !doing )
	return doing;

    if( !start && ( c = string_match( doing, "com" ) ) && c > doing && ( tp_doing_blocks_hard || ispunct(c[-1]) ) ) start = c;
    if( !start && ( c = string_match( doing, "edu" ) ) && c > doing && ( tp_doing_blocks_hard || ispunct(c[-1]) ) ) start = c;
    if( !start && ( c = string_match( doing, "org" ) ) && c > doing && ( tp_doing_blocks_hard || ispunct(c[-1]) ) ) start = c;
    if( !start && ( c = string_match( doing, "net" ) ) && c > doing && ( tp_doing_blocks_hard || ispunct(c[-1]) ) ) start = c;
    if( !start && ( c = string_match( doing, "gov" ) ) && c > doing && ( tp_doing_blocks_hard || ispunct(c[-1]) ) ) start = c;

    c = doing;
    
    while( *c ) {
	if( isdigit( *c ) ) {
	    found++;
	    if( start && c > start ) foundafter++;
	    while( *c && isdigit( *c ) ) c++;
	} else {
	    while( *c && !isdigit( *c ) ) c++;
	}
    }

    /* If a domain was found, and one number after, it's most likely an ad */
    if( start && foundafter > 0 ) return "";

    if( tp_doing_blocks_hard && start && found > 0 ) return "";

    /* If there are 4 separate numbers it is most likely an ad */
    if( found > 4 ) return "";

    return doing;
}


char   *
time_format_1(time_t dt)
{
    register struct tm *delta;
    static char buf[64];

    delta = gmtime(&dt);
    if (delta->tm_yday > 0)
	sprintf(buf, "%dd %02d:%02d",
		delta->tm_yday, delta->tm_hour, delta->tm_min);
    else
	sprintf(buf, "%02d:%02d",
		delta->tm_hour, delta->tm_min);
    return buf;
}

char   *
time_format_2(time_t dt)
{
    register struct tm *delta;
    static char buf[64];

    delta = gmtime(&dt);
    if (delta->tm_yday > 0)
	sprintf(buf, "%dd", delta->tm_yday);
    else if (delta->tm_hour > 0)
	sprintf(buf, "%dh", delta->tm_hour);
    else if (delta->tm_min > 0)
	sprintf(buf, "%dm", delta->tm_min);
    else
	sprintf(buf, "%ds", delta->tm_sec);
    return buf;
}


void
announce_puppets(dbref player, const char *msg, const char *prop)
{
    dbref what, where;
    const char *ptr, *msg2;
    char buf[BUFFER_LEN];

    for (what = 0; what < db_top; what++) {
	if (Typeof(what) == TYPE_THING && (FLAGS(what) & ZOMBIE)) {
	    if (OWNER(what) == player) {
		where = getloc(what);
		if ((!Dark(where)) /*&&(!Dark(player))&&(!Dark(what))*/) {
		    msg2 = msg;
		    if ((ptr = (char *)get_property_class(what, prop)) && *ptr)
			msg2 = ptr;
		    sprintf(buf, CCYAN "%.512s %.3000s", PNAME(what), msg2);
		    anotify_except(DBFETCH(where)->contents, what, buf, what);
		}
	    }
	}
    }
}

void 
announce_connect(dbref player)
{
    dbref   loc;
    char    buf[BUFFER_LEN];
    struct match_data md;
    dbref   exit;
#ifdef RWHO
    time_t tt;
#endif
    const char *am;

    if ((loc = getloc(player)) == NOTHING)
	return;

    if (online(player) == 1) {
	/* Set linewrap prop if it exists */
	am = get_property_class(player, PROP_LINEWRAP);
#ifdef COMPRESS
	if(am && *am) am = uncompress(am);
#endif
	if(am && *am) {
	    DBFETCH(player)->sp.player.linewrap = atoi(am);
	}

	/* Set more pager prop if it exists */
	am = get_property_class(player, PROP_MORE);
#ifdef COMPRESS
	if(am && *am) am = uncompress(am);
#endif
	if(am && *am) {
	    DBFETCH(player)->sp.player.more = atoi(am);
	}

	/* On connect we load a property and try to add any custom
	   ANSI color swaps */

	load_colorset(player);
	load_ignoring(player);

	announce_puppets(player, "wakes up.", "_/pcon");
    }

#ifdef RWHO
    if (tp_rwho) {
	time(&tt);
	sprintf(buf, "%d@%s", player, tp_muckname);
	rwhocli_userlogin(buf, NAME(player), tt);
    }
#endif

    /*
     * See if there's a connect action.  If so, and the player is the first to
     * connect, send the player through it.  If the connect action is set
     * sticky, then suppress the normal look-around.
     */

    /* queue up all _connect programs referred to by properties */
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~connect", "Connect", 1, 1);
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~oconnect", "Oconnect", 1, 0);
    if(tp_user_connect_propqueue) {
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_connect", "Connect", 1, 1);
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_oconnect", "Oconnect", 1, 0);
    }

    exit = NOTHING;
    if (online(player) == 1) {
	init_match(player, "connect", TYPE_EXIT, &md);	/* match for connect */
	md.match_level = 1;
	match_all_exits(&md);
	exit = match_result(&md);
	if (exit == AMBIGUOUS) exit = NOTHING;
    }

    if (exit != NOTHING)
	do_move(player, "connect", 1);

    if(tp_look_on_connect) {
	if (exit == NOTHING || !( (Typeof(exit) == TYPE_EXIT) && (FLAGS(exit)&STICKY) ) )	{
	    if (can_move(player, "loo", 1)) {
		do_move(player, "loo", 1);
	    } else {
		do_look_around (player);
	    }
	}
    }

    if ((loc = getloc(player)) == NOTHING)
	return;

    if (/* (!Dark(player)) && */ (!Dark(loc))) {
	sprintf(buf, CMOVE "%s has connected.", PNAME(player));
	anotify_except(DBFETCH(loc)->contents, player, buf, player);
    }

    ts_useobject(player);
    return;
}

void 
announce_disconnect(struct descriptor_data *d)
{
    dbref   player = d->player;
    dbref   loc;
    char    buf[BUFFER_LEN];

    if (!d->connected || !OkObj(player) || (Typeof(player) != TYPE_PLAYER) ||
	((loc = getloc(player)) == NOTHING)
    )	return;

#ifdef RWHO
    if (tp_rwho) {
	sprintf(buf, "%d@%s", player, tp_muckname);
	rwhocli_userlogout(buf);
    }
#endif

    if (dequeue_prog(player, 2))
	anotify(player, CINFO "Foreground program aborted.");

    if (/*(!Dark(player)) && */ (!Dark(loc))) {
	sprintf(buf, CMOVE "%s has disconnected.", PNAME(player));
	anotify_except(DBFETCH(loc)->contents, player, buf, player);
    }

    if(online(player) == 1) {
	/* Clear idle and pueblo flags if last connection */
	FLAG2(player) &= ~(F2IDLE|F2PUEBLO);
	DBFETCH(d->player)->sp.player.more = 0;
	DBFETCH(d->player)->sp.player.linewrap = 0;
	free_colorset(d->player); /* Don't need color set if offline */
	free_ignoring(d->player); /* Don't need ignores if offline */
    }

    forget_player_descr(d->player, d->descriptor);
    d->connected = 0;
    d->player = NOTHING;
    update_desc_count_table();

    /* queue up all _connect programs referred to by properties */
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~disconnect", "Disconnect", 1, 1);
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~odisconnect", "Odisconnect", 1, 0);
    if(tp_user_connect_propqueue) {
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_disconnect", "Disconnect", 1, 1);
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_odisconnect", "Odisconnect", 1, 0);
    }
    if (!online(player)) {
	/* trigger local disconnect action */
	if (can_move(player, "disconnect", 0)) {
	    do_move(player, "disconnect", 0);
	}
	announce_puppets(player, "falls asleep.", "_/pdcon");
    }

    ts_lastuseobject(player);
    DBDIRTY(player);
}

void 
announce_idle(struct descriptor_data *d)
{
    dbref   player = d->player;
    dbref   loc;
    char    buf[BUFFER_LEN];

    if (!d->connected || (loc = getloc(player)) == NOTHING)
	return;

    if (/*(!Dark(player)) && */ (!Dark(loc))) {
	sprintf(buf, CMOVE "%s has become terminally idle.", PNAME(player));
	anotify_except(DBFETCH(loc)->contents, player, buf, player);
    }

    /* queue up all _idle programs referred to by properties */
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~idle", "Idle", 1, 1);
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~oidle", "Oidle", 1, 0);
    if(tp_user_idle_propqueue) {
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_idle", "Idle", 1, 1);
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_oidle", "Oidle", 1, 0);
    }
}

void 
announce_unidle(struct descriptor_data *d)
{
    dbref   player = d->player;
    dbref   loc;
    char    buf[BUFFER_LEN];

    if (!d->connected || (loc = getloc(player)) == NOTHING)
	return;

    if (/*(!Dark(player)) && */ (!Dark(loc))) {
	sprintf(buf, CMOVE "%s has unidled.", PNAME(player));
	anotify_except(DBFETCH(loc)->contents, player, buf, player);
    }

    /* queue up all _unidle programs referred to by properties */
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~unidle", "Unidle", 1, 1);
    envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	"~ounidle", "Ounidle", 1, 0);
    if(tp_user_idle_propqueue) {
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_unidle", "Unidle", 1, 1);
	envpropqueue(player, getloc(player), NOTHING, player, NOTHING,
	    "_ounidle", "Ounidle", 1, 0);
    }
}



int
clear_prompt(dbref player) {
    struct descriptor_data *d;
    int     cnt = 0;

    for (d = descriptor_list; d; d = d->next)
	if ((d->connected) && (d->player == player)) {
	    if(d->moreline || d->prompted) {
		d->prompted = 0;
		d->moreline = 0;
		cnt++;
	    }
	}

    return (cnt);
}


struct descriptor_data *descr_count_table[MAX_SOCKETS];
int current_descr_count = 0;

void
init_descr_count_lookup()
{
    int i;
    for (i = 0; i < MAX_SOCKETS; i++)
	descr_count_table[i] = NULL;
}

void
update_desc_count_table()
{
    int c;
    struct descriptor_data *d;

    current_descr_count = 0;
    for (c = 0, d = descriptor_list; d; d = d->next)
    {
        if (d->connected)
	{
	    descr_count_table[c++] = d;
	    current_descr_count++;
	}
    }
}

struct descriptor_data *
descrdata_by_count(int count)
{
    count--;
    if (count >= current_descr_count || count < 0)
        return NULL;

    return descr_count_table[count];
}

struct descriptor_data *descr_lookup_table[MAX_SOCKETS];

void
init_descriptor_lookup()
{
    int i;
    for (i = 0; i < MAX_SOCKETS; i++)
	descr_lookup_table[i] = NULL;
}

int
index_descr(int index)
{
    if((index < 0) || (index >= MAX_SOCKETS))
	return -1;
	if(descr_lookup_table[index] == NULL)
	return -1;

	return descr_lookup_table[index]->descriptor;
}

int
descr_index(int descr)
{
    int i;
    
    for(i = 0; i < MAX_SOCKETS; i++)
	if(descr_lookup_table[i] && (descr_lookup_table[i]->descriptor == descr))
	    return i;

    return -1;
}

void
remember_player_descr(dbref player, int descr)
{
    int  count = DBFETCH(player)->sp.player.descr_count;
    int* arr   = DBFETCH(player)->sp.player.descrs;
    int index;
    
    index = descr_index(descr);

    if((index < 0) || (index >= MAX_SOCKETS))
	return;

    if (!arr) {
        arr = (int*)malloc(sizeof(int));
        arr[0] = index;
        count = 1;
    } else {
        arr = (int*)realloc(arr,sizeof(int) * (count+1));
        arr[count] = index;
        count++;
    }
    DBFETCH(player)->sp.player.descr_count = count;
    DBFETCH(player)->sp.player.descrs = arr;
}

void
forget_player_descr(dbref player, int descr)
{
    int  count = DBFETCH(player)->sp.player.descr_count;
    int* arr   = DBFETCH(player)->sp.player.descrs;
    int index;
    
    index = descr_index(descr);

    if((index < 0) || (index >= MAX_SOCKETS))
	return;

    if (!arr) {
        count = 0;
    } else if (count > 1) {
	int src, dest;
        for (src = dest = 0; src < count; src++) {
	    if (arr[src] != index) {
		if (src != dest) {
		    arr[dest] = arr[src];
		}
		dest++;
	    }
	}
	if (dest != count) {
	    count = dest;
	    arr = (int*)realloc(arr,sizeof(int) * count);
	}
    } else {
        free((void*)arr);
        arr = NULL;
        count = 0;
    }
    DBFETCH(player)->sp.player.descr_count = count;
    DBFETCH(player)->sp.player.descrs = arr;
}

int
remember_descriptor(struct descriptor_data *d)
{
    int i;

    if(!d)
	return -1;
    
    for(i = 0; i < MAX_SOCKETS; i++)
    {
	if(descr_lookup_table[i] == NULL)
	{
	    descr_lookup_table[i] = d;
	    return i;
	}
    }

    return -1;
}

void
forget_descriptor(struct descriptor_data *d)
{
    int i;

    if(!d)
	return;

    for(i = 0; i < MAX_SOCKETS; i++)
	if(descr_lookup_table[i] == d)
	    descr_lookup_table[i] = NULL;
}

struct descriptor_data *
lookup_descriptor(int index)
{
    if (index >= MAX_SOCKETS || index < 0)
	return NULL;

    return descr_lookup_table[index];
}


struct descriptor_data *
descrdata_by_index(int index)
{
    return lookup_descriptor(index);
}

struct descriptor_data *
descrdata_by_descr(int descr)
{
    return lookup_descriptor(descr_index(descr));
}


/* # online connections */
int 
online(dbref player)
{
    return DBFETCH(player)->sp.player.descr_count;
}

/* Min idle time of all connections, -1 = not connected */
int 
minidle(dbref player)
{
    struct descriptor_data *d;
    int pidle = -1, didle;
    time_t now = current_systime;

    for (d = descriptor_list; d; d = d->next)
	if ((d->connected) && (d->player == player)) {
	    didle = (int) (now - d->last_time);
	    pidle = (pidle == - 1) ? didle : Min(didle, pidle);
	}

    return (pidle);
}

int 
pcount()
{
    return current_descr_count;
}

int 
pidle(int count)
{
    struct descriptor_data *d;
    time_t    now;

    d = descrdata_by_count(count);

    if (d) {
	(void) time(&now);
	return (now - d->last_time);
    }

    return -1;
}

int 
pidler(int count)
{
    /* WORK: use player.descrs */
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d)
	return d->idle;

    return -1;
}

dbref 
pdbref(int count)
{
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d)
	return (d->player);

    return NOTHING;
}

int 
pontime(int count)
{
    struct descriptor_data *d;
    time_t    now;

    d = descrdata_by_count(count);

    (void) time(&now);
    if (d)
	return (now - d->connected_at);

    return -1;
}

char   *
phost(int count)
{
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d)
	return ((char *) d->hostname);

    return (char *) NULL;
}

char   *
puser(int count)
{
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d)
	return ((char *) d->username);

    return (char *) NULL;
}

char   *
pipnum(int count)
{
    static char ipnum[40];
    const char *p;
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d) {
	p = host_as_hex(d->hostaddr);
	strcpy(ipnum, p);
	return ((char *) ipnum);
    }

    return (char *) NULL;
}

char   *
pport(int count)
{
    static char port[40];
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d) {
	sprintf(port, "%d", d->port);
	return ((char *) port);
    }

    return (char *) NULL;
}

/*** Foxen ***/
int
pfirstconn(dbref who)
{
    struct descriptor_data *d;
    int count = 1;
    for (d = descriptor_list; d; d = d->next) {
	if (d->connected) {
	    if (d->player == who) return count;
	    count++;
	}
    }
    return 0;
}


void 
pboot(int count)
{
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d) {
	process_output(d);
	d->booted = 1;
	/* shutdownsock(d); */
    }
}

void 
pnotify(int count, char *outstr)
{
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d) {
	queue_string(d, outstr);
	queue_write(d, "\r\n", 2);
    }
}


int 
pdescr(int count)
{
    struct descriptor_data *d;

    d = descrdata_by_count(count);

    if (d)
	return d->descriptor;

    return -1;
}


int 
pnextdescr(int descr)
{
    struct descriptor_data *d;

    d = descrdata_by_descr(descr);
    if (d)
	d = d->next;
    while (d && (!d->connected))
	d = d->next;
    if (d)
	return d->descriptor;

    return 0;
}


int 
pdescrcon(int count)
{
    struct descriptor_data *d;
    int     i = 1;

    d = descriptor_list;
    while (d && (d->descriptor != count)) {
	if (d->connected)
	    i++;
	d = d->next;
    }
    if (d && d->connected)
	return i;

    return 0;
}

int 
dset_user(struct descriptor_data *d, dbref who)
{
    if (d && d->connected) {
	announce_disconnect(d);
	if (who != NOTHING) {
	    d->player = who;
	    d->connected = 1;
	    update_desc_count_table();
            remember_player_descr(who, d->descriptor);
	    announce_connect(who);
	}
	return 1;
    }
    return 0;
}

int 
pset_user(int descr, dbref who)
{
    struct descriptor_data *d;

    d = descrdata_by_descr(descr);
    return dset_user(d, who);
}

int 
pset_echo(int descr, int onoff)
{
    struct descriptor_data *d;
    int len = 0;

#ifdef SETECHO_NEED_UNIX_LOGIN
    if(!tp_unix_login)
	return 0; /* Won't send telnet-echos unless unix_login is @tuned on */
#endif /* SETECHO_NEEDS_UNIX_LOGIN */

    d = descrdata_by_descr(descr);
    if (d) {
	if(onoff)
	    len = writesocket(d->descriptor, TELNET_ECHO_ON,  strlen(TELNET_ECHO_ON) );
	else
	    len = writesocket(d->descriptor, TELNET_ECHO_OFF, strlen(TELNET_ECHO_OFF));
	if(len > 0)
	    tp_write_bytes += len;
	    
	return 1;
    }
    return 0;
}

void
dump_ignoring(dbref player)
{
    struct descriptor_data *d;

    d = descriptor_list;
    while (d) {
	if (d->connected && ignoring(d->player, player))
	    anotify_fmt(player, CINFO "%s", NAME(d->player));

	d = d->next;
    }
}


dbref 
partial_pmatch(const char *name)
{
    struct descriptor_data *d;
    dbref last = NOTHING;

    d = descriptor_list;
    while (d) {
	if (d->connected && (last != d->player) &&
		string_prefix(NAME(d->player), name)) {
	    if (last != NOTHING) {
		last = AMBIGUOUS;
		break;
	    }
	    last = d->player;
	}
	d = d->next;
    }
    return (last);
}


#ifdef RWHO
void
update_rwho()
{
    struct descriptor_data *d;
    char buf[BUFFER_LEN];

    rwhocli_pingalive();
    d = descriptor_list;
    while (d) {
	if (d->connected) {
	    sprintf(buf, "%d@%s", d->player, tp_muckname);
	    rwhocli_userlogin(buf, NAME(d->player), d->connected_at);
	}
	d = d->next;
    }
}
#endif

void 
welcome_user(struct descriptor_data * d)
{
    char   *ptr;
    char    buf[BUFFER_LEN];
    const char *fname;

    if (d->type == CT_MUCK) {
	if (tp_pueblo_support)
	    queue_string(d, "\r\nThis world is Pueblo 1.0 Enhanced.\r\n\r\n");
	fname = reg_site_welcome(d->hostaddr);
	if (fname && (*fname == '.')) {
	    strcpy(buf, WELC_FILE);
	} else if (fname && (*fname == '#')) {
	    if (tp_rand_screens > 0)
		sprintf(buf, "data/welcome%d.txt",(rand()%tp_rand_screens)+1);
	    else
		strcpy(buf, WELC_FILE);
	} else if (fname) {
	    strcpy(buf, "data/welcome/");
	    ptr = buf + strlen(buf);
	    while(*fname && (*fname != ' ')) (*(ptr++)) = (*(fname++));
	    *ptr = '\0';
	    strcat(buf, ".txt");
	} else if (reg_site_is_barred(d->hostaddr) == TRUE) {
	    strcpy(buf, BARD_FILE);
	} else if (tp_rand_screens > 0) {
	    sprintf(buf, "data/welcome%d.txt", (rand()%tp_rand_screens) + 1);
	} else {
	    strcpy(buf, WELC_FILE);
	}

	if (desc_file(0, d, buf)) {
	    sprintf(buf, "\r\nWelcome to %s!\r\n", tp_muckname);
	    queue_string(d, buf);
	}
	if (localhost_mode || wizonly_mode)
	    queue_string(d, "Due to maintenance, only " NAMEWIZ "s can connect now.\r\n");
	else if (tp_playermax && con_players_curr >= tp_playermax_limit) {
	    sprintf(buf, "%s\r\n", tp_playermax_warnmesg);
	    queue_string(d, buf);
	}
    
	if(tp_unix_login && (!d->player))
	    queue_string(d, "\r\nlogin: ");
    }
}

int
desc_file(dbref player, struct descriptor_data * d, const char *fname) {
    FILE   *f;
    char   *ptr;
    char    buf[BUFFER_LEN];

    if(player>0) d = get_descr(0, player);

    if(!d || !fname) return 1;

    if (!(f = fopen(fname, "rb"))) {
	sprintf(buf, "desc_file: %s", fname);
	perror(buf);
	return 1;
    } else {
	while (fgets(buf, sizeof buf, f)) {
	    ptr = index(buf, '\n');
	    if (ptr && ptr > buf && *(ptr-1) != '\r') {
		*ptr++ = '\r';
		*ptr++ = '\n';
		*ptr++ = '\0';
	    }
	    queue_string(d, buf);
	}
	fclose(f);
	return 0;
    }
}

void 
help_user(struct descriptor_data * d)
{

    FILE   *f;
    char    buf[BUFFER_LEN];

    if ((f = fopen("data/connect.txt", "rb")) == NULL) {
	queue_string(d, "The help file is missing, the management has been notified.\r\n");
	perror("spit_file: connect.txt");
    } else {
	while (fgets(buf, sizeof buf, f)) {
	    queue_string(d, buf);
	}
	fclose(f);
    }
}

#ifdef WIN95
void
gettimeofday(struct timeval *tval, void *tzone)
{
    if(!tval)
        return;

    tval->tv_sec = time(NULL);
    tval->tv_usec = 0;

}
#endif

