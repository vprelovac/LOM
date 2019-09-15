/**************************
 *
 * File: Arena.c
 *
 * Writen by:  Kevin Hoogheem aka Goon
 *             Modified by Billy H. Chan (STROM)
 *
 * Implementation of a event driven arena.. were players pay to kill.
 *
 * Using this code without consent by Goon will make your guts spill
 * out or worse.. Maybe I will hire Lauraina Bobbet to come visit you
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rooms.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "arena.h"
#include "constants.h"
#include "class.h"
#include "objs.h"                       


/*   external vars  */
extern FILE    *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern int      top_of_zone_table;
extern int      restrict1;
extern int      top_of_world;
extern int      top_of_mobt;
extern int      top_of_objt;
extern int      top_of_p_table;
extern sh_int   r_mortal_start_room;

int             in_arena = ARENA_OFF;
int             start_time;
int             game_length;
int             lo_lim;
int             arena_red;
int             arena_blue;
int             arena_redteam;
int             arena_blueteam;
int             hi_lim;
int             cost_per_lev;
int             time_to_start;
int             time_left_in_game;
long            arena_pot;
int             arena_sup;
long            bet_pot;
int hall_loaded=0;
struct hall_of_fame_element *fame_list = NULL;

void            sportschan(char *ar)
{
    struct descriptor_data *i;
    char            color_on[24];

    /* set up the color on code */
    *buf1 = '\0';
    sprintf(buf1, "\r\n&G[Arena]:&0 %s\r\n", ar);

    /* now send all the strings out */
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected && i->character &&
                !PRF_FLAGGED(i->character, PRF_NOTEAM) &&
                !PLR_FLAGGED(i->character, PLR_WRITING) &&
                !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {
            send_to_char(buf1, i->character);
        }
    }
    *buf1 = '\0';
    //    *ar = '\0';
}

int not_in_arena(struct char_data *ch)
{
    if (IN_ARENA(ch))
    {
        send_to_char("Unseen force prevents you to do that here.\r\n", ch);
        return 1;
    }
    return 0;
}

int arena_room()
{
    return (real_room(ARENA_ZONE*100+number(0, MAZEW*MAZEW-1)));
}

ACMD(do_bet)
{
    long            newbet;
    struct char_data *bet_on;
    if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))) {
        send_to_char("Cannot bet while in arena! Perhaps pray will do...\r\n", ch);
        return;
    }
    two_arguments(argument, arg, buf);

    if (IS_NPC(ch)) {
        send_to_char("Mobs cant bet on the arena.\r\n", ch);
        return;
    }
    if (!*arg) {
        if (in_arena == ARENA_OFF) {
            send_to_char("Sorry no arena is in going on.\r\n", ch);
            return;
        } else if (in_arena == ARENA_START) {
            send_to_char("Usage: bet <player> <coins>\r\nWarning: You can bet only once\r\n", ch);
            return;
        } else if (in_arena == ARENA_RUNNING) {
            send_to_char("Sorry the fighting has already started, no more bets.\r\n", ch);
            return;
        }
    }
    if (in_arena == ARENA_OFF) {
        send_to_char("Sorry, arena is closed, wait til it opens up to bet.\r\n", ch);
    } else if (in_arena == ARENA_RUNNING) {
        send_to_char("Sorry, the battle has started, no more bets!\r\n", ch);
    } else if (!(bet_on = get_char_vis(ch, arg)))
        send_to_char(NOPERSON, ch);
    else if (bet_on == ch)
        send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else if (!(ROOM_FLAGGED(IN_ROOM(bet_on), ROOM_ARENA)))
        send_to_char("Sorry, that person is not in the arena.\r\n", ch);
    else {
        if (GET_BET_AMT(ch) > 0) {
            send_to_char("Sorry you have already bet.\r\n", ch);
            return;
        }
        GET_BETTED_ON(ch) = bet_on;

        if (!is_number(buf)) {
            send_to_char("That is not valid ammount!\r\n", ch);
            return;
        }
        newbet = atoi(buf);

        if (newbet < 0) {
            send_to_char("Very funny...\r\n", ch);
            return;
        }
        if (newbet == 0) {
            send_to_char("Bet some gold why dont you!\r\n", ch);
            return;
        }
        if (newbet > GET_GOLD(ch)) {
            send_to_char("You don't have that much money!\r\n", ch);
            return;
        }
        if (newbet > 500000) {
            send_to_char("Sorry, the limit is 500000\r\n", ch);
            return;
        }
        *buf2 = '\0';
        GET_GOLD(ch) -= newbet; /* substract the gold - important :) */
        arena_pot += (newbet / 4);
        bet_pot += (newbet - arena_pot);
        GET_BET_AMT(ch) = newbet;
        sprintf(buf2, "You place %ld coins on %s.\r\n", newbet, GET_NAME(bet_on));
        send_to_char(buf2, ch);
        *buf = '\0';
        sprintf(buf, "%s has placed %ld coins on %s.", GET_NAME(ch),
                newbet, GET_NAME(bet_on));
        sportschan(buf);
    }
}

