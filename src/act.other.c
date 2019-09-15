/* ************************************************************************
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


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
#include "house.h"
#include "arena.h"
#include "constants.h"
#include "vt100c.h"
#include "auction.h"
#include "clan.h"
#include "events.h"
#include "ident.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern struct title_type titles1[LVL_IMPL + 1];
extern char *spells[];
extern struct syllable syls[];
extern int      top_of_world;   /* In db.c */

extern struct index_data *obj_index;   /* index table for object file	 */
extern struct obj_data *obj_proto;     /* prototypes for objs		 */

int mag_manacost(struct char_data * ch, int spellnum);
void make_scraps( struct obj_data *obj );
void write_aliases(struct char_data * ch);
void write_quests(struct char_data * ch);
void auction_reset(void);
/* extern procedures */
SPECIAL(shop_keeper);
extern int not_in_arena(struct char_data *ch);
void termScreenOff(struct char_data * ch);
extern CHAR_DATA *supermob;
#define QUAD 5
int pow(int x, int y)
{
    int p;
    for (p=1; y>0; --y)
        p*=x;
    return p;
}

void load_quad()
{
    struct obj_data *obj;
    int             to_room;
    extern int      top_of_world;

    if (players_online()<2)
        return;
    obj=get_obj_num(real_object(QUAD));

    if (obj==NULL)
    {
        obj=read_object(QUAD, VIRTUAL, 0, 0);
        do {
            to_room = number(0, top_of_world);
        } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_ARENA));
        obj_to_room(obj, to_room);
        GET_OBJ_TIMER(obj)=1000;
        INFO_OUT("\r\n&GINFO || &cQuad damage loaded.&0\r\n");
    }
}



char *is_hunted(struct char_data *ch)
{
    struct char_data *mob, *next_ch;
    extern struct char_data *character_list;
    for (mob = character_list; mob; mob = next_ch) {
        next_ch = mob->next;
        if (IS_NPC(mob) && HUNTING(mob)==ch)
        {
            return strcpy(buf, GET_NAME(mob));
        }
    }
    return NULL;
}


void update_blood(void)
{
    int i;
    extern int top_of_world;

    for (i = 0; i < top_of_world; i++)
        if (RM_BLOOD(i) > 0)
            RM_BLOOD(i) -=1;
}

ACMD(do_quit)
{
//    void die(struct char_data * ch, struct char_data * killer);
    void Crash_rentsave(struct char_data * ch, int cost);
    extern int free_rent;
    struct descriptor_data *d, *next_d;

    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    if (IS_NPC(ch) || !ch->desc)
        return;

    if (subcmd != SCMD_QUIT && GET_LEVEL(ch) < LVL_IMMORT)
        send_to_char("You have to type quit - no less, to quit!\r\n", ch);
    else if (GET_POS(ch) == POS_FIGHTING)
        send_to_char("No way!  You're fighting for your life!\r\n", ch);
    else if (AFF_FLAGGED(ch, AFF_SLEEP))
        send_to_char("You are to relaxed to quit now.\r\n", ch);
    else if (IN_ARENA(ch))
        send_to_char("First let the arena finish!\r\n", ch);
    else if (GET_POS(ch) < POS_STUNNED) {
        send_to_char("You die before your time...\r\n", ch);
        die(ch, NULL);
    } else if (IS_SET(PRF2_FLAGS(ch), PRF2_NOQUIT))
        send_to_char("You have safety Quit-Interception on.  Toggle it with '&wnoquit&0' first.\r\n", ch);
    else if (ch->pk_timer>0)
    {
        sprintf(buf, "&cYou feel your adrenaline rushing, you can not quit yet.&0\r\n");
        send_to_char(buf, ch);
        return;
    }
    else {

        if (QUESTING(ch))
        {
            if (!*arg)
            {

                ch_printf(ch, "You are curently part of a quest. Leaving now will cause you to lose exp and adventure points\r\nIf you want that, use 'quit yes' to leave.\r\n", buf);
                return;
            }
            else if (isname(arg, "yes"))
            {
                send_to_char("\r\n&RYou loose exp and adventuring points.&0\r\n\r\n", ch);
                GET_QUESTPOINTS(ch)-=GET_QUESTPOINTS(ch)/8;
                GET_EXP(ch)-=LEVELEXP(ch)/8;
            }
        }
        if (is_hunted(ch))
        {
            if (!*arg)
            {

                ch_printf(ch, "You are curently being hunted by %s.\r\nQuiting now will cause you to lose both experience and quest points.\r\nIf you want that, use 'quit yes' to leave.\r\n", buf);
                return;
            }
            else if (isname(arg, "yes"))
            {
                send_to_char("\r\n&RYou loose exp and adventuring points.&0\r\n\r\n", ch);
                GET_QUESTPOINTS(ch)-=GET_QUESTPOINTS(ch)/8;
                GET_EXP(ch)-=LEVELEXP(ch)/8;
            }
        }
        if (IS_AFFECTED(ch, AFF_POISON) || IS_AFFECTED(ch, AFF_BLIND))
        {
            if (!*arg)
            {
                ch_printf(ch, "You are curently affected by poison or blindness.\r\nQuiting now will cause you to lose both experience and quest points.\r\nIf you want that, use 'quit yes' to leave.\r\n", buf);
                return;
            }
            else if (isname(arg, "yes"))
            {
                send_to_char("\r\n&RYou loose exp and adventuring points.&0\r\n\r\n", ch);
                GET_QUESTPOINTS(ch)-=GET_QUESTPOINTS(ch)/8;
                GET_EXP(ch)-=LEVELEXP(ch)/8;
            }
        }
        send_to_char("You flicker away from reality...\r\n", ch);
        if (!GET_INVIS_LEV(ch))
            act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
        sprintf(buf, "%s has quit the game.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        /*    if (AFF2_FLAGGED(ch,AFF2_WRATH) && WRATHOBJ(ch)) {
            extract_obj(WRATHOBJ(ch));WRATHOBJ(ch)=NULL;}*/
        /* kill off all sockets connected to the same player as the one
           who is trying to quit.  Helps to maintain sanity as well as
           prevent duping. */
        for (d = descriptor_list; d; d = next_d) {
            next_d = d->next;
            if (d == ch->desc)
                continue;
            if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
                close_socket(d);
        }

        if (free_rent)
            Crash_rentsave(ch, 0);
        save_char(ch, ch->in_room);
        extract_char(ch);	/* Char is saved in extract char */
    }
}



ACMD(do_save)
{
    char arg[MAX_STRING_LENGTH];
    if (IS_NPC(ch) || !ch->desc)
        return;
    argument = one_argument(argument, arg);
    if (cmd) {
        if (isname(arg, "default"))// && (!AFF_FLAGGED(ch, AFF_VISITING)))
            sprintf(buf, "Saving default loadroom for %s.\r\n", GET_NAME(ch));
        else
            sprintf(buf, "Saving &G%s, equipment and aliases&0.\r\n", GET_NAME(ch));
        send_to_char(buf, ch);
    }
    if (isname(arg, "default"))
        save_char(ch, ch->in_room);
    else
        save_char(ch, ch->in_room);
    Crash_crashsave(ch);
    if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
        House_crashsave(world[ch->in_room].number);
}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
    send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}



ACMD(do_sneak)
{
    struct affected_type af;
    int percent;

    if (!GET_SKILL(ch, SKILL_SNEAK))
    {
        send_to_char("You don't even know how to sneak.\r\n", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_SNEAK))
    {

        affect_from_char(ch, SKILL_SNEAK);
        send_to_char("You stop sneaking.\r\n", ch);
    }
    else
    {
        send_to_char("Ok. You will now sneak.\r\n", ch);


        af.type = SKILL_SNEAK;
        af.duration = -1;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SNEAK;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        affect_to_char(ch, &af);
    }
}



ACMD(do_hide)
{
    int percent;
    struct affected_type af;

    send_to_char("You attempt to hide yourself.\r\n", ch);

    if (IS_AFFECTED(ch, AFF_HIDE))
        affect_from_char(ch, SKILL_HIDE);

    percent = number(1, 111);	/* 101% is a complete failure */

    if (percent > GET_SKILL(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
        return;


    //SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
    af.type = SKILL_HIDE;
    af.duration = -1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_HIDE;
    af.bitvector2 = 0;
    af.bitvector3 = 0;
    affect_to_char(ch, &af);

    improve_skill(ch, SKILL_HIDE, 1);
}




ACMD(do_steal)
{
    struct char_data *vict;
    struct obj_data *obj;
    char vict_name[240];
    char obj_name[240];
    int percent, gold, eq_pos, pcsteal = 0;
    extern int pt_allowed;
    bool ohoh = FALSE;

    ACMD(do_gen_comm);

    argument = one_argument(argument, obj_name);
    one_argument(argument, vict_name);

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
        send_to_char("This room has such a peaceful feeling.\r\n", ch);
        return;
    }
    if (not_in_arena(ch))
        return;

    if (!(vict = get_char_room_vis(ch, vict_name))) {
        send_to_char("Steal what from who?\r\n", ch);
        return;
    } else if (vict == ch) {
        send_to_char("Come on now, that's rather stupid!\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch,vict))
        //pt_allowed) {
        //if (!IS_NPC(vict) && !PLR_FLAGGED(vict, PLR_THIEF) &&
        // !PLR_FLAGGED(vict, PLR_KILLER) && !PLR_FLAGGED(ch, PLR_THIEF)) {
        /* SET_BIT(ch->specials.act, PLR_THIEF); send_to_char("Okay,
           you're the boss... you're now a THIEF!\r\n",ch);
           sprintf(buf, "PC Thief bit set on %s", GET_NAME(ch));
           log(buf); */
        pcsteal = 1;
    //	}
    //	if (PLR_FLAGGED(ch, PLR_THIEF))
    //	    pcsteal = 1;

    /* We'll try something different... instead of having a thief
       flag, just have PC Steals fail all the time. */
    //    }
    /* 101% is a complete failure */
    percent = number(1, 111) - dex_app_skill[GET_DEX(ch)].p_pocket + 4*(GET_LEVEL(vict)-GET_LEVEL(ch))+(int) (FOL_SKIG(ch) ? 10 : 0);

    if (GET_POS(vict) < POS_SLEEPING)
        percent = -1;		/* ALWAYS SUCCESS */

    /* NO NO With Imp's and Shopkeepers! */
    if ((GET_LEVEL(vict) >= LVL_IMMORT) || pcsteal ||
            GET_MOB_SPEC(vict) == shop_keeper)
        percent = 101;		/* Failure */

    if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

        if (!(obj = get_obj_in_list_vis(ch, obj_name, vict->carrying))) {

            for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
                if (GET_EQ(vict, eq_pos) &&
                        (isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
                        CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
                    obj = GET_EQ(vict, eq_pos);
                    break;
                }
            if (!obj) {
                act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
                return;
            } else {		/* It is equipment */
                if (GET_POS(vict)!=POS_SLEEPING && GET_POS(vict)!=POS_STUNNED) {
                    send_to_char("Your victim must be asleep before you even try doing this.\r\n", ch);
                    return;
                } else if (number(1,101)>(GET_SKILL(ch, SKILL_IMPROVED_STEAL)+(int) ( FOL_SKIG(ch) ? 10 : 0)+5*(GET_LEVEL(ch)-GET_LEVEL(vict))))
                {
                    send_to_char("Oh, clumsy you! You wake your victim!\r\n",ch);
                    if (affected_by_spell(vict, SPELL_SLEEP))
                        affect_from_char(vict, SPELL_SLEEP);
                    if (affected_by_spell(vict, SPELL_NAP))
                        affect_from_char(vict, SPELL_NAP);
                    GET_POS(vict)=POS_STANDING;
                    act("$n looks over the room suspiciously.", FALSE, vict, NULL, NULL, TO_ROOM);
                    act("You awake and find $N's hand in your pocket.", FALSE, vict, 0, ch, TO_CHAR);
                    if (!number(0,3) && IS_NPC(vict))
                    {
                        act("$n loudly exclaims, 'It must be your dirty hands!'", FALSE, vict, NULL, NULL, TO_ROOM);
                        hit(vict, ch, TYPE_UNDEFINED);
                    }
                } else {
                    act("&YYou skillfully unequip $p and steal it.&0", FALSE, ch, obj, 0, TO_CHAR);
                    act("$n skillfully unequips $p and steals it from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
                    obj_to_char(unequip_char(vict, eq_pos), ch);
                    ch->pk_timer=MAX(ch->pk_timer, IS_NPC(vict)?30*PASSES_PER_SEC:300*PASSES_PER_SEC);
                }
            }
        } else {		/* obj found in inventory */

            percent += GET_OBJ_WEIGHT(obj)/2;	/* Make heavy harder */

            if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
                ohoh = TRUE;
                act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
                act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
                act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
            } else {		/* Steal the item */
                if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
                    if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
                        obj_from_char(obj);
                        obj_to_char(obj, ch);
                        send_to_char("&YGot it!&0\r\n", ch);
                        improve_skill(ch, SKILL_STEAL, 2); 
                        improve_skill(ch, SKILL_IMPROVED_STEAL, 2);  
                        ch->pk_timer=MAX(ch->pk_timer, IS_NPC(vict)?30*PASSES_PER_SEC:300*PASSES_PER_SEC);
                    }
                } else
                    send_to_char("You cannot carry that much.\r\n", ch);
            }
        }
    } else {			/* Steal some coins */
        if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
            ohoh = TRUE;
            act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
            act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, vict, TO_VICT);
            act("$n tries to steal coins from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
        } else {
            /* Steal some coins */
            gold = (int) ((GET_GOLD(vict) * (GET_SKILL(ch, SKILL_STEAL) - 20 + number(0, 30)) / 800));
            /*      gold = MIN(1000, gold);*/
            if (gold > 0) {
                GET_GOLD(ch) += gold;
                GET_GOLD(vict) -= gold;
                sprintf(buf, "&YBingo!  You got %d coins.&0\r\n", gold);
                send_to_char(buf, ch);                    
                improve_skill(ch, SKILL_STEAL, 2);
                improve_skill(ch, SKILL_IMPROVED_STEAL, 2);  
                ch->pk_timer=MAX(ch->pk_timer, IS_NPC(vict)?30*PASSES_PER_SEC:300*PASSES_PER_SEC);
            } else {
                send_to_char("You couldn't get any coins...\r\n", ch);
            }
        }
    }

    if (ohoh && IS_NPC(vict) && AWAKE(vict))
        hit(vict, ch, TYPE_UNDEFINED);
}

ACMD(do_learned)
{
    struct char_data *vict;
    void list_learned(struct char_data * ch, struct char_data * vict);

    one_argument(argument, arg);

    if (*arg) {
        if(GET_LEVEL(ch) >= LVL_IMMORT) {
            if(!(vict = get_char_vis(ch, arg))) {
                send_to_char(NOPERSON, ch);
                return;
            }
            list_learned(vict, ch);
            return;
        }
    }
    else
        list_learned(ch, NULL);
}


ACMD(do_traps)
{
    struct char_data *vict;
    void list_traps(struct char_data * ch, struct char_data * vict);

    one_argument(argument, arg);

    if (*arg) {
        if(GET_LEVEL(ch) >= LVL_IMMORT) {
            if(!(vict = get_char_vis(ch, arg))) {
                send_to_char(NOPERSON, ch);
                return;
            }
            list_traps(vict, ch);
            return;
        }
    }
    else
        list_traps(ch, NULL);
}


ACMD(do_practice)
{
    struct char_data *vict;
    void list_skills(struct char_data * ch, struct char_data * vict);

    one_argument(argument, arg);

    if (*arg) {
        if(GET_LEVEL(ch) >= LVL_IMMORT) {
            if(!(vict = get_char_vis(ch, arg))) {
                send_to_char(NOPERSON, ch);
                return;
            }
            list_skills(vict, ch);
            return;
        }
        send_to_char("You can only practice skills in your guild.\r\n", ch);
    }
    else
        list_skills(ch, NULL);
}



ACMD(do_visible)
{
    void appear(struct char_data * ch);
    void perform_immort_vis(struct char_data * ch);

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
        perform_immort_vis(ch);
        return;
    }
    
    if (!AFF2_FLAGGED(ch, AFF2_MOVE_HIDDEN) && !IS_AFFECTED(ch, AFF_INVISIBLE))
    {
    	send_to_char("You are already visible.\r\n", ch);
    	return;
    }
    if (AFF2_FLAGGED(ch, AFF2_MOVE_HIDDEN))
    {
        affect_from_char(ch, SKILL_MOVE_HIDDEN);
        send_to_char("You step out the shadows.\r\n", ch);
    }
    if IS_AFFECTED
    (ch, AFF_INVISIBLE) {
        appear(ch);
        send_to_char("You break the spell of invisibility.\r\n", ch);
    }
                                                                 
        
    
}

ACMD(do_land)
{
    if (AFF_FLAGGED(ch, AFF_FLYING) || affected_by_spell(ch, SPELL_FLY)) {
        leech_from_char(ch, SPELL_FLY);
        affect_from_char(ch, SPELL_FLY);
        if (!(AFF_FLAGGED(ch, AFF_FLYING) || affected_by_spell(ch, SPELL_FLY)))
            send_to_char("You break the spell of flying.\r\n", ch);
    }
    if (AFF_FLAGGED(ch, AFF_WATERWALK)) {
        leech_from_char(ch, SPELL_WATERWALK);
        affect_from_char(ch, SPELL_WATERWALK);
        if (!(AFF_FLAGGED(ch, AFF_WATERWALK)))
            send_to_char("You land to the ground.\r\n", ch);
    }
}

ACMD(do_title)
{
    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (IS_NPC(ch))
        send_to_char("Your title is fine... go away.\r\n", ch);
    else if (PLR_FLAGGED(ch, PLR_NOTITLE))
        send_to_char("You do not have the power to give yourself a title yet.\r\n", ch);
    else if (strstr(argument, "(") || strstr(argument, ")"))
        send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
    else if (strlen(argument) >MAX_TITLE_LENGTH) {
        sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
                MAX_TITLE_LENGTH);
        send_to_char(buf, ch);
    } else {
        set_title(ch, argument);
        sprintf(buf, "Okay, you're now &G%s %s&0\r\n", GET_NAME(ch), GET_TITLE(ch));
        send_to_char(buf, ch);
    }
}


int perform_group(struct char_data * ch, struct char_data * vict)
{
    if (ch != vict && (IS_AFFECTED(vict, AFF_GROUP) || !CAN_SEE(ch, vict) || (vict->master!=ch))) {
        if (!CAN_SEE(ch, vict))
            act("$n tried to group you but couldn't see you.", FALSE, ch, NULL, vict, TO_VICT);
        return 0;
    }
    SET_BIT(AFF_FLAGS(vict), AFF_GROUP);
    SET_BIT(AFF_FLAGS(ch), AFF_GROUP);

    if (ch != vict) {
        if (ch->master)
            ch = ch->master;
        act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
        act("You are now a member of &W$n&0's group.", FALSE, ch, 0, vict, TO_VICT);
        act("$N is now a member of &W$n&0's group.", FALSE, ch, 0, vict, TO_NOTVICT);
    } else
        send_to_char("\r\n", ch);
    return 1;
}


void print_group(struct char_data * ch)
{

    struct char_data *k;
    struct follow_type *f;
    char bufn[100];
    if (!IS_AFFECTED(ch, AFF_GROUP) || count_group(ch) == 1) {

        send_to_char("You are not the member of any group.\r\r\n\n", ch);
        k = ch;
        strcpy(bufn,GET_NAME(k));
        bufn[15]='\0';
        CAP(bufn);

        send_to_char("&B  Rank          Name             Hit         Mana         Move      Exp to lev\r\n&b--------   ---------------   ----------   ----------   ----------   ----------&0\r\n", ch);
        sprintf(buf, "[&c%2d %s&0]   &Y%-18s&c%4d&0 /&c%-7d%4d&0 /&c%-7d%4d&0 /&c%-7d%10d&0\r\n",
                GET_LEVEL(k), CLASS_ABBR(k), bufn,
                GET_HIT(k), GET_MAX_HIT(k), GET_MANA(k), GET_MAX_MANA(k), GET_MOVE(k), GET_MAX_MOVE(k),
                //total_exp(GET_LEVEL(k))-GET_EXP(k));
                LEVELEXP(k)-GET_EXP(k));
        send_to_char(buf, ch);
    } else {
        if (ch->player_specials->group_name) {
            sprintf(buf, "&C%s&0:\r\r\n\n", ch->player_specials->group_name);
            send_to_char(buf, ch);
        } else if (ch->master && ch->master->player_specials->group_name) {
            sprintf(buf, "&C%s&0:\r\r\n\n", ch->master->player_specials->group_name);
            send_to_char(buf, ch);
        } else
            send_to_char("Your group consists of:\r\r\n\n", ch);

        k = (ch->master ? ch->master : ch);
        strcpy(bufn,GET_NAME(k));
        bufn[15]='\0';
        CAP(bufn);

        send_to_char("&B  Rank          Name             Hit         Mana         Move      Exp to lev\r\n&b--------   ---------------   ----------   ----------   ----------   ----------&0\r\n", ch);
        if (IS_AFFECTED(k, AFF_GROUP)) {
            sprintf(buf, "[&c%2d %s&0]   &Y%-18s&c%4d&0 /&c%-7d%4d&0 /&c%-7d%4d&0 /&c%-7d%10d&0\r\n",
                    GET_LEVEL(k), CLASS_ABBR(k), bufn,
                    GET_HIT(k), GET_MAX_HIT(k), GET_MANA(k), GET_MAX_MANA(k), GET_MOVE(k), GET_MAX_MOVE(k),
                    //total_exp(GET_LEVEL(k))-GET_EXP(k));
                    LEVELEXP(k)-GET_EXP(k));
            send_to_char(buf, ch);
        }
        for (f = k->followers; f; f = f->next) {
            if (!IS_AFFECTED(f->follower, AFF_GROUP))
                continue;
            strcpy(bufn,GET_NAME(f->follower));
            bufn[15]='\0';
            CAP(bufn);
            if (!IS_NPC(f->follower))
                sprintf(buf, "[&c%2d %s&0]   &Y%-18s&c%4d&0 /&c%-7d%4d&0 /&c%-7d%4d&0 /&c%-7d%10d&0\r\n",
                        GET_LEVEL(f->follower), CLASS_ABBR(f->follower), bufn,
                        GET_HIT(f->follower), GET_MAX_HIT(f->follower), GET_MANA(f->follower), GET_MAX_MANA(f->follower), GET_MOVE(f->follower), GET_MAX_MOVE(f->follower),
                        //total_exp(GET_LEVEL(f->follower))-GET_EXP(f->follower));
                        LEVELEXP(f->follower)-GET_EXP(f->follower));
            else
                sprintf(buf, "[&c%2d NPC&0]   &Y%-18s&c%4d&0 /&c%-7d%4d&0 /&c%-7d%4d&0 /&c%-7d%10d&0\r\n",
                        GET_LEVEL(f->follower), bufn,
                        GET_HIT(f->follower), GET_MAX_HIT(f->follower), GET_MANA(f->follower), GET_MAX_MANA(f->follower), GET_MOVE(f->follower), GET_MAX_MOVE(f->follower),
                        0);
            send_to_char(buf, ch);
        }
        /*if (AFF3_FLAGGED(k, AFF3_GUARD))
    {
             if (k!=ch)
                 act("\r\n$n is guarding your group.",FALSE, k, 0, ch, TO_VICT);
             else
                 send_to_char("\r\nYou are guarding the group.", ch);
    }
          */
        if (k->guarding && is_same_group(k->guarding, ch))
        {

            ch_printf(ch, "\r\n%s %s guarding %s.", k==ch? "You":GET_NAME(k),k==ch? "are":"is",k->guarding==ch?"you": GET_NAME(k->guarding));

        }

        for (f = k->followers; f; f = f->next) {
            if (!IS_AFFECTED(f->follower, AFF_GROUP))
                continue;

            k=f->follower;

            if (k->guarding && is_same_group(k->guarding, ch))
            {
                ch_printf(ch, "\r\n%s %s guarding %s.", k==ch? "You":GET_NAME(k),k==ch? "are":"is",k->guarding==ch?"you": GET_NAME(k->guarding));
            }

        }

    }
    send_to_char("\r\n", ch);




    // OLD CODE for vitality based
    /*struct char_data *k;
    struct follow_type *f;
    char bufn[100];
    if (!IS_AFFECTED(ch, AFF_GROUP) || count_group(ch) == 1) {

    send_to_char("You are not the member of any group.\r\r\n\n", ch);
    k = ch;
    strcpy(bufn,GET_NAME(k));
    bufn[15]='\0';
    CAP(bufn);
    	
    send_to_char("&B  Rank               Name               Vitality    Exp to lev\r\n&b--------   -------------------------   ----------   ----------&0\r\n", ch);
    sprintf(buf, "[&c%2d %s&0]   &Y%-28s&c%4.0f&0 /&c%-7.0f%10d&0\r\n",
    GET_LEVEL(k), CLASS_ABBR(k), bufn,
    GET_HIT(k), GET_MAX_HIT(k), total_exp(GET_LEVEL(k))-GET_EXP(k));		
    send_to_char(buf, ch);
} else {
    if (ch->player_specials->group_name) {
     sprintf(buf, "&C%s&0:\r\r\n\n", ch->player_specials->group_name);
     send_to_char(buf, ch);
} else if (ch->master && ch->master->player_specials->group_name) {
     sprintf(buf, "&C%s&0:\r\r\n\n", ch->master->player_specials->group_name);
     send_to_char(buf, ch);
} else
     send_to_char("Your group consists of:\r\r\n\n", ch);

    k = (ch->master ? ch->master : ch);
    strcpy(bufn,GET_NAME(k));
    bufn[15]='\0';
    CAP(bufn);	
    send_to_char("&B  Rank               Name               Vitality    Exp to lev\r\n&b--------   -------------------------   ----------   ----------&0\r\n", ch);
    if (IS_AFFECTED(k, AFF_GROUP)) {
    sprintf(buf, "[&c%2d %s&0]   &Y%-28s&c%4.0f&0 /&c%-7.0f%10d&0\r\n",
    GET_LEVEL(k), CLASS_ABBR(k), bufn,
    GET_HIT(k), GET_MAX_HIT(k), total_exp(GET_LEVEL(k))-GET_EXP(k));					
     send_to_char(buf, ch);
}
    for (f = k->followers; f; f = f->next) {
     if (!IS_AFFECTED(f->follower, AFF_GROUP))
    continue;
        strcpy(bufn,GET_NAME(f->follower));
        bufn[15]='\0';
        CAP(bufn);
     if (!IS_NPC(f->follower))
    sprintf(buf, "[&c%2d %s&0]   &Y%-28s&c%4.0f&0 /&c%-7.0f%10d&0\r\n",
    GET_LEVEL(f->follower), CLASS_ABBR(f->follower), bufn,
    GET_HIT(f->follower), GET_MAX_HIT(f->follower), 
    total_exp(GET_LEVEL(f->follower))-GET_EXP(f->follower));
     else
    sprintf(buf, "[&c%2d NPC&0]   &Y%-28s&c%4.0f&0 /&c%-7.0f%10d&0\r\n",
    GET_LEVEL(f->follower), bufn,
    GET_HIT(f->follower), GET_MAX_HIT(f->follower),0);
     send_to_char(buf, ch);
}
    if (AFF3_FLAGGED(k, AFF3_GUARD))
{
        if (k!=ch)
            act("\r\n$n is guarding your group.",FALSE, k, 0, ch, TO_VICT);
        else
            send_to_char("\r\nYou are guarding the group.", ch);
}

}
    send_to_char("\r\n", ch);*/
}



