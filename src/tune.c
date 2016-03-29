#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include "color.h"
#include "interface.h"
#include "tune.h"
#include "externs.h"

const char *tp_dumpwarn_mesg	= DUMPWARN_MESG;
const char *tp_deltawarn_mesg	= DELTAWARN_MESG;
const char *tp_dumpdeltas_mesg	= DUMPDELTAS_MESG;
const char *tp_dumping_mesg	= DUMPING_MESG;
const char *tp_dumpdone_mesg	= DUMPDONE_MESG;
const char *tp_shutdown_mesg	= SHUTDOWN_MESG;
const char *tp_restart_mesg	= RESTART_MESG;

const char *tp_penny			= PENNY;
const char *tp_pennies			= PENNIES;
const char *tp_cpenny			= CPENNY;
const char *tp_cpennies			= CPENNIES;

const char *tp_muckname			= MUCKNAME;
const char *tp_rwho_passwd		= RWHO_PASSWD;
const char *tp_rwho_server		= RWHO_SERVER;

const char *tp_huh_mesg			= HUH_MESSAGE;
const char *tp_leave_mesg		= LEAVE_MESSAGE;
const char *tp_idle_mesg		= IDLEBOOT_MESSAGE;
const char *tp_register_mesg		= REG_MESSAGE;

const char *tp_playermax_warnmesg	= PLAYERMAX_WARNMESG;
const char *tp_playermax_bootmesg	= PLAYERMAX_BOOTMESG;

const char *tp_path_dir			= PROP_PATHDIR;

const char *tp_html_parent_link		= "/";
const char *tp_pueblo_message		= "<xch_mudtext>";

const char *tp_home_command             = "home";

struct tune_str_entry {
    const char *name;
    const char **str;
    int writemlev;
    int readmlev;
    int isdefault;
};

struct tune_str_entry tune_str_list[] =
{
    {"dumpwarn_mesg",		&tp_dumpwarn_mesg,	LMAGE,	LMUF, 1},
    {"deltawarn_mesg",		&tp_deltawarn_mesg,	LMAGE,	LMUF, 1},
    {"dumpdeltas_mesg",		&tp_dumpdeltas_mesg,	LMAGE,	LMUF, 1},
    {"dumping_mesg",		&tp_dumping_mesg,	LMAGE,	LMUF, 1},
    {"dumpdone_mesg",		&tp_dumpdone_mesg,	LMAGE,	LMUF, 1},
    {"shutdown_mesg",		&tp_shutdown_mesg,	LMAGE,	LMUF, 1},
    {"restart_mesg",		&tp_restart_mesg,	LMAGE,	LMUF, 1},
    {"penny",			&tp_penny,		LMAGE,	LMUF, 1},
    {"pennies",			&tp_pennies,		LMAGE,	LMUF, 1},
    {"cpenny",			&tp_cpenny,		LMAGE,	LMUF, 1},
    {"cpennies",		&tp_cpennies,		LMAGE,	LMUF, 1},
    {"muckname",		&tp_muckname,		LARCH,	LMUF, 1},
    {"rwho_passwd",		&tp_rwho_passwd,	LBOY,	LBOY, 1},
    {"rwho_server",		&tp_rwho_server,	LBOY,	LMAGE, 1},
    {"huh_mesg",		&tp_huh_mesg,		LMAGE,	LMUF, 1},
    {"leave_mesg",		&tp_leave_mesg,		LMAGE,	LMUF, 1},
    {"idle_boot_mesg",		&tp_idle_mesg,		LMAGE,	LMUF, 1},
    {"register_mesg",		&tp_register_mesg,	LWIZ,	LMUF, 1},
    {"playermax_warnmesg",	&tp_playermax_warnmesg,	LMAGE,	LMUF, 1},
    {"playermax_bootmesg",	&tp_playermax_bootmesg,	LMAGE,	LMUF, 1},
    {"path_dir",		&tp_path_dir,		LARCH,	LMUF, 1},
    {"html_parent_link",	&tp_html_parent_link,	LMAGE,	LMUF, 1},
    {"pueblo_message",		&tp_pueblo_message,	LMAGE,	LMUF, 1},
    {"home_command",            &tp_home_command,       LBOY,  LMUF, 1},
    {NULL, NULL, 0, 0}
};



/* times */

time_t tp_mob_interval		= MOB_INTERVAL;
time_t tp_mud_interval		= MUD_INTERVAL;

time_t tp_rwho_interval		= RWHO_INTERVAL;

time_t tp_dump_interval		= DUMP_INTERVAL;
time_t tp_dump_warntime		= DUMP_WARNTIME;
time_t tp_monolithic_interval	= MONOLITHIC_INTERVAL;
time_t tp_clean_interval	= CLEAN_INTERVAL;
time_t tp_aging_time		= AGING_TIME;
time_t tp_maxidle		= MAXIDLE;
time_t tp_idletime		= MAXIDLE;
time_t tp_registration_wait	= TIME_HOUR(1);
time_t tp_connect_wait		= 300; /* 5 minutes */
time_t tp_fail_wait		= 5; /* 5 seconds */


struct tune_time_entry {
    const char *name;
    time_t *tim;
    int writemlev;
    int readmlev;
};

