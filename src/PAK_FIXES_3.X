
GlowMuck source code is Copyright 1996-1999 by Andrew Nelson, all rights
reserved.  This product is supplied 'AS IS' with absolutely no warranties,
express or implied.  Use it at your own risk.

The Fuzzball and TinyMuck distributions from which GlowMuck was derived have
additional copyrights and disclaimers to be found in the standard fuzzball
tinymuck source code distribution which must be adhered to when using this
server distribution.  The current site to acquire FuzzBall code is
ftp://ftp.belfry.com/pub/fuzzball. Older muck versions can be found at
ftp://ftp.tcp.com/pub/muds.

And now, PakRat and Artie Productions bring you the...

		* * * * * * * * * * * * * * * * *
		* GlowMuck Shameless Hacks List *
		* * * * * * * * * * * * * * * * *
Note: There are forums for discussion, trackers for bugs, support and feature
requests all at: http://sourceforge.net/projects/glowmuck/. The latest versions
of the standard MUF programs and glowmuck itself will be posted there.

Note: Below, I (Iain E. Davis) have flagged my entries with [IDE], so that you can
properly assign blame...:). Anything not marked is presumably done by PakRat.

03/18/04 [IDE]: Hopefully, I've correctly corrected the variable conversions in
  compress.c and interface.c that were causing MSVC to generate
  warnings. Fixed the trigraph warning thrown out by gcc in interface.c and
  the operation on 'prop' is undefined error. Fixed the 'operation on curpath is
  undefined' error in path.c.

## 08/04/02: Release 3.1.8, bug fixes, system muf prim ##

08/04/02: Fixed (?) the fold MPI prim to properly check a MesgParse return
  pointer (every other call to MesgParse checks the return pointer, this
  could be a crashing bug.)  I have confirmed it happens in FB 5.68 using
  the 'parseprop' prim, but it can be a little tricker to make it happen.
  One annoying quirk is if you have MPI parsed in what people say and pose,
  others can keep executing this bug and spy on other conversations.

  Changed the CLEARLOCK macro to just remove the _/lok property instead of
  setting it to the *UNLOCKED* state.  I'm not sure why we'd want a stray
  property floating around that evaluates to the same thing if the property
  isn't there.  This affects 'setlockstr' and '@unlock'.  It was reported
  that the *UNLOCKED* setting may cause problems with database saves, so
  just removing the property is the easiest fix.

  Changed the 'timefmt' muf primitive to abort if a negative time is given.
  On Windows the localtime() function may crash if a negative time is used.

  Added support for the system() function of Unix and Windows using the muf
  'system' primitive.  This feature will only be active if the user compiles
  the server with INSECURE_SYSTEM_MUF_PRIM defined in local.h.  Otherwise
  the feature will issue an abort_interp() saying it is not supported.  Only
  muf programs running with the MAN (M8) privilege can execute this prim.
  ***MASSIVE HUGE WARNING***: You can execute ANYTHING on the host system
  with this feature.  It is a HUGE security risk.  You have been warned!
  Also, unless you are in Unix and append an ampersand '&' to the end of the
  command, THE MUCK WILL BLOCK until the script or program finishes.  If you
  kill the spawned and stalled command in a shell prompt, the muck server
  should start up again.  Executing the system muf primitive regularly is
  NOT recommended as it WILL cause lag.  Once again, YOU HAVE BEEN WARNED!
  You should not use this feature unless you really know what you are doing
  and are willing to take these risks.  That is why this feature is disabled
  by default.  I take no responsibility for crackers figuring out a hole in
  some muf program you wrote that uses this feature that lets them e-mail
  themselves your /etc/passwd file or gain access to your server account.
  If you do use this feature, I highly recommend you do NOT support any
  user input being used as an argument to this function.  Yes, it is possible
  to build scripts that let you check on your muck's state via 'ps' and pipe
  it to a file you can read on the muck in the 'info' directory (at least in
  Unix, not sure about Windows).

## 12/11/00: Release 3.1.7, FB5.68, minor tweaks, stack usage reduction ##

12/11/00: Fixed a bug with hosts not being cached or retrieve if the
  SPAWN_HOST_RESOLVER option is not defined.  This causes hosts under
  Windows 95 to have to be looked up every time.  The implementation will
  not look at a host IP at all if it is in the cache and the resolver is
  not being used.  To compensate for this issue, I added a 'delete' option
  to the @hostcache function to remove hosts that for some reason are saved
  as their IP number instead of name.

11/03/00: Upgraded to FB5.68.  I'm a little more than a little wary of that
  newly added 'select' MPI function.  It looks a bit too complex to be stable. 
  Time will tell. :/

  Changed the MPI MesgParse function to use malloc() instead of stack space
  for its gobs and gobs of recursive buffers.  This should allow the Windows
  version to safely use the full 26 levels of recursion now.  Knock on wood.

09/04/00: Added an environment_room parameter to separate player start
  from the real environment room used for room-zero things.


## 07/09/00: Release 3.1.6, FB5.67, crash fixes, I'm 26, so old *creak* ##

07/09/00: Fixed hard-coded SET* macros for sex/pos/species props in db.h to
  use the PROP_SEX, PROP_POS, and PROP_SPECIES defines.

  Just in case there is still a problem compiling on some odd versions of
  gcc, I took out the '-Werror' from Makefile.in.  Since I know there are
  warnings on 64-bit compilers, it seems to make sense to do this.

  Fixed a few quirks in the restart.csh and restart.sh scripts, namely not
  specifying 'a' in the ps command, which should allow a server hosting
  multiple glowmucks to allow one per user and not complain.

06/29/00: Happy birthday to me, happy birthday to me, happy BIRTHday to
  me-ee.  Happy birthday to me.  What, you think I'm gonna fix bugs on my
  birthday?  >:)  Mmm, birthday cake...

06/22/00: Reworked the connection lookup speed improvements of FB 5.66 to
  use a double lookup of index numbers.  The new method introduced messes up
  on Windows 98/NT because of the way Windows allocates socket numbers.  My
  fix will support any descriptor number no matter what the table size is. 
  There is a new definition that specifies the maximum number of concurrent
  connections, defaulting at 500.  This is obviously high but shouldn't
  waste much space as it's just an integer array.  I don't know of any mucks
  running higher than 500 concurrent players online at a time (even Furry is
  only 400.)

  Added a new configurable definition called PLAYER_NAME_SPACE to fix the
  problem of inserver WHO not printing correctly for mucks that compile the
  server with a PLAYER_NAME_LIMIT higher than 16.  Just add a space
  character to this string for each letter past 16. (17 = " ", 18 = "  "
  etc.) I chose this hackish way of doing it to prevent having to add a
  couple more sprintf()s when all the text really is static and it's
  pointless to use sprintfs when static text can do.

  Added support for welcome pages to use hostname pattern matching via the
  validate_hostnames mechanism.  The format is:
    @set #0=@/welcome/*muq.org:muq
  This would cause all people whose hostnames end in 'muq.org' to see the
  file glow/game/data/welcome/muq.txt.

02/13/00: Added an NHLP status log entry for when players ask for help that
  doesn't exist.  Sysadmins can grep for NHLP in the status log and add help
  or aliases to help topics for items people often try to get help on.  This
  logging is only done when the log_failed_commands is @tuned on.


## 01/30/00: Release 3.1.5, feeps, FB5.66, crash fixes, SuperBowl XXXIV ##

