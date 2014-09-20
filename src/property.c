#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include "color.h"
#include "db.h"
#include "tune.h"
#include "mpi.h"
#include "props.h"
#include "interface.h"
#include "externs.h"


#ifdef COMPRESS
extern const char *compress(const char *);
extern const char *old_uncompress(const char *);

#define alloc_compressed(x) alloc_string(compress(x))
#else				/* !COMPRESS */
#define alloc_compressed(x) alloc_string(x)
#endif				/* COMPRESS */

/* property.c
   A whole new lachesis mod.
   Adds property manipulation routines to TinyMUCK.   */

/* Completely rewritten by darkfox and Foxen, for propdirs and other things */



void
set_property_nofetch(dbref player, const char *type, int flags, PTYPE value)
{
    PropPtr p;
    char buf[BUFFER_LEN];
    char   *n, *w;

    /* Make sure that we are passed a valid property name */
    if (!type) return;

    /* if (tp_db_readonly) return; */ /* Why did we remove this? */

    while (*type == PROPDIR_DELIMITER) type++;
    if ((!(FLAGS(player) & LISTENER)) &&
	    (string_prefix(type, "@listen" ) ||
	     string_prefix(type, "~listen" ) ||
	     string_prefix(type, "~olisten") ||
	     string_prefix(type, "_listen" ) ||
	     string_prefix(type, "_olisten"))) {
	FLAGS(player) |= LISTENER;
    }

    w = strcpy(buf, type);

    /* truncate propnames with a ':' in them at the ':' */
    n = index(buf, PROP_DELIMITER);
    if (n) *n = '\0';
    if (!*buf) return;

    p = propdir_new_elem(&(DBFETCH(player)->properties), w);

    /* free up any old values */
    clear_propnode(p);

    SetPFlagsRaw(p, flags);
    if (PropFlags(p) & PROP_ISUNLOADED) {
	SetPDataVal(p, (int)(long int)value);
	return;
    }
    switch (PropType(p)) {
	case PROP_STRTYP:
	    if (!value || !*((char *)value)) {
		SetPType(p, PROP_DIRTYP);
		SetPDataVal(p, 0);
		if (!PropDir(p))
		    remove_property_nofetch(player, type);
	    } else {
		SetPDataStr(p, alloc_compressed((char *)value));
#ifdef COMPRESS
		SetPFlagsRaw(p, (flags | PROP_COMPRESSED));
#endif
	    }
	    break;
	case PROP_INTTYP:
	    SetPDataVal(p, (int)(long int)value);
	    if (!value) {
		SetPType(p, PROP_DIRTYP);
		if (!PropDir(p))
		    remove_property_nofetch(player, type);
	    }
	    break;
	case PROP_REFTYP:
	    if (((dbref)(long int)value) == NOTHING) {
		SetPType(p, PROP_DIRTYP);
		SetPDataVal(p, 0);
		if (!PropDir(p))
		    remove_property_nofetch(player, type);
	    } else {
		SetPDataRef(p, (dbref)(long int)value);
	    }
	    break;
	case PROP_LOKTYP:
	    SetPDataLok(p, (struct boolexp *)value);
	   /*
	    * if ((struct boolexp *)value == TRUE_BOOLEXP) {
	    *     SetPType(p, PROP_DIRTYP);
	    *     SetPDataVal(p, 0);
	    *     if (!PropDir(p))
	    *	 remove_property_nofetch(player, type);
	    * } else {
	    *     SetPDataLok(p, (struct boolexp *)value);
	    * }
	    */
	    break;
	case PROP_DIRTYP:
	    SetPDataVal(p, 0);
	    if (!PropDir(p))
		remove_property_nofetch(player, type);
	    break;
    }
}


void
set_property(dbref player, const char *type, int flags, PTYPE value)
{
    /* if( tp_db_readonly ) return; */ /* Why did we remove this? */
#ifdef DISKBASE
    fetchprops(player);
    set_property_nofetch(player, type, flags, (PTYPE)value);
    dirtyprops(player);
#else
    set_property_nofetch(player, type, flags, (PTYPE)value);
#endif
    DBDIRTY(player);
}