struct tune_time_entry tune_time_list[] =
{
    {"mob_interval",		&tp_mob_interval,	LMAGE,	LMAGE},
    {"mud_interval",		&tp_mud_interval,	LMAGE,	LMAGE},
    {"rwho_interval",		&tp_rwho_interval,	LARCH,	LWIZ},
    {"dump_interval",		&tp_dump_interval,	LWIZ,	LMUF},
    {"dump_warntime",		&tp_dump_warntime,	LWIZ,	LMUF},
    {"monolithic_interval",	&tp_monolithic_interval,LWIZ,	LMUF},
    {"clean_interval",		&tp_clean_interval,	LWIZ,	LMUF},
    {"aging_time",		&tp_aging_time,		LWIZ,	LMUF},
    {"maxidle",			&tp_maxidle,		LMAGE,	LMUF},
    {"idletime",		&tp_idletime,		LMAGE,	LMUF},
    {"reg_wait",		&tp_registration_wait,	LWIZ,	LMUF},
    {"connect_wait",		&tp_connect_wait,	LWIZ,	LMUF},
    {"fail_wait",		&tp_fail_wait,		LWIZ,	LMUF},
    {NULL, NULL, 0}
};



/* integers */
int tp_max_object_endowment	= MAX_OBJECT_ENDOWMENT;
int tp_object_cost		= OBJECT_COST;
int tp_exit_cost		= EXIT_COST;
int tp_link_cost		= LINK_COST;
int tp_room_cost		= ROOM_COST;
int tp_lookup_cost		= LOOKUP_COST;
int tp_max_pennies		= MAX_PENNIES;
int tp_penny_rate		= PENNY_RATE;
int tp_start_pennies		= START_PENNIES;

int tp_kill_base_cost		= KILL_BASE_COST;
int tp_kill_min_cost		= KILL_MIN_COST;
int tp_kill_bonus		= KILL_BONUS;

int tp_command_burst_size	= COMMAND_BURST_SIZE;
int tp_commands_per_time	= COMMANDS_PER_TIME;
int tp_command_time_msec	= COMMAND_TIME_MSEC;

int tp_max_delta_objs		= MAX_DELTA_OBJS;
int tp_max_loaded_objs		= MAX_LOADED_OBJS;
int tp_max_process_limit	= MAX_PROCESS_LIMIT;
int tp_max_plyr_processes	= MAX_PLYR_PROCESSES;
int tp_max_instr_count		= MAX_INSTR_COUNT;
int tp_instr_slice		= INSTR_SLICE;
int tp_mpi_max_commands		= MPI_MAX_COMMANDS;
int tp_pause_min		= PAUSE_MIN;
int tp_free_frames_pool		= FREE_FRAMES_POOL;
int tp_max_output		= MAX_OUTPUT;
int tp_rand_screens		= 0;
int tp_listen_mlev		= LISTEN_MLEV;
int tp_playermax_limit		= PLAYERMAX_LIMIT;
int tp_max_things		= 5;
int tp_max_exits		= 5;
int tp_max_programs		= 10;
int tp_max_rooms		= 250;
int tp_muf_macro_mlevel		= LM3;
int tp_tinkerproof_mlevel	= LPLAYER;
int tp_hidden_prop_mlevel	= LARCH;
int tp_seeonly_prop_mlevel	= LMAGE;
int tp_private_prop_mlevel	= LMAGE;
int tp_quell_ignore_mlevel	= LMAGE;
int tp_muf_proglog_mlevel	= LMAGE;
int tp_path_mlevel		= LMAGE;
int tp_help_mlevel		= LM2;
int tp_muf_mpi_flag_mlevel	= LMAGE;
int tp_newobject_mlevel		= LM1;
int tp_fail_retries		= 3;
int tp_max_player_logins	= 3;
int tp_max_site_welcomes	= 5;
int tp_max_site_lma		= 40;
int tp_max_ignores		= IGNOREMAX;
int tp_www_port			= 0;
int tp_dump_copies		= 0;
int tp_read_bytes		= 0;
int tp_write_bytes		= 0;

struct tune_val_entry {
    const char *name;
    int *val;
    int writemlev;
    int readmlev;
};

struct tune_val_entry tune_val_list[] =
{
    {"max_object_endowment",	&tp_max_object_endowment,LMAGE,	LMUF},
    {"object_cost",		&tp_object_cost,	LMAGE,	LMUF},
    {"exit_cost",		&tp_exit_cost,		LMAGE,	LMUF},
    {"link_cost",		&tp_link_cost,		LMAGE,	LMUF},
    {"room_cost",		&tp_room_cost,		LMAGE,	LMUF},
    {"lookup_cost",		&tp_lookup_cost,	LMAGE,	LMUF},
    {"max_pennies",		&tp_max_pennies,	LMAGE,	LMUF},
    {"penny_rate",		&tp_penny_rate,		LMAGE,	LMUF},
    {"start_pennies",		&tp_start_pennies,	LMAGE,	LMUF},

    {"kill_base_cost",		&tp_kill_base_cost,	LMAGE,	LMUF},
    {"kill_min_cost",		&tp_kill_min_cost,	LMAGE,	LMUF},
    {"kill_bonus",		&tp_kill_bonus,		LMAGE,	LMUF},

    {"command_burst_size",	&tp_command_burst_size,	LARCH,	LMUF},
    {"commands_per_time",	&tp_commands_per_time,	LARCH,	LMUF},
    {"command_time_msec",	&tp_command_time_msec,	LARCH,	LMUF},

    {"max_delta_objs",		&tp_max_delta_objs,	LARCH,	LMUF},
    {"max_loaded_objs",		&tp_max_loaded_objs,	LARCH,	LMUF},
    {"max_process_limit",	&tp_max_process_limit,	LARCH,	LMUF},
    {"max_plyr_processes",	&tp_max_plyr_processes,	LARCH,	LMUF},
    {"max_instr_count",		&tp_max_instr_count,	LARCH,	LMUF},
    {"instr_slice",		&tp_instr_slice,	LARCH,	LMUF},
    {"mpi_max_commands",	&tp_mpi_max_commands,	LARCH,	LMUF},
    {"pause_min",		&tp_pause_min,		LARCH,	LMUF},
    {"free_frames_pool",	&tp_free_frames_pool,	LARCH,	LMUF},
    {"max_output",		&tp_max_output,		LARCH,	LMUF},
    {"rand_screens",		&tp_rand_screens,	LARCH,	LMUF},

