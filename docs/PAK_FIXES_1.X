
GlowMuck source code is Copyright 1996 by Andrew Nelson, all rights reserved.

Fuzzball and TinyMuck code has additional copyrights and disclaimers to be
found in the standard fuzzball tinymuck source code distribution which must
be adhered to when using this server distribution.

			* * * * * * * * * * * * *
			* Shameless Hacks List  *
			* * * * * * * * * * * * *

   *                                                                       *
 *    5/98 NOTE: I changed this list so versions now start from the newest   *
*     at thetop and work downwards so you don't have to scroll through the    *
*     whole thing to get to the new stuff.  (And once again there was much    *
 *    rejoicing. :)                                                          *
   *                                                                       *


## 1.9.6 Current Beta release changes ##

  4/16/98: Renamed the 'old_priorities' @tune back to
  'compatible_priorities'.  No clue why or how that got renamed.  Best to
  check this setting as it would have been lost originally from an fb db and
  again now as it gets renamed back.  Also discovered the ABATE exit support
  is missing in Glow, apparently it is yet another casualty of patch
  upgrading.  I added it back in based on how it looks in 5.58.  Essentially
  an exit set ABATE operates at a level lower than local exits with no
  priority level.  Priority levels are set by the mucker (not MPI) bit level
  set on an exit.  Also in fixing this I believe I fixed the 'compatible
  priorities' support that wasn't actually working in Glow correctly.

  4/28/98: Woo hoo! I finally got the http quirk working correctly.  In
  doing so it looks like we may not need the HTTPDELAY trick to make
  redirection work anymore, yay!  I have #undeffed HTTPDELAY in config.h
  which should be fine.  Let me know if you have trouble with redirections
  failing on browsers.  I tested it with Netscape and Lynx.  The quirk was
  making web pages get cut off often very early on if the page is transfered
  over long or slow connections.  Everything is looking hunky dory now.

  4/28/98: Added better cleanup code to parse http data file requests.  When
  people are mean and do 'http://...:9998/my////file/something/////' it will
  turn into 'http://...:9998/my/file/something'.

  4/30/98: Upgraded to fb5.60.  Looks like they fixed @tops a bit more.

  4/30/98: Tried to get the code to compile clean on Alpha Linux.  I'm
  suspect of a bad set of include files, right now the only error is in the
  'accept' function.  If anyone with an Alpha Linux finds we do compile
  clean let me know.  The code should compile a lot cleaner on 64 bit
  machines overall now.  I tidied several pointer conversions so they will
  hopefully compile cleaner.

  5/1/98: Fixed a bug from 5.60's bugfix of {links}, ref2str had obj instead
  of obj2, listing the source object for each destination instead of the
  destination.

  5/1/98: Added muf flocked?, conlocked?, and chlocked? primitives. 
  flocked? and chlocked? require the object to be set X or C respectively.

  5/1/98: Added per-player quota support!  (And there was much rejoicing.)
  Use @setquota to set per-person quota values.

  5/1/98: Html silliness: Added it so you can lsedit www_root=_/www/http/404
  and set up your own personal lsedit list for http error requests.  I also
  made the default error printout include some form of the requested title,
  but it's not too perfect.  The lsedit error message doesn't have the
  ability to include the missed page.

  5/1/98: Since standard FB allows mpi {flags} to be local (as opposed to
  Mage/wizard only), I changed glow to also allow local flag matching. 
  Tough toodles if someone wants to hide a secretly given M3 or wizbit.

  5/2/98: Added support so everyone can keep their local glowmuck's #define
  changes in a 'local.h' file that they can keep between changes.  Hopefully
  this will allow easier transitions between version upgrades when @tune
  changes just don't cover what a given site needs to update.  If you have
  things that you wish you could #define that won't work in local.h, let me
  know and I will try to add support.  (Example being I'm debating how to
  make it so you can #define a server color scheme for things like fail
  messages, etc.)

  5/2/98: Added @tune settings glow_ansi and tilde_ansi so you can disable
  either glow or lib-ansi (tilde) color in muf/MPI programs.  Since
  in-server colorations are in glow format, special care was taken to
  preserve them even if glow_ansi is off.  Also added a @tune server_ansi to
  turn off ALL server command colorations.  Whoa, it is weird, but it's just
  for you Maddax! :)

  5/2/98: Added @tune html_parent_link to allow mucks to set what page that
  references to screen such as @welcome, @credits, and @who go back to.

  5/2/98: Added 'ansi_offset', 'ansi_strcut', 'ansi_strlen', 'ansi_strip',
  'ansi_notify_except', and 'ansi_notify_exclude' muf prims.  This greatly
  increases the accuracy of the .wrapnotify macro.

  5/3/98: Added setting of @/id from inserver @pcreate.  This was a long
  time in coming.  *doh*

  5/3/98: Added sanity checking to the player_start @tune parameter.  Up
  until now its value is assumed everywhere in the server to be valid.  On
  the standard 2 object DB, creating a player will cause that player to be
  it's own home and location, containing itself.  I set the precedence order
  to be the start room tp_player_start, then the global environment room
  GLOBAL_ENVIRONMENT, and then room 0.  The assumption is that room 0 must
  always exist in the database as a room.  I also added recycle protection
  that did not previously exist for room 0 if GLOBAL_ENVIRONMENT is defined
  to a different room.

  5/3/98: Added #ifdef HTTPD wrappers around a few HTTPDELAY and PUEBLO
  clauses that got out of range of HTTPD sections (causing compiling
  problems when HTTPD is undefined.)

  5/3/98: Added @bootme inserver.  Also added a max_connects_per_player
  @tune parameter to allow mucks to keep players from being on more than
  once, or just helping the chronically stuck.  Setting mcpp to 1 will cause
  any old connections of a player to be booted when a new one happens.  The
  bootoff is a little messy as the idle player will still get the connecting
  spam of the new player, no easy way to prevent that.

  5/3/98: Added separate read and write access levels for @tune.  In doing
  so many levels were altered, usually lower, to give wizards and mages more
  power over the muck.  Many more properties are readable by all muf levels
  now to allow better support for 'glow aware' programs that often replace
  server commands that rely on the @tune information.  Parameters that
  wizards can change are marked with an asterisk before the name.

  5/3/98: Added some denial of service attack prevention of people who try
  to flood the muck with open connections or try to cause excessive repeated
  connections.  @tune max_site_welcomes and max_site_lma along with
  max_player_logins can prevent a combination of attacks.  Set
  max_player_logins so player characters cannot be connected more than X
  times at once online.  Set max_site_welcomes to prevent more than X
  unconnected players from a given 256 site subdomain.  Set this higher if
  you have active web page activity.  If either max_player_logins or
  max_site_welcomes is exceeded, the most idle connection is dropped.  Set
  max_site_lma to disallow more than X accepts per site in a minute.  Set
  this low unless you have frequent web page activity from a given site.  20
  should be safe even for heavy mucks.  If more than 75% of the max_site_lma
  is reached, the counter is reset to hold over a longer time.  Once it is
  passed the site is X blocked and new connections are dropped instantly. 
  This is designed to stop connection flooding, which is actually easy to do
  with a fast T1 connection and tinyfugue set to connect retries.

  5/4/98: Fixed an oversight in count_stats that was making the muf stats
  prim return garbage if you passed #-1 to get the total muck stats.

  5/15/98: Added 'convtime' muf prim ala MPI convtime.

  5/15/98: Added @gender alias for @sex in game.c.

  5/15/98: Added an extra variable in p_misc() to better correct 64 bit
  time problems with localtime() et all.

  5/28/98: Cleaned up tilde ~&~& -> ~& substitution, removed from tct() as
  in-server functions don't parse ~& sequences.

  5/28/98: Fixed AMBIGUOUS/NOTHING match-crash bug in look brought about by
  trying to fix 64 bit pointer comparisons.  In most systems these pointer
  values are unsigned, thus the comparisons should be done explicitly.

  5/28/98: Added a configure #ifdef clause from wiz.c into topwords.c to
  allow clean compiles under BSD using malloc.h/stdlib.h.

  5/30/98: Discovered that for the periodic program purge, program instances
  were being ignored as I never removed my fix for when they were broken
  before Foxen fixed it so the variable has meaning again.  I put that back
  in and added something so if a program is set 'IDLE' (STAY_IN_MEMORY),
  that the program will never be removed from memory (unless you unset the
  flag and wait for the next periodic program purge, or do @uncompile.)
  Also added help for @proginfo and @uncompile.

  5/30/98: Changed muf 'setname' prim to accept either the correct player
  password or the word 'yes', ie: #1234 "joesmith eatatjoes" setname since
  fuzzball uses the player password.  I had put in the 'yes' so mage
  programs could rename players without needing to know the password.

  5/30/98: Added a @tune db_help_first to make it so exits like 'page' with
  help can override outdated help in the help.txt file.