01/29/00: Fixed yet another FB crashing bug in prim_stats, it doesn't check
  if the 7 result arguments can fit before pushing them.

  Fixed a problem with linewrap leaving spaces at the start of a line if the
  line wrapped is exactly the length of the wrap.  There is also an
  additional WRAP_STRIPLEAD #define that can be put in local.h if you want a
  more agressive stripping of spaces at the start of wrapped lines.  The
  first line does not get stripleaded, only inbetween lines.

  Fixed an FB bug where @toading a character will leave a -1 in the 'online'
  muf/mpi list causing many programs to crash.  I added several safeguards
  to make sure that disconnected players are cleaned up.  Toaded players are
  @booted and unconnected immediately in the boot_player_off call so they
  can't sit for a second or so in the connection list after they are no
  longer a player (race condition).  I also added code in shutdownsock so
  any booted player will be forgotten if nothing else catches it, thus
  'dboot' can be used to cure the problem (as opposed to a @restart) should
  there still be some condition that can cause it.

  Added a NOTHING_MESG in config.h that can be put in local.h for
  customization to put something else where you see 'You see nothing
  special.' when looking at items.

01/01/00: The first change of the 19th century!  Yep Glowmuck is now
  reverted to it's pre-existing state.  All code has mysteriously
  disappeared!  Just kidding.

  The first feep of the new millenium!  Er well ok so the millenium is next
  year.  The first feep of the new century!  Not sure if centuries are
  counted starting at year 0 or year 1 either.  Argh.  The first feep of the
  new year!  Yeah I gotta be safe with that one.  Added a new @tune,
  newobject_mlevel that is independent of other checks.  This is to allow
  mucks to provide a minimum mlevel that can use newobject, newroom,
  newexit, and copyobj.  I believe right now any M1 or above can use these
  prims, but are limited to one creation per program run.  It looks like
  some of them can only be done by mages.  The default of the @tune is M1,
  to provide backward compatibility.  Mucks wanting to prevent lower mlevels
  from creating objects with these prims should raise this @tune value.