    {"listen_mlev",		&tp_listen_mlev,	LARCH,	LMUF},
    {"playermax_limit",		&tp_playermax_limit,	LMAGE,	LMUF},

    {"max_rooms",		&tp_max_rooms,		LMAGE,	LMUF},
    {"max_exits",		&tp_max_exits,		LMAGE,	LMUF},
    {"max_things",		&tp_max_things,		LMAGE,	LMUF},
    {"max_programs",		&tp_max_programs,	LMAGE,	LMUF},
    {"muf_macro_mlevel",	&tp_muf_macro_mlevel,	LBOY,	LMUF},
    {"tinkerproof_mlevel",	&tp_tinkerproof_mlevel,	LBOY,	LMUF},
    {"hidden_prop_mlevel",	&tp_hidden_prop_mlevel,	LBOY,	LMAGE},
    {"seeonly_prop_mlevel",	&tp_seeonly_prop_mlevel,LBOY,	LMUF},
    {"private_prop_mlevel",	&tp_private_prop_mlevel,LBOY,	LMUF},
    {"quell_ignore_mlevel",	&tp_quell_ignore_mlevel,LBOY,	LMUF},
    {"muf_proglog_mlevel",	&tp_muf_proglog_mlevel,	LBOY,	LMUF},
    {"path_mlevel",		&tp_path_mlevel,	LBOY,	LMUF},
    {"help_mlevel",		&tp_help_mlevel,	LBOY,	LMUF},
    {"muf_mpi_flag_mlevel",	&tp_muf_mpi_flag_mlevel,LBOY,	LMAGE},
    {"newobject_mlevel",	&tp_newobject_mlevel,	LARCH,	LMUF},
    {"fail_retries",		&tp_fail_retries,	LARCH,	LMUF},
    {"max_player_logins",	&tp_max_player_logins,	LMAGE,	LMUF},
    {"max_site_welcomes",	&tp_max_site_welcomes,	LMAGE,  LMUF},
    {"max_site_lma",		&tp_max_site_lma,	LARCH,  LMAGE},
    {"max_ignores",		&tp_max_ignores,	LMAGE,	LMUF},
    {"www_port",		&tp_www_port,		LARCH,	LMUF},
    {"dump_copies",		&tp_dump_copies,	LBOY,	LMAGE},
    {"read_bytes",		&tp_read_bytes,		LARCH,	LMUF},
    {"write_bytes",		&tp_write_bytes,	LARCH,	LMUF},
    {NULL, NULL, 0}
};




/* dbrefs */
dbref tp_player_start	= 0;		/* This MUST be zero to start with */
dbref tp_environment_room = GLOBAL_ENVIRONMENT;
dbref tp_reg_wiz	= NOTHING;
dbref tp_path_prog	= NOTHING;
dbref tp_jail_room	= NOTHING;

dbref tp_home_room	= NOTHING;
dbref tp_offered_room	= NOTHING;
dbref tp_dead_room	= NOTHING;

dbref tp_www_root	= NOTHING;
dbref tp_www_user	= NOTHING;
dbref tp_heartbeat_user = NOTHING;

struct tune_ref_entry {
    const char *name;
    int typ;
    dbref *ref;
    int writemlev;
    int readmlev;
};

struct tune_ref_entry tune_ref_list[] =
{
    {"player_start",	TYPE_ROOM,	&tp_player_start,LWIZ,	LMUF},
    {"environment_room",TYPE_ROOM,	&tp_environment_room,LWIZ,	LMUF},
    {"reg_wiz",		TYPE_PLAYER,	&tp_reg_wiz,		LWIZ,	LWIZ},
    {"path_prog",	TYPE_PROGRAM,	&tp_path_prog,		LARCH,	LMUF},
    {"jail_room",	TYPE_ROOM,	&tp_jail_room,		LMAGE,	LMAGE},
    {"home_room",	TYPE_ROOM,	&tp_home_room,		LMAGE,	LMAGE},
    {"offered_room",	TYPE_ROOM,	&tp_offered_room,	LMAGE,	LMAGE},
    {"dead_room",	TYPE_ROOM,	&tp_dead_room,		LMAGE,	LMAGE},
    {"www_root",	TYPE_ROOM,	&tp_www_root,		LARCH,	LMUF},
    {"www_user",	TYPE_PLAYER,	&tp_www_user,		LARCH,	LMUF},
    {"heartbeat_user",  TYPE_PLAYER,    &tp_heartbeat_user,     LARCH,  LMAGE},
    {NULL, 0, NULL, 0}
};