ACMD(do_group)
{
    char bbf[100];
    struct char_data *vict;
    struct follow_type *f;
    int found, cnt = 0;
    int imo = 1;

    one_argument(argument, buf);

    if (!*buf) {
        /*if (!AFF_FLAGGED(ch,AFF_GROUP)){
          SET_BIT(AFF_FLAGS(ch), AFF_GROUP);
         */
        print_group(ch);
        return;
    }
    if (ch->master) {
        act("You can not enroll group members without being head of a group.",
            FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    if (!str_cmp(buf, "all")) {
        for (found = 0, f = ch->followers; f; f = f->next)
            found += perform_group(ch, f->follower);
        if (!found && AFF_FLAGGED(ch, AFF_GROUP))
            send_to_char("Everyone following you is in your group.\r\n", ch);
        SET_BIT(AFF_FLAGS(ch), AFF_GROUP);
        for (found = 0, f = ch->followers; f; f = f->next)
            if (!IS_NPC(f->follower))
                found++;
        sprintf(bbf, "%s's group", GET_NAME(ch));
        if (found)
            ch->player_specials->group_name = str_dup(bbf);
        else {
            if (ch->player_specials->group_name) {
                DISPOSE(ch->player_specials->group_name);
                ch->player_specials->group_name = 0;
            }
        }
        return;
    }
    if (!(vict = get_char_room_vis(ch, buf)))
        send_to_char(NOPERSON, ch);
    else if ((vict->master != ch) && (vict != ch))
        act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
    else if (ch == vict)
        send_to_char("You can't do that.\r\n", ch);
    else {

        if (!IS_AFFECTED(vict, AFF_GROUP) && ch != vict) {
            SET_BIT(AFF_FLAGS(ch), AFF_GROUP);
            perform_group(ch, vict);
            if (!IS_NPC(vict) && !(ch->player_specials->group_name)) {
                sprintf(bbf, "%s's group", GET_NAME(ch));
                ch->player_specials->group_name = str_dup(bbf);
            }
        } else {
            if (ch != vict) {
                act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
                act("You are no longer member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
                act("$N is no longer member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
                REMOVE_BIT(AFF_FLAGS(vict), AFF_GROUP);
                if (count_group(ch) == 1)
                    REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
            } else
                send_to_char("\r\n", ch);
        }
    }
}

ACMD(do_ungroup)
{
    struct follow_type *f, *next_fol;
    struct char_data *tch;
    void stop_follower(struct char_data * ch);
    int pomi;

    one_argument(argument, buf);
    if (!*buf) {
        if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
            send_to_char("But you lead no group!\r\n", ch);
            return;
        }
        sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
        pomi = 1;
        for (f = ch->followers; f; f = next_fol) {
            next_fol = f->next;
            if (IS_AFFECTED(f->follower, AFF_GROUP)) {
                pomi++;
                REMOVE_BIT(AFF_FLAGS(f->follower), AFF_GROUP);
                send_to_char(buf2, f->follower);
                stop_follower(f->follower);
            }
        }

        REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
        if (pomi > 1)
            send_to_char("&BYou disband the group.&0\r\n", ch);
        if (ch->player_specials->group_name) {
            DISPOSE(ch->player_specials->group_name);
            ch->player_specials->group_name = 0;
        }
        return;
    }
    if (!(tch = get_char_room_vis(ch, buf))) {
        send_to_char("There is no such person!\r\n", ch);
        return;
    }
    if (tch->master != ch) {
        send_to_char("That person is not following you!\r\n", ch);
        return;
    }
    if (!IS_AFFECTED(tch, AFF_GROUP)) {
        send_to_char("That person isn't in your group.\r\n", ch);
        return;
    }
    REMOVE_BIT(AFF_FLAGS(tch), AFF_GROUP);

    act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
    act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
    act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);

    stop_follower(tch);
    if (count_group(ch) == 1)
        REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
}




ACMD(do_report)
{
    struct char_data *k;
    struct follow_type *f;
    float temp;


    sprintf(buf, "You report: &C%d%%&0 hp, &C%d%%&0 mana, &C%d%%&0 mv\r\n",
            (GET_HIT(ch) * 100 / GET_MAX_HIT(ch)),
            (GET_MANA(ch) * 100 / GET_MAX_MANA(ch)),
            (GET_MOVE(ch) * 100 / GET_MAX_MOVE(ch)));
    send_to_char(buf, ch);
    if (IS_AFFECTED(ch, AFF_GROUP)) {
        sprintf(buf, "%s reports: %d%% hp, %d%% mana, %d%% mv\r\n",
                GET_NAME(ch), (GET_HIT(ch) * 100 / GET_MAX_HIT(ch)),
                (GET_MANA(ch) * 100 / GET_MAX_MANA(ch)),
                (GET_MOVE(ch) * 100 / GET_MAX_MOVE(ch)));
        CAP(buf);

        k = (ch->master ? ch->master : ch);

        for (f = k->followers; f; f = f->next)
            if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower != ch)
                send_to_char(buf, f->follower);
        if (k != ch)
            send_to_char(buf, k);


        // OLD CODE for vitality

        /*  sprintf(buf, "You report &C%d%%&0 vitality.\r\n",
           (int) (GET_HIT(ch) * 100 / GET_MAX_HIT(ch)));
          send_to_char(buf, ch);
          if (IS_AFFECTED(ch, AFF_GROUP)) {
        sprintf(buf, "%s reports %d%% vitality.\r\n",
        GET_NAME(ch), (int) (GET_HIT(ch) * 100 / GET_MAX_HIT(ch)));
        CAP(buf);

        k = (ch->master ? ch->master : ch);

        for (f = k->followers; f; f = f->next)
           if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower != ch)
        send_to_char(buf, f->follower);
        if (k != ch)
           send_to_char(buf, k);

        */
    }
    /*  if (ch->player_specials->saved.killed_by_player) {
    	temp = (float) (ch->player_specials->saved.killed_player / ch->player_specials->saved.killed_by_player);
    	sprintf(buf, "%s reports:  Incarnation %d.  Player-Kill/Killed Ratio: %4.2f:1", GET_NAME(ch),
    		GET_REINCARN(ch), temp);
    	act(buf, FALSE, ch, 0, 0, TO_ROOM);
    	sprintf(buf, "You have reincarnated %d times.  Your player-kill/killed ratio is: %4.2f:1",
    		GET_REINCARN(ch), temp);
    	act(buf, FALSE, ch, 0, 0, TO_CHAR);
      }
      else {
    	temp = ch->player_specials->saved.killed_player;
    	sprintf(buf, "%s reports:  Incarnation %d.  Player-Kill/Killed Ratio: %d:0", GET_NAME(ch),
    		GET_REINCARN(ch), (int) temp);
    	act(buf, FALSE, ch, 0, 0, TO_ROOM);
    	sprintf(buf, "You have reincarnated %d times.  Your player-kill/killed ratio is: %d:0",
    		GET_REINCARN(ch), (int) temp);
    	act(buf, FALSE, ch, 0, 0, TO_CHAR);
      }
      */
}



ACMD(do_split)
{
    int amount, num, share;
    struct char_data *k;
    struct follow_type *f;

    if (IS_NPC(ch))
        return;

    one_argument(argument, buf);

    if (is_number(buf)) {
        amount = atoi(buf);
        if (amount <= 0) {
            send_to_char("Sorry, you can't do that.\r\n", ch);
            return;
        }
        if (amount > GET_GOLD(ch)) {
            send_to_char("You don't seem to have that much coins to split.\r\n", ch);
            return;
        }
        k = (ch->master ? ch->master : ch);

        if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
            num = 1;
        else
            num = 0;

        for (f = k->followers; f; f = f->next)
            if (IS_AFFECTED(f->follower, AFF_GROUP) &&
                    (!IS_NPC(f->follower)) &&
                    (f->follower->in_room == ch->in_room))
                num++;

        if (num && IS_AFFECTED(ch, AFF_GROUP))
            share = amount / num;
        else {
            send_to_char("With whom do you wish to share your coins?\r\n", ch);
            return;
        }

        GET_GOLD(ch) -= share * (num - 1);

        if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)
                && !(IS_NPC(k)) && k != ch) {
            GET_GOLD(k) += share;
            sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
                    amount, share);
            send_to_char(buf, k);
        }
        for (f = k->followers; f; f = f->next) {
            if (IS_AFFECTED(f->follower, AFF_GROUP) &&
                    (!IS_NPC(f->follower)) &&
                    (f->follower->in_room == ch->in_room) &&
                    f->follower != ch) {
                GET_GOLD(f->follower) += share;
                sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
                        amount, share);
                send_to_char(buf, f->follower);
            }
        }
        if (num > 1) {
            sprintf(buf, "You split %d coins -- %d coins each.\r\n",
                    amount, share);
            send_to_char(buf, ch);
        }
    } else {
        send_to_char("How many coins do you wish to split with your group?\r\n", ch);
        return;
    }
}

struct recite_eo
{
    struct char_data *ch;
    struct obj_data *obj;
    int step, cmd;
    char reventbuf[100];
};

EVENTFUNC(event_recite)
{
    struct recite_eo *revent=(struct recite_eo *) event_obj;
    struct char_data *ch;
    struct obj_data *obj;
    int step, cmd, quaffroll;

    ch=revent->ch;
    obj=revent->obj;
    step=revent->step;
    cmd=revent->cmd;

    if (!ch || !obj || ch->in_room<1)
        goto revent_kraj;

    if (GET_POS(ch) == POS_FIGHTING) {

        /* must fumble for a potion in battle */
        quaffroll = GET_DEX(ch);

        /* must fumble for a potion in battle */
        quaffroll += GET_SKILL(ch, SKILL_GRIP)/12;


        /* must fumble for a potion with offhand */
        if (GET_EQ(ch, WEAR_WIELD))
            quaffroll -= 4;

        /* must fumble for a potion with no free hands */
        if (GET_EQ(ch, WEAR_SHIELD) || GET_EQ(ch, WEAR_DUALWIELD))
            quaffroll -= 4;

        /* easier if already in hand */
        if (GET_EQ(ch, WEAR_HOLD) == obj)
            quaffroll += 5;
    }
    else
        quaffroll=25;


    if (step==0)	// half time, test for lost concentration
    {



        if (cmd==SCMD_QUAFF)
        {
            if (number(1,20) > quaffroll) {
                act("$n is jolted and drops $p!  It shatters!",TRUE, ch, obj, 0, TO_ROOM);
                act("You arm is jolted and $p flies from your hand, *SMASH*",TRUE, ch, obj, 0, TO_CHAR);
                /* potions take some time to get and uncap */
                extract_obj(obj);
            }
            else
            {
                if (GET_SKILL(ch, SKILL_GRIP))
                    improve_skill(ch, SKILL_GRIP, 1);
                mag_objectmagic(ch, obj, revent->reventbuf);
            }

            goto revent_kraj;

        }
        else
        {
            if (number(1,20) > quaffroll) {
                act("$n screams as $p is torn apart!",TRUE, ch, obj, 0, TO_ROOM);
                act("ARGH, $p is teared apart! ",TRUE, ch, obj, 0, TO_CHAR);
                /* potions take some time to get and uncap */
                extract_obj(obj);
                goto revent_kraj;
            }
            else
            {
                revent->step=1;
                send_to_char("You mumble mystical phrases.\r\n", ch);
                WAIT_STATE(ch, 5);
                return 5;
            }
        }

    }
    else if (step==1)
    {

        mag_objectmagic(ch, obj, revent->reventbuf);
    }


revent_kraj:
    if (revent)
        DISPOSE(revent);
    if (ch)
        GET_UTIL_EVENT(ch)=NULL;
    return 0;

}


ACMD(do_use)
{
    struct obj_data *mag_item;
    int equipped = 1;
    struct recite_eo *revent;

    if (GET_UTIL_EVENT(ch))
    {
        switch (subcmd) {
        case SCMD_RECITE:
            send_to_char("You are already trying to do something.\r\n", ch);return;break;
        case SCMD_QUAFF:
            send_to_char("You are already trying to do something.\r\n", ch);return;break;
        default:
            send_to_char("You are already trying to do something.\r\n", ch);return;break;
        }
    }

    half_chop(argument, arg, buf);
    if (!*arg) {
        sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
        send_to_char(buf2, ch);
        return;
    }
    mag_item = GET_EQ(ch, WEAR_HOLD);

    if (!mag_item || !isname(arg, mag_item->name)) {
        switch (subcmd) {
        case SCMD_RECITE:
        case SCMD_QUAFF:
            equipped = 0;
            if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
                sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
                send_to_char(buf2, ch);
                return;
            }
            break;
        case SCMD_USE:
            sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
            send_to_char(buf2, ch);
            return;
            break;
        default:
            log("SYSERR: Unknown subcmd passed to do_use");
            return;
            break;
        }
    }
    switch (subcmd) {
    case SCMD_QUAFF:
        if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
            send_to_char("You can only quaff potions.", ch);
            return;
        }
        break;
    case SCMD_RECITE:
        if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
            send_to_char("You can only recite scrolls.", ch);
            return;
        }
        break;
    case SCMD_USE:
        if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
                (GET_OBJ_TYPE(mag_item) != ITEM_STAFF)) {
            send_to_char("You can't seem to figure out how to use it.\r\n", ch);
            return;
        }
        break;
    }
    if (subcmd==SCMD_QUAFF || subcmd==SCMD_RECITE)
    {

        CREATE(revent, struct recite_eo, 1);
        revent->ch=ch;
        revent->obj=mag_item;
        revent->cmd=subcmd;
        revent->step=0;
        strncpy(revent->reventbuf, buf, 100);
        if (subcmd==SCMD_QUAFF)
        {
            act("You reach out for $p.", FALSE, ch, mag_item,0, TO_CHAR);
            act("$n reaches out for $p.", FALSE, ch, mag_item,0, TO_ROOM);
        }
        else
        {
            act("You start reciting $p.", FALSE, ch, mag_item,0, TO_CHAR);
            act("$n starts reciting $p.", FALSE, ch, mag_item,0, TO_ROOM);
        }
        GET_UTIL_EVENT(ch)=event_create(event_recite, revent, 5);    // 5 = 1 sec
        WAIT_STATE(ch, 5);
        oprog_use_trigger( ch, mag_item, NULL, NULL, NULL );
    }
    else
        mag_objectmagic(ch, mag_item, buf);
}


ACMD(do_wimpy)
{
    int wimp_lev;

    one_argument(argument, arg);

    if (!*arg) {
        if (GET_WIMP_LEV(ch)) {
            sprintf(buf, "Your current wimp level is %d hit points.\r\n",
                    GET_WIMP_LEV(ch));
            send_to_char(buf, ch);
            return;
        } else {
            send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
            return;
        }
    }
    if (isdigit(*arg)) {
        if ((wimp_lev = atoi(arg))) {
            if (wimp_lev < 0)
                send_to_char("Heh, heh, heh.. we are jolly funny today, eh?\r\n", ch);
            else if (wimp_lev > GET_MAX_HIT(ch))
                send_to_char("That doesn't make much sense, now does it?\r\n", ch);
            else if (wimp_lev < MOVE_LIMIT(ch))
                send_to_char("You can't set your wimp level so low, you wouldn't be able to flee in that condition.\r\n", ch);
            else {
                sprintf(buf, "Okay, you'll wimp out if you drop below %d hit points.\r\n",
                        wimp_lev);
                send_to_char(buf, ch);
                GET_WIMP_LEV(ch) = wimp_lev;
            }
        } else {
            send_to_char("Okay, you'll now tough out fights to the bitter end.\r\n", ch);
            GET_WIMP_LEV(ch) = 0;
        }
    } else
        send_to_char("Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n", ch);

    return;

}

ACMD(do_display)
{
    int i;

    if (IS_NPC(ch))
        return;
    one_argument(argument, arg);
    i = atoi(arg);

    switch (i) {
    case 0:
        if (GET_TERM(ch) == 0) {
            send_to_char("Display unchanged.\r\n", ch);
            return;
        }
        GET_TERM(ch) = 0;
        termScreenOff(ch);
        send_to_char("Display now turned off.\r\n", ch);
        return;

    case 1:
        if (GET_TERM(ch) == VT100) {
            send_to_char("Display unchanged.\r\n", ch);
            return;
        }
        GET_TERM(ch) = VT100;
        termScreenOff(ch);
        InitScreen(ch);
        send_to_char("Display now set to VT100.\r\n", ch);
        return;

    default:
        if (GET_TERM(ch) == VT100) {
            send_to_char("Term type is currently VT100.\r\n", ch);
            return;
        }
        send_to_char("Display is currently OFF.\r\n", ch);
        return;
    }
}

void termScreenOff(struct char_data * ch)
{
    char buf[255];

    sprintf(buf, VT_MARGSET, 0, GET_TERM_SIZE(ch));
    send_to_char(buf, ch);
    send_to_char(VT_HOMECLR, ch);
}

ACMD(do_resize)
{
    int i;


    if (IS_NPC(ch))
        return;
    one_argument(argument, arg);
    if (!*arg)
    {
        char buf[100];
        sprintf(buf,"Your current screen size is set to %d.\r\n", GET_TERM_SIZE(ch));
        send_to_char(buf,ch);
        return;
    }
    i = atoi(arg);

    if (i < 7) {
        send_to_char("Screen size must be greater than 7.\r\n", ch);
        return;
    }
    if (i > 50) {
        send_to_char("Size must be smaller than 50.\r\n", ch);
        return;
    }
    GET_TERM_SIZE(ch) = i;

    if (GET_TERM(ch) == VT100) {
        termScreenOff(ch);
        InitScreen(ch);
    }
    send_to_char("Ok.\r\n", ch);
    return;
}

ACMD(do_prompt)
{

    char *pom=ch->player_specials->saved.prompt;
    char *pro;
    char *deff;

    int i;

    if (ch->desc == NULL) {
        send_to_char("Mosters don't need prompts.  Go away.\r\n", ch);
        return;
    }
    if (!*argument) {
        send_to_char("See help prompt for usage. See 'toggle' for current prompt settings.\r\n", ch);
        return;
    }


    skip_spaces(&argument);

    pro=argument;
    deff=DEFAULT_PROMPT;


    if (!strncmp(argument, "combat", 5))
    {
        pom=ch->player_specials->saved.prompt_combat;
        pro=argument+6;
        deff=DEFAULT_COMBAT_PROMPT;
    };

    skip_spaces(&pro);




    if (!str_cmp(pro, "default"))
        strcpy(pom, deff);
    else
    {
        if (strlen(pro)>99)
        {
            send_to_char("Text too long.\r\n", ch);
            return;
        }
        strcpy(pom, pro);
    }



    send_to_char(OK, ch);
}

ACMD(do_walkset)
{

    char *pom=ch->player_specials->saved.walkin;
    char *s;

    int i;

    if (ch->desc == NULL) {
        send_to_char("Go away monster.\r\n", ch);
        return;
    }

    if (!*argument) {
        send_to_char("See 'help walkset' for usage. See 'toggle' for current settings.\r\n", ch);
        return;
    }

    skip_spaces(&argument);

    if (subcmd==SCMD_WALKOUT)
        pom=ch->player_specials->saved.walkout;

    if (strlen(argument)>79)
    {
        send_to_char("Text too long.\r\n", ch);
        return;
    }
    if (strlen(argument)<6)
    {
        send_to_char("Text too short.\r\n", ch);
        return;
    }
    if (!(s=strchr(argument, '#')) && subcmd==SCMD_WALKOUT)
    {
        send_to_char("You must include '#' somewhere in the string ie. 'walkin stomps in from the #'\r\n", ch);
        return;
    }


    strcpy(buf, argument);

    s=buf+strlen(buf)-1;

    if (*s!='.' || *s!='!')
        strcat(buf, ".");

    strcpy(pom, buf);


    send_to_char(OK, ch);
}


ACMD(do_gen_write)
{
    FILE *fl;
    char *tmp, *filename;
    struct stat fbuf;
    extern int max_filesize;
    time_t ct;

    switch (subcmd) {
    case SCMD_BUG:
        filename = BUG_FILE;
        break;
    case SCMD_TYPO:
        filename = TYPO_FILE;
        break;
    case SCMD_IDEA:
        filename = IDEA_FILE;
        break;
    default:
        return;
    }

    ct = time(0);
    tmp = asctime(localtime(&ct));

    if (IS_NPC(ch)) {
        send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
        return;
    }
    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (!*argument) {
        send_to_char("So, what did you want to write?\r\n", ch);
        return;
    }
    if (strlen(argument)<5)
    {
        send_to_char("Please be more descriptive.\r\n", ch);
        return;
    }
    sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
    mudlog(buf, CMP, GET_LEVEL(ch), FALSE);

    if (stat(filename, &fbuf) < 0) {
        perror("Error statting file");
        return;
    }
    if (fbuf.st_size >= max_filesize) {
        send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
        return;
    }
    if (!(fl = fopen(filename, "a"))) {
        perror("do_gen_write");
        send_to_char("Could not open the file.  Sorry.\r\n", ch);
        return;
    }
    fprintf(fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
            world[ch->in_room].number, argument);
    fclose(fl);
    send_to_char("Okay.  Thanks!\r\n", ch);
}



#define TOG_OFF 0
#define TOG_ON  1

#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

