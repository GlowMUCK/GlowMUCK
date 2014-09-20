/* path.c Copyright 1998 by Andrew Nelson All Rights Reserved */

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


#ifdef PATH


int
valid_path_dir(void)
{
    int len;

    /* Guarantee tp_path_dir is 78 or less chars long, doesn't end */
    /* in a '/', and doesn't contain any ':'s (propdir delimiters) */

    len = strlen(tp_path_dir) + 1;
    return(
    	(len <= 0) || (len > 78) || (len > BUFFER_LEN / 16) ||
	(tp_path_dir[len - 1] == '/') ||
	strchr(tp_path_dir, PROP_DELIMITER)
    ) ? 0 : 1;
}

int
valid_path(dbref loc, const char *pathname)
{
    return(
    	(!valid_path_dir()) ||
    	(!OkObj(loc)) ||
	(Typeof(loc) != TYPE_ROOM) ||
	(!pathname) ||
	(!*pathname) ||
	((strlen(pathname) + 1) > (BUFFER_LEN / 8)) ||
	strchr(pathname, PROP_DELIMITER) ||
	strchr(pathname, PROPDIR_DELIMITER)
    ) ? 0 : 1;
}

const char *
get_path_prop(dbref loc, const char *pathname, const char *prop) {
    const char *m;
    char pathprop[BUFFER_LEN / 4];

    if( (!valid_path(loc, pathname)) ||	(!prop) || (!*prop) )
	return NULL;

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);
    strcat(pathprop, "/");
    strcat(pathprop, prop);

    m = get_property_class(loc, pathprop);
#ifdef COMPRESS
    if(m && *m)
	m = uncompress(m);
#endif /* COMPRESS */

    return m;
}


dbref
get_path_dest(dbref loc, const char *pathname)
{
    dbref dest;
    char pathprop[BUFFER_LEN / 4];

    if(!valid_path(loc, pathname))
	return NOTHING;

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);

    dest = get_property_dbref(loc, pathprop);        

    if((!OkObj(dest)) || (
	(Typeof(dest) != TYPE_ROOM) &&
	(Typeof(dest) != TYPE_PROGRAM)
    )) {
	if(dest == NOTHING) /* No property */
	    return NOTHING;

	/* Clean up a recycled path, set destination to source room */
	dest = loc;
	set_property(loc, pathprop, PROP_REFTYP, (PTYPE)(long int)dest);
	ts_modifyobject(loc);

	log_status("PATH: Bad path '%s' relinked from/to %s.\n",
	    pathname, unparse_object(MAN, dest)
	);
    }

    return dest;
}

struct boolexp *
get_path_lock(dbref loc, const char *pathname)
{
    char pathprop[BUFFER_LEN / 4];

    if(!valid_path(loc, pathname))
	return TRUE_BOOLEXP;

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);
    strcat(pathprop, "/lok");

    return get_property_lock(loc, pathprop);        
}


int
get_path_size(dbref loc, const char *pathname, int load)
{
    char pathprop[BUFFER_LEN];

    if(!valid_path(loc, pathname))
	return 0;

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);
    strcat(pathprop, "/");

#ifdef DISKBASE
    if (load) {
	fetchprops(loc);
	fetch_propvals(loc, pathprop);
    }
#endif

    return size_proplist(get_property(loc, pathprop));
}


