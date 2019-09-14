/* ************************************************************************ *
File: interpreter.c                                 Part of CircleMUD * *  
Usage: parse user commands, search for specials, call ACMD functions   * *                                                                         
* *  All rights reserved.  See license.doc for complete information.        * *                                                                         
* *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University * *  
CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               * 
************************************************************************ */

#define __INTERPRETER_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "class.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "constants.h"
#include "olc.h"
#include "auction.h"
#include "rooms.h"
#include "vt100c.h"
#include "clan.h"

#define CLRSCR	"\033[H\033[J"
extern int clan_loadroom[];
extern const struct title_type titles1[LVL_IMPL + 1];
extern char *motd;
extern char *imotd;
extern char *background;
extern char *MENU;//, *MENU2;
extern char *WELC_MESSG;
extern char *START_MESSG;
extern char *hum;
extern char *ogr;
extern char *elf;
extern char *gno;
extern char *dwa;
extern char *helf;
extern char *hob;
extern char *dro;
extern char *namepol;
extern char *atrib;
extern char *goodnames;
extern sh_int r_frozen_start_room;

extern FILE *ff;
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int restrict1;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;

/* external functions */
void do_newbie(struct char_data * vict);
void read_aliases(struct char_data * ch);
void read_quests(struct char_data * ch);
void echo_on(struct descriptor_data * d);
void echo_off(struct descriptor_data * d);
void do_start(struct char_data * ch);
void init_char(struct char_data * ch);
int create_entry(char *name);
int special(struct char_data * ch, int cmd, char *arg);
int isbanned(char *hostname);
int Valid_Name(char *newname);
void ADD_REINCARN(struct char_data * ch);
void oedit_parse(struct descriptor_data * d, char *arg);
void redit_parse(struct descriptor_data * d, char *arg);
void zedit_parse(struct descriptor_data * d, char *arg);
void medit_parse(struct descriptor_data * d, char *arg);
void sedit_parse(struct descriptor_data * d, char *arg);
void display_hometowns(struct descriptor_data *d);
int             parse_ht(char *arg, struct descriptor_data *d);
void send_desc_help(struct descriptor_data *ch, char *argument);
int display_pc2npc_menu(struct descriptor_data *d);
 int check_pc2npc(struct descriptor_data *d, char *arg) ;
/* prototypes for all do_x functions. */

ACMD(do_showegos);
ACMD(do_modexp);
ACMD(do_newmap);
ACMD(do_pctonpc);
ACMD(do_sayto);
ACMD(do_topdam);
ACMD(do_tophurt);

ACMD(do_replay);
ACMD(do_areas);
ACMD(do_walkset);
ACMD(do_repair);
ACMD(do_irepair);
ACMD(do_bwzone);
ACMD(do_resistmagic);
ACMD(do_nerve);
ACMD(do_firstaid);
ACMD(do_skills);
ACMD(do_cutthroat);
ACMD(do_lodge);
ACMD(do_headbutt);
ACMD(do_daggerdance);
ACMD(do_push);
ACMD(do_drag);
ACMD(do_pull);

ACMD(do_mpwait);
ACMD(do_mpwaitstate);
ACMD(do_mp_open_passage);
ACMD(do_mp_close_passage);
ACMD(do_mpstat);
ACMD(do_opstat);
ACMD(do_rpstat);
ACMD(do_mpechozone);
ACMD(do_mp_practice);
ACMD(do_mpscatter);
ACMD(do_mp_slay);
ACMD(do_mp_damage) ;
ACMD(do_mp_restore);
ACMD(do_mpnothing);
ACMD(do_mpdream);
ACMD(do_mppeace);
ACMD(do_mpdelay);

ACMD(do_mpsound);


ACMD(do_abort);
ACMD(do_conceal);
ACMD(do_tumble);
ACMD(do_ambush);
ACMD(do_disguise);
ACMD(do_envenom);
ACMD(do_snooproom);
ACMD(do_dirtkick);
ACMD(do_disable);
ACMD(do_trap);
ACMD(do_traps);
ACMD(do_taunt);
ACMD(do_eavesdrop);
ACMD(do_guzva);
ACMD(do_kickflip);
ACMD(do_charge);
ACMD(do_bashdoor);
ACMD(do_knockout);
ACMD(do_gut);
ACMD(do_vwear);
ACMD(do_private_channel);
ACMD(do_index);
ACMD(do_sac);
ACMD(do_chown);

ACMD(do_wlist);
ACMD(do_newquest_players);
ACMD(do_newquest);
ACMD(do_spellhelp);
ACMD(do_clean);
ACMD(do_run);
ACMD(do_walk);
ACMD(do_bigboss);
ACMD(do_egoknown);
ACMD(do_skillget);
ACMD(do_cease);
ACMD(do_armor);
ACMD(do_leech);
ACMD(do_mobtop);
ACMD(do_mobpk);
ACMD(do_common);
ACMD(do_wrap);

ACMD(do_cover_tracks);
ACMD(do_hunt);
ACMD(do_bandage);
ACMD(do_medic);
ACMD(do_forage);
ACMD(do_spy);
ACMD(do_plant);
ACMD(do_whirlwind);
ACMD(do_guard);
ACMD(do_spin_kick);
ACMD(do_concentrate);
ACMD(do_gouge);
ACMD(do_buddha_finger);
ACMD(do_melee);
ACMD(do_knee);
ACMD(do_elbow);
ACMD(do_sweepkick);
ACMD(do_combo);
ACMD(do_retreat);
ACMD(do_struggle);
ACMD(do_scare);
ACMD(do_autopsy);
ACMD(do_sharpen);
ACMD(do_top);
ACMD(do_capture);
ACMD(do_boss);

ACMD(do_nomissf);
ACMD(do_nomisse);
ACMD(do_move_hidden);
ACMD(do_stun);
ACMD(do_stalk);
ACMD(do_meditate);
ACMD(do_trip);
ACMD(do_battlecry);
ACMD(do_archirage);
ACMD(do_zone);
ACMD(do_kata);
ACMD(do_hall);
ACMD(do_ckscore);
ACMD(do_claninfo);
ACMD(do_clanlist);
ACMD(do_pkscore);
ACMD(do_graph);
ACMD(do_map);
ACMD(do_mood);
ACMD(do_name);
ACMD(do_gwho);
ACMD(do_resize);
ACMD(do_prompt);
ACMD(do_display);
ACMD(do_label);
ACMD(do_clan);
ACMD(do_ctell);
ACMD(do_behead);
ACMD(do_feign_death);
ACMD(do_grats);
ACMD(do_fame);
ACMD(do_hammer);
ACMD(do_autoquest);
ACMD(do_copyto);
ACMD(do_connect);
ACMD(do_clear);
ACMD(do_warm);
ACMD(do_extinguish);
ACMD(do_mkscore);
ACMD(do_enrage);
ACMD(do_fist_of_doom);
ACMD(do_first_aid);
ACMD(do_test);
ACMD(do_olist);
ACMD(do_rlist);
ACMD(do_mlist);
ACMD(do_action);
ACMD(do_advance);
ACMD(do_affects);
ACMD(do_alias);
ACMD(do_alfa_nerve);
ACMD(do_arena);
ACMD(do_assist);
ACMD(do_auto);
ACMD(do_at);
ACMD(do_awho);
ACMD(do_ahall);
ACMD(do_auction);
ACMD(do_backstab);
ACMD(do_ban);
ACMD(do_bid);
ACMD(do_berserk);
ACMD(do_bash);
ACMD(do_bet);
ACMD(do_beta_nerve);
ACMD(do_brew);
ACMD(do_bury);
ACMD(do_cast);
ACMD(do_channel);
ACMD(do_chaos);
ACMD(do_circle);
ACMD(do_color);
ACMD(do_commands);
ACMD(do_compare);
ACMD(do_consider);
ACMD(do_cook);
ACMD(do_credits);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_diagnose);
ACMD(do_disarm);
ACMD(do_dig);
ACMD(do_drink);
ACMD(do_drop);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_fire);
ACMD(do_fillet);
ACMD(do_flee);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_forge);
ACMD(do_gamma_nerve);
ACMD(do_gecho);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_gold);
ACMD(do_goto);
ACMD(do_godwill);
ACMD(do_grab);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_hcontrol);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_house);
ACMD(do_info);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_kick);
ACMD(do_kill);
ACMD(do_land);
ACMD(do_last);
ACMD(do_learned);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_load);
ACMD(do_look);
ACMD(do_listen);
ACMD(do_mpasound);
ACMD(do_mpjunk);
ACMD(do_mpecho);
ACMD(do_mpechoat);
ACMD(do_mpechoaround);
ACMD(do_mpkill);
ACMD(do_mpmload);
ACMD(do_mpoload);
ACMD(do_mppurge);
ACMD(do_mpgoto);
ACMD(do_mpat);
ACMD(do_mptransfer);
ACMD(do_mpforce);
ACMD(do_move);
ACMD(do_not_here);
ACMD(do_offer);
ACMD(do_olc);
ACMD(do_order);
ACMD(do_page);
ACMD(do_poofset);
ACMD(do_pour);
ACMD(do_players);
ACMD(do_practice);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_reboot);
ACMD(do_remove);
ACMD(do_rent);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_rstat);
ACMD(do_save);
ACMD(do_say);
ACMD(do_scan);
ACMD(do_score);
ACMD(do_scribe);
ACMD(do_send);
ACMD(do_set);
ACMD(do_setquest);
ACMD(do_show);
ACMD(do_showquest);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skillset);
ACMD(do_sleep);
ACMD(do_slay);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_split);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_switch);
ACMD(do_syslog);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_time);
ACMD(do_title);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_unban);
ACMD(do_ungroup);
ACMD(do_use);
ACMD(do_users);
ACMD(do_victim);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_worthy);
ACMD(do_wake);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_who);
ACMD(do_wield);
ACMD(do_wish);
ACMD(do_wimpy);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_zap);
ACMD(do_zstat);
ACMD(do_zallow);
ACMD(do_zdeny);
ACMD(do_zreset);
ACMD(do_redit);
ACMD(do_zedit);
ACMD(do_style);

/* This is the Master Command List(tm).
 *
 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */
