# Microsoft Developer Studio Project File - Name="GlowMuck" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=GlowMuck - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GlowMuck.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GlowMuck.mak" CFG="GlowMuck - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GlowMuck - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /Zp4 /GX- /Zi /O2 /I "..\..\src\inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN95" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /verbose /map /debug /debugtype:cv /machine:I386
# SUBTRACT LINK32 /pdb:none /pdbtype:<none>
# Begin Target

# Name "GlowMuck - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\boolexp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\case.c
# End Source File
# Begin Source File

SOURCE=..\..\src\cast.c
# End Source File
# Begin Source File

SOURCE=..\..\src\color.c
# End Source File
# Begin Source File

SOURCE=..\..\src\compile.c
# End Source File
# Begin Source File

SOURCE=..\..\src\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\src\create.c
# End Source File
# Begin Source File

SOURCE=..\..\src\crt_malloc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\db.c
# End Source File
# Begin Source File

SOURCE=..\..\src\debugger.c
# End Source File
# Begin Source File

SOURCE=..\..\src\disassem.c
# End Source File
# Begin Source File

SOURCE=..\..\src\diskprop.c
# End Source File
# Begin Source File

SOURCE=..\..\src\edit.c
# End Source File
# Begin Source File

SOURCE=..\..\src\events.c
# End Source File
# Begin Source File

SOURCE=..\..\src\game.c
# End Source File
# Begin Source File

SOURCE=..\..\src\hashtab.c
# End Source File
# Begin Source File

SOURCE=..\..\src\help.c
# End Source File
# Begin Source File

SOURCE=..\..\src\inst.c
# End Source File
# Begin Source File

SOURCE=..\..\src\interface.c
# End Source File
# Begin Source File

SOURCE=..\..\src\interp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\log.c
# End Source File
# Begin Source File

SOURCE=..\..\src\look.c
# End Source File
# Begin Source File

SOURCE=..\..\src\match.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mfuns.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mfuns2.c
# End Source File
# Begin Source File

SOURCE=..\..\src\move.c
# End Source File
# Begin Source File

SOURCE=..\..\src\msgparse.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mud.c
# End Source File
# Begin Source File

SOURCE=..\..\src\netresolve.c
# End Source File
# Begin Source File

SOURCE=..\..\src\oldcompress.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_connects.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_db.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_math.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_misc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_props.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_stack.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_strings.c
# End Source File
# Begin Source File

SOURCE=..\..\src\path.c
# End Source File
# Begin Source File

SOURCE=..\..\src\player.c
# End Source File
# Begin Source File

SOURCE=..\..\src\predicates.c
# End Source File
# Begin Source File

SOURCE=..\..\src\propdirs.c
# End Source File
# Begin Source File

SOURCE=..\..\src\property.c
# End Source File
# Begin Source File

SOURCE=..\..\src\props.c
# End Source File
# Begin Source File

SOURCE=..\..\src\reg.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rob.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rwho.c
# End Source File
# Begin Source File

SOURCE=..\..\src\sanity.c
# End Source File
# Begin Source File

SOURCE=..\..\src\set.c
# End Source File
# Begin Source File

SOURCE=..\..\src\signal.c
# End Source File
# Begin Source File

SOURCE=..\..\src\smatch.c
# End Source File
# Begin Source File

SOURCE=..\..\src\speech.c
# End Source File
# Begin Source File

SOURCE=..\..\src\strftime.c
# End Source File
# Begin Source File

SOURCE=..\..\src\stringutil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\timequeue.c
# End Source File
# Begin Source File

SOURCE=..\..\src\timestamp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\tune.c
# End Source File
# Begin Source File

SOURCE=..\..\src\unparse.c
# End Source File
# Begin Source File

SOURCE=..\..\src\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\src\version.c
# End Source File
# Begin Source File

SOURCE=..\..\src\wiz.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\inc\color.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\config.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\copyright.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\crt_malloc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\db.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\defaults.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\externs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\inst.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\interface.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\interp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\local.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\match.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\mfun.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\mfunlist.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\mpi.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\msgparse.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\p_connects.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\p_db.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\p_math.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\p_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\p_props.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\p_stack.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\p_strings.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\params.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\patchlevel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\props.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\reg.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\strings.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\tune.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\version.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inc\win95conf.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
