/**************************************************************************
 * File: clan.c                       Intended to be used with CircleMUD  *
 * Usage: This is the code for clans                                      *
 * By Mehdi Keddache (Heritsun on Eclipse of Fate eclipse.argy.com 7777)  *
 * CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 * CircleMUD (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "constants.h"
#include "clan.h"
#include "vt100c.h"

extern struct zone_data *zone_table;   /* zone table			 */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
int             num_of_clans;
struct clan_rec clan[MAX_CLANS];

extern          void save_char_file_u(struct char_file_u st);
extern struct descriptor_data *descriptor_list;

char            clan_privileges[NUM_CP + 1][20] = {
            "setplan", "enroll", "expel", "promote", "demote", "setfees", "withdraw", "setapplev"};

void            send_clan_format(struct char_data * ch)
{
    int             c,
    r;

    send_to_char("Clan commands available to you:\r\n"
                 "   clan who\r\n"
                 "   clan status\r\n"
                 "   clan info <clan>\r\n", ch);
    if (GET_LEVEL(ch) >= LVL_CLAN_GOD)
        send_to_char("   clan create     <leader> <clan name>\r\n"
                     "   clan destroy    <clan>\r\n"
                     "   clan enroll     <player> <clan>\r\n"
                     "   clan expel      <player> <clan>\r\n"
                     "   clan promote    <player> <clan>\r\n"
                     "   clan demote     <player> <clan>\r\n"
                     "   clan withdraw   <amount> <clan>\r\n"
                     "   clan deposit    <amount> <clan>\r\n"
                     "   clan set ranks  <rank>   <clan>\r\n"
                     "   clan set appfee <amount> <clan>\r\n"
                     "   clan set dues   <amount> <clan>\r\n"
                     "   clan set applev <level>  <clan>\r\n"
                     "   clan set plan   <clan>\r\n"
                     "   clan privilege  <privilege>   <rank> <clan>\r\n"
                     "   clan set title  <clan number> <rank> <title>\r\n", ch);
    else {
        c = (GET_CLAN(ch));
        r = GET_LEVEL(ch);
        if (c >= 0) {
            send_to_char("   clan deposit    <amount>\r\n", ch);
            if (r >= clan[c].privilege[CP_WITHDRAW])
                send_to_char("   clan withdraw   <amount>\r\n", ch);
            /*    if(r>=clan[c].privilege[CP_ENROLL])
            //      send_to_char("   clan enroll     <player>\r\n" ,ch);
            //    if(r>=clan[c].privilege[CP_EXPEL])
            //      send_to_char("   clan expel      <player>\r\n" ,ch);
            //    if(r>=clan[c].privilege[CP_PROMOTE])
            //      send_to_char("   clan promote    <player>\r\n",ch);
            //    if(r>=clan[c].privilege[CP_DEMOTE])
            //      send_to_char("   clan demote     <player>\r\n",ch);
            //    if(r>=clan[c].privilege[CP_SET_APPLEV])
            //      send_to_char("   clan set applev <level>\r\n",ch);
            //    if(r>=clan[c].privilege[CP_SET_FEES])
            //      send_to_char("   clan set appfee <amount>\r\n"
            //                   "   clan set dues   <amount>\r\n",ch);
            //    if(r>=clan[c].privilege[CP_SET_PLAN])
            //      send_to_char("   clan set plan\r\n",ch);
            //    if(r==clan[c].ranks)
            //      send_to_char("   clan set ranks  <rank>\r\n"
            //                   "   clan set title  <rank> <title>\r\n"
            //                  "   clan privilege  <privilege> <rank>\r\n",ch);*/
        }
    }
}

