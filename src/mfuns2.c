#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <math.h>
#include <ctype.h>

#include "db.h"
#include "tune.h"
#include "mpi.h"
#include "props.h"
#include "match.h"
#include "interp.h"
#include "interface.h"
#include "msgparse.h"
#include "externs.h"

#ifdef MPI

/***** Insert MFUNs here *****/

const char *
mfn_owner(MFUNARGS)
{
    dbref   obj;
    
    obj = mesg_dbref_raw(player, what, perms, argv[0]);
    if (obj == AMBIGUOUS || obj == NOTHING || obj == UNKNOWN)
	ABORT_MPI("OWNER","Failed match");
    if (obj == PERMDENIED)
	ABORT_MPI("OWNER",NOPERM_MESG);
    if (obj == HOME)
	obj = DBFETCH(player)->sp.player.home;
    return ref2str(OWNER(obj), buf);
}


const char *
mfn_controls(MFUNARGS)
{

dbref   obj;
    dbref   obj2;
    
    obj = mesg_dbref_raw(player, what, perms, argv[0]);
    if (obj == AMBIGUOUS || obj == NOTHING || obj == UNKNOWN)
	ABORT_MPI("CONTROLS","Match failed. (1)");
    if (obj == PERMDENIED)
	ABORT_MPI("CONTROLS","Permission denied. (1)");
    if (obj == HOME) obj = DBFETCH(player)->sp.player.home;
    if (argc > 1) {
	obj2 = mesg_dbref_raw(player, what, perms, argv[1]);
	if (obj2 == AMBIGUOUS || obj2 == NOTHING || obj2 == UNKNOWN)
	    ABORT_MPI("CONTROLS","Match failed. (2)");
	if (obj2 == PERMDENIED)
	    ABORT_MPI("CONTROLS","Permission denied. (2)");
	if (obj2 == HOME) obj2 = DBFETCH(player)->sp.player.home;
	if (Typeof(obj2) != TYPE_PLAYER) obj2 = OWNER(obj2);
    } else {
	obj2 = OWNER(perms);
    }
    if (controls(obj2, obj)) {
	return "1";
    } else {
	return "0";
    }
}


const char *
mfn_links(MFUNARGS)
{
    char buf2[BUFFER_LEN];
    dbref   obj;
    int i, cnt;

    obj = mesg_dbref(player, what, perms, argv[0]);
    if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	ABORT_MPI("LINKS","Match failed");
    if (obj == PERMDENIED)
	ABORT_MPI("LINKS",NOPERM_MESG);
    switch (Typeof(obj)) {
	case TYPE_ROOM:
	    obj = DBFETCH(obj)->sp.room.dropto;
	    break;
	case TYPE_PLAYER:
	    obj = DBFETCH(obj)->sp.player.home;
	    break;
	case TYPE_THING:
	    obj = DBFETCH(obj)->sp.thing.home;
	    break;
	case TYPE_EXIT: {
	    dbref obj2;
	    *buf = '\0';
	    cnt = DBFETCH(obj)->sp.exit.ndest;
	    if (cnt) {
		for (i = 0; i < cnt; i++) {
		    obj2 = DBFETCH(obj)->sp.exit.dest[i];
		    ref2str(obj2, buf2);
		    if (strlen(buf) + strlen(buf2) + 2 < BUFFER_LEN) {
			if (*buf) strcat(buf, "\r");
			strcat(buf, buf2);
		    } else break;
		}
		return buf;
	    } else {
		return "#-1";
	    }
	    break;
	}
	case TYPE_PROGRAM:
	default:
	    return "#-1";
	    break;
    }
    return ref2str(obj, buf);
}


const char *
mfn_locked(MFUNARGS)
{
    dbref who = mesg_dbref_local(player, what, perms, argv[0]);
    dbref obj = mesg_dbref_local(player, what, perms, argv[1]);
    if (who == AMBIGUOUS || who == UNKNOWN || who == NOTHING || who == HOME)
	ABORT_MPI("LOCKED","Match failed. (1)");
    if (who == PERMDENIED)
	ABORT_MPI("LOCKED","Permission denied. (1)");
    if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	ABORT_MPI("LOCKED","Match failed. (2)");
    if (obj == PERMDENIED)
	ABORT_MPI("LOCKED","Permission denied. (2)");
    sprintf(buf, "%d", !could_doit(who, obj));
    return buf;
}


const char *
mfn_testlock(MFUNARGS)
{
    struct boolexp *lok;
    dbref who = player;
    dbref obj = mesg_dbref_local(player, what, perms, argv[0]);

    if (argc > 2)
	who = mesg_dbref_local(player, what, perms, argv[2]);
    if (who == AMBIGUOUS || who == UNKNOWN || who == NOTHING || who == HOME)
	ABORT_MPI("TESTLOCK","Match failed. (1)");
    if (who == PERMDENIED)
	ABORT_MPI("TESTLOCK","Permission denied. (1)");
    if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	ABORT_MPI("TESTLOCK","Match failed. (2)");
    if (obj == PERMDENIED)
	ABORT_MPI("TESTLOCK","Permission denied. (2)");
    lok = get_property_lock(obj, argv[1]);
    if (argc > 3 && lok == TRUE_BOOLEXP)
	return (argv[3]);
    if (eval_boolexp(who, lok, obj)) {
	return "1";
    } else {
	return "0";
    }
}


