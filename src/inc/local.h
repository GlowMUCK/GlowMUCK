/*
 * local.h
 * $Revision: 1.4 $ $Date: 2008/12/29 15:29:13 $
 * 
 * Local Glowmuck compiler directives file
 *
 */

 /*
     Put any muck-specific #define changes in this file instead of modifying
     other header files.

     Originally, this file was entirely empty. To simplify the installation of
     new mucks, I've provided a template version here that contains all of the
     current compiler directives. Simply go through this file and modify the
     ones you wish to change. Be sure to keep a copy of this file in a safe
     place so that you can upgrade easily later. (IDE 2006-July-29)

     Changes in this file will override anything in:
	config.h
	params.h
	defaults.h

     Preclude any such #defines by first #undefining the entity.  If you don't
     first #undef the entity, you will get compiler errors. No 'real' defines
     will be put in this file, you may safely overwrite it with your local
     changes.
	
     For example: Say MPI_SUCKS is normally defined to "MPI sucks" in
     defaults.h and you want to change it:

*/

#ifdef MPI_SUCKS
    #undef MPI_SUCKS
#endif
#define MPI_SUCKS "I hate MPI!"

/* Not that I have anything against MPI. O:) -Andy */

/* For toggles, you simply check what you want, and if it isn't, make it so */
/* For Example: */
#ifdef ANOTHER_OPTION_I_DONT_WANT
#undef ANOTHER_OPTION_I_DONT_WANT
#endif

/* OR */
#ifndef ANOTHER_OPTION_I_DO_WANT
#define ANOTHER_OPTION_I_DO_WANT
#endif

/************************************************************************* 
 * Administrative Options 
 *
 * Various things that affect how the muck operates and what priviledges
 * are compiled in.
 */

/* Detaches the process as a daemon so that it don't cause problems
 * keeping a terminal line open and such. Logs normal output to a file
 * and writes out a glowmuck.pid file 
 */
#ifdef DETACH
#undef DETACH
#endif

/* Use to compress string data */
/* With today's technology this seems unnecessary. Most glowmuck servers will
 * have plenty of space for their database files. Therefore, I recommend
 * turning this option off. (IDE 2008-December-29)
 * 
 */

#ifdef COMPRESS
#undef COMPRESS
#endif

/*  To use a simple disk basing scheme where properties aren't loaded
 *  from the input file until they are needed, define this.
 *  In my opinion, with modern equipment, this is a waste of time,
 *  Unless you have a big muck with a provider that gives you a fairly
 *  restrictive memory limit, I'd leave this undefined. (IDE 2006-July-29)
 */
#ifdef DISKBASE
#undef DISKBASE
#endif

/*   To make the server save using fast delta dumps that only write out the
 *   changed objects, except when @dump or @shutdown are used, or when too
 *   many deltas have already been saved to disk, #define this.
 *   I would leave this undefined unless your full database dumps reach a
 *   size that they are taking too long to save, or you have an extremely
 *   slow disk drive.
 */
#ifdef DELTADUMPS
#undef DELTADUMPS
#endif

/*
 * Ports where tinymuck lives -- Note: If you use a port lower than
 * 1024, then the program *must* run suid to root!
 * Which I really don't recommend. You do so at your own risk!
 * Port 4201 is a port with historical significance, as the original
 * TinyMUD Classic ran on that port. It was the office number of James
 * Aspnes, who wrote TinyMUD from which TinyMUCK eventually was derived.
 */
#ifdef TINYPORT
#undef TINYPORT
#endif
#define TINYPORT 9999           /* Port that tinymuck uses for playing */

/* Define MORTWHO to make it so wizards need to type WHO! to see hosts */
/* When undefined, WHO! will show the mortal WHO+@doing (without going Q) */
#ifndef MORTWHO
#define MORTWHO
#endif

/* Define to compile in RWHO support */
#ifdef RWHO
#undef RWHO
#endif

