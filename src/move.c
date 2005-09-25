#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include "color.h"
#include "db.h"
#include "props.h"
#include "tune.h"
#include "interface.h"
#include "match.h"
#include "externs.h"

void 
moveto(dbref what, dbref where)
{
    dbref   loc;

    /* do NOT move garbage */
    if (what != NOTHING && Typeof(what) == TYPE_GARBAGE) {
	return;
    }

    if(what != NOTHING && parent_loop_check(what, where)) {
	/* Prevent stupid loop holes elsewhere */
	return;
    }

    /* remove what from old loc */
    if ((loc = DBFETCH(what)->location) != NOTHING) {
	DBSTORE(loc, contents, remove_first(DBFETCH(loc)->contents, what));
    }
    /* test for special cases */
    switch (where) {
	case NOTHING:
	    DBSTORE(what, location, NOTHING);
	    return;		/* NOTHING doesn't have contents */
	case HOME:
	    switch (Typeof(what)) {
		case TYPE_PLAYER:
		    where = DBFETCH(what)->sp.player.home;
		    break;
		case TYPE_THING:
		    where = DBFETCH(what)->sp.thing.home;
		    if (parent_loop_check(what, where))
			where = DBFETCH(OWNER(what))->sp.player.home;
		    break;
		case TYPE_ROOM:
		    where = GLOBAL_ENVIRONMENT;
		    break;
		case TYPE_PROGRAM:
		    where = OWNER(what);
		    break;
	    }
    }

    /* now put what in where */
    PUSH(what, DBFETCH(where)->contents);
    DBDIRTY(where);
    DBSTORE(what, location, where);
}

dbref   reverse(dbref);
void 
send_contents(dbref loc, dbref dest)
{
    dbref   first;
    dbref   rest;

    first = DBFETCH(loc)->contents;
    DBSTORE(loc, contents, NOTHING);

    /* blast locations of everything in list */
    DOLIST(rest, first) {
	DBSTORE(rest, location, NOTHING);
    }

    while (first != NOTHING) {
	rest = DBFETCH(first)->next;
	if ((Typeof(first) != TYPE_THING)
		&& (Typeof(first) != TYPE_PROGRAM)) {
	    moveto(first, loc);
	} else {
	    /* Don't send zombies home, they're people too! */
	    if(!(FLAGS(first)&ZOMBIE))
		moveto(first, ( (FLAGS(first) & STICKY) ||
		    parent_loop_check(first, dest) )
		    ? HOME : dest);
	    else(moveto(first, loc));
	}
	first = rest;
    }

    DBSTORE(loc, contents, reverse(DBFETCH(loc)->contents));
}

void 
maybe_dropto(dbref loc, dbref dropto)
{
    dbref   thing;

    if (parent_loop_check(loc, dropto))
	return;			/* bizarre special case */

    /* check for players */
    DOLIST(thing, DBFETCH(loc)->contents) {
	if (Typeof(thing) == TYPE_PLAYER)
	    return;
    }

    /* no players, send everything to the dropto */
    send_contents(loc, dropto);
}

#define MAX_PARENTS 50

int 
parent_loop_check(dbref source, dbref dest)
{
    dbref parent[MAX_PARENTS], origdest;
    int   parents = 0, room;

    origdest = dest;

    while((dest != 0) && (dest != NOTHING)) {
	if (source == dest)
	    return 1;		/* That's an easy one! */
	if (dest == NOTHING)
	    return 0;
	if (dest == HOME)
	    return 0;
	if(!OkObj(dest))
	    return 1;
	if (Typeof(dest) == TYPE_THING &&
	    parent_loop_check(source, DBFETCH(dest)->sp.thing.home)
	)   return 1;

	/* This should never happen, but it can, and @sanfix won't fix it
	   in normal FB.  A parent should never return to it's source, this
	   will hopefully catch at least some single-loop bugs.
	*/

	for(room = 0; room < parents; room++) {
	    if((source == parent[room]) || (dest == parent[room])) {
	        log_status("LOOP: %s's location loops to itself.\n", unparse_object(MAN,origdest));
		return 1;
	    }
	}

	if(parents >= MAX_PARENTS) {
	    log_status("LOOP: %s has too many parents.\n", unparse_object(MAN,origdest));
	    return 1; /* Too many parents */
	}
	parent[parents++] = dest;

	dest = DBFETCH(dest)->location;
    }

    return 0;
}