## 4/5/98:  Release 1.9.5 (mostly bugfixes, added @when, color)

  4/5/98: Fixed the WHO end totals to be more accurate.  Players connected
  will now only reflect actual players connected (and not people on the
  login screen) for both full WHOs and WHO playername commands.  The idle
  count will also only count connected players, and will give the full count
  of all idlers even for partial WHOs.  The max players count will now be an
  accurate account of actual players connected and won't include people who
  hop on the login screen during peak activity to bump up the count.

  4/5/98: Added @tune purify_muf_files to remove evil control characters.
  *** RED FLAG ***  This is ON by default so mucks who use things like
  embedded ANSI libraries should start up their muck, @tune the flag off,
  then @restart the muck without actually editting any muf programs first.
  Muf programs will get loaded in with the control characters stripped but
  only get saved if you actually use @prog or @edit and quit the editor.

  4/5/98: Added a 'color' command that aliases to @set me=C/!C for
  yes/no/on/off.  I put this in since guests see one can set color but
  without the colors.muf program there is no color command.  It is a no
  frills alias. :)

  4/5/98: Added @when command.  Actually @when is a modified version of
  examine.  I added a 'when' flag to do_examine to only print the info that
  @when wants.  It seemed the smarter thing to do as opposed to duplicating
  examine's code just for @when.

  4/5/98 Added help for @when, @quota, dinfo, dboot, and dwall.

  4/5/98: Added #defines for names of the Man and other entities in
  defaults.h.  I didn't feel this was worth making @tune additions for, but
  it's there for those who like more customization of who runs their muck. 
  The affect is not global, just a few of the obvious text messages like
  'You can't force the Man.'

  4/5/98: Fixed some anomolies with gcc 2.8.1.

  4/5/98: Added support for @setcontents and _/co replacing the contents
  listing of things if it is set.  Even added help for it, woo!


## 3/31/98:  Release 1.9.4 (more bugfixes, added __glowver muf $define)

  2/22/98: Changed the macro whodunnit NAME() parse to use unparse_object().
  If someone did an 's' in the muf editor and a given author's dbref in the
  muf/macros file was out of range, segfault.

  3/12/98: Added mortals_need_id @tune prop for new dbs that don't want to
  require @/id props on their players.  For fuzzball compatibility I
  defaulted it to 'off', ergo people who want it should do
  @tune mortals_need_id_prop=yes.

  3/31/98: Score another for Syvel, fixed prim_parsempi() to allocate a
  local temporary 'buf' instead of using the muf global 'buf' as it has the
  possibility of running muf within do_parse_mesg() which tends to cause
  the outer parsempi's result to get screwed up bigtime.

  3/31/98: Fixed WHO title spacing. *blush*  Discovered when I went
  space->tab cleaning from the last few fuzzball upgrades that a few tab
  characters appeared in the WHO's formatting for Player Name, messing
  almost all of them up by several spaces.  Fixtut.

  3/31/98: Added "__glowver" internal muf $def.  This will allow muf
  programs to use $ifdef __glowver (glowcode) $else (fuzzcode) $endif.


## 2/20/98:  Release 1.9.3 (mostly bugfixes)

  2/10/98: Corrected a typo/oversite in Glow where only programs set Mage
  and not above can run with unlimited preempt time.  Had == instead of >=.
  Also #undeffed WIZZED_DELAY as that's how it is in fb 5.58.  Not sure why
  it was set the other way previously.

  2/10/98: Changed it so a muf program can 'set' flags on itself however it
  wants, no matter who is running the program.  This will allow macros like
  .debug-line to work no matter who is using the program.  Be careful if you
  design programs around this feature as more than one person could be using
  the program at a time.  If so you might want to use 'preempt'.

  2/10/98: Discovered half of the '@WHO ' muf WHO trick was missing, not
  quite sure why.  Half of it was there so I put the rest back from 5.58.
  I also changed several can_move instances from hard coded values to LM#s.
  Twiddled with WHO so doing WHO person won't list the idle line if it's on.

  2/18/98: Fixed an apparently long standing (sorta) quirk where W3+ wizards
  couldn't use W4 or #1 owned stuff when the higher wiz is set GUEST.

  2/20/98: Changed ParseMPI muf prim from Mage to M3.  Added docs in man page.


## 2/10/98:  Release 1.9.2 (added idle/unidle support)

  12/14/97: Added an event setup similar to connect/disconnect for
  idle/unidle.  A @tune parameter decides when someone is idle.  There
  are additional @tunes for idle propqueues and an idle title for WHO.
  Idlers can be separated to show only active players in WHO and with
  other programs using the 'conidler' prim.

  1/1/98: First fix of the year!  Moved BUILDER in @set before BOY, oops.

  1/14/98: Discovery!  I don't think this is how I intended it to work
  originally but this is how it does work, and I think I'll leave it as is. 
  If a person is set DEBUG_SPAM they will only get debug spam for programs
  they run that are set DEBUG, regardless of permissions (basically
  overrides the controls() check, so be careful.)  I didn't see this
  documented anywhere else, so I thought it best to note it.

  2/9/98: Ugly bug fixed.  Feaelin discovered typing 'pop' in the muf
  debugger (set a program Z and run it) when the stack is empty causes
  a segmentation fault.  This bug most likely exists in every fuzzball
  created since the fuzzball debugger existed, no idea when that was.

  2/9/98: Updated our code to line up with Fuzzball 5.58.  There are several
  important fixes in this release as well as some profiling support for muf
  programs, kinda cool.  It also fixes a bug in identd I've been wanting to
  fix for a looong time, yay!

  2/10/98: Changed the 'sucker' flag? muf check to happen after 'sticky', it
  was causing all wizards to appear 'silent' if you did "S" flag?  My bad.

  2/10/98: Fixed a crashing bug with the new fb5.57 profiling code, it
  wasn't checking if there was nothing compiled before doing profiling.


## 12/13/97:  Release 1.9.1 (minor fixes, corrections, just for Maddax)

  12/13/97: Added some mod %'s to WHO so large commands don't warp the
  printout.  Added full @listen/~listen/~olisten/_listen/_olisten checks
  that hopefully work correctly now.  _listen/_olisten won't parse MPI
  but ~listen/~olisten/@listen will.  I also added a @tune parameter
  that will allow MPI to be parsed from mortal _listen/_olisten props.
  Muf programs still have to be above listen_mlev.


