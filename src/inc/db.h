#include "copyright.h"

#ifndef __DB_H
#define __DB_H

#include <stdio.h>
#include <time.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/* max length of command argument to process_command */
#define MAX_COMMAND_LEN 1024
#define BUFFER_LEN ((MAX_COMMAND_LEN)*4)
#define FILE_BUFSIZ ((BUFSIZ)*8)

extern time_t current_systime;
extern char match_args[BUFFER_LEN];
extern char match_cmdname[BUFFER_LEN];

typedef int dbref;		/* offset into db */

#ifdef GDBM_DATABASE
#  define DBFETCH(x)  dbfetch(x)
#  define DBDIRTY(x)  dbdirty(x)
#else				/* !GDBM_DATABASE */
#  define DBFETCH(x)  (db + (x))
#  ifdef DEBUGDBDIRTY
#    define DBDIRTY(x)  {if (!(db[x].flags & OBJECT_CHANGED))  \
			   log2file("dirty.out", "#%d: %s %d\n", (int)x, \
			   __FILE__, __LINE__); \
		       db[x].flags |= OBJECT_CHANGED;}
#  else
#    define DBDIRTY(x)  {db[x].flags |= OBJECT_CHANGED;}
#  endif
#endif

#define DBSTORE(x, y, z)    {DBFETCH(x)->y = z; DBDIRTY(x);}
#define NAME(x)     (db[x].name)
#define PNAME(x)    (db[x].name)
#define RNAME(x)    (db[x].name)
#define FLAGS(x)    (db[x].flags)
#define FLAG2(x)    (db[x].flag2)
#define OWNER(x)    (db[x].owner)

/* MAX VAL MIN ABS */
#define MAX(x,y)    ( ((x)>(y)) ? (x) : (y) )
#define MIN(x,y)    ( ((x)<(y)) ? (x) : (y) )
#define	MID(x,y,z)  ( ((x)>(y)) ? ( ((y)>(z)) ? (y) : (z) ) : (x) )
#define ABS(x)	    ( ((x)>= 0) ? (x) : (-(x)) )

#ifdef	MUD	/* Mud values */

/* MAX MIN */
#define RANGE(x,y)  ( ((x)>(y)) ?					\
			( rand()%((x)-(y)) ) + (y) :			\
			( rand()%((y)-(x)) ) + (x)			\
		    )

/* Score slots */
#define HIT(x)	(db[x].mudv[0])
#define MGC(x)	(db[x].mudv[1])
#define EXP(x)	(db[x].mudv[2])
#define WGT(x)	(db[x].mudv[3])
#define FGT(x)	(db[x].mudv[4])

/* Max values */
#define KILLEV	     5
#define TSKLEV	     5
#define ABSHIT	  2000
#define ABSMGC	   500
#define ABSWGT	  1000
#define ABSEXP	999999
#define ABSDAM	   200
#define	ABSARM	   150

/* Levels and regeneration */
#define	LEVEXP		( 10000 )
#define TOPEXP		( LEVEXP / 5 )
#define LEVEL(x)	( (GETEXP(x) / LEVEXP) + 1 )

/* Maxes */
#define MAXLEV		((ABSEXP/LEVEXP) + 1)
#define MAXHIT(x)	((LEVEL(x)*ABSHIT)/MAXLEV)
#define MAXMGC(x)	((LEVEL(x)*ABSMGC)/MAXLEV)
#define MAXWGT(x)	((LEVEL(x)*ABSWGT)/MAXLEV)

/* Mobiles */
#define	GETHIT(x)	MID(MAXHIT(x),HIT(x),0)	/* Health */
#define	GETMGC(x)	MID(MAXMGC(x),MGC(x),0)	/* Magic Power */
#define	GETEXP(x)	MID(ABSEXP,   EXP(x),0)	/* Worth */
#define	GETWGT(x)	MID(MAXWGT(x),WGT(x),0)	/* Weight */
#define GETFGT(x)	MID(db_top-1,FGT(x),-1)	/* Mob attacking */

