#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include <ctype.h>

#include "color.h"
#include "db.h"
#include "props.h"
#include "interface.h"
#include "tune.h"
#include "match.h"
#include "strings.h"
#include "externs.h"

void    editor(dbref player, const char *command);
void    do_insert(dbref player, dbref program, int arg[], int argc);
void    do_delete(dbref player, dbref program, int arg[], int argc);
void    do_quit(dbref player, dbref program);
void    do_list(dbref player, dbref program, int arg[], int argc);
void    insert(dbref player, const char *line);
void    do_compile(dbref player, dbref program);
void    free_line(struct line * l);
void    free_prog_text(struct line * l);
void    prog_clean(struct frame * fr);
void    val_and_head(dbref player, int arg[], int argc);
void    do_list_header(dbref player, dbref program);
void    list_publics(dbref player, int arg[], int argc);
void    do_list_publics(dbref player, dbref program);
void    toggle_numbers(dbref player);

/* Editor routines --- Also contains routines to handle input */

/* This routine determines if a player is editing or running an interactive
   command.  It does it by checking the frame pointer field of the player ---
   if the program counter is NULL, then the player is not running anything
   The reason we don't just check the pointer but check the pc too is because
   I plan to leave the frame always on to save the time required allocating
   space each time a program is run.
   */
void
interactive(dbref player, const char *command)
{
    if (FLAGS(player) & READMODE) {
	/*
	 * process command, push onto stack, and return control to forth
	 * program
	 */
	handle_read_event(player, command);
    } else {
	editor(player, command);
    }
}

char *
macro_expansion(struct macrotable *node, const char *match)
{
    if (!node)
	return NULL;
    else {
	register int value = string_compare(match, node->name);

	if (value < 0)
	    return macro_expansion(node->left, match);
	else if (value > 0)
	    return macro_expansion(node->right, match);
	else
	    return alloc_string(node->definition);
    }
}

struct macrotable *
new_macro(const char *name, const char *definition, dbref player)
{
    struct macrotable *newmacro =
    (struct macrotable *) malloc(sizeof(struct macrotable));
    char    buf[BUFFER_LEN];
    int     i;

    for (i = 0; name[i]; i++)
	buf[i] = DOWNCASE(name[i]);
    buf[i] = '\0';
    newmacro->name = alloc_string(buf);
    newmacro->definition = alloc_string(definition);
    newmacro->implementor = player;
    newmacro->left = NULL;
    newmacro->right = NULL;
    return (newmacro);
}

int
grow_macro_tree(struct macrotable * node, struct macrotable * newmacro)
{
    register int value = strcmp(newmacro->name, node->name);

    if (!value)
	return 0;
    else if (value < 0) {
	if (node->left)
	    return grow_macro_tree(node->left, newmacro);
	else {
	    node->left = newmacro;
	    return 1;
	}
    } else if (node->right)
	return grow_macro_tree(node->right, newmacro);
    else {
	node->right = newmacro;
	return 1;
    }
}
int
insert_macro(const char *macroname, const char *macrodef,
	     dbref player, struct macrotable ** node)
{
    struct macrotable *newmacro;

    newmacro = new_macro(macroname, macrodef, player);
    if (!(*node)) {
	*node = newmacro;
	return 1;
    } else
	return (grow_macro_tree((*node), newmacro));
}

void
do_list_tree(struct macrotable * node, const char *first, const char *last,
	     int length, dbref player)
{
    static char buf[BUFFER_LEN];

    if (!node)
	return;
    else {
	if (strncmp(node->name, first, strlen(first)) >= 0)
	    do_list_tree(node->left, first, last, length, player);
	if ((strncmp(node->name, first, strlen(first)) >= 0) &&
		(strncmp(node->name, last, strlen(last)) <= 0)) {
	    if (length) {
		sprintf(buf, "%-16s %-16s  %s", node->name,
			unparse_object(MAN, node->implementor),
			node->definition);
		notify(player, buf);
		buf[0] = '\0';
	    } else {
		sprintf(buf + strlen(buf), "%-16s", node->name);
		if (strlen(buf) > 70) {
		    notify(player, buf);
		    buf[0] = '\0';
		}
	    }
	}
	if (strncmp(last, node->name, strlen(last)) >= 0)
	    do_list_tree(node->right, first, last, length, player);
	if ((node == macrotop) && !length) {
	    notify(player, buf);
	    buf[0] = '\0';
	}
    }
}

