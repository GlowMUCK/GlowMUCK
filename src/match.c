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
#include "match.h"
#include "interface.h"
#include "strings.h"
#include "externs.h"

/* Routines for parsing arguments */

char    match_cmdname[BUFFER_LEN];	/* triggering command */
char    match_args[BUFFER_LEN];	/* remaining text */

void
init_match(dbref player, const char *name, int type, struct match_data * md)
{
    md->exact_match = md->last_match = NOTHING;
    md->match_count = 0;
    md->match_who = player;
    md->match_from = player;
    md->match_name = name;
    md->check_keys = 0;
    md->preferred_type = type;
    md->longest_match = 0;
    md->match_level = 0;
    md->block_equals = 0;
    md->allow_muf_verbs = 0;
}

void 
init_match_check_keys(dbref player, const char *name, int type,
		      struct match_data * md)
{
    init_match(player, name, type, md);
    md->check_keys = 1;
}

void
set_match_muf_verbs(int allow,
			struct match_data * md)
{
     md->allow_muf_verbs = allow;
}

void 
init_match_remote(dbref player, dbref what, const char *name, int type,
		      struct match_data * md)
{
    init_match(player, name, type, md);
    md->match_from = what;
}

static dbref 
choose_thing(dbref thing1, dbref thing2, struct match_data * md)
{
    int     has1;
    int     has2;
    int     preferred = md->preferred_type;

    if (thing1 == NOTHING) {
	return thing2;
    } else if (thing2 == NOTHING) {
	return thing1;
    }
    if (preferred != NOTYPE) {
	if (Typeof(thing1) == preferred) {
	    if (Typeof(thing2) != preferred) {
		return thing1;
	    }
	} else if (Typeof(thing2) == preferred) {
	    return thing2;
	}
    }
    if (md->check_keys) {
	has1 = could_doit(md->match_who, thing1);
	has2 = could_doit(md->match_who, thing2);

	if (has1 && !has2) {
	    return thing1;
	} else if (has2 && !has1) {
	    return thing2;
	}
	/* else fall through */
    }
    return (RANDOM() % 2 ? thing1 : thing2);
}

void 
match_player(struct match_data * md)
{
    dbref   match;
    const char *p;

    if (*(md->match_name) == LOOKUP_TOKEN
	    && payfor(OWNER(md->match_from), tp_lookup_cost)
    ) {
	for (p = (md->match_name) + 1; isspace(*p); p++);
	if ((match = lookup_player(p)) != NOTHING) {
	    md->exact_match = match;
	}
    }
}

/* returns dbref if registered object found for name, else NOTHING */
dbref 
find_registered_obj(dbref player, const char *name)
{
    dbref   match;
    const char *p;
    char    buf[BUFFER_LEN];
    PropPtr ptr;

    if (*name != REGISTERED_TOKEN)
	return (NOTHING);
    for (p = name + 1; *p && isspace(*p); p++);
    if (!*p) return (NOTHING);
    sprintf(buf, "_reg/%s", p);
    ptr = regenvprop(&player, buf, 0);
    if (!ptr) return NOTHING;
#ifdef DISKBASE
    propfetch(player, ptr);  /* DISKBASE PROPVALS */
#endif
    switch (PropType(ptr)) {
	case PROP_STRTYP:
	    p = PropDataStr(ptr);
#ifdef COMPRESS
	    p = uncompress(p);
#endif
	    if (*p == '#') p++;
	    if (number(p)) {
		match = (dbref) atoi(p);
		if (OkObj(match) &&
			(Typeof(match) != TYPE_GARBAGE))
		    return (match);
	    }
	    break;
	case PROP_REFTYP:
	    match = PropDataRef(ptr);
	    if (OkObj(match) && (Typeof(match) != TYPE_GARBAGE))
		return (match);
	    break;
	case PROP_INTTYP:
	    match = (dbref)PropDataVal(ptr);
	    if ((match > 0) && (match < db_top) && (Typeof(match) != TYPE_GARBAGE))
		return (match);
	    break;
	case PROP_LOKTYP:
	    return (NOTHING);
	    break;
    }
    return (NOTHING);
}


void 
match_registered(struct match_data * md)
{
    dbref   match;

    match = find_registered_obj(md->match_from, md->match_name);
    if (match != NOTHING)
	md->exact_match = match;
}


