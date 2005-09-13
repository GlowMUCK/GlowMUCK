#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <stdio.h>
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif


#ifndef MALLOC_PROFILING
#  ifndef HAVE_MALLOC_H
#    include <stdlib.h>
#  else
#    include <malloc.h>
#  endif
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#include "color.h"
#include "db.h"
#include "interface.h"
#include "props.h"
#include "tune.h"
#include "match.h"
#include "reg.h"
#include "externs.h"

void
get_inquotes( char *buf, char *retbuf, int which )
{
	int i;
	retbuf[0]='\0';

	for( i=0; i<(2*(which-1)+1); i++, buf++)
	    while(*buf != '\'') if(*buf == '\0') return; else buf++;

	while(*buf != '\'' && (*buf) != '\0') (*(retbuf++)) = (*(buf++));
	*retbuf = '\0';
}

void
email_newbie( const char *name, const char *email, const char *rlname )
{
	pid_t pid;
	dbref newguy;
	char pw[ 64 ];

	if( !OkObj(tp_reg_wiz) )
	    return; /* Pooh on them! */

	/* Create random password to be mailed user: */
	reg_make_password( pw );
	if (!ok_player_name(name)) {
	    anotify( tp_reg_wiz, CFAIL "AutoReg> Sorry, that name is invalid or in use." );
	} else if (!strchr(email,'@') || !strchr(email,'.')) {
	    anotify( tp_reg_wiz, CFAIL "AutoReg> That isn't a valid email address." );
	} else if (email[0]=='<') {
	    anotify( tp_reg_wiz, CFAIL "AutoReg> Can't have <>s around the email address." );
	} else {
	    if ((newguy = create_player(name, pw)) == NOTHING) {
		anotify( tp_reg_wiz, CFAIL "AutoReg> Name exists or illegal name." );
	    } else {
		log_status("PCRE: %s(%d) by %s\n",
			   NAME(newguy), (int) newguy, unparse_object(MAN, tp_reg_wiz));
		    anotify_fmt(tp_reg_wiz,
			CSUCC "AutoReg> Player %s created as object #%d.",
			NAME(newguy), newguy);

		/* Record email address on new char: */
		{   char buf[ 1024 ];
		    char date[  40 ];

		    /* Compute date in '1993-Dec-01' format: */
		    time_t t;
		    
		    t = current_systime;
		    format_time( date,32,"%Y-%b-%d\0", localtime(&t));
		    sprintf(buf,"%s:%s:%s:%s:%s",
			NAME(newguy), rlname, email, date, NAME(tp_reg_wiz)
		    );
		    ts_modifyobject(newguy);
		    add_property(newguy, PROP_ID, buf, 0 );
		    add_property(newguy, PROP_PW, pw,  0 );
		}

		/* Mail charname and password to user: */
			/* Using system() is a HUGE security hole */
			/* We'll see if the execl code can work */
			/* You may have to experiment with your lexec, */
			/* /bin/sh, and possibly have to use system(). */
	/*	{   char buf[ 1024 ];

		    sprintf(buf, "./newuser '%s' '%s' '%s' &",
			email, NAME(newguy), pw
		    );
		    system( buf );

		}
	*/
#ifdef WIN95
		{
		    char buf[ 1024 ];

		/* Still gotta make this actually do something... */
			sprintf(buf, "./newuser '%s' '%s' '%s' &",
			email, NAME(newguy), pw
		    );
		    spawnl( P_WAIT, "/bin/sh", "sh", "-c", buf, NULL );
		}
#else
		if(!(pid=fork())) {
		    char buf[ 1024 ];

		    sprintf(buf, "./newuser '%s' '%s' '%s' &",
			email, NAME(newguy), pw
		    );
		    close(0);
		    close(1);
		    execl( "/bin/sh", "sh", "-c", buf, NULL );
		    perror("newuser execlp");
		    _exit(1);
		} else waitpid(pid,NULL,0);
#endif
	
		/** execl( "/bin/sh", "sh", "-c", "./newuser", "newuser",
			email, NAME(newguy), pw, NULL );
		**/

		{   char buf[ 80 ];
		    
		    sprintf(buf, "%s created and password emailed.", NAME(newguy));
		    do_note(-1, buf);

		    sprintf(buf, MARK "A new player has been created and mailed, %s!", NAME(newguy));
		    wall_wizards(buf);
		}
	    }
	}
}


void
hop_newbie( dbref player, int entry )
{
	char  line[BUFFER_LEN];
	char  name[BUFFER_LEN];
	char  email[BUFFER_LEN];
	char  rlname[BUFFER_LEN];

	get_file_line( LOG_HOPPER, line, entry );
	if( line[0] == '\0' )
	{
	    anotify( player, CFAIL "Invalid hopper entry." );
	    return;
	}

	get_inquotes(line, name, 1);
	get_inquotes(line, email, 2);
	get_inquotes(line, rlname, 3);

	if( (*name) == '\0' || (*email) == '\0' || (*rlname) == '\0' )
	{
	    anotify( player, CFAIL "Mangled hopper entry." );
	    return;
	}

	anotify_fmt(player, CGREEN "Name: '%s' Email: '%s' RLName: '%s'",
		name, email, rlname);

	email_newbie(name, email, rlname);
}

int hop_count(void)
{
    FILE   *f;
    char    buf[BUFFER_LEN];
    char   *p;
    int     currline;

    currline = 0;

    if ((f = fopen(LOG_HOPPER, "rb")) == NULL) {
	return 0;
    } else {
	while (fgets(buf, sizeof buf, f)) {
	    for (p = buf; *p; p++)
		if (*p == '\n') {
		    *p = '\0';
		    break;
		}
	    currline++;
	}
	fclose(f);
	return( currline );
    }
}

void
do_getpw( dbref player, const char *arg )
{
    const char *pw;
    int victim;

    if( !Boy( player ) || (Typeof(player) != TYPE_PLAYER) ) {
	anotify_fmt( player, CINFO "%s", tp_huh_mesg ); return;
    }

    if( (victim = lookup_player(arg)) == NOTHING) {
	anotify( player, CINFO WHO_MESG);
	return;
    }

    if( !controls(player, victim) ||
	(MLevel(player)<=MLevel(victim)) ||
	Man(victim)
    ) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    pw = DBFETCH(victim)->sp.player.password;

    anotify_fmt(player, "%s " CBLUE "password: '" CGLOOM "%s" CBLUE "'",
    	ansi_unparse_object(player, victim), pw ? pw : ""
    );
}