void
list_macros(const char *word[], int k, dbref player, int length)
{
    if (!k--) {
	do_list_tree(macrotop, "a", "z", length, player);
    } else {
	do_list_tree(macrotop, word[0], word[k], length, player);
    }
    anotify(player, CINFO "End of list.");
    return;
}

void 
purge_macro_tree(struct macrotable * node)
{
    if (!node)
	return;
    purge_macro_tree(node->left);
    purge_macro_tree(node->right);
    if (node->name)
	free(node->name);
    if (node->definition)
	free(node->definition);
    free(node);
}

int
erase_node(struct macrotable * oldnode, struct macrotable * node,
	   const char *killname, struct macrotable * mtop)
{
    if (!node)
	return 0;
    else if (strcmp(killname, node->name) < 0)
	return erase_node(node, node->left, killname, mtop);
    else if (strcmp(killname, node->name))
	return erase_node(node, node->right, killname, mtop);
    else {
	if (node == oldnode->left) {
	    oldnode->left = node->left;
	    if (node->right)
		grow_macro_tree(mtop, node->right);
	    if (node->name)
		free(node->name);
	    if (node->definition)
		free(node->definition);
	    free((void *) node);
	    return 1;
	} else {
	    oldnode->right = node->right;
	    if (node->left)
		grow_macro_tree(mtop, node->left);
	    if (node->name)
		free(node->name);
	    if (node->definition)
		free(node->definition);
	    free((void *) node);
	    return 1;
	}
    }
}


int
kill_macro(const char *macroname, dbref player, struct macrotable ** mtop)
{
    if (!(*mtop)) {
	return (0);
    } else if (!string_compare(macroname, (*mtop)->name)) {
	struct macrotable *macrotemp = (*mtop);
	int     whichway = ((*mtop)->left) ? 1 : 0;

	*mtop = whichway ? (*mtop)->left : (*mtop)->right;
	if ((*mtop) && (whichway ? macrotemp->right : macrotemp->left))
	    grow_macro_tree((*mtop), whichway ?
			    macrotemp->right : macrotemp->left);
	if (macrotemp->name)
	    free(macrotemp->name);
	if (macrotemp->definition)
	    free(macrotemp->definition);
	free((void *) macrotemp);
	return (1);
    } else if (erase_node((*mtop), (*mtop), macroname, (*mtop)))
	return (1);
    else
	return (0);
}


void
free_old_macros(void)
{
    purge_macro_tree(macrotop);
}


/* The editor itself --- this gets called each time every time to
 * parse a command.
 */