void
set_lock_property(dbref player, const char *type, char *lok)
{
    struct boolexp *p;
    if (!lok || !*lok) {
	p = TRUE_BOOLEXP;
    } else {
	p = parse_boolexp((dbref)1, lok, 1);
    }
    set_property(player, type, PROP_LOKTYP, (PTYPE)p);
}



/* adds a new property to an object */
void
add_prop_nofetch(dbref player, const char *type, const char *propertyClass, int value)
{
    if (propertyClass && *propertyClass) {
	set_property_nofetch(player, type, PROP_STRTYP, (PTYPE)propertyClass);
    } else if (value) {
	set_property_nofetch(player, type, PROP_INTTYP, (PTYPE)(long int)value);
    } else {
	set_property_nofetch(player, type, PROP_DIRTYP, 0);
    }
}


/* adds a new property to an object */
void
add_property(dbref player, const char *type, const char *propertyClass, int value)
{

#ifdef DISKBASE
    fetchprops(player);
    add_prop_nofetch(player, type, propertyClass, value);
    dirtyprops(player);
#else
    add_prop_nofetch(player, type, propertyClass, value);
#endif
    DBDIRTY(player);
}


void
remove_proplist_item(dbref player, PropPtr p, int allp)
{
    const char *ptr;

    /* if( tp_db_readonly ) return; */ /* Why did we remove this? */

    if (!p) return;
    ptr = PropName(p);
    if (!allp) {
	if (Prop_SeeOnly(ptr)) return;
	if (Prop_Hidden(ptr)) return;

	if (ptr[0] == '_'  &&  ptr[1] == '\0') return;
	if (PropFlags(p) & PROP_SYSPERMS) return;
    }
    /* notify(player, ptr); */ /* Why did we put this here? */
    remove_property(player, ptr);
}


/* removes property list --- if it's not there then ignore */
void 
remove_property_list(dbref player, int all)
{
    PropPtr l;
    PropPtr p;
    PropPtr n;

    /* if( tp_db_readonly ) return; */ /* Why did we remove this? */

#ifdef DISKBASE
    fetchprops(player);
#endif

    if ((l = DBFETCH(player)->properties)) {
	p = first_node(l);
	while (p) {
	    n = next_node(l, PropName(p));
	    remove_proplist_item(player, p, all);
	    l = DBFETCH(player)->properties;
	    p = n;
	}
    }

#ifdef DISKBASE
    dirtyprops(player);
#endif

    DBDIRTY(player);
}



/* removes property --- if it's not there then ignore */
void 
remove_property_nofetch(dbref player, const char *type)
{
    PropPtr l;
    char buf[BUFFER_LEN];
    char *w;

    /* if( tp_db_readonly ) return; */ /* Why did we remove this? */

    w = strcpy(buf, type);

    l = DBFETCH(player)->properties;
    l = propdir_delete_elem(l, w);
    DBFETCH(player)->properties = l;
    DBDIRTY(player);
}


void 
remove_property(dbref player, const char *type)
{

    /* if( tp_db_readonly ) return; */ /* Why did we remove this? */

#ifdef DISKBASE
    fetchprops(player);
#endif

    remove_property_nofetch(player, type);

#ifdef DISKBASE
    dirtyprops(player);
#endif
}


PropPtr 
get_property(dbref player, const char *type)
{
    PropPtr p;
    char buf[BUFFER_LEN];
    char *w;

#ifdef DISKBASE
    fetchprops(player);
#endif

    w = strcpy(buf, type);

    p = propdir_get_elem(DBFETCH(player)->properties, w);
    return (p);
}


/* checks if object has property, returning 1 if it or any of it's contents has
   the property stated						      */