void            do_clan_create(struct char_data * ch, char *arg)
{
    struct char_data *leader = NULL;
    char            arg1[MAX_INPUT_LENGTH],
    arg2[MAX_INPUT_LENGTH];
    int             new_id = 0,
                             i;

    if (!*arg) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
        send_to_char("You are not mighty enough to create new clans!\r\n", ch);
        return;
    }
    if (num_of_clans == MAX_CLANS) {
        send_to_char("Max clans reached. WOW!\r\n", ch);
        return;
    }
    /*//one_argument(arg, arg2);*/

    /*
    if(!(leader=get_char_vis(ch,arg1))) {
      send_to_char("The leader of the new clan must be present.\r\n",ch);
      return; }
    */

    if (strlen(arg) >= 32) {
        send_to_char("Clan name too long! (32 characters max)\r\n", ch);
        return;
    }

    /* if(GET_CLAN(leader)!=0 && GET_CLAN_RANK(leader)!=0) {
     * send_to_char("The leader already belongs to a clan!\r\n",ch); return;
     * } */

    if (find_clan(arg) != -1) {
        send_to_char("That clan name alread exists!\r\n", ch);
        return;
    }
    strncpy(clan[num_of_clans].name, ((char *) arg), 32);
    for (i = 0; i < num_of_clans; i++)
        if (new_id < clan[i].id)
            new_id = clan[i].id;
    clan[num_of_clans].id = new_id + 1;
    clan[num_of_clans].ranks = 2;
    strcpy(clan[num_of_clans].rank_name[0], "Member");
    strcpy(clan[num_of_clans].rank_name[1], "Leader");
    clan[num_of_clans].treasure = 0;
    clan[num_of_clans].healer_gold = 0;
    clan[num_of_clans].soldiers = 0;
    clan[num_of_clans].members = 0;
    clan[num_of_clans].power = 0;
    clan[num_of_clans].score = 0;
    clan[num_of_clans].kills = 0;
    clan[num_of_clans].app_level = DEFAULT_APP_LVL;
    for (i = 0; i < 5; i++)
        clan[num_of_clans].spells[i] = 0;
    for (i = 0; i < 20; i++)
        clan[num_of_clans].privilege[i] = 7;
    clan[num_of_clans].privilege[CP_WITHDRAW] = 51;

    for (i = 0; i < 4; i++)
        clan[num_of_clans].at_war[i] = 0;
    num_of_clans++;
    save_clans();
    send_to_char("Clan created.\r\n", ch);
    /*
    GET_CLAN(leader)=clan[num_of_clans-1].id;
    GET_CLAN_RANK(leader)=clan[num_of_clans-1].ranks;
    save_char(leader, NOWHERE);
    */
    return;
}


void            do_clan_destroy(struct char_data * ch, char *arg)
{

    int             i,
    j;
    extern int      top_of_p_table;
    extern struct player_index_element *player_table;
    struct char_file_u chdata;
    struct char_data *victim = NULL;

    if (!*arg) {
        send_clan_format(ch);
        return;
    }
    if ((i = find_clan(arg)) < 0) {
        send_to_char("Unknown clan.\r\n", ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
        send_to_char("Your not mighty enough to destroy clans!\r\n", ch);
        return;
    }
    for (j = 0; j <= top_of_p_table; j++) {
        if ((victim = is_playing((player_table + j)->name))) {
            if (GET_CLAN(victim) == clan[i].id) {
                GET_CLAN(victim) = 0;
                GET_CLAN_RANK(victim) = 0;
                save_char(victim, victim->in_room);
            }
        } else {
            load_char((player_table + j)->name, &chdata);
            if (chdata.player_specials_saved.clan == clan[i].id) {
                chdata.player_specials_saved.clan = 0;
                chdata.player_specials_saved.clan_rank = 0;
                save_char_file_u(chdata);
            }
        }
    }

    memset(&clan[i], 0, sizeof(struct clan_rec));

    for (j = i; j < num_of_clans - 1; j++)
        clan[j] = clan[j + 1];

    num_of_clans--;

    send_to_char("Clan deleted.\r\n", ch);
    save_clans();
    return;
}

void            do_clan_enroll(struct char_data * ch, int clan_num)
{
    struct char_data *vict = ch;
    int             immcom = 0;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    /*if (!(*arg)) {
      send_clan_format(ch);
      return; }

    if(GET_LEVEL(ch)<LVL_IMMORT) {
      if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
        send_to_char("You don't belong to any clan!\r\n",ch);
        return;
      }
}
    else {
      if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
        send_to_char("You do not have clan privileges.\r\n", ch);
        return; }
      immcom=1;
      half_chop(arg,arg1,arg2);
      strcpy(arg,arg1);
      if ((clan_num = find_clan(arg2)) < 0) {
        send_to_char("Unknown clan.\r\n", ch);
        return;
      }
}

    if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_ENROLL] && !immcom) {
      send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
      return;
      }

    if(!(vict=get_char_room_vis(ch,arg))) {
      send_to_char("Er, Who ??\r\n",ch);
      return;
}
    else {
      if(GET_CLAN(vict)!=clan[clan_num].id) {
        if(GET_CLAN_RANK(vict)>0) {
          send_to_char("They're already in a clan.\r\n",ch);
          return;
        }
        else {
          send_to_char("They didn't request to join your clan.\r\n",ch);
          return;
        }
      }
      else
        if(GET_CLAN_RANK(vict)>0) {
          send_to_char("They're already in your clan.\r\n",ch);
          return;
        }
      if(GET_LEVEL(vict)>=LVL_IMMORT) {
        send_to_char("You cannot enroll immortals in clans.\r\n",ch);
        return; }
}
    */
    GET_CLAN(vict) = clan_num;
    GET_CLAN_RANK(vict)++;
    save_char(vict, vict->in_room);
    clan_num = (GET_CLAN(vict));
    clan[clan_num].power += GET_LEVEL(vict);
    clan[clan_num].members++;
    /*
    send_to_char("You've been enrolled in the clan!\r\n",vict);
    send_to_char("Done.\r\n",ch);
    */
    save_clans();
    return;
}

void            do_clan_expel(struct char_data * ch, char *arg)
{
    struct char_data *vict = NULL;
    int             clan_num,
    immcom = 0;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg2)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_CLAN_RANK(ch) < clan[clan_num].privilege[CP_EXPEL] && !immcom) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("Er, Who ??\r\n", ch);
        return;
    } else {
        if (GET_CLAN(vict) != clan[clan_num].id) {
            send_to_char("They're not in your clan.\r\n", ch);
            return;
        } else {
            if (GET_CLAN_RANK(vict) >= GET_CLAN_RANK(ch) && !immcom) {
                send_to_char("You cannot kick out that person.\r\n", ch);
                return;
            }
        }
    }

    GET_CLAN(vict) = 0;
    GET_CLAN_RANK(vict) = 0;
    save_char(vict, vict->in_room);
    clan[clan_num].members--;
    clan[clan_num].power -= GET_LEVEL(vict);
    send_to_char("You've been kicked out of your clan!\r\n", vict);
    send_to_char("Done.\r\n", ch);

    return;
}

