#!/bin/csh -f
#
# Sole argument is a port number, if desired,
# if none is given defaults to 4321.
#

#
# You'll want to edit this to match the base directory of your muck's files.
# This should be the directory containig the 'data' amd 'muf' directories.
# Set MAIL to the mailer your system uses, sometimes /bin/mail or mailx.
#


#
# The default ports that the MUCK server should listen to, if no port
# numbers were supplied as command line arguments to this script.
# To list multiple ports, separate them with spaces.  ie:
# PORTS="4201 4202 4203"
#
set PORTS = "9999"


#
# You'll want to edit this to match the base directory of your muck's files.
# This should be the directory containing the 'data' amd 'muf' directories.
#
set GAMEDIR = "$HOME/glow/game"


#
# Set this to run your local mail delivery program.
#
set MAIL = "/usr/bin/mail"

#
# Set this to an '@' and then a numerical IP address to make the server only
# listen on a specific IP address.  This is used for mud hosting services
# that require muds to listen only on one address.
# set ADDR = "@128.83.194.15"
set ADDR = ""

#
# The following are the paths to the db files to load, save to, and archive to.
# DBOLD is the path and filename of the the previous database archive.
# DBIN is the path and filename of the database to read in.
# DBOUT is the path and filename of the database that the muck should save to.
#
# On a successful restart, DBIN is moved to DBOLD, and DBOUT is moved to DBIN,
# then the server is started.  The server will save it's db to DBOUT.
#
set DB = glow
set DBOLD = data/$DB.old
set DBIN = data/$DB.db
set DBOUT = data/$DB.new


#
# It's doubtful you will want to change this, unless you compile a different
# path and filename into the server.  This is the file that deltadumps are
# saved to.  After a successful restart, these deltas will be appended to the
# end of the DBIN file.
#
set DELTAS = data/deltas-file


###############################################################################
#
# You probably won't need to edit anything after this line.
#
###############################################################################

#limit stacksize 16384
limit coredumpsize 50000
limit memoryuse 16000

cd $GAMEDIR

#This is just so you get a new line if started by glow with a &.
echo

if (! -x glowmuck) then
	echo "You need to compile the glowmuck server before you can run it!"
	echo "Read glow/INSTALL to see what to do."
	exit 0
endif

echo "Restarting at: `date`"

umask 077
unlimit

set muck = `ps ux | grep glowmuck | wc -l`
set datestamp = `date +%m%d%y%H%M`

if ($muck > 1) then
	echo "A glowmuck is already running:"
	echo  `ps ux | grep glowmuck | grep -v grep`
	exit 0
endif

# Save any corefile:
set timestamp = "`date +'%y.%m.%d.%H.%M'`"
if (-r $GAMEDIR/core) then
    mv $GAMEDIR/core $GAMEDIR/core.$timestamp
endif

# Rename+compress "status", "commands", "connects", "httpd", and "programs" logs too:
if (-r $GAMEDIR/logs/status) then
    mv $GAMEDIR/logs/status $GAMEDIR/logs/oldlogs/status.$timestamp
    gzip -2 $GAMEDIR/logs/oldlogs/status.$timestamp
endif

if (-r $GAMEDIR/logs/commands) then
    mv $GAMEDIR/logs/commands $GAMEDIR/logs/oldlogs/commands.$timestamp
    gzip -2 $GAMEDIR/logs/oldlogs/commands.$timestamp
endif

if (-r $GAMEDIR/logs/mud-commands) then
    mv $GAMEDIR/logs/mud-commands $GAMEDIR/logs/oldlogs/mud-commands.$timestamp
    gzip -2 $GAMEDIR/logs/oldlogs/mud-commands.$timestamp
endif

if (-r $GAMEDIR/logs/connects) then
    mv $GAMEDIR/logs/connects $GAMEDIR/logs/oldlogs/connects.$timestamp
    gzip -2 $GAMEDIR/logs/oldlogs/connects.$timestamp
endif

if (-r $GAMEDIR/logs/httpd) then
    mv $GAMEDIR/logs/httpd $GAMEDIR/logs/oldlogs/httpd.$timestamp
    gzip -2 $GAMEDIR/logs/oldlogs/httpd.$timestamp
endif

if (-r $GAMEDIR/logs/programs) then
    mv $GAMEDIR/logs/programs $GAMEDIR/logs/oldlogs/programs.$timestamp
    gzip -2 $GAMEDIR/logs/oldlogs/programs.$timestamp
endif

touch logs/restarts
echo "`date` - `who am i`" >> logs/restarts

if (-r $DBOUT.PANIC) then
	set end = "`tail -1 $DBOUT.PANIC`"
	if ("$end" == "***END OF DUMP***" ) then
		mv $DBOUT.PANIC $DBOUT
		rm -f $DELTAS
	else
		rm $DBOUT.PANIC
		echo "Warning: PANIC dump failed on "`date` | $MAIL `whoami`
	endif
endif

#
# Fix the next lines to save all old dbs with datestamps if desired.
#

if (-r $DBOUT) then
    if (-r $DBIN) then
	# Cut these down to the # of spare backup dbs you want to keep
	mv -f $DBOLD.gz.2 $DBOLD.gz.3
	mv -f $DBOLD.gz.1 $DBOLD.gz.2
	mv -f $DBOLD.gz   $DBOLD.gz.1

	rm -f $DBOLD $DBOLD.gz
	# Do NOT run this with an '&', it will mess up deltas-file
	( ./glowmuck -convert -decompress $DBIN $DBOLD ; gzip -8 $DBOLD )
#	mv $DBOLD $DBOLD.$datestamp
#	nice -20 gzip -9 $DBOLD.$datestamp &
    endif
    mv -f $DBOUT $DBIN
endif

if (! -r $DBIN) then
	echo "Hey\!  You gotta have a "$DBIN" file to restart the server\!"
	echo "Restart attempt aborted."
	exit
endif

set end = "`tail -1 $DBIN`"
if ("$end" != '***END OF DUMP***' ) then
	echo "WARNING\!  The "$DBIN" file is incomplete and therefore corrupt\!"
	echo "Restart attempt aborted."
	exit
endif

if (-r $DELTAS) then
	echo "Restoring from delta."
	set end = "`tail -1 $DELTAS`"
	if ("$end" == "***END OF DUMP***" ) then
		cat $DELTAS >> $DBIN
	else
		echo "Last delta is incomplete.  Truncating to previous dump."
		grep -n '^***END OF DUMP***' $DELTAS|tail -1 >! .ftmp$$
		set llinum = "`cut -d: -f1 < .ftmp$$`"
		set llcnt = "`wc -l < .ftmp$$`"
		if ($llcnt > 0) then
			head -$llinum $DELTAS >> $DBIN
		else
			echo "Hmm.  No previous delta dump."
		endif
		rm .ftmp$$
	endif
	rm -f $DELTAS
endif

set dbsiz=`ls -1s $DBIN | awk '{print $1}'`
set diskfree=`df -k . | tail -1 | awk '{print $4}'`

@ diskneeded = $dbsiz * 3
@ spacediff = $diskneeded - $diskfree

if ($diskfree < $diskneeded) then
    echo "WARNING: you have insufficient disk space."
    echo "The server is starting anyways, but you should immediately clear out old log"
    echo "files, etc. to free up approximately $spacediff more K of disk space."
endif

# echo "Server started at: `date`"
( ./glowmuck $ADDR $DBIN $DBOUT $PORTS >& logs/ttyerr.log; ../bin/coremail ) &
# echo "Server stopped at: `date`"