int 
has_property(dbref player, dbref what, const char *type, const char *propertyClass, int value)
{
    dbref   things;

    if (has_property_strict(player, what, type, propertyClass, value))
	return 1;
    for (things = DBFETCH(what)->contents; things != NOTHING;
	    things = DBFETCH(things)->next) {
	if (has_property(player, things, type, propertyClass, value))
	    return 1;
    }
    if (tp_lock_envcheck) {
	things = getparent(what);
	while (things != NOTHING) {
	    if (has_property_strict(player, things, type, propertyClass, value))
		return 1;
	    things = getparent(things);
	}
    }
    return 0;
}


/* checks if object has property, returns 1 if it has the property */
int 
has_property_strict(dbref player, dbref what, const char *type, const char *propertyClass, int value)
{
    PropPtr p;
    const char *str;
    char   *ptr;
    char buf[BUFFER_LEN], xbuf[BUFFER_LEN];

    p = get_property(what, type);

    if (p) {
#ifdef DISKBASE
	propfetch(what, p);
#endif
	if (PropType(p) == PROP_STRTYP) {
#ifdef COMPRESS
	    str = uncompress(DoNull(PropDataStr(p)));
#else				/* !COMPRESS */
	    str = DoNull(PropDataStr(p));
#endif				/* COMPRESS */

	    ptr = do_parse_mesg(player, what, str, "(Lock)",
				buf, (MPI_ISPRIVATE | MPI_ISLOCK));
	    strcpy(xbuf, propertyClass);
	    return (equalstr(xbuf, ptr) || equalstr(ptr, xbuf));
	} else if (PropType(p) == PROP_LOKTYP) {
	    return 0;
	} else {
	    return (value == PropDataVal(p));
	}
    }
    return 0;
}

/* return class (string value) of property */
const char *
get_default_class(dbref player, const char *type)
{
    const char *m;
    char buf[14]; /* prefix(2) + obj type(2) + max len(9) + null(1) */
    
    m = get_property_class(player, type);
    if((!m) || (!*m)) {
	while((*type == '_') || (*type == '/'))
	    type++;

	if(*type && (strlen(type) <= 9)) {
	    strcpy(buf, "~/");

	    switch(Typeof(player)) {
		case TYPE_ROOM:
		    strcat(buf, "r/");
		    break;

		case TYPE_THING:
		    strcat(buf, "t/");
		    break;

		case TYPE_EXIT:
		    strcat(buf, "e/");
		    break;

		case TYPE_PLAYER:
		    strcat(buf, "p/");
		    break;

		case TYPE_PROGRAM:
		    strcat(buf, "f/");
		    break;

		default:
		    strcat(buf, "o/");
	    }

	    strcat(buf, type); /* Actual property */
	    return get_property_class(EnvRoom, buf);
	} else return NULL;
    } else return m;
}

/* return class (string value) of property */
const char *
get_property_class(dbref player, const char *type)
{
    PropPtr p;

    p = get_property(player, type);
    if (p) {
#ifdef DISKBASE
	propfetch(player, p);
#endif
	if (PropType(p) != PROP_STRTYP)
	    return (char *) NULL;
	return (PropDataStr(p));
    } else {
	return (char *) NULL;
    }
}

/* return value of property */
int 
get_property_value(dbref player, const char *type)
{
    PropPtr p;

    p = get_property(player, type);

    if (p) {
#ifdef DISKBASE
	propfetch(player, p);
#endif
	if (PropType(p) != PROP_INTTYP)
	    return 0;
	return (PropDataVal(p));
    } else {
	return 0;
    }
}

/* return boolexp lock of property */
dbref
get_property_dbref(dbref player, const char *propertyClass)
{
    PropPtr p;
    p = get_property(player, propertyClass);
    if (!p) return NOTHING;
#ifdef DISKBASE
    propfetch(player, p);
#endif
    if (PropType(p) != PROP_REFTYP)
	return NOTHING;
    return PropDataRef(p);
}


/* return boolexp lock of property */
struct boolexp *
get_property_lock(dbref player, const char *propertyClass)
{
    PropPtr p;
    p = get_property(player, propertyClass);
    if (!p) return TRUE_BOOLEXP;
#ifdef DISKBASE
    propfetch(player, p);
    if (PropFlags(p) & PROP_ISUNLOADED)
	return TRUE_BOOLEXP;
#endif
    if (PropType(p) != PROP_LOKTYP)
	return TRUE_BOOLEXP;
    return PropDataLok(p);
}