void            do_clan_demote(struct char_data * ch, char *arg)
{
    struct char_data *vict = NULL;
    int             clan_num,
    immcom = 0;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg2)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_CLAN_RANK(ch) < clan[clan_num].privilege[CP_DEMOTE] && !immcom) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("Er, Who ??\r\n", ch);
        return;
    } else {
        if (GET_CLAN(vict) != clan[clan_num].id) {
            send_to_char("They're not in your clan.\r\n", ch);
            return;
        } else {
            if (GET_CLAN_RANK(vict) == 1) {
                send_to_char("They can't be demoted any further, use expel now.\r\n", ch);
                return;
            }
            if (GET_CLAN_RANK(vict) >= GET_CLAN_RANK(ch) && !immcom) {
                send_to_char("You cannot demote a person of this rank!\r\n", ch);
                return;
            }
        }
    }

    GET_CLAN_RANK(vict)--;
    save_char(vict, vict->in_room);
    send_to_char("You've demoted within your clan!\r\n", vict);
    send_to_char("Done.\r\n", ch);
    return;
}

void            do_clan_promote(struct char_data * ch, char *arg)
{
    struct char_data *vict = NULL;
    int             clan_num,
    immcom = 0;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg2)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_CLAN_RANK(ch) < clan[clan_num].privilege[CP_PROMOTE] && !immcom) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("Er, Who ??\r\n", ch);
        return;
    } else {
        if (GET_CLAN(vict) != clan[clan_num].id) {
            send_to_char("They're not in your clan.\r\n", ch);
            return;
        } else {
            if (GET_CLAN_RANK(vict) == 0) {
                send_to_char("They're not enrolled yet.\r\n", ch);
                return;
            }
            if ((GET_CLAN_RANK(vict) + 1) > GET_CLAN_RANK(ch) && !immcom) {
                send_to_char("You cannot promote that person over your rank!\r\n", ch);
                return;
            }
            if (GET_CLAN_RANK(vict) == clan[clan_num].ranks) {
                send_to_char("You cannot promote someone over the top rank!\r\n", ch);
                return;
            }
        }
    }

    GET_CLAN_RANK(vict)++;
    save_char(vict, vict->in_room);
    send_to_char("You've been promoted within your clan!\r\n", vict);
    send_to_char("Done.\r\n", ch);
    return;
}

