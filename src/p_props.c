/* Primitives package */

#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#include "db.h"
#include "mpi.h"
#include "props.h"
#include "inst.h"
#include "match.h"
#include "interface.h"
#include "tune.h"
#include "strings.h"
#include "interp.h"
#include "externs.h"

extern struct inst *oper1, *oper2, *oper3, *oper4;
extern struct inst temp1, temp2, temp3;
extern int tmp, result;
extern dbref ref;
extern char buf[BUFFER_LEN];

int
prop_read_perms(dbref player, dbref obj, const char *name, int mlev)
{ 
    if ((mlev < (tp_compatible_muf ? LMAGE : tp_hidden_prop_mlevel)) && Prop_Hidden(name))
	return 0;
    if ((mlev < (tp_compatible_muf ? LM3 : tp_private_prop_mlevel)) && Prop_Private(name) && !permissions(mlev, player, obj))
	return 0;
    return 1;
}

int
prop_write_perms(dbref player, dbref obj, const char *name, int mlev)
{
    if (mlev < (tp_compatible_muf ? LMAGE : tp_hidden_prop_mlevel)) {
	if (Prop_Hidden(name)) return 0;
    } else if (PropDir_Check(name, PROP_HIDDEN, PROP_GLOW)) {
      return 0;
    }
    if (strncmp(name, "@/alias", 7) == 0 ||
	strncmp(name, "/@/alias", 8) == 0) {
      return 0;
    }
    if (mlev < (tp_compatible_muf ? LMAGE : tp_seeonly_prop_mlevel)) {
	if (Prop_SeeOnly(name)) return 0;
    }
    if (mlev < (tp_compatible_muf ? LM3 : tp_private_prop_mlevel)) {
	if (!permissions(mlev, player, obj)) {
	    if (Prop_Private(name)) return 0;
	    if (Prop_ReadOnly(name)) return 0;
	    if (!string_compare(name, PROP_SEX)) return 0;
	    if (!string_compare(name, PROP_POS)) return 0;
	    if (!string_compare(name, PROP_SPECIES)) return 0;
	}
	if (string_prefix(name, "_msgmacs/")) return 0;
	if (string_prefix(name, "_defs/")) return 0;
    }
    return 1;
}


void 
prim_getpropval(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper1->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Non-object argument (1)");
    CHECKREMOTE(oper2->data.objref);
    {
	char   *type;

	type = oper1->data.string->data;
	while ((type = index(type, PROPDIR_DELIMITER)))
	    if (!(*(++type)))
		abort_interp("Cannot access a propdir directly");
    }

    if (!prop_read_perms(ProgUID, oper2->data.objref,
			 oper1->data.string->data, mlev))
	abort_interp(NOPERM_MESG);

    {
	char   type[BUFFER_LEN];

	strcpy(type, oper1->data.string->data);
	result = get_property_value(oper2->data.objref, type);

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) GETPROPVAL: o=%d n=\"%s\" v=%d",
		 program, pc->line, oper2->data.objref, type, result);
#endif

	/* if (Typeof(oper2->data.objref) != TYPE_PLAYER)
	    ts_lastuseobject(oper2->data.objref); */
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_getprop(PRIM_PROTOTYPE)
{
    const char *temp;
    PropPtr prptr;
    dbref obj2;

    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper1->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Non-object argument (1)");
    CHECKREMOTE(oper2->data.objref);
    {
	char   type[BUFFER_LEN];
	char  *tmpptr;

	tmpptr = oper1->data.string->data;
	while ((tmpptr = index(tmpptr, PROPDIR_DELIMITER)))
	    if (!(*(++tmpptr)))
		abort_interp("Cannot access a propdir directly");

	if (!prop_read_perms(ProgUID, oper2->data.objref,
			     oper1->data.string->data, mlev))
	    abort_interp(NOPERM_MESG);

	strcpy(type, oper1->data.string->data);
	obj2 = oper2->data.objref;
	prptr = get_property(obj2, type);

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) GETPROP: o=%d n=\"%s\"",
		 program, pc->line, oper2->data.objref, type);
