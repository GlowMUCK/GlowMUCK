/* Primitives package */

#include "copyright.h"
#include "config.h"
#include "params.h"
#include "local.h"

#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#include "color.h"
#include "db.h"
#include "tune.h"
#include "props.h"
#include "inst.h"
#include "match.h"
#include "interface.h"
#include "strings.h"
#include "interp.h"
#include "externs.h"

extern struct inst *oper1, *oper2, *oper3, *oper4;
extern struct inst temp1, temp2, temp3;
extern int tmp, result;
extern dbref ref;
extern char buf[BUFFER_LEN];


void 
copyobj(dbref player, dbref oldDatabaseReference, dbref newDatabaseReference)
{
    struct object *newp = DBFETCH(newDatabaseReference);

    NAME(newDatabaseReference) = alloc_string(NAME(oldDatabaseReference));
    newp->properties = copy_prop(oldDatabaseReference);
    newp->exits = NOTHING;
    newp->contents = NOTHING;
    newp->next = NOTHING;
    newp->location = NOTHING;
    moveto(newDatabaseReference, player);

#ifdef DISKBASE
    newp->propsfpos = 0;
    newp->propsmode = PROPS_UNLOADED;
    newp->propstime = 0;
    newp->nextold = NOTHING;
    newp->prevold = NOTHING;
    dirtyprops(newDatabaseReference);
#endif

    DBDIRTY(newDatabaseReference);
}



void 
prim_addpennies(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();

    if (mlev < LM2)
	abort_interp("M2 prim");
    if (!valid_object(oper2))
	abort_interp("Invalid object");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument (2)");

    ref = oper2->data.objref;

    if (Typeof(ref) == TYPE_PLAYER) {
	result = DBFETCH(ref)->sp.player.pennies;
	/*	if ((result + oper1->data.number) < 0)
	 *   abort_interp("Result would be negative");
         */
	if (mlev < LMAGE) 
	    if (oper1->data.number > 0)
		if ((result + oper1->data.number) > tp_max_pennies)
		    abort_interp("Would exceed MAX_PENNIES");
	result += oper1->data.number;
	DBFETCH(ref)->sp.player.pennies += oper1->data.number;
	DBDIRTY(ref);
    } else if (Typeof(ref) == TYPE_THING) {
	if (mlev < LMAGE)
	    abort_interp("Mage level required");
	result = DBFETCH(ref)->sp.thing.value + oper1->data.number;
	if (result < 1)
	    abort_interp("Result must be positive");
	DBFETCH(ref)->sp.thing.value += oper1->data.number;
	DBDIRTY(ref);
    } else {
	abort_interp("Invalid object type");
    }
    CLEAR(oper1);
    CLEAR(oper2);
}

void 
prim_moveto(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2;

    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (!(valid_object(oper1) && valid_object(oper2)) && !is_home(oper1))
	abort_interp("Non-database item argument");
    {
	dbref   victim, dest;

	victim = oper2->data.objref;
	dest = oper1->data.objref;

	if (Typeof(dest) == TYPE_EXIT)
	    abort_interp("Destination argument is an exit");
	if (Typeof(victim) == TYPE_EXIT && (mlev < LM3))
	    abort_interp("M3 required for re-attaching an exit");
	if (!(FLAGS(victim) & JUMP_OK)
		&& !permissions(mlev, ProgUID, victim)
		&& (mlev < (tp_compatible_muf ? LM3 : LWIZ)))
	    abort_interp("Database item can't be moved");
	switch (Typeof(victim)) {
	    case TYPE_PLAYER:
		if (Typeof(dest) != TYPE_ROOM &&
		    Typeof(dest) != TYPE_PLAYER &&
		    Typeof(dest) != TYPE_THING )
		    abort_interp("Bad destination");
		/* Check permissions */
		if (parent_loop_check(victim, dest))
		    abort_interp("Things can't contain themselves");
		if (mlev < (tp_compatible_muf ? LM3 : LWIZ)) {
		    if (!(FLAGS(dest) & VEHICLE)
			 && ( Typeof(dest) == TYPE_THING
			   || Typeof(dest) == TYPE_PLAYER ) )
			abort_interp("Destination is not a vehicle");
		    if (!(FLAGS(DBFETCH(victim)->location) & JUMP_OK)
			 && !permissions(mlev, ProgUID, DBFETCH(victim)->location))
			abort_interp("Source not JUMP_OK");
		    if (!is_home(oper1) && !(FLAGS(dest) & JUMP_OK)
			    && !permissions(mlev, ProgUID, dest))
			abort_interp("Destination not JUMP_OK");
		    if (Typeof(dest)==TYPE_THING
			    && getloc(victim) != getloc(dest))
			abort_interp("Not in same location as vehicle");
		}
		enter_room(victim, dest, program);
		break;
	    case TYPE_THING:
		if (parent_loop_check(victim, dest))
		    abort_interp("A thing cannot contain itself");
		if ((mlev < (tp_compatible_muf ? LM3 : LWIZ)) &&
			(FLAGS(victim) & VEHICLE) &&
			(FLAGS(dest) & VEHICLE) && Typeof(dest) != TYPE_THING)
		    abort_interp("Destination doesn't accept vehicles");
		if ((mlev < (tp_compatible_muf ? LM3 : LWIZ)) &&
			(FLAGS(victim) & ZOMBIE) &&
			(FLAGS(dest) & ZOMBIE) && Typeof(dest) != TYPE_THING)
		    abort_interp("Destination doesn't accept zombies");
		ts_lastuseobject(victim);
	    case TYPE_PROGRAM:
		{
		    dbref   matchroom = NOTHING;

		    if (Typeof(dest) != TYPE_ROOM && Typeof(dest) != TYPE_PLAYER
			    && Typeof(dest) != TYPE_THING)
			abort_interp("Bad destination");
		    if (mlev < (tp_compatible_muf ? LM3 : LWIZ)) {
			if (permissions(mlev, ProgUID, dest))
			    matchroom = dest;
			if (permissions(mlev, ProgUID, DBFETCH(victim)->location))
			    matchroom = DBFETCH(victim)->location;
			if (matchroom != NOTHING && !(FLAGS(matchroom)&JUMP_OK)
				&& !permissions(mlev, ProgUID, victim))
			    abort_interp(NOPERM_MESG);
		    }
		}
		if (Typeof(victim)==TYPE_THING && (FLAGS(victim) & ZOMBIE)) {
		    enter_room(victim, dest, program);
		} else {
		    moveto(victim, dest);
		}
		break;
	    case TYPE_EXIT:
		if (!permissions(mlev, ProgUID, victim)
			|| !permissions(mlev, ProgUID, dest))
		    abort_interp(NOPERM_MESG);
		if (Typeof(dest)!=TYPE_ROOM && Typeof(dest)!=TYPE_THING &&
			Typeof(dest) != TYPE_PLAYER)
		    abort_interp("Bad destination object");
		if (!unset_source(ProgUID, getloc(player), victim))
		    break;
		set_source(ProgUID, victim, dest);
		SetMLevel(victim, 0);
		break;
	    case TYPE_ROOM:
		if (Typeof(dest) != TYPE_ROOM)
		    abort_interp("Bad destination");
		if (victim == GLOBAL_ENVIRONMENT)
		    abort_interp(NOPERM_MESG);
		if (dest == HOME) {
		    dest = GLOBAL_ENVIRONMENT;
		} else {
		    if (!permissions(mlev, ProgUID, victim)
			    || !can_link_to(ProgUID, NOTYPE, dest))
			abort_interp(NOPERM_MESG);
		    if (parent_loop_check(victim, dest)) {
			abort_interp("Parent room would create a loop");
		    }
		}
		ts_lastuseobject(victim);
		moveto(victim, dest);
		break;
	    default:
		abort_interp("Invalid object type (1)");
	}
    }
    CLEAR(oper1);
    CLEAR(oper2);
}