ACMD(do_arena)
{
    char arg[MAX_STRING_LENGTH];
    if (IS_NPC(ch)) {
        send_to_char("Mobs cant play in the arena.\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg);
    if (in_arena == ARENA_OFF) {
        send_to_char("The killing fields are closed right now.\r\n", ch);
    } else if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
        send_to_char("You must be inside a peaceful room before you can enter the arena.\r\n", ch);
    } else if (GET_LEVEL(ch) < lo_lim) {
        sprintf(buf, "Sorry, but you must be at least level %d to enter this arena.\r\n",
                lo_lim);
        send_to_char(buf, ch);
    } else if (PLR_FLAGGED(ch, PLR_KILLER) || PLR_FLAGGED(ch, PLR_THIEF)) {
        send_to_char("Sorry, wanted criminals can not play in the arena.\r\n", ch);
    } else if (GET_LEVEL(ch) > hi_lim) {
        send_to_char("Sorry, the killing fields are not open to you.\r\n", ch);
    } else if (GET_GOLD(ch) < (cost_per_lev * GET_LEVEL(ch))) {
        sprintf(buf, "Sorry, but you need %d coins to enter the arena.\r\n",
                (cost_per_lev *GET_LEVEL(ch)));
        send_to_char(buf, ch);
    } else if (GET_QUESTPOINTS(ch) < (cost_per_lev* GET_LEVEL(ch))) {
        sprintf(buf, "Sorry, but you need %d adventuring points to enter the arena.\r\n",
                (cost_per_lev * GET_LEVEL(ch)));
        send_to_char(buf, ch);
    } else if (in_arena == ARENA_RUNNING) {
        send_to_char("It's too late to join the rumble now.  Wait for the next arena to start.\r\n", ch);
    } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA)) {
        send_to_char("You are in the arena!\r\n", ch);
    } else if (!*arg || (!isname(arg, "red") && !isname(arg, "blue"))) {
        send_to_char("Type 'arena red' to join the red team.\r\nType 'arena blue' to join the blue team.\r\n", ch);
        sprintf(buf, "It will cost you %d coins and adventuring points to enter.\r\n", cost_per_lev * GET_LEVEL(ch));
        send_to_char(buf, ch);
    } else {
        arena_sup++;
        *buf = '\0';
        sprintf(buf, "You invest your coins and adventuring points.\r\n");
        send_to_char(buf, ch);
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_RED);
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_BLUE);
        if (isname(arg, "red"))
        {
            SET_BIT(AFF3_FLAGS(ch), AFF3_RED);
            send_to_char("You join the &RRED Team.&0\r\n", ch);
            sprintf(buf, "%s joins up the &RRED Team.&0", GET_NAME(ch));
            arena_redteam++;
        }
        if (isname(arg, "blue"))
        {
            SET_BIT(AFF3_FLAGS(ch), AFF3_BLUE);
            send_to_char("You join the &BBLUE Team.&0\r\n", ch);
            sprintf(buf, "%s joins up the &BBLUE Team.&0", GET_NAME(ch));
            arena_blueteam++;
        }
        act("$n has been whisked away to the killing fields.", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("Small group of demons pops in and takes you to the killing fields.\r\n", ch);
        char_from_room(ch);
        if (RED(ch))
            char_to_room(ch, real_room(10));
        else
            char_to_room(ch, real_room(11));
        act("$n is droped from the sky.", FALSE, ch, 0, 0, TO_ROOM);
        sportschan(buf);
        look_at_room(ch, 0);
        GET_GOLD(ch) -= (cost_per_lev * GET_LEVEL(ch));
        GET_QUESTPOINTS(ch) -= (cost_per_lev * GET_LEVEL(ch));
        arena_pot += (cost_per_lev * GET_LEVEL(ch));
        /* ok lets give them there free restore and take away all their */
        /* effects so they have to recast them spells onthemselfs       */
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        if (ch->affected)
            while (ch->affected)
                affect_remove(ch, ch->affected);
    }
}


