#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "db.h"
#include "tune.h"
#include "mpi.h"
#include "props.h"
#include "match.h"
#include "interp.h"
#include "interface.h"
#include "msgparse.h"
#include "mfun.h"
#include "externs.h"


#ifdef MPI

int
Permlevel(dbref what)
{
    int mlev, omlev;
    
    if(!OkObj(what))
	return 0;

    mlev = MLevel(what);
    omlev = MLevel(OWNER(what));

    return (mlev <= omlev) ? mlev : omlev;
}

int
Archperms(dbref what)
{
    if(tp_compatible_mpi)
	return Mageperms(what);

    return ((Permlevel(what) >= LARCH)
	&& (Typeof(what) != TYPE_PLAYER) && (Typeof(what) != TYPE_PROGRAM)
    ) ? 1 : 0;
}


int
Wizperms(dbref what)
{
    if(tp_compatible_mpi)
	return Mageperms(what);

    return ((Permlevel(what) >= LWIZ)
	&& (Typeof(what) != TYPE_PLAYER) && (Typeof(what) != TYPE_PROGRAM)
    ) ? 1 : 0;
}


int
Mageperms(dbref what)
{
    return ((Permlevel(what) >= LMAGE)
	&& (Typeof(what) != TYPE_PLAYER) && (Typeof(what) != TYPE_PROGRAM)
    ) ? 1 : 0;
}


int
safeputprop(dbref obj, dbref perms, char *buf, char *val)
{
    char *ptr;
    if (!buf) return 0;
    while (*buf == PROPDIR_DELIMITER) buf++;
    if (!*buf) return 0;
    if (tp_db_readonly) return 0;

    /* disallow CR's and :'s in prop names. */
    for (ptr = buf; *ptr; ptr++)
	if (*ptr == '\r' || *ptr == PROP_DELIMITER) return 0;

    if (Permlevel(perms) < tp_hidden_prop_mlevel) {
	if (Prop_Hidden(buf)) return 0;
    } else if(
	PropDir_Check(buf, PROP_HIDDEN, PROP_ALIAS) ||
	PropDir_Check(buf, PROP_HIDDEN, PROP_GLOW )
    )	return 0;

    if (Permlevel(perms) < tp_seeonly_prop_mlevel) {
	if (Prop_SeeOnly(buf)) return 0;
    }

    if ((!Mageperms(perms)) && (
	string_prefix(buf, "_msgmacs/") || string_prefix(buf, "_defs/")
    )) return 0;

    if (!val) {
	remove_property(obj, buf);
    } else {
	add_property(obj, buf, val, 0);
    }
    return 1;
}


const char *
safegetprop_strict(dbref player, dbref what, dbref perms, const char *inbuf)
{
    const char *ptr;
    char  bbuf[BUFFER_LEN];
    static char  vl[32];

    if (!inbuf) {
	notify_nolisten(player, "PropFetch: Propname required.", 1);
	return NULL;
    }
    while (*inbuf == PROPDIR_DELIMITER)
	inbuf++;
    if (!*inbuf) {
	notify_nolisten(player, "PropFetch: Propname required.", 1);
	return NULL;
    }
    strcpy(bbuf, inbuf);
    if (Permlevel(perms) < tp_hidden_prop_mlevel) {
	if (Prop_Hidden(bbuf)) {
	    notify_nolisten(player, "PropFetch: Permission denied.", 1);
	    return NULL;
	}
    }
    if (Permlevel(perms) < tp_private_prop_mlevel) {
	if (Prop_Private(bbuf) && OWNER(perms) != OWNER(what)) {
	    notify_nolisten(player, "PropFetch: Permission denied.", 1);
	    return NULL;
	}
    }
    ptr = get_property_class(what, bbuf);
    if (!ptr) {
	int i;

	i = get_property_value(what, bbuf);
	if (!i) {
	    dbref dd;

	    dd = get_property_dbref(what, bbuf);
	    if (dd == NOTHING) {
		*vl = '\0';
		ptr = vl;
		return ptr;
	    } else {
		sprintf(vl, "#%d", dd);
		ptr = vl;
	    }
	} else {
	    sprintf(vl, "%d", i);
	    ptr = vl;
	}
    }

#ifdef COMPRESS
    ptr = uncompress(ptr);
#endif
    return ptr;
}


const char *
safegetprop_limited(dbref player, dbref what, dbref whom, dbref perms, const char *inbuf)
{
    const char *ptr;

    while (what != NOTHING) {
	if ((OWNER(what) == whom) || Mageperms(what) ||
	    safegetprop_strict(player, what, perms, "~mpi_macros_ok")
	) {
	    ptr = safegetprop_strict(player, what, perms, inbuf);
	    if (!ptr || *ptr) return ptr;
	}
        what = getparent(what);
    }
    return "";
}


const char *
safegetprop(dbref player, dbref what, dbref perms, const char *inbuf)
{
    const char *ptr;

    while (what != NOTHING) {
	ptr = safegetprop_strict(player, what, perms, inbuf);
	if (!ptr || *ptr) return ptr;
	what = getparent(what);
    }
    return "";
}


