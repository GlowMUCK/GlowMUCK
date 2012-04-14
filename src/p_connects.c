/* Primitives package */

#include "copyright.h"
#include "config.h"
#include "params.h"
#include "local.h"

#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#include "db.h"
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


void 
prim_awakep(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("invalid argument");
    ref = oper1->data.objref;
    if (Typeof(ref) == TYPE_THING && (FLAGS(ref) & ZOMBIE))
	ref = OWNER(ref);
    if (Typeof(ref) != TYPE_PLAYER)
	abort_interp("invalid argument");
    result = online(ref);
    PushInt(result);
}

void 
prim_ignoringp(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper2 = POP();
    oper1 = POP();
    if (!valid_object(oper1))
	abort_interp("invalid argument (1)");
    if (!valid_object(oper2))
	abort_interp("invalid argument (2)");
    ref = oper1->data.objref;
    ref = OWNER(ref);
    if (Typeof(ref) != TYPE_PLAYER)
	abort_interp("invalid argument (1)");
    result = ignoring(ref, OWNER(oper2->data.objref));
    PushInt(result);
}

void 
prim_online(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    result = pcount();
    CHECKOFLOW(result + 1);
    while (result) {
	ref = pdbref(result--);
	PushObject(ref);
    }
    result = pcount();
    PushInt(result);
}


void 
prim_concount(PRIM_PROTOTYPE)
{
    /* -- int */
    CHECKOP(0);
    result = pcount();
    CHECKOFLOW(1);
    PushInt(result);
}

void 
prim_condbref(PRIM_PROTOTYPE)
{
    /* int -- dbref */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    result = pdbref(result);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushObject(result);
}