ACMD(do_chaos)
{
    char            cost[MAX_INPUT_LENGTH],
    lolimit[MAX_INPUT_LENGTH];
    char            hilimit[MAX_INPUT_LENGTH],
    start_delay[MAX_INPUT_LENGTH];
    char            length[MAX_INPUT_LENGTH];

    /*Usage: chaos lo hi start_delay cost/lev length*/
    half_chop(argument, lolimit, buf);
    if (isname(lolimit, "stop") && IS_GOD(ch))
    {
        in_arena = ARENA_OFF;
        start_time = 0;
        game_length = 0;
        time_to_start = 0;
        time_left_in_game = 0;
        arena_pot = 0;
        arena_sup = 0;
        bet_pot = 0;
        sprintf(buf, "Arena is shutting down...");
        sportschan(buf);
        do_end_game();
        return;
    }
    else if (isname(lolimit, "prolong"))
    {
        time_to_start++;
        sprintf(buf, "Arena enterance time prolonged...");
        sportschan(buf);
        return;
    }

    if (in_arena != ARENA_OFF) {
        send_to_char("There is an arena running already.\r\n", ch);
        return;
    }
    arena_red=0;
    arena_blue=0;

    if (isname(lolimit, "standard"))
    {
        lo_lim=7;
        hi_lim=50;
        start_time=3;
        cost_per_lev=1;
        game_length=15;
    }
    else {
        lo_lim = atoi(lolimit);
        half_chop(buf, hilimit, buf);
        hi_lim = atoi(hilimit);
        half_chop(buf, start_delay, buf);
        start_time = atoi(start_delay);
        half_chop(buf, cost, buf);
        cost_per_lev = atoi(cost);
        game_length = atoi(buf);
        if (!*lolimit || !*hilimit || !*start_delay || !*cost || !*buf) {
            send_to_char("Usage: ctf lo hi start_delay cost length\r\n", ch);
            send_to_char("See help on 'ctf' for further info.\r\n", ch);
            return;
        }
    }

    if (hi_lim > LVL_IMPL) {
        send_to_char("Please choose a hi_lim under the Imps level\r\n", ch);
        return;
    }
    if (lo_lim < 0)
        silent_end();


    if (lo_lim > hi_lim) {
        send_to_char("Sorry, low limit must be lower than hi limit.\r\n", ch);
        return;
    }
    if ((hi_lim < 0) || (cost_per_lev < 0) || (game_length < 0)) {
        send_to_char("I like positive numbers thank you.\r\n", ch);
        return;
    }
    if (start_time <= 0) {
        send_to_char("Lets at least give them a chance to enter!\r\n", ch);
        return;
    }
    if ((GET_LEVEL(ch) < LVL_IMPL) && (cost_per_lev < 1 || cost_per_lev > 1000)) {
        send_to_char("Entry cost must be minimum 1 and maximum of 1000.\r\n", ch);
        return;
    }
    in_arena = ARENA_START;
    time_to_start = start_time;
    time_left_in_game = 0;
    arena_pot = 0;
    arena_sup = 0;
    arena_redteam = 0;
    arena_blueteam = 0;

    bet_pot = 0;
    load_hall_of_fame();
    {
        struct obj_data *obj, *tmpo;
        for (obj = object_list; obj ; obj = tmpo)
        {tmpo=obj->next;
            if (GET_OBJ_TYPE(obj)==ITEM_RED_FLAG || GET_OBJ_TYPE(obj)==ITEM_BLUE_FLAG)
                extract_obj(obj);
        }
    }
    start_arena();
}