char *
stripspaces(char *buf, char *in)
{
    char *ptr;
    for (ptr = in; *ptr == ' '; ptr++);
    strcpy(buf, ptr);
    ptr = strlen(buf) + buf - 1;
    while (*ptr == ' ' && ptr > buf)
	*(ptr--) = '\0';
    return buf;
}


char *
string_substitute(const char *str, const char *oldstr, const char *newstr, char *buf, int maxlen)
{
    const char *ptr = str;
    char *ptr2 = buf;
    const char *ptr3;
    int len = strlen(oldstr);

    if (len == 0) {
	strcpy(buf, str);
	return buf;
    }
    while (*ptr) {
	if (!strncmp(ptr, oldstr, len)) {
	    for(ptr3 = newstr; ((ptr2 - buf) < (maxlen - 2)) && *ptr3;)
		*(ptr2++) = *(ptr3++);
	    ptr += len;
	} else {
	    *(ptr2++) = *(ptr++);
	}
    }
    *ptr2 = '\0';
    return buf;
}


const char *
get_list_item(dbref player, dbref what, dbref perms, const char *listname, int itemnum)
{
    char    buf[BUFFER_LEN];
    const char *ptr;

    sprintf(buf, "%.512s#/%d", listname, itemnum);
    ptr = safegetprop(player, what, perms, buf);
    if (!ptr || *ptr) return ptr;

    sprintf(buf, "%.512s/%d", listname, itemnum);
    ptr = safegetprop(player, what, perms, buf);
    if (!ptr || *ptr) return ptr;

    sprintf(buf, "%.512s%d", listname, itemnum);
    return (safegetprop(player, what, perms, buf));
}

time_t mpi_prof_start_time;

int
get_list_count(dbref player, dbref obj, dbref perms, const char *listname)
{
    char    buf[BUFFER_LEN];
    const char *ptr;
    int i;

    sprintf(buf, "%.512s#", listname);
    ptr = safegetprop(player, obj, perms, buf);
    if (ptr && *ptr) return (atoi(ptr));

    sprintf(buf, "%.512s/#", listname);
    ptr = safegetprop(player, obj, perms, buf);
    if (ptr && *ptr) return (atoi(ptr));

    for (i = 1; i < MAX_MFUN_LIST_LEN; i++) {
	ptr = get_list_item(player, obj, perms, listname, i);
	if (!ptr) return 0;
	if (!*ptr) break;
    }
    if (i-- < MAX_MFUN_LIST_LEN) return i;
    return MAX_MFUN_LIST_LEN;
}



char   *
get_concat_list(dbref player, dbref what, dbref perms, dbref obj, const char *listname, char *buf, int maxchars, int mode)
{
    int line_limit = MAX_MFUN_LIST_LEN;
    int     i;
    const char *ptr;
    char   *pos = buf;
    int     cnt = get_list_count(what, obj, perms, listname);

    if (cnt == 0) {
	return NULL;
    }
    maxchars -= 2;
    *buf = '\0';
    for (i = 1; ((pos - buf) < (maxchars - 1)) && i <= cnt && line_limit--; i++) {
	ptr = get_list_item(what, obj, perms, listname, i);
	if (ptr) {
	    while (mode && isspace(*ptr))
		ptr++;
	    if (pos > buf) {
		if (!mode) {
		    *(pos++) = '\r';
		    *pos = '\0';
		} else if (mode == 1) {
		    char    ch = *(pos - 1);

		    if ((pos - buf) >= (maxchars - 2))
			break;
		    if (ch == '.' || ch == '?' || ch == '!')
			*(pos++) = ' ';
		    *(pos++) = ' ';
		    *pos = '\0';
		} else {
		    *pos = '\0';
		}
	    }
	    while (((pos - buf) < (maxchars - 1)) && *ptr)
		*(pos++) = *(ptr++);
	    if (mode) {
		while (pos > buf && *(pos - 1) == ' ') pos--;
	    }
	    *pos = '\0';
	    if ((pos - buf) >= (maxchars - 1))
		break;
	}
    }
    return (buf);
}


int
mesg_read_perms(dbref player, dbref perms, dbref obj)
{
    if ((obj == 0) || (obj == player) || (obj == perms)) return 1;
    if (OWNER(perms) == OWNER(obj)) return 1;
    if (Mageperms(perms)) return 1;
    return 0;
}


int
isneighbor(dbref d1, dbref d2)
{
    if (d1 == d2) return 1;
    if (Typeof(d1) != TYPE_ROOM)
	if (getloc(d1) == d2) return 1;
    if (Typeof(d2) != TYPE_ROOM)
	if (getloc(d2) == d1) return 1;
    if (Typeof(d1) != TYPE_ROOM && Typeof(d2) != TYPE_ROOM)
	if (getloc(d1) == getloc(d2)) return 1;
    return 0;
}