void 
prim_pennies(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid argument");
    CHECKREMOTE(oper1->data.objref);
    switch (Typeof(oper1->data.objref)) {
	case TYPE_PLAYER:
	    result = DBFETCH(oper1->data.objref)->sp.player.pennies;
	    break;
	case TYPE_THING:
	    result = DBFETCH(oper1->data.objref)->sp.thing.value;
	    break;
	default:
	    abort_interp("Invalid argument");
    }
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_dbcomp(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_OBJECT || oper2->type != PROG_OBJECT)
	abort_interp("Invalid argument type");
    result = oper1->data.objref == oper2->data.objref;
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}

void 
prim_dbref(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument");
    ref = (dbref) oper1->data.number;
    CLEAR(oper1);
    PushObject(ref);
}

void 
prim_contents(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid argument type");
    CHECKREMOTE(oper1->data.objref);
    ref = DBFETCH(oper1->data.objref)->contents;
    while ((mlev < LM2) && (ref != NOTHING) &&
	    (FLAGS(ref) & DARK) && !controls(ProgUID, ref))
	ref = DBFETCH(ref)->next;
    if (Typeof(oper1->data.objref) != TYPE_PLAYER &&
	    Typeof(oper1->data.objref) != TYPE_PROGRAM)
	ts_lastuseobject(oper1->data.objref);
    CLEAR(oper1);
    PushObject(ref);
}

void 
prim_exits(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    ref = oper1->data.objref;
    CHECKREMOTE(ref);
    if ((mlev < (tp_compatible_muf ? LM3 : LM2)) && !permissions(mlev, ProgUID, ref))
	abort_interp(NOPERM_MESG);
    switch (Typeof(ref)) {
	case TYPE_ROOM:
	case TYPE_THING:
	    ts_lastuseobject(ref);
	case TYPE_PLAYER:
	    ref = DBFETCH(ref)->exits;
	    break;
	default:
	    abort_interp("Invalid object");
    }
    CLEAR(oper1);
    PushObject(ref);
}


void 
prim_next(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    CHECKREMOTE(oper1->data.objref);
    ref = DBFETCH(oper1->data.objref)->next;
    while ((mlev < LM2) && (ref != NOTHING) && Typeof(ref) != TYPE_EXIT &&
	    ((FLAGS(ref) & DARK) || Typeof(ref) == TYPE_ROOM) &&
	    !controls(ProgUID, ref))
	ref = DBFETCH(ref)->next;
    CLEAR(oper1);
    PushObject(ref);
}

void 
prim_truename(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid argument type");
    ref = oper1->data.objref;
    CHECKREMOTE(ref);
    if ((Typeof(ref) != TYPE_PLAYER) && (Typeof(ref) != TYPE_PROGRAM))
	ts_lastuseobject(ref);
    if (NAME(ref)) {
	strcpy(buf, NAME(ref));
    } else {
	buf[0] = '\0';
    }
    CLEAR(oper1);
    PushString(buf);
}

void 
prim_name(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid argument type");
    ref = oper1->data.objref;
    CHECKREMOTE(ref);
    if ((Typeof(ref) != TYPE_PLAYER) && (Typeof(ref) != TYPE_PROGRAM))
	ts_lastuseobject(ref);
    if (NAME(ref)) {
	strcpy(buf, PNAME(ref));
    } else {
	buf[0] = '\0';
    }
    CLEAR(oper1);
    PushString(buf);
}

void 
prim_setname(PRIM_PROTOTYPE)
{
    char *password;
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!valid_object(oper2))
	abort_interp("Invalid argument type (1)");
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    ref = oper2->data.objref;
    if (!permissions(mlev, ProgUID, ref))
	abort_interp(NOPERM_MESG);
    if (tp_db_readonly)
	abort_interp("Db is read-only");
    {
	const char *b = DoNullInd(oper1->data.string);

	if (Typeof(ref) == TYPE_PLAYER) {
	    strcpy(buf, b);
	    b = buf;
	    if (mlev < LMAGE)
		abort_interp("Mage level required to change the name of a player");
	    /* split off password */
	    for (password = buf;
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
		abort_interp("Player namechange requires \"yes\" appended");
	    } else if ( ( (!DBFETCH(ref)->sp.player.password) ||
			  (!*DBFETCH(ref)->sp.player.password) ||
			  strcmp(password, DBFETCH(ref)->sp.player.password)
			) && strcmp(password, "yes")
	    ) {
		abort_interp("Incorrect password");
	    } else if (string_compare(b, NAME(ref))
		       && !ok_player_name(b)) {
		abort_interp("You can't give a player that name");
	    }
	    /* everything ok, notify */
	    /* log_status("NAME: (MUF) %s to %s by %s(%d)\n",
		       unparse_object(MAN, ref), b, NAME(ProgUID), ProgUID); */
	    delete_player(ref);
	    if (NAME(ref))
		free((void *) NAME(ref));
	    ts_modifyobject(ref);
	    NAME(ref) = alloc_string(b);
	    add_player(ref);
	} else {
	    if (!ok_name(b))
		abort_interp("Invalid name");
	    if (NAME(ref))
		free((void *) NAME(ref));
	    NAME(ref) = alloc_string(b);
	    ts_modifyobject(ref);
	    if (MLevel(ref))
		SetMLevel(ref, 0);
	}
    }
    CLEAR(oper1);
    CLEAR(oper2);
}