#endif

	CLEAR(oper1);
	CLEAR(oper2);
	if (prptr) {
#ifdef DISKBASE
	    propfetch(obj2, prptr);
#endif
	    switch(PropType(prptr)) {
	      case PROP_STRTYP:
		temp = get_uncompress(PropDataStr(prptr));
		PushString(temp);
		break;
	      case PROP_LOKTYP:
		if (PropFlags(prptr) & PROP_ISUNLOADED) {
		    PushLock(TRUE_BOOLEXP);
		} else {
		    PushLock(PropDataLok(prptr));
		}
		break;
	      case PROP_REFTYP:
		PushObject(PropDataRef(prptr));
		break;
	      case PROP_INTTYP:
		PushInt(PropDataVal(prptr));
		break;
	      default:
		result = 0;
		PushInt(result);
		break;
	    }
	} else {
	    result = 0;
	    PushInt(result);
	}

	/* if (Typeof(oper2->data.objref) != TYPE_PLAYER)
	    ts_lastuseobject(oper2->data.objref); */
    }
}


void 
prim_getpropstr(PRIM_PROTOTYPE)
{
    const char *temp;

    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper1->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Non-object argument (1)");
    CHECKREMOTE(oper2->data.objref);
    {
	char   type[BUFFER_LEN];
	char  *tmpptr;
	PropPtr ptr;

	tmpptr = oper1->data.string->data;
	while ((tmpptr = index(tmpptr, PROPDIR_DELIMITER)))
	    if (!(*(++tmpptr)))
		abort_interp("Cannot access a propdir directly");

	if (!prop_read_perms(ProgUID, oper2->data.objref,
			     oper1->data.string->data, mlev))
	    abort_interp(NOPERM_MESG);

	strcpy(type, oper1->data.string->data);
	ptr = get_property(oper2->data.objref, type);
	if (!ptr) {
	    temp = "";
	} else {
#ifdef DISKBASE
	    propfetch(oper2->data.objref, ptr);
#endif
	    switch(PropType(ptr)) {
		case PROP_STRTYP:
		    temp = get_uncompress(PropDataStr(ptr));
		    break;
		/*
		 *case PROP_INTTYP:
		 *    sprintf(buf, "%d", PropDataVal(ptr));
		 *    temp = buf;
		 *    break;
		 */
		case PROP_REFTYP:
		    sprintf(buf, "#%d", PropDataRef(ptr));
		    temp = buf;
		    break;
		case PROP_LOKTYP:
		    if (PropFlags(ptr) & PROP_ISUNLOADED) {
			temp = "*UNLOCKED*";
		    } else {
			temp = unparse_boolexp(ProgUID, PropDataLok(ptr), 1);
		    }
		    break;
		default:
		    temp = "";
		    break;
	    }
	}

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) GETPROPSTR: o=%d n=\"%s\" s=\"%s\"",
		 program, pc->line, oper2->data.objref, type, temp);
#endif

	/* if (Typeof(oper2->data.objref) != TYPE_PLAYER)
	 *     ts_lastuseobject(oper2->data.objref); */
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushString(temp);
}


void 
prim_remove_prop(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper1->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Non-object argument (1)");
    if (tp_db_readonly)
	abort_interp(DBRO_MESG);
    CHECKREMOTE(oper2->data.objref);
    {
	char   *type;

	type = oper1->data.string->data;
	while ((type = index(type, PROPDIR_DELIMITER)))
	    if (!(*(++type)))
		abort_interp("Cannot access a propdir directly");
    }

    if (!prop_write_perms(ProgUID, oper2->data.objref,
			 oper1->data.string->data, mlev))
	abort_interp(NOPERM_MESG);

    {
	char   type[BUFFER_LEN];

	strcpy(type, oper1->data.string->data);
	remove_property(oper2->data.objref, type);

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) REMOVEPROP: o=%d n=\"%s\"",
		 program, pc->line, oper1->data.objref, type);
#endif

	ts_modifyobject(oper2->data.objref);
    }
    CLEAR(oper1);
    CLEAR(oper2);
}


