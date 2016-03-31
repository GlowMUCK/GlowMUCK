/* color.c Copyright 1998 by Andrew Nelson All Rights Reserved */

#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include "strings.h"
#include "color.h"
#include "db.h"
#include "interface.h"
#include "tune.h"
#include "props.h"
#include "match.h"
#include "msgparse.h"
#include "mpi.h"
#include "interp.h"
#include "reg.h"
#include "externs.h"

void
show_colorset(dbref player)
{
    char buf[BUFFER_LEN], buf2[80], buf3[40];
    int ai, aj, aval;
    const char *car[24] = {
	"BLACK", "CRIMSON", "FOREST", "BROWN", "NAVY", "VIOLET",
	"AQUA", "GRAY", "GLOOM", "RED", "GREEN", "YELLOW",
	"BLUE", "PURPLE", "CYAN", "WHITE", "BBLACK", "BRED",
	"BGREEN", "BBROWN", "BBLUE", "BPURPLE", "BCYAN", "BGRAY" };

    notify(player,	"Normal Color | Current Color" "      "
			"Normal Color | Current Color");

    for(ai=0; ai<24; ai+=2) {
	buf[0] = '\0';
	for(aj = 0; aj<2; aj++) {
	    if (aj>0) strcat(buf, "      ");
	    sprintf(buf3, "%s(%d)", car[ai+aj], ai+aj+1);
	    sprintf(buf2, "%12s", buf3);
	    strcat(buf, buf2);
	    strcat(buf, " | ");
	    aval = DBFETCH(player)->sp.player.ansi
		 ? DBFETCH(player)->sp.player.ansi[ai+aj]
		 : 0;
	    if ((aval<1)||(aval>24))
		aval = ai + aj;
	    else aval--;
	    sprintf(buf3, "%s(%d)", car[aval], aval+1);
	    sprintf(buf2, "%13s", buf3);
	    strcat(buf, buf2);
	}
	notify(player, buf);
   }
	notify(player,
"'@colorset save' saves the set, '@colorset load' reverts to last saved.");
	notify(player,
"'@colorset clear' reverts to defaults, @colorset oldval=newval changes a color.");
}

void
free_colorset(dbref player)
{
    if (DBFETCH(player)->sp.player.ansi)
	free(DBFETCH(player)->sp.player.ansi);

    DBFETCH(player)->sp.player.ansi = NULL;
}

void
load_colorset(dbref player)
{
    const char *am;
    char *an;
    int ai=0, aval;

    free_colorset(player);

    am = get_property_class(player, PROP_ANSI);
#ifdef COMPRESS
    if (am && *am) am = uncompress(am);
#endif
    if (am && *am) {

	/* Allocate a new color set */
	an = (char *) malloc(sizeof(char) * 24);
	if (an == NULL)
	    return; /* Can't load colorset, no memory */

	bzero((char *)an, sizeof(char) * 24);

	while((*am) && (ai<24)) {
	    while(*am == ' ') am++;
	    aval = atoi(am);
	    if ((aval < 0) || (aval > 24)) aval = 0;
	    an[ai] = aval;
	    while((*am) && (*am != ' ')) am++;
	    ai++;		
	}
	DBFETCH(player)->sp.player.ansi = (char *) an;
    }
}

void
save_colorset(dbref player)
{
    char buf[BUFFER_LEN], buf2[40];
    char an[24];
    int ai, top = 0;

    if (!DBFETCH(player)->sp.player.ansi) {
	remove_property(player, PROP_ANSI);
	return;
    }

    for(ai=0; ai<24; ai++) {
	an[ai] = DBFETCH(player)->sp.player.ansi[ai];
	if (an[ai]>0) top = ai + 1;
    }

    if (top == 0) {
	remove_property(player, PROP_ANSI);
	return;
    }

    buf[0] = buf[1] = '\0';
    for(ai = 0; ai<top; ai++) {
	sprintf(buf2, " %d", an[ai]);
	strcat(buf, buf2);
    }

    add_property(player, PROP_ANSI, buf+1, 0);
}

