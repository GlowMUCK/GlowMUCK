#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <ctype.h>

#include "color.h"
#include "db.h"
#include "mpi.h"
#include "interface.h"
#include "match.h"
#include "tune.h"
#include "props.h"
#include "externs.h"

/* Commands which involve speaking */

int     blank(const char *s);

void 
do_say(dbref player, const char *message)
{
    dbref   loc;
    char    buf[BUFFER_LEN], buf2[BUFFER_LEN];

    if ((loc = getloc(player)) == NOTHING)
	return;

    do_parse_mesg(player, player, message, "(say)", buf, MPI_ISPRIVATE);
    tct(buf,buf2);

    /* Notify player */
    sprintf(buf, CAQUA "You say, \"" CYELLOW "%.3900s" CAQUA "\"", buf2);
    anotify(player, buf);

    /* notify everybody else */
    sprintf(buf, CAQUA "%s says, \"" CYELLOW "%.3900s" CAQUA "\"", PNAME(player), buf2);
    anotify_except(DBFETCH(loc)->contents, player, buf, player);
}

void 
do_whisper(dbref player, const char *arg1, const char *arg2)
{
    int     ignored;
    dbref   who;
    char    buf[BUFFER_LEN], buf2[BUFFER_LEN];
    struct match_data md;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, arg1, TYPE_PLAYER, &md);
    match_neighbor(&md);
    match_me(&md);
    if (Mage(player) && Typeof(player) == TYPE_PLAYER) {
	match_absolute(&md);
	match_player(&md);
    }
    switch (who = match_result(&md)) {
	case NOTHING:
	case AMBIGUOUS:
	    anotify(player, CINFO WHO_MESG);
	    break;
	default:

	    ignored = ignoring(who, player);
	    if(ignored == 1) {
		anotify(player, CFAIL "That player is ignoring you.");
		return;
	    } else if(ignored == 2) {
		anotify(player, CINFO "That player is ignoring you.");
	    }

	    do_parse_mesg(player, player, arg2, "(whisper)", buf, MPI_ISPRIVATE);
	    tct(buf,buf2);

	    if (buf2[0] == ':' || buf2[0] == ';') {
		sprintf(buf, CBLUE "%s whispers, \"" CPURPLE "%s %.3900s" CBLUE "\"",
					PNAME(player), PNAME(player), buf2+1);
		if (!anotify_from(player, who, buf)) {
		    sprintf(buf, CBLUE "%s is not connected.", PNAME(who));
		    anotify(player, buf);
		    break;
		}
		sprintf(buf, CBLUE "You whisper, \"" CPURPLE "%s %.3900s" CBLUE "\" to %s.",
					 PNAME(player), buf2+1, PNAME(who));
		anotify(player, buf);
		break;
	    } else { 
		sprintf(buf, CBLUE "%s whispers, \"" CPURPLE "%.3900s" CBLUE "\"", PNAME(player), buf2);
		if (!anotify_from(player, who, buf)) {
		    sprintf(buf, CBLUE "%s is not connected.", PNAME(who));
		    anotify(player, buf);
		    break;
		}
		sprintf(buf, CBLUE "You whisper, \"" CPURPLE "%.3900s" CBLUE "\" to %s.", buf2, PNAME(who));
		anotify(player, buf);
		break;
	    }
    }
}

void 
do_pose(dbref player, const char *message)
{
    dbref   loc;
    char    buf[BUFFER_LEN], buf2[BUFFER_LEN];

    if ((loc = getloc(player)) == NOTHING)
	return;

    do_parse_mesg(player, player, message, "(pose)", buf, MPI_ISPRIVATE);
    tct(buf,buf2);

    /* notify everybody */
    sprintf(buf, CAQUA "%s%s%.3900s", PNAME(player),
		isalpha(buf2[0]) ? " " : "", buf2);
    anotify_except(DBFETCH(loc)->contents, NOTHING, buf, player);
}

void 
do_wall(dbref player, const char *message)
{
    char    buf[BUFFER_LEN];

    if (Mage(player) && Typeof(player) == TYPE_PLAYER) {
	switch(message[0]) {
	    case ':':
	    case ';':
		sprintf(buf, MARK "%s %.3900s", NAME(player), message+1);
		break;
	    case '@':
		sprintf(buf, MARK "%.3900s", message+1);
		break;
	    case '\0':
	    case '#':
		notify(player, "@wall help");
		notify(player, "~~~~~");
		notify(player, "@wall message  -- Show all players 'message'");
		notify(player, "@wall :message -- Pose message to all players");
		notify(player, "@wall @message -- Spoof message to all players");
		notify(player, "@wall #        -- Show this help list");
		return;
	    default:
		sprintf(buf, MARK "%s shouts, \"%.3900s\"", NAME(player), message);
	}
	wall_all( buf );
	/* log_status("WALL: %s: %.3900s\n", unparse_object(MAN, player), buf); */
    } else {
	anotify(player, CFAIL NOPERM_MESG);
    }
}