void            start_arena()
{

    if (time_to_start == 0) {
        if (arena_sup > 1) {
            show_jack_pot();
            in_arena = ARENA_RUNNING;   /* start the blood shed */
            time_left_in_game = game_length;
            start_game();
        } else {
            in_arena = ARENA_OFF;
            do_end_game();
            sprintf(buf, "Nobody entered. Arena is shutting down...");
            sportschan(buf);
        }

    } else {
        *buf = '\0';
        if (time_to_start > 1) {
            sprintf(buf, "The fields of blood are opened for levels %d through %d.\r\n         %d hours remaining til start.\r\n         Type 'arena' to join the rage!",
                    lo_lim, hi_lim, time_to_start);
            sportschan(buf);
        } else {
            sprintf(buf, "The fields of blood are opened for levels %d through %d.\r\n         One hour remained til the start.\r\n         Type 'arena' to join the rage!",
                    lo_lim, hi_lim);
            sportschan(buf);
        }
        time_to_start--;
    }
}

void            show_jack_pot()
{
    //    sprintf(buf, "Let the games BEGIN!\r\nThe jackpot for this arena is %ld coins.\r\nThere are %ld coins in the betting pool.", arena_pot, bet_pot);
    //    sportschan(buf);
}

void            start_game()
{
    register struct char_data *i;
    struct descriptor_data *d;
    struct obj_data *flag;
    int ii;
    if (arena_redteam<2 || arena_blueteam<2)
    {
        in_arena = ARENA_OFF;
        do_end_game();
        sprintf(buf, "Not enough participants. Arena is shutting down...");
        sportschan(buf);
        return;
    }
    /*for (ii = 0; ii <= top_of_zone_table; ii++)
      if (zone_table[ii].number == 240)*/
    reset_zone(get_zone_rnum(ARENA_ZONE));
    flag = read_object(FLAG_BLUE, VIRTUAL, 0, 0);    
    global_no_timer=1;
    obj_to_room(flag, arena_room());
    global_no_timer=0;
    flag = read_object(FLAG_RED, VIRTUAL, 0, 0);           
    global_no_timer=1;
    obj_to_room(flag, arena_room());
    global_no_timer=0;

    for (d = descriptor_list; d; d = d->next) {
        if (!d->connected) {
            i = d->character;
            if (ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA) && (i->in_room != NOWHERE)) {
                send_to_char("\r\nThe floor falls out from bellow, droping you in the arena.\r\n", i);
                char_from_room(i);
                char_to_room(i, arena_room());
            }
        }
    }


    do_game();
}

void            do_game()
{

    /*  if (num_in_arena() == 1) {
          in_arena = ARENA_OFF;

      } else
      */
    if (time_left_in_game == 0) {
        if (arena_red==arena_blue)
        {
            sportschan("&WThe match is draw! Arena prolonged by one hour!&0");
            return;
        }
        in_arena = ARENA_OFF;
        find_game_winner();
        do_end_game();
    } else if (num_in_arena() == 0) {
        in_arena = ARENA_OFF;
        silent_end();
    } else if (time_left_in_game != game_length && ((time_left_in_game % 5) || time_left_in_game <= 4)) {
        if (arena_blue>=arena_red)
            sprintf(buf,
                    "With %d hours left for the game, the result is [BLUE %d : RED %d]",
                    time_left_in_game, arena_blue, arena_red);

        else sprintf(buf,
                         "With %d hours left for the game, the result is [RED %d : BLUE %d]",
                         time_left_in_game, arena_red, arena_blue);

        sportschan(buf);
    } else if (time_left_in_game == 1) {
        if (arena_blue>=arena_red)
            sprintf(buf, "One hour left for combat! Result is [BLUE %d : RED %d]",
                    arena_blue, arena_red);
        else  sprintf(buf, "One hour left for combat! Result is [RED %d : BLUE %d]",
                          arena_red, arena_blue);

        sportschan(buf);
    }
    time_left_in_game--;
}