dbref
match_path(dbref loc, const char *pathname, char *cmdbuf, int *fullmatch)
{
    PropPtr propadr, pptr;
    char buf[BUFFER_LEN], pbuf[BUFFER_LEN / 4];
    const char *fullpathname;

    /* Get the full path name property of a partial path in a room along    */
    /* with it's destination room's dbref and argument. match_cmdname       */
    /* contains the full path name (with no propdir prefixes) if            */
    /* destination is not NOTHING. */

    if(!cmdbuf || !pathname)
	return NOTHING;

    *cmdbuf = '\0';
    *buf = '\0';
    if(fullmatch) *fullmatch = 0;

    while(*pathname == ' ')
	pathname++; /* No leading spaces, please */

    fullpathname = path_name(pathname); /* Abbreviate common ordinal paths */

    /* Find the shortened 1-word part of the path if it exists */
    strcpy(buf, fullpathname);
    if((pathname = strchr(buf, ' ')) != NULL)
	buf[pathname - buf] = '\0'; /* End word at the first space */
    pathname = buf;

    /* Check that path names are valid */
    if(!valid_path(loc, pathname))
	return NOTHING;

#ifdef DISKBASE
    fetchprops(loc);
#endif
    strcpy(pbuf, tp_path_dir);
    strcat(pbuf, "/");
    propadr = first_prop(loc, pbuf, &pptr, cmdbuf);

    while ((propadr > 0) && *cmdbuf) {

	if( exit_prefix(cmdbuf, pathname)) {
#ifdef DISKBASE
	    propfetch(loc, propadr);
#endif
	    return (PropType(propadr) == PROP_REFTYP)
		 ? PropDataRef(propadr) : NOTHING;
	} else if( exit_prefix(cmdbuf, fullpathname) ) {
#ifdef DISKBASE
	    propfetch(loc, propadr);
#endif
	    if(fullmatch) *fullmatch = 1;
	    return (PropType(propadr) == PROP_REFTYP)
		 ? PropDataRef(propadr) : NOTHING;
	}
	propadr = next_prop(pptr, propadr, cmdbuf);
    }

    *cmdbuf = '\0';
    return NOTHING;
}

const char *
full_path( const char *path )
{
    if((!path[0]) || (path[1] && path[2]))
	return path; /* Ignore empty or len > 2 names */

    if(!path[1]) {
	switch(path[0]) {
	    case 'n':	case 'N':	return "north";
	    case 's':	case 'S':	return "south";
	    case 'e':	case 'E':	return "east";
	    case 'w':	case 'W':	return "west";
	    case 'u':	case 'U':	return "up";
	    case 'd':	case 'D':	return "down";
	    case 'i':	case 'I':	return "in";
	    case 'o':	case 'O':	return "out";
	}
    } else {
	switch(path[0]) {
	    case 'n':	case 'N':
		switch(path[1]) {
		    case 'e':	case 'E':	return "northeast";
		    case 'w':	case 'W':	return "northwest";
		}
		break;
	    case 's':	case 'S':
	        switch(path[1]) {
		    case 'e':	case 'E':	return "southeast";
		    case 'w':	case 'W':	return "southwest";
	        }
	        break;
	}
    }
    
    return path;
}

const char *
full_paths( const char *path, char *fullpaths )
{
    char buf[BUFFER_LEN], *endpath;
    const char *pathname;

    if((!fullpaths) || (!path) || (!*path))
	return "";

    fullpaths[0] = '\0';
    strcpy(buf, path);

    pathname = buf;

    while((endpath = (char *)strchr(pathname, EXIT_DELIMITER)) != NULL) {
	if(*fullpaths)
	    strcat(fullpaths, ";");

	*endpath = '\0';
	pathname = full_path(pathname);
	if((strlen(pathname) + strlen(fullpaths)) < ((BUFFER_LEN / 2) - 2))
	    strcat(fullpaths, pathname);
	pathname = endpath + 1;
    }
    if(*pathname) {
	if(*fullpaths)
	    strcat(fullpaths, ";");

	pathname = full_path(pathname);
	if((strlen(pathname) + strlen(fullpaths)) < ((BUFFER_LEN / 2) - 2))
	    strcat(fullpaths, pathname);
    }

    return fullpaths;
}

const char *
path_name( const char *path )
{
    if(!path)
	return "NULL"; /* Null */

    while(*path && (*path <= ' '))
	path++; /* Get rid of preceding spaces */

    if(!*path)
	return ""; /* Blank */

    if(!path[1])
	return path; /* Length is 1 */

    switch( path[0] ) {
	case 'n':
	case 'N':
		     if( !string_compare(path, "north") )
			return "n";
		else if( !string_compare(path, "northwest") )
			return "nw";
		else if( !string_compare(path, "northeast") )
			return "ne";
	case 's':
	case 'S':
		     if( !string_compare(path, "south") )
			return "s";
		else if( !string_compare(path, "southwest") )
			return "sw";
		else if( !string_compare(path, "southeast") )
			return "se";
		break;
	case 'e':
	case 'E':
		if( !string_compare(path, "east") )
			return "e";
		break;
	case 'w':
	case 'W':
		if( !string_compare(path, "west") )
			return "w";
		break;
	case 'o':
	case 'O':
		if( !string_compare(path, "out") )
			return "o";
		break;
	case 'i':
	case 'I':
		if( !string_compare(path, "in") )
			return "i";
		break;
	case 'u':
	case 'U':
		if( !string_compare(path, "up") )
			return "u";
		break;
	case 'd':
	case 'D':
		if( !string_compare(path, "down") )
			return "d";
		break;
    }

    return path;
}

