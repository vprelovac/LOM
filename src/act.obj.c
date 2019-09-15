/* ************************************************************************
*   File: act.obj1.c                                    Part of CircleMUD *
*  Usage: object handling routines -- get/drop and container handling     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "objs.h"
#include "class.h"
#include "rooms.h"
#include "events.h"
/* extern variables */
struct index_data *obj_index;   /* index table for object file	 */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern char *drinks[];
extern int drink_aff[][3];
void wear_all_suitable(struct char_data *ch);
void mprog_give_trigger(struct char_data * mob, struct char_data * ch, struct obj_data * obj);
void mprog_bribe_trigger(struct char_data * mob, struct char_data * ch, int amount);

#define OBJ_LEVEL_DIFF 5

int cmpitems(struct char_data * ch, struct obj_data * obj1, struct obj_data * obj2)
{
    float value1, value2;
    value1 = 0;
    value2 = 0;
    if (!obj2)
        return 0;
    if (obj1 == obj2)
        return 0;
    if (obj1->obj_flags.type_flag != obj2->obj_flags.type_flag)
        return 0;

    switch (obj1->obj_flags.type_flag) {

    case ITEM_ARMOR:
        value1 = obj1->obj_flags.value[0];
        value2 = obj2->obj_flags.value[0];
        break;

    case ITEM_WEAPON:
        value1 = (float) (obj1->obj_flags.value[1] * ((float) (obj1->obj_flags.value[2] + 1) / 2) + obj1->obj_flags.value[0]);
        value2 = (float) (obj2->obj_flags.value[1] * ((float) (obj2->obj_flags.value[2] + 1) / 2) + obj2->obj_flags.value[0]);
        break;
    default:
        return 0;
        break;

    }

    if (value2 > value1)
        return 1;
    else
        return 0;
}


void perform_put(struct char_data * ch, struct obj_data * obj,
                 struct obj_data * cont)
{
    if (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj) > GET_OBJ_VAL(cont, 0))
        act("$p won't fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
    else if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        act("You can't put $p away, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
    }
    else {
        obj_from_char(obj);
        obj_to_obj(obj, cont);
        act("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
        act("$n puts $p in $P.", TRUE, ch, obj, cont, TO_ROOM);
    }
}


/* The following put modes are supported by the code below:

	1) put <object> <container>
	2) put all.<object> <container>
	3) put all <container>

	<container> must be in inventory or on ground.
	all objects to be put into container must be in inventory.
*/

ACMD(do_put)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj, *next_obj, *cont;
    struct char_data *tmp_char;
    int obj_dotmode, cont_dotmode, found = 0;

    two_arguments(argument, arg1, arg2);
    obj_dotmode = find_all_dots(arg1);
    cont_dotmode = find_all_dots(arg2);

    if (!*arg1)
        send_to_char("Put what in what?\r\n", ch);
    else if (cont_dotmode != FIND_INDIV)
        send_to_char("You can only put things into one container at a time.\r\n", ch);
    else if (!*arg2) {
        sprintf(buf, "What do you want to put %s in?\r\n",
                ((obj_dotmode == FIND_INDIV) ? "it" : "them"));
        send_to_char(buf, ch);
    } else {
        generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
        if (!cont) {
            sprintf(buf, "You don't see %s %s here.\r\n", AN(arg2), arg2);
            send_to_char(buf, ch);
        } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
            act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
        else if (IS_SET(GET_OBJ_VAL(cont, 1), CONT_CLOSED))
            send_to_char("You'd better open it first!\r\n", ch);
        else {
            if (obj_dotmode == FIND_INDIV) {	/* put <obj> <container> */
                if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
                    sprintf(buf, "You aren't carrying %s %s.\r\n", AN(arg1), arg1);
                    send_to_char(buf, ch);
                } else if (obj == cont)
                    send_to_char("You attempt to fold it into itself, but fail.\r\n", ch);
                else
                    perform_put(ch, obj, cont);
            } else {
                for (obj = ch->carrying; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
                            (obj_dotmode == FIND_ALL || isname(arg1, obj->name))) {
                        found = 1;
                        perform_put(ch, obj, cont);
                    }
                }
                if (!found) {
                    if (obj_dotmode == FIND_ALL)
                        send_to_char("You don't seem to have anything to put in it.\r\n", ch);
                    else {
                        sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
                        send_to_char(buf, ch);
                    }
                }
            }
        }
    }
}



int can_take_obj(struct char_data * ch, struct obj_data * obj)
{
    if ( GET_OBJ_TYPE(obj)==ITEM_CONTAINER && (GET_OBJ_VAL(obj, 3)==1) && obj->orig_value && IS_NPC(ch) && strcmp(GET_NAME(ch), obj->attack_verb))
    {
    		send_to_char("This corpse is protected by divine powers.\r\n", ch);
    		return 0;
    }
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
        act("$p: you can't carry that many items.", FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    } else if ((IS_CARRYING_W(ch) + IS_EQUIP_W(ch)+GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
        act("$p: you can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    } else if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) {
        act("$p: you can't take that!", FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    } else if (IN_ARENA(ch))
    {
        if ((RED(ch) && RED_FLAG(obj)) || (BLUE(ch) && BLUE_FLAG(obj)))
        {
            send_to_char("You can't take your own flag!\r\n", ch);
            return 0;
        }
    }
    return 1;
}


void get_check_money(struct char_data * ch, struct obj_data * obj)
{
    if ((GET_OBJ_TYPE(obj) == ITEM_MONEY) && (GET_OBJ_VAL(obj, 0) > 0)) {
        obj_from_char(obj);
        if (GET_OBJ_VAL(obj, 0) > 1) {
            sprintf(buf, "There were &c%d&0 coins.\r\n", GET_OBJ_VAL(obj, 0));
            send_to_char(buf, ch);
        }
        GET_GOLD(ch) += GET_OBJ_VAL(obj, 0);
        extract_obj(obj);
    }
}


void perform_get_from_container(struct char_data * ch, struct obj_data * obj,
                                struct obj_data * cont, int mode)
{
    if (mode == FIND_OBJ_INV || can_take_obj(ch, obj)) {
        if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
            act("$p: you can't hold any more items.", FALSE, ch, obj, 0, TO_CHAR);
        else {
            obj_from_obj(obj);
            obj_to_char(obj, ch);
            act("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
            if (IS_THIEF(ch) && PRF2_FLAGGED(ch, PRF2_CONCEAL) && GET_SKILL(ch, SKILL_PLANT)>number(1, 101))
                improve_skill(ch, SKILL_PLANT, 1);
            else
                act("$n gets $p from $P.", TRUE, ch, obj, cont, TO_ROOM);
            get_check_money(ch, obj);
        }
    }
}


void get_from_container(struct char_data * ch, struct obj_data * cont,
                        char *arg, int mode)
{
    struct obj_data *obj, *next_obj;
    int obj_dotmode, found = 0;

    obj_dotmode = find_all_dots(arg);
    
    if ( (GET_OBJ_VAL(cont, 3)==1) && cont->orig_value && IS_NPC(ch) && strcmp(GET_NAME(ch), cont->attack_verb))
    {
    		send_to_char("This corpse is protected by divine powers.\r\n", ch);
    		return;
    }

    if (IS_SET(GET_OBJ_VAL(cont, 1), CONT_CLOSED))
        act("$p is closed.", FALSE, ch, cont, 0, TO_CHAR);
    else if (obj_dotmode == FIND_INDIV) {
        if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains))) {
            sprintf(buf, "There doesn't seem to be %s %s in $p.", AN(arg), arg);
            act(buf, FALSE, ch, cont, 0, TO_CHAR);
        } else
            perform_get_from_container(ch, obj, cont, mode);
    } else {
        if (obj_dotmode == FIND_ALLDOT && !*arg) {
            send_to_char("Get all of what?\r\n", ch);
            return;
        }
        for (obj = cont->contains; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) &&
                    (obj_dotmode == FIND_ALL || isname(arg, obj->name))) {
                found = 1;
                perform_get_from_container(ch, obj, cont, mode);
            }
        }
        if (!found) {
            if (obj_dotmode == FIND_ALL)
                act("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
            else {
                sprintf(buf, "You can't seem to find any %ss in $p.", arg);
                act(buf, FALSE, ch, cont, 0, TO_CHAR);
            }
        }
    }
}