char mbufi[200];
const struct command_info cmd_info[] = {
                                           {"RESERVED", 0, 0, 0, 0},	/* this must be first -- for specprocs */

                                           /* directions must come before other commands but after RESERVED */
                                           {"north", POS_STANDING, do_move, 0, SCMD_NORTH},
                                           {"east", POS_STANDING, do_move, 0, SCMD_EAST},
                                           {"south", POS_STANDING, do_move, 0, SCMD_SOUTH},
                                           {"west", POS_STANDING, do_move, 0, SCMD_WEST},
                                           {"up", POS_STANDING, do_move, 0, SCMD_UP},
                                           {"down", POS_STANDING, do_move, 0, SCMD_DOWN},
                                           {"northwest", POS_STANDING, do_move, 0, SCMD_NW},
                                           {"nw", POS_STANDING, do_move, 0, SCMD_NW},
                                           {"northeast", POS_STANDING, do_move, 0, SCMD_NE},
                                           {"ne", POS_STANDING, do_move, 0, SCMD_NE},
                                           {"southwest", POS_STANDING, do_move, 0, SCMD_SW},
                                           {"sw", POS_STANDING, do_move, 0, SCMD_SW},
                                           {"southeast", POS_STANDING, do_move, 0, SCMD_SE},
                                           {"se", POS_STANDING, do_move, 0, SCMD_SE},

                                           {",", POS_RESTING, do_echo, 1, SCMD_EMOTE},
                                           {"`", POS_SLEEPING, do_gen_comm, -1, SCMD_BROADCAST},
                                           {".", POS_SLEEPING, do_gen_comm, 0, SCMD_GOSSIP},
                                           {";", POS_SLEEPING, do_gsay, 0, 0},
                                           {"/", POS_DEAD    , do_gen_comm , 0, SCMD_PRIVATE },
                                           {"'", POS_RESTING, do_say, 0, 0},
                                           {";", POS_DEAD, do_wiznet, LVL_IMMORT, 0},





                                           /* now, the main list */
                                           {"affects", POS_SLEEPING, do_affects, 0, 0},
                                           {"at", POS_DEAD, do_at, LVL_GOD, 0},
                                           {"abort", POS_SLEEPING, do_abort, 0, 0},
                                           {"advance", POS_RESTING, do_advance, LVL_GRGOD, 0},
                                           /*//  { "aid"     , POS_FIGHTING, do_first_aid  , 1, 0 },*/
                                           {"bandage", POS_STANDING, do_bandage, -1,0},
                                           {"afk", POS_DEAD, do_gen_tog, 0, SCMD_AFK},
                                           {"alias", POS_DEAD, do_alias, 0, 0},
                                           {"alfa", POS_STANDING, do_alfa_nerve, -1, 0},
                                           {"accuse", POS_SITTING, do_action, 0, 0},
                                           {"addquest", POS_DEAD, do_setquest, LVL_GOD, SCMD_ADD_QUEST_BIT},
                                           {"agree", POS_SITTING, do_action, 0, 0},
                                           {"applaud", POS_RESTING, do_action, 0, 0},
                                           //{"armor", POS_SLEEPING, do_armor, 0,0},
                                           {"archirage", POS_STANDING, do_archirage, -1,0},
                                           {"arena", POS_STANDING, do_arena, -1, 0},
                                           {"ambush", POS_STANDING, do_ambush, -1, 0},
                                           {"assist", POS_FIGHTING, do_assist, 1, 0},
                                           {"awho", POS_SLEEPING, do_awho, 0, 0},
                                           {"areas", POS_DEAD, do_areas, -1, 0},
                                           {"ahall", POS_SLEEPING, do_ahall, 0, 0},
                                           {"ask", POS_RESTING, do_spec_comm, 0, SCMD_ASK},
                                           /*  { "auction"  , POS_SLEEPING, do_gen_comm , 0, SCMD_AUCTION },*/
                                           {"auction", POS_STANDING, do_auction, -1, 0},
                                           {"auto", POS_DEAD, do_auto, 0, 0},
                                           {"autoassist", POS_DEAD, do_gen_tog, 0, SCMD_AUTOASSIST},
                                           {"autodir", POS_DEAD, do_gen_tog, 0, SCMD_AUTODIR},
                                           {"automap", POS_DEAD, do_gen_tog, 0, SCMD_AUTOMAP},
                                           {"autoexit", POS_DEAD, do_gen_tog, 0, SCMD_AUTOEXIT},
                                           {"autosac", POS_DEAD, do_gen_tog, 0, SCMD_AUTOSAC},
                                           {"autosave", POS_DEAD, do_gen_tog, 0, SCMD_AUTOSAVE},
                                           {"autotitle", POS_DEAD, do_gen_tog, 0, SCMD_AUTOTITLE},
                                           {"autoloot", POS_DEAD, do_gen_tog, 0, SCMD_AUTOLOOT},
                                           {"autosplit", POS_DEAD, do_gen_tog, 0, SCMD_AUTOSPLIT},
                                           {"autograt", POS_DEAD, do_gen_tog, 0, SCMD_AUTOGRAT},
                                           {"autoscan", POS_DEAD, do_gen_tog, 0, SCMD_AUTOSCAN},
                                           {"autopsy", POS_STANDING, do_autopsy, 1, 0},

                                           {"bash", POS_FIGHTING, do_bash, 0, 0},
                                           {"bashdoor", POS_STANDING, do_gen_door, -1, SCMD_DOORBASH},
                                           {"backstab", POS_STANDING, do_backstab, -1, 0},
                                           {"balance", POS_STANDING, do_not_here, 1, 0},
                                           {"berserk", POS_FIGHTING, do_berserk, -1, 0},
                                           {"behead", POS_FIGHTING, do_behead, -1, 0},
                                           {"ban", POS_DEAD, do_ban, LVL_GRGOD, 0},
                                           {"battlecry", POS_STANDING, do_battlecry, -1,0},
                                           //    {"bet", POS_STANDING, do_bet, 0, 0},
                                           {"beta", POS_STANDING, do_beta_nerve, -1, 0},
                                           {"beg", POS_RESTING, do_action, 0, 0},
                                           {"bestzones", POS_SLEEPING, do_bwzone, 0, SCMD_BESTZONE},
                                           //{"bigboss", POS_STANDING, do_bigboss, -1, 0},
                                           {"bid", POS_STANDING, do_bid, -1, 0},
                                           {"bounce", POS_STANDING, do_action, 0, 0},
                                           {"bark", POS_RESTING, do_action, 0, 0},
                                           {"bleed", POS_RESTING, do_action, 0, 0},
                                           {"blush", POS_RESTING, do_action, 0, 0},
                                           {"boss", POS_SLEEPING, do_boss, -1, 0},
                                           {"bonk", POS_RESTING, do_action, 0, 0},
                                           {"boink", POS_RESTING, do_action, 0, 0},
                                           {"bow", POS_STANDING, do_action, 0, 0},
                                           {"brb", POS_RESTING, do_action, 0, 0},
                                           {"brief", POS_DEAD, do_gen_tog, 0, SCMD_BRIEF},


                                           {"brew", POS_STANDING, do_brew, -1, 0},
                                           {"burp", POS_RESTING, do_action, 0, 0},
                                           {"buy", POS_STANDING, do_not_here, 0, 0},
                                           {"buddha", POS_FIGHTING, do_buddha_finger, -1, 0},
                                           {"bury", POS_STANDING, do_bury, -1, 0},
                                           {"bug", POS_DEAD, do_gen_write, 0, SCMD_BUG},
                                           {"buglist", POS_DEAD, do_gen_ps, LVL_GRGOD, SCMD_BUGLIST},
                                           {"cast", POS_SITTING, do_cast, 1, 0},
                                           {"cackle", POS_RESTING, do_action, 0, 0},
                                           {"carve", POS_STANDING, do_not_here, -1, 0},
                                           {"capture", POS_STANDING, do_capture, -1, 0},
                                           {"cease", POS_FIGHTING, do_cease, 0, 0},
                                           {"ctf", POS_STANDING, do_chaos, -1, 0},
                                           {"charge", POS_STANDING, do_hit, 0, SCMD_CHARGE},
                                           {"channel", POS_DEAD, do_channel, 0, 0},
                                           { "chown"    , POS_SLEEPING, do_chown, 54, 0 },
                                           {"check", POS_STANDING, do_not_here, 1, 0},
                                           {"cheer", POS_SLEEPING, do_action, 0, 0},
                                           {"chuckle", POS_RESTING, do_action, 0, 0},
                                           {"circle", POS_FIGHTING, do_circle, -1, 0},
                                           {"ckscore", POS_SLEEPING, do_ckscore, 0, 0},
                                           /*//  { "noclan"     , POS_RESTING , do_gen_comm , 0, SCMD_CLAN },*/
                                           {"clan", POS_SLEEPING, do_clan, LVL_GRGOD, 0},
                                           {"clanlist", POS_SLEEPING, do_clanlist, 0, 0},
                                           {"claninfo", POS_SLEEPING, do_claninfo, 0, 0},
                                           {"clanwho", POS_SLEEPING, do_clanlist, 0, 0},
                                           {"clean", POS_RESTING, do_clean, 0, 0},
                                           {"ctell", POS_SLEEPING, do_ctell, -1, 0},
                                           {"clap", POS_RESTING, do_action, 0, 0},
                                           {"clear", POS_FIGHTING, do_clear, 0, 0},
                                           {"close", POS_SITTING, do_gen_door, 0, SCMD_CLOSE},
                                           {"cls", POS_DEAD, do_gen_ps, 0, SCMD_CLEAR},
                                           {"combat", POS_SLEEPING, do_common, 0, 0},
                                           {"consider", POS_FIGHTING, do_consider, 0, 0},
                                           {"compare", POS_RESTING, do_compare, 0, 0},
                                           {"combo", POS_FIGHTING, do_combo, -1, 0},
                                           {"color", POS_DEAD, do_color, -1, 0},
                                           {"copyto", POS_DEAD, do_copyto, LVL_GRGOD, 0},
                                           //{"concentrate", POS_RESTING, do_concentrate, -1, 0},
                                           {"config", POS_DEAD, do_toggle, 0, 0},
                                           {"connect", POS_DEAD, do_connect, LVL_GRGOD, 0},
                                           {"conceal", POS_SLEEPING, do_conceal, -1, 0},
                                           {"comfort", POS_RESTING, do_action, 0, 0},
                                           {"comb", POS_RESTING, do_action, 0, 0},
                                           {"commands", POS_DEAD, do_commands, 0, SCMD_COMMANDS},
                                           {"compact", POS_DEAD, do_gen_tog, 0, SCMD_COMPACT},
                                           //	{"cook", POS_STANDING, do_cook, -1, 0},
                                           {"cover", POS_STANDING, do_cover_tracks, -1, 0},
                                           {"cough", POS_RESTING, do_action, 0, 0},
                                           {"credits", POS_DEAD, do_gen_ps, 0, SCMD_CREDITS},
                                           {"cringe", POS_RESTING, do_action, 0, 0},
                                           {"cry", POS_RESTING, do_action, 0, 0},
                                           {"cuddle", POS_RESTING, do_action, 0, 0},
                                           {"curse", POS_RESTING, do_action, 0, 0},
                                           {"curtsey", POS_STANDING, do_action, 0, 0},
                                           {"cutthroat", POS_STANDING, do_cutthroat, 0, 0},

                                           {"dance", POS_STANDING, do_action, 0, 0},
                                           {"date", POS_DEAD, do_date, LVL_IMMORT, SCMD_DATE},
                                           {"daggerdance"  , POS_FIGHTING, do_daggerdance , -1, 0 },
                                           {"daydream", POS_SLEEPING, do_action, 0, 0},
                                           {"dc", POS_DEAD, do_dc, LVL_GRGOD, 0},
                                           {"deposit", POS_STANDING, do_not_here, 1, 0},
                                           {"dirtkick", POS_FIGHTING, do_dirtkick, -1, 0},
                                           {"disarm", POS_FIGHTING, do_disarm, 0, 0},
                                           {"dig", POS_STANDING, do_dig, 0, 0},
                                           {"diagnose", POS_RESTING, do_diagnose, 0, 0},
                                           {"display", POS_SLEEPING, do_display, -1, 0},
                                           {"disable", POS_STANDING, do_disable, 0, 0},
                                           {"disguise", POS_STANDING, do_disguise, -1, 0},
                                           {"doh", POS_RESTING, do_action, 0, 0},
                                           {"donate", POS_RESTING, do_drop, 0, SCMD_DONATE},
                                           {"drink", POS_RESTING, do_drink, 0, SCMD_DRINK},
                                           {"drop", POS_RESTING, do_drop, 0, SCMD_DROP},
                                           {"drool", POS_RESTING, do_action, 0, 0},

                                           {"eat", POS_RESTING, do_eat, 0, SCMD_EAT},
                                           {"eavesdrop", POS_RESTING, do_eavesdrop, 0, 0},
                                           {"echo", POS_SLEEPING, do_echo, LVL_GOD, SCMD_ECHO},
                                           {"egoknown", POS_SLEEPING, do_egoknown, 0, 0},
                                           {"emote", POS_RESTING, do_echo, 1, SCMD_EMOTE},
                                           {"embalm", POS_STANDING, do_not_here, 1, 0},
                                           {"embrace", POS_STANDING, do_action, 0, 0},
                                           {"enter", POS_STANDING, do_enter, 0, 0},
                                           {"engrave"  , POS_STANDING, do_not_here , -1, 0 },
                                           {"envenom", POS_RESTING, do_envenom, -1, 0},
                                           {"elbow"  , POS_FIGHTING, do_elbow , -1, 0 },
                                           {"expell"  , POS_STANDING, do_not_here , 0, 0 },
                                           {"equipment", POS_SLEEPING, do_equipment, 0, 0},
                                           {"exits", POS_RESTING, do_exits, 0, 0},
                                           {"examine", POS_SITTING, do_examine, 0, 0},
                                           {"extinguish", POS_FIGHTING, do_extinguish, 0, 0},

                                           {"force", POS_SLEEPING, do_force, LVL_GRGOD, 0},
                                           {"fame", POS_STANDING, do_fame, -1, 0},
                                           {"fade", POS_RESTING, do_action, 0, 0},
                                           {"faint", POS_RESTING, do_action, 0, 0},
                                           {"fart", POS_RESTING, do_action, 0, 0},
                                           {"feign", POS_FIGHTING, do_feign_death, -1, 0},
                                           {"first", POS_SITTING, do_firstaid, 1, 0},
                                           {"fill", POS_STANDING, do_pour, 0, SCMD_FILL},
                                           {"fillet", POS_STANDING, do_fillet, -1, 0},
                                           {"fist", POS_FIGHTING, do_fist_of_doom, -1, 0},
                                           {"fire", POS_STANDING, do_fire, 0, SCMD_FIRE},

                                           {"flee", POS_FIGHTING, do_flee, 1, 0},
                                           {"flex", POS_SITTING, do_action, 0, 0},
                                           {"flip", POS_STANDING, do_action, 0, 0},
                                           {"flirt", POS_RESTING, do_action, 0, 0},
                                           {"follow", POS_RESTING, do_follow, 0, 0},
                                           {"forage", POS_STANDING, do_forage, -1, 0},
                                           {"forge", POS_STANDING, do_forge, -1, 0},
                                           {"fondle", POS_RESTING, do_action, 0, 0},
                                           {"freeze", POS_DEAD, do_wizutil, LVL_FREEZE, SCMD_FREEZE},
                                           {"french", POS_RESTING, do_action, 0, 0},
                                           {"frown", POS_RESTING, do_action, 0, 0},
                                           {"fume", POS_RESTING, do_action, 0, 0},
                                           {"fwap", POS_STANDING, do_action, 0, 0},

                                           {"group", POS_SLEEPING, do_group, 1, 0},
                                           {"graph", POS_SLEEPING, do_graph, 0, 0},
                                           {"get", POS_RESTING, do_get, 0, 0},
                                           {"gasp", POS_RESTING, do_action, 0, 0},
                                           {"gamma", POS_STANDING, do_gamma_nerve, -1, 0},
                                           {"gecho", POS_DEAD, do_gecho, LVL_GOD, 0},
                                           {"give", POS_RESTING, do_give, 0, 0},
                                           {"giggle", POS_RESTING, do_action, 0, 0},
                                           {"glare", POS_RESTING, do_action, 0, 0},
                                           {"goto", POS_SLEEPING, do_goto, LVL_IMMORT, 0},
                                           {"gold", POS_SLEEPING, do_gold, 0, 0},
                                           {"gouge", POS_STANDING, do_gouge, -1, 0},
                                           {"godwill", POS_RESTING, do_godwill, 0, 0},
                                           {"gossip", POS_SLEEPING, do_gen_comm, 0, SCMD_GOSSIP},
                                           { "gamble"   , POS_STANDING, do_not_here , 0, 0 },
                                           {"grab", POS_RESTING, do_grab, 0, 0},
                                           {"grats", POS_SLEEPING, do_grats, 0, 0},
                                           {"greet", POS_RESTING, do_action, 0, 0},                                           
                                           {"grin", POS_RESTING, do_action, 0, 0},
                                           {"grimace", POS_RESTING, do_action, 0, 0},
                                           {"groan", POS_RESTING, do_action, 0, 0},
                                           {"grope", POS_RESTING, do_action, 0, 0},
                                           {"grovel", POS_RESTING, do_action, 0, 0},
                                           {"growl", POS_RESTING, do_action, 0, 0},
                                           {"grumble", POS_RESTING, do_action, 0, 0},
                                           {"gsay", POS_SLEEPING, do_gsay, 0, 0},

                                           {"gtell", POS_SLEEPING, do_gsay, 0, 0},
                                           {"gwho", POS_SLEEPING, do_gwho, 0, 0},
                                           {"gut", POS_FIGHTING, do_gut, -1, 0},
                                           {"guard", POS_FIGHTING, do_guard, -1, 0},
                                           {"guzva", POS_DEAD, do_guzva, LVL_GOD, 0},

                                           {"help", POS_DEAD, do_help, 0, 0},
                                           {"hall", POS_SLEEPING, do_hall, -1, 0},
                                           {"heal", POS_STANDING, do_not_here, 0, 0},
                                           {"hammer", POS_STANDING, do_hammer, -1, 0},
                                           {"handbook", POS_DEAD, do_gen_ps, LVL_IMMORT, SCMD_HANDBOOK},
                                           {"headbutt", POS_FIGHTING, do_headbutt, -1, 0},
                                           {"hcontrol", POS_DEAD, do_hcontrol, LVL_GRGOD, 0},
                                           {"hiccup", POS_RESTING, do_action, 0, 0},
                                           {"hide", POS_RESTING, do_hide, -1, 0},
                                           {"hit", POS_FIGHTING, do_hit, 0, SCMD_HIT},
                                           {"hire", POS_STANDING, do_not_here, 0, 0},
                                           {"highfive", POS_RESTING, do_action, 0, 0},
                                           {"hold", POS_RESTING, do_grab, 1, 0},
                                           {"holler", POS_RESTING, do_gen_comm, 1, SCMD_HOLLER},
                                           {"holylight", POS_DEAD, do_gen_tog, LVL_IMMORT, SCMD_HOLYLIGHT},
                                           {"hop", POS_RESTING, do_action, 0, 0},
                                           {"house", POS_RESTING, do_house, 0, 0},
                                           {"hug", POS_RESTING, do_action, 0, 0},
                                           {"hunt", POS_STANDING, do_hunt, -1, 0},

                                           {"inventory", POS_DEAD, do_inventory, 0, 0},
                                           {"index", POS_DEAD, do_index, 0, 0},
                                           {"idea", POS_DEAD, do_gen_write, 0, SCMD_IDEA},
                                           {"idealist", POS_DEAD, do_gen_ps, LVL_GRGOD, SCMD_IDEALIST},
                                           { "ident"    , POS_DEAD, do_gen_tog  , LVL_IMPL, SCMD_IDENT },
                                           {"olist", POS_DEAD, do_olist, LVL_GOD, 0},
                                           {"imotd", POS_DEAD, do_gen_ps, LVL_IMMORT, SCMD_IMOTD},
                                           {"immlist", POS_DEAD, do_gen_ps, 0, SCMD_IMMLIST},
                                           {"iquest", POS_DEAD, do_newquest, LVL_GRGOD, 0},
                                           {"insert", POS_STANDING, do_not_here, 0, 0},
                                           {"info", POS_SLEEPING, do_gen_ps, 0, SCMD_INFO},
                                           {"insult", POS_RESTING, do_insult, 0, 0},
                                           {"invis", POS_DEAD, do_invis, LVL_IMMORT, 0},
                                           {"irepair", POS_DEAD, do_irepair, LVL_GOD, 0},

                                           {"junk", POS_RESTING, do_drop, 0, SCMD_JUNK},
                                           {"jeer", POS_SLEEPING, do_action, 0, 0},

                                           {"kill", POS_FIGHTING, do_kill, 0, 0},
                                           {"kata", POS_STANDING, do_kata, -1, 0},
                                           {"kick", POS_FIGHTING, do_kick, -1, 0},
                                           //{"kickflip", POS_FIGHTING, do_kickflip, -1, 0},
                                           {"knee"  , POS_FIGHTING, do_knee , -1, 0 },
                                           {"knockout", POS_FIGHTING, do_knockout, -1, 0},
                                           {"kiss", POS_RESTING, do_action, 0, 0},

                                           {"look", POS_RESTING, do_look, 0, SCMD_LOOK},
                                           {"laugh", POS_RESTING, do_action, 0, 0},
                                           {"label", POS_RESTING, do_label, 0, 0},
                                           {"land", POS_FIGHTING, do_land, 0, 0},
                                           {"last", POS_DEAD, do_last, LVL_GRGOD, 0},
                                           {"leech", POS_SLEEPING, do_leech, 0, 0},
                                           {"learned", POS_SLEEPING, do_learned, 0, 0},
                                           {"leave", POS_STANDING, do_leave, 0, 0},
                                           {"leer", POS_SLEEPING, do_action, 0, 0},
                                           //{"levels", POS_DEAD, do_levels, 0, 0},
                                           {"list", POS_STANDING, do_not_here, 0, 0},
                                           {"listen", POS_STANDING, do_listen, 0, 0},
                                           {"lick", POS_RESTING, do_action, 0, 0},
                                           {"lock", POS_SITTING, do_gen_door, 0, SCMD_LOCK},
                                           {"lodge", POS_FIGHTING, do_lodge, 1, 0},
                                           {"load", POS_DEAD, do_load, LVL_GOD, 0},
                                           {"love", POS_RESTING, do_action, 0, 0},

                                           {"mood", POS_RESTING, do_mood, 0, 0},
                                           {"mobtop", POS_SLEEPING, do_mobtop, -1, 0},
                                           {"mobpk", POS_SLEEPING, do_mobpk, -1, 0},
                                           {"map", POS_STANDING, do_map, -1, 0},
                                           {"mlist", POS_DEAD, do_mlist, LVL_GOD, 0},
                                           {"moan", POS_RESTING, do_action, 0, 0},
                                           {"motd", POS_DEAD, do_gen_ps, 0, SCMD_MOTD},
                                           {"modexp", POS_SLEEPING, do_modexp,LVL_IMPL, 0},
                                           {"mail", POS_STANDING, do_not_here, -1, 0},
                                           {"massage", POS_RESTING, do_action, 0, 0},
                                           {"meditate", POS_RESTING, do_meditate, -1, 0},
                                           {"medic", POS_RESTING, do_medic, -1, 0},
                                           {"melee"  , POS_FIGHTING, do_melee , -1, 0 },
                                           {"mobedit", POS_DEAD, do_olc, LVL_BUILDER, SCMD_OLC_MEDIT},
                                           {"movehidden", POS_STANDING, do_move_hidden, -1, 0},
                                           {"mumble", POS_RESTING, do_action, 0, 0},
                                           {"mute", POS_DEAD, do_wizutil, LVL_GOD, SCMD_SQUELCH},
                                           {"murder", POS_FIGHTING, do_hit, 0, SCMD_MURDER},
                                           {"mkscore", POS_SLEEPING, do_mkscore, 0, 0},
                                           {"muhaha", POS_RESTING, do_action, 0, 0},

                                           {"name", POS_SLEEPING, do_name, 0, 0},
                                           {"newmap", POS_STANDING, do_newmap, 0, LVL_GOD},
                                           {"news", POS_SLEEPING, do_gen_ps, 0, SCMD_NEWS},
                                           {"nerve", POS_STANDING, do_nerve, 1, 0},
                                           {"nibble", POS_RESTING, do_action, 0, 0},
                                           {"nod", POS_RESTING, do_action, 0, 0},
                                           //    {"noalert", POS_DEAD, do_gen_tog, 0, SCMD_NOALERT},
                                           {"nowho", POS_DEAD, do_gen_tog, 0, SCMD_NOWHO},
                                           {"noauction", POS_DEAD, do_gen_tog, 0, SCMD_NOAUCTION},
                                           {"noarena", POS_DEAD, do_gen_tog, 0, SCMD_NOTEAM},
                                           {"noooc", POS_DEAD, do_gen_tog, 0, SCMD_NOCHAT},
                                           /*//  { "noclan"   , POS_DEAD    , do_gen_tog  , 0, SCMD_NOCLAN },*/
                                           {"nogossip", POS_DEAD, do_gen_tog, 0, SCMD_NOGOSSIP},
                                           {"nograts", POS_DEAD, do_gen_tog, 0, SCMD_NOGRATZ},
                                           {"nohassle", POS_DEAD, do_gen_tog, LVL_IMMORT, SCMD_NOHASSLE},
                                           {"noquit", POS_DEAD, do_gen_tog, 0, SCMD_NOQUIT},
                                           {"norepeat", POS_DEAD, do_gen_tog, 0, SCMD_NOREPEAT},
                                           {"nodam", POS_DEAD, do_gen_tog, 0, SCMD_DISPDAM},
                                           {"nodesc", POS_DEAD, do_gen_tog, 0, SCMD_DISPSDESC},
                                           {"noinfo", POS_DEAD, do_gen_tog, 0, SCMD_NOINFO},
                                           {"nosummon", POS_DEAD, do_gen_tog, 1, SCMD_NOSUMMON},
                                           {"noshout", POS_SLEEPING, do_gen_tog, 1, SCMD_DEAF},
                                           {"nomisse", POS_DEAD, do_nomisse, 1, 1},
                                           {"nomissf", POS_DEAD, do_nomissf, 1, 1},
                                           {"notell", POS_DEAD, do_gen_tog, 1, SCMD_NOTELL},
                                           {"notitle", POS_DEAD, do_wizutil, LVL_GRGOD, SCMD_NOTITLE},
                                           /*  { "nowar"    , POS_DEAD    , do_gen_tog  , 0, SCMD_NOWAR },*/
                                           {"nowiz", POS_DEAD, do_gen_tog, LVL_GRGOD, SCMD_NOWIZ},
                                           {"nudge", POS_RESTING, do_action, 0, 0},
                                           {"nuzzle", POS_RESTING, do_action, 0, 0},



                                           {"open", POS_STANDING, do_gen_door, 0, SCMD_OPEN},
                                           {"ooc", POS_SLEEPING, do_gen_comm, 0, SCMD_CHAT},
                                           {"olc", POS_DEAD, do_olc, LVL_GRGOD, SCMD_OLC_SAVEINFO},
                                           {"oedit", POS_DEAD, do_olc, LVL_BUILDER, SCMD_OLC_OEDIT},
                                           {"order", POS_RESTING, do_order, -1, 0},
                                           {"offer", POS_STANDING, do_not_here, 1, 0},

					   {"prayfor", POS_SITTING, do_cast, 1, SCMD_PRAY},
                                           {"put", POS_RESTING, do_put, 0, 0},

                                           {"page", POS_DEAD, do_page, 1, 0},
                                           {"pardon", POS_DEAD, do_wizutil, LVL_GRGOD, SCMD_PARDON},
                                           {"pick", POS_STANDING, do_gen_door, -1, SCMD_PICK},
                                           {"policy", POS_DEAD, do_gen_ps, 0, SCMD_POLICIES},
                                           {"ponder", POS_RESTING, do_action, 0, 0},
                                           {"poofin", POS_DEAD, do_poofset, LVL_IMMORT, SCMD_POOFIN},
                                           {"poofout", POS_DEAD, do_poofset, LVL_IMMORT, SCMD_POOFOUT},
                                           {"pour", POS_STANDING, do_pour, 0, SCMD_POUR},
                                           {"plant", POS_STANDING, do_plant, -1, 0},
                                           //    {"players", POS_DEAD, do_players, LVL_IMPL, 0},
                                           {"practice", POS_SLEEPING, do_practice, 1, 0},
                                           {"preference", POS_DEAD, do_toggle, 0, 0},
                                           {"prompt", POS_DEAD, do_prompt, 0, 0},
                                           {"pkscore", POS_SLEEPING, do_pkscore, 0, 0},
                                           // {"pull", POS_STANDING, do_not_here, 0, 0},
                                           {"pull"  , POS_FIGHTING, do_pull , -1, 0 },
                                           {"purge", POS_DEAD, do_purge, LVL_GOD, 0},
                                           {"push"  , POS_FIGHTING, do_push , -1, 0 },
                                           {"drag"  , POS_FIGHTING, do_drag , -1, 0 },
                                           { "private"  , POS_DEAD    , do_gen_comm , 0, SCMD_PRIVATE },
                                            {"pctonpc"  , POS_DEAD    , do_pctonpc  , LVL_IMPL, 0 },

                                           { "padd"     , POS_DEAD    , do_private_channel , 0, PRIVATE_ADD },
                                           { "pclose"   , POS_DEAD    , do_private_channel , 0, PRIVATE_CLOSE },
                                           { "phelp"    , POS_DEAD    , do_private_channel , 0, PRIVATE_HELP },
                                           { "poff"     , POS_DEAD    , do_private_channel , 0, PRIVATE_OFF },
                                           { "popen"    , POS_DEAD    , do_private_channel , 0, PRIVATE_OPEN },
                                           { "premove"  , POS_DEAD    , do_private_channel , 0, PRIVATE_REMOVE },
                                           { "pwho"     , POS_DEAD    , do_private_channel , 0, PRIVATE_WHO },
                                           { "pcheck"   , POS_DEAD    , do_private_channel,LVL_IMPL, PRIVATE_CHECK },
                                           {"pat", POS_RESTING, do_action, 0, 0},
                                           {"peer", POS_RESTING, do_action, 0, 0},
                                           {"point", POS_RESTING, do_action, 0, 0},
                                           {"poke", POS_RESTING, do_action, 0, 0},
                                           {"pose", POS_SLEEPING, do_action, 0, 0},
                                           {"pout", POS_RESTING, do_action, 0, 0},
                                           {"pray", POS_SITTING, do_action, 0, 0},
                                           {"puke", POS_RESTING, do_action, 0, 0},
                                           {"punch", POS_RESTING, do_action, 0, 0},
                                           {"purr", POS_RESTING, do_action, 0, 0},


                                           {"quaff", POS_RESTING, do_use, 0, SCMD_QUAFF},
                                           {"quickthrow", POS_FIGHTING, do_fire, 0, SCMD_QUICKTHROW},
                                           {"qecho", POS_DEAD, do_qcomm, LVL_IMMORT, SCMD_QECHO},
                                           {"quest", POS_SLEEPING, do_autoquest, 0, 0},
                                           //{"quest", POS_SLEEPING, do_newquest_players	, 0, 0},
                                           //{"noquest", POS_DEAD, do_gen_tog, 0, SCMD_QUEST},
                                           {"qui", POS_DEAD, do_quit, 0, 0},
                                           {"quit", POS_DEAD, do_quit, 0, SCMD_QUIT},
                                           {"qsay", POS_RESTING, do_qcomm, 0, SCMD_QSAY},


                                           {"report", POS_SLEEPING, do_report, 0, 0},
                                           {"reply", POS_SLEEPING, do_reply, 0, 0},
                                           {"replay", POS_SLEEPING, do_replay, 0, 0},
                                           {"rlist", POS_DEAD, do_rlist, LVL_GOD, 0},
                                           {"rest", POS_RESTING, do_rest, 0, 0},
                                           {"respond", POS_RESTING, do_not_here, 0, 0},
                                           {"read", POS_RESTING, do_look, 0, SCMD_READ},
                                           {"recite", POS_SITTING, do_use, 0, SCMD_RECITE},
                                           {"receive", POS_STANDING, do_not_here, 1, 0},
                                           {"register", POS_STANDING, do_not_here, 0, 0},
                                           {"reincarn", POS_STANDING, do_not_here, 0, 0},
                                           {"reload", POS_DEAD, do_reboot, LVL_IMPL, 0},
                                           {"remove", POS_RESTING, do_remove, 0, 0},
                                           {"rage", POS_RESTING, do_action, 0, 0},
                                           {"ren", POS_RESTING, do_action, 0, 0},
                                           {"rent", POS_STANDING, do_not_here, 1, 0},
                                           //{"repair", POS_RESTING, do_repair, 0, 0},
                                           {"repair", POS_STANDING, do_not_here, 0, 0},
                                           {"resize", POS_SLEEPING, do_resize, -1, 0},
                                           {"reroll", POS_DEAD, do_wizutil, LVL_IMPL, SCMD_REROLL},
                                           {"rescue", POS_FIGHTING, do_rescue, 1, 0},
                                           {"resist", POS_RESTING, do_resistmagic, 0, 0},
                                           {"restore", POS_DEAD, do_restore, LVL_GRGOD, 0},

                                           {"return", POS_DEAD, do_return, 0, 0},
                                           {"retreat", POS_FIGHTING, do_retreat, -1, 0},                                           
                                           {"roll", POS_STANDING, do_not_here, 0, 0},
                                           {"rofl", POS_RESTING, do_action, 0, 0},
                                           {"roomflags", POS_DEAD, do_gen_tog, LVL_GOD, SCMD_ROOMFLAGS},
                                           {"rstat", POS_DEAD, do_rstat, LVL_GOD, 0},
#ifdef EVENT_MOVE
                                           {"run", POS_SLEEPING, do_run, -1, 0},
#endif
                                           {"ruffle", POS_STANDING, do_action, 0, 0},
                                           {"redit", POS_DEAD, do_olc, LVL_BUILDER, SCMD_OLC_REDIT},
                                           /*  { "rzedit"    , POS_DEAD   , do_rzedit   , LVL_IMPL, 0 }, */

                                           {"say", POS_RESTING, do_say, 0, 0},
                                           {"sayto", POS_RESTING, do_sayto, 0, 0},

                                           {"sacrifice", POS_STANDING, do_sac, 0, 0},
                                           {"save", POS_SLEEPING, do_save, 0, 0},
                                           {"score", POS_DEAD, do_score, 0, 0},
                                           {"scan", POS_RESTING, do_scan, 0, 0},
                                           {"scare", POS_FIGHTING, do_scare, -1, 0},
                                           {"scribe", POS_STANDING, do_scribe, -1, 0},
                                           {"scream", POS_RESTING, do_action, 0, 0},
                                           {"sell", POS_STANDING, do_not_here, 0, 0},
                                           {"send", POS_SLEEPING, do_send, LVL_GOD, 0},
                                           {"set", POS_DEAD, do_set, LVL_GRGOD, 0},
                                           {"sedit", POS_DEAD, do_olc, LVL_BUILDER, SCMD_OLC_SEDIT},
                                           {"setquest", POS_DEAD, do_setquest, LVL_GOD, SCMD_SET_QUEST_BIT},

                                           {"shout", POS_RESTING, do_gen_comm, 0, SCMD_SHOUT},
                                           {"yell", POS_RESTING, do_gen_comm, 0, SCMD_SHOUT},
                                           {"shoot", POS_STANDING, do_fire, 0, SCMD_FIRE},
                                           {"sharpen", POS_STANDING, do_sharpen, -1, 0},
                                           {"shake", POS_RESTING, do_action, 0, 0},
                                           {"shishkabob", POS_RESTING, do_action, 0, 0},
                                           {"shiver", POS_RESTING, do_action, 0, 0},
                                           {"show", POS_DEAD, do_show, 0, 0},
                                           {"showquest", POS_DEAD, do_showquest, LVL_GOD, 0},
                                           {"showegos", POS_DEAD, do_showegos, LVL_IMPL, 0},
                                           {"shrug", POS_RESTING, do_action, 0, 0},
                                           {"shudder", POS_RESTING, do_action, 0, 0},
                                           {"shutdow", POS_DEAD, do_shutdown, LVL_IMPL, 0},
                                           {"shutdown", POS_DEAD, do_shutdown, LVL_IMPL, SCMD_SHUTDOWN},
                                           {"sigh", POS_RESTING, do_action, 0, 0},
                                           {"sing", POS_RESTING, do_action, 0, 0},
                                           {"sip", POS_RESTING, do_drink, 0, SCMD_SIP},
                                           {"sit", POS_RESTING, do_sit, 0, 0},
                                           {"sks", POS_SLEEPING, do_skillset, LVL_GRGOD, 0},
                                           {"skg", POS_SLEEPING, do_skillget, LVL_GRGOD, 0},
                                           {"sleep", POS_SLEEPING, do_sleep, 0, 0},
                                           {"slap", POS_RESTING, do_action, 0, 0},
                                           {"slay", POS_FIGHTING, do_slay, LVL_IMPL, 0},
                                           {"slowns", POS_DEAD, do_gen_tog, LVL_IMPL, SCMD_SLOWNS},
                                           {"smile", POS_RESTING, do_action, 0, 0},
                                           {"smirk", POS_RESTING, do_action, 0, 0},
                                           {"snicker", POS_RESTING, do_action, 0, 0},
                                           {"snap", POS_RESTING, do_action, 0, 0},
                                           {"snarl", POS_RESTING, do_action, 0, 0},
                                           {"sneeze", POS_RESTING, do_action, 0, 0},
                                           {"sneak", POS_STANDING, do_sneak, -1, 0},
                                           {"sniff", POS_RESTING, do_action, 0, 0},
                                           {"snore", POS_SLEEPING, do_action, 0, 0},
                                           {"snowball", POS_STANDING, do_action, LVL_IMMORT, 0},
                                           {"snoop", POS_DEAD, do_snoop, LVL_GOD, 0},
                                           {"snooproom", POS_DEAD, do_snooproom, LVL_GOD, 0},
                                           {"snuggle", POS_RESTING, do_action, 0, 0},
                                           {"socials", POS_DEAD, do_commands, 0, SCMD_SOCIALS},
                                           {"split", POS_SITTING, do_split, 1, 0},
                                           {"spank", POS_RESTING, do_action, 0, 0},
                                           {"spew", POS_RESTING, do_action, 0, 0},
                                           {"spit", POS_STANDING, do_action, 0, 0},
                                           {"spin", POS_FIGHTING, do_spin_kick, -1, 0},
                                           {"spy", POS_STANDING, do_spy, 0, 0},
                                           {"squeeze", POS_RESTING, do_action, 0, 0},
                                           {"stand", POS_RESTING, do_stand, 0, 0},
                                           {"stalk", POS_STANDING, do_stalk, -1, 0},
                                           {"style", POS_FIGHTING, do_style, -1, 0},
                                           {"stun", POS_STANDING, do_stun, -1, 0},
                                           {"stop", POS_STANDING, do_not_here, 0, 0},
                                           {"stare", POS_RESTING, do_action, 0, 0},
                                           {"stat", POS_DEAD, do_stat, LVL_IMMORT, 0},
                                           {"steal", POS_STANDING, do_steal, -1, 0},
                                           {"steam", POS_RESTING, do_action, 0, 0},
                                           {"stimpy", POS_RESTING, do_action, 0, 0},
                                           {"struggle", POS_RESTING, do_struggle, 0, 0},
                                           {"stroke", POS_RESTING, do_action, 0, 0},
                                           {"strut", POS_STANDING, do_action, 0, 0},
                                           {"sulk", POS_RESTING, do_action, 0, 0},
                                           {"switch", POS_DEAD, do_switch, LVL_GOD, 0},
                                           {"sweepkick"  , POS_FIGHTING, do_sweepkick , -1, 0 },
                                           {"syslog", POS_DEAD, do_syslog, LVL_GRGOD, 0},
                                           {"shelp", POS_SLEEPING, do_spellhelp, 0, 0},
                                           {"slist", POS_SLEEPING, do_skills, 1, 0},
                                           {"skills", POS_SLEEPING, do_skills, 1, 0},
                                           {"spells", POS_SLEEPING, do_skills, 1, 0},



                                           {"tell", POS_DEAD, do_tell, 0, 0},
                                           {"teamtell", POS_SLEEPING, do_gen_comm, -1, SCMD_BROADCAST},
                                           {"tackle", POS_RESTING, do_action, 0, 0},
                                           {"take", POS_RESTING, do_get, 0, 0},
                                           {"tango", POS_STANDING, do_action, 0, 0},
                                           {"tap", POS_STANDING, do_action, 0, 0},
                                           {"taunt", POS_STANDING, do_taunt, 0, 0},
                                           {"taste", POS_RESTING, do_eat, 0, SCMD_TASTE},
                                           //    {"test", POS_STANDING, do_test, LVL_IMPL, 0},
                                           {"teleport", POS_DEAD, do_teleport, LVL_GOD, 0},
                                           {"thank", POS_RESTING, do_action, 0, 0},
                                           {"think", POS_RESTING, do_action, 0, 0},
                                           {"thaw", POS_DEAD, do_wizutil, LVL_FREEZE, SCMD_THAW},
                                           {"throw", POS_STANDING, do_fire, 0, SCMD_THROW},
                                           {"thumbsup", POS_STANDING, do_action, 0, 0},
                                           {"title", POS_DEAD, do_title, 0, 0},
                                           {"tickle", POS_RESTING, do_action, 0, 0},
                                           {"time", POS_DEAD, do_time, 0, 0},
                                           {"toggle", POS_DEAD, do_toggle, 0, 0},
                                           {"top", POS_SLEEPING, do_top, -1, 0},
                                           {"topdam", POS_SLEEPING, do_topdam, -1, 0},
                                           {"tophurt", POS_SLEEPING, do_tophurt, -1, 0},
                                           { "toad"     , POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_TOAD },
                                           { "toadoff"  , POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_TOADOFF },
                                           {"trip", POS_FIGHTING, do_trip, -1, 0},
                                           {"track", POS_STANDING, do_track, 0, 0},
                                           {"trap", POS_STANDING, do_trap, -1, 0},
                                           {"traps", POS_SLEEPING, do_traps, 0, 0},
                                           {"trip", POS_FIGHTING, do_trip, -1, 0},
                                           {"transfer", POS_SLEEPING, do_trans, LVL_GOD, 0},
                                           {"train", POS_STANDING, do_not_here, 0, 0},
                                           {"tumble", POS_SLEEPING, do_tumble, -1, 0},
                                           {"twiddle", POS_RESTING, do_action, 0, 0},
                                           {"typo", POS_DEAD, do_gen_write, 0, SCMD_TYPO},
                                           {"typolist", POS_DEAD, do_gen_ps, LVL_GRGOD, SCMD_TYPOLIST},

                                           {"unlock", POS_SITTING, do_gen_door, 0, SCMD_UNLOCK},
                                           {"ungroup", POS_DEAD, do_ungroup, 0, 0},
                                           {"unban", POS_DEAD, do_unban, LVL_GRGOD, 0},
                                           {"unaffect", POS_DEAD, do_wizutil, LVL_GOD, SCMD_UNAFFECT},
                                           {"unengrave", POS_STANDING, do_not_here, 0, 0},
                                           {"uptime", POS_DEAD, do_date, LVL_IMMORT, SCMD_UPTIME},
                                           {"use", POS_SITTING, do_use, 1, SCMD_USE},
                                           {"users", POS_DEAD, do_users, LVL_IMMORT, 0},

                                           {"victim", POS_DEAD, do_victim, 0, 0},
                                           {"value", POS_STANDING, do_not_here, 0, 0},
                                           {"version", POS_DEAD, do_gen_ps, 0, SCMD_VERSION},
                                           {"visible", POS_RESTING, do_visible, 1, 0},
                                           {"vnum", POS_DEAD, do_vnum, LVL_IMMORT, 0},
                                           {"vstat", POS_DEAD, do_vstat, LVL_IMMORT, 0},
                                           {"vwear", POS_DEAD, do_vwear, LVL_IMMORT, 0},


                                           {"wake", POS_SLEEPING, do_wake, 0, 0},
#ifdef EVENT_MOVE
                                           {"walk", POS_SLEEPING, do_walk, -1, 0},
#endif
                                           {"warm", POS_FIGHTING, do_warm, 0, 0},
                                           {"wave", POS_RESTING, do_action, 0, 0},
                                           {"wear", POS_RESTING, do_wear, 0, 0},
                                           {"weather", POS_RESTING, do_weather, 0, 0},
                                           {"wee", POS_RESTING, do_action, 0, 0},
                                           {"weep", POS_RESTING, do_action, 0, 0},
                                           {"who", POS_DEAD, do_who, 0, 0},
                                           {"whoami", POS_DEAD, do_gen_ps, 0, SCMD_WHOAMI},
                                           {"walkin", POS_SLEEPING, do_walkset, 0, SCMD_WALKIN},
                                           {"walkout", POS_SLEEPING, do_walkset, 0, SCMD_WALKOUT},
                                           {"worsezones", POS_SLEEPING, do_bwzone, 0, SCMD_WORSEZONE},
                                           {"whirlwind", POS_FIGHTING, do_whirlwind, -1, 0},
                                           {"where", POS_RESTING, do_where, LVL_IMMORT, 0},
                                           {"whisper", POS_RESTING, do_spec_comm, 0, SCMD_WHISPER},
                                           {"whimper", POS_RESTING, do_action, 0, 0},
                                           {"whine", POS_RESTING, do_action, 0, 0},
                                           {"whistle", POS_RESTING, do_action, 0, 0},
                                           {"wield", POS_RESTING, do_wield, 0, 0},
                                           {"wish", POS_STANDING, do_wish, 0, 0},
                                           {"wiggle", POS_STANDING, do_action, 0, 0},
                                           {"wimpy", POS_DEAD, do_wimpy, 0, 0},
                                           {"wink", POS_RESTING, do_action, 0, 0},
                                           {"withdraw", POS_STANDING, do_not_here, 1, 0},
                                           {"wiznet", POS_DEAD, do_wiznet, LVL_IMMORT, 0},
                                           {"warcry", POS_FIGHTING, do_enrage, 0, 0},

                                           {"wizhelp", POS_SLEEPING, do_commands, LVL_IMMORT, SCMD_WIZHELP},
                                           {"wizlist", POS_DEAD, do_gen_ps, 0, SCMD_WIZLIST},
                                           {"wizlock", POS_DEAD, do_wizlock, LVL_IMPL, 0},
                                           {"wlist", POS_SLEEPING, do_wlist, LVL_GRGOD, 0},
                                           {"worthy", POS_DEAD, do_worthy, 0, 0},
                                           {"worship", POS_RESTING, do_action, 0, 0},
                                           {"write", POS_STANDING, do_write, 1, 0},
                                           {"wrap", POS_SLEEPING, do_wrap, 0, 0},

                                           {"yawn", POS_RESTING, do_action, 0, 0},
                                           {"yodel", POS_RESTING, do_action, 0, 0},

                                           {"zone", POS_SLEEPING, do_zone, 1, 0},
                                           {"zap", POS_DEAD, do_zap, LVL_GRGOD, 0},
                                           {"zreset", POS_DEAD, do_zreset, LVL_GRGOD, 0},
                                           {"zedit", POS_DEAD, do_olc, LVL_BUILDER, SCMD_OLC_ZEDIT},
                                           {"zstat", POS_DEAD, do_zstat, LVL_GOD, 0},
                                           {"zallow", POS_DEAD, do_zallow, LVL_IMPL, 0},
                                           {"zdeny", POS_DEAD, do_zdeny, LVL_IMPL, 0},


                                           {"mpstat", POS_DEAD, do_mpstat, LVL_GOD, 0},
                                           {"opstat", POS_DEAD, do_opstat, LVL_GOD, 0},
                                           {"rpstat", POS_DEAD, do_rpstat, LVL_GOD, 0},



                                           //socials later added
                                           {"ack", POS_RESTING, do_action, 0, 0},
                                           {"ache", POS_RESTING, do_action, 0, 0},
                                           {"aww", POS_RESTING, do_action, 0, 0},
                                           {"beckon", POS_RESTING, do_action, 0, 0},
                                           {"blow", POS_RESTING, do_action, 0, 0},
                                           {"bored", POS_RESTING, do_action, 0, 0},
                                           {"caress", POS_RESTING, do_action, 0, 0},
                                           {"chill", POS_RESTING, do_action, 0, 0},
                                           {"clip", POS_RESTING, do_action, 0, 0},
                                           {"comprehend", POS_RESTING, do_action, 0, 0},
                                           {"daze", POS_RESTING, do_action, 0, 0},
                                           {"court", POS_RESTING, do_action, 0, 0},
                                           {"disturb", POS_RESTING, do_action, 0, 0},
                                           {"despair", POS_RESTING, do_action, 0, 0},
                                           {"describe", POS_STANDING, do_not_here, 0, 0},
                                           {"duck", POS_RESTING, do_action, 0, 0},
                                           {"eyebrow", POS_RESTING, do_action, 0, 0},
                                           {"excite", POS_RESTING, do_action, 0, 0},
                                           {"flutter", POS_RESTING, do_action, 0, 0},
                                           {"forehead", POS_RESTING, do_action, 0, 0},
                                           {"grunt", POS_RESTING, do_action, 0, 0},
                                           {"gyrate", POS_RESTING, do_action, 0, 0},
                                           {"halo", POS_RESTING, do_action, 0, 0},
                                           {"handbag", POS_RESTING, do_action, 0, 0},
                                           {"hickey", POS_RESTING, do_action, 0, 0},
                                           {"headbutt", POS_RESTING, do_action, 0, 0},
                                           {"howl", POS_RESTING, do_action, 0, 0},
                                           {"holdon", POS_RESTING, do_action, 0, 0},
                                           {"hmm", POS_RESTING, do_action, 0, 0},
                                           {"horn", POS_RESTING, do_action, 0, 0},
                                           {"hungry", POS_RESTING, do_action, 0, 0},
                                           {"kneel", POS_RESTING, do_action, 0, 0},
                                           {"lust", POS_RESTING, do_action, 0, 0},
                                           {"melt", POS_RESTING, do_action, 0, 0},
                                           {"moof", POS_RESTING, do_action, 0, 0},
                                           {"moon", POS_RESTING, do_action, 0, 0},
                                           {"neck", POS_RESTING, do_action, 0, 0},
                                           {"nose", POS_RESTING, do_action, 0, 0},
                                           {"pant", POS_RESTING, do_action, 0, 0},
                                           {"panic", POS_RESTING, do_action, 0, 0},
                                           {"peck", POS_RESTING, do_action, 0, 0},
                                           {"pimpslap", POS_RESTING, do_action, 0, 0},
                                           {"pin", POS_RESTING, do_action, 0, 0},
                                           {"piss", POS_RESTING, do_action, 0, 0},
                                           {"puzzle", POS_RESTING, do_action, 0, 0},
                                           {"quiver", POS_RESTING, do_action, 0, 0},
                                           {"raspberry", POS_RESTING, do_action, 0, 0},
                                           {"ready", POS_RESTING, do_action, 0, 0},
                                           {"roar", POS_RESTING, do_action, 0, 0},
                                           {"rock", POS_RESTING, do_action, 0, 0},
                                           {"romance", POS_RESTING, do_action, 0, 0},
                                           {"rub", POS_RESTING, do_action, 0, 0},
                                           {"scratch", POS_RESTING, do_action, 0, 0},
                                           {"shame", POS_RESTING, do_action, 0, 0},
                                           {"shishkabab", POS_RESTING, do_action, 0, 0},
                                           {"shoulder", POS_RESTING, do_action, 0, 0},
                                           {"squeel", POS_RESTING, do_action, 0, 0},
                                           {"stomp", POS_RESTING, do_action, 0, 0},
                                           {"stretch", POS_RESTING, do_action, 0, 0},
                                           {"fireball", POS_RESTING, do_action, 0, 0},
                                           {"seduce", POS_RESTING, do_action, 0, 0},
                                           {"smug", POS_RESTING, do_action, 0, 0},
                                           {"squirm", POS_RESTING, do_action, 0, 0},
                                           {"tip", POS_RESTING, do_action, 0, 0},
                                           {"tutt", POS_RESTING, do_action, 0, 0},
                                           {"urinate", POS_RESTING, do_action, 0, 0},
                                           {"violin", POS_RESTING, do_action, 0, 0},
                                           {"waterpistol", POS_RESTING, do_action, 0, 0},
                                           {"wed", POS_RESTING, do_action, 0, 0},
                                           {"wedgie", POS_RESTING, do_action, 0, 0},
                                           {"whack", POS_RESTING, do_action, 0, 0},
                                           {"whip", POS_RESTING, do_action, 0, 0},
                                           {"whoosh", POS_RESTING, do_action, 0, 0},
                                           {"white", POS_RESTING, do_action, 0, 0},
                                           {"wrinkle", POS_RESTING, do_action, 0, 0},


                                           {"mpasound", POS_DEAD, do_mpasound, -2, 0},
                                           {"mpjunk", POS_DEAD, do_mpjunk,-2, 0},
                                           {"mpecho", POS_DEAD, do_mpecho, -2, 0},
                                           {"mpechoat", POS_DEAD, do_mpechoat, -2, 0},
                                           {"mea", POS_DEAD, do_mpechoat, -2, 0},
                                           {"mpechoaround", POS_DEAD, do_mpechoaround, -2, 0},
                                           {"mer", POS_DEAD, do_mpechoaround, -2, 0},
                                           {"mpkill", POS_DEAD, do_mpkill, -2, 0},
                                           {"mpmload", POS_DEAD, do_mpmload, -2, 0},
                                           {"mpoload", POS_DEAD, do_mpoload, -2, 0},
                                           {"mppurge", POS_DEAD, do_mppurge, -2, 0},
                                           {"mpgoto", POS_DEAD, do_mpgoto, -2, 0},
                                           {"mpat", POS_DEAD, do_mpat, -2, 0},
                                           {"mptransfer", POS_DEAD, do_mptransfer, -2, 0},
                                           {"mpforce", POS_DEAD, do_mpforce, -2, 0},


                                           {"mpdamage", POS_DEAD, do_mp_damage, -2, 0},
                                           {"mpdelay", POS_DEAD, do_mpdelay, -2, 0},
                                           {"mprestore", POS_DEAD, do_mp_restore, -2, 0},
                                           {"mpnothing", POS_DEAD, do_mpnothing, -2, 0},
                                           {"mpdream", POS_DEAD, do_mpdream, -2, 0},
                                           {"mpslay", POS_DEAD, do_mp_slay, -2, 0},
                                           {"mppractice", POS_DEAD, do_mp_practice, -2, 0},
                                           {"mpscatter", POS_DEAD, do_mpscatter, -2, 0},
                                           {"mppeace", POS_DEAD, do_mppeace, -2, 0},
                                           {"mpechozone", POS_DEAD, do_mpechozone, -2, 0},
                                           {"mez", POS_DEAD, do_mpechozone, -2, 0},
                                           
                                           {"mpwait", POS_DEAD, do_mpwait, -2, 0},
                                           {"mpwaitstate", POS_DEAD, do_mpwaitstate, -2, 0},

                                           {"mpdeposit", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpinvis", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpapply", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpadvance", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpapplyb", POS_DEAD, do_mpsound, -2, 0},
                                           {"mppkset", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpclosepassage", POS_DEAD, do_mp_close_passage, -2, 0},
                                           {"mpopenpassage", POS_DEAD, do_mp_open_passage, -2, 0},
                                           {"mpwithdraw", POS_DEAD, do_mpsound, -2, 0},
                                           {"mppardon", POS_DEAD, do_mpsound, -2, 0},
                                           {"mplog", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpasupress", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpmorph", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpunmorph", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpunnuisance", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpnuisance", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpbodybag", POS_DEAD, do_mpsound, -2, 0},
                                           {"mpfillin", POS_DEAD, do_mpsound, -2,0},
                                           
                                           {"wait", POS_DEAD, do_mpwait, -2, 0},

                                           {"\n", 0, 0, 0, 0}};		/* this must be last */


char *fill[] =
    {
        "in",
        "from",
        "with",
        "the",
        "on",
        "at",
        "to",
        "\n"
    };

char *reserved[] =
    {
        "a",
        "an",
        "self",
        "me",
        "all",
        "room",
        "someone",
        "something",
        "\n"
    };

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */                       

#define MAX_TYPOS  7 


struct typo_type
{
    char *typo;
};


const struct typo_type typo_table [MAX_TYPOS] =
    {

        { "Yer wot?" },
        { "I'm sorry, I don't understand!" },
        { "Sorry, can you repeat that?" },
        { "What was that?" },
        { "Stop mumbling..." },
        { "Huh?!?"},
        { "Huh?!?"}
    };

void do_rand_typo( struct char_data *ch )
{
    char buf[MAX_STRING_LENGTH];
    int number1;

    number1 = number( 1, MAX_TYPOS ) -1;

    sprintf ( buf, "%s\r\n", typo_table[number1].typo);

    send_to_char ( buf, ch );
    return;
}


extern int global_action;
char last_command[10000];
void command_interpreter(struct char_data * ch, char *argument)
{
    int cmd, length;
    extern int no_specials;
    char *line;


    if (DEAD(ch))
    {
        logs("ERROR: dead man wants to do something - %s %s", GET_NAME(ch), argument);
        return;
    }

    if(IS_NPC(ch) && GET_WAIT_EVENT(ch))
    {

        strcat(ch->wait_buffer, argument);
        strcat(ch->wait_buffer, "\n");
        return;
    }

    if (!AFF2_FLAGGED(ch, AFF2_MOVE_HIDDEN) && AFF_FLAGGED(ch, AFF_HIDE))
        //REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);
        affect_from_char(ch, SKILL_HIDE);

    if (AFF3_FLAGGED(ch, AFF3_AMBUSH))
    {
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_AMBUSH);
        if (ch->ambush_name)
            DISPOSE(ch->ambush_name);
        send_to_char("You abort the ambush.\r\n", ch);
    }
    /*if (ch->listening_to && !IS_IMMORT(ch)) {
        struct char_data *temp;
        REMOVE_FROM_LIST(ch, world[ch->listening_to].listeners, next_listener);
        ch->listening_to = 0;
        send_to_char("You stop listening.\r\n", ch);
    } */
    /* just drop to next line for hitting CR */
    skip_spaces(&argument);
    if (!*argument)
        return;
    if (!global_action)
    {
        mprog_action_trigger(argument, ch);
        oprog_action_trigger(argument, ch);
        rprog_action_trigger(argument, ch);
        if (global_action==1)
        {
            global_action=0;
            return;
        }
        global_action=0;
    }
    /* special case to handle one-character, non-alphanumeric commands;
       requested by many people so "'hi" or ";godnet test" is possible.
       Patch sent by Eric Green and Stefan Wasilewski. */
    if (!isalpha(*argument) && !isdigit(*argument)) {
        arg[0] = argument[0];
        arg[1] = '\0';
        line = argument + 1;
    } else
        line = any_one_arg(argument, arg);


    if (!IS_NPC(ch)) {
        if (!PRF2_FLAGGED(ch, PRF2_NOQUIT))
        {
            if (*arg && !isname("quit", arg))
            {
                SET_BIT(PRF2_FLAGS(ch), PRF2_NOQUIT);
                send_to_char("Quit protection off.\r\n", ch);
            }
        }
        /* If someone just typed a number, then put that number in the
          argument and put 'goto' in the command string. -gg                            */
        if (GET_LEVEL(ch) >= LVL_IMMORT && isdigit(*arg)) {
            strcpy(line, arg);
            strcpy(arg, "goto");
        }
    }


    /* otherwise, find the command */
    for (length = strlen(arg), cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
        if (!strncmp(cmd_info[cmd].command, arg, length))
            if (GET_LEVEL(ch) >= cmd_info[cmd].minimum_level)
                //if ((!IS_NPC(ch) && GET_LEVEL(ch) >= cmd_info[cmd].minimum_level)) || (IS_NPC(ch) && cmd_info[cmd].minimum_level!=-1))
                break;

    if (*cmd_info[cmd].command == '\n')
        do_rand_typo(ch);
    else if ((PLR_FLAGGED(ch, PLR_FROZEN) || (ch->in_room==r_frozen_start_room)) && GET_LEVEL(ch) < LVL_IMPL)
        send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
    else if (PLR_FLAGGED(ch, PLR_TOAD) && GET_LEVEL(ch) < LVL_IMPL) {
        send_to_char("Ribbit. Ribbit. You hop aimlessly around, eyeing flies.\r\n", ch);
        sprintf(buf2, "&GAn ugly toad hops about aimlessly, looking for flies.&0");
        act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    }

    else if (cmd_info[cmd].command_pointer == NULL)
        send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
    else if (IS_NPC(ch) && cmd_info[cmd].minimum_level >= LVL_IMMORT)
        send_to_char("You can't use immortal commands while switched.\r\n", ch);
    else if (GET_POS(ch) < cmd_info[cmd].minimum_position)
        switch (GET_POS(ch)) {
        case POS_DEAD:
            send_to_char("Lie still; you are DEAD!!! :-(\r\n", ch);
            break;
        case POS_INCAP:
        case POS_MORTALLYW:
            send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
            break;
        case POS_STUNNED:
            send_to_char("All you can do right now is think about the stars!\r\n", ch);
            break;
        case POS_SLEEPING:
            send_to_char("In your dreams, or what?\r\n", ch);
            break;
        case POS_RESTING:
            send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
            break;
        case POS_SITTING:
            send_to_char("Maybe you should get on your feet first?\r\n", ch);
            break;
        case POS_FIGHTING:
            send_to_char("No way!  You're fighting for your life!\r\n", ch);
            break;
        }
    else if (no_specials || !special(ch, cmd, line)){
        sprintf(last_command, "%s (#%d %s) F: %s cmd: %s", GET_NAME(ch), world[ch->in_room].number,world[ch->in_room].name, FIGHTING(ch)?GET_NAME(FIGHTING(ch)):"noone", argument);
        if (!IS_NPC(ch) && GET_LEVEL(ch)>50 && argument[1])
        {
            time_t          ct;
            char           *tmstr;

            ct = time(0);
            tmstr = asctime(localtime(&ct));
            *(tmstr + strlen(tmstr) - 1) = '\0';
            fprintf(ff, "%-15.15s %s: %s\n",  tmstr + 4, GET_NAME(ch), argument);
        }
        ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));


    }


}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias *find_alias(struct alias * alias_list, char *str)
{
    while (alias_list != NULL) {
        /*    if (*str == *alias_list->alias)	hey, every little bit counts :-) */
        if (!strcmp(str, alias_list->alias))
            return alias_list;

        alias_list = alias_list->next;
    }

    return NULL;
}