/* return flags of property */
int 
get_property_flags(dbref player, const char *type)
{
    PropPtr p;

    p = get_property(player, type);

    if (p) {
	return (PropFlags(p));
    } else {
	return 0;
    }
}


/* return type of property */
int 
get_property_type(dbref player, const char *type)
{
    PropPtr p;

    p = get_property(player, type);

    if (p) {
	return (PropType(p));
    } else {
	return 0;
    }
}



PropPtr
copy_prop(dbref old)
{
    PropPtr p, n = NULL;

#ifdef DISKBASE
    fetchprops(old);
#endif

    p = DBFETCH(old)->properties;
    copy_proplist(old, &n, p);
    return (n);
}

/* return old gender values for pronoun substitution code */
int 
genderof(dbref player)
{
    if (has_property_strict(player, player, PROP_SEX, GENDER_LIST_MALE, 0))
	return GENDER_MALE;
    else if (has_property_strict(player, player, PROP_SEX, GENDER_LIST_FEMALE, 0))
	return GENDER_FEMALE;
    else if (has_property_strict(player, player, PROP_SEX, GENDER_LIST_NEUTER, 0))
	return GENDER_NEUTER;
    else if (has_property_strict(player, player, PROP_SEX, GENDER_LIST_BOTH, 0))
	return GENDER_BOTH;
    else
	return GENDER_UNASSIGNED;
}


/* return a pointer to the first property in a propdir and duplicates the
   property name into 'name'.  returns 0 if the property list is empty
   or -1 if the property list does not exist. */
PropPtr 
first_prop_nofetch(dbref player, const char *dir, PropPtr *list, char *name)
{
    char buf[BUFFER_LEN];
    PropPtr p;

    if (dir) {
	while (*dir && *dir == PROPDIR_DELIMITER) {
	    dir++;
	}
    }
    if (!dir || !*dir) {
	*list = DBFETCH(player)->properties;
	p = first_node(*list);
	if (p) {
	    strcpy(name, PropName(p));
	} else {
	    *name = '\0';
	}
	return (p);
    }

    strcpy(buf, dir);
    *list = p = propdir_get_elem(DBFETCH(player)->properties, buf);
    if (!p) {
	*name = '\0';
	return((PropPtr) -1);
    }
    *list = PropDir(p);
    p = first_node(*list);
    if (p) {
	strcpy(name, PropName(p));
    } else {
	*name = '\0';
    }

    return (p);
}


/* first_prop() returns a pointer to the first property.
 * player    dbref of object that the properties are on.
 * dir       pointer to string name of the propdir
 * list      pointer to a proplist pointer.  Returns the root node.
 * name      printer to a string.  Returns the name of the first node.
 */

PropPtr 
first_prop(dbref player, const char *dir, PropPtr *list, char *name)
{

#ifdef DISKBASE
    fetchprops(player);
#endif

    return (first_prop_nofetch(player, dir, list, name));
}



/* next_prop() returns a pointer to the next property node.
 * list    Pointer to the root node of the list.
 * prop    Pointer to the previous prop.
 * name    Pointer to a string.  Returns the name of the next property.
 */

PropPtr 
next_prop(PropPtr list, PropPtr prop, char *name)
{
    PropPtr p = prop;

    if (!p || !(p = next_node(list, PropName(p))))
	return ((PropPtr) 0);

    strcpy(name, PropName(p));
    return (p);
}



/* next_prop_name() returns a ptr to the string name of the next property.
 * player   object the properties are on.
 * outbuf   pointer to buffer to return the next prop's name in.
 * name     pointer to the name of the previous property.
 *
 * Returns null if propdir doesn't exist, or if no more properties in list.
 * Call with name set to "" to get the first property of the root propdir.
 */