char fbuf[200];
int perform_get_from_room(struct char_data * ch, struct obj_data * obj)
{
    if (can_take_obj(ch, obj)) {
        obj_from_room(obj);
        if (GET_OBJ_TYPE(obj)==ITEM_RED_FLAG || GET_OBJ_TYPE(obj)==ITEM_BLUE_FLAG)
        {
            if (GET_OBJ_TYPE(obj)==ITEM_RED_FLAG)
            {
                send_to_char("You get and hold the RED FLAG.\r\n", ch);
                sprintf(fbuf,"&RRED FLAG&0 picked up by %s!", GET_NAME(ch));
                SET_BIT(AFF3_FLAGS(ch), AFF3_HAS_RED);
            }
            else
            {
                send_to_char("You get and hold the BLUE FLAG.\r\n", ch);
                sprintf(fbuf,"&BBLUE FLAG&0 picked up by %s!", GET_NAME(ch));
                SET_BIT(AFF3_FLAGS(ch), AFF3_HAS_BLUE);
            }
            sportschan(fbuf);
            oprog_get_trigger(ch, obj);
            extract_obj(obj);
            return 1;
        }
        obj_to_char(obj, ch);
        act("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
        if (IS_THIEF(ch) && PRF2_FLAGGED(ch, PRF2_CONCEAL) && GET_SKILL(ch, SKILL_PLANT)>number(1, 101))
            improve_skill(ch, SKILL_PLANT, 1);
        else
            act("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
        get_check_money(ch, obj);
        oprog_get_trigger(ch, obj);
        return 1;
    }
    return 0;
}


void get_from_room(struct char_data * ch, char *arg)
{
    struct obj_data *obj, *next_obj;
    int dotmode, found = 0;

    if (DEAD(ch))
        return;
    dotmode = find_all_dots(arg);

    if (dotmode == FIND_INDIV) {
        if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
            sprintf(buf, "You don't see %s %s here.\r\n", AN(arg), arg);
            send_to_char(buf, ch);
        } else
            perform_get_from_room(ch, obj);
    } else {
        if (dotmode == FIND_ALLDOT && !*arg) {
            send_to_char("Get all of what?\r\n", ch);
            return;
        }
        for (obj = world[ch->in_room].contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) &&
                    (dotmode == FIND_ALL || isname(arg, obj->name))) {
                found = 1;
                perform_get_from_room(ch, obj);
            }
        }
        if (!found) {
            if (dotmode == FIND_ALL)
                send_to_char("There doesn't seem to be anything here.\r\n", ch);
            else {
                sprintf(buf, "You don't see any %ss here.\r\n", arg);
                send_to_char(buf, ch);
            }
        }
    }
}

ACMD(do_wear);
ACMD(do_wield);


ACMD(do_get)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    int cont_dotmode, found = 0, mode;
    struct obj_data *cont;
    struct char_data *tmp_char;

    two_arguments(argument, arg1, arg2);

    //    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    //send_to_char("Your arms are already full!\r\n", ch);
    //  else
    if (!*arg1)
        send_to_char("Get what?\r\n", ch);
    else if (!*arg2)
        get_from_room(ch, arg1);
    else {
        cont_dotmode = find_all_dots(arg2);
        if (cont_dotmode == FIND_INDIV) {
            mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
            if (!cont) {
                sprintf(buf, "You don't have %s %s.\r\n", AN(arg2), arg2);
                send_to_char(buf, ch);
            } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
                act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
            else
                get_from_container(ch, cont, arg1, mode);
        } else {
            if (cont_dotmode == FIND_ALLDOT && !*arg2) {
                send_to_char("Get from all of what?\r\n", ch);
                return;
            }
            for (cont = ch->carrying; cont; cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) &&
                        (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
                    if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
                        found = 1;
                        get_from_container(ch, cont, arg1, FIND_OBJ_INV);
                    } else if (cont_dotmode == FIND_ALLDOT) {
                        found = 1;
                        act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
                    }
                }
            for (cont = world[ch->in_room].contents; cont; cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) &&
                        (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
                    if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
                        get_from_container(ch, cont, arg1, FIND_OBJ_ROOM);
                        found = 1;
                    } else if (cont_dotmode == FIND_ALLDOT) {
                        act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
                        found = 1;
                    }
                }
            if (!found) {
                if (cont_dotmode == FIND_ALL)
                    send_to_char("You can't seem to find any containers.\r\n", ch);
                else {
                    sprintf(buf, "You can't seem to find any %ss here.\r\n", arg2);
                    send_to_char(buf, ch);
                }
            }
        }
    }
    if (IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_CHARM))
        wear_all_suitable(ch);
}


void perform_drop_gold(struct char_data * ch, int amount,
                       int mode, sh_int RDR)
{
    struct obj_data *obj;

    if (amount <= 0)
        send_to_char("Heh heh heh.. we are jolly funny today, eh?\r\n", ch);
    else if (GET_GOLD(ch) < amount)
        send_to_char("You don't have that many coins!\r\n", ch);
    else {
        if (mode != SCMD_JUNK) {
            WAIT_STATE(ch, PULSE_VIOLENCE);	/* to prevent coin-bombing */
            obj = create_money(amount);
            if (mode == SCMD_DONATE) {
                send_to_char("You throw some coin into the air where it disappears in a puff of smoke!\r\n", ch);
                act("$n throws some coins into the air where it disappears in a puff of smoke!",
                    FALSE, ch, 0, 0, TO_ROOM);
                obj_to_room(obj, RDR);
                act("$p suddenly appears in a puff of orange smoke!", 0, 0, obj, 0, TO_ROOM);
            } else {
                send_to_char("You drop some coins.\r\n", ch);
                sprintf(buf, "$n drops %s.", money_desc(amount));
                if (IS_THIEF(ch) && PRF2_FLAGGED(ch, PRF2_CONCEAL) && GET_SKILL(ch, SKILL_PLANT)>number(1, 101))
                    improve_skill(ch, SKILL_PLANT, 1);
                else
                    act(buf, TRUE, ch, 0, 0, TO_ROOM);
                obj_to_room(obj, ch->in_room);
            }
        } else {
            sprintf(buf, "$n drops %s which disappears in a puff of smoke!",
                    money_desc(amount));
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            send_to_char("You drop some coins which disappears in a puff of smoke!\r\n", ch);
        }
        GET_GOLD(ch) -= amount;
    }
}


#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")

int perform_drop(struct char_data * ch, struct obj_data * obj,
                 int mode, char *sname, sh_int RDR)
{
    int value;

    if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        sprintf(buf, "You can't %s $p, it must be CURSED!", sname);
        act(buf, FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    }
    sprintf(buf, "You %s $p.%s", sname, VANISH(mode));
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    sprintf(buf, "$n %ss $p.%s", sname, VANISH(mode));
    if (IS_THIEF(ch) && PRF2_FLAGGED(ch, PRF2_CONCEAL) && GET_SKILL(ch, SKILL_PLANT)>number(1, 101))
        improve_skill(ch, SKILL_PLANT, 1);
    else
        act(buf, TRUE, ch, obj, 0, TO_ROOM);
    obj_from_char(obj);

    if ((mode == SCMD_DONATE) && IS_OBJ_STAT(obj, ITEM_NODONATE))
        mode = SCMD_JUNK;

    switch (mode) {
    case SCMD_DROP:
        if ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && GET_OBJ_VAL(obj, 3))
       		global_no_timer=1;
        obj_to_room(obj, ch->in_room);
        global_no_timer=0;
        oprog_drop_trigger ( ch, obj );   /* mudprogs */
        break;
    case SCMD_DONATE:
        global_no_timer=1;
        obj_to_room(obj, RDR);
        global_no_timer=0;
        act("$p suddenly appears in a puff a smoke!", FALSE, 0, obj, 0, TO_ROOM);

        break;
    case SCMD_JUNK:
        value = MAX(1, MIN(200, GET_OBJ_COST(obj) >> 4));
        extract_obj(obj);
        break;
    default:
        log("SYSERR: Incorrect argument passed to perform_drop");
        break;
    }

    return 0;
}



ACMD(do_drop)
{
    extern sh_int donation_room_1;
#if 0
    extern sh_int donation_room_2;	/* uncomment if needed! */
    extern sh_int donation_room_3;	/* uncomment if needed! */
#endif
    struct obj_data *obj, *next_obj;
    sh_int RDR = 0;
    int mode = SCMD_DROP;
    int dotmode, amount = 0;
    char *sname;

    switch (subcmd) {
    case SCMD_JUNK:
        sname = "junk";
        mode = SCMD_JUNK;
        break;
    case SCMD_DONATE:
        sname = "donate";
        mode = SCMD_DONATE;
        switch (number(0, 2)) {
        case 0:
            mode = SCMD_JUNK;
            break;
        case 1:
        case 2:
            RDR = real_room(donation_room_1);
            break;
            /*    case 3: RDR = real_room(donation_room_2); break;
                  case 4: RDR = real_room(donation_room_3); break;
            */
        }
        if (RDR == NOWHERE) {
            send_to_char("Sorry, you can't donate anything right now.\r\n", ch);
            return;
        }
        break;
    default:
        sname = "drop";
        break;
    }

    argument = one_argument(argument, arg);

    if (!*arg) {
        sprintf(buf, "What do you want to %s?\r\n", sname);
        send_to_char(buf, ch);
        return;
        /*} else if (subcmd == SCMD_DROP && (SECT(ch->in_room) == SECT_WATER_SWIM ||
              SECT(ch->in_room) == SECT_WATER_NOSWIM) &&
            !strstr(argument, "water")) {
        send_to_char("You must type 'water' after the object name if you want to drop it in water.\r\n", ch);
        return;
        */
    } else if (is_number(arg)) {
        amount = atoi(arg);
        argument = one_argument(argument, arg);
        if (!str_cmp("coins", arg) || !str_cmp("coin", arg))
            perform_drop_gold(ch, amount, mode, RDR);
        else {
            /* code to drop multiple items.  anyone want to write it? -je */
            send_to_char("Sorry, you can't do that to more than one item at a time.\r\n", ch);
        }
        return;
    } else {
        dotmode = find_all_dots(arg);

        /* Can't junk or donate all */
        if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
            if (subcmd == SCMD_JUNK)
                send_to_char("Go to the dump if you want to junk EVERYTHING!\r\n", ch);
            else
                send_to_char("Go do the donation room if you want to donate EVERYTHING!\r\n", ch);
            return;
        }
        if (dotmode == FIND_ALL) {
            if (!ch->carrying)
                send_to_char("You don't seem to be carrying anything.\r\n", ch);
            else
                for (obj = ch->carrying; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    amount += perform_drop(ch, obj, mode, sname, RDR);
                }
        } else if (dotmode == FIND_ALLDOT) {
            if (!*arg) {
                sprintf(buf, "What do you want to %s all of?\r\n", sname);
                send_to_char(buf, ch);
                return;
            }
            if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
                sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
                send_to_char(buf, ch);
            }
            while (obj) {
                next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
                amount += perform_drop(ch, obj, mode, sname, RDR);
                obj = next_obj;
            }
        } else {
            if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
                sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
                send_to_char(buf, ch);
            } else
                amount += perform_drop(ch, obj, mode, sname, RDR);
        }
    }

    if (amount && (subcmd == SCMD_JUNK)) {
        send_to_char("You have been rewarded by the gods!\r\n", ch);
        act("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
        GET_GOLD(ch) += amount;
    }
}