/* returns nnn if name = #nnn, else NOTHING */
static dbref 
absolute_name(struct match_data * md)
{
    dbref   match;

    if (*(md->match_name) == NUMBER_TOKEN) {
	match = parse_dbref((md->match_name) + 1);
	if (!OkObj(match)) {
	    return NOTHING;
	} else {
	    return match;
	}
    } else {
	return NOTHING;
    }
}

void 
match_absolute(struct match_data * md)
{
    dbref   match;

    if ((match = absolute_name(md)) != NOTHING) {
	md->exact_match = match;
    }
}

void 
match_me(struct match_data * md)
{
    if (!string_compare(md->match_name, "me")) {
	md->exact_match = md->match_who;
    }
}

void 
match_here(struct match_data * md)
{
    if (!string_compare(md->match_name, "here")
	    && DBFETCH(md->match_who)->location != NOTHING) {
	md->exact_match = DBFETCH(md->match_who)->location;
    }
}

void 
match_home(struct match_data * md)
{
    if (!string_compare(md->match_name, "home"))
	md->exact_match = HOME;
}


static void 
match_list(dbref first, struct match_data * md)
{
    dbref   absolute;

    absolute = absolute_name(md);
    if (!controls(OWNER(md->match_from), absolute))
	absolute = NOTHING;

    DOLIST(first, first) {
	if (first == absolute) {
	    md->exact_match = first;
	    return;
	} else if (!string_compare(RNAME(first), md->match_name)) {
	    /* if there are multiple exact matches, randomly choose one */
	    md->exact_match = choose_thing(md->exact_match, first, md);
	} else if (string_match(RNAME(first), md->match_name)) {
	    md->last_match = first;
	    (md->match_count)++;
	}
    }
}

void 
match_possession(struct match_data * md)
{
    match_list(DBFETCH(md->match_from)->contents, md);
}

void 
match_neighbor(struct match_data * md)
{
    dbref   loc;

    if ((loc = DBFETCH(md->match_from)->location) != NOTHING) {
	match_list(DBFETCH(loc)->contents, md);
    }
}

/*
 * match_exits matches a list of exits, starting with 'first'.
 * It will match exits of players, rooms, or things.
 */
void 
match_exits(dbref first, struct match_data * md)
{
    dbref   exit, absolute;
    const char *exitname, *p;
    int     i, exitprog, lev;

    if (first == NOTHING)
	return;			/* Easy fail match */
    if ((DBFETCH(md->match_from)->location) == NOTHING)
	return;

    absolute = absolute_name(md);	/* parse #nnn entries */
    if (!controls(OWNER(md->match_from), absolute))
	absolute = NOTHING;

    DOLIST(exit, first) {
	if (exit == absolute) {
	    md->exact_match = exit;
	    continue;
	}
	exitprog = 0;
	if (FLAGS(exit) & HAVEN) {
	    exitprog = 1;
	} else if (DBFETCH(exit)->sp.exit.dest) {
	    for (i = 0; i < DBFETCH(exit)->sp.exit.ndest; i++)
		if (Typeof((DBFETCH(exit)->sp.exit.dest)[i]) == TYPE_PROGRAM)
		    exitprog = 1;
	}
	exitname = NAME(exit);
	while (*exitname) {	/* for all exit aliases */
	    for (p = md->match_name;	/* check out 1 alias */
		    *p
		    && (DOWNCASE(*p) == DOWNCASE(*exitname))
		    && (*exitname != EXIT_DELIMITER);
		    p++, exitname++);
	    /* did we get a match on this alias? */
	    if (*p == '\0' || (*p == ' ' && exitprog && md->allow_muf_verbs)) {
		/* make sure there's nothing afterwards */
		while (isspace(*exitname))
		    exitname++;
		lev = PLevel(exit);
		if (tp_compatible_priorities && (lev == 1) &&
			    (DBFETCH(exit)->location == NOTHING ||
			     Typeof(DBFETCH(exit)->location) != TYPE_THING ||
			     controls(OWNER(exit), getloc(md->match_from)) )
		)
		    lev = LM1 + 1;
		if (*exitname == '\0' || *exitname == EXIT_DELIMITER) {
		    /* we got a match on this alias */
		    if (lev >= md->match_level) {
			if (strlen(md->match_name) - strlen(p) > md->longest_match) {
			    if (lev > md->match_level) {
				md->match_level = lev;
				md->block_equals = 0;
			    }
			    md->exact_match = exit;
			    md->longest_match = strlen(md->match_name) - strlen(p);
			    if (*p == ' ') {
				strcpy(match_args, p + 1);
				{
				    char   *pp;
				    int     ip;

				    for (ip = 0, pp = (char *) md->match_name; *pp && (pp != p); pp++)
					match_cmdname[ip++] = *pp;
				    match_cmdname[ip] = '\0';
				}
			    } else {
				*match_args = '\0';
				strcpy(match_cmdname, (char *) md->match_name);
			    }
			} else if ((strlen(md->match_name) - strlen(p) == md->longest_match) && !((lev == md->match_level) && (md->block_equals))) {
			    if (lev > md->match_level) {
				md->exact_match = exit;
				md->match_level = lev;
				md->block_equals = 0;
			    } else {
				md->exact_match = choose_thing(md->exact_match, exit, md);
			    }
			    if (md->exact_match == exit) {
				if (*p == ' ') {
				    strcpy(match_args, p + 1);
				    {
					char   *pp;
					int     ip;

					for (ip = 0, pp = (char *) md->match_name; *pp && (pp != p); pp++)
					    match_cmdname[ip++] = *pp;
					match_cmdname[ip] = '\0';
				    }
				} else {
				    *match_args = '\0';
				    strcpy(match_cmdname, (char *) md->match_name);
				}
			    }
			}
		    }
		    goto next_exit;
		}
	    }
	    /* we didn't get it, go on to next alias */
	    while (*exitname && *exitname++ != EXIT_DELIMITER);
	    while (isspace(*exitname))
		exitname++;
	}			/* end of while alias string matches */
next_exit:
	;
    }
}

