#include "copyright.h"
#include "config.h"
#include "local.h"

#include "db.h"
#include "props.h"
#include "interface.h"
#include "interp.h"
#include "inst.h"
#include "externs.h"

void
disassemble(dbref player, dbref program)
{
    struct inst *curr;
    struct inst *codestart;
    int     i;
    char    buf[BUFFER_LEN];

    codestart = curr = DBFETCH(program)->sp.program.code;
    if (!DBFETCH(program)->sp.program.siz) {
	notify(player, "Nothing to disassemble!");
	return;
    }
    for (i = 0; i < DBFETCH(program)->sp.program.siz; i++, curr++) {
	switch (curr->type) {
	    case PROG_PRIMITIVE:
		if (curr->data.number >= BASE_MIN && curr->data.number <= BASE_MAX)
		    sprintf(buf, "%d: (line %d) PRIMITIVE: %s", i,
			curr->line, base_inst[curr->data.number - BASE_MIN]);
		else
		    sprintf(buf, "%d: (line %d) PRIMITIVE: %d", i,
			    curr->line, curr->data.number);
		break;
	    case PROG_STRING:
		sprintf(buf, "%d: (line %d) STRING: \"%s\"", i, curr->line,
			curr->data.string ? curr->data.string->data : "");
		break;
	    case PROG_FUNCTION:
		sprintf(buf,"%d: (line %d) FUNCTION HEADER: %s",i,curr->line,
			curr->data.string ? curr->data.string->data : "");
		break;
	    case PROG_LOCK:
		sprintf(buf, "%d: (line %d) LOCK: [%s]", i, curr->line,
			curr->data.lock == TRUE_BOOLEXP ? "TRUE_BOOLEXP" :
			unparse_boolexp(0, curr->data.lock, 0));
		break;
	    case PROG_INTEGER:
		sprintf(buf, "%d: (line %d) INTEGER: %d", i,
			curr->line, curr->data.number);
		break;
	    case PROG_ADD:
		sprintf(buf, "%d: (line %d) ADDRESS: %d", i,
			curr->line, (int)(curr->data.addr->data - codestart));
		break;
	    case PROG_IF:
		sprintf(buf, "%d: (line %d) IF: %d", i,
			curr->line, (int)(curr->data.call - codestart));
		break;
	    case PROG_JMP:
		sprintf(buf, "%d: (line %d) JMP: %d", i,
			curr->line, (int)(curr->data.call - codestart));
		break;
	    case PROG_EXEC:
		sprintf(buf, "%d: (line %d) EXEC: %d", i,
			curr->line, (int)(curr->data.call - codestart));
		break;
	    case PROG_OBJECT:
		sprintf(buf, "%d: (line %d) OBJECT REF: %d", i,
			curr->line, curr->data.number);
		break;
	    case PROG_VAR:
		sprintf(buf, "%d: (line %d) VARIABLE: %d", i,
			curr->line, curr->data.number);
		break;
	    case PROG_LVAR:
		sprintf(buf, "%d: (line %d) LOCALVAR: %d", i,
			curr->line, curr->data.number);
	}
	notify(player, buf);
    }
}