ACMD(do_gen_tog)
{
    long result;
    extern int nameserver_is_slow;

    char *tog_messages[][2] = {
                                  {"You are now safe from summoning by other players.\r\n",
                                   "You may now be summoned by other players.\r\n"},
                                  {"Nohassle disabled.\r\n",
                                   "Nohassle enabled.\r\n"},
                                  {"Brief mode off.\r\n",
                                   "Brief mode on.\r\n"},
                                  {"Compact mode off.\r\n",
                                   "Compact mode on.\r\n"},
                                  {"You can now hear tells.\r\n",
                                   "You are now deaf to tells.\r\n"},
                                  {"You can now hear auctions.\r\n",
                                   "You are now deaf to auctions.\r\n"},
                                  {"You can now hear shouts.\r\n",
                                   "You are now deaf to shouts.\r\n"},
                                  {"You can now hear global gossip.\r\n",
                                   "You are now deaf to global gossip.\r\n"},
                                  {"You can now hear the congratulation messages.\r\n",
                                   "You are now deaf to the congratulation messages.\r\n"},
                                  {"You can now hear the Wiz-channel.\r\n",
                                   "You are now deaf to the Wiz-channel.\r\n"},
                                  {"You are now deaf to quest-channel.\r\n",
                                   "You can now hear quest-channel.\r\n"},
                                  {"You will no longer see the room flags.\r\n",
                                   "You will now see the room flags.\r\n"},
                                  {"You will now have your communication repeated.\r\n",
                                   "You will no longer have your communication repeated.\r\n"},
                                  {"HolyLight mode off.\r\n",
                                   "HolyLight mode on.\r\n"},
                                  {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
                                   "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
                                  {"Autoexits disabled.\r\n",
                                   "Autoexits enabled.\r\n"},
                                  {"You are no longer protected from accidentally quiting.\r\n",
                                   "You are now protected from accidentally quiting.\r\n"},
                                  {"Autodir disabled.\r\n",
                                   "Autodir enabled.\r\n"},
                                  {"Autosac off.\r\n",
                                   "Autosac on.\r\n"},
                                  {"Autotitle enabled.  Title will change on leveling.\r\n",
                                   "Autotitle disabled.  Title will not change on leveling.\r\n"},
                                  {"You can now hear encoded messages.\r\n",
                                   "You are now deaf to encoded messages.\r\n"},
                                  {"Yay!  You're back!  You don't know what you missed while you were gone.\r\n",
                                   "Okay, love you bye bye!  Come back to the keyboard soon, 'cuz we'll miss ya!\r\n"},
                                  {"You can now hear Out Of Character channel.\r\n",
                                   "You are now deaf to Out Of Character channel.\r\n"},
                                  {"You can now hear arena broadcasts.\r\n",
                                   "You are now deaf to arena broadcasts.\r\n"},
                                  {"You can now hear inner-clan discussions.\r\n",
                                   "You are now deaf inner-clan discussions.\r\n"},
                                  {"Autoloot disabled.\r\n",
                                   "Autoloot enabled.\r\n"},
                                  {"You can now hear information channel.\r\n",
                                   "You are now deaf to information channel.\r\n"},
                                  {"Autosplit disabled.\r\n",
                                   "Autosplit enabled.\r\n"},
                                  {"Autograt disabled.\r\n",
                                   "Autograt enabled.\r\n"},
                                  {"Autoassist disabled.\r\n",
                                   "Autoassist enabled.\r\n"},
                                  {"Alert summon disabled.\r\n",
                                   "Alert summon enabled.\r\n"},
                                  {"Nowho disabled..\r\n",
                                   "Nowho enabled.\r\n"},
                                  {"Autoscan disabled.\r\n",
                                   "Autoscan enabled.\r\n"},
                                  {"Display numerical damage disabled.\r\n",
                                   "Display numerical damage is now enabled.\r\n"},
                                  {"Automap disabled.\r\n",
                                   "Automap enabled.\r\n"},
                                  {"Ident changed to NO;  remote username lookups will not be attempted.\r\n",
                                   "Ident changed to YES;  remote usernames lookups will be attempted.\r\n"},
                                  {"Autosave off.\r\n",
                                   "Autosave on.\r\n"},
                                  {"Display skill descriptions is now enabled.\r\n",
                                   "Display skill descriptions disabled.\r\n"}
                              };

    if (IS_NPC(ch))
        return;

    switch (subcmd) {
    case SCMD_NOSUMMON:
        result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
        break;
    case SCMD_NOHASSLE:
        result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
        break;
    case SCMD_BRIEF:
        result = PRF_TOG_CHK(ch, PRF_BRIEF);
        break;
    case SCMD_COMPACT:
        result = PRF_TOG_CHK(ch, PRF_COMPACT);
        break;
    case SCMD_NOTELL:
        result = PRF_TOG_CHK(ch, PRF_NOTELL);
        break;
    case SCMD_NOAUCTION:
        result = PRF_TOG_CHK(ch, PRF_NOAUCT);
        break;
    case SCMD_DEAF:
        result = PRF_TOG_CHK(ch, PRF_DEAF);
        break;
    case SCMD_NOGOSSIP:
        result = PRF_TOG_CHK(ch, PRF_NOGOSS);
        break;
    case SCMD_NOGRATZ:
        result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
        break;
    case SCMD_NOWIZ:
        result = PRF_TOG_CHK(ch, PRF_NOWIZ);
        break;
    case SCMD_QUEST:
        result = PRF_TOG_CHK(ch, PRF_QUEST);
        break;
    case SCMD_ROOMFLAGS:
        result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
        break;
    case SCMD_NOREPEAT:
        result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
        break;
    case SCMD_HOLYLIGHT:
        result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
        break;
    case SCMD_SLOWNS:
        result = (nameserver_is_slow = !nameserver_is_slow);
        break;
    case SCMD_AUTOEXIT:
        result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
        break;
    case SCMD_NOQUIT:
        result = PRF2_TOG_CHK(ch, PRF2_NOQUIT);
        break;
    case SCMD_AUTODIR:
        result = PRF_TOG_CHK(ch, PRF_AUTODIR);
        break;
    case SCMD_AUTOSAC:
        result = PRF_TOG_CHK(ch, PRF_AUTOSAC);
        break;
    case SCMD_AUTOTITLE:
        result = PLR_TOG_CHK(ch, PLR_NOSETTITLE);
        break;
    case SCMD_NOWAR:
        result = PRF_TOG_CHK(ch, PRF_NOWAR);
        break;
    case SCMD_AFK:
        result = PRF2_TOG_CHK(ch, PRF2_AFK);
        break;
    case SCMD_NOCHAT:
        result = PRF_TOG_CHK(ch, PRF_NOCHAT);
        break;
    case SCMD_NOTEAM:
        result = PRF_TOG_CHK(ch, PRF_NOTEAM);
        break;
    case SCMD_NOCLAN:
        result = PRF_TOG_CHK(ch, PRF_NOCLAN);
        break;
    case SCMD_AUTOLOOT:
        result = PRF2_TOG_CHK(ch, PRF2_AUTOLOOT);
        break;
    case SCMD_NOINFO:
        result = PRF2_TOG_CHK(ch, PRF2_NOINFO);
        break;
    case SCMD_AUTOSPLIT:
        result = PRF2_TOG_CHK(ch, PRF2_AUTOSPLIT);
        break;
    case SCMD_AUTOGRAT:
        result = PRF2_TOG_CHK(ch, PRF2_AUTOGRAT);
        break;
    case SCMD_AUTOASSIST:
        result = PRF2_TOG_CHK(ch, PRF2_AUTOASSIST);
        break;
    case SCMD_NOALERT:
        result = PRF2_TOG_CHK(ch, PRF2_NOALERT);
        break;
    case SCMD_NOWHO:
        result = PRF2_TOG_CHK(ch, PRF2_NOWHO);
        break;
    case SCMD_AUTOSCAN:
        result = PRF2_TOG_CHK(ch, PRF2_AUTOSCAN);
        break;
    case SCMD_DISPDAM:
        result=PRF2_TOG_CHK(ch, PRF2_DISPDAM);
        break;
    case SCMD_AUTOMAP:
        result = PRF2_TOG_CHK(ch, PRF2_AUTOMAP);
        break;         
    case SCMD_IDENT:
        result = (ident = !ident);
        break;
    case SCMD_AUTOSAVE:
        result = PRF_TOG_CHK(ch, PRF_AUTOSAVE);
        break;
    
     case SCMD_DISPSDESC:
        result=PRF2_TOG_CHK(ch, PRF2_DISPSDESC);
        break;    
    default:
        log("SYSERR: Unknown subcmd in do_gen_toggle");
        return;
        break;
    }

    if (result)
        send_to_char(tog_messages[subcmd][TOG_ON], ch);
    else
        send_to_char(tog_messages[subcmd][TOG_OFF], ch);

    return;
}


ACMD(do_channel)
{
    int channel = 0;
    int cmd1 = -1;
    one_argument(argument, buf);

    if (!*buf) {
        send_to_char("Usage: channel all/none/specific_channel.\r\nSee 'toggle' for channels in use.\r\n", ch);
        return;
    }
    if (isname(buf, "all")) {
        REMOVE_BIT(PRF_FLAGS(ch), PRF_NOTEAM | PRF_NOAUCT | PRF_DEAF | PRF_NOGOSS | PRF_NOGRATZ | PRF_NOCHAT | PRF_NOTELL);
        REMOVE_BIT(PRF2_FLAGS(ch), PRF2_NOINFO);
        //SET_BIT(PRF_FLAGS(ch), PRF_QUEST);
        send_to_char("Turning ON all channels...\r\n", ch);
        return;
    }
    if (isname(buf, "none")) {
        SET_BIT(PRF_FLAGS(ch), PRF_NOTEAM | PRF_NOAUCT | PRF_DEAF | PRF_NOGOSS | PRF_NOGRATZ | PRF_NOCHAT | PRF_NOTELL);
        SET_BIT(PRF2_FLAGS(ch), PRF2_NOINFO);
        //REMOVE_BIT(PRF_FLAGS(ch), PRF_QUEST);
        send_to_char("Turning OFF all channels...\r\n", ch);
        return;
    }
    if (isname(buf, "auction"))
        cmd1 = SCMD_NOAUCTION;
    else if (isname(buf, "tell"))
        cmd1 = SCMD_NOTELL;
    else if (isname(buf, "info"))
        cmd1 = SCMD_NOINFO;
    else if (isname(buf, "shout"))
        cmd1 = SCMD_DEAF;
    else if (isname(buf, "gossip"))
        cmd1 = SCMD_NOGOSSIP;
    else if (isname(buf, "grats"))
        cmd1 = SCMD_NOGRATZ;
    else if (isname(buf, "arena"))
        cmd1 = SCMD_NOTEAM;
    else if (isname(buf, "quest"))
        cmd1 = SCMD_QUEST;
    else if (isname(buf, "ooc"))
        cmd1 = SCMD_NOCHAT;
    if (cmd1!=-1)
        do_gen_tog(ch, 0, 0, cmd1);
    return;
}



void auction_update(void)
{
    if (auction.ticks == AUC_NONE)
        return;
    if (!(auction.seller)) {
        if (auction.obj)
            extract_obj(auction.obj);
        auction_reset();
        return;
    }
    if (auction.ticks >= AUC_BID && auction.ticks <= AUC_TWICE) {
        if (auction.bidder)
            sprintf(buf2, "%s for %ld coin%sto %s.%s%s",
                    auction.obj->short_description, auction.bid,
                    auction.bid != 1 ? "s " : " ",
                    auction.bidder->player.name,
                    auction.ticks == AUC_ONCE ? " ONCE!" : "",
                    auction.ticks == AUC_TWICE ? " &GTWICE!!&0" : "");
        else if (auction.ticks == AUC_TWICE) {
            sprintf(buf2, "%s withdrawn, no interest.",
                    auction.obj->short_description);
            if (auction.seller) obj_to_char(auction.obj, (auction.seller));
            auction_reset();
            auction.ticks--;

        } else
            sprintf(buf2, "%s, %ld coin%s minimum.",
                    auction.obj->short_description, auction.bid, auction.bid != 1 ?
                    "s " : " ");
        AUC_OUT(buf2);
        auction.ticks++;
        return;
    }
    if (auction.ticks == AUC_SOLD) {
        sprintf(buf2, "%s &CSOLD&0 to %s for %ld coin%s",
                auction.obj->short_description, auction.bidder->player.name,
                auction.bid, auction.bid != 1 ? "s." : "!");
        AUC_OUT(buf2);

        if (auction.seller) {
            GET_BANK_GOLD(auction.seller) += auction.bid;
            act("You have sold $p.", FALSE,
                auction.seller, auction.obj, 0, TO_CHAR);
        }
        if (auction.bidder) {
            GET_BANK_GOLD(auction.bidder) -= auction.bid;
            act("You bought $p.", FALSE,
                auction.bidder, auction.obj, 0, TO_CHAR);
            if (!can_take_obj(auction.bidder, auction.obj)){
                send_to_char("Small demon pops in and drops it in front of you.\r\n", auction.bidder);
                obj_to_room(auction.obj, auction.bidder->in_room);
            }
            else
            {
                send_to_char("Small demon pops in and gives it to you.\r\n", auction.bidder);
                obj_to_char(auction.obj, auction.bidder);
            }
        }
        auction_reset();
        return;
    }
}

ACMD(do_bid)
{
    long bid;
    if (ch->master && AFF_FLAGGED(ch, AFF_CHARM))
        return;
    if (not_in_arena(ch))
        return;


    if (auction.ticks == AUC_NONE) {
        send_to_char("Nothing is up for sale.\r\n", ch);
        return;
    }
    one_argument(argument, buf);
    bid = atoi(buf);
    if (isname("more", buf))
        bid=auction.bid*1.15 +1;

    if (!*buf) {
        sprintf(buf2, "Current bid: %ld coin%s\r\n", auction.bid,
                auction.bid != 1 ? "s." : ".");
        send_to_char(buf2, ch);
    } else if (ch == auction.bidder)
        send_to_char("You're trying to outbid yourself.\r\n", ch);
    else if (ch->in_room == real_room(DEATHR))
        send_to_char("You can not auction from here.\r\n", ch);
    else if (ch == auction.seller)
        send_to_char("You can't bid on your own item.\r\n", ch);
    else if (bid < (auction.bid * 1.05) || bid == 0) {
        sprintf(buf2, "Try bidding at least 5%% over the current bid of %ld (%.0f coins) or type 'bid more'.\r\n",
                auction.bid, auction.bid * 1.05 + 1);
        send_to_char(buf2, ch);
    } else if (GET_BANK_GOLD(ch) < bid) {
        sprintf(buf2, "You have only %d coins in bank.\r\n", GET_BANK_GOLD(ch));
        send_to_char(buf2, ch);
    } else if (PRF_FLAGGED(ch, PRF_NOAUCT))
        send_to_char("You can't auction. Type noauct to toggle.\r\n", ch);
    else {
        auction.bid = bid;
        auction.bidder = ch;
        auction.ticks = AUC_BID;
        sprintf(buf2, "%ld coin", bid);
        if (auction.bid != 1)
            strcat(buf2, "s.");
        else
            strcat(buf2, ".");
        AUC_OUT(buf2);
    }
}

ACMD(do_auction)
{
    struct obj_data *obj;

    struct char_data *seller;


    if (ch->master && AFF_FLAGGED(ch, AFF_CHARM))
        return;

    if (not_in_arena(ch))
        return;


    two_arguments(argument, buf1, buf2);

    seller = (auction.seller);

    if (!str_cmp(buf1, "stop") && (GET_LEVEL(ch) == LVL_IMPL) && (auction.ticks != AUC_NONE)) {
        sprintf(buf2, "Sale of %s stoped by Implementor. Item confiscated!",
                auction.obj->short_description);
        obj_to_char(auction.obj, ch);
        AUC_OUT(buf2);
        auction_reset();
    } else {
        if (!*buf1)
            send_to_char("Auction usage:  auction <object> <cost>.\r\n", ch);
        else if (auction.ticks != AUC_NONE) {
            if (seller) {
                sprintf(buf2, "%s is currently auctioning %s for %ld coins.\r\n",
                        auction.seller->player.name, auction.obj->short_description,
                        auction.bid);
                send_to_char(buf2, ch);
            } else {
                sprintf(buf2, "No one is currently auctioning %s for %ld coins.\r\n",
                        auction.obj->short_description, auction.bid);
                send_to_char(buf2, ch);
            }
        } else if ((obj = get_obj_in_list_vis(ch, buf1, ch->carrying)) == NULL)
            send_to_char("You don't seem to have that to sell.\r\n", ch);
        // Can not auction corpses because they may decompose
        else if ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && (GET_OBJ_VAL(obj, 3)))
            send_to_char("You can't auction corpses.\r\n", ch);
        else if (IS_OBJ_STAT(obj, ITEM_NODROP))
            act("You can't auction $p, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
        else {
            auction.ticks = AUC_BID;
            auction.seller = ch;
            auction.bid = (atoi(buf2) > 0 ? (atoi(buf2)>GET_OBJ_COST(obj)?atoi(buf2):GET_OBJ_COST(obj)) : MAX(1, GET_OBJ_COST(obj)));
            auction.obj = obj;
            obj_from_char(auction.obj);
            auction.spec = auction.bid;
            sprintf(buf2, "%s, minimum %ld coin%s", auction.obj->short_description,
                    auction.bid, auction.bid != 1 ? "s." : ".");
            AUC_OUT(buf2);
        }
    }
}

void auction_reset(void)
{
    auction.bidder = NULL;
    auction.seller = NULL;
    auction.obj = NULL;
    auction.ticks = AUC_NONE;
    auction.bid = 0;
    auction.spec = 0;
}


struct char_data *get_ch_by_id(long idnum)
{
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next)
        if (d && d->character && GET_IDNUM(d->character) == idnum)
            return (d->character);

    return NULL;
}

/* If you want to do object sanity checking at time of sale, here's what you
 * can do, or at least one way.  Make a function that returns an object
 * pointer when given someone and something about the object.  Loop through
 * his inventory, comparing the passed argument to the appropriate place
 * in every inventory object, this results in a more accurate match, then
 * return that object, otherwise null.  I removed this due to the fact that
 * A) some people would rather throw checks all over the place to simply
 * prevent the object from moving and B) it'd make it too easy ;)
 */


struct drunk_struct {
    int min_drunk_level;
    int number_of_rep;
    char *replacement[11];
};

char *makedrunk(char *string, struct char_data * ch);

/* How to make a string look drunk... by Apex (robink@htsa.hva.nl) */
/* Modified and enhanced for envy(2) by the Maniac from Mythran    */
/* Ported to Stock Circle 3.0 by Haddixx (haddixx@megamed.com)     */