static int donelook = 0;
void enter_room(dbref player, dbref loc, dbref exit)
{
    dbref   old;
    dbref   dropto;
    char    buf[BUFFER_LEN];

    /* check for room == HOME */
    if (loc == HOME)
	loc = DBFETCH(player)->sp.player.home;	/* home */

    /* get old location */
    old = DBFETCH(player)->location;

    if( (old == tp_jail_room) && (tp_jail_room != NOTHING) && Guest(player) )
	loc = old; /* Ain't going nowhere buddy */

    /* check for self-loop */
    /* self-loops don't do move or other player notification */
    /* but you still get autolook and penny check */
    if (loc != old) {

	/* go there */
	moveto(player, loc);

	if (old != NOTHING) {
	    propqueue(player, old, exit, player, NOTHING,
		"~depart", "Depart", 1, 1);
	    envpropqueue(player, old, exit, old, NOTHING,
		"~depart", "Depart", 1, 1);

	    propqueue(player, old, exit, player, NOTHING,
		"~odepart", "Odepart", 1, 0);
	    envpropqueue(player, old, exit, old, NOTHING,
		"~odepart", "Odepart", 1, 0);

	    if(tp_user_arrive_propqueue) {
		propqueue(player, old, exit, player, NOTHING,
		    "_depart", "Depart", 1, 1);
		envpropqueue(player, old, exit, old, NOTHING,
		    "_depart", "Depart", 1, 1);

		propqueue(player, old, exit, player, NOTHING,
		    "_odepart", "Odepart", 1, 0);
		envpropqueue(player, old, exit, old, NOTHING,
		    "_odepart", "Odepart", 1, 0);
	    }

	    /* notify others unless DARK */
	    if (!Dark(old) /* && !Dark(player) */
		&& ((Typeof(exit) != TYPE_EXIT) ||
		    (!tp_quiet_dark_exits) || !Dark(exit))
	    ) {
#ifndef QUIET_MOVES
		if(!tp_quiet_moves) {
		    sprintf(buf, CMOVE "%s has left.", PNAME(player));
		    anotify_except(DBFETCH(old)->contents, player, buf, player);
		}
#endif /* QUIET_MOVES */
	    }
	}

	/* if old location has STICKY dropto, send stuff through it */
	if (old != NOTHING && Typeof(old) == TYPE_ROOM
		&& (dropto = DBFETCH(old)->sp.room.dropto) != NOTHING
		&& (FLAGS(old) & STICKY)) {
	    maybe_dropto(old, dropto);
	}

	/* tell other folks in new location if not DARK */
	if (!Dark(loc) /* && !Dark(player) */
	    && ((Typeof(exit) != TYPE_EXIT) ||
	        (!tp_quiet_dark_exits) || !Dark(exit))
	) {
#ifndef QUIET_MOVES
	    if(!tp_quiet_moves) {
		sprintf(buf, CMOVE "%s has arrived.", PNAME(player));
		anotify_except(DBFETCH(loc)->contents, player, buf, player);
	    }
#endif /* QUIET_MOVES */
	}
    }
    /* autolook */
    if (donelook < 8) {
	donelook++;
	if (!( (Typeof(exit) == TYPE_EXIT) && (FLAGS(exit)&HAVEN) )) {
	    /* These 'look's were changed to 'loo' per Grisson's bugfix */
	    if (can_move(player, "loo", 0)) {
		do_move(player, "loo", 0);
	    } else {
		do_look_around (player);
	    }
	}
	donelook--;
    } else
	anotify(player, CINFO "Look aborted because of look action loop.");

    if (tp_penny_rate != 0) {
	/* check for pennies */
	if (!controls(player, loc)
		&& DBFETCH(player)->sp.player.pennies <= tp_max_pennies
		&& RANDOM() % tp_penny_rate == 0) {
	    anotify_fmt(player, CINFO "You found a %s!", tp_penny);
	    DBFETCH(OWNER(player))->sp.player.pennies++;
	    DBDIRTY(OWNER(player));
	}
    }

    if (loc != old) {
	envpropqueue(player,loc,exit,player,NOTHING,"~arrive","Arrive",1,1);
	envpropqueue(player,loc,exit,player,NOTHING,"~oarrive","Oarrive",1,0);
	if(tp_user_arrive_propqueue) {
	    envpropqueue(player,loc,exit,player,NOTHING,"_arrive","Arrive",1,1);
	    envpropqueue(player,loc,exit,player,NOTHING,"_oarrive","Oarrive",1,0);
	}
    }
}

void 
send_home(dbref thing, int puppethome)
{
    dbref loc;
    
    loc = NOTHING;

    switch (Typeof(thing)) {
	    case TYPE_PLAYER:
	    /* send his possessions home first! */
	    /* that way he sees them when he arrives */
	    send_contents(thing, HOME);
#ifdef MUD
	    loc = getloc(thing);
	    if( loc != NOTHING &&
		Typeof(loc) == TYPE_ROOM &&
		(FLAGS(loc)&KILL_OK) &&
		(tp_home_room != NOTHING)
	    )
	    	enter_room(thing, tp_home_room, DBFETCH(thing)->location);
	    else
#endif
		enter_room(thing, DBFETCH(thing)->sp.player.home, DBFETCH(thing)->location);
	    break;
	case TYPE_THING:
	    if (puppethome)
		send_contents(thing, HOME);
	    if (FLAGS(thing) & (ZOMBIE | LISTENER)) {
		enter_room(thing, DBFETCH(thing)->sp.player.home, DBFETCH(thing)->location);
		break;
	    }
	    moveto(thing, HOME);	/* home */
	    break;
	case TYPE_PROGRAM:
	    moveto(thing, OWNER(thing));
	    break;
	default:
	    /* no effect */
	    break;
    }
    return;
}

int 
can_move(dbref player, const char *direction, int lev)
{
    struct match_data md;

    if (!string_compare(direction, "home"))
	return 1;

    /* otherwise match on exits */
    init_match(player, direction, TYPE_EXIT, &md);
    set_match_muf_verbs(1, &md);
    md.match_level = lev;
    match_all_exits(&md);
    return (last_match_result(&md) != NOTHING);
}