void free_alias(struct alias * a)
{
    if (a->alias)
        DISPOSE(a->alias);
    if (a->replacement)
        DISPOSE(a->replacement);
    DISPOSE(a);
}

/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
    char *repl;
    struct alias *a, *temp;

    if (IS_NPC(ch))
        return;

    repl = any_one_arg(argument, arg);

    if (!*arg) {		/* no argument specified -- list currently
           defined aliases */
        send_to_char("Currently defined aliases:\r\n", ch);
        if ((a = GET_ALIASES(ch)) == NULL)
            send_to_char(" None.\r\n", ch);
        else {
            while (a != NULL) {
                sprintf(buf, "%-15s %s\r\n", a->alias, a->replacement);
                send_to_char(buf, ch);
                a = a->next;
            }
        }
    } else {			/* otherwise, add or remove aliases */
        /* is this an alias we've already defined? */
        if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
            REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
            free_alias(a);
        }
        /* if no replacement string is specified, assume we want to delete */
        if (!*repl) {
            if (a == NULL)
                send_to_char("No such alias.\r\n", ch);
            else
                send_to_char("Alias deleted.\r\n", ch);
        } else {		/* otherwise, either add or redefine an
            			   alias */
            if (!str_cmp(arg, "alias")) {
                send_to_char("You can't alias 'alias'.\r\n", ch);
                return;
            }
            if ((strlen(repl) > 120) || (strlen(arg) > 120)) {
                send_to_char("Your alias can't be more than 120 characters.\r\n", ch);
                return;
            }
            CREATE(a, struct alias, 1);
            a->alias = str_dup(arg);
            delete_doubledollar(repl);
            a->replacement = str_dup(repl);
            if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
                a->type = ALIAS_COMPLEX;
            else
                a->type = ALIAS_SIMPLE;
            a->next = GET_ALIASES(ch);
            GET_ALIASES(ch) = a;
            send_to_char("Alias added.\r\n", ch);
        }
    }
}