/* booleans */
int tp_hostnames 		= HOSTNAMES;
int tp_log_commands 		= LOG_COMMANDS;
int tp_log_interactive		= LOG_INTERACTIVE;
int tp_log_wizards		= LOG_WIZARDS;
int tp_log_connects		= LOG_CONNECTS;
int tp_log_http			= LOG_WWW_ACTIVITY;
int tp_log_mud_commands		= LOG_MUD_COMMANDS;
int tp_log_failed_commands 	= LOG_FAILED_COMMANDS;
int tp_log_programs 		= LOG_PROGRAMS;
int tp_log_guests		= LOG_GUESTS;
int tp_log_with_names		= 0;
int tp_dbdump_warning 		= DBDUMP_WARNING;
int tp_deltadump_warning 	= DELTADUMP_WARNING;
int tp_periodic_program_purge 	= PERIODIC_PROGRAM_PURGE;
int tp_mob			= 0;
int tp_mud			= 0;
int tp_rwho 			= 0;
int tp_secure_who 		= SECURE_WHO;
int tp_who_doing 		= WHO_DOING;
int tp_realms_control 		= REALMS_CONTROL;
int tp_listeners 		= LISTENERS;
int tp_listeners_obj 		= LISTENERS_OBJ;
int tp_listeners_env 		= LISTENERS_ENV;
int tp_zombies 			= ZOMBIES;
int tp_wiz_vehicles 		= WIZ_VEHICLES;
int tp_wiz_name			= 0;
int tp_recycle_frobs		= 0;
int tp_m1_name_notify		= M1_NAME_NOTIFY;
int tp_restrict_kill 		= RESTRICT_KILL;
int tp_registration 		= REGISTRATION;
int tp_online_registration	= 1;
int tp_fast_registration	= 0;
int tp_validate_hostname	= 0;
int tp_validate_warning		= 1;
int tp_teleport_to_player 	= TELEPORT_TO_PLAYER;
int tp_secure_teleport 		= SECURE_TELEPORT;
int tp_exit_darking 		= EXIT_DARKING;
int tp_thing_darking 		= THING_DARKING;
int tp_dark_sleepers 		= DARK_SLEEPERS;
int tp_who_hides_dark 		= WHO_HIDES_DARK;
int tp_compatible_priorities 	= COMPATIBLE_PRIORITIES;
int tp_compatible_muf		= 0;
int tp_compatible_mpi		= 0;
int tp_parse_help_as_mpi	= 0;
int tp_do_mpi_parsing 		= DO_MPI_PARSING;
int tp_look_propqueues		= LOOK_PROPQUEUES;
int tp_lock_envcheck		= LOCK_ENVCHECK;
int tp_diskbase_propvals	= DISKBASE_PROPVALS;
int tp_idleboot			= IDLEBOOT;
int tp_playermax		= PLAYERMAX;
int tp_db_readonly		= 0;
int tp_building			= 1;
int tp_restricted_building	= 0;
int tp_restricted_mpi		= 0;
int tp_mortal_mpi_listen_props	= 0;
int tp_date_motd		= 1;
int tp_author_motd		= 0;
int tp_building_quotas		= 0;
int tp_quotas_with_bbits	= 0;
int tp_space_nag		= 0;
int tp_user_connect_propqueue	= 1;
int tp_user_arrive_propqueue	= 1;
int tp_look_on_connect		= 1;
int tp_show_idlers		= 0;
int tp_user_idle_propqueue	= 0;
int tp_doing_blocks_ads		= 0;
int tp_doing_blocks_hard	= 0;
int tp_pause_after_motd		= 0;
int tp_mortals_need_id_prop	= 0;
int tp_purify_muf_files		= 1;
int tp_db_help_first		= 0;
int tp_server_ansi		= 1;
int tp_glow_ansi		= 1;
int tp_tilde_ansi		= 1;
int tp_rp_who_lists_ic		= 0;
int tp_ignore_support		= 1;
int tp_alias_support		= 1;
int tp_global_aliases		= 0;
int tp_chat_tokens		= 1;
int tp_dash_tokens		= 0;
int tp_default_messages		= 0;
int tp_multi_wiz_levels		= 0;
int tp_quiet_moves		= 0;
int tp_quiet_dark_exits		= 0;
int tp_pueblo_support		= 0;
int tp_dump_copies_decay	= 0;
int tp_unix_login		= 0;
int tp_save_glow_flags		= 1;
int tp_gender_commands		= 1;
int tp_www_player_pages		= 1;
int tp_restricted_www		= 0;
int tp_transparent_paths	= 1;
int tp_exit_guest_flag          = 0;
int tp_heartbeat                = 0;
int tp_division_by_zero_error = 0;

struct tune_bool_entry {
    const char *name;
    int* booleanEntry;
    int writemlev;
    int readmlev;
};

struct tune_bool_entry tune_bool_list[] =
{
    {"use_hostnames",		&tp_hostnames,		LARCH,	LMAGE},
    {"log_commands",		&tp_log_commands,	LBOY,	LMAGE},
    {"log_interactive",		&tp_log_interactive,	LARCH,	LMAGE},
    {"log_wizards",		&tp_log_wizards,	LBOY,	LBOY},
    {"log_www_activity",	&tp_log_http,		LARCH,	LMAGE},
    {"log_connects",		&tp_log_connects,	LARCH,	LMAGE},
    {"log_mud_commands",	&tp_log_mud_commands,	LMAGE,	LMAGE},
    {"log_failed_commands",	&tp_log_failed_commands,LARCH,	LMAGE},
    {"log_programs",		&tp_log_programs,	LARCH,	LMAGE},
    {"log_guests",		&tp_log_guests,		LARCH,	LMAGE},
    {"log_with_names",		&tp_log_with_names,	LBOY,	LMAGE},
    {"dbdump_warning",		&tp_dbdump_warning,	LMAGE,	LMUF},
    {"deltadump_warning",	&tp_deltadump_warning,	LMAGE,	LMUF},
    {"periodic_program_purge",	&tp_periodic_program_purge,LMAGE,LMAGE},

    {"mobile_support",		&tp_mob,		LMAGE,	LMAGE},
    {"mud_support",		&tp_mud,		LMAGE,	LMAGE},

    {"run_rwho",		&tp_rwho,		LARCH,	LWIZ},