/*
 * trigger()
 *
 * This procedure triggers a series of actions, or meta-actions
 * which are contained in the 'dest' field of the exit.
 * Locks other than the first one are over-ridden.
 *
 * `player' is the player who triggered the exit
 * `exit' is the exit triggered
 * `pflag' is a flag which indicates whether player and room exits
 * are to be used (non-zero) or ignored (zero).  Note that
 * player/room destinations triggered via a meta-link are
 * ignored.
 *
 */

void 
trigger(dbref player, dbref exit, int pflag)
{
    int     i;
    dbref   dest;
    int     sobjact;		/* sticky object action flag, sends home
				 * source obj */
    int     succ;

    sobjact = 0;
    succ = 0;

    for (i = 0; i < DBFETCH(exit)->sp.exit.ndest; i++) {
	dest = (DBFETCH(exit)->sp.exit.dest)[i];
	if (dest == HOME)
	    dest = DBFETCH(player)->sp.player.home;
	switch (Typeof(dest)) {
	    case TYPE_ROOM:
		if (pflag) {
		    if (parent_loop_check(player, dest)) {
			anotify(player, CINFO "That would cause a paradox.");
			break;
		    }
		    if (!Mage(OWNER(player)) && Typeof(player)==TYPE_THING
			    && (FLAGS(dest)&ZOMBIE)) {
			anotify(player, CFAIL NOWAY_MESG);
			break;
		    }
		    if ((FLAGS(player) & VEHICLE) &&
			    ((FLAGS(dest) | FLAGS(exit)) & VEHICLE)) {
			anotify(player, CFAIL NOWAY_MESG);
			break;
		    }
		    if (GETDROP(exit))
			exec_or_notify(player, exit, GETDROP(exit), "(@Drop)");
		    if (GETODROP(exit) /*&& !Dark(player)*/) {
			parse_omessage(player, dest, exit, GETODROP(exit),
					PNAME(player), "(@Odrop)");
		    }
		    enter_room(player, dest, exit);
		    succ = 1;
		}
		break;
	    case TYPE_THING:
		if (dest == getloc(exit) && (FLAGS(dest) & VEHICLE)) {
		    if (pflag) {
			if (parent_loop_check(player, dest)) {
			    anotify(player, CINFO "That would cause a paradox.");
			    break;
			}
			if (GETDROP(exit))
			    exec_or_notify(player, exit, GETDROP(exit),
					   "(@Drop)");
			if (GETODROP(exit) /*&& !Dark(player)*/) {
			    parse_omessage(player, dest, exit, GETODROP(exit),
				    PNAME(player), "(@Odrop)");
			}
			enter_room(player, dest, exit);
			succ = 1;
		    }
		} else {
		    if (Typeof(DBFETCH(exit)->location) == TYPE_THING) {
			if (parent_loop_check(dest, getloc(getloc(exit)))) {
			    anotify(player, CINFO "That would cause a paradox.");
			    break;
			}
			moveto(dest, DBFETCH(DBFETCH(exit)->location)->location);
			if (!(FLAGS(exit) & STICKY)) {
			    /* send home source object */
			    sobjact = 1;
			}
		    } else {
			if (parent_loop_check(dest, getloc(exit))) {
			    anotify(player, CINFO "That would cause a paradox.");
			    break;
			}
			moveto(dest, DBFETCH(exit)->location);
		    }
		    if (GETSUCC(exit))
			succ = 1;
		}
		break;
	    case TYPE_EXIT:	/* It's a meta-link(tm)! */
		ts_useobject(dest);
		trigger(player, (DBFETCH(exit)->sp.exit.dest)[i], 0);
		if (GETSUCC(exit))
		    succ = 1;
		break;
	    case TYPE_PLAYER:
		if (pflag && DBFETCH(dest)->location != NOTHING) {
		    if (parent_loop_check(player, dest)) {
			anotify(player, CINFO "That would cause a paradox.");
			break;
		    }
		    succ = 1;
		    if (FLAGS(dest) & JUMP_OK) {
			if (GETDROP(exit)) {
			    exec_or_notify(player, exit,
				    GETDROP(exit), "(@Drop)");
			}
			if (GETODROP(exit) /*&& !Dark(player)*/) {
			    parse_omessage(player, getloc(dest), exit,
				    GETODROP(exit), PNAME(player), "(@Odrop)");
			}
			enter_room(player, DBFETCH(dest)->location, exit);
		    } else {
			anotify(player, CINFO "That player does not wish to be disturbed.");
		    }
		}
		break;
	    case TYPE_PROGRAM:
		(void) interp(player, DBFETCH(player)->location, dest, exit,
			      FOREGROUND, STD_REGUID, 0);
		return;
	}
    }
    if (sobjact)
	send_home(DBFETCH(exit)->location, 0);
    if (!succ && pflag)
	anotify(player, CINFO "Done.");
}