/* Define to compile in MPI support */
#ifndef MPI
#define MPI
#endif

/* Define to compile in MUD support */
#ifdef MUD
#undef MUD
#endif

/* Define to compile in roleplaying support */
#ifndef ROLEPLAY
#define ROLEPLAY
#endif

/* Define to compile in HTTPD server WWW page support */
#ifndef HTTPD
#define HTTPD
#endif

/* Define this to do proper delayed link references for HTTPD */
/* The _/www:http://... links won't work on some clients without it. */
/* This shouldn't be necessary, but its here just in case */
#ifdef HTTPDELAY
#undef HTTPDELAY
#endif

/* Allows the Heartbeat processes to run. Store a dbref in the #0=/@heartbeat 
 * propdir for periodic execution.
 */

#ifndef HEARTBEAT
#define HEARTBEAT
#endif

/* Defines how long between heartbeats (in seconds) */
#ifdef BASE_PULSE
#undef BASE_PULSE
#endif
#define BASE_PULSE 15

/* Define this if you want the {ansi} mpi prim to not return anything. */
/* Normally it returns the uncolored version of the passed string. */
#ifdef NULL_ANSI
#undef NULL_ANSI
#endif

/* Define this if you want the {oansi} mpi prim to not return anything. */
/* Normally it returns the uncolored version of the passed string. */
#ifdef NULL_OANSI
#undef NULL_OANSI
#endif

/* Define this to allow totally insecure (your account can be at risk!)   */
/* access to the system() call that allows the muck to execute ANY        */
/* arbitrary program or script on the host system.  USE AT YOUR OWN RISK! */
/* This is intended to support muf-based character request systems that   */
/* need to send an e-mail but can't use the inserver system which is more */
/* secure as it normally uses lpexec or a sister function.                */
#ifdef INSECURE_SYSTEM_MUF_PRIM
#undef INSECURE_SYSTEM_MUF_PRIM
#endif

/************************************************************************
 *  Game Options
 *
 *  These are the ones players will notice. 
 */

/* Set this to your mark for shouts, dumps, etc.  Also change @tunes */
#ifdef MARK
#undef MARK
#endif
#define MARK "[!] "

/* Make the `examine' command display full names for types and flags */
#ifndef VERBOSE_EXAMINE
#define VERBOSE_EXAMINE
#endif


/* penny related stuff */
/* amount of object endowment, based on cost */
#ifdef OBJECT_ENDOWMENT
#undef OBJECT_ENDOWMENT
#endif
#define OBJECT_ENDOWMENT(cost) (((cost)-5)/5)

#ifdef OBJECT_DEPOSIT
#undef OBJECT_DEPOSIT
#endif
#define OBJECT_DEPOSIT(endow) ((endow)*5+4)


#ifdef MAX_LINKS
#undef MAX_LINKS
#endif 
#define MAX_LINKS 10		/* maximum number of destinations for an exit*/

#ifdef PCREATE_FLAGS
#undef PCREATE_FLAGS
#endif
#define PCREATE_FLAGS (0)	/* default flag bits for created players */

#ifdef PCREATE_FLAG2
#undef PCREATE_FLAG2
#endif
#define PCREATE_FLAG2 (F2MPI)	/* default 2nd set flag bits for players */

#ifdef TCREATE_FLAGS
#undef TCREATE_FLAGS
#endif
#define TCREATE_FLAGS (0)	/* default flag bits for created things */

#ifdef TCREATE_FLAG2
#undef TCREATE_FLAG2
#endif
#define TCREATE_FLAG2 (0)	/* default 2nd set flag bits for things */

#ifdef RCREATE_FLAGS
#undef RCREATE_FLAGS
#endif
#define RCREATE_FLAGS (0)	/* default flag bits for created rooms */

#ifdef RCREATE_FLAG2
#undef RCREATE_FLAG2
#endif
#define RCREATE_FLAG2 (0)	/* default 2nd set flag bits for rooms */