int
ignoring(dbref player, dbref jerk)
{
    int ai, ignores;
    dbref *ignoring;

    if (!tp_ignore_support)
	return 0;

    if ((!OkObj(player)) || (!OkObj(jerk)))
	return 0;

    player = OWNER(player);
    jerk = OWNER(jerk);

    ignoring = DBFETCH(player)->sp.player.ignoring;
    ignores = DBFETCH(player)->sp.player.ignores;

    if ((ignores <= 0) || (!ignoring))
	return 0; /* Not ignoring anyone */

    for(ai = 0; ai < ignores; ai++)
	if (ignoring[ai] == jerk) {
	    if (QLevel(jerk) >= tp_quell_ignore_mlevel)
		return 2;
	    else
		return 1;
	}

    return 0;
}

int
add_ignoring(dbref player, dbref jerk)
{
    int ai, ignores;
    dbref *ignoring, *ignew;

    if (!tp_ignore_support)
	return 0;

    if ((!OkObj(player)) || (!OkObj(jerk)) || (Typeof(player) != TYPE_PLAYER))
	return 0;

    player = OWNER(player);
    jerk = OWNER(jerk);

    if (!online(player)) /* Can only add if online, lest junk lists exist */
	return 0;

    if ((!OkObj(jerk)) || (Typeof(jerk) != TYPE_PLAYER))
	return 0; /* Invalid jerk */

    ignoring = DBFETCH(player)->sp.player.ignoring;
    ignores = DBFETCH(player)->sp.player.ignores;

    if ((ignores > 0) && (!ignoring))
	return 0; /* Bad list */

    for(ai = 0; ai < ignores; ai++)
	if (ignoring[ai] == jerk)
	    return 1; /* Already in list */

    ignew = (dbref *) malloc(sizeof(dbref) * (ignores + 1));

    if (!ignew)
	return 0; /* Couldn't alloc new list */

    for(ai = 0; ai < ignores; ai++)
	ignew[ai] = ignoring[ai];
    ignew[ignores] = jerk;

    free(ignoring); /* Dispose of old list */
    DBFETCH(player)->sp.player.ignoring = ignew;
    DBFETCH(player)->sp.player.ignores = ignores + 1;

    save_ignoring(player); /* Save prop on player */
    return 1;
}

int
remove_ignoring(dbref player, dbref jerk)
{
    int ai, ignores, an;
    dbref *ignoring, *ignew;

    if (!tp_ignore_support)
	return 0;

    if ((!OkObj(player)) || (jerk >= db_top) ||
	(Typeof(player) != TYPE_PLAYER)
    )
	return 0;

    if (jerk >= 0) /* Allow < 0 to clean out non-player ignorees */
	jerk = OWNER(jerk);

    ignoring = DBFETCH(player)->sp.player.ignoring;
    ignores = DBFETCH(player)->sp.player.ignores;

    if ((ignores > 0) && (!ignoring))
	return 0; /* Bad list */

    an = 0;
    for(ai = 0; ai < ignores; ai++) {
        if ((!OkObj(ignoring[ai])) ||
	   (Typeof(ignoring[ai]) != TYPE_PLAYER) ||
	   (ignoring[ai] == jerk) ||
	   (ignoring[ai] == player)
	) continue;
	ignoring[an++] = ignoring[ai];
    }

    if (an == ignores)
	return (jerk >= 0) ? 0 : 1; /* No changes needed, err if valid jerk */
    ignores = an;

    if (ignores > 0) {
	ignew = (dbref *) malloc(sizeof(dbref) * ignores);

	if (!ignew)
	    return 0; /* Couldn't alloc new list */

	for(ai = 0; ai < ignores; ai++)
	    ignew[ai] = ignoring[ai];
    } else {
	ignew = NULL;
    }

    free(ignoring); /* Dispose of old list */
    DBFETCH(player)->sp.player.ignoring = ignew;
    DBFETCH(player)->sp.player.ignores = ignores;

    save_ignoring(player); /* Save prop on player */
    return 1;
}

