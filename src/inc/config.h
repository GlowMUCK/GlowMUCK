/*
 * config.h
 * $Revision: 1.5 $ $Date: 2005/09/11 18:31:51 $
 * 
 * Tunable parameters -- Edit to you heart's content (override in local.h)
 *
 */

/***********************************************************************
WAIT!  Before you modify this file, see if it's possible to put your
changes in 'local.h'.  If so you will only have to copy ONE file over
between version upgrades, and won't have to worry about missing changes
or overwriting a file and losing some upgrade material.

************************************************************************/

#include "copyright.h"

/*
 * $Log: config.h,v $
 * Revision 1.5  2005/09/11 18:31:51  feaelin
 * Added the FD_SETSIZE define allowing changes in the descriptor limit at
 * compile time.
 *
 * Revision 1.4  2005/04/26 19:53:06  feaelin
 * Added 256 color support.
 * Added the debug inserver defines
 *
 * Revision 1.3  2005/03/08 18:57:36  feaelin
 * Added the heartbeat modifications. You can add programs to the @heartbeat
 * propdir and the programs will be executed every 15 seconds.
 *
 */

/************************************************************************
   Administrative Options 

   Various things that affect how the muck operates and what privs
 are compiled in.
 ************************************************************************/

/* Detaches the process as a daemon so that it don't cause problems
 * keeping a terminal line open and such. Logs normal output to a file
 * and writes out a glowmuck.pid file 
 */
#undef DETACH

/* Use to compress string data (recomended)
 */
#define COMPRESS

/* To use a simple disk basing scheme where properties aren't loaded
 * from the input file until they are needed, define this. 
 */
#define DISKBASE

/* To make the server save using fast delta dumps that only write out the
 * changed objects, except when @dump or @shutdown are used, or when too
 * many deltas have already been saved to disk, #define this. 
 */
#define DELTADUMPS

/*
 * Ports where tinymuck lives -- Note: If you use a port lower than
 * 1024, then the program *must* run suid to root!
 * Port 4201 is a port with historical significance, as the original
 * TinyMUD Classic ran on that port.  It was the office number of James
 * Aspnes, who wrote TinyMUD from which TinyMUCK eventually was derived.
 */
#define TINYPORT 9999           /* Port that tinymuck uses for playing */

/*
 * Some systems can hang for up to 30 seconds while trying to resolve
 * hostnames.  Define this to use a non-blocking second process to resolve
 * hostnames for you.  NOTE:  You need to compile the 'resolver' program
 * (make resolver) and put it in the directory that the glowmuck program is
 * run from.
 */
#ifndef WIN95
#define SPAWN_HOST_RESOLVER
#endif

/* Debugging info for database loading */
#undef VERBOSELOAD

/* A little extra debugging info for read()/write() on process input/output */
/* I put this in when I couldn't figure out why sockets were failing from */
/* a bad net connection for the server. */
#undef DEBUGPROCESS

/* Define MORTWHO to make it so wizards need to type WHO! to see hosts */
/* When undefined, WHO! will show the mortal WHO+@doing (without going Q) */
#define MORTWHO

/* Define to compile in RWHO support */
#undef RWHO

/* Define to compile in MPI support */
#define MPI

/* Define to compile in MUD support */
#define MUD

/* Define to compile in roleplaying uspport */
#define ROLEPLAY

/* Define to compile in HTTPD server WWW page support */
#define HTTPD

/* Define this to do proper delayed link references for HTTPD */
/* The _/www:http://... links won't work on some clients without it. */
#undef HTTPDELAY

/* Define this to compile in paths (virtual exits) */
#define PATH

/* Define this to limit MPI to a recursion 'exec' level of 6 levels.  */
/* Running many levels of 'exec' is VERY memory intensive, allocating */
/* roughly 64k of stack per level (very dangerous and inefficient).   */
/* This is intended to keep WinGlow from causing a stack overflow.    */
/* As of 3.1.7, malloc memory is used instead of stack space for MPI. */
#ifdef WIN95
  /* #define SAFE_MPI */
#endif

/* Allows the Heartbeat processes to run */
#define HEARTBEAT

/* Defines how long between heartbeats (in seconds) */
#define BASE_PULSE 15

/* Define this to make {time} and {ftime} behave as FB brokenly makes them */
#undef FB_TIMEZONE_QUIRK

/* Define this if you want the {ansi} mpi prim to not return anything. */
/* Normally it returns the uncolored version of the passed string. */
#undef NULL_ANSI