const char *
mfn_contents(MFUNARGS)
{
    char buf2[50];
    int list_limit = MAX_MFUN_LIST_LEN;
    dbref obj = mesg_dbref_local(player, what, perms, argv[0]);
    int typchk, ownroom;
    int outlen, nextlen;

    if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	ABORT_MPI("CONTENTS","Match failed");
    if (obj == PERMDENIED)
	ABORT_MPI("CONTENTS",NOPERM_MESG);

    typchk = NOTYPE;
    if (argc > 1) {
	if (!string_compare(argv[1], "Room")) {
	    typchk = TYPE_ROOM;
	} else if (!string_compare(argv[1], "Exit")) {
	    typchk = TYPE_EXIT;  /* won't find any, though */
	} else if (!string_compare(argv[1], "Player")) {
	    typchk = TYPE_PLAYER;
	} else if (!string_compare(argv[1], "Program")) {
	    typchk = TYPE_PROGRAM;
	} else if (!string_compare(argv[1], "Thing")) {
	    typchk = TYPE_THING;
	} else {
	    ABORT_MPI("CONTENTS","Type must be 'player', 'room', 'thing', 'program', or 'exit'. (2)");
	}
    }
    strcpy(buf, "");
    outlen = 0;
    ownroom = controls(perms, obj);
    obj = DBFETCH(obj)->contents;
    while (obj != NOTHING && list_limit) {
	if ((typchk == NOTYPE || Typeof(obj) == typchk) &&
		(ownroom || controls(perms, obj) ||
		!((FLAGS(obj) & DARK) || (FLAGS(getloc(obj)) & DARK) ||
		(Typeof(obj) == TYPE_PROGRAM && !(FLAGS(obj) & LINK_OK)))) &&
		!(Typeof(obj) == TYPE_ROOM && typchk != TYPE_ROOM)) {
	    ref2str(obj, buf2);
	    nextlen = strlen(buf2);
	    if ((outlen + nextlen) >= (BUFFER_LEN - 3))
		break;
	    if (outlen) strcat((buf+(outlen++)), "\r");
	    strcat((buf + outlen), buf2);
	    outlen += nextlen;
	    list_limit--;
	}
	obj = DBFETCH(obj)->next;
    }
    return buf;
}



const char *
mfn_exits(MFUNARGS)
{
    int outlen, nextlen;
    char buf2[50];
    int list_limit = MAX_MFUN_LIST_LEN;
    dbref obj = mesg_dbref(player, what, perms, argv[0]);

    if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	ABORT_MPI("EXITS","Match failed");
    if (obj == PERMDENIED)
	ABORT_MPI("EXITS",NOPERM_MESG);

    switch(Typeof(obj)) {
	case TYPE_ROOM:
	case TYPE_THING:
	case TYPE_PLAYER:
	    obj = DBFETCH(obj)->exits;
	    break;
	default:
	    obj = NOTHING;
	    break;
    }
    *buf = '\0';
    outlen = 0;
    while (obj != NOTHING && list_limit) {
	ref2str(obj, buf2);
	nextlen = strlen(buf2);
	if ((outlen + nextlen) >= (BUFFER_LEN - 3))
	    break;
	if (outlen) strcat((buf + (outlen++)), "\r");
	strcat((buf + outlen), buf2);
	outlen += nextlen;
	list_limit--;
	obj = DBFETCH(obj)->next;
    }
    return buf;
}


const char *
mfn_v(MFUNARGS)
{
    char *ptr = get_mvar(argv[0]);

    if (!ptr)
	ABORT_MPI("V","No such variable defined");
    return ptr;
}


const char *
mfn_set(MFUNARGS)
{
    char *ptr = get_mvar(argv[0]);
    if (!ptr)
	ABORT_MPI("SET","No such variable currently defined");
    strcpy(ptr, argv[1]);
    return ptr;
}


const char *
mfn_ref(MFUNARGS)
{
    dbref obj;
    char *p;

    for (p = argv[0]; *p && isspace(*p); p++);
    if (*p == '#' && number(p+1)) {
	obj = atoi(p+1);
    } else {
	obj = mesg_dbref_local(player, what, perms, argv[0]);
	if (obj == PERMDENIED)
	    ABORT_MPI("REF",NOPERM_MESG);
	if (obj == UNKNOWN) obj = NOTHING;
    }
    sprintf(buf, "#%d", obj);
    return buf;
}


const char *
mfn_name(MFUNARGS)
{
    char *ptr;
    dbref obj;

    obj = tp_compatible_mpi
    	? mesg_dbref_raw(player, what, perms, argv[0])
    	: mesg_dbref_local(player, what, perms, argv[0])
	;

    if (obj == UNKNOWN)
	ABORT_MPI("NAME","Match failed");
    if (obj == PERMDENIED)
	ABORT_MPI("NAME",NOPERM_MESG);
    if (obj == NOTHING) {
	strcpy(buf, "#NOTHING#");
	return buf;
    }
    if (obj == AMBIGUOUS) {
	strcpy(buf, "#AMBIGUOUS#");
	return buf;
    }
    if (obj == HOME) {
	strcpy(buf, "#HOME#");
	return buf;
    }
    strcpy(buf, RNAME(obj));
    if (Typeof(obj) == TYPE_EXIT) {
	ptr = index(buf, ';');
	if (ptr) *ptr = '\0';
    }
    return buf;
}


const char *
mfn_fullname(MFUNARGS)
{
    dbref obj;
    
    obj = tp_compatible_mpi
    	? mesg_dbref_raw(player, what, perms, argv[0])
    	: mesg_dbref_local(player, what, perms, argv[0])
	;

    if (obj == UNKNOWN)
	ABORT_MPI("NAME","Match failed");
    if (obj == PERMDENIED)
	ABORT_MPI("NAME",NOPERM_MESG);
    if (obj == NOTHING) {
	strcpy(buf, "#NOTHING#");
	return buf;
    }
    if (obj == AMBIGUOUS) {
	strcpy(buf, "#AMBIGUOUS#");
	return buf;
    }
    if (obj == HOME) {
	strcpy(buf, "#HOME#");
	return buf;
    }
    strcpy(buf, RNAME(obj));
    return buf;
}


int
countlitems(char *list, char *sep)
{
    char *ptr;
    int seplen;
    int count = 1;

    if (!list || !*list) return 0;
    seplen = strlen(sep);
    ptr = list;
    while(*ptr) {
	while (*ptr && strncmp(ptr, sep, seplen)) ptr++;
	if (*ptr) {
	    ptr += seplen;
	    count++;
	}
    }
    return count;
}



/* buf is outbut buffer.  list is list to take item from.
 * line is list line to take. */

char *
getlitem(char *buf, char *list, char *sep, int line)
{
    char *ptr, *ptr2;
    char tmpchr;
    int seplen;

    seplen = strlen(sep);
    ptr = ptr2 = list;
    while (*ptr && line--) {
	for (ptr2 = ptr; *ptr2 && strncmp(ptr2, sep, seplen); ptr2++);
	if (!line) break;
	if (*ptr2) {
	    ptr2 += seplen;
	}
	ptr = ptr2;
    }
    tmpchr = *ptr2;
    *ptr2 = '\0';
    strcpy(buf, ptr);
    *ptr2 = tmpchr;
    return buf;
}