12/31/99: The last changes before the end of the millenium!!! Wow, this
  could be it folks, this could be the end.  The next change will actually
  be done in the year 1900 since I haven't been using Y2K compliant date
  formats for my change log entries. ;)

  Added two more @tunes... read_bytes and write_bytes.  These are cumulative
  values that keep track of the total number of bytes written from and read
  by the muck.

  Added a timestamp to the end of the property that is generated when a
  DOS attack occurs.  (In #0=@/sites/0.0.0 x:DOS: ...)  The format is the
  same as ctime() (24 character format).

12/29/99: Added in fb5.66 changes.  I'm not sure if more things need to be
  converted to use the new 'fast-find' feature Foxen put in to speed up
  things that do player descriptor lookups.

12/21/99: Changed the quirky way the more pager interacts with the
  readblankline muf feature.  Before the change, a blank line (just pressing
  return) would trigger the more pager and send a space to the readblankline
  muf program, which would be very confusing to the end user.  Now the more
  pager has priority.  Once all of the paged text has been sent, additional
  returns will then send a single space to readblankline muf programs.

  Removed a couple tp_who_doing's from look.c's Pecho and Oecho clauses.

12/20/99: Added a new IMORE_MESSAGE and a WHO-like 'MORE' hook so it is
  possible within the muf editor and other editors like lsedit to still get
  the next page if the user can't just press enter to get it.  The hook
  works us like WHO, it must be typed in all caps.  It is possible to
  specify line numbers or turn off more paging by doing 'MORE off'.  The
  IMORE_MESSAGE is intended to let the user they need to type MORE in all
  caps.

  Fixed exit priorities for ABATE objects.  My last attempt at getting it
  right caused it so no exits set ABATE are matched at all.  Now they are
  properly treated as one level below not having a muffer bit set as they
  should be.  ABATE negates any other mucker level set on the exit.

12/16/99: Increased the amount of text that can be specified for CGI scripts
  from the current 80 to the full command length, about 1024 characters. 
  The web address is part of this maximum, so account for that.

  Fixed a typo in the muf recycle prim that let M3 users recycle other
  people's belongings if compatible_muf is @tuned on.

  Added a muf_mpi_flag_mlevel @tune to set what wizard level can set mucker,
  meeper, and builder flags on players.

12/8/99: Made it so Guests see the old standard dbtop count when trying
  @stats.  Also made @force non-Guest due to easiness of using @force and
  MPI to take over a muck with poorly protected wiz-objects.  Guests can
  be prevented from using MPI in say/pose by leaving them unset Meeper.

  Fixed a typo that omitted the "quota" part of the quota propdir, causing
  user specified quotas to not function.

  Fixed the {time} and {ftime} MPI functions to obey their documentation.
  It is supposed to be possible to do {time} and {time:{tzoffset}} and get
  the same result according to the documentation.  The same holds true for
  {ftime}.  If a muck actually depends on the previous quirky behavior, (I
  can't imagine how), you can #define FB_TIMEZONE_QUIRK and it will compile
  the old way.  I highly recommend the fix, however.

  Fixed yet another FB crashing bug, this time in {filter}.  Some fixes will
  try to keep tacking on string data if it will fit in the buffer.  I set it
  up so it will stop after one piece doesn't fit, otherwise it's possible to
  have separators or data pile up in the last few bytes of the string.  If
  the data element won't fit, a trailing separator is omitted so there are
  only separators between elements (as it's supposed to be.)

  Updated man pages for flags2, call, and lib3 (removed).

12/4/99: Added profiling to the parsempi muf prim.

  Fixed a problem where purify_muf_files sticks a space on the end of each
  line each time a program is saved (growing one space per line per save).

  Plugged a hole where an M3 program could execute arbitrary MPI code with
  wizard permissions using parseprop (again).  This was fixed long ago,
  until I put the FB version of parseprop back in somewhere around 3.0.0.
  I wanted to maintain as much compatibility with FB as I could.  That was
  until I discovered just how easy it is to shut the muck down or do an
  @armageddon.  I'm still not sure how it's possible for M1 programs to
  run with wizard permissions.  This is supposedly still possible in stock
  Fuzzball, I don't know if Glow's permissions system prevents it or not.

11/21/99: Added transparent_paths @tune, defaulting to on.  Turn this @tune
  off if you don't want people to be able to look at a path and see what is
  in the room it is linked to.  A program will have to thus be written to
  show any transparency text in lieu of the intercepted @$trans that exists
  only when transparent_paths is enabled.  This @tune was added because it
  is possible to link a path to a room and then look at it remotely, causing
  a potential security issue to some mucks that consider this form of remote
  looking a privacy violation as one can potentially look at any room set
  Link_Ok using @path.

  Fixed can_move and do_move to use the correct mucker levels.  Long ago I
  misread how Foxen was treating exit levels, not realizing that one is
  added to each mucker level before being compared in a 'match'.  Thus an
  M0 (no bit) exit would pass as an M1 exit.  This may cause what looks
  like compatibility issues if you have compatible_priorities turned off.
  The server is actually acting like it was supposed to now.

  Added help_mlevel @tune to fix a problem with exit priorities that can be
  accessed via help.  The default is now to use level M2 instead of M1,
  because if compatible_priorities is enabled, exits without a mucker bit
  show up as level M1.  If you want the behavior of 3.1.4 and previous, set
  this @tune to 1 instead of 2.  This will also fix the apparent problem of
  being able to look at paths through help.  This is only actually possible
  if you have an exit set up to call $cmd/path, and only because of the bug
  that M0 exits could be 'seen' via help.

11/20/99: Added quiet_dark_exits @tune.  This allows exits that are set DARK
  to not print the 'has left/has arrived' messages.  It is off by default.

  Fixed a condition where a @pcreate could cause database corruption and
  eventually cause the server to spin it's wheels going in a loop.  The
  default 'player start' room has been set to room #0 instead of the
  previous hard-coded #2.


## 11/20/99: Release 3.1.4, Got engaged, minor feeps, annoying crash fixes ##

11/20/99: Added a 'SAFE_MPI' config #define to deal with the fact that the
  standard 26 level recursion of MPI parsing goes past the initial 1-meg
  stack allocated by Windows.  The level limit is reduced to 6 for Windows
  programs.  It is possible to change the initial reserved stack address
  range in Windows programs to something other than 1mb, but that doesn't
  get past the obvious issue of just how horridly inefficient that MPI
  parsing is, especially when done recursively.  I would recommend any
  server wishing to keep a small memory footprint to put an entry in local.h
  that sets MAX_MPI_LEVELS to as small as possible, I recommend 6, which can
  be done by #define SAFE_MPI, or you can set the level manually by
  explicitly defining MAX_MPI_LEVELS as a number.  It is possible if you use
  the Unix limit or ulimit command to set the stack to under 10mb, that you
  could hit the problem with excessively large recursion loops in MPI.

  Cleaned up various debug sprintf calls in msgparse.c.  I am honestly
  amazed that using 'debug' in MPI doesn't crash in Windows.  Many of the
  same problems I saw with a previous sprintf fix exist in msgparse.c.  It
  may be a Borland specific issue, right now I'm using MS VC 6.0.  I may
  do the next WinGlow release as MS instead of Borland, despite the increase
  in executable size.  The MPI parsing code is truly inefficient and needs
  a serious revision.  It's using at least 64k or more per recursion level.

  Set it up so W4 and #1 can change @/alias and @/glow properties on objects
  that they control.  This can only be done using inserver @set and not from
  a @force or alias.  No muf or MPI programs are allowed to mess with these
  directories as bad things can happen if garbage is put in them.

  Revised my safer muf program reader in db.c so it won't eat the hidden
  'escape' characters in muf files if purify_muf_files is off.  This was
  an unintended change due to a newline eater added to prevent crashing if
  you read in a file with lines having NULL characters in them.  Lines are
  still terminated at the first NULL, \r, or \n.  I also made it so truly
  blank lines are read in as a line having a single space.  Not reading
  these lines in may cause apparent errors in how the compiler reports
  line numbers.

  Found another spot in db.c that can poke str[-1] if a blank line is read
  in.  To make Glow/Fuzzball work with DOS linefeeds, it would appear there
  are many functions using fgets() and this horrid strlen(x) - 1 = '\0' to
  eat the ending newline.  They probably need to be converted to search the
  line for the first \r or \n, each and ever time fgets is called. Oye!

11/19/99: Changed check in do_linewrap and do_more from checking the guest
  flag do doing the proper Guest() macro check.

  Finally nailed a nasty very rare crashing bug in Glow!  This one was
  totally my stupidity.  I find it very weird that it still exists because
  a while back I ran through every source code file to search for anything
  that would access the db_top object (which is actually 1 past the last
  valid dbref).  After a @dump, the glowflags 'save' function would attempt
  to set properties on the db_top dbref, which is a Bad Thing (TM).  I
  believe what was happening was that 'db_top' invalid dbref was getting
  saved as the first dbref in the 'prop_changed' queue list.  When that
  dbref actually got created, it wasn't removed from the list but it's
  'next' and 'prev' dbrefs were set to #-1.  Because of the total lack of
  data validity checking in the diskprop code, it crept up only after a
  new object is created with that db_top value, and then a property is
  'set' on the new object.  Very very evil bug to track down!

11/18/99: Got engaged to my wonderful sweetheart of over 2 years.  Yay us. :)

10/11/99: Removed an excessive httpd GET log entry, effectively halving the
  amount of logging done for http accesses.

10/10/99: Added local.h configurable parameters in params.h for the say,
  pose, chat, and dash tokens.  define SAY_COMMAND, POSE_COMMAND,
  CHAT_COMMAND, CHATPOSE_COMMAND, and DASH_COMMAND to what you actually want
  the server to do when " ' : ; . , or - is used to prefix a command.  I
  also changed the "dash" verb to "minus" as default because some mucks use
  "dash" to mean run fast.  The CHATPOSE command will have a space put after
  whatever the command is, which defaults to 'chat :'.  Thus mufs will need
  to strip a space after the colon or rename it to something like chatpose
  or chatp and adjust mufs to use the command @ name.

10/8/99: Fixed an FB bug with {ltimestr} eating part of its string in
  Windows.  It really should have happened in Unix too.

  Added help for dforce and made it not do 'Forced.' if the descriptor is
  connected to a player, as the force is not done then.

  Made it so the more paging system and word wrap are disabled for muf
  and MPI cgi scripts, or more specifically, for any text sent out via
  an 'html' mode descriptor.  Also set it so the www_user will not receive
  the 'If this just spammed past you' message when doing info on large
  files.

9/22/99: Fixed log_interactive @tune.  It's meaning was reversed in the code.


## 9/21/99: Release 3.1.3, WWW muf and mpi cgi scripts, big crash fix ##

9/10/99: Added www_player_pages and restricted_www @tune parameters.  The
  first one allows a muck to prevent '~' from expanding to a player object. 
  The page will be read directly from the root object.  The second tune
  forces all players and mufs to have a WWW_OK flag to be accessed.  The
  root www object and MPI scripts don't not need a WWW_OK flag to function
  properly.

  Added WWW_OK flag for restricted_www @tune.  All players and mufs must be
  set with this flag if restricted_www is turned on.  @find can be used to
  search for WWW_OK using the '&' ampersand character.

9/9/99: The world didn't end.

9/8/99: Added a block so guests can't add or remove players from their
  ignore lists, but can see if players are ignoring them.

8/31/99: Put in a stop-gap measure for a *serious* bug that may or may not
  crash the muck server.  If it doesn't crash the server, it will take down
  the whole host machine!  Ugh!  This was tested and proven to happen on
  stock Fb5.64, it is not a Glow specific bug.  Every single Fuzzball server
  in existance most likely has this bug in it.  At least on Linux machines
  it will allow an M1 programmer to take down the whole host machine.  More
  research needs to be done to see if other ways interp() is called could
  loop infinitely and bog down the system.

  Set up the new CONNECT command to only run if there is a player already
  connected on the descriptor.  Added a NO_CONNECT_HOOK #ifndef so local.h
  can be modified to prevent the CONNECT WHO-like hook from being compiled
  in.

8/21/99: Fixed a crashing bug in read_program() in db.c.  Set up a filter to
  catch non-printable chars in muf program files.  Discovered this bug
  exists in several functions intended to strip the newline off of fgets.  I
  tried to clean them up to handle blank lines.  I think this may be why the
  server crashes without the 'read binary' mode in Windows 95.

  Added a connect_wait @tune parameter.  Can't believe this was hard coded.

  Added a dforce command to send commands to players on the welcome screen. 
  People with tinyfugue may get booted from a muck crash and try forever to
  reconnect.  If they don't have a player defined to log in, they will
  continually get booted.  There's no way for the muck to stop tf, so this
  command is here to force the connection to connect until the player comes
  back.  Not sure of a simple solution beyond a siteban to prevent a rogue
  reconnecting tf process.

  Fixed problems where frequent web traffic can trigger the DOS routine.
  Also made sure localhost can still get on even if it has passed the
  DOS threshold.

  Added a CONNECT command to reconnect to another player.  It works just
  like the connect command on the login screen, CONNECT player password.

  Added muf and MPI scripting for web pages.  All programs need to be set L
  to be called.  To call a muf program, @set the dbref number of the
  program, ie @set me=_/www/myprog:#1234 with the # sign.  To run MPI,
  the web storage object must be set M.  Start the property with an '&'
  and then insert MPI as usual, just like what is done in _connect/
  propdirs.  Any properties that don't start with # or & are assumed
  to be redirected addresses as usual.  The www_user parameter must
  be @tuned to a user set Guest that will receive the web page generations. 
  Do not log into this character unless you are wanting to be spammed to
  death!  Unlike the way Neon does web pages, content in Glow is done
  using normal notify primitives.  It is even possible to use the force
  prim to get output of in-server and muf commands like 'help'.  Don't set
  linewrap, more paging, or other special flags like more paging on this
  character.  Flags are set while the muf or MPI sequence runs so that
  ANSI colors are stripped.

8/20/99: Set the default @tune settings for idleboot and restricted_mpi to
  off instead of on.  No idea why these were enabled as defaults. Argh.

8/15/99: Changed default of the pause_after_motd @tune from on to off.  It's
  not something most mucks like, and gets confusing when starting up new
  mucks.

8/14/99: Put in a pattern match so Glow can read Neon V2 databases.  I am
  living on the possibly wrong assumption that Neon does nothing different to
  the format vs Foxen 5 format.  The DB still needs to have Unix linefeeds.

  Added a '-' dash token and support @tune, dash_tokens.  To use it, turn on
  dash_tokens and use dash as part of a common muf verb.

  Added a @tune to give paths a priority level so exits can override them.
  @setting and other commands will still take paths first, but moving and
  actually typing the verb will obey the new priority setting.  Set the
  path_mlevel @tune parameter to what level exits need to be to override
  the path, regardless of location.  No path matching is done now unless
  either no exit is found or the exit found is lower than the @tune level.

7/11/99: Added a feep such that the www port won't be openned if the
  -localhost or -wizonly command line options are used.  This will allow
  changing the www port without having to edit the database in case a site
  admin blocks a port or gives it to another user to use.


## 7/11/99: Release 3.1.2, Mostly bugfixes ##

7/11/99: Cleaned up a LOT of references that reference db_top in an unsafe
  way.  Added @toad back in and created a @soul alias for @frob.  The logs
  will still reference 'FROB' entries.  Fixed a CLEAR() bug in the muf
  locked? prim where an fb fix somehow got lost.  Added the fix into the
  other prims, flocked?, chlocked?, and conlocked?.  Cleaned up the help
  files.

5/20/99: Added an uncompress() around GETPECHO in interface.c, there is
  the potential for it to print out compressed garbage otherwise.

5/2/99: Added gender_commands @tune parameter that will allow admins to
  disable the in-server @gender/@sex, @species, and @position commands as
  well as prevent them from being listed via 'examine'.  Also added #defines
  of PROP_SEX, PROP_SPECIES, and PROP_POS that can be customized in local.h
  for wiz-only props (ie ~/sex instead of sex).  These defines are mainly
  to allow a prop other than 'sex' to affect pronoun substitution.

  Added a look_on_connect @tune parameter so admins can disable doing a look
  after spamming any muf connect programs, or cause a look to happen (via
  muf or MPI in the connect queue) at some other area withing the connect
  sequence.  The look queue is also not performed on connect if this @tune
  is disabled (default on).  The 'connect' action is still handled as-was
  whether or not this @tune parameter is disabled.

4/11/99: Added help for @flock, @conlock, and @chlock.


## 4/6/99: Release 3.1.1, Mostly bugfixes, a few security plugs ##

4/6/99: Fixed a typo that caused examining a mud mobile to list its stats as
  an item instead of a mob.

  Made setting the Viewable flag on programs only not allowed in a muf
  program.  Also changed @set so only Boys can @set the Viewable flag on a
  program unless it is done by the program owner without being influenced by
  an @force or @alias, similar to the Tinkerproof flag.  There should be no
  honest reason for a muf program to set the viewable flag on a program (but
  many a dishonest reason sadly).

  When I put the original FB parseprop prim back in, I didn't include the
  fix I did to parsempi that prevents the muf buffer from being corrupted if
  an MPI performs a muf call while parsing.  I copied the 'buf' declaration
  from parsempi to parseprop in order to cure the buffer corruption.  As far
  as I know this bug still exists in FB (and Neon?).

  Changed motd to add new entries with 2 spaces at the start of each line
  instead of just the first line.  Because of the wrapping (which apparently
  FB has adopted?) no fewer characters are lost per line, it wraps always at
  76 chars, the lines are just spaced out cleaner now.

3/28/99: Added _defs/ propdir to have the same special property protections
  that _msgmacs/ has.  Why do a half-job in virus protection?  Essentially
  muf and MPI programs not set Mage (W) cannot alter these propdirs.  This
  is to prevent a virus (or two) that spreads this way.  It's also decent
  common sense.  Players can still set these properties normally.  I also
  added some help in 'man' for property permissions: man property.

3/26/99: Added muf_proglog_mlevel @tune to set what level players are not
  logged if log_programs is enabled.  This will allow mucks to decide just
  how much program log spam they want to bloat their accounts with.

  Fixed up prim_recycle that will allow Non-Boy level programs to recycle
  objects owned by the program owner, excluding the program and the owner.

3/20/99: Fixed a quirk where mortals could use @teleport to enter other
  players set Jump_Ok.  Added a clause in @teleport such that only Mages
  can cause this silliness to happen (which was the original intent.)
  The prevention was most likely lost in the changeover from Mage() style
  checks in favor of controls() checks.


## 3/20/99: Release 3.1.0, Security with hostnames, pueblo fix, wordwrap! ##

3/07/99: Added the validate_hostname and validate_warning @tune parameters.
  When validate_hostname is enabled, entries in the @/sites directory can
  now match hostnames instead of just IP addresses.  The format is standard
  smatch format, ie *host.com.  Numerical ranges still need to be specified
  using the old method, this is purely for efficiency reasons.  (Every
  connection runs through the list roughly twice by the time a character
  is connected.)  If hostnames are @tuned off, validate_hostname will not
  function.  The validate_warning is a courtesy message displayed to users
  logging in while their hostnames are resolved.  Some mucks may prefer not
  to have such a message displayed.  All new connections will lag if their
  hostname isn't in the inserver cache until the hostname is resolved.  This
  lag can be from several seconds to several minutes if a person is logging
  in from a site with a very bad ISP or nameserver.  With the current setup,
  addresses that don't resolve to names can only be blocked with the
  numerical range matching.  A final recommendation: do NOT use the 'X' ban
  with hostnames, only numbers.  Unless the hostname is already in the cache,
  the X ban won't work.

  Examples: (for posterity)
  @set #0=@/sites/*.aol.com g:AOL users can't request or use guests
  @set #0=@/sites/128.0.0.0 x:Drop all connections from this 24 bit set

3/07/99: Added @hostcache to load, save, clear, and list information stored
  in the inserver hostname cache.  Information can be queried using
  wildcards for the hostname or number, providing interesting statistical data
  on who is connecting to the muck.  ie: @host list=*aol.com or just *.com.

3/07/99: Added pueblo_message @tune to set what is sent when Pueblo clients
  send the PUEBLOCLIENT command.  The original setting was wrong and caused
  all muck output to appear as HTML to Pueblo, which really messed up
  formatting and color output.  The new method recommends having a Pueblo
  MPI macro of some sort that wraps all pueblo/html content, ie:

  (Put all these on the same line, I cut it out for lesser clients)
  @set #0=_msgmacs/html:{if:{instr:{flags:{owner:me}},U},
  </xch_mudtext><img xch_mode=html>{:1}<xch_mudtext>,{:2}}

  Then using {html:text with pueblo,plain text version} in descriptions
  will show special things to people with Pueblo clients activated.

2/27/99: Finally a useful fix, @wrap will now do wordwrapping instead of
  just linewrapping, yay!  It's automatic, nothing to do different.

2/27/99: Added a full_paths() function to allow paths listed by examine to
  be listed with their full names if they contain multiple names, ie w;o
  will now list as west;out in examine.

2/27/99: Discovered that the resolver needs the SysCtl feature enabled in
  Linux, if it is disabled the resolver can't talk to the main server.  This
  is a feature you have to compile into the Linux kernel.  Other things like
  gpm don't work well without SysCtl enabled either.

2/27/99: Fixed a bug in paths where if you use a path with multiple names,
  the first name is not expanded, ie 'w;o' for 'west;out', when you move in
  the either west or out directions.  The first name will now be expanded.

2/15/99: Added @<address> command-line parameter.  Set this to an internet
  IP number if your ISP requires you to listen only on a specific address. 
  Don't specify it to listen on all addresses your TCP/IP stack can be
  accessed.  Setting this to a valid octet will prevent access from
  localhost (127.0.0.1) which can be a security risk as all other addresses
  can be blocked out.  For this reason the set address will act as localhost
  for security comparisons (localhost cannot be X-sitebanned.)


## 2/8/99: Release 3.0.9, Upgrade to FB5.64. ##

2/8/99: Upgraded to Fuzzball 5.64.

  Added * to match wizard WHO along with the usual ! token.


## 1/24/99: Release 3.0.8, Save flags as props, crash fixes. ##

1/23/99: Fixed @set object=level=0 to clear all mud props.

  Added @tune parameter quell_ignoring_mlevel to set what mucker/wiz level
  will break through @gags if the player is not quelled.  Made ignoring?
  return 2 instead of 0 if a player being ignored is a wizard passing the
  mlevel check.  Thus now wizards can be told if they are breaking a @gag.

  Added @glowflags command and save_glow_flags @tune parameter to save the
  special information stored in the database behind the flags value as
  properties on objects.  The @glowflags command is universal allowing for
  save/load of extra flags by the Man.  The save_glow_flags will store the
  current flags/statistics any time a full (non-delta) dump is performed.
  See help for the command and tune parameter for more info.

1/19/99: Made the unix_login stuff only happen for MUCK connections, not www.

1/12/99: Fixed a bug where the command @ variable in paths does not get
  set for @succ and @lock messages or in parsed MPI.

  Discovered that the 'X' site-block can affect localhost, fixed that.
  Localhost should always be able to connect to the muck.

1/10/99: Added ! lookup support for Mages to override local muf help.
  Removed the spit_file_segment_lines function and merged what it did
  with spit_file_segment to remove redundant code.

  Added gender lists in defaults.h for names that people can set in their
  'sex' property that will count as male, female, or neuter.  These lists
  can now be changed in local.h.

  Fixed it so mudders can only kill if they are at least kill level.

  Added the READBLANKLINE flag that only a program can set/clear that
  will send a space to the program if the user presses return (a blank
  line).  This flag must be set once for each such read.  A macro like
  'def prompt me @ "readblankline" set read pop' could be used.  read
  returns a single space if the user just hits return.  This can be
  useful in editors such that normal 'blank lines' don't get eaten.

  Put in a rather convoluted hack to make sure that no muf program can
  @set a fake player alias on a W4 wizard using either @set, MPI, or muf.
  This should make it safe to use @force in aliases, until someone proves
  me wrong.

  Fixed a bug in ansi_notify_exclude that wasn't colorizing text sent to
  some listeners.

1/9/99: Fixed a crashing bug in FB that will crash the muck if you call
  descr_setuser with only 2 items on the stack.

  Added new descr_setecho prim to turn telnet echo on/off if unix_login is
  @tuned on.  Also added the unix_login @tune which makes logins act like
  Unix/Dikumuds.

  Fixed a crasher bug in @proginfo if a large # is passed.


## 1/8/99: Release 3.0.7, new decaying dump copy-saving mechanism. ##

1/1/99: The first crashing bug of the year!  Fixed a crasher in 'say' and
  other in-server speech commands that parse MPI and crash due to a buffer
  overflow.

12/18/98: Made the @dump decay use its own dumpcount counter because delta
  dumps also update the epoch value.  This new dumpcount value will print
  out in parentheses after the dump filename for full dumps only.  This will
  allow you to figure out the depth of the dump for testing purposes.

  Added SUSPECT and LOGWALL to 'look' for W4 players (and only them.)

  Set it up so vehicles or other containers set QUELL won't notify their
  contents of what happens 'outside'.

  Removed the & after the glowmuck server run to decompress old databases
  before they are compressed with gzip.  This copy of the server opens the
  game/data/deltas-file along with the main server process, causing the file
  to be opened by both and then deleted.  This causes the deltas-file
  contents to essentially be written to nilsville until a full monolithic
  dump closes and reopens the deltas-file.  Updated both restart.sh and
  restart.csh.

12/17/98: Added dump_copies @tune parameter to allow full database saves to
  keep N copies of the new output database.  See the @tune help for info.
  By default this feature is disabled and the system acts normally.

  Added dump_copies_decay @tune parameter to produce exponential decay in
  saving the db copies when dump_copies is turned on.  See the @tune help
  for more info.

  Added a few more NAMEWIZ type entries to defaults.h to clean out hardcoded
  wizard names in look.c.

12/12/98: Added -localhost command option to allow logins only from
  localhost.  Also added a @localhost command to turn it off.  Acts like
  @restrict for the -wizonly option.

12/03/98: Fixed why_controls() where certain muf programs could crash a muck
  when an object is @recycled or @purged and triggered muf on moving
  objects.


## 11/10/98: Release 3.0.6, Upgrade to FB5.63, added USR2 @dump signal. ##

11/22/98: Changed default of parse_help_as_mpi to 'off' as this feature was
  removed in Fuzzball 5.63.  There were no effective changes to FB5.63
  beyond this.  To clean up what was also cleaned up in 5.63, I changed all
  references to fb5.30 and other old references to fb5.xx and Glowmuck 3.0.0
  references to 3.x.x.

11/18/98: Added @dump via kill signal USR2.


## 11/10/98: Release 3.0.5, Upgrade to FB5.62, minor touchups. ##

11/10/98: Bonehead mistake found after I released.  At the last moment
  I moved the www port @tune checking code too far out of the main loop,
  so it would always end up being '0' since tune-parms hadn't been loaded
  from the db yet.  Fixtut.

11/08/98: Removed the pueblo connect port and changed the WWW port to be
  set via the @tune www_port parameter.  I also added a @tune pueblo_support
  parameter to enable the small feeps Pueblo likes to see.

11/08/98: Remapped the Glow F2SUSPECT and F2IDLE flags to higher numbers to
  allow Neon DBs to be usable as-is in Glow.  The rest of the flag conflicts
  should be negligible because they were discarded flags with no value.

10/28/98: Fixed a crasher bug that only showed its face to date in WinGlow. 
  In the 'ignore' code when a player logs in, I call remove_ignoring() with
  a jerk dbref of -1.  The purpose of this is to strip all non-player
  objects (@toads) from the list to keep it trim.  Unfortunately I forgot to
  not get the OWNER of the jerk object if the jerk dbref is not valid.

10/19/98: Added in diffs for fuzzball 5.62.  Need to look over what was put
  in a bit closer.  The multi-port code wasn't particularly clean and I took
  the cheap way out (code duplication) to support web page/pueblo ports.  Will
  rework both shortly.


## 10/18/98: Release 3.0.4, In-server path commands. ##

10/18/98: It's been a month of alpha/beta testing of the new path code
  and noone has found any new significant holes, so the 'official' 3.0.4
  release will be the same as the last alpha source set, number 9.  This
  will make sure everyone has the right help files now.

9/13/98: Added @tune admin_prop_mlevel to set the wizbit level or muffer
  level that can read and write @props, or properties that start with the
  '@' character.  For previous releases this level was W2, I am setting
  the 'default' to W3 just to maintain a bit more security.  In muf
  compatible mode, any program set mage (W) will still see @props.

  Cleaned up the @path and @junk command to create shortened versions
  of each ; delimited path name, as well as cleaning out stupid things
  like blank names, ie @path ;;;in;;out;;;in;;; becomes i;o;i.  @junk
  will match full paths with these names, either i;o;i or in;out;in
  will recycle an i;o;i path.

  Expanded the @tunes for prop permissions to also adhere to the @set
  command, examine, and MPI.  The @tunes are kinda useless if you want
  hidden info but it's only hidden to muf.

  Added @tune log_with_names to log commands with both player names and
  dbrefs.  Normally only dbrefs are logged to make the logs more anonymous.

  Added a few more entries in the .debug/ propdir of a crashed program.
  The last player who ran it is logged, along with the last known command
  and argument, however that command and argument may not be valid if the
  program slept or was backgrounded.

  Changed parsempi to use the perms of ProgUID instead of program dbref #.
  As I guessed using the perms of the program object makes parsempi pretty
  darn useless.  Hopefully running as the effective user of the program will
  make it more useful.

  Fixed a quirk in the ported parseprop code where fuzzball is checking
  something that makes no sense with prop_write_perms().

9/10/98: Fixed permissions() so W3+ wizards get the same loopholes that
  are provided for lesser wizzes.

  Came up with a work-around to still allow paths with spaces.  Paths
  that will run muf programs with arguments cannot have spaces in them,
  as the argument won't match, but otherwise paths can have spaces.

  Altered the muf permissions() function to use why_controls(), which
  will get muf abilities in line with regular wizard permissions.  On
  enhancement is I set it up so muf programs owned by #1 and set W4 will
  run at the 'man' level, which is like W5, but I just call it level #1.

  Added a new muf 'perms' primitive which will help make more muf compatible
  with the new changes.  I recommend doing:

    def controls ( dp do -- i ) over over controls rot rot perms and

  This will make it such that a player (dp) can only use a muf program to do
  something if they physically control it and the muf program has
  permissions to modify the object (do).  If you only check 'perms' and not
  'controls', your program will essentially give any player a wizbit to your
  program.

  Fixed it so {force} of a player passing FLock should work unless the
  forcing action is a lower wiz-level than the player, thus mortals should
  find it always work if properly flocked.

  At this point we've theoretically broken enough in permissions code that
  the next release is going to end up being 3.1.0, if I ever get an alpha
  out. >:)

  In order to solve the never-ending dilema of parseprop/parsempi, I put the
  original parseprop back in as it's own prim, copied out of fb 5.61.  It
  should run much more like the original FB version with far fewer
  limitations.  I changed parsempi to run with the permissions of the
  calling program instead of the permissions of the object passed.  This
  will probably produce wild and weird results and will need to be worked
  out over time I am guessing.  Having in-server paths should take away
  one of the larger needs to fix this.