char *makedrunk(char *string, struct char_data * ch)
{

    /* This structure defines all changes for a character */
    struct drunk_struct drunk[] =
        {
            {3, 10,
                {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
            {8, 5,
             {"b", "b", "b", "B", "B", "vb"}},
            {3, 5,
             {"c", "c", "C", "cj", "sj", "zj"}},
            {5, 2,
             {"d", "d", "D"}},
            {3, 3,
             {"e", "e", "eh", "E"}},
            {4, 5,
             {"f", "f", "ff", "fff", "fFf", "F"}},
            {8, 2,
             {"g", "g", "G"}},
            {9, 6,
             {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
            {7, 6,
             {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
            {9, 5,
             {"j", "j", "jj", "Jj", "jJ", "J"}},
            {7, 2,
             {"k", "k", "K"}},
            {3, 2,
             {"l", "l", "L"}},
            {5, 8,
             {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
            {6, 6,
             {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
            {3, 6,
             {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
            {3, 2,
             {"p", "p", "P"}},
            {5, 5,
             {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
            {4, 2,
             {"r", "r", "R"}},
            {2, 5,
             {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
            {5, 2,
             {"t", "t", "T"}},
            {3, 6,
             {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
            {4, 2,
             {"v", "v", "V"}},
            {4, 2,
             {"w", "w", "W"}},
            {5, 6,
             {"x", "x", "X", "ks", "iks", "kz", "xz"}},
            {3, 2,
             {"y", "y", "Y"}},
            {2, 9,
             {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
        };
    char buf[2000];
    char temp;
    int pos = 0;
    int randomnum;
    char debug[256];

    if (GET_COND(ch, DRUNK) > 0) {	/* character is drunk */
        do {
            temp = toupper(*string);
            if ((temp >= 'A') && (temp <= 'Z')) {
                if (GET_COND(ch, DRUNK) > drunk[(temp - 'A')].min_drunk_level) {
                    randomnum = number(0, (drunk[(temp - 'A')].number_of_rep));
                    strcpy(&buf[pos], drunk[(temp - 'A')].replacement[randomnum]);
                    pos += strlen(drunk[(temp - 'A')].replacement[randomnum]);
                } else
                    buf[pos++] = *string;
            } else {
                if ((temp >= '0') && (temp <= '9')) {
                    temp = '0' + number(0, 9);
                    buf[pos++] = temp;
                } else
                    buf[pos++] = *string;
            }
        } while (*string++);

        buf[pos] = '\0';	/* Mark end of the string... */
        strcpy(string, buf);
        return (string);
    }
    return (string);		/* character is not drunk, just return the
       string */
}

ACMD(do_enrage)
{
    skip_spaces(&argument);
    if (!(*argument)) {
        send_to_char("Usage: warcry <your_enrage_text> or enrage off.\r\n", ch);
        return;
    }
    if (!strcmp("off", argument)) {
        strcpy(ch->player_specials->saved.enrage, "off");
        send_to_char("Warcry now off.\r\n", ch);
    } else {
        if (strlen(argument)>39)
        {
            send_to_char("Text too long.\r\n", ch);
            return;
        }
        strcpy(ch->player_specials->saved.enrage, argument);
        sprintf(buf, "Ok. This will be seen when you engage in combat:\r\n%s screams '%s' and attacks!\r\n", GET_NAME(ch), (argument));
        send_to_char(buf, ch);
    }
    return;
}

ACMD(do_grats)
{
    char bufm[500];
    skip_spaces(&argument);
    if (!(*argument)) {
        sprintf(buf, "Current: %s\r\ngrats <msg> to change.\r\n", ch->player_specials->saved.grats);
        send_to_char(buf, ch);
        return;
    }
    if (strlen(argument)>49)
    {
        send_to_char("Text too long.\r\n", ch);
        return;
    }
    strcpy(ch->player_specials->saved.grats, argument);
    sprintf(buf, "Ok. This will be seen when you congratulate:\r\n[Grats from %s]:  %s\r\n", GET_NAME(ch), ch->player_specials->saved.grats);
    send_to_char(buf, ch);
    return;
}

ACMD(do_ckscore)
{
    int i;
    sprintf(buf,  "CLAN Killing Score:\r\n=====================\r\n\r\n\r\nName            Kills     Score\r\n");
    sprintf(buf,"%s-------------------------------\r\n", buf);
    for (i = 1; i<4; i++) {
        sprintf(buf, "%s%-15s %-6ld %8d\r\n", buf, clan[i].name,clan[i].kills, clan[i].score);
    }
    send_to_char(buf, ch);
}



ACMD(do_mkscore)
{
    struct descriptor_data *d, *next_d;
    sprintf(buf,  "MOB Killing Score:\r\n=====================\r\n\r\n\r\nName            Kills     Score\r\n");
    sprintf(buf,"%s-------------------------------\r\n", buf);
    for (d = descriptor_list; d; d = next_d) {
        next_d = d->next;
        if (d && d->character && STATE(d)==CON_PLAYING && CAN_SEE(ch,d->character))
            sprintf(buf, "%s%-15s %-6ld %8d\r\n", buf, GET_NAME(d->character), d->character->player_specials->saved.killed_mob, d->character->player_specials->saved.mkscore);
    }
    send_to_char(buf, ch);
}

ACMD(do_pkscore)
{
    struct descriptor_data *d, *next_d;

    sprintf(buf,  "PLAYER Killing Score:\r\n=====================\r\n\r\n\r\nName            Kills     Score\r\n");
    sprintf(buf,"%s-------------------------------\r\n", buf);
    for (d = descriptor_list; d; d = next_d) {
        next_d = d->next;
        if (d && d->character && STATE(d)==CON_PLAYING && CAN_SEE(ch,d->character))
            sprintf(buf, "%s%-15s %-6ld %8d\r\n", buf, GET_NAME(d->character), d->character->player_specials->saved.killed_player, d->character->player_specials->saved.pkscore);
    }
    send_to_char(buf, ch);
}



char *get_spell_name(char *argument)
{
    char *s;

    s = strtok(argument, "'");
    s = strtok(NULL, "'");

    return s;
}


char *potion_names[] = {
                           "milky white",
                           "bubbling white",
                           "glowing ivory",
                           "glowing blue",
                           "bubbling yellow",
                           "light green",
                           "gritty brown",
                           "blood red",
                           "swirling purple",
                           "flickering green",
                           "cloudy blue",
                           "glowing red",
                           "sparkling white",
                           "incandescent blue"
                       };

void make_potion(struct char_data * ch, int potion, struct obj_data * container)
{
    struct obj_data *final_potion;
    struct extra_descr_data *new_descr;
    int can_make = TRUE, mana, dam, num = 0;

    /* Modify this list to suit which spells you want to be able to mix. */
    switch (potion) {
    case SPELL_CURE_BLIND:
        num = 0;
        break;

    case SPELL_DETECT_ALIGN:
        num = 1;
        break;

    case SPELL_BLESS:
        num = 2;
        break;

    case SPELL_DETECT_MAGIC:
        num = 3;
        break;

    case SPELL_DETECT_INVIS:
        num = 4;
        break;

    case SPELL_DETECT_POISON:
        num = 5;
        break;

    case SPELL_REMOVE_POISON:
        num = 6;
        break;

    case SPELL_PROT_LIGHTNING:
        num = 7;
        break;

    case SPELL_CURE_DRUNK:
        num = 8;
        break;

    case SPELL_PROT_COLD:
        num = 9;
        break;

    case SPELL_CURE_LIGHT:
        num = 10;
        break;

    case SPELL_PROT_FIRE:
        num = 11;
        break;

    case SPELL_CREATE_WATER:
        num = 12;
        break;

    case SPELL_SANCTUARY:
        num = 13;
        break;

    default:
        can_make = FALSE;
        break;
    }

    if (can_make == FALSE) {
        send_to_char("That spell cannot be mixed into a"
                     " potion.\r\n", ch);
        return;
    } else if (GET_SKILL(ch, SKILL_BREW) + GET_LEVEL(ch) < number(1, 150)) {
        send_to_char("As you begin mixing the potion, it violently"
                     " explodes!\r\n", ch);
        act("$n begins to mix a potion, but it suddenly explodes!",
            FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(container);
        dam = number(15, mag_manacost(ch, potion) * 2);
        GET_HIT(ch) = MAX(4, GET_HIT(ch)-dam);
        update_pos(ch);
        return;
    }
    /* requires x3 mana to mix a potion than the spell */
    mana = mag_manacost(ch, potion) * 3;
    if (GET_MANA(ch) - mana > 0) {
        if (GET_LEVEL(ch) < LVL_IMMORT)
            GET_MANA(ch) -= mana;
        sprintf(buf, "You play a little with the spell and finnaly create a %s potion.\r\n",
                spells[potion]);
        send_to_char(buf, ch);
        act("$n creates a potion!", FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(container);
    } else {
        send_to_char("You don't have enough mana to mix"
                     " that potion!\r\n", ch);
        return;
    }

    final_potion = create_obj();

    final_potion->item_number = NOTHING;
    final_potion->in_room = NOWHERE;
    sprintf(buf2, "%s %s potion", potion_names[num], spells[potion]);
    final_potion->name = str_dup(buf2);

    sprintf(buf2, "A %s potion lies here.", potion_names[num]);
    final_potion->description = str_dup(buf2);

    sprintf(buf2, "a %s potion", potion_names[num]);
    final_potion->short_description = str_dup(buf2);

    /* extra description coolness! */
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(final_potion->name);
    sprintf(buf2, "It appears to be a %s potion.", spells[potion]);
    new_descr->description = str_dup(buf2);
    new_descr->next = NULL;
    final_potion->ex_description = new_descr;

    GET_OBJ_TYPE(final_potion) = ITEM_POTION;
    GET_OBJ_WEAR(final_potion) = ITEM_WEAR_TAKE;
    GET_OBJ_EXTRA(final_potion) = ITEM_NORENT;
    GET_OBJ_VAL(final_potion, 0) = GET_LEVEL(ch)-5;
    GET_OBJ_VAL(final_potion, 1) = potion;
    GET_OBJ_VAL(final_potion, 2) = -1;
    GET_OBJ_VAL(final_potion, 3) = -1;
    GET_OBJ_COST(final_potion) = GET_LEVEL(ch) * 100;
    GET_OBJ_WEIGHT(final_potion) = 1;
    GET_OBJ_RENT(final_potion) = 0;

    obj_to_char(final_potion, ch);
}

ACMD(do_brew)
{
    struct obj_data *container = NULL;
    struct obj_data *obj, *next_obj;
    char bottle_name[MAX_STRING_LENGTH];
    char spell_name[MAX_STRING_LENGTH];
    char *temp1, *temp2;
    int potion, found = FALSE;

    temp1 = one_argument(argument, bottle_name);

    /* sanity check */
    if (temp1) {
        temp2 = get_spell_name(temp1);
        if (temp2)
            strcpy(spell_name, temp2);
    } else {
        bottle_name[0] = '\0';
        spell_name[0] = '\0';
    }


    if (!GET_SKILL(ch, SKILL_BREW)) {
        send_to_char("You have no idea how to mix potions!\r\n", ch);
        return;
    }
    if (!*bottle_name || !*spell_name) {
        send_to_char("Brew <drink_container> <'spell'>.\r\n", ch);
        return;
    }
    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(container = get_obj_in_list_vis(ch, bottle_name,
                               ch->carrying)))
            continue;
        else
            found = TRUE;
    }
    if (found != FALSE && (GET_OBJ_TYPE(container) != ITEM_DRINKCON)) {
        send_to_char("That item is not a drink container!\r\n", ch);
        return;
    }
    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
                bottle_name);
        send_to_char(buf, ch);
        return;
    }
    if ( !*spell_name) {
        send_to_char("Spell names must be enclosed in single quotes!\r\n",
                     ch);
        return;
    }
    potion = find_skill_num(spell_name);

    if (!IS_SPELL(potion)) {
        send_to_char("Mix what spell?!?\r\n", ch);
        return;
    }
    if (!IS_GOD(ch) && ((GET_LEVEL(ch) - 5) < spell_info[potion].min_level[bti(GET_CLASS(ch))])) {
        send_to_char("You are not able to brew that potion properly.\r\n",
                     ch);
        return;
    }
    if (GET_SKILL(ch, potion) == 0) {
        send_to_char("You are unfamiliar with that spell.\r\n",
                     ch);
        return;
    }
    make_potion(ch, potion, container);
}


char *garble_spell(int spellnum)
{
    char lbuf[500];
    int j, ofs = 0;

    *buf = '\0';
    strcpy(lbuf, spells[spellnum]);

    while (*(lbuf + ofs)) {
        for (j = 0; *(syls[j].org); j++) {
            if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
                strcat(buf, syls[j].new);
                ofs += strlen(syls[j].org);
            }
        }
    }
    return buf;
}

void make_scroll(struct char_data * ch, int scroll, struct obj_data * paper)
{
    struct obj_data *final_scroll;
    struct extra_descr_data *new_descr;
    int can_make = TRUE, mana, dam = 0;

    /* add a case statement here for prohibited spells */
    switch (scroll) {

    case SPELL_MAGIC_MISSILE:
    case SPELL_BLINDNESS:
    case SPELL_CURSE:
    case SPELL_DETECT_ALIGN:
    case SPELL_DETECT_MAGIC:
    case SPELL_DETECT_INVIS:
    case SPELL_FIREBALL:
    case SPELL_IDENTIFY:
    case SPELL_SLEEP:
    case SPELL_WEAKEN:
    case SPELL_TELEPORT:
    case SPELL_GUST_OF_WIND:
        break;

    default:
        can_make = FALSE;
        break;
    }

    if (can_make == FALSE) {
        send_to_char("That spell cannot be scribed into a"
                     " scroll.\r\n", ch);
        return;
    } else if (GET_SKILL(ch, SKILL_SCRIBE) + GET_LEVEL(ch) < number(1, 150)) {
        send_to_char("As you begin inscribing the final rune, the"
                     " scroll violently explodes!\r\n", ch);
        act("$n tries to scribe a spell, but it explodes!",
            FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(paper);
        dam = number(15, mag_manacost(ch, scroll) * 2);
        GET_HIT(ch) = MAX(4, GET_HIT(ch)-dam);
        update_pos(ch);
        return;
    }
    /* requires x3 mana to scribe a scroll than the spell */
    mana = mag_manacost(ch, scroll) * 3;

    if (GET_MANA(ch) - mana > 0) {
        if (GET_LEVEL(ch) < LVL_IMMORT)
            GET_MANA(ch) -= mana;
        sprintf(buf, "You play a little with the spell and finnaly create a scroll of %s.\r\n",
                spells[scroll]);
        send_to_char(buf, ch);
        act("$n creates a scroll!", FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(paper);
    } else {
        send_to_char("You don't have enough mana to scribe such"
                     " a powerful spell!\r\n", ch);
        return;
    }

    final_scroll = create_obj();

    final_scroll->item_number = NOTHING;
    final_scroll->in_room = NOWHERE;
    sprintf(buf2, "%s %s scroll",
            spells[scroll], garble_spell(scroll));
    final_scroll->name = str_dup(buf2);

    sprintf(buf2, "Some parchment inscribed with the runes '%s' lies here.",
            garble_spell(scroll));
    final_scroll->description = str_dup(buf2);

    sprintf(buf2, "a %s scroll", garble_spell(scroll));
    final_scroll->short_description = str_dup(buf2);

    /* extra description coolness! */
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(final_scroll->name);
    sprintf(buf2, "It appears to be a %s scroll.", spells[scroll]);
    new_descr->description = str_dup(buf2);
    new_descr->next = NULL;
    final_scroll->ex_description = new_descr;

    GET_OBJ_TYPE(final_scroll) = ITEM_SCROLL;
    GET_OBJ_WEAR(final_scroll) = ITEM_WEAR_TAKE;
    GET_OBJ_EXTRA(final_scroll) = ITEM_NORENT;
    GET_OBJ_VAL(final_scroll, 0) = GET_LEVEL(ch)-5;
    GET_OBJ_VAL(final_scroll, 1) = scroll;
    GET_OBJ_VAL(final_scroll, 2) = -1;
    GET_OBJ_VAL(final_scroll, 3) = -1;
    GET_OBJ_COST(final_scroll) = GET_LEVEL(ch) * 100;
    GET_OBJ_WEIGHT(final_scroll) = 1;
    GET_OBJ_RENT(final_scroll) = 0;

    obj_to_char(final_scroll, ch);
}


ACMD(do_scribe)
{
    struct obj_data *paper = NULL;
    struct obj_data *obj, *next_obj;
    char paper_name[MAX_STRING_LENGTH];
    char spell_name[MAX_STRING_LENGTH];
    char *temp1, *temp2;
    int scroll = 0, found = FALSE;

    temp1 = one_argument(argument, paper_name);

    /* sanity check */
    if (temp1) {
        temp2 = get_spell_name(temp1);
        if (temp2)
            strcpy(spell_name, temp2);
    } else {
        paper_name[0] = '\0';
        spell_name[0] = '\0';
    }


    if (!GET_SKILL(ch, SKILL_SCRIBE)) {
        send_to_char("You have no idea how to scribe scrolls!\r\n", ch);
        return;
    }
    if (!*paper_name || !*spell_name) {
        send_to_char("Scribe <paper> <'spell'>.\r\n", ch);
        return;
    }
    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(paper = get_obj_in_list_vis(ch, paper_name,
                                               ch->carrying)))
            continue;
        else
            found = TRUE;
    }
    if (found && GET_OBJ_TYPE(paper) != ITEM_NOTE && GET_OBJ_TYPE(paper)!= ITEM_SCROLL) {
        send_to_char("You can't write on that!\r\n", ch);
        return;
    }
    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
                paper_name);
        send_to_char(buf, ch);
        return;
    }
    if ( !*spell_name) {
        send_to_char("Spell names must be enclosed in single quotes!\r\n",
                     ch);
        return;
    }
    scroll = find_skill_num(spell_name);

    if (!IS_SPELL(scroll)) {
        send_to_char("Scribe what spell?!?\r\n", ch);
        return;
    }
    if (!IS_GOD(ch) && ((GET_LEVEL(ch) - 5) < spell_info[scroll].min_level[bti(GET_CLASS(ch))])) {
        send_to_char("You are still not able to write that spell properly.\r\n",
                     ch);
        return;
    }
    if (GET_SKILL(ch, scroll) == 0) {
        send_to_char("You aren't learned in that spell!\r\n",
                     ch);
        return;
    }
    make_scroll(ch, scroll, paper);
}


ACMD(do_forge)
{
    /* PLEASE NOTE!!!  This command alters the object_values of the target
       weapon, and this will save to the rent files.  It should not cause
       a problem with stock Circle, but if your weapons use the first
       position [ GET_OBJ_VAL(weapon, 0); ], then you WILL have a problem.
       This command stores the character's level in the first value to
       prevent the weapon from being "forged" more than once by mortals.
       Install at your own risk.  You have been warned... */

    struct obj_data *weapon = NULL;
    struct obj_data *obj, *next_obj;
    char weapon_name[MAX_STRING_LENGTH];
    int found = FALSE, prob = 0, dam = 0;     
    char buftr[1000];

    one_argument(argument, weapon_name);

    if (!GET_SKILL(ch, SKILL_FORGE)) {
        send_to_char("You have no idea how to forge weapons!\r\n", ch);
        return;
    }
    if (!*weapon_name) {
        send_to_char("What do you want to forge?\r\n", ch);
        return;
    }
    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(weapon = get_obj_in_list_vis(ch, weapon_name,
                                                ch->carrying)))
            continue;
        else
            found = TRUE;
    }

    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
                weapon_name);
        send_to_char(buf, ch);
        return;
    }
    if (found && (GET_OBJ_TYPE(weapon) != ITEM_WEAPON)) {
        sprintf(buf, "It doesn't look like %s would make a"
                " good weapon...\r\n", weapon_name);
        send_to_char(buf, ch);
        return;
    }
    if (GET_OBJ_EXTRA2(weapon) & ITEM2_NOFORGE) {
        send_to_char("The weapon is imbued with magical powers beyond"
                     "your grasp.\r\nYou cannot further affect its form.\r\n", ch);
        return;
    }
    if (IS_ORGANIC(weapon))
    {
        send_to_char("You can not forge organic weapons.\r\n", ch);
        return;
    }
    /* determine success probability */
    if (GET_OBJ_RENT(weapon)>=0)
        prob=GET_SKILL(ch, SKILL_FORGE);
    else
        prob=(GET_SKILL(ch, SKILL_FORGE))/(MIN(10, -GET_OBJ_RENT(weapon)+1));
    if ((number(1, 101)>GET_SKILL(ch, GET_OBJ_VAL(weapon, 3) + TYPE_HIT)) ||            
            number(1, 111) > prob) { 
        
        GET_OBJ_DAMAGE(weapon)-=number(90, 120);    	
        if (GET_OBJ_DAMAGE(weapon)<=0)
	{            	
        send_to_char("As you pound out the dents in the weapon,"
                     " you hit a weak spot breaking it! Ouch!\r\n", ch);
        act("$n tries to forge a weapon, but it breaks apart!",
            FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(weapon);
	}
	else 
	{
	send_to_char("As you pound out the dents in the weapon,"
                     " you hit a weak spot almost ruining it! Ouch!\r\n", ch);
        act("$n tries to forge a weapon, but almost ruins it!",
            FALSE, ch, 0, 0, TO_ROOM);        		
	}


        dam = number(50, 100);
        GET_HIT(ch) -= dam;
        check_kill(ch, "forging");
        return;
    }


    
    if (number(1, 1000)<GET_SKILL(ch, SKILL_FORGE))
        GET_OBJ_VAL(weapon, 1) +=1;  
    else
    	GET_OBJ_VAL(weapon, 2) +=1;

    GET_OBJ_VAL(weapon, 0) += number(1 , 3);
    if (GET_OBJ_RENT(weapon)<0)
        GET_OBJ_RENT(weapon)--;
    else GET_OBJ_RENT(weapon)=-1;
    
    GET_OBJ_COST(weapon)=115*GET_OBJ_COST(weapon)/100;


    act("You sit and start vigorously pounding on $p for few minutes. After \r\nyou clear the sweat of your face, you take a look at it.",FALSE,  ch, weapon, NULL, TO_CHAR);
    send_to_char("\r\n&BYou have forged a new life into the weapon!&0\r\n", ch);

    save_char(ch, ch->in_room);
    act("$n vigorously pounds on $p!",	FALSE, ch, weapon, 0, TO_ROOM);
    if (GET_OBJ_RENT(weapon)==-1) {
        
      //  sprintf(buftr, "%s (forged by %s)", weapon->short_description, GET_NAME(ch));
        sprintf(buftr, "%s (*)", weapon->short_description);
        weapon->short_description = str_dup(buftr);
    } 
    else
    {
    	char *p;
    	p=strstr(weapon->short_description, "*)");
    	if (!p)                                        	
    		sprintf(buftr, "%s (*)", weapon->short_description);
    	else
    	{
    		*p=0;
    		sprintf(buftr, "%s**)%s", weapon->short_description, p+2);
        	weapon->short_description = str_dup(buftr);
        }
     }
    	
    
    

    improve_skill(ch, SKILL_FORGE, 1);
}

ACMD(do_sharpen)
{
    struct obj_data *weapon = NULL;
    struct obj_data *obj, *next_obj;
    char weapon_name[MAX_STRING_LENGTH];
    int found = FALSE, prob = 0, dam = 0;

    one_argument(argument, weapon_name);

    if (!GET_SKILL(ch, SKILL_SHARPEN)) {
        send_to_char("You have no idea how to sharpen arrows!\r\n", ch);
        return;
    }
    if (!*weapon_name) {
        send_to_char("What do you want to sharpen?\r\n", ch);
        return;
    }
    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(weapon = get_obj_in_list_vis(ch, weapon_name,
                                                ch->carrying)))
            continue;
        else
            found = TRUE;
    }

    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
                weapon_name);
        send_to_char(buf, ch);
        return;
    }
    if (found && (GET_OBJ_TYPE(weapon) != ITEM_MISSILE)) {
        sprintf(buf, "You can sharpen arrows only.\r\n");
        send_to_char(buf, ch);
        return;
    }
    if (GET_OBJ_EXTRA2(weapon) & ITEM2_NOFORGE) {
        send_to_char("It is imbued with magical powers beyond"
                     "your grasp.\r\nYou cannot further affect its form.\r\n", ch);
        return;
    }
    if (IS_METAL(weapon))
    {
        send_to_char("It is metal, you can not sharpen it.\r\n", ch);
        return;
    }
    /* determine success probability */
    if (GET_OBJ_RENT(weapon)>=0)
        prob=GET_SKILL(ch, SKILL_SHARPEN)+GET_INT(ch);
    else
        prob=(GET_SKILL(ch, SKILL_SHARPEN)+GET_INT(ch))/(MIN(4, -GET_OBJ_RENT(weapon)+1));
    if (number(1, 111) > GET_SKILL(ch, SKILL_SHARPEN)*prob/100) {

        send_to_char("As you sharpen the arrow, you make a cut to deep"
                     " breaking it!\r\n", ch);
        act("$n tries to sharpen an arrow, but $e cuts it to strong!",
            FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(weapon);
        dam = number(10, 50);
        GET_HIT(ch) -= dam;
        check_kill(ch, "sharpening");
        return;
    }

    if (GET_OBJ_VAL(weapon, 2)>GET_OBJ_VAL(weapon, 1))
        GET_OBJ_VAL(weapon, 2) += number(1, 2);
    else
        GET_OBJ_VAL(weapon, 1) += number(1, 2);
    if (!number(0, 5)) GET_OBJ_VAL(weapon, 0) += 1;
    if (GET_OBJ_RENT(weapon)<0)
        GET_OBJ_RENT(weapon)--;
    else GET_OBJ_RENT(weapon)=-1;

    /*    {
        char buftr[1000];
        	    sprintf(buftr, "%s (forged)", weapon->short_description);
    	    weapon->short_description = str_dup(buftr);
        }*/
    act("You get your pocket knife and sharpen $p for few minutes. After \r\nyou clear the sweat of your face, you take a look at it...",FALSE,  ch, weapon, NULL, TO_CHAR);
    send_to_char("\r\n&B...It's much sharper now!&0\r\n", ch);
    act("$n gets $s pocket knife and carefully sharpens $p!",	FALSE, ch, weapon, 0, TO_ROOM);
    improve_skill(ch, SKILL_SHARPEN, 1);
    save_char(ch, ch->in_room);
}

ACMD(do_hammer)
{
    struct obj_data *weapon = NULL;
    struct obj_data *obj, *next_obj;
    char weapon_name[MAX_STRING_LENGTH];
    int found = FALSE, prob = 0, dam = 0;

    one_argument(argument, weapon_name);

    if (!GET_SKILL(ch, SKILL_HAMMER)) {
        send_to_char("You have no idea how to hammer armors!\r\n", ch);
        return;
    }
    if (!*weapon_name) {
        send_to_char("What do you want to hammer?\r\n", ch);
        return;
    }
    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(weapon = get_obj_in_list_vis(ch, weapon_name,
                                                ch->carrying)))
            continue;
        else
            found = TRUE;
    }

    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
                weapon_name);
        send_to_char(buf, ch);
        return;
    }
    if (found && (GET_OBJ_TYPE(weapon) != ITEM_ARMOR)) {
        sprintf(buf, "It doesn't look like %s would make a"
                " good armor...\r\n", weapon_name);
        send_to_char(buf, ch);
        return;
    }
    if (GET_OBJ_EXTRA2(weapon) & ITEM2_NOHAMMER) {
        send_to_char("The armor is imbued with magical powers beyond"
                     "your grasp.\r\nYou cannot further affect its form.\r\n", ch);
        return;
    }
    if (IS_ORGANIC(weapon))
    {
        send_to_char("You can not hamemr this armor.\r\n", ch);
        return;
    }
    /* determine success probability */
    if (GET_OBJ_RENT(weapon)>=0)
        prob=GET_SKILL(ch, SKILL_HAMMER);
    else
        prob=GET_SKILL(ch, SKILL_HAMMER)/(MIN(4, -GET_OBJ_RENT(weapon)+1));
    if (number(1, 111) > GET_SKILL(ch, SKILL_HAMMER)*prob/100) {

        act("As you pound out the dents in $p, you hit a weak spot breaking it!\r\n", FALSE, ch, weapon, NULL, TO_CHAR);
        act("$n tries to hammer an armor, but it breaks!",
            FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(weapon);
        dam = number(40, 80);
        GET_HIT(ch) -= dam;
        check_kill(ch, "hammering");
        return;
    }

    GET_OBJ_VAL(weapon, 0) +=number(GET_LEVEL(ch)/16, GET_LEVEL(ch)/8);
    if (GET_OBJ_RENT(weapon)<0)
        GET_OBJ_RENT(weapon)--;
    else GET_OBJ_RENT(weapon)=-1;


    act("You sit and start vigorously pounding on $p for few minutes. After \r\nyou clear the sweat of your face, you take a look at it.",FALSE,  ch, weapon, NULL, TO_CHAR);
    act("&B\r\nYou have hammered $p into the new shape.&0\r\n", FALSE, ch, weapon, NULL, TO_CHAR);
    act("$n pounds on $p for some time.",FALSE, ch, weapon, 0, TO_ROOM);
    improve_skill(ch, SKILL_HAMMER, 1);
    save_char(ch, ch->in_room);

}

ACMD(do_godwill)
{   int percent;
    extern struct time_info_data time_info;
    my_srandp(GET_ROOM_VNUM(ch->in_room) + time_info.hours);
    percent=number(1, 3)                                                    ;
    my_srand(rand());
    switch (percent) {

    case 1:
        send_to_char("The voice in your head booms out '&BYES&0!'\r\n", ch);
        break;
    case 2:
        send_to_char("The voice in your head booms out '&RNO&0!'\r\n", ch);
        break;
    case 3:
        send_to_char("The voice in your head booms out '&YMAYBE&0!'\r\n", ch);
        break;
    }
}

ACMD(do_alfa_nerve)
{
    struct char_data *vict;
    int howm;
    if (GET_SKILL(ch, SKILL_ALFA_NERVE) == 0) {
        send_to_char("You are untrained in that area.\r\n", ch);
        return;
    }
    if (GET_MOVE(ch) < GET_LEVEL(ch)*2) {
        send_to_char("You are too tired too concentrate for that.\r\n", ch);
        return;
    }


    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("That person is not here.\r\n", ch);
        return;
    }

    if (GET_HIT(vict) > GET_MAX_HIT(vict)*1/2) {
        send_to_char("You can't do much more with the alfa nerve.\r\n", ch);
        return;
    }
    howm = number(GET_LEVEL(ch)/2, GET_LEVEL(ch)*2);
    //GET_MOVE(ch) = MAX(1, GET_MOVE(ch) - howm * 2);

    if (GET_SKILL(ch, SKILL_ALFA_NERVE) +GET_INT(ch)< number(1, 100)) {
        if (ch==vict)
            act("You unsuccessfully try to press your alfa nerve. All you felt was pain.", FALSE, ch, 0, vict, TO_VICT);
        else {
            act("You unsuccessfully try to find $N's alfa nerve!", FALSE, ch, 0, vict, TO_CHAR);
            act("$n tries to find your alfa nerve unsuccessfully.", FALSE, ch, 0, vict, TO_VICT);
        }
    } else {
        if (ch==vict)
            act("You press a nerve near your heart.", FALSE, ch, 0, vict, TO_CHAR);
        else {
            act("You press a nerve near $N's heart.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n presses his finger near your heart.", FALSE, ch, 0, vict, TO_VICT);
        }
        GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict) + howm);

    }
}

ACMD(do_beta_nerve)
{
    struct char_data *vict;
    int howm;
    if (GET_SKILL(ch, SKILL_BETA_NERVE) == 0) {
        send_to_char("You are untrained in that area.\r\n", ch);
        return;
    }
    if (GET_MOVE(ch) < GET_LEVEL(ch)*2) {
        send_to_char("You are too tired for that now.\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("That person is not here.\r\n", ch);
        return;
    }
    howm = number(GET_LEVEL(ch)/2, GET_LEVEL(ch)*2);
    GET_MOVE(ch) = MAX(1, GET_MOVE(ch) - howm * 2);

    if (GET_SKILL(ch, SKILL_BETA_NERVE) < number(1, 100)) {
        if (ch==vict)
            act("You unsuccessfully try to press your beta nerve.", FALSE, ch, 0, vict, TO_VICT);
        else {
            act("You unsuccessfully try to find $N's beta nerve!", FALSE, ch, 0, vict, TO_CHAR);
            act("$n tries to find your beta nerve unsuccessfully.", FALSE, ch, 0, vict, TO_VICT);
        }
    } else {
        if (ch==vict)
            act("You press a nerve on your forehead.", FALSE, ch, 0, vict, TO_CHAR);
        else {
            act("You press a nerve on $N's forehead.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n puts his finger on your forehead.", FALSE, ch, 0, vict, TO_VICT);
        }
        GET_MANA(vict) = MIN(GET_MAX_MANA(vict), GET_MANA(vict) + howm);
    }
}

ACMD(do_gamma_nerve)
{
    struct char_data *vict;
    int howm;
    if (GET_SKILL(ch, SKILL_GAMMA_NERVE) == 0) {
        send_to_char("You are untrained in that area.\r\n", ch);
        return;
    }
    if (GET_MOVE(ch) < GET_LEVEL(ch)*2) {
        send_to_char("You are too tired for that now.\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("That person is not here.\r\n", ch);
        return;
    }
    howm = number(GET_LEVEL(ch)/2, GET_LEVEL(ch)*2);
    GET_MOVE(ch) = MAX(1, GET_MOVE(ch) - howm * 2);

    if (GET_SKILL(ch, SKILL_GAMMA_NERVE) < number(1, 100)) {
        if (ch==vict)
            act("You unsuccessfully try to press your gamma nerve.", FALSE, ch, 0, vict, TO_VICT);
        else {
            act("You unsuccessfully try to find $N's gamma nerve!", FALSE, ch, 0, vict, TO_CHAR);
            act("$n tries to find your gamma nerve unsuccessfully.", FALSE, ch, 0, vict, TO_VICT);
        }
    } else {
        if (ch==vict)
            act("You press a nerve on your foot.", FALSE, ch, 0, vict, TO_CHAR);
        else {
            act("You press a nerve on $N's foot.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n puts his finger on your foot.", FALSE, ch, 0, vict, TO_VICT);
        }
        GET_MOVE(vict) = MIN(GET_MAX_MOVE(vict), GET_MOVE(vict) + howm);
    }
}

char *dbmsgs[][2] = {
                        {"You start to break some floor boards when you dig.\r\n",
                         "$n starts to break some floor boards as $e starts digging.\r\n"},
                        {"You wonder if this is a good place after all, with all the gravel.\r\n",
                         "$n breaks a sweat digging up all the gravel here.\r\n"},
                        {"You make a nice hole while digging up a lot of dirt.\r\n",
                         "$n digs a hole and goes about $s business.\r\n"},
                        {"You seem to be hitting alot of roots when you dig.\r\n",
                         "$n look like $e is trying to dig up a tree!\r\n"},
                        {"You dig up more clay than dirt here.\r\n",
                         "$n seems to be digging up alot of clay.\r\n"},
                        {"You start to chip away at the rock here.\r\n",
                         "$n bangs away at the side of the mountain.\r\n"},
                        {"You can't dig in the water!\r\n",
                         NULL},
                        {"You can't dig in the water!\r\n",
                         NULL},
                        {"You can't dig in the water!\r\n",
                         NULL},
                        {"You can't dig up air!\r\n",
                         NULL},
                        /* always keep this as the last message */
                        {"If you see this please tell a god.\r\n", NULL}
                    };

//#pragma argsused
ACMD(do_bury)
{
    struct obj_data *obj;

    half_chop(argument, arg, buf);
    if (!*arg) {
        sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
        send_to_char(buf2, ch);
        return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "You don't have %s %s.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
        return;
    }
    if (IS_OBJ_STAT(obj, ITEM_NODROP))
    {
        act("You can't bury $p, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }
    
    if (GET_OBJ_TYPE(obj)==ITEM_KEY)
    {
    	send_to_char("You are forbidden to bury keys.\r\n", ch);
    	return;
    }
    /* * find the sector types that you don't want people * to be able to
       dig or bury in. */
    if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
            (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {
        /* display the messages if available */
        if (dbmsgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
            send_to_char(dbmsgs[world[IN_ROOM(ch)].sector_type][0], ch);
        if (dbmsgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
            act(dbmsgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL, TO_ROOM);
        return;
    }
    /* set a wait state */
    //WAIT_STATE(ch, 4 RL_SEC);

    if (dbmsgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
        send_to_char(dbmsgs[world[IN_ROOM(ch)].sector_type][0], ch);
    if (dbmsgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
        act(dbmsgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL, TO_ROOM);

    act("You lay on the work, and after some time you bury $a $o here.", TRUE, ch, obj, NULL, TO_CHAR);
    act("$n digs a hole and buries $a $o here.", TRUE, ch, obj, NULL, TO_ROOM);

    obj_from_char(obj);
    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BURIED);  
    global_no_timer=1;
    obj_to_room(obj, IN_ROOM(ch));
    global_no_timer=0;
};

//#pragma argsused
ACMD(do_dig)
{
    struct obj_data *obj;
    int found_item = 0;

    /* * find the sector types that you don't want people * to be able to
       dig or bury in. */
    if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
            (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {
        /* display the messages if available */
        if (dbmsgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
            send_to_char(dbmsgs[world[IN_ROOM(ch)].sector_type][0], ch);
        if (dbmsgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
            act(dbmsgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL, TO_ROOM);
        return;
    }
    /* set a wait state */
    //WAIT_STATE(ch, 1 RL_SEC);

    /* * Now that we have established that we can dig lets go * ahead and
       do it! */
    if (dbmsgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
        send_to_char(dbmsgs[world[IN_ROOM(ch)].sector_type][0], ch);
    if (dbmsgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
        act(dbmsgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL, TO_ROOM);

    /* * search for an object in the room that has a ITEM_BURIED flag */
    obj = world[IN_ROOM(ch)].contents;

    while (obj != NULL) {
        if (IS_BURIED(obj)) {
            /* Remove the buried bit so the player can see it. */
            REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_BURIED);

            if (CAN_SEE_OBJ(ch, obj)) {
                found_item = 1;	/* player found something */
                act("\r\nAs you start to dig you found $a $o buried here!", TRUE, ch, obj, NULL, TO_CHAR);
                act("$n digs out $a $o, buried here!", TRUE, ch, obj, NULL, TO_ROOM);
                obj_from_room(obj);
                if (!can_take_obj(ch, obj)) 
                {global_no_timer=1;
                    obj_to_room(obj, ch->in_room);
                    global_no_timer=0;
                   }
                else
                    obj_to_char(obj, ch);
            } else {
                /* add the bit back becuase the player can't unbury what *
                   what he can't find...  */
                SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BURIED);
            }
        }
        /* go to the next obj */
        obj = obj->next_content;
    }

    if (!found_item)		/* if the player didn't find anything */
        send_to_char("Sorry! You didn't find anything.\r\n", ch);
}

ACMD(do_label)
{
    struct obj_data *obj;

    two_arguments(argument, buf1, buf2);
    if (!*buf1) {
        sprintf(buf, "What do you want to %s?\r\n", CMD_NAME);
        send_to_char(buf, ch);
        return;
    }
    if (!(obj = get_obj_in_list_vis(ch, buf1, ch->carrying))) {
        sprintf(buf, "You don't have %s %s.\r\n", AN(buf1), buf1);
        send_to_char(buf, ch);
        return;
    }
    if ((GET_OBJ_TYPE(obj) != ITEM_CONTAINER)) {
        send_to_char("You can label containers such as bags only.\r\n", ch);
        return;
    }
    if ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && (GET_OBJ_VAL(obj, 3))) {
        send_to_char("You can not label corpses.\r\n", ch);
        return;
    }
    if (!*buf2) {
        send_to_char("How do you wish label it?\r\n", ch);
        return;
    }
    if (IS_SET(GET_OBJ_EXTRA(obj), ITEM_LABEL)) {
        send_to_char("That is already labeled.\r\n", ch);
        return;
    }
    strcpy(buf1, obj->name);
    strcat(buf1, " ");
    strcat(buf1, buf2);
    obj->name = str_dup(buf1);
    strcpy(buf1, obj->short_description);
    strcat(buf1, " labeled ");
    strcat(buf1, buf2);
    obj->short_description = str_dup(buf1);
    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_LABEL);
    send_to_char("Done.\r\n", ch);
}



ACMD(do_listen)
{
    struct char_data *tch, *tch_next;
    int dir, percent, found = 0;
    char *heard_nothing = "You strain your guts out but don't hear anything unusual.\r\n";
    char *room_spiel = "$n seems to listen intently for something.";

    percent = number(1, 150);

    if (GET_SKILL(ch, SKILL_LISTEN) + GET_LEVEL(ch) < percent) {
        send_to_char(heard_nothing, ch);
        return;
    }
    one_argument(argument, buf);

    if (!*buf) {
        /* no argument means that the character is listening for hidden or
           invisible beings in the room he/she is in */
        for (tch = world[ch->in_room].people; tch; tch = tch_next) {
            tch_next = tch->next_in_room;
            if ((tch != ch) && (GET_LEVEL(tch) < LVL_IMMORT))
                found++;
        }
        if (found) {
            sprintf(buf, "As you close your eyes you manage to hear what might be about %d other creatures here.\r\n", \
                    found);
            send_to_char(buf, ch);
        } else
            send_to_char(heard_nothing, ch);
        act(room_spiel, TRUE, ch, 0, 0, TO_ROOM);
        return;
    } else {
        /* the argument must be one of the cardinal directions: north,
           south, etc. */
        for (dir = 0; dir < NUM_OF_DIRS; dir++) {
            if (!strncmp(buf, dirs[dir], strlen(buf)))
                break;
        }
        if (dir == NUM_OF_DIRS) {
            send_to_char("Listen where?\r\n", ch);
            return;
        }
        if (CAN_GO(ch, dir) || CAN_LISTEN_BEHIND_DOOR(ch, dir)) {
            for (tch = world[EXIT(ch, dir)->to_room].people; tch; tch = tch_next) {
                tch_next = tch->next_in_room;
                found++;
            }
            if (found) {
                sprintf(buf, "As you close your eyes you manage to hear what might be %d creatures %s%s.\r\n", \
                        found,
                        ((dir == 5) ? "below" : (dir == 4) ? "above" : "to the "),
                        ((dir == 5) ? "" : (dir == 4) ? "" : dirs[dir]));
                send_to_char(buf, ch);
            } else
                send_to_char(heard_nothing, ch);
            act(room_spiel, TRUE, ch, 0, 0, TO_ROOM);
            return;
        } else
            send_to_char("You can't listen in that direction.\r\n", ch);
        return;
    }
    return;
}

#define NOTI "$N looks uninterested.",FALSE,ch,NULL,victim,TO_CHAR

ACMD(do_fame)
{
    struct affected_type af;
    struct char_data *victim;
    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_FAME)) {
        send_to_char("You would if you knew how.\r\n", ch);
        return;
    }
    if (!(victim = get_char_room_vis(ch, arg))) {
        send_to_char("That person is not here.\r\n", ch);
        return;
    }
    if (victim == NULL || ch == NULL)
        return;
    if (victim == ch) {
        send_to_char("You better find a doctor.\r\n", ch);
        return;
    }
    /*	if (!MOB_FLAGGED(ch,MOB_GANG_LEADER))
        if (GET_MOVE(ch) < GET_LEVEL(victim) * (GET_LEVEL(victim) / 4)) {
    	send_to_char("You are too exhausted too even try that now.\r\n", ch);
    		return;
        }
    	if (!MOB_FLAGGED(ch,MOB_GANG_LEADER))
    	    GET_MOVE(ch) -= GET_LEVEL(victim) * (MAX(1, GET_LEVEL(victim) / 4));
    */
    if (FIGHTING(victim))
    {
        send_to_char("Can't you see they are fighting now?\r\n", ch);
        return;
    }
    if (count_pets(ch)>=NUM_PETS_ALLOWED)
    {
        send_to_char(MSG_TOO_MANY_PETS, ch);
        return;
    };
    act("You expostulate your heroic adventures to $N.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$n expostulates $s heroic adventures to $N.", FALSE, ch, NULL, victim, TO_NOTVICT);
    if ((GET_LEVEL(ch)) < GET_LEVEL(victim))
        act(NOTI);
    else if (IS_AFFECTED(victim, AFF_SANCTUARY))
        send_to_char("Your victim is protected by Sanctuary!\r\n", ch);
    else if (MOB_FLAGGED(victim, MOB_NOCHARM))
        act(NOTI);
    else if (IS_AFFECTED(ch, AFF_CHARM))
        send_to_char("You can't have any followers of your own!\r\n", ch);
    else if (IS_AFFECTED(victim, AFF_CHARM))
        act(NOTI);
    /* player charming another player - no legal reason for this */
    else if (!IS_NPC(victim))
        send_to_char("You fail - shouldn't be doing it anyway.\r\n", ch);
    else if (circle_follow(victim, ch))
        act(NOTI);
    else if (!MOB_FLAGGED(ch,MOB_GANG_LEADER) && number(1, 125) > GET_CHA(ch)+GET_SKILL(ch, SKILL_FAME))
        act("$N looks you with interest.", FALSE, ch, NULL, victim, TO_CHAR);
    else if (INT_CHECK(victim) && WIL_CHECK(victim) && INT_CHECK(victim))
    {
        act("$n mummbles.", FALSE, victim,NULL, NULL,  TO_ROOM);
    }
    else {
        if (victim->master)
            stop_follower(victim);

        act("Amazed by your stories, $N decides to join your travels.", FALSE, ch, NULL, victim, TO_CHAR);
        act("Amazed by $n's stories, $N decides to join $m.", FALSE, ch, NULL, victim, TO_ROOM);
        act("Isn't $n such a cool guy?", FALSE, ch, 0, victim, TO_VICT);
        add_follower(victim, ch);
        perform_group(ch, victim);

        af.type = 0;
        af.bitvector = 0;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.type = SPELL_CHARM;

        if (MOB_FLAGGED(ch,MOB_GANG_LEADER))
            af.duration=50;
        else
            af.duration = 24 + GET_LEVEL(ch) - GET_LEVEL(victim) + GET_CHA(ch) - GET_CHA(victim);

        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);


        if (IS_NPC(victim)) {
            REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);           
            REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
        }
    }
}

ACMD(do_feign_death)
{
    if (ch == NULL)
        return;

    if (GET_SKILL(ch, SKILL_FEIGN_DEATH) == 0) {
        send_to_char("You better not try it.\r\n", ch);
        return;
    }
    if (!FIGHTING(ch)) {
        send_to_char("You don't need to do that.\r\n", ch);
        return;
    }
    if (!(FIGHTING(FIGHTING(ch)) == ch)) {
        send_to_char("You decide that would be to wimpy.\r\n", ch);
        return;
    }
    send_to_char("You lie down, pretending dead.\r\n", ch);

    if (number(1, 111) > GET_SKILL(ch, SKILL_FEIGN_DEATH)) {
        stop_fighting(ch);
        GET_POS(ch) = POS_SLEEPING;
        act("$n tries to feign death but manages to fool only some flies.", FALSE, ch, NULL, NULL, TO_ROOM);
        return;
    }
    act("$n is DEAD! R.I.P.", FALSE, ch, NULL, NULL, TO_ROOM);
    death_cry(ch);

    stop_fighting(FIGHTING(ch));
    stop_fighting(ch);
    GET_POS(ch) = POS_SLEEPING;
    WAIT_STATE(ch, PULSE_VIOLENCE);
}



#define MAX_MAP 72
#define MAX_MAP_DIR 10

char *map[MAX_MAP][MAX_MAP];
int offsets[10][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1},{0,0},{0,0},{-1,1},{-1,-1},{1,1},{1,-1} };

void MapArea (struct room_data *room, struct char_data *ch, int x, int y, int min, int max, int show_closed)
{
    struct room_data prospect_room;
    struct room_direction_data *pexit;
    int door;

    /* marks the room as visited */
    switch (room->sector_type)
    {
    case SECT_INSIDE:	        map[x][y]="&C+&0";		break;
    case SECT_CITY:		        map[x][y]="&C.&0";		break;
    case SECT_FIELD:	        map[x][y]="&C.&0";		break;
    case SECT_FOREST:	        map[x][y]="&GF&0";		break;
    case SECT_HILLS:	        map[x][y]="&p^&0";		break;
    case SECT_MOUNTAIN:	        map[x][y]="&P^&0";		break;
    case SECT_WATER_SWIM:	    	map[x][y]="&B~&0";		break;
    case SECT_WATER_NOSWIM:	   	map[x][y]="&B~&0";		break;
    case SECT_UNDERWATER:	    	map[x][y]="&B~&0";		break;
    case SECT_FLYING:		map[x][y]="&w:&0";		break;
    case SECT_QUICKSAND:		map[x][y]="&y=&0";		break;
    case SECT_LAVA:     		map[x][y]="&y=&0";		break;
    case SECT_ARCTIC:	        map[x][y]="&y=&0";		break;
    default: 		        map[x][y]="&P?&0";
    }

    //if (!number(0,20))
    //  map[x][y]="&P?&0";

    for ( door = 0; door < MAX_MAP_DIR; door++ )
    {
        if (door==UP || door==DOWN)
            continue;
        pexit=room->dir_option[door];
        if (pexit  && (pexit->to_room>0) && !IS_SET(pexit->exit_info, EX_HIDDEN))
        {
            if (IS_SET(pexit->exit_info, EX_CLOSED) ||
                    (SECT(pexit->to_room) == SECT_FLYING && !can_fly(ch)) ||
                    (((SECT(pexit->to_room) == SECT_WATER_NOSWIM) || (SECT(pexit->to_room) == SECT_WATER_NOSWIM)) && !can_fly(ch) && !has_boat(ch)) )
                if (show_closed)
                {
                    map[x][y]="&R!&0";
                    continue;
                }


            prospect_room = world[pexit->to_room];

            if ( prospect_room.dir_option[rev_dir[door]] &&
                    prospect_room.dir_option[rev_dir[door]]->to_room_vnum!=room->number)
            { /* if not two way */
                if ((prospect_room.sector_type==SECT_CITY)
                        ||  (prospect_room.sector_type==SECT_INSIDE))
                    map[x][y]="&W+&0";
                else
                    map[x][y]="&C.&0";
                return;
            } /* end two way */

            if ((x<=min)||(y<=min)||(x>=max)||(y>=max)) return;
            if (map[x+offsets[door][0]][y+offsets[door][1]]==NULL) {
                MapArea (&world[pexit->to_room],ch,
                         x+offsets[door][0], y+offsets[door][1],min,max, show_closed);
            }

        } /* end if exit there */
    }
    return;
}
int write_to_mapobj(struct char_data *ch, struct obj_data *paper, char *map)
{
    struct obj_data *final_scroll=NULL;                               
    struct extra_descr_data *new_descr;
    char buf2[1000];

    if (GET_OBJ_TYPE(paper) == ITEM_NOTE && GET_OBJ_VAL(paper, 3) < 1)
    {

        extract_obj(paper);
        return 0;
    }


    final_scroll = create_obj();

    final_scroll->item_number = NOTHING;
    final_scroll->in_room = NOWHERE;
    sprintf(buf2, "map of %s", world[ch->in_room].name);
    final_scroll->name = str_dup(buf2);

    sprintf(buf2, "A piece of map lies here.");
    final_scroll->description = str_dup(buf2);

    sprintf(buf2, "a map of %s",  world[ch->in_room].name);
    final_scroll->short_description = str_dup(buf2);

    /* extra description coolness! */
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(final_scroll->name);
    sprintf(buf2, "This map was made by %s at %s.\r\n\r\n%s", GET_NAME(ch), world[ch->in_room].name, map);
    new_descr->description = str_dup(buf2);
    new_descr->next = NULL;
    final_scroll->ex_description = new_descr;

    if (GET_OBJ_TYPE(paper) == ITEM_NOTE)
        GET_OBJ_VAL(final_scroll, 3) = GET_OBJ_VAL(paper, 3)-1;
    else
        GET_OBJ_VAL(final_scroll, 3) = 3;
    GET_OBJ_TYPE(final_scroll) = ITEM_NOTE;
    GET_OBJ_WEAR(final_scroll) = ITEM_WEAR_TAKE;
    GET_OBJ_EXTRA(final_scroll) = ITEM_NORENT;
    GET_OBJ_VAL(final_scroll, 0) = -1;
    GET_OBJ_VAL(final_scroll, 1) = -1;
    GET_OBJ_VAL(final_scroll, 2) = -1;
    GET_OBJ_COST(final_scroll) = GET_LEVEL(ch) * 10;
    GET_OBJ_WEIGHT(final_scroll) = 1;
    GET_OBJ_RENT(final_scroll) = 0;

    extract_obj(paper);
    obj_to_char(final_scroll, ch);
    return 1;
}


int ShowMap(struct char_data *ch, int min, int max, struct obj_data *paper)
{
    int x,y;
    char bufsm[10000]="\0";;

    for (x = min; x < max; ++x)
    {
        for (y = min; y < max; ++y)
        {
            if (map[x][y]==NULL)
                strcat(bufsm," ");
            else
                strcat(bufsm,map[x][y]);
        }
        strcat(bufsm,"\r\n");
    }

    return (write_to_mapobj(ch, paper, (char *) bufsm));
}


int do_mapper(struct char_data *ch, int size , int show_closed, struct obj_data *paper)
{
    int center,x,y,min,max;
    char arg1[10];

    size=MIN(72,MAX(3,size));
    center=MAX_MAP/2;

    min = MAX_MAP/2-size/2;
    max = MAX_MAP/2+size/2;

    for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
            map[x][y]=NULL;

    /* starts the mapping with the center room */
    MapArea(&world[ch->in_room], ch, center, center, min, max, show_closed);

    /* marks the center, where ch is */
    map[center][center]="&Y&F@&0";
    return (ShowMap (ch, min, max, paper));

}


void show_surrounding(struct char_data *ch)
{
    int center,x,y,min,max, size, show_closed;
    char bufsm[10000]="\r\n";
    char arg1[10];

    show_closed=1;
    size =6;//GET_HITROLL(ch);


    //size=MIN(72,MAX(3,size));

    center=MAX_MAP/2;

    min = MAX_MAP/2-size/2;
    max = MAX_MAP/2+size/2;

    for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
            map[x][y]=NULL;

    /* starts the mapping with the center room */
    MapArea(&world[ch->in_room], ch, center, center, min, max, show_closed);

    /* marks the center, where ch is */
    map[center][center]="&Y@&0";



    for (x = min; x <= max; ++x)
    {
        strcat(bufsm, "    ");
        for (y = min; y <= max; ++y)
        {

            if (map[x][y]==NULL)
                strcat(bufsm," ");
            else
                strcat(bufsm,map[x][y]);
        }
        strcat(bufsm,"\r\n");
    }

    send_to_char(bufsm, ch);
}

ACMD(do_map)
{
    struct obj_data *paper = NULL;
    struct obj_data *obj, *next_obj;
    char paper_name[MAX_STRING_LENGTH];

    char *temp1, *temp2;
    int scroll = 0, found = FALSE;

    temp1 = one_argument(argument, paper_name);

    if (!GET_SKILL(ch, SKILL_MAP)) {
        send_to_char("You have only faint idea how to do that. Better use scan for now.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
    {
        send_to_char("You can not do that here!\r\n", ch);
        return;
    }

    if ( !*paper_name){
        send_to_char("You need to specify on what should be map drawn.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<5*PERCENT(ch))
    {
        send_to_char("You do not have enough movement points for that.\r\n",ch);
        return;
    }
    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(paper = get_obj_in_list_vis(ch, paper_name,
                                               ch->carrying)))
            continue;
        else
            found = TRUE;
    }
    if (found && GET_OBJ_TYPE(paper) != ITEM_NOTE && GET_OBJ_TYPE(paper)!= ITEM_SCROLL) {
        send_to_char("You can't draw on that.\r\n", ch);
        return;
    }
    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
                paper_name);
        send_to_char(buf, ch);
        return;
    }

    if (SECT(ch->in_room)==SECT_CITY)
    {
        send_to_char("All this surrounding noise distracts you.\r\n",ch);
        write_to_mapobj(ch, paper,"???\r\n?*?\r\n???\r\n");
        return;
    }

    send_to_char("You explore the terrain.\r\n",ch);
    act("$n grabs $p and starts exploring the terrain.",FALSE, ch, paper, NULL, TO_ROOM);
    if (!do_mapper(ch, GET_SKILL(ch,SKILL_MAP)/10+1, 1, paper))
        send_to_char("\r\nThe paper was used so many times that you finnaly couldn't understand a single\r\nword you write so you decide to tear it apart.\r\n",ch);
    else
    {
        GET_MOVE(ch)-=5*PERCENT(ch);
        //        WAIT_STATE(ch, 3*PULSE_VIOLENCE);
        improve_skill(ch, SKILL_MAP, 1);
    }
}

ACMD(do_kata)
{

    int i;
    struct affected_type af;

    if (!GET_SKILL(ch, SKILL_KATA))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<2*PERCENT(ch))
    {
        send_to_char("You are too exhausted too even try that now.\r\n",ch);
        return;
    }

    GET_MOVE(ch)-=2*PERCENT(ch);

    if (number(0,101)>GET_SKILL(ch, SKILL_KATA))
    {
        send_to_char("You try to perform a kata but you lost your concentration.\r\n", ch);
        return;
    }

    if (AFF2_FLAGGED(ch, AFF2_KATA))
    {
        send_to_char("You already feel ready and agile.\r\n",ch);
        return;
    }
    i=GET_LEVEL(ch)/4;
    /*if (isname("great", get_mood(ch)))
    	i+=4;

    i=MAX(1,i);
    */	
    af.type = SKILL_KATA;
    af.duration = 1;
    af.modifier = i;
    af.location = APPLY_HITROLL;
    af.bitvector =0;
    af.bitvector2 = AFF2_KATA;
    af.bitvector3 = 0;
    affect_to_char(ch, &af);

    send_to_char("&cYou perform kata with fluid motions.&0\r\n", ch);
    act("&c$n performs kata with fluid motions.&0", FALSE, ch, NULL, NULL, TO_ROOM);
    improve_skill(ch, SKILL_KATA, 1);

}




ACMD(do_battlecry)
{
    struct affected_type af;
    struct char_data *tch,
                *k;
    struct follow_type *f,
                *f_next;
    int flag=0;

    if (!GET_SKILL(ch, SKILL_BATTLECRY))
{
        send_to_char("Go play with your toys and leave this to true warriors.\r\n", ch);
        return;
    }


    if (!IS_AFFECTED(ch, AFF_GROUP) || count_group(ch)<2)
    {
        send_to_char("You lead no group now!\r\n", ch);
        return;
    }

    if (ch->master != NULL)
    {
        send_to_char("You lead no group now!\r\n", ch);
        return;
    }


    if (FIGHTING(ch))
    {
        send_to_char("You are fighting now!\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
        send_to_char("This room has such a peaceful feeling for such an uncivilized behaviour.\r\n", ch);
        return;
    }


    if (AFF2_FLAGGED(ch, AFF2_BATTLECRY))
    {
        send_to_char("You are already raging!\r\n",ch);
        return;
    }

    if (number(1,101)>GET_SKILL(ch, SKILL_BATTLECRY))
    {
        send_to_char("Starting a mighty battlecry, you end up entertaining your group.\r\n", ch);
        return;
    }


    k = ch;
    for (f = k->followers; f; f = f_next) {
        f_next = f->next;
        tch = f->follower;
        if (tch->in_room != ch->in_room)
            continue;
        if (!is_same_group(ch, tch))
            continue;
        if (ch == tch)
            continue;
        if (IS_MONK(tch))
            continue;
        if (flag==0)
        {
            send_to_char("&RARGH! AAAARGH! AAAAAAAAAAAAAAAAAAARGH!&0\r\n",ch);
            act("$n screams, '&RARGH! AAAARGH! AAAAAAAAAAAAAAAAAAARGH!&0'",FALSE, ch, NULL, NULL, TO_ROOM);
            improve_skill(ch, SKILL_BATTLECRY, 1);
            flag=1;
        }
        send_to_char("\r\nYou join the rage!\r\n", tch);

        af.type = SKILL_BATTLECRY;
        af.duration = 4;
        af.modifier = MAX(4, (GET_SKILL(ch, SKILL_BATTLECRY)-60)/5);
        af.location = APPLY_HITROLL;
        af.bitvector =0;
        af.bitvector2 = AFF2_BATTLECRY;
        af.bitvector3 = 0;
        affect_to_char(tch, &af);
        af.location = APPLY_DAMROLL;
        affect_to_char(tch, &af);
    }

    if (flag==1)
    {
        af.type = SKILL_BATTLECRY;
        af.duration = 4;
        af.modifier =MAX(4, (GET_SKILL(ch, SKILL_BATTLECRY)-60)/5);
        af.location = APPLY_HITROLL;
        af.bitvector =0;
        af.bitvector2 = AFF2_BATTLECRY;
        af.bitvector3 = 0;
        affect_to_char(ch, &af);
        af.location = APPLY_DAMROLL;
        affect_to_char(ch, &af);

    }
    else
    {
        send_to_char("You have no group members here!\r\n", ch);
        return;
    }
}


ACMD(do_meditate)
{

    int i;
    struct affected_type af;

    if (!GET_SKILL(ch, SKILL_MEDITATE))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
            (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {
        send_to_char("You can not meditate here!\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<50)
    {
        send_to_char("You are too exhausted too even try that now.\r\n",ch);
        return;
    }

    GET_MOVE(ch)-=50;

    if (number(0,101)>GET_SKILL(ch, SKILL_MEDITATE))
    {
        send_to_char("You try to relax, to no avail.\r\n", ch);
        return;
    }


    af.type = SKILL_MEDITATE;
    af.duration = 0;
    af.bitvector =AFF_SLEEP;
    af.bitvector2 = AFF2_MEDITATE;
    af.bitvector3 = 0;
    af.modifier = 0;
    af.location = APPLY_NONE;
    affect_to_char(ch, &af);

    send_to_char("You sit down and sink into a deep trans.\r\n", ch);
    act("$n sits down and sinks into a deep trans.", FALSE, ch, NULL, NULL, TO_ROOM);
    GET_POS(ch)=POS_SLEEPING;

    improve_skill(ch, SKILL_MEDITATE, 1);
}

ACMD(do_move_hidden)
{

    int i;
    int percent;
    struct affected_type af;


    if (!GET_SKILL(ch, SKILL_HIDE))
    {
        send_to_char("You don't even know how to hide!\r\n", ch);
        return;
    }
    if (!GET_SKILL(ch, SKILL_MOVE_HIDDEN))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (AFF2_FLAGGED(ch, AFF2_MOVE_HIDDEN))
    {

        affect_from_char(ch, SKILL_MOVE_HIDDEN);
        send_to_char("You stop moving hidden.\r\n", ch);
    }
    else
    {
        send_to_char("You will now try to move hidden.\r\n", ch);



        af.type = SKILL_MOVE_HIDDEN;
        af.duration = -1;//GET_SKILL(ch, SKILL_MOVE_HIDDEN)/15;
        af.bitvector =AFF_HIDE;
        af.bitvector2 = AFF2_MOVE_HIDDEN;
        af.bitvector3 = 0;
        af.modifier = 0;
        af.location = APPLY_NONE;
        affect_to_char(ch, &af);

    }


}

ACMD(do_nomissf)
{
    TOGGLE_BIT(PRF2_FLAGS(ch), PRF2_NOMISSF);
    sprintf(buf,"Friendly misses are &c%s&0\r\n", ONOFF(!PRF2_FLAGGED(ch, PRF2_NOMISSF)));
    send_to_char(buf, ch);
}

ACMD(do_nomisse)
{
    TOGGLE_BIT(PRF2_FLAGS(ch), PRF2_NOMISSE);
    sprintf(buf,"Enemy misses are &c%s&0\r\n", ONOFF(!PRF2_FLAGGED(ch, PRF2_NOMISSE)));
    send_to_char(buf, ch);
}

ACMD(do_cover_tracks)
{


    int i;
    int percent;
    struct affected_type af;


    if (!GET_SKILL(ch, SKILL_COVER_TRACKS))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg);
    if (*arg=='!')
    {
        if (AFF_FLAGGED(ch, AFF_NOTRACK))
        {
            send_to_char("You stop covering your tracks.", ch);
            affect_from_char(ch, SKILL_COVER_TRACKS);
        }
        send_to_char("\r\n", ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_NOTRACK))
    {
        send_to_char("You are already covering your tracks.\r\n", ch);
        return;
    }


    af.type = SKILL_COVER_TRACKS;
    af.duration = GET_SKILL(ch, SKILL_COVER_TRACKS)/20;
    af.bitvector =AFF_NOTRACK;
    af.bitvector2 =0;
    af.bitvector3 = 0;
    af.modifier = 0;
    af.location = APPLY_NONE;
    affect_to_char(ch, &af);

    send_to_char("You start covering your tracks.\r\n", ch);

    improve_skill(ch, SKILL_COVER_TRACKS, 1);

}



ACMD(do_bandage)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_BANDAGE))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }


    if (!*arg)
        vict=ch;
    else
        if (!(vict = get_char_room_vis(ch, arg))) {
            send_to_char("Whom do you wish to bandage?\r\n", ch);
            return;
        }

    if (GET_HIT(vict)>GET_MAX_HIT(vict)/3)
    {
        send_to_char("There is no need for that.\r\n", ch);
        return;
    }
    if (GET_MOVE(ch)<30)
    {
        send_to_char("You do not have enough movement points.\r\n",ch);
        return;
    }

    GET_MOVE(ch)-=30;



    if (number(1, 111) <= GET_SKILL(ch, SKILL_BANDAGE)) {
        //GET_HIT(vict) += dice(1, 8) + GET_LEVEL(ch) / 3;
        //GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict));
        GET_HIT(vict) = GET_MAX_HIT(vict)/3;
        if (vict!=ch)
        {
            act("You apply bandages to $N's wounds.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n touches bandages to your wounds.", FALSE, ch, 0, vict, TO_VICT);
            act("$n begins to bandage $N.", FALSE, ch, 0, vict, TO_NOTVICT);
        }
        else
        {
            act("You apply bandages to your wounds.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n applies bandages to $s wounds.", FALSE, ch, 0, 0, TO_ROOM);
        }
        improve_skill(ch, SKILL_BANDAGE, 2);
    } else
        send_to_char("You fail.\r\n", ch);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    WAIT_STATE(vict, PULSE_VIOLENCE);
}



ACMD(do_medic)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_MEDIC))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }



    if (!*arg)
        vict=ch;
    else
        if (!(vict = get_char_room_vis(ch, arg))) {
            send_to_char("Whom do you wish to medic?\r\n", ch);
            return;
        }

    if (GET_HIT(vict)>GET_MAX_HIT(vict)/2)
    {
        send_to_char("There is no need for that.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<60)
    {
        send_to_char("You do not have enough movement points.\r\n",ch);
        return;
    }

    GET_MOVE(ch)-=60;

    if (number(1, 111) <= GET_SKILL(ch, SKILL_MEDIC)) {
        GET_HIT(vict) = GET_MAX_HIT(vict)/2;
        if (ch!=vict)
        {
            act("You apply your medicine skills to $N.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n applies $s medicine skills to you.", FALSE, ch, 0, vict, TO_VICT);
            act("$n applies $s medicine on $N.", FALSE, ch, 0, vict, TO_NOTVICT);
        }
        else
        {
            act("You apply medicine to yourself.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n applies medicine to $sself.", FALSE, ch, 0, vict, TO_ROOM);
        }
        improve_skill(ch, SKILL_MEDIC, 2);
    } else
        send_to_char("You fail.\r\n", ch);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    WAIT_STATE(vict, 2*PULSE_VIOLENCE);
}




ACMD(do_forage)
{
    struct obj_data *item_found = NULL;
    int item_no = 2505; /* Initialize with first item poss. */
    *buf = '\0';

    if(!GET_SKILL(ch, SKILL_FORAGE))
    {
        send_to_char("You have no idea how to forage for survival!\r\n", ch);
        return; }

    if(GET_MOVE(ch) < 5*PERCENT(ch))
    {
        send_to_char("You are too tired to forage right now.\r\n", ch);
        return; }

    if(SECT(ch->in_room) != SECT_FIELD && SECT(ch->in_room) != SECT_FOREST && SECT(ch->in_room) != SECT_HILLS && SECT(ch->in_room) != SECT_MOUNTAIN)
    {
        send_to_char("You cannot forage around here!\r\n", ch);
        return;
    }

    send_to_char("You start searching the area for any signs of food.\r\n", ch);
    act("$n starts foraging the area for food.\r\n", FALSE, ch, 0, 0, TO_ROOM);
    GET_MOVE(ch) -= 5*PERCENT(ch);

    if(number(1,101) > GET_SKILL(ch, SKILL_FORAGE))
    {
        send_to_char("\r\nYou have no luck finding anything to eat.\r\n", ch);
        return;
    }
    else
    {
        switch (number(1,7))
        {
        case 1:
            item_no = 2505; break;  /*<--- Here are the objects you need to code */
        case 2:                   /* Add more or remove some, just change the */
            item_no = 602; break;  /* switch(number(1, X) */
        case 3:
            item_no = 603; break;
        case 4:
            item_no = 6018; break;
        case 5:
            item_no = 6023; break;
        case 6:
            item_no = 6024; break;
        case 7:
            item_no = 3015; break;
        }
        WAIT_STATE( ch, PULSE_VIOLENCE);  /* Not really necessary */
        item_found = read_object( item_no, VIRTUAL,0, 0);
        obj_to_char(item_found, ch);
        send_to_char("Bingo! You manage to find some food.\r\n", ch);
        act("$n has found something in his forage attempt.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        improve_skill(ch, SKILL_FORAGE, 2);
        return;
    }
}




ACMD(do_spy)
{
    int percent, prob, spy_type, return_room;
    char *spy_dirs[] = {
                           "north",
                           "east",
                           "south",
                           "west",
                           "up",
                           "down",
                           "ne",
                           "nw",
                           "se",
                           "sw",
                           "\n"
                       };

    if (!GET_SKILL(ch, SKILL_SPY))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    } 
      one_argument(argument, arg);     
    if (!*arg)
    {
    	send_to_char("You need to specify a valid exit.\r\n", ch);
        return;
    }

    /* 101% is a complete failure */
    percent = number(1, 111);
    prob = GET_SKILL(ch, SKILL_SPY);
    spy_type = search_block(argument + 1, spy_dirs, FALSE);

    if (spy_type < 0 || !EXIT(ch, spy_type) || EXIT(ch, spy_type)->to_room == NOWHERE) {
        send_to_char("That's not a valid exit.\r\n", ch);
        return;
    }
    else {
        if (!(GET_MOVE(ch) >= 20)) {
            send_to_char("You are too exhausted too even try that now.\r\n", ch);
        }
        else {
            if (percent > prob) {
                send_to_char("You fail.\r\n", ch);
                GET_MOVE(ch) -=20;
            }
            else {
                if (IS_SET(EXIT(ch, spy_type)->exit_info, EX_CLOSED) && EXIT(ch, spy_type)->keyword) {
                    sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, spy_type)->keyword));
                    send_to_char(buf, ch);
                    GET_MOVE(ch) -=20;
                }
                else {
                    GET_MOVE(ch) -=20;
                    return_room = ch->in_room;
                    char_from_room(ch);
                    SET_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
                    char_to_room(ch, world[return_room].dir_option[spy_type]->to_room);
                    send_to_char("You sneak up and spy into the next room: \r\n\r\n", ch);
                    look_at_room(ch, 1);
                    spot_trap(ch, ch->in_room);
                    char_from_room(ch);
                    REMOVE_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
                    char_to_room(ch, return_room);
                    act("$n peeks into the next room.", TRUE, ch, 0, 0, TO_NOTVICT);
                    improve_skill(ch, SKILL_SPY, 1);
                }
            }
        }
    }
}