const char *
mfn_sublist(MFUNARGS)
{
    char   *ptr;
    char sepbuf[BUFFER_LEN];
    char buf2[BUFFER_LEN];
    int count = 1;
    int which;
    int end;
    int incr = 1;
    int i;
    int pflag;

    if (argc > 1) {
	which = atoi(argv[1]);
    } else {
	strcpy(buf, argv[0]);
	return buf;
    }

    strcpy(sepbuf, "\r");
    if (argc > 3) {
	if (!*argv[3])
	    ABORT_MPI("SUBLIST","Can't use null seperator string");
	strcpy(sepbuf, argv[3]);
    }

    count = countlitems(argv[0], sepbuf);  /* count of items in list */

    if (which == 0) return "";
    if (which > count) which = count;
    if (which < 0) which += count + 1;
    if (which < 1) which = 1;

    end = which;

    if (argc > 2) {
	end = atoi(argv[2]);
    }

    if (end == 0) return "";
    if (end > count) end = count;
    if (end < 0) end += count + 1;
    if (end < 1) end = 1;

    if (end < which) {
	incr = -1;
    }

    *buf = '\0';
    pflag = 0;
    for (i = which; ((i <= end) && (incr == 1)) ||
		    ((i >= end) && (incr == -1)); i += incr) {
	if (pflag) {
	    strcat(buf, sepbuf);
	} else {
	    pflag++;
	}
	ptr = getlitem(buf2, argv[0], sepbuf, i);
	strcat(buf, ptr);
    }
    return buf;
}


const char *
mfn_lrand(MFUNARGS)
{
    /* {lrand:list,sep}  */
    char sepbuf[BUFFER_LEN];
    int count = 1;
    int which = 0;

    strcpy(sepbuf, "\r");
    if (argc > 1) {
	if (!*argv[1])
	    ABORT_MPI("LRAND","Can't use null seperator string");
	strcpy(sepbuf, argv[1]);
    }

    count = countlitems(argv[0], sepbuf);
    if (count) {
	which = ((RANDOM() / 256) % count) + 1;
	getlitem(buf, argv[0], sepbuf, which);
    } else {
	*buf = '\0';
    }
    return buf;
}


const char *
mfn_count(MFUNARGS)
{
    strcpy(buf, "\r");
    if (argc > 1) {
	if (!*argv[1])
	    ABORT_MPI("COUNT","Can't use null seperator string");
	strcpy(buf, argv[1]);
    }
    sprintf(buf, "%d", countlitems(argv[0], buf));
    return buf;
}


const char *
mfn_with(MFUNARGS)
{
    char vbuf[BUFFER_LEN];
    char *ptr, *valptr;
    int v, cnt;

    ptr = MesgParse(argv[0], argv[0]);
    CHECKRETURN(ptr,"WITH","arg 1");
    v = new_mvar(ptr, vbuf);
    if (v == 1)
	ABORT_MPI("WITH","Variable name too long");
    if (v == 2)
	ABORT_MPI("WITH","Too many variables already defined");
    valptr = MesgParse(argv[1], argv[1]);
    CHECKRETURN(valptr,"WITH","arg 2");
    *buf = '\0';
    strcpy(vbuf, valptr);
    for (cnt = 2; cnt < argc; cnt++) {
	ptr = MesgParse(argv[cnt],argv[cnt]);
	if (!ptr) {
	    sprintf(buf, "%s %cWITH%c (%d)", get_mvar("how"),
		    MFUN_LEADCHAR, MFUN_ARGEND, cnt);
	    notify(player, buf);
	    return NULL;
	}
    }
    free_top_mvar();
    return ptr;
}


const char *
mfn_fold(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char tmp[BUFFER_LEN];
    char tmp2[BUFFER_LEN];
    char   *ptr, *ptr2;
    char *sepin = argv[4];
    int seplen, v;

    ptr = MesgParse(argv[0],argv[0]);
    CHECKRETURN(ptr,"FOLD","arg 1");
    v = new_mvar(ptr, tmp);
    if (v == 1)
	ABORT_MPI("FOLD","Variable name too long");
    if (v == 2)
	ABORT_MPI("FOLD","Too many variables already defined");

    ptr = MesgParse(argv[1],argv[1]);
    CHECKRETURN(ptr,"FOLD","arg 2");
    v = new_mvar(ptr, tmp2);
    if (v == 1)
	ABORT_MPI("FOLD","Variable name too long");
    if (v == 2)
	ABORT_MPI("FOLD","Too many variables already defined");

    if (argc > 4) {
	ptr = MesgParse(sepin,sepin);
	CHECKRETURN(ptr,"FOLD","arg 5");
	if (!*ptr)
	    ABORT_MPI("FOLD","Can't use Null seperator string");
    } else {
	strcpy(sepin, "\r");
    }
    seplen = strlen(sepin);
    ptr = MesgParse(argv[2],argv[2]);
    CHECKRETURN(ptr,"FOLD","arg 3");
    for (ptr2 = ptr; *ptr2 && strncmp(ptr2, sepin, seplen); ptr2++);
    if (*ptr2) {
	*ptr2 = '\0';
	ptr2 += seplen;
    }
    strcpy(buf, ptr);
    ptr = ptr2;
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && strncmp(ptr2, sepin, seplen); ptr2++);
	if (*ptr2) {
	    *ptr2 = '\0';
	    ptr2 += seplen;
	}
	strcpy(tmp2, ptr);
	strcpy(tmp, buf);
	MesgParse(argv[3],buf);
	CHECKRETURN(ptr,"FOLD","arg 4");
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("FOLD","Iteration limit exceeded");
    }
    free_top_mvar();
    free_top_mvar();
    return buf;
}