void
free_ignoring(dbref player)
{
    /* Always allow cleanup */

    if ((!OkObj(player)) || (Typeof(player) != TYPE_PLAYER))
	return;

    if (DBFETCH(player)->sp.player.ignoring)
	free(DBFETCH(player)->sp.player.ignoring);

    DBFETCH(player)->sp.player.ignoring = NULL;
    DBFETCH(player)->sp.player.ignores = 0;
}

void
load_ignoring(dbref player)
{
    const char *am;
    dbref *ignoring;
    int ai, ignores;

    if (!tp_ignore_support)
	return;

    if ((!OkObj(player)) || (Typeof(player) != TYPE_PLAYER))
	return;

    free_ignoring(player); /* Clears ignoring and sets ignores to 0 */

    am = get_property_class(player, PROP_IGNORING);
#ifdef COMPRESS
    if (am && *am) am = uncompress(am);
#endif
    if (am && *am && ((ignores = atoi(am)) > 0)) {
	/* ignores should be the lesser of IGNOREMAX and tp_max_ignores */

	if (ignores > IGNOREMAX) /* Hard-coded sanity limit */
	    ignores = IGNOREMAX;

	if (ignores > tp_max_ignores) /* Lesser amount for small mucks */
	    ignores = tp_max_ignores;

	if (ignores <= 0)
	    return;

	/* Allocate a new ignoring list */
	ignoring = (dbref *) malloc(sizeof(dbref) * ignores);
	if (ignoring == NULL)
	    return; /* Can't load ignoring, no memory */

	while((*am >= '0') && (*am <= '9'))
	    am++; /* Skip count value */

	for(ai = 0; ai < ignores; ai++) {
	    while(*am && ((*am < '0') || (*am > '9')))
		am++;
	    ignoring[ai] = *am ? atoi(am) : NOTHING;
	    while((*am >= '0') && (*am <= '9'))
		am++; /* Skip value */
	}

	DBFETCH(player)->sp.player.ignores = ignores;
	DBFETCH(player)->sp.player.ignoring = ignoring;

	remove_ignoring(player, -1); /* Clean out any @toads */
    }
}

void
save_ignoring(dbref player)
{
    char buf[BUFFER_LEN], buf2[40];
    const dbref *ignoring;
    int ai, ignores, left;

    if (!tp_ignore_support)
	return;

    if ((!OkObj(player)) || (Typeof(player) != TYPE_PLAYER))
	return;

    ignores = DBFETCH(player)->sp.player.ignores;
    ignoring = DBFETCH(player)->sp.player.ignoring;

    /* ignores should be the lesser of IGNOREMAX and tp_max_ignores */
    if (ignores > IGNOREMAX) /* Hard-coded sanity limit */
	ignores = IGNOREMAX;

    if (ignores > tp_max_ignores) /* Lesser amount for small mucks */
	ignores = tp_max_ignores;

    if ((!ignoring) || (ignores <= 0)) {
	remove_property(player, PROP_IGNORING);
	return;
    }

    sprintf(buf, "%d", ignores); /* First # is count */
    left = BUFFER_LEN - 2;
    for(ai = 0; ai < ignores; ai++) {
        if (left <= 40) break;
	sprintf(buf2, ",%d", ignoring[ai]);
        left -= strlen(buf2);
        strcat(buf, buf2);
    }

    add_property(player, PROP_IGNORING, buf, 0);
}

void
set_color(dbref player, int oldColor, int newColor)
{
    if ((oldColor<1)||(oldColor>24)) {
	anotify(player, CFAIL "Old color is not in range. (1-24)");
	return;
    }

    if ((newColor<0)||(newColor>24)) {
	anotify(player, CFAIL "New color is not in range. (0-24, 0=default)");
	return;
    }

    if (oldColor == newColor) newColor = 0;

    if (!online(player))
	return;

    if (!DBFETCH(player)->sp.player.ansi) {
	char *an;

	/* Allocate a new color set */
	an = (char *) malloc(sizeof(char) * 24);
	if (an == NULL)
	    return; /* Can't load colorset, no memory */

	bzero((char *)an, sizeof(char) * 24);
	DBFETCH(player)->sp.player.ansi = an;
    }

    DBFETCH(player)->sp.player.ansi[oldColor-1] = newColor;
    anotify(player, CINFO "Color changed.  Don't forget to save your changes.");
}

