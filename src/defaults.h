/***********************************************************************

WAIT!  Before you modify this file, see if it's possible to put your
changes in 'local.h'.  If so you will only have to copy ONE file over
between version upgrades, and won't have to worry about missing changes
or overwriting a file and losing some upgrade material.

************************************************************************/

#define DUMPWARN_MESG   MARK "Save soon."
#define DELTAWARN_MESG  MARK "Save soon."
#define DUMPDELTAS_MESG MARK "Saving..."
#define DUMPING_MESG    MARK "Saving..."
#define DUMPDONE_MESG   MARK "Done."
#define SHUTDOWN_MESG	"Going down, be back in a bit!"
#define RESTART_MESG	"Muck restarting, be back in a moment!"

/* Changes to naming the establishment, these need to sound good with s */
/* or 's on the end of their names, cept for the man. */

#define NAMEMAN		"the man"
#define NAMECMAN	"The man"
#define NAMEFMAN	"MAN"
#define NAMEFARCH	"ARCHWIZARD"
#define NAMEWIZ		"wizard"
#define NAMECWIZ	"Wizard"
#define NAMEFWIZ	"WIZARD"
#define NAMEFMAGE	"MAGE"

/* Change the default color scheme of the server */
/* It could be possible to include both BG and FG color combos here, */
/* just separate them by spaces */

#define CCFAIL	"RED"
#define CCSUCC	"GREEN"
#define CCINFO	"YELLOW"
#define CCNOTE	"WHITE"
#define CCMOVE	"CYAN"


/* Change this to the name of your muck.  ie: FurryMUCK, or Tapestries */
#define MUCKNAME	"GlowMuck"
#define WWWDIR		"_/www"

#define	PROP_LOCKOUTMSG	"@/lockout-msg"
#define	PROP_SUSPEND	"@/suspend-until"
#define PROP_SUSPENDMSG	"@/suspend-msg"
#define PROP_PRECIOUS	"@/precious"
#define PROP_ID		"@/id"
#define PROP_PW		"@/pw"
#define PROP_ANSI	"@/ansi"
#define PROP_LINEWRAP	"@/lw"
#define PROP_MORE	"@/more"
#define PROP_IGNORING	"@/ignoring"
#define PROP_MOTDPAUSE  "@/nomotdpause"
#define PROP_OBJDIR	"_obj"
#define PROP_DETDIR	"_det"
#define PROP_PATHDIR	"@u/d"
#define PROP_ALIAS      "alias"
#define PROP_ALIASDIR	"@/" PROP_ALIAS
#define PROP_QUOTA	"quota"
#define PROP_QUOTADIR	"@/" PROP_QUOTA

/* Registration stuff */
#define REG_OBJ		0 
#define REG_JERK	"@/jerks"
#define REG_NAME	"@/badnames"
#define REG_SITE	"@/sites"
#define REG_JERKSITES	"@/jerksites"
#define REG_LOGIN	"@/login"
#define REG_WELC	"@/welcome"

/* Glow flags */
#define PROP_GLOW       "glow"
#define PROP_GLOWDIR    "@/" PROP_GLOW
#define PROP_GLOWFL2	PROP_GLOWDIR "/fl2"
#define PROP_GLOWHIT	PROP_GLOWDIR "/hit"
#define PROP_GLOWMGC	PROP_GLOWDIR "/mgc"
#define PROP_GLOWEXP	PROP_GLOWDIR "/exp"
#define PROP_GLOWWGT	PROP_GLOWDIR "/wgt"
#define PROP_GLOWLDR	PROP_GLOWDIR "/ldr"

/* Change these if your muck requires a different set of properties to */
/* hold gender and other special info. */

#define PROP_SEX	"sex"
#define PROP_SPECIES	"species"
#define PROP_POS	"pos"

/* name of the monetary unit being used */
#define PENNY "pobble"
#define PENNIES "pobbles"
#define CPENNY "Pobble"
#define CPENNIES "Pobbles"

/*  More pager message.  */
#define MORE_MESSAGE	"-- More -- (Press Return or type 'more')\r\n"
#define IMORE_MESSAGE	"-- More -- (Press Return or type 'MORE')\r\n"
#define MOTD_MESSAGE    "-- More -- (Press Return or type 'more', to avoid this type 'motd pause')\r\n"

/* message seen when a player enters a line the server doesn't understand */
#define HUH_MESSAGE "Hmm?  For help just type help."

/*  Idleboot message.  */
#define IDLEBOOT_MESSAGE "*poke* *poke*  Psst!"

/*  How long someone can idle for.  */
#define MAXIDLE TIME_HOUR(4)