void 
prim_match(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument");
    if (!oper1->data.string)
	abort_interp("Empty string argument");
    {
	char    tmppp[BUFFER_LEN];
	struct match_data md;

	(void) strcpy(buf, match_args);
	(void) strcpy(tmppp, match_cmdname);
	init_match(player, oper1->data.string->data, NOTYPE, &md);
	if (oper1->data.string->data[0] == REGISTERED_TOKEN) {
	    match_registered(&md);
	} else {
	    match_all_exits(&md);
	    match_neighbor(&md);
	    match_possession(&md);
	    match_me(&md);
	    match_here(&md);
	    match_home(&md);
	}
	if (mlev >= LMAGE) {
	    match_absolute(&md);
	    match_player(&md);
	}
	ref = match_result(&md);
	(void) strcpy(match_args, buf);
	(void) strcpy(match_cmdname, tmppp);
    }
    CLEAR(oper1);
    PushObject(ref);
}


void 
prim_rmatch(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument (2)");
    if (oper2->type != PROG_OBJECT
	    || (!OkObj(oper2->data.objref))
	    || Typeof(oper2->data.objref) == TYPE_PROGRAM
	    || Typeof(oper2->data.objref) == TYPE_EXIT)
	abort_interp("Invalid argument (1)");
    CHECKREMOTE(oper2->data.objref);
    {
	char    tmppp[BUFFER_LEN];
	struct match_data md;

	(void) strcpy(buf, match_args);
	(void) strcpy(tmppp, match_cmdname);
	init_match(player, DoNullInd(oper1->data.string), TYPE_THING, &md);
	match_rmatch(oper2->data.objref, &md);
	ref = match_result(&md);
	(void) strcpy(match_args, buf);
	(void) strcpy(match_cmdname, tmppp);
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushObject(ref);
}


void 
prim_copyobj(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    CHECKREMOTE(oper1->data.objref);
    if ((mlev < (tp_compatible_muf ? LM3 : LMAGE)) && (fr->already_created))
	abort_interp("Can't create any more objects");
    if (!tp_building || tp_db_readonly)
	abort_interp("Building is currently disabled");
    ref = oper1->data.objref;
    if (Typeof(ref) != TYPE_THING)
	abort_interp("Invalid object type");
    if ((mlev < (tp_compatible_muf ? LM3 : LMAGE)) && !permissions(mlev, ProgUID, ref))
	abort_interp(NOPERM_MESG);
    if (mlev < tp_newobject_mlevel)
	abort_interp(NOPERM_MESG);
    fr->already_created++;
    {
	dbref   newobj;

	newobj = new_object();
	*DBFETCH(newobj) = *DBFETCH(ref);
	copyobj(player, ref, newobj);
	CLEAR(oper1);
	PushObject(newobj);
    }
}


void 
prim_set(PRIM_PROTOTYPE)
/* SET */
{
    int tmp2 = 0;

    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument type (2)");
    if (!(oper1->data.string))
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Invalid object");
    if (tp_db_readonly)
	abort_interp("Db is read-only");
    ref = oper2->data.objref;
    CHECKREMOTE(ref);
    tmp = 0;
    result = (*oper1->data.string->data == '!');
    {
	char   *flag = oper1->data.string->data;

	if (result)
	    flag++;

	if (!*flag)
	    abort_interp("Empty flag");

	if (string_prefix("dark", flag)
		|| string_prefix("debug_spam", flag))
	    tmp = DARK;
	else if (string_prefix("abode", flag)
		|| string_prefix("autostart", flag)
		|| string_prefix("abate", flag))
	    tmp = ABODE;
	else if (string_prefix("chown_ok", flag)
		|| string_prefix("color_ansi", flag)
		|| string_prefix("count_insts", flag))
	    tmp = CHOWN_OK;
	else if (string_prefix("haven", flag)
		 || string_prefix("harduid", flag))
	    tmp = HAVEN;
	else if (string_prefix("jump_ok", flag))
	    tmp = JUMP_OK;
	else if (string_prefix("link_ok", flag))
	    tmp = LINK_OK;

	else if (string_prefix("kill_ok", flag))
	    tmp = KILL_OK;

	else if (string_prefix("builder", flag))
	    tmp = BUILDER;
	else if (string_prefix("interactive", flag))
	    tmp = INTERACTIVE;
	else if (string_prefix("sticky", flag)
		 || string_prefix("silent", flag))
	    tmp = STICKY;
	else if (string_prefix("xforcible", flag))
	    tmp = XFORCIBLE;
	else if (string_prefix("zombie", flag))
	    tmp = ZOMBIE;
	else if (string_prefix("vehicle", flag))
	    tmp = VEHICLE;
	else if (string_prefix("quell", flag))
	    tmp = QUELL;
	else if (string_prefix("guest", flag))
	    tmp2 = F2GUEST;
	else if (string_prefix("offer_task", flag)
		|| string_prefix("task_pending", flag))
	    tmp2 = F2OFFER;
	else if (string_prefix("ic", flag)
		|| string_prefix("in_character", flag))
	    tmp2 = F2IC;
	else if (string_prefix("idle", flag)) 
	    tmp2 = F2IDLE;
	else if (string_prefix("pueblo", flag)) 
	    tmp2 = F2PUEBLO;
	else if (string_prefix("tinkerproof", flag)) 
	    tmp2 = F2TINKERPROOF;
	else if (string_prefix("logwall", flag)) 
	    tmp2 = F2LOGWALL;
	else if (string_prefix("www_ok", flag)) 
	    tmp2 = F2WWW;
	else if (string_prefix("suspect", flag)) 
	    tmp2 = F2SUSPECT;
	else if (string_prefix("meeper", flag)) 
	    tmp2 = F2MPI;
	else if (string_prefix("readblankline", flag)) 
	    tmp2 = F2READBLANKLINE;
	else abort_interp("Unrecognized flag");

    }
    if( !tmp && !tmp2 )
	abort_interp("Unrecognized flag (?!)");
    if ((ref != program) && (!permissions(mlev, ProgUID, ref)))
	abort_interp(NOPERM_MESG);

    if(tmp) {
      if (((mlev < (tp_compatible_muf ? LMAGE : LARCH)) && ((tmp == DARK && ((Typeof(ref) == TYPE_PLAYER)
			   || (!tp_exit_darking && Typeof(ref) == TYPE_EXIT)
			   || (!tp_thing_darking && Typeof(ref) == TYPE_THING)
			  )
			)
			|| ((tmp == ZOMBIE) && (Typeof(ref) == TYPE_THING)
			    && (FLAGS(ProgUID) & ZOMBIE))
			|| ((tmp == ZOMBIE) && (Typeof(ref) == TYPE_PLAYER))
			|| (tmp == BUILDER)
		    )
	    )
	    || ((tmp == BUILDER) && (mlev < tp_muf_mpi_flag_mlevel))
	    || (tmp == W1) || (tmp == W2) || (tmp == W3)
	    || (tmp == QUELL) || (tmp == INTERACTIVE)
	    ||	( ( (tmp == ABODE) || (tmp == VEHICLE) )
		    && (Typeof(ref) == TYPE_PROGRAM)
		)
	    || (tmp == XFORCIBLE)
	    )
	abort_interp(NOPERM_MESG);
/*    if (result && Typeof(ref) == TYPE_THING) {
	dbref obj = DBFETCH(ref)->contents;
	for (; obj != NOTHING; obj = DBFETCH(obj)->next) {
	    if (Typeof(obj) == TYPE_PLAYER) {
		abort_interp(NOPERM_MESG);
	    }
	}
      }
*/
      if (!result) {
	FLAGS(ref) |= tmp;
	DBDIRTY(ref);
      } else {
	FLAGS(ref) &= ~tmp;
	DBDIRTY(ref);
      }
    } else { /* new flags */
      if ( ((mlev < LMAGE) && ((tmp2 == F2GUEST) || (tmp2 == F2WWW) ||
	(tmp2 == F2OFFER) || (tmp2 == F2IDLE) || (tmp2 == F2MPI)
      )) )
	abort_interp(NOPERM_MESG);
      if ((tmp2 == F2MPI) && (mlev < tp_muf_mpi_flag_mlevel))
	abort_interp(NOPERM_MESG);
      if ( ((mlev < (tp_compatible_muf ? LMAGE : LBOY)) && ((tmp2 == F2LOGWALL) ||
	(tmp2 == F2SUSPECT)
      )) )
	abort_interp(NOPERM_MESG);
      

/*    if (result && Typeof(ref) == TYPE_THING) {
	dbref obj = DBFETCH(ref)->contents;
	for (; obj != NOTHING; obj = DBFETCH(obj)->next) {
	    if (Typeof(obj) == TYPE_PLAYER) {
		abort_interp(NOPERM_MESG);
	    }
	}
      }
*/
      if (!result) {
	FLAG2(ref) |= tmp2;
	DBDIRTY(ref);
      } else {
	FLAG2(ref) &= ~tmp2;
	DBDIRTY(ref);
      }
    }
    CLEAR(oper1);
    CLEAR(oper2);
}

void 
prim_mlevel(PRIM_PROTOTYPE)
/* MLEVEL */
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    ref = oper1->data.objref;
    CHECKREMOTE(ref);
    result = MLevel(ref);
    CLEAR(oper1);
    PushInt(result);
}

