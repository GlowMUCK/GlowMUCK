#include "copyright.h"
#include "config.h"
#include "params.h"
#include "defaults.h"
#include "local.h"

#include "color.h"
#include "db.h"
#include "tune.h"
#include "props.h"
#include "interface.h"
#include "externs.h"

#define HASH1_MSG "HASH: Playername hashtable is inconsistent.  Rebuilding it."
#define HASH2_MSG "HASH: Playername hashtable still inconsistent after rebuild."


static hash_tab player_list[PLAYER_HASH_SIZE];
void refresh_players(void);

void
refresh_players(void)
{
    int i;

    clear_players();
    for (i = db_top; i-->0; ) {
	if (Typeof(i) == TYPE_PLAYER) {
	    add_player(i);
	}
    }
}

dbref 
lookup_player(const char *name)
{
    hash_data *hd;

    if ((hd = find_hash(name, player_list, PLAYER_HASH_SIZE)) == NULL)
	return NOTHING;
    else
	return (hd->dbval);
}

dbref 
connect_player(const char *name, const char *password)
{
    dbref   player, i;

    if (*name == '#' && number(name+1) && atoi(name+1)) {
	player = (dbref) atoi(name + 1);
	if ((!OkObj(player)) || (Typeof(player) != TYPE_PLAYER))
	    player = NOTHING;
    } else {
	player = lookup_player(name);
    }
    if (player == NOTHING) {
	/* Check for a player not in the hashtable */
	for(i = (db_top - 1); i > NOTHING; i--) {
	    if(Typeof(i) == TYPE_PLAYER) {
		if(!string_compare(name, NAME(i))) {
		    /* Oooga, found a player that lookup didn't! */
		    log_status(HASH1_MSG);
		    wall_wizards(MARK HASH1_MSG);
		    refresh_players();
		    player = lookup_player(name);
		    if(player == NOTHING) {
			log_status(HASH2_MSG);
			wall_wizards(MARK HASH2_MSG);
		    }
		    break;
		}
	    }
	}
	if(player == NOTHING)
	    return NOTHING;
    }
    if (DBFETCH(player)->sp.player.password
	    && *DBFETCH(player)->sp.player.password
	    && strcmp(DBFETCH(player)->sp.player.password, password))
	return NOTHING;

    return player;
}

dbref 
create_player(const char *name, const char *password)
{
    char buf[80];
    dbref player;

    if (!ok_player_name(name) || !ok_password(password))
	return NOTHING;
    if (!tp_building || tp_db_readonly) return NOTHING;

    /* else he doesn't already exist, create him */
    player = new_object();

    /* initialize everything */
    NAME(player) = alloc_string(name);
    FLAGS(player) = TYPE_PLAYER | PCREATE_FLAGS;
    FLAG2(player) = PCREATE_FLAG2;
    DBFETCH(player)->location = RootRoom;	/* home */
    OWNER(player) = player;
    DBFETCH(player)->sp.player.home = RootRoom;
    DBFETCH(player)->exits = NOTHING;
    DBFETCH(player)->sp.player.pennies = tp_start_pennies;
    DBFETCH(player)->sp.player.password = alloc_string(password);
    DBFETCH(player)->sp.player.curr_prog = NOTHING;
    DBFETCH(player)->sp.player.insert_mode = 0;

    /* link him to tp_player_start */
    PUSH(player, DBFETCH(RootRoom)->contents);
    add_player(player);
    DBDIRTY(player);
    DBDIRTY(RootRoom);

    sprintf(buf, CNOTE "%s is born!", PNAME(player));
    anotify_except(DBFETCH(RootRoom)->contents, NOTHING, buf, player);

    return player;
}

void 
do_password(dbref player, const char *old, const char *newobj)
{
    if(Guest(player)) {
	anotify(player, CFAIL NOGUEST_MESG);
	return;
    }

    if ((!DBFETCH(player)->sp.player.password) ||
	(!*DBFETCH(player)->sp.player.password) ||
	strcmp(old, DBFETCH(player)->sp.player.password)
    ) {
	anotify(player, CFAIL "Sorry.");
    } else if (!ok_password(newobj)) {
	anotify(player, CFAIL "Bad new password.");
    } else {
	free((void *) DBFETCH(player)->sp.player.password);
	DBSTORE(player, sp.player.password, alloc_string(newobj));
	ts_modifyobject(player);
	remove_property(player, PROP_PW);
	anotify(player, CSUCC "Password changed.");
    }
}

void 
clear_players(void)
{
    kill_hash(player_list, PLAYER_HASH_SIZE, 0);
    return;
}

void 
add_player(dbref who)
{
    hash_data hd;

    hd.dbval = who;
    if (add_hash(NAME(who), hd, player_list, PLAYER_HASH_SIZE) == NULL)
	panic("Out of memory");

    if(who != lookup_player(NAME(who))) {
	log_status(HASH1_MSG);
	wall_wizards(MARK HASH1_MSG);
	refresh_players();
	if(who != lookup_player(NAME(who))) {
	    log_status(HASH2_MSG);
	    wall_wizards(MARK HASH2_MSG);
	}
    }
    return;
}


void 
delete_player(dbref who)
{
    int result;

    result = free_hash(NAME(who), player_list, PLAYER_HASH_SIZE);

    if (result) {
	log_status(HASH1_MSG);
	wall_wizards(MARK HASH1_MSG);
	refresh_players();
	result = free_hash(NAME(who), player_list, PLAYER_HASH_SIZE);
	if (result) {
	    log_status(HASH2_MSG);
	    wall_wizards(MARK HASH2_MSG);
	}
    }

    return;
}