/* Define this if you want the {oansi} mpi prim to not return anything. */
/* Normally it returns the uncolored version of the passed string. */
#undef NULL_OANSI

/* Define this to allow totally insecure (your account can be at risk!)   */
/* access to the system() call that allows the muck to execute ANY        */
/* arbitrary program or script on the host system.  USE AT YOUR OWN RISK! */
/* This is intended to support muf-based character request systems that   */
/* need to send an e-mail but can't use the inserver system which is more */
/* secure as it normally uses lpexec or a sister function.                */
#undef INSECURE_SYSTEM_MUF_PRIM


/************************************************************************
   Game Options

   These are the ones players will notice. 
 ************************************************************************/

/* Set this to your mark for shouts, dumps, etc.  Also change @tunes */

#define MARK "[!] "

/* Make the `examine' command display full names for types and flags */
#define VERBOSE_EXAMINE

/* limit on player name length */
#define PLAYER_NAME_LIMIT 16
/* If you change player name limit, add one space per character to the */
/* following definition for each character beyond 16 (the default). */
#define PLAYER_NAME_SPACE ""

/************************************************************************
   Various Messages 
 
   Printed from the server at times, esp. during login.
 ************************************************************************/

#define NOBUILD_MESG	"Building is currently disabled."
#define NOGUEST_MESG	"This command is unavailable to guests."
#define NOQUOTA_MESG	"That would exceed your quota limit.  Type '@quota'."

#define PATH_MESG	"Please use @path whenever possible.  Type 'help @path' for help."
#define OBJ_MESG	"Please use @object or @detail if possible.  Type '@object' for help."

#define NOBBIT_MESG	"You're not a builder."
#define NOMBIT_MESG	"You're not a programmer."
#define NOEDIT_MESG	"That is already being edited."

#define NOPERM_MESG	"Permission denied."
#define WHICH_MESG	"I don't know which one you mean!"
#define NOTHERE_MESG	"I don't see that here."
#define BADKEY_MESG	"I don't understand that key."
#define WHO_MESG	"Who?"
#define NOWAY_MESG	"You can't go that way."
#define HUH_MESG	"Need help?  Just type \"help\"."
#define NOTHING_MESG	"You see nothing special."

#define WARN_MESG	PLAYERMAX_WARNMESG
#define BOOT_MESG	PLAYERMAX_BOOTMESG
#define REG_MESSAGE	"legacy@legacy.muq.org"
#define DBRO_MESG	"The muck is currently read-only."

/*
 * Message if someone trys the create command and your system is
 * setup for registration.
 */
#define CFG_REG_MSG "Your character isn't allowed to connect from this site.\r\nSend email to %s to be added to this site.\r\n"
#define CFG_REG_CRE "Character registration is enabled now.\r\nPlease try the request command or send email to %s.\r\n"
#define CFG_NO_ID   "Your character has no real-life information.\r\nPlease send email to %s with your real name.\r\n"
#define CFG_NO_OLR  "Online registration is not open now.\r\nEmail questions to %s.\r\n"
#define CFG_REG_BLK "Sorry, but we are not accepting %s from your site.\r\nEmail questions to %s.\r\n"

/*
 * Goodbye message.
 */
#define LEAVE_MESSAGE "May your heart always guide your actions."


/*
 * Error messeges spewed by the help system.
 */
#define NO_NEWS_MSG "That topic does not exist.  Type 'news topics' to list the news topics available."
#define NO_HELP_MSG "That topic does not exist.  Type 'help index' to list the help topics available."
#define NO_MAN_MSG "That topic does not exist.  Type 'man' to list the MUF topics available."
#define NO_INFO_MSG "That file does not exist.  Type 'info' to get a list of the info files available."

/*
 *  An attempt to standardize where preferences are stored
 */

#define PREFS_PROPDIR  "/_prefs/"
#define PREF_256COLORS PREFS_PROPDIR"256-colors"
/************************************************************************
   File locations
 
   Where the system looks for its datafiles.
 ************************************************************************/
#define WELC_FILE "data/welcome.txt"     /* For the opening screen	*/
#define BARD_FILE "data/welcome/fbi.txt" /* For naughty sites		*/
#define MOTD_FILE "data/motd.txt"        /* For the message of the day	*/
#define NOTE_FILE "data/note.txt"        /* For the char request notes	*/

