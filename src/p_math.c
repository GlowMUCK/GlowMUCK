/* Primitives package */

#include "copyright.h"
#include "config.h"
#include "params.h"
#include "local.h"

#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#include "db.h"
#include "inst.h"
#include "match.h"
#include "interface.h"
#include "tune.h"
#include "strings.h"
#include "interp.h"
#include "externs.h"

extern struct inst *oper1, *oper2, *oper3, *oper4;
extern struct inst temp1, temp2, temp3;
extern int tmp, result;
extern dbref ref;


int 
arith_type(struct inst * op1, struct inst * op2)
{
    return ((op1->type == PROG_INTEGER && op2->type == PROG_INTEGER)	/* real stuff */
            ||(op1->type == PROG_FLOAT && op2->type == PROG_INTEGER)    /* float stuff */
            ||(op1->type == PROG_INTEGER && op2->type == PROG_FLOAT)    /* float stuff */
	    ||(op1->type == PROG_FLOAT && op2->type == PROG_FLOAT)      /* act. float stuff */
	    ||(op1->type == PROG_OBJECT && op2->type == PROG_INTEGER)	/* inc. dbref */
	    ||(op1->type == PROG_VAR && op2->type == PROG_INTEGER)	/* offset array */
	    ||(op1->type == PROG_LVAR && op2->type == PROG_INTEGER));
}


void 
prim_add(PRIM_PROTOTYPE)
{
    double dbl_result = 0.0;
    
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!arith_type(oper2, oper1))
    {
			abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) 
    {
      dbl_result = oper2->data.float_n + oper1->data.float_n;
      tmp = PROG_FLOAT;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER)
    {
      dbl_result = oper2->data.float_n + oper1->data.number;
      tmp = PROG_FLOAT;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT)
    {
      dbl_result = oper2->data.number + oper1->data.float_n;
      tmp = PROG_FLOAT;
    } else {
      result = oper2->data.number + oper1->data.number;
      tmp = oper2->type;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    if (tmp == PROG_FLOAT) {
      push(arg, top, tmp, MIPSCAST &dbl_result);
    } else {
      push(arg, top, tmp, MIPSCAST &result);
    }
}

void 
prim_subtract(PRIM_PROTOTYPE)
{
    double dbl_result = 0.0;
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!arith_type(oper2, oper1))
    {
	abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) 
    {
      dbl_result = oper2->data.float_n - oper1->data.float_n;
      tmp = PROG_FLOAT;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER)
    {
      dbl_result = oper2->data.float_n - oper1->data.number;
      tmp = PROG_FLOAT;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT)
    {
      dbl_result = oper2->data.number - oper1->data.float_n;
      tmp = PROG_FLOAT;
    } else {
      result = oper2->data.number - oper1->data.number;
      tmp = oper2->type;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    if (tmp == PROG_FLOAT) {
      push(arg, top, tmp, MIPSCAST &dbl_result);
    } else {
      push(arg, top, tmp, MIPSCAST &result);
    }
}


void 
prim_multiply(PRIM_PROTOTYPE)
{
    double dbl_result = 0.0;
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!arith_type(oper2, oper1))
    {
	abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) 
    {
      dbl_result = oper2->data.float_n * oper1->data.float_n;
      tmp = PROG_FLOAT;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER)
    {
      dbl_result = oper2->data.float_n * oper1->data.number;
      tmp = PROG_FLOAT;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT)
    {
      dbl_result = oper2->data.number * oper1->data.float_n;
      tmp = PROG_FLOAT;
    } else {
      result = oper2->data.number * oper1->data.number;
      tmp = oper2->type;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    if (tmp == PROG_FLOAT) {
      push(arg, top, tmp, MIPSCAST &dbl_result);
    } else {
      push(arg, top, tmp, MIPSCAST &result);
    }
}

