#include "copyright.h"

/* Prototypes for externs not defined elsewhere */

extern char match_args[];
extern char match_cmdname[];

/* From debugger.c */
extern int muf_debugger(dbref player, dbref program, const char *text, struct frame *fr);
extern void muf_backtrace(dbref player, dbref program, int count, struct frame *fr);
extern void list_proglines(dbref player, dbref program, struct frame *fr, int start, int end);
extern char *show_line_prims(dbref program, struct inst *pc, int maxprims, int markpc);


/* From disassem.c */
extern void disassemble(dbref player, dbref program);

/* from event.c */
extern time_t next_muckevent_time(void);
extern void next_muckevent(void);
extern void dump_db_now(void);
#ifdef DELTADUMPS
extern void delta_dump_now(void);
#endif

/* from timequeue.c */
extern int scan_instances(dbref program);
extern void handle_read_event(dbref player, const char *command);
extern int add_muf_read_event(dbref player, dbref prog, struct frame *fr);
extern int add_muf_queue_event(dbref player, dbref loc, dbref trig, dbref prog,
		const char *argstr, const char *cmdstr, int listen_p);
extern int add_muf_delay_event(int delay, dbref player, dbref loc, dbref trig, dbref prog,
		struct frame *fr, const char *mode);
extern int add_muf_delayq_event(int delay, dbref player, dbref loc, dbref trig,
		dbref prog, const char *argstr, const char *cmdstr,
		int listen_p);
extern int add_mpi_event(int delay, dbref player, dbref loc, dbref trig,
		const char *mpi, const char *cmdstr, const char *argstr,
		int listen_p, int omesg_p);
extern int add_event(int event_type, int subtyp, int dtime, dbref player,
		dbref loc, dbref trig, dbref program, struct frame * fr,
		const char *strdata, const char *strcmd, const char *str3);
void listenqueue(dbref player, dbref where, dbref trigger, dbref what,
		dbref xclude, const char *propname, const char *toparg,
		int mlev, int mt, int mpi_p);
extern void next_timequeue_event(void);
extern int  in_timequeue(int pid);
extern time_t next_event_time(void);
extern void list_events(dbref program);
extern int  dequeue_prog(dbref program, int sleeponly);
extern int  dequeue_process(int procnum);
extern int  control_process(dbref player, int procnum);
extern void do_dequeue(dbref player, const char *arg1);
extern void propqueue(dbref player, dbref where, dbref trigger, dbref what,
	    dbref xclude, const char *propname, const char *toparg,
	    int mlev, int mt);
extern void envpropqueue(dbref player, dbref where, dbref trigger, dbref what,
	    dbref xclude, const char *propname, const char *toparg,
	    int mlev, int mt);

/* From compress.c */
extern void init_compress_from_file(FILE *dicto);
extern void save_compress_words_to_file(FILE *f);

/* From create.c */
extern void do_action(dbref player, const char *action_name, const char *source_name);
extern void do_attach(dbref player, const char *action_name, const char *source_name);
extern void do_open(dbref player, const char *direction, const char *linkto);
extern void do_link(dbref player, const char *name, const char *room_name);
extern void do_dig(dbref player, const char *name, const char *pname);
extern void do_create(dbref player, char *name, char *cost);
extern void do_prog(dbref player, const char *name);
extern void do_edit(dbref player, const char *name);
extern int unset_source(dbref player, dbref loc, dbref action);
extern int link_exit(dbref player, dbref exit, char *dest_name, dbref * dest_list);
extern void set_source(dbref player, dbref action, dbref source);
extern int exit_loop_check(dbref source, dbref dest);

/* from diskprop.c */
extern void diskbase_debug(dbref player);
extern int fetchprops_priority(dbref obj, int mode);
extern void dispose_all_oldprops(void);
#ifdef FLUSHCHANGED
extern void undirtyprops(dbref obj);
#endif
extern void update_fetchstats(void);

/* from edit.c */
extern struct macrotable
    *new_macro(const char *name, const char *definition, dbref player);
extern char *macro_expansion(struct macrotable *node, const char *match);
extern void match_and_list(dbref player, const char *name, char *linespec);
extern void do_list(dbref player, dbref program, int arg[], int argc);
extern void free_old_macros(void);