void 
prim_flagp(PRIM_PROTOTYPE)
/* FLAG? */
{
    int     truwiz = 0, tmp2 = 0, lev = 0;

    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument type (2)");
    if (!(oper1->data.string))
	abort_interp("Empty string argument (2)");
    if (!valid_object(oper2))
	abort_interp("Invalid object");
    ref = oper2->data.objref;
    CHECKREMOTE(ref);
    tmp = 0;
    result = 0;
    {
	char   *flag = oper1->data.string->data;

	while (*flag == '!') {
	    flag++;
	    result = (!result);
	}
	if (!*flag)
	    abort_interp("Empty flag string");

	if (string_prefix("dark", flag)
		|| string_prefix("debug", flag)) {
	    tmp = DARK;
	} else if (string_prefix("abode", flag)
		|| string_prefix("autostart", flag)
		|| string_prefix("abate", flag)) {
	    tmp = ABODE;
	} else if (string_prefix("chown_ok", flag)
		|| string_prefix("color_ansi", flag)
		|| string_prefix("count_insts", flag)) {
	    tmp = CHOWN_OK;
	} else if (string_prefix("haven", flag)
		 || string_prefix("harduid", flag)) {
	    tmp = HAVEN;
	} else if (string_prefix("jump_ok", flag)) {
	    tmp = JUMP_OK;
	} else if (string_prefix("link_ok", flag)) {
	    tmp = LINK_OK;
	} else if (string_prefix("kill_ok", flag)) {
	    tmp = KILL_OK;
	} else if (string_prefix("builder", flag)) {
	    tmp = BUILDER;
	} else if (string_prefix("mucker", flag)) {
	    lev = LMUF;
	} else if (string_prefix("muffer", flag)) {
	    lev = LMUF;
	} else if (string_prefix("nucker", flag)) {
	    lev = LM2;
	} else if (string_prefix("interactive", flag)) {
	    tmp = INTERACTIVE;
	} else if (string_prefix("sticky", flag)
		 || string_prefix("silent", flag)) {
	    tmp = STICKY;
	} else if (string_prefix("sucker", flag)) {
	    lev = LM3;

	} else if (tp_multi_wiz_levels && string_prefix("mage", flag)) {
	    lev = LMAGE;
	} else if (tp_multi_wiz_levels && string_prefix("truemage", flag)) {
	    lev = LMAGE;
	    truwiz = 1;
	} else if (tp_multi_wiz_levels && string_prefix("wizard", flag)) {
	    lev = LWIZ;
	} else if (tp_multi_wiz_levels && string_prefix("truewizard", flag)) {
	    lev = LWIZ;
	    truwiz = 1;
	} else if (tp_multi_wiz_levels && string_prefix("archwizard", flag)) {
	    lev = LARCH;
	} else if (tp_multi_wiz_levels && string_prefix("truearchwizard", flag)) {
	    lev = LARCH;
	    truwiz = 1;
	} else if (tp_multi_wiz_levels && string_prefix("boy", flag)) {
	    lev = LBOY;
	} else if (tp_multi_wiz_levels && string_prefix("trueboy", flag)) {
	    lev = LBOY;
	    truwiz = 1;

	} else if (
	    (!tp_multi_wiz_levels) && (
		string_prefix("mage", flag) ||
		string_prefix("wizard", flag) ||
		string_prefix("archwizard", flag) ||
		string_prefix("boy", flag)
	    )
	) {
	    lev = LARCH;
	} else if (
	    (!tp_multi_wiz_levels) && (
		string_prefix("truemage", flag) ||
		string_prefix("truewizard", flag) ||
		string_prefix("truearchwizard", flag) ||
		string_prefix("trueboy", flag)
	    )
	) {
	    lev = LARCH;
	    truwiz = 1;

	} else if (string_prefix("man", flag)) {
	    lev = LMAN;
	} else if (string_prefix("trueman", flag)) {
	    lev = LMAN;
	    truwiz = 1;
	} else if (string_prefix("zombie", flag)) {
	    tmp = ZOMBIE;
	} else if (string_prefix("xforcible", flag)) {
	    tmp = XFORCIBLE;
	} else if (string_prefix("vehicle", flag)
		 || string_prefix("viewable", flag)) {
	    tmp = VEHICLE;
	} else if (string_prefix("quell", flag)) {
	    tmp = QUELL;
	} else if (string_prefix("meeper", flag)) {
	    tmp2 = F2MPI;
	} else if (string_prefix("guest", flag)) {
	    tmp2 = F2GUEST;
	} else if (string_prefix("ic", flag)
		|| string_prefix("in_character", flag)) {
	    tmp2 = F2IC;
	} else if (string_prefix("idle", flag)) {
	    tmp2 = F2IDLE;
	} else if (string_prefix("pueblo", flag)) {
	    tmp2 = F2PUEBLO;
	} else if (string_prefix("tinkerproof", flag)) {
	    tmp2 = F2TINKERPROOF;
	} else if (string_prefix("www_ok", flag)) {
	    tmp2 = F2WWW;
	} else if (string_prefix("logwall", flag) && (mlev >= (tp_compatible_muf ? LMAGE : LBOY))) {
	    tmp2 = F2LOGWALL;
	} else if (string_prefix("suspect", flag) && (mlev >= (tp_compatible_muf ? LMAGE : LBOY))) {
	    tmp2 = F2SUSPECT;
	} else if (string_prefix("offer_task", flag)
		|| string_prefix("task_pending", flag)
	) {
	    tmp2 = F2OFFER;
	} else if (string_prefix("readblankline", flag)) {
	    tmp2 = F2READBLANKLINE;
	} else abort_interp("Unknown flag");
    }

  if (lev) {
    if (result)
	result = (lev >  ((truwiz) ? MLevel(ref) : QLevel(ref)));
    else
	result = (lev <= ((truwiz) ? MLevel(ref) : QLevel(ref)));
  } else if (tmp2) {
    if (result)
	result = ((FLAG2(ref) & tmp2) == 0);
    else
	result = ((FLAG2(ref) & tmp2) != 0);
  } else if (tmp) {
    if (result)
	result = (tmp && ((FLAGS(ref) & tmp) == 0));
    else
	result = (tmp && ((FLAGS(ref) & tmp) != 0));
  } else abort_interp("Unknown flag (?!)");

    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_playerp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_OBJECT)
	abort_interp("Invalid argument type");
    if (!valid_object(oper1) && !is_home(oper1)) {
	result = 0;
    } else {
	ref = oper1->data.objref;
	CHECKREMOTE(ref);
	result = (Typeof(ref) == TYPE_PLAYER);
    }
    PushInt(result);
}