    {"secure_who",		&tp_secure_who,		LWIZ,	LMUF},
    {"who_doing",		&tp_who_doing,		LWIZ,	LMUF},
    {"realms_control",		&tp_realms_control,	LWIZ,	LMUF},
    {"allow_listeners",		&tp_listeners,		LARCH,	LMUF},
    {"allow_listeners_obj",	&tp_listeners_obj,	LARCH,	LMUF},
    {"allow_listeners_env",	&tp_listeners_env,	LARCH,	LMUF},
    {"allow_zombies",		&tp_zombies,		LARCH,	LMUF},
    {"wiz_vehicles",		&tp_wiz_vehicles,	LARCH,	LMUF},
    {"wiz_name",		&tp_wiz_name,		LARCH,	LMUF},
    {"recycle_frobs",		&tp_recycle_frobs,	LARCH,	LMUF},
    {"m1_name_notify",		&tp_m1_name_notify,	LWIZ,	LMUF},
    {"restrict_kill",		&tp_restrict_kill,	LMAGE,	LMUF},
    {"registration",		&tp_registration,	LWIZ,	LMAGE},
    {"online_registration",	&tp_online_registration,LWIZ,	LMAGE},
    {"fast_registration",	&tp_fast_registration,	LWIZ,	LMAGE},
    {"validate_hostname",	&tp_validate_hostname,	LWIZ,	LMAGE},
    {"validate_warning",	&tp_validate_warning,	LWIZ,	LMAGE},
    {"teleport_to_player",	&tp_teleport_to_player,	LWIZ,	LMUF},
    {"secure_teleport",		&tp_secure_teleport,	LWIZ,	LMUF},
    {"exit_darking",		&tp_exit_darking,	LARCH,	LMUF},
    {"thing_darking",		&tp_thing_darking,	LARCH,	LMUF},
    {"dark_sleepers",		&tp_dark_sleepers,	LARCH,	LMUF},
    {"who_hides_dark",		&tp_who_hides_dark,	LARCH,	LMUF},
    {"compatible_priorities",	&tp_compatible_priorities,LARCH,LMUF},
    {"compatible_muf",		&tp_compatible_muf,	LBOY,	LMUF},
    {"compatible_mpi",		&tp_compatible_mpi,	LBOY,	LMUF},
    {"parse_help_as_mpi",	&tp_parse_help_as_mpi,	LBOY,	LMUF},
    {"do_mpi_parsing",		&tp_do_mpi_parsing,	LARCH,	LMUF},
    {"look_propqueues",		&tp_look_propqueues,	LARCH,	LMUF},
    {"lock_envcheck",		&tp_lock_envcheck,	LARCH,	LMUF},
    {"diskbase_propvals",	&tp_diskbase_propvals,	LBOY,	LMAGE},
    {"idleboot",		&tp_idleboot,		LMAGE,	LMUF},
    {"playermax",		&tp_playermax,		LMAGE,	LMUF},
    {"db_readonly",		&tp_db_readonly,	LARCH,	LMUF},

    {"building",		&tp_building,		LWIZ,	LMUF},
    {"restricted_building",	&tp_restricted_building,LWIZ,	LMUF},
    {"restricted_mpi",		&tp_restricted_mpi,	LWIZ,	LMUF},
    {"mortal_mpi_listen_props", &tp_mortal_mpi_listen_props,LARCH,LMUF},
    {"date_motd",		&tp_date_motd,		LMAGE,	LMUF},
    {"author_motd",		&tp_author_motd,	LMAGE,	LMUF},
    {"building_quotas",		&tp_building_quotas,	LWIZ,	LMUF},
    {"quotas_with_bbits",	&tp_quotas_with_bbits,	LWIZ,	LMUF},
    {"space_nag",		&tp_space_nag,		LMAGE,	LMUF},
    {"user_connect_propqueue",	&tp_user_connect_propqueue,LARCH,LMUF},
    {"user_arrive_propqueue",	&tp_user_arrive_propqueue,LARCH,LMUF},
    {"user_idle_propqueue",	&tp_user_idle_propqueue,LARCH,	LMUF},
    {"look_on_connect",		&tp_look_on_connect,	LARCH,	LMUF},
    {"show_idlers",		&tp_show_idlers,	LMAGE,	LMUF},
    {"doing_blocks_ads",	&tp_doing_blocks_ads,	LMAGE,	LMUF},
    {"doing_blocks_hard",	&tp_doing_blocks_hard,	LMAGE,	LMUF},
    {"pause_after_motd",	&tp_pause_after_motd,	LMAGE,	LMUF},
    {"mortals_need_id_prop",	&tp_mortals_need_id_prop,LARCH,	LMUF},
    {"purify_muf_files",	&tp_purify_muf_files,	LBOY,	LMUF},
    {"db_help_first",		&tp_db_help_first,	LARCH,	LMUF},
    {"server_ansi",		&tp_server_ansi,	LBOY,	LMUF},
    {"glow_ansi",		&tp_glow_ansi,		LBOY,	LMUF},
    {"tilde_ansi",		&tp_tilde_ansi,		LBOY,	LMUF},
    {"rp_who_lists_ic",		&tp_rp_who_lists_ic,	LARCH,	LMUF},
    {"ignore_support",		&tp_ignore_support,	LBOY,	LMUF},
    {"alias_support",		&tp_alias_support,	LBOY,	LMUF},
    {"global_aliases",		&tp_global_aliases,	LBOY,	LMUF},
    {"chat_tokens",		&tp_chat_tokens,	LBOY,	LMUF},
    {"dash_tokens",		&tp_dash_tokens,	LBOY,	LMUF},
    {"default_messages",	&tp_default_messages,	LBOY,	LMUF},
    {"multi_wiz_levels",	&tp_multi_wiz_levels,	LMAN,	LMUF},
    {"quiet_moves",		&tp_quiet_moves,	LARCH,	LMUF},
    {"quiet_dark_exits",	&tp_quiet_dark_exits,	LARCH,	LMUF},
    {"pueblo_support",		&tp_pueblo_support,	LARCH,	LMUF},
    {"dump_copies_decay",	&tp_dump_copies_decay,	LBOY,	LMAGE},
    {"unix_login",		&tp_unix_login,		LBOY,	LMAGE},
    {"save_glow_flags",		&tp_save_glow_flags,	LMAN,	LMAGE},
    {"gender_commands",		&tp_gender_commands,	LARCH,	LMUF},
    {"www_player_pages",	&tp_www_player_pages,	LARCH,	LMUF},
    {"restricted_www",		&tp_restricted_www,	LARCH,	LMUF},
    {"transparent_paths",	&tp_transparent_paths,	LARCH,	LMUF},
    {"exit_guest_flag",         &tp_exit_guest_flag,    LMAGE,  LMUF},
    {"heartbeat",               &tp_heartbeat,          LBOY,   LMUF},
		{"division_by_zero_error", &tp_division_by_zero_error, LBOY, LMUF},
    {NULL, NULL, 0}
};