void 
do_move(dbref player, const char *direction, int lev)
{
    dbref   exit, loc, dest, who, whonext;
    char    buf[BUFFER_LEN];
    struct match_data md;

    if (!string_compare(direction, "home")) {
	/* send him home */
	if ((loc = DBFETCH(player)->location) != NOTHING) {
	    /* tell everybody else */
	    sprintf(buf, CMOVE "%s goes home.", PNAME(player));
	    anotify_except(DBFETCH(loc)->contents, player, buf, player);
	}
	/* give the player the messages */
	anotify(player, CRED "There's no place like home...");
	anotify(player, CWHITE "There's no place like home...");
	anotify(player, CBLUE "There's no place like home...");
	send_home(player, 1);
    } else {
#ifdef PATH
	int fullmatch = 0;
#endif /* PATH */

	/* find the exit */
	init_match_check_keys(player, direction, TYPE_EXIT, &md);
	set_match_muf_verbs(1, &md);
	md.match_level = lev;
	match_all_exits(&md);
	exit = match_result(&md);

#ifdef PATH
	if( (
	      (!OkObj(exit)) ||
	      (PLevel(exit) <= tp_path_mlevel) /* PLevel = MLevel + 1 -> <= */
	    ) &&
	    OkObj(match_path(getloc(player), direction, buf, &fullmatch))
	) {
	    move_path(player, buf, direction, fullmatch);
	    return;
	}
#endif /* PATH */

	switch (exit) {
	    case NOTHING:
		anotify(player, CFAIL NOWAY_MESG);
		break;
	    case AMBIGUOUS:
		anotify(player, CINFO "I don't know which way you mean!");
		break;
	    default:
		/* we got one */
		/* check to see if we got through */
		ts_useobject(exit);
		loc = DBFETCH(player)->location;
		if (can_doit(player, exit, CFAIL NOWAY_MESG)) {
		    trigger(player, exit, 1);

		    /* lead/follow loop */
		    if((DBFETCH(exit)->location == loc) &&
		       (DBFETCH(exit)->sp.exit.ndest >= 1) &&
		       ((dest = DBFETCH(exit)->sp.exit.dest[0]) != NOTHING) &&
		       (Typeof(dest) == TYPE_ROOM)
		    ) {
			who = DBFETCH(loc)->contents;
			while(who != NOTHING) {
			    whonext = DBFETCH(who)->next;
			    if( (getleader(who) == player) &&
				(DBFETCH(who)->location == loc)
			    ) {
				anotify_fmt(who, CMOVE "You follow %s.", NAME(player));
				if (can_doit(who, exit, CFAIL NOWAY_MESG))
				    trigger(who, exit, 1);
			    }
			    who = whonext;
			}
		    }
		}
		break;
	}
    }
}


void
do_leave(dbref player)
{
    dbref loc, dest;

    loc = DBFETCH(player)->location;
    if (loc == NOTHING || Typeof(loc) == TYPE_ROOM) {
	anotify(player, CFAIL NOWAY_MESG);
	return;
    }

    if (!(FLAGS(loc) & VEHICLE) && !(Typeof(loc) == TYPE_PLAYER)) {
	anotify(player, CFAIL "You can only exit vehicles.");
	return;
    }

    dest = DBFETCH(loc)->location;
    if(!OkObj(dest)) return;

    if (Typeof(dest) != TYPE_ROOM && Typeof(dest) != TYPE_THING) {
	anotify(player, CFAIL "You can't exit a vehicle inside of a player.");
	return;
    }

/*
 *  if (Typeof(dest) == TYPE_ROOM && !controls(player, dest)
 *	  && !(FLAGS(dest) | JUMP_OK)) {
 *      anotify(player, CFAIL NOWAY_MESG);
 *      return;
 *  }
 */

    if (parent_loop_check(player, dest)) {
	anotify(player, CFAIL NOWAY_MESG);
	return;
    }

    anotify(player, CSUCC "You exit the vehicle.");
    enter_room(player, dest, loc);
}


void 
do_get(dbref player, const char *what, const char *obj)
{
    dbref   thing, cont, loc;
    int cando;
    struct match_data md;

    if( tp_db_readonly ) {
	anotify( player, CFAIL DBRO_MESG );
	return;
    }

    init_match_check_keys(player, what, TYPE_THING, &md);
    match_neighbor(&md);
    match_possession(&md);
    if (Mage(OWNER(player)))
	match_absolute(&md);	/* the wizard has long fingers */

    /* if ((thing = noisy_match_result(&md)) != NOTHING) { */

    switch (thing = match_result(&md)) {
      case NOTHING:
	if( ((dbref)(long int)match_detail(player, what) == NOTHING) &&
	    ( ((loc = getloc(player)) == NOTHING) ||
	      ((dbref)(long int)match_detail(loc, what) == NOTHING) 
	    )
	) {
	    anotify(player, CINFO NOMATCH_MESSAGE); break;
	} else {
	    anotify(player, CINFO "That's part of the scenery.");
	}
	break;
      case AMBIGUOUS:
	anotify(player, CINFO AMBIGUOUS_MESSAGE);
	break;
      default: 
	cont = thing;
	if (obj && *obj) {
	    init_match_check_keys(player, obj, TYPE_THING, &md);
	    match_rmatch(cont, &md);
	    if (Mage(OWNER(player)))
		match_absolute(&md);	/* the wizard has long fingers */
	    if ((thing = noisy_match_result(&md)) == NOTHING) {
		return;
	    }
	    if (Typeof(cont) == TYPE_PLAYER) {
		anotify(player, CFAIL "You can't steal things from players.");
		return;
	    }
	    if (!test_lock_false_default(player, cont, "_/clk")) {
		anotify(player, CFAIL "You can't open that container.");
		return;
	    }
	}
	if (Typeof(player) != TYPE_PLAYER) {
	    if (Typeof(DBFETCH(thing)->location) != TYPE_ROOM) {
		if (OWNER(player) != OWNER(thing)) {
		    notify(player, "Zombies aren't allowed to be thieves!");
		    return;
		}
	    }
	}
	if (DBFETCH(thing)->location == player) {
	    anotify(player, CINFO "You already have that.");
	    return;
	}
	if (Typeof(cont) == TYPE_PLAYER) {
	    anotify(player, CFAIL "You can't steal stuff from players.");
	    return;
	}
	if (parent_loop_check(thing, player)) {
	    anotify(player, CFAIL "You can't pick yourself up by your bootstraps!");
	    return;
	}
	switch (Typeof(thing)) {
	    case TYPE_THING:
		ts_useobject(thing);
	    case TYPE_PROGRAM:
		if (obj && *obj) {
		    cando = could_doit(player, thing);
		    if (!cando)
			anotify(player, CFAIL "You can't get that.");
		} else {
		    cando=can_doit(player, thing, CFAIL "You can't pick that up.");
		}
		if (cando) {
		    moveto(thing, player);
		    anotify(player, CSUCC "Taken.");
		}
		break;
	    default:
		anotify(player, CFAIL "You can't take that!");
		break;
	}
    }
}