## 11/29/97:  Release 1.9.0 (changed meaning of Mucker/Wizard bits)

  11/9/97:  ** SIGNIFICANT CHANGE **   The level system has been changed to
  a system more compatible with the FB flag set.  This will require anyone
  using an old Glow or Neon db to use the @fixwizbits command to convert the
  DB back to this flag format.  The reasoning behind it is to allow Glow dbs
  to be more easily converted back to fuzzball DBs or run on fuzzball
  systems.  There may be adverse effects bringing the dbs back to a Glow
  server due to lost @tune settings as well as the second flag set and any
  mud stats stored on players or things.  But this change will at least
  allow Glow dbs to run 'as is' on fuzzball servers if such a need arises. 
  In updating the @fixwizbits I found a bug where I didn't end the command
  if #1 wasn't set to the proper mucker level, so it ran through and did the
  update even if #1 was the wrong level.  This is bad as most admins would
  then set to the proper level and run the command again.  @fixwizbits is a
  1 time only command with @armageddon the only recourse!

  The major sacrifice to this change is that the MEEPER or MPI flag has been
  moved to the second flag set and is now separate from the concept of
  mucker levels.  A @tune setting now exists to allow for 'free MPI' similar
  to the 'free building' concept.  The @fixw command will set MPI bits on
  players and things that were of the MPI level.  This may cause some
  confusion as you can now have players without MPI bits who do have muf or
  wizbits.

  Also of note is that the BOY flag is now a W4 flag instead of Y.  This
  will allow for a better clue of what power they have.

  A note on loading fuzzball DBs:  Mucker flags set on fuzzball Wizbit items
  will affect the true Glow wizard level of the item.  A Fuzzball program
  set WM3 will become W4 in Glow.  See 'help newlevels' for the list of old
  to new level conversions.

  We are ever reaching that point of bumping a major revision level, but I
  want a bit more stability testing before moving to either 2.x or 3.x.

  11/9/97: Added an alias similar to " for say as . for 'chat' and , for
  'chat :'. I also added inserver aliases of 'chat' as say and pose in case
  :.  a muck doesn't have a muf chat program installed.  I might get
  inspired and make a public chat in-server some day, but I think that would
  be evil. :)

  11/23/97: Oops, fixed a longstanding typo/logic bug with zombies and
  vehicles that just wouldn't do what they should for non-mages.  I had long
  ago removed 'dark' checks as I don't overly approve of dark objects. 
  Unfortunately I commented them out wrong and turned the check from 'if
  dark or mage' to just 'if mage', when it should jave just been 'if any'
  for that part of the check.




	#### End of 1.8 versions, moved to 1.9 ####


## 11/07/97:  Release 1.8.3 (mostly bugfixes, some minor enhancements)

  10/14/97: Fixed the disconnect propqueue so it runs even if it is only a
  partial disconnection.  Actual 'disconnect' actions and puppets falling
  asleep only occur if all instances of a player have disconnected.

  10/16/97: Added support so a 'more' return prompt is displayed after the
  motd and in-server messages, but before muf programs are run.  This will
  allow players to catch things on the motd that normally scroll past.  For
  the easily annoyed they can disable this with 'more pause' to toggle the
  pausing at connection.  For whole mucks who hate the concept, a @tune
  parameter pause_after_motd exists to disable the pause completely.  Even
  though the output is paused, the muf programs and room look still occurs
  and gets buffered just like normal more paging.  So wizards can see if
  someone hasn't pressed return yet when connecting I added a 'Output'
  entry in 'dinfo' that will show the # of bytes waiting from a more
  prompt.  If this number gets greater than max_output, the output is
  flushed and lost automatically.

  Hrm!  Not sure if it is a bug or a feature, but wizards from sites that
  can't request characters can't request any chars online as a wiz.  I set
  it so even if a site is blocked from sending requests, a wiz char from
  that site can still request chars.  I'm under the assumption you have a
  reason for wizards connecting from a banned site, namely the different
  levels of bans to prevent other people from such sites from being a jerk.

  10/26/97: Added a @tune parameter to include the author's name in new
  additions to motd entries.

  11/3/97: BLAH!  Legacy got hit by that annoying guest-renaming bug that
  is very evident in the standard Foxen multi-guest program.  Our guest
  character disappeared and we had to @name #17=Guest yes to fix it.  To
  make this more automatic I added code in player.c to rebuild the
  hashtable if a player is added and cannot be found or if someone
  connects to a player that cannot be found with lookup_player() but the
  player still exists in the database.  Either the code I put in will
  fix the hashtable and nothing but a status-log entry will be made, or
  else a coredump will occur and maybe we can narrow this problem down.

  11/3/97: Add a @tune tinkerproof_mlevel to allow mucks to restrict who
  can set tinkerproof flags on themselves and their possessions.  This
  can allow mucks to make sure newbies figure out how to use the muck before
  getting MPI or muf bits, distinguishing them.  For mucks who totally hate
  tinkerproof, just set the level higher than the max level in help
  newlevels.  Boys and above can always set tinkerproof flags.

  11/3/97: Changed the motd pause to tell how to disable it.  Also made
  disabling the pause to clear the more prompt if it is active.  Typing
  'more' will also now page ahead a page and print nothing if there was
  output waiting.  This allows people with screwy linefeeds to still use
  more.


## 10/13/97:  Release 1.8.2 (mostly bugfixes)

  10/13/97: Added @tune muf_macro_mlevel to set the mucker level a player
  can define macros in the editor.  Note that this value is the 0-9 or so
  mlevel and not the M1/M2/M3 mucker bit level.

  Added @tune path_dir to set the propdir used by $cmd/path, normally
  '@u/d'.  This propdir must match what you set $cmd/path with using
  $lib/userprop or you won't be able to look or walk in any direction for
  paths unless you have the exits actually created and linked to $cmd/path.

  Made it so the Man can newpassword boys.  Added unparsing to the
  newpassworder and victim.  Updated several log_status entries to use
  unparse_object for at least one of the arguments (can't use both as a
  static string is returned, need to add temporary buffers.)

  Made BOY listed in examine and added to unparse() as 'Y'.

  Added a list of #define options to the @version command to allow a way
  for admins to tell what their server is and isn't set up to do.

  Fixed it so W2 muf programs can read/write '@' props, should have before.

  Changed @tune to use a 24 string space for names instead of 20.


## 9/14/97:  Release 1.8.1 (Windows 95 port and misc fixes & additions)