void 
prim_thingp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_OBJECT)
	abort_interp("Invalid argument type");
    if (!valid_object(oper1) && !is_home(oper1)) {
	result = 0;
    } else {
	ref = oper1->data.objref;
	CHECKREMOTE(ref);
	result = (Typeof(ref) == TYPE_THING);
    }
    PushInt(result);
}


void 
prim_roomp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_OBJECT)
	abort_interp("Invalid argument type");
    if (!valid_object(oper1) && !is_home(oper1)) {
	result = 0;
    } else {
	ref = oper1->data.objref;
	CHECKREMOTE(ref);
	result = (Typeof(ref) == TYPE_ROOM);
    }
    PushInt(result);
}


void 
prim_programp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_OBJECT)
	abort_interp("Invalid argument type");
    if (!valid_object(oper1) && !is_home(oper1)) {
	result = 0;
    } else {
	ref = oper1->data.objref;
	CHECKREMOTE(ref);
	result = (Typeof(ref) == TYPE_PROGRAM);
    }
    PushInt(result);
}


void 
prim_exitp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_OBJECT)
	abort_interp("Invalid argument type");
    if (!valid_object(oper1) && !is_home(oper1)) {
	result = 0;
    } else {
	ref = oper1->data.objref;
	CHECKREMOTE(ref);
	result = (Typeof(ref) == TYPE_EXIT);
    }
    PushInt(result);
}


void 
prim_okp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (valid_object(oper1));
    CLEAR(oper1);
    PushInt(result);
}

void 
prim_location(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    CHECKREMOTE(oper1->data.objref);
    ref = DBFETCH(oper1->data.objref)->location;
    CLEAR(oper1);
    PushObject(ref);
}

void 
prim_owner(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    CHECKREMOTE(oper1->data.objref);
    ref = OWNER(oper1->data.objref);
    CLEAR(oper1);
    PushObject(ref);
}

void 
prim_controls(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object (2)");
    if (!valid_object(oper2))
	abort_interp("Invalid object (1)");
    CHECKREMOTE(oper1->data.objref);
    result = controls(oper2->data.objref, oper1->data.objref);
    CLEAR(oper1);
    PushInt(result);
}

void 
prim_perms(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object (2)");
    if (!valid_object(oper2))
	abort_interp("Invalid object (1)");
    CHECKREMOTE(oper1->data.objref);
    result = permissions(mlev, oper2->data.objref, oper1->data.objref);
    CLEAR(oper1);
    PushInt(result);
}

