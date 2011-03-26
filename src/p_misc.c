/* Primitives package */

#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
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
#include "props.h"

extern struct inst *oper1, *oper2, *oper3, *oper4;
extern struct inst temp1, temp2, temp3;
extern int tmp, result;
extern dbref ref;
extern char buf[BUFFER_LEN];
struct tm *time_tm;
time_t lt;

void 
prim_time(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    CHECKOFLOW(3);
    lt = current_systime;
    time_tm = localtime(&lt);
    result = time_tm->tm_sec;
    PushInt(result);
    result = time_tm->tm_min;
    PushInt(result);
    result = time_tm->tm_hour;
    PushInt(result);
}


void 
prim_convtime(PRIM_PROTOTYPE)
{
    const char *tstr;

    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument (1)");
    if (oper1->data.string == (struct shared_string *) NULL)
	abort_interp("Illegal NULL string (1)");
    tstr = oper1->data.string->data;
    result = 0;
    if(tstr && *tstr) {
	struct tm otm;
	int mo, dy, yr, hr, mn, sc;
	yr = 70;
	mo = dy = 1;
	hr = mn = sc = 0;

	if (sscanf(tstr, "%d:%d:%d %d/%d/%d", &hr, &mn, &sc, &mo, &dy, &yr) != 6 ||
	    hr < 0 || hr > 23 ||
	    mn < 0 || mn > 59 ||
	    sc < 0 || sc > 59 ||
	    yr < 0 || yr > 99 ||
	    mo < 1 || mo > 12 ||
	    dy < 1 || dy > 31
	) abort_interp("Invalid HH:MM:SS MO/DY/YR format string (1)");

	otm.tm_mon = mo - 1;
	otm.tm_mday = dy;
	otm.tm_hour = hr;
	otm.tm_min = mn;
	otm.tm_sec = sc;
	otm.tm_year = (yr >= 70) ? yr : (yr + 100);
#ifdef SUNOS
	result = timelocal(&otm);
#else
	result = (int) mktime(&otm);
#endif
    }
    CLEAR(oper1);
    PushInt(result);
}


void
prim_date(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    CHECKOFLOW(3);
    lt = current_systime;
    time_tm = localtime(&lt);
    result = time_tm->tm_mday;
    PushInt(result);
    result = time_tm->tm_mon + 1;
    PushInt(result);
    result = time_tm->tm_year + 1900;
    PushInt(result);
}

void
prim_gmtoffset(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    CHECKOFLOW(1);
    result = get_tz_offset();
    PushInt(result);
}

void 
prim_systime(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    result = current_systime;
    CHECKOFLOW(1);
    PushInt(result);
}


void 
prim_timesplit(PRIM_PROTOTYPE)
{

    CHECKOP(1);
    oper1 = POP();		/* integer: time */
    if (oper1->type != PROG_INTEGER)
	abort_interp("Invalid argument");

    lt = (time_t) oper1->data.number;
    time_tm = localtime(&lt);
    CHECKOFLOW(8);
    CLEAR(oper1);
    result = time_tm->tm_sec;
    PushInt(result);
    result = time_tm->tm_min;
    PushInt(result);
    result = time_tm->tm_hour;
    PushInt(result);
    result = time_tm->tm_mday;
    PushInt(result);
    result = time_tm->tm_mon + 1;
    PushInt(result);
    result = time_tm->tm_year + 1900;
    PushInt(result);
    result = time_tm->tm_wday + 1;
    PushInt(result);
    result = time_tm->tm_yday + 1;
    PushInt(result);
}


void 
prim_timefmt(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper2 = POP();		/* integer: time */
    oper1 = POP();		/* string: format */
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument (1)");
    if (oper1->data.string == (struct shared_string *) NULL)
	abort_interp("Illegal NULL string (1)");
    if (oper2->type != PROG_INTEGER)
	abort_interp("Invalid argument (2)");
    if(oper2->data.number < 0)
	abort_interp("Negative time (2)");
    lt = (time_t) oper2->data.number;
    time_tm = localtime(&lt);
    if (!format_time(buf, BUFFER_LEN, oper1->data.string->data, time_tm))
	abort_interp("Operation would result in overflow");
    CHECKOFLOW(1);
    CLEAR(oper1);
    CLEAR(oper2);
    PushString(buf);
}


