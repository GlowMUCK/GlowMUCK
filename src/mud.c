/* mud.c Copyright 1996 by Andrew Nelson All Rights Reserved */
/* This code is still in alpha testing, it's far from usable */

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

/* Item of greatest damage potential on mob */

dbref
dam_obj(dbref mob) {
    dbref item, best=NOTHING;
    int dam = 0;

    DOLIST(item, DBFETCH(mob)->contents) {
	if(ITEM(item) && (GETDAM(item) > dam)) {
	    best = item;
	    dam = GETDAM(item);
	}
    }
    return best;
}


/* Item of greatest protection on mob */

dbref
arm_obj(dbref mob) {
    dbref item, best=NOTHING;
    int arm = 0;

    DOLIST(item, DBFETCH(mob)->contents) {
	if (ITEM(item) && (GETARM(item) > arm)) {
	    best = item;
	    arm = GETARM(item);
	}
    }
    return best;
}


/* Item/object stats */

void 
stats(dbref player, dbref thing) {
    char buf[BUFFER_LEN];

    if (ITEM(thing)) {
	sprintf(buf,
	    CRED "Damage: " CYELLOW "%d" CCYAN "  Armor: " CYELLOW "%d"
	    CPURPLE "  Level: " CYELLOW "%d" CGREEN "  Exp: " CYELLOW "%d",
	    GETDAM(thing), GETARM(thing),
	    LEVEL(thing), (GETEXP(thing) % LEVEXP)
	);
	anotify(player, buf);
    } else if (KILLER(thing)) {
	if (MOB(thing)) {
	    sprintf(buf,
	        CRED "Health: " CYELLOW "%d" CBLUE "/" CYELLOW "%d"
		CCYAN "  Magic: " CYELLOW "%d" CBLUE "/" CYELLOW "%d"
		CPURPLE "  Level: " CYELLOW "%d"
		CGREEN "  Exp: " CYELLOW "%d",
		GETHIT(thing), MAXHIT(thing),
		GETMGC(thing), MAXMGC(thing),
		LEVEL(thing), (GETEXP(thing) % LEVEXP)
	    );
	    anotify(player, buf);
	} else {
	    sprintf(buf,
	        CRED "Health: " CYELLOW "%d" CBLUE "/" CYELLOW "%d"
		CCYAN "  Magic: " CYELLOW "%d" CBLUE "/" CYELLOW "%d"
		CPURPLE "  Level: " CYELLOW "%d"
		CGREEN "  Next Up: " CYELLOW "%d"
		CWHITE " (%s" CWHITE ")",
		GETHIT(thing), MAXHIT(thing),
		GETMGC(thing), MAXMGC(thing),
		LEVEL(thing), LEVEXP - (GETEXP(thing) % LEVEXP),
		(FLAG2(thing)&F2OFFER) ? CRED "task pending" : CGREEN "no task"
	    );
	    anotify(player, buf);
	}
    }
}


/* Get experience */

int
gain_exp(dbref mob, dbref prize) {
    int mlev = LEVEL(mob), plev = LEVEL(prize), exp = 0;

    if ( (Typeof(mob) != TYPE_PLAYER) || !KILLER(mob) ) return 0;

    if(MOB(prize)) {
	if ((plev > mlev + KILLEV) || (mlev > plev + KILLEV)) {
	    return 0;
	}
	exp = MID(TOPEXP, ((GETEXP(prize)%LEVEXP) / (ABS(plev - mlev) + 1)), 0);
    } else if(ITEM(prize)) {
	if ((plev > mlev + KILLEV) || (mlev > plev + KILLEV)) {
	    return 0;
	}
	exp = MID(TOPEXP, ((GETEXP(prize)%LEVEXP) / (ABS(plev - mlev) + 1)), 0);
    }
    ADDEXP(mob, exp);
    return exp;
}


/* Check for level advancement */

void
check_level(dbref mob) {

    if ( (Typeof(mob) != TYPE_PLAYER) || !KILLER(mob) ) return;

    if ( LEVEL(mob) >= MAXLEV ) {
    	SETLEV(mob, MAXLEV);
    	anotify(mob, CNOTE "You have already reached the highest level.");
    	return;
    }

    if ( (GETEXP(mob)%LEVEXP) == (LEVEXP - 1) ) {
	if(FLAG2(mob)&F2OFFER) {
	    anotify(mob, CNOTE
	"You must complete your task before you can go to the next level.");
	    return;
	}
	do_score(mob, 1);
	SETLEV(mob, LEVEL(mob)+1);
	if(!(LEVEL(mob) % TSKLEV)) FLAG2(mob) |= F2OFFER;
	anotify(mob, CNOTE "You have reached the next level.");
	do_score(mob, 1);
	return;
    }
}


/* Check for a kill */