9/9/98: Added help for logwall.

  Changed @dig, @create, @program, @open, and @action to list the full
  object name, dbref, and flags when created.  This will hopefully help
  alleviate confusion to the new [RTFE]CREATE_FLAG[S2] feature(s).

  Fixed a typo for @tune db_help_first which had the flag meaning reversed.

  Fixed a few reason typos in why_controls(), namely players could appear
  to be under a mage's control because they are a guest when they are not.

  Fixed at least 1 hole in my new listen code, this is still a big hack
  in progress.  My goal is that a person and a player inside them can
  carry on a conversation, right now it would only work in poses.  I fixed
  a quirk where you could get Zombie> Outside> PakRat smiles. if you have
  your zombie in your inventory (doh).  This code is so horrendous. :)

8/30/98: Added {oansi} prim since the new ANSI notify functions actually
  make it possible now, whee!  Also fixed both {ansi} and {oansi} such
  that if you put color codes before your name that you won't be penalized
  by having an additional name prefixed.  Ie previously doing
  {tell:^red^Andy eeps!,pakrat} would produce '> Andy Andy eeps!'.  Now
  the addtional '> Andy' will not occur.

  Fixed one or more loopholes in tell, ansi, and otell such that if
  you exec MPI on a remote room you can create a spoofer that won't list
  your name as the spoofee.

  Added [RTEF]CREATE_FLAG[S2] to parmas.h so mucks who want rooms, things,
  programs, or exits to have specific flags on creation by default can
  do so.  The best way to override them is in local.h.

  Added 'X' to @find for the XFORCIBLE flag.

  Spent some time trying to clean up how speaking inside/outside of objects
  works.  Right now you can pose and have people inside see it, but not
  speak.  Not quite sure how to get around that beyond making the notify
  except actually iterate through the contents of the object.

