#include "copyright.h"
#include "config.h"
#include "params.h"
#include "local.h"

#include "color.h"
#include "db.h"
#include "tune.h"
#include "interface.h"
#include "props.h"
#include "externs.h"

static char upb[BUFFER_LEN];

const char *
unparse_flags(dbref thing, char buf[BUFFER_LEN])
{
    char   *p;
    const char *type_codes = "R-EPFG";

    p = buf;
    if (Typeof(thing) != TYPE_THING)
	*p++ = type_codes[Typeof(thing)];
    if (FLAGS(thing) & ~TYPE_MASK) {
	/* print flags */
	if (FLAGS(thing) & BUILDER)
	    *p++ = 'B';
	if (FLAGS(thing) & JUMP_OK)
	    *p++ = 'J';
	if (FLAGS(thing) & LINK_OK)
	    *p++ = 'L';
	if (FLAGS(thing) & KILL_OK)
	    *p++ = 'K';
	if (FLAGS(thing) & DARK)
	    *p++ = 'D';
	if (FLAGS(thing) & STICKY)
	    *p++ = 'S';
	if (FLAGS(thing) & QUELL)
	    *p++ = 'Q';
	if (FLAGS(thing) & CHOWN_OK)
	    *p++ = 'C';
	if (FLAGS(thing) & HAVEN)
	    *p++ = 'H';
	if (FLAGS(thing) & ABODE)
	    *p++ = 'A';
	if (FLAGS(thing) & VEHICLE)
	    *p++ = 'V';
	if (FLAGS(thing) & XFORCIBLE)
	    *p++ = 'X';
	if (FLAGS(thing) & ZOMBIE)
	    *p++ = 'Z';
    }
    if (FLAG2(thing)) {
	if (FLAG2(thing) & F2MPI)
	    *p++ = 'M';
	if (FLAG2(thing) & F2TINKERPROOF)
	    *p++ = 'T';
	if (FLAG2(thing) & F2GUEST)
	    *p++ = 'G';
	if (FLAG2(thing) & F2IC)
	    *p++ = 'I';
	if (FLAG2(thing) & F2OFFER)
	    *p++ = 'O';
	if (FLAG2(thing) & F2PUEBLO)
	    *p++ = 'U';
    }

    if(tp_multi_wiz_levels) {
	switch(RawMLevel(thing)) {
	    case LMAGE:	*p++ = 'W';		break;
	    case LWIZ:	*p++ = 'W'; *p++ = '2'; break;
	    case LARCH:	*p++ = 'W'; *p++ = '3'; break;
	    case LBOY:	*p++ = 'W'; *p++ = '4'; break;
	}
    } else if(FLAGS(thing)&W3)
	*p++ = 'W';

    switch (RawMLevel(thing)) {
	case LM1:	*p++ = 'M'; *p++ = '1'; break;
	case LM2:	*p++ = 'M'; *p++ = '2'; break;
	case LM3:	*p++ = 'M'; *p++ = '3'; break;
    }

    *p = '\0';
    return buf;
}

const char *
unparse_object(dbref player, dbref loc)
{
    char tbuf[BUFFER_LEN];

    if (Typeof(player) != TYPE_PLAYER)
	player = OWNER(player);
    switch (loc) {
	case NOTHING:
	    return "*NOTHING*";
	case AMBIGUOUS:
	    return "*AMBIGUOUS*";
	case HOME:
	    return "*HOME*";
	default:
	    if (!OkObj(loc))
#ifdef SANITY
	    {
		sprintf(upb, "*INVALID*(#%d)", loc);
		return upb;
	    }
#else
		return "*INVALID*";
#endif
#ifndef SANITY
	    if (!(FLAGS(player) & STICKY) &&
		    (TMage(player) ||
		     can_link_to(player, NOTYPE, loc) ||
		     controls_link(player, loc) ||
		     ((Typeof(loc) != TYPE_PLAYER) &&
		     (FLAGS(loc) & CHOWN_OK))
		    )) {
		/* show everything */
#endif
		sprintf(upb, "%s(#%d%s)", NAME(loc), loc, unparse_flags(loc, tbuf));
		return upb;
#ifndef SANITY
	    } else {
		/* show only the name */
		return NAME(loc);
	    }
#endif
    }
}


