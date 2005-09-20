#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <ctype.h>

#include "color.h"
#include "db.h"
#include "props.h"
#include "interface.h"
#include "tune.h"
#include "reg.h"
#include "externs.h"

/* Predicates for testing various conditions */

int 
can_link_to(dbref who, object_flag_type what_type, dbref where)
{
    if (where == HOME)
	return 1;
    if (!OkObj(where))
	return 0;
    switch (what_type) {
	case TYPE_EXIT:
	    return (controls(who, where) || (FLAGS(where) & LINK_OK));
	    /* NOTREACHED */
	    break;
	case TYPE_PLAYER:
	    return (Typeof(where) == TYPE_ROOM && (controls(who, where)
						   || Linkable(where)));
	    /* NOTREACHED */
	    break;
	case TYPE_ROOM:
	    return ((Typeof(where) == TYPE_ROOM || Typeof(where) == TYPE_THING)
		    && (controls(who, where) || Linkable(where)));
	    /* NOTREACHED */
	    break;
	case TYPE_THING:
	    return ((Typeof(where) == TYPE_ROOM || Typeof(where) == TYPE_PLAYER || Typeof(where) == TYPE_THING)
		    && (controls(who, where) || Linkable(where)));
	    /* NOTREACHED */
	    break;
	case NOTYPE:
	    return (controls(who, where) || (FLAGS(where) & LINK_OK) ||
		    (Typeof(where) != TYPE_THING && (FLAGS(where) & ABODE)));
	    /* NOTREACHED */
	    break;
    }
    return 0;
}

int 
can_link(dbref who, dbref what)
{
    return (controls(who, what) || ((Typeof(what) == TYPE_EXIT)
				    && DBFETCH(what)->sp.exit.ndest == 0));
}

/*
 * Revision 1.2 -- SECURE_TELEPORT
 * you can only jump with an action from rooms that you own
 * or that are jump_ok, and you cannot jump to players that are !jump_ok.
 */

int 
could_doit(dbref player, dbref thing)
{
    dbref   source, dest, owner;

    if (Typeof(thing) == TYPE_EXIT) {

	if (DBFETCH(thing)->sp.exit.ndest == 0) {
	    return 0;
	}

	owner = OWNER(thing);
	source = DBFETCH(player)->location;
	dest = *(DBFETCH(thing)->sp.exit.dest);

	if (Typeof(dest) == TYPE_PLAYER) {
	    dbref destplayer = dest;
	    dest = DBFETCH(dest)->location;
	    if (!(FLAGS(destplayer) & JUMP_OK) || (FLAGS(dest) & BUILDER)) {
		return 0;
	    }
	}

	if( (dest != HOME) &&  
	    (Typeof(dest)==TYPE_ROOM) &&
	    Guest(player) &&
	    !(FLAG2(dest)&F2GUEST)
	) {
	    anotify(player, CFAIL "Guests aren't allowed there.");
	    return 0;
	}

	/* for actions */
	if ((DBFETCH(thing)->location != NOTHING) &&
		(Typeof(DBFETCH(thing)->location) != TYPE_ROOM))
	{

	    if ((Typeof(dest) == TYPE_ROOM || Typeof(dest) == TYPE_PLAYER) &&
		    (FLAGS(source) & BUILDER))
		return 0;

	    if (tp_secure_teleport && Typeof(dest) == TYPE_ROOM) {
		if ((dest != HOME) && (!controls(owner, source))
			&& ((FLAGS(source) & JUMP_OK) == 0)) {
		    return 0;
		}
	    }
	}
    }
    return (eval_boolexp(player, GETLOCK(thing), thing));
}


int
test_lock(dbref player, dbref thing, const char *lockprop)
{
    struct boolexp *lokptr;

    lokptr = get_property_lock(thing, lockprop);
    return (eval_boolexp(player, lokptr, thing));
}


