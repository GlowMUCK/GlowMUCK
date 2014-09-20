/* Stuff the interpreter needs. */
/* Some icky machine/compiler #defines. --jim */
#ifdef MIPS
typedef char * voidptr;
#define MIPSCAST (char *)
#else
typedef void * voidptr;
#define MIPSCAST
#endif
#ifdef COMPRESS
#define alloc_compressed(x) alloc_string(compress(x))
#define get_compress(x) compress(x)
#define get_uncompress(x) uncompress(x)
#else /* COMPRESS */
#define alloc_compressed(x) alloc_string(x)
#define get_compress(x) (x)
#define get_uncompress(x) (x)
#endif /* COMPRESS */
#define DoNullInd(x) ((x) ? (x) -> data : "")
  
extern void do_abort_silent(void);

extern void RCLEAR(struct inst *oper, const char *file, int line);
#define CLEAR(oper) RCLEAR(oper, __FILE__, __LINE__)
extern void push (struct inst *stack, int *top, int type, voidptr res);
extern int valid_object(struct inst *oper);
  
extern int muf_false(struct inst *p);
  
extern void copyinst(struct inst *from, struct inst *to);
  
#define abort_loop(S, C1, C2) \
{ \
  do_abort_loop(player, program, (S), fr, pc, atop, stop, (C1), (C2)); \
  return 0; \
}
  
extern void do_abort_loop(dbref player, dbref program, const char *msg,
                          struct frame *fr, struct inst *pc,
                          int atop, int stop, struct inst *clinst1,
                          struct inst *clinst2);
  
extern void interp_err(dbref player, dbref program, struct inst *pc,
                       struct inst *arg, int atop, dbref origprog,
                       const char *msg1, const char *msg2);
  
extern void push (struct inst *stack, int *top, int type, voidptr res);
  
extern int valid_player(struct inst *oper);
  
extern int valid_object(struct inst *oper);
  
extern int is_home(struct inst *oper);
  
extern int permissions(int mlev, dbref player, dbref thing);
  
extern int arith_type(struct inst *op1, struct inst *op2);
  
#define CHECKOP(N) { if ((*top) < (N)) { interp_err(player, program, pc, arg, *top, fr->caller.st[1], insttotext(pc, 30, program), "Stack underflow."); return; } nargs = (N); }

#define POP() (arg + --(*top))
  
#define abort_interp(C) \
{ \
  do_abort_interp(player, (C), pc, arg, *top, fr, oper1, oper2, oper3, oper4, \
                  nargs, program, __FILE__, __LINE__); \
  return; \
}
extern void do_abort_interp(dbref player, const char *msg, struct inst *pc,
     struct inst *arg, int atop, struct frame *fr,
     struct inst *oper1, struct inst *oper2, struct inst *oper3,
     struct inst *oper4, int nargs, dbref program, const char *file, int line);
  
extern void purge_all_free_frames(void);

#define CurrVar (*(fr->varset.st[fr->varset.top]))

#define Min(x,y) ((x < y) ? x : y)
#define ProgMLevel(x) (find_mlev(x, fr, fr->caller.top))

#define ProgUID find_uid(player, fr, fr->caller.top, program)
extern int find_mlev(dbref prog, struct frame *fr, int st);
extern dbref find_uid(dbref player, struct frame *fr, int st, dbref program);

#define CHECKREMOTE(x) if ((mlev < 2) && (getloc(x) != player) &&  \
                           (getloc(x) != getloc(player)) && \
                           ((x) != getloc(player)) && ((x) != player) \
			   && !controls(ProgUID, x)) \
                 abort_interp("Mucker Level 2 required to get remote info.");

#define PushObject(x)   push(arg, top, PROG_OBJECT, MIPSCAST &x)
#define PushInt(x)      push(arg, top, PROG_INTEGER, MIPSCAST &x)
#define PushFloat(x)    push(arg, top, PROG_FLOAT, MIPSCAST &x)
#define PushLock(x)     push(arg, top, PROG_LOCK, MIPSCAST copy_bool(x))
#define PushTrueLock(x) push(arg, top, PROG_LOCK, MIPSCAST TRUE_BOOLEXP)
#define PushStrRaw(x)   push(arg, top, PROG_STRING, MIPSCAST x)
#define PushString(x)   PushStrRaw(alloc_prog_string(x))
#define PushNullStr     PushStrRaw(0)

#define CHECKOFLOW(x) if((*top + (x - 1)) >= STACK_SIZE) \
			  abort_interp("Stack Overflow!");

#define PRIM_PROTOTYPE dbref player, dbref program, int mlev, \
                       struct inst *pc, struct inst *arg, int *top, \
                       struct frame *fr

extern int    nargs; /* DO NOT TOUCH THIS VARIABLE */

#include "p_connects.h"
#include "p_db.h"
#include "p_math.h"
#include "p_misc.h"
#include "p_props.h"
#include "p_stack.h"
#include "p_strings.h"