/* Things */
#define GETDAM(x)	MID(ABSDAM,HIT(x),0)	/* Damage Power */
#define GETARM(x)	MID(ABSARM,MGC(x),0)	/* Armor Protection */

#define SETHIT(x,y)	(HIT(x)=MID(MAXHIT(x),(y),0))
#define SETMGC(x,y)	(MGC(x)=MID(MAXMGC(x),(y),0))
#define SETEXP(x,y)	(EXP(x)=((LEVEL(x)-1)*LEVEXP)+MID(LEVEXP-1,(y)%LEVEXP,0))
#define ADDEXP(x,y)	(EXP(x)=((LEVEL(x)-1)*LEVEXP)+MID(LEVEXP-1,((y)%LEVEXP) + (GETEXP(x)%LEVEXP),0))
#define SETLEV(x,y)	(EXP(x)=(MID(MAXLEV,(y),1)-1)*LEVEXP)
#define SETWGT(x,y)	(WGT(x)=MID(ABSWGT,(y),0))
#define SETFGT(x,y)	(FGT(x)=MID(db_top-1,(y),-1))

#define SETDAM(x,y)	(HIT(x)=MID(ABSDAM,(y),0))
#define SETARM(x,y)	(MGC(x)=MID(ABSARM,(y),0))

#define MOB(x)	    ((Typeof(x)==TYPE_THING) && (FLAGS(x)&ZOMBIE) &&	\
			(FLAGS(x)&KILL_OK)				\
		    )

#define ITEM(x)	    ((Typeof(x)==TYPE_THING) && !(FLAGS(x)&ZOMBIE) &&	\
			(FLAGS(x)&KILL_OK)				\
		    )

#define KILLER(x)   ( MOB(x) || ( (Typeof(x)==TYPE_PLAYER) &&		\
			(FLAGS(x)&KILL_OK)				\
		    ) )

#define MUDLOC(x)   ((getloc(x)!=NOTHING) && (FLAGS(getloc(x))&KILL_OK))
#define HAVLOC(x)   ((getloc(x)!=NOTHING) && (FLAGS(getloc(x))&HAVEN))

#define MUDDER(x)   (KILLER(x) && MUDLOC(x))

#define KILLABLE(x) (KILLER(x) && ( 					\
			MUDLOC(x) &&					\
			!(HAVLOC(x)) &&					\
			(LEVEL(x)>=KILLEV)				\
		    ) )

#endif /* MUD */

/* Quotas */

#define has_quotas(x)		(tp_building_quotas && !TMage(x)	\
				&& ( tp_quotas_with_bbits || !Builder(x) ))
#define QROOM(x)		(quota_check((x),NOTHING,TYPE_ROOM))
#define QEXIT(x)		(quota_check((x),NOTHING,TYPE_EXIT))
#define QTHING(x)		(quota_check((x),NOTHING,TYPE_THING))
#define QPROGRAM(x)		(quota_check((x),NOTHING,TYPE_PROGRAM))

/* defines for possible data access mods. */
#define GETMESG(x,y)		(tp_default_messages ? get_default_class(x, y) : get_property_class(x, y))
#ifdef MUD
#define GETONAME(x)		GETMESG(x, "_/ona")
#endif
#define GETDESC(x)		GETMESG(x, "_/de")
#define GETIDESC(x)		GETMESG(x, "_/ide")
#define GETSUCC(x)		GETMESG(x, "_/sc")
#define GETOSUCC(x)		GETMESG(x, "_/osc")
#define GETFAIL(x)		GETMESG(x, "_/fl")
#define GETOFAIL(x)		GETMESG(x, "_/ofl")
#define GETDROP(x)		GETMESG(x, "_/dr")
#define GETODROP(x)		GETMESG(x, "_/odr")
#define GETDOING(x)		GETMESG(x, "_/do")
#define GETOECHO(x)		GETMESG(x, "_/oecho")
#define GETPECHO(x)		GETMESG(x, "_/pecho")
#define GETCONTENTS(x)		GETMESG(x, "_/co")
#define GETPOS(x)		GETMESG(x, PROP_POS)
#define GETSEX(x)		GETMESG(x, PROP_SEX)
#define GETSPECIES(x)		GETMESG(x, PROP_SPECIES)