int
mesg_local_perms(dbref player, dbref perms, dbref obj)
{
    dbref loc, oloc;

    if((loc = getloc(player)) == NOTHING) /* Player is in limbo */
	return 0;

    if(loc == obj) /* Player is in object */
	return 1;

    if((oloc = getloc(obj)) == NOTHING) /* Object is in limbo, ie #0 */
	return 0;

    if (OWNER(perms) == OWNER(oloc)) return 1;
    if (isneighbor(perms, obj)) return 1;
    if (isneighbor(player, obj)) return 1;

    if (Mageperms(perms)) return 1;
    if (mesg_read_perms(player, perms, obj)) return 1;

    if(!tp_compatible_mpi) {
	/* See if player is in a non-room object local to the room */
	/* the object is in. */

	loc = player;
	while(OkObj(loc) && (Typeof(loc) != TYPE_ROOM)) {
	    loc = getloc(loc);
	    if(OkObj(loc))
		if(isneighbor(loc, obj)) return 1;
	}
    }

    return 0;
}


dbref
mesg_dbref_raw(dbref player, dbref what, dbref perms, const char *buf)
{
    struct match_data md;
    dbref   obj = UNKNOWN;

    if (buf && *buf) {
	if (!string_compare(buf, "this")) {
	    obj = what;
	} else if (!string_compare(buf, "me")) {
	    obj = player;
	} else if (!string_compare(buf, "here")) {
	    obj = getloc(player);
	} else if (!string_compare(buf, "home")) {
	    obj = HOME;
	} else {
	    init_match(player, buf, NOTYPE, &md);
	    match_all_exits(&md);
	    match_neighbor(&md);
	    match_possession(&md);
	    match_registered(&md);
	    match_absolute(&md);
	    obj = match_result(&md);
	    if (obj == NOTHING) {
		init_match_remote(player, what, buf, NOTYPE, &md);
		match_possession(&md);
		match_neighbor(&md);
		match_all_exits(&md);
		match_player(&md);
		match_registered(&md);
		obj = match_result(&md);
	    }
	}
    }

    if ((!OkObj(obj)) || Typeof(obj) == TYPE_GARBAGE)
	obj = UNKNOWN;
    return obj;
}


dbref
mesg_dbref(dbref player, dbref what, dbref perms, char *buf)
{
    dbref obj = mesg_dbref_raw(player, what, perms, buf);

    if (obj == UNKNOWN) return obj;
    if (!mesg_read_perms(player, perms, obj)) {
	obj = PERMDENIED;
    }
    return obj;
}


dbref
mesg_dbref_mage(dbref player, dbref what, dbref perms, char *buf)
{
    dbref obj = mesg_dbref_raw(player, what, perms, buf);

    if (obj == UNKNOWN) return obj;
    if (!Mageperms(perms) && OWNER(perms) != OWNER(obj)) {
	obj = PERMDENIED;
    }
    return obj;
}


dbref
mesg_dbref_strict(dbref player, dbref what, dbref perms, char *buf)
{
    dbref obj = mesg_dbref_raw(player, what, perms, buf);

    if (obj == UNKNOWN) return obj;
    if (!Mageperms(perms) && OWNER(perms) != OWNER(obj)) {
	obj = PERMDENIED;
    }
    return obj;
}


dbref
mesg_dbref_local(dbref player, dbref what, dbref perms, char *buf)
{
    dbref obj = mesg_dbref_raw(player, what, perms, buf);

    if (obj == UNKNOWN) return obj;
    if (!mesg_local_perms(player, perms, obj)) {
	obj = PERMDENIED;
    }
    return obj;
}


char *
ref2str(dbref obj, char *buf)
{
    if ((obj < -3) || (obj >= db_top)) {
	sprintf(buf, "Bad");
	return buf;
    }

    if ((obj >= 0) && (Typeof(obj) == TYPE_PLAYER)) {
	sprintf(buf, "*%s", RNAME(obj));
    } else {
	sprintf(buf, "#%d", obj);
    }
    return buf;
}


int
truestr(char *buf)
{
    while (isspace(*buf)) buf++;
    if (!*buf || (number(buf) && !atoi(buf)))
	return 0;
    return 1;
}


/******** MPI Variable handling ********/

struct mpivar {
    char name[MAX_MFUN_NAME_LEN + 1];
    char *buf;
};

static struct mpivar varv[MPI_MAX_VARIABLES];
int varc = 0;

int
new_mvar(const char *varname, char *buf)
{
    if (strlen(varname) > MAX_MFUN_NAME_LEN) return 1;
    if (varc > MPI_MAX_VARIABLES) return 2;
    strcpy(varv[varc].name, varname);
    varv[varc++].buf = buf;
    return 0;
}

char *
get_mvar(const char *varname)
{
    int i = 0;
    for (i = varc-1; i >= 0 && string_compare(varname, varv[i].name); i--);
    if (i < 0) return NULL;
    return varv[i].buf;
}

int
free_top_mvar()
{
    if (varc < 1) return 1;
    varc--;
    return 0;
}


/***** MPI function handling *****/


struct mpifunc {
    char name[MAX_MFUN_NAME_LEN + 1];
    char *buf;
};

static struct mpifunc funcv[MPI_MAX_FUNCTIONS];
int funcc = 0;

