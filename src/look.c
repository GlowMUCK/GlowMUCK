#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

/* commands which look at things */

#include <ctype.h>

#include "color.h"
#include "db.h"
#include "tune.h"
#include "mpi.h"
#include "props.h"
#include "interface.h"
#include "match.h"
#include "externs.h"


#define EXEC_SIGNAL '@'		/* Symbol which tells us what we're looking at
				 * is an execution order and not a message.    */

/* prints owner of something */
static void
print_owner(dbref player, dbref thing)
{
    char    buf[BUFFER_LEN];

    switch (Typeof(thing)) {
	case TYPE_PLAYER:
	    sprintf(buf, CGREEN "%s " CINFO "is a player.", NAME(thing));
	    break;
	case TYPE_ROOM:
	case TYPE_THING:
	case TYPE_EXIT:
	case TYPE_PROGRAM:
	    sprintf(buf, CAQUA "Owner: %s", ansi_unparse_object(player, OWNER(thing)));
	    break;
	case TYPE_GARBAGE:
	    sprintf(buf, CGLOOM "%s " CINFO "is garbage.", NAME(thing));
	    break;
    }
    anotify(player, buf);
}

void 
exec_or_notify(dbref player, dbref thing,
	       const char *message, const char *whatcalled)
{
    char   *p;
    char   *p2;
    char   *p3;
    char    buf[BUFFER_LEN];
    char    tmpcmd[BUFFER_LEN];
    char    tmparg[BUFFER_LEN];

#ifdef COMPRESS
    p = (char *) uncompress((char *) message);
#else				/* !COMPRESS */
    p = (char *) message;
#endif				/* COMPRESS */

    if (*p == EXEC_SIGNAL) {
	int     i;

	if (*(++p) == REGISTERED_TOKEN) {
	    strcpy(buf, p);
	    for (p2 = buf; *p && !isspace(*p); p++);
	    if (*p) p++;

	    for (p3 = buf; *p3 && !isspace(*p3); p3++);
	    if (*p3) *p3 = '\0';

	    if (*p2) {
		i = (dbref) find_registered_obj(thing, p2);
	    } else {
		i = 0;
	    }
	} else {
	    i = atoi(p);
	    for (; *p && !isspace(*p); p++);
	    if (*p) p++;
	}
	if ((!OkObj(i)) || (Typeof(i) != TYPE_PROGRAM)) {
	    if (*p) {
		notify(player, p);
	    } else {
		notify(player, NOTHING_MESG);
	    }
	} else {
	    strcpy(tmparg, match_args);
	    strcpy(tmpcmd, match_cmdname);
	    p = do_parse_mesg(player,thing,p,whatcalled,buf,MPI_ISPRIVATE);
	    strcpy(match_args, p);
	    strcpy(match_cmdname, whatcalled);
	    (void) interp(player, DBFETCH(player)->location, i, thing,
			  PREEMPT, STD_HARDUID, 0);
	    strcpy(match_args, tmparg);
	    strcpy(match_cmdname, tmpcmd);
	}
    } else {
	p = do_parse_mesg(player, thing, p, whatcalled, buf, MPI_ISPRIVATE);
	notify(player, p);
    }
}

int
count_details(dbref player, dbref what, const char *propname)
{
    const	char *pname;
    char	buf[BUFFER_LEN];
    char	exbuf[BUFFER_LEN];
    int		count;

    count = 0;

    strcpy(buf, propname);
    if (is_propdir(what, buf)) {
	strcat(buf, "/");
	while ((pname = next_prop_name(what, exbuf, buf))) {
	    strcpy(buf, pname);
	    count++;
	}
    }
    return count;
}

void
look_details(dbref player, dbref what, const char *propname)
{
    const	char *pname;
    char	*tmpchr;
    char	buf[BUFFER_LEN], buf2[BUFFER_LEN];
    char	exbuf[BUFFER_LEN];

    strcpy(buf, propname);
    if (is_propdir(what, buf)) {
	strcat(buf, "/");
	while ( (pname = next_prop_name(what, exbuf, buf)) ) {
	    strcpy(buf, pname);
	    strcpy(buf2, CPURPLE);
	    strcat(buf2, pname + strlen(PROP_OBJDIR "/"));
	    tmpchr = buf2 + strlen(CPURPLE);
	    while( ((*tmpchr) != '\0') )
	    {
		if( ((*tmpchr) == ';') ) {
		    (*tmpchr) = '\0';
		} else
		    tmpchr++;
	    }
	    if(controls(player, what))
		strcat(buf2, CYELLOW "(detail)");
	    anotify(player, buf2);
	}
    }
}

#ifdef MUD
void
look_mud_details(dbref player, dbref what, const char *propname)
{
    const	char *pname;
    char	*tmpchr;
    char	buf[BUFFER_LEN], buf2[BUFFER_LEN];
    char	exbuf[BUFFER_LEN];

    strcpy(buf, propname);
    if (is_propdir(what, buf)) {
	strcat(buf, "/");
	while ( (pname = next_prop_name(what, exbuf, buf)) ) {
	    strcpy(buf, pname);
	    strcpy(buf2, pname + strlen("_obj/"));
	    tmpchr = buf2;
	    while( ((*tmpchr) != '\0') )
	    {
		if( ((*tmpchr) == ';') ) {
		    (*tmpchr) = '\0';
		} else
		    tmpchr++;
	    }
	    anotify_fmt(player, "        " CPURPLE "%s", buf2);
	}
    }
}


static void 
look_mud_contents(dbref player, dbref loc, const char *contents_name)
{
    dbref   thing;
    dbref   can_see_loc;
    int	    saw_something = 0;
    char    buf[BUFFER_LEN];
    const char *name;

    /* check to see if he can see the location */
    can_see_loc = (!Dark(loc) || controls(player, loc));

    /* check to see if there is anything there */
    look_mud_details(player, loc, PROP_OBJDIR);

    if( ( (Typeof(loc) != TYPE_ROOM) || !Dark(loc) ) )
      DOLIST(thing, DBFETCH(loc)->contents) {
	if (can_see(player, thing, can_see_loc)) {
	    /* something exists!  show him everything */
	    saw_something = 1;
	    /* anotify(player, contents_name); */
	    DOLIST(thing, DBFETCH(loc)->contents) {
		if (can_see(player, thing, can_see_loc)) {
		    if(MOB(thing) || (Typeof(thing) == TYPE_PLAYER)) {
			name = GETONAME(thing); /* title */
			if(!name) name = "";
			sprintf(buf, "        " CGREEN "%s " CCYAN "%s",
			    NAME(thing), name);
			anotify(player, buf);
		    } else {
			name = GETONAME(thing);
			if(!name || !*name)
			    name = NAME(thing);
			sprintf(buf, "        " CPURPLE "%s", name);
			anotify(player, buf);
		    }
		}
	    }
	    break;		/* we're done */
	}
    }
}
#endif

static void 
look_contents(dbref player, dbref loc, const char *contents_name)
{
    dbref   thing;
    dbref   can_see_loc;
    int	    saw_something = 0;
    const char *pos;
    char    buf[BUFFER_LEN], buf2[BUFFER_LEN];

#ifdef MUD
    if((Typeof(loc) == TYPE_ROOM) && (FLAGS(loc)&KILL_OK)) {
	look_mud_contents(player, loc, contents_name);
	return;
    }
#endif

    /* check to see if he can see the location */
    can_see_loc = (!Dark(loc) || controls(player, loc));

    /* check to see if there is anything there */
    if(((Typeof(loc) != TYPE_ROOM) || !Dark(loc)))
      DOLIST(thing, DBFETCH(loc)->contents) {
	if (can_see(player, thing, can_see_loc)) {
	    /* something exists!  show him everything */
	    saw_something = 1;
	    anotify(player, contents_name);
	    DOLIST(thing, DBFETCH(loc)->contents) {
		if (can_see(player, thing, can_see_loc)) {
		    pos = GETPOS(thing);
		    if(pos) {
			strcpy(buf, pos);
			buf[60] = '\0';
			pos = tct(buf, buf2);
			sprintf(buf, "%.2048s^BLUE^(%.120s)",
			    ansi_unparse_object(player, thing),
			    pos
			);
			pos = buf;
		    } else
			pos = ansi_unparse_object(player, thing);
		    anotify(player, pos);
		}
	    }
	    break;		/* we're done */
	}
    }
    if(!saw_something) {
	if(count_details(player, loc, PROP_OBJDIR))
	{
	    anotify(player, contents_name);
	    look_details(player, loc, PROP_OBJDIR);
	}
    }
    else
	look_details(player, loc, PROP_OBJDIR);
}

static void 
look_simple(dbref player, dbref thing, const char *name)
{
    if (GETDESC(thing)) {
	exec_or_notify(player, thing, GETDESC(thing), name);
    } else {
	notify(player, NOTHING_MESG);
    }
}