#define SETMESG(x,y,z)		{add_property(x, y, z, 0);}
#ifdef MUD
#define SETONAME(x,y)		SETMESG(x,"_/ona",y)
#endif
#define SETDESC(x,y)		SETMESG(x,"_/de",y)
#define SETIDESC(x,y)		SETMESG(x,"_/ide",y)
#define SETSUCC(x,y)		SETMESG(x,"_/sc",y)
#define SETFAIL(x,y)		SETMESG(x,"_/fl",y)
#define SETDROP(x,y)		SETMESG(x,"_/dr",y)
#define SETOSUCC(x,y)		SETMESG(x,"_/osc",y)
#define SETOFAIL(x,y)		SETMESG(x,"_/ofl",y)
#define SETODROP(x,y)		SETMESG(x,"_/odr",y)
#define SETDOING(x,y)		SETMESG(x,"_/do",y)
#define SETOECHO(x,y)		SETMESG(x,"_/oecho",y)
#define SETPECHO(x,y)		SETMESG(x,"_/pecho",y)
#define SETPOS(x,y)		SETMESG(x,PROP_POS,y)
#define SETSEX(x,y)		SETMESG(x,PROP_SEX,y)
#define SETSPECIES(x,y)		SETMESG(x,PROP_SPECIES,y)
#define SETCONTENTS(x,y)	SETMESG(x,"_/co",y)

#define LOADMESG(x,y,z)		{add_prop_nofetch(x,y,z,0); DBDIRTY(x);}
#define LOADDESC(x,y)		LOADMESG(x,"_/de",y)
#define LOADIDESC(x,y)		LOADMESG(x,"_/ide",y)
#define LOADSUCC(x,y)		LOADMESG(x,"_/sc",y)
#define LOADFAIL(x,y)		LOADMESG(x,"_/fl",y)
#define LOADDROP(x,y)		LOADMESG(x,"_/dr",y)
#define LOADOSUCC(x,y)		LOADMESG(x,"_/osc",y)
#define LOADOFAIL(x,y)		LOADMESG(x,"_/ofl",y)
#define LOADODROP(x,y)		LOADMESG(x,"_/odr",y)

#define SETLOCK(x,y)	{set_property(x, "_/lok", PROP_LOKTYP, (PTYPE)y);}
#define LOADLOCK(x,y)	{set_property_nofetch(x, "_/lok", PROP_LOKTYP, (PTYPE)y); DBDIRTY(x);}

/* Bugfix: Appears to be an issue with setting a NULL lock vs just removing it */
/* #define CLEARLOCK(x)	{set_property(x, "_/lok", PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);} */
#define CLEARLOCK(x)	{remove_property(x, "_/lok");}

#define GETLOCK(x)	(get_property_lock(x, "_/lok"))

#define DB_PARMSINFO	0x0001
#define DB_COMPRESSED	0x0002

#define TYPE_ROOM	   0x0
#define TYPE_THING	   0x1
#define TYPE_EXIT	   0x2
#define TYPE_PLAYER	   0x3
#define TYPE_PROGRAM	   0x4
#define TYPE_GARBAGE	   0x6
#define NOTYPE		   0x7	/* no particular type */
#define TYPE_MASK	   0x7	/* room for expansion */
#define ANTILOCK	   0x8		/* negates key (*OBSOLETE*) */
#define W3		  0x10	/* gets automatic control */
#define LINK_OK		  0x20	/* anybody can link to this room */
#define DARK		  0x40	/* contents of room are not printed */
#define INTERNAL	  0x80	/* internal-use-only flag */
#define STICKY		 0x100	/* this object goes home when dropped */
#define BUILDER		 0x200	/* this player can use construction commands */
#define CHOWN_OK	 0x400	/* this player can be @chowned to */
#define JUMP_OK		 0x800	/* A room which can be jumped from, or */
                                /* a player who can be jumped to */
