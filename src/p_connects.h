extern void prim_awakep(PRIM_PROTOTYPE);
extern void prim_online(PRIM_PROTOTYPE);
extern void prim_concount(PRIM_PROTOTYPE);
extern void prim_condbref(PRIM_PROTOTYPE);
extern void prim_conidle(PRIM_PROTOTYPE);
extern void prim_conidler(PRIM_PROTOTYPE);
extern void prim_contime(PRIM_PROTOTYPE);
extern void prim_conhost(PRIM_PROTOTYPE);
extern void prim_conuser(PRIM_PROTOTYPE);
extern void prim_conipnum(PRIM_PROTOTYPE);
extern void prim_conport(PRIM_PROTOTYPE);
extern void prim_conboot(PRIM_PROTOTYPE);
extern void prim_connotify(PRIM_PROTOTYPE);
extern void prim_condescr(PRIM_PROTOTYPE);
extern void prim_descrcon(PRIM_PROTOTYPE);
extern void prim_nextdescr(PRIM_PROTOTYPE);
extern void prim_descriptors(PRIM_PROTOTYPE);
extern void prim_descr_setuser(PRIM_PROTOTYPE);
extern void prim_descr_setecho(PRIM_PROTOTYPE);
extern void prim_ignoringp(PRIM_PROTOTYPE);
extern void prim_con_listener_port(PRIM_PROTOTYPE);

#define PRIMS_CONNECTS_FUNCS prim_awakep, prim_online, prim_concount,      \
    prim_condbref, prim_conidle, prim_contime, prim_conhost, prim_conuser, \
    prim_conboot, prim_connotify, prim_condescr, prim_descrcon,            \
    prim_nextdescr, prim_descriptors, prim_descr_setuser,		   \
    prim_descr_setecho, prim_conipnum, prim_conport, prim_conidler,	   \
    prim_ignoringp, prim_con_listener_port

#define PRIMS_CONNECTS_NAMES "AWAKE?", "ONLINE", "CONCOUNT",  \
    "CONDBREF", "CONIDLE", "CONTIME", "CONHOST", "CONUSER",   \
    "CONBOOT", "CONNOTIFY", "CONDESCR", "DESCRCON",           \
    "NEXTDESCR", "DESCRIPTORS", "DESCR_SETUSER",	      \
    "DESCR_SETECHO", "CONIPNUM", "CONPORT", "CONIDLER",	      \
    "IGNORING?", "CON_LISTENER_PORT"

#define PRIMS_CONNECTS_CNT 21