#ifdef ECREATE_FLAGS
#undef ECREATE_FLAGS
#endif
#define ECREATE_FLAGS (0)	/* default flag bits for created exits */

#ifdef ECREATE_FLAG2
#undef ECREATE_FLAG2
#endif
#define ECREATE_FLAG2 (0)	/* default 2nd set flag bits for exits */

#ifdef FCREATE_FLAGS
#undef FCREATE_FLAGS
#endif
#define FCREATE_FLAGS (0)	/* default flag bits for created programs */

#ifdef FCREATE_FLAG2
#undef FCREATE_FLAG2
#endif
#define FCREATE_FLAG2 (0)	/* default 2nd set flag bits for programs */

/* Changes to naming the establishment, these need to sound good with s */
/* or 's on the end of their names, cept for the man. */

#ifdef NAMEMAN
#undef NAMEMAN
#endif
#define NAMEMAN		"the man"

#ifdef NAMECMAN
#undef NAMECMAN
#endif
#define NAMECMAN	"The man"

#ifdef NAMEFMAN
#undef NAMEFMAN
#endif
#define NAMEFMAN	"MAN"

#ifdef NAMEFARCH
#undef NAMEFARCH
#endif
#define NAMEFARCH	"ARCHWIZARD"

#ifdef NAMEWIZ
#undef NAMEWIZ
#endif
#define NAMEWIZ		"wizard"

#ifdef NAMECWIZ
#undef NAMECWIZ
#endif
#define NAMECWIZ	"Wizard"

#ifdef NAMEFWIZ
#undef NAMEFWIZ
#endif
#define NAMEFWIZ	"WIZARD"

#ifdef NAMEFMAGE
#undef NAMEFMAGE
#endif
#define NAMEFMAGE	"MAGE"

/* Change the default color scheme of the server */
/* It could be possible to include both BG and FG color combos here, */
/* just separate them by spaces */

#ifdef CCFAIL
#undef CCFAIL
#endif
#define CCFAIL	"RED"

#ifdef CCSUCC
#undef CCSUCC
#endif
#define CCSUCC	"GREEN"

#ifdef CCINFO
#undef CCINFO
#endif
#define CCINFO	"YELLOW"

#ifdef CCNOTE
#undef CCNOTE
#endif
#define CCNOTE	"WHITE"

#ifdef CCMOVE
#undef CCMOVE
#endif
#define CCMOVE	"CYAN"

/* Change these if your muck requires a different set of properties to */
/* hold gender and other special info. */

#ifdef PROP_SEX
#undef PROP_SEX
#endif
#define PROP_SEX	"sex"

#ifdef PROP_SPECIES
#undef PROP_SPECIES
#endif
#define PROP_SPECIES	"species"

#ifdef PROP_POS
#undef PROP_POS
#endif
#define PROP_POS	"pos"

/*  More pager message.  */
#ifdef MORE_MSSAGE
#undef MORE_MESSAGE
#endif
#define MORE_MESSAGE	"-- More -- (Press Return or type 'more')\r\n"

#ifdef IMORE_MESSAGE
#undef IMORE_MESSAGE
#endif
#define IMORE_MESSAGE	"-- More -- (Press Return or type 'MORE')\r\n"

#ifdef MOTD_MESSAGE
#undef MOTD_MESSAGE
#endif
#define MOTD_MESSAGE    "-- More -- (Press Return or type 'more', to avoid this type 'motd pause')\r\n"

/**************************************************************************
 *   Various Messages 
 *
 *   Printed from the server at times, esp. during login.
 */
#ifdef NOBUILD_MESG
#undef NOBUILD_MESG
#endif
#define NOBUILD_MESG	"Building is currently disabled."

#ifdef NOGUEST_MESG
#undef NOGUEST_MESG
#endif
#define NOGUEST_MESG	"This command is unavailable to guests."

#ifdef NOQUOTA_MESG
#undef NOQUOTA_MESG
#endif
#define NOQUOTA_MESG	"That would exceed your quota limit.  Type '@quota'."