#define GENDER_MASK	0x3000	/* 2 bits of gender */
#define GENDER_SHIFT	    12	/* 0x1000 is 12 bits over (for shifting) */
#define GENDER_UNASSIGNED  0x0	/* unassigned - the default */
#define GENDER_NEUTER	   0x1	/* neuter */
#define GENDER_FEMALE	   0x2	/* for women */
#define GENDER_MALE        0x3	/* for men */
#define GENDER_BOTH	   0x4  /* For both */

#define KILL_OK		 0x4000	/* Kill_OK bit.  Means you can be killed. */

#define HAVEN		0x10000	/* can't kill here */
#define ABODE		0x20000	/* can set home here */

#define W1		0x40000	/* programmer */

#define QUELL		0x80000	/* When set, a wizard is considered to not be
				 * a wizard. */
#define W2	       0x100000	/* second programmer bit.  For levels */

#define INTERACTIVE    0x200000 /* when this is set, player is either editing
				 * a program or in a READ. */
#define OBJECT_CHANGED 0x400000 /* when an object is dbdirty()ed, set this */
#define SAVED_DELTA    0x800000 /* object last saved to delta file */

#define VEHICLE       0x1000000 /* Vehicle flag */
#define ZOMBIE        0x2000000 /* Zombie flag */

#define LISTENER      0x4000000 /* listener flag */
#define XFORCIBLE     0x8000000 /* externally forcible flag */
#define SANEBIT      0x10000000 /* used to check db sanity */

#define READMODE     0x10000000 /* when set, player is in a READ */


/* F2 flags */

#define F2GUEST		    0x1 /* Guest character */
#define F2LOGWALL	    0x2 /* Wizard sees logs walled */
#define F2MPI	    	    0x4 /* Object can parse MPI */
#define F2IC		    0x8 /* In Character roleplay flag (was Ethereal) */
#define F2OFFER		   0x10 /* Offer/task pending object */
#define F2PUEBLO	   0x20 /* Pueblo-support flag */
#define F2WWW              0x40 /* Object may be accessed via the web */
#define F2MCP              0x80 /* Program is a MUCK C Program (neon-compat) */
#define F2TINKERPROOF	  0x100 /* Object is wizard-proof */
#define F2SUSPECT	  0x200 /* Suspect flag for TinyJerks */
#define F2IDLE		  0x400 /* Player always looks idle */
#define F2READBLANKLINE	  0x800 /* 'read' will return blank lines */
#define F2CRONTAB	 0x1000 /* Cron Tab Entry (neon-compat) */
#define F2LOKI		 0x2000 /* A flag for Loki (neon-compat) */
#define F2LOKI2		 0x4000 /* Another flag for Loki (neon-compat) */

/* Some Neon -> Glow notes about the flag2 set */
/***********************************************
  NeonMuck has become unsupported and so I made some changes to the Glow
  second flag set to allow using Neon DBs on Glow without losing any
  important functionality.  To do this the Glow SUSPECT and IDLE flags were
  moved to higher numbers.  Most mucks probably don't use either of these,
  and the Neon flags taking their place have no meaning currently in Glow. 
  The Neon MUFCOUNT and ETHEREAL flags also have no meaning in Glow, and so
  it should be safe to allow the MPI and IC flags to exist as they are.
  Specifically F2IDLE was moved from 0x80 to 0x400 and F2SUSPECT was moved
  from 0x20 to 0x200.
 ***********************************************/

/* what flags to NOT dump to disk. */
#define DUMP_MASK	(INTERACTIVE | SAVED_DELTA | OBJECT_CHANGED | LISTENER | READMODE | SANEBIT)
#define DUM2_MASK	(F2PUEBLO)

typedef int object_flag_type;