/* From game.c */
extern int  try_alias(dbref player, const char *command);
extern void do_dump(dbref player, const char *newfile);
extern void do_shutdown(dbref player, const char *msg);
extern void do_restart(dbref player, const char *msg);
extern void fork_and_dump(void);
extern void dump_database(void);
extern void do_muf_topprofs(dbref player, const char *arg1);
extern void do_mpi_topprofs(dbref player, const char *arg1);
extern void do_all_topprofs(dbref player, const char *arg1);

extern void dump_warning(void);
#ifdef DELTADUMPS
extern void dump_deltas(void);
#endif

/* From hashtab.c */
extern hash_data *find_hash(const char *s, hash_tab *table, unsigned size);
extern hash_entry *add_hash(const char *name, hash_data data, hash_tab *table,
                            unsigned size);
extern int free_hash(const char *name, hash_tab *table, unsigned size);
extern void kill_hash(hash_tab *table, unsigned size, int freeptrs);

/* From help.c */
extern void get_file_line( const char *filename, char *retbuf, int line );
extern void remove_file_line (const char *filename, int line);
extern void spit_file(dbref player, const char *filename);
extern int spit_file_segment(dbref player, const char *filename, const char *seg, int linenums);
extern void do_help(dbref player, const char *topic, const char *seg);
extern void do_sysparm(dbref player, const char *topic);
extern void do_mpihelp(dbref player, const char *topic, const char *seg);
extern void do_news(dbref player, const char *topic, const char *seg);
extern void do_man(dbref player, const char *topic, const char *seg);
extern void do_motd(dbref player, const char *text);
extern void do_note(dbref player, const char *text);
extern void do_info(dbref player, const char *topic, const char *seg);

/* From look.c */
extern int size_object(dbref i, int load);
extern void look_room(dbref player, dbref room, int verbose);
/* extern void look_room_simple(dbref player, dbref room); */
extern void do_look_around(dbref player);
extern void do_look_at(dbref player, const char *name, const char *detail);
extern void do_check(dbref player, const char *name);
extern void do_examine(dbref player, const char *name, const char *dir, int when);
extern void do_inventory(dbref player);
extern void do_score(dbref player, int domud);
extern void do_find(dbref player, const char *name, const char *flags);
extern void do_owned(dbref player, const char *name, const char *flags);
extern void do_sweep(dbref player, const char *name);
extern void do_trace(dbref player, const char *name, int depth);
extern void do_entrances(dbref player, const char *name, const char *flags);
extern void do_contents(dbref player, const char *name, const char *flags);
extern void exec_or_notify(dbref player, dbref thing, const char *message,
			   const char *whatcalled);
extern void do_quota(dbref player, const char *name);

/* From move.c */
extern void moveto(dbref what, dbref where);
extern void enter_room(dbref player, dbref loc, dbref exit);
extern void send_home(dbref thing, int homepuppet);
extern void send_contents(dbref loc, dbref dest);
extern int parent_loop_check(dbref source, dbref dest);
extern int can_move(dbref player, const char *direction, int lev);
extern void do_move(dbref player, const char *direction, int lev);
extern void do_follow(dbref player, const char *name);
extern void do_lead(dbref player, const char *name, int release);
extern dbref getleader(dbref who);
extern void setleader(dbref who, dbref leader);
extern int unfollowall(dbref who);

extern void do_leave(dbref player);
extern void do_get(dbref player, const char *what, const char *obj);
extern void do_drop(dbref player, const char *name, const char *obj);
extern void do_recycle(dbref player, const char *name);
extern void recycle(dbref player, dbref thing);

/* From player.c */
extern void clear_players(void);
extern dbref lookup_player(const char *name);
extern void do_password(dbref player, const char *old, const char *newobj);
extern void add_player(dbref who);
extern void delete_player(dbref who);
extern void clear_players(void);

/* From predicates.c */
extern int ok_name(const char *name);
extern int ok_password(const char *password);
extern int ok_player_name(const char *name);
extern int test_lock(dbref player, dbref thing, const char *lockprop);
extern int test_lock_false_default(dbref player, dbref thing, const char *lockprop);
extern int can_link_to(dbref who, object_flag_type what_type, dbref where);
extern int can_link(dbref who, dbref what);
extern int could_doit(dbref player, dbref thing);
extern int can_doit(dbref player, dbref thing, const char *default_fail_msg);
extern int can_see(dbref player, dbref thing, int can_see_location);
extern int controls(dbref who, dbref what);
extern int why_controls(dbref who, dbref what, int mlev);
extern int controls_link(dbref who, dbref what);
extern int restricted(dbref player, dbref thing, object_flag_type flag);
extern int restricted2(dbref player, dbref thing, object_flag_type flag);
extern int payfor(dbref who, int cost);
extern int ok_name(const char *name);
extern const char *hostname_domain(const char *hostname);

