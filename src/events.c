#include "config.h"
#include "params.h"
#include "local.h"

#include <time.h>

#include "db.h"
#include "interface.h"
#include "props.h"
#include "tune.h"
#include "externs.h"


/****************************************************************
 * Dump the database every so often.
 ****************************************************************/

static time_t last_dump_time = 0L;
static int dump_warned = 0;
time_t next_dump_time(void);
time_t next_clean_time(void);
#ifdef MUD
time_t next_mud_time(void);
time_t next_mob_time(void);
#endif


time_t
next_dump_time(void)
{
    time_t currtime = time((time_t *) NULL);

    if (!last_dump_time)
	last_dump_time = time((time_t *)NULL);

    if (tp_dbdump_warning && !dump_warned) {
	if (((last_dump_time + tp_dump_interval) - tp_dump_warntime)
		< currtime) {
	    return (0L);
	} else {
	    return (last_dump_time+tp_dump_interval-tp_dump_warntime-currtime);
	}
    }

    if ((last_dump_time + tp_dump_interval) < currtime)
	return (0L);
    
    return (last_dump_time + tp_dump_interval - currtime);
}

    
void
check_dump_time (void)
{
    time_t currtime = time((time_t *) NULL);
    if (!last_dump_time)
	last_dump_time = time((time_t *)NULL);

    if (!dump_warned) {
	if (((last_dump_time + tp_dump_interval) - tp_dump_warntime)
		< currtime) {
	    dump_warning();
	    dump_warned = 1;
	}
    }

    if ((last_dump_time + tp_dump_interval) < currtime) {
	last_dump_time = currtime;
	add_property((dbref)0, "~sys/lastdumptime", NULL, (int)currtime);

	if (tp_periodic_program_purge)
	    free_unused_programs();

#ifdef DELTADUMPS
	dump_deltas();
#else
	fork_and_dump();
#endif

	add_property((dbref)0, "~sys/lastdumpdone", NULL, (int)time((time_t *) NULL));

	dump_warned = 0;
    }
}


void
dump_db_now(void)
{
    time_t currtime = time((time_t *) NULL);
    add_property((dbref)0, "~sys/lastdumptime", NULL, (int)currtime);
    fork_and_dump();
    last_dump_time = currtime;
    add_property((dbref)0, "~sys/lastdumpdone", NULL, (int)time((time_t *) NULL));
    dump_warned = 0;
}

#ifdef DELTADUMPS
void
delta_dump_now(void)
{
    time_t currtime = time((time_t *) NULL);
    add_property((dbref)0, "~sys/lastdumptime", NULL, (int)currtime);
    dump_deltas();
    last_dump_time = currtime;
    add_property((dbref)0, "~sys/lastdumpdone", NULL, (int)time((time_t *) NULL));
    dump_warned = 0;
}
#endif



/*********************
 * Periodic cleanups *
 *********************/

static time_t last_clean_time = 0L;

time_t
next_clean_time(void)
{
    time_t currtime = time((time_t *) NULL);

    if (!last_clean_time)
	last_clean_time = time((time_t *)NULL);

    if ((last_clean_time + tp_clean_interval) < currtime)
	return (0L);
    
    return (last_clean_time + tp_clean_interval - currtime);
}


void
check_clean_time (void)
{
    time_t currtime = time((time_t *) NULL);

    if (!last_clean_time)
	last_clean_time = time((time_t *)NULL);

    if ((last_clean_time + tp_clean_interval) < currtime) {
	last_clean_time = currtime;
	add_property((dbref)0, "~sys/lastcleantime", NULL, (int)currtime);
	if (tp_periodic_program_purge)
	    free_unused_programs();
#ifdef DISKBASE
	dispose_all_oldprops();
#endif
    }
}


/****************
 * RWHO updates *
 ****************/

#ifdef RWHO

static time_t last_rwho_time = 0L;
static int last_rwho_nogo = 1;

time_t
next_rwho_time()
{
    time_t currtime;

    if (!tp_rwho) {
	if (!last_rwho_nogo)
	    rwhocli_shutdown();
	last_rwho_nogo = 1;
	return(3600L);
    }

    currtime = time((time_t *) NULL);

    if (last_rwho_nogo) {
	rwhocli_setup(tp_rwho_server, tp_rwho_passwd, tp_muckname, VERSION);
	last_rwho_time = currtime;
	update_rwho();
    }
    last_rwho_nogo = 0;

    if (!last_rwho_time)
	last_rwho_time = currtime;

    if ((last_rwho_time + tp_rwho_interval) < currtime)
	return (0L);
    
    return (last_rwho_time + tp_rwho_interval - currtime);
}



void
check_rwho_time (void)
{
    time_t currtime;

    if (!tp_rwho) return;

    currtime = time((time_t *) NULL);

    if (!last_rwho_time)
	last_rwho_time = currtime;

    if ((last_rwho_time + tp_rwho_interval) < currtime) {
	last_rwho_time = currtime;
	update_rwho();
    }
}

#endif /* RWHO */


/***************
 * MUD updates *
 ***************/

#ifdef MUD

static time_t last_mob_time = 0L;

time_t
next_mob_time(void)
{
    time_t currtime;

    currtime = time((time_t *) NULL);

    if (!last_mob_time)
	last_mob_time = currtime;

    if ((last_mob_time + tp_mob_interval) < currtime)
	return (0L);
    
    return (last_mob_time + tp_mob_interval - currtime);
}



void
check_mob_time (void)
{
    time_t currtime;

    if (!tp_mob || !tp_mud) return;

    currtime = time((time_t *) NULL);

    if (!last_mob_time)
	last_mob_time = currtime;

    if ((last_mob_time + tp_mob_interval) < currtime) {
	last_mob_time = currtime;
	update_mob();
    }
}


static time_t last_mud_time = 0L;

time_t
next_mud_time(void)
{
    time_t currtime;

    currtime = time((time_t *) NULL);

    if (!last_mud_time)
	last_mud_time = currtime;

    if ((last_mud_time + tp_mud_interval) < currtime)
	return (0L);
    
    return (last_mud_time + tp_mud_interval - currtime);
}



void
check_mud_time (void)
{
    time_t currtime;

    if (!tp_mud) return;

    currtime = time((time_t *) NULL);

    if (!last_mud_time)
	last_mud_time = currtime;

    if ((last_mud_time + tp_mud_interval) < currtime) {
	last_mud_time = currtime;
	update_mud();
    }
}

#endif /* MUD */


/**********************************************************************
 *  general handling for timed events like dbdumps, timequeues, etc.
 **********************************************************************/

time_t
mintime (time_t a, time_t b)
{
  return ((a>b)?b:a);
}


time_t
next_muckevent_time(void)
{
    time_t nexttime = 1000L;

    nexttime = mintime(next_event_time(), nexttime);
    nexttime = mintime(next_dump_time(), nexttime);
    nexttime = mintime(next_clean_time(), nexttime);
#ifdef RWHO
    nexttime = mintime(next_rwho_time(), nexttime);
#endif
#ifdef MUD
    nexttime = mintime(next_mud_time(), nexttime);
    nexttime = mintime(next_mob_time(), nexttime);
#endif

    return (nexttime);
}

void
next_muckevent (void)
{
    next_timequeue_event();
    check_dump_time();
    check_clean_time();
#ifdef RWHO
    check_rwho_time();
#endif
#ifdef MUD
    check_mud_time();
    check_mob_time();
#endif
}