void perform_give(struct char_data * ch, struct char_data * vict,
                  struct obj_data * obj)
{
    if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }
    if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
        act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }
    if (GET_OBJ_WEIGHT(obj) + IS_EQUIP_W(vict)+IS_CARRYING_W(vict) > CAN_CARRY_W(vict)) {
        act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }
    obj_from_char(obj);
    obj_to_char(obj, vict);
    MOBTrigger = FALSE;
    act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
    MOBTrigger = FALSE;
    act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
    MOBTrigger = FALSE;
    act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);
    mprog_give_trigger(vict, ch, obj);
}

/* utility function for give */
struct char_data *give_find_vict(struct char_data * ch, char *arg)
{
    struct char_data *vict;

    if (!*arg) {
        send_to_char("To who?\r\n", ch);
        return NULL;
    } else if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char(NOPERSON, ch);
        return NULL;
    } else if (vict == ch) {
        send_to_char("What's the point of that?\r\n", ch);
        return NULL;
    } else
        return vict;
}


void perform_give_gold(struct char_data * ch, struct char_data * vict,
                       int amount)
{
    if (amount <= 0) {
        send_to_char("Heh heh heh ... we are jolly funny today, eh?\r\n", ch);
        return;
    }
    if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))) {
        send_to_char("You don't have that many coins!\r\n", ch);
        return;
    }
    send_to_char(OK, ch);
    mprog_bribe_trigger(vict, ch, amount);
    sprintf(buf, "$n gives you %d coins.", amount);
    MOBTrigger = FALSE;
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    sprintf(buf, "$n gives %s to $N.", money_desc(amount));
    MOBTrigger = FALSE;
    act(buf, TRUE, ch, 0, vict, TO_NOTVICT);
    if (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))
        GET_GOLD(ch) -= amount;
    GET_GOLD(vict) += amount;
}


ACMD(do_give)
{
    int amount, dotmode;
    struct char_data *vict;
    struct obj_data *obj, *next_obj;

    argument = one_argument(argument, arg);

    if (!*arg)
        send_to_char("Give what to who?\r\n", ch);
    else if (is_number(arg)) {
        amount = atoi(arg);
        argument = one_argument(argument, arg);
        if (!str_cmp("coins", arg) || !str_cmp("coin", arg)) {
            argument = one_argument(argument, arg);
            if ((vict = give_find_vict(ch, arg)))
                perform_give_gold(ch, vict, amount);
            return;
        } else {
            /* code to give multiple items.  anyone want to write it? -je */
            send_to_char("You can't give more than one item at a time.\r\n", ch);
            return;
        }
    } else {
        one_argument(argument, buf1);
        if (!(vict = give_find_vict(ch, buf1)))
            return;
        dotmode = find_all_dots(arg);
        if (dotmode == FIND_INDIV) {
            if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
                sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
                send_to_char(buf, ch);
            } else
                perform_give(ch, vict, obj);
        } else {
            if (dotmode == FIND_ALLDOT && !*arg) {
                send_to_char("All of what?\r\n", ch);
                return;
            }
            if (!ch->carrying)
                send_to_char("You don't seem to be holding anything.\r\n", ch);
            else
                for (obj = ch->carrying; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (CAN_SEE_OBJ(ch, obj) &&
                            ((dotmode == FIND_ALL || isname(arg, obj->name))))
                        perform_give(ch, vict, obj);
                }
        }
    }
}


/* Everything from here down is what was formerly act.obj2.c */


void weight_change_object(struct obj_data * obj, int weight)
{
    struct obj_data *tmp_obj;
    struct char_data *tmp_ch;

    if (obj->in_room != NOWHERE) {
        GET_OBJ_WEIGHT(obj) += weight;
    } else if ((tmp_ch = obj->carried_by)) {
        obj_from_char(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_char(obj, tmp_ch);
    } else if ((tmp_obj = obj->in_obj)) {
        obj_from_obj(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_obj(obj, tmp_obj);
    } else {
        log("SYSERR: Unknown attempt to subtract weight from an object.");
    }
}




 extern struct obj_data *obj_proto;
    extern char *drinknames[];
void name_from_drinkcon(struct obj_data * obj)
{
  char *new_name, *cur_name, *next;
  char *liqname;
  int liqlen, cpylen;

  if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  liqname = drinknames[GET_OBJ_VAL(obj, 2)];
  if (!isname(liqname, obj->name)) {
    logs("SYSERR: Can't remove liquid '%s' from '%s' (%d) item.", liqname, obj->name, obj->item_number);
    return;
  }

  liqlen = strlen(liqname);
  CREATE(new_name, char, strlen(obj->name) - strlen(liqname)); /* +1 for NUL, -1 for space */

  for (cur_name = obj->name; cur_name; cur_name = next) {
    if (*cur_name == ' ')
      cur_name++;

    if ((next = strchr(cur_name, ' ')))
      cpylen = next - cur_name;
    else
      cpylen = strlen(cur_name);

    if (!strn_cmp(cur_name, liqname, liqlen))
      continue;

    if (*new_name)
      strcat(new_name, " ");
    strncat(new_name, cur_name, cpylen);
  }

  if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);
  obj->name = new_name;
}



void name_to_drinkcon(struct obj_data *obj, int type)
{
  char *new_name;

  if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
  sprintf(new_name, "%s %s", obj->name, drinknames[type]);
  if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);
  obj->name = new_name;
}
/*

void name_from_drinkcon(struct obj_data * obj)
{
    int i;
    char *new_name;
    

    for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++);

    if (*((obj->name) + i) == ' ') {
        new_name = str_dup((obj->name) + i + 1);
        if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
            DISPOSE(obj->name);
        obj->name = new_name;
    }
}



void name_to_drinkcon(struct obj_data * obj, int type)
{
    char *new_name;
   

    CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
    sprintf(new_name, "%s %s", drinknames[type], obj->name);
    if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
        DISPOSE(obj->name);
    obj->name = new_name;
}
  */


ACMD(do_drink)
{
    struct obj_data *temp;
    struct affected_type af;
    int amount, weight;
    int on_ground = 0;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Drink from what?\r\n", ch);
        return;
    }
    if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        if (!(temp = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
            act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
            return;
        } else
            on_ground = 1;
    }
    if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
            (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
        send_to_char("You can't drink from that!\r\n", ch);
        return;
    }
    if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON)) {
        send_to_char("You have to be carrying that to drink from it.\r\n", ch);
        return;
    }
    if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
        /* The pig is drunk */
        send_to_char("You can't seem to get close enough to your mouth.\r\n", ch);
        act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
        return;
    }
    if ((GET_COND(ch, FULL) > 20) && (GET_COND(ch, THIRST) > 0)) {
        send_to_char("Your stomach can't contain anymore!\r\n", ch);
        return;
    }
    if (!GET_OBJ_VAL(temp, 1)) {
        send_to_char("It's empty.\r\n", ch);
        return;
    }
    if (subcmd == SCMD_DRINK) {
        if ( !oprog_use_trigger( ch, temp, NULL, NULL, NULL ) )
        {
            sprintf(buf, "$n drinks %s from $p.", drinks[GET_OBJ_VAL(temp, 2)]);
            act(buf, TRUE, ch, temp, 0, TO_ROOM);

            sprintf(buf, "You drink the %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
            send_to_char(buf, ch);
        }
        if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
            amount = (25 - GET_COND(ch, THIRST)) / drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK];
        else
            amount = number(3, 10);

    } else {
        if ( !oprog_use_trigger( ch, temp, NULL, NULL, NULL ) )
        {
            act("$n sips from $p.", TRUE, ch, temp, 0, TO_ROOM);
            sprintf(buf, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
            send_to_char(buf, ch);
        }
        amount = 1;
    }

    if (GET_OBJ_VAL(temp, 1)==-1)
        amount=24;
    else
        amount = MIN(amount, GET_OBJ_VAL(temp, 1));

    if (GET_OBJ_VAL(temp, 0)!=-1)
    {
        /* You can't subtract more than the object weighs */
        weight = MIN(amount, GET_OBJ_WEIGHT(temp));

        weight_change_object(temp, -weight);	/* Subtract amount */
    }

    gain_condition(ch, DRUNK,
                   (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] * amount) / 3);

    gain_condition(ch, FULL,
                   (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][FULL] * amount) / 4);

    gain_condition(ch, THIRST,
                   (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][THIRST] * amount) / 4);

    if (GET_COND(ch, DRUNK) > 10)
        send_to_char("You feel drunk.\r\n", ch);

    if (GET_COND(ch, THIRST) > 20)
        send_to_char("&cYou don't feel thirsty any more.&0\r\n", ch);

    if (GET_COND(ch, FULL) > 20)
        send_to_char("&cYou are full.&0\r\n", ch);

    if (GET_OBJ_VAL(temp, 3)) {	/* The shit was poisoned ! */
        send_to_char("Oops, it tasted rather strange!\r\n", ch);
        act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

        {
            apply_poison(ch, 1);
        }
    }
    /* empty the container, and no longer poison. */
    if (GET_OBJ_VAL(temp, 0)!=-1)
        GET_OBJ_VAL(temp, 1) -= amount;
    if (!GET_OBJ_VAL(temp, 1)) {/* The last bit */
        GET_OBJ_VAL(temp, 2) = 0;
        GET_OBJ_VAL(temp, 3) = 0;
        name_from_drinkcon(temp);
    }
    return;
}



