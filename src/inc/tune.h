/*
 * tune.h
 * $Revision: 1.3 $ $Date: 2005/03/08 18:57:36 $
 */

/*
 * $Log: tune.h,v $
 * Revision 1.3  2005/03/08 18:57:36  feaelin
 * Added the heartbeat modifications. You can add programs to the @heartbeat
 * propdir and the programs will be executed every 15 seconds.
 *
 */

/* Function Prototypes */
extern void do_tune(dbref player, char *parmname, char *parmval);
extern int tune_count_parms(void);
extern void tune_load_parms_from_file(FILE *f, dbref player, int cnt);
extern void tune_save_parms_to_file(FILE *f);
extern void tune_load_parmsfile(dbref player);
extern void tune_save_parmsfile(void);
extern int tune_setparm(const dbref player, const char *parmname, const char *val);

/* strings */
extern const char *tp_dumpwarn_mesg;
extern const char *tp_deltawarn_mesg;
extern const char *tp_dumpdeltas_mesg;
extern const char *tp_dumping_mesg;
extern const char *tp_dumpdone_mesg;
extern const char *tp_shutdown_mesg;
extern const char *tp_restart_mesg;

extern const char *tp_penny;
extern const char *tp_pennies;
extern const char *tp_cpenny;
extern const char *tp_cpennies;

extern const char *tp_muckname ;

extern const char *tp_rwho_passwd;
extern const char *tp_rwho_server;

extern const char *tp_huh_mesg;
extern const char *tp_leave_mesg;
extern const char *tp_idle_mesg;
extern const char *tp_register_mesg;
extern const char *tp_playermax_warnmesg;
extern const char *tp_playermax_bootmesg;
extern const char *tp_path_dir;
extern const char *tp_html_parent_link;
extern const char *tp_pueblo_message;

/* times */

extern time_t tp_mob_interval;
extern time_t tp_mud_interval;

extern time_t tp_rwho_interval;

extern time_t tp_dump_interval;
extern time_t tp_dump_warntime;
extern time_t tp_monolithic_interval;
extern time_t tp_clean_interval;
extern time_t tp_aging_time;
extern time_t tp_maxidle;
extern time_t tp_idletime;
extern time_t tp_registration_wait;
extern time_t tp_connect_wait;
extern time_t tp_fail_wait;



/* integers */
extern int tp_max_object_endowment;
extern int tp_object_cost;
extern int tp_exit_cost;
extern int tp_link_cost;
extern int tp_room_cost;
extern int tp_lookup_cost;
extern int tp_max_pennies;
extern int tp_penny_rate;
extern int tp_start_pennies;

extern int tp_kill_base_cost;
extern int tp_kill_min_cost;
extern int tp_kill_bonus;

extern int tp_command_burst_size;
extern int tp_commands_per_time;
extern int tp_command_time_msec;

extern int tp_max_delta_objs;
extern int tp_max_loaded_objs;
extern int tp_max_process_limit;
extern int tp_max_plyr_processes;
extern int tp_max_instr_count;
extern int tp_instr_slice;
extern int tp_mpi_max_commands;
extern int tp_pause_min;
extern int tp_free_frames_pool;
extern int tp_max_output;
extern int tp_rand_screens;

extern int tp_listen_mlev;
extern int tp_playermax_limit;

extern int tp_max_things;
extern int tp_max_exits;
extern int tp_max_programs;
extern int tp_max_rooms;

extern int tp_muf_macro_mlevel;
extern int tp_tinkerproof_mlevel;
extern int tp_hidden_prop_mlevel;
extern int tp_seeonly_prop_mlevel;
extern int tp_private_prop_mlevel;
extern int tp_quell_ignore_mlevel;
extern int tp_muf_proglog_mlevel;
extern int tp_path_mlevel;
extern int tp_help_mlevel;
extern int tp_muf_mpi_flag_mlevel;
extern int tp_newobject_mlevel;
extern int tp_fail_retries;
extern int tp_max_player_logins;
extern int tp_max_site_welcomes;
extern int tp_max_site_lma;
extern int tp_max_ignores;
extern int tp_www_port;
extern int tp_dump_copies;
extern int tp_read_bytes;
extern int tp_write_bytes;


