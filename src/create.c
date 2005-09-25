#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <ctype.h>

#include "color.h"
#include "db.h"
#include "props.h"
#include "tune.h"
#include "interface.h"
#include "match.h"
#include "strings.h"
#include "externs.h"

/* Commands that create new objects */

struct line *read_program(dbref i);

int link_exit(dbref player, dbref exit, char *dest_name, dbref *dest_list);

/* parse_linkable_dest()
 *
 * A utility for open and link which checks whether a given destination
 * string is valid.  It returns a parsed dbref on success, and NOTHING
 * on failure.
 */

static dbref 
parse_linkable_dest(dbref player, dbref exit,
		    const char *dest_name)
{
    dbref   dobj;		/* destination room/player/thing/link */
    static char buf[BUFFER_LEN];
    struct match_data md;

    init_match(player, dest_name, NOTYPE, &md);
    match_everything(&md);
    match_absolute(&md);
    match_home(&md);

    if ((dobj = match_result(&md)) == NOTHING || dobj == AMBIGUOUS) {
	sprintf(buf, CINFO "I couldn't find '%s'.", dest_name);
	anotify(player, buf);
	return NOTHING;

    }

    if (!tp_teleport_to_player && Typeof(dobj) == TYPE_PLAYER) {
	sprintf(buf, CFAIL "You can't link to players.  Destination %s ignored.",
		unparse_object(player, dobj));
	anotify(player, buf);
	return NOTHING;
    }

    if (!can_link(player, exit)) {
	anotify(player, CFAIL "You can't link that.");
	return NOTHING;
    }
    if (!can_link_to(player, Typeof(exit), dobj)) {
	sprintf(buf, CFAIL "You can't link to %s.", unparse_object(player, dobj));
	anotify(player, buf);
	return NOTHING;
    } else {
	return dobj;
    }
}

/* exit_loop_check()
 *
 * Recursive check for loops in destinations of exits.  Checks to see
 * if any circular references are present in the destination chain.
 * Returns 1 if circular reference found, 0 if not.
 */
int 
exit_loop_check(dbref source, dbref dest)
{

    int     i;

    if (source == dest)
	return 1;		/* That's an easy one! */
    if (Typeof(dest) != TYPE_EXIT)
	return 0;
    for (i = 0; i < DBFETCH(dest)->sp.exit.ndest; i++) {
	if ((DBFETCH(dest)->sp.exit.dest)[i] == source) {
	    return 1;		/* Found a loop! */
	}
	if (Typeof((DBFETCH(dest)->sp.exit.dest)[i]) == TYPE_EXIT) {
	    if (exit_loop_check(source, (DBFETCH(dest)->sp.exit.dest)[i])) {
		return 1;	/* Found one recursively */
	    }
	}
    }
    return 0;			/* No loops found */
}