void 
prim_envprop(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper1->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Non-object argument (1)");
    CHECKREMOTE(oper2->data.objref);
    {
	char   *type;
	char    tname[BUFFER_LEN];
	dbref   what;
	PropPtr ptr;

	type = oper1->data.string->data;
	while ((type = index(type, PROPDIR_DELIMITER)))
	    if (!(*(++type)))
		abort_interp("Cannot access a propdir directly");
	strcpy(tname, oper1->data.string->data);
	what = oper2->data.objref;
	ptr = envprop(&what, tname, 0);
	if (what != NOTHING) {
	    if (!prop_read_perms(ProgUID,what,oper1->data.string->data,mlev))
		abort_interp(NOPERM_MESG);
	}
	CLEAR(oper1);
	CLEAR(oper2);
	PushObject(what);

	if (!ptr) {
	    result = 0;
	    PushInt(result);
	} else {
#ifdef DISKBASE
	    propfetch(what, ptr);
#endif
	    switch(PropType(ptr)) {
		case PROP_STRTYP:
		    PushString(get_uncompress(PropDataStr(ptr)));
		    break;
		case PROP_INTTYP:
		    result = PropDataVal(ptr);
		    PushInt(result);
		    break;
		case PROP_REFTYP:
		    ref = PropDataRef(ptr);
		    PushObject(ref);
		    break;
		case PROP_LOKTYP:
		    if (PropFlags(ptr) & PROP_ISUNLOADED) {
			PushLock(TRUE_BOOLEXP);
		    } else {
			PushLock(PropDataLok(ptr));
		    }
		    break;
		default:
		    result = 0;
		    PushInt(result);
		    break;
	    }
	}
    }
}


void 
prim_envpropstr(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper1->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Non-object argument (1)");
    CHECKREMOTE(oper2->data.objref);
    {
	char   *type;
	char    tname[BUFFER_LEN];
	dbref   what;
	PropPtr ptr;
	const char *temp;

	type = oper1->data.string->data;
	while ((type = index(type, PROPDIR_DELIMITER)))
	    if (!(*(++type)))
		abort_interp("Cannot access a propdir directly");
	strcpy(tname, oper1->data.string->data);
	what = oper2->data.objref;
	ptr = envprop(&what, tname, 0);
	if (!ptr) {
	    temp = "";
	} else {
#ifdef DISKBASE
	    propfetch(what, ptr);
#endif
	    switch(PropType(ptr)) {
		case PROP_STRTYP:
		    temp = get_uncompress(PropDataStr(ptr));
		    break;
		/*
		 *case PROP_INTTYP:
		 *    sprintf(buf, "%d", PropDataVal(ptr));
		 *    temp = buf;
		 *    break;
		 */
		case PROP_REFTYP:
		    sprintf(buf, "#%d", PropDataRef(ptr));
		    temp = buf;
		    break;
		case PROP_LOKTYP:
		    if (PropFlags(ptr) & PROP_ISUNLOADED) {
			temp = "*UNLOCKED*";
		    } else {
			temp = unparse_boolexp(ProgUID, PropDataLok(ptr), 1);
		    }
		    break;
		default:
		    temp = "";
		    break;
	    }
	}

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) ENVPROPSTR: o=%d so=%d n=\"%s\" s=\"%s\"",
		 program, pc->line, what, oper2->data.objref, tname, temp);
#endif

	if (what != NOTHING) {
	    if (!prop_read_perms(ProgUID, what, oper1->data.string->data,mlev))
		abort_interp(NOPERM_MESG);
	    /* if (Typeof(what) != TYPE_PLAYER)
	     *     ts_lastuseobject(what); */
	}
	CLEAR(oper1);
	CLEAR(oper2);
	PushObject(what);
	PushString(temp);
    }
}