void 
prim_queue(PRIM_PROTOTYPE)
{
    dbref temproom;

    /* int dbref string -- */
    CHECKOP(3);
    oper1 = POP();
    oper2 = POP();
    oper3 = POP();
    if (mlev < LM3)
	abort_interp("M3 prim");
    if (oper3->type != PROG_INTEGER)
	abort_interp("Non-integer argument (1)");
    if (oper2->type != PROG_OBJECT)
	abort_interp("Argument must be a dbref (2)");
    if (!valid_object(oper2))
	abort_interp("Invalid dbref (2)");
    if (Typeof(oper2->data.objref) != TYPE_PROGRAM)
	abort_interp("Object must be a program (2)");
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (3)");

    if ((oper4 = fr->variables + 1)->type != PROG_OBJECT)
	temproom = DBFETCH(player)->location;
    else
	temproom = oper4->data.objref;

    result = add_muf_delayq_event(oper3->data.number, player, temproom,
		    NOTHING, oper2->data.objref, DoNullInd(oper1->data.string),
		     "Queued event.", 0);

    CLEAR(oper1);
    CLEAR(oper2);
    CLEAR(oper3);
    PushInt(result);
}


void 
prim_kill(PRIM_PROTOTYPE)
{
    /* i -- i */
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument (1)");
    if (oper1->data.number == fr->pid) {
	do_abort_silent();
    } else {
	if (mlev < (tp_compatible_muf ? LM3 : LMAGE)) {
	    if (!control_process(ProgUID, oper1->data.number)) {
		abort_interp(NOPERM_MESG);
	    }
	}
	result = dequeue_process(oper1->data.number);
    }
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_force(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2;

    /* d s -- */
    CHECKOP(2);
    oper1 = POP();		/* string to @force */
    oper2 = POP();		/* player dbref */
    if (mlev < LMAGE)
	abort_interp("Mage prim");
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (oper1->type != PROG_STRING)
	abort_interp("Non-string argument (2)");
    if (oper2->type != PROG_OBJECT)
	abort_interp("Non-object argument (1)");
    ref = oper2->data.objref;
    if (!OkObj(ref))
	abort_interp("Invalid object to force (1)");
    if (Typeof(ref) != TYPE_PLAYER && Typeof(ref) != TYPE_THING)
	abort_interp("Object to force not a thing or player (1)");
    if (!oper1->data.string)
	abort_interp("Null string argument (2)");
    if (index(oper1->data.string->data, '\r'))
	abort_interp("Carriage returns not allowed in command string (2)");
    if (MLevel(oper2->data.objref) > mlev)
	abort_interp("Cannot force a level higher than yours");
    if (Man(oper2->data.objref))
	abort_interp("Cannot force " NAMEMAN " (1)");
    force_level++;
    process_command(oper2->data.objref, oper1->data.string->data, 0);
    force_level--;
    CLEAR(oper1);
    CLEAR(oper2);
}


void 
prim_timestamps(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if ((!tp_compatible_muf) && (mlev < LM2))
	abort_interp("M2 prim");
    if (oper1->type != PROG_OBJECT)
	abort_interp("Non-object argument (1)");
    if (!valid_object(oper1))
	abort_interp("Invalid object");
    CHECKREMOTE(oper1->data.objref);
    CHECKOFLOW(4);
    ref = oper1->data.objref;
    CLEAR(oper1);
    result = DBFETCH(ref)->ts.created;
    PushInt(result);
    result = DBFETCH(ref)->ts.modified;
    PushInt(result);
    result = DBFETCH(ref)->ts.lastused;
    PushInt(result);
    result = DBFETCH(ref)->ts.usecount;
    PushInt(result);
}

extern int top_pid;

void 
prim_fork(PRIM_PROTOTYPE)
{
    int     i, j;
    struct frame *tmpfr;

    CHECKOP(0);
    CHECKOFLOW(1);

    if (mlev < LM3)
	abort_interp("M3 prim");

    fr->pc = pc;

    tmpfr = (struct frame *) calloc(1, sizeof(struct frame));

    tmpfr->system.top = fr->system.top;
    for (i = 0; i < fr->system.top; i++)
	tmpfr->system.st[i] = fr->system.st[i];

    tmpfr->argument.top = fr->argument.top;
    for (i = 0; i < fr->argument.top; i++)
	copyinst(&fr->argument.st[i], &tmpfr->argument.st[i]);

    tmpfr->caller.top = fr->caller.top;
    for (i = 0; i <= fr->caller.top; i++) {
	tmpfr->caller.st[i] = fr->caller.st[i];
	if (i > 0) DBFETCH(fr->caller.st[i])->sp.program.instances++;
    }

    for (i = 0; i < MAX_VAR; i++)
	copyinst(&fr->variables[i], &tmpfr->variables[i]);

    tmpfr->varset.top = fr->varset.top;
    for (i = fr->varset.top; i >= 0; i--) {
	tmpfr->varset.st[i] = (vars *) calloc(1, sizeof(vars));
	for (j = 0; j < MAX_VAR; j++)
	    copyinst(&((*fr->varset.st[i])[j]), &((*tmpfr->varset.st[i])[j]));
    }

    tmpfr->pc = pc;
    tmpfr->pc++;
    tmpfr->level = fr->level;
    tmpfr->already_created = fr->already_created;
    tmpfr->trig = fr->trig;

    tmpfr->brkpt.debugging = 0;
    tmpfr->brkpt.count = 0;
    tmpfr->brkpt.showstack = 0;
    tmpfr->brkpt.isread = 0;
    tmpfr->brkpt.bypass = 0;
    tmpfr->brkpt.lastcmd = NULL;

    tmpfr->pid = top_pid++;
    tmpfr->multitask = BACKGROUND;
    tmpfr->writeonly = 1;
    tmpfr->started = current_systime;
    tmpfr->instcnt = 0;

    /* child process gets a 0 returned on the stack */
    result = 0;
    push(tmpfr->argument.st, &(tmpfr->argument.top),
	 PROG_INTEGER, MIPSCAST & result);

    result = add_muf_delay_event(0, player, NOTHING, NOTHING, program,
				tmpfr, "BACKGROUND");

    /* parent process gets the child's pid returned on the stack */
    if (!result)
	result = -1;
    PushInt(result);
}


void 
prim_pid(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    CHECKOFLOW(1);
    result = fr->pid;
    PushInt(result);
}

void
prim_proctime(PRIM_PROTOTYPE)
{
  int result = 0;

  CHECKOP(1);
  CHECKOFLOW(1);

  oper1 = POP();
  if (oper1->type != PROG_INTEGER) {
    abort_interp("Non-integer argument (1)");
  }
  
  result = time_for_pid(oper1->data.number);
  CLEAR(oper1);
  PushInt(result);
}

void 
prim_stats(PRIM_PROTOTYPE)
{
    int rooms, exits, things, players, programs, garbage;

    rooms = exits = things = players = programs = garbage = 0;

    /* A WhiteFire special. :) */
    CHECKOP(1);
    CHECKOFLOW(7);
    oper1 = POP();
    if (mlev < (tp_compatible_muf ? LM3 : LM2))
	abort_interp(tp_compatible_muf ? "M3 prim" : "M2 prim");
    if (!valid_player(oper1) && (oper1->data.objref != NOTHING))
	abort_interp("non-player argument (1)");
    ref = oper1->data.objref;
    CLEAR(oper1);
    ref = count_stats(ref,
		&rooms, &exits, &things, &players, &programs, &garbage);
    PushInt(ref);
    PushInt(rooms);
    PushInt(exits);
    PushInt(things);
    PushInt(programs);
    PushInt(players);
    PushInt(garbage);
    /* push results */
}

void 
prim_abort(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument");
    strcpy(buf, DoNullInd(oper1->data.string));
    abort_interp(buf);
}


void 
prim_ispidp(PRIM_PROTOTYPE)
{
    /* i -- i */
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument (1)");
    if (oper1->data.number == fr->pid) {
	result = 1;
    } else {
	result = in_timequeue(oper1->data.number);
    }
    CLEAR(oper1);
    PushInt(result);
}


void 
prim_parselock(PRIM_PROTOTYPE)
{
    struct boolexp *lok;

    CHECKOP(1);
    oper1 = POP();		/* string: lock string */
    CHECKOFLOW(1);
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument");
    if (oper1->data.string != (struct shared_string *) NULL) {
	lok = parse_boolexp(ProgUID, oper1->data.string->data, 0);
    } else {
	lok = TRUE_BOOLEXP;
    }
    CLEAR(oper1);
    PushLock(lok);
}


void 
prim_unparselock(PRIM_PROTOTYPE)
{
    const char *ptr;

    CHECKOP(1);
    oper1 = POP();		/* lock: lock */
    if (oper1->type != PROG_LOCK)
	abort_interp("Invalid argument");
    if (oper1->data.lock != (struct boolexp *) TRUE_BOOLEXP) {
	ptr = unparse_boolexp(ProgUID, oper1->data.lock, 0);
    } else {
	ptr = NULL;
    }
    CHECKOFLOW(1);
    CLEAR(oper1);
    if (ptr) {
	PushString(ptr);
    } else {
	PushNullStr;
    }
}


void 
prim_prettylock(PRIM_PROTOTYPE)
{
    const char *ptr;

    CHECKOP(1);
    oper1 = POP();		/* lock: lock */
    if (oper1->type != PROG_LOCK)
	abort_interp("Invalid argument");
    ptr = unparse_boolexp(ProgUID, oper1->data.lock, 1);
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushString(ptr);
}


void 
prim_testlock(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2;

    /* d d - i */
    CHECKOP(2);
    oper1 = POP();		/* boolexp lock */
    oper2 = POP();		/* player dbref */
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (!valid_object(oper2))
	abort_interp("Invalid argument (1).");
    if (Typeof(oper2->data.objref) != TYPE_PLAYER &&
	Typeof(oper2->data.objref) != TYPE_THING )
    {
	abort_interp("Invalid object type (1).");
    }
    CHECKREMOTE(oper2->data.objref);
    if (oper1->type != PROG_LOCK)
	abort_interp("Invalid argument (2)");
    result = eval_boolexp(oper2->data.objref, oper1->data.lock, player);
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_sysparm(PRIM_PROTOTYPE)
{
    const char *ptr;
    const char *tune_get_parmstring(const char *name, int mlev);

    CHECKOP(1);
    oper1 = POP();		/* string: system parm name */
    if (oper1->type != PROG_STRING)
	abort_interp("Invalid argument");
    if (oper1->data.string) {
	ptr = tune_get_parmstring(oper1->data.string->data, mlev);
    } else {
	ptr = "";
    }
    CHECKOFLOW(1);
    CLEAR(oper1);
    PushString(ptr);
}


void 
prim_system(PRIM_PROTOTYPE)
{
    const char *tstr;

    CHECKOP(1);
    oper1 = POP();
    result = 0;
    if (mlev < LMAN)
	abort_interp("System prim can only be used by #1");
    if (fr->level > 8)
	abort_interp("Interp call loops not allowed");
    if (oper1->type != PROG_STRING || !oper1->data.string)
	abort_interp("Invalid argument");

    tstr = oper1->data.string->data;

    if((!tstr) || (!*tstr))
	abort_interp("Illegal NULL string (1)");

#ifdef INSECURE_SYSTEM_MUF_PRIM
    result = system(tstr);
#else
    abort_interp("system is not supported");
#endif

    CLEAR(oper1);
    PushInt(result);
}


void 
prim_email_password(PRIM_PROTOTYPE)
{
  const char* email = NULL;
  const char* pw = NULL;
  pid_t pid = 0;

  CHECKOP(1);
  oper1 = POP();

  if (mlev < LBOY) {
    abort_interp("Email_password prim requires " NAMEMAN);
  }
  if (!valid_object(oper1)) {
    abort_interp("Invalid argument");
  }

  ref = oper1->data.objref;
  if (Typeof(ref) != TYPE_PLAYER) {
    abort_interp("DBref is not a player");
  }

  if (MLevel(ref) > LM3) {
    abort_interp("Cannot use email_password for a wizard");
  }

  email = get_property_class(ref, "/@/Registration/E-MailAddress");

#ifdef COMPRESS
    if (email) email = uncompress(email);
#endif

  if (email == NULL) {
    abort_interp("EMail address not available");
  }

  pw = DBFETCH(ref)->sp.player.password;

  CHECKREMOTE(oper1->data.objref);
#ifdef WIN95
  {
    char buf[ 1024 ];
    /* Still gotta make this actually do something... */
    sprintf(buf, "./sendpass '%s' '%s' '%s' &", email, NAME(ref), pw );
    spawnl( P_WAIT, "/bin/sh", "sh", "-c", buf, NULL );
  }
#else
  if (!(pid=fork()) ) {
    char buf[ 1024 ];
    
    sprintf(buf, "./sendpass '%s' '%s' '%s' &", email, NAME(ref), pw );
    close(0);
    close(1);
    execl( "/bin/sh", "sh", "-c", buf, NULL );
    perror("sendpass execlp");
    _exit(1);
  } else {
    waitpid(pid,NULL,0);
  }
#endif
	
  {
    char buf[ 80 ];
    
    sprintf(buf, "E-mailed password to %s.", NAME(ref));
    do_note(-1, buf);
  }
  CLEAR(oper1);
}