const char *
path_name_set( const char *pathname, char *buf )
{
    const char *endpath, *curpath;
    char pathbuf[BUFFER_LEN], endstr[2];

    endstr[0] = EXIT_DELIMITER;
    endstr[1] = '\0';

    if((!pathname) || (!*pathname) || !buf)
	return "";

    buf[0] = '\0';
    while(*pathname && (endpath = strchr(pathname, EXIT_DELIMITER)) != NULL) {
	curpath = pathname;
	while(curpath < endpath) {
            curpath++;
	    pathbuf[curpath - pathname] = *curpath;
        }
	pathbuf[curpath - pathname] = '\0';
	while(*endpath == EXIT_DELIMITER) endpath++;
	if(*pathbuf) {
	    strcat(buf, path_name(pathbuf));
	    if(*endpath) strcat(buf, endstr); /* Don't put ; if nothing after */
	}
	pathname = endpath;
    }
    if(*pathname) strcat(buf, path_name(pathname));

    return buf;
}

void
examine_path(dbref player, dbref loc, const char *pathname)
{
    dbref dest;
    const char *prettyname, *m;
    char buf[BUFFER_LEN], buf2[BUFFER_LEN], *p;

    pathname = path_name(pathname); /* Abbreviate common ordinal paths */

    if(!valid_path(loc, pathname)) {
	anotify(player, CFAIL "Invalid path.");
	return;
    }

    if(!controls(player, loc)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    dest = get_path_dest(loc, pathname);
    if(!OkObj(dest)) {
	anotify(player, CINFO NOTHERE_MESG);
	return;
    }

    /* Get the obvious exits name */
    prettyname = get_path_prop(loc, pathname, "na");

    /* List name and owner */
    if(prettyname && *prettyname) {
	strcpy(buf, pathname);
	p = strchr(buf, ';');
	if(p)
	    *p = '\0';
	p = buf;
	while(*p) {
	    *p = toupper(*p);
	    p++;
	}
	p = buf;
        if((buf[0] == 'I') && !buf[1]) {
	    buf[1] = 'N';
	    buf[2] = '\0';
        }

	anotify_fmt(player, CBLUE "%s  " CCYAN "%s " CYELLOW "<%s>  " CAQUA "Owner: %s",
	    full_paths(pathname, buf2), prettyname, p,
	    ansi_unparse_object(player, OWNER(loc))
	);
    } else {
	anotify_fmt(player, CBLUE "%s  " CAQUA "Owner: %s",
	    full_paths(pathname, buf2), ansi_unparse_object(player, OWNER(loc))
	);
    }

    /* Type and flags */
    m = get_path_prop(loc, pathname, "d"); /* Dark flag */
    anotify_fmt(player,
	CGREEN "Type: " CYELLOW "PATH"
	CGREEN "  Flags:" CYELLOW "%s",
	(m && *m) ? " DARK" : ""
    );

    /* Description */
    m = get_path_prop(loc, pathname, "de");
    if(m && *m) notify(player, m);

    /* Lock */
    anotify_fmt(player, CVIOLET "Key:" CPURPLE " %s",
	unparse_boolexp(player, get_path_lock(loc, pathname), 1)
    );

    /* Messages */
    
    m = get_path_prop(loc, pathname, "sc");
    if(m && *m) anotify_fmt(player, CAQUA "Succ:" CCYAN " %s", tct(m,buf));
    
    m = get_path_prop(loc, pathname, "fl");
    if(m && *m) anotify_fmt(player, CAQUA "Fail:" CCYAN " %s", tct(m,buf));
    
    m = get_path_prop(loc, pathname, "dr");
    if(m && *m) anotify_fmt(player, CAQUA "Drop:" CCYAN " %s", tct(m,buf));
    
    m = get_path_prop(loc, pathname, "osc");
    if(m && *m) anotify_fmt(player, CAQUA "OSucc:" CCYAN " %s", tct(m,buf));
    
    m = get_path_prop(loc, pathname, "ofl");
    if(m && *m) anotify_fmt(player, CAQUA "OFail:" CCYAN " %s", tct(m,buf));
    
    m = get_path_prop(loc, pathname, "odr");
    if(m && *m) anotify_fmt(player, CAQUA "ODrop:" CCYAN " %s", tct(m,buf));

    /* Memory bloating */
    anotify_fmt(player, CVIOLET "In Memory:" CPURPLE " %d bytes", get_path_size(loc, pathname, 0));
    
    /* Source and destination */
    anotify_fmt(player, CAQUA "Source: %s", ansi_unparse_object(player, loc));
    anotify_fmt(player, CAQUA "Destination: %s", ansi_unparse_object(player, dest));

}


void
path_setprop(dbref player, dbref loc, const char *pathname, const char *prop, const char *message)
{
    char pathprop[BUFFER_LEN / 4];

    pathname = path_name(pathname); /* Abbreviate common ordinal paths */

    if(!valid_path(loc, pathname)) {
	anotify(player, CFAIL "Invalid path.");
	return;
    }

    if(!controls(player, loc)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if(	(!prop) || (!*prop) ) {
	anotify(player, CINFO "Set what property?");
	return;
    }

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);
    strcat(pathprop, "/");
    strcat(pathprop, prop);

    if(!string_compare(prop, "d")) {
	prop = "dark flag";
    } else if(!string_compare(prop, "na")) {
	prop = "obvious-exits name";
    } else if(!string_compare(prop, "de")) {
	prop = "description";
    } else prop = "message";

    if(message && *message) {
	set_property(loc, pathprop, PROP_STRTYP, (PTYPE)message);	
	anotify_fmt(player, CSUCC "Path %s set.", prop);
    } else {
	remove_property(loc, pathprop);
	anotify_fmt(player, CSUCC "Path %s cleared.", prop);
    }
    ts_modifyobject(loc);
}


void
path_setlock(dbref player, dbref loc, const char *pathname, const char *lock)
{
    struct boolexp *key;
    char pathprop[BUFFER_LEN / 4];

    pathname = path_name(pathname); /* Abbreviate common ordinal paths */

    if(!valid_path(loc, pathname)) {
	anotify(player, CFAIL "Invalid path.");
	return;
    }

    if(!controls(player, loc)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);
    strcat(pathprop, "/");
    strcat(pathprop, "lok");

    if(lock && *lock) {
	key = parse_boolexp(player, lock, 0);

	if(key == TRUE_BOOLEXP) {
	    anotify(player, CINFO BADKEY_MESG);
	} else {
	    set_property(loc, pathprop, PROP_LOKTYP, (PTYPE)key);	
	    ts_modifyobject(loc);
	    anotify(player, CSUCC "Path locked.");
	} 
    } else {
	set_property(loc, pathprop, PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);
	ts_modifyobject(loc);
	anotify(player, CSUCC "Path unlocked.");
    }
}


void
do_path(dbref player, const char *pathname, const char *destination, const char *prettyname)
{
    dbref dest, loc;
    struct match_data md;
    char buf[BUFFER_LEN];
    char pathprop[BUFFER_LEN / 4];

    loc = getloc(player);

    pathname = path_name_set(pathname, buf);

    if(!valid_path(loc, pathname)) {
	anotify(player, CFAIL "Invalid path.");
	return;
    }

    if(!controls(player, loc)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    /* Find the destination */

    init_match(player, destination, TYPE_ROOM, &md);
    match_here(&md);
    match_home(&md);
    match_absolute(&md);

    if ((dest = match_result(&md)) == NOTHING || dest == AMBIGUOUS) {
	anotify(player, CINFO "What room is that?");
	return;
    }

    if( (!controls(player,dest)) && (!(FLAGS(dest) & LINK_OK)) ) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);

    set_property(loc, pathprop, PROP_REFTYP, (PTYPE)(long int)dest);
    if(prettyname && *prettyname) {
	strcat(pathprop, "/na");
	set_property(loc, pathprop, PROP_STRTYP, (PTYPE)prettyname);
    }
    ts_modifyobject(loc);

    anotify_fmt(player, CSUCC "Path linked to %s.",
	unparse_object(player, dest)
    );
}

void
do_path_junk(dbref player, const char *pathname)
{
    dbref loc, dest;
    char buf[BUFFER_LEN];
    char pathprop[BUFFER_LEN / 4];

    loc = getloc(player);
    pathname = path_name_set(pathname, buf); /* Abbreviate common ordinal paths */

    if(!valid_path(loc, pathname)) {
	anotify(player, CFAIL "Invalid path.");
	return;
    }

    if(!controls(player, loc)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    dest = get_path_dest(loc, pathname);
    if(!OkObj(dest)) {
	anotify(player, CINFO NOTHERE_MESG);
	return;
    }

    strcpy(pathprop, tp_path_dir);
    strcat(pathprop, "/");
    strcat(pathprop, pathname);

    remove_property(loc, pathprop);
    ts_modifyobject(loc);

    anotify(player, CSUCC "Path removed.");
}

void 
do_path_setprop(dbref player, const char *name, const char *prop, const char *message)
{
    dbref loc;
    char buf[BUFFER_LEN];

    loc = getloc(player);

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if(OkObj(loc) && (Typeof(loc) == TYPE_ROOM) && controls(player, loc)) {
	if(OkObj(match_path(loc, name, buf, NULL))) {
	    path_setprop(player, loc, buf, prop, message);
	} else anotify(player, CINFO NOTHERE_MESG);
    } else anotify(player, CFAIL NOPERM_MESG);
}

void
move_path(dbref player, const char *pathname, const char *command, int fullmatch)
{
    dbref loc, dest;
    struct boolexp *lock;
    char *arg;

    loc = getloc(player);
    pathname = path_name(pathname); /* Abbreviate common ordinal paths */

    if((!valid_path(loc, pathname)) || (!command)) {
	anotify(player, CFAIL NOWAY_MESG);
	return;
    }

    strcpy(match_cmdname, command);
    arg = strchr(match_cmdname, ' ');
    if(arg && (*arg == ' ') && (!fullmatch)) {
	*arg = '\0';
	strcpy(match_args, arg + 1);
    } else *match_args = '\0';

    /* Find the destination */
    dest = get_path_dest(loc, pathname);
    if(!OkObj(dest)) {
	anotify(player, CFAIL NOWAY_MESG);
	return;
    }

    /* Check the lock */    
    lock = get_path_lock(loc, pathname);
    /* TODO: Guest check */
    if(eval_boolexp(player, lock, loc)) {
	path_exec_mesg(player, pathname, "sc");
	path_exec_mesg(player, pathname, "dr");

	switch(Typeof(dest)) {
	    case TYPE_ROOM:
		enter_room(player, dest, loc);
		break;

	    case TYPE_PROGRAM:
		interp(player, loc, dest, loc, FOREGROUND, STD_REGUID, 0);
		break;
	}

    } else {
	/* Notify of failure */
	path_exec_mesg(player, pathname, "fl");
    }
}

void
path_exec_mesg(dbref player, const char *pathname, const char *mesg)
{
    dbref loc, dest, nloc;
    int moved = 0;
    const char *how = "(@Path)";
    const char *ohow = "(@Opath)";
    const char *m;
    const char *def = NULL;
    char omesg[4] = "o";
    char buf[BUFFER_LEN];

    nloc = loc = getloc(player);
    dest = get_path_dest(loc, pathname); /* Verifies loc too */

    if((!OkObj(dest)) || (!mesg) || (!*mesg) || (strlen(mesg) > 2))
	return;

    strcat(omesg, mesg);
    if(!string_compare(mesg, "sc")) {
	how = "(@Succ)";
	ohow = "(@Osucc)";
    } else if(!string_compare(mesg, "fl")) {
	def = CFAIL NOWAY_MESG;
	how = "(@Fail)";
	ohow = "(@Ofail)";
    } else if(!string_compare(mesg, "dr")) {
	moved = 1;
	nloc = dest; /* Room to notify messages in */
	how = "(@Drop)";
	ohow = "(@Odrop)";
    }

    /* Find message for player */
    m = path_find_mesg(loc, pathname, mesg, Typeof(dest) == TYPE_ROOM);
    if(m && *m) {
	if(strlen(m) < BUFFER_LEN) {
	    strcpy(buf, m);
	    path_subst(player, loc, dest, pathname, buf, moved);
	    m = buf;
	}
	exec_or_notify(player, loc, m, how);
    } else if(def && *def) {
	anotify(player, def);
    }

    /* Find message for everyone else */

    m = path_find_mesg(loc, pathname, omesg, Typeof(dest) == TYPE_ROOM);
    if(m && *m) {
	if(strlen(m) < BUFFER_LEN) {
	    strcpy(buf, m);
	    path_subst(player, loc, dest, pathname, buf, moved);
	    m = buf;
	}
	parse_omessage(player, nloc, loc, m, PNAME(player), ohow);
    }
}

const char *
path_find_mesg(dbref loc, const char *pathname, const char *mesg, int envsearch)
{
    int number = 0;
    const char *m;

    m = pathname;

    if(isdigit(*m)) {
	/* Check if the first alias is a number */
	while(*m && (*m != EXIT_DELIMITER) && isdigit(*m))
	    m++;

	if((!*m) || (*m == EXIT_DELIMITER))
	    number = 1; /* We found a full number! */
    }

    while(OkObj(loc) && (Typeof(loc) == TYPE_ROOM)) {
	if((m = get_path_prop(loc, pathname, mesg)) != NULL)
	    return m;

	if((number) && ((m = get_path_prop(loc, "num", mesg)) != NULL))
	    return m;

	if((m = get_path_prop(loc, "all", mesg)) != NULL)
	    return m;

	if(!envsearch) break;
	loc = getloc(loc);
    }

    return NULL;
}

/* mesg must be a buffer of size BUFFER_LEN */
void
path_subst(dbref player, dbref loc, dbref dest, const char *pathname, char *mesg, int moved)
{
    dbref env;
    const char *m, *pt, *pf;
    char pname[BUFFER_LEN], *p;

    if( (!OkObj(player)) || (!OkObj(loc)) || (!OkObj(dest)) ||
	(!mesg) || (!*mesg)
    ) return; /* Bad values or no substs in string */

    /* Substitute each path value in turn */

    strcpy(pname, pathname);
    p = strchr(pname, EXIT_DELIMITER);
    if(p) *p = '\0'; /* Cut at first semicolon */

    pf = pt = pname;
    if((pname[0]) && (!pname[1])) {
	switch(pname[0]) {
	    case 'o': case 'O':	pf = "out"  ; pt = "in"   ; break;
	    case 'n': case 'N':	pf = "north"; pt = "south"; break;
	    case 's': case 'S':	pf = "south"; pt = "north"; break;
	    case 'e': case 'E':	pf = "east" ; pt = "west" ; break;
	    case 'w': case 'W':	pf = "west" ; pt = "east" ; break;
	    case 'u': case 'U':	pf = "up"   ; pt = "down" ; break;
	    case 'd': case 'D':	pf = "down" ; pt = "up"   ; break;
	    case 'i': case 'I':	pf = "in"   ; pt = "out"  ; break;
	}
    } else if((pname[0]) && pname[1] && (!pname[2])) {
	if((pname[0] == 'n') || (pname[0] == 'N')) {
	    if((pname[1] == 'e') || (pname[1] == 'E')) {
		pf = "northeast"; pt = "southwest";
	    } else if ((pname[1] == 'w') || (pname[1] == 'W')) {
		pf = "northwest"; pt = "southeast";
	    }
	} else if ((pname[0] == 's') || (pname[0] == 'S')) {
	    if((pname[1] == 'e') || (pname[1] == 'E')) {
		pf = "southeast"; pt = "northwest";
	    } else if ((pname[1] == 'w') || (pname[1] == 'W')) {
		pf = "southwest"; pt = "northeast";
	    }
	}
    }

    /* Path names from and to, almost always used */
    string_subst(mesg, "%pathfrom", pf);
    string_subst(mesg, "%pathto",   pt);

    /* Some places may not use default %verbs */
    if(!strchr(mesg, '%'))
	return; /* No more %substs */

    env = moved ? dest : loc;
    m = envpropstr(&env, "_pathverb");
#ifdef COMPRESS
    if(m && *m) m = uncompress(m);
#endif
    if((!m) || (!*m)) m = "walk";
    string_subst(mesg, "%verb", m);

    env = moved ? dest : loc;
    m = envpropstr(&env, "_opathverb");
#ifdef COMPRESS
    if(m && *m) m = uncompress(m);
#endif
    if((!m) || (!*m)) m = "walks";
    string_subst(mesg, "%overb", m);

    /* Even less use room names */
    if(!strchr(mesg, '%'))
	return; /* No more %substs */

    string_subst(mesg, "%roomfrom",  NAME(loc));
    string_subst(mesg, "%roomto",    NAME(dest));
    string_subst(mesg, "%ownerfrom", NAME(OWNER(loc)));
    string_subst(mesg, "%ownerto",   NAME(OWNER(dest)));

}

#endif /* PATH */