void 
look_room(dbref player, dbref loc, int verbose)
{
    char obj_num[20];
 
    /* tell him the name, and the number if he can link to it */
    anotify(player, ansi_unparse_object(player, loc));

    /* tell him the description */
    if (Typeof(loc) == TYPE_ROOM) {
	if (GETDESC(loc)) {
	    exec_or_notify(player, loc, GETDESC(loc), "(@Desc)");
	}
	/* tell him the appropriate messages if he has the key */
	can_doit(player, loc, 0);
    } else {
	if (GETIDESC(loc)) {
	    exec_or_notify(player, loc, GETIDESC(loc), "(@Idesc)");
	}
    }
    ts_useobject(loc);

    /* tell him the contents */

    if (GETCONTENTS(loc)) {
	exec_or_notify(player, loc, GETCONTENTS(loc), "(@Contents)");
    } else {
	look_contents(player, loc, CINFO "Contents:");
    }
    if (tp_look_propqueues) {
	sprintf(obj_num, "#%d", loc);
	envpropqueue(player,loc,player,loc, NOTHING, "_lookq", obj_num, 1, 1);
    }
}

void 
do_look_around(dbref player)
{
    dbref   loc;

    if ((loc = getloc(player)) == NOTHING)
	return;
    look_room(player, loc, 1);
}

void 
do_look_at(dbref player, const char *name, const char *detail)
{
    dbref   thing, lastthing, loc;
    struct match_data md;
    int nomatch;
    char buf[BUFFER_LEN];
    char obj_num[20];

    loc = getloc(player);

    if (*name == '\0' || !string_compare(name, "here")) {
	if ((thing = getloc(player)) != NOTHING) {
	    look_room(player, thing, 1);
	}
    } else {
#ifdef PATH
	dbref dest;
	char propname[BUFFER_LEN];
#endif /* PATH */

#ifdef DISKBASE
	fetchprops(loc);
#endif

#ifdef PATH
	dest = match_path(loc, name, propname, NULL);
	if(OkObj(dest)) {
	    const char *m;
	    int trans = 0;
	    char tmpcmd[BUFFER_LEN];
	    char tmparg[BUFFER_LEN];

	    m = get_path_prop(loc, propname, "de");
	    if(m && *m && tp_transparent_paths &&
				string_prefix(m, "@$trans")) {
		trans = 1;
		m = strchr(m, ' ');
		if(m && (*m == ' '))
		    m++; /* Move to letter after space */
	    }

	    if(m && *m) { /* Recheck if strchr found a space */
		/* Show path's description */
		exec_or_notify(player, loc, m, "(@Desc)");
		if(trans == 0) return;
	    }

	    /* If transparent_paths are off, show nothing */	    
	    if (!tp_transparent_paths) {
		notify(player, NOTHING_MESG);
		return;
	    }

	    switch(Typeof(dest)) {
		case TYPE_ROOM:
		    look_room(player, dest, 1);
		    return;

		case TYPE_PROGRAM:
		    strcpy(tmpcmd, match_cmdname);
		    strcpy(tmparg, match_args);

		    strcpy(match_cmdname, name);
		    *match_args = 0;
		    interp(player, DBFETCH(player)->location, dest,
		    DBFETCH(player)->location, FOREGROUND, STD_REGUID, 0);

		    strcpy(match_cmdname, tmpcmd);
		    strcpy(match_args, tmparg);
		    return;

		default: /* Check for non-path items */
		    anotify(player, CFAIL NOWAY_MESG);
		    return;
	    }
	}
#endif /* PATH */

	/* look at a thing here */
	init_match(player, name, NOTYPE, &md);
	match_all_exits(&md);
	match_neighbor(&md);
	match_possession(&md);
	/* match_registered(&md); */
	if (Mage(OWNER(player))) {
	    match_absolute(&md);
	    match_player(&md);
	}
	match_here(&md);
	match_me(&md);

	thing = match_result(&md);
	if (thing != NOTHING && thing != AMBIGUOUS && !*detail) {
	    switch (Typeof(thing)) {
		case TYPE_ROOM:
		    if (getloc(player) != thing
			    && !can_link_to(player, TYPE_ROOM, thing)) {
			anotify(player, CFAIL NOPERM_MESG);
		    } else {
			look_room(player, thing, 1);
		    }
		    break;
		case TYPE_PLAYER:
		    if (getloc(player) != getloc(thing)
			    && !controls(player, thing)) {
			anotify(player, CFAIL NOPERM_MESG);
		    } else {
			look_simple(player, thing, name);
			if (GETCONTENTS(thing)) {
			    exec_or_notify(player, thing, GETCONTENTS(thing), "(@Contents)");
			} else {
			    look_contents(player, thing, CINFO "Carrying:");
			}
			if (tp_look_propqueues) {
			    sprintf(obj_num, "#%d", thing);
			    envpropqueue(player, thing, player, thing,
				     NOTHING, "_lookq", obj_num, 1, 1);
			}
		    }
		    break;
		case TYPE_THING:
		    if (getloc(player) != getloc(thing)
			    && getloc(thing) != player
			    && !controls(player, thing)) {
			anotify(player, CFAIL NOPERM_MESG);
		    } else {
			look_simple(player, thing, name);
			if (!(FLAGS(thing) & HAVEN)) {
			    if (GETCONTENTS(thing)) {
				exec_or_notify(player, thing, GETCONTENTS(thing), "(@Contents)");
			    } else {
				look_contents(player, thing, CINFO "Contains:");
			    }
			    ts_useobject(thing);
			}
			if (tp_look_propqueues) {
			    sprintf(obj_num, "#%d", thing);
			    envpropqueue(player, thing, player, thing,
				     NOTHING, "_lookq", obj_num, 1, 1);
			}
		    }
		    break;
		default:
		    look_simple(player, thing, name);
		    if (Typeof(thing) != TYPE_PROGRAM)
			ts_useobject(thing);
		    if (tp_look_propqueues) {
			sprintf(obj_num, "#%d", thing);
			envpropqueue(player, thing, player, thing,
				 NOTHING, "_lookq", obj_num, 1, 1);
		    }
		    break;
	    }
	} else if (thing == NOTHING || (*detail && thing != AMBIGUOUS)) {
	    char propname[BUFFER_LEN];
	    PropPtr propadr, pptr, lastmatch;

	    lastthing = NOTHING;
	    if (thing == NOTHING) {
		nomatch=1;
		thing = player;
		sprintf(buf, "%s", name);
	    } else {
		nomatch=0;
		sprintf(buf, "%s", detail);
	    }

#ifdef DISKBASE
	    fetchprops(thing);
#endif

	    lastmatch = (PropPtr) NOTHING;

	    repeat_match:
	    propadr = first_prop(thing, PROP_OBJDIR "/", &pptr, propname);
	    while (MatchOk(propadr) && *propname) {
		if (exit_prefix(propname, buf)) {
		    if(lastmatch != (PropPtr) NOTHING) {
			lastmatch = (PropPtr) AMBIGUOUS;
			break;
		    } else {
			lastmatch = propadr;
			lastthing = thing;
		    }
		}
		propadr = next_prop(pptr, propadr, propname);
	    }
	    propadr = first_prop(thing, PROP_DETDIR "/", &pptr, propname);
	    while ((propadr > 0) && *propname) {
		if (exit_prefix(propname, buf)) {
		    if(lastmatch != (PropPtr) NOTHING) {
			lastmatch = (PropPtr) AMBIGUOUS;
			break;
		    } else {
			lastmatch = propadr;
			lastthing = thing;
		    }
		}
		propadr = next_prop(pptr, propadr, propname);
	    }
	    if(nomatch) if(thing == player) {
		thing = getloc(player);
		if(thing != player) goto repeat_match;
	    }

	    thing=lastthing;
	    if (lastmatch == (PropPtr) AMBIGUOUS) {
		anotify(player, CINFO AMBIGUOUS_MESSAGE);

	    } else if (MatchOk(lastmatch) && (PropType(lastmatch) == PROP_STRTYP)) {
#ifdef DISKBASE
		propfetch(thing, lastmatch);  /* DISKBASE PROPVALS */
#endif
		exec_or_notify(player, thing,
				PropDataStr(lastmatch), "(@Detail)");
	    } else if (*detail) {
		notify(player, NOTHING_MESG);
	    } else {
		anotify(player, CINFO NOMATCH_MESSAGE);
	    }
	} else {
	    anotify(player, CINFO AMBIGUOUS_MESSAGE);
	}
    }
}