void 
prim_setprop(PRIM_PROTOTYPE)
{
    PTYPE str;

    CHECKOP(3);
    oper1 = POP();
    oper2 = POP();
    oper3 = POP();
    if ((oper1->type != PROG_STRING) &&
	    (oper1->type != PROG_INTEGER) &&
	    (oper1->type != PROG_LOCK) &&
	    (oper1->type != PROG_OBJECT))
	abort_interp("Invalid argument type (3)");
    if (oper2->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper2->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper3))
	abort_interp("Non-object argument (1)");
    if (tp_db_readonly)
	abort_interp(DBRO_MESG);
    CHECKREMOTE(oper3->data.objref);

    if ((mlev < LM2) && (!permissions(mlev, ProgUID, oper3->data.objref)))
	abort_interp(NOPERM_MESG);

    if (!prop_write_perms(ProgUID, oper3->data.objref,
			 oper2->data.string->data, mlev))
	abort_interp(NOPERM_MESG);

    {
	char   *tmpe;
	char    tname[BUFFER_LEN];

	tmpe = oper2->data.string->data;
	while (*tmpe && *tmpe != '\r' && *tmpe != ':') tmpe++;
	if (*tmpe)
	    abort_interp("Illegal propname");

	tmpe = oper2->data.string->data;
	while ((tmpe = index(tmpe, PROPDIR_DELIMITER)))
	    if (!(*(++tmpe)))
		abort_interp("Cannot access a propdir directly");

	strcpy(tname, oper2->data.string->data);

	switch(oper1->type) {
	  case PROG_STRING:
	    str = (oper1->data.string ? oper1->data.string->data : 0);
	    set_property(oper3->data.objref, tname, PROP_STRTYP, str);
	    break;
	  case PROG_INTEGER:
	    result = oper1->data.number;
	    set_property(oper3->data.objref, tname, PROP_INTTYP, (char *)(long int)result);
	    break;
	  case PROG_OBJECT:
	    ref = oper1->data.objref;
	    set_property(oper3->data.objref, tname, PROP_REFTYP, (char *)(long int)ref);
	    break;
	  case PROG_LOCK:
	    str = (PTYPE) copy_bool(oper1->data.lock);
	    set_property(oper3->data.objref, tname, PROP_LOKTYP, str);
	    break;
	}

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) SETPROP: o=%d n=\"%s\"",
		 program, pc->line, oper3->data.objref, tname);
#endif

	ts_modifyobject(oper3->data.objref);
    }
    CLEAR(oper1);
    CLEAR(oper2);
    CLEAR(oper3);
}


void 
prim_addprop(PRIM_PROTOTYPE)
{
    CHECKOP(4);
    oper1 = POP();
    oper2 = POP();
    oper3 = POP();
    oper4 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument (4)");
    if (oper2->type != PROG_STRING)
	abort_interp("Non-string argument (3)");
    if (oper3->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (!oper3->data.string)
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper4))
	abort_interp("Non-object argument (1)");
    CHECKREMOTE(oper4->data.objref);

    if ((mlev < LM2) && (!permissions(mlev, ProgUID, oper4->data.objref)))
	abort_interp(NOPERM_MESG);

    if (!prop_write_perms(ProgUID, oper4->data.objref,
			 oper3->data.string->data, mlev))
	abort_interp(NOPERM_MESG);

    {
	const char *temp;
	char   *tmpe;
	char    tname[BUFFER_LEN];

	temp = (oper2->data.string ? oper2->data.string->data : 0);
	tmpe = oper3->data.string->data;
	while (*tmpe && *tmpe != '\r') tmpe++;
	if (*tmpe)
	    abort_interp("CRs not allowed in propname");

	tmpe = oper3->data.string->data;

	while ((tmpe = index(tmpe, PROPDIR_DELIMITER)))
	    if (!(*(++tmpe)))
		abort_interp("Cannot access a propdir directly");

	strcpy(tname, oper3->data.string->data);

	/* if ((temp) || (oper1->data.number)) */
	{
	    add_property(oper4->data.objref, tname, temp, oper1->data.number);

#ifdef LOG_PROPS
	    log2file("props.log", "#%d (%d) ADDPROP: o=%d n=\"%s\" s=\"%s\" v=%d",
		     program, pc->line, oper4->data.objref, tname, temp,
		     oper1->data.number);
#endif

	    ts_modifyobject(oper4->data.objref);
	}
    }
    CLEAR(oper1);
    CLEAR(oper2);
    CLEAR(oper3);
    CLEAR(oper4);
}