ACMD(do_eat)
{
    struct obj_data *food=NULL;
    struct obj_data *obj;
    struct affected_type af;
       struct obj_data                 *o,
                *next_obj;
    int amount;
    int found=0;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Eat what?\r\n", ch);
        return;
    }
    else if (isname(arg, "food"))
    {
        for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
            if (CAN_SEE_OBJ(ch, obj) && GET_OBJ_TYPE(obj)==ITEM_FOOD)
            {
                found = 1;
                food=obj;
            }
        }
    }
    else if (!(food = get_obj_in_list_vis(ch, arg, ch->carrying)))
        if (!(food = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {



            sprintf(buf, "You can't seem to find any of that.\r\n");
            send_to_char(buf, ch);
            return;
        }   
        
    if (!food)
    {    
    	       sprintf(buf, "You can't seem to find any of that.\r\n");
            send_to_char(buf, ch);
            return;
    }
        
    if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE(food) == ITEM_DRINKCON) ||
                                 (GET_OBJ_TYPE(food) == ITEM_FOUNTAIN))) {
        do_drink(ch, argument, 0, SCMD_SIP);
        return;
    }
    if ((GET_OBJ_TYPE(food) != ITEM_FOOD) && (GET_LEVEL(ch) < LVL_GOD)) {
        if (GET_RACE(ch)!=RACE_TROLL)
        {
            send_to_char("You can't eat THAT!\r\n", ch);
            return;
        }
    }
    if (GET_COND(ch, FULL) > 22) {	/* Stomach full */
        act("You are too full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    if (subcmd == SCMD_EAT) {
        if ( !oprog_use_trigger( ch, food, NULL, NULL, NULL ) )
        {
            if (GET_OBJ_TYPE(food)!=ITEM_FOOD && GET_RACE(ch)==RACE_TROLL)
                switch (GET_OBJ_TYPE(food))
                {
                case ITEM_SCROLL:
                    act("You rip $p into shreds and happily munch it.\r\nAnd who said trolls don't eat salad?", FALSE, ch, food, 0, TO_CHAR);
                    act("$n rips $p into shreds and happily munchs it. And who said trolls don't eat salad?", FALSE, ch, food, 0, TO_ROOM);
                    break;
                case ITEM_CONTAINER:
                    if ( GET_OBJ_VAL(food, 3)==1)   // its a corpse
                    {
                    	if ( food->orig_value && IS_NPC(ch) && strcmp(GET_NAME(ch), food->attack_verb))
    			{
    				send_to_char("This corpse is protected by divine powers.\r\n", ch);
    				return ;
    			}                    	
                        act("You gleefully pull apart $p and eat it piece by piece.",FALSE, ch, food, 0, TO_CHAR);
                        act("$n pulls apart $p and eats it piece by piece making a disgusting sight.", FALSE, ch, food, 0, TO_ROOM );
                        for (o = food->contains; o; o = next_obj) {
         		   	next_obj = o->next_content;
            			 if ( GET_OBJ_TYPE(o)==ITEM_CONTAINER && (GET_OBJ_VAL(o, 3)==1) && o->orig_value && IS_NPC(ch) && strcmp(GET_NAME(ch), o->attack_verb))
   				 {            			
            			  obj_from_obj(o);
            			  obj_to_room(o, ch->in_room);
            			 }
		        }
                    }
                    else
                    {
                        act("You eat $p.", FALSE, ch, food, 0, TO_CHAR);
                        act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
                    }
                    break;
                default:
                    if (food==get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))
                    {
                        send_to_char("No, no you nasty trolls. No more fountain-eating.\r\n", ch);
                        return;
                    }
                    act("You swallow $p.", FALSE, ch, food, 0, TO_CHAR);
                    act("$n swallows $p.", TRUE, ch, food, 0, TO_ROOM);
                    break;
                }
            else
            {
                act("You eat $p.", FALSE, ch, food, 0, TO_CHAR);
                act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
            }

        }
    } else {
        if ( !oprog_use_trigger( ch, food, NULL, NULL, NULL ) )
        {
            act("You nibble a little bit of $p.", FALSE, ch, food, 0, TO_CHAR);
            act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
        }
    }

    amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL(food, 0) : 1);
    if ((GET_RACE(ch)==RACE_TROLL) && (subcmd==SCMD_EAT))
        amount=GET_OBJ_WEIGHT(food);

    gain_condition(ch, FULL, amount);

    if (GET_COND(ch, FULL) > 22)
        act("&cYou are full.&0", FALSE, ch, 0, 0, TO_CHAR);

    if (GET_OBJ_VAL(food, 3) && (GET_LEVEL(ch) < LVL_IMMORT) && GET_RACE(ch)!=RACE_TROLL) {
        /* The shit was poisoned ! */
        send_to_char("Oops, that tasted rather strange!\r\n", ch);
        act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

        apply_poison(ch, 1);
    }
    if (subcmd == SCMD_EAT)
        extract_obj(food);
    else {
        if (!(--GET_OBJ_VAL(food, 0))) {
            send_to_char("There's nothing left now.\r\n", ch);
            extract_obj(food);
        }
    }
}


ACMD(do_pour)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct obj_data *from_obj = NULL;
    struct obj_data *to_obj = NULL;
    int amount;

    two_arguments(argument, arg1, arg2);

    if (subcmd == SCMD_POUR) {
        if (!*arg1) {		/* No arguments */
            act("What do you want to pour from?", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
            act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
    }
    if (subcmd == SCMD_FILL) {
        if (!*arg1) {		/* no arguments */
            send_to_char("What do you want to fill?  And what are you filling it from?\r\n", ch);
            return;
        }
        if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            send_to_char("You can't find it!", ch);
            return;
        }
        if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
            act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
            return;
        }
        if (!*arg2) {		/* no 2nd argument */
            act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
            return;
        }
        if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents))) {
            sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg2), arg2);
            send_to_char(buf, ch);
            return;
        }
        if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN) {
            act("You can't fill it from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
            return;
        }
    }
    if (GET_OBJ_VAL(from_obj, 1) == 0) {
        act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
        return;
    }
    if (subcmd == SCMD_POUR) {	/* pour */
        if (!*arg2) {
            act("Where do you want it?  Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        if (!str_cmp(arg2, "out")) {
            act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
            act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

            weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1));	/* Empty */

            GET_OBJ_VAL(from_obj, 1) = 0;
            GET_OBJ_VAL(from_obj, 2) = 0;
            GET_OBJ_VAL(from_obj, 3) = 0;
            name_from_drinkcon(from_obj);

            return;
        }
        if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
            act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) &&
                (GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)) {
            act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
    }
    if (to_obj == from_obj) {
        act("A most unproductive effort.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    if ((GET_OBJ_VAL(to_obj, 1) != 0) &&
            (GET_OBJ_VAL(to_obj, 2) != GET_OBJ_VAL(from_obj, 2))) {
        act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    if (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0))) {
        act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    if (subcmd == SCMD_POUR) {
        sprintf(buf, "You pour the %s into the %s.",
                drinks[GET_OBJ_VAL(from_obj, 2)], arg2);
        send_to_char(buf, ch);
    }
    if (subcmd == SCMD_FILL) {
        act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
        act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
    }
    /* New alias */
    if (GET_OBJ_VAL(to_obj, 1) == 0)
        name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));

    /* First same type liq. */
    GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

    /* Then how much to pour */
    GET_OBJ_VAL(from_obj, 1) -= (amount =
                                     (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));

    GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

    if (GET_OBJ_VAL(from_obj, 1) < 0) {	/* There was too little */
        GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
        amount += GET_OBJ_VAL(from_obj, 1);
        GET_OBJ_VAL(from_obj, 1) = 0;
        GET_OBJ_VAL(from_obj, 2) = 0;
        GET_OBJ_VAL(from_obj, 3) = 0;
        name_from_drinkcon(from_obj);
    }
    /* Then the poison boogie */
    GET_OBJ_VAL(to_obj, 3) =
        (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3));

    /* And the weight boogie */
    weight_change_object(from_obj, -amount);
    weight_change_object(to_obj, amount);	/* Add weight */

    return;
}