int
test_lock_false_default(dbref player, dbref thing, const char *lockprop)
{
    struct boolexp *lok = get_property_lock(thing, lockprop);

    if (lok == TRUE_BOOLEXP) return 0;
    return (eval_boolexp(player, lok, thing));
}


int 
can_doit(dbref player, dbref thing, const char *default_fail_msg)
{
    dbref   loc;

    if ((loc = getloc(player)) == NOTHING) {
      return 0;
    }

    if (!Mage(OWNER(player)) && Typeof(player) == TYPE_THING &&
	    (FLAGS(thing) & ZOMBIE)) {
      anotify(player, CFAIL "Sorry, but zombies can't do that.");
      return 0;
    }

    if (!Mage(player) && Guest(player) && Typeof(thing) == TYPE_EXIT ) {
      if (tp_exit_guest_flag) {
	if (!(FLAG2(thing)&F2GUEST)) {
	  anotify(player, CFAIL "Sorry, but guests can't do that.");
	  return 0;
	}
      } else {
	if ((FLAG2(thing)&F2GUEST)) {
	  anotify(player, CFAIL "Sorry, but guests can't do that.");
	  return 0;
	}
      }
    }

    if (!could_doit(player, thing)) {
      /* can't do it */
      if (GETFAIL(thing)) {
	exec_or_notify(player, thing, GETFAIL(thing), "(@Fail)");
      } else if (default_fail_msg) {
	anotify(player, default_fail_msg);
      }
      if (GETOFAIL(thing) /*&& !Dark(player)*/) {
	parse_omessage(player, loc, thing, GETOFAIL(thing),
		       PNAME(player), "(@Ofail)");
      }
      return 0;
    } else {
      /* can do it */
      if (GETSUCC(thing)) {
	exec_or_notify(player, thing, GETSUCC(thing), "(@Succ)");
      }
      if (GETOSUCC(thing) /*&& !Dark(player)*/) {
	parse_omessage(player, loc, thing, GETOSUCC(thing),
		       NAME(player), "(@Osucc)");
      }
      return 1;
    }
}

int 
can_see(dbref player, dbref thing, int can_see_loc)
{
    if (player == thing || Typeof(thing) == TYPE_EXIT
	    || Typeof(thing) == TYPE_ROOM)
	return 0;

    if (can_see_loc) {
	switch (Typeof(thing)) {
	    case TYPE_PROGRAM:
		return ((FLAGS(thing) & LINK_OK) || controls(player, thing));
	    case TYPE_PLAYER:
		if (tp_dark_sleepers) {
		    return (/* !Dark(thing) && */ online(thing));
		} else return 1;
	    default:
		return 1;
		return (!Dark(thing) ||
		     (controls(player, thing) && !(FLAGS(player) & STICKY)));
		
	}
    } else {
	/* can't see loc */
	return (controls(player, thing) && !(FLAGS(player) & STICKY));
    }
}

int
controls(dbref who, dbref what)
{
  return why_controls(who, what, 0) > 0;
}

