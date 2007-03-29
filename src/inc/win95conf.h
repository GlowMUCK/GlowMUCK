/* inc/win95conf.h.  Not generated automatically by configure.  */

/* uname -a output for certain local programs. */
#define UNAME_VALUE "Windows 95"

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you have dirent.h.  */
/* #undef DIRENT */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if your struct tm has tm_zone.  */
/* #define HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define if you don't have dirent.h, but have ndir.h.  */
/* #undef NDIR */

/* Define to `int' if <sys/types.h> doesn't define.  */
#define pid_t int

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you don't have dirent.h, but have sys/dir.h.  */
/* #undef SYSDIR */

/* Define if you don't have dirent.h, but have sys/ndir.h.  */
/* #undef SYSNDIR */

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
/* #undef TIME_WITH_SYS_TIME */

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define if the closedir function returns void instead of int.  */
/* #undef VOID_CLOSEDIR */

/* Define if you have getrlimit.  */
/* #undef HAVE_GETRLIMIT */

/* Define if you have getrusage.  */
/* #undef HAVE_GETRUSAGE */

/* Define if you have mallinfo.  */
/* #undef HAVE_MALLINFO */

/* Define if you have random.  */
/* #undef HAVE_RANDOM */

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/resource.h> header file.  */
/* #undef HAVE_SYS_RESOURCE_H */

/* Define if you have the <sys/signal.h> header file.  */
/* #undef HAVE_SYS_SIGNAL_H */

/* Define if you have the <sys/time.h> header file.  */
/* #undef HAVE_SYS_TIME_H */

/* Define if you have the <sys/wait.h> header file.  */
/* #undef HAVE_SYS_WAIT_H */

/* Define if you have the <unistd.h> header file.  */
/* #undef HAVE_UNISTD_H */				

/* Define if you have the m library (-lm	).  */
/* #undef HAVE_LIBM */	
	
/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */


/* if tm_gmtoff is defined in time.h, define this */
/* #undef HAVE_TM_GMTOFF */

/* if tm_gmtoff is defined in sys/time.h, define this */
/* #undef HAVE_SYS_TM_GMTOFF */

/* In some cases, this isn't defined */
#ifndef socklen_t
typedef int socklen_t;
#endif