static const char *
timestr_full(time_t dtime)
{
    static char buf[32];
    int days, hours, minutes;

    days = dtime / 86400;
    dtime %= 86400;
    hours = dtime / 3600;
    dtime %= 3600;
    minutes = dtime / 60;
    dtime %= 60;

    sprintf(buf, "%3dd %2d:%02d:%02d", days, hours, minutes, (int)dtime);

    return buf;
}


int
tune_count_parms(void)
{
    int total = 0;
    struct tune_str_entry *tstr = tune_str_list;
    struct tune_time_entry *ttim = tune_time_list;
    struct tune_val_entry *tval = tune_val_list;
    struct tune_ref_entry *tref = tune_ref_list;
    struct tune_bool_entry *tbool = tune_bool_list;

    while ((tstr++)->name) total++;
    while ((ttim++)->name) total++;
    while ((tval++)->name) total++;
    while ((tref++)->name) total++;
    while ((tbool++)->name) total++;

    return total;
}


void
tune_display_parms(dbref player, char *name)
{
    int total=0;
    const char *lastname = NULL;
    char buf[BUFFER_LEN + 50], tbuf[BUFFER_LEN];
    struct tune_str_entry *tstr = tune_str_list;
    struct tune_time_entry *ttim = tune_time_list;
    struct tune_val_entry *tval = tune_val_list;
    struct tune_ref_entry *tref = tune_ref_list;
    struct tune_bool_entry *tbool = tune_bool_list;

    while (tstr->name) {
	strcpy(buf, tstr->name);
	if ((MLevel(OWNER(player)) >= tstr->readmlev) &&
		(!*name || equalstr(name, buf))
	) {
	    sprintf(buf, CAQUA "(str)  " CRED "%c" CGREEN "%-24s" CRED " = " CCYAN "%.4096s",
		(WLevel(OWNER(player)) >= tstr->writemlev) ? '*' : ' ',
		tstr->name,
		tct(*tstr->str,tbuf)
	    );
	    lastname = tstr->name;
	    anotify(player, buf);
	    total++;
	}
	tstr++;
    }

    while (ttim->name) {
	strcpy(buf, ttim->name);
	if ((MLevel(OWNER(player)) >= ttim->readmlev) &&
		(!*name || equalstr(name, buf))
	) {
	    sprintf(buf, CVIOLET "(time) " CRED "%c" CGREEN "%-24s" CRED " = " CPURPLE "%s",
		(WLevel(OWNER(player)) >= ttim->writemlev) ? '*' : ' ',
		ttim->name,
		timestr_full(*ttim->tim)
	    );
	    lastname = ttim->name;
	    anotify(player, buf);
	    total++;
	}
	ttim++;
    }

    while (tval->name) {
	strcpy(buf, tval->name);
	if ((MLevel(OWNER(player)) >= tval->readmlev) &&
		(!*name || equalstr(name, buf))
	) {
	    sprintf(buf, CFOREST "(int)  " CRED "%c" CGREEN "%-24s" CRED " = " CYELLOW "%d",
		(WLevel(OWNER(player)) >= tval->writemlev) ? '*' : ' ',
		tval->name,
		*tval->val
	    );
	    lastname = tval->name;
	    anotify(player, buf);
	    total++;
	}
	tval++;
    }

    while (tref->name) {
	strcpy(buf, tref->name);
	if ((MLevel(OWNER(player)) >= tref->readmlev) &&
		(!*name || equalstr(name, buf))
	) {
	    sprintf(buf, CBROWN "(ref)  " CRED "%c" CGREEN "%-24s" CRED " = %s",
		(WLevel(OWNER(player)) >= tref->writemlev) ? '*' : ' ',
		tref->name,
		ansi_unparse_object(player, *tref->ref)
	    );
	    lastname = tref->name;
	    anotify(player, buf);
	    total++;
	}
	tref++;
    }

    while (tbool->name) {
	strcpy(buf, tbool->name);
	if ((MLevel(OWNER(player)) >= tbool->readmlev) &&
		(!*name || equalstr(name, buf))
	) {
	    sprintf(buf, CWHITE "(bool) " CRED "%c" CGREEN "%-24s" CRED " = " CBLUE "%s",
		(WLevel(OWNER(player)) >= tbool->writemlev) ? '*' : ' ',
		tbool->name,
		((*tbool->booleanEntry)? "yes" : "no")
	    );
	    lastname = tbool->name;
	    anotify(player, buf);
	    total++;
	}
	tbool++;
    }
    if((total == 1) && lastname && *lastname) {
	do_sysparm(player, lastname);
    } else {
	anotify_fmt(player, CINFO "%d sysparm%s listed.", total,
	    (total==1) ? "" : "s"
	);
    }
}