void            find_game_winner()
{
    register struct char_data *i;
    struct descriptor_data *d;
    struct hall_of_fame_element *fame_node;
    int score;
    char winbuf[1000];
    char winteam[2000]="";
    if (arena_red>arena_blue)
        sportschan("&R&FRED Team&0&W wins the arena!!!&0");
    else
        sportschan("&B&FBLUE Team&0 &Wwins the arena!!!&0");
    sprintf(winbuf, "Members of the winning team: ");
    if (arena_red>arena_blue) {
        for (d = descriptor_list; d; d = d->next)
            if (d->connected==CON_PLAYING) {
                i = d->character;
                if (IN_ARENA(i) && RED(i)) {
                    sprintf(winbuf,"%s&c%s&0 ",winbuf, GET_NAME(i));
                    GET_GOLD(i) += 2*(cost_per_lev * GET_LEVEL(i));
                    GET_QUESTPOINTS(i) += 2*(cost_per_lev * GET_LEVEL(i));
                    sprintf(buf2, "\r\nYou have been awarded %ld coins and %ld adventuring points for winning the arena.\r\n",
                            2*(cost_per_lev * GET_LEVEL(i)), 2*(cost_per_lev * GET_LEVEL(i)));
                    send_to_char(buf2, i);
                    //          log(buf2);
                    strcat(winteam, GET_NAME(i));
                    strcat(winteam, " ");
                    score=arena_red;
                }
            }
    }
    else for (d = descriptor_list; d; d = d->next)
            if (d->connected==CON_PLAYING) {
                i = d->character;
                if (IN_ARENA(i) && BLUE(i)) {
                    sprintf(winbuf,"%s&c%s&0 ",winbuf, GET_NAME(i));
                    GET_GOLD(i) += 2*(cost_per_lev * GET_LEVEL(i));
                    GET_QUESTPOINTS(i) += 2*(cost_per_lev * GET_LEVEL(i));
                    sprintf(buf2, "\r\nYou have been awarded %ld coins and %ld adventuring points for winning the arena.\r\n",
                            2*(cost_per_lev * GET_LEVEL(i)), 2*(cost_per_lev * GET_LEVEL(i)));
                    send_to_char(buf2, i);
                    //        log(buf2);
                    strcat(winteam, GET_NAME(i));
                    strcat(winteam, " ");
                    score=arena_red;

                }
            }

    CREATE(fame_node, struct hall_of_fame_element, 1);
    strncpy(fame_node->name, winteam, 99);
    strncpy(fame_node->lastname, GET_TITLE(i), 80);
    fame_node->name[MAX_NAME_LENGTH] = '\0';
    fame_node->lastname[80] = '\0';
    fame_node->date = time(0);
    fame_node->award = score;//(arena_pot);
    fame_node->next = fame_list;
    fame_node->type=ARENA_CTF;
    fame_list = fame_node;
    write_fame_list();
    sportschan(winbuf);

    //find_bet_winners(i);
}

void            silent_end()
{
    struct obj_data *obj, *tmpo;
    for (obj = object_list; obj ; obj = tmpo)
    {tmpo=obj->next;
        if (GET_OBJ_TYPE(obj)==ITEM_RED_FLAG || GET_OBJ_TYPE(obj)==ITEM_BLUE_FLAG)
            extract_obj(obj);
    }

    in_arena = ARENA_OFF;
    start_time = 0;
    game_length = 0;
    time_to_start = 0;
    time_left_in_game = 0;
    arena_pot = 0;
    arena_sup = 0;
    bet_pot = 0;
    sprintf(buf, "Arena is shutting down...");
    sportschan(buf);
}