char   *
next_prop_name(dbref player, char *outbuf, char *name)
{
    char   *ptr;
    char    buf[BUFFER_LEN];
    PropPtr p, l;

#ifdef DISKBASE
    fetchprops(player);
#endif

    strcpy(buf, name);
    if (!*name || name[strlen(name)-1] == PROPDIR_DELIMITER) {
	l = DBFETCH(player)->properties;
	p = propdir_first_elem(l, buf);
	if (!p) {
	    *outbuf = '\0';
	    return NULL;
	}
	strcat(strcpy(outbuf, name), PropName(p));
    } else {
	l = DBFETCH(player)->properties;
	p = propdir_next_elem(l, buf);
	if (!p) {
	    *outbuf = '\0';
	    return NULL;
	}
	strcpy(outbuf, name);
	ptr = rindex(outbuf, PROPDIR_DELIMITER);
	if (!ptr) ptr = outbuf;
	*(ptr++) = PROPDIR_DELIMITER;
	strcpy(ptr, PropName(p));
    }
    return outbuf;
}


long
size_properties(dbref player, int load)
{
#ifdef DISKBASE
    if (load) {
	fetchprops(player);
	fetch_propvals(player, "/");
    }
#endif
    return size_proplist(DBFETCH(player)->properties);
}


/* return true if a property contains a propdir */
int 
is_propdir_nofetch(dbref player, const char *type)
{
    PropPtr p;
    char w[BUFFER_LEN];

    strcpy(w, type);
    p = propdir_get_elem(DBFETCH(player)->properties, w);
    if (!p) return 0;
    return (PropDir(p) != (PropPtr) NULL);
}


int 
is_propdir(dbref player, const char *type)
{

#ifdef DISKBASE
    fetchprops(player);
#endif

    return (is_propdir_nofetch(player, type));
}


PropPtr
regenvprop(dbref *where, const char *propname, int typ)
{
    PropPtr temp;
	temp = get_property(0, propname);
#ifdef DISKBASE
	if (temp) propfetch(0, temp);
#endif
	if (temp && (!typ || PropType(temp) == typ))
	    return temp;

    while (*where != NOTHING && *where != 0) {
	temp = get_property(*where, propname);
#ifdef DISKBASE
	if (temp) propfetch(*where, temp);
#endif
	if (temp && (!typ || PropType(temp) == typ))
	    return temp;
	*where = getparent(*where);
    }
    return NULL;
}


PropPtr
envprop(dbref *where, const char *propname, int typ)
{
    PropPtr temp;
    while (*where != NOTHING) {
	temp = get_property(*where, propname);
#ifdef DISKBASE
	if (temp) propfetch(*where, temp);
#endif
	if (temp && (!typ || PropType(temp) == typ))
	    return temp;
	*where = getparent(*where);
    }
    return NULL;
}


const char *
envpropstr(dbref * where, const char *propname)
{
    PropPtr temp;

    temp = envprop(where, propname, PROP_STRTYP);
    if (!temp) return NULL;
    if (PropType(temp) == PROP_STRTYP)
	return (PropDataStr(temp));
    return NULL;
}