8/29/98: Fixed parent loop sanity problems.  In normal FB there is no way
  to clean up an object that gets parented to itself, or gets stuck in a
  parent loop.  Unfortunately if it does happen, parent_loop_check in FB
  will cause the whole muck to hang.  I rewrote parent_loop_check to keep
  track of up to a fixed # of parent rooms (figure 30 is more than anyone
  could ever need legitimately, could increase this) and if any are
  contained in any other rooms, the check fails.  @sanfix then uses this
  new improved check to ensure that looped objects are sent to room #0.
  If in this release you get a looped object sanity error, run @sanfix
  and then @sanity to make sure it is gone.  Let me know of any cases
  where this doesn't get fixed.  That sanity code is a mess!

  After the sanity quirk was fixed, I fixed that lingering nuisance
  (again?) where setting a room's dropto to a thing and then dropping
  that thing in the room will lock up a muck.  Even Ctrl-C will hang
  because the muck is stuck in parent_loop_check().  Now even in cases
  of loops that somehow get into the db, parent_loop_check() should not
  get hung up or end up calling itself recursively forever.

8/3/98: Added @lock and @unlock path support.  I really do wonder why doing
  '@lock something=' gives a bad key error instead of unlocking the item.
  I fixed it so giving no lock will unlock something instead.

  Set it up so killing glowmuck with the USR1 signal will do a @restart
  instead of a @shutdown.