ACMD (do_clanlist)
{
    struct descriptor_data *d;
    struct char_data *tch;
    char            line_disp[200];

    if (GET_CLAN(ch) == CLAN_OUTLAW) {
        send_to_char("You can not use this option as an Outlaw.\r\n", ch);
        return;
    }
    send_to_char("\r\nList of your clan members\r\n", ch);
    send_to_char("=========================\r\n", ch);
    for (d = descriptor_list; d; d = d->next) {
        if (d->connected)
            continue;
        if ((tch = d->character))
            if (GET_CLAN(tch) == GET_CLAN(ch)) {
                /*                if (GET_CLAN(tch)==CLAN_OUTLAW && PRF2_FLAGGED(tch, PRF2_NOWHO))
                                    sprintf(line_disp, "[--- -- ---] %-15s - %s\r\n",GET_NAME(tch), world[tch->in_room].name);
                                else*/
                sprintf(line_disp, "[%s %2d %s] %-15s - %s\r\n", RACE_ABBR(tch), GET_LEVEL(tch), CLASS_ABBR(tch),GET_NAME(tch), zone_table[world[tch->in_room].zone].name);
                send_to_char(line_disp, ch);
            }
    }
    return;
}

void            do_clan_status(struct char_data * ch)
{
    char            line_disp[90];
    int             clan_num;

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
        send_to_char("You are immortal and cannot join any clan!\r\n", ch);
        return;
    }
    clan_num = (GET_CLAN(ch));

    if (GET_CLAN_RANK(ch) == 0) {
        if (clan_num >= 0) {
            sprintf(line_disp, "You applied to %s.\r\n", clan[clan_num].name);
            send_to_char(line_disp, ch);
            return;
        } else {
            send_to_char("You do not belong to a clan!\r\n", ch);
            return;
        }
    }

    sprintf(line_disp, "You are %s (Rank %d) of %s (ID %d)\r\n",
            clan[clan_num].rank_name[GET_CLAN_RANK(ch) - 1], GET_CLAN_RANK(ch),
            clan[clan_num].name, clan[clan_num].id);
    send_to_char(line_disp, ch);

    return;
}