void
do_hopper( dbref player, const char *arg )
{
  char buf[BUFFER_LEN];
  char *a, *p;
  int e;
  
  a = strcpy( buf, arg );
  
  if (!Wiz(OWNER(player))) {
    anotify_fmt (player, CINFO "%s", tp_huh_mesg);
    return;
  }
  
  if ((*a == '\0') || !string_compare( a, "help" )) {
    notify (player,    "Type:");
    notify (player, "@hopper list <num1>-<num2> -- list 'num1' to 'num2' registrations");
    notify (player, "@hopper count              -- show number of regs in hopper");
    notify (player, "@hopper clear              -- clear the hopper");
    notify (player, "@hopper reg <entry>        -- perform a registration");
    notify (player, "@hopper del <entry>        -- delete a hopper entry");
    notify (player, "  ");
    notify (player, "To add a bad email to the jerk list:");
    notify (player, "@set #0=@/jerks/email@address:DYMonYR:YourWizName:Reason");
    return;
  }

  p = a;
  while( (*p) != '\0' && (*p) != ' ' ) p++;
  
  if( *p == ' ' ) *(p++) = '\0'; 
  
  while( (*p) == ' ' ) p++;

  if (!string_compare(a, "count")) {
    anotify_fmt (player, CNOTE
		 "There are %d registrations in the hopper.",
		 hop_count()
		 );
    return;
  }

  if (!string_compare (a, "list")) {
    if (hop_count() > 0) {
      spit_file_segment( player, LOG_HOPPER, p, 1 );
      anotify( player, CINFO "Done." );
    } else {
      anotify( player, CINFO "The registration hopper is empty." );
    }
    return;
  }

  if (tp_reg_wiz != player) {
    anotify( player, CFAIL "You are not set as the registration " NAMEWIZ "." );
    anotify( player, CINFO "To process or clear registrations, type:" );
    anotify( player, CNOTE "@tune reg_wiz=me" );
    return;
  }

  if (!string_compare(a, "reg")) {
    e = atoi(p);
    
    if( e <= 0 || (*p) == '\0' ) {
      anotify( player, CFAIL "Missing or invalid file entry number." );
      return;
    }
    hop_newbie( player, e );
    return;
  }

  if (!string_compare(a, "del")) {
    e = atoi(p);
    
    if (e <= 0 || (*p) == '\0') {
      anotify (player, CFAIL "Missing or invalid file entry number.");
      return;
    }
    remove_file_line(LOG_HOPPER, e);
    anotify (player, CSUCC "Removed.");
    return;
  }

  if (!string_compare(a, "clear" )) {
    if (unlink( LOG_HOPPER)) {
      perror(LOG_HOPPER);
    }
    anotify (player, CSUCC "Registration hopper cleared." );
    anotify (player, CINFO "Type 'note clear' to clear the request note list.");
    return;
  }

  anotify( player, CINFO "Unknown option, type '@hopper' for help." );
}

void
do_wizchat(dbref player, const char *arg)
{
    char buf[BUFFER_LEN];

    if( !Mage(OWNER(player)) )
    {
	anotify_fmt( player, CINFO "%s", tp_huh_mesg );
	return;
    }

    switch( arg[0] ){

      case '@':	sprintf( buf, "WizChat> %s", arg+1 );
		break;

      case ':':
      case ';': sprintf( buf, "WizChat> %s %s", NAME(player), arg+1 );
		break;

      case '?': show_wizards( player );
		return;

      case '#':
      case '\0':
		notify(player, "WizChat Help");
		notify(player, "~~~~~~~");
		notify(player, "wc message  -- say 'message'");
		notify(player, "wc :yerfs   -- pose 'yerfs'");
		notify(player, "wc ?        -- show who's listening");
		notify(player, "wc #        -- show this help list");
		return;

      default:	sprintf( buf, "WizChat> %s says, \"%s\"",
			NAME(player), arg );
		break;
    }
    wall_wizards( buf );
}

void 
do_teleport(dbref player, const char *arg1, const char *arg2)
{
    dbref   victim;
    dbref   destination;
    const char *to;
    struct match_data md;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    /* get victim, destination */
    if (*arg2 == '\0') {
	victim = player;
	to = arg1;
    } else {
	init_match(player, arg1, NOTYPE, &md);
	match_neighbor(&md);
	match_possession(&md);
	match_me(&md);
	match_here(&md);
	match_absolute(&md);
	match_registered(&md);
	if (Mage(OWNER(player)))
	    match_player(&md);

	if ((victim = noisy_match_result(&md)) == NOTHING) {
	    return;
	}
	to = arg2;
    }

    /* get destination */
    init_match(player, to, TYPE_PLAYER, &md);
    match_neighbor(&md);
    match_possession(&md);
    match_me(&md);
    match_here(&md);
    match_home(&md);
    match_absolute(&md);
    match_registered(&md);
    if (Mage(OWNER(player))) {
	match_player(&md);
    }
    switch (destination = match_result(&md)) {
	case NOTHING:
	    anotify(player, CINFO "Send it where?");
	    break;
	case AMBIGUOUS:
	    anotify(player, CINFO "I don't know where you mean!");
	    break;
	case HOME:
	    switch (Typeof(victim)) {
		case TYPE_PLAYER:
		    destination = DBFETCH(victim)->sp.player.home;
		    if (parent_loop_check(victim, destination))
			destination = DBFETCH(OWNER(victim))->sp.player.home;
		    break;
		case TYPE_THING:
		    destination = DBFETCH(victim)->sp.thing.home;
		    if (parent_loop_check(victim, destination))
			destination = DBFETCH(OWNER(victim))->sp.player.home;
		    break;
		case TYPE_ROOM:
		    destination = OkRoom(GLOBAL_ENVIRONMENT) ? GLOBAL_ENVIRONMENT : 0;
		    break;
		case TYPE_PROGRAM:
		    destination = OWNER(victim);
		    break;
		default:
		    destination = RootRoom;		/* caught in the next
							 * switch anyway */
		    break;
	    }
	default:
	    switch (Typeof(victim)) {
		case TYPE_PLAYER:
		    if (    !controls(player, victim) || 
			    ( (!controls(player, destination)) &&
			      (!(FLAGS(destination)&JUMP_OK)) &&
			      (destination!=DBFETCH(victim)->sp.player.home)
			    ) ||
			    ( (!controls(player, getloc(victim))) &&
			      (!(FLAGS(getloc(victim))&JUMP_OK)) &&
			      (getloc(victim)!=DBFETCH(victim)->sp.player.home)
			    ) ||
			    ( (Typeof(destination) == TYPE_THING) &&
			      !controls(player, getloc(destination))
			    ) ||
			    ( (Typeof(destination) == TYPE_PLAYER) &&
			      !Mage(OWNER(player))
			    )
		    ) {
			anotify(player, CFAIL NOPERM_MESG);
			break;
		    }
		    if ( Typeof(destination) != TYPE_ROOM &&
			 Typeof(destination) != TYPE_PLAYER &&
			 Typeof(destination) != TYPE_THING) {
			anotify(player, CFAIL "Bad destination.");
			break;
		    }
		    if (!Mage(victim) &&
			    (Typeof(destination) == TYPE_THING &&
				!(FLAGS(destination) & VEHICLE))) {
			anotify(player, CFAIL "Destination object is not a vehicle.");
			break;
		    }
		    if (parent_loop_check(victim, destination)) {
			anotify(player, CFAIL "Objects can't contain themselves.");
			break;
		    }
		    if((Typeof(destination)==TYPE_ROOM) && Guest(player) &&
					!(FLAG2(destination)&F2GUEST)) {
			anotify(player, CFAIL "Guests aren't allowed there.");
			break;
		    }
		    anotify(victim, CINFO "You feel a wrenching sensation...");
		    enter_room(victim, destination, DBFETCH(victim)->location);
		    anotify(player, CSUCC "Teleported.");
		    break;
		case TYPE_THING:
		    if (parent_loop_check(victim, destination)) {
			anotify(player, CFAIL "You can't make a container contain itself!");
			break;
		    }
		case TYPE_PROGRAM:
		    if (Typeof(destination) != TYPE_ROOM
			    && Typeof(destination) != TYPE_PLAYER
			    && Typeof(destination) != TYPE_THING) {
			anotify(player, CFAIL "Bad destination.");
			break;
		    }
		    if (!((controls(player, destination) ||
			    can_link_to(player, NOTYPE, destination)) &&
			    (controls(player, victim) ||
			    controls(player, DBFETCH(victim)->location)))) {
			anotify(player, CFAIL NOPERM_MESG);
			break;
		    }
		    /* check for non-sticky dropto */
		    if (Typeof(destination) == TYPE_ROOM
			    && DBFETCH(destination)->sp.room.dropto != NOTHING
			    && !(FLAGS(destination) & STICKY)
			    && !(FLAGS(victim) & ZOMBIE))
			destination = DBFETCH(destination)->sp.room.dropto;
		    moveto(victim, destination);
		    anotify(player, CSUCC "Teleported.");
		    break;
		case TYPE_ROOM:
		    if (Typeof(destination) != TYPE_ROOM) {
			anotify(player, CFAIL "Bad destination.");
			break;
		    }
		    if (!controls(player, victim)
			    || !can_link_to(player, NOTYPE, destination)
			    || victim == GLOBAL_ENVIRONMENT) {
			anotify(player, CFAIL NOPERM_MESG);
			break;
		    }
		    if (parent_loop_check(victim, destination)) {
			anotify(player, CFAIL "Parent would create a loop.");
			break;
		    }
		    moveto(victim, destination);
		    anotify(player, CSUCC "Parent set.");
		    break;
		case TYPE_GARBAGE:
		    anotify(player, CFAIL "That is garbage.");
		    break;
		default:
		    anotify(player, CFAIL "You can't teleport that.");
		    break;
	    }
	    break;
    }
    return;
}