void
editor(dbref player, const char *command)
{
    dbref   program;
    int     arg[MAX_ARG + 1];
    char    buf[BUFFER_LEN];
    const char *word[MAX_ARG + 1];
    int     i, j;	       /* loop variables */

    program = DBFETCH(player)->sp.player.curr_prog;

    /* check to see if we are insert mode */
    if (DBFETCH(player)->sp.player.insert_mode) {
	insert(player, command);/* insert it! */
	return;
    }
    /* parse the commands */
    for (i = 0; i <= MAX_ARG && *command; i++) {
	while (*command && isspace(*command))
	    command++;
	j = 0;
	while (*command && !isspace(*command)) {
	    buf[j] = *command;
	    command++, j++;
	}

	buf[j] = '\0';
	word[i] = alloc_string(buf);
	if ((i == 1) && !string_compare(word[0], "def")) {
	    while (*command && isspace(*command))
		command++;
	    word[2] = alloc_string(command);
	    if (MLevel(player) < tp_muf_macro_mlevel) {
		anotify(player, CFAIL NOPERM_MESG);
	    } else if (!word[2])
		anotify(player, CFAIL "Invalid definition syntax.");
	    else {
		if (insert_macro(word[1], word[2], player, &macrotop)) {
		    anotify(player, CSUCC "Entry created.");
		} else {
		    anotify(player, CINFO "That macro already exists.");
		}
	    }
	    for (; i >= 0; i--) {
		if (word[i])
		    free((void *) word[i]);
	    }
	    return;
	}
	arg[i] = atoi(buf);
	if (arg[i] < 0) {
	    anotify(player, CFAIL "Negative arguments not allowed.");
	    for (; i >= 0; i--) {
		if (word[i])
		    free((void *) word[i]);
	    }
	    return;
	}
    }
    i--;
    while ((i >= 0) && !word[i])
	i--;
    if (i < 0) {
	return;
    } else {
	switch (word[i][0]) {
	    case KILL_COMMAND:
		if (!Mage(player)) {
		    anotify(player, CFAIL NOPERM_MESG);
		} else {
		    if (kill_macro(word[0], player, &macrotop))
			anotify(player, CSUCC "Macro entry deleted.");
		    else
			anotify(player, CINFO "Macro to delete not found.");
		}
		break;
	    case SHOW_COMMAND:
		list_macros(word, i, player, 1);
		break;
	    case SHORTSHOW_COMMAND:
		list_macros(word, i, player, 0);
		break;
	    case INSERT_COMMAND:
		do_insert(player, program, arg, i);
		anotify(player, CINFO "Entering insert mode.");
		break;
	    case DELETE_COMMAND:
		do_delete(player, program, arg, i);
		break;
	    case QUIT_EDIT_COMMAND:
		do_quit(player, program);
		anotify(player, CINFO "Editor exited.");
		break;
	    case COMPILE_COMMAND:
		/* compile code belongs in compile.c, not in the editor */
		do_compile(player, program);
		anotify(player, CSUCC "Compiler done.");
		break;
	    case LIST_COMMAND:
		do_list(player, program, arg, i);
		break;
	    case EDITOR_HELP_COMMAND:
		spit_file(player, EDITOR_HELP_FILE);
		break;
	    case VIEW_COMMAND:
		val_and_head(player, arg, i);
		break;
	    case UNASSEMBLE_COMMAND:
		disassemble(player, program);
		break;
	    case NUMBER_COMMAND:
		toggle_numbers(player);
		break;
	    case PUBLICS_COMMAND:
		list_publics(player, arg, i);
		break;
	    default:
		anotify(player, CFAIL "Illegal editor command.");
		break;
	}
    }
    for (; i >= 0; i--) {
	if (word[i])
	    free((void *) word[i]);
    }
}


/* puts program into insert mode */
void
do_insert(dbref player, dbref program, int arg[], int argc)
{
    DBFETCH(player)->sp.player.insert_mode++;
    /* DBDIRTY(player); */
    if (argc)
	DBSTORE(program, sp.program.curr_line, arg[0] - 1);
    /* set current line to something else */
}

/* deletes line n if one argument,
   lines arg1 -- arg2 if two arguments
   current line if no argument */
void
do_delete(dbref player, dbref program, int arg[], int argc)
{
    struct line *curr, *temp;
    char    buf[BUFFER_LEN];
    int     i;

    switch (argc) {
	case 0:
	    arg[0] = DBFETCH(program)->sp.program.curr_line;
	case 1:
	    arg[1] = arg[0];
	case 2:
	    /* delete from line 1 to line 2 */
	    /* first, check for conflict */
	    if (arg[0] > arg[1]) {
		anotify(player, CFAIL "Nonsensical arguments.");
		return;
	    }
	    i = arg[0] - 1;
	    for (curr = DBFETCH(program)->sp.program.first; curr && i; i--)
		curr = curr->next;
	    if (curr) {
		DBFETCH(program)->sp.program.curr_line = arg[0];
		i = arg[1] - arg[0] + 1;
		/* delete n lines */
		while (i && curr) {
		    temp = curr;
		    if (curr->prev)
			curr->prev->next = curr->next;
		    else
			DBFETCH(program)->sp.program.first = curr->next;
		    if (curr->next)
			curr->next->prev = curr->prev;
		    curr = curr->next;
		    free_line(temp);
		    i--;
		}
		sprintf(buf, CSUCC "%d lines deleted.", arg[1] - arg[0] - i + 1);
		anotify(player, buf);
	    } else
		anotify(player, CINFO "No line to delete.");
	    break;
	default:
	    anotify(player, CINFO "Too many arguments.");
	    break;
    }
}




/* quit from edit mode.  Put player back into the regular game mode */
void
do_quit(dbref player, dbref program)
{
    log_status("PSAV: %s by %s(%d)\n", unparse_object(MAN, program),
		NAME(player), player);
    write_program(DBFETCH(program)->sp.program.first, program);

    if (tp_log_programs && (QLevel(OWNER(player)) < tp_muf_proglog_mlevel))
	log_program_text(DBFETCH(program)->sp.program.first, player, program);

    free_prog_text(DBFETCH(program)->sp.program.first);
    DBFETCH(program)->sp.program.first = NULL;
    FLAGS(program) &= ~INTERNAL;
    FLAGS(player) &= ~INTERACTIVE;
    DBFETCH(player)->sp.player.curr_prog = NOTHING;
    DBDIRTY(player);
    DBDIRTY(program);
}