ACMD(do_plant)
{
    struct char_data *vict;
    struct obj_data *obj;
    char vict_name[240];
    char obj_name[240];
    int percent, gold, eq_pos, pcsteal = 0;
    extern int pt_allowed;
    bool ohoh = FALSE;
    ACMD(do_gen_comm);

    if (not_in_arena(ch))
        return;

    argument = one_argument(argument, obj_name);
    one_argument(argument, vict_name);

    if (!GET_SKILL(ch, SKILL_PLANT))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, vict_name))) {
        send_to_char("Plant what to whom?\r\n", ch);
        return;
    } else if (vict == ch) {
        send_to_char("Come on now, that's rather stupid!\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch,vict))
        pcsteal = 1;

    /* 101% is a complete failure */
    percent = number(1, 111) - GET_DEX(ch);

    if (GET_POS(vict) < POS_SLEEPING)
        percent = -1;		/* ALWAYS SUCCESS */

    /* NO NO With Imp's and Shopkeepers! */
    if ((GET_LEVEL(vict) >= LVL_IMMORT) || pcsteal ||
            GET_MOB_SPEC(vict) == shop_keeper)
        percent = 101;		/* Failure */

    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
    {
        send_to_char("You don't have that item.\r\n", ch);
        return;
    } else {		/* obj found in inventory */
        if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
            sprintf(buf, "You can't plant $p, it must be CURSED!");
            act(buf, FALSE, ch, obj, 0, TO_CHAR);
            return;
        }
        percent += GET_OBJ_WEIGHT(obj)/3;	/* Make heavy harder */

        if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_PLANT))) {
            ohoh = TRUE;
            act("Oops.. $E saw you try.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n tried to plant you something!", FALSE, ch, 0, vict, TO_VICT);
            act("$n tried to plant something to $N.", TRUE, ch, 0, vict, TO_NOTVICT);
        } else {		/* Steal the item */
            if ((IS_CARRYING_N(vict) + 1 < CAN_CARRY_N(vict))) {
                if ((IS_CARRYING_W(vict) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(vict)) {
                    obj_from_char(obj);
                    obj_to_char(obj, vict);
                    send_to_char("&cYou plant it, unnoticed.&0\r\n", ch);
                    improve_skill(ch, SKILL_PLANT, 1);
                }
            } else
                send_to_char("Your victim cannot carry that much.\r\n", ch);
        }
    }

    if (ohoh && IS_NPC(vict) && AWAKE(vict))
        hit(vict, ch, TYPE_UNDEFINED);
}