void
slay(dbref killer, dbref victim) {
    char buf[BUFFER_LEN];
    dbref loc = getloc(killer);
    int exp = 0;

    if (!KILLER(killer) || !KILLER(victim) || (loc == NOTHING)) return;

    sprintf(buf, MARK "%s has been killed by %s!", NAME(victim), NAME(killer));
    wall_mud(buf);

    if (Typeof(victim) == TYPE_PLAYER) {
	send_home(victim, 1);
	SETHIT(victim, 1);
	SETEXP(victim, GETEXP(victim) / 2 ); /* Punishment :) */
    } else {
	if(Typeof(killer) == TYPE_PLAYER) exp = gain_exp(killer, victim);
	anotify_fmt(killer, CSUCC "You receive %d %s for your kill.",
	    exp, (exp == 1) ? tp_penny : tp_pennies);

	if(tp_dead_room != NOTHING) {
	    ts_lastuseobject(victim);
	    moveto(victim, tp_dead_room);
	}
	restore(victim, 0);
    }
    SETFGT(victim, NOTHING);
    if(GETFGT(killer) == victim) SETFGT(killer, NOTHING);
    check_level(killer);
    check_level(victim);
}


/* Perform one attack */

void
attack(dbref killer, dbref victim) {
    char buf[BUFFER_LEN];
    dbref loc = getloc(killer);
    dbref weapon=dam_obj(killer);
    dbref armor=arm_obj(victim);
    int damage, protection, pain, chance;
    int klev=LEVEL(killer), vlev=LEVEL(victim);

    if ( loc == NOTHING ) return;
    if ( !KILLER(killer) || !KILLABLE(victim) || (GETHIT(killer) == 0))
	return;

    if ( weapon != NOTHING )
	damage = MID((vlev * 3 * ABSDAM) / MAXLEV, GETDAM(weapon), 1);
    else {
	if (Typeof(killer) == TYPE_PLAYER)
	    damage = (klev * 5 * ABSDAM) / (3 * MAXLEV);
	else
	    damage = (klev * ABSDAM) / MAXLEV;
    }

    if ( armor != NOTHING )
	protection = MID((LEVEL(victim) * 3 * ABSARM) / (MAXLEV * 2), GETARM(armor), 0);
    else
	protection = (LEVEL(victim) * ABSARM) / MAXLEV;

    pain = (damage * klev * klev * 2) / (vlev * protection);
    chance = 3 * (klev - vlev + KILLEV + 10) + rand() % 30;

    if( chance < 50 ) {	/* miss */
	sprintf(buf, CFAIL "%s swings at %s and misses.", NAME(killer), NAME(victim));
	anotify_except(DBFETCH(loc)->contents, NOTHING, buf, killer);
	return;
    }

    /* hit */

    sprintf(buf, CFAIL "%s swings at %s!", NAME(killer), NAME(victim));
    anotify_except(DBFETCH(loc)->contents, NOTHING, buf, killer);

    SETHIT(victim, GETHIT(victim) - pain);
}


/* Check if mob should attack along with his partner */

void
check_attack(dbref mob) {
    dbref loc = getloc(mob), prey=GETFGT(mob), preyloc;

    if (loc == NOTHING) return;
    if (!KILLER(mob) || (prey == NOTHING)) return;
    if (((preyloc = getloc(prey)) == NOTHING) || (loc != preyloc)) return;
    if (!KILLABLE(prey) || (mob == prey)) {
	SETFGT(mob, NOTHING);
	return;
    }
    if (GETFGT(prey) != NOTHING) {
	if(GETFGT(prey) != mob) {
	    SETFGT(mob, NOTHING);
	    return;
	}
	/* We already attacked with them */
	if (prey < mob) return;
    } else SETFGT(prey, mob);

    if (rand()%2) {
	attack( mob, prey );
	if( GETHIT(prey) == 0 ) {
		slay(mob, prey);
	} else {
	    attack( prey, mob );
	    if( GETHIT(mob) == 0 )
		slay(prey, mob);
	}
    } else {
	attack( prey, mob );
	if( GETHIT(mob) == 0 ) {
		slay(prey, mob);
	} else {
	    attack( mob, prey );
	    if( GETHIT(prey) == 0 )
		slay(mob, prey);
	}
    }
}


/* Update the hitpoints and magic power every so often */

void
refresh(dbref mob) {
    int val, better=0;

    if(!KILLER(mob)) return;

    val = MAXHIT(mob);
    if ( GETHIT(mob) < val ) {
	SETHIT( mob, GETHIT(mob) + MAX( RANGE( val/4, val/8 ), 1 ) );
	better=1;
    } else if ( GETHIT(mob) > val ) {
	SETHIT( mob, val );
    }

    val = MAXMGC(mob);
    if ( GETMGC(mob) < val ) {
	SETMGC( mob, GETMGC(mob) + MAX( RANGE( val/4, val/8 ), 1 ) );
	better=1;
    } else if ( GETMGC(mob) > val ) {
	SETMGC( mob, val );
    }

    if ( better && (Typeof(mob) == TYPE_PLAYER) )
	anotify(mob, CINFO "You feel better.");
}