#ifdef VERBOSE_EXAMINE
const char *
flag_description(char *buf, dbref player, dbref thing)
{
    strcpy(buf, CGREEN "Type: " CYELLOW );
    switch (Typeof(thing)) {
	case TYPE_ROOM:
	    strcat(buf, "ROOM");
	    break;
	case TYPE_EXIT:
	    strcat(buf, "EXIT/ACTION");
	    break;
	case TYPE_THING:
	    strcat(buf, "THING");
	    break;
	case TYPE_PLAYER:
	    strcat(buf, "PLAYER");
	    break;
	case TYPE_PROGRAM:
	    strcat(buf, "PROGRAM");
	    break;
	case TYPE_GARBAGE:
	    strcat(buf, "GARBAGE");
	    break;
	default:
	    strcat(buf, "**UNKNOWN**");
	    break;
    }

    if (FLAGS(thing) & ~TYPE_MASK) {
	/* print flags */
	strcat(buf, CGREEN "  Flags:" CYELLOW );

	if(!tp_multi_wiz_levels) {
	    if( TMan(thing) || ((FLAGS(thing) & W3) && TMan(OWNER(thing))) )
		strcat(buf, " " NAMEFMAN);
	    if( FLAGS(thing) & W3 )
		strcat(buf, " " NAMEFWIZ);
	}

	if(tp_multi_wiz_levels) switch( RawMLevel(thing) ) {
	    case LBOY:	strcat(buf, " " NAMEFMAN ); break;
	    case LARCH:	strcat(buf, " " NAMEFARCH); break;
	    case LWIZ:	strcat(buf, " " NAMEFWIZ ); break;
	    case LMAGE:	strcat(buf, " " NAMEFMAGE); break;
	}

	switch( RawMLevel(thing) ) {
	    case LM3:	strcat(buf, " MUFFER3"); break;
	    case LM2:	strcat(buf, " MUFFER2"); break;
	    case LM1:	strcat(buf, " MUFFER"); break;
	}

	if (FLAG2(thing) & F2MPI)
	    strcat(buf, " MEEPER");
	if (FLAGS(thing) & QUELL)
	    strcat(buf, " QUELL");
	if (FLAGS(thing) & STICKY)
	    strcat(buf, (Typeof(thing) == TYPE_PROGRAM) ? " SETUID" :
		   (Typeof(thing) == TYPE_PLAYER) ? " SILENT" : " STICKY");
	if (FLAGS(thing) & DARK)
	    strcat(buf, (Typeof(thing) == TYPE_PROGRAM) ? " DEBUGGING" :
		   (Typeof(thing) == TYPE_PLAYER) ? " DEBUG_SPAM" : " DARK");
	if (FLAGS(thing) & LINK_OK)
	    strcat(buf, " LINK_OK");

	if (FLAGS(thing) & KILL_OK)
	    strcat(buf, " KILL_OK");

	if (FLAGS(thing) & BUILDER)
	    strcat(buf, (Typeof(thing) != TYPE_PROGRAM) ? " BUILDER" : " BOUND");
	if (FLAGS(thing) & CHOWN_OK)
	    strcat(buf,  (Typeof(thing) == TYPE_PROGRAM) ? " COUNT_INSTS" :
		   (Typeof(thing) == TYPE_PLAYER) ? " COLOR_ANSI" : " CHOWN_OK");
	if (FLAGS(thing) & JUMP_OK)
	    strcat(buf, " JUMP_OK");
	if (FLAGS(thing) & VEHICLE)
	    strcat(buf, (Typeof(thing) != TYPE_PROGRAM) ? " VEHICLE" : " VIEWABLE");
	if (FLAGS(thing) & XFORCIBLE)
	    strcat(buf, " XFORCIBLE");
	if (FLAGS(thing) & ZOMBIE)
	    strcat(buf, (Typeof(thing) != TYPE_PROGRAM) ? " ZOMBIE" : " INTERACTIVE_DEBUG");
	if (FLAGS(thing) & HAVEN)
	    strcat(buf, (Typeof(thing) != TYPE_PROGRAM) ? " HAVEN" : " HARDUID");
	if (FLAGS(thing) & ABODE)
	    strcat(buf, (Typeof(thing) != TYPE_PROGRAM) ?
	       (Typeof(thing) != TYPE_EXIT ? " ABODE" : " ABATE") : " AUTOSTART");
	if (FLAG2(thing) & F2TINKERPROOF)
	    strcat(buf, (Typeof(thing) == TYPE_PLAYER) ? " TINKERPROOF" : " TINKERABLE");
	if (FLAG2(thing) & F2GUEST)
	    strcat(buf, " GUEST");
	if (FLAG2(thing) & F2IC)
	    strcat(buf, " IN_CHARACTER");
	if (FLAG2(thing) & F2IDLE)
	    strcat(buf, (Typeof(thing) != TYPE_PROGRAM) ? " IDLE" : " STAY_IN_MEMORY");
	if (FLAG2(thing) & F2PUEBLO)
	    strcat(buf, " PUEBLO");
	if (FLAG2(thing) & F2OFFER)
	    strcat(buf, (Typeof(thing) == TYPE_PLAYER) ? " TASK_PENDING" : ((Typeof(thing) == TYPE_ROOM) ? " OFFER" : " OFFER_TASK") );
	if (FLAG2(thing) & F2WWW)
	    strcat(buf, " WWW_OK");
	if(Boy(OWNER(player))) {
	    if(FLAG2(thing) & F2LOGWALL)
		strcat(buf, " LOGWALL");
	    if(FLAG2(thing) & F2SUSPECT)
		strcat(buf, " SUSPECT");
	}
    }
    return buf;
}

#endif				/* VERBOSE_EXAMINE */

#ifdef PATH
int list_path_props(dbref player, dbref loc) {
    PropPtr propadr, pptr;
    int cnt = 0;
    dbref dest;
    char propname[BUFFER_LEN], buf[BUFFER_LEN];
    char pathdir[80];

    if((!OkObj(loc)) || !valid_path_dir())
	return 0;

    strcpy(pathdir, tp_path_dir);
    strcat(pathdir, "/");

    propadr = first_prop(loc, pathdir, &pptr, propname);
    while ((propadr > 0) && *propname) {
	dest = get_path_dest(loc, propname);
	if(OkObj(dest)) {
	    if(cnt == 0)
		anotify(player, CINFO "Paths:");

	    anotify_fmt(player, CBLUE "%s " CCYAN "to %s",
		full_paths(propname, buf), ansi_unparse_object(player, dest)
	    );
	    cnt++;
	}
	propadr = next_prop(pptr, propadr, propname);
    }
    return cnt;

}
#endif /* PATH */

int
listprops_wildcard(dbref player, dbref thing, const char *dir, const char *wild)
{
    char propname[BUFFER_LEN];
    char wld[BUFFER_LEN];
    char buf[BUFFER_LEN];
    char buf2[BUFFER_LEN];
    char *ptr, *wldcrd = wld;
    PropPtr propadr, pptr;
    int i, cnt = 0;
    int recurse = 0;

    strcpy(wld, wild);
    i = strlen(wld);
    if (i && wld[i-1] == PROPDIR_DELIMITER)
	strcat(wld, "*");
    for (wldcrd = wld; *wldcrd == PROPDIR_DELIMITER; wldcrd++);
    if (!strcmp(wldcrd, "**")) recurse = 1;

    for (ptr = wldcrd; *ptr && *ptr != PROPDIR_DELIMITER; ptr++);
    if (*ptr) *ptr++ = '\0';

    propadr = first_prop(thing, (char *)dir, &pptr, propname);
    while ((propadr > 0) && *propname) {
	if (equalstr(wldcrd, propname)) {
	    sprintf(buf, "%s%c%s", dir, PROPDIR_DELIMITER, propname);
	    if ((!Prop_Hidden(buf) && !(PropFlags(propadr) & PROP_SYSPERMS))
		    || (QLevel(OWNER(player)) >= tp_hidden_prop_mlevel)) {
		if (!*ptr || recurse) {
		    cnt++;
		    displayprop(player, thing, buf, buf2);
		    anotify(player, buf2);
		} 
		if (recurse) ptr = "**";
		cnt += listprops_wildcard(player, thing, buf, ptr);
	    }
	}
	propadr = next_prop(pptr, propadr, propname);
    }
    return cnt;
}


int 
size_object(dbref i, int load)
{
    int byts;
    byts = sizeof(struct object);
    if (NAME(i)) {
	byts += strlen(NAME(i)) + 1;
    }
    byts += size_properties(i, load);

    if (Typeof(i) == TYPE_EXIT && DBFETCH(i)->sp.exit.dest) {
	byts += sizeof(dbref) * DBFETCH(i)->sp.exit.ndest;
    } else if (Typeof(i) == TYPE_PLAYER && DBFETCH(i)->sp.player.password) {
	byts += strlen(DBFETCH(i)->sp.player.password) + 1;
    } else if (Typeof(i) == TYPE_PROGRAM) {
	byts += size_prog(i);
    }
    return byts;
}