void 
prim_getlink(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    CHECKREMOTE(oper1->data.objref);
    if (Typeof(oper1->data.objref) == TYPE_PROGRAM)
	abort_interp("Illegal object referenced");
    switch (Typeof(oper1->data.objref)) {
	case TYPE_EXIT:
	    ref = (DBFETCH(oper1->data.objref)->sp.exit.ndest) ?
		(DBFETCH(oper1->data.objref)->sp.exit.dest)[0] : NOTHING;
	    break;
	case TYPE_PLAYER:
	    ref = DBFETCH(oper1->data.objref)->sp.player.home;
	    break;
	case TYPE_THING:
	    ref = DBFETCH(oper1->data.objref)->sp.thing.home;
	    break;
	case TYPE_ROOM:
	    ref = DBFETCH(oper1->data.objref)->sp.room.dropto;
	    break;
	default:
	    ref = NOTHING;
	    break;
    }
    CLEAR(oper1);
    PushObject(ref);
}

int 
prog_can_link_to(int mlev, dbref who, object_flag_type what_type, dbref where)
{
    if (where == HOME)
	return 1;
    if (!OkObj(where < 0))
	return 0;
    switch (what_type) {
	case TYPE_EXIT:
	    return (permissions(mlev, who, where) || (FLAGS(where) & LINK_OK));
	    break;
	case TYPE_PLAYER:
	    return (Typeof(where) == TYPE_ROOM && (permissions(mlev, who, where)
						   || Linkable(where)));
	    break;
	case TYPE_ROOM:
	    return ((Typeof(where) == TYPE_ROOM)
		    && (permissions(mlev, who, where) || Linkable(where)));
	    break;
	case TYPE_THING:
	    return ((Typeof(where) == TYPE_ROOM || Typeof(where) == TYPE_PLAYER)
		    && (permissions(mlev, who, where) || Linkable(where)));
	    break;
	case NOTYPE:
	    return (permissions(mlev, who, where) || (FLAGS(where) & LINK_OK) ||
		    (Typeof(where) != TYPE_THING && (FLAGS(where) & ABODE)));
	    break;
    }
    return 0;
}


void 
prim_setlink(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();		/* dbref: destination */
    oper2 = POP();		/* dbref: source */
    if ((oper1->type != PROG_OBJECT) || (oper2->type != PROG_OBJECT))
	abort_interp("setlink requires two dbrefs");
    if (!valid_object(oper2) && oper2->data.objref != HOME)
	abort_interp("Invalid object (1)");
    if ( tp_db_readonly )
	abort_interp( DBRO_MESG );
    ref = oper2->data.objref;
    if (oper1->data.objref == -1) {
	if (!permissions(mlev, ProgUID, ref))
	    abort_interp(NOPERM_MESG);
	switch (Typeof(ref)) {
	    case TYPE_EXIT:
		DBSTORE(ref, sp.exit.ndest, 0);
		if (DBFETCH(ref)->sp.exit.dest) {
		    free((void *) DBFETCH(ref)->sp.exit.dest);
		    DBSTORE(ref, sp.exit.dest, NULL);
		}
		if (MLevel(ref))
		    SetMLevel(ref, 0);
		break;
	    case TYPE_ROOM:
		DBSTORE(ref, sp.room.dropto, NOTHING);
		break;
	    default:
		abort_interp("Invalid object (1)");
	}
    } else {
	if (!valid_object(oper1))
	    abort_interp("Invalid object (2)");
	if (Typeof(ref) == TYPE_PROGRAM)
	    abort_interp("Program objects are not linkable (1)");
	if (!prog_can_link_to(mlev, ProgUID, Typeof(ref), oper1->data.objref))
	    abort_interp("Can't link source to destination");
	switch (Typeof(ref)) {
	    case TYPE_EXIT:
		if (DBFETCH(ref)->sp.exit.ndest != 0) {
		    if (!permissions(mlev, ProgUID, ref))
			abort_interp(NOPERM_MESG);
		    abort_interp("Exit is already linked");
		}
		if (exit_loop_check(ref, oper1->data.objref))
		    abort_interp("Link would cause a loop");
		DBFETCH(ref)->sp.exit.ndest = 1;
		DBFETCH(ref)->sp.exit.dest = (dbref *) malloc(sizeof(dbref));
		(DBFETCH(ref)->sp.exit.dest)[0] = oper1->data.objref;
		break;
	    case TYPE_PLAYER:
		if (!permissions(mlev, ProgUID, ref))
		    abort_interp(NOPERM_MESG);
		DBFETCH(ref)->sp.player.home = oper1->data.objref;
		break;
	    case TYPE_THING:
		if (!permissions(mlev, ProgUID, ref))
		    abort_interp(NOPERM_MESG);
		if (parent_loop_check(ref, oper1->data.objref))
		    abort_interp("That would cause a parent paradox");
		DBFETCH(ref)->sp.thing.home = oper1->data.objref;
		break;
	    case TYPE_ROOM:
		if (!permissions(mlev, ProgUID, ref))
		    abort_interp(NOPERM_MESG);
		DBFETCH(ref)->sp.room.dropto = oper1->data.objref;
		break;
	}
    }
    CLEAR(oper1);
    CLEAR(oper2);
}

void 
prim_setown(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();		/* dbref: new owner */
    oper2 = POP();		/* dbref: what */
    if (!valid_object(oper2))
	abort_interp("Invalid argument (1)");
    if (!valid_player(oper1))
	abort_interp("Invalid argument (2)");
    if (tp_db_readonly)
	abort_interp(DBRO_MESG);
    ref = oper2->data.objref;
    if ((mlev < (tp_compatible_muf ? LMAGE : LWIZ)) && oper1->data.objref != player)
	abort_interp(NOPERM_MESG);
    if ((mlev < MLevel(OWNER(oper1->data.objref))) ||
	(mlev < MLevel(OWNER(ref)))  )
	abort_interp(NOPERM_MESG);
    if ((mlev < (tp_compatible_muf ? LMAGE : LWIZ)) && (!(FLAGS(ref) & CHOWN_OK) ||
	!test_lock(player, ref, "_/chlk")))
	abort_interp(NOPERM_MESG);
    if ((mlev < LMAGE) && has_quotas(OWNER(oper1->data.objref)) &&
	!quota_check(OWNER(oper1->data.objref),ref,0))
	abort_interp(NOQUOTA_MESG);

    if(!tp_multi_wiz_levels) {
	/* Can't steal to/from the Man unless you are the Man */
	if ((mlev < LMAN) &&
	    (TMan(OWNER(oper1->data.objref)) || TMan(OWNER(ref)) ))
	    abort_interp(NOPERM_MESG);
    }

    switch (Typeof(ref)) {
	case TYPE_ROOM:
	    if ((mlev < LMAGE) && DBFETCH(player)->location != ref)
		abort_interp(NOPERM_MESG);
	    break;
	case TYPE_THING:
	    if ((mlev < LMAGE) && DBFETCH(ref)->location != player)
		abort_interp(NOPERM_MESG);
	    break;
	case TYPE_PLAYER:
	    abort_interp("Players always own themselves");
	case TYPE_EXIT:
	case TYPE_PROGRAM:
	    break;
	case TYPE_GARBAGE:
	    abort_interp("Can't chown garbage");
    }
    OWNER(ref) = OWNER(oper1->data.objref);
    DBDIRTY(ref);
}