#define DoNull(s) ((s) ? (s) : "")
#define Typeof(x) ((x == HOME) ? TYPE_ROOM : (FLAGS(x) & TYPE_MASK))

/* Dbref of the Man */
#define MAN	(1)

#define LMAN	(8)
#define LBOY	(7)
#define LARCH	(6)
#define LWIZ	(5)
#define LMAGE	(4)
#define LM3	(3)
#define LM2	(2)
#define LM1	(1)
#define LMUF	(1)
#define LPLAYER (0)

#define TMan(x)		( (x) == MAN )
#define Man(x)		( (FLAGS(x) & QUELL) ? 0 : TMan(x) )

extern int RawMLevel(dbref player);
extern int MLevel(dbref player);
extern int WLevel(dbref player);
extern void SetMLevel(dbref player, int mlev);

#define PLevel(x)	( (FLAGS(x) & ABODE) ? 0 : MLevel(x) + 1 )

#define QLevel(x)	( (FLAGS(x) & QUELL) ? 0 : MLevel(x) )

#define TBoy(x)		(MLevel(x) >= LBOY)
#define Boy(x)		(QLevel(x) >= LBOY)

#define TArch(x)	(MLevel(x) >= LARCH)
#define TWiz(x)		(MLevel(x) >= LWIZ)
#define TMage(x)	(MLevel(x) >= LMAGE)

#define Arch(x)		(QLevel(x) >= LARCH)
#define Wiz(x)		(QLevel(x) >= LWIZ)
#define Mage(x)		(QLevel(x) >= LMAGE)

#define Mucker3(x)	(MLevel(x) >= LM3)
#define Mucker2(x)	(MLevel(x) >= LM2)
#define Mucker1(x)	(MLevel(x) >= LM1)
#define Mucker(x)	(MLevel(x) >= LMUF)


#define PREEMPT 0
#define FOREGROUND 1
#define BACKGROUND 2

#define Dark(x) ( (FLAGS(x) & DARK) != 0 )

#define Builder(x) ( (FLAGS(x) & BUILDER) || TMage(x) )
#define Meeper(x) ( (FLAG2(x) & F2MPI) || TMage(x) )

#define Guest(x) ( (FLAG2(x) & F2GUEST) && !TMage(x) )

#define Viewable(x) ( FLAGS(x) & VEHICLE )

#define Linkable(x) ((x) == HOME || \
                     (((Typeof(x) == TYPE_ROOM || Typeof(x) == TYPE_THING) ? \
                      (FLAGS(x) & ABODE) : (FLAGS(x) & LINK_OK)) != 0))


/* Boolean expressions, for locks */
typedef char boolexp_type;

#define BOOLEXP_AND 0
#define BOOLEXP_OR 1
#define BOOLEXP_NOT 2
#define BOOLEXP_CONST 3
#define BOOLEXP_PROP 4

struct boolexp {
    boolexp_type type;
    struct boolexp *sub1;
    struct boolexp *sub2;
    dbref   thing;
    struct plist *prop_check;
};

#define TRUE_BOOLEXP ((struct boolexp *) 0)

/* special dbref's */
#define NOTHING ((dbref) -1)	/* null dbref */
#define AMBIGUOUS ((dbref) -2)	/* multiple possibilities, for matchers */
#define HOME ((dbref) -3)	/* virtual room, represents mover's home */

/* editor data structures */

/* Line data structure */
struct line {
    const char *this_line;	/* the line itself */
    struct line *next, *prev;	/* the next line and the previous line */
};

/* stack and object declarations */
/* Integer types go here */
#define PROG_CLEARED     0
#define PROG_PRIMITIVE   1	/* forth prims and hard-coded C routines */
#define PROG_INTEGER     2	/* integer types */
#define PROG_OBJECT      3	/* database objects */
#define PROG_VAR         4	/* variables */
#define PROG_LVAR        5	/* variables */
/* Pointer types go here, numbered *AFTER* PROG_STRING */
#define PROG_STRING      6	/* string types */
#define PROG_FUNCTION    7	/* function names for debugging. */
#define PROG_LOCK        8	/* boolean expression */
#define PROG_ADD         9	/* program address - used in calls&jmps */
#define PROG_IF          10	/* A low level IF statement */
#define PROG_EXEC        11	/* EXECUTE shortcut */
#define PROG_JMP         12	/* JMP shortcut */