/*
 * Valid numeric replacements are only &1 .. &9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "&*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q * input_q, char *orig, struct alias * a)
{
    struct txt_q temp_queue;
    char *tokens[NUM_TOKENS], *temp, *write_point;
    int num_of_tokens = 0, num;

    /* First, parse the original string */
    temp = strtok(strcpy(buf2, orig), " ");
    while (temp != NULL && num_of_tokens < NUM_TOKENS) {
        tokens[num_of_tokens++] = temp;
        temp = strtok(NULL, " ");
    }

    /* initialize */
    write_point = buf;
    temp_queue.head = temp_queue.tail = NULL;

    /* now parse the alias */
    for (temp = a->replacement; *temp; temp++) {
        if (*temp == ALIAS_SEP_CHAR) {
            *write_point = '\0';
            buf[MAX_INPUT_LENGTH - 1] = '\0';
            write_to_q(buf, &temp_queue, 1);
            write_point = buf;
        } else if (*temp == ALIAS_VAR_CHAR) {
            temp++;
            if ((num = *temp - '1') < num_of_tokens && num >= 0) {
                strcpy(write_point, tokens[num]);
                write_point += strlen(tokens[num]);
            } else if (*temp == ALIAS_GLOB_CHAR) {
                strcpy(write_point, orig);
                write_point += strlen(orig);
            } else if ((*(write_point++) = *temp) == '$')	/* redouble $ for act
                			   safety */
                *(write_point++) = '$';
        } else
            *(write_point++) = *temp;
    }

    *write_point = '\0';
    buf[MAX_INPUT_LENGTH - 1] = '\0';
    write_to_q(buf, &temp_queue, 1);

    /* push our temp_queue on to the _front_ of the input queue */
    if (input_q->head == NULL)
        *input_q = temp_queue;
    else {
        temp_queue.tail->next = input_q->head;
        input_q->head = temp_queue.head;
    }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data * d, char *orig)
{
    char first_arg[MAX_INPUT_LENGTH], *ptr;
    struct alias *a, *tmp;

    /* bail out immediately if the guy doesn't have any aliases */
    if ((tmp = GET_ALIASES(d->character)) == NULL)
        return 0;

    /* find the alias we're supposed to match */
    ptr = any_one_arg(orig, first_arg);

    /* bail out if it's null */
    if (!*first_arg)
        return 0;

    /* if the first arg is not an alias, return without doing anything */
    if ((a = find_alias(tmp, first_arg)) == NULL)
        return 0;

    if (a->type == ALIAS_SIMPLE) {
        strcpy(orig, a->replacement);
        return 0;
    } else {
        perform_complex_alias(&d->input, ptr, a);
        return 1;
    }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, char **list, bool exact)
{
    register int i, l;

    /* Make into lower case, and get length of string */
    for (l = 0; *(arg + l); l++)
        *(arg + l) = LOWER(*(arg + l));

    if (exact) {
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strcmp(arg, *(list + i)))
                return (i);
    } else {
        if (!l)
            l = 1;		/* Avoid "" to match the first available
          string */
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strncmp(arg, *(list + i), l))
                return (i);
    }

    return -1;
}