/* use this to create an exit */
void 
do_open(dbref player, const char *direction, const char *linkto)
{
    dbref   loc, exit = NOTHING;
    dbref   good_dest[MAX_LINKS];
    char    buf[BUFFER_LEN];
    char    buf2[BUFFER_LEN];
    char   *rname, *qname;
    int     i, ndest;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (tp_restricted_building && !Builder(player)) {
	anotify(player, CFAIL NOBBIT_MESG);
	return;
    }

    if (!tp_building || tp_db_readonly) {
	anotify(player, CFAIL NOBUILD_MESG);
	return;
    }

    if(has_quotas(OWNER(player)) && !QEXIT(OWNER(player))) {
	anotify(player, CFAIL NOQUOTA_MESG);
#ifdef PATH
	if(tp_space_nag) anotify(player, CINFO PATH_MESG);
#endif
	return;
    }

    strcpy(buf2, linkto);
    for (rname = buf2; (*rname && (*rname != '=')); rname++);
    qname = rname;
    if (*rname)
	rname++;
    qname = '\0';

    while (((qname--) > buf2) && (isspace(*qname)))
	*qname = '\0';
    qname = buf2;
    for (; *rname && isspace(*rname); rname++);

    if ((loc = getloc(player)) == NOTHING)
	return;
    if (!*direction) {
	anotify(player, CINFO "You must specify a direction or action name to open.");
	return;
    } else if (!ok_name(direction)) {
	anotify(player, CINFO "That's a strange name for an exit!");
	return;
    }
    if (!controls(player, loc)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    } else if (!payfor(player, tp_exit_cost)) {
	anotify_fmt(player, CFAIL "You don't have enough %s to open an exit.",
		   tp_pennies);
	return;
    } else {
	/* create the exit */
	exit = new_object();

	/* initialize everything */
	NAME(exit) = alloc_string(direction);
	DBFETCH(exit)->location = loc;
	OWNER(exit) = OWNER(player);
	FLAGS(exit) = TYPE_EXIT | ECREATE_FLAGS;
	FLAG2(exit) = ECREATE_FLAG2;
	DBFETCH(exit)->sp.exit.ndest = 0;
	DBFETCH(exit)->sp.exit.dest = NULL;

	/* link it in */
	PUSH(exit, DBFETCH(loc)->exits);
	DBDIRTY(loc);

	/* and we're done */
	sprintf(buf, CSUCC "Exit %s created and opened.", unparse_object(player, exit));
	anotify(player, buf);
#ifdef PATH
	if(tp_space_nag) anotify(player, CINFO PATH_MESG);
#endif

	/* check second arg to see if we should do a link */
	if (*qname != '\0') {
	    anotify(player, CNOTE "Trying to link...");
	    if (!payfor(player, tp_link_cost)) {
		anotify_fmt(player, CFAIL "You don't have enough %s to link.",
			   tp_pennies);
	    } else {
		ndest = link_exit(player, exit, (char *) qname, good_dest);
		DBFETCH(exit)->sp.exit.ndest = ndest;
		DBFETCH(exit)->sp.exit.dest = (dbref *) malloc(sizeof(dbref) * ndest);
		for (i = 0; i < ndest; i++) {
		    (DBFETCH(exit)->sp.exit.dest)[i] = good_dest[i];
		}
		DBDIRTY(exit);
	    }
	}
    }

    if (*rname && (exit != NOTHING)) {
	sprintf(buf, CSUCC "Registered as '$%s'.", rname);
	anotify(player, buf);
	sprintf(buf, "_reg/%s", rname);
	set_property(player, buf, PROP_REFTYP, (PTYPE)(long int)exit);
    }
}

/*
 * link_exit()
 *
 * This routine connects an exit to a bunch of destinations.
 *
 * 'player' contains the player's name.
 * 'exit' is the the exit whose destinations are to be linked.
 * 'dest_name' is a character string containing the list of exits.
 *
 * 'dest_list' is an array of dbref's where the valid destinations are
 * stored.
 *
 */

int 
link_exit(dbref player, dbref exit, char *dest_name, dbref * dest_list)
{
    char   *p, *q;
    int     prdest;
    dbref   dest;
    int     ndest;
    char    buf[BUFFER_LEN], qbuf[BUFFER_LEN];

    prdest = 0;
    ndest = 0;

    while (*dest_name) {
	while (isspace(*dest_name))
	    dest_name++;	/* skip white space */
	p = dest_name;
	while (*dest_name && (*dest_name != EXIT_DELIMITER))
	    dest_name++;
	q = (char *)strncpy(qbuf, p, BUFFER_LEN);	/* copy word */
	q[(dest_name - p)] = '\0';	/* terminate it */
	if (*dest_name)
	    for (dest_name++; *dest_name && isspace(*dest_name); dest_name++);

	if ((dest = parse_linkable_dest(player, exit, q)) == NOTHING)
	    continue;

	switch (Typeof(dest)) {
	    case TYPE_PLAYER:
	    case TYPE_ROOM:
	    case TYPE_PROGRAM:
		if (prdest) {
		    sprintf(buf, CFAIL "One non-thing link allowed. Destination %s ignored.", unparse_object(player, dest));
		    anotify(player, buf);
		    continue;
		}
		dest_list[ndest++] = dest;
		prdest = 1;
		break;
	    case TYPE_THING:
		dest_list[ndest++] = dest;
		break;
	    case TYPE_EXIT:
		if (exit_loop_check(exit, dest)) {
		    sprintf(buf, CFAIL "Destination %s would create a loop, ignored.",
			    unparse_object(player, dest));
		    anotify(player, buf);
		    continue;
		}
		dest_list[ndest++] = dest;
		break;
	    default:
		anotify(player, CFAIL "Weird object type.");
		log_status("*BUG: weird object: Typeof(%d) = %d\n",
			   dest, Typeof(dest));
		break;
	}
	if (dest == HOME) {
	    anotify(player, CSUCC "Linked to HOME.");
	} else {
	    sprintf(buf, CSUCC "Linked to %s.", unparse_object(player, dest));
	    anotify(player, buf);
	}
	if (ndest >= MAX_LINKS) {
	    anotify(player, CSUCC "Too many destinations, extra ignored.");
	    break;
	}
    }
    return ndest;
}

