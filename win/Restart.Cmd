@echo off

REM  Sole argument is a port number, if desired,
REM  if none is given defaults to 9999.

SET PORT=9999
IF NOT "%1"=="" SET PORT=%1


REM  You'll want to edit this to match the base directory of your muck's files.
REM  This should be the directory containig the 'data' amd 'muf' directories.

SET GAMEDRV=F:
SET GAMEDIR=\GLOW\GAME


REM  The following are the paths to the db files to load, save to, and archive to.
REM  DBOLD is the path and filename of the the previous database archive.
REM  DBIN is the path and filename of the database to read in.
REM  DBOUT is the path and filename of the database that the muck should save to.


REM  On a successful restart, DBIN is moved to DBOLD, and DBOUT is moved to DBIN,
REM  then the server is started.  The server will save it's db to DBOUT.

SET DBOLD=data\glow.old
SET DBIN=data\glow.db
SET DBOUT=data\glow.new


REM  It's doubtful you will want to change this, unless you compile a different
REM  path and filename into the server.  This is the file that deltadumps are
REM  saved to.  After a successful restart, these deltas will be appended to the
REM  end of the DBIN file.


REM  #########################################################
REM
REM  You probably won't need to edit anything after this line.
REM
REM  #########################################################


%GAMEDRV%
cd %GAMEDIR%

IF NOT EXIST glowmuck.exe GOTO NOEXE

echo Glowmuck Restarting...

REM  Append "status", "httpd", and "programs" logs to:
IF EXIST logs\status   TYPE logs\status   >>logs\oldlogs\status.old
IF EXIST logs\httpd    TYPE logs\httpd    >>logs\oldlogs\httpd.old
IF EXIST logs\programs TYPE logs\programs >>logs\oldlogs\programs.old

IF EXIST %DBOUT% IF EXIST %DBIN% IF EXIST %DBOLD% DEL %DBOLD%
IF EXIST %DBOUT% IF EXIST %DBIN% MOVE %DBIN% %DBOLD%
IF EXIST %DBOUT% MOVE %DBOUT% %DBIN%

IF NOT EXIST %DBIN% GOTO NODBIN


ECHO Server starting...
START glowmuck %DBIN% %DBOUT% %PORT% >logs\ttyerr.log

GOTO END


:NOEXE
ECHO You need to compile the glowmuck server before you can run it!
ECHO Read glow\win\INSTALL_WIN95 to see what to do.
GOTO END


:NODBIN
ECHO Hey!  You gotta have a %DBIN% file to restart the server!
ECHO Restart attempt aborted.
GOTO END


:END

REM exit