void 
prim_newobject(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();		/* string: name */
    oper2 = POP();		/* dbref: location */
    if ((mlev < (tp_compatible_muf ? LM3 : LMAGE)) && (fr->already_created))
	abort_interp("Only 1 per run");
    CHECKOFLOW(1);
    ref = oper2->data.objref;
    if (!valid_object(oper2) || (!valid_player(oper2) && (Typeof(ref) != TYPE_ROOM)))
	abort_interp("Invalid argument (1)");
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument (2)");
    CHECKREMOTE(ref);
    if (!permissions(mlev, ProgUID, ref))
	abort_interp(NOPERM_MESG);
    if (!tp_building || tp_db_readonly)
	abort_interp(NOBUILD_MESG);
    if (mlev < tp_newobject_mlevel)
	abort_interp(NOPERM_MESG);
    {
	const char *b = DoNullInd(oper1->data.string);
	dbref   loc;

	if (!ok_name(b))
	    abort_interp("Invalid name (2)");

	ref = new_object();

	/* initialize everything */
	NAME(ref) = alloc_string(b);
	DBFETCH(ref)->location = oper2->data.objref;
	OWNER(ref) = OWNER(ProgUID);
	DBFETCH(ref)->sp.thing.value = 1;
	DBFETCH(ref)->exits = NOTHING;
	FLAGS(ref) = TYPE_THING;

	if ((loc = DBFETCH(player)->location) != NOTHING
		&& controls(player, loc)) {
	    DBFETCH(ref)->sp.thing.home = loc;	/* home */
	} else {
	    DBFETCH(ref)->sp.thing.home = DBFETCH(player)->sp.player.home;
	    /* set to player's home instead */
	}
    }

    /* link it in */
    PUSH(ref, DBFETCH(oper2->data.objref)->contents);
    DBDIRTY(oper2->data.objref);

    CLEAR(oper1);
    CLEAR(oper2);
    PushObject(ref);
}

void 
prim_newroom(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();		/* string: name */
    oper2 = POP();		/* dbref: location */
    if ((mlev < (tp_compatible_muf ? LM3 : LMAGE)) && (fr->already_created))
	abort_interp("Only 1 per run");
    CHECKOFLOW(1);
    ref = oper2->data.objref;
    if (!valid_object(oper2) || (Typeof(ref) != TYPE_ROOM))
	abort_interp("Invalid argument (1)");
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument (2)");
    if (!permissions(mlev, ProgUID, ref))
	abort_interp(NOPERM_MESG);
    if (!tp_building || tp_db_readonly)
	abort_interp(NOBUILD_MESG);
    if (mlev < tp_newobject_mlevel)
	abort_interp(NOPERM_MESG);
    {
	const char *b = DoNullInd(oper1->data.string);

	if (!ok_name(b))
	    abort_interp("Invalid name (2)");

	ref = new_object();

	/* Initialize everything */
	NAME(ref) = alloc_string(b);
	DBFETCH(ref)->location = oper2->data.objref;
	OWNER(ref) = OWNER(ProgUID);
	DBFETCH(ref)->exits = NOTHING;
	DBFETCH(ref)->sp.room.dropto = NOTHING;
	FLAGS(ref) = TYPE_ROOM | (FLAGS(player) & JUMP_OK);
	PUSH(ref, DBFETCH(oper2->data.objref)->contents);
	DBDIRTY(ref);
	DBDIRTY(oper2->data.objref);

	CLEAR(oper1);
	CLEAR(oper2);
	PushObject(ref);
    }
}

void 
prim_newexit(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();		/* string: name */
    oper2 = POP();		/* dbref: location */
    if (mlev < (tp_compatible_muf ? LM3 : LMAGE))
	abort_interp(tp_compatible_muf ? "M3 prim" : "Mage prim");
    CHECKOFLOW(1);
    ref = oper2->data.objref;
    if (!valid_object(oper2) || ((!valid_player(oper2)) && (Typeof(ref) != TYPE_ROOM) && (Typeof(ref) != TYPE_THING)))
	abort_interp("Invalid argument (1)");
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument (2)");
    CHECKREMOTE(ref);
    if (!permissions(mlev, ProgUID, ref))
	abort_interp(NOPERM_MESG);
    if (!tp_building || tp_db_readonly)
	abort_interp(NOBUILD_MESG);
    if (mlev < tp_newobject_mlevel)
	abort_interp(NOPERM_MESG);
    {
	const char *b = DoNullInd(oper1->data.string);

	if (!ok_name(b))
	    abort_interp("Invalid name (2)");

	ref = new_object();

	/* initialize everything */
	NAME(ref) = alloc_string(oper1->data.string->data);
	DBFETCH(ref)->location = oper2->data.objref;
	OWNER(ref) = OWNER(ProgUID);
	FLAGS(ref) = TYPE_EXIT;
	DBFETCH(ref)->sp.exit.ndest = 0;
	DBFETCH(ref)->sp.exit.dest = NULL;

	/* link it in */
	PUSH(ref, DBFETCH(oper2->data.objref)->exits);
	DBDIRTY(oper2->data.objref);

	CLEAR(oper1);
	CLEAR(oper2);
	PushObject(ref);
    }
}