int is_number(char *str)
{
    if (!str || !*str)		/* Test for NULL pointer or string. */
        return FALSE;
    if (*str == '-')		/* -'s in front are valid. */
        str++;
    while (*str)
        if (!isdigit(*(str++)))
            return FALSE;

    return TRUE;
}


void skip_spaces(char **string)
{
    for (; **string && isspace(**string); (*string)++);
}


char *delete_doubledollar(char *string)
{
    char *read, *write;

    if ((write = strchr(string, '$')) == NULL)
        return string;

    read = write;

    while (*read)		/* Until we reach the end of the string... */
        if ((*(write++) = *(read++)) == '$')	/* copy one char */
            if (*read == '$')
                read++;		/* skip if we saw 2 $'s in a row */

    *write = '\0';

    return string;
}


int fill_word(char *argument)
{
    return (search_block(argument, fill, TRUE) >= 0);
}


int reserved_word(char *argument)
{
    return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
        argument++;

    cEnd = ' ';
    //if ( *argument == '\'' || *argument == '"' )
    //cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *arg_first = LOWER(*argument);
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
        argument++;

    return argument;
}
char *one_argument_old(char *argument, char *first_arg)
{
    char *begin = first_arg;
    if (!argument) {
        log("SYSERR: one_argument received a NULL pointer.");
        *first_arg = '\0';
        return NULL;
    }
    do {
        skip_spaces(&argument);

        first_arg = begin;
        while (*argument && !isspace(*argument)) {
            *(first_arg++) = LOWER(*argument);
            argument++;
        }

        *first_arg = '\0';
    } while (fill_word(begin));

    return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
    skip_spaces(&argument);

    while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
    }

    *first_arg = '\0';

    return argument;
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
    return one_argument(one_argument(argument, first_arg), second_arg);	/* :-) */
}


/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 *
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(char *arg1, char *arg2)
{
    if (!*arg1)
        return 0;

    for (; *arg1 && *arg2; arg1++, arg2++)
        if (LOWER(*arg1) != LOWER(*arg2))
            return 0;

    if (!*arg1)
        return 1;
    else
        return 0;
}



