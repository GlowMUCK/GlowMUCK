/***********************************************************************

WAIT!  Before you modify this file, see if it's possible to put your
changes in 'local.h'.  If so you will only have to copy ONE file over
between version upgrades, and won't have to worry about missing changes
or overwriting a file and losing some upgrade material.

************************************************************************/

#include "copyright.h"
#include "version.h"

/* penny related stuff */
/* amount of object endowment, based on cost */
#define OBJECT_ENDOWMENT(cost) (((cost)-5)/5)
#define OBJECT_DEPOSIT(endow) ((endow)*5+4)


/* timing stuff */
#define TIME_MINUTE(x)  (60 * (x))                /* 60 seconds */
#define TIME_HOUR(x)    ((x) * (TIME_MINUTE(60))) /* 60 minutes */
#define TIME_DAY(x)     ((x) * (TIME_HOUR(24)))   /* 24 hours   */


#define MAX_OUTPUT 65536        /* maximum amount of queued output */

#define DB_INITIAL_SIZE 100  /* initial malloc() size for the db */

/* The maximum number of ports that the muck can listen for connections on. */
#define MAX_LISTEN_PORTS 16



/* User interface low level commands */
#define QUIT_COMMAND "QUIT"
#define WHO_COMMAND "WHO"
#define CONNECT_COMMAND "CONNECT"
#define PREFIX_COMMAND "OUTPUTPREFIX"
#define SUFFIX_COMMAND "OUTPUTSUFFIX"
#define PUEBLO_COMMAND "PUEBLOCLIENT"
#define MORE_COMMAND "MORE"

/* Turn this back on when you want MUD to set from root to some user_id */
/* #define MUD_ID "MUCK" */

/* Used for breaking out of muf READs or for stopping foreground programs. */
#define BREAK_COMMAND "@@Q"

#define EXIT_DELIMITER ';'	/* delimiter for lists of exit aliases  */
#define MAX_LINKS 10		/* maximum number of destinations for an exit */


#define PCREATE_FLAGS (JUMP_OK)	/* default flag bits for created players */
#define PCREATE_FLAG2 (F2MPI)	/* default 2nd set flag bits for players */

#define TCREATE_FLAGS (0)	/* default flag bits for created things */
#define TCREATE_FLAG2 (0)	/* default 2nd set flag bits for things */

#define RCREATE_FLAGS (0)	/* default flag bits for created rooms */
#define RCREATE_FLAG2 (0)	/* default 2nd set flag bits for rooms */

#define ECREATE_FLAGS (0)	/* default flag bits for created exits */
#define ECREATE_FLAG2 (0)	/* default 2nd set flag bits for exits */

#define FCREATE_FLAGS (0)	/* default flag bits for created programs */
#define FCREATE_FLAG2 (0)	/* default 2nd set flag bits for programs */


#define GLOBAL_ENVIRONMENT ((dbref) 0)  /* parent of all rooms.  Always #0 */

/* magic cookies (not chocolate chip) :) */

#define NOT_TOKEN '!'
#define AND_TOKEN '&'
#define OR_TOKEN '|'
#define LOOKUP_TOKEN '*'
#define REGISTERED_TOKEN '$'
#define NUMBER_TOKEN '#'
#define ARG_DELIMITER '='
#define PROP_DELIMITER ':'
#define PROPDIR_DELIMITER '/'
#define PROP_RDONLY '_'
#define PROP_RDONLY2 '%'
#define PROP_PRIVATE '.'
#define PROP_HIDDEN '@'
#define PROP_SEEONLY '~'

/* magic command cookies (oh me, oh my!) */

#define SAY_TOKEN '"'
#define SAY_TOKEN2 '\''
#define SAY_COMMAND "say"

#define POSE_TOKEN ':'
#define POSE_TOKEN2 ';'
#define POSE_COMMAND "pose"

#define CHAT_TOKEN '.'
#define CHAT_COMMAND "chat"

#define CHATPOSE_TOKEN ','
#define CHATPOSE_COMMAND "chat :"

#define DASH_TOKEN '-'
#define DASH_COMMAND "minus"

#define OVERIDE_TOKEN '!'


/* @edit'or stuff */

#define EXIT_INSERT "."         /* character to exit from insert mode    */
#define INSERT_COMMAND 'i'
#define DELETE_COMMAND 'd'
#define QUIT_EDIT_COMMAND   'q'
#define COMPILE_COMMAND 'c'
#define LIST_COMMAND   'l'
#define EDITOR_HELP_COMMAND 'h'
#define KILL_COMMAND 'k'
#define SHOW_COMMAND 's'
#define SHORTSHOW_COMMAND 'a'
#define VIEW_COMMAND 'v'
#define UNASSEMBLE_COMMAND 'u'
#define NUMBER_COMMAND 'n'
#define PUBLICS_COMMAND 'p'

/* maximum number of arguments */
#define MAX_ARG  2

/* Usage comments:
   Line numbers start from 1, so when an argument variable is equal to 0, it
   means that it is non existent.

   I've chosen to put the parameters before the command, because this should
   more or less make the players get used to the idea of forth coding..     */