void            do_end_game()
{
    register struct char_data *i;
    struct descriptor_data *d;
    struct obj_data *obj, *tmpo;
    for (obj = object_list; obj ; obj = tmpo)
    {tmpo=obj->next;
        if (GET_OBJ_TYPE(obj)==ITEM_RED_FLAG || GET_OBJ_TYPE(obj)==ITEM_BLUE_FLAG)
            extract_obj(obj);
    }

    for (d = descriptor_list; d; d = d->next)
        if (!d->connected) {
            i = d->character;
            if (ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA)
                    && (i->in_room != NOWHERE) && (!IS_NPC(i))) {
                GET_HIT(i) = GET_MAX_HIT(i);
                GET_MANA(i) = GET_MAX_MANA(i);
                GET_MOVE(i) = GET_MAX_MOVE(i);
                
                
                leech_from_char(i, -123454321);
                i->mana_leech=0;
                if (i->affected)
                    while (i->affected)
                        affect_remove(i, i->affected);
                REMOVE_BIT(AFF3_FLAGS(i), AFF3_RED);
                REMOVE_BIT(AFF3_FLAGS(i), AFF3_BLUE);
                char_from_room(i);
                char_to_room(i, r_mortal_start_room);
                send_to_char("You are transported from arena to Midgaard!\r\n", i);
                act("$n falls from the sky.", FALSE, i, 0, 0, TO_ROOM);
                if (i->master)
                    stop_follower(i);
                save_char(i, i->in_room);
            }
        }
    time_left_in_game = 0;
}

int             num_in_arena()
{
    register struct char_data *i;
    struct descriptor_data *d;
    int             num = 0;

    for (d = descriptor_list; d; d = d->next)
        if (!d->connected) {
            i = d->character;
            if (ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA)
                    && (i->in_room != NOWHERE)) {
                if (GET_LEVEL(i) < LVL_IMMORT)
                    num++;
            }
        }
    return num;
}

ACMD(do_awho)
{
    struct descriptor_data *d;
    struct char_data *tch;
    int             num = 0;
    *buf2 = '\0';

    if (in_arena == ARENA_OFF) {
        send_to_char("There is no Arena going on right now.\r\n", ch);
        return;
    }
    sprintf(buf, "Gladiators in the Arena\r\n-----------------------\r\n");
    for (d = descriptor_list; d; d = d->next)
        if (!d->connected) {
            tch = d->character;
            if (ROOM_FLAGGED(IN_ROOM(tch), ROOM_ARENA) &&
                    (tch->in_room != NOWHERE) && GET_LEVEL(tch) < LVL_IMMORT) {
                sprintf(buf, "%s  [%-8s %2d %3s] %-20.20s\r\n", buf,
                        (RED(tch) ? "&RRED&0" : "&BBLUE&0"),  GET_LEVEL(tch), CLASS_ABBR(tch), GET_NAME(tch));
            }
        }
    if (arena_blue==arena_red)
        sprintf(buf, "%s-----------------------\r\n\r\nBoth teams are tied with %d points.\r\n", buf, arena_blue);
    else if (arena_blue>arena_red)
        sprintf(buf, "%s-----------------------\r\n\r\n&BBLUE Team&0 leads %d:%d\r\n", buf, arena_blue, arena_red);
    else
        sprintf(buf, "%s-----------------------\r\n\r\n&RRED Team&0 leads %d:%d.\r\n", buf, arena_red, arena_blue);
    if (time_to_start>0)
        sprintf(buf, "%sTime til start: %d hours\r\n", buf, time_to_start+1);
    else
        sprintf(buf, "%sTime left: %d hours\r\n", buf, time_left_in_game+1);
    page_string(ch->desc, buf, 1);
}