/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
    char *temp;

    temp = any_one_arg(string, arg1);
    skip_spaces(&temp);
    strcpy(arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(char *command)
{
    int cmd;

    for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
        if (!strcmp(cmd_info[cmd].command, command))
            return cmd;

    return -1;
}


int special(struct char_data * ch, int cmd, char *arg)
{
    register struct obj_data *i;
    register struct char_data *k;
    int j;

    if (DEAD(ch))
        return 0;
    /* special in room? */
    if (GET_ROOM_SPEC(ch->in_room) != NULL)
        if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
            return 1;

    /* special in equipment list? */
    for (j = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
            if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
                return 1;

    /* special in inventory? */
    for (i = ch->carrying; i; i = i->next_content)
        if (GET_OBJ_SPEC(i) != NULL)
            if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
                return 1;

    /* special in mobile present? */
    for (k = world[ch->in_room].people; k; k = k->next_in_room)
        if (GET_MOB_SPEC(k) != NULL)
            if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
                return 1;

    /* special in object present? */
    for (i = world[ch->in_room].contents; i; i = i->next_content)
        if (GET_OBJ_SPEC(i) != NULL)
            if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
                return 1;

    return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
    int i;

    for (i = 0; i <= top_of_p_table; i++) {
        if (!str_cmp((player_table + i)->name, name))
            return i;
    }

    return -1;
}


int _parse_name(char *arg, char *name)
{
    int i;

    /* skip whitespaces */
    for (; isspace(*arg); arg++);

    for (i = 0; (*name = *arg); arg++, i++, name++)
        if (!isalpha(*arg))
            return 1;

    if (!i)
        return 1;

    return 0;
}

int cst(int poc, int max)
{
    return (19 - max + poc);
}

int hmc(int d, int poc, int max)
{
    int b, i, sum;
    sum = 0;
    if (d > 0) {
        i = poc;
        for (; d--; d > 0) {
            sum += cst(i, max);
            i++;
        }
        return (sum);
    } else if (d < 0) {
        d = -d;
        i = poc - 1;
        for (; d--; d > 0) {
            sum += cst(i, max);
            i--;
        }
        return (-sum);
    } else
        return (sum);
}

void display_stats(struct descriptor_data * d)
{
    sprintf(buf, "\033[H\033[J\r\n\r\n               &CAttribute customization&0\r\n               &c--------- -------------&0\r\n\r\n\r\nStat              Current     Racial max    Cost to inc\r\n-------------------------------------------------------&0\r\n\r\n");
    SEND_TO_Q(buf, d);
    sprintf(buf, "[&CS&0]&Ctrength          &G%-2i&B            %-2i&R             %-2i&0\r\n", GET_STRR(d->character), race_app[GET_RACE(d->character)].str, cst(GET_STRR(d->character), race_app[GET_RACE(d->character)].str));
    SEND_TO_Q(buf, d);
    sprintf(buf, "[&CI&0]&Cntelligence      &G%-2i&B            %-2i&R             %-2i&0\r\n", GET_INTR(d->character), race_app[GET_RACE(d->character)].intel, cst(GET_INTR(d->character), race_app[GET_RACE(d->character)].intel));
    SEND_TO_Q(buf, d);
    sprintf(buf, "[&CW&0]&Cill power        &G%-2i&B            %-2i&R             %-2i&0\r\n", GET_WISR(d->character), race_app[GET_RACE(d->character)].wis, cst(GET_WISR(d->character), race_app[GET_RACE(d->character)].wis));
    SEND_TO_Q(buf, d);
    sprintf(buf, "[&CD&0]&Cexterity         &G%-2i&B            %-2i&R             %-2i&0\r\n", GET_DEXR(d->character), race_app[GET_RACE(d->character)].dex, cst(GET_DEXR(d->character), race_app[GET_RACE(d->character)].dex));
    SEND_TO_Q(buf, d);
    sprintf(buf, "[&CC&0]&Constitution      &G%-2i&B            %-2i&R             %-2i&0\r\n", GET_CONR(d->character), race_app[GET_RACE(d->character)].con, cst(GET_CONR(d->character), race_app[GET_RACE(d->character)].con));
    SEND_TO_Q(buf, d);
    SEND_TO_Q("\r\n[&CF&0]&Cinish&0  [&C?&0] &CHelp.&0\r\n", d);
    sprintf(buf, "\r\n&cYou have &C%i&c points to add.&0\r\n", d->stat_points);
    SEND_TO_Q(buf, d);
    SEND_TO_Q("&cStat to modify (type s,i,w,d,c or f,?):&0 ", d);
}



void send_email(struct descriptor_data * d )
{
    char                 tmp  [ MAX_INPUT_LENGTH ];
    char             confirm  [ 10 ];
    FILE*                 fp;
    int                    i;



    for( i = 0; i < 8; i++ )
        confirm[i] = 'a'+number( 0, 25 );
    confirm[8] = '\0';

    strncpy(GET_PASSWD(d->character), confirm, MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';


    if( ( fp = fopen( "email.msg", "w" ) ) == NULL )
        return;


    fprintf( fp, "A request was entered at Lands of Myst MUD to create\n" );
    fprintf( fp, "a new character for this email address. If this is\n" );
    fprintf( fp, "indeed the case, then can use the following data\n" );
    fprintf( fp, "to log your new character.\n\n" );
    fprintf( fp, "Name    : %s\nPassword: %s\n\n\n",GET_NAME(d->character), GET_PASSWD(d->character));
    fprintf( fp, "- You can always change your password (press '4' in\n  the main menu).\n" );
    fprintf( fp, "- If this is your first character in LOM it\n" );
    fprintf( fp, "  is advisable that you type 'help newbie' once you\n  enter the mud world.\n\n" );
    fprintf( fp, "Have fun!\n\n\n\n\n\n" );
    fprintf( fp, "* Lands of Myst MUD\n* 148.251.247.173 4000\n" );


    fclose( fp );

    sprintf( tmp,
             "cat email.msg | mail -s 'Lands of Myst MUD' %s", d->character->player_specials->saved.email );

    system( tmp );

    return;
}



#define RECON		1
#define USURP		2
#define UNSWITCH	3

/*
 * XXX: Make immortals 'return' instead of being disconnected when switched
 *      into person returns.  This function seems a bit over-extended too.
 */
int perform_dupe_check(struct descriptor_data *d)
{
    struct descriptor_data *k, *next_k;
    struct char_data *target = NULL, *ch, *next_ch;
    int mode = 0;

    int id = GET_IDNUM(d->character);
    int pref_temp=0;
    /*
     * Now that this descriptor has successfully logged in, disconnect all
     * other descriptors controlling a character with the same ID number.
     */

    for (k = descriptor_list; k; k = next_k) {
        next_k = k->next;

        if (k == d)
            continue;

        if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
            SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
            STATE(k) = CON_CLOSE;
            pref_temp=GET_PREF(k->character);
            if (!target) {
                target = k->original;
                mode = UNSWITCH;
            }
            if (k->character)
                k->character->desc = NULL;
            k->character = NULL;
            k->original = NULL;
        } else if (k->character && (GET_IDNUM(k->character) == id)) {
            pref_temp=GET_PREF(k->character);
            if (!target && STATE(k) == CON_PLAYING) {
                SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
                target = k->character;
                mode = USURP;
            }
            k->character->desc = NULL;
            k->character = NULL;
            k->original = NULL;
            SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
            STATE(k) = CON_CLOSE;
        }
    }

    /*
     * now, go through the character list, deleting all characters that
     * are not already marked for deletion from the above step (i.e., in the
     * CON_HANGUP state), and have not already been selected as a target for
     * switching into.  In addition, if we haven't already found a target,
     * choose one if one is available (while still deleting the other
     * duplicates, though theoretically none should be able to exist).
     */

    for (ch = character_list; ch; ch = next_ch) {
        next_ch = ch->next;

        if (IS_NPC(ch))
            continue;
        if (GET_IDNUM(ch) != id)
            continue;

        /* ignore chars with descriptors (already handled by above step) */
        if (ch->desc)
            continue;

        /* don't extract the target char we've found one already */
        if (ch == target)
            continue;

        /* we don't already have a target and found a candidate for switching */
        if (!target) {
            target = ch;
            mode = RECON;
            continue;
        }

        /* we've found a duplicate - blow him away, dumping his eq in limbo. */
        if (ch->in_room != NOWHERE)
            char_from_room(ch);
        char_to_room(ch, 1);
        extract_char(ch);
    }

    /* no target for swicthing into was found - allow login to continue */
    /* stupid-case rule for setting hostname on char:
    any time you set the char's pref .. */

    if (!target) {
        GET_HOST(d->character) = strdup(d->host);
        GET_PREF(d->character)=  get_new_pref();
        return 0;
    } else {
        if(GET_HOST(target)) {
            DISPOSE(GET_HOST(target));
        }
        GET_HOST(target) = strdup(d->host);
        GET_PREF(target)=pref_temp;
        add_llog_entry(target,LAST_RECONNECT);
    }
    /* Okay, we've found a target.  Connect d to target. */
    free_char(d->character); /* get rid of the old char */
    d->character = target;
    d->character->desc = d;
    d->original = NULL;
    d->character->char_specials.timer = 0;
    REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
    REMOVE_BIT(AFF_FLAGS(d->character), AFF_GROUP);
    STATE(d) = CON_PLAYING;

    switch (mode) {
    case RECON:
        SEND_TO_Q("Reconnecting.\r\n", d);
        act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
        sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
        break;
    case USURP:
        SEND_TO_Q("You take over your own body, already in use!\r\n", d);
        act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
            "$n's body has been taken over by a new spirit!",
            TRUE, d->character, 0, 0, TO_ROOM);
        sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
                GET_NAME(d->character));
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
        break;
    case UNSWITCH:
        SEND_TO_Q("Reconnecting to unswitched char.", d);
        sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
        break;
    }

    return (1);
}

