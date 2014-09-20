#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <stdio.h>
#include <ctype.h>

#include "strings.h"
#include "color.h"
#include "db.h"
#include "tune.h"
#include "props.h"
#include "match.h"
#include "interface.h"
#include "externs.h"

/* commands which set parameters */

#ifdef COMPRESS
#define alloc_compressed(x) alloc_string(compress(x))
#else				/* COMPRESS */
#define alloc_compressed(x) alloc_string(x)
#endif				/* COMPRESS */


static dbref 
match_controlled(dbref player, const char *name)
{
    dbref   match;
    struct match_data md;

    init_match(player, name, NOTYPE, &md);
    match_everything(&md);
    match_absolute(&md);

    match = noisy_match_result(&md);
    if (match != NOTHING && !controls(player, match)) {
	anotify(player, CFAIL NOPERM_MESG);
	return NOTHING;
    } else {
	return match;
    }
}

void 
do_name(dbref player, const char *name, char *newname)
{
    dbref   thing;
    char   *password;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "na", newname);
	    return;
	}
    }
#endif /* PATH */


    if ((thing = match_controlled(player, name)) != NOTHING) {
	/* check for bad name */
	if (*newname == '\0') {
	    anotify(player, CINFO "Give it what new name?");
	    return;
	}
	/* check for renaming a player */
	if (Typeof(thing) == TYPE_PLAYER) {

	    if(tp_wiz_name && (!Mage(player)) ) {
		    anotify(player, CINFO "Only " NAMEWIZ "s can change player names.");
		    return;
	    }

	    /* split off password */
	    for (password = newname;
		    *password && !isspace(*password);
		    password++);
	    /* eat whitespace */
	    if (*password) {
		*password++ = '\0';	/* terminate name */
		while (*password && isspace(*password))
		    password++;
	    }
	    /* check for null password */
	    if (!*password) {
		anotify(player, CINFO "You must specify a password to change a player name.");
		anotify(player, CNOTE "E.g.: name player = newname password");
		if( Mage(OWNER(player)))
		    anotify(player,
			CINFO NAMECWIZ "s may use 'yes' for non-" NAMEWIZ " players."
		    );
		return;
	    }
	    if(!Mage(player)||TMage(thing)||strcmp(password,"yes")) {
		if (strcmp(password, DoNull(DBFETCH(thing)->sp.player.password))) {
		    anotify(player, CFAIL "Incorrect password.");
		    return;
		}
	    }
	    if (string_compare(newname, NAME(thing))
		       && !ok_player_name(newname)) {
		anotify(player, CFAIL "That name is either taken or invalid.");
		return;
	    }
	    /* everything ok, notify */
	    log_status("NAME: %s to %s by %s(%d)\n",
		       unparse_object(MAN, thing), newname, NAME(player), player);
	    delete_player(thing);
	    if (NAME(thing))
		free((void *) NAME(thing));
	    ts_modifyobject(thing);
	    NAME(thing) = alloc_string(newname);
	    add_player(thing);
	    anotify(player, CSUCC "Name set.");
	    return;
	} else {
	    if (!ok_name(newname)) {
		anotify(player, CFAIL "That is not a reasonable name.");
		return;
	    }
	}

	/* everything ok, change the name */
	if (NAME(thing)) {
	    free((void *) NAME(thing));
	}
	ts_modifyobject(thing);
	NAME(thing) = alloc_string(newname);
	anotify(player, CSUCC "Name set.");
	DBDIRTY(thing);
	if (Typeof(thing) == TYPE_EXIT && MLevel(thing)) {
	    SetMLevel(thing, 0);
	    anotify(player, CINFO "Action priority Level reset to zero.");
	}
    }
}

#ifdef MUD
void 
do_oname(dbref player, const char *name, const char *oname)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETONAME(thing, oname);

	if(Typeof(thing) == TYPE_PLAYER) {
	    if(*oname)
		anotify(player, CSUCC "Title set.");
	    else
		anotify(player, CSUCC "Title cleared.");
	} else {
	    if(*oname)
		anotify(player, CSUCC "Contents name set.");
	    else
		anotify(player, CSUCC "Contents name cleared.");
	}
    }
}
#endif

void 
do_describe(dbref player, const char *name, const char *description)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "de", description);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETDESC(thing, description);
	if(*description)
	    anotify(player, CSUCC "Description set.");
	else
	    anotify(player, CSUCC "Description cleared.");
    }
}

void 
do_idescribe(dbref player, const char *name, const char *description)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETIDESC(thing, description);
	if(*description)
	    anotify(player, CSUCC "Description set.");
	else
	    anotify(player, CSUCC "Description cleared.");
    }
}

void 
do_doing(dbref player, const char *name, const char *mesg)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if( *mesg ) {
	thing = match_controlled(player, name);
    } else {
	thing = player;
	mesg = name;
    }
    if (thing != NOTHING) {
	ts_modifyobject(thing);
	SETDOING(thing, mesg);
	if(*mesg)
	    anotify(player, CSUCC "Doing set.");
	else
	    anotify(player, CSUCC "Doing cleared.");
    }
}