int 
why_controls(dbref who, dbref what, int mlev)
{
    dbref index;
    int realms = 0, perms = 0;

    /* Invalid objects control nothing */
    if (!OkObj(who))
	return -1;

    /* No one controls invalid objects */
    if (!OkObj(what))
	return -1;

    if(mlev <= 0 || mlev > LMAN)
	mlev = QLevel(who);
    else perms = 1; /* We are checking permissions for muf */

    /* No one controls garbage */
    if (Typeof(what) == TYPE_GARBAGE)
	return -2;

    /* All controls checks are based on owning player, not zombies */
    who = OWNER(who);

    /* owners control their own stuff */
    if (who == OWNER(what))
	return 1;

    /* Player #1 is always in control */
    if(mlev >= LMAN)
	return 2;

    /* W3 and up can change most anything unless it's more powerful */
    if ((mlev >= LARCH) && (mlev >= WLevel(OWNER(what))))
	return 3;
    
    if (mlev >= LMAGE) {
	/* Mages+ control everything else whose owners aren't tinkerproof
	   If a wizard sets themself 'GUEST', all wizards can tinker with
	   things they control unless the object's Mucker level is higher
	   than the wizard's mucker level.
	*/
	if ( ( (FLAG2( what ) & F2TINKERPROOF) ||
	       (FLAG2(OWNER(what)) & F2GUEST)
	     ) && (Typeof(what) != TYPE_PLAYER)
	) {
	    return ( mlev >= WLevel(what) ) ? 5 : -4 ;
	} else if ( !(FLAG2( OWNER(what) ) & F2TINKERPROOF) ) {
	    return ( mlev >= WLevel(OWNER(what)) ) ? 3 : -4;
	} else {
	    /* Mages+ control any object whose owner is set 'G' */
	    /* Fortunately, wizards can't be guests */
	    return ( Guest(OWNER(what)) ) ? 6 : -5 ;
	}
    } else if( tp_realms_control && (!(FLAGS(who) & QUELL)) && !perms ) {
	/* Realm Owner controls every non-wizard owned thing, room, and exit
	   under his environment, but not in muf. */
	for (index=what; index != NOTHING; index = getloc(index)) {
	    if ((OWNER(index) == who) && (Typeof(index) == TYPE_ROOM)
		    && TMage(index)
	    ) {
		realms = 1; /* Player is mortal but could control object */
		break;
	    }
	}
	if(realms) {
	    if( TMage( OWNER(what) ) ) {
		return -6;
	    } else if( (FLAG2( OWNER(what) ) & F2TINKERPROOF) ) {
		return -5;
	    } else {
		return ((Typeof(what) == TYPE_THING) ||
			(Typeof(what) == TYPE_ROOM) ||
			(Typeof(what) == TYPE_EXIT)
		) ? 4 : -7 ;
	    }
	} else return 0;
    } else return 0;
}

int 
restricted(dbref player, dbref thing, object_flag_type flag)
{
    switch (flag) {
	case ABODE:
	    return (!Mage(OWNER(player)) &&
		    (Typeof(thing) == TYPE_PROGRAM));
	    break;
	case ZOMBIE:
	    if (Typeof(thing) == TYPE_PLAYER)
		return(!Mage(OWNER(player)));
	    if ((Typeof(thing) == TYPE_THING) &&
		    (FLAGS(OWNER(player)) & ZOMBIE))
		return(!Mage(OWNER(player)));
	    return(0);
	case VEHICLE:
	    if (Typeof(thing) == TYPE_PLAYER)
		return(!Mage(OWNER(player)));
	    if (Typeof(thing) == TYPE_PROGRAM)
		return ( (!Boy(OWNER(player))) &&
		     (	(!( OWNER(player) == OWNER(thing) )) ||
			force_level || alias_level
		     )
		   );
	    if (tp_wiz_vehicles) {
		if (Typeof(thing) == TYPE_THING)
		    return(!Mage(OWNER(player)));
	    } else {
		if ((Typeof(thing) == TYPE_THING) && (FLAGS(player) & VEHICLE))
		    return(!Mage(OWNER(player)));
	    }
	    return(0);
	case DARK:
	    if (!Mage(OWNER(player))) {
		if (Typeof(thing) == TYPE_PLAYER)
		    return(1);
		if (!tp_exit_darking && Typeof(thing) == TYPE_EXIT)
		    return(1);
		if (!tp_thing_darking && Typeof(thing) == TYPE_THING)
		    return(1);
	    }
	    return(0);
	    break;
	case QUELL:
	    return ((TBoy(thing) || (!Boy(player) && TMage(thing))) &&
	    	    (thing != player) && !Man(player) &&
		    (Typeof(thing) == TYPE_PLAYER));
	    break;
	case BUILDER:
	    return ( !Mage(OWNER(player)) ||
		     (MLevel(OWNER(player)) < tp_muf_mpi_flag_mlevel)
	           );
	    break;
#ifdef MUD
	case KILL_OK:
	    return (!Mage(OWNER(player)));
	    break;
#endif
	case W3:
	    if(!tp_multi_wiz_levels) {
		return (TMan(OWNER(thing)) || (Typeof(thing) == TYPE_PLAYER))
		    ? (!Man(OWNER(player)))
		    : (!Arch(OWNER(player)));
		break;
	    }
	case W1:	/* We use @set to make our own rules for these */
	case W2:
	    return 1;
	    break;
	default:
	    return 0;
	    break;
    }

    return 0;
}