int
new_mfunc(const char *funcname, const char *buf)
{
    if (strlen(funcname) > MAX_MFUN_NAME_LEN) return 1;
    if (funcc > MPI_MAX_FUNCTIONS) return 2;
    strcpy(funcv[funcc].name, funcname);
    funcv[funcc++].buf = (char *)string_dup(buf);
    return 0;
}

const char *
get_mfunc(const char *funcname)
{
    int i = 0;
    for (i = funcc-1; i >= 0 && string_compare(funcname, funcv[i].name); i--);
    if (i < 0) return NULL;
    return funcv[i].buf;
}

int
free_mfuncs(int downto)
{
    if (funcc < 1) return 1;
    if (downto < 0) return 1;
    while (funcc > downto)
	free(funcv[--funcc].buf);
    return 0;
}



/*** End of MFUNs. ***/

int
msg_is_macro(dbref player, dbref what, dbref perms, const char *name)
{
    const char   *ptr;
    char    buf2[BUFFER_LEN];
    dbref   obj;

    if (!*name) return 0;
    sprintf(buf2, "_msgmacs/%s", name);
    obj = what;
    ptr = get_mfunc(name);
    if (!ptr || !*ptr) ptr = safegetprop_strict(player, OWNER(obj), perms, buf2);
    if (!ptr || !*ptr) ptr = safegetprop_limited(player, obj, OWNER(obj), perms, buf2);
    if (!ptr || !*ptr) ptr = safegetprop_strict(player, 0, perms, buf2);
    if (!ptr || !*ptr) return 0;
    return 1;
}


void
msg_unparse_macro(dbref player, dbref what, dbref perms, char *name, int argc, argv_typ argv, char *rest, int maxchars)
{
    const char   *ptr;
    char *ptr2;
    char    buf[BUFFER_LEN];
    char    buf2[BUFFER_LEN];
    dbref   obj;
    int     i, p = 0;

    strcpy(buf, rest);
    sprintf(buf2, "_msgmacs/%s", name);
    obj = what;
    ptr = get_mfunc(name);
    if (!ptr || !*ptr) ptr = safegetprop_strict(player, OWNER(obj), perms, buf2);
    if (!ptr || !*ptr) ptr = safegetprop_limited(player, obj, OWNER(obj), perms, buf2);
    if (!ptr || !*ptr) ptr = safegetprop_strict(player, 0, perms, buf2);
    while (ptr && *ptr && p < (maxchars - 1)) {
	if (*ptr == '\\') {
	    if (*(ptr+1) == 'r') {
		rest[p++] = '\r';
		ptr++; ptr++;
	    } else {
		rest[p++] = *(ptr++);
		rest[p++] = *(ptr++);
	    }
	} else if (*ptr == MFUN_LEADCHAR) {
	    if (*(ptr+1) == MFUN_ARGSTART && isdigit(*(ptr + 2)) &&
		    *(ptr + 3) == MFUN_ARGEND) {
		ptr++; ptr++;
		i = *(ptr++) - '1';
		ptr++;
		if (i >= argc || i < 0) {
		    ptr2 = NULL;
		} else {
		    ptr2 = argv[i];
		}
		while (ptr2 && *ptr2 && p < (maxchars - 1)) {
		    rest[p++] = *(ptr2++);
		}
	    } else {
		rest[p++] = *(ptr++);
	    }
	} else {
	    rest[p++] = *(ptr++);
	}
    }
    ptr2 = buf;
    while (ptr2 && *ptr2 && p < (maxchars - 1)) {
	rest[p++] = *(ptr2++);
    }
    rest[p] = '\0';
}


#ifndef MSGHASHSIZE
#define MSGHASHSIZE 256
#endif

static hash_tab msghash[MSGHASHSIZE];

int
find_mfn(const char *name)
{
    hash_data *exp = find_hash(name, msghash, MSGHASHSIZE);

    if (exp)
	return (exp->ival);
    return (0);
}


void
insert_mfn(const char *name, int i)
{
    hash_data hd;

    (void) kill_def(name);
    hd.ival = i;
    (void) add_hash(name, hd, msghash, MSGHASHSIZE);
}


void
purge_mfns(void)
{
    kill_hash(msghash, MSGHASHSIZE, 0);
}



#include "mfunlist.h"

/******** HOOK ********/
void
mesg_init(void)
{
    int i;

    for (i = 0; mfun_list[i].name; i++)
	insert_mfn(mfun_list[i].name, i + 1);
    mpi_prof_start_time = current_systime;
}



/******** HOOK ********/
int
mesg_args(char *wbuf, argv_typ argv, char ulv, char sep, char dlv, char quot, int maxargs)
{
    int     r, lev, argc = 0;
    char    buf[BUFFER_LEN];
    char   *ptr;
    int     litflag = 0;

    /* for (ptr = wbuf; ptr && isspace(*ptr); ptr++); */
    strcpy(buf, wbuf);
    ptr = buf;
    for (lev = r = 0; (r < (BUFFER_LEN - 2)); r++) {
	if (buf[r] == '\0') {
	    return (-1);
	} else if (buf[r] == '\\') {
	    r++;
	    continue;
	} else if (buf[r] == quot) {
	    litflag = (!litflag);
	} else if (!litflag && buf[r] == ulv) {
	    lev++;
	    continue;
	} else if (!litflag && buf[r] == dlv) {
	    lev--;
	    if (lev < 0) {
		buf[r] = '\0';
		strcpy(argv[argc++], ptr);
		ptr = buf + r + 1;
		break;
	    }
	    continue;
	} else if (!litflag && lev < 1 && buf[r] == sep) {
	    if (argc < maxargs - 1) {
		buf[r] = '\0';
		strcpy(argv[argc++], ptr);
		ptr = buf + r + 1;
	    }
	}
    }
    buf[BUFFER_LEN - 1] = '\0';
    strcpy(wbuf, ptr);
    return argc;
}