8/2/98: Started adding more in-server path support.  Examine now lists paths
  in a room and destinations.  Exits are also now listed with destinations. 
  Examine will also now give info about paths themselves.  @desc, @succ,
  @fail, @drop, @osucc, @ofail, @odrop and @name all now match paths if you
  control the room.  You must be in the room to tinker with them.  I
  modified the color scheme in examine and elsewhere for 'Exits:' and
  'Contents:' from blue to yellow, it was seeming too blurred having them
  all blue.  The color is now actually CINFO (which is yellow by default.)


## 8/1/98: Release 3.0.3, Can disable multiple wizlevels via @tune. ##

7/31/98: Fixed a printing of double ports in log status messages if the
  hostname resolver is disabled.  Also fixed that #1 can't @tune on/off
  multi wizlevels, had to take out the MAN_IS_SUPREME defines, it is just
  assumed at this point.  No way to easily support it reliably.

  Put a stop-gap in to prevent Win95's gethostbyaddr() from returning
  trash that can get stored on players.

7/28/98: Added log_interactive @tune to allow logging of text entered in the
  muf editor or in a muf program.  Normally text that isn't an actual
  command doesn't get logged.  Unless this text would have been logged if it
  weren't entered in the editor or program it won't be logged.  Previously
  all interactive text has been getting logged if command logging is turned
  on, but this can be very spammy in muf-heavy mucks.  It is now disabled by
  default.

  Made @tune say 'Permission denied.' if you try to set a @tune
  parameter you don't have permission to instead of listing
  'Unknown parameter.'

  Just for fun I made a @tune for the 'quiet_moves' #define in
  move.c.  Turning this on will disable all standard inserver 'xxx has
  left.' and 'xxx has arrived.' messages.  I would only recommend using this
  if your muck has in-server default messages for @succ or such (see default
  messsages) or else uses muf teleport and @paths (with proper messages set
  up) only.

  Fixed a very obscure ANSI bug courtesy of Cherry.  If you use
  parsempi with a \r or {nl} on the end, and ansi_notify the resulting
  string (with embedded newlines), you can end up with an odd blank line,
  as well as 'bleeding color' onto lines aftewards.  I never realized one
  could slip \r\n sequences into notify output, thus never provided for
  this possibility.  Since people set !C get no extra line, while people
  set C do, this is a violation of my attempt to make the actual text of
  output identical whether a player is set to see ANSI color or not.

  Added integrated support for @tune and parameter help.  Now when
  you use @tune to list a single parameter, help for that parameter will
  be displayed after the setting.  If more than one parameter is matched,
  only a total count is displayed.  Pattern matching will work so long
  as there is only one parameter listed.

  Added support for the default color schemes that can be defined
  in defaults.h.  Using ^SUCC^, ^FAIL^, ^INFO^, ^NOTE^, or ^MOVE^ will
  show the color of whatever it is #defined to in defaults.h.  These
  colors can be overridden in local.h.

  Added compatible_mpi @tune to make MPI behave more FB-like.
  This @tune like compatible_muf pays no attention to security holes,
  and as such is only recommended for converting large FB dbs.