void 
do_force(dbref player, const char *what, char *command)
{
    dbref   victim, loc;
    struct match_data md;
    int zombie = 0;

    if (force_level) {
	anotify(player, CFAIL "You can't @force an @force.");
	return;
    }

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if (!tp_zombies && (!Mage(player) || Typeof(player) != TYPE_PLAYER)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    /* get victim */
    init_match(player, what, NOTYPE, &md);
    match_neighbor(&md);
    match_possession(&md);
    match_me(&md);
    match_here(&md);
    match_absolute(&md);
    match_registered(&md);
    match_player(&md);

    if ((victim = noisy_match_result(&md)) == NOTHING) {
	return;
    }

    if (Typeof(victim) != TYPE_PLAYER && Typeof(victim) != TYPE_THING) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (Man(victim)) {
	anotify(player, CFAIL "You cannot force " NAMEMAN ".");
	return;
    }

    if (!controls(player, victim)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (!Mage(player) && !(FLAGS(victim) & XFORCIBLE)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if (!Mage(player) && !test_lock_false_default(player,victim,"@/flk")) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    loc = getloc(victim);
    if (!Mage(player) && Typeof(victim) == TYPE_THING && loc != NOTHING &&
	    (FLAGS(loc) & ZOMBIE) && Typeof(loc) == TYPE_ROOM) {
	anotify(player, CFAIL "It is in a no-puppet zone.");
	return;
    }

    if (!Mage(OWNER(player)) && Typeof(victim) == TYPE_THING) {
	const char *ptr = NAME(victim);
	char objname[BUFFER_LEN], *ptr2;
	if ((FLAGS(player) & ZOMBIE)) {
	    anotify(player, CFAIL NOPERM_MESG);
	    return;
	}
	if (FLAGS(victim) & DARK) {
	    anotify(player, CFAIL NOPERM_MESG);
	    return;
	}
	for (ptr2 = objname; *ptr && !isspace(*ptr);)
	    *(ptr2++) = *(ptr++);
	*ptr2 = '\0';
	if (lookup_player(objname) != NOTHING) {
	    anotify(player, CFAIL "Puppets cannot have a player's name.");
	    return;
	}
	zombie = 1;
    }

    if (Typeof(victim) == TYPE_PLAYER && MLevel(player) <= MLevel(victim)) {
      anotify(player, CFAIL NOPERM_MESG);
    }

    if( !zombie )
	log_status("FORC: %s by %s(%d): %s\n", unparse_object(MAN, victim),
	       NAME(player), player, command);

    /* force victim to do command */
    force_level++;
    process_command(victim, command, 0);
    force_level--;
}

void 
do_stats(dbref player, const char *name)
{
    int     rooms = 0;
    int     exits = 0;
    int     things = 0;
    int     players = 0;
    int     programs = 0;
    int     garbage = 0;
    int     tgsize = 0;
    int     total = 0;
    int     altered = 0;
    int     oldobjs = 0;
#ifdef DISKBASE
    int     loaded = 0;
    int     changed = 0;
#endif
    int     currtime = (int)current_systime;
    int     tosize=0;
    int     tpsize=0;
    int     tpcnt=0;
    int     tocnt=0;
    dbref   i;
    dbref   owner;

    if (Guest(player)) {
	anotify_fmt(player, CYELLOW "The universe contains %d objects.", db_top);
    } else {
	if (name != NULL && *name != '\0') {
	    if( !strcmp(name, "me") )
		owner = player;
	    else
		owner = lookup_player(name);
	    if (owner == NOTHING) {
		anotify(player, CINFO WHO_MESG);
		return;
	    }
	    if (   (!Mage(OWNER(player)))
		&& (OWNER(player) != owner)) {
		anotify(player, CFAIL NOPERM_MESG);
		return;
	    }
	} else owner = NOTHING;

	for (i = 0; i < db_top; i++) {

#ifdef DISKBASE
		if (((owner == NOTHING) || (OWNER(i) == owner)) &&
			DBFETCH(i)->propsmode != PROPS_UNLOADED) loaded++;
		if (((owner == NOTHING) || (OWNER(i) == owner)) &&
			DBFETCH(i)->propsmode == PROPS_CHANGED) changed++;
#endif

		/* count objects marked as changed. */
		if (((owner == NOTHING) || (OWNER(i) == owner)) && (FLAGS(i) & OBJECT_CHANGED))
		    altered++;

		/* if unused for 90 days, inc oldobj count */
		if (((owner == NOTHING) || (OWNER(i) == owner)) &&
		    (currtime - DBFETCH(i)->ts.lastused) > tp_aging_time)
		    oldobjs++;

		if ((owner == NOTHING) || (OWNER(i) == owner))
		  switch (Typeof(i)) {
		    case TYPE_ROOM:
			tocnt++, total++, rooms++;
			tosize += size_object(i,0);
			break;

		    case TYPE_EXIT:
			tocnt++, total++, exits++;
			tosize += size_object(i,0);
			break;

		    case TYPE_THING:
			tocnt++, total++, things++;
			tosize += size_object(i,0);
			break;

		    case TYPE_PLAYER:
			tocnt++, total++, players++;
			tosize += size_object(i,0);
			break;

		    case TYPE_PROGRAM:
			total++, programs++;
			
			if (DBFETCH(i)->sp.program.siz > 0) {
			    tpcnt++; tpsize += size_object(i,0);
			} else {
			    tocnt++; tosize += size_object(i,0);
			}
			break;
		}
		if ((owner == NOTHING) && Typeof(i) == TYPE_GARBAGE) {
		    total++; garbage++;
		    tgsize += size_object(i,0);
		}
	}

	anotify_fmt(player, CYELLOW "%7d room%s        %7d exit%s        %7d thing%s",
		   rooms, (rooms == 1) ? " " : "s",
		   exits, (exits == 1) ? " " : "s",
		   things, (things == 1) ? " " : "s");

	anotify_fmt(player, CYELLOW "%7d program%s     %7d player%s      %7d garbage",
		   programs, (programs == 1) ? " " : "s",
		   players, (players == 1) ? " " : "s",
		   garbage);

	anotify_fmt(player, CBLUE
		   "%7d total object%s                     %7d old & unused",
		   total, (total == 1) ? " " : "s", oldobjs);

#ifdef DISKBASE
	if (Mage(OWNER(player))) {
	    anotify_fmt(player, CWHITE
		   "%7d proploaded object%s                %7d propchanged object%s",
		   loaded, (loaded == 1) ? " " : "s",
		   changed, (changed == 1) ? "" : "s");

	}
#endif

#ifdef DELTADUMPS
	{
	    char buf[80];
	    char buf2[40];
	    struct tm *time_tm;
	    time_t lasttime=(time_t)get_property_value(0,"~sys/lastdumptime");
	    time_t lastdone=(time_t)get_property_value(0,"~sys/lastdumpdone");

	    time_tm = localtime(&lasttime);
	    (void)format_time(buf, 40, "%a %b %e %T %Z\0", time_tm);
	    buf2[0] = '\0';
	    if(lastdone>=lasttime) {
		strcpy(buf2, " took ");
		strcat(buf2, time_format_1((lastdone - lasttime)*60));
		strcat(buf2, " m:s");
	    }
	    anotify_fmt(player, CRED "%7d unsaved object%s     Last dump: %s%s",
		altered, (altered == 1) ? "" : "s", buf, buf2);
	}
#endif

	if( garbage > 0 )
	    anotify_fmt(player, CGLOOM
		"%7d piece%s of garbage %s using %d bytes of RAM.",
		garbage, (garbage == 1) ? "" : "s",
		(garbage == 1) ? "is" : "are", tgsize );

	if( tpcnt > 0 )
	    anotify_fmt(player, CPURPLE
		"%7d active program%s %s using %d bytes of RAM.",
		tpcnt, (tpcnt == 1) ? "" : "s",
		(tpcnt == 1) ? "is" : "are", tpsize );

	anotify_fmt(player, CGREEN
	    "%7d %sobject%s %s using %d bytes of RAM.",
		tocnt, tpcnt ? "other " : "",
		(tocnt == 1) ? "" : "s",
		(tocnt == 1) ? "is" : "are", tosize );
    }
}

void 
do_boot(dbref player, const char *name)
{
    dbref   victim;

    if ( Typeof(player) != TYPE_PLAYER ) return;

    if ((!Mage(player)) || (!*name)) {
	if(boot_idlest(player, 1))
	    anotify(player, CSUCC "All old connections booted.");
	else
	    anotify(player, CSUCC "You have no extra connections.");
	return;
    }
    if ((victim = lookup_player(name)) == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }
    if (Typeof(victim) != TYPE_PLAYER) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (Boy(victim)) {
	anotify(player, CFAIL "You can't boot " NAMEMAN "!");
	return;
    }
    if (!Boy(player) && TMage(victim)) {
	anotify(player, CFAIL "You can't boot " NAMEWIZ "s.");
	return;
    }

	anotify(victim, CBLUE "Shaaawing!  See ya!");
	if (boot_off(victim)) {
	    log_status("BOOT: %s by %s(%d)\n", unparse_object(MAN, victim),
		NAME(player), player);
	    if (player != victim)
		anotify_fmt(player, CSUCC "You booted %s off!", PNAME(victim));
	} else
	    anotify_fmt(player, CFAIL "%s is not connected.", PNAME(victim));
}

void 
do_jail(dbref player, const char *name, int free)
{
    dbref   victim, loc;

    if ( Typeof(player) != TYPE_PLAYER ) return;

    if (!Mage(player)) {
	anotify(player, CFAIL "Only " NAMEWIZ "s can jail someone.");
	return;
    }
    if ((victim = lookup_player(name)) == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }
    if (Typeof(victim) != TYPE_PLAYER) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (TMage(victim)) {
	anotify(player, CFAIL "You can't jail " NAMEWIZ "s.");
	return;
    }

    if((tp_jail_room == NOTHING) || (Typeof(tp_jail_room) != TYPE_ROOM) ||
	(!TMage(OWNER(tp_jail_room)))
    ) {
	anotify(player, CNOTE "@tune jail_room must be set to a room owned by a " NAMEWIZ ".");
	anotify(player, CFAIL "The jail is not set up correctly.");
	return;
    }

    loc = getloc(victim);
    
    if(loc == NOTHING) {
	anotify(player, CFAIL "That player is in limbo!");
	return;
    }

    if(free) { /* I'm free, I'm free, hahahaha! */
	if( (!Guest(victim)) || (loc != tp_jail_room)) {
	    anotify(player, CFAIL "That player is not in jail.");
	    return;
	}

	log_status("FREE: %s by %s(%d)\n", unparse_object(MAN, victim),
	    NAME(player), player);

	ts_modifyobject(victim);
	FLAG2(victim) &= ~F2GUEST;
	DBDIRTY(victim);
	send_home(victim, 0);
	anotify(victim, CNOTE MARK CSUCC "You have been set free from jail.");
	anotify(player, CSUCC "Player freed.");

    } else { /* Into the slammer sucker! */
	if( Guest(victim) && (loc == tp_jail_room)) {
	    anotify(player, CFAIL "That player is already in jail.");
	    return;
	}

	log_status("JAIL: %s by %s(%d)\n", unparse_object(MAN, victim),
	    NAME(player), player);

	ts_modifyobject(victim);
	FLAG2(victim) |= F2GUEST;
	DBDIRTY(victim);

	enter_room(victim, tp_jail_room, DBFETCH(victim)->location);

	anotify(victim, CFAIL MARK "You have been sent to jail.  Do not pass go, do not collect $200.");
	anotify(player, CSUCC "Player sent directly to jail.");
    }
}

void 
do_frob(dbref player, const char *name, const char *recip, int how)
{
    dbref   victim;
    dbref   recipient;
    dbref   stuff;
    char    buf[BUFFER_LEN];

    if (!Arch(player) || Typeof(player) != TYPE_PLAYER) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    /* if(tp_db_readonly) {
	anotify(player, CFAIL DBRO_MESG);
	return;
    } */
    if ((victim = lookup_player(name)) == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }
    if (Typeof(victim) != TYPE_PLAYER) {
	anotify(player, CFAIL "That's not a player.");
	return;
    }
    if (get_property_class( victim, PROP_PRECIOUS )) {
	anotify(player, CFAIL "That player is precious.");
	return;
    }
    if (TMage(victim)) {
	anotify(player, CFAIL "That player is a " NAMEWIZ ".");
	return;
    }
    if (!*recip || (recipient = lookup_player(recip)) == NOTHING
		|| recipient == victim
    ) {
	anotify(player, CINFO "Give their stuff to who?");
	return;
    }

    if(!controls(player, recipient)) {
	anotify_fmt(player, CFAIL "You can't force %s to take %s's belongings.",
	    NAME(recipient), NAME(victim)
	);
	return;
    }

    /* we're ok, do it */
	send_contents(player, HOME);
	for (stuff = 0; stuff < db_top; stuff++) {
	    if (OWNER(stuff) == victim) {
		switch (Typeof(stuff)) {
		    case TYPE_PROGRAM:
			dequeue_prog(stuff, 0);  /* dequeue player's progs */
			FLAGS(stuff) &= ~(ABODE | W1 | W2 | W3);
			SetMLevel(stuff,0);
		    case TYPE_ROOM:
		    case TYPE_THING:
		    case TYPE_EXIT:
			OWNER(stuff) = recipient;
			DBDIRTY(stuff);
			break;
		}
	    }
	    if (Typeof(stuff) == TYPE_THING &&
		    DBFETCH(stuff)->sp.thing.home == victim) {
		DBSTORE(stuff, sp.thing.home, RootRoom);
	    }
	}
	anotify(victim, CBLUE "You have deceased.  Been nice knowing you.");
	anotify_fmt(player, CSUCC "You turn %s into a %s.", PNAME(victim),
	    how ? "soul" : "slimy toad");
	log_status("FROB: %s by %s(%d)\n", unparse_object(MAN, victim),
		   NAME(player), player);

	if (DBFETCH(victim)->sp.player.password) {
	    free((void *) DBFETCH(victim)->sp.player.password);
	    DBFETCH(victim)->sp.player.password = 0;
	}
	dequeue_prog(victim, 0);  /* dequeue progs that player's running */

	/* reset name */
	delete_player(victim);
	sprintf(buf,
	    how ? "The soul of %s" : "A slimy toad named %s",
	    PNAME(victim));
	free((void *) NAME(victim));
	NAME(victim) = alloc_string(buf);
	DBDIRTY(victim);
	boot_player_off(victim);

	FLAGS(victim) = (FLAGS(victim) & ~TYPE_MASK) | TYPE_THING;
	OWNER(victim) = player;	/* you get it */
	DBFETCH(victim)->sp.thing.value = 1; /* lose the wealth */

	if(tp_recycle_frobs) recycle(player, victim);
}

void 
do_purge(dbref player, const char *arg1, const char *arg2)
{
    dbref   victim, thing;
    int count=0;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if( *arg1 == '\0' ) {
	notify(player, "Usage: @purge me=yourpassword");
	notify(player, "@purge deletes EVERYTHING you own.");
	notify(player, "Do this at your own risk!");
	notify(player, "You must now use your password to ensure that Joe Blow doesn't");
	notify(player, "delete all your prized treasures while you take a nature break.");
	return;
    }

    if (!strcmp( arg1, "me" ))
    	victim = player;
    else if ((victim = lookup_player(arg1)) == NOTHING) {
	anotify(player, CINFO WHO_MESG);
	return;
    }
    if (
	!controls(player, victim) ||
	Typeof(player) != TYPE_PLAYER ||
    	Typeof(victim) != TYPE_PLAYER || 
	TMage(victim)
    ) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (get_property_class( victim, PROP_PRECIOUS )) {
	anotify(player, CFAIL "That player is precious.");
	return;
    }

    if(
	((!DBFETCH(victim)->sp.player.password) ||
	 (!*DBFETCH(victim)->sp.player.password) ||
	  strcmp(arg2, DBFETCH(victim)->sp.player.password) ) &&
	( strcmp( arg2, "yes" ) ||
	  !Arch(player) )
    ) {
	anotify(player, CFAIL "Wrong password.");
	return;
    }
	
    for (thing = 2; thing < db_top; thing++) if (victim == OWNER(thing))
    {
	switch (Typeof(thing)) {
	    case TYPE_GARBAGE:
		anotify_fmt(player, CFAIL "Player owns garbage object #%d.", thing );
	    case TYPE_PLAYER:
		break;
	    case TYPE_ROOM:
		if ((thing == RootRoom) || (thing == EnvRoom) ||
		(thing == EnvRoomX) || (thing == 0))
		{
			anotify(player, CFAIL
				"Cannot recycle player start or global environment or room 0.");
			break;
		}
	    case TYPE_THING:
	    case TYPE_EXIT:
	    case TYPE_PROGRAM:
		recycle(player, thing);
		count++;
		break;
	    default:
		anotify_fmt(player, CFAIL "Unknown object type for #%d.", thing );
	}
    }
    anotify_fmt(player, CSUCC "%d objects purged.", count);
}

void 
do_newpassword(dbref player, const char *name, const char *password)
{
    dbref   victim;

    if (!Arch(player) || Typeof(player) != TYPE_PLAYER) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    } else if ((victim = lookup_player(name)) == NOTHING) {
	anotify(player, CINFO WHO_MESG);
    } else if (*password != '\0' && !ok_password(password)) {
	/* Wizards can set null passwords, but not bad passwords */
	anotify(player, CFAIL "Poor password.");

    } else if (Man(victim)) {
	anotify(player, CFAIL "You can't change " NAMEMAN "'s password!");
	return;
    } else if (TBoy(victim) && !Man(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    } else if (TMage(victim) && !Boy(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    } else {

	/* it's ok, do it */
	if (DBFETCH(victim)->sp.player.password)
	    free((void *) DBFETCH(victim)->sp.player.password);
	DBSTORE(victim, sp.player.password, alloc_string(password));
	ts_modifyobject(player);
	remove_property(victim, PROP_PW);
	anotify(player, CSUCC "Password changed.");
	anotify_fmt(victim, CINFO
		"Your password has been changed by %s.", NAME(player));
	log_status("NPAS: %s by %s(%d)\n", unparse_object(MAN, victim),
		   NAME(player), player);
    }
}

void
do_pcreate(dbref player, const char *user, const char *password)
{
    dbref   newguy;
    char    buf[BUFFER_LEN];

    if (!Mage(player) || Typeof(player) != TYPE_PLAYER) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    newguy = create_player(user, password);
    if (newguy == NOTHING) {
	anotify(player, CFAIL "Create failed.");
    } else {
	log_status("PCRE: %s(%d) by %s\n",
		   NAME(newguy), newguy, unparse_object(MAN, player));
	sprintf(buf, "original name:%s|created by:%s", NAME(newguy), NAME(player));
	add_property(newguy, PROP_ID, buf, 0 );
	anotify_fmt(player, CSUCC "Player %s(%d) created.", user, newguy);
    }
}



#ifdef DISKBASE
extern int propcache_hits;
extern int propcache_misses;
#endif

void
do_serverdebug(dbref player, const char *arg1, const char *arg2)
{
    if (!Mage(OWNER(player))) {
	anotify_fmt(player, CINFO "%s", tp_huh_mesg);
	return;
    }

#ifdef DISKBASE
    if (!*arg1 || string_prefix(arg1, "cache")) {
	anotify(player, CINFO "Cache info:");
	diskbase_debug(player);
    }
#endif

    anotify(player, CINFO "Done.");
}


#ifndef NO_USAGE_COMMAND
int max_open_files(void); 	/* from interface.c */

void 
do_usage(dbref player)
{
    int     pid, psize;
#ifdef HAVE_GETRUSAGE
    struct rusage usage;
#endif

    if (!Mage(OWNER(player))) {
	anotify_fmt(player, CINFO "%s", tp_huh_mesg);
	return;
    }
    pid = getpid();
#ifdef HAVE_GETRUSAGE
    psize = getpagesize();
    getrusage(RUSAGE_SELF, &usage);
#endif

    notify_fmt(player, "Compiled on: %s", UNAME_VALUE);
    notify_fmt(player, "Process ID: %d", pid);
    notify_fmt(player, "Max descriptors/process: %ld", max_open_files());
#ifdef HAVE_GETRUSAGE
    notify_fmt(player, "Performed %d input servicings.", usage.ru_inblock);
    notify_fmt(player, "Performed %d output servicings.", usage.ru_oublock);
    notify_fmt(player, "Sent %d messages over a socket.", usage.ru_msgsnd);
    notify_fmt(player, "Received %d messages over a socket.", usage.ru_msgrcv);
    notify_fmt(player, "Received %d signals.", usage.ru_nsignals);
    notify_fmt(player, "Page faults NOT requiring physical I/O: %d",
	       usage.ru_minflt);
    notify_fmt(player, "Page faults REQUIRING physical I/O: %d",
	       usage.ru_majflt);
    notify_fmt(player, "Swapped out of main memory %d times.", usage.ru_nswap);
    notify_fmt(player, "Voluntarily context switched %d times.",
	       usage.ru_nvcsw);
    notify_fmt(player, "Involuntarily context switched %d times.",
	       usage.ru_nivcsw);
    notify_fmt(player, "User time used: %d sec.", usage.ru_utime.tv_sec);
    notify_fmt(player, "System time used: %d sec.", usage.ru_stime.tv_sec);
    notify_fmt(player, "Pagesize for this machine: %d", psize);
    notify_fmt(player, "Maximum resident memory: %dk",
	       (usage.ru_maxrss * (psize / 1024)));
    notify_fmt(player, "Integral resident memory: %dk",
	       (usage.ru_idrss * (psize / 1024)));
#endif /* HAVE_GETRUSAGE */
}

#endif				/* NO_USAGE_COMMAND */



void
do_muf_topprofs(dbref player, const char *arg1)
{
    struct profnode {
	struct profnode *next;
	dbref  prog;
	double proftime;
	double pcnt;
	long   comptime;
	long   usecount;
    } *tops = NULL;
    struct profnode *curr = NULL;
    int nodecount = 0;
    char buf[BUFFER_LEN];
    dbref i = NOTHING;
    int count = atoi(arg1);

    if (!Mage(OWNER(player))) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if (!string_compare(arg1, "reset")) {
	for (i = db_top; i-->0; ) {
	    if (Typeof(i) == TYPE_PROGRAM && DBFETCH(i)->sp.program.code) {
		struct inst *first = DBFETCH(i)->sp.program.code;
		first->data.number = 0;
		first[1].data.number = 0;
		first[2].data.number = current_systime;
		first[3].data.number = 0;
	    }
	}
	anotify(player, CSUCC "MUF profiling statistics cleared.");
	return;
    }
    if (count < 0) {
	anotify(player, CFAIL "Count has to be a positive number.");
	return;
    } else if (count == 0) {
	count = 10;
    }

    for (i = db_top; i-->0; ) {
	if (Typeof(i) == TYPE_PROGRAM && DBFETCH(i)->sp.program.code) {
	    struct inst *first = DBFETCH(i)->sp.program.code;
	    struct profnode *newnode = (struct profnode *)malloc(sizeof(struct profnode));
	    newnode->next = NULL;
	    newnode->prog = i;
	    newnode->proftime = first->data.number;
	    newnode->proftime += (first[1].data.number / 1000000.0);
	    newnode->comptime = current_systime - first[2].data.number;
	    newnode->usecount = first[3].data.number;
	    if (newnode->comptime > 0) {
		newnode->pcnt = 100.0 * newnode->proftime / newnode->comptime;
	    } else {
		newnode->pcnt =  0.0;
	    }
	    if (!tops) {
		tops = newnode;
		nodecount++;
	    } else if (newnode->pcnt < tops->pcnt) {
		if (nodecount < count) {
		    newnode->next = tops;
		    tops = newnode;
		    nodecount++;
		} else {
		    free(newnode);
		}
	    } else {
		if (nodecount >= count) {
		    curr = tops;
		    tops = tops->next;
		    free(curr);
		} else {
		    nodecount++;
		}
		if (!tops) {
		    tops = newnode;
		} else if (newnode->pcnt < tops->pcnt) {
		    newnode->next = tops;
		    tops = newnode;
		} else {
		    for (curr = tops; curr->next; curr = curr->next) {
			if (newnode->pcnt < curr->next->pcnt) {
			    break;
			}
		    }
		    newnode->next = curr->next;
		    curr->next = newnode;
		}
	    }
	}
    }
    anotify(player, CINFO "     %CPU   TotalTime  UseCount  Program");
    while (tops) {
	curr = tops;
	sprintf(buf, CINFO "%10.3f %10.3f %9d %s", curr->pcnt, curr->proftime, (int) curr->usecount, unparse_object(player, curr->prog));
	anotify(player, buf);
	tops = tops->next;
	free(curr);
    }
    sprintf(buf, CINFO "Profile Length (sec): %5ld  %%idle: %5.2f%%  Total Cycles: %5lu",
	(current_systime-sel_prof_start_time),
        ((double)(sel_prof_idle_sec+(sel_prof_idle_usec/1000000.0))*100.0)/
	(double)((current_systime-sel_prof_start_time)+0.01),
	sel_prof_idle_use
    );
    anotify(player, buf);
    anotify(player, CINFO "*Done*");
}

void
do_mpi_topprofs(dbref player, const char *arg1)
{
    struct profnode {
        struct profnode *next;
        dbref  prog;
        double proftime;
        double pcnt;
        long   comptime;
        long   usecount;
    } *tops = NULL;
    struct profnode *curr = NULL;
    int nodecount = 0;
    char buf[BUFFER_LEN];
    dbref i = NOTHING;
    int count = atoi(arg1);

    if (!Mage(OWNER(player))) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if (!string_compare(arg1, "reset")) {
        for (i = db_top; i-->0; ) {
	    if (DBFETCH(i)->mpi_prof_use) {
	      DBFETCH(i)->mpi_prof_use = 0;
	      DBFETCH(i)->mpi_prof_usec = 0;
	      DBFETCH(i)->mpi_prof_sec = 0;
	    }
        }
	mpi_prof_start_time = current_systime;
        anotify(player, CSUCC "MPI profiling statistics cleared.");
        return;
    }
    if (count < 0) {
	anotify(player, CFAIL "Count has to be a positive number.");
	return;
    } else if (count == 0) {
        count = 10;
    }

    for (i = db_top; i-->0; ) {
        if (DBFETCH(i)->mpi_prof_use) {
            struct profnode *newnode = (struct profnode *)malloc(sizeof(struct profnode));
	    newnode->next = NULL;
            newnode->prog = i;
            newnode->proftime = DBFETCH(i)->mpi_prof_sec;
            newnode->proftime += (DBFETCH(i)->mpi_prof_usec / 1000000.0);
            newnode->comptime = current_systime - mpi_prof_start_time;
            newnode->usecount = DBFETCH(i)->mpi_prof_use;
            if (newnode->comptime > 0) {
		newnode->pcnt = 100.0 * newnode->proftime / newnode->comptime;
            } else {
		newnode->pcnt =  0.0;
            }
	    if (!tops) {
		tops = newnode;
		nodecount++;
	    } else if (newnode->pcnt < tops->pcnt) {
		if (nodecount < count) {
		    newnode->next = tops;
		    tops = newnode;
		    nodecount++;
		} else {
		    free(newnode);
		}
	    } else {
	        if (nodecount >= count) {
	            curr = tops;
	            tops = tops->next;
	            free(curr);
	        } else {
		    nodecount++;
		}
		if (!tops) {
		    tops = newnode;
		} else if (newnode->pcnt < tops->pcnt) {
		    newnode->next = tops;
		    tops = newnode;
		} else {
		    for (curr = tops; curr->next; curr = curr->next) {
			if (newnode->pcnt < curr->next->pcnt) {
			    break;
			}
		    }
		    newnode->next = curr->next;
		    curr->next = newnode;
		}
	    }
        }
    }
    anotify(player, CINFO "     %CPU   TotalTime  UseCount  Object");
    while (tops) {
        curr = tops;
        sprintf(buf, CINFO "%10.3f %10.3f %9d %s", curr->pcnt, curr->proftime, (int) curr->usecount, unparse_object(player, curr->prog));
        anotify(player, buf);
        tops = tops->next;
        free(curr);
    }
    sprintf(buf, CINFO "Profile Length (sec): %5ld  %%idle: %5.2f%%  Total Cycles: %5lu",
	    (current_systime-sel_prof_start_time),
            (((double)sel_prof_idle_sec+(sel_prof_idle_usec/1000000.0))*100.0)/
	    (double)((current_systime-sel_prof_start_time)+0.01),
	    sel_prof_idle_use);
    anotify(player, buf);
    anotify(player, CINFO "*Done*");
}

void
do_all_topprofs(dbref player, const char *arg1)
{
    struct profnode {
        struct profnode *next;
        dbref  prog;
        double proftime;
        double pcnt;
        long   comptime;
        long   usecount;
        short  type;
    } *tops = NULL;
    struct profnode *curr = NULL;
    int nodecount = 0;
    char buf[BUFFER_LEN];
    dbref i = NOTHING;
    int count = atoi(arg1);

    if (!Mage(OWNER(player))) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if (!string_compare(arg1, "reset")) {
        for (i = db_top; i-->0; ) {
	    if (DBFETCH(i)->mpi_prof_use) {
	      DBFETCH(i)->mpi_prof_use = 0;
	      DBFETCH(i)->mpi_prof_usec = 0;
	      DBFETCH(i)->mpi_prof_sec = 0;
	    }
	    if (Typeof(i) == TYPE_PROGRAM && DBFETCH(i)->sp.program.code) {
                struct inst *first = DBFETCH(i)->sp.program.code;
                first->data.number = 0;
                first[1].data.number = 0;
                first[2].data.number = current_systime;
                first[3].data.number = 0;
            }
        }
	sel_prof_idle_sec = 0;
	sel_prof_idle_usec = 0;
	sel_prof_start_time = current_systime;
	sel_prof_idle_use = 0;
	mpi_prof_start_time = current_systime;
        anotify(player, CSUCC "All profiling statistics cleared.");
        return;
    }
    if (count < 0) {
	anotify(player, CFAIL "Count has to be a positive number.");
	return;
    } else if (count == 0) {
        count = 10;
    }

    for (i = db_top; i-->0; ) {
        if (DBFETCH(i)->mpi_prof_use) {
            struct profnode *newnode = (struct profnode *)malloc(sizeof(struct profnode));
	    newnode->next = NULL;
            newnode->prog = i;
            newnode->proftime = DBFETCH(i)->mpi_prof_sec;
            newnode->proftime += (DBFETCH(i)->mpi_prof_usec / 1000000.0);
            newnode->comptime = current_systime - mpi_prof_start_time;
            newnode->usecount = DBFETCH(i)->mpi_prof_use;
	    newnode->type = 0;
            if (newnode->comptime > 0) {
		newnode->pcnt = 100.0 * newnode->proftime / newnode->comptime;
            } else {
		newnode->pcnt =  0.0;
            }
	    if (!tops) {
		tops = newnode;
		nodecount++;
	    } else if (newnode->pcnt < tops->pcnt) {
		if (nodecount < count) {
		    newnode->next = tops;
		    tops = newnode;
		    nodecount++;
		} else {
		    free(newnode);
		}
	    } else {
	        if (nodecount >= count) {
	            curr = tops;
	            tops = tops->next;
	            free(curr);
	        } else {
		    nodecount++;
		}
		if (!tops) {
		    tops = newnode;
		} else if (newnode->pcnt < tops->pcnt) {
		    newnode->next = tops;
		    tops = newnode;
		} else {
		    for (curr = tops; curr->next; curr = curr->next) {
			if (newnode->pcnt < curr->next->pcnt) {
			    break;
			}
		    }
		    newnode->next = curr->next;
		    curr->next = newnode;
		}
	    }
        }
        if (Typeof(i) == TYPE_PROGRAM && DBFETCH(i)->sp.program.code) {
            struct inst *first = DBFETCH(i)->sp.program.code;
            struct profnode *newnode = (struct profnode *)malloc(sizeof(struct profnode));
            newnode->next = NULL;
            newnode->prog = i;
            newnode->proftime = first->data.number;
            newnode->proftime += (first[1].data.number / 1000000.0);
            newnode->comptime = current_systime - first[2].data.number;
            newnode->usecount = first[3].data.number;
	    newnode->type = 1;
            if (newnode->comptime > 0) {
                newnode->pcnt = 100.0 * newnode->proftime / newnode->comptime;
            } else {
                newnode->pcnt =  0.0;
            }
            if (!tops) {
                tops = newnode;
                nodecount++;
            } else if (newnode->pcnt < tops->pcnt) {
                if (nodecount < count) {
                    newnode->next = tops;
                    tops = newnode;
                    nodecount++;
                } else {
                    free(newnode);
                }
            } else {
                if (nodecount >= count) {
                    curr = tops;
                    tops = tops->next;
                    free(curr);
                } else {
                    nodecount++;
                }
                if (!tops) {
                    tops = newnode;
                } else if (newnode->pcnt < tops->pcnt) {
                    newnode->next = tops;
                    tops = newnode;
                } else {
                    for (curr = tops; curr->next; curr = curr->next) {
                        if (newnode->pcnt < curr->next->pcnt) {
                            break;
                        }
                    }
                    newnode->next = curr->next;
                    curr->next = newnode;
                }
            }
        }
    }
    anotify(player, CINFO "     %CPU   TotalTime  UseCount  Type  Object");
    while (tops) {
        curr = tops;
        sprintf(buf, CINFO "%10.3f %10.3f %9d%5s   %s", curr->pcnt, curr->proftime, (int) curr->usecount, curr->type ? "MUF" : "MPI", unparse_object(player, curr->prog));
        anotify(player, buf);
        tops = tops->next;
        free(curr);
    }
    sprintf(buf, CINFO "Profile Length (sec): %5ld  %%idle: %5.2f%%  Total Cycles: %5lu",
	    (current_systime-sel_prof_start_time),
            ((double)(sel_prof_idle_sec+(sel_prof_idle_usec/1000000.0))*100.0)/
	    (double)((current_systime-sel_prof_start_time)+0.01),
	    sel_prof_idle_use);
    anotify(player, buf);
    anotify(player, CINFO "*Done*");
}

void 
do_memory(dbref who)
{
    if (!Mage(OWNER(who))) {
	anotify_fmt(who, CINFO "%s", tp_huh_mesg);
	return;
    }

#ifndef NO_MEMORY_COMMAND
# ifdef HAVE_MALLINFO
    {
	struct mallinfo mi;

	mi = mallinfo();
	notify_fmt(who,"Total memory arena size: %6dk", (mi.arena / 1024));
	notify_fmt(who,"Small block mem alloced: %6dk", (mi.usmblks / 1024));
	notify_fmt(who,"Small block memory free: %6dk", (mi.fsmblks / 1024));
	notify_fmt(who,"Small block mem overhead:%6dk", (mi.hblkhd / 1024));

	notify_fmt(who,"Memory allocated:        %6dk", (mi.uordblks / 1024));
	notify_fmt(who,"Mem allocated overhead:  %6dk", 
				    ((mi.uordbytes - mi.uordblks) / 1024));
	notify_fmt(who,"Memory free:             %6dk", (mi.fordblks / 1024));
	notify_fmt(who,"Memory free overhead:    %6dk",(mi.treeoverhead/1024));

	notify_fmt(who, "Small block grain: %6d", mi.grain);
	notify_fmt(who, "Small block count: %6d", mi.smblks);
	notify_fmt(who, "Memory chunks:     %6d", mi.ordblks);
	notify_fmt(who, "Mem chunks alloced:%6d", mi.allocated);
    }
# endif /* HAVE_MALLINFO */
#endif /* NO_MEMORY_COMMAND */

#ifdef MALLOC_PROFILING
    notify(who, "  ");
    CrT_summarize(who);
    CrT_summarize_to_file("malloc_log", "Manual Checkpoint");
#endif

    anotify(who, CINFO "Done.");
}

void
do_fixw(dbref player, const char *msg)
{
    int i;

    if( !Man(player) ) {
	anotify_fmt(player, CINFO "%s", tp_huh_mesg);
	return;
    }
    if( strcmp(msg, "Convert Neon flags to Glow flags.") ) {
	anotify(player, CINFO "What's the magic phrase?");
	return;
    }
    if( RawMLevel(player) != LBOY ) {
	anotify(player, CFAIL "If you want to do @fixw, you must be set W4 (neon W3).");
	return;
    }
/* Conversions
    neon 001 -> glow MPI
    neon 010 -> glow LM1
    neon 011 -> glow LM2
    neon 100 -> glow LM3
    neon 101 -> glow LMAGE
    neon 110 -> glow LWIZ
    neon 111 -> glow LARCH
    neon BOY -> glow LBOY
*/
    for( i = 0; i < db_top; i++ ) {
	if(FLAG2(i) & 0x40) /* old boy flag */
	    SetMLevel(i, LBOY);
	else if(RawMLevel(i) == 1) {
	    FLAG2(i) |= F2MPI;
	    SetMLevel(i, 0);
	} else if(RawMLevel(i) == 2)
	    SetMLevel(i, LM1);
	else if(RawMLevel(i) == 3)
	    SetMLevel(i, LM2);
	else if(RawMLevel(i) == 4)
	    SetMLevel(i, LM3);
	else if(RawMLevel(i) == 5)
	    SetMLevel(i, LMAGE);
	else if(RawMLevel(i) == 6)
	    SetMLevel(i, LWIZ);
	else if(RawMLevel(i) == 7)
	    SetMLevel(i, LARCH);
    }

    anotify(player, CINFO "Done.");
}

#if 0
    /* Converting from fuzzball to neon, don't uncomment this! */
    for( i = 0; i < db_top; i++ ) {
	if(FLAGS(i) & W3)
	    SetMLevel(i, LWIZ);
	else if((FLAGS(i) & (W2)) && (FLAGS(i) & (W1)))
	    SetMLevel(i, LM3);
	else if(FLAGS(i) & (W1))
	    SetMLevel(i, LM2);
	else if(FLAGS(i) & (W2))
	    SetMLevel(i, LM1);
    }
#endif /* method to upgrade dbs to neon mucker level system */

void
do_glowflags(dbref player, const char *msg, const char *what)
{
    int i, set = 0;
#ifdef MUD
    int val;
#endif

    if( (player != NOTHING) && !Man(player) ) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if(!string_compare(what, "all")) {
	set = 3;
    } else if(!string_compare(what, "flags")) {
	set = 1;
#ifdef MUD
    } else if(!string_compare(what, "mud")) {
	set = 2;
#endif
    } else {
	if(player != NOTHING)
	    anotify(player, CINFO "Which flags?");
	return;
    }

    if(!string_compare(msg, "load")) {
	for(i = 0; i < db_top; i++) {
	    if(!OkType(i))
		continue;

	    /* ts_modifyobject(i); */
	    if(set & 1)
		FLAGS(i) |= get_property_value(i, PROP_GLOWFL2);
#ifdef MUD
	    if(set & 2) {
		if((val = get_property_value(i, PROP_GLOWHIT)) > 0)
		    HIT(i) = val;
		if((val = get_property_value(i, PROP_GLOWMGC)) > 0)
		    MGC(i) = val;
		if((val = get_property_value(i, PROP_GLOWEXP)) > 0)
		    EXP(i) = val;
		if((val = get_property_value(i, PROP_GLOWWGT)) > 0)
		    WGT(i) = val;
		if(OkObj(val = get_property_value(i, PROP_GLOWLDR)))
		    setleader(i, val);
	    }
#endif
	}

	if(player != NOTHING)
	    anotify(player, CSUCC "Glow information loaded.");

    } else if(!string_compare(msg, "save")) {
	for(i = 0; i < db_top; i++) {
	    if(!OkType(i))
		continue;

	    /* ts_modifyobject(i); */
	    if(set & 1)
		set_property(i, PROP_GLOWFL2, PROP_INTTYP, (PTYPE)(long int)FLAG2(i));
#ifdef MUD
	    if(set & 2) {
		set_property(i, PROP_GLOWHIT, PROP_INTTYP, (PTYPE)(long int)HIT(i));
		set_property(i, PROP_GLOWMGC, PROP_INTTYP, (PTYPE)(long int)MGC(i));
		set_property(i, PROP_GLOWEXP, PROP_INTTYP, (PTYPE)(long int)EXP(i));
		set_property(i, PROP_GLOWWGT, PROP_INTTYP, (PTYPE)(long int)WGT(i));
		set_property(i, PROP_GLOWLDR, PROP_INTTYP, (PTYPE)(long int)getleader(i));
	    }
#endif
	}

	if(player != NOTHING)
	    anotify(player, CSUCC "Glow information saved.");

    } else if(!string_compare(msg, "clear")) {
	for(i = 0; i < db_top; i++) {
	    if(!OkType(i))
		continue;

	    /* ts_modifyobject(i); */
	    if(set & 1)
		set_property(i, PROP_GLOWFL2, PROP_INTTYP, (PTYPE)(long int)0);
#ifdef MUD
	    if(set & 2) {
		set_property(i, PROP_GLOWHIT, PROP_INTTYP, (PTYPE)(long int)0);
		set_property(i, PROP_GLOWMGC, PROP_INTTYP, (PTYPE)(long int)0);
		set_property(i, PROP_GLOWEXP, PROP_INTTYP, (PTYPE)(long int)0);
		set_property(i, PROP_GLOWWGT, PROP_INTTYP, (PTYPE)(long int)0);
		set_property(i, PROP_GLOWLDR, PROP_INTTYP, (PTYPE)(long int)0);
	    }
#endif
	}

	if(player != NOTHING)
	    anotify(player, CSUCC "Glow information saved.");

    } else if(player != NOTHING)
	anotify(player, CINFO "Unknown option.");


}

void
do_hostcache(dbref player, const char *arg1, const char *arg2)
{
    if( (player != NOTHING) && !Arch(player) ) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if(!string_compare(arg1, "load")) {
	host_load();
	anotify(player, CINFO "Hostname cache loaded from storage file.");
	host_list(player, "");

    } else if(!string_compare(arg1, "save")) {
	host_save();
	anotify(player, CINFO "Hostname cache saved to storage file.");
	host_list(player, "");

    } else if(!string_compare(arg1, "clear")) {
	host_free();
	anotify(player, CINFO "Hostname cache list in memory cleared.");
	host_list(player, "");

    } else if(!string_compare(arg1, "list")) {
	host_list(player, *arg2 ? arg2 : "*" );

    } else if(!string_compare(arg1, "delete")) {
	if(*arg2) {
	    int ip;
	    ip = str2ip(arg2);
	    if(host_del(ip))
		anotify(player, CINFO "Host deleted from cache.");
	    else
		anotify(player, CINFO "Host not found in cache.");
	}
    } else
	anotify(player, CINFO "Unknown option.");
}