char *
cr2slash(char *buf, const char *in)
{
    char *ptr = buf;
    const char *ptr2 = in;
    do {
	if (*ptr2 == '\r') {
	    ptr2++;
	    *(ptr++) = '\\';
	    *(ptr++) = 'r';
	} else if (*ptr2 == '\\') {
	    ptr2++;
	    *(ptr++) = '\\';
	    *(ptr++) = '\\';
	} else {
	    *(ptr++) = *(ptr2++);
	}
    } while (*(ptr - 1) && (ptr - buf < BUFFER_LEN - 3));
    buf[BUFFER_LEN - 1] = '\0';
    return buf;
}


static int mesg_rec_cnt = 0;
static int mesg_instr_cnt = 0;


/******** HOOK ********/
char   *
mesg_parse(dbref player, dbref what, dbref perms, const char *inbuf, char *outbuf, int maxchars, int mesgtyp)
{
    char    *mem;
    char    *wbuf; /* [BUFFER_LEN] */
    char    *buf; /* [BUFFER_LEN] */
    char    *buf2; /* [BUFFER_LEN] */
    char    *dbuf; /* [BUFFER_LEN] */
    char    *ebuf; /* [BUFFER_LEN] */
    char    *argv[9]; /* [BUFFER_LEN] */
    const char *ptr;
    char *dptr;
    int     p, q, s, inlen;
    int i;
    int     argc;
    int showtextflag = 0;
    int literalflag = 0;
    char cmdbuf[MAX_MFUN_NAME_LEN + 1];

    mesg_rec_cnt++;
    if (mesg_rec_cnt > MAX_MPI_LEVELS) {
	mesg_rec_cnt--;
	strncpy(outbuf, inbuf, maxchars);
	outbuf[maxchars - 1] = '\0';
	return outbuf;
    }
    if (Typeof(player) == TYPE_GARBAGE) {
	return NULL;
    }
    if (Typeof(what) == TYPE_GARBAGE) {
	notify_nolisten(player, "MPI Error: Garbage trigger.", 1);
	return NULL;
    }

    mem = malloc(((5 + 9) * BUFFER_LEN) + (MAX_MFUN_NAME_LEN + 1));
    if(!mem)
    	return NULL;
    wbuf = mem + (BUFFER_LEN * 0);
    buf = mem + (BUFFER_LEN * 1);
    buf2 = mem + (BUFFER_LEN * 2);
    dbuf = mem + (BUFFER_LEN * 3);
    ebuf = mem + (BUFFER_LEN * 4);
    for(i = 0; i < 9; i++)
	argv[i] = mem + (BUFFER_LEN * (5 + i));

    strcpy(wbuf, inbuf);

    /* Bugfix: At 'unknown substitution', p can == end of input string */
    /* I could have done a check of wbuf[p] before allowing the p++    */
    /* that occurs at the end of the for() construct, but this will    */
    /* catch ALL cases where p could increment past the end of the     */
    /* input string. */
    inlen = strlen(wbuf);

    for (p = q = 0; (p < inlen) && wbuf[p] && (p < maxchars - 1) && (q < (maxchars - 1)); p++) {
	if (wbuf[p] == '\\') {
	    p++;
	    showtextflag = 1;
	    if (wbuf[p] == 'r') {
		outbuf[q++] = '\r';
	    } else {
		outbuf[q++] = wbuf[p];
	    }
	} else if (wbuf[p] == MFUN_LITCHAR) {
	    literalflag = (!literalflag);
	} else if (!literalflag && wbuf[p] == MFUN_LEADCHAR) {
	    if (wbuf[p + 1] == MFUN_LEADCHAR) {
		showtextflag = 1;
		outbuf[q++] = wbuf[p++];
		ptr = "";
	    } else {
		ptr = wbuf + (++p);
		s = 0;
		while (wbuf[p] && wbuf[p] != MFUN_LEADCHAR &&
			!isspace(wbuf[p]) && wbuf[p] != MFUN_ARGSTART &&
			wbuf[p] != MFUN_ARGEND && s < MAX_MFUN_NAME_LEN) {
		    p++; s++;
		}
		if (s < MAX_MFUN_NAME_LEN &&
			(wbuf[p] == MFUN_ARGSTART || wbuf[p] == MFUN_ARGEND)) {
		    int varflag;

		    strncpy(cmdbuf,ptr,s);
		    cmdbuf[s] = '\0';

		    varflag = 0;
		    if (*cmdbuf == '&') {
			s = find_mfn("sublist");
			varflag = 1;
		    } else if (*cmdbuf) {
			s = find_mfn(cmdbuf);
		    } else {
			s = 0;
		    }
		    if (s) {
			s--;
			if (++mesg_instr_cnt > tp_mpi_max_commands) {
			    char *zptr = get_mvar("how");
			    sprintf(dbuf, "%s %c%s%c: Instruction limit exceeded.",
				    zptr, MFUN_LEADCHAR,
				    (varflag? cmdbuf : mfun_list[s].name),
				    MFUN_ARGEND);
			    notify_nolisten(player, dbuf, 1);
			    if(mem) free(mem);
			    return NULL;
			}
			if (wbuf[p] == MFUN_ARGEND) {
			    argc = 0;
			} else {
			    argc = mfun_list[s].maxargs;
			    if (argc < 0) {
				argc = mesg_args((wbuf+p+1),
					&argv[(varflag? 1 : 0)],
					MFUN_LEADCHAR, MFUN_ARGSEP,
					MFUN_ARGEND, MFUN_LITCHAR,
					(-argc) + (varflag? 1 : 0));
			    } else {
				argc = mesg_args((wbuf+p+1),
					&argv[(varflag? 1 : 0)],
					MFUN_LEADCHAR, MFUN_ARGSEP,
					MFUN_ARGEND, MFUN_LITCHAR,
					(varflag? 8 : 9));
			    }
			    if (argc == -1) {
				char *zptr = get_mvar("how");
				sprintf(ebuf, "%s %c%s%c: End brace not found.",
					zptr, MFUN_LEADCHAR, cmdbuf,
					MFUN_ARGEND);
				notify_nolisten(player, ebuf, 1);
				if(mem) free(mem);
				return NULL;
			    }
			}
			if (varflag) {
			    char dbuf[20], *zptr = dbuf;

			    argc++;
			    dbuf[0] = '\0';
			    if(!string_compare(cmdbuf + 1, "player"))
				sprintf(dbuf, "#%d", player);
			    else if(!string_compare(cmdbuf + 1, "what"))
				sprintf(dbuf, "#%d", what);
			    else if(!string_compare(cmdbuf + 1, "perms"))
				sprintf(dbuf, "#%d", perms);
			    else zptr = get_mvar(cmdbuf + 1);
			    if (!zptr) {
				zptr = get_mvar("how");
				sprintf(ebuf, "%s %c%s%c: Unrecognized variable.",
					zptr, MFUN_LEADCHAR, cmdbuf, MFUN_ARGEND);
				notify_nolisten(player, ebuf, 1);
				if(mem) free(mem);
				return NULL;
			    }
			    strcpy(argv[0], zptr);
			}
			if (mesgtyp & MPI_ISDEBUG) {
			    char *zptr = get_mvar("how");
			    sprintf(dbuf, "%s %*s%c%s%c", zptr,
				    (mesg_rec_cnt*2-4), "", MFUN_LEADCHAR,
				    (varflag? cmdbuf : mfun_list[s].name),
				    MFUN_ARGSTART);
			    for (i = (varflag? 1 : 0); i < argc; i++) {
				if (i) {
				    dbuf[512] = '\0';
				    sprintf(dbuf + strlen(dbuf), "%c ",
					    MFUN_ARGSEP);
				}
				cr2slash(ebuf, argv[i]);
				if (strlen(ebuf) > 512) {
				    dbuf[512] = '\0';
				    ebuf[512] = '\0';
				    sprintf(dbuf + strlen(dbuf), "\"%s...\"",
					    ebuf);
				} else {
				    dbuf[512] = '\0';
				    sprintf(dbuf + strlen(dbuf), "\"%s\"", ebuf);
				}
			    }
			    dbuf[512] = '\0';
			    sprintf(dbuf + strlen(dbuf), "%c", MFUN_ARGEND);
			    notify_nolisten(player, dbuf, 1);
			}
			if (mfun_list[s].stripp) {
			    for (i = (varflag? 1 : 0); i < argc; i++) {
				strcpy(argv[i], stripspaces(buf, argv[i]));
			    }
			}
			if (mfun_list[s].parsep) {
			    for (i = (varflag? 1 : 0); i < argc; i++) {
				ptr = MesgParse(argv[i], argv[i]);
				if (!ptr) {
				    char *zptr = get_mvar("how");
				    sprintf(dbuf, "%s %c%s%c (arg %d)", zptr,
					    MFUN_LEADCHAR,
					    (varflag?cmdbuf:mfun_list[s].name),
					    MFUN_ARGEND, i+1);
				    notify_nolisten(player, dbuf, 1);
				    if(mem) free(mem);
				    return NULL;
				}
			    }
			}
			if (mesgtyp & MPI_ISDEBUG) {
			    char *zptr = get_mvar("how");
			    sprintf(dbuf, "%.512s %*s%c%.512s%c", zptr,
				    (mesg_rec_cnt*2-4), "", MFUN_LEADCHAR,
				    (varflag? cmdbuf : mfun_list[s].name),
				    MFUN_ARGSTART);
			    for (i = (varflag? 1 : 0); i < argc; i++) {
				if (i) {
				    dbuf[512] = '\0';
				    sprintf(dbuf + strlen(dbuf), "%c ",
					    MFUN_ARGSEP);
				}
				cr2slash(ebuf, argv[i]);
				if (strlen(ebuf) > 128) {
				    dbuf[512] = '\0';
				    ebuf[128] = '\0';
				    sprintf(dbuf, "\"%s...\"",
					    ebuf);
				} else {
				    dbuf[512] = '\0';
				    sprintf(dbuf + strlen(dbuf), "\"%s\"",
					    ebuf);
				}
			    }
			    sprintf(dbuf + strlen(dbuf), "%c", MFUN_ARGEND);
			}
			if (argc < mfun_list[s].minargs) {
			    char *zptr = get_mvar("how");
			    sprintf(ebuf, "%s %c%s%c: Too few arguments",
				    zptr, MFUN_LEADCHAR,
				    (varflag? cmdbuf : mfun_list[s].name),
				    MFUN_ARGEND);
			    notify_nolisten(player, ebuf, 1);
			    if(mem) free(mem);
			    return NULL;
			} else if (mfun_list[s].maxargs > 0 &&
				    argc > mfun_list[s].maxargs) {
			    char *zptr = get_mvar("how");
			    sprintf(ebuf, "%s %c%s%c: Too many arguments",
				    zptr, MFUN_LEADCHAR,
				    (varflag? cmdbuf : mfun_list[s].name),
				    MFUN_ARGEND);
			    notify_nolisten(player, ebuf, 1);
			    if(mem) free(mem);
			    return NULL;
			} else {
			    ptr = mfun_list[s].mfn(player, what, perms, argc,
						   argv, buf, mesgtyp);
			    if (!ptr) {
				outbuf[q] = '\0';
				if(mem) free(mem);
				return NULL;
			    }
			    if (mfun_list[s].postp) {
				dptr = MesgParse(ptr, buf);
				if (!dptr) {
				    char *zptr = get_mvar("how");
				    sprintf(ebuf, "%s %c%s%c (returned string)",
					    zptr, MFUN_LEADCHAR,
					    (varflag?cmdbuf:mfun_list[s].name),
					    MFUN_ARGEND);
				    notify_nolisten(player, ebuf, 1);
				    if(mem) free(mem);
				    return NULL;
				}
				ptr = dptr;
			    }
			}
			if (mesgtyp & MPI_ISDEBUG) {
			    dbuf[512] = '\0';
			    cr2slash(ebuf, ptr);
			    ebuf[512] = '\0';
			    sprintf(dbuf + strlen(dbuf), " = \"%s\"",
				    ebuf);
			    notify_nolisten(player, dbuf, 1);
			}
		    } else if (msg_is_macro(player, what, perms, cmdbuf)) {
			if (wbuf[p] == MFUN_ARGEND) {
			    argc = 0;
			    p++;
			} else {
			    p++;
			    argc = mesg_args(wbuf+p, argv, MFUN_LEADCHAR,
				 MFUN_ARGSEP, MFUN_ARGEND, MFUN_LITCHAR, 9);
			    if (argc == -1) {
				char *zptr = get_mvar("how");
				sprintf(ebuf, "%s %c%s%c: End brace not found.",
					zptr, MFUN_LEADCHAR, cmdbuf,
					MFUN_ARGEND);
				notify_nolisten(player, ebuf, 1);
				if(mem) free(mem);
				return NULL;
			    }
			}
			msg_unparse_macro(player, what, perms, cmdbuf, argc,
					    argv, (wbuf+p), (BUFFER_LEN - p));
			p--;
			ptr = NULL;
		    } else {
			/* unknown function */
			char *zptr = get_mvar("how");
			sprintf(ebuf, "%s %c%s%c: Unrecognized function.",
				zptr, MFUN_LEADCHAR, cmdbuf, MFUN_ARGEND);
			notify_nolisten(player, ebuf, 1);
			if(mem) free(mem);
			return NULL;
		    }
		} else {
		    showtextflag = 1;
		    p = (int) (ptr - wbuf);
		    if (q < (maxchars - 1))
			outbuf[q++] = MFUN_LEADCHAR;
		    ptr = "";   /* unknown substitution type */
		}
		while (ptr && *ptr && q < (maxchars -1))
		    outbuf[q++] = *(ptr++);
	    }
	} else {
	    outbuf[q++] = wbuf[p];
	    showtextflag = 1;
	}
    }
    outbuf[q] = '\0';
    if ((mesgtyp & MPI_ISDEBUG) && showtextflag) {
	char *zptr = get_mvar("how");
	sprintf(dbuf, "%s %*s\"%s\"", zptr, (mesg_rec_cnt*2-4), "",
		cr2slash(buf2, outbuf));
	notify_nolisten(player, dbuf, 1);
    }
    mesg_rec_cnt--;
    if(mem) free(mem);
    return (outbuf);
}