void 
prim_conidle(PRIM_PROTOTYPE)
{
    /* int -- int */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    result = pidle(result);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_conidler(PRIM_PROTOTYPE)
{
    /* int -- int */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    result = pidler(result);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_contime(PRIM_PROTOTYPE)
{
    /* int -- int */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    result = pontime(result);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_conhost(PRIM_PROTOTYPE)
{
    /* int -- char * */
    const char   *pname;

    CHECKOP(1);
    oper1 = POP();
    if (mlev < LMAGE)
	abort_interp("Mage prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    pname = phost(result);
    if((!tp_compatible_muf) && (mlev < LWIZ))
	pname = hostname_domain(pname);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushString(pname);
}

void 
prim_conuser(PRIM_PROTOTYPE)
{
    /* int -- char * */
    char   *pname;

    CHECKOP(1);
    oper1 = POP();
    if (mlev < (tp_compatible_muf ? LMAGE : LWIZ))
	abort_interp(tp_compatible_muf ? "Mage prim" : "Wiz prim");
    if (oper1->type != PROG_INTEGER)
       abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
       abort_interp("Invalid connection number (1)");
    pname = puser(result);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushString(pname);
}

void 
prim_conipnum(PRIM_PROTOTYPE)
{
    /* int -- char * */
    char   *pname;

    CHECKOP(1);
    oper1 = POP();
    if (mlev < (tp_compatible_muf ? LMAGE : LWIZ))
	abort_interp(tp_compatible_muf ? "Mage prim" : "Wiz prim");
    if (oper1->type != PROG_INTEGER)
       abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
       abort_interp("Invalid connection number (1)");
    pname = pipnum(result);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushString(pname);
}

void
prim_con_listener_port(PRIM_PROTOTYPE)
{
  /* int -- int */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < LMAGE)
       abort_interp("Mage prim");
    if (oper1->type != PROG_INTEGER)
       abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
       abort_interp("Invalid connection number (1)");
    CHECKOFLOW(1);
    CLEAR(oper1);
    result = lport(result);
    PushInt(result);
}

void 
prim_conport(PRIM_PROTOTYPE)
{
    /* int -- char * */
    char   *pname;

    CHECKOP(1);
    oper1 = POP();
    if (mlev < LMAGE)
       abort_interp("Mage prim");
    if (oper1->type != PROG_INTEGER)
       abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
       abort_interp("Invalid connection number (1)");
    pname = pport(result);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushString(pname);
}

void
prim_conboot(PRIM_PROTOTYPE)
{
    /* int --  */
    CHECKOP(1);
    if (mlev < (tp_compatible_muf ? LMAGE : LARCH))
	abort_interp(tp_compatible_muf ? "Mage prim" : "Arch prim");
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    CLEAR(oper1);
    pboot(result);
}


void 
prim_connotify(PRIM_PROTOTYPE)
{
    /* int string --  */
    CHECKOP(2);
    oper2 = POP();		/* string */
    oper1 = POP();		/* int */
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    if (oper2->type != PROG_STRING)
	abort_interp("Argument not an string (2)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    if (oper2->data.string)
	pnotify(result, oper2->data.string->data);
    CLEAR(oper1);
    CLEAR(oper2);
}


void 
prim_condescr(PRIM_PROTOTYPE)
{
    /* int -- int */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < LM3)
	abort_interp("M3 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    if ((result < 1) || (result > pcount()))
	abort_interp("Invalid connection number (1)");
    result = pdescr(result);
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_descrcon(PRIM_PROTOTYPE)
{
    /* int -- int */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < LM3)
	abort_interp("M3 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    result = pdescrcon(result);
    CLEAR(oper1);
    PushInt(result);
}

void 
prim_nextdescr(PRIM_PROTOTYPE)
{
    /* int -- int */
    CHECKOP(1);
    oper1 = POP();
    if (mlev < LM3)
	abort_interp("M3 prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Argument not an integer (1)");
    result = oper1->data.number;
    result = pnextdescr(result);
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_descriptors(PRIM_PROTOTYPE)
{
    int  mydescr, mycount = 0;
    int  di, dcount, descr;
    int* darr;

    CHECKOP(1);
    oper1 = POP();
    if (mlev < LM3)
	abort_interp("M3 prim");
    if (oper1->type != PROG_OBJECT)
	abort_interp("Argument not a dbref");
    if (oper1->data.objref != NOTHING && !valid_object(oper1))
	abort_interp("Bad dbref");
    ref = oper1->data.objref;
    if ((ref != NOTHING) && (!valid_player(oper1)))
	abort_interp("Non-player argument");
    CLEAR(oper1);
    CHECKOP(0);

    if (ref == NOTHING) {
        for (result = pcount(); result; result--) {
            CHECKOFLOW(1);
            mydescr = pdescr(result);
            PushInt(mydescr);
            mycount++;
        }
    } else {
        if (Typeof(ref) == TYPE_PLAYER) {
            darr   = DBFETCH(ref)->sp.player.descrs;
            dcount = DBFETCH(ref)->sp.player.descr_count;
            if (!darr)
                dcount = 0;
        } else {
            darr = NULL;
            dcount = 0;
        }
        for (di = 0; di < dcount; di++) {
            CHECKOFLOW(1);
            descr = index_descr(darr[di]);
            PushInt(descr);
            mycount++;
        }
    }
    CHECKOFLOW(1);
    PushInt(mycount);
}

void
prim_descr_setuser(PRIM_PROTOTYPE)
{
    char *ptr;

    CHECKOP(3);
    oper3 = POP();
    oper2 = POP();
    oper1 = POP();

    if (mlev < (tp_compatible_muf ? LMAGE : LARCH))
	abort_interp(tp_compatible_muf ? "Mage prim" : "Arch prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Integer descriptor number expected (1)");
    if (oper2->type != PROG_OBJECT)
	abort_interp("Player dbref expected (2)");
    ref = oper2->data.objref;
    if (ref != NOTHING && !valid_player(oper2))
	abort_interp("Player dbref expected (2)");
    if (oper3->type != PROG_STRING)
	abort_interp("Password string expected");
    ptr = oper3->data.string? oper3->data.string->data : "";
    if ((ref != NOTHING) && DBFETCH(ref)->sp.player.password &&
	(*DBFETCH(ref)->sp.player.password) &&
	strcmp(ptr, DBFETCH(ref)->sp.player.password)
    )	abort_interp("Incorrect password");

    if (ref != NOTHING) {
	log_status("SUID: %d %s to %s(%d)\n",
		oper1->data.number, unparse_object(MAN, player), NAME(ref), ref);
    }
    tmp = oper1->data.number;

    CLEAR(oper1);
    CLEAR(oper2);
    CLEAR(oper3);

    result = pset_user(tmp, ref);

    PushInt(result);
}


void
prim_descr_setecho(PRIM_PROTOTYPE)
{
    int onoff;

    CHECKOP(2);
    oper2 = POP();
    oper1 = POP();

    if (mlev < (tp_compatible_muf ? LMAGE : LARCH))
	abort_interp(tp_compatible_muf ? "Mage prim" : "Arch prim");
    if (oper1->type != PROG_INTEGER)
	abort_interp("Integer descriptor number expected (1)");
    if (oper2->type != PROG_INTEGER)
	abort_interp("Integer on/off value expected (2)");

    tmp = oper1->data.number;
    onoff = oper2->data.number;

    CLEAR(oper1);
    CLEAR(oper2);

    result = pset_echo(tmp, onoff);

    PushInt(result);
}