const char *
mfn_for(MFUNARGS)
{
    int   iter_limit = MAX_MFUN_LIST_LEN;
    char  tmp[BUFFER_LEN];
    char *ptr, *dptr;
    int   v, i, start, end, incr;

    ptr = MesgParse(argv[0],argv[0]);
    CHECKRETURN(ptr,"FOR","arg 1 (varname)");
    v = new_mvar(ptr, tmp);
    if (v == 1)
	ABORT_MPI("FOR","Variable name too long");
    if (v == 2)
	ABORT_MPI("FOR","Too many variables already defined");

    dptr = MesgParse(argv[1],argv[1]);
    CHECKRETURN(dptr,"FOR","arg 2 (start num)");
    start = atoi(dptr);

    dptr = MesgParse(argv[2],argv[2]);
    CHECKRETURN(dptr,"FOR","arg 3 (end num)");
    end = atoi(dptr);

    dptr = MesgParse(argv[3],argv[3]);
    CHECKRETURN(dptr,"FOR","arg 4 (increment)");
    incr = atoi(dptr);

    *buf = '\0';
    for (i = start; ((incr>=0 && i<=end) || (incr<0 && i>=end)); i += incr) {
	sprintf(tmp, "%d", i);
	dptr = MesgParse(argv[4],buf);
	CHECKRETURN(dptr,"FOR","arg 5 (repeated command)");
	if (!(--iter_limit))
	    ABORT_MPI("FOR","Iteration limit exceeded");
    }
    free_top_mvar();
    return buf;
}


const char *
mfn_foreach(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char tmp[BUFFER_LEN];
    char   *ptr, *ptr2, *dptr;
    char *sepin = argv[3];
    int seplen, v;

    ptr = MesgParse(argv[0],argv[0]);
    CHECKRETURN(ptr,"FOREACH","arg 1");
    v = new_mvar(ptr, tmp);
    if (v == 1)
	ABORT_MPI("FOREACH","Variable name too long");
    if (v == 2)
	ABORT_MPI("FOREACH","Too many variables already defined");

    dptr = MesgParse(argv[1],argv[1]);
    CHECKRETURN(dptr,"FOREACH","arg 2");

    if (argc > 3) {
	ptr = MesgParse(argv[3],argv[3]);
	CHECKRETURN(ptr,"FOREACH","arg 4");
	if (!*ptr)
	    ABORT_MPI("FOREACH","Can't use Null seperator string");
    } else {
	strcpy(sepin, "\r");
    }
    seplen = strlen(sepin);
    ptr = dptr;
    *buf= '\0';
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && strncmp(ptr2, sepin, seplen); ptr2++);
	if (*ptr2) {
	    *ptr2 = '\0';
	    ptr2 += seplen;
	}
	strcpy(tmp, ptr);
	dptr = MesgParse(argv[2],buf);
	CHECKRETURN(dptr,"FOREACH","arg 3");
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("FOREACH","Iteration limit exceeded");
    }
    free_top_mvar();
    return buf;
}


const char *
mfn_filter(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char buf2[BUFFER_LEN];
    char tmp[BUFFER_LEN];
    char   *ptr, *ptr2, *dptr;
    char *sepin = argv[3];
    char *sepbuf = argv[4];
    int seplen, len, buflen = 0, s2len, v;

    ptr = MesgParse(argv[0],argv[0]);
    CHECKRETURN(ptr,"FILTER","arg 1");
    v = new_mvar(ptr, tmp);
    if (v == 1)
	ABORT_MPI("FILTER","Variable name too long");
    if (v == 2)
	ABORT_MPI("FILTER","Too many variables already defined");

    dptr = MesgParse(argv[1],argv[1]);
    CHECKRETURN(dptr,"FILTER","arg 2");
    if (argc > 3) {
	ptr = MesgParse(sepin,sepin);
	CHECKRETURN(ptr,"FILTER","arg 4");
	if (!*ptr)
	    ABORT_MPI("FILTER","Can't use Null seperator string");
    } else {
	strcpy(sepin, "\r");
    }
    if (argc > 4) {
	ptr = MesgParse(sepbuf,sepbuf);
	CHECKRETURN(ptr,"FILTER","arg 5");
    } else {
	strcpy(sepbuf, sepin);
    }
    seplen = strlen(sepin);
    s2len = strlen(sepbuf);
    *buf = '\0';
    ptr = dptr;
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && strncmp(ptr2, sepin, seplen); ptr2++);
	if (*ptr2) {
	    *ptr2 = '\0';
	    ptr2 += seplen;
	}
	strcpy(tmp, ptr);
	dptr = MesgParse(argv[2],buf2);
	CHECKRETURN(dptr,"FILTER","arg 3");
	if (truestr(buf2)) {
	    len = strlen(ptr);
	    if (*buf) {
		/* There is no point in tacking a separator on if the */
		/* next data element doesn't fit either. */
		if ((buflen + s2len + len) >= BUFFER_LEN)
		    break;
		strcat(buf, sepbuf);
		buflen += s2len;
	    }
	    if ((buflen + len) >= BUFFER_LEN)
		break;
	    strcat(buf, ptr);
	    buflen += len;
	}
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("FILTER","Iteration limit exceeded");
    }
    free_top_mvar();
    return buf;
}


const char *
mfn_lremove(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char   *ptr, *ptr2, *p, *q;
    int len;

    ptr = argv[0];
    *buf = '\0';
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && *ptr2 != '\r'; ptr2++);
	if (*ptr2) *(ptr2++) = '\0';
	len = strlen(ptr);
	p = argv[1]; 
	do {
	    if (string_prefix(p, ptr) && (!p[len] || p[len] == '\r'))
		break;
	    while (*p && *p != '\r') p++;
	    if (*p) p++;
	} while (*p);
	q = buf; 
	do {
	    if (string_prefix(q, ptr) && (!q[len] || q[len] == '\r'))
		break;
	    while (*q && *q != '\r') q++;
	    if (*q) q++;
	} while (*q);
	if (!*p && !*q) {
	    if (*buf) strcat(buf, "\r");
	    strcat(buf, ptr);
	}
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("LREMOVE","Iteration limit exceeded");
    }
    return buf;
}


const char *
mfn_lcommon(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char   *ptr, *ptr2, *p, *q;
    int len;

    ptr = argv[1];
    *buf = '\0';
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && *ptr2 != '\r'; ptr2++);
	if (*ptr2) *(ptr2++) = '\0';
	len = strlen(ptr);
	p = argv[0]; 
	do {
	    if (string_prefix(p, ptr) && (!p[len] || p[len] == '\r'))
		break;
	    while (*p && *p != '\r') p++;
	    if (*p) p++;
	} while (*p);
	q = buf; 
	do {
	    if (string_prefix(q, ptr) && (!q[len] || q[len] == '\r'))
		break;
	    while (*q && *q != '\r') q++;
	    if (*q) q++;
	} while (*q);
	if (*p && !*q) {
	    if (*buf) strcat(buf, "\r");
	    strcat(buf, ptr);
	}
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("LCOMMON","Iteration limit exceeded");
    }
    return buf;
}