/* From rob.c */
extern void do_kill(dbref player, const char *what, int cost);
extern void do_give(dbref player, const char *recipient, int amount);
extern void do_rob(dbref player, const char *what);

/* From set.c */
extern void do_name(dbref player, const char *name, char *newname);
#ifdef MUD
extern void do_oname(dbref player, const char *name, const char *oname);
#endif
extern void do_describe(dbref player, const char *name, const char *description);
extern void do_idescribe(dbref player, const char *name, const char *description);
extern void do_fail(dbref player, const char *name, const char *message);
extern void do_success(dbref player, const char *name, const char *message);
extern void do_drop_message(dbref player, const char *name, const char *message);
extern void do_osuccess(dbref player, const char *name, const char *message);
extern void do_ofail(dbref player, const char *name, const char *message);
extern void do_odrop(dbref player, const char *name, const char *message);
extern void do_oecho(dbref player, const char *name, const char *message);
extern void do_pecho(dbref player, const char *name, const char *message);
extern void do_doing(dbref player, const char *name, const char *mesg);
extern void do_setquota(dbref player, const char *name, const char *type);
extern void do_setcontents(dbref player, const char *name, const char *mesg);
extern void do_propset(dbref player, const char *name, const char *prop);
extern int setlockstr(dbref player, dbref thing, const char *keyname);
extern void do_lock(dbref player, const char *name, const char *keyname);
extern void do_unlock(dbref player, const char *name);
extern void do_unlink(dbref player, const char *name);
extern void do_chown(dbref player, const char *name, const char *newobj);
extern void do_species(dbref player, const char *name, const char *species);
extern void do_position(dbref player, const char *name, const char *position);
extern void do_sex(dbref player, const char *name, const char *gender);
extern void do_set(dbref player, const char *name, const char *flag);
extern void do_chlock(dbref player, const char *name, const char *keyname);
extern void do_conlock(dbref player, const char *name, const char *keyname);
extern void do_flock(dbref player, const char *name, const char *keyname);
extern void do_alias(dbref player, const char *alias, const char *format);

/* From speech.c */
extern void do_pose(dbref player, const char *message);
extern void do_whisper(dbref player, const char *arg1, const char *arg2);
extern void do_wall(dbref player, const char *message);
extern void do_gripe(dbref player, const char *message);
extern void do_say(dbref player, const char *message);
extern void do_page(dbref player, const char *arg1, const char *arg2);
extern int notify_listeners(dbref who, dbref xprog, dbref obj, dbref room, const char *msg, int isprivate);
extern int ansi_notify_listeners(dbref who, dbref xprog, dbref obj, dbref room, const char *msg, int isprivate, int parseansi);
extern void notify_except(dbref first, dbref exception, const char *msg, dbref who);
extern void anotify_except(dbref first, dbref exception, const char *msg, dbref who);
extern void parse_omessage(dbref player, dbref dest, dbref exit, const char *msg, const char *prefix, const char *whatcalled);
extern void do_gag(dbref player, const char *who);
extern void do_ungag(dbref player, const char *who);

/* From stringutil.c */
extern int alphanum_compare(const char *s1, const char *s2);
extern int string_compare(const char *s1, const char *s2);
extern const char *exit_prefix(register const char *string, register const char *prefix);
extern int string_prefix(const char *string, const char *prefix);
extern const char *string_match(const char *src, const char *sub);
extern char *pronoun_substitute(dbref player, const char *str);
void string_subst(char *string, const char *match, const char *replacement);
extern char *intostr(int i);

#if !defined(MALLOC_PROFILING)
extern char *string_dup(const char *s);
#endif


/* From utils.c */
extern int member(dbref thing, dbref list);
extern dbref remove_first(dbref first, dbref what);
extern void max_stats( dbref ref, int *maxrooms, int *maxexits, int *maxthings,
			int *maxprograms);
extern int count_stats( dbref ref, int *rooms, int *exits, int *things,
			int *players, int *programs, int *garbage);
extern int quota_check( dbref player, dbref thing, int flags);
extern const char *host_as_hex( /* CrT */ unsigned int addr );

extern int str2ip( const char *ipstr );


