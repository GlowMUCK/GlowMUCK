#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

/* rob and kill */

#include "color.h"
#include "db.h"
#include "props.h"
#include "tune.h"
#include "interface.h"
#include "match.h"
#include "externs.h"

void 
do_rob(dbref player, const char *what)
{
    dbref   thing;
    char    buf[BUFFER_LEN];
    struct match_data md;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, what, TYPE_PLAYER, &md);
    match_neighbor(&md);
    match_me(&md);
    if (Mage(OWNER(player))) {
	match_absolute(&md);
	match_player(&md);
    }
    thing = match_result(&md);

    switch (thing) {
	case NOTHING:
	case AMBIGUOUS:
	    anotify(player, CINFO WHO_MESG);
	    break;
	default:
	    if (Typeof(thing) != TYPE_PLAYER) {
		anotify(player, CFAIL "You can only rob players.");
	    } else if (DBFETCH(thing)->sp.player.pennies < 1) {
		sprintf(buf, CFAIL "%s has no %s.", NAME(thing), tp_pennies);
		anotify(player, buf);
		sprintf(buf, CBLUE
		     "%s tried to rob you, but you have no %s to take.",
			NAME(player), tp_pennies);
		anotify(thing, buf);
	    } else if (can_doit(player, thing,
				"Your conscience tells you not to.")) {
		/* steal a penny */
		DBFETCH(player)->sp.player.pennies++;
		DBDIRTY(player);
		DBFETCH(thing)->sp.player.pennies--;
		DBDIRTY(thing);
		anotify_fmt(player, CSUCC "You stole a %s.", tp_penny);
		sprintf(buf, CBLUE "%s stole one of your %s!", NAME(player), tp_pennies);
		anotify(thing, buf);
	    }
	    break;
    }
}

void 
do_kill(dbref player, const char *what, int cost)
{
    dbref   victim, loc = getloc(player);
    char    buf[BUFFER_LEN];
    struct match_data md;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if( loc == NOTHING ) return;

    init_match(player, what, TYPE_PLAYER, &md);
    match_neighbor(&md);
    match_me(&md);
    if (Mage(OWNER(player))) {
	match_player(&md);
	match_absolute(&md);
    }
    victim = match_result(&md);

#ifdef MUD
    if(KILLER(player)) {
	switch (victim) {
	  case NOTHING:
	    anotify(player, CINFO "Kill who?");
	    break;
	  case AMBIGUOUS:
	    anotify(player, CINFO "Kill which one?");
	    break;
	  default:
	    if ((!KILLABLE(player)) ||
		(!KILLABLE(victim)) ||
		(loc != getloc(victim))
	    ) {
		anotify_fmt(player, CFAIL "You can't kill %s here.", NAME(victim));
		return;
	    }
	    SETFGT(player, victim);
	    sprintf(buf, CSUCC "You charge at %s!", NAME(victim));
	    anotify(player, buf);
	    sprintf(buf, CBLUE "%s charges at %s!", NAME(player), NAME(victim));
	    anotify_except( DBFETCH(loc)->contents, player, buf, player);
	}
    } else
#endif /* MUD */
    switch (victim) {
	case NOTHING:
	    anotify(player, CINFO "Kill who?");
	    break;
	case AMBIGUOUS:
	    anotify(player, CINFO "Kill which one?");
	    break;
	default:
	    if (Typeof(victim) != TYPE_PLAYER) {
		anotify(player, CFAIL "You can only kill players.");
	    } else {
		/* go for it */
		/* set cost */
		if (cost < tp_kill_min_cost)
		    cost = tp_kill_min_cost;

		if (FLAGS(DBFETCH(player)->location) & HAVEN) {
		    anotify(player, CFAIL "You can't kill anyone here!");
		    break;
		}

		if (tp_restrict_kill) {
		    if (!(FLAGS(player) & KILL_OK)) {
			anotify(player, CINFO "You have to be set Kill_OK to kill someone.");
			break;
		    }
		    if (!(FLAGS(victim) & KILL_OK)) {
			anotify(player, CFAIL "They don't want to be killed.");
			break;
		    }
		}

		/* see if it works */
		if (!payfor(player, cost)) {
		    anotify_fmt(player, CFAIL "You don't have enough %s.", tp_pennies);
		} else if ((RANDOM() % tp_kill_base_cost) < cost
			   && !Mage(OWNER(victim))) {
		    /* you killed him */
		    if (GETDROP(victim))
			/* give him the drop message */
			notify(player, GETDROP(victim));
		    else {
			sprintf(buf, CSUCC "You killed %s!", NAME(victim));
			anotify(player, buf);
		    }

		    /* now notify everybody else */
		    if (GETODROP(victim)) {
			sprintf(buf, CBLUE "%s killed %s! ", PNAME(player),
				PNAME(victim));
			parse_omessage(player, getloc(player), victim,
					GETODROP(victim), buf, "(@Odrop)");
		    } else {
			sprintf(buf, CBLUE "%s killed %s!", NAME(player), NAME(victim));
		    }
		    anotify_except(DBFETCH(DBFETCH(player)->location)->contents, player, buf, player);

		    /* maybe pay off the bonus */
		    if (DBFETCH(victim)->sp.player.pennies < tp_max_pennies) {
			sprintf(buf, CNOTE "Your insurance policy pays %d %s.",
				tp_kill_bonus, tp_pennies);
			anotify(victim, buf);
			DBFETCH(victim)->sp.player.pennies += tp_kill_bonus;
			DBDIRTY(victim);
		    } else {
			anotify(victim, CBLUE "Your insurance policy has been revoked.");
		    }
		    /* send him home */
		    send_home(victim, 1);

		} else {
		    /* notify player and victim only */
		    anotify(player, CFAIL "Your murder attempt failed.");
		    sprintf(buf, CBLUE "%s tried to kill you!", NAME(player));
		    anotify(victim, buf);
		}
		break;
	    }
    }
}