char   *
do_parse_mesg_2(dbref player, dbref what, dbref perms, const char *inbuf, const char *abuf, char *outbuf, int mesgtyp)
{
    char *mem;
    char *howvar; /* [BUFFER_LEN] */
    char *cmdvar; /* [BUFFER_LEN] */
    char *argvar; /* [BUFFER_LEN] */
    char *tmparg; /* [BUFFER_LEN] */
    char *tmpcmd; /* [BUFFER_LEN] */
    char *dptr;
    int  mvarcnt = varc;
    int  mfunccnt = funcc;
    int  tmprec_cnt = mesg_rec_cnt;
    int  tmpinst_cnt = mesg_instr_cnt;

#ifdef COMPRESS
    abuf = uncompress(abuf);
#endif			  /* COMPRESS */

    *outbuf = '\0';

    mem = (char *) malloc(BUFFER_LEN * 5);
    howvar = mem + (BUFFER_LEN * 0);
    cmdvar = mem + (BUFFER_LEN * 1);
    argvar = mem + (BUFFER_LEN * 2);
    tmparg = mem + (BUFFER_LEN * 3);
    tmpcmd = mem + (BUFFER_LEN * 4);

    if (new_mvar("how", howvar)) {
	if(mem) free(mem);
	return outbuf;
    }
    strcpy(howvar, abuf);

    if (new_mvar("cmd", cmdvar)) {
	if(mem) free(mem);
	return outbuf;
    }
    strcpy(cmdvar, match_cmdname);
    strcpy(tmpcmd, match_cmdname);

    if (new_mvar("arg", argvar)) {
	if(mem) free(mem);
	return outbuf;
    }
    strcpy(argvar, match_args);
    strcpy(tmparg, match_args);

#ifdef COMPRESS
    inbuf = uncompress(inbuf);
#endif			  /* COMPRESS */

    dptr = MesgParse(inbuf, outbuf);
    if (!dptr) {
	*outbuf = '\0';
    }

    varc = mvarcnt;
    free_mfuncs(mfunccnt);
    mesg_rec_cnt = tmprec_cnt;
    mesg_instr_cnt = tmpinst_cnt;

    strcpy(match_cmdname, tmpcmd);
    strcpy(match_args, tmparg);

    if(mem) free(mem);
    return outbuf;
}