ACMD(do_guard)
{

    struct char_data *vict, *pom;

    one_argument(argument, arg);

    if (!*arg) {
        if (AFF3_FLAGGED(ch, AFF3_GUARD))
        {
            act("You stop guarding $N.", FALSE, ch, 0, ch->guarding, TO_CHAR);
            if (ch->in_room==ch->guarding->in_room)
            {
                act("$n stops guarding you.", FALSE, ch, 0, ch->guarding, TO_VICT);
                act("$n stops guarding $N.", FALSE, ch, 0, ch->guarding, TO_NOTVICT);
            }
            REMOVE_BIT(AFF3_FLAGS(ch), AFF3_GUARD);
            vict=ch->guarding;
            ch->guarding=NULL;
            if (!is_guarded(vict))
                REMOVE_BIT(AFF2_FLAGS(vict), AFF2_GUARDED);

        }
        else
            send_to_char("Guard who?\r\n", ch);
        return;
    }

    if (!GET_SKILL(ch, SKILL_GUARD))
    {
        send_to_char("You dont know how tou guard someone.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg)))
    {
        send_to_char("They aren't here.\r\n", ch);
        return;
    }
    else if (ch == vict)
    {
        if (AFF3_FLAGGED(ch, AFF3_GUARD))
        {
            act("You stop guarding $N.", FALSE, ch, 0, ch->guarding, TO_CHAR);
            if (ch->in_room==ch->guarding->in_room)
            {
                act("$n stops guarding you.", FALSE, ch, 0, ch->guarding, TO_VICT);
                act("$n stops guarding $N.", FALSE, ch, 0, ch->guarding, TO_NOTVICT);
            }
            REMOVE_BIT(AFF3_FLAGS(ch), AFF3_GUARD);
            vict=ch->guarding;
            ch->guarding=NULL;
            if (!is_guarded(vict))
                REMOVE_BIT(AFF2_FLAGS(vict), AFF2_GUARDED);
        }
        else
            send_to_char("You are already watching over yerself, pal.\r\n", ch);
        return;
    }


    /*if (!is_same_group(ch, vict))
{
    	send_to_char("You are not grouped with them.\r\n", ch);
    	return;
}
      */    


    if (vict->guarding==ch)
    {
        send_to_char("Guarding in loops is not allowed.\r\n", ch);
        return;
    }

    if (AFF3_FLAGGED(ch, AFF3_GUARD))
    {
        act("You stop guarding $N.", FALSE, ch, 0, ch->guarding, TO_CHAR);
        if (ch->in_room==ch->guarding->in_room)
        {
            act("$n stops guarding you.", FALSE, ch, 0, ch->guarding, TO_VICT);
            act("$n stops guarding $N.", FALSE, ch, 0, ch->guarding, TO_NOTVICT);
        }
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_GUARD);
        pom=ch->guarding;
        ch->guarding=NULL;
        if (!is_guarded(pom))
            REMOVE_BIT(AFF2_FLAGS(pom), AFF2_GUARDED);
    }



    SET_BIT(AFF3_FLAGS(ch), AFF3_GUARD);
    SET_BIT(AFF2_FLAGS(vict), AFF2_GUARDED);
    ch->guarding=vict;
    act("OK. Now guarding $N.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n guards you now.", FALSE, ch, 0, vict, TO_VICT);
    act("$n is guarding $N now.", FALSE, ch, 0, vict, TO_NOTVICT);



    /*struct affected_type af;
        struct char_data *tch,
                       *k;
        struct follow_type *f,
                       *f_next;
    int flag=0;

    if (!GET_SKILL(ch, SKILL_GUARD))
{
        send_to_char("You can barely guard self, let alone others.\r\n", ch);
        return;
}

    one_argument(argument, arg);

    if (*arg && isname("self", arg))
{
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_GUARD);
        send_to_char("You now watch after yourself.\r\n", ch);
        return;
}
        


        if (!IS_AFFECTED(ch, AFF_GROUP) && count_group(ch)<2)
        {
            send_to_char("No need for that cause you lead no group now.\r\n", ch);
            return;
        }

        if (ch->master != NULL)
        {
            send_to_char("You are not the leader of your group.\r\n", ch);
            return;
        }

        if (AFF3_FLAGGED(ch, AFF3_GUARD))
        {
        	send_to_char("You are already guarding your group.\r\n",ch);
        	return;
        }

        send_to_char("You spread your arms, and move all of your group members behind you.\r\n", ch);
            k = ch;
        for (f = k->followers; f; f = f_next) {
            f_next = f->next;
            tch = f->follower;
            if (tch->in_room != ch->in_room)
                continue;
            if (!is_same_group(ch, tch))
                continue;
            if (ch == tch)
                continue;
            act("$n spreads $s arms, and moves you behind $m.", FALSE, ch, NULL, tch, TO_VICT);
            act("$n moves $N behind $m.", FALSE, ch, NULL, tch, TO_NOTVICT);
        }
        SET_BIT(AFF3_FLAGS(ch), AFF3_GUARD);*/
}



ACMD(do_concentrate)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);



    if (!GET_SKILL(ch, SKILL_TRANSFER))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (GET_MANA(ch)==0)
    {
        send_to_char("You have no mana!\r\n", ch);
        return;
    }


    vict=ch;


    if (number(1, 111) <= GET_SKILL(ch, SKILL_TRANSFER)) {
        GET_HIT(vict) += GET_MANA(ch);
        GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict));
        GET_MANA(ch)=0;
        act("You close your eyes and deeply concentrate.\r\nA warm feeling floods your body.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n closes $s eyes and deeply concentrates.", FALSE, ch, 0, vict, TO_ROOM);
        improve_skill(ch, SKILL_TRANSFER, 2);
    } else
        send_to_char("You couldn't concentrate enough!\r\n", ch);
}


ACMD(do_retreat)
{


    if (!GET_SKILL(ch, SKILL_RETREAT))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }


    if (!FIGHTING(ch))
    {
        send_to_char("You are not even in a fight.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<5*PERCENT(ch))
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    GET_MOVE(ch)-=5*PERCENT(ch);

    if (number(1, 135)>(GET_SKILL(ch, SKILL_RETREAT)+2*GET_DEX(ch)))
    {
        send_to_char("You PANIC! You couldn't retreat!\r\n", ch);
        return;
    }

    do_flee(ch, "", 0, 0);

}


ACMD(do_autopsy)
{
    char abuf[1000], arg[MAX_STRING_LENGTH];
    struct obj_data *tobj, *o, *next_obj;
    argument = one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_AUTOPSY))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (!(tobj = get_obj_in_list_vis(ch, arg, ch->carrying)))
        tobj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents);


    if (!tobj)
    {
        send_to_char("You don't see that here.\r\n", ch);
        return;
    }
    if (GET_OBJ_RENT(tobj)!=9999)
    {
        send_to_char("That's not a fresh corpse.\r\n", ch);
        return;
    }

    send_to_char("You approach and perform an autopsy of the corpse.\r\n", ch);
    act("$n approaches $p, and performs an autopsy.", FALSE, ch, tobj, 0, TO_ROOM);

    if (number(1, 111)>GET_SKILL(ch, SKILL_AUTOPSY))
    {
        send_to_char("You completly splatter the corpse without finding any clues.\r\n", ch);
        act("$n completly splatters $p.", FALSE, ch, tobj, 0, TO_ROOM);
        for (o = tobj->contains; o; o = next_obj) {
            next_obj = o->next_content;
            obj_from_obj(o);
            obj_to_room(o, ch->in_room);
        }

        extract_obj(tobj);
        return;
    }
    if (tobj->action_description)
        sprintf(abuf, "After a while you conclude it was killed by %s.\r\n", tobj->action_description);
    else
        sprintf(abuf, "You couldn't discover any clues.\r\n", ch);
    send_to_char(abuf, ch);
}


ACMD(do_capture);
struct capture_eo
{
    struct char_data *ch;

};

EVENTFUNC(event_capture)
{
    struct capture_eo *cevent=(struct capture_eo *) event_obj;
    struct char_data *ch, *vict;


    ch=cevent->ch;


    if (!ch || DEAD(ch))
        goto kraj_capture;


    do_capture(ch, "", 0, SUB_EVENT);


kraj_capture:
    if (cevent)
        free (cevent);
    if (ch)
        GET_UTIL_EVENT(ch)=NULL;
    return 0;
}


void assign_capture_event(struct char_data *ch, int when)
{
    struct capture_eo *cevent;

    CREATE(cevent, struct capture_eo, 1);
    cevent->ch=ch;
    GET_UTIL_EVENT(ch)=event_create(event_capture, cevent, when);
}



extern int arena_red, arena_blue;
ACMD(do_capture)
{
    struct obj_data *flag;

    if (IN_EVENT(ch) && subcmd!=SUB_EVENT)
    {
        send_to_char("You are already doing something!\r\n", ch);
        return;
    }

    if (!IN_ARENA(ch))
    {
        send_to_char("This command is used only in arena to capture enemy flag in your base.\r\n", ch);
        return;
    }

    if (RED(ch))
    {

        if (subcmd==SUB_EVENT)
        {
            if (!ROOM_FLAGGED(ch->in_room, ROOM_RED_BASE) || !HAS_BLUE(ch))
            {
                //send_to_char("You abort the capture.\r\n", ch);
                return;
            }

        }

        if (!ROOM_FLAGGED(ch->in_room, ROOM_RED_BASE))
        {
            send_to_char("You must be in your base!\r\n", ch);
            return;
        }
        if (!HAS_BLUE(ch))
        {
            send_to_char("You aren't carrying BLUE FLAG!\r\n", ch);
            return;
        }

        if (subcmd!=SUB_EVENT)
        {

            sprintf(buf, "%s tries to capture enemy flag!", GET_NAME(ch));
            sportschan(buf);
            assign_capture_event(ch, 9*PULSE_VIOLENCE/2);
            return;
        }



        sprintf(buf, "%s (&RRED Team&0) captures enemy flag!", GET_NAME(ch));
        sportschan(buf);
        arena_red+=5;
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_HAS_BLUE);
        flag = read_object(FLAG_BLUE, VIRTUAL, 0, 0); 
        global_no_timer=1;
        obj_to_room(flag, arena_room());
        global_no_timer=0;
    }

    if (BLUE(ch)) {


        if (subcmd==SUB_EVENT)
        {
            if (!ROOM_FLAGGED(ch->in_room, ROOM_BLUE_BASE) || !HAS_RED(ch))
            {
                //send_to_char("You abort the capture.\r\n", ch);
                return;
            }

        }

        if (!ROOM_FLAGGED(ch->in_room, ROOM_BLUE_BASE))
        {
            send_to_char("You must be in your base!\r\n", ch);
            return;
        }

        if (!HAS_RED(ch))
        {
            send_to_char("You aren't carrying RED FLAG!\r\n", ch);
            return;
        }
        if (subcmd!=SUB_EVENT)
        {

            sprintf(buf, "%s tries to capture enemy flag!", GET_NAME(ch));
            sportschan(buf);
            assign_capture_event(ch, 9*PULSE_VIOLENCE/2);
            return;
        }

        sprintf(buf, "%s (&BBLUE Team&0) captures enemy flag!", GET_NAME(ch));
        sportschan(buf);
        arena_blue+=5;
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_HAS_RED);
        flag = read_object(FLAG_RED, VIRTUAL, 0, 0);
        global_no_timer=1;
        obj_to_room(flag, arena_room());
        global_no_timer=0;

    }
}

ACMD(do_boss)
{
    extern char *boss_screen;
    send_to_char(boss_screen, ch);
}

ACMD(do_style)
{
    int type=-1;
    char *styles[3]=
        {"normal", "evasive","aggresive"};
    char     arg[MAX_STRING_LENGTH];
    argument = one_argument(argument, arg);
    
    if (FOL_URG(ch))
    {
    	send_to_char("As Urg's follower, your fighting style is always set to aggressive.\r\n", ch);
    	SET_STYLE(ch, 2);
    	return;
    }
    if (!*arg)
    {
        sprintf(buf,"Your fighting style is &Y%s&0.\r\nUse 'style <normal | evasive | aggresive>' to change your style of fighting.\r\n",styles[GET_STYLE(ch)]);
        send_to_char(buf,ch);
        return;
    }
    else {
        if (isname(arg,"normal")) type=0;
        if (isname(arg,"evasive")) type=1;
        if (isname(arg,"aggresive")) type=2;
        if (type==-1)
        {
            sprintf(buf,"Your fighting style is &Y%s&0.\r\nUse 'style <normal | evasive | aggresive>' to change your style of fighting.\r\n",styles[GET_STYLE(ch)]);
            send_to_char(buf,ch);
            return;
        }
        if (type==1 && !GET_SKILL(ch, SKILL_EVASIVE))
        {
            send_to_char("You are not practiced enough in the art of evasive fighting.\r\n", ch);
            return;
        }
        if (type==2 && !GET_SKILL(ch, SKILL_AGGRESIVE))
        {
            send_to_char("You are not practiced enough in the art of aggresive fighting.\r\n", ch);
            return;
        }

        SET_STYLE(ch, type);
        sprintf(buf,"You change your fighting style to &Y%s&0.\r\n",styles[GET_STYLE(ch)]);
        if (FIGHTING(ch))
            act("$n changes his fighting style to &Y$T&0.", FALSE, ch,  NULL,styles[GET_STYLE(ch)], TO_ROOM);
        WAIT_STATE(ch, 1 RL_SEC);
        send_to_char(buf,ch);
    }
}


ACMD(do_wrap)
{
    char     arg[MAX_STRING_LENGTH];
    argument = one_argument(argument, arg);

    if (!*arg)
    {
        sprintf(buf, "Illegal usage. Use 'wrap <num>'.\r\nCurrent linewrap set to %d.\r\n", GET_LINEWRAP(ch));
        send_to_char(buf,ch);
        return;
    }
    GET_LINEWRAP(ch)=atoi(arg);
    if (atoi(arg)>=40)
        sprintf(buf,"Line wrap set at %d.\r\n", atoi(arg));
    else
        sprintf(buf,"Line wrap disabled.\r\n");
    send_to_char(buf, ch);


}