Side Note:  I would like to get our versioning past 2.0 at this point to
  avoid further confusion with NeonMuck.  The latest changes to the wizard
  levels call for a larger step in version to indicate the major breakoff from
  current philosophy.  All wizard levels from W to W3 have been weakened and
  then now strengthend again, possibly requiring admins to rethink their
  layout of wizards and the bits they have.  I hate making a release that
  could cause such turmoil as I know what that silly little # change can do to
  pride and moral despite inequal changes to the power that # holds. *sigh*

  7/27/97: Hmm, I fixed a pointer bug in the unparse_ansi() function that
  looked like it could jump past the \0 if you have ^ as the very last
  character of a source text string.  I also put a touchup on parse_ansi()
  so if a trailing ^ isn't found for an ending color, the color is ignored.

  8/6/97: Stupid clueless award goes to me!  Fixed it so @getpw won't let
  Boys get the Man's password, or each others.  *blush*

  8/14/97: Added a new @tune doing_blocks_ads intended to prevent people
  from putting advertisements for other mucks in your @doings.  The only
  hard to catch ad is for foreign nations, org net gov and com are checked
  for as well as numeric addresses.  Since most sites are .coms or numeric
  these days, this should work.  It checks for num.num.num.num num or else
  .xxx num (where xxx is org net gov of com)  The '.' is any non-number
  character, so if someone does @doing 1 2 3 4 5 that would be blocked too.
  This should be rare, but this very relaxed checking prevents people from
  trying to get past more specific checks by adding spaces or changing the
  periods to commas, etc.  It should stump most advertisers I hope. :)

  As for the rational behind it, to me @doing advertising is tasteless and
  detracts from the flow of other @poll responses.  Yes, I do advertise in
  @doings on other mucks, but that doesn't mean I think it's tasteful. ;)
  I DO maintain a list of other mucks  I log into that I may or may not
  totally support, but for people who don't feel my muck is right, I offer
  other places they could check out (or if someone doesn't fit in... ;)

  I added an additional @tune doing_blocks_hard that will block a few more
  ways people could try to get around the system like putting a port before
  a named address or put a space between periods for named addresses.  But
  this has a much better chance of blocking normal @doings so I made it a
  separate parameter.  The regular doing_blocks_ads will block standard
  formatting unless someone really tries to thwart it.  The only way I
  thought of to get past the hard blocker is a space between domain letters
  or to mispell the domain.

  8/28/97: Added section in restart script to copy httpd log to old directory.

  9/14/97: Windows 95 port!  The compiler I decided to use for now is
  Microsoft Visual C++ 4.0 cuz I could get it for $40 at a computer store. :)
  I specifically did not want to use the Cygnus GCC libraries because I did
  not like some of the redistribution copyright provisions of the DLLs
  needed to make the program run.  And also you have to install more than
  just glowmuck.exe to run the muck.

  Win95 Caveats:
  -- Online Char Requests:  The 'request' command is pretty useless to email
     out new player requests as there is no easy way to 'mail' them.  If you
     can come up with a way to make a 'newuser' program that accepts an
     email addy, name, and password and mail it out, then in theory it would
     work.  The code is in place but I haven't the foggiest how to do the
     external newuser program.
  -- Scripting and Restarting:  csh or sh scripting isn't available without
     the Cygnus stuff, so I made a basic RESTART.BAT that does most of what
     the usual restart script does.  This is in the 'win' directory.  You
     can copy it to anywhere in your path statement.  You MUST modify this
     file in the same way you would modify the normal 'restart' script,
     setting the game directory's drive, path, and data file names.  One
     thing RESTART.BAT does NOT do is try to make sense of .PANIC and
     delta-files that don't properly get saved into the glow.new file if the
     muck crashes.  For normal operation as far as I know you don't really
     need this, and at worst you would lose the same kind of data as if the
     muck actually crashed, with the side effect of having the PANIC file
     there to get lost data from.
  -- Hostname resolving: Will work but right now the separate 'resolver' is
     not used.  Thus bad hostnames could cause the muck to lag up to a
     minute, but Cynbe's codes in there to make that a bit easier on the
     muck is still in there, so hopefully that lag will be minimal if at all.

  To aid creating characters I added the normal fuzzball code back in to
  do 'create' on the logon screen.  To enable it do @tune registration=no
  Site checking is the same as for the request command so wizards can
  prevent bad sites from free-for-all char creation and limit the amount
  of characters created per time unit.

  In doing the Win95 port I found and fixed a bug in mud.c that I am amazed
  hasn't crashed by now, yet another uninitialized variable quirk.  Apparently
  Microsoft C won't allow access to uninitialized variables, cool if true.

  Made @frob so you must supply a valid recipient.

  Made it so Boys can set/unset Q bits on wizards, #1 can set/unset Q bits
  on boys.


## 7/23/97:  Release 1.8.0 (New wizlevel definitions for W2 & W3)

  7/22/97: Added a '-connect' command that will allow users to connect without
  seeing the spammy MOTD if they want, similar to -user@address for ftp.

  Fixed the check for max_players, there were competing static variables
  for the original Cynbe hack and the Foxen cloned Cynbe hack, I adopted
  Foxen's method as Cynbe's original only works if someone types WHO when
  the max player count is less than the current player count.

  I'm not sure if it will be enough or not to reduce the logwall spam of
  @force logging, but I added a check to not log the @force if it seems to
  be for a zombie from a non-wiz player.  MPI and muf force are both not
  logged, so try to implement pet programs using them.

  ** Significant Change **   W2 wizards are going to be given access to
                             @/props because with the addition of the
  TinkerProof flag, the W and W2 wizard levels are virtually identical.  As
  such I've decided to make the current W3 level actually W2, and make the
  W3 level be the same except that it won't be affected by the tinkerproof
  flag.  This can allow muf programs that would otherwise fail due to the
  tinkerproof flag to work, as well as MPI programs.  Further research is
  needed to verify that muf and mpi permissions on wizbit programs function
  properly with tinkerproof flags.

  For now I will go with only 1.8 until the newest changes in wizard powers
  can be tested to ensure they work properly.




	#### End of 1.7 versions, moved to 1.8 ####


## 7/19/97:  Release 1.7.5  (New features, some misc bug/typo fixes)

  7/19/97: Discovered support for realms_control was missing, added it back
  in with a few changes.  Realms managers can't touch wiz-owned things, and
  can only control non-tinkeproof set rooms, things, and exits.

  Added a '@check' command to see if you control something, very useful with
  all the added rules of tinkerproof and realms.  Added help for @check and
  realms control.

  Changed the 'strncmp' to 'strncasecmp' for the logon connect, help, and
  other commands for people who have their caps-lock key on. (shoot me now :)


## 6/27/97:  Release 1.7.4 (bug fix release)

  6/21/97: Oops, fixed a bug so all players display correctly in mud rooms.
  Set it so @set thing=W sets it MAGE instead of WIZARD (W2) now.  W1 is no
  officially just refered to as the 'W' or wizbit.
  Added color and flags to several 'owner' fields in examine.

  Fixed several perms in MPI parsing for mages.  It will take some time to
  decide whether or not to adapt muf and MPI to fully use tinkerproof flags.
  My main concern is with teleporting and spying devices, which Foxen has
  worked hard with MPI to make it safer in those respects.  the 'controls'
  keyword if used properly in muf/MPI programs should be sufficient and not
  break every current program in existance until they can be adapted.

  Set it so wizards set GUEST will allow other wizards to modify objects
  they control.  It makes the wizard (except for the player him/herself) act
  like a mortal player without Tinkerproof set.  The only caveat is if an
  object's mucker level is higher than the wizard trying to modify it.  A
  wizard can only modify objects of their wizlevel or lower.

  6/26/97:  Ooga!  Fixed a bug if a player typed 'alias' but didn't have
  any aliases set.  I changed a lot of references that use 'propadr > 0'
  but don't also check if the prop name isn't empty.


## 6/16/97:  Release 1.7.3 (slightly less beta-ish)

  6/14/97: Changed lead/follow to require a thing to be set ZOMBIE to be
  a valid target for lead/follow.

  Set it so Zombies set QUELL don't spam the owner with everything it sees.
  Changed it so Zombies don't get swept home like normal things or exits
  into droptos or after everyone leaves a room.

  6/16/97: Rearranged the TINKERPROOF flag to make it somewhat useful:
  If a player is TINKERPROOF, a wizard can't modify anything they own,
  unless that object is set TINKER.  This applies to wizzes too.  If a
  wizard sets something TINKER whether they are tinkerproof or not, other
  lesser wizards will be able to modify that object unless they don't pass
  the Wbit level restriction for that object.

  6/16/97: Made more space for names in the wizard WHO list and added in
  player #s.  Added 'J' flag for rooms.