void 
do_check(dbref player, const char *name)
{
    dbref   thing;
    char    buf[BUFFER_LEN], *why = "the force is not with you";
    struct match_data md;
    int     perm;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (*name == '\0') {
	if ((thing = getloc(player)) == NOTHING)
	    return;
    } else {
	/* look it up */
	init_match(player, name, NOTYPE, &md);

	match_all_exits(&md);
	match_neighbor(&md);
	match_possession(&md);
	match_absolute(&md);
	match_registered(&md);

	/* only Wizards can examine other players */
	if (Mage(OWNER(player)))
	    match_player(&md);

	match_here(&md);
	match_me(&md);

	/* get result */
	if ((thing = noisy_match_result(&md)) == NOTHING)
	    return;
    }

    perm = why_controls(player, thing, 0);
    if(perm > 0) {
	switch(perm) {
	    case 1: why = "you own it"; break;
	    case 2: why = "you are " NAMEMAN; break;
	    case 3: why = "you are a " NAMEWIZ; break;
	    case 4: why = "you own its realm"; break;
	    case 5: why = "the owner allows it"; break;
	    case 6: why = "the owner is a guest"; break;
	}
    } else {
	switch(-perm) {
	    case 0: why = "you don't own it"; break;
	    case 1: why = "dbref is invalid"; break;
	    case 2: why = "dbref is garbage"; break;
	    case 3: why = "you are set QUELL"; break;
	    case 4: why = "you have no power over it"; break;
	    case 5: why = "the owner is set TINKERPROOF"; break;
	    case 6: why = "a " NAMEWIZ " owns it"; break;
	    case 7: why = "it isn't a room, exit, or thing"; break;
	}
    }

    if(perm > 0)
	sprintf(buf, CSUCC "You control it because %s.", why);
    else
	sprintf(buf, CFAIL "You don't control it because %s.", why);

    anotify(player, buf);
}


void 
do_examine(dbref player, const char *name, const char *dir, int when)
{
    dbref   thing;
    char    buf[BUFFER_LEN];
    char    buf2[BUFFER_LEN];
    dbref   content;
    dbref   exit;
    int     i, cnt;
    struct match_data md;
    struct tm *time_tm;		/* used for timestamps */

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((thing = getloc(player)) == NOTHING)
	return;

    if (*name != '\0') {

#ifdef PATH
	if( OkObj(thing) &&
	    (Typeof(thing) == TYPE_ROOM) &&
	    controls(player, thing)
	) {
	    dbref dest;

	    /* Match paths in room */
	    if((dest = match_path(thing, name, buf, NULL)) != NOTHING)
		examine_path(player, thing, buf);

	    if(dest != NOTHING) return;
	}
#endif
	thing = NOTHING;

	/* look it up */
	init_match(player, name, NOTYPE, &md);

	match_all_exits(&md);
	match_neighbor(&md);
	match_possession(&md);
	match_absolute(&md);
	match_registered(&md);

	/* only Wizards can examine other players */
	if (Mage(OWNER(player)))
	    match_player(&md);

	match_here(&md);
	match_me(&md);

	/* get result */
	if ((thing = noisy_match_result(&md)) == NOTHING)
	    return;
    }

    if ( !can_link(player, thing) && !Mage(OWNER(player)) ) {
	print_owner(player, thing);
	return;
    }
    if (*dir && !when) {
	/* show him the properties */
	cnt = listprops_wildcard(player, thing, "", dir);
	sprintf(buf, CINFO "%d propert%s listed.", cnt, (cnt == 1 ? "y" : "ies"));
	anotify(player, buf);
	return;
    }
    switch (Typeof(thing)) {
	case TYPE_EXIT:
	case TYPE_PROGRAM:
	case TYPE_ROOM:
	    sprintf(buf, "%s  " CAQUA "Owner: ",
		ansi_unparse_object(player, thing));
	    strcat(buf, ansi_unparse_object(player, OWNER(thing)));
	    break;
	case TYPE_THING:
	    sprintf(buf, "%s  " CBROWN "Value: " CYELLOW "%d  " CAQUA "Owner: ",
		ansi_unparse_object(player, thing),
		DBFETCH(thing)->sp.thing.value);
	    strcat(buf, ansi_unparse_object(player, OWNER(thing)));
	    break;
	case TYPE_PLAYER:
	    sprintf(buf, "%s  " CBROWN "%s: " CYELLOW "%d",
		ansi_unparse_object(player, thing),
		tp_cpennies, DBFETCH(thing)->sp.player.pennies);
	    break;
	case TYPE_GARBAGE:
	    strcpy(buf, ansi_unparse_object(player, thing));
	    break;
    }
    anotify(player, buf);

#ifdef VERBOSE_EXAMINE
    anotify(player, flag_description(buf, player, thing));
#endif				/* VERBOSE_EXAMINE */

    if(!when) {
	if (GETDESC(thing))
	    notify(player, GETDESC(thing));

	sprintf(buf, CVIOLET "Key:" CPURPLE " %s", unparse_boolexp(player, GETLOCK(thing), 1));
	anotify(player, buf);

	sprintf(buf, CVIOLET "Chown_OK Key:" CPURPLE " %s",
		unparse_boolexp(player, get_property_lock(thing, "_/chlk"), 1) );
	anotify(player, buf);

	sprintf(buf, CVIOLET "Container Key:" CPURPLE " %s",
		unparse_boolexp(player, get_property_lock(thing, "_/clk"), 1) );
	anotify(player, buf);

	sprintf(buf, CVIOLET "Force Key:" CPURPLE " %s",
		unparse_boolexp(player, get_property_lock(thing, "@/flk"), 1) );
	anotify(player, buf);

	if (GETIDESC(thing)) {
	    anotify(player, CAQUA "IDesc:");
	    notify(player, GETIDESC(thing));
	}
	if(tp_gender_commands) {
	    if (GETSEX(thing)) {
		sprintf(buf, CAQUA "Gender:" CCYAN " %s", tct(GETSEX(thing),buf2));
		anotify(player, buf);
	    }
	    if (GETSPECIES(thing)) {
		sprintf(buf, CAQUA "Species:" CCYAN " %s", tct(GETSPECIES(thing),buf2));
		anotify(player, buf);
	    }
	    if (GETPOS(thing)) {
		sprintf(buf, CAQUA "Position:" CCYAN " %s", tct(GETPOS(thing),buf2));
		anotify(player, buf);
	    }
	}
	if (GETSUCC(thing)) {
	    sprintf(buf, CAQUA "Succ:" CCYAN " %s", tct(GETSUCC(thing),buf2));
	    anotify(player, buf);
	}
	if (GETFAIL(thing)) {
	    sprintf(buf, CAQUA "Fail:" CCYAN " %s", tct(GETFAIL(thing),buf2));
	    anotify(player, buf);
	}
	if (GETDROP(thing)) {
	    sprintf(buf, CAQUA "Drop:" CCYAN " %s", tct(GETDROP(thing),buf2));
	    anotify(player, buf);
	}
#ifdef MUD
	if (GETONAME(thing)) {
	    sprintf(buf, CAQUA "OName:" CCYAN " %s", tct(GETONAME(thing),buf2));
	    anotify(player, buf);
	}
#endif
	if (GETOSUCC(thing)) {
	    sprintf(buf, CAQUA "OSucc:" CCYAN " %s", tct(GETOSUCC(thing),buf2));
	    anotify(player, buf);
	}
	if (GETOFAIL(thing)) {
	    sprintf(buf, CAQUA "OFail:" CCYAN " %s", tct(GETOFAIL(thing),buf2));
	    anotify(player, buf);
	}
	if (GETODROP(thing)) {
	    sprintf(buf, CAQUA "ODrop:" CCYAN " %s", tct(GETODROP(thing),buf2));
	    anotify(player, buf);
	}

	if (tp_who_doing && GETDOING(thing)) {
	    sprintf(buf, CAQUA "Doing:" CCYAN " %s", tct(GETDOING(thing),buf2));
	    anotify(player, buf);
	}
	if (GETOECHO(thing)) {
	    sprintf(buf, CAQUA "Oecho:" CCYAN " %s", tct(GETOECHO(thing),buf2));
	    anotify(player, buf);
	}
	if (GETPECHO(thing)) {
	    sprintf(buf, CAQUA "Pecho:" CCYAN " %s", tct(GETPECHO(thing),buf2));
	    anotify(player, buf);
	}
    } /* !when */

    /* Timestamps */
    /* ex: time_tm = localtime((time_t *)(&(DBFETCH(thing)->ts.created))); */

    time_tm = localtime((&(DBFETCH(thing)->ts.created)));
    (void) format_time(buf, BUFFER_LEN,
		       (char *) CFOREST "Created:" CGREEN "  %a %b %e %T %Z %Y\0", time_tm);
    anotify(player, buf);
    time_tm = localtime((&(DBFETCH(thing)->ts.modified)));
    (void) format_time(buf, BUFFER_LEN,
		       (char *) CFOREST "Modified:" CGREEN " %a %b %e %T %Z %Y\0", time_tm);
    anotify(player, buf);
    time_tm = localtime((&(DBFETCH(thing)->ts.lastused)));
    (void) format_time(buf, BUFFER_LEN,
		       (char *) CFOREST "Lastused:" CGREEN " %a %b %e %T %Z %Y\0", time_tm);
    anotify(player, buf);
    if (Typeof(thing) == TYPE_PROGRAM) {
	sprintf(buf, CFOREST "Usecount:" CGREEN " %d     "
		     CFOREST "Instances:" CGREEN " %d",
	       DBFETCH(thing)->ts.usecount,
	       DBFETCH(thing)->sp.program.instances);
    } else {
	sprintf(buf, CFOREST "Usecount:" CGREEN " %d", DBFETCH(thing)->ts.usecount);
    }
    anotify(player, buf);

    if(when) /* That's all we want for @when */
	return;

    sprintf(buf, CVIOLET "In Memory:" CPURPLE " %d bytes", size_object(thing, 0));
    anotify(player, buf);

#ifdef MUD
    if (Mage(OWNER(player))) stats(player, thing);
#endif

    /* anotify(player, CINFO "[ Use 'examine <object>=/' to list root properties. ]"); */

    switch (Typeof(thing)) {
	case TYPE_ROOM:
	    /* print parent room */
	    sprintf(buf, CAQUA "Parent: %s", ansi_unparse_object(player,
		DBFETCH(thing)->location));
	    anotify(player, buf);

	    /* print dropto if present */
	    if (DBFETCH(thing)->sp.room.dropto != NOTHING) {
		sprintf(buf, CAQUA "Dropped things go to: %s",
		     ansi_unparse_object(player, DBFETCH(thing)->sp.room.dropto));
		anotify(player, buf);
	    }

	    /* tell him about exits */
	    if (DBFETCH(thing)->exits != NOTHING) {
		anotify(player, CINFO "Exits:");
		DOLIST(exit, DBFETCH(thing)->exits) {
		    strcpy(buf, ansi_unparse_object(player, exit));
		    anotify_fmt(player, "%s " CCYAN "to %s", buf,
			ansi_unparse_object(player,
			    DBFETCH(exit)->sp.exit.ndest > 0
			    ? DBFETCH(exit)->sp.exit.dest[0] : NOTHING
			)
		    );
		}
	    } else anotify(player, CINFO "No exits.");

#ifdef PATH
	    /* list paths */
	    if(!list_path_props(player, thing))
	        anotify(player, CINFO "No paths.");
#endif
	    break;
	case TYPE_THING:
	    /* print home */
	    sprintf(buf, CAQUA "Home: %s",
		    ansi_unparse_object(player, DBFETCH(thing)->sp.thing.home));	/* home */
	    anotify(player, buf);
	    /* print location if player can link to it */
	    if (DBFETCH(thing)->location != NOTHING && (controls(player, DBFETCH(thing)->location)
		 || can_link_to(player, NOTYPE, DBFETCH(thing)->location))) {
		sprintf(buf, CAQUA "Location: %s",
			ansi_unparse_object(player, DBFETCH(thing)->location));
		anotify(player, buf);
	    }
	    /* print thing's actions, if any */
	    if (DBFETCH(thing)->exits != NOTHING) {
		anotify(player, CINFO "Actions:");
		DOLIST(exit, DBFETCH(thing)->exits) {
		    strcpy(buf, ansi_unparse_object(player, exit));
		    anotify_fmt(player, "%s " CCYAN "to %s", buf,
			ansi_unparse_object(player,
			    DBFETCH(exit)->sp.exit.ndest > 0
			    ? DBFETCH(exit)->sp.exit.dest[0] : NOTHING
			)
		    );
		}
	    } else anotify(player, CINFO "No actions attached.");
	    break;
	case TYPE_PLAYER:

	    /* print home */
	    sprintf(buf, CAQUA "Home: %s",
		    ansi_unparse_object(player, DBFETCH(thing)->sp.player.home));	/* home */
	    anotify(player, buf);

	    /* print location if player can link to it */
	    if (DBFETCH(thing)->location != NOTHING && (controls(player, DBFETCH(thing)->location)
		 || can_link_to(player, NOTYPE, DBFETCH(thing)->location))) {
		sprintf(buf, CAQUA "Location: %s",
			ansi_unparse_object(player, DBFETCH(thing)->location));
		anotify(player, buf);
	    }
	    /* print player's actions, if any */
	    if (DBFETCH(thing)->exits != NOTHING) {
		anotify(player, CINFO "Actions:");
		DOLIST(exit, DBFETCH(thing)->exits) {
		    strcpy(buf, ansi_unparse_object(player, exit));
		    anotify_fmt(player, "%s " CCYAN "to %s", buf,
			ansi_unparse_object(player,
			    DBFETCH(exit)->sp.exit.ndest > 0
			    ? DBFETCH(exit)->sp.exit.dest[0] : NOTHING
			)
		    );
		}
	    } else anotify(player, CINFO "No actions attached.");
#if 0
	    if (Boy(player)) {
		notify_fmt(player, "Ansi: %p  Ignore: %d %p",
		    DBFETCH(thing)->sp.player.ansi,
		    DBFETCH(thing)->sp.player.ignores,
		    DBFETCH(thing)->sp.player.ignoring
		);
	    }
#endif
	    break;
	case TYPE_EXIT:
	    if (DBFETCH(thing)->location != NOTHING) {
		sprintf(buf, CAQUA "Source: %s", ansi_unparse_object(player, DBFETCH(thing)->location));
		anotify(player, buf);
	    }
	    /* print destinations */
	    if (DBFETCH(thing)->sp.exit.ndest == 0)
		break;
	    for (i = 0; i < DBFETCH(thing)->sp.exit.ndest; i++) {
		switch ((DBFETCH(thing)->sp.exit.dest)[i]) {
		    case NOTHING:
			break;
		    case HOME:
			anotify(player, CAQUA "Destination: *HOME*");
			break;
		    default:
			sprintf(buf, CAQUA "Destination: %s",
				ansi_unparse_object(player, (DBFETCH(thing)->sp.exit.dest)[i]));
			anotify(player, buf);
			break;
		}
	    }
	    break;
	case TYPE_PROGRAM:
	    if (DBFETCH(thing)->sp.program.siz) {
		struct inst *first = DBFETCH(thing)->sp.program.code;
		sprintf(buf, CVIOLET "Program compiled size:" CPURPLE " %d instructions", DBFETCH(thing)->sp.program.siz);
		anotify(player, buf);
		sprintf(buf, CVIOLET "Cummulative runtime:" CPURPLE " %d.%06d seconds", first->data.number, first[1].data.number);
		anotify(player, buf);
	    } else {
		anotify(player, CVIOLET "Program not compiled.");
	    }

	    /* print location if player can link to it */
	    if (DBFETCH(thing)->location != NOTHING && (controls(player, DBFETCH(thing)->location)
		 || can_link_to(player, NOTYPE, DBFETCH(thing)->location))) {
		sprintf(buf, CAQUA "Location: %s", ansi_unparse_object(player, DBFETCH(thing)->location));
		anotify(player, buf);
	    }
	    break;
	default:
	    /* do nothing */
	    break;
    }

    /* show the contents */
    if (DBFETCH(thing)->contents != NOTHING) {
	if (Typeof(thing) == TYPE_PLAYER)
	    anotify(player, CYELLOW "Carrying:");
	else
	    anotify(player, CYELLOW "Contents:");
	DOLIST(content, DBFETCH(thing)->contents) {
	    anotify(player, ansi_unparse_object(player, content));
	}
    }
}