ACMD(do_cease)
{
    if (!FIGHTING(ch))
    {
        send_to_char("You are not fighting.\r\n", ch);
        return;
    }
    if (ch->in_room!=(FIGHTING(ch)->in_room))
    {
        send_to_char("You are not in the same room with the person you are fighitng.\r\n", ch);
        return;
    }

    if (FIGHTING(FIGHTING(ch))==ch)
    {
        send_to_char("Better try fleeing!\r\n", ch);
        return;
    }

    act("You stop attacking $N.", FALSE, ch, 0, FIGHTING(ch), TO_CHAR);
    act("$n stops attacking $N.", FALSE, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n stops attacking you.", FALSE, ch, 0, FIGHTING(ch), TO_VICT);

    stop_fighting(ch);
}


ACMD(do_walk)
{
    send_to_char("OK, you will walk now.\r\n", ch);
    REMOVE_BIT(PRF2_FLAGS(ch), PRF2_RUNNING);

}

ACMD(do_run)
{
    send_to_char("OK, you will run now.\r\n", ch);
    SET_BIT(PRF2_FLAGS(ch), PRF2_RUNNING);

}


int blessings[]=
    {
        SPELL_DIVINITY,
        SPELL_HOLY_ARMOR,
        SPELL_HASTE,
        SPELL_BENEDICTION,
        SPELL_REGENERATE,
        SPELL_STRENGTH,
        SPELL_BARKSKIN,
        SPELL_BLESS,
        SPELL_ARMOR
    };
int curses[]=
    {
        SPELL_SLEEP,
        SPELL_LIGHTNING_BOLT,
        SPELL_FEEBLEMIND,
        SPELL_BLINDNESS,
        SPELL_CURSE,
    };

#define NUM_BLESSINGS (sizeof(blessings)/sizeof(blessings[0]))
#define NUM_CURSES (sizeof(curses)/sizeof(curses[0]))

void sac_reward(struct char_data *ch, struct obj_data *obj)
{
    int level=((GET_OBJ_TYPE(obj)==ITEM_CONTAINER) && (GET_OBJ_VAL(obj, 3) ==1)?GET_OBJ_VAL(obj, 2):GET_OBJ_LEVEL(obj));
    int prob, ldiff;
    int ukup;
    int sum=0, a;


    prob=level-GET_LEVEL(ch);
    ldiff=prob;


    if (ldiff>-5)
    {
        prob+=number(1, 30);
        ch_printf(ch, "&CYou have been rewarded by %s.&0\r\n", DEITY_NAME(ch));

        if (prob>=29)
        {
            send_to_char("You are fully restored.\r\n", ch);
            GET_HIT(ch) = GET_MAX_HIT(ch);
            GET_MANA(ch) = GET_MAX_MANA(ch);
            GET_MOVE(ch) = GET_MAX_MOVE(ch);
            GET_COND(ch, DRUNK) = 0;
            GET_COND(ch, THIRST) = 24;
            GET_COND(ch, FULL) = 24;
            update_pos(ch);
        }
        else if (prob>=23)
        {
            //ch_printf(ch, "You feel different.\r\n", DEITY_NAME(ch));
            ukup=NUM_BLESSINGS*(NUM_BLESSINGS+1)/2;

            prob=MAX(1, MIN(ukup, number(1, ukup)+ldiff));

            for (a=NUM_BLESSINGS;a>0;a--)
            {
                sum+=a;
                if (sum>=prob)
                    break;
            }
            call_magic(supermob, ch, 0, blessings[a-1], 54, CAST_SCROLL, 0);
        }
        else if (prob>=19)
        {
            send_to_char("You are refreshed.\r\n", ch);
            GET_MOVE(ch) = GET_MAX_MOVE(ch);
            GET_COND(ch, THIRST) = 24;
            GET_COND(ch, FULL) = 24;
        }
        else// if (prob>15)
        {   
            	
            send_to_char("You feel better.\r\n", ch);
            GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch)+GET_MAX_HIT(ch)/(((21-prob)/4)+4));
            GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch), GET_MOVE(ch)+GET_MAX_MOVE(ch)/(((21-prob)/4)+4));
            GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch)+GET_MAX_MANA(ch)/(((21-prob)/4)+4));
        }
    }
    else if (!number(0, 2))
    {
        ch_printf(ch,  "&R%s is angry with your sacrifice.&0\r\n", DEITY_NAME(ch));
        ukup=NUM_CURSES*(NUM_CURSES+1)/2;

        prob=MAX(1, MIN(ukup, number(1, ukup)-ldiff/5));

        for (a=NUM_CURSES;a>0;a--)
        {
            sum+=a;
            if (sum>=prob)
                break;
        }
        if (curses[a-1]==SPELL_LIGHTNING_BOLT)
        {
            GET_HIT(ch)-=number(50, GET_MAX_HIT(ch)/2);
            send_to_char("&WYou are struck by heavenly BOLT of LIGHTNING!&0\r\n", ch);
            act("$n is suddenly struck by BOLT of LIGHTNING!", FALSE, ch, 0, 0, TO_ROOM);
            sprintf(buf, "the wrath of %s", DEITY_NAME(ch));
            check_kill(ch, "the wrath of his deity");
        }
        else
            call_magic(supermob, ch, 0, curses[a-1], 54, CAST_SCROLL, 0);
    }


}









ACMD(do_sac)
{
    int ldiff;
    struct obj_data *obj;
    struct obj_data *corpse,
                *o,
                *next_obj;

    one_argument(argument, arg);

    // note, I like to take care of no arg and wrong args up front, not
    // at the end of a function, lets get the wrongness out of the way :)
    if (!*arg)
{
        send_to_char(" What do you wish to sacrifice? \r\n",ch);
        return;
    }

    // if it's not in the room, we ain't gonna sac it
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)))
    {
        if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying)))
        {
            send_to_char("You do not see that here.\r\n",ch);
            return;}
    }
    if (GET_OBJ_TYPE(obj)==ITEM_FOUNTAIN || !IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_TAKE))
    {
        send_to_char("You can't sacrifice that!\r\n",ch);
        return;                                                                                                                                  
    }
    else if (IS_OBJ_STAT(obj, ITEM_NODROP))
    {
            act("You can't sacrifice $p, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
            return;
    }
    if ( GET_OBJ_TYPE(obj)==ITEM_CONTAINER && (GET_OBJ_VAL(obj, 3)==1) && obj->orig_value && IS_NPC(ch) && strcmp(GET_NAME(ch), obj->attack_verb))
    {
    		send_to_char("This corpse is protected by divine powers.\r\n", ch);
    		return;
    }
        
    // seems as if everything checks out eh? ok now do it
    act("$n sacrifices $p.", FALSE, ch, obj, 0, TO_ROOM);
    act("You sacrifice $p to $T.",FALSE, ch, obj, GET_DEITY(ch)?deity_list[GET_DEITY(ch)].name:"your god", TO_CHAR);

    // nifty, got the object in the room, now check its flags
    if ((GET_OBJ_TYPE(obj)!=ITEM_CONTAINER) || (GET_OBJ_VAL(obj, 3) !=1))
    {


    }

    else{ 
    
      /*ldiff=2*(GET_OBJ_VAL(obj, 2)-GET_LEVEL(ch));
        if (ldiff>0)
            ldiff*=2;*/

        for (o = obj->contains; o; o = next_obj) {
            next_obj = o->next_content;
            obj_from_obj(o);
            obj_to_room(o, ch->in_room);
        }
    }
    {
    	ldiff=((GET_OBJ_TYPE(obj)==ITEM_CONTAINER) && (GET_OBJ_VAL(obj, 3) ==1)?GET_OBJ_LEVEL(obj)-GET_LEVEL(ch)+4:(GET_OBJ_LEVEL(obj)-GET_LEVEL(ch)+4)/2);    	
    	
        SET_FAITH(ch, GET_FAITH(ch)+ldiff);
        if (FOL_MUGRAK(ch) && (GET_OBJ_TYPE(obj)==ITEM_CONTAINER) && (GET_OBJ_VAL(obj, 3) ==1) && ldiff>0)
        	GET_HIT(ch)+=ldiff*GET_LEVEL(ch);
        if (ldiff>5)                                                                                                          
        	ch_printf(ch, "%s is very pleased with your sacrifice.\r\n", deity_list[GET_DEITY(ch)].name);
        else if (ldiff>=0)
        	ch_printf(ch, "%s is pleased with your sacrifice.\r\n", deity_list[GET_DEITY(ch)].name);
        else if (ldiff<0)
        	ch_printf(ch, "%s is not pleased with your sacrifice.\r\n", deity_list[GET_DEITY(ch)].name);
     }
    
    
    //if (!FOL_SKIG(ch))
    if (number(1, 320)<(ldiff*ldiff))
        sac_reward(ch, obj);
      
    oprog_sac_trigger( ch, obj );
    if (!PURGED(obj))
        extract_obj(obj);
}







ACMD(do_eavesdrop) {
    int dir;
    int here=0;
    one_argument(argument, buf);
    
            
            if (!GET_SKILL(ch, SKILL_EAVESDROP))
            {
            	send_to_char("You don't know that skill.\r\n", ch);
            	return;
            }
    if (!*buf) { 
    	if (ch->listening_to)
    	{
    		struct char_data *temp;
        	REMOVE_FROM_LIST(ch, world[ch->listening_to].listeners, next_listener);
        	ch->listening_to = 0;
        	send_to_char("You stop listening.\r\n", ch);
        	return;
        }
    		
        send_to_char("In which direction would you like to eavesdrop?\r\n", ch);
        return;
    }          
    if (isname(buf, "here"))
    	here=1;
    	
    else if ((dir = search_block(buf, dirs, FALSE)) < 0) { 
    	
        send_to_char("Which directions is that?\r\n", ch);
        return;
    }
    if (here)
    {
    	      my_srandp(GET_ROOM_VNUM(ch->in_room));
            if (number(1, 101)> GET_SKILL(ch, SKILL_EAVESDROP)) {
                send_to_char("You try to eavesdrop here, but are unable.\r\n", ch);
                my_srand(rand());
                return;
            }   
            	if (ch->listening_to)
    		{
    		struct char_data *temp;
        	REMOVE_FROM_LIST(ch, world[ch->listening_to].listeners, next_listener);
        	ch->listening_to = 0;
        	}
            my_srand(rand());
            ch->next_listener = world[ch->in_room].listeners;
            world[ch->in_room].listeners = ch;
            ch->listening_to = ch->in_room;
            send_to_char(OK, ch);
    }
else
    if (EXIT(ch, dir)) {
        if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) && EXIT(ch, dir)->keyword) {
            sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
            send_to_char(buf, ch);
        } else {
            my_srandp(GET_ROOM_VNUM(EXIT(ch, dir)->to_room));
            if (number(1, 101)> GET_SKILL(ch, SKILL_EAVESDROP)) {
                send_to_char("You try to eavesdrop on that room, but are unable.\r\n", ch);
                my_srand(rand());
                return;
            }
            if (ch->listening_to)
    		{
    		struct char_data *temp;
        	REMOVE_FROM_LIST(ch, world[ch->listening_to].listeners, next_listener);
        	ch->listening_to = 0;
        	}
            my_srand(rand());
            ch->next_listener = world[EXIT(ch, dir)->to_room].listeners;
            world[EXIT(ch, dir)->to_room].listeners = ch;
            ch->listening_to = EXIT(ch, dir)->to_room;
            send_to_char(OK, ch);
        }
    } else
        send_to_char("There is no room there...\r\n", ch);
}


ACMD(do_envenom)
{

    struct obj_data *weapon = NULL;
    struct obj_data *obj, *next_obj;
    char weapon_name[MAX_STRING_LENGTH];
    int found = FALSE, prob = 0, dam = 0;


	two_arguments(argument, weapon_name, buf2);

    if (!GET_SKILL(ch, SKILL_ENVENOM)) {
        send_to_char("You have no idea how to envenom weapons!\r\n", ch);
        return;
    }
    if (FIGHTING(ch))
    {
        send_to_char("You are in a middle of a combat now.\r\n", ch);
        return;
    }

    if (!*weapon_name) {
        send_to_char("What do you want to envenom?\r\n", ch);
        return;
    }

    if ((obj=GET_EQ(ch, WEAR_WIELD)) && isname(weapon_name, obj->name))
    {
        weapon=obj;
        found = TRUE;
    }
    else if ((obj=GET_EQ(ch, WEAR_DUALWIELD)) && isname(weapon_name, obj->name))
    {
        weapon=obj;
        found = TRUE;
    }
    /* else for (obj = ch->carrying; obj; obj = next_obj) {
    next_obj = obj->next_content;
    if (obj == NULL)
      return;
    else if (!(weapon = get_obj_in_list_vis(ch, weapon_name,
    			ch->carrying)))
      continue;
    else
      found = TRUE;
     }*/


    else {found=TRUE;
        if (!(weapon = get_obj_in_list_vis(ch, weapon_name, ch->carrying))) {
            if (!(weapon = get_obj_in_list_vis(ch, weapon_name, world[ch->in_room].contents))) {
                act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
                return;
            }
        }
    }



    if (found == FALSE) {
        act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }



    if (found && (GET_OBJ_TYPE(weapon) != ITEM_WEAPON)) {
        if ((GET_OBJ_TYPE(weapon) != ITEM_DRINKCON) && GET_OBJ_TYPE(weapon) != ITEM_FOOD)
        {
            send_to_char("You can't envenom that!\r\n", ch);
            return;
        }
        if (number(1, 111)<GET_SKILL(ch, SKILL_ENVENOM))
        {
            GET_OBJ_VAL(weapon, 3)=1;
            improve_skill(ch, SKILL_ENVENOM, 2);

            act("&yYou envenom $p.&0", FALSE, ch, weapon, NULL, TO_CHAR);
        }
        else
            send_to_char("You fail.\r\n", ch);
        WAIT_STATE(ch, 1 RL_SEC);
        return;
    }

    if ((GET_OBJ_VAL(weapon, 3) != TYPE_SLASH - TYPE_HIT) && (GET_OBJ_VAL(weapon, 3) != TYPE_PIERCE - TYPE_HIT)) {
        sprintf(buf, "Only slashing and piercing weapons may be envenomed.\r\n");
        send_to_char(buf, ch);
        return;
    }
    /* determine success probability */

   // my_srandp(GET_OBJ_VNUM(weapon));
    prob=MIN(110, GET_OBJ_LEVEL(weapon)*2);//number(1, 105);//
   // my_srand(time(0));

    if (prob>GET_SKILL(ch, SKILL_ENVENOM)) {
        send_to_char("You fail.\r\n", ch);
        return;
    }
    SET_BIT(GET_OBJ_EXTRA2(weapon), ITEM2_POISONED);


    act("\r\n&yYou skillfully apply poison to $p's blade.&0",FALSE,  ch, weapon, NULL, TO_CHAR);


    act("$n covers $p's blade with something.",TRUE, ch, weapon, 0, TO_ROOM);
    improve_skill(ch, SKILL_ENVENOM, 1);
    WAIT_STATE(ch, 1 RL_SEC);
}



ACMD(do_disguise)
{

    if (!GET_SKILL(ch, SKILL_DISGUISE))
    {
        send_to_char("You dont know how to do that.\r\n", ch);
        return;
    }

    skip_spaces(&argument);

    if (!*argument)
    {
        if (ch->player.long_descr)
        {
            send_to_char("You stop disguising.\r\n", ch);
            act("$n appears, removing $s disguise.", FALSE, ch, 0, 0, TO_ROOM);
            STRFREE(ch->player.long_descr);
        }
        else
            send_to_char("Usage: Disguise <text>\r\n", ch);
        return;
    }


    if (strlen(argument)>70)
    {
        send_to_char("Text too long.\r\n", ch);
        return;
    }
    if (strlen(argument)<10)
    {
        send_to_char("Text too short.\r\n", ch);
        return;
    }


    //	strcat(argument, "\r\n");
    if (ch->player.long_descr)
    {
        STRFREE(ch->player.long_descr);
        sprintf(buf, "You change your disguise to:\r\n\r\n%s\r\n", argument);
        send_to_char(buf, ch);
    }
    else
    {
        sprintf(buf, "You disguise to:\r\n\r\n%s\r\n", argument);
        send_to_char(buf, ch);
    }

    sprintf(buf, "%s\r\n", argument);
    ch->player.long_descr=str_dup(buf);
    act("$n disguises $mself.", FALSE, ch, 0, 0, TO_ROOM);
}

ACMD(do_conceal)
{
    if (!GET_SKILL(ch, SKILL_PLANT))
        send_to_char("You dont know how to do that.\r\n", ch);
    else
    {
        TOGGLE_BIT(PRF2_FLAGS(ch), PRF2_CONCEAL);
        sprintf(buf, "Ok. Conceal mode turned %s.\r\n", ONOFF(PRF2_FLAGGED(ch, PRF2_CONCEAL)));
        send_to_char(buf, ch);
    }

}

ACMD(do_tumble)
{
    if (!GET_SKILL(ch, SKILL_TUMBLE))
        send_to_char("You dont know how to do that.\r\n", ch);
    else
    {
        TOGGLE_BIT(PRF2_FLAGS(ch), PRF2_TUMBLE);
        if (PRF2_FLAGGED(ch, PRF2_TUMBLE))
            sprintf(buf, "Ok. You will now use tumble.\r\n");
        else
            sprintf(buf, "Ok. You will no longer use tumble.\r\n");
        send_to_char(buf, ch);
    }
}


ACMD(do_abort)
{
    if (!IN_EVENT(ch))
        send_to_char("You are not performing any action.\r\n", ch);
    else
    {
        send_to_char("&cAborting current action.&0\r\n", ch);
        if (GET_UTIL_EVENT(ch)) {
            event_cancel(GET_UTIL_EVENT(ch));
            GET_UTIL_EVENT(ch) = NULL;
        }
        if (GET_WEAR_EVENT(ch)) {
            event_cancel(GET_WEAR_EVENT(ch));
            GET_WEAR_EVENT(ch) = NULL;
        }
        if (GET_MOVE_EVENT(ch)) {
            event_cancel(GET_MOVE_EVENT(ch));
            GET_MOVE_EVENT(ch) = NULL;
        }
        if (GET_SPELL_EVENT(ch)) {
            event_cancel(GET_SPELL_EVENT(ch));
            GET_SPELL_EVENT(ch) = NULL;
        }

    }
}


/*
 * teleport a character to another room
 */
void teleportch( CHAR_DATA *ch, room_num room, bool show )
{
    char buf[MAX_STRING_LENGTH];

    //    if ( room_is_private( room ) )
    //    return;
    act( "$n disappears suddenly!",FALSE, ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    act( "$n arrives suddenly!", FALSE,ch, NULL, NULL, TO_ROOM );
    if ( show )
        look_at_room( ch, 1 );
    if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT) {
        char buf2[200];
        log_death_trap(ch);
        sprintf(buf2, "\r\n&BINFO || &C%s&c hits the death trap.&0\r\n", GET_NAME(ch));
        INFO_OUT(buf2);

        die(ch, NULL);
    }
}

void teleport( CHAR_DATA *ch, sh_int room, int flags )
{
    CHAR_DATA *nch, *nch_next;
    room_num dest;
    bool show;

    dest = real_room( room );
    if ( dest<0 )
    {
        logs( "SYSERR: teleport: bad room vnum %d", room );
        return;
    }

    if ( IS_SET( flags, TELE_SHOWDESC ) )
        show = TRUE;
    else
        show = FALSE;
    if ( !IS_SET( flags, TELE_TRANSALL ) )
    {
        teleportch( ch, dest, show );
        return;
    }

    /* teleport everybody in the room */
    for ( nch = world[ch->in_room].people; nch; nch = nch_next )
    {
        nch_next = nch->next_in_room;
        teleportch( nch, dest, show );
    }

    /* teleport the objects on the ground too */
    if ( IS_SET( flags, TELE_TRANSALLPLUS ) )
    {
        OBJ_DATA *obj, *obj_next;
        for ( obj = world[ch->in_room].contents; obj; obj = obj_next )
        {
            obj_next = obj->next_content;
            obj_from_room(obj);
            obj_to_room(obj, dest);
        }
    }
}


/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( room_num pRoomIndex, room_num to_room, sh_int door )
{

    EXIT_DATA *pexit, *texit;
    bool broke;

    CREATE( pexit, EXIT_DATA, 1 );
    pexit->to_room		= to_room;
    if ( to_room )
        pexit->to_room_vnum = GET_ROOM_VNUM(to_room);


    if (world[pRoomIndex].dir_option[door])
    {
        logs("SYSERR: exit alreadt existing while trying to build a new one room %d", pexit->to_room_vnum);
        extract_exit(pRoomIndex, world[pRoomIndex].dir_option[door], door);
    }
    SET_BIT( pexit->exit_info, EX_PASSAGE);
    world[pRoomIndex].dir_option[door]=pexit;

    CREATE( texit, EXIT_DATA, 1 );
    texit->to_room		= pRoomIndex;
    //if ( to_room )
    //   pexit->to_room_vnum = GET_ROOM_VNUM(to_room);

    door=rev_dir[door];
    if (world[to_room].dir_option[door])
    {
        logs("SYSERR: exit alreadt existing while trying to build a new one room %d", pexit->to_room_vnum);
        extract_exit(to_room, world[to_room].dir_option[door], door);
    }
    SET_BIT( texit->exit_info, EX_PASSAGE);
    world[to_room].dir_option[door]=texit;

    return pexit;

}