const char *
mfn_lunion(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char   *ptr, *ptr2, *p;
    int len;
    int outlen, nextlen;

    *buf = '\0';
    outlen = 0;
    ptr = argv[0];
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && *ptr2 != '\r'; ptr2++);
	if (*ptr2) *(ptr2++) = '\0';
	len = strlen(ptr);
	p = buf; 
	do {
	    if (string_prefix(p, ptr) && (!p[len] || p[len] == '\r'))
		break;
	    while (*p && *p != '\r') p++;
	    if (*p) p++;
	} while (*p);
	if (!*p) {
	    nextlen = strlen(ptr);
	    if (outlen + nextlen > BUFFER_LEN - 3)
		break;
	    if (outlen) strcat((buf+(outlen++)), "\r");
	    strcat((buf+outlen), ptr);
	    outlen += nextlen;
	}
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("LUNION","Iteration limit exceeded");
    }
    ptr = argv[1];
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && *ptr2 != '\r'; ptr2++);
	if (*ptr2) *(ptr2++) = '\0';
	len = strlen(ptr);
	p = buf; 
	do {
	    if (string_prefix(p, ptr) && (!p[len] || p[len] == '\r'))
		break;
	    while (*p && *p != '\r') p++;
	    if (*p) p++;
	} while (*p);
	if (!*p) {
	    nextlen = strlen(ptr);
	    if (outlen + nextlen > BUFFER_LEN - 3)
		break;
	    if (outlen) strcat((buf+(outlen++)), "\r");
	    strcat((buf+outlen), ptr);
	    outlen += nextlen;
	}
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("LUNION","Iteration limit exceeded");
    }
    return buf;
}


const char *
mfn_lsort(MFUNARGS)
{
    char *litem[MAX_MFUN_LIST_LEN];
    char vbuf[BUFFER_LEN];
    char vbuf2[BUFFER_LEN];
    char *ptr, *ptr2, *tmp;
    int i, j, count;

    if (argc > 1 && argc < 4)
	ABORT_MPI("LSORT","Takes 1 or 4 arguments");
    for (i = 0; i < MAX_MFUN_LIST_LEN; i++)
	litem[i] = NULL;
    ptr = MesgParse(argv[0],argv[0]);
    CHECKRETURN(ptr,"LSORT","arg 1");
    if (argc > 1) {
	ptr2 = MesgParse(argv[1], argv[1]);
	CHECKRETURN(ptr2,"LSORT","arg 2");
	j = new_mvar(ptr2, vbuf);
	if (j == 1)
	    ABORT_MPI("LSORT","Variable name too long");
	if (j == 2)
	    ABORT_MPI("LSORT","Too many variables already defined");
	ptr2 = MesgParse(argv[2],argv[2]);
	CHECKRETURN(ptr2,"LSORT","arg 3");
	j = new_mvar(ptr2, vbuf2);
	if (j == 1)
	    ABORT_MPI("LSORT","Variable name too long");
	if (j == 2)
	    ABORT_MPI("LSORT","Too many variables already defined");
    }
    count = 0;
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && *ptr2 != '\r'; ptr2++);
	if (*ptr2 == '\r') *(ptr2++) = '\0';
	litem[count++] = ptr;
	ptr = ptr2;
	if (count >= MAX_MFUN_LIST_LEN)
	    ABORT_MPI("LSORT","Iteration limit exceeded");
    }
    for (i = 0; i < count; i++) {
	for (j = i + 1; j < count; j++) {
	    if (argc > 1) {
		strcpy(vbuf, litem[i]);
		strcpy(vbuf2, litem[j]);
		ptr = MesgParse(argv[3],buf);
		CHECKRETURN(ptr,"LSORT","arg 4");
		if (truestr(buf)) {
		    tmp = litem[i];
		    litem[i] = litem[j];
		    litem[j] = tmp;
		}
	    } else {
		if (alphanum_compare(litem[i], litem[j]) > 0) {
		    tmp = litem[i];
		    litem[i] = litem[j];
		    litem[j] = tmp;
		}
	    }
	}
    }
    *buf = '\0';
    for (i = 0; i < count; i++) {
	if (*buf) strcat(buf, "\r");
	strcat(buf, litem[i]);
    }
    if (argc > 1) {
	free_top_mvar();
	free_top_mvar();
    }
    return buf;
}


const char *
mfn_lunique(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char   *ptr, *ptr2, *p;
    int len;
    int outlen, nextlen;

    *buf = '\0';
    outlen = 0;
    ptr = argv[0];
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && *ptr2 != '\r'; ptr2++);
	if (*ptr2) *(ptr2++) = '\0';
	len = strlen(ptr);
	p = buf; 
	do {
	    if (string_prefix(p, ptr) && (!p[len] || p[len] == '\r'))
		break;
	    while (*p && *p != '\r') p++;
	    if (*p) p++;
	} while (*p);
	if (!*p) {
	    nextlen = strlen(ptr);
	    if (outlen) strcat((buf + (outlen++)), "\r");
	    strcat((buf + outlen), ptr);
	    outlen += nextlen;
	}
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("LUNIQUE","Iteration limit exceeded");
    }
    return buf;
}