void 
do_score(dbref player, int domud)
{
    char    buf[BUFFER_LEN];
#ifdef MUD
    dbref loc = getloc(player);

    if((loc != NOTHING) && (FLAGS(loc) & KILL_OK) && KILLER(player) && domud) {
	stats(player, player);
    } else
#endif
    {
	sprintf(buf, CINFO "You have %d %s.", DBFETCH(player)->sp.player.pennies,
	    DBFETCH(player)->sp.player.pennies == 1 ? tp_penny : tp_pennies);
	anotify(player, buf);
    }
}

void 
do_inventory(dbref player)
{
    dbref   thing;

    if ((thing = DBFETCH(player)->contents) == NOTHING &&
	  !count_details(player, player, PROP_OBJDIR) ) {
	anotify(player, CYELLOW "You aren't carrying anything.");
    } else {
	anotify(player, CYELLOW "You are carrying:");
	if(thing != NOTHING ) { DOLIST(thing, thing) {
	    anotify(player, ansi_unparse_object(player, thing));
	} }
	look_details(player, player, PROP_OBJDIR);
    }

    do_score(player, 0);
}

extern const char *uppercase;

#define UPCASE(x) (uppercase[x])

struct flgchkdat {
    int     fortype;		/* check FOR a type? */
    int     istype;		/* If check FOR a type, which one? */
    int     isnotroom;		/* not a room. */
    int     isnotexit;		/* not an exit. */
    int     isnotthing;		/* not type thing */
    int     isnotplayer;	/* not a player */
    int     isnotprog;		/* not a program */
    int     forlevel;		/* check for a mucker level? */
    int     islevel;		/* if check FOR a mucker level, which level? */
    int     isnotzero;		/* not ML0 */
    int     isnotone;		/* not ML1 */
    int     isnottwo;		/* not ML2 */
    int     isnotthree;		/* not ML3 */
    int     isnotfour;		/* not ML4 */
    int     isnotfive;		/* not ML5 */
    int     isnotsix;		/* not ML6 */
    int     isnotseven;		/* not ML7 */
    int     isnoteight;		/* not ML8 */
    int     isnotnine;		/* not ML9 */
    int     isnotwiz;		/* not ML5+ */
    int     iswiz;		/* is  ML5+ */
    int     setflags;		/* flags that are set to check for */
    int     clearflags;		/* flags to check are cleared. */
    int     setflag2;		/* flags that are set to check for */
    int     clearflag2;		/* flags to check are cleared. */
    int     forlink;		/* check linking? */
    int     islinked;		/* if yes, check if not unlinked */
    int     forold; 		/* check for old object? */
    int     isold;   		/* if yes, check if old */
    int     loadedsize;	 	/* check for propval-loaded size? */
    int     issize;	     	/* list objs larger than size? */
    int     size;	       	/* what size to check against. No check if 0 */
};