/*  Boot idle players?  */
#define IDLEBOOT 0

/*  How long before items reset */
#define ITEMWAIT TIME_MINUTE(7)

/*  How long before mobs reset */
#define MOBWAIT TIME_MINUTE(7)

/* Limit max number of ignores a player can maintain */
#define IGNOREMAX 250

/* Limit max number of players to allow connected?  (wizards are immune) */
#define PLAYERMAX 0

/* How many connections before blocking? */
#define PLAYERMAX_LIMIT 120

/* The mesg to warn users that nonwizzes can't connect due to connect limits */
#define PLAYERMAX_WARNMESG MARK "We're pretty full now, it may be hard to get on."

/* The mesg to display when booting a connection because of connect limit */
#define PLAYERMAX_BOOTMESG MARK "Uh oh, we're full!  Try again in a few minutes."

/* various times */
#define AGING_TIME TIME_DAY(180) /* Unused time before obj shows as old. */
#define DUMP_INTERVAL TIME_HOUR(4) /* time between dumps (or deltas) */
#define DUMP_WARNTIME TIME_MINUTE(2) /* warning time before a dump */
#define MONOLITHIC_INTERVAL TIME_DAY(1) /* max time between full dumps */
#define CLEAN_INTERVAL TIME_MINUTE(15) /* time between unused obj purges */


/* Information needed for RWHO. */
#define RWHO_INTERVAL TIME_MINUTE(4)
#define RWHO_PASSWD "potrzebie"
#define RWHO_SERVER "riemann.math.okstate.edu"

/* Mud support */
#define MOB_INTERVAL	(5)
#define MUD_INTERVAL	TIME_MINUTE(1)

/* amount of object endowment, based on cost */
#define MAX_OBJECT_ENDOWMENT 0

/* minimum costs for various things */
#define OBJECT_COST 100          /* Amount it costs to make an object    */
#define EXIT_COST 1000           /* Amount it costs to make an exit      */
#define LINK_COST 0              /* Amount it costs to make a link       */
#define ROOM_COST 1000           /* Amount it costs to dig a room        */
#define LOOKUP_COST 0            /* cost to do a scan                    */
#define MAX_PENNIES 500          /* amount at which temple gets cheap    */
#define PENNY_RATE 10            /* 1/chance of getting a penny per room */
#define START_PENNIES 500        /* # of pennies a player's created with */

/* costs of kill command */
#define KILL_BASE_COST 1000     /* prob = expenditure/KILL_BASE_COST    */
#define KILL_MIN_COST 100       /* minimum amount needed to kill        */
#define KILL_BONUS 50           /* amount of "insurance" paid to victim */


#define MAX_DELTA_OBJS 20  /* max %age of objs changed before a full dump */

/* player spam input limiters */
#define COMMAND_BURST_SIZE 500  /* commands allowed per user in a burst */
#define COMMANDS_PER_TIME 20    /* commands per time slice after burst  */
#define COMMAND_TIME_MSEC 1000  /* time slice length in milliseconds    */


/* Max %of db in unchanged objects allowed to be loaded.  Generally 5% */
/* This is only needed if you defined DISKBASED in config.h */
#define MAX_LOADED_OBJS 3

/* Max # of timequeue events allowed. */
#define MAX_PROCESS_LIMIT 100

/* Max # of timequeue events allowed for any one player. */
#define MAX_PLYR_PROCESSES 8

/* Max # of instrs in uninterruptable muf programs before timeout. */
#define MAX_INSTR_COUNT 20000


/* INSTR_SLICE is the number of instructions to run before forcing a
 * context switch to the next waiting muf program.  This is for the
 * multitasking interpreter.
 */
#define INSTR_SLICE 5000


/* Max # of instrs in uninterruptable programs before timeout. */
#define MPI_MAX_COMMANDS 500


/* PAUSE_MIN is the minimum time in milliseconds the server will pause
 * in select() between player input/output servicings.  Larger numbers
 * slow FOREGROUND and BACKGROUND MUF programs, but reduce CPU usage.
 */
#define PAUSE_MIN 100

/* FREE_FRAMES_POOL is the number of program frames that are always
 *  available without having to allocate them.  Helps prevent memory
 *  fragmentation.
 */
#define FREE_FRAMES_POOL 8

/* Use gethostbyaddr() for hostnames in logs and the wizard WHO list. */
#define HOSTNAMES 1

/* Server support of @doing (reads the _/do prop on WHOs) */
#define WHO_DOING 1

/* To enable logging of all regular commands */
#define LOG_COMMANDS 0