void 
prim_divide(PRIM_PROTOTYPE)
{
		double dbl_result = 0.0;

		CHECKOP(2);

		oper1 = POP();
		oper2 = POP();

		if (!arith_type(oper2, oper1)) 
		{
				abort_interp("Invalid argument type");
		}

		if ((oper1->type == PROG_FLOAT && oper1->data.float_n == 0.0) ||
				(oper1->type == PROG_INTEGER && oper1->data.number == 0))
		{
				if (tp_division_by_zero_error)
				{
						abort_interp("Division by zero error");
				}
				else
				{
						if (oper2->type == PROG_FLOAT || oper1->type == PROG_FLOAT)
						{
								dbl_result = 0.0;
								tmp = PROG_FLOAT;
						}
						else
						{
								result = 0;
								tmp = PROG_INTEGER;
						}
				}
		}
		else
		{
				if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT)
				{
						if (oper1->data.float_n)
						{
								dbl_result = oper2->data.float_n / oper1->data.float_n;
						} else {
								dbl_result = 0.0;
						}
						tmp = PROG_FLOAT;
				} 
				else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER) 
				{
						if (oper1->data.number)
						{
								dbl_result = oper2->data.float_n / oper1->data.number;
						} else {
								dbl_result = 0.0;
						}
						tmp = PROG_FLOAT;
				}
				else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT)
				{
						if (oper1->data.float_n)
						{
								dbl_result = oper2->data.number / oper1->data.float_n;
						} else {
								dbl_result = 0.0;
						}
						tmp = PROG_FLOAT;
				} 
				else
				{
						if (oper1->data.number)
						{
								result = oper2->data.number / oper1->data.number;
						} else {
								result = 0;
						}
						tmp = oper2->type;
				}
		} // Divide by zero check, else closing brace

		CLEAR(oper1);
		CLEAR(oper2);

		if (tmp == PROG_FLOAT) {
				push(arg, top, tmp, MIPSCAST &dbl_result);
		} else {
				push(arg, top, tmp, MIPSCAST &result);
		}
}


void 
prim_mod(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();

    if (!arith_type(oper2, oper1))
    {
				abort_interp("Invalid argument type");
    }

    if (oper1->type == PROG_FLOAT || oper2->type == PROG_FLOAT)
    {
        abort_interp("Modulus cannot be performed on floating point numbers.");
    }

    if (oper1->data.number == 0)
    {
				if (tp_division_by_zero_error)
				{
						abort_interp("Divide by zero error when performing modulus");
				}
				else
				{
						result = 0;
				}
    }
		else
		{
				result = oper2->data.number % oper1->data.number;
		}

    tmp = oper2->type;

    CLEAR(oper1);
    CLEAR(oper2);
    push(arg, top, tmp, MIPSCAST & result);
}


void 
prim_bitor(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!arith_type(oper2, oper1)) 
    {
	abort_interp("Invalid argument type");
    }
    if (oper1->type == PROG_FLOAT || oper2->type == PROG_FLOAT)
    {
        abort_interp("Bit operations cannot be performed on floating point numbers.");
    }
    result = oper2->data.number | oper1->data.number;
    tmp = oper2->type;
    CLEAR(oper1);
    CLEAR(oper2);
    push(arg, top, tmp, MIPSCAST & result);
}


void 
prim_bitxor(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!arith_type(oper2, oper1)) 
    {
	abort_interp("Invalid argument type");
    }
    if (oper1->type == PROG_FLOAT || oper2->type == PROG_FLOAT)
    {
        abort_interp("Bit operations cannot be performed on floating point numbers.");
    }
    result = oper2->data.number ^ oper1->data.number;
    tmp = oper2->type;
    CLEAR(oper1);
    CLEAR(oper2);
    push(arg, top, tmp, MIPSCAST & result);
}


void 
prim_bitand(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!arith_type(oper2, oper1)) 
    {
	abort_interp("Invalid argument type");
    }
    if (oper1->type == PROG_FLOAT || oper2->type == PROG_FLOAT)
    {
        abort_interp("Bit operations cannot be performed on floating point numbers.");
    }
    result = oper2->data.number & oper1->data.number;
    tmp = oper2->type;
    CLEAR(oper1);
    CLEAR(oper2);
    push(arg, top, tmp, MIPSCAST & result);
}


void 
prim_bitshift(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!arith_type(oper2, oper1)) 
    {
	abort_interp("Invalid argument type");
    }
    if (oper1->type == PROG_FLOAT || oper2->type == PROG_FLOAT)
    {
        abort_interp("Bit operations cannot be performed on floating point numbers.");
    }
    if (oper1->data.number > 0)
	result = oper2->data.number << oper1->data.number;
    else if (oper1->data.number < 0)
	result = oper2->data.number >> (-(oper1->data.number));
    else
	result = oper2->data.number;
    tmp = oper2->type;
    CLEAR(oper1);
    CLEAR(oper2);
    push(arg, top, tmp, MIPSCAST & result);
}