7/27/98: Converted the MULTIWIZ #define to a @tune multi_wiz_levels.  This
  will allow databases to maintain their internal permissions structures
  without requiring specific server defines.  Otherwise running an
  incorrectly compiled server could seriously mess up a running db.  Because
  of the severity of what this @tune does, only #1 (not even Boys) can set
  this @tune.

7/23/98: Added ability to disable @alias support with @tune alias_support.
  This will only disable alias expansion, players can still set aliases
  on their persons.  Also added @tune global_aliases to allow setting of
  global aliases on room #0 to affect all players.  Be certain of any such
  aliases set as they will override any global actions existing on the muck.
  Global aliases are @tuned off by default for this reason.  They would be
  stored as #0=@/alias/verbname:macro sequence.

  Added 'ic' and 'ooc' commands to do set/unset player's IC flags.
  It will also accept an object and is the same as @set object = [!] IC.

7/19/98: Made WHO's 'IC' Yes green and No red to differentiate them.

  Added @tune compatible_muf to give Glow as close to fb compatible
  muf support as it can.  This will be updated as incompatibilities are
  discovered and need to be corrected.

  Added program errors to logwall spam.  It should help wizards
  track down problems faster/easier if things go awry, specially with all
  the different compatibilities for muf proggies out there.

  In fixing Caspian color so invert/flash works, bolding got messed
  up, fixtut where it would stick to the end of a line.  Fixtut.

  Added a #define called MULTIWIZ.  If you #undef MULTIWIZ in
  local.h, you will effectively have a #1 / W instead of a W1-W4 muck. 
  Wizards have the power of a W3, #1 is the only W4.  MAN_IS_SUPREME does
  affect single-wizlevel permissions by allowing other wizards to tinker
  with things #1 owns if it is not defined.  However 'Man only' commands are
  limited to #1.  If MAN_IS_SUPREME is defined there is some additional
  checking to make sure wizards can't @chown wizbit programs to/from #1 as
  WizBit programs owned by #1 run at the Boy/Man mlevel.  Otherwise regular
  WizBit programs can execute muf commands that are normally Man-only.
  (And there as much rejoicing and confusion.)  If you do discover things
  that wizards can or can't do, please let me know, I can't gaurantee I
  got every little quirk out.  M1-M3 permissions are unchanged.

  Discovered a bug in the Windows 95 version of the server where
  gethostbyaddr returns garbage, not sure what I can do about it.  May
  have to do with specific winsocks, and/or just localhost depending on
  how networks are set up?

  Fixed bug in @ungag where it was trying to malloc() 0 bytes if
  there are no ignores when removing one.  This doesn't appear to happen in
  Linux, it would seem it allows malloc()ing of 0 bytes, however Windows 95
  does not.

## 7/18/98: Release 3.0.2, Added @gag support, fixed _listen w/color. ##

