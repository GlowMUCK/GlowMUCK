/*
 * Copyright (c) 1990 Chelsea Dyerman
 * University of California, Berkeley (XCF)
 *
 * Some parts of this code -- in particular the dictionary based compression
 * code is Copyright 1995 by Dragon's Eye Productions
 *
 */

#include "config.h"
#include "params.h"
#include "local.h"

#include "color.h"
#include "interface.h"
#include "externs.h"

const char *servopts =
#ifdef DETACH
	CGREEN "+"
#else
	CRED "-"
#endif
	"Detach  "
#ifdef COMPRESS
	CGREEN "+"
#else
	CRED "-"
#endif
	"Compression  "
#ifdef WIN95
	CGREEN "+"
#else
	CRED "-"
#endif
	"Windows 95  "
#ifdef DISKBASE
	CGREEN "+"
#else
	CRED "-"
#endif
	"Disk based  "
#ifdef DELTADUMPS
	CGREEN "+"
#else
	CRED "-"
#endif
	"Delta dumps  "
#ifdef RWHO
	CGREEN "+"
#else
	CRED "-"
#endif
	"RWHO "
#ifdef MPI
	CGREEN "+"
#else
	CRED "-"
#endif
	"MPI  "
#ifdef MUD
	CGREEN "+"
#else
	CRED "-"
#endif
	"Mud/Mobs  "
#ifdef ROLEPLAY
	CGREEN "+"
#else
	CRED "-"
#endif
	"Roleplaying  "
#ifdef HTTPD
	CGREEN "+"
#else
	CRED "-"
#endif
	"WWW pages  "
#ifdef PATH
	CGREEN "+"
#else
	CRED "-"
#endif
	"Paths";

const char *infotext[] =
{

    CRED VERSION CWHITE " -- " CAQUA GLOWVER,
    " ",
    CWHITE
    "Based on the original code written by these programmers:",
    CGREEN
    "  David Applegate    James Aspnes    Timothy Freeman    Bennet Yee",
    " ",
    CWHITE
    "Others who have done major coding work along the way:",
    CGREEN
    "  Lachesis, ChupChups, FireFoot, Russ 'Random' Smith, and Dr. Cat",
    " ",
    CCYAN
    "This is a user-extendible, user-programmable multi-user adventure game.",
    CCYAN
    "TinyMUCK was derived from TinyMUD v1.5.2, with extensive modifications.",
    CCYAN
    "Because of all the modifications, this program is not in any way, shape,",
    CCYAN
    "or form being supported by any of the original authors.  Any bugs, ideas,",
    CCYAN
    "suggestions,  etc, should be directed to the persons listed below.",
    CCYAN
    "Do not send diff files, send us mail about the bug and describe as best",
    CCYAN
    "as you can, where you were at when the bug occured, and what you think",
    CCYAN
    "caused the bug to be was produced, so we can try to reproduce it and",
    CCYAN
    "track it down.",
    " ",
    CWHITE
    "The following programmers currently maintain the code:",
    CYELLOW
    "  PakRat/Artie:  artie@muq.org           GlowMuck Enhancements",
    CYELLOW
    "  Foxen/Revar:   foxen@belfry.com        Main Fuzzball Developer",
    CYELLOW
    "  Fre'ta:                                Crazy Fuzzball Developer",
    " ",
    CWHITE
    "The following people helped out a lot along the way:",
    CGREEN
    "  Fre'ta, Chris, Ermafelna, Lynx, WhiteFire, Kimi, Cynbe, Myk, Taldin,",
    CGREEN
    "  Howard, darkfox, Moonchilde, Felorin, Xixia, Doran, Riss and King_Claudius.",
    " ",
    CWHITE
    "Alpha and beta test sites, who put up with this nonsense:",
    CPURPLE
    "  HighSeasMUCK, TygryssMUCK, FurryMUCK, CyberFurry, PendorMUCK, Kalasia,",
    CPURPLE
    "  AnimeMUCK, Realms, FurryII, Tapestries, Unbridled Desires, TruffleMUCK,",
    CPURPLE
    "  and Brazillian Dreams.",
    " ",
    CWHITE
    "Places silly enough to give Foxen a wizbit at some time or another:",
    CBLUE
    "  ChupMuck, HighSeas, TygMUCK, TygMUCK II, Furry, Pendor, Realms,",
    CBLUE
    "  Kalasia, Anime, CrossRoadsMUCK, TestMage, MeadowFaire, TruffleMUCK,",
    CBLUE
    "  Tapestries, Brazillian Dreams, SocioPolitical Ramifications, & more.",
    " ",
    CYELLOW
    "Thanks also goes to those persons not mentioned here who have added",
    CYELLOW
    "their advice, opinions, and code to TinyMUCK.",
    0,
};


void
do_credits(dbref player)
{
    int i;

    for (i = 0; infotext[i]; i++) {
	anotify(player, infotext[i]);
    }
}