## 6/14/97:  Release 1.7.2 (beta-ish)

  6/13/97: Added lead/follow/release support.  For now I'm allowing the
  possibility of following sleeping people as I want people to be able to
  follow mobs in the mud.  If the game shuts down however all follows are
  lost, as they don't get saved in the db.  Support for following through
  both @paths and normal exits works.  The exit must exist on the room.

  6/14/97: Added command alias support.  $0-$9,$-0-$-9,$*,$$ parsing is
  supported.  The aliases are stored in the @/alias propdir in the form of:
  @set me=@/alias/command:$0 $1=$-1  (would turn 'command a b c d' into
  'command a=b c d' and run the expanded command.)  Added an @alias/alias
  command to list/set/clear aliases.

  Because of the complexity of the 2 above additions I'm going to make a
  release as I would hate to lose the code locally and have to figure these
  out again! :)  I'd also like to have others try them out and see if there
  are any quirks I missed.


## 6/11/97:  Release 1.7.1

  5/27/97: Security fixes for Tinkerproof: Added a 'force_level' check for
  flag setting permissions.  Uncommented do_force() logging.  Players
  themselves really do need to be set 'T' to ensure that wizards can't
  @force them to do things to objects they set 'T'.  I'm still debating if
  @this is a bug or a feature.

  5/27/97: Discovered that the 'newuser' script had a '$1' when it should
  have been "$1", causing mail to not get sent.  Not sure how that snuck in
  or when.

  5/31/97: Added a wiz-wall when a new player is emailed.  (Wizzes can also
  type note now to see if someone is really new, or examine their creation
  date.)  Removed an estranged "@listen" prop reference, updated man.txt.
  Added @sex and @species commands.

  6/11/97: Upgraded to Fuzzball 5.56.  It fixes some crashing bugs for muf
  programs, but Cynbe had already found this bug many a year ago.  The fix
  in 5.56 was a bit larger so I put it in to be more compatible.

  Removed the @connect, @disconnect, @arrive, and @depart props and added
  2 new @tune parms, user_connect_propqueue and user_arrive_propqueue so
  _connect, _disconnect, _arrive, and _depart can be used along with the
  usual ~propqueue set.


#### 5/26/97:  Release 1.7.0

5/21/97: Finally realized why @dumps only registered as 00:00 m:s, cuz the
  function is for h:m not m:s, fixed. :)
  Fixed a bug in look_mud_contents where the details instead of items
  were not being shown in the room if the room is set dark.
  Fixed a typo that crept into can_see() for dark_sleepers that broke it.
  Changed 'score' so it shows normal pobbles outside mud rooms.

5/24/97: Fixed a few quirks in the mud code involving combat.
  Fixed the BOOT_MESG and LEAVE_MESG that should have been @tune strings.
  (I had missed a few entries in interface.c)
  #defined NO_MEMORY_COMMAND as default as some systems don't seem to have it.
  Fixed it so Mages+ can request characters without the time restriction.

  Added --More-- Paging to the server!  You can set your page size with the
  'more' command.  You press return to scroll between pages.  If you type a
  command it will clear the line count.  Thus you will only get -- More --
  if you really need it, ie you're reading a long description or post, or
  you went idle and people are pagespamming you.  Typing 'more nn' will also
  release a page of spam too, but typing just 'more' does not, you have to
  press a bare return for normal scrolling.

  This adds a lot of functionality into the server I have wanted for some
  time, namely the ability to page through help files and bbs posts.  Thus I
  deem it worthy of a release on its own as I'd like it tried by the masses.
  An example of the power more-paging gives you is my 'bbnext' feep in $bbs. 
  You can now type 'bbnext 1000' with more paging on and just press return
  and read all new messages with raw telnet, don't need tinyfugue for this
  anymore.  You don't have to limit your posts to 1 page at a time now! :)

  Added an 'idle' flag for wizards who want to appear idle when not.
  @find =I will show them.

5/25/97:  ** SIGNIFICANT CHANGE **   Added a 'TINKERPROOF' flag.  This is a
                                     rather significant change in wizpower
  and abilities they have over normal players.  Anything set TINKERPROOF
  forbids all wizards from having omnipotent control over it.  Mages have
  thus been upgraded to have 'control' over anything that is not set
  TINKERPROOF.  Wizards and ArchWizards are also forced to abide by this
  rule.  The exception is the GUEST flag.  Anything whose owner is set GUEST
  can be modified by any Mage, Wiz, or Archwiz.  W1 programs now have the
  same read and write abilities as W2 programs.  Use the 'controls' prim
  always.  The Boy and Man levels are unaffected by Tinkerproof flags.

  Created in-server versions of @jail and @free.  This will allow Wizards to
  demuckerbit players who just won't behave and set themselves TINKERPROOF
  to annoy wizards.  People @jailed are put in a certain room that they
  aren't allowed to leave, and set GUEST.  Jail and free are logged for
  security purposes.

  Fixed a bug where #1 isn't seen as 'the man' (when using Boy(player)).
  Changed it so Mages can do ex thing=/ and set Mucker bits on players.
  Added @find =N for Tinkerproof things.

  Fixed a few mud bugs: Added a lastused timestamp update when objects
  go to the dead or offered rooms so the 7 minute reset works right.
  Changed the timewait second value from 5 seconds to 4 seconds (it's 10
  seconds in normal fuzzball).  Changed a few FGT()s to GETFGT()s.

  Added a feep in 'help' that will 'look' at an action global if it has its
  exit set at least priority level M1.  Thus now you can do 'help globals'
  (if you have a globals program, @desc the 'globals' exit with something
  like... @desc globals=@$cmd/globals and if there is no help for the world
  'globals' in the standard help.txt file, after doing @set globals=M1, you
  can do 'help globals' and the server will look at the exit and show a list
  of globals.  This essentially gives built-in help for every in-server muf
  program or MPI global command.  Keen!  :)

  Added a few more loggings at the connection screen for the help command,
  typing WHO, and if they typo something.  This should allow wizards to
  determine if users are having excessive difficulty logging in.

  With the change in mage policy IMHO time for a 2nd version # bump. :)




	#### End of 1.6 versions, moved to 1.7 ####


## 5/21/97 Release 1.6.4

4/15/97:  Oops, put in a guest check for @colorset. O:)

4/17/97:  Nuther oops, set LMAN entries in tune.c to LBOY.

4/26/97:  Moved the 'Parent:' stat in examine to where 'Location:' in other
  object types is.  Moved contents list to end of examine.

4/26/97:  Added a new contents list format for rooms set K (Mud rooms).
  Added a new @oname command to set names of objects shown in the room this way.
  Added 'oname' and 'idesc' defines in compile.c for muf programs.
  Added 'oname' and 'idesc' to examine.

*** Renamed the distribution from Neon to Glow to keep version confusion
*** out of the way with Loki's Pueblo enhanced version (eventually I'll
*** see what we need to put Pueblo stuff in GlowMuck code.)  Yes I've
*** already been reamed for the name, but I wanted something colorful.

5/21/97: Finished up the linewrap (inserver) code.  Set it with the @wrap
  command.  This is still experimental but has survived grueling testing
  with massive spamming test rats so it appears to be doing ok.


## 3/23/97 Release 1.6.3

2/1/97  Added an @/pw prop on players to store the initial password sent to
  new players.  Wizards can check this if mail gets lost being sent to
  users.  The prop is also used to warn new players to change their password
  when they log in until they do.  Using @password clears this property.

2/1/97  Separated the no-ID check from reg_user_is_barred() in reg.c and
  moved it to interface.c.  If a character has no @/id property they are
  told to send email with RL info. @set *player=@/id:something to allow
  players without RL info to log in.