/* Restore hitpoints and magic power */

void
restore(dbref mob, int verbose) {
    int val, max, better = 0;

    if(!KILLER(mob)) return;

    max = MAXHIT(mob);
    val = GETHIT(mob);
    if ( val != max ) {
	SETHIT( mob, max );
	if( val < max )
	    better = 1;
    }

    max = MAXMGC(mob);
    val = GETMGC(mob);
    if ( val != max ) {
	SETMGC( mob, max );
	if( val < max )
	    better = 1;
    }

    if ( verbose && better && (Typeof(mob) == TYPE_PLAYER) )
	anotify(mob, CINFO "You feel great!");
}


/* Update mob movements once every 5 seconds, perform attacks */

void
update_mob(void) {
    dbref ref, loc;
    for(ref = 0; ref < db_top; ref++) {
	loc = getloc(ref);
	if (loc == NOTHING) continue;
	if (KILLER(ref)) check_attack(ref);
    } /* db_top list */
}

/* Check for resets, hitpoint regeneration */

void
update_mud(void) {
    dbref ref, loc;
    int gohome;
    time_t now;

    time(&now);

    for (ref = 0; ref < db_top; ref++) {
	gohome = 0;
	loc = getloc(ref);
	if (loc == NOTHING) continue;
	if(KILLER(ref)) refresh(ref);
	if (MOB(ref)) {
	    if ((DBFETCH(ref)->ts.lastused < (now - MOBWAIT)) && (
		(loc != DBFETCH(ref)->sp.thing.home)
	    )) gohome = 1;
	    if(gohome) {
		send_home(ref, 1);
		loc = getloc(ref);
		if( loc != NOTHING ) {
		    notify_except(DBFETCH(loc)->contents, NOTHING, MARK
			"A puff of smoke clears, revealing a monster!",
			ref
		    );
		}
	    }
	} else if (ITEM(ref)) {
	    switch(Typeof(loc)) {
		case TYPE_PLAYER:
		    if ( (getloc(loc)==NOTHING) ||
			 !(FLAGS(getloc(loc))&KILL_OK) ||
			 !online(loc) ||
			 (minidle(loc) >= tp_maxidle)
		       ) gohome = 1;
		    break;
		case TYPE_ROOM:
		case TYPE_THING:
		    if ((DBFETCH(ref)->ts.lastused < (now - ITEMWAIT)) && (
			(loc != DBFETCH(ref)->sp.thing.home)
		    )) gohome = 1;
		    break;
	    }
	    if(gohome) {
		send_home(ref, 1);
		loc = getloc(ref);
		if( loc != NOTHING ) {
		    notify_except(DBFETCH(loc)->contents, NOTHING, MARK
			"The caretaker appears, drops something, and vanishes.",
			ref
		    );
		}
	    }
	} /* ITEM */
    }
}

void
do_offer( dbref player, const char *what ) {
    dbref thing, loc;
    int exp;
    struct match_data md;

    if (tp_db_readonly) {
	anotify( player, CFAIL DBRO_MESG );
	return;
    }

    if (!KILLER(player)) {
	anotify(player, NOPERM_MESG);
	return;
    }

    if ( (loc = getloc(player)) == NOTHING) return;

    init_match(player, what, NOTYPE, &md);
    match_possession(&md);
    if ((thing = noisy_match_result(&md)) == NOTHING || thing == AMBIGUOUS)
	return;

    if (!(FLAGS(loc)&KILL_OK) || !(FLAG2(loc)&F2OFFER)) {
	anotify(player, CINFO "You cannot offer things here.");
	return;
    }

    if ((!OkObj(tp_offered_room)) ||
	(Typeof(tp_offered_room) != TYPE_ROOM)
    ) {
	anotify( player, CINFO "There is no room for your offering." );
	return;
    }

    if (Typeof(thing) == TYPE_THING) {
	ts_useobject(thing);
	if (!ITEM(thing) || (DBFETCH(thing)->location != player)) {
	    anotify(player, CFAIL "You can't offer that.");
	    return;
	}
	if (parent_loop_check(thing, tp_offered_room)) {
	    anotify(player, CFAIL "You can't offer that.");
	    return;
	}
	ts_lastuseobject(thing);
	moveto(thing, tp_offered_room);
	exp = gain_exp(player, thing);
	anotify_fmt(player, CSUCC "You receive %d %s for your offering.",
	    exp, (exp == 1) ? tp_penny : tp_pennies);
	check_level(player);
    } else {
	anotify(player, CFAIL "You can't offer that.");
	return;
    }
}

#endif /* MUD */