void 
do_gripe(dbref player, const char *message)
{
    dbref   loc;
    char buf[BUFFER_LEN];

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (!message || !*message) {
	if (Mage(player)) {
	    spit_file(player, LOG_GRIPE);
	} else {
	    anotify(player, CINFO "What's wrong?");
	}
	return;
    }

    loc = DBFETCH(player)->location;
    log_gripe("%s(%d) in %s(%d): %.3900s\n",
	      NAME(player), player, NAME(loc), loc, message);

    anotify(player, CINFO "Your complaint has been filed.");

    sprintf(buf, MARK "Gripe from %s: %.3900s", NAME(player), message);
    wall_wizards(buf);
}

/* doesn't really belong here, but I couldn't figure out where else */
void 
do_page(dbref player, const char *arg1, const char *arg2)
{
    int     ignored;
    char    buf[BUFFER_LEN], buf2[BUFFER_LEN];
    dbref   target;

    if (!payfor(player, tp_lookup_cost)) {
	anotify_fmt(player, CFAIL "You don't have enough %s.", tp_pennies);
	return;
    }
    if ( strcmp(arg1, "me") ) {
	if ((target = lookup_player(arg1)) == NOTHING) {
	    anotify(player, CINFO WHO_MESG);
	    return;
	}
    } else target = player;

    if(Guest(player)) {
	if(!Mage(target)) {
	    anotify(player, CINFO "Guests can only page " NAMEWIZ "s, type 'wizzes'.");
	    return;
	}
    }

    if (FLAGS(target) & HAVEN) {
	anotify(player, CFAIL "That player is haven.");
	return;
    }

    ignored = ignoring(target, player);
    if(ignored == 1) {
	anotify(player, CFAIL "That player is ignoring you.");
	return;
    } else if(ignored == 2) {
	anotify(player, CINFO "That player is ignoring you.");
    }

    do_parse_mesg(player, player, arg2, "(page)", buf, MPI_ISPRIVATE);
    tct(buf,buf2);

    if (!*buf2) {
	sprintf(buf, CGREEN "You sense that %s is looking for you in %s.",
			PNAME(player), NAME(DBFETCH(player)->location));
    } else {
	if(buf2[0] == ':' || buf2[0] == ';') {
	    sprintf(buf, CGREEN "%s pages \"" CYELLOW "%s %.3900s" CGREEN "\"",
			PNAME(player), PNAME(player), buf2);
	} else {
	    sprintf(buf, CGREEN "%s pages \"" CYELLOW "%.3900s" CGREEN "\"",
			PNAME(player), buf2);
	}
    }
    if (anotify_from(player, target, buf))
	anotify(player, CSUCC "Your message has been sent.");
    else {
	sprintf(buf, CINFO "%s is not connected.", PNAME(target));
	anotify(player, buf);
    }
}

int
notify_listeners(dbref who, dbref xprog, dbref obj,
		 dbref room, const char *msg, int isprivate)
{
    return ansi_notify_listeners(who, xprog, obj, room, msg, isprivate, 0);
}

int
ansi_notify_listeners(dbref who, dbref xprog, dbref obj,
		 dbref room, const char *msg, int isprivate,
		 int parseansi)
{
    const char *lmsg;
    char buf[BUFFER_LEN];
    dbref ref;

    if (obj == NOTHING)
	return 0;

    /* Gag jerks, but not wizards or selves */
    if((who != obj) && (ignoring(obj, who) == 1))
	return 0;

    if(parseansi > 0) {
	buf[0] = '\0';
	unparse_ansi(buf, msg, parseansi);
        lmsg = buf;
    } else lmsg = msg;

    if (tp_listeners && (tp_listeners_obj || Typeof(obj) == TYPE_ROOM)) {
	listenqueue(who,room,obj,obj,xprog,"@listen",lmsg, tp_listen_mlev,1,1);
	listenqueue(who,room,obj,obj,xprog,"~listen",lmsg, tp_listen_mlev,1,1);
	listenqueue(who,room,obj,obj,xprog,"~olisten",lmsg,tp_listen_mlev,0,1);
	listenqueue(who,room,obj,obj,xprog,"_listen",lmsg, tp_listen_mlev,1,tp_mortal_mpi_listen_props);
	listenqueue(who,room,obj,obj,xprog,"_olisten",lmsg,tp_listen_mlev,0,tp_mortal_mpi_listen_props);
    }

    if ((Typeof(obj) == TYPE_THING || Typeof(obj) == TYPE_PLAYER) && !isprivate) {
	if (!(FLAGS(obj) & QUELL)) {
	    if (getloc(who) == getloc(obj)) {
		char pbuf[BUFFER_LEN];
		const char *prefix;

		prefix = GETOECHO(obj);
		if (prefix && *prefix) {
		    prefix = do_parse_mesg(who, obj, prefix,
				    "(@Oecho)", pbuf, MPI_ISPRIVATE
			     );
		}
		if (!prefix || !*prefix)
		    prefix = "Outside>";
		sprintf(buf, "%s %.*s", prefix,
		    (int)(BUFFER_LEN - 2 - strlen(prefix)), msg
		);
		ref = DBFETCH(obj)->contents;
		while(ref != NOTHING) {
		    if(obj != OWNER(ref)) /* Don't tell us what we already know */
			ansi_notify_nolisten(ref, buf, isprivate, parseansi);

		    ref = DBFETCH(ref)->next;
		}
	    }
	}
    }

    if (Typeof(obj) == TYPE_PLAYER || Typeof(obj) == TYPE_THING) {
	return ansi_notify_nolisten(obj, msg, isprivate, parseansi);
    } else return 0;
}