#ifdef OBJ_MESG
#undef OBJ_MESG
#endif
#define OBJ_MESG	"Please use @object or @detail if possible.  Type '@object' for help."

#ifdef NOBBIT_MESG
#undef NOBBIT_MESG
#endif
#define NOBBIT_MESG	"You're not a builder."

#ifdef NOMBIT_MESG
#undef NOMBIT_MESG
#endif
#define NOMBIT_MESG	"You're not a programmer."

#ifdef NOEDIT_MESG
#undef NOEDIT_MESG
#endif
#define NOEDIT_MESG	"That is already being edited."

#ifdef NOPERM_MESG
#undef NOPERM_MESG
#endif
#define NOPERM_MESG	"Permission denied."

#ifdef WHICH_MESG
#undef WHICH_MESG
#endif
#define WHICH_MESG	"I don't know which one you mean!"

#ifdef NOTHERE_MESG
#undef NOTHERE_MESG
#endif
#define NOTHERE_MESG	"I don't see that here."

#ifdef BADKEY_MESG
#undef BADKEY_MESG
#endif
#define BADKEY_MESG	"I don't understand that key."

#ifdef WHO_MESG
#undef WHO_MESG
#endif
#define WHO_MESG	"Who?"

#ifdef NOWAY_MESG
#undef NOWAY_MESG
#endif
#define NOWAY_MESG	"You can't go that way."

#ifdef HUH_MESG
#undef HUH_MESG
#endif
#define HUH_MESG	"Need help?  Just type \"help\"."

#ifdef NOTHING_MESG
#undef NOTHING_MESG
#endif
#define NOTHING_MESG	"You see nothing special."

#ifdef DBRO_MESG
#undef DBRO_MESG
#endif
#define DBRO_MESG	"The muck is currently read-only."

/*
 * Message if someone trys the create command and your system is
 * setup for registration.
 */
#ifdef CFG_REG_MSG
#undef CFG_REG_MSG
#endif
#define CFG_REG_MSG "Your character isn't allowed to connect from this site.\r\nSend email to %s to be added to this site.\r\n"

#ifdef CFG_REG_CRE
#undef CFG_REG_CRE
#endif
#define CFG_REG_CRE "Character registration is enabled now.\r\nPlease try the request command or send email to %s.\r\n"

#ifdef CFG_NO_ID
#undef CFG_NO_ID
#endif
#define CFG_NO_ID   "Your character has no real-life information.\r\nPlease send email to %s with your real name.\r\n"

#ifdef CFG_NO_OLR
#undef CFG_NO_OLR
#endif
#define CFG_NO_OLR  "Online registration is not open now.\r\nEmail questions to %s.\r\n"

#ifdef CFG_REG_BLK
#undef CFG_REG_BLK
#endif
#define CFG_REG_BLK "Sorry, but we are not accepting %s from your site.\r\nEmail questions to %s.\r\n"

/*
 * Goodbye message.
 */
#ifdef LEAVE_MESSAGE
#undef LEAVE_MESSAGE
#endif
#define LEAVE_MESSAGE "May your heart always guide your actions."


/*
 * Error messeges spewed by the help system.
 */
#ifdef NO_NEWS_MSG
#undef NO_NEWS_MSG
#endif
#define NO_NEWS_MSG "That topic does not exist.  Type 'news topics' to list the news topics available."

#ifdef NO_HELP_MSG
#undef NO_HELP_MSG
#endif
#define NO_HELP_MSG "That topic does not exist.  Type 'help index' to list the help topics available."

#ifdef NO_MAN_MSG
#undef NO_MAN_MSG
#endif
#define NO_MAN_MSG "That topic does not exist.  Type 'man' to list the MUF topics available."

#ifdef NO_INFO_MSG
#undef NO_INFO_MSG
#endif
#define NO_INFO_MSG "That file does not exist.  Type 'info' to get a list of the info files available."
