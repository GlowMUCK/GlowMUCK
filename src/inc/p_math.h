extern void prim_add (PRIM_PROTOTYPE);
extern void prim_subtract (PRIM_PROTOTYPE);
extern void prim_multiply (PRIM_PROTOTYPE);
extern void prim_divide (PRIM_PROTOTYPE);
extern void prim_mod (PRIM_PROTOTYPE);
extern void prim_bitor (PRIM_PROTOTYPE);
extern void prim_bitxor (PRIM_PROTOTYPE);
extern void prim_bitand (PRIM_PROTOTYPE);
extern void prim_bitshift (PRIM_PROTOTYPE);
extern void prim_and (PRIM_PROTOTYPE);
extern void prim_or (PRIM_PROTOTYPE);
extern void prim_not (PRIM_PROTOTYPE);
extern void prim_lessthan (PRIM_PROTOTYPE);
extern void prim_greathan (PRIM_PROTOTYPE);
extern void prim_equal (PRIM_PROTOTYPE);
extern void prim_lesseq (PRIM_PROTOTYPE);
extern void prim_greateq (PRIM_PROTOTYPE);
extern void prim_random (PRIM_PROTOTYPE);
extern void prim_int(PRIM_PROTOTYPE);
extern void prim_float(PRIM_PROTOTYPE);

#define PRIMS_MATH_FUNCS prim_add, prim_subtract, prim_multiply, prim_divide, \
    prim_mod, prim_bitor, prim_bitxor, prim_bitand, prim_bitshift, prim_and,  \
    prim_or, prim_not, prim_lessthan, prim_greathan, prim_equal, prim_lesseq, \
    prim_greateq, prim_random, prim_int, prim_float

#define PRIMS_MATH_NAMES  "+",  "-",  "*",  "/",          \
    "%", "BITOR", "BITXOR", "BITAND", "BITSHIFT", "AND",  \
    "OR",  "NOT",  "<",  ">",  "=",  "<=",                \
    ">=", "RANDOM", "INT", "FLOAT"
 
#define PRIMS_MATH_CNT 20