void 
do_drop(dbref player, const char *name, const char *obj)
{
    dbref   loc, cont;
    dbref   thing;
    char    buf[BUFFER_LEN];
    struct match_data md;

    if( tp_db_readonly ) {
	anotify( player, CFAIL DBRO_MESG );
	return;
    }

    if ((loc = getloc(player)) == NOTHING)
	return;

    init_match(player, name, NOTYPE, &md);
    match_possession(&md);
    if ((thing = noisy_match_result(&md)) == NOTHING
	    || thing == AMBIGUOUS)
	return;

    cont = loc;
    if (obj && *obj) {
	init_match(player, obj, NOTYPE, &md);
	match_possession(&md);
	match_neighbor(&md);
	if (Mage(OWNER(player)))
	    match_absolute(&md);	/* the wizard has long fingers */
	if ((cont=noisy_match_result(&md))==NOTHING || thing==AMBIGUOUS) {
	    return;
	}
    }
    switch (Typeof(thing)) {
	case TYPE_THING:
	    ts_useobject(thing);
	case TYPE_PROGRAM:
	    if (DBFETCH(thing)->location != player) {
		/* Shouldn't ever happen. */
		anotify(player, CFAIL "You can't drop that.");
		break;
	    }
	    if (Typeof(cont) != TYPE_ROOM && Typeof(cont) != TYPE_PLAYER &&
		    Typeof(cont) != TYPE_THING) {
		anotify(player, CFAIL "You can't put anything in that.");
		break;
	    }
	    if (Typeof(cont)!=TYPE_ROOM &&
		    !test_lock_false_default(player, cont, "_/clk")) {
		anotify(player, CFAIL "You don't have permission to put something in that.");
		break;
	    }
	    if (parent_loop_check(thing, cont)) {
		anotify(player, CFAIL "You can't put something inside of itself.");
		break;
	    }
	    if( (Typeof(thing) != TYPE_THING) || !(FLAGS(thing)&ZOMBIE) ) {
		if ( (Typeof(cont) == TYPE_ROOM) && (FLAGS(thing) & STICKY) &&
		    (Typeof(thing) == TYPE_THING)
		) {
		    send_home(thing, 0);
		} else {
		    int immediate_dropto = (Typeof(cont) == TYPE_ROOM &&
			DBFETCH(cont)->sp.room.dropto!=NOTHING
			&& !(FLAGS(cont) & STICKY)
			&& !parent_loop_check(thing, DBFETCH(cont)->sp.room.dropto)
		    );

		    moveto(thing, immediate_dropto 
		        ? DBFETCH(cont)->sp.room.dropto : cont);
		}
	    } else moveto(thing, cont);

	    if (Typeof(cont) == TYPE_THING) {
		anotify(player, CSUCC "Put away.");
		return;
	    } else if (Typeof(cont) == TYPE_PLAYER) {
		anotify_fmt(cont, CINFO "%s hands you %s.", PNAME(player), PNAME(thing));
		anotify_fmt(player, CSUCC "You hand %s to %s.",
				    PNAME(thing), PNAME(cont));
		return;
	    }

	    if (GETDROP(thing))
		exec_or_notify(player, thing, GETDROP(thing), "(@Drop)");
	    else
		anotify(player, CSUCC "Dropped.");

	    if (GETDROP(loc))
		exec_or_notify(player, loc, GETDROP(loc), "(@Drop)");

	    if (GETODROP(thing)) {
		parse_omessage(player, loc, thing, GETODROP(thing),
				PNAME(player), "(@Odrop)");
	    } else {
		sprintf(buf, CBLUE "%s drops %s.", PNAME(player), PNAME(thing));
	    anotify_except(DBFETCH(loc)->contents, player, buf, player);
	    }

	    if (GETODROP(loc)) {
		parse_omessage(player, loc, loc, GETODROP(loc),
				PNAME(thing), "(@Odrop)");
	    }
	    break;
	default:
	    anotify(player, CFAIL "You can't drop that.");
	    break;
    }
}