void 
do_setquota(dbref player, const char *name, const char *type)
{
    int     val;
    dbref   victim;
    char    *prop;

    if(tp_db_readonly) return;

    if(!Mage(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    val = 0;
    while(type[val] && (type[val] != '=') && (type[val] != ':')) {
	val++;
    }

    if((type[val] == '=') || (type[val] == ':'))
	val = atoi(&type[val+1]);
    else val = 0;

    victim = NOTHING;
    if(*name) victim = lookup_player(name);

    if(victim == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }

    if(TMage(victim)) {
	anotify(player, CFAIL "That player doesn't need quota.");
	return;
    }

    switch(type[0]) {
	case 'r':
	case 'R':
	    prop = PROP_QUOTADIR "/rooms";
	    break;
	case 'e':
	case 'E':
	    prop = PROP_QUOTADIR "/exits";
	    break;
	case 't':
	case 'T':
	    prop = PROP_QUOTADIR "/things";
	    break;
	case 'p':
	case 'P':
	    prop = PROP_QUOTADIR "/programs";
	    break;
	default:
	    anotify(player, CFAIL "Invalid quota type.");
	    return;
    }

    set_property(victim, prop, PROP_INTTYP, (PTYPE)(long int)val);
    ts_modifyobject(victim);
    if(val)
	anotify(player, CSUCC "Personalized quota set.");
    else
	anotify(player, CSUCC "Personalized quota cleared.");
}


void 
do_setcontents(dbref player, const char *name, const char *mesg)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETCONTENTS(thing, mesg);
	if(*mesg)
	    anotify(player, CSUCC "Contents set.");
	else
	    anotify(player, CSUCC "Contents cleared.");
    }
}


void 
do_fail(dbref player, const char *name, const char *message)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "fl", message);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETFAIL(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

void 
do_success(dbref player, const char *name, const char *message)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "sc", message);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETSUCC(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

/* sets the drop message for player */
void
do_drop_message(dbref player, const char *name, const char *message)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "dr", message);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETDROP(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

void 
do_osuccess(dbref player, const char *name, const char *message)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "osc", message);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETOSUCC(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

void 
do_ofail(dbref player, const char *name, const char *message)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "ofl", message);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETOFAIL(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

void
do_odrop(dbref player, const char *name, const char *message)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, "odr", message);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETODROP(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

void 
do_oecho(dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETOECHO(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

void 
do_pecho(dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETPECHO(thing, message);
	anotify(player, CSUCC "Message set.");
    }
}

/* sets a lock on an object to the lockstring passed to it.
   If the lockstring is null, then it unlocks the object.
   this returns a 1 or a 0 to represent success. */
int 
setlockstr(dbref player, dbref thing, const char *keyname)
{
    struct boolexp *key;

    if (*keyname != '\0') {
	key = parse_boolexp(player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    return 0;
	} else {
	    /* everything ok, do it */
	    ts_modifyobject(thing);
	    SETLOCK(thing, key);
	    return 1;
	}
    } else {
	ts_modifyobject(thing);
	CLEARLOCK(thing);
	return 1;
    }
}

void 
do_conlock(dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, name, NOTYPE, &md);
    match_everything(&md);
    match_absolute(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify(player, CINFO NOTHERE_MESG);
	    return;
	case AMBIGUOUS:
	    anotify(player, CINFO WHICH_MESG);
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify(player, CFAIL NOPERM_MESG);
		return;
	    }
	    break;
    }

    if (force_level) {
	anotify(player, CFAIL "You can't @conlock from an @force or {force}.");
	return;
    }

    if (!*keyname) {
	set_property(thing, "_/clk", PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);
	ts_modifyobject(thing);
	anotify(player, CSUCC "Container lock cleared.");
    } else {
	key = parse_boolexp(player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    anotify(player, CINFO BADKEY_MESG);
	} else {
	    /* everything ok, do it */
	    set_property(thing, "_/clk", PROP_LOKTYP, (PTYPE)key);
	    ts_modifyobject(thing);
	    anotify(player, CSUCC "Container lock set.");
	}
    }
}

void 
do_flock(dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, name, NOTYPE, &md);
    match_everything(&md);
    match_absolute(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify(player, CINFO NOTHERE_MESG);
	    return;
	case AMBIGUOUS:
	    anotify(player, CINFO WHICH_MESG);
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify(player, CFAIL NOPERM_MESG);
		return;
	    }
	    break;
    }

    if (!*keyname) {
	set_property(thing, "@/flk", PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);
	ts_modifyobject(thing);
	anotify(player, CSUCC "Force lock cleared.");
    } else {
	key = parse_boolexp(player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    anotify(player, CINFO BADKEY_MESG);
	} else {
	    /* everything ok, do it */
	    set_property(thing, "@/flk", PROP_LOKTYP, (PTYPE)key);
	    ts_modifyobject(thing);
	    anotify(player, CSUCC "Force lock set.");
	}
    }
}