/* To enable logging of all INTERACTIVE commands, (muf editor, muf READ, etc) */
#define LOG_INTERACTIVE 1

/* To enable logging of wizard commands */
#define LOG_WIZARDS 0

/* To enable logging of connection commands */ /* Actually for seeing http */
#define LOG_CONNECTS 0

/* To enable logging of mud commands */
#define LOG_WWW_ACTIVITY 0

/* To enable logging of mud commands */
#define LOG_MUD_COMMANDS 0

/* Log failed commands ( HUH'S ) to status log */
#define LOG_FAILED_COMMANDS 0

/* To enable logging of guests */
#define LOG_GUESTS 0

/* Log the text of changed programs when they are saved.  This is helpful
 * for catching people who upload scanners, use them, then recycle them. */
#define LOG_PROGRAMS 1

/* give a bit of warning before a database dump. */
#define DBDUMP_WARNING 1

/* give a bit of warning before a delta dump. */
/* only warns if DBDUMP_WARNING is also 1 */
#define DELTADUMP_WARNING 1

/* clear out unused programs every so often */
#define PERIODIC_PROGRAM_PURGE 1

/* Makes WHO unavailable before connecting to a player, or when Interactive.
 * This lets you prevent bypassing of a global @who program. */
#define SECURE_WHO 1

/* Makes all items under the environment of a room set Wizard, be controlled
 * by the owner of that room, as well as by the object owner, and Wizards. */
#define REALMS_CONTROL 0

/* Allows 'listeners' (see CHANGES file) */
#define LISTENERS 0

/* Forbid listener programs of less than given mucker level. 6 = wiz2 */
#define LISTEN_MLEV 7

/* Allows listeners to be placed on objects. */
#define LISTENERS_OBJ 0

/* Searches up the room environments for listeners */
#define LISTENERS_ENV 0

/* Allow mortal players to @force around objects that are set ZOMBIE. */
#define ZOMBIES 0

/* Allow only wizards to @set the VEHICLE flag on Thing objects. */
#define WIZ_VEHICLES 0

/* Force Mucker Level 1 muf progs to prepend names on notify's to players
 * other than the using player, and on notify_except's and notify_excludes. */
#define M1_NAME_NOTIFY 1

/* Define these to let players set TYPE_THING and TYPE_EXIT objects dark. */
#define EXIT_DARKING 1
#define THING_DARKING 0

/* Define this to 1 to make sleeping players effectively DARK */
#define DARK_SLEEPERS 0

/* Define this to 1 if you want DARK players to not show up on the WHO list
 * for mortals, in addition to not showing them in the room contents. */
#define WHO_HIDES_DARK 0

/* Allow a player to use teleport-to-player destinations for exits */
#define TELEPORT_TO_PLAYER 1

/* Make using a personal action require that the source room
 * be controlled by the player, or else be set JUMP_OK */
#define SECURE_TELEPORT 0

/* With this defined to 1, exits that aren't on TYPE_THING objects will */
/* always act as if they have at least a Priority Level of 1.  */
/* Define this if you want to use this server with an old db, and don't want */
/* to re-set the Levels of all the LOOK, DISCONNECT, and CONNECT actions. */
#define COMPATIBLE_PRIORITIES 1

/* With this defined to 1, Messages and Omessages run thru the MPI parser. */
/* This means that whatever imbedded MPI commands are in the strings get */
/* interpreted and evaluated.  MPI commands are sort of like MUSH functions, */
/* only possibly more bizzarre. The users will probably love it. */
#define DO_MPI_PARSING 1

/* Only allow killing people with the Kill_OK bit. */
#define RESTRICT_KILL 1

/* To have a private MUCK, this restricts player
 * creation to Wizards using the @pcreate command */
#define REGISTRATION 1

/* Define to 1 to allow _look/ propqueue hooks. */
#define LOOK_PROPQUEUES 0

/* Define to 1 to allow locks to check down the environment for props. */
#define LOCK_ENVCHECK 0

/* Define to 0 to prevent diskbasing of property values, or to 1 to allow. */
#define DISKBASE_PROPVALS 1

/* Set the maximum number of database copies kept via @dump. */
#define MAX_DUMP_COPIES 16

/* Gender substitution lists.  These are lists enclosed in braces that
 * are used to determine if a player is male, female, or neuter.
 * smatch is used to compare these lists with the 'sex' property. */
#define GENDER_LIST_MALE   "{male|man|boy}"
#define GENDER_LIST_FEMALE "{female|woman|girl}"
#define GENDER_LIST_NEUTER "{none|neuter|eunuch}"
#define GENDER_LIST_BOTH   "{both|herm|shemale|hermaphrodite}"