void 
do_recycle(dbref player, const char *name)
{
    dbref   thing;
    struct match_data md;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if( tp_db_readonly ) {
	anotify( player, CFAIL DBRO_MESG );
	return;
    }

    init_match(player, name, TYPE_THING, &md);
    match_all_exits(&md);
    match_neighbor(&md);
    match_possession(&md);
    match_registered(&md);
    match_here(&md);
    if (Mage(OWNER(player))) {
	match_absolute(&md);
    }
    if ((thing = noisy_match_result(&md)) != NOTHING) {
	if (!controls(player, thing)) {
	    anotify(player, CFAIL NOPERM_MESG);
	} else {
	    switch (Typeof(thing)) {
		case TYPE_ROOM:
		    if (OWNER(thing) != OWNER(player)) {
			anotify(player, CFAIL NOPERM_MESG);
			return;
		    }
		    if ((thing == RootRoom) || (thing == EnvRoom) ||
			(thing == EnvRoomX) || (thing == (dbref) 0)) {
			anotify(player, CFAIL "This room may not be recycled.");
			return;
		    }
		    break;
		case TYPE_THING:
		    if (OWNER(thing) != OWNER(player)) {
			anotify(player, CFAIL NOPERM_MESG);
			return;
		    }
		    break;
		case TYPE_EXIT:
		    if (OWNER(thing) != OWNER(player)) {
			anotify(player, CFAIL NOPERM_MESG);
			return;
		    }
		    if (!unset_source(player, DBFETCH(player)->location, thing)) {
			anotify(player, CFAIL "You can't do that to an exit in another room.");
			return;
		    }
		    break;
		case TYPE_PLAYER:
		    anotify(player, CFAIL "You can't recycle a player!");
		    return;
		    /* NOTREACHED */
		    break;
		case TYPE_PROGRAM:
		    if (OWNER(thing) != OWNER(player)) {
			anotify(player, CFAIL NOPERM_MESG);
			return;
		    }
		    break;
		case TYPE_GARBAGE:
		    anotify(player, CINFO "That's already garbage.");
		    return;
		    /* NOTREACHED */
		    break;
	    }
	    recycle(player, thing);
	    anotify(player, CINFO "Thank you for recycling.");
	}
    }
}