void            do_clan_apply(struct char_data * ch, char *arg)
{
    int             clan_num;

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) >= LVL_IMMORT) {
        send_to_char("Gods cannot apply for any clan.\r\n", ch);
        return;
    }
    if (GET_CLAN_RANK(ch) > 0) {
        send_to_char("You already belong to a clan!\r\n", ch);
        return;
    } else {
        if ((clan_num = find_clan(arg)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_LEVEL(ch) < clan[clan_num].app_level) {
        send_to_char("You are not mighty enough to apply to this clan.\r\n", ch);
        return;
    }
    /*    if (GET_GOLD(ch) < clan[clan_num].app_fee) {
            send_to_char("You cannot afford the application fee!\r\n", ch);
            return;
        }
        GET_GOLD(ch) -= clan[clan_num].app_fee;
        clan[clan_num].treasure += clan[clan_num].app_fee;*/
    save_clans();
    GET_CLAN(ch) = clan[clan_num].id;
    save_char(ch, ch->in_room);
    send_to_char("You've applied to the clan!\r\n", ch);

    return;
}

ACMD(do_claninfo)
{
    int             i = 0,
                        j;

    if (num_of_clans == 0) {
        send_to_char("No clans have formed yet.\r\n", ch);
        return;
    }
    sprintf(buf, "\r");
    for (i = 0; i < num_of_clans; i++)
        sprintf(buf, "%s%-20s Members: %3d  Power: %5d \r\n", buf,
                clan[i].name, clan[i].members, clan[i].power);
    page_string(ch->desc, buf, 1);
    return;
}



void            do_clan_info(struct char_data * ch, char *arg)
{
    int             i = 0,
                        j;

    if (num_of_clans == 0) {
        send_to_char("No clans have formed yet.\r\n", ch);
        return;
    }
    if (!(*arg)) {
        sprintf(buf, "\r");
        for (i = 0; i < num_of_clans; i++)
            sprintf(buf, "%s[%-3d]  %-20s Members: %3d  Power: %5d  Kills: %d\r\n", buf, i,
                    clan[i].name, clan[i].members, clan[i].power, clan[i].kills);
        page_string(ch->desc, buf, 1);
        return;
    } else if ((i = find_clan(arg)) < 0) {
        send_to_char("Unknown clan.\r\n", ch);
        return;
    }
    sprintf(buf, "Info for clan <<%s>>:\r\n", clan[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "Ranks      : %d\r\nTitles     : ", clan[i].ranks);
    for (j = 0; j < clan[i].ranks; j++)
        sprintf(buf, "%s%s ", buf, clan[i].rank_name[j]);
    sprintf(buf, "%s\r\nMembers    : %d\r\nPower      : %d\t\nTreasure   : %ld\r\nSpells     : ", buf, clan[i].members, clan[i].power, clan[i].treasure);
    for (j = 0; j < 5; j++)
        if (clan[i].spells[j])
            sprintf(buf, "%s%d ", buf, clan[i].spells[j]);
    sprintf(buf, "%s\r\n", buf);
    send_to_char(buf, ch);
    sprintf(buf, "Clan privileges:\r\n");
    for (j = 0; j < NUM_CP; j++)
        sprintf(buf, "%s   %-10s: %d\r\n", buf, clan_privileges[j], clan[i].privilege[j]);
    sprintf(buf, "%s\r\n", buf);
    send_to_char(buf, ch);
    /*
    sprintf(buf, "Description:\r\n%s\r\n\n", clan[i].description);
    send_to_char(buf, ch);
    */
    if ((clan[i].at_war[0] == 0) && (clan[i].at_war[1] == 0) && (clan[i].at_war[2] == 0) && (clan[i].at_war[3] == 0))
        send_to_char("This clan is at peace with all others.\r\n", ch);
    else
        send_to_char("This clan is at war.\r\n", ch);
    sprintf(buf, "Kills  : %d gold\r\nScore     : %d gold\r\n", clan[i].kills, clan[i].score);
    send_to_char(buf, ch);
    sprintf(buf, "Application level: %d\r\n", clan[i].app_level);
    send_to_char(buf, ch);

    return;
}

sh_int          find_clan_by_id(int idnum)
{
    int             i;
    for (i = 0; i < num_of_clans; i++)
        if (idnum == clan[i].id)
            return i;
    return -1;
}

sh_int          find_clan(char *name)
{
    int             i;
    for (i = 0; i < num_of_clans; i++)
        if (is_abbrev((name), (clan[i].name)))
            return i;
    return -1;
}

void            save_clans()
{
    FILE           *fl;

    if (!(fl = fopen(CLAN_FILE, "wb"))) {
        log("SYSERR: Unable to open clan file");
        return;
    }
    fwrite(&num_of_clans, sizeof(int), 1, fl);
    fwrite(clan, sizeof(struct clan_rec), num_of_clans, fl);
    fflush(fl);
    fclose(fl);
    return;
}


void            init_clans()
{
    FILE           *fl;
    int             i,
    j;
    extern int      top_of_p_table;
    extern struct player_index_element *player_table;
    struct char_file_u chdata;

    memset(clan, 0, sizeof(struct clan_rec) * MAX_CLANS);
    num_of_clans = 0;
    i = 0;

    if (!(fl = fopen(CLAN_FILE, "rb"))) {
        log("   Clan file does not exist. Will create a new one");
        save_clans();
        return;
    }
    fread(&num_of_clans, sizeof(int), 1, fl);
    fread(clan, sizeof(struct clan_rec), num_of_clans, fl);
    fclose(fl);

    log("   Calculating powers and members");
    for (i = 0; i < num_of_clans; i++) {
        clan[i].power = 0;
        clan[i].members = 0;
    }

    for (j = 0; j <= top_of_p_table; j++) {

        load_char((player_table + j)->name, &chdata);

        if ((i = (chdata.player_specials_saved.clan)) >= 0) {
            clan[i].power += chdata.level;
            clan[i].members++;
        }
    }

    return;
}


void            do_clan_bank(struct char_data * ch, char *arg, int action)
{
    int             clan_num,
    immcom = 0;
    long            amount = 0;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg2)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_LEVEL(ch) < clan[clan_num].privilege[CP_WITHDRAW] && !immcom && action == CB_WITHDRAW) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(*arg)) {
        send_to_char("Deposit how much?\r\n", ch);
        return;
    }
    if (!is_number(arg)) {
        send_to_char("Deposit what?\r\n", ch);
        return;
    }
    amount = atoi(arg);

    if (!immcom && action == CB_DEPOSIT && GET_GOLD(ch) < amount) {
        send_to_char("You do not have that kind of money!\r\n", ch);
        return;
    }
    if (action == CB_WITHDRAW && clan[clan_num].treasure < amount) {
        send_to_char("The clan is not wealthy enough for your needs!\r\n", ch);
        return;
    }
    switch (action) {
    case CB_WITHDRAW:
        GET_GOLD(ch) += amount;
        clan[clan_num].treasure -= amount;
        send_to_char("You withdraw from the clan's treasure.\r\n", ch);
        break;
    case CB_DEPOSIT:
        if (!immcom)
            GET_GOLD(ch) -= amount;
        clan[clan_num].treasure += amount;
        send_to_char("You add to the clan's treasure.\r\n", ch);
        break;
    default:
        send_to_char("Problem in command, please report.\r\n", ch);
        break;
    }
    save_char(ch, ch->in_room);
    save_clans();
    return;
}