void 
notify_except(dbref first, dbref exception, const char *msg, dbref who)
{
    dbref   room, srch;

    if (first != NOTHING) {

	srch = room = DBFETCH(first)->location;

	if (tp_listeners) {
	    notify_from_echo(who, srch, msg, 0);

	    if (tp_listeners_env) {
		srch = DBFETCH(srch)->location;
		while (srch != NOTHING) {
		    notify_from_echo(who, srch, msg, 0);
		    srch = getparent(srch);
		}
	    }
	}

	DOLIST(first, first) {
	    if ((Typeof(first) != TYPE_ROOM) && (first != exception)) {
		/* don't want excepted player or child rooms to hear */
		notify_from_echo(who, first, msg, 0);
	    }
	}
    }
}


void 
anotify_except(dbref first, dbref exception, const char *msg, dbref who)
{
    dbref   room, srch;

    if (first != NOTHING) {

	srch = room = DBFETCH(first)->location;

	if (tp_listeners) {
	    anotify_from_echo(who, srch, msg, 0);

	    if (tp_listeners_env) {
		srch = DBFETCH(srch)->location;
		while (srch != NOTHING) {
		    anotify_from_echo(who, srch, msg, 0);
		    srch = getparent(srch);
		}
	    }
	}

	DOLIST(first, first) {
	    if ((Typeof(first) != TYPE_ROOM) && (first != exception)) {
		/* don't want excepted player or child rooms to hear */
		anotify_from_echo(who, first, msg, 0);
	    }
	}
    }
}


void
parse_omessage(dbref player, dbref dest, dbref exit, const char *msg, const char *prefix, const char *whatcalled)
{
    char buf[BUFFER_LEN * 2];
    char *ptr;

    do_parse_mesg(player, exit, msg, whatcalled, buf, MPI_ISPUBLIC);
    ptr = pronoun_substitute(player, buf);
    if (!*ptr) return;
    if (*ptr == '\'' || *ptr == ' ' || *ptr == ',' || *ptr == '-') {
	sprintf(buf, "%s%.3900s", NAME(player), ptr);
    } else {
	sprintf(buf, "%s %.3900s", NAME(player), ptr);
    }
    notify_except(DBFETCH(dest)->contents, player, buf, player);
}


int
blank(const char *s)
{
    while (*s && isspace(*s))
	s++;

    return !(*s);
}


void
do_gag(dbref player, const char *who)
{
    dbref victim;

    if(Typeof(player) != TYPE_PLAYER)
	return;

    if(!tp_ignore_support) {
	anotify(player, CINFO "Player gagging is disabled.");
	return;
    }

    if(!*who) {
	int ai, ignores;
	dbref *ignoring;

	ignores = DBFETCH(player)->sp.player.ignores;
	ignoring = DBFETCH(player)->sp.player.ignoring;

	anotify(player, CNOTE "Players you are ignoring:");
	if((ignores > 0) && ignoring) {
	    for(ai = 0; ai < ignores; ai++)
		if(OkObj(ignoring[ai]) && (Typeof(ignoring[ai]) == TYPE_PLAYER))
		    anotify_fmt(player, CINFO "%s%s",
			NAME(ignoring[ai]),
			(QLevel(ignoring[ai]) >= tp_quell_ignore_mlevel)
			? " [WIZ]" : ""
		    );
	}
	anotify(player, CNOTE "Players online ignoring you:");
	dump_ignoring(player);
	anotify(player, CSUCC "Done.");
	return;
    }

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if((victim = lookup_player(who)) == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }

    if(victim == player) {
	anotify(player, CINFO "Gag you with a spoon?");
	return;
    }

    if(add_ignoring(player, victim))
	anotify(player, CSUCC "Player gagged.");
    else
	anotify(player, CFAIL "Couldn't gag player.");
}


void
do_ungag(dbref player, const char *who)
{
    dbref victim;

    if(Typeof(player) != TYPE_PLAYER)
	return;

    if(!tp_ignore_support) {
	anotify(player, CINFO "Player gagging is disabled.");
	return;
    }

    if(!*who) {
	do_gag(player, "");
	return;
    }

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if((victim = lookup_player(who)) == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }

    if(victim == player) {
	anotify(player, CINFO "Ungag you with a spoon?");
	return;
    }

    if(remove_ignoring(player, victim))
	anotify(player, CSUCC "Player ungagged.");
    else
	anotify(player, CFAIL "Couldn't ungag player.");
}