void 
prim_nextprop(PRIM_PROTOTYPE)
{
    /* dbref pname -- pname */
    char   *pname;
    char    exbuf[BUFFER_LEN];

    CHECKOP(2);
    oper2 = POP();		/* pname */
    oper1 = POP();		/* dbref */
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    if (oper2->type != PROG_STRING)
	abort_interp("String required (2)");
    if (oper1->type != PROG_OBJECT)
	abort_interp("Dbref required (1)");
    if (!valid_object(oper1))
	abort_interp("Invalid dbref (1)");

    ref = oper1->data.objref;
    (void) strcpy(buf, ((oper2->data.string) && (oper2->data.string->data)) ?
		  oper2->data.string->data : "");
    CLEAR(oper1);
    CLEAR(oper2);

    {
	char   *tmpname;

	pname = next_prop_name(ref, exbuf, buf);

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) NEXTPROP: o=%d n=\"%s\" on=\"%s\"",
		 program, pc->line, ref, pname, buf);
#endif

	if (mlev < (tp_compatible_muf ? LMAGE : LARCH)) {
	    while (pname && !prop_read_perms(ProgUID, ref, pname, mlev)) {
		tmpname = next_prop_name(ref, exbuf, pname);

#ifdef LOG_PROPS
		log2file("props.log", "#%d (%d) NEXTPROP: o=%d n=\"%s\" on=\"%s\"",
			 program, pc->line, ref, tmpname, pname);
#endif

		pname = tmpname;
	    }
	}
    }
    if (pname) {
	PushString(pname);
    } else {
	PushNullStr;
    }
}


void 
prim_propdirp(PRIM_PROTOTYPE)
{
    /* dbref dir -- int */
    CHECKOP(2);
    oper2 = POP();		/* prop name */
    oper1 = POP();		/* dbref */
    if (mlev < LM2)
	abort_interp("M2 prim");
    if (oper1->type != PROG_OBJECT)
	abort_interp("Argument must be a dbref (1)");
    if (!valid_object(oper1))
	abort_interp("Invalid dbref (1)");
    if (oper2->type != PROG_STRING)
	abort_interp("Argument not a string (2)");
    if (!oper2->data.string)
	abort_interp("Null string not allowed (2)");
    ref = oper1->data.objref;
    (void) strcpy(buf, oper2->data.string->data ? oper2->data.string->data : "");
    CLEAR(oper1);
    CLEAR(oper2);

    result = is_propdir(ref, buf);

#ifdef LOG_PROPS
    log2file("props.log", "#%d (%d) PROPDIR?: o=%d n=\"%s\" v=%d",
	     program, pc->line, ref, buf, result);
#endif

    PushInt(result);
}



void 
prim_parsempi(PRIM_PROTOTYPE)
{
    const char *temp;
    char *ptr;
    struct inst *oper1, *oper2, *oper3, *oper4;
    char buf[BUFFER_LEN]; /* Needed lest buffer get trashed by do_parse_mesg */

    CHECKOP(4);
    oper4 = POP();  /* int */
    oper2 = POP();  /* arg str */
    oper1 = POP();  /* mpi str */
    oper3 = POP();  /* object dbref */
    if (mlev < LM3)
	abort_interp("M3 prim");
    if (oper3->type != PROG_OBJECT)
	abort_interp("Non-object argument (1)");
    if (!valid_object(oper3))
	abort_interp("Invalid object (1)");
    if (oper2->type != PROG_STRING)
	abort_interp("String expected (3)");
    if (oper1->type != PROG_STRING)
	abort_interp("String expected (2)");
    /* if (!oper1->data.string)
	abort_interp("Empty string argument (2)"); */
    if (oper4->type != PROG_INTEGER)
	abort_interp("Integer expected (4)");
    if (oper4->data.number < 0 || oper4->data.number > 1)
	abort_interp("Integer of 0 or 1 expected (4)");
    CHECKREMOTE(oper3->data.objref);

#if 0
    /* Players and programs can't execute with MPI Wizperms, so
       their MPI parses safely, must check rooms, things, exits */
    if ((Typeof(oper3->data.objref) != TYPE_PLAYER) &&
	(Typeof(oper3->data.objref) != TYPE_PROGRAM) &&
	Mage(oper3->data.objref) &&
	TMage(OWNER(oper3->data.objref)) &&
	(mlev < MLevel(oper3->data.objref))
    )	abort_interp(NOPERM_MESG);
#endif /* Removed code */

    temp = (oper1->data.string)? oper1->data.string->data : "";
    ptr  = (oper2->data.string)? oper2->data.string->data : "";
    if(temp && *temp && ptr) {
	result = oper4->data.number & (~MPI_ISLISTENER);
	ptr = do_parse_perms(player, oper3->data.objref, ProgUID, temp,
			    ptr, buf, result);
	CLEAR(oper1);
	CLEAR(oper2);
	CLEAR(oper3);
	CLEAR(oper4);
	PushString(ptr); /* Was buf to remove unwanted \r's */
    } else {
	CLEAR(oper1);
	CLEAR(oper2);
	CLEAR(oper3);
	CLEAR(oper4);
	PushNullStr;
    }
}


