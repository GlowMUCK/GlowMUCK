#include "copyright.h"
#include "config.h"
#include "defaults.h"
#include "local.h"

#include "db.h"
#include "tune.h"
#include "props.h"
#include "interface.h"
#include "externs.h"

/* remove the first occurence of what in list headed by first */
dbref 
remove_first(dbref first, dbref what)
{
    dbref   prev;

    /* special case if it's the first one */
    if (first == what) {
	return DBFETCH(first)->next;
    } else {
	/* have to find it */
	DOLIST(prev, first) {
	    if (DBFETCH(prev)->next == what) {
		DBSTORE(prev, next, DBFETCH(what)->next);
		return first;
	    }
	}
	return first;
    }
}

int 
member(dbref thing, dbref list)
{
    DOLIST(list, list) {
	if (list == thing)
	    return 1;
	if ((DBFETCH(list)->contents)
		&& (member(thing, DBFETCH(list)->contents))) {
	    return 1;
	}
    }

    return 0;
}

dbref 
reverse(dbref list)
{
    dbref   newlist;
    dbref   rest;

    newlist = NOTHING;
    while (list != NOTHING) {
	rest = DBFETCH(list)->next;
	PUSH(list, newlist);
	DBDIRTY(newlist);
	list = rest;
    }
    return newlist;
}


int count_stats(dbref ref,
		int *rooms,	int *exits,	int *things,
		int *players,	int *programs,	int *garbage
) {
    dbref   i;

    if(!rooms || !exits || !things || !players || !programs || !garbage)
	return 0;

    (*rooms) = (*exits) = (*things) =
    (*players) = (*programs) = (*garbage) = 0;

    if((ref < NOTHING) || (ref >= db_top))
	return 0;

    for (i = 0; i < db_top; i++) {
	    if (ref == NOTHING || OWNER(i) == ref) {
		switch (Typeof(i)) {
		    case TYPE_ROOM:
			(*rooms)++;
			break;
		    case TYPE_EXIT:
			(*exits)++;
			break;
		    case TYPE_THING:
			(*things)++;
			break;
		    case TYPE_PLAYER:
			(*players)++;
			break;
		    case TYPE_PROGRAM:
			(*programs)++;
			break;
		    case TYPE_GARBAGE:
			(*garbage)++;
			break;
		}
	    }
    }
    return *rooms + *exits + *things + *players + *programs + *garbage;
}

void max_stats(int ref,
	int *maxrooms,  int *maxexits,
	int *maxthings, int *maxprograms
) {
    if(!OkObj(ref))
	return;

    if(!maxrooms || !maxexits || !maxthings || !maxprograms)
	return;

    (*maxrooms) = (*maxexits) = (*maxthings) = (*maxprograms) = 0;

    *maxrooms = get_property_value(ref, PROP_QUOTADIR "/rooms");
    if(!*maxrooms)
	*maxrooms = tp_max_rooms;
	
    *maxexits = get_property_value(ref, PROP_QUOTADIR "/exits");
    if(!*maxexits)
	*maxexits = tp_max_exits;
	
    *maxthings = get_property_value(ref, PROP_QUOTADIR "/things");
    if(!*maxthings)
	*maxthings = tp_max_things;
	
    *maxprograms = get_property_value(ref, PROP_QUOTADIR "/programs");
    if(!*maxprograms)
	*maxprograms = tp_max_programs;
	
}

/* Set people to have negative quota values to prevent them from getting stuff */

int quota_check(dbref player, dbref thing, int flags) {
    int maxrooms = 0, maxexits = 0, maxthings = 0, maxprograms = 0;
    int rooms = 0, exits = 0, things = 0, programs = 0, other = 0;
    
    max_stats(player, &maxrooms, &maxexits, &maxthings, &maxprograms);
    count_stats(player, &rooms, &exits, &things, &other, &programs, &other);

    if(!OkObj(player)) return 0;

    if(OkObj(thing))
	flags = Typeof(thing);

    switch(flags & TYPE_MASK) {
	case TYPE_ROOM:		return rooms < maxrooms;
	case TYPE_EXIT:		return exits < maxexits;
	case TYPE_THING:	return things < maxthings;
	case TYPE_PROGRAM:	return programs < maxprograms;
	default:		return 0;
    }
}

int
str2ip( const char *ipstr )
{
    int ip1, ip2, ip3, ip4;

    if(sscanf(ipstr, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4) != 4)
	return -1;

    if (ip1 < 0 || ip2 < 0 || ip3 < 0 || ip4 < 0) return -1;
    if (ip1>255 || ip2>255 || ip3>255 || ip4>255) return -1;

    return( (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4 );
    /* return( htonl((ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4) ); */
}

const char *
host_as_hex( /* CrT */ unsigned int addr ) {
    static char buf[32];

    sprintf(buf,
	"%d.%d.%d.%d",
	(addr >> 24) & 0xff,
	(addr >> 16) & 0xff,
	(addr >>  8) & 0xff,
	 addr	& 0xff
    );

    return buf;
}

