/* MPI msgparse.c header file. */

#define MPI_ISPUBLIC	0x00  /* never test for this one */
#define MPI_ISPRIVATE	0x01
#define MPI_ISLISTENER	0x02
#define MPI_ISLOCK	0x04
#define MPI_ISDEBUG	0x08

#ifdef MPI

extern char *
mesg_parse(dbref player, dbref what, dbref perms,
            const char *inbuf, char *outbuf,
            int maxchars, int mesgtyp);

#endif /* MPI */

extern char *
do_parse_mesg(dbref player, dbref what, const char *inbuf,
	    const char *abuf, char *outbuf, int mesgtyp);

extern char *
do_parse_perms(dbref player, dbref what, dbref perms, const char *inbuf,
	    const char *abuf, char *outbuf, int mesgtyp);
