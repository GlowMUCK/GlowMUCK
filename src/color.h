/* color.h Copyright 1998 by Andrew Nelson All Rights Reserved */


/* Ansi Colors */
/* Refer to: http://rtfm.etla.org/xterm/ctlseq.html */

#define ANSI256BACK     "\033[48;5;"
#define ANSI256FORE     "\033[38;5;"

#define ANSINORMAL	"\033[0m"
#define ANSICLS		"\033[H\033[J\033[r"
#define ANSIBEEP	"\007"
#define ANSIFLASH	"\033[5m"
#define ANSIINVERT	"\033[7m"

#define ANSIBLACK	"\033[0;30m"
#define ANSICRIMSON	"\033[0;31m"
#define ANSIFOREST	"\033[0;32m"
#define ANSIBROWN	"\033[0;33m"
#define ANSINAVY	"\033[0;34m"
#define ANSIVIOLET	"\033[0;35m"
#define ANSIAQUA	"\033[0;36m"
#define ANSIGRAY	"\033[0;37m"

#define ANSIGLOOM	"\033[1;30m"
#define ANSIRED		"\033[1;31m"
#define ANSIGREEN	"\033[1;32m"
#define ANSIYELLOW	"\033[1;33m"
#define ANSIBLUE	"\033[1;34m"
#define ANSIPURPLE	"\033[1;35m"
#define ANSICYAN	"\033[1;36m"
#define ANSIWHITE	"\033[1;37m"

#define ANSIBBLACK	"\033[40m"
#define ANSIBRED	"\033[41m"
#define ANSIBGREEN	"\033[42m"
#define ANSIBBROWN	"\033[43m"
#define ANSIBBLUE	"\033[44m"
#define ANSIBPURPLE	"\033[45m"
#define ANSIBCYAN	"\033[46m"
#define ANSIBGRAY	"\033[47m"

/* Colors */

#define CCNORMAL	"NORMAL"
#define CCFLASH		"FLASH"
#define CCINVERT	"INVERT"

#define CCBLACK		"BLACK"
#define CCCRIMSON	"CRIMSON"
#define CCFOREST	"FOREST"
#define CCBROWN		"BROWN"
#define CCNAVY		"NAVY"
#define CCVIOLET	"VIOLET"
#define CCAQUA		"AQUA"
#define CCGRAY		"GRAY"

#define CCGLOOM		"GLOOM"
#define CCRED		"RED"
#define CCGREEN		"GREEN"
#define CCYELLOW	"YELLOW"
#define CCBLUE		"BLUE"
#define CCPURPLE	"PURPLE"
#define CCCYAN		"CYAN"
#define CCWHITE		"WHITE"

#define CCBBLACK	"BBLACK"
#define CCBRED		"BRED"
#define CCBGREEN	"BGREEN"
#define CCBBROWN	"BBROWN"
#define CCBBLUE		"BBLUE"
#define CCBPURPLE	"BPURPLE"
#define CCBCYAN		"BCYAN"
#define CCBGRAY		"BGRAY"


/* Color translations */

#define CNORMAL		"^" CCNORMAL "^"
#define CFLASH		"^" CCFLASH "^"
#define CINVERT		"^" CCINVERT "^"

#define CBLACK		"^" CCBLACK "^"
#define CCRIMSON	"^" CCCRIMSON "^"
#define CFOREST		"^" CCFOREST "^"
#define CBROWN		"^" CCBROWN "^"
#define CNAVY		"^" CCNAVY "^"
#define CVIOLET		"^" CCVIOLET "^"
#define CAQUA		"^" CCAQUA "^"
#define CGRAY		"^" CCGRAY "^"

#define CGLOOM		"^" CCGLOOM "^"
#define CRED		"^" CCRED "^"
#define CGREEN		"^" CCGREEN "^"
#define CYELLOW		"^" CCYELLOW "^"
#define CBLUE		"^" CCBLUE "^"
#define CPURPLE		"^" CCPURPLE "^"
#define CCYAN		"^" CCCYAN "^"
#define CWHITE		"^" CCWHITE "^"

#define CBBLACK		"^" CCBBLACK "^"
#define CBRED		"^" CCBRED "^"
#define CBGREEN		"^" CCBGREEN "^"
#define CBBROWN		"^" CCBBROWN "^"
#define CBBLUE		"^" CCBBLUE "^"
#define CBPURPLE	"^" CCBPURPLE "^"
#define CBCYAN		"^" CCBCYAN "^"
#define CBGRAY		"^" CCBGRAY "^"

/* These are defined in defaults.h */

#define CFAIL		"^" CCFAIL "^"
#define CSUCC		"^" CCSUCC "^"
#define CINFO		"^" CCINFO "^"
#define CNOTE		"^" CCNOTE "^"
#define CMOVE		"^" CCMOVE "^"


/* Macros */

#define TildeAnsiDigit(x)	(((x) == '-') || (((x) >= '0') && ((x) <= '9')))