void 
prim_lockedp(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2;

    /* d d - i */
    CHECKOP(2);
    oper1 = POP();		/* objdbref */
    oper2 = POP();		/* player dbref */
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (!valid_object(oper2))
	abort_interp("invalid object (1)");
    if (!valid_player(oper2) && Typeof(oper2->data.objref) != TYPE_THING)
	abort_interp("Non-player argument (1)");
    CHECKREMOTE(oper2->data.objref);
    if (!valid_object(oper1))
	abort_interp("invalid object (2)");
    CHECKREMOTE(oper1->data.objref);
    result = !could_doit(oper2->data.objref, oper1->data.objref);
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_flockedp(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2;

    /* d d - i */
    CHECKOP(2);
    oper1 = POP();		/* objdbref */
    oper2 = POP();		/* player dbref */
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (!valid_object(oper2))
	abort_interp("invalid object (1)");
    if (!valid_player(oper2) && Typeof(oper2->data.objref) != TYPE_THING)
	abort_interp("Non-player argument (1)");
    CHECKREMOTE(oper2->data.objref);
    if (!valid_object(oper1))
	abort_interp("invalid object (2)");
    CHECKREMOTE(oper1->data.objref);
    result = (FLAGS(oper1->data.objref)&XFORCIBLE)
	? !test_lock_false_default(oper2->data.objref, oper1->data.objref, "@/flk")
	: 1;
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_conlockedp(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2;

    /* d d - i */
    CHECKOP(2);
    oper1 = POP();		/* objdbref */
    oper2 = POP();		/* player dbref */
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (!valid_object(oper2))
	abort_interp("invalid object (1)");
    if (!valid_player(oper2) && Typeof(oper2->data.objref) != TYPE_THING)
	abort_interp("Non-player argument (1)");
    CHECKREMOTE(oper2->data.objref);
    if (!valid_object(oper1))
	abort_interp("invalid object (2)");
    CHECKREMOTE(oper1->data.objref);
    result = !test_lock_false_default(oper2->data.objref, oper1->data.objref, "_/clk");
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_chlockedp(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2;

    /* d d - i */
    CHECKOP(2);
    oper1 = POP();		/* objdbref */
    oper2 = POP();		/* player dbref */
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (!valid_object(oper2))
	abort_interp("invalid object (1)");
    if (!valid_player(oper2) && Typeof(oper2->data.objref) != TYPE_THING)
	abort_interp("Non-player argument (1)");
    CHECKREMOTE(oper2->data.objref);
    if (!valid_object(oper1))
	abort_interp("invalid object (2)");
    CHECKREMOTE(oper1->data.objref);
    result = (FLAGS(oper1->data.objref)&CHOWN_OK)
	? !test_lock(oper2->data.objref, oper1->data.objref, "_/chlk")
	: 1;
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_recycle(PRIM_PROTOTYPE)
{
    /* d -- */
    CHECKOP(1);
    oper1 = POP();		/* object dbref to recycle */
    if (oper1->type != PROG_OBJECT)
	abort_interp("Non-object argument (1)");
    if (!valid_object(oper1))
	abort_interp("Invalid object (1)");
    result = oper1->data.objref;
    if (mlev < LM3)
	abort_interp("M3 level required for recycle");
    
    if ( tp_db_readonly )
	abort_interp( DBRO_MESG );
    if ((result == RootRoom) || (result == EnvRoom) ||
	(result == EnvRoomX) || (result == 0))
	abort_interp("Cannot recycle that room");
    if (Typeof(result) == TYPE_PLAYER)
	abort_interp("Cannot recycle a player");
    if (result == program)
	abort_interp("Cannot recycle currently running program");
    if ( (OWNER(result) != OWNER(program)) && (
	   (mlev < (tp_compatible_muf ? LMAGE : LBOY)) ||
	   !permissions(mlev, ProgUID, result)
	 )
    )	abort_interp(NOPERM_MESG);

    {
	int     ii;

	for (ii = 0; ii < fr->caller.top; ii++)
	    if (fr->caller.st[ii] == result)
		abort_interp("Cannot recycle active program");
    }
    if (Typeof(result) == TYPE_EXIT)
	if (!unset_source(player, DBFETCH(player)->location, result))
	    abort_interp("Cannot recycle old style exits");
    CLEAR(oper1);
    recycle(player, result);
}


void 
prim_setlockstr(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!valid_object(oper2))
	abort_interp("Invalid argument type (1)");
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    ref = oper2->data.objref;
    if (!permissions(mlev, ProgUID, ref))
	abort_interp(NOPERM_MESG);
    if ( tp_db_readonly )
	abort_interp( DBRO_MESG );
    result = setlockstr(player, ref,
		oper1->data.string ? oper1->data.string->data : (char *) "");
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_getlockstr(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("Invalid argument type");
    ref = oper1->data.objref;
    CHECKREMOTE(ref);
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(NOPERM_MESG);
    {
	char   *tmpstr;

	tmpstr = (char *) unparse_boolexp(player, GETLOCK(ref), 0);
	CLEAR(oper1);
	PushString(tmpstr);
    }
}


void 
prim_part_pmatch(PRIM_PROTOTYPE)
{
    dbref ref;

    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument");
    if (!oper1->data.string)
	abort_interp("Empty string argument");
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    ref = partial_pmatch(oper1->data.string->data);
    CLEAR(oper1);
    PushObject(ref);
}


void
prim_checkpassword(PRIM_PROTOTYPE)
{
    char *ptr;

    CHECKOP(2);
    oper2 = POP();
    oper1 = POP();

    if (mlev < (tp_compatible_muf ? LMAGE : LARCH))
       abort_interp(tp_compatible_muf ? "Mage prim" : "Arch prim");
    if (oper1->type != PROG_OBJECT)
       abort_interp("Player dbref expected (1)");
    ref = oper1->data.objref;
    if ((ref != NOTHING && !valid_player(oper1)) || ref == NOTHING)
       abort_interp("Player dbref expected (1)");
    if (oper2->type != PROG_STRING)
       abort_interp("Password string expected (2)");
    ptr = (char *)(oper2->data.string? oper2->data.string->data : "");

    /* If password is blank, anything will match it */
    if ((ref != NOTHING) && (
	(!DBFETCH(ref)->sp.player.password) ||
	(!*DBFETCH(ref)->sp.player.password) ||
	!strcmp(ptr, DBFETCH(ref)->sp.player.password)
    ))
       result=1;
    else
       result=0;

    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