void            do_clan_money(struct char_data * ch, char *arg, int action)
{
    int             clan_num,
    immcom = 0;
    long            amount = 0;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg2)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_CLAN_RANK(ch) < clan[clan_num].privilege[CP_SET_FEES] && !immcom) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(*arg)) {
        send_to_char("Set it to how much?\r\n", ch);
        return;
    }
    if (!is_number(arg)) {
        send_to_char("Set it to what?\r\n", ch);
        return;
    }
    amount = atoi(arg);

    switch (action) {
        /*    case CM_APPFEE:
                clan[clan_num].app_fee = amount;
                send_to_char("You change the application fee.\r\n", ch);
                break;
            case CM_DUES:
                clan[clan_num].dues = amount;
                send_to_char("You change the monthly dues.\r\n", ch);
                break;*/
    default:
        send_to_char("Problem in command, please report.\r\n", ch);
        break;
    }

    save_clans();
    return;
}

void            do_clan_ranks(struct char_data * ch, char *arg)
{
    int             i,
    j;
    int             clan_num,
    immcom = 0;
    int             new_ranks;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];
    extern int      top_of_p_table;
    extern struct player_index_element *player_table;
    struct char_file_u chdata;
    struct char_data *victim = NULL;

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg2)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_CLAN_RANK(ch) != clan[clan_num].ranks && !immcom) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(*arg)) {
        send_to_char("Set how many ranks?\r\n", ch);
        return;
    }
    if (!is_number(arg)) {
        send_to_char("Set the ranks to what?\r\n", ch);
        return;
    }
    new_ranks = atoi(arg);

    if (new_ranks == clan[clan_num].ranks) {
        send_to_char("The clan already has this number of ranks.\r\n", ch);
        return;
    }
    if (new_ranks < 2 || new_ranks > 20) {
        send_to_char("Clans must have from 2 to 20 ranks.\r\n", ch);
        return;
    }
    if (GET_GOLD(ch) < 750000 && !immcom) {
        send_to_char("Changing the clan hierarchy requires 750,000 coins!\r\n", ch);
        return;
    }
    if (!immcom)
        GET_GOLD(ch) -= 750000;

    for (j = 0; j <= top_of_p_table; j++) {
        if ((victim = is_playing((player_table + j)->name))) {
            if (GET_CLAN(victim) == clan[clan_num].id) {
                if (GET_CLAN_RANK(victim) < clan[clan_num].ranks && GET_CLAN_RANK(victim) > 0)
                    GET_CLAN_RANK(victim) = 1;
                if (GET_CLAN_RANK(victim) == clan[clan_num].ranks)
                    GET_CLAN_RANK(victim) = new_ranks;
                save_char(victim, victim->in_room);
            }
        } else {
            load_char((player_table + j)->name, &chdata);
            if (chdata.player_specials_saved.clan == clan[clan_num].id) {
                if (chdata.player_specials_saved.clan_rank < clan[clan_num].ranks && chdata.player_specials_saved.clan_rank > 0)
                    chdata.player_specials_saved.clan_rank = 1;
                if (chdata.player_specials_saved.clan_rank == clan[clan_num].ranks)
                    chdata.player_specials_saved.clan_rank = new_ranks;
                save_char_file_u(chdata);
            }
        }
    }

    clan[clan_num].ranks = new_ranks;
    for (i = 0; i < clan[clan_num].ranks - 1; i++)
        strcpy(clan[clan_num].rank_name[i], "Member");
    strcpy(clan[clan_num].rank_name[clan[clan_num].ranks - 1], "Leader");
    for (i = 0; i < NUM_CP; i++)
        clan[clan_num].privilege[i] = new_ranks;

    save_clans();
    return;
}