void
tune_save_parms_to_file(FILE *f)
{
    struct tune_str_entry *tstr = tune_str_list;
    struct tune_time_entry *ttim = tune_time_list;
    struct tune_val_entry *tval = tune_val_list;
    struct tune_ref_entry *tref = tune_ref_list;
    struct tune_bool_entry *tbool = tune_bool_list;

    while (tstr->name) {
	fprintf(f, "%s=%.4096s\n", tstr->name, *tstr->str);
	tstr++;
    }

    while (ttim->name) {
	fprintf(f, "%s=%s\n", ttim->name, timestr_full(*ttim->tim));
	ttim++;
    }

    while (tval->name) {
	fprintf(f, "%s=%d\n", tval->name, *tval->val);
	tval++;
    }

    while (tref->name) {
	fprintf(f, "%s=#%d\n", tref->name, *tref->ref);
	tref++;
    }

    while (tbool->name) {
	fprintf(f, "%s=%s\n", tbool->name, (*tbool->booleanEntry)? "yes" : "no");
	tbool++;
    }
}

void
tune_save_parmsfile(void)
{
    FILE   *f;

    f = fopen(PARMFILE_NAME, "wb");
    if (!f) {
	log_status("TUNE: Couldn't open %s\n", PARMFILE_NAME);
	return;
    }

    tune_save_parms_to_file(f);

    fclose(f);
}



const char *
tune_get_parmstring(const char *name, int mlev)
{
    static char buf[BUFFER_LEN + 50];
    struct tune_str_entry *tstr = tune_str_list;
    struct tune_time_entry *ttim = tune_time_list;
    struct tune_val_entry *tval = tune_val_list;
    struct tune_ref_entry *tref = tune_ref_list;
    struct tune_bool_entry *tbool = tune_bool_list;

    while (tstr->name) {
	if (!string_compare(name, tstr->name)) {
	    if (tstr->readmlev > mlev) return "";
	    return (*tstr->str);
	}
	tstr++;
    }

    while (ttim->name) {
	if (!string_compare(name, ttim->name)) {
	    if (ttim->readmlev > mlev) return "";
	    sprintf(buf, "%d", (int)*ttim->tim);
	    return (buf);
	}
	ttim++;
    }

    while (tval->name) {
	if (!string_compare(name, tval->name)) {
	    if (tval->readmlev > mlev) return "";
	    sprintf(buf, "%d", *tval->val);
	    return (buf);
	}
	tval++;
    }

    while (tref->name) {
	if (!string_compare(name, tref->name)) {
	    if (tref->readmlev > mlev) return "";
	    sprintf(buf, "#%d", *tref->ref);
	    return (buf);
	}
	tref++;
    }

    while (tbool->name) {
	if (!string_compare(name, tbool->name)) {
	    if (tbool->readmlev > mlev) return "";
	    sprintf(buf, "%s", ((*tbool->booleanEntry)? "yes" : "no"));
	    return (buf);
	}
	tbool++;
    }

    /* Parameter was not found.  Return null string. */
    strcpy(buf, "");
    return (buf);
}



/* return values:
 *  0 for success.
 *  1 for unrecognized parameter name.
 *  2 for bad parameter value syntax.
 *  3 for bad parameter value.
 */

#define TUNESET_SUCCESS 0
#define TUNESET_UNKNOWN 1
#define TUNESET_SYNTAX 2
#define TUNESET_BADVAL 3
#define TUNESET_NOPERM 4