/* From wiz.c */
extern int hop_count(void);
extern void email_newbie(const char *name, const char *email, const char *rlname);
extern void do_wizchat(dbref player, const char *arg);
extern void do_memory(dbref who);
extern void do_newpassword(dbref player, const char *name, const char *password);
extern void do_frob(dbref player, const char *name, const char *recip, int how);
extern void do_fixw(dbref player, const char *msg);
extern void do_serverdebug(dbref player, const char *arg1, const char *arg2);
extern void do_getpw(dbref player, const char *arg);
extern void do_hopper(dbref player, const char *arg);
extern void do_teleport(dbref player, const char *arg1, const char *arg2);
extern void do_force(dbref player, const char *what, char *command);
extern void do_stats(dbref player, const char *name);
extern void do_toad(dbref player, const char *name, const char *recip);
extern void do_purge(dbref player, const char *arg1, const char *arg2);
extern void do_boot(dbref player, const char *name);
extern void do_jail(dbref player, const char *name, int free);
extern void do_pcreate(dbref player, const char *arg1, const char *arg2);
extern void do_usage(dbref player);
extern void do_glowflags(dbref player, const char *arg1, const char *arg2);
extern void do_hostcache(dbref player, const char *arg1, const char *arg2);

/* From boolexp.c */
extern int size_boolexp(struct boolexp * b);
extern int eval_boolexp(dbref player, struct boolexp *b, dbref thing);
extern struct boolexp *parse_boolexp(dbref player, const char *string, int dbloadp);
extern struct boolexp *copy_bool(struct boolexp *old);
extern struct boolexp *getboolexp(FILE *f);
extern struct boolexp *negate_boolexp(struct boolexp *b);
extern void free_boolexp(struct boolexp *b);

/* From unparse.c */
extern const char *unparse_flags(dbref thing, char buf[BUFFER_LEN]);
extern const char *ansi_unparse_object(dbref player, dbref object);
extern const char *unparse_object(dbref player, dbref object);
extern const char *unparse_boolexp(dbref player, struct boolexp *b, int fullname);

/* From compress.c */
#ifdef COMPRESS
extern void save_compress_words_to_file(FILE *f);
extern void init_compress_from_file(FILE *dicto);
extern const char *compress(const char *);
extern const char *uncompress(const char *);
extern void init_compress(void);
#endif /* COMPRESS */

/* From edit.c */
extern void interactive(dbref player, const char *command);

/* From compile.c */
extern void kill_def(const char *defname);
extern int size_prog(dbref prog);
extern void uncompile_program(dbref i);
extern void do_uncompile(dbref player);
extern void do_proginfo(dbref player, const char *arg);
extern void free_unused_programs(void);
extern void do_compile(dbref in_player, dbref in_program);
extern void clear_primitives(void);
extern void init_primitives(void);

/* From interp.c */
extern struct inst *interp_loop(dbref player, dbref program,
				struct frame *fr, int rettyp);
extern struct inst *interp(dbref player, dbref location, dbref program,
			   dbref source, int nosleeping, int whichperms,
			   int rettyp);

/* From log.c */
extern void log2file(char *myfilename, char *format, ...);
extern void log2filetime(char *myfilename, char *format, ...);
extern void log_command(char *format, ...);
extern void log_error(char *format, ...);
extern void log_gripe(char *format, ...);
extern void log_muf(char *format, ...);
extern void log_conc(char *format, ...);
extern void log_status(char *format, ...);
extern void log_http(char *format, ...);
extern void log_other(char *format, ...);
extern void notify_fmt(dbref player, char *format, ...);
extern void anotify_fmt(dbref player, char *format, ...);
extern void log_program_text(struct line * first, dbref player, dbref i);

/* From signal.c */
extern void set_signals(void);

/* From strftime.c */
extern int format_time(char *buf, int max_len, const char *fmt, struct tm * tmval);

/* From timestamp.c */
extern void ts_newobject(struct object *thing);
extern void ts_useobject(dbref thing);
extern void ts_lastuseobject(dbref thing);
extern void ts_modifyobject(dbref thing);

/* From smatch.c */
extern int  equalstr(char *s, char *t);

extern void CrT_summarize( dbref player );
extern time_t get_tz_offset(void);

extern int force_level;
extern int alias_level;

/* From credits.c */

extern void do_credits(dbref player);

extern void disassemble(dbref player, dbref program);

/* For MPI profiling */
extern time_t mpi_prof_start_time;
/* For select idle profiling */
extern time_t sel_prof_start_time;
extern long sel_prof_idle_sec;
extern long sel_prof_idle_usec;
extern unsigned long sel_prof_idle_use;
/* From rwho.c */