int 
init_checkflags(dbref player, const char *flags, struct flgchkdat *check)
{
    char    buf[BUFFER_LEN];
    char   *cptr;
    int     output_type = 0;
    int     mode = 0;

    strcpy(buf, flags);
    for (cptr = buf; *cptr && (*cptr != '='); cptr++);
    if (*cptr == '=')
	*(cptr++) = '\0';
    flags = buf;
    while (*cptr && isspace(*cptr))
	cptr++;

    if (!*cptr) {
	output_type = 0;
    } else if (string_prefix("owners", cptr)) {
	output_type = 1;
    } else if (string_prefix("locations", cptr)) {
	output_type = 3;
    } else if (string_prefix("links", cptr)) {
	output_type = 2;
    } else if (string_prefix("count", cptr)) {
	output_type = 4;
    } else if (string_prefix("size", cptr)) {
	output_type = 5;
    } else {
	output_type = 0;
    }

    check->fortype = 0;
    check->istype = 0;
    check->isnotroom = 0;
    check->isnotexit = 0;
    check->isnotthing = 0;
    check->isnotplayer = 0;
    check->isnotprog = 0;
    check->setflags = 0;
    check->clearflags = 0;
    check->setflag2 = 0;
    check->clearflag2 = 0;

    check->forlevel = 0;
    check->islevel = 0;
    check->isnotzero = 0;
    check->isnotone = 0;
    check->isnottwo = 0;
    check->isnotthree = 0;
    check->isnotfour = 0;
    check->isnotfive = 0;
    check->isnotsix = 0;
    check->isnotseven = 0;
    check->isnoteight = 0;
    check->isnotnine = 0;
    check->isnotwiz = 0;
    check->iswiz = 0;

    check->forlink = 0;
    check->islinked = 0;
    check->forold = 0;
    check->isold = 0;

    check->loadedsize = 0;
    check->issize = 0;
    check->size = 0;

    while (*flags) {
	switch (UPCASE((int)*flags)) {
	    case '!':
		if (mode)
		    mode = 0;
		else
		    mode = 2;
		break;
	    case 'R':
		if (mode) {
		    check->isnotroom = 1;
		} else {
		    check->fortype = 1;
		    check->istype = TYPE_ROOM;
		}
		break;
	    case 'T':
		if (mode) {
		    check->isnotthing = 1;
		} else {
		    check->fortype = 1;
		    check->istype = TYPE_THING;
		}
		break;
	    case 'E':
		if (mode) {
		    check->isnotexit = 1;
		} else {
		    check->fortype = 1;
		    check->istype = TYPE_EXIT;
		}
		break;
	    case 'P':
		if (mode) {
		    check->isnotplayer = 1;
		} else {
		    check->fortype = 1;
		    check->istype = TYPE_PLAYER;
		}
		break;
	    case 'F':
		if(!Mucker(OWNER(player))) break;
		if (mode) {
		    check->isnotprog = 1;
		} else {
		    check->fortype = 1;
		    check->istype = TYPE_PROGRAM;
		}
		break;
	    case '~':
	    case '^':
		check->loadedsize = (Mage(OWNER(player)) && *flags == '^');
		check->size = atoi(flags+1);
	    	check->issize = !mode;
	    	while (isdigit(flags[1])) flags++;
		break;
	    case '%':
		check->forlink = 1;
		if (mode) {
		    check->islinked = 1;
		} else {
		    check->islinked = 0;
		}
		break;
	    case '@':
		check->forold = 1;
		if (mode) {
		    check->isold = 0;
		} else {
		    check->isold = 1;
		}
		break;
	    case '0':
		if (mode) {
		    check->isnotzero = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 0;
		}
		break;
	    case '1':
		if(!Mucker1(OWNER(player))) break;
		if (mode) {
		    check->isnotone = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 1;
		}
		break;
	    case '2':
		if(!Mucker2(OWNER(player))) break;
		if (mode) {
		    check->isnottwo = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 2;
		}
		break;
	    case '3':
		if(!Mucker3(OWNER(player))) break;
		if (mode) {
		    check->isnotthree = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 3;
		}
		break;
	    case '4':
		if(!tp_multi_wiz_levels) break;
		if(!Mage(OWNER(player))) break;
		if (mode) {
		    check->isnotfour = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 4;
		}
		break;
	    case '5':
		if(!tp_multi_wiz_levels) break;
		if(!Mage(OWNER(player))) break;
		if (mode) {
		    check->isnotfive = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 5;
		}
		break;
	    case '6':
		if(!tp_multi_wiz_levels) break;
		if(!Mage(OWNER(player))) break;
		if (mode) {
		    check->isnotsix = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 6;
		}
		break;
	    case '7':
		if(!tp_multi_wiz_levels) break;
		if(!Mage(OWNER(player))) break;
		if (mode) {
		    check->isnotseven = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 7;
		}
		break;
	    case '8':
		if(!tp_multi_wiz_levels) break;
		if(!Mage(OWNER(player))) break;
		if (mode) {
		    check->isnoteight = 1;
		} else {
		    check->forlevel = 1;
		    check->islevel = 8;
		}
		break;
	    case 'M':
		if(!Meeper(OWNER(player))) break;
		if (mode)
		    check->clearflag2 |= F2MPI;
		else
		    check->setflag2 |= F2MPI;
		break;
	    case 'A':
		if (mode)
		    check->clearflags |= ABODE;
		else
		    check->setflags |= ABODE;
		break;
	    case 'B':
		if (mode)
		    check->clearflags |= BUILDER;
		else
		    check->setflags |= BUILDER;
		break;
	    case 'C':
		if (mode)
		    check->clearflags |= CHOWN_OK;
		else
		    check->setflags |= CHOWN_OK;
		break;
	    case 'D':
		if (mode)
		    check->clearflags |= DARK;
		else
		    check->setflags |= DARK;
		break;
	    case 'H':
		if (mode)
		    check->clearflags |= HAVEN;
		else
		    check->setflags |= HAVEN;
		break;
	    case 'J':
		if (mode)
		    check->clearflags |= JUMP_OK;
		else
		    check->setflags |= JUMP_OK;
		break;
	    case 'K':
		if (mode)
		    check->clearflags |= KILL_OK;
		else
		    check->setflags |= KILL_OK;
		break;
	    case 'L':
		if (mode)
		    check->clearflags |= LINK_OK;
		else
		    check->setflags |= LINK_OK;
		break;
	    case 'Q':
		if (mode)
		    check->clearflags |= QUELL;
		else
		    check->setflags |= QUELL;
		break;
	    case 'S':
		if (mode)
		    check->clearflags |= STICKY;
		else
		    check->setflags |= STICKY;
		break;
	    case 'V':
		if (mode)
		    check->clearflags |= VEHICLE;
		else
		    check->setflags |= VEHICLE;
		break;
	    case 'X':
		if (mode)
		    check->clearflags |= XFORCIBLE;
		else
		    check->setflags |= XFORCIBLE;
		break;
	    case 'Z':
		if (mode)
		    check->clearflags |= ZOMBIE;
		else
		    check->setflags |= ZOMBIE;
		break;
	    case 'W':
		if(!Mage(OWNER(player))) break;
		if (mode)
		    check->isnotwiz = 1;
		else
		    check->iswiz = 1;
		break;
	    case 'G':
		if (mode)
		    check->clearflag2 |= F2GUEST;
		else
		    check->setflag2 |= F2GUEST;
		break;
	    case 'O':
		if (mode)
		    check->clearflag2 |= F2OFFER;
		else
		    check->setflag2 |= F2OFFER;
		break;
	    case 'I':
		if (mode)
		    check->clearflag2 |= F2IC;
		else
		    check->setflag2 |= F2IC;
		break;
	    case 'N':
		if (mode)
		    check->clearflag2 |= F2TINKERPROOF;
		else
		    check->setflag2 |= F2TINKERPROOF;
		break;
	    case 'U':
		if (mode)
		    check->clearflag2 |= F2PUEBLO;
		else
		    check->setflag2 |= F2PUEBLO;
		break;
	    case '|':
		if (mode)
		    check->clearflag2 |= F2IC;
		else
		    check->setflag2 |= F2IC;
		break;
	    case '$':
		if(!Boy(OWNER(player))) break;
		if (mode)
		    check->clearflag2 |= F2LOGWALL;
		else
		    check->setflag2 |= F2LOGWALL;
		break;
	    case '&':
		if (mode)
		    check->clearflag2 |= F2WWW;
		else
		    check->setflag2 |= F2WWW;
		break;
	    case '#':
		if(!Boy(OWNER(player))) break;
		if (mode)
		    check->clearflag2 |= F2SUSPECT;
		else
		    check->setflag2 |= F2SUSPECT;
		break;
	    case ' ':
		if (mode)
		    mode = 2;
		break;
	}
	if (mode)
	    mode--;
	flags++;
    }
    return output_type;
}


