/*
	Local Glowmuck/Fuzzball changes file

	Put any muck-specific #define changes in this file instead of
	modifying other header files.

	Changes in this file will override anything in:
		config.h
		params.h
		defaults.h

	Preclude any such #defines by first #undefining the entity.  If you
	don't first #undef the entity, you will get compiler errors.  No
	'real' defines will be put in this file, you may safely overwrite it
	with your local changes.
	
	Ex: Say MPI_SUCKS is normally defined to "MPI sucks" in defaults.h
	    and you want to change it...
*/

#ifdef MPI_SUCKS
    #undef MPI_SUCKS
#endif
#define MPI_SUCKS "I hate MPI!"

/* Not that I have anything against MPI. O:) -Andy */