int 
restricted2(dbref player, dbref thing, object_flag_type flag)
{
    switch (flag) {
	case F2IDLE:
	case F2OFFER:
	case F2GUEST:
	case F2WWW:
	    return (!Mage(OWNER(player)));
	    break;
	case F2LOGWALL:
	case F2SUSPECT:
	    return (!Boy(OWNER(player)));
	case F2TINKERPROOF:
	    return ( (!Boy(OWNER(player))) &&
		     (	(!( OWNER(player) == OWNER(thing) )) ||
			force_level || alias_level ||
			( tp_tinkerproof_mlevel > MLevel(OWNER(player)) )
		     )
		   );
	case F2MPI:
	    if(Typeof(thing) == TYPE_PLAYER)
		return ( (!Mage(OWNER(player))) ||
			 (MLevel(OWNER(player)) < tp_muf_mpi_flag_mlevel)
		       );
	    else
		return (!Meeper(OWNER(player)));
	default:
	    return 0;
	    break;
    }

    return 0;
}


int 
payfor(dbref who, int cost)
{
    who = OWNER(who);
    if (Mage(who)) {
	return 1;
    } else if (DBFETCH(who)->sp.player.pennies >= cost) {
	DBFETCH(who)->sp.player.pennies -= cost;
	DBDIRTY(who);
	return 1;
    } else {
	return 0;
    }
}

int 
word_start(const char *str, const char let)
{
    int     chk;

    for (chk = 1; *str; str++) {
	if (chk && *str == let)
	    return 1;
	chk = *str == ' ';
    }
    return 0;
}

int 
ok_name(const char *name)
{
    return (name
	    && *name
	    && *name != LOOKUP_TOKEN
	    && *name != REGISTERED_TOKEN
	    && *name != NUMBER_TOKEN
	    && !index(name, ARG_DELIMITER)
	    && !index(name, AND_TOKEN)
	    && !index(name, OR_TOKEN)
	    && !index(name, '^')
	    && !index(name, '\r')
	    && !index(name, '\n')
	    && !word_start(name, NOT_TOKEN)
	    && string_compare(name, "me")
	    && string_compare(name, "home")
	    && string_compare(name, "here"));
}

int 
ok_player_name(const char *name)
{
    const char *scan;

    if (!ok_name(name) || strlen(name) > PLAYER_NAME_LIMIT)
	return 0;

    for (scan = name; *scan; scan++) {
	if (!(isprint(*scan) && !isspace(*scan))) {	/* was isgraph(*scan) */
	    return 0;
	}
    }

   if(name_is_bad(name)) return 0;

    /* lookup name to avoid conflicts */
    return (lookup_player(name) == NOTHING);
}

int 
ok_password(const char *password)
{
    const char *scan;

    if (*password == '\0')
	return 0;

    for (scan = password; *scan; scan++) {
	if (!(isprint(*scan) && !isspace(*scan))) {
	    return 0;
	}
    }

    return 1;
}

const char *
hostname_domain(const char *hostname)
{
    const char *domain, *end, *p;

    domain=end=p=hostname;
    while(*p) {
	if(*(p++) == '.') { domain = end; end = p; }
    }
    return domain;
}