int
tune_setparm(const dbref player, const char *parmname, const char *val)
{
    struct tune_str_entry *tstr = tune_str_list;
    struct tune_time_entry *ttim = tune_time_list;
    struct tune_val_entry *tval = tune_val_list;
    struct tune_ref_entry *tref = tune_ref_list;
    struct tune_bool_entry *tbool = tune_bool_list;
    char buf[BUFFER_LEN];
    char *parmval;

    strcpy(buf, val);
    parmval = buf;

    while (tstr->name) {
	if (!string_compare(parmname, tstr->name)) {

	    if ((player != NOTHING) && (WLevel(OWNER(player)) < tstr->writemlev))
		return TUNESET_NOPERM;

	    if (!tstr->isdefault) free((char *) *tstr->str);
	    if (*parmval == '-') parmval++;
	    *tstr->str = string_dup(parmval);
	    tstr->isdefault = 0;
	    return TUNESET_SUCCESS;
	}
	tstr++;
    }

    while (ttim->name) {
	if (!string_compare(parmname, ttim->name)) {
	    int days, hrs, mins, secs, result;
	    char *end;

	    if ((player != NOTHING) && (WLevel(OWNER(player)) < ttim->writemlev))
		return TUNESET_NOPERM;

	    days = hrs = mins = secs = 0;
	    end = parmval + strlen(parmval) - 1;
	    switch (*end) {
		case 's':
		case 'S':
		    *end = '\0';
		    if (!number(parmval)) return TUNESET_SYNTAX;
		    secs = atoi(parmval);
		    break;
		case 'm':
		case 'M':
		    *end = '\0';
		    if (!number(parmval)) return TUNESET_SYNTAX;
		    mins = atoi(parmval);
		    break;
		case 'h':
		case 'H':
		    *end = '\0';
		    if (!number(parmval)) return TUNESET_SYNTAX;
		    hrs = atoi(parmval);
		    break;
		case 'd':
		case 'D':
		    *end = '\0';
		    if (!number(parmval)) return TUNESET_SYNTAX;
		    days = atoi(parmval);
		    break;
		default:
		    result = sscanf(parmval, "%dd %2d:%2d:%2d",
				    &days, &hrs, &mins, &secs);
		    if (result != 4) return TUNESET_SYNTAX;
		    break;
	    }
	    *ttim->tim = (days * 86400) + (3600 * hrs) + (60 * mins) + secs;
	    return TUNESET_SUCCESS;
	}
	ttim++;
    }

    while (tval->name) {
	if (!string_compare(parmname, tval->name)) {

	    if ((player != NOTHING) && (WLevel(OWNER(player)) < tval->writemlev))
		return TUNESET_NOPERM;

	    if (!number(parmval)) return TUNESET_SYNTAX;
	    *tval->val = atoi(parmval);
	    return TUNESET_SUCCESS;
	}
	tval++;
    }

    while (tref->name) {
	if (!string_compare(parmname, tref->name)) {
	    dbref obj;

	    if ((player != NOTHING) && (WLevel(OWNER(player)) < tref->writemlev))
		return TUNESET_NOPERM;

	    if( !strcmp( parmval, "me") )
		obj = player;

	    else if( !strcmp( parmval, "nothing") )
		obj = NOTHING;
#ifndef SANITY
	    else if( *parmval == '*' )
		obj = lookup_player( parmval + 1 );
#endif
	    else {
		if (*parmval != '#') return TUNESET_SYNTAX;
		if (!number(parmval+1)) return TUNESET_SYNTAX;
		obj = (dbref) atoi(parmval+1);
	    }
	    if ((obj < NOTHING) || (obj >= db_top)) return TUNESET_SYNTAX;
	    if ((obj != NOTHING) && (tref->typ != NOTYPE) &&
	     (Typeof(obj) != tref->typ))
		return TUNESET_BADVAL;
	    *tref->ref = obj;
	    return TUNESET_SUCCESS;
	}
	tref++;
    }

    while (tbool->name) {
	if (!string_compare(parmname, tbool->name)) {

	    if ((player != NOTHING) && (WLevel(OWNER(player)) < tbool->writemlev))
		return TUNESET_NOPERM;

	    if (*parmval == 'y' || *parmval == 'Y') {
		*tbool->booleanEntry = 1;
	    } else if (*parmval == 'n' || *parmval == 'N') {
		*tbool->booleanEntry = 0;
	    } else {
		return TUNESET_SYNTAX;
	    }
	    return TUNESET_SUCCESS;
	}
	tbool++;
    }

    return TUNESET_UNKNOWN;
}

void
tune_load_parms_from_file(FILE *f, dbref player, int cnt)
{
    char buf[BUFFER_LEN];
    char *c, *p;
    int result = 0;

    while (!feof(f) && (cnt<0 || cnt--)) {
      if (!fgets(buf, sizeof(buf), f)) {
	fprintf(stderr, "tune_load_parms_from_file: fgets failed.\n");
      }
	if (*buf != '#') {
	    p = c = index(buf, '=');
	    if (c) {
		*c++ = '\0';
		while (p > buf && isspace(*(--p))) *p = '\0';
		while(isspace(*c)) c++;
		for (p = c; *p && *p != '\n'; p++);
		*p = '\0';
		for (p = buf; isspace(*p); p++);
		if (*p) {
		    result = tune_setparm(player, p, c);
		}
		switch (result) {
		    case TUNESET_SUCCESS:
			strcat(p, ": Parameter set.");
			break;
		    case TUNESET_UNKNOWN:
			strcat(p, ": Unknown parameter.");
			break;
		    case TUNESET_SYNTAX:
			strcat(p, ": Bad parameter syntax.");
			break;
		    case TUNESET_BADVAL:
			strcat(p, ": Bad parameter value.");
			break;
		}
		if (result && player != NOTHING) notify(player, p);
	    }
	}
    }
}

void
tune_load_parmsfile(dbref player)
{
    FILE   *f;

    f = fopen(PARMFILE_NAME, "rb");
    if (!f) {
	log_status("TUNE: Couldn't open %s\n", PARMFILE_NAME);
	return;
    }

    tune_load_parms_from_file(f, player, -1);

    fclose(f);
}


void
do_tune(dbref player, char *parmname, char *parmval)
{
    int result;

    if (!Mucker(player)) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (!Arch(player) && *parmval) {
	anotify(player, CFAIL NOPERM_MESG);
	return;
    }

    if (*parmname && *parmval) {
	result = tune_setparm( player, parmname, parmval);
	switch (result) {
	    case TUNESET_SUCCESS:
		log_status("TUNE: %s tuned %s to %s\n",
			    unparse_object(MAN, player), parmname, parmval);
		tune_display_parms(player, parmname);
		anotify(player, CSUCC "Parameter set.");
		break;
	    case TUNESET_UNKNOWN:
		anotify(player, CINFO "Unknown parameter.");
		break;
	    case TUNESET_NOPERM:
		anotify(player, CFAIL NOPERM_MESG);
		break;
	    case TUNESET_SYNTAX:
		anotify(player, CFAIL "Bad parameter syntax.");
		break;
	    case TUNESET_BADVAL:
		anotify(player, CFAIL "Bad parameter value.");
		break;
	}
	return;
    } else if (*parmname) {
	/* if (!string_compare(parmname, "save")) {
	    tune_save_parmsfile();
	    anotify(player, CSUCC "Saved parameters to configuration file.");
	} else if (!string_compare(parmname, "load")) {
	    tune_load_parmsfile(player);
	    anotify(player, CSUCC "Restored parameters from configuration file.");
	} else
	*/ {
	    tune_display_parms(player, parmname);
	}
	return;
    } else if (!*parmval && !*parmname) {
	tune_display_parms(player, parmname);
    } else {
	anotify(player, CINFO "Tune what?");
	return;
    }
}