#define MAX_VAR         54	/* maximum number of variables including the
				 * basic ME and LOC                */
#define RES_VAR          4	/* no of reserved variables */

#define STACK_SIZE       1024	/* maximum size of stack */

struct shared_string {		/* for sharing strings in programs */
    int     links;		/* number of pointers to this struct */
    int     length;		/* length of string data */
    char    data[1];		/* shared string data */
};

struct prog_addr {              /* for 'addres references */
    int     links;              /* number of pointers */
    dbref   progref;            /* program dbref */
    struct inst *data;          /* pointer to the code */
};

struct stack_addr {             /* for the system calstack */
    dbref   progref;            /* program call was made from */
    struct inst *offset;        /* the address of the call */
};

struct inst {			/* instruction */
    short   type;
    short   line;
    union {
	struct shared_string *string;  /* strings */
	struct boolexp *lock;   /* booleam lock expression */
	int     number;		/* used for both primitives and integers */
	dbref   objref;		/* object reference */
	struct inst *call;	/* use in IF and JMPs */
	struct prog_addr *addr; /* the result of 'funcname */
    }       data;
};

typedef struct inst vars[MAX_VAR];

struct stack {
    int     top;
    struct inst st[STACK_SIZE];
};

struct sysstack {
    int     top;
    struct stack_addr st[STACK_SIZE];
};

struct callstack {
    int     top;
    dbref   st[STACK_SIZE];
};

struct varstack {
    int     top;
    vars   *st[STACK_SIZE];
};


#define MAX_BREAKS 16
struct debuggerdata {
    unsigned debugging:1;   /* if set, this frame is being debugged */
    unsigned bypass:1;      /* if set, bypass breakpoint on starting instr */
    unsigned isread:1;      /* if set, the prog is trying to do a read */
    unsigned showstack:1;   /* if set, show stack debug line, each inst. */
    int lastlisted;         /* last listed line */
    char *lastcmd;          /* last executed debugger command */
    short breaknum;         /* the breakpoint that was just caught on */

    dbref lastproglisted;   /* What program's text was last loaded to list? */
    struct line *proglines; /* The actual program text last loaded to list. */

    short count;            /* how many breakpoints are currently set */
    short temp[MAX_BREAKS];         /* is this a temp breakpoint? */
    short level[MAX_BREAKS];        /* level breakpnts.  If -1, no check. */
    struct inst *lastpc;            /* Last inst interped.  For inst changes. */
    struct inst *pc[MAX_BREAKS];    /* pc breakpoint.  If null, no check. */
    int pccount[MAX_BREAKS];        /* how many insts to interp.  -2 for inf. */
    int lastline;                   /* Last line interped.  For line changes. */
    int line[MAX_BREAKS];           /* line breakpts.  -1 no check. */
    int linecount[MAX_BREAKS];      /* how many lines to interp.  -2 for inf. */
    dbref prog[MAX_BREAKS];         /* program that breakpoint is in. */
};

#define STD_REGUID 0
#define STD_SETUID 1
#define STD_HARDUID 2

/* frame data structure necessary for executing programs */
struct frame {
    struct frame *next;
    struct sysstack system;	/* system stack */
    struct stack argument;	/* argument stack */
    struct callstack caller;	/* caller prog stack */
    struct varstack varset;	/* local variables */
    vars    variables;		/* global variables */
    struct inst *pc;		/* next executing instruction */
    int     writeonly;		/* This program should not do reads */
    int     multitask;		/* This program's multitasking mode */
    int     perms;              /* permissions restrictions on program */
    int     level;		/* prevent interp call loops */
    short   already_created;	/* this prog already created an object */
    short   been_background;	/* this prog has run in the background */
    dbref   trig;		/* triggering object */
    dbref   prog;		/* program dbref */
    dbref   player;		/* person who ran the program */
    time_t  started;		/* When this program started. */
    int     instcnt;		/* How many instructions have run. */
    int     pid;		/* what is the process id? */
    struct debuggerdata brkpt;  /* info the debugger needs */
    struct timeval proftime;    /* profiling timing code */
    struct timeval totaltime;   /* profiling timing code */
};