#ifndef SANITY
char *
displayprop(dbref player, dbref obj, const char *name, char *buf)
{
    char mybuf[BUFFER_LEN], tbuf[BUFFER_LEN];
    int pdflag;
    PropPtr p = get_property(obj, name);

    if (!p) {
	sprintf(buf, CGLOOM "%s: No such property.", name);
	return buf;
    }
#ifdef DISKBASE
    propfetch(obj, p);
#endif
    pdflag = (PropDir(p) != NULL);
    sprintf(tbuf, "%.*s%c", (BUFFER_LEN/4), name,
	    (pdflag)? PROPDIR_DELIMITER:'\0');
    tct(tbuf, mybuf);
    switch (PropType(p)) {
	case PROP_STRTYP:
	    sprintf(buf, CAQUA "str " CGREEN "%s" CRED ":" CCYAN "%.*s", mybuf, (BUFFER_LEN/2),
		    tct(PropDataStr(p),tbuf));
	    break;
	case PROP_REFTYP:
	    sprintf(buf, CBROWN "ref " CGREEN "%s" CRED ":%s", mybuf,
		    ansi_unparse_object(player, PropDataRef(p)));
	    break;
	case PROP_INTTYP:
	    sprintf(buf, CFOREST "int " CGREEN "%s" CRED ":" CYELLOW "%d", mybuf, PropDataVal(p));
	    break;
	case PROP_LOKTYP:
	    if (PropFlags(p) & PROP_ISUNLOADED) {
		sprintf(buf, CCRIMSON "lok " CGREEN "%s" CRED ":" CPURPLE "*UNLOCKED*", mybuf);
	    } else {
		sprintf(buf, CCRIMSON "lok " CGREEN "%s" CRED ":" CPURPLE "%.*s", mybuf, (BUFFER_LEN/2),
			tct(unparse_boolexp(player, PropDataLok(p), 1),tbuf));
	    }
	    break;
	case PROP_DIRTYP:
	    sprintf(buf, CWHITE "dir " CGREEN "%s" CRED ":", mybuf);
	    break;
    }
    return buf;
}
#endif




extern short db_conversion_flag;
extern short db_decompression_flag;

int
db_get_single_prop(FILE *f, dbref obj, long pos)
{
    char getprop_buf[BUFFER_LEN*3];
    char *name, *flags, *value, *p;
    int flg;
    long tpos = 0;
    struct boolexp *lok;
    short do_diskbase_propvals;

#ifdef DISKBASE
    do_diskbase_propvals = tp_diskbase_propvals;
#else
    do_diskbase_propvals = 0;
#endif

    if (pos) {
	fseek(f, pos, 0);
    } else if (do_diskbase_propvals) {
	tpos = ftell(f);
    }
    name = fgets(getprop_buf, sizeof(getprop_buf), f);
    if (!name) abort();
    if (*name == '*') {
	if (!strcmp(name,"*End*\n")) {
	    return 0;
	}
    }

    flags = index(name, PROP_DELIMITER);
    if (!flags) abort();
    *flags++ = '\0';

    value = index(flags, PROP_DELIMITER);
    if (!value) abort();
    *value++ = '\0';

    p = index(value, '\n');
    if (p) *p = '\0';

    if (!number(flags)) abort();
    flg = atoi(flags);

    switch (flg & PROP_TYPMASK) {
	case PROP_STRTYP:
	    if (!do_diskbase_propvals || pos) {
		flg &= ~PROP_ISUNLOADED;
#ifdef COMPRESS
		if (!(flg & PROP_COMPRESSED)) {
		    value = (char *)old_uncompress(value);
		}
#endif
		set_property_nofetch(obj, name, flg, (PTYPE)value);
	    } else {
		flg |= PROP_ISUNLOADED;
		set_property_nofetch(obj, name, flg, (PTYPE)(long int)tpos);
	    }
	    break;
	case PROP_LOKTYP:
	    if (!do_diskbase_propvals || pos) {
		lok = parse_boolexp((dbref)1, value, 32767);
		flg &= ~PROP_ISUNLOADED;
		set_property_nofetch(obj, name, flg, (PTYPE)lok);
	    } else {
		flg |= PROP_ISUNLOADED;
		set_property_nofetch(obj, name, flg, (PTYPE)(long int)tpos);
	    }
	    break;
	case PROP_INTTYP:
	    if (!number(value)) abort();
	    set_property_nofetch(obj, name, flg, (PTYPE)(long int)atoi(value));
	    break;
	case PROP_REFTYP:
	    if (!number(value)) abort();
	    set_property_nofetch(obj, name, flg, (PTYPE)(long int)atoi(value));
	    break;
	case PROP_DIRTYP:
	    break;
    }
    return 1;
}


void
db_getprops(FILE *f, dbref obj)
{
    while (db_get_single_prop(f, obj, 0L));
}