/* dbrefs */
extern dbref tp_player_start;
extern dbref tp_environment_room;
extern dbref tp_reg_wiz;
extern dbref tp_path_prog;
extern dbref tp_jail_room;
extern dbref tp_home_room;
extern dbref tp_offered_room;
extern dbref tp_dead_room;
extern dbref tp_www_root;
extern dbref tp_www_user;
extern dbref tp_heartbeat_user;

/* booleans */
extern int tp_hostnames;
extern int tp_log_commands;
extern int tp_log_interactive;
extern int tp_log_wizards;
extern int tp_log_connects;
extern int tp_log_http;
extern int tp_log_mud_commands;
extern int tp_log_failed_commands;
extern int tp_log_programs;
extern int tp_log_guests;
extern int tp_log_with_names;
extern int tp_dbdump_warning;
extern int tp_deltadump_warning;
extern int tp_periodic_program_purge;

extern int tp_mob;
extern int tp_mud;

extern int tp_rwho;

extern int tp_secure_who;
extern int tp_who_doing;
extern int tp_realms_control;
extern int tp_listeners;
extern int tp_listeners_obj;
extern int tp_listeners_env;
extern int tp_zombies;
extern int tp_wiz_vehicles;
extern int tp_wiz_name;
extern int tp_recycle_frobs;
extern int tp_m1_name_notify;
extern int tp_restrict_kill;
extern int tp_registration;
extern int tp_online_registration;
extern int tp_fast_registration;
extern int tp_validate_hostname;
extern int tp_validate_warning;
extern int tp_teleport_to_player;
extern int tp_secure_teleport;
extern int tp_exit_darking;
extern int tp_thing_darking;
extern int tp_dark_sleepers;
extern int tp_who_hides_dark;
extern int tp_compatible_priorities;
extern int tp_compatible_muf;
extern int tp_compatible_mpi;
extern int tp_parse_help_as_mpi;
extern int tp_do_mpi_parsing;
extern int tp_look_propqueues;
extern int tp_lock_envcheck;
extern int tp_diskbase_propvals;
extern int tp_idleboot;
extern int tp_playermax;
extern int tp_db_readonly;

extern int tp_building;
extern int tp_restricted_building;
extern int tp_restricted_mpi;
extern int tp_mortal_mpi_listen_props;
extern int tp_date_motd;
extern int tp_author_motd;

extern int tp_building_quotas;
extern int tp_quotas_with_bbits;
extern int tp_space_nag;
extern int tp_user_connect_propqueue;
extern int tp_user_arrive_propqueue;
extern int tp_user_idle_propqueue;
extern int tp_look_on_connect;
extern int tp_show_idlers;
extern int tp_doing_blocks_ads;
extern int tp_doing_blocks_hard;
extern int tp_pause_after_motd;
extern int tp_mortals_need_id_prop;
extern int tp_purify_muf_files;
extern int tp_db_help_first;
extern int tp_server_ansi;
extern int tp_glow_ansi;
extern int tp_tilde_ansi;
extern int tp_rp_who_lists_ic;
extern int tp_ignore_support;
extern int tp_alias_support;
extern int tp_global_aliases;
extern int tp_chat_tokens;
extern int tp_dash_tokens;
extern int tp_default_messages;
extern int tp_multi_wiz_levels;
extern int tp_quiet_moves;
extern int tp_quiet_dark_exits;
extern int tp_pueblo_support;
extern int tp_dump_copies_decay;
extern int tp_unix_login;
extern int tp_save_glow_flags;
extern int tp_gender_commands;
extern int tp_www_player_pages;
extern int tp_restricted_www;
extern int tp_transparent_paths;
extern int tp_exit_guest_flag;
extern int tp_heartbeat;

