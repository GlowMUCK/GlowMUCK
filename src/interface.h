#include "copyright.h"
#include "db.h"

/* structures */

struct text_block {
    int     nchars;
    struct text_block *nxt;
    char   *start;
    char   *buf;
};

struct text_queue {
    int     lines;
    struct text_block *head;
    struct text_block **tail;
};

struct descriptor_data {
    int     descriptor;
    int     connected;
    int     booted;
    int	    fails;
    dbref   player;
    char   *output_prefix;
    char   *output_suffix;
    int     output_size;
    struct text_queue output;
    struct text_queue input;
    char   *raw_input;
    char   *raw_input_at;
    time_t  last_time;
    time_t  last_fail;
    time_t  connected_at;
    int hostaddr;
    int	port;       /* Actual port used for communication */
    int listener_port; /* The port the client initially connected to */
    const char *hostname;
    const char *username;
    int quota;
    int	commands;
    int linelen;
    int moreline;
    int prompted;
    int type;
    int idle;
    int resolved;
#ifdef HTTPD
 #ifdef HTTPDELAY
    const char *httpdata;
 #endif
#endif
    struct descriptor_data *next;
    struct descriptor_data **prev;
};

#define TELNET_ECHO_ON	"\377\374\001"
		/* IAC WONT TELOPT_ECHO */
#define TELNET_ECHO_OFF	"\377\373\001"
		/* IAC WILL TELOPT_ECHO */

#define Min(x,y) ((x < y) ? x : y)

#define CT_MUCK		0
#define CT_HTML		1
#define CT_PUEBLO	2

/* these symbols must be defined by the interface */

#ifdef HTTPD
extern void httpd(struct descriptor_data *d, const char *name, const char *http);
extern void httpd_unknown(struct descriptor_data *d, const char *name);
extern int httpd_get_lsedit(struct descriptor_data *d, dbref who, const char *prop);
extern void httpd_get(struct descriptor_data *d, const char *name, const char *http);
#endif

#ifdef RWHO
extern void update_rwho(void);
#endif

#define anotify(x,y) ansi_notify((x),(y),2)
#define anotify_nolisten(x,y,z) ansi_notify_nolisten((x),(y),(z),2)
#define anotify_from(x,y,z) ansi_notify_from((x),(y),(z),2)
#define anotify_from_echo(w,x,y,z) ansi_notify_from_echo((w),(x),(y),(z),2)

extern int notify(dbref player, const char *msg);
extern int notify_nolisten(dbref player, const char *msg, int ispriv);
extern int notify_descriptor(int c, const char *msg);
extern int ansi_notify(dbref player, const char *msg, int parseansi);
extern int queue_ansi(struct descriptor_data *d, const char *s);
extern int queue_string(struct descriptor_data *d, const char *s);
extern int ansi_notify_nolisten(dbref player, const char *msg, int isprivate, int parseansi);
extern int notify_from_echo(dbref from, dbref player, const char *msg, int isprivate);
extern int ansi_notify_from_echo(dbref from, dbref player, const char *msg, int isprivate, int parseansi);
extern int notify_from(dbref from, dbref player, const char *msg);
extern int ansi_notify_from(dbref from, dbref player, const char *msg, int parseansi);
extern int desc_file(dbref player, struct descriptor_data * d, const char *fname);
extern void wall_and_flush(const char *msg);
extern void flush_user_output(dbref player);
extern void wall_all(const char *msg);
extern void wall_status(const char *msg);
extern void wall_mud(const char *msg);
extern void wall_arches(const char *msg);
extern void wall_wizards(const char *msg);
extern void show_wizards(dbref player);
extern int shutdown_flag; /* if non-zero, interface should shut down */
extern int restart_flag; /* if non-zero, should restart after shut down */
extern void emergency_shutdown(void);
extern int boot_off(dbref player);
extern int boot_idlest(dbref player, int keep_only_one);
extern int boot_welcome_site_idlest(int site);
extern void boot_player_off(dbref player);
extern char *time_format_1(time_t);
extern char *time_format_2(time_t);
extern int clear_prompt(dbref player);
extern int online(dbref player);
extern int minidle(dbref player);
extern int pcount(void);
extern int pidle(int c);
extern int pidler(int c);
extern int pdbref(int c);
extern int pontime(int c);
extern char *phost(int c);
extern char *puser(int c);
extern char *pipnum(int c);
extern char *pport(int c);
extern int lport(int);
extern int pfirstdescr(dbref who);
extern void pboot(int c);
extern void pnotify(int c, char *outstr);
extern int pdescr(int c);
extern int pdescrcon(int c);
extern int pnextdescr(int c);
extern int pfirstconn(dbref who);
extern int dset_user(struct descriptor_data *d, dbref who);
extern int pset_user(int c, dbref who);
extern int pset_echo(int c, int onoff);
extern dbref partial_pmatch(const char *name);
extern void do_linewrap( dbref, const char * );
extern void do_more( dbref, const char * );
extern void do_armageddon( dbref, const char * );
extern void do_dinfo( dbref, const char *arg);
extern void do_dboot(dbref player, const char *name);
extern void do_dwall( dbref, const char *name, const char *msg);
extern void do_dforce(dbref player, const char *name, const char *arg);
extern int request( dbref, struct descriptor_data *, const char *msg );

/* the following symbols are provided by game.c */

extern void process_command(dbref player, const char *command, int alias);

extern dbref create_player(const char *name, const char *password);
extern dbref connect_player(const char *name, const char *password);
extern void do_look_around(dbref player);

extern int init_game(const char *infile, const char *outfile);
extern void dump_database(void);
extern void panic(const char *);

/* CRT additions */

extern const char *host_as_hex( unsigned int addr );
extern void dump_ignoring( dbref player );
extern int descr_index(int descr);
extern int index_descr(int index);