void            do_clan_titles(struct char_data * ch, char *arg)
{
    char            arg1[MAX_INPUT_LENGTH],
    arg2[MAX_INPUT_LENGTH];
    int             clan_num = 0,
                               rank;

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
        if (GET_CLAN_RANK(ch) != clan[clan_num].ranks) {
            send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg2);
        if (!is_number(arg1)) {
            send_to_char("You need to specify a clan number.\r\n", ch);
            return;
        }
        if ((clan_num = atoi(arg1)) < 0 || clan_num >= num_of_clans) {
            send_to_char("There is no clan with that number.\r\n", ch);
            return;
        }
    }

    half_chop(arg, arg1, arg2);

    if (!is_number(arg1)) {
        send_to_char("You need to specify a rank number.\r\n", ch);
        return;
    }
    rank = atoi(arg1);

    if (rank < 1 || rank > clan[clan_num].ranks) {
        send_to_char("This clan has no such rank number.\r\n", ch);
        return;
    }
    if (strlen(arg2) < 1 || strlen(arg2) > 19) {
        send_to_char("You need a clan title of under 20 characters.\r\n", ch);
        return;
    }
    strcpy(clan[clan_num].rank_name[rank - 1], arg2);
    save_clans();
    send_to_char("Done.\r\n", ch);
    return;
}

void            do_clan_application(struct char_data * ch, char *arg)
{
    int             clan_num,
    immcom = 0;
    int             applevel;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg2)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_CLAN_RANK(ch) < clan[clan_num].privilege[CP_SET_APPLEV] && !immcom) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(*arg)) {
        send_to_char("Set to which level?\r\n", ch);
        return;
    }
    if (!is_number(arg)) {
        send_to_char("Set the application level to what?\r\n", ch);
        return;
    }
    applevel = atoi(arg);

    if (applevel < 1 || applevel > 999) {
        send_to_char("The application level can go from 1 to 999.\r\n", ch);
        return;
    }
    clan[clan_num].app_level = applevel;
    save_clans();

    return;
}

void            do_clan_sp(struct char_data * ch, char *arg, int priv)
{
    int             clan_num,
    immcom = 0;
    int             rank;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];

    if (!(*arg)) {
        send_clan_format(ch);
        return;
    }
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        immcom = 1;
        half_chop(arg, arg1, arg2);
        strcpy(arg, arg1);
        if ((clan_num = find_clan(arg1)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (GET_CLAN_RANK(ch) != clan[clan_num].ranks && !immcom) {
        send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
        return;
    }
    if (!(*arg)) {
        send_to_char("Set the privilege to which rank?\r\n", ch);
        return;
    }
    if (!is_number(arg)) {
        send_to_char("Set the privilege to what?\r\n", ch);
        return;
    }
    rank = atoi(arg);

    if (rank < 1 || rank > clan[clan_num].ranks) {
        send_to_char("There is no such rank in the clan.\r\n", ch);
        return;
    }
    clan[clan_num].privilege[priv] = rank;
    save_clans();

    return;
}

void            do_clan_plan(struct char_data * ch, char *arg)
{
    int             clan_num;

    send_to_char("Command not ready yet\r\n", ch);
    return;

    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if ((clan_num = (GET_CLAN(ch))) < 0) {
            send_to_char("You don't belong to any clan!\r\n", ch);
            return;
        }
        if (GET_CLAN_RANK(ch) < clan[clan_num].privilege[CP_SET_PLAN]) {
            send_to_char("You're not influent enough in the clan to do that!\r\n", ch);
            return;
        }
    } else {
        if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
            send_to_char("You do not have clan privileges.\r\n", ch);
            return;
        }
        if (!(*arg)) {
            send_clan_format(ch);
            return;
        }
        if ((clan_num = find_clan(arg)) < 0) {
            send_to_char("Unknown clan.\r\n", ch);
            return;
        }
    }

    if (strlen(clan[clan_num].description) == 0) {
        sprintf(buf, "Enter the description, or plan for clan <<%s>>.\r\n", clan[clan_num].name);
        send_to_char(buf, ch);
    } else {
        sprintf(buf, "Old plan for clan <<%s>>:\r\n", clan[clan_num].name);
        send_to_char(buf, ch);
        send_to_char(clan[clan_num].description, ch);
        send_to_char("Enter new plan:\r\n", ch);
    }
    send_to_char("End with @ on a line by itself.\r\n", ch);
    ch->desc->str = (char **) &(clan[clan_num].description);
    ch->desc->max_str = CLAN_PLAN_LENGTH;
    save_clans();
    return;
}