const char *
mfn_parse(MFUNARGS)
{
    int iter_limit = MAX_MFUN_LIST_LEN;
    char buf2[BUFFER_LEN];
    char tmp[BUFFER_LEN];
    char *ptr, *ptr2, *dptr;
    char *sepin = argv[3];
    char *sepbuf = argv[4];
    int seplen, outseplen, v;
    int outlen, nextlen;

    ptr = MesgParse(argv[0], argv[0]);
    CHECKRETURN(ptr,"PARSE","arg 1");
    v = new_mvar(ptr, tmp);
    if (v == 1)
	ABORT_MPI("PARSE","Variable name too long");
    if (v == 2)
	ABORT_MPI("PARSE","Too many variables already defined");

    dptr = MesgParse(argv[1], argv[1]);
    CHECKRETURN(dptr,"PARSE","arg 2");

    if (argc > 3) {
	ptr = MesgParse(sepin, sepin);
	CHECKRETURN(ptr,"PARSE","arg 4");
	if (!*ptr)
	    ABORT_MPI("PARSE","Can't use Null seperator string");
    } else {
	strcpy(sepin, "\r");
    }

    if (argc > 4) {
	ptr = MesgParse(sepbuf, sepbuf);
	CHECKRETURN(ptr,"PARSE","arg 5");
    } else {
	strcpy(sepbuf, sepin);
    }
    seplen = strlen(sepin);
    outseplen = strlen(sepbuf);

    *buf = '\0';
    outlen = 0;
    ptr = dptr;
    while (*ptr) {
	for (ptr2 = ptr; *ptr2 && strncmp(ptr2, sepin, seplen); ptr2++);
	if (*ptr2) {
	    *ptr2 = '\0';
	    ptr2 += seplen;
	}
	strcpy(tmp, ptr);
	dptr = MesgParse(argv[2], buf2);
	CHECKRETURN(dptr,"PARSE","arg 3");
	nextlen = strlen(buf2);
	if (outlen + nextlen + outseplen > BUFFER_LEN - 3)
	    break;
	if (outlen) {
	    strcat((buf+outlen), sepbuf);
	    outlen += outseplen;
	}
	strcat((buf + outlen), buf2);
	outlen += nextlen;
	ptr = ptr2;
	if (!(--iter_limit))
	    ABORT_MPI("PARSE","Iteration limit exceeded");
    }
    free_top_mvar();
    return buf;
}


const char *
mfn_smatch(MFUNARGS)
{
    if (equalstr(argv[1], argv[0])) {
	return "1";
    } else {
	return "0";
    }
}


const char *
mfn_strlen(MFUNARGS)
{
    sprintf(buf, "%d", (int) strlen(argv[0]));
    return buf;
}


const char *
mfn_subst(MFUNARGS)
{
    return string_substitute(argv[0], argv[1], argv[2], buf, BUFFER_LEN);
}