/*
 * match_invobj_actions
 * matches actions attached to objects in inventory
 */
void 
match_invobj_actions(struct match_data * md)
{
    dbref   thing;

    if (DBFETCH(md->match_from)->contents == NOTHING)
	return;
    DOLIST(thing, DBFETCH(md->match_from)->contents) {
	if (Typeof(thing) == TYPE_THING
		&& DBFETCH(thing)->exits != NOTHING) {
	    match_exits(DBFETCH(thing)->exits, md);
	}
    }
}

/*
 * match_roomobj_actions
 * matches actions attached to objects in the room
 */
void 
match_roomobj_actions(struct match_data * md)
{
    dbref   thing, loc;

    if ((loc = DBFETCH(md->match_from)->location) == NOTHING)
	return;
    if (DBFETCH(loc)->contents == NOTHING)
	return;
    DOLIST(thing, DBFETCH(loc)->contents) {
	if (Typeof(thing) == TYPE_THING && DBFETCH(thing)->exits != NOTHING) {
	    match_exits(DBFETCH(thing)->exits, md);
	}
    }
}

/*
 * match_player_actions
 * matches actions attached to player
 */
void 
match_player_actions(struct match_data * md)
{
    dbref obj;
    switch (Typeof(md->match_from)) {
	case TYPE_PLAYER:
	case TYPE_ROOM:
	case TYPE_THING:
	    obj = DBFETCH(md->match_from)->exits;
	    break;
	default:
	    obj = NOTHING;
	    break;
    }
    if (obj == NOTHING)
	return;
    match_exits(obj, md);
}

/*
 * match_room_exits
 * Matches exits and actions attached to player's current room.
 * Formerly 'match_exit'.
 */
void 
match_room_exits(dbref loc, struct match_data * md)
{
    dbref obj;
    switch (Typeof(loc)) {
	case TYPE_PLAYER:
	case TYPE_ROOM:
	case TYPE_THING:
	    obj = DBFETCH(loc)->exits;
	    break;
	default:
	    obj = NOTHING;
	    break;
    }
    if (obj == NOTHING)
	return;
    match_exits(obj, md);
}

/*
 * match_all_exits
 * Matches actions on player, objects in room, objects in inventory,
 * and room actions/exits (in reverse order of priority order).
 */