void            do_clan_privilege(struct char_data * ch, char *arg)
{
    char            arg1[MAX_INPUT_LENGTH],
    arg2[MAX_INPUT_LENGTH];
    int             i;

    half_chop(arg, arg1, arg2);

    if (is_abbrev(arg1, "setplan")) {
        do_clan_sp(ch, arg2, CP_SET_PLAN);
        return;
    }
    if (is_abbrev(arg1, "enroll")) {
        do_clan_sp(ch, arg2, CP_ENROLL);
        return;
    }
    if (is_abbrev(arg1, "expel")) {
        do_clan_sp(ch, arg2, CP_EXPEL);
        return;
    }
    if (is_abbrev(arg1, "promote")) {
        do_clan_sp(ch, arg2, CP_PROMOTE);
        return;
    }
    if (is_abbrev(arg1, "demote")) {
        do_clan_sp(ch, arg2, CP_DEMOTE);
        return;
    }
    if (is_abbrev(arg1, "withdraw")) {
        do_clan_sp(ch, arg2, CP_WITHDRAW);
        return;
    }
    if (is_abbrev(arg1, "setfees")) {
        do_clan_sp(ch, arg2, CP_SET_FEES);
        return;
    }
    if (is_abbrev(arg1, "setapplev")) {
        do_clan_sp(ch, arg2, CP_SET_APPLEV);
        return;
    }
    send_to_char("\r\nClan privileges:\r\n", ch);
    for (i = 0; i < NUM_CP; i++) {
        sprintf(arg1, "\t%s\r\n", clan_privileges[i]);
        send_to_char(arg1, ch);
    }
}

void            do_clan_set(struct char_data * ch, char *arg)
{
    char            arg1[MAX_INPUT_LENGTH],
    arg2[MAX_INPUT_LENGTH];

    half_chop(arg, arg1, arg2);

    if (is_abbrev(arg1, "plan")) {
        do_clan_plan(ch, arg2);
        return;
    }
    if (is_abbrev(arg1, "ranks")) {
        do_clan_ranks(ch, arg2);
        return;
    }
    if (is_abbrev(arg1, "title")) {
        do_clan_titles(ch, arg2);
        return;
    }
    if (is_abbrev(arg1, "privilege")) {
        do_clan_privilege(ch, arg2);
        return;
    }
    /*    if (is_abbrev(arg1, "dues")) {
            do_clan_money(ch, arg2, CM_DUES);
            return;
        }
        if (is_abbrev(arg1, "appfee")) {
            do_clan_money(ch, arg2, CM_APPFEE);
            return;
        }*/
    if (is_abbrev(arg1, "applev")) {
        do_clan_application(ch, arg2);
        return;
    }
    send_clan_format(ch);
}

ACMD(do_clan)
{
    char            arg1[MAX_INPUT_LENGTH],
    arg2[MAX_INPUT_LENGTH];

    half_chop(argument, arg1, arg2);

    if (is_abbrev(arg1, "create")) {
        do_clan_create(ch, arg2);
        return;
    }
    if (is_abbrev(arg1, "destroy")) {
        do_clan_destroy(ch, arg2);
        return;
    }
    /*//if (is_abbrev(arg1, "enroll"  )) { do_clan_enroll(ch,arg2);   return ;}*/
    /*//if (is_abbrev(arg1, "expel"   )) { do_clan_expel(ch,arg2);    return ;}*/
    /*  if (is_abbrev(arg1, "who")) {
          do_clan_who(ch);
          return;
      }*/
    /*    if (is_abbrev(arg1, "status")) {
            do_clan_status(ch);
            return;
        }*/
    if (is_abbrev(arg1, "info")) {
        do_clan_info(ch, arg2);
        return;
    }
    /*if (is_abbrev(arg1, "apply"   )) { do_clan_apply(ch,arg2);    return ;}
    //if (is_abbrev(arg1, "demote"  )) { do_clan_demote(ch,arg2);   return ;}
    //if (is_abbrev(arg1, "promote" )) { do_clan_promote(ch,arg2);  return ;}
    //if (is_abbrev(arg1, "set"     )) { do_clan_set(ch,arg2);      return ;}*/
    if (is_abbrev(arg1, "withdraw")) {
        do_clan_bank(ch, arg2, CB_WITHDRAW);
        return;
    }
    if (is_abbrev(arg1, "deposit")) {
        do_clan_bank(ch, arg2, CB_DEPOSIT);
        return;
    }
    send_clan_format(ch);
}