2/1/97  HISTORIAM AND TIMEFRAME COMMENTARY.  Went through this ChangeLog and
  added the few known release dates, and will put dates of changes from now
  on to give timeframe info.  First changes were those made by Cynbe to
  Fuzzball 4.44, which I converted to 5.43, mayhaps one of the most stable
  Fuzzball releases.  This conversion was done for the infamous Qwest
  resurrection of 1995 to help cut down on RAM bloat and increase stability
  of the server.  Changes were continued based on the desires of the wizzes. 
  After Qwest was shut down I started another muck for the dead Qwest's then
  headwiz called Euphoria.  Yet more changes were added.  After being with
  Euphoria roughly two months I resigned, and the code we call GlowMuck most
  likely would have died then.  But I was lonely and bored and had to keep
  adding feeps.  At some point I upgraded my Linux system and lost the
  server source code, ack!  (Roughly March 96.) I had given one copy of an
  earlier version of the server code that could run on 64 bit alphas, and
  had to revert to that version and re-add the multiple wizard levels and
  many other new feeps.  At this point I decided public releases would be a
  good thing, and put the code on my local ftp site.  I started with release
  1.1 or so, then 1.2, 1.3, 1.4, 1.5, not all added to the ftp site.  At
  this point someone from chaco.com showed interest in adding Pueblo and
  other support to the server.  The server was renamed to Neon, with the
  Pueblo version starting with 2.0.  I wanted to keep a separate not quite
  as feepy version that I maintain as 1.5 and up.  Not sure what I will do
  at 1.9.9.

3/13/97  Added a 'note' command that works similar to motd for newly
  requested characters to check if their request was processed.  Added
  mention of this in connect.txt too. 'note clear' clears the list.  When a
  player is created and emailed or if a request is filed away an entry is
  automatically added to the list.  Clean the file out when you clean out
  the @hopper.  The main purpose for 'note' is to allow something to be put
  in easy reach of people without characters if an email bounces.  I also
  changed the now seemingly missing 'Log>' mark to [$].

3/22/97  Added a ~sys/lastdumpdone prop to be set when a dbsave completes to
  allow the ability to track dbsave times.  ~sys/lastdumptime shows when the
  last dump started, but places like Furry can take over 10-20 minutes to
  finish.  Whee!  Also added something to @stats to show save times.

3/22/97:  Added a 'BOY' flag that only #1 can set. (The real #1.)  This will
  allow muck owners to have more than one truly omnipotent char.  This flag
  can be searched for by BOY set chars but only them using @find =Y.  Made
  @find so you can't search for objects higher than your mucker/wizard
  level.  I removed '$' from showing up for logwall, to find logwall chars
  use @find =$ (Arch+).  Use @find =# to find Suspect chars.  Use @find =&
  to find ethereal (still not implemented) chars.

3/23/97:  Added a '@getpw' #1 only command to get player passwords.  Changed
  the SUSPECT flag to only be settable by players set BOY.

3/23/97:  Because prim_testlock uses interp() to evaluate its lock
  expressions, the oper1 and oper2 vars can end up getting changed by other
  primitives.  I changed it so those are now local variables and hopefully
  the spam in tty.errs will go down now.

3/23/97:  Obseleted the MUFCOUNT flag '+' and changed it to use CHOWN_OK on
  muf programs (which does nothing now.) I also moved the dir that stats are
  stored in from ~muf/ to _muf/, not sure what the mentality for that was.
  Instead of the player being set mufcount to see programs, any program set
  'C' will print the instruction count after it runs to everyone.

3/23/97:  Added a @colorset command and functionality to change which ANSI
  color each ANSI color displays.  This changed a lot of the core hardcoded
  strings in interface.c amongst other places.  Tests so far have proven to
  work great, but please let me know if something prints weird or you get
  Color when you are not set to receive it or vice versa.


## 1/30/97 Release 1.6.2

Added help lists for wizchat and @wall, changed spoof # char to @, # does help.

Ok ok, you can shoot me now, I added quotas. :)  Recently I've seen Legacy
  have a need for people to have a few objects and exits apiece, and also
  wished to limit the number of muf programs people have.  Thus, @quota. 
  The quotas are set using @tune separately for rooms, exits, things, and
  programs.  Restricted_building still works for more custom tailoring,
  along with a new option to decide whether quotas are in effect for builder
  bits.  (Thus you can decide if a B bit means if a player can build or if
  it means they can get past the quota limitations.) building_quotas
  enables/disables quotas.  Whee!

Added a 'space_nag' @tune parameter to turn on/off @path/@object/@detail
  nagging when a person opens an exit or creates an object.

Added a 'suspect' flag that stays on people expected of excessive vulgar
  language or such.  I set it so the 'suspect', 'mufcount', and 'ethereal'
  flags will get cleared on @shutdown to keep them from running over time. 
  (Etherealness is still unsupported, just there as a hook for now.) The
  main purpose of the suspect flag is to watch tinyjerks.  Word of mouth has
  just become not an option, without actual proof of wrongdoing and having
  to go on the 'majority rules' who said the bad stuff, it just does not
  work.  As such, a person can be set suspect and have what they do observed
  for a time in the command file (semi-anonymously, logged by dbref) and if
  complaints continue, the logs can be grepped for offending comments or
  behavior, and the player properly @frobbed.  Please only use this in severe
  cases when there is reasonable evidence to believe the player is a TJ.