/* do_link
 *
 * Use this to link to a room that you own.  It also sets home for
 * objects and things, and drop-to's for rooms.
 * It seizes ownership of an unlinked exit, and costs 1 penny
 * plus a penny transferred to the exit owner if they aren't you
 *
 * All destinations must either be owned by you, or be LINK_OK.
 */
void 
do_link(dbref player, const char *thing_name, const char *dest_name)
{
    dbref   thing;
    dbref   dest;
    dbref   good_dest[MAX_LINKS];
    struct match_data md;

    int     ndest, i;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (tp_db_readonly) {
	anotify(player, CFAIL DBRO_MESG);
	return;
    }

    if (tp_restricted_building && !Builder(player)) {
	anotify(player, CFAIL NOBBIT_MESG);
	return;
    }

    init_match(player, thing_name, TYPE_EXIT, &md);
    match_all_exits(&md);
    match_neighbor(&md);
    match_possession(&md);
    if (Mage(OWNER(player))) {
	match_player(&md);
    }
    match_absolute(&md);
    match_registered(&md);
    match_me(&md);
    match_here(&md);

    if ((thing = noisy_match_result(&md)) == NOTHING)
	return;

    switch (Typeof(thing)) {
	case TYPE_EXIT:
	    /* we're ok, check the usual stuff */
	    if (DBFETCH(thing)->sp.exit.ndest != 0) {
		if (controls(player, thing)) {
		    anotify(player, CINFO "That exit is already linked.");
		    return;
		} else {
		    anotify(player, CFAIL NOPERM_MESG);
		    return;
		}
	    }

	    if (tp_link_cost != 0) {
	      /* handle costs */
	      if (OWNER(thing) == OWNER(player)) {
		if (!payfor(player, tp_link_cost)) {
		  anotify_fmt(player, CFAIL "It costs %d %s to link this exit.",
			      tp_link_cost, (tp_link_cost==1)?tp_penny:tp_pennies);
		  return;
		}
	      } else {
		if (!payfor(player, tp_link_cost + tp_exit_cost)) {
		  anotify_fmt(player, CFAIL "It costs %d %s to link this exit.",
			      (tp_link_cost+tp_exit_cost),
			      (tp_link_cost+tp_exit_cost == 1)?tp_penny:tp_pennies);
		  return;
		} else if (tp_restricted_building && !Builder(player)) {
		  anotify(player, CFAIL NOBBIT_MESG);
		  return;
		} else {
		  /* pay the owner for his loss */
		  dbref   owner = OWNER(thing);
		  
		  DBFETCH(owner)->sp.player.pennies += tp_exit_cost;
		  DBDIRTY(owner);
		}
	      }
	    }

	    /* link has been validated and paid for; do it */
	    OWNER(thing) = OWNER(player);
	    ndest = link_exit(player, thing, (char *) dest_name, good_dest);
	    if (ndest == 0) {
		anotify(player, CFAIL "No destinations linked.");
		if (tp_link_cost != 0) {
		  DBFETCH(player)->sp.player.pennies += tp_link_cost;	/* Refund! */
		}
		DBDIRTY(player);
		break;
	    }
	    DBFETCH(thing)->sp.exit.ndest = ndest;
	    DBFETCH(thing)->sp.exit.dest = (dbref *) malloc(sizeof(dbref) * ndest);
	    for (i = 0; i < ndest; i++) {
		(DBFETCH(thing)->sp.exit.dest)[i] = good_dest[i];
	    }
	    break;
	case TYPE_THING:
	case TYPE_PLAYER:
	    init_match(player, dest_name, TYPE_ROOM, &md);
	    match_neighbor(&md);
	    if (Mage(OWNER(player))) {
		match_player(&md);
	    }
	    match_registered(&md);
	    match_absolute(&md);
	    match_me(&md);
	    match_here(&md);
	    if (Typeof(thing) == TYPE_THING)
		match_possession(&md);
	    if ((dest = noisy_match_result(&md)) == NOTHING)
		return;
	    if (!controls(player, thing)
		    || !can_link_to(player, Typeof(thing), dest)) {
		anotify(player, CFAIL NOPERM_MESG);
		return;
	    }
	    if (parent_loop_check(thing, dest)) {
		anotify(player, CFAIL "That would cause a parent paradox.");
		return;
	    }
	    /* do the link */
	    if (Typeof(thing) == TYPE_THING) {
		DBFETCH(thing)->sp.thing.home = dest;
	    } else
		DBFETCH(thing)->sp.player.home = dest;
	    anotify(player, CSUCC "Home set.");
	    break;
	case TYPE_ROOM:	/* room dropto's */
	    init_match(player, dest_name, TYPE_ROOM, &md);
	    match_neighbor(&md);
	    match_possession(&md);
	    if (Mage(OWNER(player))) {
		match_player(&md);
	    }
	    match_registered(&md);
	    match_absolute(&md);
	    match_home(&md);
	    if ((dest = noisy_match_result(&md)) == NOTHING)
		break;
	    if (!controls(player, thing) || !can_link_to(player, Typeof(thing), dest)
		    || (thing == dest)) {
		anotify(player, CFAIL NOPERM_MESG);
	    } else {
		DBFETCH(thing)->sp.room.dropto = dest;	/* dropto */
		anotify(player, CSUCC "Dropto set.");
	    }
	    break;
	case TYPE_PROGRAM:
	    anotify(player, CFAIL "You can't link programs to things!");
	    break;
	default:
	    anotify(player, CFAIL "Weird object type.");
	    log_status("*BUG: weird object: Typeof(%d) = %d\n",
		       thing, Typeof(thing));
	    break;
    }
    DBDIRTY(thing);
    return;
}