ACMD(do_ahall)
{
    char            site[MAX_INPUT_LENGTH],
    format[MAX_INPUT_LENGTH],
    *timestr;
    char            format2[MAX_INPUT_LENGTH];
    struct hall_of_fame_element *fame_node;

    *buf = '\0';
    *buf2 = '\0';


    load_hall_of_fame();
    if (!fame_list) {
        send_to_char("No-one is in the Hall of Fame.\r\n", ch);
        return;
    }
    sprintf(buf2, "%s|---------------------------------------|%s\r\n",
            CCBLU(ch, C_NRM), CCNRM(ch, C_NRM));
    sprintf(buf2, "%s%s|%s         Past Winners of Arena%s         |%s\r\n",
            buf2, CCBLU(ch, C_NRM), CCNRM(ch, C_NRM),
            CCBLU(ch, C_NRM), CCNRM(ch, C_NRM));
    sprintf(buf2, "%s%s|---------------------------------------|%s\r\r\n\n",
            buf2, CCBLU(ch, C_NRM), CCNRM(ch, C_NRM));

    strcpy(format, "%-10.10s  %-16.16s  %-40s\r\n");
    sprintf(buf, format,
            "Date",
            "Score",
            "Team Members"
           );
    strcat(buf2, buf);
    sprintf(buf, format,
            "---------------------------------",
            "---------------------------------",
            "------------------------------------------------------------");
    strcat(buf2, buf);

    strcpy(format2, "%-10.10s  %-16d  %s\r\n");

    for (fame_node = fame_list; fame_node; fame_node = fame_node->next) {
        if (fame_node->date) {
            timestr =asctime(localtime(&(fame_node->date)));
            *(timestr + 10) = 0;
            strcpy(site, timestr);
        } else
            strcpy(site, "Unknown");
        //sprintf(buf, format2, site, fame_node->award, CAP(fame_node->name), CAP(fame_node->lastname));
        if (fame_node->type==ARENA_CTF)
            sprintf(buf, format2, site, fame_node->award, CAP(fame_node->name));

        strcat(buf2, buf);
    }

    page_string(ch->desc, buf2, 1);
    return;
}

void            load_hall_of_fame(void)
{
    FILE           *fl;
    int             date,
    award, type;
    char            name[21],
    lastname[81];

    struct hall_of_fame_element *next_node;

    if (hall_loaded)
        return;
    hall_loaded=1;

    fame_list = NULL;

    if (!(fl = fopen(HALL_FAME_FILE, "rb"))) {
        log("SYSERR: Unable to open hall of fame file");
        return;
    }
    while (fscanf(fl, "%s %s %ld %d %d", name, lastname, &date, &award, &type) == 4) {
        CREATE(next_node, struct hall_of_fame_element, 1);
        strncpy(next_node->name, name, 99);
        strncpy(next_node->lastname, lastname, 80);
        next_node->name[99] = '\0';
        next_node->lastname[80] = '\0';
        next_node->date = date;
        next_node->award = award;
        next_node->type = type;
        next_node->next = fame_list;
        fame_list = next_node;
    }

    fclose(fl);
}


void            write_fame_list(void)
{
    FILE           *fl;

    if (!(fl = fopen(HALL_FAME_FILE, "wb"))) {
        /*    syserrlog("Error writing _hall_of_fame_list", FALSE); */
        log("Error writing _hall_of_fame_list");
        return;
    }
    write_one_fame_node(fl, fame_list); /* recursively write from end to
                                         * start */
    fclose(fl);

    return;
}

void            write_one_fame_node(FILE * fp, struct hall_of_fame_element * node)
{
    if (node) {
        write_one_fame_node(fp, node->next);
        fprintf(fp, "%s %s %ld %ld %d\n", node->name, node->lastname,
                (long) node->date, node->award, node->type);
    }
}

void            find_bet_winners(struct char_data * winner)
{
    register struct char_data *i;
    struct descriptor_data *d;

    *buf = '\0';

    for (d = descriptor_list; d; d = d->next)
        if (!d->connected) {
            i = d->character;
            if ((!IS_NPC(i)) && (i->in_room != NOWHERE) &&
                    (GET_BETTED_ON(i) == winner) && GET_BET_AMT(i) > 0) {
                sprintf(buf, "You have won %d coins for your bet.\r\n",
                        GET_BET_AMT(i) * 2);
                send_to_char(buf, i);
                GET_GOLD(i) += GET_BET_AMT(i) * 2;
                GET_BETTED_ON(i) = NULL;
                GET_BET_AMT(i) = 0;
            }
        }
}