struct publics {
    char   *subname;
    union {
	struct inst *ptr;
	int     no;
    }       addr;
    struct publics *next;
};

/* union of type-specific fields */

union specific {      /* I've been railroaded! */
    struct {			/* ROOM-specific fields */
	dbref   dropto;
    }       room;
    struct {			/* THING-specific fields */
	dbref   home;
	dbref	leader;		/* leader following, -dbref = requesting */
	int     value;
    }       thing;
    struct {			/* EXIT-specific fields */
	int     ndest;
	dbref  *dest;
    }       exit;
    struct {			/* PLAYER-specific fields */
	dbref   home;
	dbref	leader;		/* leader following, -dbref = requesting */
	int     pennies;
	dbref   curr_prog;	/* program I'm currently editing */
	short   insert_mode;	/* in insert mode? */
	short   block;
	const char *password;
        int     *descrs;
        short   descr_count;
	int	linewrap;	/* Linewrap columns */
	int	more;		/* # lines in more page */
	char	*ansi;		/* List of 24 ANSI color settings */
	dbref	ignores;	/* # of ignored in list */
	dbref	*ignoring;	/* List of ignored dbrefs */
    }       player;
    struct {			/* PROGRAM-specific fields */
	short   curr_line;	/* current-line */
	unsigned short instances;  /* #instances of this prog running */
	int     siz;		/* size of code */
	struct inst *code;	/* byte-compiled code */
	struct inst *start;	/* place to start executing */
	struct line *first;	/* first line */
	struct publics *pubs;	/* public subroutine addresses */
    }       program;
};


/* timestamps record */

struct timestamps {
    time_t created;
    time_t modified;
    time_t lastused;
    int    usecount;
};


struct object {

    const char *name;
    dbref   location;		/* pointer to container */
    dbref   owner;
    dbref   contents;
    dbref   exits;
    dbref   next;		/* pointer to next in contents/exits chain */
    struct plist *properties;

#ifdef DISKBASE
    int	    propsfpos;
    time_t  propstime;
    dbref   nextold;
    dbref   prevold;
    short   propsmode;
    short   spacer;
#endif

    object_flag_type flags, flag2;
    struct timestamps ts;
    union specific sp;
#ifdef MUD
    int     mudv[5];
#endif
    unsigned int     mpi_prof_sec;
    unsigned int     mpi_prof_usec;
    unsigned int     mpi_prof_use;
};

struct macrotable {
    char   *name;
    char   *definition;
    dbref   implementor;
    struct macrotable *left;
    struct macrotable *right;
};

/* Possible data types that may be stored in a hash table */
union u_hash_data {
    int     ival;		/* Store compiler tokens here */
    dbref   dbval;		/* Player hashing will want this */
    void   *pval;		/* compiler $define strings use this */
};

/* The actual hash entry for each item */
struct t_hash_entry {
    struct t_hash_entry *next;	/* Pointer for conflict resolution */
    const char *name;		/* The name of the item */
    union u_hash_data dat;	/* Data value for item */
};

typedef union u_hash_data hash_data;
typedef struct t_hash_entry hash_entry;
typedef hash_entry *hash_tab;

#define PLAYER_HASH_SIZE   (1024)	/* Table for player lookups */
#define COMP_HASH_SIZE     (256)	/* Table for compiler keywords */
#define DEFHASHSIZE        (256)	/* Table for compiler $defines */