void wear_message(struct char_data * ch, struct obj_data * obj, int where)
{
    char *wear_messages[][2] = {
                                   {"$n lights &c$p&0 and holds it.&0",
                                    "You light $p and hold it.&0"},

                                   {"$n slides &c$p&0 on to $s &cright ring finger.&0",
                                    "You slide &c$p&0 on to your &cright ring finger.&0"},

                                   {"$n slides &c$p&0 on to $s &cleft ring finger.&0",
                                    "You slide &c$p&0 on to your &cleft ring finger.&0"},

                                   {"$n wears &c$p&0 around $s &cneck.&0",
                                    "You wear &c$p&0 around your &cneck.&0"},

                                   {"$n wears &c$p&0 around $s &cneck.&0",
                                    "You wear &c$p&0 around your &cneck.&0"},

                                   {"$n wears &c$p&0 on $s &cbody.&0",
                                    "You wear &c$p&0 on your &cbody.&0",},

                                   {"$n wears &c$p&0 on $s &chead.&0",
                                    "You wear &c$p&0 on your &chead.&0"},

                                   {"$n puts &c$p&0 on $s &clegs.&0",
                                    "You put &c$p&0 on your &clegs.&0"},

                                   {"$n wears &c$p&0 on $s &cfeet.&0",
                                    "You wear &c$p&0 on your &cfeet.&0"},

                                   {"$n puts &c$p&0 on $s &chands.&0",
                                    "You put &c$p&0 on your &chands.&0"},

                                   {"$n wears &c$p&0 on $s &carms.&0",
                                    "You wear &c$p&0 on your &carms.&0"},

                                   {"$n wears &c$p&0 in $s &carm&0 as a shield.",
                                    "You start to use &c$p&0 as a shield.&0"},

                                   {"$n wears &c$p&0 about $s &cbody.&0",
                                    "You wear &c$p&0 around your &cbody.&0"},

                                   {"$n wears &c$p&0 around $s &cwaist.&0",
                                    "You wear &c$p&0 around your &cwaist.&0"},

                                   {"$n puts &c$p&0 on around $s &cright wrist.&0",
                                    "You put &c$p&0 on around your &cright wrist.&0"},

                                   {"$n puts &c$p&0 on around $s &cleft wrist.&0",
                                    "You put &c$p&0 on around your &cleft wrist.&0"},

                                   {"$n wields &c$p.&0",
                                    "You wield &c$p.&0"},

                                   {"$n grabs &c$p&0 and holds it.",
                                    "You grab &c$p&0 and hold it."},

                                   {"$n wields &c$p&0 in $s &coff-hand.&0",
                                    "You wield &c$p&0 in your &coff-hand.&0"},

                                   {"$n hoists &c$p&0 onto $s &cback.&0",
                                    "You hoist &c$p&0 onto your &cback.&0"},

                                   {"$n puts &c$p&0 on $s &cface.&0",
                                    "You put &c$p&0 on your &cface.&0",},

                                   {"$n wears &c$p&0 on $s &cears.&0",
                                    "You wear &c$p&0 on your &cears.&0",},

                                   {"$n puts &c$p&0 on $s &ceyes.&0",
                                    "You put &c$p&0 on your &ceyes.&0",},

                                   {"$n grabs &c$p&0 firmly with both of $s &chands.&0",
                                    "You grab &c$p&0 firmly with both of your &chands.&0"}

                               };
    if (IS_OBJ_STAT(obj, ITEM_2HANDED)) where=23;

    act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM);
    act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}


int perform_wear(struct char_data * ch, struct obj_data * obj, int where, int skloni)
{
    void perform_remove(struct char_data * ch, int pos);
    struct obj_data *obj2;

    int wear_bitvectors[] = {
                                ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
                                ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
                                ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
                                ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
                                ITEM_WEAR_WIELD, ITEM_WEAR_HOLD, ITEM_WEAR_WIELD,  ITEM_WEAR_BACK,
                                ITEM_WEAR_FACE, ITEM_WEAR_EARS, ITEM_WEAR_EYES};

    char *already_wearing[] = {
                                  "&c$p&0: You're already using a light.",
                                  "&c$p&0: YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.",
                                  "&c$p&0: You're already wearing something on both of your ring fingers.",
                                  "&c$p&0: YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.",
                                  "&c$p&0: You can't wear anything else around your neck.",
                                  "&c$p&0: You're already wearing something on your body.",
                                  "&c$p&0: You're already wearing something on your head.",
                                  "&c$p&0: You're already wearing something on your legs.",
                                  "&c$p&0: You're already wearing something on your feet.",
                                  "&c$p&0: You're already wearing something on your hands.",
                                  "&c$p&0: You're already wearing something on your arms.",
                                  "&c$p&0: You're already using something with your shield hand.",
                                  "&c$p&0: You're already wearing something about your body.",
                                  "&c$p&0: You already have something around your waist.",
                                  "&c$p&0: YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.",
                                  "&c$p&0: You're already wearing something around both of your wrists.",
                                  "&c$p&0: You're already wielding a weapon.",
                                  "&c$p&0: You're already holding something.",
                                  "&c$p&0: You're already wielding a weapon in your off-hand.",
                                  "&c$p&0: You're already wearing something on your back.",
                                  "&c$p&0: You already have something on your face.",
                                  "&c$p&0: You already have something on your ears.",
                                  "&c$p&0: You already have something on your eyes."
                              };

    /* first, make sure that the wear position is valid. */
    if (!CAN_WEAR(obj, wear_bitvectors[where])) {
        act("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    }           
    
    
    if (MOB2_FLAGGED(ch, MOB2_NOWEAR) || IS_AFFECTED(ch, AFF_CHARM))
    	return 0;
    /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
    if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R) || (where==WEAR_WIELD))
        if (GET_EQ(ch, where)) {
            if (where==WEAR_WIELD)
                where+=2;
            else
                where++;
        }

    if (where == WEAR_DUALWIELD) {
        if (IS_OBJ_STAT(obj, ITEM_2HANDED)) {
            act("You cannot dual wield $p because it's a two-handed weapon.", FALSE, ch, obj, 0, TO_CHAR);
            return 0;
        }
        else if (!GET_SKILL(ch, SKILL_DUAL_WIELD)) {
            act("You try to wield $p in your off-hand but it flips away.", FALSE, ch, obj, 0, TO_CHAR);
            return 0;
        }
        else if (!GET_SKILL(ch, SKILL_AMBIDEX) && GET_OBJ_WEIGHT(obj)>= GET_OBJ_WEIGHT(GET_EQ(ch,WEAR_WIELD)))
        {
            act("You try to wield $p in you off-hand, but it's too heavy to balance.", FALSE, ch, obj, 0, TO_CHAR);
            return 0;
        }
        else if ((obj2 = GET_EQ(ch, WEAR_SHIELD))) {
            act("$p: You're already wearing a shield.", FALSE, ch, obj, 0, TO_CHAR);
            return 0;
        }
    }
    if (skloni && GET_EQ(ch, where))
    {
        if (IS_CARRYING_N(ch) == CAN_CARRY_N(ch))
        {
            send_to_char("First drop something.\r\n",ch);
            return 0;
        }
        perform_remove(ch, where);
        if (GET_EQ(ch, where))
            return 0;
    }
    else if (GET_EQ(ch, where)) {
        act(already_wearing[where], FALSE, ch, obj, 0, TO_CHAR);
        return 0;
    }
    if (where == WEAR_SHIELD && (obj2 = GET_EQ(ch, WEAR_WIELD)) && IS_OBJ_STAT(obj2, ITEM_2HANDED)) {
        act("$P: You are already using $p, which is a two handed weapon!", FALSE, ch, obj2, obj, TO_CHAR);
        return 0;
    }
    if (where == WEAR_SHIELD && (obj2 = GET_EQ(ch, WEAR_DUALWIELD))) {
        act("$P: You're already using $p... remove it first.", FALSE, ch, obj2, obj, TO_CHAR);
        return 0;
    }
    if (IS_OBJ_STAT(obj, ITEM_ENGRAVED) && (strcmp(obj->owner_name, GET_NAME(ch)) != 0)) {
        act("$P: It's engraved to someone else!", FALSE, ch, 0, obj, TO_CHAR);
        return 0;
    }

    if (!oprog_use_trigger( ch, obj, NULL, NULL, NULL ))
        wear_message(ch, obj, where);
    if (where==WEAR_WIELD || where==WEAR_DUALWIELD)
    {
        if ((GET_OBJ_LEVEL(obj)>50?50:GET_OBJ_LEVEL(obj))>GET_SKILL(ch, TYPE_HIT+GET_OBJ_VAL(obj, 3))/2   +15)
        {
            send_to_char("Feeling this weapon is no match for your skills, you wonder if you could\r\nhandle it properly.\r\n", ch);
            act("$n really looks funny with $p, doesn't $e.", FALSE, ch, obj, 0, TO_ROOM);
        }
        else if ((GET_OBJ_LEVEL(obj)>50?50:GET_OBJ_LEVEL(obj))>GET_SKILL(ch, TYPE_HIT+GET_OBJ_VAL(obj, 3))/2+4)
        {
            send_to_char("You feel a bit clumsy wielding it.\r\n", ch);
            act("You notice $n is having problems properly wielding $p.", FALSE, ch, obj, 0, TO_ROOM);
        }
        else if ((GET_OBJ_LEVEL(obj)>50?50:GET_OBJ_LEVEL(obj))<GET_SKILL(ch, TYPE_HIT+GET_OBJ_VAL(obj, 3))/2-2)
        {
            send_to_char("With ease and elegance, you swing it around a few times.\r\n", ch);
            act("With ease and elegance, $n swings $p around a few times.\r\n", FALSE, ch, obj, 0, TO_ROOM);
        }
    }
    obj_from_char(obj);
    equip_char(ch, obj, where);
    oprog_wear_trigger( ch, obj );
    return 1;
}