#endif /* MPI */

char *
do_parse_mesg(dbref player, dbref what, const char *inbuf, const char *abuf, char *outbuf, int mesgtyp)
{
#ifdef MPI
    if (tp_do_mpi_parsing &&
	( ( Meeper(what) && Meeper(OWNER(what)) ) ||
	  !tp_restricted_mpi
	)
    ) {
        char *tmp = NULL;
	struct timeval st, et;

        /* Quickie additions to do rought per-object MPI profiling */
        gettimeofday(&st,NULL);
	tmp=do_parse_mesg_2(player, what, what, inbuf, abuf, outbuf, mesgtyp);
	gettimeofday(&et,NULL);
	if (tmp && strcmp(tmp,inbuf)) {
	    et.tv_usec -= st.tv_usec;
	    et.tv_sec -= st.tv_sec;
	    if (et.tv_usec < 0) {
		et.tv_usec += 1000000;
		et.tv_sec -= 1;
	    }
	    DBFETCH(what)->mpi_prof_sec += et.tv_sec;
	    DBFETCH(what)->mpi_prof_usec += et.tv_usec;
	    if (DBFETCH(what)->mpi_prof_usec >= 1000000) {
		DBFETCH(what)->mpi_prof_usec -= 1000000;
		DBFETCH(what)->mpi_prof_sec += 1;
	    }
	    DBFETCH(what)->mpi_prof_use++;
	}
	return(tmp);
    } else
#endif
	strcpy(outbuf, inbuf);
    return outbuf;
}