#define HELP_FILE "data/help.txt"    /* For the 'help' command      */
#define HELP_DIR  "data/help"        /* For 'help' subtopic files   */
#define NEWS_FILE "data/news.txt"    /* For the 'news' command      */
#define NEWS_DIR  "data/news"        /* For 'news' subtopic files   */
#define MAN_FILE  "data/man.txt"     /* For the 'man' command       */
#define MAN_DIR   "data/man"         /* For 'man' subtopic files    */
#define MPI_FILE  "data/mpihelp.txt" /* For the 'mpi' command       */
#define MPI_DIR   "data/mpihelp"     /* For 'mpi' subtopic files    */
#define INFO_DIR  "data/info/"
#define EDITOR_HELP_FILE "data/edit-help.txt" /* editor help file   */
#define SYSPARM_HELP_FILE "data/sysparms.txt" /* sysparm help file  */

#define DELTAFILE_NAME "data/deltas-file"  /* The file for deltas */
#define PARMFILE_NAME "data/parmfile.cfg"  /* The file for config parms */
#define WORDLIST_FILE "data/wordlist.txt"  /* File for compression dict. */

#define LOG_GRIPE   "logs/gripes"       /* Gripes Log */
#define LOG_HOPPER  "logs/hopper"	/* Registration hopper */
#define LOG_STATUS  "logs/status"       /* System errors and stats */
#define LOG_HTTP    "logs/httpd"	/* HTTP/WWW accesses */
#define LOG_CONC    "logs/concentrator" /* Concentrator errors and stats */
#define LOG_MUF     "logs/muf-errors"   /* Muf compiler errors and warnings. */
#define LOG_SANITY  "logs/sanfixed"	/* sanity log file */
#define COMMAND_LOG "logs/commands"     /* Player commands */
#define PROGRAM_LOG "logs/programs"     /* text of changed programs */
#define CONNECT_LOG "logs/connects"	/* text of connection info */
#define MUD_LOG	    "logs/mud-commands"	/* text of mud kills/offers/gives */

#define MACRO_FILE  "muf/macros"
#define PID_FILE    "glowmuck.pid"	/* Write the server pid to ... */

#define RESOLVER_PID_FILE "hostfind.pid" /* Write the resolver pid to ... */

#define LOCKOUT_FILE	"data/lockout.txt"
#define LOG_FILE	"logs/glowmuck.out"	/* Log stdout to ... */      
#define LOG_ERR_FILE	"logs/glowmuck.err"	/* Log stderr to ... */      

/************************************************************************
  System Dependency Defines. 

  You probably will not have to monkey with this unless the muck fails
 to compile for some reason.
 ************************************************************************/

/* If you get problems compiling strftime.c, define this. */
#undef USE_STRFTIME

/* Use this only if your realloc does not allocate in powers of 2
 * (if your realloc is clever, this option will cause you to waste space).
 * SunOS requires DB_DOUBLING.  ULTRIX doesn't.  */
#define  DB_DOUBLING

/* Prevent Many Fine Cores. */
#undef NOCOREDUMP

/* if do_usage() in wiz.c gives you problems compiling, define this */
#undef NO_USAGE_COMMAND

/* if do_memory() in wiz.c gives you problems compiling, define this */
#define NO_MEMORY_COMMAND

/* This gives some debug malloc profiling, but also eats some overhead,
   so only define if your using it. */
#undef MALLOC_PROFILING
#undef CRT_DEBUG_ALSO

/* Change this if you want to use descr_setecho without unix_login @tuned on */
#define SETECHO_NEEDS_UNIX_LOGIN

/* Change this value to the maximum number of concurrent connections. */
#define MAX_SOCKETS 500

/************************************************************************/
/************************************************************************/
/*    FOR INTERNAL USE ONLY.  DON'T CHANGE ANYTHING PAST THIS POINT.    */
/************************************************************************/
/************************************************************************/

#ifdef SAFE_MPI
#define MAX_MPI_LEVELS 6
#else
#define MAX_MPI_LEVELS 26
#endif

#ifdef SANITY
#undef MALLOC_PROFILING
#endif

/*
 * Very general defines 
 */
#define TRUE  1
#define FALSE 0

/*
 * Memory/malloc stuff.
 */
#undef LOG_PROPS
#undef LOG_DISKBASE
#undef DEBUGDBDIRTY
#define FLUSHCHANGED /* outdated, needs to be removed from the source. */


/*
 * Include all the good standard headers here.
 */
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef WIN95
  /* If you really know what you're doing you can make this bigger. */