void 
do_chlock(dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, name, NOTYPE, &md);
    match_everything(&md);
    match_absolute(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify(player, CINFO NOTHERE_MESG);
	    return;
	case AMBIGUOUS:
	    anotify(player, CINFO WHICH_MESG);
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify(player, CFAIL NOPERM_MESG);
		return;
	    }
	    break;
    }

    if (!*keyname) {
	set_property(thing, "_/chlk", PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);
	ts_modifyobject(thing);
	anotify(player, CSUCC "Chown lock cleared.");
    } else {
	key = parse_boolexp(player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    anotify(player, CINFO BADKEY_MESG);
	} else {
	    /* everything ok, do it */
	    set_property(thing, "_/chlk", PROP_LOKTYP, (PTYPE)key);
	    ts_modifyobject(thing);
	    anotify(player, CSUCC "Chown lock set.");
	}
    }
}

void 
do_lock(dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(!*keyname) {
	do_unlock(player, name);
	return;
    }

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setlock(player, loc, buf, keyname);
	    return;
	}
    }
#endif /* PATH */

    init_match(player, name, NOTYPE, &md);
    match_everything(&md);
    match_absolute(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify(player, CINFO NOTHERE_MESG);
	    return;
	case AMBIGUOUS:
	    anotify(player, CINFO WHICH_MESG);
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify(player, CFAIL NOPERM_MESG);
		return;
	    }
	    break;
    }

    key = parse_boolexp(player, keyname, 0);
    if (key == TRUE_BOOLEXP) {
	anotify(player, CINFO BADKEY_MESG);
    } else {
	/* everything ok, do it */
	SETLOCK(thing, key);
	ts_modifyobject(thing);
	anotify(player, CSUCC "Locked.");
    }
}

void 
do_unlock(dbref player, const char *name)
{
    dbref   thing;
#ifdef PATH
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);
#endif /* PATH */

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

#ifdef PATH
    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setlock(player, loc, buf, NULL);
	    return;
	}
    }
#endif /* PATH */

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	CLEARLOCK(thing);
	anotify(player, CSUCC "Unlocked.");
    }
}

int
controls_link(dbref who, dbref what)
{
    switch (Typeof(what)) {
	case TYPE_EXIT:
	    {
		int     i = DBFETCH(what)->sp.exit.ndest;

		while (i > 0) {
		    if (controls(who, DBFETCH(what)->sp.exit.dest[--i]))
			return 1;
		}
		if (who == OWNER(DBFETCH(what)->location))
		    return 1;
		return 0;
	    }

	case TYPE_ROOM:
	    {
		if (controls(who, DBFETCH(what)->sp.room.dropto))
		    return 1;
		return 0;
	    }

	case TYPE_PLAYER:
	    {
		if (controls(who, DBFETCH(what)->sp.player.home))
		    return 1;
		return 0;
	    }

	case TYPE_THING:
	    {
		if (controls(who, DBFETCH(what)->sp.thing.home))
		    return 1;
		return 0;
	    }

	case TYPE_PROGRAM:
	default:
	    return 0;
    }
}

void 
do_unlink(dbref player, const char *name)
{
    dbref   exit;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, name, TYPE_EXIT, &md);
    match_all_exits(&md);
    match_player(&md);
    match_registered(&md);
    match_absolute(&md);
    match_here(&md);
    switch (exit = match_result(&md)) {
	case NOTHING:
	    anotify(player, CINFO NOTHERE_MESG);
	    break;
	case AMBIGUOUS:
	    anotify(player, CINFO WHICH_MESG);
	    break;
	default:
	    if (!controls(player, exit) && !controls_link(player, exit)) {
		anotify(player, CFAIL NOPERM_MESG);
	    } else {
		switch (Typeof(exit)) {
		    case TYPE_EXIT:
			if (DBFETCH(exit)->sp.exit.ndest != 0) {
			    DBFETCH(OWNER(exit))->sp.player.pennies += tp_link_cost;
			    DBDIRTY(OWNER(exit));
			}
			ts_modifyobject(exit);
			DBSTORE(exit, sp.exit.ndest, 0);
			if (DBFETCH(exit)->sp.exit.dest) {
			    free((void *) DBFETCH(exit)->sp.exit.dest);
			    DBSTORE(exit, sp.exit.dest, NULL);
			}
			anotify(player, CSUCC "Unlinked.");
			if (MLevel(exit)) {
			    SetMLevel(exit, 0);
			    anotify(player, CINFO "Action priority Level reset to 0.");
			}
			break;
		    case TYPE_ROOM:
			ts_modifyobject(exit);
			DBSTORE(exit, sp.room.dropto, NOTHING);
			anotify(player, CSUCC "Dropto removed.");
			break;
		    case TYPE_THING:
			ts_modifyobject(exit);
			DBSTORE(exit, sp.thing.home, OWNER(exit));
			anotify(player, CSUCC "Thing's home reset to owner.");
			break;
		    case TYPE_PLAYER:
			ts_modifyobject(exit);
			DBSTORE(exit, sp.player.home, RootRoom);
			anotify(player, CSUCC "Player's home reset to default player start room.");
			break;
		    default:
			anotify(player, CFAIL "You can't unlink that!");
			break;
		}
	    }
    }
}