#ifdef RWHO
extern int rwhocli_setup(const char *server, const char *serverpw, const char *myname, const char *comment);
extern int rwhocli_shutdown(void);
extern int rwhocli_pingalive(void);
extern int rwhocli_userlogin(const char *uid, const char *name, time_t tim);
extern int rwhocli_userlogout(const char *uid);
#endif

/* From sanity.c */

extern int sanity_violated;
extern void sanfix(dbref player);
extern void san_main(void);
extern void sane_dump_object(dbref player, const char *arg);
extern void sanity(dbref player);
extern void sanechange(dbref player, const char *command);

/* From netresolve.c */

extern void do_dos( dbref, const char *, const char * );
extern void host_list( dbref, const char * );
extern void host_add( int, const char *, time_t );
extern int host_del( int );
extern char *host_fetch( int );
extern void host_touch_lastreq( int );
extern int host_touch_last_min_accepts( int );
extern time_t host_lastreq( int );
extern time_t domain_lastreq( int );
extern int host_last_min_accepts( int );
extern void host_init(void);
extern void host_free(void);
extern void host_load(void);
extern void host_save(void);
extern void host_shutdown(void);

#ifdef MUD

/* From mud.c */

extern dbref dam_obj(dbref);
extern dbref arm_obj(dbref);
extern void  stats(dbref, dbref);
extern int   gain_exp(dbref, dbref);
extern void  check_level(dbref);
extern void  check_death(dbref);
extern void  attack(dbref, dbref);
extern void  check_attack(dbref);
extern void  refresh(dbref);
extern void  restore(dbref, int);
extern void  do_offer(dbref, const char *);
extern void  update_mob(void);
extern void  update_mud(void);

/* From cast.c */

extern void  do_cast(dbref, const char *, const char *);

#endif /* MUD */

/* From color.c */

extern char *ansi( char *buf, const char *from, const char *an, int coloron, int parseansi );
extern int ansi_striplen( const char *word, int parseansi );
extern int ansi_offset( const char *from, int offset, int mode );
extern char *parse_ansi( char *buf, const char *from, const char *an, int parseansi );
extern char *unparse_ansi( char *buf, const char *from, int parseansi );
extern char *tct( const char *in, char out[BUFFER_LEN] );
extern void do_colorset( dbref, const char *, const char * );
extern const char *anco( const char *an, int ani, const char *defcol );
extern void load_colorset( dbref player );
extern void save_colorset( dbref player );
extern void free_colorset( dbref player );
extern void load_ignoring( dbref player );
extern void save_ignoring( dbref player );
extern void free_ignoring( dbref player );
extern int ignoring(dbref player, dbref jerk);
extern int add_ignoring(dbref player, dbref jerk);
extern int remove_ignoring(dbref player, dbref jerk);

/* From path.c */

#ifdef PATH

extern int valid_path_dir(void);
extern int valid_path(dbref loc, const char *pathname);
extern const char *get_path_prop(dbref loc, const char *pathname, const char *prop);
extern dbref get_path_dest(dbref loc, const char *pathname);
extern struct boolexp *get_path_lock(dbref loc, const char *pathname);
extern int get_path_size(dbref loc, const char *pathname, int load);
extern dbref match_path(dbref loc, const char *pathname, char *cmdbuf, int *fullmatch);
extern const char *full_path(const char *path);
extern const char *full_paths(const char *path, char *fullpaths);
extern const char *path_name(const char *path);
extern const char *path_name_set(const char *pathname, char *buf);
extern void examine_path(dbref player, dbref loc, const char *pathname);
extern void path_setprop(dbref player, dbref loc, const char *pathname, const char *prop, const char *message);
extern void path_setlock(dbref player, dbref loc, const char *pathname, const char *lock);
extern void do_path(dbref player, const char *pathname, const char *destination, const char *prettyname);
extern void do_path_junk(dbref player, const char *pathname);
extern void do_path_setprop(dbref player, const char *name, const char *prop, const char *message);
extern void move_path(dbref player, const char *pathname, const char *command, int fullmatch);
extern void path_exec_mesg(dbref player, const char *pathname, const char *mesg);
extern const char *path_find_mesg(dbref loc, const char *pathname, const char *mesg, int envsearch);
extern void path_subst(dbref player, dbref loc, dbref dest, const char *pathname, char *mesg, int moved);



#endif /* PATH */