void 
prim_and(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    result = !muf_false(oper1) && !muf_false(oper2);
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}

void 
prim_or(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    result = !muf_false(oper1) || !muf_false(oper2);
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}

void 
prim_not(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = muf_false(oper1);
    CLEAR(oper1);
    PushInt(result);
}

void 
prim_lessthan(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!((oper1->type == PROG_INTEGER || oper1->type == PROG_FLOAT) &&
          (oper2->type == PROG_INTEGER || oper2->type == PROG_FLOAT)))
    {
	abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) {
      result = oper2->data.float_n < oper1->data.float_n;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER) {
      result = oper2->data.float_n < oper1->data.number;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT) {
      result = oper2->data.number < oper1->data.float_n;
    } else {
      result = oper2->data.number < oper1->data.number;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_greathan(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!((oper1->type == PROG_INTEGER || oper1->type == PROG_FLOAT) &&
          (oper2->type == PROG_INTEGER || oper2->type == PROG_FLOAT)))
    {
	abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) {
      result = oper2->data.float_n > oper1->data.float_n;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER) {
      result = oper2->data.float_n > oper1->data.number;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT) {
      result = oper2->data.number > oper1->data.float_n;
    } else {
      result = oper2->data.number > oper1->data.number;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_equal(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!((oper1->type == PROG_INTEGER || oper1->type == PROG_FLOAT) &&
          (oper2->type == PROG_INTEGER || oper2->type == PROG_FLOAT)))
    {
	abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) {
      result = oper2->data.float_n == oper1->data.float_n;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER) {
      result = oper2->data.float_n == oper1->data.number;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT) {
      result = oper2->data.number == oper1->data.float_n;
    } else {
      result = oper2->data.number == oper1->data.number;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_lesseq(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!((oper1->type == PROG_INTEGER || oper1->type == PROG_FLOAT) &&
          (oper2->type == PROG_INTEGER || oper2->type == PROG_FLOAT)))
    {
	abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) {
      result = oper2->data.float_n <= oper1->data.float_n;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER) {
      result = oper2->data.float_n <= oper1->data.number;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT) {
      result = oper2->data.number <= oper1->data.float_n;
    } else {
      result = oper2->data.number <= oper1->data.number;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}


void 
prim_greateq(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (!((oper1->type == PROG_INTEGER || oper1->type == PROG_FLOAT) &&
          (oper2->type == PROG_INTEGER || oper2->type == PROG_FLOAT)))
    {
	abort_interp("Invalid argument type");
    }
    if (oper2->type == PROG_FLOAT && oper1->type == PROG_FLOAT) {
      result = oper2->data.float_n >= oper1->data.float_n;
    } else if (oper2->type == PROG_FLOAT && oper1->type == PROG_INTEGER) {
      result = oper2->data.float_n >= oper1->data.number;
    } else if (oper2->type == PROG_INTEGER && oper1->type == PROG_FLOAT) {
      result = oper2->data.number >= oper1->data.float_n;
    } else {
      result = oper2->data.number >= oper1->data.number;
    }
    CLEAR(oper1);
    CLEAR(oper2);
    PushInt(result);
}

void 
prim_random(PRIM_PROTOTYPE)
{
    result = RANDOM();
    CHECKOFLOW(1);
    PushInt(result);
}

void prim_int(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (!(oper1->type == PROG_OBJECT || oper1->type == PROG_VAR ||
	  oper1->type == PROG_LVAR || oper1->type == PROG_FLOAT))
	abort_interp("Invalid argument type");
    if (oper1->type == PROG_FLOAT) 
    {
      result  = (int)(oper1->data.float_n);
    } else {
      result = (int) ((oper1->type == PROG_OBJECT) ?
	  	      oper1->data.objref : oper1->data.number);
    }
    CLEAR(oper1);
    PushInt(result);
}

void prim_float(PRIM_PROTOTYPE)
{
    double dbl_result = 0.0;
    CHECKOP(1);
    oper1 = POP();

    if (oper1->type == PROG_INTEGER)
    {
      dbl_result = (double)(oper1->data.number);
    } else {
	abort_interp("Integer number expected");
    }
    CLEAR(oper1);
    PushFloat(dbl_result);
}