/*
 * do_dig
 *
 * Use this to create a room.
 */
void 
do_dig(dbref player, const char *name, const char *pname)
{
    dbref   room;
    dbref   parent;
    dbref   newparent;
    char    buf[BUFFER_LEN];
    char    rbuf[BUFFER_LEN];
    char    qbuf[BUFFER_LEN];
    char   *rname;
    char   *qname;
    struct match_data md;

    if (Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (tp_restricted_building && !Builder(player)) {
	anotify(player, CFAIL NOBBIT_MESG);
	return;
    }

    if (!tp_building || tp_db_readonly) {
	anotify(player, CFAIL NOBUILD_MESG);
	return;
    }

    if(has_quotas(OWNER(player)) && !QROOM(OWNER(player))) {
	anotify(player, CFAIL NOQUOTA_MESG);
	return;
    }

    if (*name == '\0') {
	anotify(player, CINFO "You need a name for the room.");
	return;
    }
    if (!ok_name(name)) {
	anotify(player, CINFO "That's a silly name for a room!");
	return;
    }
    if (!payfor(player, tp_room_cost)) {
	anotify_fmt(player, CFAIL "You don't have enough %s to dig a room.",
		   tp_pennies);
	return;
    }
    room = new_object();

    /* Initialize everything */
    newparent = DBFETCH(DBFETCH(player)->location)->location;
    while ((newparent != NOTHING) && !(FLAGS(newparent) & ABODE))
	newparent = DBFETCH(newparent)->location;
    if (newparent == NOTHING)
	newparent = GLOBAL_ENVIRONMENT;

    NAME(room) = alloc_string(name);
    DBFETCH(room)->location = newparent;
    OWNER(room) = OWNER(player);
    DBFETCH(room)->exits = NOTHING;
    DBFETCH(room)->sp.room.dropto = NOTHING;
    FLAGS(room) = TYPE_ROOM | (FLAGS(player) & JUMP_OK) | RCREATE_FLAGS;
    FLAG2(room) = (FLAG2(player) & F2MPI) | RCREATE_FLAG2;
    PUSH(room, DBFETCH(newparent)->contents);
    DBDIRTY(room);
    DBDIRTY(newparent);

    sprintf(buf, CSUCC "Room %s created.", unparse_object(player, room));
    anotify(player, buf);

    strcpy(buf, pname);
    for (rname = buf; (*rname && (*rname != '=')); rname++);
    qname = rname;
    if (*rname)
	*(rname++) = '\0';
    while ((qname > buf) && (isspace(*qname)))
	*(qname--) = '\0';
    qname = buf;
    for (; *rname && isspace(*rname); rname++);
    rname = strcpy(rbuf, rname);
    qname = strcpy(qbuf, qname);

    if (*qname) {
	anotify(player, CNOTE "Trying to set parent...");
	init_match(player, qname, TYPE_ROOM, &md);
	match_absolute(&md);
	match_registered(&md);
	match_here(&md);
	if ((parent = noisy_match_result(&md)) == NOTHING
		|| parent == AMBIGUOUS) {
	    anotify(player, CINFO "Parent set to default.");
	} else {
	    if (!can_link_to(player, Typeof(room), parent) || room == parent) {
		anotify(player, CFAIL "Permission denied.  Parent set to default.");
	    } else {
		moveto(room, parent);
		sprintf(buf, CSUCC "Parent set to %s.", unparse_object(player, parent));
		anotify(player, buf);
	    }
	}
    }

    if (*rname) {
	sprintf(buf, "_reg/%s", rname);
	set_property(player, buf, PROP_REFTYP, (PTYPE)(long int)room);
	sprintf(buf, CINFO "Room registered as $%s", rname);
	anotify(player, buf);
    }
}

/*
  Use this to create a program.
  First, find a program that matches that name.  If there's one,
  then we put him into edit mode and do it.
  Otherwise, we create a new object for him, and call it a program.
  */
void
do_prog(dbref player, const char *name)
{
    dbref   i;
    int     jj;
    dbref   newprog;
    char    buf[BUFFER_LEN];
    struct match_data md;

    if (Typeof(player) != TYPE_PLAYER) {
	anotify(player, CFAIL "Only players can edit programs.");
	return;
    }

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (!Mucker(player)) {
	anotify(player, CFAIL NOMBIT_MESG);
	return;
    }

    if (!tp_building || tp_db_readonly) {
	anotify(player, CFAIL NOBUILD_MESG);
	return;
    }

    if(has_quotas(OWNER(player)) && !QPROGRAM(OWNER(player))) {
	anotify(player, CFAIL NOQUOTA_MESG);
	return;
    }

    if (!*name) {
	anotify(player, CINFO "No program name given.");
	return;
    }

    init_match(player, name, TYPE_PROGRAM, &md);
    match_neighbor(&md);
    match_possession(&md);
    match_registered(&md);
    match_absolute(&md);

    if ((i = match_result(&md)) == NOTHING) {
	newprog = new_object();

	NAME(newprog) = alloc_string(name);
	sprintf(buf, "A scroll containing a spell called %s", name);
	SETDESC(newprog, buf);
	DBFETCH(newprog)->location = player;
	FLAGS(newprog) = TYPE_PROGRAM | FCREATE_FLAGS;
	FLAG2(newprog) = FCREATE_FLAG2;
	jj = MLevel(player);
	if (jj < 1)
	    jj = 2;
	if (jj > 3)
	    jj = 3;
	SetMLevel(newprog, jj);

	OWNER(newprog) = OWNER(player);
	DBFETCH(newprog)->sp.program.first = 0;
	DBFETCH(newprog)->sp.program.curr_line = 0;
	DBFETCH(newprog)->sp.program.siz = 0;
	DBFETCH(newprog)->sp.program.code = 0;
	DBFETCH(newprog)->sp.program.start = 0;
	DBFETCH(newprog)->sp.program.pubs = 0;

	DBFETCH(player)->sp.player.curr_prog = newprog;

	PUSH(newprog, DBFETCH(player)->contents);
	DBDIRTY(newprog);
	DBDIRTY(player);
	sprintf(buf, CSUCC "Program %s created.", unparse_object(player, newprog));
	anotify(player, buf);
	anotify(player, CINFO "Entering editor.");
    } else if (i == AMBIGUOUS) {
	anotify(player, CINFO WHICH_MESG);
	return;
    } else {
	if ((Typeof(i) != TYPE_PROGRAM) || !controls(player, i)) {
	    anotify(player, CFAIL NOPERM_MESG);
	    return;
	}
	if (FLAGS(i) & INTERNAL) {
	    anotify(player, CFAIL NOEDIT_MESG);
	    return;
	}
	DBFETCH(i)->sp.program.first = read_program(i);
	FLAGS(i) |= INTERNAL;
	DBFETCH(player)->sp.player.curr_prog = i;
	anotify(player, CINFO "Entering editor.");
	/* list current line */
	do_list(player, i, 0, 0);
	DBDIRTY(i);
    }
    FLAGS(player) |= INTERACTIVE;
    DBDIRTY(player);
}

void
do_edit(dbref player, const char *name)
{
    dbref   i;
    struct match_data md;

    if (Typeof(player) != TYPE_PLAYER) {
	anotify(player, CFAIL "Only players can edit programs.");
	return;
    }

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (!Mucker(player)) {
	anotify(player, CFAIL NOMBIT_MESG);
	return;
    }

    if (tp_db_readonly) {
	anotify(player, CFAIL DBRO_MESG);
	return;
    }

    if (!*name) {
	anotify(player, CINFO "No program name given.");
	return;
    }
    init_match(player, name, TYPE_PROGRAM, &md);

    match_neighbor(&md);
    match_possession(&md);
    match_registered(&md);
    match_absolute(&md);

    if ((i = noisy_match_result(&md)) == NOTHING || i == AMBIGUOUS)
	return;

    if ((Typeof(i) != TYPE_PROGRAM) || !controls(player, i)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if (FLAGS(i) & INTERNAL) {
	anotify(player, CFAIL NOEDIT_MESG);
	return;
    }
    FLAGS(i) |= INTERNAL;
    DBFETCH(i)->sp.program.first = read_program(i);
    DBFETCH(player)->sp.player.curr_prog = i;
    anotify(player, CINFO "Entering editor.");
    /* list current line */
    do_list(player, i, 0, 0);
    FLAGS(player) |= INTERACTIVE;
    DBDIRTY(i);
    DBDIRTY(player);
}

/*
 * do_create
 *
 * Use this to create an object.
 */
void 
do_create(dbref player, char *name, char *acost)
{
    dbref   loc;
    dbref   thing;
    int     cost;

    static char buf[BUFFER_LEN];
    char    buf2[BUFFER_LEN];
    char   *rname, *qname;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (tp_restricted_building && !Builder(player)) {
	anotify(player, CFAIL NOBBIT_MESG);
	return;
    }

    if (!tp_building || tp_db_readonly) {
	anotify(player, CFAIL NOBUILD_MESG);
	return;
    }

    if(has_quotas(OWNER(player)) && !QTHING(OWNER(player))) {
	anotify(player, CFAIL NOQUOTA_MESG);
#ifdef PATH
	if(tp_space_nag) anotify(player, CINFO OBJ_MESG);
#endif
	return;
    }

    strcpy(buf2, acost);
    for (rname = buf2; (*rname && (*rname != '=')); rname++);
    qname = rname;
    if (*rname)
	*(rname++) = '\0';
    while ((qname > buf2) && (isspace(*qname)))
	*(qname--) = '\0';
    qname = buf2;
    for (; *rname && isspace(*rname); rname++);

    cost = atoi(qname);
    if (*name == '\0') {
	anotify(player, CINFO "Create what?");
	return;
    } else if (!ok_name(name)) {
	anotify(player, CINFO "That's a silly name for a thing!");
	return;
    } else if (cost < 0) {
	anotify(player, CINFO "You can't create an object for less than nothing!");
	return;
    } else if (cost < tp_object_cost) {
	cost = tp_object_cost;
    }
    if (!payfor(player, cost)) {
	anotify_fmt(player, CFAIL "You don't have enough %s.", tp_pennies);
	return;
    } else {
	/* create the object */
	thing = new_object();

	/* initialize everything */
	NAME(thing) = alloc_string(name);
	DBFETCH(thing)->location = player;
	OWNER(thing) = OWNER(player);
	DBFETCH(thing)->sp.thing.value = OBJECT_ENDOWMENT(cost);
	DBFETCH(thing)->exits = NOTHING;
	FLAGS(thing) = TYPE_THING | TCREATE_FLAGS;
	FLAG2(thing) = TCREATE_FLAG2;

	/* endow the object */
	if (DBFETCH(thing)->sp.thing.value > tp_max_object_endowment) {
	    DBFETCH(thing)->sp.thing.value = tp_max_object_endowment;
	}
	if ((loc = DBFETCH(player)->location) != NOTHING
		&& controls(player, loc)) {
	    DBFETCH(thing)->sp.thing.home = loc;	/* home */
	} else {
	    DBFETCH(thing)->sp.thing.home = player;
	    /* set thing's home to player instead */
	}

	/* link it in */
	PUSH(thing, DBFETCH(player)->contents);
	DBDIRTY(player);

	/* and we're done */
	sprintf(buf, CSUCC "Object %s created.", unparse_object(player, thing));
	anotify(player, buf);
	DBDIRTY(thing);
    }
    if (*rname) {
	sprintf(buf, CSUCC "Registered as '$%s'.", rname);
	anotify(player, buf);
	sprintf(buf, "_reg/%s", rname);
	set_property(player, buf, PROP_REFTYP, (PTYPE)(long int)thing);
    }
#ifdef PATH
    if(tp_space_nag) anotify(player, CINFO OBJ_MESG);
#endif
}

/*
 * parse_source()
 *
 * This is a utility used by do_action and do_attach.  It parses
 * the source string into a dbref, and checks to see that it
 * exists.
 *
 * The return value is the dbref of the source, or NOTHING if an
 * error occurs.
 *
 */
dbref 
parse_source(dbref player, const char *source_name)
{
    dbref   source;
    struct match_data md;

    init_match(player, source_name, NOTYPE, &md);	/* source type can be
							 * any */
    match_neighbor(&md);
    match_possession(&md);
    if (Mage(OWNER(player))) {
	match_player(&md);
    }
    match_registered(&md);
    match_absolute(&md);
    match_me(&md);
    match_here(&md);
    source = noisy_match_result(&md);

    if (source == NOTHING)
	return NOTHING;

    /* You can only attach actions to things you control */
    if (!controls(player, source)) {
	anotify(player, CFAIL NOPERM_MESG);
	return NOTHING;
    }
    if (Typeof(source) == TYPE_EXIT) {
	anotify(player, CFAIL "You can't attach an action to an action.");
	return NOTHING;
    }
    if (Typeof(source) == TYPE_PROGRAM) {
	anotify(player, CFAIL "You can't attach an action to a program.");
	return NOTHING;
    }
    return source;
}

/*
 * set_source()
 *
 * This routine sets the source of an action to the specified source.
 * It is called by do_action and do_attach.
 *
 */
void 
set_source(dbref player, dbref action, dbref source)
{
    switch (Typeof(source)) {
	case TYPE_ROOM:
	case TYPE_THING:
	case TYPE_PLAYER:
	    PUSH(action, DBFETCH(source)->exits);
	    break;
	default:
	    anotify(player, CFAIL "Weird object type.");
	    log_status("*BUG: tried to source %d to %d: type: %d\n",
		       action, source, Typeof(source));
	    return;
	    break;
    }
    DBDIRTY(source);
    DBSTORE(action, location, source);
    return;
}

int 
unset_source(dbref player, dbref loc, dbref action)
{

    dbref   oldsrc;

    if ((oldsrc = DBFETCH(action)->location) == NOTHING) {
	/* old-style, sourceless exit */
	if (!member(action, DBFETCH(loc)->exits)) {
	    return 0;
	}
	DBSTORE(DBFETCH(player)->location, exits,
		remove_first(DBFETCH(DBFETCH(player)->location)->exits, action));
    } else {
	switch (Typeof(oldsrc)) {
	    case TYPE_PLAYER:
	    case TYPE_ROOM:
	    case TYPE_THING:
		DBSTORE(oldsrc, exits,
			remove_first(DBFETCH(oldsrc)->exits, action));
		break;
	    default:
		log_status("PANIC: source of action #%d was type: %d.\n",
			   action, Typeof(oldsrc));
		return 0;
		/* NOTREACHED */
		break;
	}
    }
    return 1;
}

/*
 * do_action()
 *
 * This routine attaches a new existing action to a source object,
 * where possible.
 * The action will not do anything until it is LINKed.
 *
 */
void 
do_action(dbref player, const char *action_name, const char *source_name)
{
    dbref   action, source;
    static char buf[BUFFER_LEN];
    char    buf2[BUFFER_LEN];
    char   *rname, *qname;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (tp_restricted_building && !Builder(player)) {
	anotify(player, CFAIL NOBBIT_MESG);
	return;
    }

    if (!tp_building || tp_db_readonly) {
	anotify(player, CFAIL NOBUILD_MESG);
	return;
    }

    if(has_quotas(OWNER(player)) && !QEXIT(OWNER(player))) {
	anotify(player, CFAIL NOQUOTA_MESG);
#ifdef PATH
	if(tp_space_nag) anotify(player, CINFO PATH_MESG);
#endif
	return;
    }

    strcpy(buf2, source_name);
    for (rname = buf2; (*rname && (*rname != '=')); rname++);
    qname = rname;
    if (*rname)
	*(rname++) = '\0';
    while ((qname > buf2) && (isspace(*qname)))
	*(qname--) = '\0';
    qname = buf2;
    for (; *rname && isspace(*rname); rname++);

    if (!*action_name || !*qname) {
	anotify(player, CINFO "You must specify an action name and a source object.");
	return;
    } else if (!ok_name(action_name)) {
	anotify(player, CINFO "That's a strange name for an action!");
	return;
    } else if (!payfor(player, tp_exit_cost)) {
	anotify_fmt(player, CFAIL
		   "You don't have enough %s to make an action.",
		   tp_pennies);
	return;
    }
    if (((source = parse_source(player, qname)) == NOTHING))
	return;

    action = new_object();

    NAME(action) = alloc_string(action_name);
    DBFETCH(action)->location = NOTHING;
    OWNER(action) = OWNER(player);
    DBFETCH(action)->sp.exit.ndest = 0;
    DBFETCH(action)->sp.exit.dest = NULL;
    FLAGS(action) = TYPE_EXIT | ECREATE_FLAGS;
    FLAG2(action) = ECREATE_FLAG2;

    set_source(player, action, source);
    sprintf(buf, CSUCC "Action %s created and attached.", unparse_object(player, action));
    anotify(player, buf);
    DBDIRTY(action);

    if (*rname) {
	sprintf(buf, CSUCC "Registered as '$%s'.", rname);
	anotify(player, buf);
	sprintf(buf, "_reg/%s", rname);
	set_property(player, buf, PROP_REFTYP, (PTYPE)(long int)action);
    }
#ifdef PATH
    if(tp_space_nag) anotify(player, CINFO PATH_MESG);
#endif
}

/*
 * do_attach()
 *
 * This routine attaches a previously existing action to a source object.
 * The action will not do anything unless it is LINKed.
 *
 */
void 
do_attach(dbref player, const char *action_name, const char *source_name)
{
    dbref   action, source;
    dbref   loc;		/* player's current location */
    struct match_data md;

    if ((loc = DBFETCH(player)->location) == NOTHING)
	return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (tp_restricted_building && !Builder(player)) {
	anotify(player, CFAIL NOBBIT_MESG);
	return;
    }

    if (tp_db_readonly) {
	anotify(player, CFAIL DBRO_MESG);
	return;
    }

    if (!*action_name || !*source_name) {
	anotify(player, CINFO "You must specify an action name and a source object.");
	return;
    }
    init_match(player, action_name, TYPE_EXIT, &md);
    match_all_exits(&md);
    if (Mage(OWNER(player))) {
	match_player(&md);
    }
    match_registered(&md);
    match_absolute(&md);

    if ((action = noisy_match_result(&md)) == NOTHING)
	return;

    if (Typeof(action) != TYPE_EXIT) {
	anotify(player, CINFO "That's not an action.");
	return;
    } else if (!controls(player, action)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if (((source = parse_source(player, source_name)) == NOTHING)
	    || Typeof(source) == TYPE_PROGRAM)
	return;

    if (!unset_source(player, loc, action)) {
	return;
    }
    set_source(player, action, source);
    anotify(player, CSUCC "Action re-attached.");
    if (MLevel(action)) {
	SetMLevel(action, 0);
	anotify(player, CINFO "Action priority Level reset to zero.");
    }
}
