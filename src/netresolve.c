#include "copyright.h"
#include "config.h"
#include "defaults.h"
#include "local.h"

#include <time.h>

#include "color.h"
#include "db.h"
#include "interface.h"
#include "tune.h"
#include "externs.h"


#define MALLOC(result, type, number) do {   \
				       if (!((result) = (type *) malloc ((number) * sizeof (type)))) \
				       abort();			     \
				     } while (0)

#define FREE(x) (free((void *) x))


struct hostcache {
    struct hostcache *next;
    struct hostcache **prev;
    int ipnum;
    time_t lastreq;
    time_t lastaccept;
    unsigned short accepts;
    char name[64];
} *hostcache_list = NULL;


void dump_site(dbref player, struct hostcache *ptr) {
    time_t now;
    now = current_systime;
    anotify_fmt(player, CBROWN "%s %08X  LMA: %d  LA: %d:%02d  LR: %d:%02d  Host: %s",
	host_as_hex(ptr->ipnum),
	ptr->ipnum,
	ptr->accepts,
	MIN((now - ptr->lastaccept) / 60, 9999),
	(now - ptr->lastaccept) % 60,
	MIN((now - ptr->lastreq) / 60, 9999),
	(now - ptr->lastreq) % 60,
	ptr->name
    );
}

void host_list(dbref player, const char *name) {
    struct hostcache *ptr;
    int i;
    char buf[BUFFER_LEN], buf2[BUFFER_LEN];

    strcpy(buf, name);

    for (i = 0, ptr = hostcache_list; ptr; ptr = ptr->next) {
	if(!*name) {
	    i++; /* Just count entries */
	} else if(
	    equalstr(buf, strcpy(buf2, ptr->name)) ||
	    equalstr(buf, strcpy(buf2, host_as_hex(ptr->ipnum)))
	) {
	    dump_site(player, ptr);
	    i++;
	}
    }
    anotify_fmt(player, CINFO "%d entr%s %s.",
	i, (i == 1) ? "y" : "ies", (*name) ? "listed" : "in cache"
    );
}

void
do_dos(dbref player, const char *arg1, const char *arg2) {
    struct hostcache *ptr;

    if(!Wiz(OWNER(player))) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    anotify(player, CINFO "Current Denial of Service activity:");
    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if ((tp_max_site_lma > 1) && (ptr->accepts >= tp_max_site_lma))
	    dump_site(player, ptr);
    }
    anotify(player, CINFO "Done.");
}


int
host_del(int ip)
{
    struct hostcache *ptr;
    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if (ptr->ipnum == ip) {
	    if (ptr->next) {
		ptr->next->prev = ptr->prev;
	    }
	    *ptr->prev = ptr->next;
	    FREE(ptr);
	    return TRUE;
	}
    }
    
    return FALSE;
}

char *
host_fetch(int ip)
{
    struct hostcache *ptr;

    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if (ptr->ipnum == ip) {
	    if (ptr != hostcache_list) {
		*ptr->prev = ptr->next;
		if (ptr->next) {
		    ptr->next->prev = ptr->prev;
		}
		ptr->next = hostcache_list;
		if (ptr->next) {
		    ptr->next->prev = &ptr->next;
		}
		ptr->prev = &hostcache_list;
		hostcache_list = ptr;
	    }
	    return (ptr->name);
	}
    }
    return NULL;
}

void
host_touch_lastreq(int ip)
{
    struct hostcache *ptr;

    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if (ptr->ipnum == ip) {
	    time(&(ptr->lastreq));
	    return;
	}
    }
    return;
}

int
host_touch_last_min_accepts(int ip)
{
    struct hostcache *ptr;
    time_t now;

    now = current_systime;

    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if (ptr->ipnum == ip) {
	    /* If no attempts in past minute, start over */
	    if ((now - ptr->lastaccept) > 60) {
		ptr->lastaccept = now;
		ptr->accepts = 1;
	    } else {
		/* If we hit more than 75% of max_site_lma, restart the minute */
		if((tp_max_site_lma > 1) && (ptr->accepts > ((tp_max_site_lma*3)/4)))
		    ptr->lastaccept = now;
		ptr->accepts++;
		/* Prevent rollover, but allow monitoring of speed with dinfo */
		if(ptr->accepts > 20000)
		    ptr->accepts = ((tp_max_site_lma > 1) ? tp_max_site_lma : 1) + 1;
	    }
	    return ptr->accepts;
	}
    }
    return 0;
}

time_t
host_lastreq(int ip)
{
    struct hostcache *ptr;

    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if (ptr->ipnum == ip)
	    return ptr->lastreq;
    }
    return (time_t) 0;
}

time_t
domain_lastreq(int ip)
{
    time_t last = (time_t) 0;
    struct hostcache *ptr;

    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if ((ptr->ipnum>>16) == (ip>>16)) {
	    if(last < ptr->lastreq) last = ptr->lastreq;
	}
    }
    return last;
}

int
host_last_min_accepts(int ip)
{
    struct hostcache *ptr;

    for (ptr = hostcache_list; ptr; ptr = ptr->next) {
	if (ptr->ipnum == ip)
	    return (int) ptr->accepts;
    }
    return 0;
}

void
host_add(int ip, const char *name, time_t last)
{
    struct hostcache *ptr;

    MALLOC(ptr, struct hostcache, 1);
    ptr->next = hostcache_list;
    if (ptr->next) {
	ptr->next->prev = &ptr->next;
    }
    ptr->prev = &hostcache_list;
    hostcache_list = ptr;
    ptr->ipnum = ip;
    ptr->lastreq = last;
    ptr->lastaccept = last;
    ptr->accepts = 0;
    
    
    strncpy(ptr->name, name, 63);
    ptr->name[63] = '\0';
}

void
host_free(void)
{
    struct hostcache *next, *list;

    if( !hostcache_list ) return;

    list = hostcache_list;
    hostcache_list = NULL;

    while( list ) {
	next = list->next;
	FREE( list );
	list = next;
    }
}

void
host_load(void)
{
    FILE *f;
    int ip;
    char name[80];
    char *p = name;
    time_t last = (time_t) 0;
    int tmp;

    if(!( f = fopen( "nethost.cache", "rb" ))) return;

    host_free();

    while( fscanf( f, "%x %s %d\n", &ip, p,  &tmp ) == 3 ) {
      last = (time_t) tmp;
      host_add(ip, name, last);
    }

    fclose( f );
}

void
host_save()
{
    FILE *f;
    struct hostcache *ptr;

    if(!( f = fopen( "nethost.cache", "wb" ))) return;

    for (ptr = hostcache_list; ptr; ptr = ptr->next)
	fprintf( f, "%X %s %d\n", ptr->ipnum, ptr->name, (int) ptr->lastreq );

    fclose( f );
}

void
host_init()
{
    host_load();
}

void
host_shutdown()
{
    host_save();
    host_free();
}