Revamped @tune so people from MPI levels to Wizlevels can see what settings
  that they can access (although MPI can't actually use them).  Only Arches
  can change @tune settings, and only #1 can change some @tune settings
  (like logging) to prevent overzealous wizwars.  Obviously each site can
  modify these but be sure to keep a list of what you change for future
  server upgrades.  If you feel certain levels should have access to certain
  parameters send me mail and I'll see about working something out for
  future releases.


## 12/23/96 Release 1.6.1

Changed 'Go back.' to use '/' instead of '..', apparently lynx doesn't
  support '..'.

Changed MPI Arch/Wiz/MagePerm() to not give Player or Program objects any
  form of wizardly MPI abilities.  This is part of the ever growing security
  concern of regular players being able to gain wizard powers via MPI and
  {force} by setting properties on WizBit objects.

Change {online} to be a Mage MPI prim, was W2.

Added missing logging check for MUD commands, only had the @tune defined.
  Also added a 'GST' and 'MUD' tag for such logged commands.  Missed a few
  #ifdefs for MUD support for the log_mud parameter too.

Moved HTTP activity to a separate log file in the logs directory.  Added a
  @tune parameter to enable/disable logging of HTTP activity.  (No more
  logwall spam of GETs. :)

Added the command count to disconnect logs to get a clue what people do
  online (whether or not they connect.)

Colorized and changed various wall() functions and connection messages to use
  anotify().

Changed command logging structure for a cleaner logfile and a bit more
  privacy for players online.  (Only dbref #s are saved in the log.)
  Added log_wizards back (forgot why I took that out).

Changed LOGWALL's '!' flag to '$' and added it so you can @set and @find the
  $ symbol for LOGWALL and the + symbol for MUFCOUNT.  Set it so Non-W3s can
  be set LOGWALL and actually receive log info, but only by W3s.

Ah, finally, I cleaned up one big fat hole that people can use to bloat the
  db without even knowing who did it (not to mention spam the net with
  mail.) I modified netresolve.c to store an additional field in the cache
  to note the last time a character was requested from each registered site
  (basically all net sites that have connected to the muck.) Before allowing
  a request to go through, a check is performed to see if anyone in the
  requestor's domain has made a request in the last half hour or so (@tune
  time parameter.) If so the request is tossed and a note of how long the
  player must wait to get a request in will be displayed.

  This is a hole that has always existed in the request system.  The new fix
  will prevent players from requesting more than X players per unit time. 
  Set to a reasonable value, between a half hour and 2 to 4 hours, this
  should prevent massive db bloat by someone having fun with requesting a
  pile of characters over and over again.  In theory one could still request
  characters over a long time and bloat the muck, but as long as a wizard
  checks at least once a day to once a week, the damage can be minimized. 
  The offending sites or domains can then be set to block further character
  requests.  The ability to basically email any site a request is still in
  theory a flaw, but by keeping offending sites blocked from automatic
  processing, this problem should be minimal.  With the bad-name checker and
  this new spam limitation code in place, this effect should be very small.




	#### End of 1.5 versions, moved to 1.6 ####


## 12/8/96  Release 1.5.9/1.6

Fixed a bug where guests could create rooms if all_can_create_rooms is on.

Fixed a bug where anyone can @set their KILL_OK flag even if MUD is defined.

Fixed a bug so http://yoursite:9998/@credits hides the color codes.

Fixed @link, @action, and @attach so mages+ can link to player names.

Oops, added GUEST and a few other new flags in to the muf 'set' command.

Cleaned up the 'motd' printout slightly.  Dates can now be turned on or off
  using a @tune command.

Cleaned up Fuzzball so one can compile it without COMPRESSION #defined. 
  There is an apparent subtle bug in Dr. Cat's compression routine that may
  cause words to be mixed up, so I have set compression off by default in the
  GlowMuck distribution until a fix can be worked out.

Added a /@help http special entry to show connection help.  Added a 'Go back.'
  tag to all http @funcs.


## 7/21/96  Release 1.5.0

Features added that were in the lost code:

Switch to M, M1-M3, W1-W3 wizard levels, see 'help newlevels'.  Added
  GUEST, LOGWALL, MUFCOUNT, ETHEREAL, and MOBILE flags to a 2nd flagset
  that stores on the same line as the 'flags' line of fuzzball mucks.

Added logwall, guest, mufcount functionality.  logwall on a wizard will
  show all status_logs to them while online.  Mufcount lists the # of
  instructions when a muf program finishes.  Ethereal and Mobile have yet
  to be given meaning.

The 'dark' flag on players now debugspams them with ALL programs that they
  run.  No permissions checking, etc.  The reason for this is so I can debug
  problems with Guest programs without having to set them Wizard so they see
  the debug spam.  Heh.  I know it's a bad hack. ;)

Re-added the 'viewable' flag so only owners can @set them on things.

## End of features added from lost source version ##


Fixed it so guests can do 'requests' online.

Registered objects now start with #0 then work down.  Didn't do this for
  normal envprops so @path and such defaults still work.

Added 'dwall' and 'dinfo' commands to get info or wall descriptors like
  one would for @wall or wizchat.  @wall and dwall now can do # for spoofs
  and : for poses ala wizchat.

Added two feeps for @path so they now work with better in-server
  functionality.  These fixes allow 'w' to mean 'whisper' amongst other
  feeps.  Players can now create path names of whatever they want, and
  even have them separated by ;'s.  The fixes are in game.c to do the
  path-moves and in look.c to do the look of the @path.

Changed ANSI so ^^ makes a ^ and any unknown color, like ^ ^ turns things
  back to normal.

Added a ? command to wizchat to show wizards listening.

Added a LOT more color to the server functions.  Green is good, Red is bad.
  Made a 'tct()' function to convert ^s to ^^s to make things work well.
  Changed the old COLOR defs to ANSICOLOR to make incorporating global
  color defines into other programs easier, defining a new set of COLORs
  ala #def RED "^RED^" to be used in various messages.  Renamed ansi_notify
  to anotify and added many other ansi funtions for notify except/from etc.
  Added MPI parsing to inserver say/pose/whisper/page also.

Added an alias 'scan' for '@sweep'. (puppet sniffer.)

Fixed the connect action sequence again so it goes arrive -> connect.

Changed it so with Mageperms objects won't do name notifies in MPI for
tell(), otell(), or ansi().

Added a sprintf() to dump_users() to keep really long names in WHO! from
  spamming, hopefully it won't crash stuff as I used a basic C nono. ;)

Fixed a blunder where I had a host_free() call in host_save() in netresolve.c
  and thus after each db save the hostcache gets cleared.  Doh!

Nuther blunder, had the check inverted for setting vehicles and Zombie
  flags, mortals but not wizards could set them.  Doh!

Added a {propdir} MPI prim, upgraded to fb5.49, hopefully bug free.

Started adding mud code to kill and event queues.  Added 'offer' and 'cast'
  commands.  Very unpolished right now.  This will need to be documented
  once we get to a point it is actually playable/usable.  Need 'flee'
  support.

Added 'connect' log support for all input on the login screen to a
  logs/connects file.  Also added 'dboot' to boot descriptors.

Tinkering with a 'GET' catcher on the login screen to allow the muck to
  serve WWW pages.

Yay! We have home pages!  The '_/www' property is used on players to store
  player personal pages.  This trick REQUIRES that the 'welcome.txt' file
  have NO blank lines and may need Ctrl-M CR/LFs to work correctly for some
  browsers.  (put a space on the line, use the 'joe' editor to be sure it is
  spaced properly.)  You can remove the HTTPD in config.h.  A @tune parameter
  was added for the standard / page.  User pages are accessed with ~username.

Added feep to NOT log programs to logs/programs compiled by ArchWizards.  Other
  mucks may wish to change this from TArch to TWiz or TMage depending on
  trust, disk space, and wizard activity. The fix for this is in edit.c,
  grep for tp_log_programs.

Spruced up @stats and @proginfo more with program memory usage, total
  memory counts too.  @stats now also lists other objects in memory and
  usage besides active programs.

Added 'fast_registration' tune parameter for auto-email-registration. 
  Banned sites and those with ##jerk## email addresses will not be
  automatically created when this is turned on, otherwise when a player
  makes a request the player is automatically created and the email is
  sent out without needing to do a @hop reg.  The data is still stored in
  the hopper logfile in case something goes wrong (many typos in email
  addresses occur.)

Upgraded to 5.50, minor cosmetic fixes to the new code to bring it in line
  with the ratty muck setup.

Added a 'x' BLOCKED status flag to #0=@/sites/ to keep really bad TJs from
  even getting to the login screen.  The connection is canned before ACPT.

Added @tune 'max_output' to set max allowed queued output without recompiling.
  Set this to 16000 or 8000 if you're really tight on memory, or higher to 32000
  or 64000 if you are archiving or listing large programs.

Added {showlist} MPI prim to list a full list out past the mpi limit, like tell.

Upgraded to 5.51, changes {propdir} to {dirprops} to accomodate new foxen stuff.

Revamped the HTTP server to do lsedit lists and have a better chance of having
  site redirection working with more browsers.  Users can have 'trees' of home
  pages using lsedit lists.

Added site-based and random welcome_user screens.  A data/welcome directory
  can hold many custom pages for certain domains.  welcome1.txt-welcomeN.txt
  can be linked to pages in the welcome directory for random pages.  Use
  @tune rand_screens=num of welcomeN.txt files.  You can do frequencies by
  putting a page in the series more than once.  A BARD_FILE in config.h can
  be set to the file to show 'barred' sites.  This can be use a symbolic
  link to another file too.  You set site-specific pages with the format:

  @set #0=@/welcome/1 128.30.5.0:pagename why

  Where the '1' is an order prefix so if you want to do single-machine pages
  and also have larger domains with other pages, basically set this # to the
  # of '0' masks in the site name.  Use 0-4.  The pagename can be either a
  data/welcome/pagename.txt file, '#', or '.'.  '#' means to use randomness
  despite possibly a blockout or other default page.  '.' means to use the
  standard welcome.txt file for that site/domain.  You can put a comment
  after a space in the pagename or the # or . character. Whew!

Fixed a bug in the parse_connect code that truncated email addresses in the
  auto-registration hopper.  Set it so Mages can see the wizard WHO list,
  and use the conhost and conipnum prims.  Also fixed it so Arches can see
  the username in the wizard WHO list again too.  (Long overdue. :)

Upgraded to Fb5.55, requiring some misc mods to existing functions so they
  work with 5.55's new debugging features.  Also some misc fixes: Set
  MAX_COMMAND to 1024 bytes as it is in stock fuzzball; Changed the newbie
  hopper walls to be arch only to spare other wizzes; officially named this
  code 'GlowMuck'; respaced the WHO list again and changed sites to show
  user@host in the wholist for arches.

Added \0s to strings called by format_time(), it would appear sometimes
  format_time will not append a NULL.  I'm not sure if this is a Linux or fb
  bug new to 5.55.  the muf and MPI ftime calls don't have \0s on the end
  just cuz it would require adding a buffer.  Time will tell what this bug is.




######## 3/96  SOURCE CODE WAS LOST, STARTED OVER WITH SLIDER'S COPY ########




    #### 8/95  ORIGINAL SERVER CHANGES #### -- see CYNBE_FIXES for Qwest mods

Set something so program logs show a player's mucker level.

@/lockout-msg: show to player and not let them log in. (CYNBE_FIX)

Searched and cleaned up #ifdef FUZZ's, many other things Cynbe changed.

Added @tune parameters for codewizard and corewizard (for logging, etc.)

Set program log to do {{{ }}} folding syntax and to show player flags.

Added @tune parameters for max_mortals, can disable all non-wiz logins.

Added !WHO to override muf-program '@WHO ' replacements.

Added @tune parameter reg_email = me@wherever to set email of registration
  wizard for special registration cases and lockouts.
  Note: welcome.txt needs to be changed to note how to register ala:
  register playername email-address

Added @hopper command and code to "register" for new online e-mail
  registration system for wizards.

Added 'wizchat'/'wc' command so wizards can chat amongst themselves, notify
  others wizards of potential tinyjerk situations, etc.

wall_wizards was changed not to go to codewizard/corewizard, as they most
  likely would prefer not to be bothered with such things, and player gripes
  should be as private as possible.  Being set '@nowizwall' or 'Q' should keep
  a wizard from hearing the walls.

Added @tune parameter reg_wiz to ensure the registration hopper isn't worked
  over doubly at the same time.  Note that the status log also has all
  register requests in case a @hop clear is done right as a new one comes in.

Changed @tune so both *player matches and 'me' will match for dbref entries.

Added @tune parameter building to stop db-growth if dangerous.
  Commands that lower db bloat such as @recycle are not affected.
  Other commands such as @link, @attach, and @edit are also unaffected.

Added @tune parameter db_readonly to stop major/minor db changes.
  This is mainly useful to let characters log in to an old db to @archive
  belongings without fear of players mass-recycling themselves, etc.
  @toad, @newpassword, and @tune still work to handle baddies.
  Otherwise the game is basically in 'look but don't touch' mode.

Changed @name so a wizard can '@name *player=newname yes' if that player does
  not have a wizard bit.  This will help the onslaught of new players claiming
  to be old players that we have no way to verify.  We can safely move the
  old player with an old name in case the real owner might come back.  This
  will protect any page #mails, morphs, etc, even if the player has no
  belongings.  We will also be able to rename profane names without
  @newpasswording, but I'm not sure that's a good thing yet. :)