void 
match_all_exits(struct match_data * md)
{
    dbref   loc;
    int limit = 88;

    strcpy(match_args, "\0");
    strcpy(match_cmdname, "\0");
    if ((loc = DBFETCH(md->match_from)->location) != NOTHING)
	match_room_exits(loc, md);

    if (md->exact_match != NOTHING)
	md->block_equals = 1;
    match_invobj_actions(md);

    if (md->exact_match != NOTHING)
	md->block_equals = 1;
    match_roomobj_actions(md);

    if (md->exact_match != NOTHING)
	md->block_equals = 1;
    match_player_actions(md);

    if (loc == NOTHING)
	return;

    /* if player is in a vehicle, use environment of vehicle's home */
    if (Typeof(loc) == TYPE_THING) {
	loc = DBFETCH(loc)->sp.thing.home;
	if (loc == NOTHING) return;
	if (md->exact_match != NOTHING)
	    md->block_equals = 1;
	match_room_exits(loc, md);
    }

    while ((loc = DBFETCH(loc)->location) != NOTHING) {
	if (md->exact_match != NOTHING)
	    md->block_equals = 1;
	match_room_exits(loc, md);
	if (!limit--) break;
    }
}

void 
match_everything(struct match_data * md)
{
    match_all_exits(md);
    match_neighbor(md);
    match_possession(md);
    match_registered(md);
    if (Mage(OWNER(md->match_from)) || Mage(md->match_who)) {
	match_player(md);
	match_absolute(md);
    }
    match_me(md);
    match_here(md);
}

dbref 
match_result(struct match_data * md)
{
    if (md->exact_match != NOTHING) {
	return (md->exact_match);
    } else {
	switch (md->match_count) {
	    case 0:
		return NOTHING;
	    case 1:
		return (md->last_match);
	    default:
		return AMBIGUOUS;
	}
    }
}

/* use this if you don't care about ambiguity */
dbref 
last_match_result(struct match_data * md)
{
    if (md->exact_match != NOTHING) {
	return (md->exact_match);
    } else {
	return (md->last_match);
    }
}

dbref 
noisy_match_result(struct match_data * md)
{
    dbref   match;

    switch (match = match_result(md)) {
	case NOTHING:
	    anotify(md->match_who, CINFO NOMATCH_MESSAGE);
	    return NOTHING;
	case AMBIGUOUS:
	    anotify(md->match_who, CINFO AMBIGUOUS_MESSAGE);
	    return NOTHING;
	default:
	    return match;
    }
}

void 
match_rmatch(dbref arg1, struct match_data * md)
{
    if (arg1 == NOTHING)
	return;
    switch (Typeof(arg1)) {
	case TYPE_PLAYER:
	case TYPE_ROOM:
	case TYPE_THING:
	    match_list(DBFETCH(arg1)->contents, md);
	    match_exits(DBFETCH(arg1)->exits, md);
	    break;
    }
}

/* Detail matcher reformatted from look.c
   Returns Property name, NOTHING, or AMBIGUOUS
*/

const char *
match_detail(dbref thing, const char *detail) {
    char propname[BUFFER_LEN], buf[BUFFER_LEN];
    PropPtr propadr, pptr, lastmatch;

    strcpy(buf, detail);

#ifdef DISKBASE
    fetchprops(thing);
#endif

    lastmatch = (PropPtr) NOTHING;

    propadr = first_prop(thing, PROP_OBJDIR "/", &pptr, propname);
    while ((propadr > 0) && *propname) {
	if (exit_prefix(propname, buf)) {
	    if(lastmatch != (PropPtr) NOTHING) {
		lastmatch = (PropPtr) AMBIGUOUS;
		break;
	    } else {
		lastmatch = propadr;
	    }
	}
	propadr = next_prop(pptr, propadr, propname);
    }
    propadr = first_prop(thing, PROP_DETDIR "/", &pptr, propname);
    while (MatchOk(propadr) && *propname) {
	if (exit_prefix(propname, buf)) {
	    if(lastmatch != (PropPtr) NOTHING) {
		lastmatch = (PropPtr) AMBIGUOUS;
		break;
	    } else {
		lastmatch = propadr;
	    }
	}
	propadr = next_prop(pptr, propadr, propname);
    }

    if (lastmatch == (PropPtr) AMBIGUOUS) {
	return (const char *) AMBIGUOUS;
    } else if (lastmatch == (PropPtr) NOTHING) {
	return (const char *) NOTHING;
    } else if (MatchOk(lastmatch) && (PropType(lastmatch) == PROP_STRTYP)) {
	return PropName(lastmatch);
    } else
	return (const char *) NOTHING;
}