#  define FD_SETSIZE 128

# include "win95conf.h"
# include "process.h"
/* Define WINNT >= 4.0 to get better WinSock compile */
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0400
# include <windows.h>
# define  strcasecmp(x,y) stricmp((x),(y))
# define  strncasecmp(x,y,z) strnicmp((x),(y),(z))
# define  waitpid(x,y,z) cwait((y),(x),_WAIT_CHILD)
# define  ioctl(x,y,z) ioctlsocket((x),(y),(z))
# define  EWOULDBLOCK WSAEWOULDBLOCK
# define  EINTR WSAEWOULDBLOCK
# define  getdtablesize() (FD_SETSIZE)
# define  readsocket(x,y,z) recv((x),(y),(z),0)
# define  writesocket(x,y,z) send((x),(y),(z),0)
extern void gettimeofday(struct timeval *tval, void *tzone);
# define  errnosocket WSAGetLastError()
#else
# include "autoconf.h"
# define  readsocket(x,y,z) read((x),(y),(z))
# define  writesocket(x,y,z) write((x),(y),(z))
# define  closesocket(x) close(x)
# define  errnosocket    errno
#endif /* Win95 */

#ifdef STDC_HEADERS
# include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

/*
 * Which set of memory commands do we have here...
 */
#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
# include <string.h>
/* An ANSI string.h and pre-ANSI memory.h might conflict.  */
# if !defined(STDC_HEADERS) && defined(HAVE_MEMORY_H)
#  include <memory.h>
# endif /* not STDC_HEADERS and HAVE_MEMORY_H */
/* Map BSD funcs to ANSI ones. */
# define index		strchr
# define rindex		strrchr
# define bcopy(s, d, n) memcpy ((d), (s), (n))
# define bcmp(s1, s2, n) memcmp ((s1), (s2), (n))
# define bzero(s, n) memset ((s), 0, (n))
#else /* not STDC_HEADERS and not HAVE_STRING_H */
# include <strings.h>
/* Map ANSI funcs to BSD ones. */
# define strchr		index
# define strrchr	rindex
# define memcpy(d, s, n) bcopy((s), (d), (n))
# define memcmp(s1, s2, n) bcmp((s1), (s2), (n))
/* no real way to map memset to bzero, unfortunatly. */
#endif /* not STDC_HEADERS and not HAVE_STRING_H */

#ifdef HAVE_RANDOM
# define SRANDOM(seed)	srandom((seed))
# define RANDOM()	random()
#else
# define SRANDOM(seed)	srand((seed))
# define RANDOM()	rand()
#endif

/*
 * Time stuff.
 */
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/*
 * Include some of the useful local headers here.
 */
#ifdef MALLOC_PROFILING
#include "crt_malloc.h"
#endif

/******************************************************************/
/* System configuration stuff... Figure out who and what we are.  */
/******************************************************************/

/*
 * Try and figure out what we are.
 * 
 * Not realy used for anything any more, probably can be scrapped,
 * will see in a version or so.
 */
#if defined(linux) || defined(__linux__) || defined(LINUX)
# define SYS_TYPE "Linux"
# define LINUX
#endif

#ifdef sgi
# define SYS_TYPE "SGI"
#endif

#ifdef sun
# define SYS_TYPE "SUN"
# define SUN_OS
# define BSD43
#endif

#ifdef ultrix
# define SYS_TYPE "ULTRIX"
# define ULTRIX
#endif

#ifdef bds4_3
# ifndef SYS_TYPE
#  define SYS_TYPE "BSD 4.3"
# endif
# define BSD43
#endif

#ifdef bds4_2
# ifndef SYS_TYPE
#  define SYS_TYPE "BSD 4.2"
# endif
#endif

#if defined(SVR3) 
# ifndef SYS_TYPE
#  define SYS_TYPE "SVR3"
# endif
#endif

#if defined(SYSTYPE_SYSV) || defined(_SYSTYPE_SYSV)
# ifndef SYS_TYPE
#  define SYS_TYPE "SVSV"
# endif
#endif

#if defined(__WINDOWS__)
# ifndef SYS_TYPE
#  define SYS_TYPE "Win95"
# endif
#endif

#ifndef SYS_TYPE
# define SYS_TYPE "UNKNOWN"
#endif

/******************************************************************/
/* Final line of defense for self configuration, systems we know  */
/* need special treatment.                                        */ 
/******************************************************************/