const char *
ansiname(dbref loc, char buf[BUFFER_LEN])
{
    char tbuf[BUFFER_LEN];

    *buf = '\0';
    switch(Typeof(loc)) {
	case TYPE_PLAYER:
	    strcpy( buf, CGREEN ); break;
	case TYPE_THING:
	    strcpy( buf, CPURPLE ); break;
	case TYPE_EXIT:
	    strcpy( buf, CBLUE ); break;
	case TYPE_PROGRAM:
	    strcpy( buf, CRED ); break;
	case TYPE_ROOM:
	    strcpy( buf, CCYAN ); break;
	default:
	    strcpy( buf, CNORMAL );
    }

    strcat( buf, tct(NAME(loc),tbuf) );
    return buf;
}

const char *
ansi_unparse_object(dbref player, dbref loc)
{
    char tbuf[BUFFER_LEN], tbuf2[BUFFER_LEN];

    if (Typeof(player) != TYPE_PLAYER)
	player = OWNER(player);
    switch (loc) {
	case NOTHING:
	    return CGLOOM "*NOTHING*";
	case AMBIGUOUS:
	    return CPURPLE "*AMBIGUOUS*";
	case HOME:
	    return CWHITE "*HOME*";
	default:
	    if (!OkObj(loc))
		return CRED "*INVALID*";
#ifndef SANITY
	    if (!(FLAGS(player) & STICKY) &&
		    (TMage(player) ||
		     can_link_to(player, NOTYPE, loc) ||
		     controls_link(player, loc) ||
		     ((Typeof(loc) != TYPE_PLAYER) &&
		     (FLAGS(loc) & CHOWN_OK))
		    )) {
#endif
		/* show everything */
		sprintf(upb, "%s" CYELLOW "(#%d%s)",
		    ansiname(loc, tbuf), loc, unparse_flags(loc, tbuf2));
		return upb;
#ifndef SANITY
	    } else {
		/* show only the name */
		return ansiname(loc, upb);
	    }
#endif
    }
}

static char boolexp_buf[BUFFER_LEN];
static char *buftop;

static void 
unparse_boolexp1(dbref player, struct boolexp * b,
		 boolexp_type outer_type, int fullname)
{
    if ((buftop - boolexp_buf) > (BUFFER_LEN / 2))
	return;
    if (b == TRUE_BOOLEXP) {
	strcpy(buftop, "*UNLOCKED*");
	buftop += strlen(buftop);
    } else {
	switch (b->type) {
	    case BOOLEXP_AND:
		if (outer_type == BOOLEXP_NOT) {
		    *buftop++ = '(';
		}
		unparse_boolexp1(player, b->sub1, b->type, fullname);
		*buftop++ = AND_TOKEN;
		unparse_boolexp1(player, b->sub2, b->type, fullname);
		if (outer_type == BOOLEXP_NOT) {
		    *buftop++ = ')';
		}
		break;
	    case BOOLEXP_OR:
		if (outer_type == BOOLEXP_NOT || outer_type == BOOLEXP_AND) {
		    *buftop++ = '(';
		}
		unparse_boolexp1(player, b->sub1, b->type, fullname);
		*buftop++ = OR_TOKEN;
		unparse_boolexp1(player, b->sub2, b->type, fullname);
		if (outer_type == BOOLEXP_NOT || outer_type == BOOLEXP_AND) {
		    *buftop++ = ')';
		}
		break;
	    case BOOLEXP_NOT:
		*buftop++ = '!';
		unparse_boolexp1(player, b->sub1, b->type, fullname);
		break;
	    case BOOLEXP_CONST:
		if (fullname) {
#ifndef SANITY
		    strcpy(buftop, unparse_object(player, b->thing));
#endif
		} else {
		    sprintf(buftop, "#%d", b->thing);
		}
		buftop += strlen(buftop);
		break;
	    case BOOLEXP_PROP:
		strcpy(buftop, PropName(b->prop_check));
		strcat(buftop, ":");
		if (PropType(b->prop_check) == PROP_STRTYP)
		    strcat(buftop, PropDataStr(b->prop_check));
		buftop += strlen(buftop);
		break;
	    default:
		abort();	/* bad type */
		break;
	}
    }
}

const char *
unparse_boolexp(dbref player, struct boolexp * b, int fullname)
{
    buftop = boolexp_buf;
    unparse_boolexp1(player, b, BOOLEXP_CONST, fullname);  /* no outer type */
    *buftop++ = '\0';

    return boolexp_buf;
}