int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg)
{
    int where = -1;

    static char *keywords[] = {
                                  "!RESERVED!",
                                  "finger",
                                  "!RESERVED!",
                                  "neck",
                                  "!RESERVED!",
                                  "body",
                                  "head",
                                  "legs",
                                  "feet",
                                  "hands",
                                  "arms",
                                  "shield",
                                  "about",
                                  "waist",
                                  "wrist",
                                  "!RESERVED!",
                                  "!RESERVED!",
                                  "!RESERVED!",
                                  "!RESERVED!",
                                  "back",
                                  "face",
                                  "ears",
                                  "eyes",
                                  "\n"
                              };

    if (!arg || !*arg) {
        if (CAN_WEAR(obj, ITEM_WEAR_FINGER))
            where = WEAR_FINGER_R;
        if (CAN_WEAR(obj, ITEM_WEAR_NECK))
            where = WEAR_NECK_1;
        if (CAN_WEAR(obj, ITEM_WEAR_BODY))
            where = WEAR_BODY;
        if (CAN_WEAR(obj, ITEM_WEAR_HEAD))
            where = WEAR_HEAD;
        if (CAN_WEAR(obj, ITEM_WEAR_LEGS))
            where = WEAR_LEGS;
        if (CAN_WEAR(obj, ITEM_WEAR_FEET))
            where = WEAR_FEET;
        if (CAN_WEAR(obj, ITEM_WEAR_HANDS))
            where = WEAR_HANDS;
        if (CAN_WEAR(obj, ITEM_WEAR_ARMS))
            where = WEAR_ARMS;
        if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))
            where = WEAR_SHIELD;
        if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))
            where = WEAR_ABOUT;
        if (CAN_WEAR(obj, ITEM_WEAR_WAIST))
            where = WEAR_WAIST;
        if (CAN_WEAR(obj, ITEM_WEAR_WRIST))
            where = WEAR_WRIST_R;
        if (CAN_WEAR(obj, ITEM_WEAR_BACK))
            where = WEAR_BACKPACK;
        if (CAN_WEAR(obj, ITEM_WEAR_FACE))
            where = WEAR_FACE;
        if (CAN_WEAR(obj, ITEM_WEAR_EARS))
            where = WEAR_EARS;
        if (CAN_WEAR(obj, ITEM_WEAR_EYES))
            where = WEAR_EYES;
    } else {
        if ((where = search_block(arg, keywords, FALSE)) < 0) {
            sprintf(buf, "'%s'?  What part of your body is THAT?\r\n", arg);
            send_to_char(buf, ch);
        }
    }

    return where;
}

int num_free_arms(struct char_data *ch)
{int kolko=2;
    if (GET_EQ(ch, WEAR_WIELD))
    {
        kolko--;
        if (IS_OBJ_STAT(GET_EQ(ch, WEAR_WIELD),ITEM_2HANDED))
            kolko--;
    }
    if (GET_EQ(ch, WEAR_DUALWIELD))
        kolko--;
    if (GET_EQ(ch, WEAR_SHIELD))
        kolko--;
    if (GET_EQ(ch, WEAR_HOLD))
        kolko--;
    return kolko;
}

int can_hold(struct char_data *ch, int where)
{
    if (((where==WEAR_WIELD) || (where==WEAR_DUALWIELD) || (where==WEAR_SHIELD) || (where==WEAR_HOLD)) && num_free_arms(ch)<1)
        return FALSE;
    return TRUE;
}

struct wear_all_event_obj {
    struct char_data *ch;
    int num, mode;
    char arg1[500], arg2[500];
};

EVENTFUNC(wear_all_event)
{
    int items_worn=0, where;
    struct obj_data *obj, *next_obj;
    struct char_data *ch;
    struct wear_all_event_obj *sniff=(struct wear_all_event_obj *) event_obj;
    int p=0;
    ch=sniff->ch;


    for (obj = ch->carrying; obj && ch; obj = next_obj, p++) {
        next_obj = obj->next_content;
        if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, 0)) >= 0) {
            if (can_hold(ch,where) && p>=sniff->num){
                if (IS_MONK(ch) && (where == WEAR_SHIELD))
                        continue;
                else if (MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(obj))>(GET_LEVEL(ch)+10))
                        continue;              	
                   
                items_worn++;
                sniff->num=p;
                if (perform_wear(ch, obj, where, 0)) p=1;
                else
                {sniff->num=p+1;
                    p=2;

                }
                break;

            }
        }
    }
    if (items_worn)
    {
        WAIT_STATE(ch, 6/p);
        return (6/p);
    }
    else
    {
        GET_WEAR_EVENT(ch)=0;
        DISPOSE(event_obj);
        return 0;
    }

}


EVENTFUNC(wear_all_dot_event)
{
    int items_worn=0, where;
    struct obj_data *obj, *next_obj;
    struct char_data *ch;
    struct wear_all_event_obj *sniff=(struct wear_all_event_obj *) event_obj;
    int p=0;
    char arg1[500];
    ch=sniff->ch;
    strcpy(arg1, sniff->arg1);

    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
        //sprintf(buf, "You don't seem to have any '%s'.\r\n", arg1);
        //send_to_char(buf, ch);
        GET_WEAR_EVENT(ch)=0;
        DISPOSE(event_obj);
        return 0;

    } else
        while (obj && ch) {
            next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
            if ((where = find_eq_pos(ch, obj, 0)) >= 0)
                if (can_hold(ch,where) && p>=sniff->num){
                    if (IS_MONK(ch) && (where == WEAR_SHIELD))
                    {
                    	act("$p: the way of Monk prohibits you usage of shields.", FALSE, ch, obj, 0, TO_CHAR);
                        continue;
                      }
                    else if (MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(obj))>(GET_LEVEL(ch)+10))  
                    {   
                    	act("$p: you are not powerful enough to wear this item.", FALSE, ch, obj, 0, TO_CHAR);
                        continue;              	                                                               
                       }
                    
                    items_worn++;
                    sniff->num=p;
                    if (perform_wear(ch, obj, where, 0))
                        p=1;
                    else
                    {
                        sniff->num=p+1;
                        p=3;
                    }
                    break;
                }
            obj = next_obj; p++;
        }
    if (items_worn)
    {
        WAIT_STATE(ch, 6/p);
        return (6/p);
    }
    else
    {
        GET_WEAR_EVENT(ch)=0;
        DISPOSE(event_obj);
        return 0;
    }

}

EVENTFUNC(wear_event)
{

    int items_worn=0, where;
    struct obj_data *obj, *next_obj;
    struct char_data *ch;
    struct wear_all_event_obj *sniff=(struct wear_all_event_obj *) event_obj;
    int p=0;
    char arg1[500], arg2[500];
    ch=sniff->ch;
    strcpy(arg1, sniff->arg1);
    strcpy(arg2, sniff->arg2);

    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
        sprintf(buf, "You don't seem to have %s '%s'.\r\n", AN(arg1), arg1);
        send_to_char(buf, ch);
    } else
        if ((where = find_eq_pos(ch, obj, arg2)) >= 0) {
            if (IS_MONK(ch) && (where == WEAR_SHIELD))
                send_to_char("The way of Monk prohibits you usage of shields.\r\n", ch);
            else if (MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(obj))>(GET_LEVEL(ch)+OBJ_LEVEL_DIFF))
                ch_printf(ch, "You can not wear that item until you reach level %d.\r\n", GET_OBJ_LEVEL(obj)-OBJ_LEVEL_DIFF);                       
            else {
                if (can_hold(ch,where))
                    perform_wear(ch, obj, where, 1);
            }
        }
    GET_WEAR_EVENT(ch)=0;
    DISPOSE(event_obj);
    return 0;
}