char *
do_parse_perms(dbref player, dbref what, dbref perms, const char *inbuf, const char *abuf, char *outbuf, int mesgtyp)
{
#ifdef MPI
    if (tp_do_mpi_parsing &&
	( ( Meeper(what) && Meeper(OWNER(what)) ) ||
	  !tp_restricted_mpi
	)
    ) {
        char *tmp = NULL;
	struct timeval st, et;

        /* Quickie additions to do rought per-object MPI profiling */
        gettimeofday(&st,NULL);
	tmp = do_parse_mesg_2(player, what, perms, inbuf, abuf, outbuf, mesgtyp);
	gettimeofday(&et,NULL);
	if (strcmp(tmp,inbuf)) {
	    et.tv_usec -= st.tv_usec;
	    et.tv_sec -= st.tv_sec;
	    if (et.tv_usec < 0) {
		et.tv_usec += 1000000;
		et.tv_sec -= 1;
	    }
	    DBFETCH(what)->mpi_prof_sec += et.tv_sec;
	    DBFETCH(what)->mpi_prof_usec += et.tv_usec;
	    if (DBFETCH(what)->mpi_prof_usec >= 1000000) {
		DBFETCH(what)->mpi_prof_usec -= 1000000;
		DBFETCH(what)->mpi_prof_sec += 1;
	    }
	    DBFETCH(what)->mpi_prof_use++;
	}
	return(tmp);
    } else
#endif
	strcpy(outbuf, inbuf);
    return outbuf;
}