extern int unlink(const char *);
void 
recycle(dbref player, dbref thing)
{
    extern dbref recyclable;
    static int depth = 0;
    dbref   first;
    dbref   rest;
    char    buf[BUFFER_LEN];
    int     looplimit;

    depth++;
    switch (Typeof(thing)) {
	case TYPE_ROOM:
	  if (!Mage(OWNER(thing)) && tp_room_cost != 0 ) {
	    DBFETCH(OWNER(thing))->sp.player.pennies += tp_room_cost;
	  }
	    DBDIRTY(OWNER(thing));
	    for (first = DBFETCH(thing)->exits; first != NOTHING; first = rest) {
		rest = DBFETCH(first)->next;
		if (DBFETCH(first)->location == NOTHING || DBFETCH(first)->location == thing)
		    recycle(player, first);
	    }
	    anotify_except(DBFETCH(thing)->contents, NOTHING, CINFO
			  "You feel a wrenching sensation...", player);
	    break;
	case TYPE_THING:
 	  if (!Mage(OWNER(thing)) && tp_room_cost != 0 ) {
	    DBFETCH(OWNER(thing))->sp.player.pennies += DBFETCH(thing)->sp.thing.value;
	  }
	    DBDIRTY(OWNER(thing));
	    for (first = DBFETCH(thing)->exits; first != NOTHING;
		    first = rest) {
		rest = DBFETCH(first)->next;
		if (DBFETCH(first)->location == NOTHING || DBFETCH(first)->location == thing)
		    recycle(player, first);
	    }
	    break;
	case TYPE_EXIT:
 	  if (!Mage(OWNER(thing)) && tp_room_cost != 0 ) {
	    DBFETCH(OWNER(thing))->sp.player.pennies += tp_exit_cost;
	  }
	    if (!Mage(OWNER(thing)))
		if (DBFETCH(thing)->sp.exit.ndest != 0)
		    DBFETCH(OWNER(thing))->sp.player.pennies += tp_link_cost;
	    DBDIRTY(OWNER(thing));
	    break;
	case TYPE_PROGRAM:
	    dequeue_prog(thing, 0);
	    sprintf(buf, "muf/%d.m", (int) thing);
	    if(unlink(buf))
		perror(buf);
	    break;
    }

    for (rest = 0; rest < db_top; rest++) {
	switch (Typeof(rest)) {
	    case TYPE_ROOM:
		if (DBFETCH(rest)->sp.room.dropto == thing) {
		    DBFETCH(rest)->sp.room.dropto = NOTHING;
		    DBDIRTY(rest);
		}
		if (DBFETCH(rest)->exits == thing) {
		    DBFETCH(rest)->exits = DBFETCH(thing)->next;
		    DBDIRTY(rest);
		}
		if (OWNER(rest) == thing) {
		    OWNER(rest) = MAN;
		    DBDIRTY(rest);
		}
		break;
	    case TYPE_THING:
		if (DBFETCH(rest)->sp.thing.home == thing) {
		    if (DBFETCH(OWNER(rest))->sp.player.home == thing)
			DBSTORE(OWNER(rest), sp.player.home, RootRoom);
		    DBFETCH(rest)->sp.thing.home = DBFETCH(OWNER(rest))->sp.player.home;
		    DBDIRTY(rest);
		}
		if (DBFETCH(rest)->exits == thing) {
		    DBFETCH(rest)->exits = DBFETCH(thing)->next;
		    DBDIRTY(rest);
		}
		if (abs(DBFETCH(rest)->sp.thing.leader) == thing) {
		    DBFETCH(rest)->sp.thing.leader = 0;
		    DBDIRTY(rest);
		}
		if (OWNER(rest) == thing) {
		    OWNER(rest) = MAN;
		    DBDIRTY(rest);
		}
		break;
	    case TYPE_EXIT:
		{
		    int     i, j;

		    for (i = j = 0; i < DBFETCH(rest)->sp.exit.ndest; i++) {
			if ((DBFETCH(rest)->sp.exit.dest)[i] != thing)
			    (DBFETCH(rest)->sp.exit.dest)[j++] =
				(DBFETCH(rest)->sp.exit.dest)[i];
		    }
		    if (j < DBFETCH(rest)->sp.exit.ndest) {
		      if (tp_link_cost != 0) {
			DBFETCH(OWNER(rest))->sp.player.pennies += tp_link_cost;
		      }
			DBDIRTY(OWNER(rest));
			DBFETCH(rest)->sp.exit.ndest = j;
			DBDIRTY(rest);
		    }
		}
		if (OWNER(rest) == thing) {
		    OWNER(rest) = MAN;
		    DBDIRTY(rest);
		}
		break;
	    case TYPE_PLAYER:
		if (Typeof(thing) == TYPE_PROGRAM && (FLAGS(rest) & INTERACTIVE)
			&& (DBFETCH(rest)->sp.player.curr_prog == thing)) {
		    if (FLAGS(rest) & READMODE) {
			anotify(rest, CINFO "The program you were running has been recycled.  Aborting program.");
		    } else {
			free_prog_text(DBFETCH(thing)->sp.program.first);
			DBFETCH(thing)->sp.program.first = NULL;
			DBFETCH(rest)->sp.player.insert_mode = 0;
			FLAGS(thing) &= ~INTERNAL;
			FLAGS(rest) &= ~INTERACTIVE;
			DBFETCH(rest)->sp.player.curr_prog = NOTHING;
			anotify(rest, CINFO "The program you were editing has been recycled.  Exiting Editor.");
		    }
		}
		if (DBFETCH(rest)->sp.player.home == thing) {
		    DBFETCH(rest)->sp.player.home = RootRoom;
		    DBDIRTY(rest);
		}
		if (abs(DBFETCH(rest)->sp.player.leader) == thing) {
		    DBFETCH(rest)->sp.player.leader = 0;
		    DBDIRTY(rest);
		}
		if (DBFETCH(rest)->exits == thing) {
		    DBFETCH(rest)->exits = DBFETCH(thing)->next;
		    DBDIRTY(rest);
		}
		if (DBFETCH(rest)->sp.player.curr_prog == thing)
		    DBFETCH(rest)->sp.player.curr_prog = 0;
		break;
	    case TYPE_PROGRAM:
		if (OWNER(rest) == thing) {
		    OWNER(rest) = MAN;
		    DBDIRTY(rest);
		}
	}
	/*
	 *if (DBFETCH(rest)->location == thing)
	 *    DBSTORE(rest, location, NOTHING);
	 */
	if (DBFETCH(rest)->contents == thing)
	    DBSTORE(rest, contents, DBFETCH(thing)->next);
	if (DBFETCH(rest)->next == thing)
	    DBSTORE(rest, next, DBFETCH(thing)->next);
    }

    looplimit = db_top;
    while ((looplimit-->0) && ((first = DBFETCH(thing)->contents) != NOTHING)){
	if (Typeof(first) == TYPE_PLAYER) {
	    enter_room(first, HOME, DBFETCH(thing)->location);
	    /* If the room is set to drag players back, there'll be no
	     * reasoning with it.  DRAG the player out.
	     */
	    if (DBFETCH(first)->location == thing) {
		notify_fmt(player, "Escaping teleport loop!");
		moveto(first, HOME);
	    }
	} else {
	    moveto(first, HOME);
	}
    }


    moveto(thing, NOTHING);

    depth--;

    db_free_object(thing);
    db_clear_object(thing);


    NAME(thing) = "<garbage>";
    SETDESC(thing, "<recyclable>");
    OWNER(thing) = NOTHING;
    FLAGS(thing) = TYPE_GARBAGE;

    DBFETCH(thing)->next = recyclable;
    recyclable = thing;
    DBDIRTY(thing);
}

dbref getleader(dbref who) {
    if(!OkObj(who)) return 0;

    if(( (Typeof(who) != TYPE_THING) || !(FLAGS(who)&ZOMBIE) ) &&
	(Typeof(who) != TYPE_PLAYER)
    ) return 0;

    switch(Typeof(who)) {
	case TYPE_THING:
	    return DBFETCH(who)->sp.thing.leader;
	case TYPE_PLAYER:
	    return DBFETCH(who)->sp.player.leader;
    }

    return 0;
}

void setleader(dbref who, dbref leader) {
    if((!OkObj(who)) || (abs(leader) >= db_top)) return;

    switch(Typeof(who)) {
	case TYPE_THING:
	    DBFETCH(who)->sp.thing.leader = leader;
	    return;
	case TYPE_PLAYER:
	    DBFETCH(who)->sp.player.leader = leader;
	    return;
    }
}