ACMD(do_wear)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj, *next_obj;
    int where, dotmode, items_worn = 0;
    struct wear_all_event_obj *sniff;

    two_arguments(argument, arg1, arg2);

    if (!*arg1) {
        send_to_char("Wear what?\r\n", ch);
        return;
    }
    else if (GET_WEAR_EVENT(ch))
    {
    	send_to_char("You are already busy wearing equipment.\r\n", ch);
    	return;
    }
    
    dotmode = find_all_dots(arg1);

    if (*arg2 && (dotmode != FIND_INDIV)) {
        send_to_char("You can't specify the same body location for more than one item!\r\n", ch);
        return;
    }
    if (dotmode == FIND_ALL) {
        for (obj = ch->carrying; obj && ch; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, 0)) >= 0) {
                if (IS_MONK(ch) && (where == WEAR_SHIELD))
                    continue;
                else if (can_hold(ch,where)){
                    items_worn++;
                }
            }
        }
        if (!items_worn)
            send_to_char("You don't seem to have anything wearable.\r\n", ch);
        else
        {
            send_to_char("&PYou start wearing your equipment...&0\r\n", ch);
            CREATE(sniff, struct wear_all_event_obj, 1);
            sniff->ch=ch;
            sniff->num=-1;
            GET_WEAR_EVENT(ch)=event_create(wear_all_event, sniff, 4);
            WAIT_STATE(ch, 4);
        }
        //	    CUREF(ch);
        return;
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg1) {
            send_to_char("Wear all of what?\r\n", ch);
            return;
        }
        if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            sprintf(buf, "You don't seem to have any '%s'.\r\n", arg1);
            send_to_char(buf, ch);
            return;
        } else
            send_to_char("&PYou start wearing your equipment...&0\r\n", ch);
        CREATE(sniff, struct wear_all_event_obj, 1);
        sniff->ch=ch;
        sniff->num=-1;
        strcpy(sniff->arg1, arg1);
        GET_WEAR_EVENT(ch)=event_create(wear_all_dot_event, sniff, 4);
        WAIT_STATE(ch, 4);
        return;
    } else {
        if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            sprintf(buf, "You don't seem to have %s '%s'.\r\n", AN(arg1), arg1);
            send_to_char(buf, ch);
        } else {
            if ((where = find_eq_pos(ch, obj, arg2)) >= 0) {
                if (IS_MONK(ch) && (where == WEAR_SHIELD))
                    send_to_char("The way of Monk prohibits you usage of shields.\r\n", ch);
                else if (MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(obj))>(GET_LEVEL(ch)+OBJ_LEVEL_DIFF))
                	ch_printf(ch, "You can not wear that item until you reach level %d.\r\n", GET_OBJ_LEVEL(obj)-OBJ_LEVEL_DIFF);
                else
                {
                    if (can_hold(ch,where))
                    {
                        act("You grab &c$p&0, and start wearing it.", FALSE, ch, obj, 0, TO_CHAR);
                        CREATE(sniff, struct wear_all_event_obj, 1);
                        sniff->ch=ch;
                        sniff->num=-1;
                        strcpy(sniff->arg1, arg1);
                        strcpy(sniff->arg2, arg2);
                        GET_WEAR_EVENT(ch)=event_create(wear_event, sniff, 4);
                        WAIT_STATE(ch, 4);
                    }
                    else
                        send_to_char("Both of your arms are already used.\r\n",ch);
                }
            } else if (!*arg2)
                act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
        }
    }
}



ACMD(do_wield)
{
    struct obj_data *obj,*next_obj;
    int dotmode,where,items_worn=0;
    one_argument(argument, arg);
    dotmode = find_all_dots(arg);

    if (!*arg)
        send_to_char("Wield what?\r\n", ch);
    /*
    else    if (dotmode == FIND_ALL) {
    CREF(ch, CHAR_NULL);
    for (obj = ch->carrying; obj && ch; obj = next_obj) {
        next_obj = obj->next_content;
        if (CAN_SEE_OBJ(ch, obj) && GET_OBJ_TYPE(obj)==ITEM_WEAPON) {
    if (IS_MONK(ch))
        send_to_char("The way of Monk prohibits you usage of weapons.\r\n", ch);
    else if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
        send_to_char("You can't wield that.\r\n", ch);
    else if (!IS_NPC(ch) && GET_OBJ_WEIGHT(obj) > str_app[GET_STR(ch)].wield_w)
        send_to_char("It's too heavy for you to use.\r\n", ch);
    else if (IS_OBJ_STAT(obj, ITEM_2HANDED) && num_free_arms(ch)<2)
        send_to_char("You need both of your hands free to wield this weapon.\r\n", ch);
    else{
       if (can_hold(ch,WEAR_WIELD))
       	 {
       	 perform_wear(ch, obj, WEAR_WIELD, 1);items_worn++;}	   	 
    	}
    	}
}
    if (!items_worn)
        send_to_char("You don't seem to have anything you can wield.\r\n", ch);
    //CUREF(ch);
    return;
}*/	
    else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "You don't seem to have %s '%s'.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
    } else {
        if (IS_MONK(ch))
            send_to_char("The way of Monk prohibits you usage of weapons.\r\n", ch);
        else if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
            send_to_char("You can't wield that.\r\n", ch);
        else if (MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(obj))>(GET_LEVEL(ch)+OBJ_LEVEL_DIFF))
              	ch_printf(ch, "You can not wield that weapon until you reach level %d.\r\n", GET_OBJ_LEVEL(obj)-OBJ_LEVEL_DIFF);                        
        else if (GET_OBJ_WEIGHT(obj) > str_app[GET_STR(ch)].wield_w)
            send_to_char("It's too heavy for you to use.\r\n", ch);
        else if (IS_OBJ_STAT(obj, ITEM_2HANDED) && num_free_arms(ch)<2)
            send_to_char("You need both of your hands free to wield this weapon.\r\n", ch);               
        else
            if (num_free_arms(ch)<1)
                send_to_char("Both of your arms are already used.\r\n",ch);
            else if (can_hold(ch,WEAR_WIELD))
            {
                perform_wear(ch, obj, WEAR_WIELD, 1);

            }

    }
}

ACMD(do_grab)
{
    struct obj_data *obj;
    int kolko = 0;
    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Hold what?\r\n", ch);
    else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
    } else {
        if (GET_OBJ_TYPE(obj) == ITEM_LIGHT) 
        {   if (MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(obj))>(GET_LEVEL(ch)+OBJ_LEVEL_DIFF))
            {  	ch_printf(ch, "You can not use this item until you reach level %d.\r\n", GET_OBJ_LEVEL(obj)-OBJ_LEVEL_DIFF); 
            return;
            }
            perform_wear(ch, obj, WEAR_LIGHT, 1);
        }
        else {
            if (!CAN_WEAR(obj, ITEM_WEAR_HOLD) && GET_OBJ_TYPE(obj) != ITEM_WAND &&
                    GET_OBJ_TYPE(obj) != ITEM_STAFF && GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
                    GET_OBJ_TYPE(obj) != ITEM_POTION)
                send_to_char("You can't hold that.\r\n", ch);
            else {
                if (num_free_arms(ch)<1) {
                    send_to_char("You have both of your hands busy.\r\n", ch);
                    return;
                }
                  if (MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(obj))>(GET_LEVEL(ch)+OBJ_LEVEL_DIFF))
            {  	ch_printf(ch, "You can not use this item until you reach level %d.\r\n", GET_OBJ_LEVEL(obj)-OBJ_LEVEL_DIFF); 
            return;
            }
                perform_wear(ch, obj, WEAR_HOLD, 1);
            }
        }
    }
}


void perform_remove(struct char_data * ch, int pos)
{
    struct obj_data *obj;

    if (!(obj = GET_EQ(ch, pos))) {
        log("Error in perform_remove: bad pos passed.");
        return;
    }
    if (IS_OBJ_STAT(obj, ITEM_NODROP))
        act("You can't remove $p, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
    else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
        act("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
    else if (IS_OBJ_STAT(obj, ITEM_NOREMOVE))
        act("$p: you can't seem to remove that item!", FALSE, ch, obj, 0, TO_CHAR);
    else {
        obj_to_char(unequip_char(ch, pos), ch);
        act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);
        if (pos == WEAR_WIELD) {
            if ((obj = GET_EQ(ch, WEAR_DUALWIELD))) {
                act("You flip $p to your other hand.", FALSE, ch, obj, NULL, TO_CHAR);
                act("$n flips $p to $s other hand.", FALSE, ch, obj, NULL, TO_ROOM);
                obj_to_char(unequip_char(ch, WEAR_DUALWIELD), ch);
                obj_from_char(obj);
                equip_char(ch, obj, WEAR_WIELD);
            }
        }
        oprog_remove_trigger( ch, obj );
    }
}

ACMD(do_remove)
{
    struct obj_data *obj;
    int i, dotmode, found;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Remove what?\r\n", ch);
        return;
    }
    dotmode = find_all_dots(arg);

    if (dotmode == FIND_ALL) {
        found = 0;
        obj=GET_EQ(ch, WEAR_DUALWIELD);
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i)) {
                perform_remove(ch, i);
                //	if (i==WEAR_WIELD && GET_EQ(ch, WEAR_WIELD)) //flipped weapon to primary hand
                //		i--;
                found = 1;
            }
        if (obj && GET_EQ(ch, WEAR_WIELD)==obj)
            perform_remove(ch, WEAR_WIELD);
        if (!found)
            send_to_char("You're not using anything.\r\n", ch);
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg)
            send_to_char("Remove all of what?\r\n", ch);
        else {
            found = 0;
            for (i = 0; i < NUM_WEARS; i++)
                if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
                        isname(arg, GET_EQ(ch, i)->name)) {
                    perform_remove(ch, i);
                    found = 1;
                }
            if (!found) {
                sprintf(buf, "You don't seem to be using any %ss.\r\n", arg);
                send_to_char(buf, ch);
            }
        }
    } else {
        if (!(obj = get_object_in_equip_vis(ch, arg, ch->equipment, &i))) {
            sprintf(buf, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
            send_to_char(buf, ch);
        } else
            perform_remove(ch, i);
    }
}