void 
do_chown(dbref player, const char *name, const char *newowner)
{
    dbref   thing;
    dbref   owner;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, name, NOTYPE, &md);
    match_everything(&md);
    if ((thing = noisy_match_result(&md)) == NOTHING)
	return;

    if (*newowner && string_compare(newowner, "me")) {
	if ((owner = lookup_player(newowner)) == NOTHING) {
	    anotify(player, CINFO WHO_MESG);
	    return;
	}
    } else {
	owner = OWNER(player);
    }

    if ( !controls(OWNER(player), owner) ) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if(!tp_multi_wiz_levels) {
	/* Can't steal to/from the Man unless you are the Man */
	if( (!TMan(OWNER(player))) && (
		TMan(owner) || TMan(OWNER(thing))
	    )
	) {
	    anotify(player, CFAIL NOPERM_MESG);
	    return;
	}
    }

    if (!controls(OWNER(player),thing) && (
	    !(FLAGS(thing) & CHOWN_OK) ||
	    Typeof(thing) == TYPE_PROGRAM ||
	    !test_lock(player, thing, "_/chlk")
	)
    ) {
	anotify(player, CFAIL "You can't take possession of that.");
	return;
    }
    if (has_quotas(OWNER(player))) {
	if(!quota_check(OWNER(player),thing,0)) {
	    anotify(player, CFAIL NOQUOTA_MESG);
	    return;
	}
    }
    if (tp_realms_control && !Mage(OWNER(player)) && TMage(thing) &&
	Typeof(thing) == TYPE_ROOM
    ) {
	anotify(player, CFAIL "You can't take possession of realms parent rooms.");
	return;
    }
    switch (Typeof(thing)) {
	case TYPE_ROOM:
	    if (!Mage(OWNER(player)) && DBFETCH(player)->location != thing) {
		anotify(player, CINFO "You can only chown \"here\".");
		return;
	    }
	    ts_modifyobject(thing);
	    OWNER(thing) = OWNER(owner);
	    break;
	case TYPE_THING:
	    if (!Mage(OWNER(player)) && DBFETCH(thing)->location != player) {
		anotify(player, CINFO "You aren't carrying that.");
		return;
	    }
	    ts_modifyobject(thing);
	    OWNER(thing) = OWNER(owner);
	    break;
	case TYPE_PLAYER:
	    anotify(player, CFAIL "Players always own themselves.");
	    return;
	case TYPE_EXIT:
	case TYPE_PROGRAM:
	    ts_modifyobject(thing);
	    OWNER(thing) = OWNER(owner);
	    break;
	case TYPE_GARBAGE:
	    anotify(player, CFAIL "Nobody wants garbage.");
	    return;
    }
    if (owner == player) {
	char    buf[BUFFER_LEN], buf1[BUFFER_LEN];
	strcpy( buf1, unparse_object(player, thing));
	sprintf(buf, CSUCC "Owner of %s changed to you.", buf1 );
	anotify(player, buf);
    } else {
	char    buf[BUFFER_LEN], buf1[BUFFER_LEN], buf2[BUFFER_LEN];

	strcpy( buf1, unparse_object(player, thing));
	strcpy( buf2, unparse_object(player, owner));
	sprintf(buf, CSUCC "Owner of %s changed to %s.", buf1, buf2 );
	anotify(player, buf);
    }
    DBDIRTY(thing);
}


/* Note: Gender code taken out.  All gender references are now to be handled
   by property lists...
   Setting of flags and property code done here.  Note that the PROP_DELIMITER
   identifies when you're setting a property.
   A @set <thing>= :clear
   will clear all properties.
   A @set <thing>= type:
   will remove that property.
   A @set <thing>= propname:string
   will add that string property or replace it.
   A @set <thing>= propname:^value
   will add that integer property or replace it.
 */

void
do_sm( dbref player, dbref thing, int mlev ) {

    if(Typeof(player) != TYPE_PLAYER) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if(tp_multi_wiz_levels) {
	if( (mlev > MLevel(player)) || (MLevel(thing) > MLevel(player)) ||
	    ( (!Mage(player)) &&
	      (Typeof(thing) != TYPE_PROGRAM )
	    ) ||
	    ( (Typeof(thing) == TYPE_PLAYER) && !Boy(player) &&
		    ( (mlev >= LMAGE) || TMage(thing) )
	    ) ||
	    ( (Typeof(thing) == TYPE_PLAYER) && !Man(player) &&
		    ( (mlev >= LBOY) || TBoy(thing) )
	    ) ||
	    ( (Typeof(thing) == TYPE_PLAYER) &&
	      ( !Mage(player) ||
	        (MLevel(player) < tp_muf_mpi_flag_mlevel) ||
		(mlev >= MLevel(player)) ||
		(MLevel(thing) >= MLevel(player))
	      ) &&
	      !Boy(player)
	    )
	) {
	    anotify(player, CFAIL NOPERM_MESG);
	    return;
	}
    } else { /* Single wiz sm only sets M1-M3 */
	mlev &= 3; /* M1-M3 only */

	if( ( (!Mage(player)) && (Typeof(thing) != TYPE_PROGRAM) ) ||
	    ( mlev > MLevel(player) )
	) {
	    anotify(player, CFAIL NOPERM_MESG);
	    return;
	}
    }

    if ((force_level || alias_level) &&
	((mlev >= LMAGE) || (Typeof(thing) == TYPE_PLAYER))
    ) {
	anotify(player, CFAIL "You can't set this flag from an @force, {force}, or alias.");
	return;
    }

    if (mlev >= LMAGE)
	anotify(player, CSUCC NAMECWIZ " level set.");	
    else if (mlev)
	anotify(player, CSUCC "Mucker level set.");
    else if(RawMLevel(thing) >= LMAGE)
	anotify(player, CSUCC NAMECWIZ " bit removed.");
    else
	anotify(player, CSUCC "Mucker bit removed.");

    SetMLevel(thing, mlev);
}