void 
do_give(dbref player, const char *recipient, int amount)
{
    dbref   who;
    char    buf[BUFFER_LEN];
    struct match_data md;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    /* do amount consistency check */
    if (amount < 0 && !Mage(OWNER(player))) {
	anotify(player, CINFO "Try using the \"rob\" command.");
	return;
    } else if (amount == 0) {
	anotify_fmt(player, CFAIL "You must specify a positive number of %s.",
		   tp_pennies);
	return;
    }
    /* check recipient */
    init_match(player, recipient, TYPE_PLAYER, &md);
    match_neighbor(&md);
    match_me(&md);
    if (Mage(OWNER(player))) {
	match_player(&md);
	match_absolute(&md);
    }
    switch (who = match_result(&md)) {
	case NOTHING:
	case AMBIGUOUS:
	    anotify(player, CINFO WHO_MESG);
	    return;
	default:
	    if (!Mage(OWNER(player))) {
		if (Typeof(who) != TYPE_PLAYER) {
		    anotify(player, CFAIL "You can only give to other players.");
		    return;
		} else if (DBFETCH(who)->sp.player.pennies + amount >
			   tp_max_pennies) {
		    anotify_fmt(player, CFAIL
			"That player doesn't need that many %s!",
			tp_pennies);
		    return;
		}
	    }
	    break;
    }

    /* try to do the give */
    if (!payfor(player, amount)) {
	anotify_fmt(player, CFAIL "You don't have that many %s to give!", tp_pennies);
    } else {
	/* he can do it */
	switch (Typeof(who)) {
	    case TYPE_PLAYER:
		DBFETCH(who)->sp.player.pennies += amount;
		sprintf(buf, CSUCC "You give %d %s to %s.",
			amount,
			amount == 1 ? tp_penny : tp_pennies,
			NAME(who));
		anotify(player, buf);
		sprintf(buf, CINFO "%s gives you %d %s.",
			NAME(player),
			amount,
			amount == 1 ? tp_penny : tp_pennies);
		anotify(who, buf);
		break;
	    case TYPE_THING:
		DBFETCH(who)->sp.thing.value += amount;
		sprintf(buf, CSUCC "You change the value of %s to %d %s.",
			NAME(who),
			DBFETCH(who)->sp.thing.value,
		    DBFETCH(who)->sp.thing.value == 1 ? tp_penny : tp_pennies);
		anotify(player, buf);
		break;
	    default:
		anotify_fmt(player, CFAIL "You can't give %s to that!", tp_pennies);
		break;
	}
	DBDIRTY(who);
    }
}
