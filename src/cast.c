/* cast.c Copyright 1996 by Andrew Nelson All Rights Reserved */

#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include "color.h"
#include "db.h"
#include "props.h"
#include "interface.h"
#include "match.h"
#include "tune.h"
#include "externs.h"

#ifdef MUD




void
do_cast( dbref player, const char *arg1, const char *arg2 ) {
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

    init_match(player, arg1, NOTYPE, &md);
    match_possession(&md);
    if ((thing = noisy_match_result(&md)) == NOTHING
	    || thing == AMBIGUOUS)
	return;

    cont = NOTHING;
    if (arg2 && *arg2) {
	init_match(player, arg2, NOTYPE, &md);
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
	    if (Typeof(cont) == TYPE_THING) {
		anotify(player, CSUCC "Put away.");
		return;
	    } else if (Typeof(cont) == TYPE_PLAYER) {
		anotify_fmt(cont, CINFO "%s hands you %s", PNAME(player), PNAME(thing));
		anotify_fmt(player, CSUCC "You hand %s to %s",
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
		sprintf(buf, CINFO "%s drops %s.", PNAME(player), PNAME(thing));
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

#endif /* MUD */