int unfollowall(dbref who) {
    dbref leader, thing, count = 0;

    if(!OkObj(who)) return 0;

    for(thing = 0; thing < db_top; thing++) {
	leader = getleader(thing);
	if(abs(leader) == who) {
	    setleader(thing, 0);
	    count++;
	    if(leader > 0) anotify_fmt(thing, CINFO "You stop following %s.", NAME(leader));
	    else anotify_fmt(thing, CINFO "You stop waiting to follow %s.", NAME(-leader));
	}
    }
    return count;
}

void 
do_follow(dbref player, const char *name)
{
    struct match_data md;
    dbref leader, newleader;

    if(abs(leader = getleader(player)) >= db_top) leader = 0;

    if(!*name) {
	if(leader > 0)
	    anotify_fmt(player, CINFO "You are following %s.", NAME(leader));
	else if(leader < 0)
	    anotify_fmt(player, CINFO "You are waiting to follow %s.", NAME(-leader));
	else
	    anotify(player, CINFO "You aren't following anyone.");
	return;
    }

    if(!string_compare(name, "me")) {
	newleader = 0;
    } else {
	init_match(player, name, NOTYPE, &md);
	match_neighbor(&md);
	if((newleader = noisy_match_result(&md)) == NOTHING) return;

	if(( (Typeof(newleader) != TYPE_THING) || !(FLAGS(newleader)&ZOMBIE) ) &&
	   (Typeof(newleader) != TYPE_PLAYER)
	) {
	    anotify(player, CFAIL "You can't follow that.");
	    return;
	}

	if(newleader == player) {
	    anotify(player, CFAIL "You'd end up going in circles!");
	    return;
	}
    }

    /* End follow */
    if(leader > 0) {
	setleader(player, 0);
	anotify_fmt(leader, CINFO "%s stops following you.", NAME(player));
	anotify_fmt(player, CINFO "You stop following %s.", NAME(leader));
    } else if(leader < 0) {
	setleader(player, 0);
	anotify_fmt(-leader, CINFO "%s stops waiting to follow you.", NAME(player));
	anotify_fmt(player, CINFO "You stop waiting to follow %s.", NAME(-leader));
    } else if(newleader == 0) {
	anotify(player, CINFO "You aren't following anyone.");
    }

    setleader(player, -newleader);
    if(newleader) {
	anotify_fmt(newleader, CINFO "%s asks to follow you.  Type 'lead %s' to accept.", NAME(player), NAME(player));
	anotify_fmt(player, CINFO "You ask to follow %s.", NAME(newleader));
    }
}

void 
do_lead(dbref player, const char *name, int release)
{
    char buf[BUFFER_LEN], buft[BUFFER_LEN];
    struct match_data md;
    dbref thing, leader;

    if(!*name) {
	buf[0] = buft[0] = '\0';

	for(thing = 0; thing < db_top; thing++) {
	    if(((leader = getleader(thing)) == 0) ||
	       (abs(leader) >= db_top) ||
	       (abs(leader) != player)
	    ) continue;
	    strcat((leader>0)?buf:buft, " ");
	    strcat((leader>0)?buf:buft, NAME(abs(thing)));
	}

	if(*buf)  anotify_fmt(player, CINFO "Followers:%s.", buf);
	if(*buft) anotify_fmt(player, CINFO "Waiting to follow:%s.", buft);
	if((!*buf)&&(!*buft))
	    anotify(player, CINFO "You aren't leading anyone.");
	return;
    }

    if(release && !string_compare(name, "all")) {
	if(unfollowall(player))
	     anotify(player, CINFO "You release all of your followers.");
	else anotify(player, CINFO "You aren't leading anyone.");
	return;
    }

    init_match(player, name, NOTYPE, &md);
    match_neighbor(&md);

    if((thing = noisy_match_result(&md)) != NOTHING) {
	if(( (Typeof(thing) != TYPE_THING) || !(FLAGS(thing)&ZOMBIE) ) &&
	   (Typeof(thing) != TYPE_PLAYER)
	) {
	    anotify(player, CFAIL "That can't follow you.");
	    return;
	}

	if(thing == player) {
	    anotify(player, CFAIL "You'd end up going in circles!");
	    return;
	}

	leader = getleader(thing);

	if(abs(leader) != player) {
	    if(release) anotify_fmt(player, CINFO "You aren't leading %s.", NAME(thing));
	    else anotify_fmt(player, CINFO "%s hasn't asked to follow you.", NAME(thing));
	    return;
	}

	if(release) {
	    setleader(thing, 0);
	    if(leader > 0) {
		anotify_fmt(thing, CINFO "%s stops leading you.", NAME(player));
		anotify_fmt(player, CINFO "You stop leading %s.", NAME(thing));
	    } else {
		anotify_fmt(thing, CINFO "%s doesn't want to lead you.", NAME(player));
		anotify_fmt(player, CINFO "You refuse to lead %s.", NAME(thing));
	    }
	} else {
	    if(leader > 0) {
		anotify_fmt(player, CINFO "You are already leading %s.", NAME(thing));
		return;
	    }

	    setleader(thing, player);
	    anotify_fmt(thing, CINFO "%s starts leading you.", NAME(player));
	    anotify_fmt(player, CSUCC "You start leading %s.", NAME(thing));
	}
    }
}