void do_species(dbref player, const char *name, const char *species) {
    dbref   thing;

    if(!tp_gender_commands) {
	anotify_fmt( player, CINFO "%s", tp_huh_mesg );
	return;
    }

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if(!*name) {
	if(GETSPECIES(player))
	    anotify_fmt(player, CINFO "Your species is: %s", GETSPECIES(player));
	else
	    anotify(player, CINFO "You have no species set.");
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETSPECIES(thing, species);
	if(*species)
	    anotify(player, CSUCC "Species set.");
	else
	    anotify(player, CSUCC "Species cleared.");
    }
}


void do_sex(dbref player, const char *name, const char *gender) {
    dbref   thing;

    if(!tp_gender_commands) {
	anotify_fmt( player, CINFO "%s", tp_huh_mesg );
	return;
    }

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if(!*name) {
	if(GETSEX(player))
	    anotify_fmt(player, CINFO "Your gender is: %s", GETSEX(player));
	else
	    anotify(player, CINFO "You have no gender set.");
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETSEX(thing, gender);
	if(*gender)
	    anotify(player, CSUCC "Gender set.");
	else
	    anotify(player, CSUCC "Gender cleared.");
    }
}


void do_position(dbref player, const char *name, const char *position) {
    dbref   thing;

    if(!tp_gender_commands) {
	anotify_fmt( player, CINFO "%s", tp_huh_mesg );
	return;
    }

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if(!*name) {
	if(GETPOS(player))
	    anotify_fmt(player, CINFO "Your position is: %s", GETPOS(player));
	else
	    anotify(player, CINFO "You have no position set.");
	return;
    }

    if ((thing = match_controlled(player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETPOS(thing, position);
	if(*position)
	    anotify(player, CSUCC "Position set.");
	else
	    anotify(player, CSUCC "Position cleared.");
    }
}


void 
do_set(dbref player, const char *name, const char *flag)
{
    dbref   thing;
    const char *p;
    object_flag_type f=0, f2=0;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (tp_db_readonly) {
	anotify(player, CFAIL DBRO_MESG);
	return;
    }

    /* find thing */
    if ((thing = match_controlled(player, name)) == NOTHING)
	return;

    /* move p past NOT_TOKEN if present */
    for (p = flag; *p && (*p == NOT_TOKEN || isspace(*p)); p++);

    /* Now we check to see if it's a property reference */
    /* if this gets changed, please also modify boolexp.c */
    if (index(flag, PROP_DELIMITER)) {
	/* copy the string so we can muck with it */
	char   *type = alloc_string(flag);	/* type */
	char   *class = (char *) index(type, PROP_DELIMITER);	/* class */
	char   *x;	/* to preserve string location so we can free it */
	char   *temp;
	int ival = 0;

	x = type;
	while (isspace(*type) && (*type != PROP_DELIMITER))
	    type++;
	if (*type == PROP_DELIMITER) {
	    /* clear all properties */
	    for (type++; isspace(*type); type++);
	
	    if (string_compare(type, "clear")) {
		anotify(player, CINFO "Use '@set <obj>=:clear' to clear all props on an object.");
		free((void *) x);
		return;
	    }
	    remove_property_list(thing, Wiz(OWNER(player)));
	    ts_modifyobject(thing);
	    anotify(player, CSUCC "All user-owned properties removed.");
	    free((void *) x);
	    return;
	}
	/* get rid of trailing spaces and slashes */
	for (temp = class - 1; temp >= type && isspace(*temp); temp--);
	while (temp >= type && *temp == '/') temp--;
	*(++temp) = '\0';

	class++;		/* move to next character */
	/* while (isspace(*class) && *class) class++; */
	if (*class == '^' && number(class+1))
	    ival = atoi(++class);

	if ((QLevel(OWNER(player)) < tp_hidden_prop_mlevel) && Prop_Hidden(type)){
	    anotify(player, CFAIL NOPERM_MESG);
	    free((void *) x);
	    return;
	} else if(
	    (
		PropDir_Check(type, PROP_HIDDEN, PROP_ALIAS) ||
		PropDir_Check(type, PROP_HIDDEN, PROP_GLOW )
	    ) && (
		(force_level) || (alias_level) ||
		(!controls(OWNER(player), thing)) || (!Boy(OWNER(player)))
	    )
	) {
	    anotify(player, CFAIL NOPERM_MESG);
	    free((void *) x);
	    return;
	}
	if ((QLevel(OWNER(player)) < tp_seeonly_prop_mlevel) && Prop_SeeOnly(type)){
	    anotify(player, CFAIL NOPERM_MESG);
	    free((void *) x);
	    return;
	}
	if (!(*class)) {
	    ts_modifyobject(thing);
	    remove_property(thing, type);
	    anotify(player, CSUCC "Property removed.");
	} else {
	    ts_modifyobject(thing);
	    if (ival) {
		add_property(thing, type, NULL, ival);
	    } else {
		add_property(thing, type, class, 0);
	    }
	    anotify(player, CSUCC "Property set.");
	}
	free((void *) x);
	return;

#ifdef MUD
    } else if (index(flag, '=')) {
	/* copy the string so we can muck with it */
	char   *type = alloc_string(flag);	/* type */
	char   *class = (char *) index(type, '=');	/* class */
	char   *x;	/* to preserve string location so we can free it */
	char   *temp;
	int ival = 0;

	x = type;

	if (!Mage(OWNER(player))){
	    anotify(player, CFAIL NOPERM_MESG);
	    free((void *) x);
	    return;
	}

	while (isspace(*type) && (*type != '='))
	    type++;

	/* get rid of trailing spaces and slashes */
	for (temp = class - 1; temp >= type && isspace(*temp); temp--);
	*(++temp) = '\0';

	class++;		/* move to next character */
	while (isspace(*class) && *class) class++;
	ival = atoi(class);

	if (KILLER(thing)) {
		   if (string_prefix("hitpoints", type)) {
		SETHIT(thing, ival);
	    } else if (string_prefix("magicpoints", type)) {
		SETMGC(thing, ival);
	    } else if (string_prefix("experience", type)) {
		SETEXP(thing, ival);
	    } else if (string_prefix("level", type)) {
		if(ival <= 0) {
		    SETEXP(thing, 0);
		    SETHIT(thing, 0);
		    SETMGC(thing, 0);
		    SETWGT(thing, 0);
		    SETFGT(thing, 0);
		    FLAGS(thing) &= ~KILL_OK;
		} else {
		    SETLEV(thing, ival);
		    restore(thing, 0);
		}
	    } else if (string_prefix("weight", type)) {
		SETWGT(thing, ival);
	    } else if (string_prefix("restore", type)) {
		restore(thing, 1);
	    } else {
		anotify(player, CFAIL "Invalid or unknown setting.");
		free((void *) x);
		return;
	    }
	} else if (ITEM(thing)) {
		   if (string_prefix("armor", type)) {
		SETARM(thing, ival);
	    } else if (string_prefix("damage", type)) {
		SETDAM(thing, ival);
	    } else if (string_prefix("experience", type)) {
		SETEXP(thing, ival);
	    } else if (string_prefix("level", type)) {
		SETLEV(thing, ival);
	    } else if (string_prefix("weight", type)) {
		SETWGT(thing, ival);
	    } else {
		anotify(player, CFAIL "Invalid or unknown setting.");
		free((void *) x);
		return;
	    }
	} else {
	    anotify(player, CFAIL "That isn't an item or mob.");
	    free((void *) x);
	    return;
	}
	anotify(player, CSUCC "Set.");
	free((void *) x);
	return;
#endif
    }
	/* identify flag */
    if (*p == '\0') {
	anotify(player, CINFO "You must specify a flag to set.");
	return;
    } else if ( string_prefix("ABODE", p) ||
		string_prefix("AUTOSTART", p) ||
		string_prefix("ABATE", p) ) {
	f = ABODE;
    } else if (string_prefix("BUILDER", p) || string_prefix("BOUND", p)) {
	f = BUILDER;
    } else if ( string_prefix("MPI", p) || string_prefix("MEEPER", p) ) {
	f2 = F2MPI;
    } else if ( !string_compare("0", p) || !string_compare("M0", p) ||
	( tp_multi_wiz_levels && (
		!string_compare("W0", p) || ( (*flag == NOT_TOKEN) && (
		 string_prefix("MUFFER", p) || string_prefix("MUCKER", p) ||
		 string_prefix("MAGE", p) || string_prefix("WIZARD", p) ||
		 string_prefix("ARCHWIZARD", p) || string_prefix("BOY", p) ||
		!string_compare("1", p) || !string_compare("2", p) ||
		!string_compare("3", p) || !string_compare("M1", p) ||
		!string_compare("M2", p) || !string_compare("M3", p) ||
		!string_compare("W1", p) || !string_compare("W2", p) ||
		!string_compare("W3", p) || !string_compare("W4", p)
	       ) )
	) ) || ( (!tp_multi_wiz_levels) && (
		( (*flag == NOT_TOKEN) && (
		 string_prefix("MUFFER", p) || string_prefix("MUCKER", p) ||
		!string_compare("1", p) || !string_compare("2", p) ||
		!string_compare("3", p) || !string_compare("M1", p) ||
		!string_compare("M2", p) || !string_compare("M3", p)
	       ) )
	) )
    ) {
	do_sm(player, thing, 0);
	return;
    } else if ( !string_compare("1", p) || !string_compare("M1", p) ||
		string_prefix("MUCKER", p) || string_prefix("MUFFER", p) ) {
	do_sm(player, thing, LM1);
	return;
    } else if ( !string_compare("2", p) || !string_compare("M2", p) ) {
	do_sm(player, thing, LM2);
	return;
    } else if ( !string_compare("3", p) || !string_compare("M3", p) ) {
	do_sm(player, thing, LM3);
	return;
    } else if ( tp_multi_wiz_levels && (
	!string_compare("W", p) || string_prefix("MAGE", p)
    ) ) {
	do_sm(player, thing, LMAGE);
	return;
    } else if ( tp_multi_wiz_levels && (
	!string_compare("W2", p) || string_prefix("WIZARD", p)
    ) ) {
	do_sm(player, thing, LWIZ);
	return;
    } else if ( tp_multi_wiz_levels && (
	!string_compare("W3", p) || string_prefix("ARCHWIZARD", p)
    ) ) {
	do_sm(player, thing, LARCH);
	return;
    } else if ( tp_multi_wiz_levels && (
	!string_compare("W4", p) || string_prefix("BOY", p)
    ) ) {
	do_sm(player, thing, LBOY);
	return;
    } else if ( (!tp_multi_wiz_levels) && (
	string_prefix("WIZARD", p)
    ) ) {
	f = W3;
    } else if (string_prefix("ZOMBIE", p)) {
	f = ZOMBIE;
    } else if (string_prefix("VEHICLE", p) || string_prefix("VIEWABLE", p) ) {
	if (*flag == NOT_TOKEN && Typeof(thing) == TYPE_THING) {
	    dbref obj = DBFETCH(thing)->contents;
	    for (; obj != NOTHING; obj = DBFETCH(obj)->next) {
		if (Typeof(obj) == TYPE_PLAYER) {
		    anotify(player, CINFO "That vehicle still has players in it!");
		    return;
		}
	    }
	}
	f = VEHICLE;
    } else if (string_prefix("LINK_OK", p)) {
	f = LINK_OK;

    } else if (string_prefix("XFORCIBLE", p)) {
	if (force_level || alias_level) {
	    anotify(player, CFAIL "You can't set this flag from an @force, {force}, or alias.");
	    return;
	}
	f = XFORCIBLE;

    } else if (string_prefix("KILL_OK", p)) {
	f = KILL_OK;

    } else if (string_prefix("DARK", p) || string_prefix("DEBUG", p)) {
	f = DARK;
    } else if (string_prefix("STICKY", p) || string_prefix("SETUID", p) ||
	       string_prefix("SILENT", p)) {
	f = STICKY;
    } else if (string_prefix("QUELL", p)) {
	f = QUELL;
    } else if (string_prefix("CHOWN_OK", p) || string_prefix("COUNT_INSTS", p) ||
    	       string_prefix("COLOR_ANSI", p)) {
	f = CHOWN_OK;
    } else if (string_prefix("JUMP_OK", p)) {
	f = JUMP_OK;
    } else if (string_prefix("HAVEN", p) || string_prefix("HARDUID", p)) {
	f = HAVEN;

    } else if (string_prefix("GUEST", p)) {
	f2 = F2GUEST;
    } else if (string_prefix("LOGWALL", p) || !string_compare("$", p)) {
	f2 = F2LOGWALL;
    } else if (string_prefix("WWW_OK", p) || !string_compare("&", p)) {
	f2 = F2WWW;
    } else if (string_prefix("IC", p) || string_prefix("IN_CHARACTER", p)) {
	f2 = F2IC;
    } else if (string_prefix("IDLE", p)) {
	f2 = F2IDLE;
    } else if (string_prefix("PUEBLO", p)) {
	f2 = F2PUEBLO;
    } else if (string_prefix("TINKERPROOF", p)) {
	f2 = F2TINKERPROOF;
    } else if ( string_prefix("OFFER_TASK", p) ||
		string_prefix("TASK_PENDING", p)
    ) {
	f2 = F2OFFER;
    } else if (string_prefix("SUSPECT", p)) {
	f2 = F2SUSPECT;
    } else {
	anotify(player, CINFO "I don't recognize that flag.");
	return;
    }
  if( f2 ) { /* New flags */
    /* check for restricted flag */
    if (restricted2(player, thing, f2)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    /* else everything is ok, do the set */
    if (*flag == NOT_TOKEN) {
	/* reset the flag */
	ts_modifyobject(thing);
	FLAG2(thing) &= ~f2;
	DBDIRTY(thing);
	anotify(player, CSUCC "Flag reset.");
    } else {
	/* set the flag */
	ts_modifyobject(thing);
	FLAG2(thing) |= f2;
	DBDIRTY(thing);
	anotify(player, CSUCC "Flag set.");
    }
  } else {
    /* check for restricted flag */
    if (restricted(player, thing, f)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    /* else everything is ok, do the set */
    if (*flag == NOT_TOKEN) {
	/* reset the flag */
	ts_modifyobject(thing);
	FLAGS(thing) &= ~f;
	DBDIRTY(thing);
	anotify(player, CSUCC "Flag reset.");
    } else {
	/* set the flag */
	ts_modifyobject(thing);
	FLAGS(thing) |= f;
	DBDIRTY(thing);
	anotify(player, CSUCC "Flag set.");
    }
  }
}

void
do_propset(dbref player, const char *name, const char *prop)
{
    dbref   thing, ref;
    char *p, *q;
    char buf[BUFFER_LEN];
    char *type, *pname, *value;
    struct match_data md;
    struct boolexp *lok;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    /* find thing */
    if ((thing = match_controlled(player, name)) == NOTHING)
	return;

    while (isspace(*prop)) prop++;
    strcpy(buf, prop);

    type = p = buf;
    while(*p && *p != PROP_DELIMITER) p++;
    if (*p) *p++ = '\0';

    if (*type) {
	q = type + strlen(type) - 1;
	while (q >= type && isspace(*q)) *(q--) = '\0';
    }

    pname = p;
    while(*p && *p != PROP_DELIMITER) p++;
    if (*p) *p++ = '\0';
    value = p;

    while (*pname == PROPDIR_DELIMITER || isspace(*pname)) pname++;
    if (*pname) {
	q = pname + strlen(pname) - 1;
	while (q >= pname && isspace(*q)) *(q--) = '\0';
    }

    if (!*pname) {
	anotify(player, CINFO "What property?");
	return;
    }

    if ((QLevel(OWNER(player)) < tp_hidden_prop_mlevel) && Prop_Hidden(pname)){
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if ((QLevel(OWNER(player)) < tp_seeonly_prop_mlevel) && Prop_SeeOnly(pname)){
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (!*type || string_prefix("string", type)) {
	add_property(thing, pname, value, 0);
    } else if (string_prefix("integer", type)) {
	if (!number(value)) {
	    anotify(player, CINFO "That's not an integer.");
	    return;
	}
	add_property(thing, pname, NULL, atoi(value));
    } else if (string_prefix("dbref", type)) {
	init_match(player, value, NOTYPE, &md);
	match_absolute(&md);
	match_everything(&md);
	if ((ref = noisy_match_result(&md)) == NOTHING) return;
	set_property(thing, pname, PROP_REFTYP, (PTYPE)(long int)ref);
    } else if (string_prefix("lock", type)) {
	lok = parse_boolexp(player, value, 0);
	if (lok == TRUE_BOOLEXP) {
	    anotify(player, CINFO BADKEY_MESG);
	    return;
	}
	set_property(thing, pname, PROP_LOKTYP, (PTYPE)lok);
    } else if (string_prefix("erase", type)) {
	if (*value) {
	    anotify(player, CINFO "Don't give a value when erasing a property.");
	    return;
	}
	remove_property(thing, pname);
	anotify(player, CSUCC "Property erased.");
	return;
    } else {
	anotify(player, CINFO "What type of property?");
	anotify(player, CNOTE "Valid types are string, int, dbref, lock, and erase.");
	return;
    }
    anotify(player, CSUCC "Property set.");
}

void
do_alias(dbref player, const char *alias, const char *format)
{
    char buf[BUFFER_LEN];

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if((!*alias) || (!*format)) {
	const char *prop;
	char propname[BUFFER_LEN];
	PropPtr propadr, pptr;

#ifdef DISKBASE
	fetchprops(player);
#endif
	anotify(player, CINFO "Command aliases:");
	propadr = first_prop(player, PROP_ALIASDIR "/", &pptr, propname);
	while ((propadr > 0) && *propname) {
	    if((!*alias) || (string_prefix(propname, alias))) {
#ifdef DISKBASE
		propfetch(player, propadr);
#endif
		if((PropType(propadr) == PROP_STRTYP) &&
		   (prop = PropDataStr(propadr))
		) {
#ifdef COMPRESS
		    prop = uncompress(prop);
#endif
		    sprintf(buf, CSUCC "%s" CRED " = " CCYAN "%s", propname, prop);
		    anotify(player, buf);
		}
	    }
	    propadr = next_prop(pptr, propadr, propname);
	}
	anotify(player, CINFO "Done.");
	return;
    }

    if (force_level || alias_level) {
	anotify(player, CFAIL "You can't set an alias from an @force, {force}, or alias.");
	return;
    }

    if( index(alias, ':') || index(alias, ' ') ) {
	anotify(player, CFAIL "Command alias names can't have spaces or colons.");
	return;
    }

    sprintf(buf, PROP_ALIASDIR "/%s", alias);
    ts_modifyobject(player);

    if(string_compare(format, ":clear")) {
	/* add property */
	add_property(player, buf, format, 0);
	anotify(player, CSUCC "Command alias set.");
    } else {
	/* clear property */
	remove_property(player, buf);
	anotify(player, CSUCC "Command alias cleared.");
    }
}