ACMD(do_fillet)
{
    struct obj_data *obj, *next_obj, *fillet, *o;
    char name[50];

    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Fillet what?\r\n", ch);
    else if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
        sprintf(buf, "I can't seem to find %s %s.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
    } else if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
        send_to_char("You can't fillet that.\r\n", ch);
    } else if (!DEX_CHECK(ch)) {
	act("$n completly tears away $p.", FALSE, ch, obj, 0, TO_ROOM);
        act("You completly tear away $p but fail to produce a fillet.", FALSE, ch, obj, 0, TO_CHAR);        
        for (o = obj->contains; o; o = next_obj) {
            next_obj = o->next_content;
            obj_from_obj(o);
            obj_to_room(o, ch->in_room);
        }

        extract_obj(obj);
    } else {

        fillet = read_object(93, VIRTUAL, 0, 0);
        fillet->in_room = NOWHERE;
        fillet->name = str_dup("fillet");
        strcpy(name, "A fillet ");
        strcat(name, strstr(obj->description, "of"));
        fillet->description = str_dup(name);
        strcpy(name, "a fillet ");
        strcat(name, strstr(obj->short_description, "of"));
        fillet->short_description = str_dup(name);
        GET_OBJ_TYPE(fillet) = ITEM_FOOD;
        GET_OBJ_WEAR(fillet) = ITEM_WEAR_TAKE;
        GET_OBJ_EXTRA(fillet) = 0;
        GET_OBJ_VAL(fillet, 0) = 10;
        GET_OBJ_VAL(obj, 2) = 3;
        GET_OBJ_VAL(fillet, 3) = 0;
        GET_OBJ_WEIGHT(fillet) = 1;
        GET_OBJ_TIMER(fillet) = 0;
        GET_OBJ_TIMER(obj) -= 5;
        obj_to_char(fillet, ch);
        act("$n completly tears away $p producing a raw fillet.", FALSE, ch, obj, 0, TO_ROOM);
        act("You mess up $p producing a raw fillet.", FALSE, ch, obj, 0, TO_CHAR);
        for (o = obj->contains; o; o = next_obj) {
            next_obj = o->next_content;
            obj_from_obj(o);
            obj_to_room(o, ch->in_room);
        }

        extract_obj(obj);
    }
}

ACMD(do_cook)
{
    struct obj_data *obj;
    char buf[300];
    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Cook what?\r\n", ch);
    else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "I can't seem to find %s %s.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
    } else if (GET_OBJ_TYPE(obj) != ITEM_FOOD) {
        send_to_char("You can't cook that.\r\n", ch);
    } else {
        if (GET_OBJ_VAL(obj, 3) == 0)
        {
            act("$p does not need cooking.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        }
        else if (GET_OBJ_VAL(obj, 2) == 0) {
            act("$n cooks $p.", FALSE, ch, obj, 0, TO_ROOM);
            act("You fully cook $p.", FALSE, ch, obj, 0, TO_CHAR);
            GET_OBJ_VAL(obj, 3) = 0;
            GET_OBJ_VAL(obj, 2) = 3;
            sprintf(buf, "%s (cooked)", obj->short_description);
            obj->short_description = str_dup(buf);
        }
        else send_to_char("You can't cook that.\r\n", ch);
        GET_MOVE(ch)-=2;

    }
}


int suitable(struct obj_data *obj, int level)
{
    int i;
    
    if (CAN_WEAR(obj, ITEM_WEAR_WIELD) && GET_OBJ_LEVEL(obj)<level)
    	return 0;
    
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
        if (obj->affected[i].location )
            if (obj->affected[i].modifier<0 )//&& !(obj->affected[i].location & APPLY_AC))
                return 0;
    return 1;
}

void wear_all_suitable(struct char_data *ch)
{
    struct obj_data *obj, *next_obj;
    int i;
    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (suitable(obj, GET_LEVEL(ch)))
        {
            i=find_eq_pos(ch, obj, "");
            if (i>=0 && i<=NUM_WEARS)
            {
                if (GET_EQ(ch, i) && cmpitems(ch,GET_EQ(ch, i),obj))
                    perform_wear(ch, obj, i, 1);
                else
                    perform_wear(ch, obj, i, 0);
            }
            else if (CAN_WEAR(obj, ITEM_WEAR_WIELD))
            {
                i=WEAR_WIELD;
                if (GET_EQ(ch, i) && cmpitems(ch,GET_EQ(ch, i),obj))
                    perform_wear(ch, obj, i, 1);
                else
                    perform_wear(ch, obj, i, 0);
            }
        }
    }
}



ACMD(do_clean)
{
    struct obj_data *obj;
    char buf[300];
    one_argument(argument, arg);
    if (!*arg)
        send_to_char("What do you want to clean?\r\n", ch);
    else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "I can't seem to find %s %s.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
    } else if (!IS_BLOODY(obj)) {
        send_to_char("That's not covered in blood anyway.\r\n", ch);
    } else {
        act("You clean of the blood from $p.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n cleans of the blood from $s $p.", FALSE, ch, obj, 0, TO_ROOM);
        REMOVE_BIT(GET_OBJ_EXTRA2(obj), ITEM2_BLOODY);
    }
}


/*
 * Turn an object into scraps.		-Thoric
 */      
#define OBJ_VNUM_SCRAPS  57
extern int max_npc_corpse_time;
void make_scraps( struct obj_data *obj )
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data  *scraps=NULL, *tmpobj;
    struct char_data *ch=NULL;     
    struct obj_data *corpse,
                *o,
                *next_obj;

    int t;

    scraps= read_object( OBJ_VNUM_SCRAPS, VIRTUAL, 0, 1 );
    GET_OBJ_TIMER(scraps) = max_npc_corpse_time;

    //don't make scraps of scraps of scraps of ...
    if ( GET_OBJ_VNUM(obj) == OBJ_VNUM_SCRAPS )
    {
        //STRFREE( scraps->short_description );  don't free the prototype!
        scraps->short_description = str_dup( "some debris" );
        //STRFREE( scraps->description );                                 don't free the prototype!
        scraps->description = str_dup( "Bits of debris lie on the ground here." );
    }
    else
    {
        sprintf( buf, scraps->short_description, obj->short_description );
        //STRFREE( scraps->short_description );   don't free the prototype!
        scraps->short_description = str_dup( buf );
        sprintf( buf, scraps->description, obj->short_description );
        //STRFREE( scraps->description );     don't free the prototype!
        scraps->description = str_dup( buf );
    }

    if ( obj->carried_by )
    {
        ch=obj->carried_by;
        act( "$p falls to the ground in scraps!",FALSE,  ch, obj, NULL, TO_CHAR );
        obj_from_char(obj);
        obj_to_room( scraps, ch->in_room);
    }
    else if (obj->worn_by)
    {
        ch=obj->worn_by;
        act( "$p falls to the ground in scraps!",FALSE, ch, obj, NULL, TO_CHAR );
        t=obj->worn_on;
        obj_to_char(unequip_char(ch, t), ch);
        obj_to_room( scraps, ch->in_room);
        obj_from_char(obj);

        if ((t==WEAR_WIELD) && (tmpobj=GET_EQ(ch, WEAR_DUALWIELD))) {
            obj_to_char(unequip_char(ch, WEAR_DUALWIELD), ch);
            obj_from_char(tmpobj);
            equip_char(ch, tmpobj, WEAR_WIELD);
        }
    }
    else if ( obj->in_room )
    {
        act( "$p is reduced to little more than scraps.",FALSE,  NULL, obj, NULL, TO_ROOM );
        obj_to_room( scraps, obj->in_room);
    }
    
    if (obj->contains)
    for (o = obj->contains; o; o = next_obj) {
            next_obj = o->next_content;
            obj_from_obj(o);
            obj_to_room(o, ch?ch->in_room:obj->in_room);
    }

    extract_obj( obj );
}


/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */
int damage_obj( CHAR_DATA *ch, OBJ_DATA *obj, int percent )
{

    int val;
    if (!ch || DEAD(ch) || !obj || PURGED(obj))
        return 0;

    // act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_ROOM );


    //if ( obj->item_type != ITEM_LIGHT )
    GET_OBJ_DAMAGE(obj)-=percent;
    GET_OBJ_DAMAGE(obj)=MAX(0, GET_OBJ_DAMAGE(obj));

    oprog_damage_trigger(ch, obj);

    if (PURGED(obj))
        return 2;

    switch( GET_OBJ_TYPE(obj) )
    {

    case ITEM_ARMOR:
        /*val=GET_OBJ_VAL(obj, 1)*percent/100;
        val=MIN(val, GET_OBJ_VAL(obj, 0));

        GET_OBJ_VAL(obj, 0)-=val;
        GET_AC(ch)-=val;*/
        val=GET_OBJ_VAL(obj, 0);
        GET_OBJ_VAL(obj, 0)=GET_OBJ_VAL(obj, 1)*GET_OBJ_DAMAGE(obj)/100;
        GET_AC(ch)+=GET_OBJ_VAL(obj, 0)-val;
        //GET_OBJ_COST(obj)=ARMOR_COST(obj);
        //affect_total(ch);
        break;
    case ITEM_WEAPON:
        // GET_OBJ_COST(obj)=WEAPON_COST(obj);
        break;
    default: break;
    }
    if (GET_OBJ_DAMAGE(obj)<=0)
        make_scraps( obj );
    return 1;
}

/*
 * how resistant an object is to damage				-Thoric
 */ 
int get_obj_resistance( OBJ_DATA *obj )
{
    int resist;

    resist = GET_OBJ_LEVEL(obj)+FORGE_LEVEL(obj);

    /* magical items are more resistant */
    if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
        resist += 3;
    /* blessed objects should have a little bonus */
    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
        resist += 3;


    /* and lasty... take armor or weapon's condition into consideration */

    resist += GET_OBJ_DAMAGE(obj)*GET_OBJ_DAMAGE(obj)/820-6;  // -6..+6

    return MAX(1, MAX( GET_OBJ_LEVEL(obj)/2,(resist*resist)/30));
}