extern struct object *db;
extern struct macrotable *macrotop;
extern dbref db_top;
#define OkObj(x)  ( ((x) >= 0) && ((x) < db_top) )
#define OkRoom(x) ( OkObj(x) && (Typeof(x)==TYPE_ROOM) )
#define EnvRoomX  ( (dbref) OkRoom(GLOBAL_ENVIRONMENT) ? GLOBAL_ENVIRONMENT : 0 )
#define EnvRoom   ( (dbref) (OkRoom(tp_environment_room) ? tp_environment_room : EnvRoomX) )
#define RootRoom  ( (dbref) (OkRoom(tp_player_start) ? tp_player_start : EnvRoomX) )
#define OkType(x) (	(Typeof(x)==TYPE_THING) ||	\
			(Typeof(x)==TYPE_ROOM) ||	\
			(Typeof(x)==TYPE_EXIT) ||	\
			(Typeof(x)==TYPE_PLAYER) ||	\
			(Typeof(x)==TYPE_PROGRAM)	)

#ifndef MALLOC_PROFILING
extern char *alloc_string(const char *);
#endif

extern struct line *get_new_line(void);

extern void autostart_progs(void);

extern struct line *read_program(dbref i);

extern int fetch_propvals(dbref obj, const char *dir);

extern dbref getparent(dbref obj);

extern dbref db_write_deltas(FILE *f);

extern void free_prog_text(struct line * l);

extern void write_program(struct line * first, dbref i);

extern void log_program_text(struct line * first, dbref player, dbref i);

extern struct shared_string *alloc_prog_string(const char *);

extern dbref new_object(void);	/* return a new object */

extern dbref getref(FILE *);	/* Read a database reference from a file. */

extern void putref(FILE *, dbref);	/* Write one ref to the file */

extern struct boolexp *getboolexp(FILE *);	/* get a boolexp */
extern void putboolexp(FILE *, struct boolexp *);	/* put a boolexp */

extern int db_write_object(FILE *, dbref);	/* write one object to file */

extern dbref db_write(FILE * f);/* write db to file, return # of objects */

extern dbref db_read(FILE * f);	/* read db from file, return # of objects */

 /* Warning: destroys existing db contents! */

extern void db_free(void);

extern dbref parse_dbref(const char *);	/* parse a dbref */

extern int  number(const char *s);

extern void putproperties(FILE *f, int obj);

extern void getproperties(FILE *f, int obj);

extern void free_line(struct line *l);

extern void db_free_object(dbref i);

extern void db_clear_object(dbref i);

extern void macrodump(struct macrotable *node, FILE *f);

extern void macroload(FILE * f);


#define DOLIST(var, first) \
  for((var) = (first); (var) != NOTHING; (var) = DBFETCH(var)->next)
#define PUSH(thing, locative) \
    {DBSTORE((thing), next, (locative)); (locative) = (thing);}
#define getloc(thing) (DBFETCH(thing)->location)

/*
  Usage guidelines:

  To obtain an object pointer use DBFETCH(i).  Pointers returned by DBFETCH
  may become invalid after a call to new_object().

  To update an object, use DBSTORE(i, f, v), where i is the object number,
  f is the field (after ->), and v is the new value.

  If you have updated an object without using DBSTORE, use DBDIRTY(i) before
  leaving the routine that did the update.

  When using PUSH, be sure to call DBDIRTY on the object which contains
  the locative (see PUSH definition above).

  Some fields are now handled in a unique way, since they are always memory
  resident, even in the GDBM_DATABASE disk-based muck.  These are: name,
  flags and owner.  Refer to these by NAME(i), FLAGS(i) and OWNER(i).
  Always call DBDIRTY(i) after updating one of these fields.

  The programmer is responsible for managing storage for string
  components of entries; db_read will produce malloc'd strings.  The
  alloc_string routine is provided for generating malloc'd strings
  duplicates of other strings.  Note that db_free and db_read will
  attempt to free any non-NULL string that exists in db when they are
  invoked.
*/
#endif				/* __DB_H */