void log_character(struct descriptor_data *d, int mode)
{       
	    char buf[128];
    int player_i, load_result;
    int i;
    char tmp_name[MAX_INPUT_LENGTH];
    struct char_file_u tmp_store;
    int points;
    struct char_data *tmp_ch;
    struct descriptor_data *k, *next;
    extern struct descriptor_data *descriptor_list;
    extern sh_int r_mortal_start_room;
    extern sh_int r_immort_start_room;

    extern const char *class_menu1;
    extern const char *class_menu2;
    extern const char *class_menu3;
    extern const char *class_menu4;
    extern const char *deity_menu;
    extern const char *race_menu1;
    extern const char *race_menu2;
    extern const char *race_menu3;
    extern const char *race_menu4;
    extern const char *race_menu5;
    extern const char *race_menu6;
    extern int max_bad_pws;
    sh_int load_room;

    int load_char(char *name, struct char_file_u * char_element);
    int parse_class(char arg, int race);
    int parse_race(char arg);
    int parse_deity(char arg);
    void roll_real_abils(struct char_data * ch);

	  /* this code is to prevent people from multiply logging in */
            for (k = descriptor_list; k; k = next) {
                next = k->next;
                if (!k->connected && k->character &&
                        !str_cmp(GET_NAME(k->character), GET_NAME(d->character))) {
                    SEND_TO_Q("Your character has been deleted.\r\n", d);
                    STATE(d) = CON_CLOSE;
                    return;
                }
            }
            reset_char(d->character);
            if (PLR_FLAGGED(d->character, PLR_INVSTART))
                GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);

            if (PLR_FLAGGED(d->character, PLR_FROZEN))
                load_room = r_frozen_start_room;

            if ((load_room = (GET_LOADROOM(d->character))) <= 0)
                if (GET_LEVEL(d->character) >= LVL_IMMORT)
                    load_room = r_immort_start_room;
                else
                    load_room = r_mortal_start_room;

            d->character->in_room=load_room;

            load_result = Crash_load(d->character);

            send_to_char(WELC_MESSG, d->character);
            d->character->next = character_list;
            character_list = d->character;

            if (!GET_LEVEL(d->character)) {
                load_room = real_room(d->character->player.hometown);//r_mortal_start_room;
                do_start(d->character);
                send_to_char(START_MESSG, d->character);
                do_newbie(d->character);
            }
            else if (load_room==NOWHERE)
                load_room=d->character->player.hometown>1?d->character->player.hometown:clan_loadroom[GET_CLAN(d->character)];
            char_to_room(d->character, load_room);

            /* if (AFF2_FLAGGED(d->character,AFF2_WRATH)){
            struct obj_data *tmpo;
            tmpo=read_object(WRATH_CLOUD,VIRTUAL,0);
            obj_to_room(tmpo,load_room);
            WRATHOBJ(d->character)=tmpo;} */
            REMOVE_BIT(PRF_FLAGS(d->character), PRF_QUEST);
            REMOVE_BIT(PLR_FLAGS(d->character), PLR_JUSTDIED);
            d->character->pk_timer=0;
            save_char(d->character, load_room);
            act("&C$n appears in a puff of smoke.&0", TRUE, d->character, 0, 0, TO_ROOM);
            STATE(d) = CON_PLAYING;
            if (GET_TERM(d->character) == VT100)
                InitScreen(d->character);
            if (mode==1)
            look_at_room(d->character, 0);
            if (d->character->player.level>6 && GET_CLAN(d->character)==CLAN_NEWBIE)
                GET_CLAN(d->character)=CLAN_OUTLAW;
            if (has_mail(GET_IDNUM(d->character)))
                send_to_char("&c&fYou have mail waiting&0.\r\n", d->character);
            if (GET_TELLAS(d->character)>0)
                ch_printf(d->character, "&cYou have sold %d items at travelling salesman.&0\r\n", GET_TELLAS(d->character));
            GET_TELLAS(d->character)=0;
            if (load_result == 2) /* rented items lost */
                send_to_char("\r\n\007You could not afford your rent!\r\nYour possesions have been donated to the Equipment Liberation Army!\r\n",d->character);
            d->has_prompt = 0;
            read_aliases(d->character);
            read_quests(d->character);
            sprintf(mbufi,"&C[Info] &GWelcome, %s.&0\r\n",GET_NAME(d->character));
            if (!IS_GOD(d->character))// || mode==2)
            	 INFO_OUT(mbufi);
}           
/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data * d, char *arg)
{
    char buf[128];
    int player_i, load_result;
    int i;
    char tmp_name[MAX_INPUT_LENGTH];
    struct char_file_u tmp_store;
    int points;
    struct char_data *tmp_ch;
    struct descriptor_data *k, *next;
    extern struct descriptor_data *descriptor_list;
    extern sh_int r_mortal_start_room;
    extern sh_int r_immort_start_room;

    extern const char *class_menu1;
    extern const char *class_menu2;
    extern const char *class_menu3;
    extern const char *class_menu4;
    extern const char *deity_menu;
    extern const char *race_menu1;
    extern const char *race_menu2;
    extern const char *race_menu3;
    extern const char *race_menu4;
    extern const char *race_menu5;
    extern const char *race_menu6;
    extern int max_bad_pws;
    sh_int load_room;

    int load_char(char *name, struct char_file_u * char_element);
    int parse_class(char arg, int race);
    int parse_race(char arg);
    int parse_deity(char arg);
    void roll_real_abils(struct char_data * ch);

    skip_spaces(&arg);

    switch (STATE(d)) {

        /* . OLC states . */
    case CON_OEDIT:
        oedit_parse(d, arg);
        break;
    case CON_REDIT:
        redit_parse(d, arg);
        break;
    case CON_ZEDIT:
        zedit_parse(d, arg);
        break;
    case CON_MEDIT:
        medit_parse(d, arg);
        break;
    case CON_SEDIT:
        sedit_parse(d, arg);
        break;
        /* . End of OLC states . */

    case CON_GET_NAME1:
        SEND_TO_Q("\r\nNow choose a name for yourself: ", d);
        STATE(d)=CON_GET_NAME;
        break;

    case CON_GET_NAME:		/* wait for input of name */
        if (d->character == NULL) {
            CREATE(d->character, struct char_data, 1);
            clear_char(d->character);
            CREATE(d->character->player_specials, struct player_special_data, 1);
            d->character->desc = d;
        }
        if (!*arg)
            close_socket(d);
        else {
            if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
                    strlen(tmp_name) > MAX_NAME_LENGTH ||
                    fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
                SEND_TO_Q("Invalid name, try another.\r\n"
                          "By what name do you wish to be known? ", d);
                return;
            }
            if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
                store_to_char(&tmp_store, d->character);
                GET_PFILEPOS(d->character) = player_i;

                if (PLR_FLAGGED(d->character, PLR_DELETED)) {
                    free_char(d->character);
                    CREATE(d->character, struct char_data, 1);
                    clear_char(d->character);
                    CREATE(d->character->player_specials, struct player_special_data, 1);
                    d->character->desc = d;
                    CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
                    strcpy(d->character->player.name, CAP(tmp_name));
                    GET_PFILEPOS(d->character) = player_i;
                    //		    page_string(d, namepol, 0);
                    sprintf(buf, "So, you want %s engraved on your tombstone (y/n)? ", tmp_name);
                    SEND_TO_Q(buf, d);
                    STATE(d) = CON_NAME_CNFRM;
                } else {
                    /* undo it just in case they are set */
                    REMOVE_BIT(PLR_FLAGS(d->character),
                               PLR_WRITING | PLR_MAILING | PLR_CRYO | PLR_EDITING);

                    //sprintf(buf, "* %s *\r\n", GET_PASSWD(d->character));
                    //SEND_TO_Q(buf, d);
                    SEND_TO_Q("Password: ", d);

                    echo_off(d);
                    d->idle_tics = 0;
                    STATE(d) = CON_PASSWORD;
                }
            } else {
                /* player unknown -- make new character */

                if (!Valid_Name(tmp_name)) {
                    SEND_TO_Q("Invalid name, try another.\r\n", d);
                    SEND_TO_Q("By what name do you wish to be known? ", d);
                    return;
                }
                CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
                strcpy(d->character->player.name, CAP(tmp_name));
                //	page_string(d, namepol, 0);
                sprintf(buf, "No character by name %s.\r\nDo you want to create a new character (y/n)? ", tmp_name);
                SEND_TO_Q(buf, d);
                STATE(d) = CON_NAME_CNFRM;
            }
        }
        break;
    case CON_NAME_CNFRM:	/* wait for conf. of new name	 */
        if (UPPER(*arg) == 'Y') {
            if (isbanned(d->host) >= BAN_NEW) {
                sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
                        GET_NAME(d->character), d->host);
                mudlog(buf, NRM, LVL_GOD, TRUE);
                SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
                STATE(d) = CON_CLOSE;
                return;
            }
            if (restrict1) {
                SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
                sprintf(buf, "Request for new char %s denied from %s (wizlock)",
                        GET_NAME(d->character), d->host);
                mudlog(buf, NRM, LVL_GOD, TRUE);
                STATE(d) = CON_CLOSE;
                return;
            }
            SEND_TO_Q("\r\nCreating new character.\r\n", d);
            page_string(d, namepol, 0);
            STATE(d)=CON_QNAMEPOL;
        } else if (*arg == 'n' || *arg == 'N') {
            SEND_TO_Q("Okay, what is thy name? ", d);
            DISPOSE(d->character->player.name);
            d->character->player.name = NULL;
            STATE(d) = CON_GET_NAME;
        } else {
            SEND_TO_Q("Please type Yes or No: ", d);
        }
        break;

    case CON_QNAMEPOL:
        if (UPPER(*arg) == 'Y') {
            //sprintf(buf, "Good.\r\n\r\nNow, %s, choose your secret password: ", GET_NAME(d->character));
            //SEND_TO_Q(buf, d);
            //echo_off(d);
            //    STATE(d) = CON_NEWPASSWD;
            SEND_TO_Q("\r\nCan your terminal display ANSI colors (y/n)?", d);
            STATE(d)=CON_QCOLOR;

        }
        else {
            page_string(d, goodnames, 0);
            STATE(d)=CON_GET_NAME1;
        }
        break;


    case CON_PASSWORD:		/* get pwd for known player	 */
        /* turn echo back on */
        echo_on(d);

        if (!*arg)
            close_socket(d);
        else {
            if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH) && strcmp(arg, "quick")) {
                sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
                mudlog(buf, BRF, LVL_GOD, TRUE);
                GET_BAD_PWS(d->character)++;
                /*//	save_char(d->character, NOWHERE);*/
                if (++(d->bad_pws) >= max_bad_pws) {	/* 3 strikes and you're
                    					   out. */
                    SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
                    STATE(d) = CON_CLOSE;
                } else {
                    SEND_TO_Q("Wrong password.\r\nPassword: ", d);
                    echo_off(d);
                }
                return;
            }
            load_result = GET_BAD_PWS(d->character);
            GET_BAD_PWS(d->character) = 0;
            d->bad_pws = 0;
            /*//        save_char(d->character, NOWHERE);*/

            if (isbanned(d->host) == BAN_SELECT &&
                    !PLR_FLAGGED(d->character, PLR_SITEOK)) {
                SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
                STATE(d) = CON_CLOSE;
                sprintf(buf, "Connection attempt for %s denied from %s",
                        GET_NAME(d->character), d->host);
                mudlog(buf, NRM, LVL_GOD, TRUE);
                return;
            }
            if (GET_LEVEL(d->character) < restrict1) {
                SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n", d);
                STATE(d) = CON_CLOSE;
                sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
                        GET_NAME(d->character), d->host);
                mudlog(buf, NRM, LVL_GOD, TRUE);
                return;
            }
            /* first, check to see if this person is already logged in,
               but switched.  If so, disconnect the switched persona. */

            if (perform_dupe_check(d))
                return;
            /*
                {
             int pref_temp=0;
                for (k = descriptor_list; k; k = k->next)
            	if (k->original && (GET_IDNUM(k->original) == GET_IDNUM(d->character))) {
            	    SEND_TO_Q("Disconnecting for return to unswitched char.\r\n", k);
            	    STATE(k) = CON_CLOSE;
             pref_temp=GET_PREF(k->character);		    
            	    
            	    
               if(GET_HOST(k->original)) {
                 DISPOSE(GET_HOST(k->original));
               }
               GET_HOST(k->original) = strdup(d->host);
               GET_PREF(k->original)=pref_temp;
               add_llog_entry(k->original,LAST_RECONNECT);
               
            	    free_char(d->character);
            	    d->character = k->original;
            	    d->character->desc = d;
            	    d->original = NULL;
            	    d->character->char_specials.timer = 0;
            	    if (k->character)
            		k->character->desc = NULL;
            	    k->character = NULL;
            	    k->original = NULL;
            	    SEND_TO_Q("Reconnecting to unswitched char.", d);
            	    REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING
            		       | PLR_EDITING);
            	    STATE(d) = CON_PLAYING;
            	    sprintf(buf, "%s [%s] has reconnected.",
            		    GET_NAME(d->character), d->host);
            	    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            	    return;
            	}
                 //now check for linkless and usurpable 
                for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
            	if (!IS_NPC(tmp_ch) &&
            	    GET_IDNUM(d->character) == GET_IDNUM(tmp_ch)) {
              pref_temp=GET_PREF(tmp_ch);		    	
              
            	    if (!tmp_ch->desc) {
            		SEND_TO_Q("Reconnecting.\r\n", d);
            		act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
            		sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
            		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            	    } else {
            		sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
            			GET_NAME(tmp_ch));
            		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(tmp_ch)), TRUE);
            		SEND_TO_Q("This body has been usurped!\r\n", tmp_ch->desc);
            		STATE(tmp_ch->desc) = CON_CLOSE;
            		tmp_ch->desc->character = NULL;
            		tmp_ch->desc = NULL;
            		SEND_TO_Q("You take over your own body, already in use!\r\n", d);
            		act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
            		"$n's body has been taken over by a new spirit!",
            		    TRUE, tmp_ch, 0, 0, TO_ROOM);
            	    }
            	   //  if player is editing, and gets usurped, nuke all
            	      // editing constructs 

               GET_HOST(d->character) = strdup(d->host); 
               GET_PREF(d->character)=  get_new_pref();
            	    free_char(d->character);
            	    tmp_ch->desc = d;
            	    d->character = tmp_ch;
            	    tmp_ch->char_specials.timer = 0;
            	    REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING | PLR_EDITING);

            	    STATE(d) = CON_PLAYING;
            	    if (GET_TERM(d->character) == VT100)
            		InitScreen(d->character);
            	    return;
            	}
        }		
            	*/


            //SEND_TO_Q("\r\n", d);
            SEND_TO_Q(CLRSCR, d);
            
            if (GET_LEVEL(d->character) >= LVL_IMMORT)
                SEND_TO_Q(imotd, d);
            else
                SEND_TO_Q(motd, d);

            sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
            mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);

            if (load_result) {
                sprintf(buf, "\r\r\n\n\007\007\007"
                        "%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
                        CCRED(d->character, C_SPR), load_result,
                        (load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
                SEND_TO_Q(buf, d);
            }
            SEND_TO_Q("\r\n\r\n\r\n*** PRESS ENTER: ", d);
            STATE(d) = CON_RMOTD;
        }
        break;

    case CON_NEWPASSWD:
    case CON_CHPWD_GETNEW :
        if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
                !str_cmp(arg, GET_NAME(d->character))) {
            SEND_TO_Q("\r\nIllegal password.\r\n", d);
            SEND_TO_Q("Password: ", d);
            return;
        }
        strncpy(GET_PASSWD(d->character), CRYPT(arg, GET_NAME(d->character)), MAX_PWD_LENGTH);
        *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

        SEND_TO_Q("\r\nPlease retype password: ", d);
        if (STATE(d) == CON_NEWPASSWD)
            STATE(d) = CON_CNFPASSWD;
        else
            STATE(d) = CON_CHPWD_VRFY;

        break;

    case CON_CNFPASSWD:
    case CON_CHPWD_VRFY:
        if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
                    MAX_PWD_LENGTH)) {
            SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
            SEND_TO_Q("Password: ", d);
            if (STATE(d) == CON_CNFPASSWD)
                STATE(d) = CON_NEWPASSWD;
            else
                STATE(d) = CON_CHPWD_GETNEW;
            save_char(d->character, NOWHERE);
            return;
        }
        echo_on(d);

        if (STATE(d) == CON_CNFPASSWD) {
            SEND_TO_Q("Can your terminal display ANSI colors (y/n)?", d);
            STATE(d)=CON_QCOLOR;
        } else {
            save_char(d->character, NOWHERE);
            echo_on(d);
            SEND_TO_Q("\r\nDone.\r\n", d);
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
        }

        break;

    case CON_QCOLOR:
        switch (*arg) {
        case 'y':
        case 'Y':
            SET_BIT(PRF_FLAGS(d->character), PRF_COLOR_1 | PRF_COLOR_2);
            SEND_TO_Q("&CGood. &cYou can always toggle your color status with the &Ccolor&c command.&0\r\n", d);
            break;
        case 'n':
        case 'N':
            REMOVE_BIT(PRF_FLAGS(d->character), PRF_COLOR_1 | PRF_COLOR_2);
            SEND_TO_Q("Okey. You can always toggle your color status with the &Ccolor&0 command.\r\n", d);
            break;
        default:
            SEND_TO_Q("Please answer yes or no.\r\n"
                      "Can your terminal display ANSI colors (y/n)? ", d);
            return;
            break;
        }


        SEND_TO_Q("\r\n\r\nWhat is the gender of your character - (&cM&0)&cale&0 or (&cF&0)&cemale&0 ? ", d);
        STATE(d) = CON_QSEX;
        break;

    case CON_QEMAIL:
        if (!*arg || strlen(arg)<5 || !strchr( arg,'@') || !strchr( arg,'.'))
        {
            SEND_TO_Q("That's not a valid adress.\r\nEnter your email address: ", d);
            STATE(d)=CON_QEMAIL;
        }
        else {

            strncpy(d->character->player_specials->saved.email, arg, 59);

            if (GET_PFILEPOS(d->character) < 0)
                    GET_PFILEPOS(d->character) = create_entry(GET_NAME(d->character));
                send_email(d);
                init_char(d->character);
                save_char(d->character, NOWHERE);
                sprintf(buf1, "\r\nYou have just created a new character on Lands of Myst.\r\nEmail was sent to your email adrress (%s)\r\ncontaining password for this character (make sure to check SPAM folder as well!). You should\r\nchange this password next time you log in with the\r\noption '4' in the main menu.\r\n\r\nClosing link...\r\n", d->character->player_specials->saved.email);
                SEND_TO_Q(buf1, d);
                //   SEND_TO_Q(motd, d);
                // SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
                //STATE(d) = CON_RMOTD;
                sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
                mudlog(buf, NRM, LVL_IMMORT, TRUE);
                STATE(d)=CON_CLOSE;
                break;
        }
        break;

    case CON_ASKTOWN:
	i=parse_ht(arg, d);
        if (i==-1)
        {
        	SEND_TO_Q("That's not a valid town.\r\n", d);
        	break;
        }                              
        d->character->player.hometown=i;
        SEND_TO_Q("\r\nConfirmation code for this charater will be now sent to your email address.\r\nEnter your email address: ",d);
        STATE(d)=CON_QEMAIL;

	break;
    
    
    case CON_QSEX:		/* query sex of new user	 */
        switch (*arg) {
        case 'm':
        case 'M':
            d->character->player.sex = SEX_MALE;
            break;
        case 'f':
        case 'F':
            d->character->player.sex = SEX_FEMALE;
            break;
        default:
            SEND_TO_Q("That is not an available gender.\r\n"
                      "What IS your gender? ", d);
            return;
            break;
        }


        SEND_TO_Q(race_menu1, d);
        SEND_TO_Q("\r\nSelect your race: ", d);
        STATE(d) = CON_QRACE;
        //SEND_TO_Q("\r\n\r\n\r\nNow choose your alignment.\r\n\r\nYour choice:  &B(G)ood&0, &G(N)eutral&0 or &R(E)vil&0? ",d);
        //STATE(d)=CON_QALIGN;
        break;

    case CON_QALIGN:
        switch (*arg) {
        case 'g':
        case 'G':
            GET_REAL_ALIGNMENT(d->character) =1000;
            break;
        case 'n':
        case 'N':
            GET_REAL_ALIGNMENT(d->character) =0;
            break;
        case 'e':
        case 'E':
            GET_REAL_ALIGNMENT(d->character) =-1000;
            break;
        default:
            SEND_TO_Q("That is not an available alignment.\r\n"
                      "Choose G, N or E : ", d);
            return;
            break;
        }
        SEND_TO_Q(race_menu1, d);
        SEND_TO_Q("\r\nRace: ", d);
        STATE(d) = CON_QRACE;
        break;


    case CON_QRACE:
        if (*arg=='h')
            switch (arg[1])
            {
            case '0': SEND_TO_Q(hum,d);SEND_TO_Q("\r\n Press Enter to continue...",d);STATE(d)=CON_RS;return;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': SEND_TO_Q("\r\nNo help availabe yet...\r\n\r\nSelect your race: ", d);
                return;
            default: SEND_TO_Q("\r\nNo help on that...\r\n\r\nSelect your race: ", d);
                return;

            }
        load_result = parse_race(*arg);
        if (load_result == -500) {
            SEND_TO_Q("\r\nThat race is not available to you.", d);
            SEND_TO_Q(race_menu1, d);
            SEND_TO_Q("\r\nSelect your race: ", d);
            return;
        }
        if (load_result < 0) {
            load_result+=1;
            load_result=-load_result;
            GET_REAL_ALIGNMENT(d->character) =-1000;
        }
        else
            GET_REAL_ALIGNMENT(d->character) =1000;
        /*        SEND_TO_Q("\r\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\n",d);*/
        d->stat_points = race_app[load_result].points;
        /*        switch (load_result){
                	case RACE_HUMAN:SEND_TO_Q(hum,d);break;
                        case RACE_ELF:SEND_TO_Q(elf,d);break;
                	case RACE_DWARF:SEND_TO_Q(dwa,d);break;
                	case RACE_GNOME:SEND_TO_Q(gno,d);break;
                	case RACE_OGRE:SEND_TO_Q(ogr,d);break;
                	case RACE_GOBLIN:SEND_TO_Q(helf,d);break;
                	case RACE_HALFLING:SEND_TO_Q(hob,d);break;
                       	case RACE_DROW:SEND_TO_Q(dro,d);break;
                	default: SEND_TO_Q("No help available.\r\n",d);
                }
           SEND_TO_Q("\r\r\n\nDo you want to have this race? ",d);
           GET_RACE(d->character) = load_result;
                STATE(d)=CON_RS;
                break;*/
        GET_RACE(d->character) = load_result;
        SEND_TO_Q(class_menu1, d);
        SEND_TO_Q("\r\nClass: ", d);
        STATE(d) = CON_QCLASS;
        break;


    case CON_RS:
        SEND_TO_Q(race_menu1, d);
        SEND_TO_Q("\r\nSelect your race: ", d);
        STATE(d) = CON_QRACE;
        break;
    
    case CON_ASKDEITY:
        if (*arg=='h')
            switch (arg[1])
            {
            
            case '1':send_desc_help(d, "amuron");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            case '2':send_desc_help(d, "valeria");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            case '3':send_desc_help(d, "az");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            case '4':send_desc_help(d, "skig");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            case '5':send_desc_help(d, "urg");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            case '6':send_desc_help(d, "bougar");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            case '7':send_desc_help(d, "mugrak");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            case '8':send_desc_help(d, "vetiin");SEND_TO_Q("\r\n Press Enter to continue...",d);SEND_TO_Q(deity_menu, d); SEND_TO_Q("\r\nSelect your deity: ", d);STATE(d)=CON_ASKDEITY;return;
            default:
                return;
            }
        load_result = parse_deity(*arg);
        if (load_result == -500) {
            SEND_TO_Q("\r\nThat deity is not available.", d);
            SEND_TO_Q(deity_menu, d);
            SEND_TO_Q("\r\nSelect your deity: ", d);
            return;
        }
        if (load_result==DEITY_AZ)
        {
        	int r=0;
        	
        	if (IS_THIEF(d->character) || IS_GOBLIN(d->character) || IS_TROLL(d->character))
        	{
        		SEND_TO_Q("Az will not accept you as a follower (race and class condition).\r\n", d);
        		r=1;        	
        	}
        	else if (GET_INTR(d->character)<18 || GET_WISR(d->character)<18)
        	{
        		SEND_TO_Q("Az will not accept you as a follower (INT and WIL condition).\r\n", d);
        		r=1;
        	}
        	
        	if (r)
        	{
        		SEND_TO_Q(deity_menu, d);
            		SEND_TO_Q("\r\nSelect your deity: ", d);
            		return;
            	}
         }
        	
        		
        
            SET_DEITY(d->character,load_result);                
            SET_FAITH(d->character, 0);                
            
            STATE(d)=CON_ASKTOWN;
        	
	       display_hometowns(d);                         
        break;

    
    case CON_DEITY:
    	SEND_TO_Q(deity_menu, d);
        SEND_TO_Q("\r\nSelect your deity: ", d);
        STATE(d) = CON_ASKDEITY;
        break;

    case CON_QATTRIB:
        switch (*arg) {
        case 'y':
        case 'Y':
            break;
        default:
            sprintf(buf, "\r\r\n\nYour Attributes are:\r\n");
            SEND_TO_Q(buf, d);
            roll_real_abils(d->character);
            sprintf(buf, "\r\nDo you want to keep these stats? ");
            SEND_TO_Q(buf, d);
            return;
            break;
        }
        /* SEND_TO_Q("\r\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\n",d); */
        SEND_TO_Q(class_menu1, d);
        SEND_TO_Q("\r\nClass: ", d);
        STATE(d) = CON_QCLASS;
        break;

    case CON_QCLASS:
        load_result = parse_class(*arg, GET_RACE(d->character));
        if (load_result == CLASS_UNDEFINED) {
            SEND_TO_Q("\r\nSorry: that class is not available to you!\r\n", d);
            SEND_TO_Q(class_menu1, d);
            SEND_TO_Q("\r\nClass: ", d);
            return;
        } else
            GET_CLASS(d->character) = load_result;

        GET_STRR(d->character) = race_app[GET_RACE(d->character)].str-10;
        GET_CONR(d->character) = race_app[GET_RACE(d->character)].con-10;
        GET_DEXR(d->character) = race_app[GET_RACE(d->character)].dex-10;
        GET_INTR(d->character) = race_app[GET_RACE(d->character)].intel-10;
        GET_WISR(d->character) = race_app[GET_RACE(d->character)].wis-10;
        switch (GET_RACE(d->character))
        {
        case RACE_HALFLING:
        case RACE_HUMAN:
            GET_CHAR(d->character) = number(17, 19);break;
        case RACE_DROW:
        case RACE_ELF:
            GET_CHAR(d->character) = number(18, 20);break;
        case RACE_GNOME:
            GET_CHAR(d->character) = number(16, 18);break;
        case RACE_DWARF:
            GET_CHAR(d->character) = number(16, 18);break;
        case RACE_ORC:
            GET_CHAR(d->character) = number(12, 14);break;
        case RACE_OGRE:
            GET_CHAR(d->character) = number(13, 15);break;
        case RACE_TROLL:
            GET_CHAR(d->character) = number(11, 13);break;
        case RACE_GOBLIN:
            GET_CHAR(d->character) = number(11, 15);break;
        default:
            GET_CHAR(d->character) = number(16,18);break;
        }
        /*
              SEND_TO_Q("\r\n   Strength measures your pure muscle power. It determines things like how much\r\nweight you can carry, how big weapons you can wield, your bonus to doing damage\r\nand so on.",d);
              SEND_TO_Q("\r\n   Intelligence is just that. It will measure how quick you'll be learning your\r\nskills, your resistance to some magical things. It will also greatly affect the\r\ndestructive power of your spells as well as mana points gain per level. It will also determine the possibility of you making a critical hit.",d);
              SEND_TO_Q("\r\n   Will power ... not finished..",d);
              SEND_TO_Q("\r\n   Dexterity mirrors your reflexes, agility and speed. If you have high\r\ndexterity, you'll be hitting your opponent more often and accurate, at the same\r\ntime avoiding more blows. It's a general combat attribute.",d);
              SEND_TO_Q("\r\n   Constitution is what decides your combat endurance. High constitution\r\ncharacter will have much more hit points to tank a combat. This is the most\r\nimportant stat for a warriors.",d);
        */

        STATE(d) = CON_QSTAT;

             display_stats(d);
        

        break;

    case CON_RMOTD:		/* read CR after printing motd	 */
        add_llog_entry(d->character,LAST_CONNECT);
        SEND_TO_Q(CLRSCR, d);
        SEND_TO_Q(MENU, d);
        STATE(d) = CON_MENU;
        break;

    case CON_QSTAT:

        for (; isspace(*arg); arg++);
        *arg = LOWER(*arg);
        if (*arg != 's' && *arg != 'i' && *arg != 'w' && *arg != 'c' &&
                *arg != 'd' && *arg != 'f' && *arg != '?') {
            SEND_TO_Q("\r\nThat's not a stat. \r\nStat: ", d);
            return;
        }
        switch (*arg) {
        case 's':{
                SEND_TO_Q("How many points to add: ", d);
                STATE(d) = CON_QSTATS;
                break;
            }
        case 'i':{
                SEND_TO_Q("How many points to add: ", d);
                STATE(d) = CON_QSTATI;
                break;
            }
        case 'w':{
                SEND_TO_Q("How many points to add: ", d);
                STATE(d) = CON_QSTATW;
                break;
            }
        case 'd':{
                SEND_TO_Q("How many points to add: ", d);
                STATE(d) = CON_QSTATD;
                break;
            }
        case 'c':{
                SEND_TO_Q("How many points to add: ", d);
                STATE(d) = CON_QSTATC;
                break;
            }
        case '?':{
                SEND_TO_Q("\r\n   &cStrength&0 measures your pure physical power. It determines things like\r\nhow much weight you can carry or push, how heavy weapons you can wield. It\r\nalso defines your bonus to damage, where strength of 16 is considered neutral,\r\nmore strength gives bigger bonus, less gives penalty.", d);
                SEND_TO_Q("\r\n   &cIntelligence&0 determines how fast you'll be learning your skills. It also\r\ndirectly defines how much energy you have and the rate of regeneration.", d);
                SEND_TO_Q("\r\n   &cWill power&0 also your ability to remember the learned, and is connected\r\nin this way to how good you learn your skills. Having bigger will power\r\nwill also help you better resist harmful effects of magic.", d);
                SEND_TO_Q("\r\n   &cDexterity&0 mirrors your reflexes, agility and speed. If you have high\r\ndexterity, you'll be hitting your opponent more often and accurate, at\r\nthe same time avoiding more blows. It's a general combat attribute.", d);
                SEND_TO_Q("\r\n   &cConstitution&0 is what decides your combat endurance. High constitution\r\ncharacter will have much more hit points and better regeneration. At the same\r\ntime, characters with higher constitution will generally receive smaller\r\ndamage and be able to better resist some physical effects like poison.", d);

                SEND_TO_Q("\r\r\n\nStat to modify:", d);
                break;
            }
        case 'f':{
        	SEND_TO_Q(deity_menu, d);
        	SEND_TO_Q("\r\nSelect your deity: ", d);
        	STATE(d) = CON_ASKDEITY;
        	//STATE(d)=CON_DEITY;        	       	                
            }
        }
        break;


    case CON_QSTATS:
        for (; isspace(*arg); arg++);
        if (hmc(atoi(arg), GET_STRR(d->character), race_app[GET_RACE(d->character)].str) > d->stat_points) {
            SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ", d);
            return;
        }
        points = atoi(arg);
        if (GET_STRR(d->character) + points > race_app[GET_RACE(d->character)].str) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        if (GET_STRR(d->character) + points < race_app[GET_RACE(d->character)].str-10) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        points = hmc(atoi(arg), GET_STRR(d->character), race_app[GET_RACE(d->character)].str);

        GET_STRR(d->character) += atoi(arg);
        d->stat_points -= points;

        STATE(d) = CON_QSTAT;
        display_stats(d);
        break;

    case CON_QSTATI:
        for (; isspace(*arg); arg++);
        if (hmc(atoi(arg), GET_INTR(d->character), race_app[GET_RACE(d->character)].intel) > d->stat_points) {
            SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ", d);
            return;
        }
        points = atoi(arg);
        if (GET_INTR(d->character) + points > race_app[GET_RACE(d->character)].intel) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        if ((GET_INTR(d->character) + points) < race_app[GET_RACE(d->character)].intel-10) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        points = hmc(points, GET_INTR(d->character), race_app[GET_RACE(d->character)].intel);
        GET_INTR(d->character) += atoi(arg);
        d->stat_points -= points;
        STATE(d) = CON_QSTAT;
        display_stats(d);
        break;

    case CON_QSTATW:
        for (; isspace(*arg); arg++);
        if (hmc(atoi(arg), GET_WISR(d->character), race_app[GET_RACE(d->character)].wis) > d->stat_points) {
            SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ", d);
            return;
        }
        points = atoi(arg);
        if (GET_WISR(d->character) + points > race_app[GET_RACE(d->character)].wis) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        if (GET_WISR(d->character) + points < race_app[GET_RACE(d->character)].wis-10) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        points = hmc(points, GET_WISR(d->character), race_app[GET_RACE(d->character)].wis);
        GET_WISR(d->character) += atoi(arg);
        d->stat_points -= points;
        STATE(d) = CON_QSTAT;
        display_stats(d);
        break;

    case CON_QSTATD:
        for (; isspace(*arg); arg++);
        if (hmc(atoi(arg), GET_DEXR(d->character), race_app[GET_RACE(d->character)].dex) > d->stat_points) {
            SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ", d);
            return;
        }
        points = atoi(arg);
        if (GET_DEXR(d->character) + points > race_app[GET_RACE(d->character)].dex) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        if (GET_DEXR(d->character) + points < race_app[GET_RACE(d->character)].dex-10) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        points = hmc(points, GET_DEXR(d->character), race_app[GET_RACE(d->character)].dex);
        GET_DEXR(d->character) += atoi(arg);
        d->stat_points -= points;
        STATE(d) = CON_QSTAT;
        display_stats(d);
        break;


    case CON_QSTATC:
        for (; isspace(*arg); arg++);
        if (hmc(atoi(arg), GET_CONR(d->character), race_app[GET_RACE(d->character)].con) > d->stat_points) {
            SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ", d);
            return;
        }
        points = atoi(arg);
        if (GET_CONR(d->character) + points > race_app[GET_RACE(d->character)].con) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        if (GET_CONR(d->character) + points < race_app[GET_RACE(d->character)].con-10) {
            STATE(d) = CON_QSTAT;
            display_stats(d);
            break;
        }
        points = hmc(points, GET_CONR(d->character), race_app[GET_RACE(d->character)].con);
        GET_CONR(d->character) += atoi(arg);
        d->stat_points -= points;
        STATE(d) = CON_QSTAT;
        display_stats(d);
        break;

    case CON_MENU:		/* get selection from main menu	 */
        switch (*arg) {
        case '0':
            SEND_TO_Q("Goodbye.\r\n", d);
            STATE(d) = CON_CLOSE;
            add_llog_entry(d->character,LAST_QUIT);
            break;

        case '1':

            /* this code is to prevent people from multiply logging in */
		log_character(d, 1);
            break;
            
        case '2':
        	display_pc2npc_menu(d);
        	break;
        
        case '3':
            page_string(d, background, 0);
            STATE(d) = CON_RMOTD;
            break;

        case '4':
            SEND_TO_Q("\r\nEnter your old password: ", d);
            echo_off(d);
            STATE(d) = CON_CHPWD_GETOLD;
            break;

        case '5':
            SEND_TO_Q("\r\nEnter your password for verification: ", d);
            echo_off(d);
            STATE(d) = CON_DELCNF1;
            break;
            /*	case '*':
            	    SEND_TO_Q("\r\nEnter your password for verification: ", d);
            	    echo_off(d);
            	    STATE(d) = CON_REINCNF1;
            	    break;*/
    
        case '6':
            SEND_TO_Q("Enter the text you'd like others to see when they look at you.\r\n", d);
            SEND_TO_Q("(/s saves /h for help)\r\n", d);
            if (d->character->player.description) {
                SEND_TO_Q("Current description:\r\n", d);
                SEND_TO_Q(d->character->player.description, d);
                /* don't free this now... so that the old description gets
                   loaded */
                /* as the current buffer in the editor */
                /* DISPOSE(d->character->player.description); */
                /* d->character->player.description = NULL; */
                /* BUT, do setup the ABORT buffer here */
                d->backstr = str_dup(d->character->player.description);

            }
            d->str = &d->character->player.description;
            d->max_str = EXDSCR_LENGTH;
            STATE(d) = CON_EXDESC;
            break;
          case '7':
          	if (GET_LEVEL(d->character)<LVL_IMMORT-1)
          	{                                       
          		SEND_TO_Q("\r\nYou are not ready to remort yet!\r\n", d);
          		SEND_TO_Q(MENU, d);
          	}
          	else if (GET_LEVEL(d->character)>=LVL_IMMORT)
          	{                                                            
          		SEND_TO_Q("\r\nYou can not remort an immortal character!\r\n", d);
          		SEND_TO_Q(MENU, d);
          	}       
          	else
          	{                                                        
          		SEND_TO_Q("\r\nRemort system is currently in test!\r\n", d);
          		SEND_TO_Q(MENU, d);
          	}
          	break;
          		
          		
            	    
        default:
            SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
            SEND_TO_Q(MENU, d);
            break;
        }
        break;
    case CON_PC2NPC:
    	check_pc2npc(d, arg);
    	break;
    	
    case CON_CHPWD_GETOLD:
        if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
            echo_on(d);
            SEND_TO_Q("\r\nIncorrect password.\r\n", d);
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
            return;
        } else {
            SEND_TO_Q("\r\nEnter a new password: ", d);
            STATE(d) = CON_CHPWD_GETNEW;
            return;
        }
        break;
    case CON_REINCNF1:
        echo_on(d);
        if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
            SEND_TO_Q("\r\nIncorrect password.\r\n", d);
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
        } else {
            if (GET_LEVEL(d->character) < 29) {
                SEND_TO_Q("\r\nYOU ARE ABOUT TO REINCARNATE THIS CHARACTER.\r\n"
                          "SINCE YOUR LEVEL IS LESS THEN 29, YOU WILL NOT ADVANCE ON THE GOLDEN PATH.\r\n"
                          "ARE YOU ABSOLUTELY SURE YOU WANT TO GO BACK TO THE BEGINNING OF THIS CIRCLE?\r\r\n\n"
                          "Please type \"yes\" to confirm: ", d);
            } else {
                SEND_TO_Q("\r\nYOU ARE ABOUT TO BE REINCARNATED.  YOU WILL BEGIN ONCE MORE AT\r\n"
                          "LEVEL ONE.  HOWEVER, YOU WILL ADVANCE ONE STEP UP THE GOLDEN PATH,\r\n"
                          "ONWARD TOWARD IMMORTALITY.\r\n"
                          "ARE YOU ABSOLUTELY PREPARED TO STEP INTO THE NEXT CIRCLE?\r\n"
                          "Please type \"yes\" to confirm: ", d);
            }
            STATE(d) = CON_REINCNF2;
        }
        break;

    case CON_DELCNF1:
        echo_on(d);
        if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH) && strcmp(arg, "quick")) {
            SEND_TO_Q("\r\nIncorrect password.\r\n", d);
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
        } else {
            SEND_TO_Q("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER *** P E R M A N E N T L Y ***\r\n"
                      "ARE YOU *** ABSOLUTELY *** SURE?\r\r\n\n"
                      "Please type \"yes\" to confirm: ", d);
            STATE(d) = CON_DELCNF2;
        }
        break;

    case CON_REINCNF2:
        if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
            if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
                SEND_TO_Q("You try to force-reincarnate yourself, but the ice stops you.\r\n", d);
                SEND_TO_Q("Character not deleted.\r\r\n\n", d);
                STATE(d) = CON_CLOSE;
                return;
            }
            if (GET_LEVEL(d->character) >= LVL_IMMORT) {
                sprintf(buf, "\r\nSorry, %s, immortals and up can not restart their characters.", GET_NAME(d->character));
                SEND_TO_Q(buf, d);
                return;
            }
            if (GET_LEVEL(d->character) >= 29 && GET_REINCARN(d->character) < 4) {
                ADD_REINCARN(d->character);
                GET_LEVEL(d->character) = 0;
                /* Some dramatic rumbling is called for */
            }
            SET_BIT(PLR_FLAGS(d->character), PLR_JUSTREIN);
            sprintf(buf, "\r\n%s, you are being rerolled from scratch.  Good Luck.\r\nWhat is your gender? ", GET_NAME(d->character));
            SEND_TO_Q(buf, d);
            STATE(d) = CON_QSEX;
            return;
        } else {
            SEND_TO_Q("\r\nCharacter not reincarnated.\r\n", d);
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
        }
        break;

    case CON_DELCNF2:
        if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
            if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
                SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
                SEND_TO_Q("Character not deleted.\r\r\n\n", d);
                STATE(d) = CON_CLOSE;
                return;
            }

            if (PLR_FLAGGED(d->character, PLR_TOAD)) {
                SEND_TO_Q("Don't be such a baby. Ribbit. Ribbit.\r\n", d);
                SEND_TO_Q("Character not deleted.\r\n\r\n", d);
                STATE(d) = CON_CLOSE;
                return;
            }



            SEND_TO_Q("\r\n\r\nStop here! Think it over! What are you doing???\r\nTake a deep breath and type qpz8ym=t2*73vr to proceed.\r\n\r\nYour choice (don't do it man): ", d);
            STATE(d)=CON_QDD;
            return;
        } else {
            SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
        }
        break;

    case CON_QDD:
        if (!strcmp(arg, "qpz8ym=t2*73vr")) {
            if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
                SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
                SEND_TO_Q("Character not deleted.\r\r\n\n", d);
                STATE(d) = CON_CLOSE;
                return;
            }
            //	    if (GET_LEVEL(d->character) < LVL_GRGOD)
            if (strcmp(GET_NAME(d->character), "Bog"))
                SET_BIT(PLR_FLAGS(d->character), PLR_DELETED);
            save_char(d->character, NOWHERE);
            Crash_delete_file(GET_NAME(d->character));
            sprintf(buf, "Character '%s' deleted!\r\n"
                    "Goodbye.\r\n", GET_NAME(d->character));
            SEND_TO_Q(buf, d);
            sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character),
                    GET_LEVEL(d->character));
            mudlog(buf, NRM, LVL_GOD, TRUE);
            STATE(d) = CON_CLOSE;
            return;
        } else {
            SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
        }
        break;

    case CON_CLOSE:
        close_socket(d);
        break;

    default:
        log("SYSERR: Nanny: illegal state of con'ness; closing connection");
        close_socket(d);
        break;
    }
}