void
do_colorset(dbref player, const char *oldColor, const char *newColor)
{
    if (!OkObj(player)) return;
    if (Typeof(player) != TYPE_PLAYER ) return;

    if (Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((!newColor)||(!*newColor)) {
	if ((oldColor)&&(*oldColor)) {
	    if (oldColor[0]=='s'||oldColor[0]=='S') {
		save_colorset(player);
		anotify(player, CINFO "Color set saved.");
	    } else if (oldColor[0]=='l'||oldColor[0]=='L') {
		load_colorset(player);
		anotify(player, CINFO "Color set loaded.");
	    } else if (oldColor[0]=='c'||oldColor[0]=='C') {
		free_colorset(player);
		save_colorset(player);
		anotify(player, CINFO "Color set reset to default values.");
	    } else show_colorset(player);
	} else show_colorset(player);
    } else set_color(player, atoi(oldColor), atoi(newColor));
}

const char *
anco(const char *an, int ani, const char *defcol)
{
    const char *car[24] = {
	ANSIBLACK, ANSICRIMSON, ANSIFOREST, ANSIBROWN, ANSINAVY, ANSIVIOLET,
	ANSIAQUA, ANSIGRAY, ANSIGLOOM, ANSIRED, ANSIGREEN, ANSIYELLOW,
	ANSIBLUE, ANSIPURPLE, ANSICYAN, ANSIWHITE, ANSIBBLACK, ANSIBRED,
	ANSIBGREEN, ANSIBBROWN, ANSIBBLUE, ANSIBPURPLE, ANSIBCYAN, ANSIBGRAY
    };

    /* 'an' can be NULL up to this function, no checking is ever done on it */

    if ((!an) || (an[ani]<=0) || (an[ani]>24))
	return defcol;
    else
	return car[an[ani]-1];
}

const char *
color_lookup(const char *color, const char *an, const int allow_256, char *buf)
{
  int color_num = 0;
  int red = 0;
  int green = 0;
  int blue = 0;
  char *tmp = NULL;

  if ((!color) || (!*color) )
    return ANSINORMAL;

  /* Do wrapper lookups */

  if (!strcasecmp( "SUCC", color )) {
    color = CCSUCC;
  } else if (!strcasecmp( "FAIL", color )) {
    color = CCFAIL;
  } else if (!strcasecmp( "INFO", color )) {
    color = CCINFO;
  } else if (!strcasecmp( "NOTE", color )) {
    color = CCNOTE;
  } else if (!strcasecmp( "MOVE", color )) {
    color = CCMOVE;
  } else if (strncasecmp("256", color, 3) == 0) {
    if (allow_256) {
      if (strncasecmp("G", color+6, 1) == 0) {
	strcpy(buf, color);
	tmp = strrchr(buf, ':');
	if (tmp != NULL) {
	  tmp[0] = '\0';
	}
	color_num = atoi(color+7);
	if (color_num < 0 || color_num > 24) {
	  return ANSINORMAL;
	} else {
	  color_num = color_num + 231;
	}
      } else {
	red = color[6] - '0';
	green = color[7] - '0';
	blue = color[8] - '0';
	if (red < 0 || red > 5 ||
	    green < 0 || green > 5 ||
	    blue < 0 || blue > 5) {
	  return ANSINORMAL;
	}
	color_num = 16 + (red * 36) + (green * 6) + blue;
      }
      if (strncasecmp("256BG-", color, 6) == 0) {
	sprintf(buf, ANSI256BACK"%0dm", color_num);
	return buf;
      }
      if (strncasecmp("256FG-", color, 6) == 0) {
	sprintf(buf, ANSI256FORE"%0dm", color_num);
	return buf;
      }
      return ANSINORMAL;
    } else {
      /* Handle Alternate Color, because the user can't handle 256 colors */
        const char* temp = strrchr(color, ':');
      if (temp == NULL || temp+1 == NULL) {
	return ANSINORMAL;
      } else {
	color = temp+1;
      }
    }
  }
  
  if (!strcasecmp( "NORMAL", color )) {
    return ANSINORMAL;
  } else if (!strcasecmp( "CLS", color )) {
    return ANSICLS;
  } else if (!strcasecmp( "BEEP", color ) ||
	     !strcasecmp( "BELL", color )) {
    return ANSIBEEP;
  } else if (!strcasecmp( "FLASH", color ) ||
	     !strcasecmp( "BLINK", color )) {
    return ANSIFLASH;
  } else if (!strcasecmp( "INVERT", color ) ||
	     !strcasecmp( "REVERSE", color ) ) {
    return ANSIINVERT;
  } else if (!strcasecmp( "BLACK", color )) {
    return anco(an, 0, ANSIBLACK);
  } else if (!strcasecmp( "CRIMSON", color )) {
    return anco(an, 1, ANSICRIMSON);
  } else if (!strcasecmp( "FOREST", color )) {
    return anco(an, 2, ANSIFOREST);
  } else if (!strcasecmp( "BROWN", color )) {
    return anco(an, 3, ANSIBROWN);
  } else if (!strcasecmp( "NAVY", color )) {
    return anco(an, 4, ANSINAVY);
  } else if (!strcasecmp( "VIOLET", color )) {
    return anco(an, 5, ANSIVIOLET);
  } else if (!strcasecmp( "AQUA", color )) {
    return anco(an, 6, ANSIAQUA);
  } else if (!strcasecmp( "GRAY", color )) {
    return anco(an, 7, ANSIGRAY);
  } else if (!strcasecmp( "GLOOM", color )) {
    return anco(an, 8, ANSIGLOOM);
  } else if (!strcasecmp( "RED", color )) {
    return anco(an, 9, ANSIRED);
  } else if (!strcasecmp( "GREEN", color )) {
    return anco(an, 10, ANSIGREEN);
  } else if (!strcasecmp( "YELLOW", color )) {
    return anco(an, 11, ANSIYELLOW);
  } else if (!strcasecmp( "BLUE", color )) {
    return anco(an, 12, ANSIBLUE);
  } else if (!strcasecmp( "PURPLE", color )) {
    return anco(an, 13, ANSIPURPLE);
  } else if (!strcasecmp( "CYAN", color )) {
    return anco(an, 14, ANSICYAN);
  } else if (!strcasecmp( "WHITE", color )) {
    return anco(an, 15, ANSIWHITE);
  } else if (!strcasecmp( "BBLACK", color )) {
    return anco(an, 16, ANSIBBLACK);
  } else if (!strcasecmp( "BRED", color )) {
    return anco(an, 17, ANSIBRED);
  } else if (!strcasecmp( "BGREEN", color )) {
    return anco(an, 18, ANSIBGREEN);
  } else if (!strcasecmp( "BBROWN", color )) {
    return anco(an, 19, ANSIBBROWN);
  } else if (!strcasecmp( "BBLUE", color )) {
    return anco(an, 20, ANSIBBLUE);
  } else if (!strcasecmp( "BPURPLE", color )) {
    return anco(an, 21, ANSIBPURPLE);
  } else if (!strcasecmp( "BCYAN", color )) {
    return anco(an, 22, ANSIBCYAN);
  } else if (!strcasecmp( "BGRAY", color )) {
    return anco(an, 23, ANSIBGRAY);
  } else {
    return ANSINORMAL;
  }
}

/* ParseAnsi values:
   0 = no parsing, black&white
   1 = muf parsing, base on glow_ansi and tilde_ansi @tunes
   2 = server parsing, base on server_ansi
*/

char *
parse_ansi( char *buf, const char *from, const char *an, int parseansi, int allow_256 )
{
    int isbold = 0;
    char *to, *color, cbuf[BUFFER_LEN + 2];
    const char *ansi;
    char newansi[30];

    if (parseansi <= 0) {
	strcpy(buf, from);
	return buf;
    }

    to=buf;
    while(*from && (((to - buf) + 1) < BUFFER_LEN)) {
	/* Append the ANSINORMAL string if at end of line embedded */
	if (((*from) == '\r') || ((*from) == '\n')) {

	    if ((to - buf) + strlen(ANSINORMAL) < BUFFER_LEN)
		strcpy(to, ANSINORMAL);
	    else break;
	    while(*to) to++;

	    /* Copy all the line feeds */
	    while( (((to - buf) + 1) < BUFFER_LEN) &&
		(((*from) == '\r') || ((*from) == '\n'))
	    ) (*(to++)) = (*(from++));

	    if (!*from) {
		/* If end is linefeeds, don't append ANSINORMAL */
		*to = '\0';
		return buf;
	    }
	}

	if (((parseansi == 2) || tp_glow_ansi) && (*from == '^')) { /* Glow/Neon ANSI color */
	    from++;
	    color = cbuf;
	    while(*from && *from != '^')
		*(color++) = (*(from++));
	    *color = '\0';
	    if (!*from) break;
	    if (*from) from++;
	    if (*cbuf) {
              ansi = color_lookup(cbuf, an, allow_256, newansi);
	      if (ansi == NULL) {
		ansi = (const char *) newansi;
	      }
	      if (ansi && (((to - buf) + strlen(ansi)) < BUFFER_LEN) ) {
		while(*ansi) {
		  *(to++) = (*(ansi++));
		}
	      }
	    } else
		*(to++) = '^'; /* Escape ^^ -> ^ */
        } else if (((parseansi == 1) && tp_tilde_ansi) &&
                  (*from == '~') && (*(from+1) == '&')
	) { /* lib-ansi color */
	    from+=2;
	    if (!*from) break;

	    if ((*(from) == '~') && (*(from+1) == '&')) {
		/* Escape ~&~& -> ~& */
		ansi = "~&";
		if (((to - buf) + strlen(ansi)) < BUFFER_LEN )
		    while(*ansi) *(to++) = (*(ansi++));
		from += 2;
		if (!*from) break;
	    } else if (TildeAnsiDigit(*from)) {
		char attr;

		/* ~&000 pattern */

		if ((!from[1]) || (!from[2]) ||
		   (!TildeAnsiDigit(from[1])) || (!TildeAnsiDigit(from[2]))
		) continue;

		/* First position, set bold now, save others for later */
		attr = *from;
		isbold = (attr == '1') ? 1 : 0;
		from++;
		if (!*from) break;

		/* second position, foreground color */
		ansi = NULL;
		switch(*from) {
		    case '0': ansi = isbold ? "GLOOM"  : "BLACK";   break;
		    case '1': ansi = isbold ? "RED"    : "CRIMSON"; break;
		    case '2': ansi = isbold ? "GREEN"  : "FOREST";  break;
		    case '3': ansi = isbold ? "YELLOW" : "BROWN";   break;
		    case '4': ansi = isbold ? "BLUE"   : "NAVY";    break;
		    case '5': ansi = isbold ? "PURPLE" : "VIOLET";  break;
		    case '6': ansi = isbold ? "CYAN"   : "AQUA";    break;
		    case '7': ansi = isbold ? "WHITE"  : "GRAY";    break;
		    case '-':					    break;
		}
		if (ansi) {
		  ansi = color_lookup(ansi, an, allow_256, newansi);
		  if (ansi == NULL) {
		    ansi = newansi;
		  }
		  if (ansi && (((to - buf) + strlen(ansi)) < BUFFER_LEN) ) {
		    while(*ansi) *(to++) = (*(ansi++));
		  }
		}

		/* now handle others in first position, which is attributes */
		ansi = NULL;
		switch(attr) {
		    case '2': /* These both set reverse */
		    case '8':
			ansi = "INVERT"; break;

		    case '5': /* set for BLINK foreground */
			ansi = "FLASH"; break;

		    case '-': /* keep the same */
			break;
		}
		if (ansi) {
		  ansi = color_lookup(ansi, an, allow_256, newansi);
		  if (ansi == NULL) {
		    ansi = newansi;
		  }
		  if (ansi && (((to - buf) + strlen(ansi)) < BUFFER_LEN) ) {
		    while(*ansi) *(to++) = (*(ansi++));
		  }
		}
		from++;
		if (!*from) break;

		/* third and last position, background color */
		ansi = NULL;
		switch(*from) {
		    case '0': ansi = "BBLACK";	break;
		    case '1': ansi = "BRED";	break;
		    case '2': ansi = "BGREEN";	break;
		    case '3': ansi = "BBROWN";	break;
		    case '4': ansi = "BBLUE";	break;
		    case '5': ansi = "BPURPLE";	break;
		    case '6': ansi = "BCYAN";	break;
		    case '7': ansi = "BGRAY";	break;
		    case '-':			break;
		}
                if (ansi) {
		  ansi = color_lookup(ansi, an, allow_256, newansi);
		  if (ansi == NULL) {
		    ansi = newansi;
		  }
		  if (ansi && (((to - buf) + strlen(ansi)) < BUFFER_LEN) ) {
		    while(*ansi) *(to++) = (*(ansi++));
		  }
		}
		from++;
		if (!*from) break;

	    } else {
		/* ~&X pattern */

		/* Single-letter attributes */
		ansi = NULL;
		switch(*from) {
		    case 'r': /* RESET to normal colors. */
		    case 'R':
			ansi = "NORMAL"; break;

		    case 'c': /* this used to clear the screen, its retained */
		    case 'C': /* for parsing only, doesnt actually do it.    */
			ansi = "CLS"; break;

		    case 'b': /* this is for BELL.. or CTRL-G */
		    case 'B':
			ansi = "BEEP"; break;
		}
		if (ansi) {
		  ansi = color_lookup(ansi, an, allow_256, newansi);
		  if (ansi == NULL) {
		    ansi = newansi;
		  }
		  if (ansi && (((to - buf) + strlen(ansi)) < BUFFER_LEN) ) {
		    while(*ansi) *(to++) = (*(ansi++));
		  }
		}
		from++;
		if (!*from) break;
	    }
	} else *(to++) = (*(from++));
    }
    *to='\0';

    if ((to - buf) + strlen(ANSINORMAL) < BUFFER_LEN)
	strcpy(to, ANSINORMAL);

    return buf;
}

/* tct pads '^' characters with an extra '^' which will be changed back to
   one total '^' from in-server notify()s.  Since only ^^ codes are used
   inserver, we don't do tilde ansi sequences.
*/

char *
tct( const char *in, char out[BUFFER_LEN] )
{
    char *p=out;
    if (!out) perror("tct: Null buffer");

    if (in && (*in)) {
	while ( *in && ((p - out) < (BUFFER_LEN-3)) ) {
	    if ( (*(p++) = (*(in++))) == '^') {
		*(p++) = '^'; /* Glow ANSI escape sequence */
#if 0
	    } else if ( (*(p-2) == '~') && (*(p-1) == '&')) {
		*(p++) = '~'; /* Tilde ANSI escape sequence */
		*(p++) = '&';
#endif
	    }
	}
    }
    *p = '\0';
    return out;
}

int
ansi_striplen( const char *word, int parseansi )
{
    const char *from;

    if (parseansi <= 0)
	return 0;

    from = word;
    if (((parseansi == 2) || tp_glow_ansi) && (*from == '^')) {
	from++;
	if (*from == '^') {
	    return 2; /* Eat both ^^s, lest ^ be reparsed later */
	} else while(*from) {
	    if (*from == '^') { from++; break; }
	    from++;
	}
    } else if (((parseansi == 1) && tp_tilde_ansi) &&
	      (*from == '~') && (*(from+1) == '&') &&
	      (*(from + 2) == '~') && (*(from+3) == '&')
    ) {
	return 4;
    } else if (((parseansi == 1) && tp_tilde_ansi) &&
	      (*from == '~') && (*(from+1) == '&')
    ) {
	from += 2;
	if (*from) {
	    if (TildeAnsiDigit(*from)) {
		if (from[1] && from[2] &&
		   TildeAnsiDigit(from[1]) &&
		   TildeAnsiDigit(from[2])
		) from += 3;
	    } else from++; /* eat single-letter */
	}
    }

    return from - word;
}

int
ansi_offset(const char *from, int offset, int mode)
{
    int cooked = 0;
    int raw = 0;
    unsigned int count;
    
    for(cooked = 0, raw = 0; (mode ? cooked : raw) < offset; raw++) {
	if (!from[raw]) {
	    break; /* past end of string */
	}

	if (tp_glow_ansi && (from[raw] == '^') && (from[raw+1] == '^')) {
	    /* Special case for ^^ -> ^ */
	    raw++; /* Skip 1 extra char */
	    cooked++; /* Add 1 letter */
	} else if (
	    (tp_tilde_ansi && (from[raw] == '~') && (from[raw+1] == '&')) &&
	    (from[raw+2] == '~') && (from[raw+3] == '&')
	) {
	    /* Special case for ~&~& -> ~& */
	    raw+=3; /* Skip 3 extra chars */
	    cooked+=2; /* Add 2 letters */
	} else if (
	    (tp_glow_ansi  && (from[raw] == '^')) ||
	    (tp_tilde_ansi && (from[raw] == '~') && (from[raw+1] == '&'))
	) {
	    count = (unsigned int)ansi_striplen(&from[raw], 1);
	    if ((count > 0) && (count <= strlen(&from[raw]))) {
		raw += count - 1;
	    } else {
		log_status("BUGO: ansi_offset %d out of range on '%s'\n", count, &from[raw]);
		break;
	    }
	} else cooked++;
    }

    return mode ? raw : cooked; /* raw holds position in undecoded ansi string */
}

char *
unparse_ansi( char *buf, const char *from, int parseansi )
{
    int count;
    char *to;

    if (parseansi <= 0) {
	strcpy(buf, from);
	return buf;
    }

    to=buf;
    while(*from) {
	if (((parseansi == 2) || tp_glow_ansi) && (*from == '^') && (*(from+1) == '^')) {
	    /* Special case for ^^ -> ^ */
	    from++;
	    *(to++) = (*(from++));
	} else if (
		((parseansi == 1) && tp_tilde_ansi) &&
		(*(from+0) == '~') && (*(from+1) == '&') &&
		(*(from+2) == '~') && (*(from+3) == '&')
	) {
	    /* Special case for ~&~& -> ~& */
	    from += 2;
	    *(to++) = (*(from++));
	    *(to++) = (*(from++));
	} else if (
		  (((parseansi == 2) || tp_glow_ansi) && (*from == '^')) ||
		  (((parseansi == 1) && tp_tilde_ansi) && (*from == '~') && (*(from+1) == '&'))
	) {
	    count = ansi_striplen(from, parseansi);
	    if ((count > 0) && (count <= strlen(from))) {
		from += count;
	    } else {
		log_status("BUGO: ansi_striplen %d out of range on '%s'\n", count, from);
		break;
	    }
	} else *(to++) = (*(from++));
    }
    *to='\0';
    return buf;
}

char *
ansi( char *buf, const char *from, const char *an, int docolor, int parseansi, int allow_256 )
{
  *buf = '\0';
  
  if (!*from)
    return buf;
  
  if (docolor == 0) {
    return unparse_ansi(buf, from, parseansi);
  } else {
    return parse_ansi(buf, from, an, parseansi, allow_256);
  }

}