void 
prim_parseprop(PRIM_PROTOTYPE)
{
    const char *temp;
    char *ptr;
    struct inst *oper1, *oper2, *oper3, *oper4;
    char buf[BUFFER_LEN]; /* Needed lest buffer get trashed by do_parse_mesg */

    CHECKOP(4);
    oper4 = POP();  /* int */
    oper2 = POP();  /* arg str */
    oper1 = POP();  /* propname str */
    oper3 = POP();  /* object dbref */
    if (mlev < LM3)
	abort_interp("M3 prim");
    if (oper3->type != PROG_OBJECT)
	abort_interp("Non-object argument (1)");
    if (!valid_object(oper3))
	abort_interp("Invalid object (1)");
    if (oper2->type != PROG_STRING)
	abort_interp("String expected (3)");
    if (oper1->type != PROG_STRING)
	abort_interp("String expected (2)");
    if (!oper1->data.string)
	abort_interp("Empty string argument (2)");
    if (oper4->type != PROG_INTEGER)
	abort_interp("Integer expected (4)");
    if (oper4->data.number < 0 || oper4->data.number > 1)
	abort_interp("Integer of 0 or 1 expected (4)");

    /* Players and programs can't execute with MPI Wizperms, so
       their MPI parses safely, must check rooms, things, exits */
    if ((Typeof(oper3->data.objref) != TYPE_PLAYER) &&
	(Typeof(oper3->data.objref) != TYPE_PROGRAM) &&
	Mage(oper3->data.objref) &&
	TMage(OWNER(oper3->data.objref)) &&
	(mlev < MLevel(oper3->data.objref))
    )	abort_interp(NOPERM_MESG);

    CHECKREMOTE(oper3->data.objref);
    {
	char   type[BUFFER_LEN];
	char  *tmpptr;

	tmpptr = oper1->data.string->data;
	while ((tmpptr = index(tmpptr, PROPDIR_DELIMITER)) != NULL)
	    if (!(*(++tmpptr)))
		abort_interp("Cannot access a propdir directly");

	if (!prop_read_perms(ProgUID, oper3->data.objref,
			     oper1->data.string->data, mlev))
	    abort_interp(NOPERM_MESG);

#if 0
	/* Why is this here??? It doesn't even make sense. */
	if ((mlev > LM3) && !permissions(mlev, player, oper3->data.objref) &&
		prop_write_perms(ProgUID, oper3->data.objref,
				 oper1->data.string->data, mlev))
	    abort_interp(NOPERM_MESG);
#endif /* Stupidity? */

	strcpy(type, oper1->data.string->data);
	temp = get_property_class(oper3->data.objref, type);
	if(temp) {
	    temp = get_uncompress(temp);
	}

#ifdef LOG_PROPS
	log2file("props.log", "#%d (%d) GETPROPSTR: o=%d n=\"%s\" s=\"%s\"",
		 program, pc->line, oper3->data.objref, type, temp);
#endif

    }
    ptr = (oper2->data.string)? oper2->data.string->data : "";
    if(temp) {
	result = oper4->data.number & (~MPI_ISLISTENER);
	ptr = do_parse_mesg(player, oper3->data.objref, temp,
			    ptr, buf, result);
	CLEAR(oper1);
	CLEAR(oper2);
	CLEAR(oper3);
	CLEAR(oper4);
	PushString(ptr);
    } else {
	CLEAR(oper1);
	CLEAR(oper2);
	CLEAR(oper3);
	CLEAR(oper4);
	PushNullStr;
    }
}