void 
match_and_list(dbref player, const char *name, char *linespec)
{
    dbref   thing;
    char   *p;
    char   *q;
    int     range[2];
    int     argc;
    struct match_data md;
    struct line *tmpline;

    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    init_match(player, name, TYPE_PROGRAM, &md);
    match_neighbor(&md);
    match_possession(&md);
    match_registered(&md);
    match_absolute(&md);
    if ((thing = noisy_match_result(&md)) == NOTHING)
	return;
    if (Typeof(thing) != TYPE_PROGRAM) {
	anotify(player, CINFO "You can't list anything but a program.");
	return;
    }
    if (!controls(player, thing) && !Viewable(thing) &&
	!(Mage(player) && (OWNER(thing) != MAN))
    ) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }
    if (!*linespec) {
	range[0] = 1;
	range[1] = -1;
	argc = 2;
    } else {
	q = p = linespec;
	while (*p) {
	    while (*p && !isspace(*p))
		*q++ = *p++;
	    while (*p && isspace(*++p));
	}
	*q = '\0';

	argc = 1;
	if (isdigit(*linespec)) {
	    range[0] = atoi(linespec);
	    while (*linespec && isdigit(*linespec))
		linespec++;
	} else {
	    range[0] = 1;
	}
	if (*linespec) {
	    argc = 2;
	    while (*linespec && !isdigit(*linespec))
		linespec++;
	    if (*linespec)
		range[1] = atoi(linespec);
	    else
		range[1] = -1;
	}
    }
    tmpline = DBFETCH(thing)->sp.program.first;
    DBSTORE(thing, sp.program.first, read_program(thing));
    do_list(player, thing, range, argc);
    free_prog_text(DBFETCH(thing)->sp.program.first);
    DBSTORE(thing, sp.program.first, tmpline);
    return;
}


/* list --- if no argument, redisplay the current line
   if 1 argument, display that line
   if 2 arguments, display all in between   */
void
do_list(dbref player, dbref program, int oarg[], int argc)
{
    struct line *curr;
    int     i, count;
    int     arg[2];
    char    buf[BUFFER_LEN];

    if (oarg) {
	arg[0] = oarg[0];
	arg[1] = oarg[1];
    } else
	arg[0] = arg[1] = 0;
    switch (argc) {
	case 0:
	    arg[0] = DBFETCH(program)->sp.program.curr_line;
	case 1:
	    arg[1] = arg[0];
	case 2:
	    if ((arg[0] > arg[1]) && (arg[1] != -1)) {
		anotify_nolisten(player, CFAIL "Arguments don't make sense.", 1);
		return;
	    }
	    i = arg[0] - 1;
	    for (curr = DBFETCH(program)->sp.program.first; i && curr; i--)
		curr = curr->next;
	    if (curr) {
		i = arg[1] - arg[0] + 1;
		/* display n lines */
		for (count = arg[0]; curr && (i || (arg[1] == -1)); i--) {
		    if (FLAGS(player) & INTERNAL)
			sprintf(buf, "%3d: %s", count, DoNull(curr->this_line));
		    else
			sprintf(buf, "%s", DoNull(curr->this_line));
		    notify_nolisten(player, buf, 1);
		    count++;
		    curr = curr->next;
		}
		if (count - arg[0] > 1) {
		    sprintf(buf, CBLUE "%d lines displayed.", count - arg[0]);
		    anotify_nolisten(player, buf, 1);
		}
	    } else
		anotify_nolisten(player, CBLUE "Line not available for display.", 1);
	    break;
	default:
	    anotify_nolisten(player, CINFO "Too many arguments.", 1);
	    break;
    }
}
void
val_and_head(dbref player, int arg[], int argc)
{
    dbref   program;

    if (argc != 1) {
	anotify(player, CINFO "I don't understand which header you're trying to look at.");
	return;
    }
    program = arg[0];
    if ((!OkObj(program))
	    || (Typeof(program) != TYPE_PROGRAM)) {
	anotify(player, CINFO "That isn't a program.");
	return;
    }
    if (!(controls(player, program) || Viewable(program))) {
	anotify(player, CFAIL "That's not a public program.");
	return;
    }
    do_list_header(player, program);
}