const char *
mfn_awake(MFUNARGS)
{
    dbref obj = mesg_dbref_local(player, what, perms, argv[0]);

    if (obj == PERMDENIED || obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	return("0");
    
    if (Typeof(obj) == TYPE_THING && (FLAGS(obj) & ZOMBIE)) {
	obj = OWNER(obj);
    } else if (Typeof(obj) != TYPE_PLAYER) {
	return("0");
    }
    
    sprintf(buf, "%d", online(obj));
    return(buf);
}


const char *
mfn_type(MFUNARGS)
{
    dbref obj = mesg_dbref_local(player, what, perms, argv[0]);

    if (obj == NOTHING || obj == AMBIGUOUS || obj == UNKNOWN)
	return("Bad");
    if (obj == HOME) return("Room");
    if (obj == PERMDENIED)
	ABORT_MPI("TYPE",NOPERM_MESG);
    
    switch(Typeof(obj)) {
	case TYPE_PLAYER:
	    return "Player";
	    break;
	case TYPE_ROOM:
	    return "Room";
	    break;
	case TYPE_EXIT:
	    return "Exit";
	    break;
	case TYPE_THING:
	    return "Thing";
	    break;
	case TYPE_PROGRAM:
	    return "Program";
	    break;
	default:
	    return "Bad";
	    break;
    }
    return "Bad";
}


const char *
mfn_istype(MFUNARGS)
{
    dbref obj = mesg_dbref_raw(player, what, perms, argv[0]);

    if (obj == NOTHING || obj == AMBIGUOUS || obj == UNKNOWN)
	return(string_compare(argv[1], "Bad") ? "0" : "1");
    if (obj == PERMDENIED)
	ABORT_MPI("TYPE",NOPERM_MESG);
    if (obj == HOME)
	return(string_compare(argv[1], "Room") ? "0" : "1");
    
    switch(Typeof(obj)) {
	case TYPE_PLAYER:
	    return(string_compare(argv[1], "Player") ? "0" : "1");
	    break;
	case TYPE_ROOM:
	    return(string_compare(argv[1], "Room") ? "0" : "1");
	    break;
	case TYPE_EXIT:
	    return(string_compare(argv[1], "Exit") ? "0" : "1");
	    break;
	case TYPE_THING:
	    return(string_compare(argv[1], "Thing") ? "0" : "1");
	    break;
	case TYPE_PROGRAM:
	    return(string_compare(argv[1], "Program") ? "0" : "1");
	    break;
	default:
	    return(string_compare(argv[1], "Bad") ? "0" : "1");
	    break;
    }
    return(string_compare(argv[1], "Bad") ? "0" : "1");
}


const char *
mfn_fox(MFUNARGS)
{
    return "YARF!";
}


const char *
mfn_rat(MFUNARGS)
{
    return "EEP!";
}


const char *
mfn_debugif(MFUNARGS)
{
    char *ptr = MesgParse(argv[0], argv[0]);
    CHECKRETURN(ptr,"DEBUGIF","arg 1");
    if (truestr(argv[0])) {
	ptr = mesg_parse(player, what, perms, argv[1],
			   buf, BUFFER_LEN, (mesgtyp | MPI_ISDEBUG));
    } else {
	ptr = MesgParse(argv[1], buf);
    }
    CHECKRETURN(ptr,"DEBUGIF","arg 2");
    return buf;
}


const char *
mfn_debug(MFUNARGS)
{
    char *ptr = mesg_parse(player, what, perms, argv[0],
			   buf, BUFFER_LEN, (mesgtyp | MPI_ISDEBUG));
    CHECKRETURN(ptr,"DEBUG","arg 1");
    return buf;
}


const char *
mfn_delay(MFUNARGS)
{
    char *argchr, *cmdchr;
    int i = atoi(argv[0]);
    if (i < 1) i = 1;
#ifdef WIZZED_DELAY
    if (!Mageperms(perms))
	ABORT_MPI("delay",NOPERM_MESG);
#endif
    cmdchr = get_mvar("cmd");
    argchr = get_mvar("arg");
    i = add_mpi_event(i, player, getloc(player), perms, argv[1], cmdchr, argchr,
		(mesgtyp & MPI_ISLISTENER), (!(mesgtyp & MPI_ISPRIVATE)));
    sprintf(buf, "%d", i);
    return buf;
}



const char *
mfn_kill(MFUNARGS)
{
    int i = atoi(argv[0]);
    if (i > 0) {
	if (in_timequeue(i)) {
	    if (!control_process(perms, i)) {
		ABORT_MPI("KILL",NOPERM_MESG);
	    }
	    i = dequeue_process(i);
	} else {
	    i = 0;
	}
    } else if (i == 0) {
	i = dequeue_prog(perms, 0);
    } else {
	ABORT_MPI("KILL","Invalid process ID");
    }
    sprintf(buf, "%d", i);
    return buf;
}



static int mpi_muf_call_levels = 0;

const char *
mfn_muf(MFUNARGS)
{
    char *ptr;
    struct inst *rv = NULL;
    dbref obj = mesg_dbref_raw(player, what, perms, argv[0]);

    if (obj == UNKNOWN)
	ABORT_MPI("MUF","Match failed");
    if (obj <= NOTHING || Typeof(obj) != TYPE_PROGRAM)
	ABORT_MPI("MUF","Bad program reference");
    if (!(FLAGS(obj) & LINK_OK) && !controls(perms,obj))
	ABORT_MPI("MUF",NOPERM_MESG);
    if ((mesgtyp & (MPI_ISLISTENER | MPI_ISLOCK)) && (MLevel(obj) < LM3))
	ABORT_MPI("MUF",NOPERM_MESG);

    if (++mpi_muf_call_levels > 18)
	ABORT_MPI("MUF","Too many call levels");

    strcpy(match_args, argv[1]);
    ptr = get_mvar("how");
    strcpy(match_cmdname, ptr);
    strcat(match_cmdname, "(MPI)");
    rv = interp(player, DBFETCH(player)->location,
		obj, perms, PREEMPT, STD_HARDUID, 1);

    mpi_muf_call_levels--;

    if (!rv) return "";
    switch(rv->type) {
	case PROG_STRING:
	    if (rv->data.string) {
		strcpy(buf, rv->data.string->data);
		CLEAR(rv);
		return buf;
	    } else {
		CLEAR(rv);
		return "";
	    }
	    break;
	case PROG_INTEGER:
	    sprintf(buf, "%d", rv->data.number);
	    CLEAR(rv);
	    return buf;
	    break;
	case PROG_OBJECT:
	    ptr = ref2str(rv->data.objref, buf);
	    CLEAR(rv);
	    return ptr;
	    break;
	default:
	    CLEAR(rv);
	    return "";
	    break;
    }

    return "";
}


const char *
mfn_force(MFUNARGS)
{
    char *nxt, *ptr;
    dbref obj = mesg_dbref_raw(player, what, perms, argv[0]);
    if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	ABORT_MPI("FORCE","Failed match. (1)");
    if (obj == PERMDENIED)
	ABORT_MPI("FORCE","Permission denied. (1)");
    if (Typeof(obj) != TYPE_THING && Typeof(obj) != TYPE_PLAYER)
	ABORT_MPI("FORCE","Bad object reference. (1)");
    if (!*argv[1])
	ABORT_MPI("FORCE","Null command string. (2)");
    if (!tp_zombies && !Archperms(perms))
	ABORT_MPI("FORCE",NOPERM_MESG);
    if (!Archperms(perms)) {
	const char *ptr = RNAME(obj);
	char objname[BUFFER_LEN], *ptr2;
	dbref loc = getloc(obj);

	if (Typeof(obj) == TYPE_THING) {
	    if (FLAGS(obj) & DARK)
		ABORT_MPI("FORCE","Cannot force a dark puppet");
	    if ((FLAGS(OWNER(obj)) & ZOMBIE))
		ABORT_MPI("FORCE",NOPERM_MESG);
	    if (loc != NOTHING && (FLAGS(loc) & ZOMBIE) &&
		    Typeof(loc) == TYPE_ROOM)
		ABORT_MPI("FORCE","Cannot force a Puppet in a no-puppets room");
	    for (ptr2 = objname; *ptr && !isspace(*ptr);)
		*(ptr2++) = *(ptr++);
	    *ptr2 = '\0';
	    if (lookup_player(objname) != NOTHING)
		ABORT_MPI("FORCE","Cannot force a thing named after a player");
	}
	if (!(FLAGS(obj) & XFORCIBLE)) {
	    ABORT_MPI("FORCE",NOPERM_MESG);
	}
	if (!test_lock_false_default(perms, obj, "@/flk")) {
	    ABORT_MPI("FORCE",NOPERM_MESG);
	}
    }
    if (Man(obj) && !TMan(perms))
	ABORT_MPI("FORCE","You can't force " NAMEMAN);
    if (!controls(OWNER(perms),obj))
	ABORT_MPI("FORCE",NOPERM_MESG);
    if ((WLevel(OWNER(perms)) < WLevel(obj)) ||
	(WLevel(perms) < WLevel(obj))
    )	ABORT_MPI("FORCE",NOPERM_MESG);
    if (force_level)
	ABORT_MPI("FORCE","You can't force recursively");
    strcpy(buf, argv[1]);
    ptr = buf;
    do {
	nxt = index(ptr, '\r');
	if (nxt) *nxt++ = '\0';
	force_level++;
	if (*ptr) process_command(obj, ptr, 0);
	force_level--;
	ptr = nxt;
    } while (ptr);
    *buf = '\0';
    return "";
}


const char *
mfn_midstr(MFUNARGS)
{
    int i, len = strlen(argv[0]);
    int pos1 = atoi(argv[1]);
    int pos2 = pos1;
    char *ptr = buf;

    if (argc > 2)
	pos2 = atoi(argv[2]);

    if (pos1 == 0) return "";
    if (pos1 > len) pos1 = len;
    if (pos1 < 0) pos1 += len + 1;
    if (pos1 < 1) pos1 = 1;

    if (pos2 == 0) return "";
    if (pos2 > len) pos2 = len;
    if (pos2 < 0) pos2 += len + 1;
    if (pos2 < 1) pos2 = 1;

    if (pos2 >= pos1) {
	for (i = pos1; i <= pos2; i++) *(ptr++) = argv[0][i-1];
    } else {
	for (i = pos1; i >= pos2; i--) *(ptr++) = argv[0][i-1];
    }
    *ptr = '\0';
    return buf;
}


const char *
mfn_instr(MFUNARGS)
{
    char *ptr;
    if (!*argv[1])
	ABORT_MPI("INSTR","Can't search for a null string");
    for (ptr = argv[0]; *ptr && !string_prefix(ptr, argv[1]); ptr++);
    if (!*ptr) return "0";
    sprintf(buf, "%d", (int)(ptr - argv[0] + 1));
    return buf;
}


const char *
mfn_lmember(MFUNARGS)
{
    /* {lmember:list,item,delim} */
    int i = 1;
    char *ptr = argv[0];
    int len;
    int len2 = strlen(argv[1]);

    if (argc < 3) strcpy(argv[2], "\r");
    if (!*argv[2])
	ABORT_MPI("LMEMBER","List delimiter cannot be a null string");
    len = strlen(argv[2]);
    while (*ptr && !(string_prefix(ptr, argv[1]) &&
	    (!ptr[len2] || string_prefix(ptr+len2, argv[2])))) {
	while (*ptr && !string_prefix(ptr, argv[2])) ptr++;
	if (*ptr) ptr += len;
	i++;
    }
    if (!*ptr) return "0";
    sprintf(buf, "%d", i);
    return buf;
}


const char *
mfn_tolower(MFUNARGS)
{
    char *ptr = argv[0];
    char *ptr2 = buf;
    while (*ptr) {
	if (isupper(*ptr)) {
	    *ptr2++ = tolower(*ptr++);
	} else {
	    *ptr2++ = *ptr++;
	}
    }
    *ptr2++ = '\0';
    return buf;
}


const char *
mfn_toupper(MFUNARGS)
{
    char *ptr = argv[0];
    char *ptr2 = buf;
    while (*ptr) {
	if (islower(*ptr)) {
	    *ptr2++ = toupper(*ptr++);
	} else {
	    *ptr2++ = *ptr++;
	}
    }
    *ptr2++ = '\0';
    return buf;
}


const char *
mfn_commas(MFUNARGS)
{
    int v, i, count;
    char *ptr;
    char buf2[BUFFER_LEN];
    char tmp[BUFFER_LEN];

    if (argc == 3)
	ABORT_MPI("COMMAS","Takes 1, 2, or 4 arguments");

    ptr = MesgParse(argv[0], argv[0]);
    CHECKRETURN(ptr,"COMMAS","arg 1");
    count = countlitems(argv[0], "\r");
    if (count == 0) return "";

    if (argc > 1) {
	ptr = MesgParse(argv[1], argv[1]);
	CHECKRETURN(ptr,"COMMAS","arg 2");
    } else {
	strcpy(argv[1], " and ");
    }

    if (argc > 2) {
	ptr = MesgParse(argv[2], buf2);
	CHECKRETURN(ptr,"COMMAS","arg 3");
	v = new_mvar(ptr, tmp);
	if (v == 1)
	    ABORT_MPI("COMMAS","Variable name too long");
	if (v == 2)
	    ABORT_MPI("COMMAS","Too many variables already defined");
    }

    *buf = '\0';
    for (i = 1; i <= count; i++) {
	ptr = getlitem(buf2, argv[0], "\r", i);
	if (argc > 2) {
	    strcpy(tmp, ptr);
	    ptr = MesgParse(argv[3], buf2);
	    CHECKRETURN(ptr,"COMMAS","arg 3");
	}
	strcat(buf, ptr);
	switch (count - i) {
	  case 0:
	    if (argc > 2) free_top_mvar();
	    return buf;
	    break;
	  case 1:
	    strcat(buf, argv[1]);
	    break;
	  default:
	    strcat(buf, ", ");
	    break;
	}
    }
    if (argc > 2) free_top_mvar();
    return buf;
}


const char *
mfn_dirprops(MFUNARGS)
{
    char propname[BUFFER_LEN];
    PropPtr propadr, pptr;
    char buf2[BUFFER_LEN];
    int list_limit = MAX_MFUN_LIST_LEN;
    dbref obj;
    int outlen, nextlen;

    if(argc > 1) {
	obj = mesg_dbref_local(player, what, perms, argv[1]);
	if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	    ABORT_MPI("DIRPROPS","Match failed");
	if (obj == PERMDENIED)
	    ABORT_MPI("DIRPROPS",NOPERM_MESG);
    } else obj = what;

    buf[0] = '\0';
    outlen = 0;

#ifdef DISKBASE
	fetchprops(obj);
#endif

    propadr = first_prop(obj, argv[0], &pptr, propname);
    while ((propadr > 0) && *propname) {
	    if ( ( !Prop_Hidden(propname) && !(PropFlags(propadr) & PROP_SYSPERMS))
		    || (Permlevel(perms) >= tp_hidden_prop_mlevel)) {
		sprintf(buf2, "%s", propname);
		nextlen = strlen(buf2);
		if ((outlen + nextlen) >= (BUFFER_LEN - 3))
		    break;
		if (outlen) strcat((buf+(outlen++)), "\r");
		strcat((buf + outlen), buf2);
		outlen += nextlen;
		list_limit--;
	    }
	propadr = next_prop(pptr, propadr, propname);
    }

    return buf;
}


const char *
mfn_showlist(MFUNARGS)
{
    dbref obj;
    const char *m = NULL;
    int lines=0;

    if(argc > 1) {
	obj = mesg_dbref_mage(player, what, perms, argv[1]);
	if (obj == AMBIGUOUS || obj == UNKNOWN || obj == NOTHING || obj == HOME)
	    ABORT_MPI("SHOWLIST","Match failed");
	if (obj == PERMDENIED)
	    ABORT_MPI("SHOWLIST",NOPERM_MESG);
    } else obj = what;

    if (Prop_Hidden(argv[0]) && (Permlevel(perms) < tp_hidden_prop_mlevel))
	ABORT_MPI("SHOWLIST",NOPERM_MESG);
    while ( (lines < MAX_MFUN_LIST_LEN) && (!lines || (m && *m)) ) {
	sprintf(buf, "%s#/%d", argv[0], ++lines);
	m = safegetprop_strict(player, obj, perms, buf);
	if( m && *m ) {
	    notify_nolisten(player, m, 1);
	}
    }
    buf[0] = '\0';
    return buf;
}

#endif /* MPI */