Removed all abilities of Players to be 'dark' anywhere but in dark rooms.

Changed look so 'dark' rooms will not show contents even if the owner controls
  the room.  Examine will still show all contents.

Changed 'simple_look' in look.c so the 'command @' variable is set to the
  name of the thing that was looked at instead of '(@desc)'.  All mods are
  in look.c alone to do this.

Added a small feep to record _sys/lastcleantime for cleanup intervals.

Fixed program logging with the new unparsing of both player and program.  A
  buffer is needed for each unparse_object if it is used more than once in a
  notify or printf style command.  On Betz (an Irix) the second unparseobjed
  object would be printed for both values, on Linux the first one would be
  printed.  So much for thinking we know which way commands inside commands
  process in C (and possibly if clauses too.)

Added logging failed attempts by mortals to @armageddon.

Added support so mortals can do !@ps.

Set it so wizards can't @force other wizards.

Fixed MPI ref() and flags() to require strict privileges to get that info.

Added @/mage feeps to some wizard commands: (@set *player=@/mage:when:who:why)
@stats   -- check anything and see DB usage stats.
@usage   -- can use it
@memory  -- can use it
@dbginfo -- can use it (whatever it is ;)

Fixed @tel so players can @tel to Jump_Ok rooms and to/from their home.
  Rooms set _private are an interesting thought.  If the room is set _private
  and Jump_Ok, only the owner would be able to enter it.  If the room is set
  _private:yes and not Jump_Ok, only the owner would be able to enter it.  This
  seems to make the Jump_Ok flag pointless, if we were to add the check for
  _private:yes rooms.  Thus a check to _private:yes is redundant since the
  Jump_Ok flag is already part of the server.  In conclusion, if a person
  wants a room truly private, it should not be set Jump_Ok.

Fixed @teleport matching so it will see things in the room (I hope.)

Feeped @teleport so you can teleport into players. O:) (Well wizzes at least.)

Fixed a signal handler conflict so the resolver works with @hopper register!

Added : and ; pose capability to wizchat, special @ feep for spoofs.

Fixed muf 'parseprop' prim to strip any nonprinting chars from return string.

Fixed muf 'moveto' prim to allow moving to players and vehicles.  Non W-bit
  muf programs must have player or thing destination be set vehicle, and
  follow other moveto restrictions.

Changed no-login idle boot time from 5 mins to 1 min.

Added a call to resolve_hostnames() in the registration connection command
  in hopes of getting a bit better chance of getting non-numerical site ids.

Condensed the wizard WHO list so hosts have about 7 more characters to fit
  on a line.  The 'Doing...' title in WHO with who_doing @tuned on is now
  replaced with what is in the _poll property of Room #0, if it is set.  The
  max # of the _poll and @doing lines was set to 43 characters, to make the
  max WHO line length 79 characters.  (Bob always demands I make everything
  a max of 79 chars wide.  Some weird terminals don't handle the 80th one
  right.)

Fixed @tune so you can @tune db_ref=#-1 to unset dbrefs.

Changed the KILL_OK flag into GUEST, removed tp_restrict_kill and KILL flag
  references.  Added code everywhere to prevent commands a Guest might use
  to mangle the Guest character or bloat the db.  This flag should also be
  good to limit what potential tinyjerks can do.  No building, programming,
  examining, or @setting of any properties.  Muf programs could give guests
  the ability to do these things still, however.

Added a 'help' command to the connection screen which spits a file similar
  to data/welcome.txt called connect.txt.  This should allow for an easier
  to understand connection process while allowing a prettier login screen.

Fixed bugs with db_readonly which caused room descriptions to disappear. 
  Use db_readonly=yes ONLY if you have a backup of your db!!!  It wasn't
  designed for dbs that you care about, that's why it's so hard to type!

Altered several Muf and MPI prim mucker levels, disabled M3 from being able
  to write to _props.  Let M2 use parseprop (not sure if M1 should too?)

Added ANSI COLOR to Fuzzball via the C flag on players.  Several In-Server
  changes and many muf modifications were needed to pull this off.  To note,
  an ansi_notify muf prim, {ansi:text} MPI notify prim, ansi_unparse_obj,
  and several other support routines were added.