void
do_list_header(dbref player, dbref program)
{
    struct line *curr = read_program(program);

    while (curr && (curr->this_line)[0] == '(') {
	notify(player, curr->this_line);
	curr = curr->next;
    }
    if (!(FLAGS(program) & INTERNAL)) {
	free_prog_text(curr);
    }
    anotify(player, CINFO "Done.");
}

void
list_publics(dbref player, int arg[], int argc)
{
    dbref   program;

    if (argc > 1) {
	anotify(player, CINFO "I don't understand which program you want to list PUBLIC functions for.");
	return;
    }
    program = (argc == 0) ? DBFETCH(player)->sp.player.curr_prog : arg[0];
    if (Typeof(program) != TYPE_PROGRAM) {
	anotify(player, CINFO "That isn't a program.");
	return;
    }
    if (!(controls(player, program) || Viewable(program))) {
	anotify(player, CFAIL "That's not a public program.");
	return;
    }
    if (!(DBFETCH(program)->sp.program.code)) {
	if (program == DBFETCH(player)->sp.player.curr_prog) {
	    do_compile(OWNER(program), program);
	} else {
	    struct line *tmpline;

	    tmpline = DBFETCH(program)->sp.program.first;
	    DBFETCH(program)->sp.program.first = (struct line *) read_program(program);
	    do_compile(OWNER(program), program);
	    free_prog_text(DBFETCH(program)->sp.program.first);
	    DBSTORE(program, sp.program.first, tmpline);
	}
	if (!(DBFETCH(program)->sp.program.code)) {
	    anotify(player, CFAIL "Program not compilable.");
	    return;
	}
    }
    do_list_publics(player, program);
}

void
do_list_publics(dbref player, dbref program)
{
    struct publics *ptr;

    anotify(player, CINFO "PUBLIC funtions:");
    for (ptr = DBFETCH(program)->sp.program.pubs; ptr; ptr = ptr->next)
	notify(player, ptr->subname);
}

void 
toggle_numbers(dbref player)
{
    if (FLAGS(player) & INTERNAL) {
	FLAGS(player) &= ~INTERNAL;
	anotify(player, CINFO "Line numbers off.");
    } else {
	FLAGS(player) |= INTERNAL;
	anotify(player, CINFO "Line numbers on.");
    }
}



/* insert this line into program */
void
insert(dbref player, const char *line)
{
    dbref   program;
    int     i;
    struct line *curr;
    struct line *new_line;

    program = DBFETCH(player)->sp.player.curr_prog;
    if (!string_compare(line, EXIT_INSERT)) {
	DBSTORE(player, sp.player.insert_mode, 0);      /* turn off insert mode */
	return;
    }
    i = DBFETCH(program)->sp.program.curr_line - 1;
    for (curr = DBFETCH(program)->sp.program.first; curr && i && i + 1; i--)
	curr = curr->next;
    new_line = get_new_line();  /* initialize line */
    new_line->this_line = alloc_string(line);
    if (!DBFETCH(program)->sp.program.first) {  /* nothing --- insert in front */
	DBFETCH(program)->sp.program.first = new_line;
	DBFETCH(program)->sp.program.curr_line = 2;     /* insert at the end */
	/* DBDIRTY(program); */
	return;
    }
    if (!curr) {		/* insert at the end */
	i = 1;
	for (curr = DBFETCH(program)->sp.program.first; curr->next; curr = curr->next)
	    i++;		/* count lines */
	DBFETCH(program)->sp.program.curr_line = i + 2;
	new_line->prev = curr;
	curr->next = new_line;
	/* DBDIRTY(program); */
	return;
    }
    if (!DBFETCH(program)->sp.program.curr_line) {      /* insert at the
							 * beginning */
	DBFETCH(program)->sp.program.curr_line = 1;     /* insert after this new
							 * line */
	new_line->next = DBFETCH(program)->sp.program.first;
	DBFETCH(program)->sp.program.first = new_line;
	/* DBDIRTY(program); */
	return;
    }
    /* inserting in the middle */
    DBFETCH(program)->sp.program.curr_line++;
    new_line->prev = curr;
    new_line->next = curr->next;
    if (new_line->next)
	new_line->next->prev = new_line;
    curr->next = new_line;
    /* DBDIRTY(program); */
}
