/*  
 *  heartbeat.c
 *  $Revision: 1.3 $ $Date: 2007/03/14 00:29:09 $
 *  We can pretend that MUF provides us a heartbeat through the use of a MUF
 *  dispatcher program in conjunction with process backgrounding. The result
 *  is less than desired, however. Adding a true heartbeat which can execute
 *  MUF programs individually is a preferred dispatching method.
 *  This eliminates the issues with the MUF programs failing or other was
 *  corrupting the stack for the master program.
 */
/*
 * $Log: heartbeat.c,v $
 * Revision 1.3  2007/03/14 00:29:09  feaelin
 * Fixed grammatical errors
 *
 * Revision 1.2  2005/03/08 18:57:36  feaelin
 * Added the heartbeat modifications. You can add programs to the @heartbeat
 * propdir and the programs will be executed every 15 seconds.
 *
 */

#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

/* commands which look at things */

#include <ctype.h>

#include "color.h"
#include "db.h"
#include "tune.h"
#include "mpi.h"
#include "props.h"
#include "interface.h"
#include "match.h"
#include "tune.h"
#include "externs.h"
#include "heartbeat.h"

int heartbeat (void)
{
  if (!OkObj(tp_heartbeat_user)) {
    log_status("ERROR: Invalid database item #%d set as the heartbeat_user. Automatically tuning heartbeat off.",
	       tp_heartbeat_user);
    tune_setparm((dbref) 1, "heartbeat", "no");
    return(0);
  }
  if (Typeof(tp_heartbeat_user) != TYPE_PLAYER) {
    log_status("HRTB: Heartbeat_user is set to Database item #%d which is not a player. Automatically turning heartbeat off.",
	       tp_heartbeat_user);
    tune_setparm((dbref) 1, "heartbeat", "no");
    return(0);
  }

  envpropqueue(tp_heartbeat_user, getloc(tp_heartbeat_user), NOTHING ,
               tp_heartbeat_user, NOTHING,
	       "@heartbeat", "HeartBeat",
	       1, 1);

  /* Move Along, nothing to see here */
  return(1);
}