int 
checkflags(dbref what, struct flgchkdat check)
{
    if (check.fortype && (Typeof(what) != check.istype))
	return (0);
    if (check.isnotroom && (Typeof(what) == TYPE_ROOM))
	return (0);
    if (check.isnotexit && (Typeof(what) == TYPE_EXIT))
	return (0);
    if (check.isnotthing && (Typeof(what) == TYPE_THING))
	return (0);
    if (check.isnotplayer && (Typeof(what) == TYPE_PLAYER))
	return (0);
    if (check.isnotprog && (Typeof(what) == TYPE_PROGRAM))
	return (0);

    if (check.forlevel && (MLevel(what) != check.islevel))
	return (0);
    if (check.isnotzero && (MLevel(what) == 0))
	return (0);
    if (check.isnotone && (MLevel(what) >= 1))
	return (0);
    if (check.isnottwo && (MLevel(what) >= 2))
	return (0);
    if (check.isnotthree && (MLevel(what) >= 3))
	return (0);
    if (check.isnotfour && (MLevel(what) >= 4))
	return (0);
    if (check.isnotfive && (MLevel(what) >= 5))
	return (0);
    if (check.isnotsix && (MLevel(what) >= 6))
	return (0);
    if (check.isnotseven && (MLevel(what) >= 7))
	return (0);
    if (check.isnoteight && (MLevel(what) >= 8))
	return (0);
    if (check.isnotnine && (MLevel(what) >= 9))
	return (0);
    if (check.isnotwiz && (WLevel(what) >= LMAGE))
	return (0);
    if (check.iswiz && (WLevel(what) < LMAGE))
	return (0);

    if (FLAGS(what) & check.clearflags)
	return (0);
    if ((~FLAGS(what)) & check.setflags)
	return (0);

    if (FLAG2(what) & check.clearflag2)
	return (0);
    if ((~FLAG2(what)) & check.setflag2)
	return (0);

    if (check.forlink) {
	switch (Typeof(what)) {
	    case TYPE_ROOM:
		if ((DBFETCH(what)->sp.room.dropto == NOTHING) !=
			(!check.islinked))
		    return (0);
		break;
	    case TYPE_EXIT:
		if ((!DBFETCH(what)->sp.exit.ndest) != (!check.islinked))
		    return (0);
		break;
	    case TYPE_PLAYER:
	    case TYPE_THING:
		if (!check.islinked)
		    return (0);
		break;
	    default:
		if (check.islinked)
		    return (0);
	}
    }
    if (check.forold) {
	if( (((current_systime-DBFETCH(what)->ts.lastused) < tp_aging_time) ||
	     ((current_systime-DBFETCH(what)->ts.modified) < tp_aging_time))
	     != (!check.isold) )
	    return(0);
    }
    if (check.size) {
	if ((size_object(what, check.loadedsize) < check.size)
		!= (!check.issize)) {
	    return 0;
	}
    }
    return (1);
}


void 
display_objinfo(dbref player, dbref obj, int output_type)
{
    char    buf[BUFFER_LEN];
    char    buf2[BUFFER_LEN];

    strcpy(buf2, ansi_unparse_object(player, obj));
    strcat(buf2, CNORMAL);

    switch (output_type) {
	case 1:		/* owners */
	    sprintf(buf, "%-38.512s  %.512s", buf2,
		    ansi_unparse_object(player, OWNER(obj)));
	    break;
	case 2:		/* links */
	    switch (Typeof(obj)) {
		case TYPE_ROOM:
		    sprintf(buf, "%-38.512s  %.512s", buf2,
		       ansi_unparse_object(player, DBFETCH(obj)->sp.room.dropto));
		    break;
		case TYPE_EXIT:
		    if (DBFETCH(obj)->sp.exit.ndest == 0) {
			sprintf(buf, "%-38.512s  %.512s", buf2, "*UNLINKED*");
			break;
		    }
		    if (DBFETCH(obj)->sp.exit.ndest > 1) {
			sprintf(buf, "%-38.512s  %.512s", buf2, "*METALINKED*");
			break;
		    }
		    sprintf(buf, "%-38.512s  %.512s", buf2,
		      ansi_unparse_object(player, DBFETCH(obj)->sp.exit.dest[0]));
		    break;
		case TYPE_PLAYER:
		    sprintf(buf, "%-38.512s  %.512s", buf2,
		       ansi_unparse_object(player, DBFETCH(obj)->sp.player.home));
		    break;
		case TYPE_THING:
		    sprintf(buf, "%-38.512s  %.512s", buf2,
			ansi_unparse_object(player, DBFETCH(obj)->sp.thing.home));
		    break;
		default:
		    sprintf(buf, "%-38.512s  %.512s", buf2, "N/A");
		    break;
	    }
	    break;
	case 3:		/* locations */
	    sprintf(buf, "%-38.512s  %.512s", buf2,
		    ansi_unparse_object(player, DBFETCH(obj)->location));
	    break;
	case 4:
	    return;
	case 5:
	    sprintf(buf, "%-38.512s  %d bytes.", buf2, size_object(obj, 0));
	    break;
	case 0:
	default:
	    strcpy(buf, buf2);
	    break;
    }
    anotify(player, buf);
}


void 
do_find(dbref player, const char *name, const char *flags)
{
    dbref   i;
    struct flgchkdat check;
    char    buf[BUFFER_LEN+2];
    int     total = 0;
    int     output_type = init_checkflags(player, flags, &check);

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    strcpy(buf, "*");
    strcat(buf, name);
    strcat(buf, "*");

    if (!payfor(player, tp_lookup_cost)) {
	anotify_fmt(player, CFAIL "You don't have enough %s.", tp_pennies);
    } else {
	for (i = 0; i < db_top; i++) {
	    if ((Mage(OWNER(player)) || OWNER(i) == OWNER(player)) &&
		    checkflags(i, check) && NAME(i) &&
		    (!*name || equalstr(buf, (char *) NAME(i)))) {
		display_objinfo(player, i, output_type);
		total++;
	    }
	}
	anotify(player, CINFO "***End of List***");
	anotify_fmt(player, CINFO "%d database item%s found.", total, (total!=1) ? "s" : "");
    }
}


void 
do_owned(dbref player, const char *name, const char *flags)
{
    dbref   victim, i;
    struct flgchkdat check;
    int     total = 0;
    int     output_type = init_checkflags(player, flags, &check);

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (!payfor(player, tp_lookup_cost)) {
	anotify_fmt(player, CFAIL "You don't have enough %s.", tp_pennies);
	return;
    }
    if (Mage(OWNER(player)) && *name) {
	if ((victim = strcmp(name, "me") ? lookup_player(name) : player)
		== NOTHING) {
	    anotify(player, CINFO WHO_MESG);
	    return;
	}
    } else
	victim = player;

    for (i = 0; i < db_top; i++) {
	if ((OWNER(i) == OWNER(victim)) && checkflags(i, check)) {
	    display_objinfo(player, i, output_type);
	    total++;
	}
    }
    anotify(player, CINFO "***End of List***");
    anotify_fmt(player, CINFO "%d database item%s found.", total, (total!=1) ? "s" : "");
}

void 
do_trace(dbref player, const char *name, int depth)
{
    dbref   thing;
    int     i;
    struct match_data md;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, name, NOTYPE, &md);
    match_absolute(&md);
    match_here(&md);
    match_me(&md);
    match_neighbor(&md);
    match_possession(&md);
    match_registered(&md);
    if ((thing = noisy_match_result(&md)) == NOTHING
	    || thing == AMBIGUOUS)
	return;

    for (i = 0; (!depth || i < depth) && thing != NOTHING; i++) {
	if (controls(player, thing) || can_link_to(player, NOTYPE, thing))
	    anotify(player, ansi_unparse_object(player, thing));
	else
	    anotify(player, CINFO "**Missing**");
	thing = DBFETCH(thing)->location;
    }
    anotify(player, CINFO "***End of List***");
}