7/18/98: Added support so @tuned dump messages can have Glow color strings
  in them for more flare.  Whee!

  Fixed a quirk in Caspian ANSI where Invert and Blink modes would
  get eaten for non-bolded foreground colors.

  Fixed several quirks with descr_setuser where because various
  things loaded on players aren't set until they connect, and they aren't
  done in announce_connect, that descr_setuser appears to 'lose' player
  settings.  Fixed by moving various loaded entities into announce_connect
  and announce_disconnect.

  Made @colorset only malloc() a table if you have settings. 
  Connecting while connected unless you save your settings will clear the
  table, this is a feature, not a bug. :)  Changes aren't committed until
  you do @colorset save to make sure you can still read the screen.

  Fixed a bug where the match_local function for MPI would reference
  the OWNER() of #0's location (-1), which I am pretty sure is a bad thing.
  You are at risk if you aren't in #0 but say {name:#0} works (inserver say.)

  Yay! Two more fuzzball crashing bugs tracked down and fixed.
  If one uses 'descr_setuser' or 'checkpassword' on a player object with
  a blank (NULL) password, down goes muck.  Lessee how long it takes for
  this one to be fixed in FB.  Impact should be minimal as only a wizard
  can set a player object's password blank.  But it's still pretty bad.

  Added @tune default_messages that will use the message on #0 for an
  object's @succ, @fail, etc. if one doesn't exist on an object.  The entry
  on #0 exists as ~/msg instead of _/msg.

  Added @tune chat_tokens to enable/disable . and , turning into
  'chat' hotkeys.

  Woo woo!  I undeafened color!  I think I got it and cleaned out a
  pile of _listen related problems along the way.  When I incorporated color
  into the server I didn't have a total grasp of how listening works. 
  Looking through the pile of notify() functions (there are a lot) I found
  many inconsistencies in how I implemented the ansi_notify() versions.  I
  cleaned up many functions.  In doing so a lot of text that _listen
  programs wouldn't previously hear will show up now, in particular muf
  ansi_notify's.  (And there was MUCH rejoicing. :) This has been broken
  since I first put in ANSI color, and as such would be broken in every Neon
  server out there.  The main reason for cleaning all this out was to fix a
  problem with parsed ANSI color leaking into _listen source strings (bad
  bad bad) and could be seen on the muf debug stack, which was very wrong.

  Since the noted change below to parsempi will allow #1 to use
  parsempi without a W5 (which can't exist) program, I #defined
  MAN_IS_SUPREME as default.  The easiest way to tell if #1 is above W4s is
  to examine a W4 object.  If it is listed as MAN then #1 is the same mucker
  level as W4s, if it lists as BOY, then #1 is one level higher.

  Changed parsempi/parseprop so it will allow source objects of
  players or programs since they can't parse wizperm-MPI without checking
  for a greater mlevel.  This will alleviate some problems in the ongoing
  MPI security hole fix fixing.  I am still trying to figure out a safe way
  to allow older programs to still work without W4 wizbits.  'say' programs
  that allow players to parse MPI can at least work without a wizbit now.

7/17/98: Added muf $defines for the various entities listed under Glow's
  @version command for various #defined server options.

7/16/98: In a night of inspiration, @gag support!  It looks usable but needs
  some significant testing due to all the pointer operations.  It works both
  for in-server say/pose as well as all muf notifies.  Also think I fixed
  listening for ansi_notify.  Added ignoring? muf prim and @tune to
  enable/disable support and set max # of players one player can gag.

  Added code to the MPI match_local function to go up the environment
  tree to include everything up to and including the first room a player or
  other source object exists in.  This should help vehicle code that wants
  the name of the room it is in without letting players get names of every
  item in the database.  In theory 'local' means everything in the current
  room, but if the room isn't a room, but a container in a room, really
  everything in the room at least going upwards should be included.

7/13/98: Added #define MAN_IS_SUPREME (to be put in local.h) if a given muck
  wants #1 to not be affected by Boys.  This will cause #1 to crash any
  program trying to make him use parsempi or parseprop.  Caveat emptor.

6/29/98: It's my birthday!  I'm 24, hear my bones creak!  :)  Nuther year
  for lower auto insurance, woo.

6/28/98: Added hooks for @gag/@ignore and @ungag/@unignore.  Right now they
  point to dummy functions that don't do anything of note (beyond telling me
  they are the right function so I know the commands are in right.) This
  will be an ongoing project with eventual muf/MPI hooks.

  Added @permissions alias to the permissions-checking @check
  command.  This will allow mucks with muf @check commands used to verify
  room correctness to still have access to this command.  Updated help for
  it and added abbreviations for things like @description, @succces, etc.

  Fixed the socket non-blocking function for Windows 95/NT, it should
  work with NT4 and WinSock 2.0 now.


## 6/23/98: Release 3.0.1, mostly bugfixes, typo corrections. ##

6/22/98: Added muck name listed in 'maintenance mode' message.  Updated man
  help for 'set' to list most new flags.

6/21/98: Fixed a bug where @set something=!1 doesn't clear the mucker bit. 
  Also made @alias unavailable to guests. (Oops.)

  Ported the Win95 release from Microsoft Visual C 4.0 to Borland's
  Visual Builder 3 Standard.  The main reason for that is to use newer
  libraries for winsock that should allow GlowMuck to run under Windows NT.
  Right now my NT isn't booting so I can't test that it works, however.

  Upgraded to FB 5.61. *smirk* Well actually all I did was bump the
  revision # as I already fixed the only bug corrected in this release about
  a month and a half ago, a problem with MPI's {links}.  Nothing else at all
  in FB 5.61.

  Added an @tune for the shutdown_mesg and restart_mesg.  @armegeddon
  will still rat on whoever did it, however.

* NOTE * Permissions change: At this point upgrading servers may require
  some programs to be set W4 to allow W4s to use them (as well as making the
  muf storage character W4.) This is to prevent several severe security
  holes discovered since the last release that have been part of both Glow
  and Neon mucks (and FB in some cases) for some time.

6/17/98: Fixed a typo in set.c that was setting everything IN_CHARACTER.

  In a word, crap.  This parsempi bug has effectively broken our security
  setup.  To fix it, #1 will have the same permissions as W4s, since there
  is no way to set a muf program W5.  'muf' wizards will need to set
  programs W4 to affect people set W4.  Working on making sure #1 can't be
  overtaken by W4s, but W4s will have same permissions as #1, just can't set
  other W4s or get #1's password, or force #1.  Fixed up forcing of boys but
  not #1.  Because boys will have the same mlevel as #1 now, they will show
  up in 'look' as MAN and not BOY anymore.

  Fixed parsempi and thus parseprop to check that the effective program user
  controls the entity to run as the 'this' item in the function call. 
  Previously an M3 program could run MPI with the permissions of any object
  of its level or higher.  MPI doesn't allow for a way to specify a level to
  run at unfortunately.

  Fixed security hole where a W3 wiz could set an 'alias' property on
  a higher wiz and acquire wizbits or cause the other wiz to perform other
  odd actions.  Added 'alias_level' to check similar to 'force_level'.

6/13/98: Fixed an offset error that kept @getpw from being usable in game.c.

6/4/98: Created #defines in defaults.h to allow for customization of the
  default color scheme of GREEN/YELLOW/RED/WHITE/CYAN for success, info,
  failings, important notes, and movement, respectively.  The best way to
  modify these colors is to change the CSUCC, CINFO, CFAIL, CNOTE, and CMOVE
  #defines by overriding them in local.h.

5/31/98: Added starting hooks for roleplaying support.  Added an IC flag and
  made @tune rp_who_lists_ic to list players set IC.  The 'I' has been
  reassigned from IDLE to IN_CHARACTER in @find and unparse_flags.  IDLE can
  now be found using @find =|, etc.  Also made @find et all print '1 object
  found.' correctly.

  Fixed several permission anomalies:  @frob requires the person doing
  the frobbing to control the recipient in order to transfer frobbed items. 
  @kill all is now 'Man only' ala @uncompile.  @restrict is now Arch only,
  ala sanity and @sanfix.  Cleaned up help/man files a lot based on Athena's
  comments.

  Added 'glowver' muf prim to return GlowMuck part of the muck's
  'version' string.


## 5/30/98: Initial 3.0.0 release starts from Glow 1.9.6 ##

5/30/98: I decided to push the ersion up from all the beta 1.9.6 releases
  because they just became so many and the spiffy features grew so much. 
  See the 1.9.6 changes list for all the things that start here in base
  3.0.0.