void
db_putprop(FILE *f, const char *dir, PropPtr p)
{
    char buf[BUFFER_LEN*2];
    char *ptr;
    const char *ptr2;

    if (PropType(p) == PROP_DIRTYP)
	return;

    for (ptr = buf, ptr2 = dir+1; *ptr2;) *ptr++ = *ptr2++;
    for (ptr2 = PropName(p); *ptr2;) *ptr++ = *ptr2++;
    *ptr++ = PROP_DELIMITER;
    ptr2 = intostr(PropFlagsRaw(p) & ~(PROP_TOUCHED | PROP_ISUNLOADED));
    while (*ptr2) *ptr++ = *ptr2++;
    *ptr++ = PROP_DELIMITER;

    ptr2 = "";
    switch (PropType(p)) {
	case PROP_INTTYP:
	    if (!PropDataVal(p)) return;
	    ptr2 = intostr(PropDataVal(p));
	    break;
	case PROP_REFTYP:
	    if (PropDataRef(p) == NOTHING) return;
	    ptr2 = intostr((int)PropDataRef(p));
	    break;
	case PROP_STRTYP:
	    if (!*PropDataStr(p)) return;
	    if (db_decompression_flag) {
#ifdef COMPRESS
		ptr2 = uncompress(PropDataStr(p));
#else
		ptr2 = PropDataStr(p);
#endif
	    } else {
		ptr2 = PropDataStr(p);
	    }
	    break;
	case PROP_LOKTYP:
	    if (PropFlags(p) & PROP_ISUNLOADED) return;
	    if (PropDataLok(p) == TRUE_BOOLEXP) return;
	    ptr2 = unparse_boolexp((dbref)1, PropDataLok(p), 0);
	    break;
    }
    while (*ptr2) *ptr++ = *ptr2++;
    *ptr++ = '\n';
    *ptr++ = '\0';
    if (fputs(buf, f) == EOF) {
	abort();
    }
}


void
db_dump_props_rec(dbref obj, FILE *f, const char *dir, PropPtr p)
{
    char buf[BUFFER_LEN];
#ifdef DISKBASE
    int tpos = 0;
    int flg;
    short wastouched = 0;
#endif

    if (!p) return;

    db_dump_props_rec(obj, f, dir, AVL_LF(p));
#ifdef DISKBASE
    if (tp_diskbase_propvals) {
	tpos = ftell(f);
	wastouched = (PropFlags(p) & PROP_TOUCHED);
    }
    if (propfetch(obj, p)) {
	fseek(f, 0L, 2);
    }
#endif
    db_putprop(f, dir, p);
#ifdef DISKBASE
    if (tp_diskbase_propvals && !wastouched) {
	if (PropType(p) == PROP_STRTYP || PropType(p) == PROP_LOKTYP) {
	    flg = PropFlagsRaw(p) | PROP_ISUNLOADED;
	    clear_propnode(p);
	    SetPFlagsRaw(p, flg);
	    SetPDataVal(p, tpos);
	}
    }
#endif
    if (PropDir(p)) {
	sprintf(buf, "%s%s%c", dir, PropName(p), PROPDIR_DELIMITER);
	db_dump_props_rec(obj, f, buf, PropDir(p));
    }
    db_dump_props_rec(obj, f, dir, AVL_RT(p));
}


void
db_dump_props(FILE *f, dbref obj)
{
    db_dump_props_rec(obj, f, "/", DBFETCH(obj)->properties);
}


void
untouchprop_rec(PropPtr p)
{
    if (!p) return;
    SetPFlags(p, (PropFlags(p) & ~PROP_TOUCHED));
    untouchprop_rec(AVL_LF(p));
    untouchprop_rec(AVL_RT(p));
    untouchprop_rec(PropDir(p));
}

static dbref untouch_lastdone = 0;
void
untouchprops_incremental(int limit)
{
    PropPtr p;

    while (untouch_lastdone < db_top) {
	/* clear the touch flags */
	p = DBFETCH(untouch_lastdone)->properties;
	if (p) {
	    if (!limit--) return;
	    untouchprop_rec(p);
	}
	untouch_lastdone++;
    }
    untouch_lastdone = 0;
}