void 
do_entrances(dbref player, const char *name, const char *flags)
{
    dbref   i, j;
    dbref   thing;
    struct match_data md;
    struct flgchkdat check;
    int     total = 0;
    int     output_type = init_checkflags(player, flags, &check);

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (*name == '\0') {
	thing = getloc(player);
    } else {
	init_match(player, name, NOTYPE, &md);
	match_possession(&md);
	match_neighbor(&md);
	match_all_exits(&md);
	match_registered(&md);
	if (Mage(OWNER(player))) {
	    match_absolute(&md);
	    match_player(&md);
	}
	match_here(&md);
	match_me(&md);

	thing = noisy_match_result(&md);
    }
    if (thing == NOTHING) {
	anotify(player, CINFO WHICH_MESG);
	return;
    }
    if (!controls(OWNER(player), thing)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    init_checkflags(player, flags, &check);
    for (i = 0; i < db_top; i++) {
	if (checkflags(i, check)) {
	    switch (Typeof(i)) {
		case TYPE_EXIT:
		    for (j = DBFETCH(i)->sp.exit.ndest; j--;) {
			if (DBFETCH(i)->sp.exit.dest[j] == thing) {
			    display_objinfo(player, i, output_type);
			    total++;
			}
		    }
		    break;
		case TYPE_PLAYER:
		    if (DBFETCH(i)->sp.player.home == thing) {
			display_objinfo(player, i, output_type);
			total++;
		    }
		    break;
		case TYPE_THING:
		    if (DBFETCH(i)->sp.thing.home == thing) {
			display_objinfo(player, i, output_type);
			total++;
		    }
		    break;
		case TYPE_ROOM:
		    if (DBFETCH(i)->sp.room.dropto == thing) {
			display_objinfo(player, i, output_type);
			total++;
		    }
		    break;
		case TYPE_PROGRAM:
		case TYPE_GARBAGE:
		    break;
	    }
	}
    }
    anotify(player, CINFO "***End of List***");
    anotify_fmt(player, CINFO "%d database item%s found.", total, (total!=1) ? "s" : "");
}

void 
do_contents(dbref player, const char *name, const char *flags)
{
    dbref   i;
    dbref   thing;
    struct match_data md;
    struct flgchkdat check;
    int     total = 0;
    int     output_type = init_checkflags(player, flags, &check);

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (*name == '\0') {
	thing = getloc(player);
    } else {
	init_match(player, name, NOTYPE, &md);
	match_me(&md);
	match_here(&md);
	match_possession(&md);
	match_neighbor(&md);
	match_all_exits(&md);
	match_registered(&md);
	if (Mage(OWNER(player))) {
	    match_absolute(&md);
	    match_player(&md);
	}

	thing = noisy_match_result(&md);
    }
    if (thing == NOTHING) return;
    if (!controls(OWNER(player), thing)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    init_checkflags(player, flags, &check);
    DOLIST(i, DBFETCH(thing)->contents) {
	if (checkflags(i, check)) {
	    display_objinfo(player, i, output_type);
	    total++;
	}
    }
    switch(Typeof(thing)) {
	case TYPE_EXIT:
	case TYPE_PROGRAM:
	case TYPE_GARBAGE:
	    i = NOTHING;
	    break;
	case TYPE_ROOM:
	case TYPE_THING:
	case TYPE_PLAYER:
	    i = DBFETCH(thing)->exits;
	    break;
    }
    DOLIST(i, i) {
	if (checkflags(i, check)) {
	    display_objinfo(player, i, output_type);
	    total++;
	}
    }
    anotify(player, CINFO "***End of List***");
    anotify_fmt(player, CINFO "%d database item%s found.", total, (total!=1) ? "s" : "");
}

static int
exit_matches_name(dbref exit, const char *name)
{
    char buf[BUFFER_LEN];
    char *ptr, *ptr2;

    strcpy(buf, NAME(exit));
    for (ptr2 = ptr = buf; *ptr; ptr = ptr2) {
	while (*ptr2 && *ptr2 != ';') ptr2++;
	if (*ptr2) *ptr2++ = '\0';
	while (*ptr2 == ';') ptr2++;
	if (string_prefix(name, ptr) && DBFETCH(exit)->sp.exit.ndest &&
		Typeof((DBFETCH(exit)->sp.exit.dest)[0]) == TYPE_PROGRAM)
	    return 1;
    }
    return 0;
}

void
exit_match_exists(dbref player, dbref obj, const char *name)
{
    dbref exit;
    char buf[BUFFER_LEN];
    exit = DBFETCH(obj)->exits;
    while (exit != NOTHING) {
	if (exit_matches_name(exit, name)) {
	    sprintf(buf, "  %ss are trapped on %.2048s",
		    name, ansi_unparse_object(player, obj));
	    anotify(player, buf);
	}
	exit = DBFETCH(exit)->next;
    }
}

void
do_sweep(dbref player, const char *name)
{
    dbref thing, ref, loc;
    int flag, tellflag;
    struct match_data md;
    char buf[BUFFER_LEN];

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (*name == '\0') {
	thing = getloc(player);
    } else {
	init_match(player, name, NOTYPE, &md);
	match_me(&md);
	match_here(&md);
	match_possession(&md);
	match_neighbor(&md);
	match_all_exits(&md);
	match_registered(&md);
	if (Mage(OWNER(player))) {
	    match_absolute(&md);
	    match_player(&md);
	}
	thing = noisy_match_result(&md);
    }
    if (thing == NOTHING) {
	anotify(player, CINFO WHICH_MESG);
	return;
    }

    if (*name && !controls(OWNER(player), thing)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    
    sprintf(buf, CINFO "Listeners in %s:", ansi_unparse_object(player, thing));
    anotify(player, buf);

    ref = DBFETCH(thing)->contents;
    for (; ref != NOTHING; ref = DBFETCH(ref)->next) {
	switch (Typeof(ref)) {
	    case TYPE_PLAYER:
		if (/* !Dark(thing) || */online(ref)) {
		    sprintf(buf, "  %s" CNORMAL " is a %splayer.",
			    ansi_unparse_object(player, ref),
			    online(ref)? "" : "sleeping ");
		    anotify(player, buf);
		}
		break;
	    case TYPE_THING:
		if (FLAGS(ref) & (ZOMBIE | LISTENER)) {
		    tellflag = 0;
		    sprintf(buf, "  %.255s" CNORMAL " is a",
			    ansi_unparse_object(player, ref));
		    if (FLAGS(ref) & ZOMBIE) {
			tellflag = 1;
			if (!online(OWNER(ref))) {
			    tellflag = 0;
			    strcat(buf, " sleeping");
			}
			strcat(buf, " zombie");
		    }
		    if ((FLAGS(ref) & LISTENER) &&
			    (get_property(ref, "@listen" ) ||
			     get_property(ref, "~listen" ) ||
			     get_property(ref, "~olisten") ||
			     get_property(ref, "_listen" ) ||
			     get_property(ref, "_olisten"))) {
			strcat(buf, " listener");
			tellflag = 1;
		    }
		    strcat(buf, " database item owned by ");
		    strcat(buf, ansi_unparse_object(player, OWNER(ref)));
		    strcat(buf, CNORMAL ".");
		    if (tellflag) anotify(player, buf);
		}
		exit_match_exists(player, ref, "page");
		exit_match_exists(player, ref, "whisper");
		exit_match_exists(player, ref, "pose");
		exit_match_exists(player, ref, "say");
		break;
	}
    }
    flag = 0;
    loc = thing;
    while (loc != NOTHING) {
	if (controls(player, loc)) {
	    if (!flag) {
		anotify(player, CINFO "Listening rooms down the environment:");
		flag = 1;
	    }

	    if ((FLAGS(loc) & LISTENER) &&
		    (get_property(loc, "@listen" ) ||
		     get_property(loc, "~listen" ) ||
		     get_property(loc, "~olisten") ||
		     get_property(loc, "_listen" ) ||
		     get_property(loc, "_olisten"))) {
		sprintf(buf, "  %s" CNORMAL " is a listening room.",
			ansi_unparse_object(player, loc));
		anotify(player, buf);
	    }

	    exit_match_exists(player, loc, "page");
	    exit_match_exists(player, loc, "whisper");
	    exit_match_exists(player, loc, "pose");
	    exit_match_exists(player, loc, "say");
	}
	loc = getparent(loc);
    }
    anotify(player, CINFO "**End of list**");
}


void
do_quota(dbref player, const char *name) {
    int maxrooms = 0, maxexits = 0, maxthings = 0, maxprograms = 0;
    int rooms = 0, exits = 0, things = 0, programs = 0, other = 0;
    dbref who;
    if (*name == '\0' || !Mage(OWNER(player)))
	who = OWNER(player);
    else
	who = lookup_player(name);

    if (who == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }

    if (!has_quotas(who)) {
	anotify(player, CINFO "No quota limits.");
	return;
    }

    max_stats(who, &maxrooms, &maxexits, &maxthings, &maxprograms);
    count_stats(who, &rooms, &exits, &things, &other, &programs, &other);
    anotify(player, CYELLOW "Quota     Limit  Used  Left");
    anotify_fmt(player, CBROWN "Rooms:    %5d %5d %5d",
    	maxrooms, rooms, maxrooms - rooms);
    anotify_fmt(player, CBROWN "Exits:    %5d %5d %5d",
    	maxexits, exits, maxexits - exits);
    anotify_fmt(player, CBROWN "Things:   %5d %5d %5d",
    	maxthings, things, maxthings - things);
  if(Mucker(who))
    anotify_fmt(player, CBROWN "Programs: %5d %5d %5d",
	maxprograms, programs, maxprograms - programs);
}