void extract_exit(room_num room, EXIT_DATA *pexit, int dir )
{   room_num to_room;
    EXIT_DATA *texit;
    if (pexit)
    {

        if (to_room>-1)
        {
            to_room=pexit->to_room;
            texit=world[to_room].dir_option[rev_dir[dir]];
            if (!texit)
            {
                logs("SYSERR: no exit2 to extract_exit()");
            }
            else
            {
                if (texit->keyword )
                    STRFREE( texit->keyword );
                if (texit->general_description )
                    STRFREE( texit->general_description );
                DISPOSE( texit );
                if (room>-1)
                    world[to_room].dir_option[rev_dir[dir]]=NULL;
            }
        }



        if (pexit->keyword )
            STRFREE( pexit->keyword );
        if (pexit->general_description )
            STRFREE( pexit->general_description );
        DISPOSE( pexit );
        if (room>-1)
            world[room].dir_option[dir]=NULL;




    }
    else logs("SYSERR: no exit to extract_exit()");

}
void pullorpush( CHAR_DATA *ch, OBJ_DATA *obj, bool pull )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA		*rch;
    bool		 isup;
    room_num room,  to_room;
    EXIT_DATA		*pexit, *pexit_rev;
    int			 edir;
    char		*txt;

    if ( IS_SET( obj->obj_flags.value[0], TRIG_UP ) )
        isup = TRUE;
    else
        isup = FALSE;
    switch( GET_OBJ_TYPE(obj) )
    {
    default:
        sprintf( buf, "You can't %s that!\n\r", pull ? "pull" : "push" );
        send_to_char( buf, ch );
        return;
        break;

    case ITEM_LEVER:

        if ( (!pull && isup) || (pull && !isup) )
        {
            sprintf( buf, "It is already %s.\n\r", isup ? "up" : "down" );
            send_to_char( buf, ch );
            return;
        }
    case ITEM_BUTTON:
        if ( (!pull && isup) || (pull && !isup) )
        {
            sprintf( buf, "It is already %s.\n\r", isup ? "in" : "out" );
            send_to_char( buf, ch );
            return;
        }
        break;
    }
    if( (pull) && HAS_OBJ_PROG(obj,PULL_PROG) )
    {
        if ( !IS_SET(obj->obj_flags.value[0], TRIG_AUTORETURN ) )
            REMOVE_BIT( obj->obj_flags.value[0], TRIG_UP );
        oprog_pull_trigger( ch, obj );
        return;
    }
    if( (!pull) && HAS_OBJ_PROG(obj,PUSH_PROG) )
    {
        if ( !IS_SET(obj->obj_flags.value[0], TRIG_AUTORETURN ) )
            SET_BIT( obj->obj_flags.value[0], TRIG_UP );
        oprog_push_trigger( ch, obj );
        return;
    }

    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
        sprintf( buf, "$n %s $p.", pull ? "pulls" : "pushes" );
        act(  buf, FALSE,  ch, obj, NULL, TO_ROOM );
        sprintf( buf, "You %s $p.", pull ? "pull" : "push" );
        act(  buf,FALSE, ch, obj, NULL, TO_CHAR );
    }

    if ( !IS_SET(obj->obj_flags.value[0], TRIG_AUTORETURN ) )
    {
        if ( pull )
            REMOVE_BIT( obj->obj_flags.value[0], TRIG_UP );
        else
            SET_BIT( obj->obj_flags.value[0], TRIG_UP );
    }
    if ( IS_SET( obj->obj_flags.value[0], TRIG_TELEPORT )
            ||   IS_SET( obj->obj_flags.value[0], TRIG_TELEPORTALL )
            ||   IS_SET( obj->obj_flags.value[0], TRIG_TELEPORTPLUS ) )
    {
        int flags;

        if ( ( room = real_room(obj->obj_flags.value[1] ) ) == -1 )
        {
            logs( "ERROR: PullOrPush: obj points to invalid room %d", obj->obj_flags.value[1] );
            return;
        }
        flags = 0;
        if ( IS_SET( obj->obj_flags.value[0], TRIG_SHOWROOMDESC ) )
            SET_BIT( flags, TELE_SHOWDESC );
        if ( IS_SET( obj->obj_flags.value[0], TRIG_TELEPORTALL ) )
            SET_BIT( flags, TELE_TRANSALL );
        if ( IS_SET( obj->obj_flags.value[0], TRIG_TELEPORTPLUS ) )
            SET_BIT( flags, TELE_TRANSALLPLUS );

        teleport( ch, obj->obj_flags.value[1], flags );
        return;
    }

    if ( IS_SET( obj->obj_flags.value[0], TRIG_RAND4 )
            ||	 IS_SET( obj->obj_flags.value[0], TRIG_RAND6 ) )
    {
        int maxd;

        if ( ( room = real_room( obj->obj_flags.value[1] ) ) ==-1 )
        {
            logs( "ERROR: PullOrPush: obj points to invalid room %d", obj->obj_flags.value[1] );
            return;
        }

        if ( IS_SET( obj->obj_flags.value[0], TRIG_RAND4 ) )
            maxd = 3;
        else
            maxd = 5;

        //randomize_exits( room, maxd );
        for ( rch = world[ch->in_room].people; rch; rch = rch->next_in_room )
        {
            if (AWAKE(ch))
            {
                send_to_char( "You hear a loud rumbling sound.\n\r", rch );
                send_to_char( "You feel dizzy...\n\r", rch );
                GET_POS(ch)=POS_SLEEPING;
                GET_COND(ch, DRUNK)=13;

            }
        }
    }

    if ( IS_SET( obj->obj_flags.value[0], TRIG_DOOR ) )
    {
        room = real_room( obj->obj_flags.value[1] );
        if ( !room )
            room = obj->in_room;
        if ( !room )
        {
            logs( "SYSERR: PullOrPush: obj points to invalid room %d", obj->obj_flags.value[1] );
            return;
        }
        if ( IS_SET( obj->obj_flags.value[0], TRIG_D_NORTH ) )
        {
            edir = NORTH;
            strcpy(txt,"to the north");
        }
        else
            if ( IS_SET( obj->obj_flags.value[0], TRIG_D_SOUTH ) )
            {
                edir = SOUTH;
                strcpy(txt,"to the south");
            }
            else
                if ( IS_SET( obj->obj_flags.value[0], TRIG_D_EAST ) )
                {
                    edir = EAST;
                    strcpy(txt, "to the east");
                }
                else
                    if ( IS_SET( obj->obj_flags.value[0], TRIG_D_WEST ) )
                    {
                        edir = WEST;
                        strcpy(txt,"to the west");
                    }
                    else
                        if ( IS_SET( obj->obj_flags.value[0], TRIG_D_UP ) )
                        {
                            edir = UP;
                            strcpy(txt,"from above");
                        }
                        else
                            if ( IS_SET( obj->obj_flags.value[0], TRIG_D_DOWN ) )
                            {
                                edir = DOWN;
                                strcpy(txt,"from below");
                            }
                            else
                            {
                                logs( "ERROR: PullOrPush: door: no direction flag set.", 0 );
                                return;
                            }
        pexit = EXITN(room, edir);
        if ( !pexit )
        {
            if ( !IS_SET( obj->obj_flags.value[0], TRIG_PASSAGE ) )
            {
                logs( "ERROR: PullOrPush: obj points to non-exit %d", obj->obj_flags.value[1] );
                return;
            }
            to_room = real_room( obj->obj_flags.value[2] );
            if ( !to_room )
            {
                logs( "ERROR: PullOrPush: dest points to invalid room %d", obj->obj_flags.value[2] );
                return;
            }
            pexit = make_exit( room, to_room, edir );
            pexit->keyword	= NULL;
            pexit->general_description	= NULL;
            pexit->key		= -1;
            pexit->exit_info	= 0;

            act("A passage opens!",FALSE,  ch, NULL, NULL, TO_CHAR );
            act(  "A passage opens!",FALSE, ch, NULL, NULL, TO_ROOM );
            return;
        }
        if ( IS_SET( obj->obj_flags.value[0], TRIG_UNLOCK )
                &&   IS_SET( pexit->exit_info, EX_LOCKED) )
        {
            REMOVE_BIT(pexit->exit_info, EX_LOCKED);
            act( "You hear a faint click $T.",FALSE, ch, NULL, txt, TO_CHAR );
            act( "You hear a faint click $T.",FALSE, ch, NULL, txt, TO_ROOM );
            pexit_rev = world[pexit->to_room].dir_option[rev_dir[edir]];
            if (pexit_rev && pexit_rev->to_room == ch->in_room )
                REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
            return;
        }
        if ( IS_SET( obj->obj_flags.value[0], TRIG_LOCK   )
                &&  !IS_SET( pexit->exit_info, EX_LOCKED) )
        {
            SET_BIT(pexit->exit_info, EX_LOCKED);
            act( "You hear a faint click $T.", FALSE,ch, NULL, txt, TO_CHAR );
            act( "You hear a faint click $T.", FALSE,ch, NULL, txt, TO_ROOM );
            pexit_rev = world[pexit->to_room].dir_option[rev_dir[edir]];
            if (pexit_rev && pexit_rev->to_room == ch->in_room )
                SET_BIT( pexit_rev->exit_info, EX_LOCKED );
            return;
        }
        if ( IS_SET( obj->obj_flags.value[0], TRIG_OPEN   )
                &&   IS_SET( pexit->exit_info, EX_CLOSED) )
        {
            REMOVE_BIT(pexit->exit_info, EX_CLOSED);
            sprintf(buf, "The %s opens.", pexit->keyword?pexit->keyword:"door");
            act(buf, FALSE, ch, NULL, NULL, TO_CHAR);
            act(buf, FALSE, ch, NULL, NULL, TO_ROOM);
            pexit_rev = world[pexit->to_room].dir_option[rev_dir[edir]];
            if (pexit_rev && pexit_rev->to_room == ch->in_room )
            {
                REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
                sprintf(buf, "The %s opens.", pexit->keyword?pexit->keyword:"door");
                if (world[pexit->to_room].people)
                {
                    act(buf, FALSE, world[pexit->to_room].people, NULL, NULL, TO_CHAR);
                    act(buf, FALSE, world[pexit->to_room].people, NULL, NULL, TO_ROOM);
                }
            }

            return;
        }
        if ( IS_SET( obj->obj_flags.value[0], TRIG_CLOSE   )
                &&  !IS_SET( pexit->exit_info, EX_CLOSED) )
        {
            SET_BIT(pexit->exit_info, EX_CLOSED);
            sprintf(buf, "The %s closes.", pexit->keyword?pexit->keyword:"door");
            act(buf, FALSE, ch, NULL, NULL, TO_CHAR);
            act(buf, FALSE, ch, NULL, NULL, TO_ROOM);
            pexit_rev = world[pexit->to_room].dir_option[rev_dir[edir]];
            if (pexit_rev && pexit_rev->to_room == ch->in_room )
            {
                SET_BIT( pexit_rev->exit_info, EX_CLOSED );
                sprintf(buf, "The %s closes.", pexit->keyword?pexit->keyword:"door");
                if (world[pexit->to_room].people)
                {
                    act(buf, FALSE, world[pexit->to_room].people, NULL, NULL, TO_CHAR);
                    act(buf, FALSE, world[pexit->to_room].people, NULL, NULL, TO_ROOM);
                }
            }

            return;
        }
    }
}

char *ppmodes[3]=
    {
        "push",
        "pull",
        "drag"
    };

void do_push_drag( CHAR_DATA *ch, char *argument, char *verb1, int pull )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    // char buf[MAX_STRING_LENGTH];
    room_num in_room;
    room_num to_room;
    CHAR_DATA *victim=NULL;
    struct room_direction_data *pexit=NULL;
    OBJ_DATA *obj=NULL;
    int door;
    char verb[100];
    strcpy(verb, verb1);


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        if (!pull)
            sprintf( buf, "%s whom or what?\n\r", CAP(verb));
        else
            sprintf(buf,"Pull what?\r\n");
        send_to_char( buf, ch );
        return;
    }
    if (!(victim = get_char_room(arg1, ch->in_room)))
        obj = get_obj_in_list_vis( ch, arg1, world[ch->in_room].contents );


    if ( (!victim || !CAN_SEE(ch,victim))
            && (!obj || !CAN_SEE_OBJ(ch,obj)) )
    {
        if (!pull)
            sprintf(buf,"%s whom or what?\n\r", CAP(verb));
        else
            sprintf(buf,"Pull what?\r\n");
        send_to_char( buf, ch );
        return;
    }
    if (obj && (GET_OBJ_TYPE(obj)==ITEM_LEVER || GET_OBJ_TYPE(obj)==ITEM_BUTTON))
    {
        pullorpush(ch, obj, pull);
        return;
    }
    if ( arg2[0] == '\0' )
    {
        if (!pull)
            sprintf( buf, "%s where?\n\r", CAP(verb));
        else
            sprintf(buf,"Pull what?\r\n");
        send_to_char( buf, ch );
        return;
    }
    if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = NORTH;
    else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = EAST;
    else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = SOUTH;
    else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = WEST;
    else if ( !str_cmp( arg2, "ne" ) || !str_cmp( arg2, "north-east" ) ) door = NORTHEAST;
    else if ( !str_cmp( arg2, "nw" ) || !str_cmp( arg2, "north-west"  ) ) door = NORTHWEST;
    else if ( !str_cmp( arg2, "se" ) || !str_cmp( arg2, "south-east" ) ) door = SOUTHEAST;
    else if ( !str_cmp( arg2, "sw" ) || !str_cmp( arg2, "south-west"  ) ) door = SOUTHWEST;
    else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = UP;
    else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = DOWN;
    else
    {
        sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
        send_to_char( buf, ch );
        return;
    }

    if ( obj )
    {

        if (FIGHTING(ch))
        {
            send_to_char("You can not do that while fighting!\r\n", ch);
            return;
        }

        in_room = ch->in_room;
        pexit   = world[in_room].dir_option[door];
        if ( !pexit
                ||   (( to_room = pexit->to_room   ) == 0 ))
        {
            sprintf( buf, "Alas, you cannot %s in that direction.\n\r", verb );
            send_to_char( buf, ch );
            return;
        }

        if ( !IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_TAKE))
        {
            send_to_char( "It won't budge.\n\r", ch );
            return;
        }

        if ( IS_SET(pexit->exit_info, EX_CLOSED))
        {
            act( "The $T is closed!", FALSE, ch, NULL, pexit->keyword, TO_CHAR );
            act( "$n decides to move $p around!", FALSE, ch, obj,NULL, TO_ROOM );
            return;
        }

        //act( "You attempt to $T $p out of the room.", FALSE, ch, obj, verb, TO_CHAR );
        //act( "$n is attempting to $T $p out of the room.", FALSE, ch, obj, verb, TO_ROOM );

        if ( GET_OBJ_WEIGHT(obj) >  (2 * CAN_CARRY_W(ch)) )
        {
            act( "$p is too heavy to $T.\n\r", FALSE, ch, obj, verb, TO_CHAR);
            act( "$n attempts to move $p, but it is too heavy.\n\r", FALSE, ch, obj, NULL, TO_ROOM);
            return;
        }
        /*if 	 ( !IS_IMMORTAL(ch) || ROOM_FLAGGED(world[to_room], ROOM_PRIVATE))
    {
        send_to_char( "It won't budge.\n\r", ch );
        return;
    } */

        if (GET_MOVE(ch) > 10 )
        {

            if (!str_cmp( verb, "drag" ))
            {
                if (perform_move(ch, door, 0))
                {
                  //  sprintf(buf, "%s drags $p to the $T.", CAP(GET_NAME(ch)));
                    act( "$n drags $p to the $T.", FALSE, ch, obj, dirs[door], TO_ROOM );
                    act("You drag $p with you.", FALSE, ch, obj, 0, TO_CHAR );
                    obj_from_room( obj );
                    obj_to_room( obj, ch->in_room );
                    GET_MOVE(ch)-= 10;
                }
            }
            else
            {
                act( "$n pushes $p to the $T.", FALSE, ch, obj, dirs[door], TO_ROOM );
                act("You push $p to the $T.", FALSE, ch, obj, dirs[door], TO_CHAR );
                obj_from_room( obj );
                obj_to_room( obj, to_room );
                GET_MOVE(ch)-= 10;
            }
        }

        else
        {
            sprintf( buf, "You are too tired to %s anything around!\n\r", verb );
            send_to_char( buf, ch );
        }
    }
    else
    {
        if ( ch == victim )
        {
            act( "You $T yourself about the room and look very silly.", FALSE, ch, NULL,verb, TO_CHAR );
            act( "$n decides to be silly and $T $mself about the room.", FALSE, ch, NULL, verb, TO_ROOM);
            return;
        }

        in_room = ch->in_room;
        if ( (( pexit   = world[in_room].dir_option[door] ) == NULL) || (( to_room = pexit->to_room   ) == 0 ))
        {
            sprintf( buf, "Alas, you cannot %s them that way.\n\r", verb );
            send_to_char( buf, ch );
            return;
        }

        if (IS_SET(pexit->exit_info, EX_CLOSED))
        {

            ch_printf(ch,"The %s is closed!\r\n", pexit->keyword);
            return;
        }

        //act( "You attempt to move $N out of the room.", FALSE, ch, 0,victim, TO_CHAR );
        //act( "$n is attempting to move you out of the room!", FALSE, ch, 0, victim, TO_VICT );
        //act( "$n is attempting to move $N out of the room.", FALSE, ch, 0, victim, TO_NOTVICT );

        if 	 (
            (IS_NPC(victim) && IS_SHOPKEEPER(victim))//MOB_FLAGGED(victim, MOB_SPEC))
            //||  ROOM_FLAGGED(world[to_room], ROOM_PRIVATE)
            ||	 (!str_cmp( verb, "drag" ) && GET_POS(victim) >= POS_STANDING)
            ||   (!str_cmp( verb, "push" ) && GET_POS(victim) != POS_STANDING)
            || 	 number(1, 56)>(GET_STR(ch)-GET_TOTAL_WEIGHT(victim)/100+(!str_cmp( verb, "drag" )?GET_SKILL(ch, SKILL_DRAG)/5:GET_SKILL(ch, SKILL_PUSH)/5))
            //||   number(1, 56)<(GET_STR(victim)+GET_TOTAL_WEIGHT(victim)/200-(!str_cmp( verb, "drag" )?GET_SKILL(ch, SKILL_DRAG)/5:GET_SKILL(ch, SKILL_PUSH)/5))
        )
            if (!IS_IMMORT(ch))
            {
                sprintf(buf, "$n attempts to %s $N out.", verb);
                act(buf, FALSE, ch, 0, victim, TO_NOTVICT);
                sprintf(buf, "$n attempts to %s you out.", verb);
                act(buf, FALSE, ch, 0, victim, TO_VICT);
                send_to_char( "They won't budge.\n\r", ch );
                if (IS_NPC(victim))
                {
                    switch(number(0, 2))
                    {
                    case 0:
                        do_action(victim, "", find_command("eyebrow"), 0);	break;
                    case 1:
                        act("$n looks around in disbelief.", FALSE, victim, 0, 0, TO_ROOM); break;
                    case 2:
                        if (!number(0, 5))
                        {
                        	act("$n turns to attack you!", FALSE, victim, 0, ch, TO_VICT);
                        	hit(victim, ch, TYPE_UNDEFINED);
                        }
                        break;
                    
                    }
                }
                return;
            }

        if (GET_MOVE(ch) > 20 )
        {
            if (!str_cmp( verb, "drag" ))
            {
                if (perform_move(ch, door, 0))
                {

                    if (GET_POS(victim)==POS_SLEEPING)
                    {
                        if (!IS_AFFECTED(victim, AFF_SLEEP) && number(1, 111)>GET_SKILL(ch, SKILL_DRAG) && !STR_CHECK(ch) && WIL_CHECK(victim) && !number(0, 4))
                        {
                            send_to_char("You are awakened by some strange noise.\r\n", victim);
                            act("$n awakens.", TRUE, victim, 0, 0, TO_ROOM);
                            GET_POS(victim) = POS_RESTING;
                            look_at_room(victim, 0);
                        }
                    }
                    improve_skill(ch, SKILL_DRAG, 1);
                    sprintf(buf, "You drag $N along.", dirs[door]);
                    act( buf, FALSE, ch, 0, victim, TO_CHAR );
                    sprintf(buf, "$N drags $n to the %s,", dirs[door]);
                    act( buf, FALSE, victim, 0, ch, TO_NOTVICT );
                    sprintf(buf, "$n drags you to the %s.", dirs[door]);
                    act( buf, FALSE, ch,  0, victim, TO_VICT );
                    GET_MOVE(ch)-= 20;

                    char_from_room( victim );
                    char_to_room( victim, to_room);

                    act( "$n drags $N along.", FALSE, ch, 0, victim, TO_NOTVICT );
                    if (ROOM_AFFECTED(victim->in_room, RAFF_TRAP) && number(1, 250)>GET_CHA(victim))
                        FireRoomTrap(victim);
                    if (IS_NPC(victim) && !DEAD(victim) && !DEAD(ch))
                    {
                        act("$n turns to attack you!", FALSE, victim, 0, ch, TO_VICT);
                        hit(victim, ch, TYPE_UNDEFINED);
                    }
                }
            }
            else
            {
                improve_skill(ch, SKILL_PUSH, 1);
                sprintf(buf, "You push $N out to the %s!", dirs[door]);
                act( buf, FALSE, ch, 0, victim, TO_CHAR );
                sprintf(buf, "$n pushes $N out to the %s!", dirs[door]);
                act( buf, FALSE, ch, 0, victim, TO_NOTVICT );
                sprintf(buf, "$n pushes you out to the %s!", dirs[door]);
                act( buf, FALSE, ch,  0, victim, TO_VICT );
                GET_MOVE(ch)-= 20;
                char_from_room( victim );
                char_to_room( victim, to_room );
                sprintf(buf, "$n stumbles in from %s.", dirs2[rev_dir[door]]);
                act( buf, FALSE, victim,  0, 0, TO_ROOM );
                if (ROOM_AFFECTED(victim->in_room, RAFF_TRAP) && number(1, 250)>GET_CHA(victim))
                    FireRoomTrap(victim);
                if (IS_NPC(victim) && !DEAD(victim) && !DEAD(ch))
                {
                    if (!HUNTING(victim))
                        HUNTING(victim)=ch;
                }
            }
        }

        else
        {
            sprintf( buf, "You are too tired to %s anything around!\n\r", verb );
            send_to_char( buf, ch );
        }
    }

    return;
}

ACMD(do_push)
{
    do_push_drag( ch, argument, "push" ,0);
    return;
}

ACMD(do_drag)
{
    do_push_drag( ch, argument, "drag" ,0);
    return;
}

ACMD(do_pull)
{
    do_push_drag( ch, argument, "pull" ,1);
    return;
}






ACMD(do_firstaid)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_FIRSTAID))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }


    if (!*arg)
        vict=ch;
    else
        if (!(vict = get_char_room_vis(ch, arg))) {
            send_to_char("Whom do you wish to first aid?\r\n", ch);
            return;
        }
    
    if (FIGHTING(ch))
    {
        send_to_char("You can not do this while fighting!!\r\n", ch);
        return;
    }
    if (FIGHTING(vict))
    {
        send_to_char("Impossible! They are fighting!\r\n", ch);
        return;
    }

    /*if (GET_HIT(vict)<4*GET_MAX_HIT(vict)/5)
{
        send_to_char("Those injuries can not be first aided.\r\n", ch); 
        return;
} */
    if (GET_MOVE(vict)<20)
    {
        send_to_char("You are too tired to try that now.\r\n", ch);
        return;
    }

    if (GET_HIT(vict)==GET_MAX_HIT(vict))
    {
        send_to_char("There is no need for first aid.\r\n", ch);
        return;
    }
    GET_MOVE(ch)-=20;   
    WAIT_STATE(ch, PULSE_VIOLENCE/2);   
    if (number(1, 101) <= GET_SKILL(ch, SKILL_FIRSTAID)) {       	 
        if (vict!=ch)
        {
            act("You heal $N's wounds.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n heals your wounds.", FALSE, ch, 0, vict, TO_VICT);
            act("$n heals $N's wounds.", FALSE, ch, 0, vict, TO_NOTVICT);
        }
        else
        {
            act("You heal your wounds.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n heals $s wounds.", FALSE, ch, 0, 0, TO_ROOM);
        }
        if (FOL_MUGRAK(ch) || FOL_MUGRAK(vict))
        {         
            send_to_char("You feel sharp pain.\r\n", vict);
            act("$n grimaces in pain.", FALSE, vict, 0, 0, TO_ROOM);      
    	    GET_HIT(vict)=MAX(1, GET_HIT(vict)-(number(1, (GET_SKILL(ch, SKILL_FIRSTAID)-50)/4)+MAX(1, GET_MAX_HIT(vict)/( 35-(GET_SKILL(ch, SKILL_FIRSTAID)-50)/4 ))));
    	    if (ch!=vict && !IS_NPC(vict))
    		check_fight(ch, vict);   
    		return;
        }
        
    	GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict)+number(1, (GET_SKILL(ch, SKILL_FIRSTAID)-50)/4)+MAX(1, GET_MAX_HIT(vict)/( 35-(GET_SKILL(ch, SKILL_FIRSTAID)-50)/4 )));
        improve_skill(ch, SKILL_FIRSTAID, 2);
        
        
    } else
        send_to_char("You fail.\r\n", ch);
        
     
}



ACMD(do_nerve)
{
    struct char_data *vict;
    int prob;
    struct affected_type af;
    if (!GET_SKILL(ch, SKILL_NERVE)) {
        send_to_char("You are untrained in that area.\r\n", ch);
        return;
    }


    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("That person is not here.\r\n", ch);
        return;
    }

    if (vict==ch)
    {
        send_to_char("You wouldn't like to hurt yourself.\r\n", ch);
        return;
    }
    if (FIGHTING(vict))
    {
        send_to_char("Impossible! They are fighting!\r\n", ch);
        return;
    }

    if (affected_by_spell(vict, SPELL_WEAKEN))
    {
        send_to_char("Your victim is already weakened.\r\n", ch);
        return;
    }
    prob=GET_SKILL(ch, SKILL_NERVE);


    if (!AWAKE(vict) || (number(1, 111) <= prob) || (!CAN_SEE(vict, ch) && !INT_CHECK(vict)))
    {
        act("You approach $N and quickly squeeze a spot on $S shoulder.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n approaches $N and quickly squeezes a spot on $S shoulder.", FALSE, ch, 0, vict, TO_NOTVICT);
        act("Someone from behind squeezes a spot on your shoulder.", FALSE, ch, 0, vict, TO_VICT);
        af.type = SPELL_WEAKEN;
        af.duration = 0;
        af.modifier = - MAX(1, (GET_SKILL(ch, SKILL_NERVE)-50)/12);//        ((GET_LEVEL(ch)-10)/10);
        af.location = APPLY_STR;
        af.bitvector = 0;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        affect_to_char(vict, &af);
        send_to_char("You feel your strength wither.\r\n", vict);
        act("$n's strength seem to wither.", FALSE, vict, 0, 0, TO_ROOM);
        improve_skill(ch, SKILL_NERVE, 1);
    }
    else
    {
        act("$E notices you approaching.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n tried to sneak on to you.", FALSE, ch, 0, vict, TO_VICT);
        if (IS_NPC(vict))
            hit(vict, ch, TYPE_UNDEFINED);
    }

}

ACMD(do_resistmagic)
{
    struct affected_type af;
    if (!GET_SKILL(ch, SKILL_RESISTANCE))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    
    
    if (AFF2_FLAGGED(ch, AFF2_RESISTMAGIC))
    {
        send_to_char("You are already resisting magic.\r\n", ch);
        return;
    
    }


    af.type = SKILL_RESISTANCE;
    af.duration = 0;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.bitvector2 = AFF2_RESISTMAGIC;
    af.bitvector3 = 0;

    affect_to_char(ch, &af);
    send_to_char("You prepare yourself for magical damage.\r\n", ch);
    act("$n will try to resist magic.", FALSE, ch, 0, 0, TO_ROOM);
    if (FIGHTING(ch))
    	WAIT_STATE(ch, 2*PULSE_VIOLENCE);
}





ACMD(do_repair)
{
    struct obj_data *obj;
    int percent, prob, cost;


    argument = two_arguments(argument, buf1, buf2);

    percent=IS_IMMORT(ch)? 100:0;


    if (!percent)
    {
        send_to_char("Try to find someone who knows how to repair stuff.\r\n", ch);
        return;
    }


    if (!*buf1) {

        send_to_char("Usage: repair <obj>\r\n", ch);
        return;
    }


    if (!(obj = get_obj_in_list_vis(ch, buf1, ch->carrying))) {
        sprintf(buf, "You don't seem to have that in your inventory.\r\n");
        send_to_char(buf, ch);
        return;
    }

    if (GET_OBJ_DAMAGE(obj)==100)
    {
        send_to_char("That is already in excellent condition.\r\n", ch);
        return;
    }




    prob=100-GET_OBJ_DAMAGE(obj);

    cost=MAX(prob/10, 5);

    if (cost>GET_MOVE(ch))
    {
        send_to_char("You are too tired to try that now.\r\n", ch);
        return;
    }
    if (percent<prob)
    {
        send_to_char("That is damaged beyond your repairing skills.\r\n", ch);
        return;
    }

    if (cost>GET_MOVE(ch))
    {
        send_to_char("You are too tired to try that now.\r\n", ch);
        return;
    }
    act("You sit and start vigorously pounding on $p.",  FALSE, ch, obj, NULL, TO_CHAR);
    act("$n sits and starts vigorously pounding on $p.",  FALSE, ch, obj, NULL, TO_ROOM);

    GET_MOVE(ch)-=cost;
    if (number(1, percent)<prob)
    {
        if (number(1, percent)<prob)
        {
            send_to_char("\r\nYou only manage to make some even more damage to it.\r\n", ch);
            act("\r\n$e only manages to make some even more damage to it.", FALSE,  ch, obj, NULL, TO_ROOM);
            GET_OBJ_DAMAGE(obj)-=number(1, 3);
            oprog_damage_trigger(ch, obj);

        }
        else
        {
            send_to_char("\r\nYou fail to repair it.\r\n", ch);
        }
    }
    else
    {
        act("\r\nYou skillfully repair all damages to it.",  FALSE, ch, obj, NULL, TO_CHAR);
        act("\r\n$e manages to fully repair it.", FALSE,  ch, obj, NULL, TO_ROOM);
        GET_OBJ_DAMAGE(obj)=100;
        if (GET_OBJ_TYPE(obj)==ITEM_ARMOR)
            GET_OBJ_VAL(obj, 0)=GET_OBJ_VAL(obj, 1);
        oprog_repair_trigger(ch, obj);

    }
    if (PURGED(obj))
        return;
    if (GET_OBJ_DAMAGE(obj)<=0)
        make_scraps(obj);


}
