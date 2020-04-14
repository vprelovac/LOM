/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include <sys/time.h>
#include <sys/types.h>

#include "arena.h"
#include "structs.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "screen.h"
#include "constants.h"
#include "olc.h"
#include "clan.h"
/*   external vars  */
extern FILE *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles1[LVL_IMPL + 1];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern int restrict1;
extern int top_of_world;
extern int top_of_mobt;
extern int top_of_objt;
extern int top_of_p_table;
extern int cur_qchars, cur_qobjs;
extern char *spells[];
extern char *pc_race_types[];
extern char *pc_class_types[];
extern struct attack_hit_type attack_hit_text[];
extern int      top_of_world;   /* In db.c */
extern void chatperformtoroom(struct char_data * ch, char *txt);
extern struct mob_kill_info mobkills[];
extern struct char_data *mob_proto;
extern struct obj_data *obj_proto;
#define USE_MOB 1
#define USE_OBJ 2      


int modexp=15;

int get_vnum_from_name(char* searchname, struct char_data* ch, int do_what){
    if(do_what==USE_MOB){
        int nr,mob_number=-1;
        for(nr = 0; nr <= top_of_mobt; nr++){
            if(isname(searchname, mob_proto[nr].player.name)){
                mob_number=mob_index[nr].virtual;
            }
        }
        return mob_number;
    }
    else{
        int nr,obj_number=-1;
        for(nr =0; nr <= top_of_objt; nr++){
            if(isname(searchname, obj_proto[nr].name)){
                obj_number=obj_index[nr].virtual;
            }
        }
        return obj_number;
    }
}



ACMD(do_echo)
{
    skip_spaces(&argument);

    if (!*argument)
        send_to_char("Yes.. but what?\r\n", ch);
    else {
        if (subcmd == SCMD_EMOTE)
            sprintf(buf, "$n %s", argument);
        else
            strcpy(buf, argument);
        MOBTrigger = FALSE;
        if (IS_SUPERMOB(ch)) super_silent=0;
        act(buf, FALSE, ch, 0, 0, TO_ROOM);

        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else {
            MOBTrigger = FALSE;
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }
        super_silent=1;
        if (subcmd == SCMD_EMOTE)
            chatperformtoroom(ch, argument);

    }
}


ACMD(do_send)
{
    struct char_data *vict;

    half_chop(argument, arg, buf);

    if (!*arg) {
        send_to_char("Send what to who?\r\n", ch);
        return;
    }
    if (!(vict = get_char_vis(ch, arg))) {
        send_to_char(NOPERSON, ch);
        return;
    }
    send_to_char(buf, vict);
    send_to_char("\r\n", vict);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char("Sent.\r\n", ch);
    else {
        sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
        send_to_char(buf2, ch);
    }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
sh_int find_target_room(struct char_data * ch, char *rawroomstr)
{
    int tmp;
    sh_int location;
    struct char_data *target_mob;
    struct obj_data *target_obj;
    char roomstr[MAX_INPUT_LENGTH];

    one_argument(rawroomstr, roomstr);

    if (!*roomstr) {
        send_to_char("You must supply a room number or name.\r\n", ch);
        return NOWHERE;
    }
    if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
        tmp = atoi(roomstr);
        if ((location = real_room(tmp)) < 0) {
            send_to_char("No room exists with that number.\r\n", ch);
            return NOWHERE;
        }
    } else if ((target_mob = get_char_vis(ch, roomstr)))
        location = target_mob->in_room;
    else if ((target_obj = get_obj_vis(ch, roomstr))) {
        if (target_obj->in_room != NOWHERE)
            location = target_obj->in_room;
        else {
            send_to_char("That object is not available.\r\n", ch);
            return NOWHERE;
        }
    } else {
        send_to_char("No such creature or object around.\r\n", ch);
        return NOWHERE;
    }

    /* a location has been found -- if you're < GOD, check restrictions. */
    if (GET_LEVEL(ch) < LVL_GOD && !IS_SUPERMOB(ch)) {
        if (ROOM_FLAGGED(location, ROOM_GODROOM)) {
            send_to_char("You are not godly enough to use that room!\r\n", ch);
            return NOWHERE;
        }
        if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
                world[location].people && world[location].people->next_in_room) {
            send_to_char("There's a private conversation going on in that room.\r\n", ch);
            return NOWHERE;
        }
        if (ROOM_FLAGGED(location, ROOM_HOUSE) &&
                !House_can_enter(ch, world[location].number)) {
            send_to_char("That's private property -- no trespassing!\r\n", ch);
            return NOWHERE;
        }
    }
    return location;
}



ACMD(do_at)
{
    char command[MAX_INPUT_LENGTH];
    int location, original_loc;

    half_chop(argument, buf, command);
    if (!*buf) {
        send_to_char("You must supply a room number or a name.\r\n", ch);
        return;
    }
    if (!*command) {
        send_to_char("What do you want to do there?\r\n", ch);
        return;
    }
    if ((location = find_target_room(ch, buf)) < 0)
        return;

    /* a location has been found. */
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, command);

    /* check if the char is still there */
    if (ch->in_room == location) {
        char_from_room(ch);
        char_to_room(ch, original_loc);
    }
}


ACMD(do_goto)
{
    sh_int location;

    if ((location = find_target_room(ch, argument)) < 0)
        return;

    if (POOFOUT(ch))
        sprintf(buf, "$n %s", POOFOUT(ch));
    else
        strcpy(buf, "$n disappears in a puff of smoke.");

    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);

    if (POOFIN(ch))
        sprintf(buf, "$n %s", POOFIN(ch));
    else
        strcpy(buf, "$n appears with an ear-splitting bang.");

    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);
}



ACMD(do_trans)
{
    struct descriptor_data *i;
    struct char_data *victim;

    one_argument(argument, buf);
    if (!*buf)
        send_to_char("Whom do you wish to transfer?\r\n", ch);
    else if (str_cmp("all", buf)) {
        if (!(victim = get_char_vis(ch, buf)))
            send_to_char(NOPERSON, ch);
        else if (victim == ch)
            send_to_char("That doesn't make much sense, does it?\r\n", ch);
        else {
            if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
                send_to_char("Go transfer someone your own size.\r\n", ch);
                return;
            }
            act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
            char_from_room(victim);
            char_to_room(victim, ch->in_room);
            act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
            act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
            look_at_room(victim, 0);
        }
    } else {			/* Trans All */
        if (GET_LEVEL(ch) < LVL_GRGOD && !IS_SUPERMOB(ch)) {
            send_to_char("I think not.\r\n", ch);
            return;
        }
        for (i = descriptor_list; i; i = i->next)
            if (!i->connected && i->character && i->character != ch) {
                victim = i->character;
                if (GET_LEVEL(victim) >= GET_LEVEL(ch))
                    continue;
                act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
                char_from_room(victim);
                char_to_room(victim, ch->in_room);
                act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
                act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
                look_at_room(victim, 0);
            }
        send_to_char(OK, ch);
    }
}

ACMD(do_teleport)
{
    struct char_data *victim;
    sh_int target;

    two_arguments(argument, buf, buf2);

    if (!*buf)
        send_to_char("Whom do you wish to teleport?\r\n", ch);
    else if (!(victim = get_char_vis(ch, buf)))
        send_to_char(NOPERSON, ch);
    else if (victim == ch)
        send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
    else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
        send_to_char("Maybe you shouldn't do that.\r\n", ch);
    else if (!*buf2)
        send_to_char("Where do you wish to send this person?\r\n", ch);
    else if ((target = find_target_room(ch, buf2)) >= 0) {
        send_to_char(OK, ch);
        act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
        char_from_room(victim);
        char_to_room(victim, target);
        act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
        act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
        look_at_room(victim, 0);
    }
}



ACMD(do_vnum)
{
    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj"))) {
        send_to_char("Usage: vnum { obj | mob } <name>\r\n", ch);
        return;
    }
    if (is_abbrev(buf, "mob"))
        if (!vnum_mobile(buf2, ch))
            send_to_char("No mobiles by that name.\r\n", ch);

    if (is_abbrev(buf, "obj"))
        if (!vnum_object(buf2, ch))
            send_to_char("No objects by that name.\r\n", ch);
}



void do_stat_room(struct char_data * ch)
{
    struct extra_descr_data *desc;
    struct room_data *rm = &world[ch->in_room];
    int i, found = 0;
    struct obj_data *j = 0;
    struct char_data *k = 0;
    extern char *room_affections[];

    sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
            CCNRM(ch, C_NRM));
    send_to_char(buf, ch);

    sprinttype(rm->sector_type, sector_types, buf2);
    sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
            zone_table[rm->zone].number, CCGRN(ch, C_NRM), rm->number,
            CCNRM(ch, C_NRM), ch->in_room, buf2);

    send_to_char(buf, ch);

    sprintbit((long) rm->room_flags, room_bits, buf2);
    sprintf(buf, "SpecProc: %s, Flags: %s\r\n",
            (rm->func == NULL) ? "None" : "Exists", buf2);
    send_to_char(buf, ch);

    sprintbit((long) rm->room_affections, room_affections, buf2);
    sprintf(buf, "Room aff bits: %s\r\n", buf2);
    send_to_char(buf, ch);
    for (i=1;i<31;i++)
        if (IS_SET(ROOM_AFFECTIONS(ch->in_room), 1<<i))
        {
            struct raff_node *raff=0;
            raff=find_raff_by_aff(ch->in_room, 1<<i);
            if (raff)
            {
                sprintf(buf, "(&c%s&0 set by &c%s&0)\r\n", spells[raff->spell], raff->name);
                send_to_char(buf, ch);
            }
            else
                send_to_char("ERROR: ROOM AFF without affection", ch);
        }




    send_to_char("Description:\r\n", ch);
    if (rm->description)
        send_to_char(rm->description, ch);
    else
        send_to_char("  None.\r\n", ch);

    if (rm->ex_description) {
        sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
        for (desc = rm->ex_description; desc; desc = desc->next) {
            strcat(buf, " ");
            strcat(buf, desc->keyword);
        }
        strcat(buf, CCNRM(ch, C_NRM));
        send_to_char(strcat(buf, "\r\n"), ch);
    }
    /*    if (IS_SET(rm->room_flags, ROOM_BROADCAST)) {
    	if (rm->broad == NULL) {
    	    sprintf(buf, "Room does not have a broadcast struct... ERROR");
    	} else {
    	    send_to_char("Broadcasts to:\r\n", ch);
    	    if (rm->broad->channel != 0) {
    		sprintf(buf, "   Channel : %s\r\n", channel_bits[rm->broad->channel - 1]);
    		send_to_char(buf, ch);
    	    }
    	    if (rm->broad->targ1 != 0 && real_room(rm->broad->targ1) != -1) {
    		sprintf(buf, "   Room    : %s (%d)\t\n",
    			world[real_room(rm->broad->targ1)].name,
    			rm->broad->targ1);
    		send_to_char(buf, ch);
    	    }
    	    if (rm->broad->targ2 != 0 && real_room(rm->broad->targ2) != -1) {
    		sprintf(buf, "   Room %s (%d)\t\n",
    			world[real_room(rm->broad->targ2)].name,
    			rm->broad->targ2);
    		send_to_char(buf, ch);
    	    }
    	}
        }
        if (rm->tele != NULL) {
    	sprintf(buf, "Teleports every %d*10 seconds to %20s (Room %d)\r\n",
    		rm->tele->time, world[real_room(rm->tele->targ)].name, rm->tele->targ);
    	send_to_char(buf, ch);
    	send_to_char("Teleport Flags  :", ch);
    	sprintbit(rm->tele->mask, teleport_bits, buf);
    	send_to_char(buf, ch);
    	send_to_char("\r\n", ch);
    	if (IS_SET(rm->tele->mask, TELE_OBJ) || IS_SET(rm->tele->mask, TELE_NOOBJ)) {
    	    j = read_object(rm->tele->obj, VIRTUAL, world[ch->in_room].zone);
    	    sprintf(buf, "Teleport Object : %s\r\n", j->short_description);
    	    send_to_char(buf, ch);
    	    extract_obj(j);
    	}
        }*/
    sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
    for (found = 0, k = rm->people; k; k = k->next_in_room) {
        if (!CAN_SEE(ch, k))
            continue;
        sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
                (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
        strcat(buf, buf2);
        if (strlen(buf) >= 62) {
            if (k->next_in_room)
                send_to_char(strcat(buf, ",\r\n"), ch);
            else
                send_to_char(strcat(buf, "\r\n"), ch);
            *buf = found = 0;
        }
    }

    if (*buf)
        send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);

    if (rm->contents) {
        sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
        for (found = 0, j = rm->contents; j; j = j->next_content) {
            if (!CAN_SEE_OBJ(ch, j))
                continue;
            sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
            strcat(buf, buf2);
            if (strlen(buf) >= 62) {
                if (j->next_content)
                    send_to_char(strcat(buf, ",\r\n"), ch);
                else
                    send_to_char(strcat(buf, "\r\n"), ch);
                *buf = found = 0;
            }
        }

        if (*buf)
            send_to_char(strcat(buf, "\r\n"), ch);
        send_to_char(CCNRM(ch, C_NRM), ch);
    }
    for (i = 0; i < NUM_OF_DIRS; i++) {
        if (rm->dir_option[i]) {
            if (rm->dir_option[i]->to_room == NOWHERE)
                sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
            else
                sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
                        world[rm->dir_option[i]->to_room].number, CCNRM(ch, C_NRM));
            sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
            sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
                    CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
                    rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
                    buf2);
            send_to_char(buf, ch);
            if (rm->dir_option[i]->general_description)
                strcpy(buf, rm->dir_option[i]->general_description);
            else
                strcpy(buf, "  No exit description.\r\n");
            send_to_char(buf, ch);
        }
    }
}



void do_stat_object(struct char_data * ch, struct obj_data * j)
{
    int i, virtual, found;
    struct obj_data *j2;
    struct extra_descr_data *desc;

    virtual = GET_OBJ_VNUM(j);
    sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
            ((j->short_description) ? j->short_description : "<None>"),
            CCNRM(ch, C_NRM), j->name);
    send_to_char(buf, ch);
    sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
    if (GET_OBJ_RNUM(j) >= 0)
        strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Exists" : "None"));
    else
        strcpy(buf2, "None");
    sprintf(buf, "Level: %d  VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
            GET_OBJ_LEVEL(j),CCGRN(ch, C_NRM),  virtual, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1, buf2);
    send_to_char(buf, ch);
    sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
    send_to_char(buf, ch);

    if (j->ex_description) {
        sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
        for (desc = j->ex_description; desc; desc = desc->next) {
            strcat(buf, " ");
            strcat(buf, desc->keyword);
        }
        strcat(buf, CCNRM(ch, C_NRM));
        send_to_char(strcat(buf, "\r\n"), ch);
    }
    send_to_char("Can be worn on: ", ch);
    sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    send_to_char("Set char bits : ", ch);
    sprintbit(j->obj_flags.bitvector, affected_bits, buf);
    send_to_char(buf, ch);
    send_to_char("\r\nSet char bits2: ", ch);
    sprintbit(j->obj_flags.bitvector2, affected_bits2, buf);
    send_to_char(buf, ch);
    send_to_char("\r\nSet char bits3: ", ch);
    sprintbit(j->obj_flags.bitvector3, affected_bits3, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    send_to_char("Extra flags   : ", ch);
    sprintbit(GET_OBJ_EXTRA(j), extra_bits, buf);
    send_to_char(buf, ch);
    send_to_char("\r\nExtra flags2  : ", ch);
    sprintbit(GET_OBJ_EXTRA2(j), extra_bits2, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (j->bound_spell)
        sprintf(buf,"Bound spell: &c%s at level %d, time %d&0  Condition: %d%%\r\n", spells[j->bound_spell], j->bound_spell_level, j->bound_spell_timer, GET_OBJ_DAMAGE(j));
    else
        sprintf(buf, "Bound spell: none  Condition: %d%%\r\n", GET_OBJ_DAMAGE(j));
    send_to_char(buf, ch);




    sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
            GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j));
    send_to_char(buf, ch);

    strcpy(buf, "In room: ");
    if (j->in_room == NOWHERE)
        strcat(buf, "Nowhere");
    else {
        sprintf(buf2, "%d", world[j->in_room].number);
        strcat(buf, buf2);
    }
    strcat(buf, "    In object: ");
    strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
    strcat(buf, "\r\nCarried by: ");
    strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
    strcat(buf, "    Worn by: ");
    strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
    strcat(buf, "    Owned by: ");
    strcat(buf, j->owner_name ? j->owner_name : "Nobody");
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(j)) {
    case ITEM_LIGHT:
        sprintf(buf, "Color: [%d], Type: [%d], Hours: [%d]",
                GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2));
        break;
    case ITEM_SCROLL:
    case ITEM_POTION:
        sprintf(buf, "Spells: %d, %d, %d, %d", GET_OBJ_VAL(j, 0),
                GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
        break;
    case ITEM_WAND:
    case ITEM_STAFF:
        sprintf(buf, "Spell: %d, Mana: %d", GET_OBJ_VAL(j, 0),
                GET_OBJ_VAL(j, 1));
        break;
    case ITEM_FIREWEAPON:
        sprintf(buf, "Todam: %dd%d+%d, MissileType: %d", GET_OBJ_VAL(j, 1),
                GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 3));
        break;
    case ITEM_WEAPON:
        sprintf(buf, "Todam: %dd%d+%d, Type: %d", GET_OBJ_VAL(j, 1),
                GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 3));
        break;
    case ITEM_MISSILE:
        sprintf(buf, "Range: %d, Todam: %dd%d, MissileType: %d", GET_OBJ_VAL(j, 0),
                GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
        break;
    case ITEM_ARMOR:
        sprintf(buf, "AC-apply: [%d]", GET_OBJ_VAL(j, 0));
        break;
    case ITEM_TRAP:
        sprintf(buf, "Spell: %d, - Hitpoints: %d",
                GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1));
        break;
    case ITEM_CONTAINER:
        sprintf(buf, "Max-contains: %d, Locktype: %d, Corpse: %s",
                GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
                GET_OBJ_VAL(j, 3) ? "Yes" : "No");
        break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
        sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
        sprintf(buf, "Max-contains: %d, Contains: %d, Poisoned: %s, Liquid: %s",
                GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
                GET_OBJ_VAL(j, 3) ? "Yes" : "No", buf2);
        break;
    case ITEM_NOTE:
        sprintf(buf, "Tongue: %d", GET_OBJ_VAL(j, 0));
        break;
    case ITEM_KEY:
        sprintf(buf, "Keytype: %d", GET_OBJ_VAL(j, 0));
        break;
    case ITEM_FOOD:
        sprintf(buf, "Makes full: %d, Poisoned: %d",
                GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 3));
        break;
    default:
        sprintf(buf, "Values 0-3: [%d] [%d] [%d] [%d]",
                GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
                GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
        break;
    }
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    /* I deleted the "equipment status" code from here because it seemed
       more or less useless and just takes up valuable screen space. */

    if (j->contains) {
        sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
        for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
            sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
            strcat(buf, buf2);
            if (strlen(buf) >= 62) {
                if (j2->next_content)
                    send_to_char(strcat(buf, ",\r\n"), ch);
                else
                    send_to_char(strcat(buf, "\r\n"), ch);
                *buf = found = 0;
            }
        }

        if (*buf)
            send_to_char(strcat(buf, "\r\n"), ch);
        send_to_char(CCNRM(ch, C_NRM), ch);
    }
    found = 0;
    send_to_char("Affections:", ch);
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
        if (j->affected[i].modifier) {
            if (j->affected[i].location>=0)
                sprinttype(j->affected[i].location, apply_types, buf2);
            else
                sprintf(buf2, "'%s'", spells[-j->affected[i].location]);
            sprintf(buf, "%s %+d to %s", found++ ? "," : "",
                    j->affected[i].modifier, buf2);
            send_to_char(buf, ch);
        }
    if (!found)
        send_to_char(" None", ch);

    send_to_char("\r\n", ch);
}

void do_stat_character(struct char_data * ch, struct char_data * k)
{
    int i, i2, found = 0;
    struct obj_data *j;
    struct follow_type *fol;
    struct affected_type *aff;


/*    if (!IS_NPC(k) && !k->desc)
    {
        send_to_char("Can not stat a linkless char.\r\n", ch);
        return;
    }*/
    switch (GET_SEX(k)) {
    case SEX_NEUTRAL:
        strcpy(buf, "NEUTRAL-SEX");
        break;
    case SEX_MALE:
        strcpy(buf, "MALE");
        break;
    case SEX_FEMALE:
        strcpy(buf, "FEMALE");
        break;
    default:
        strcpy(buf, "ILLEGAL-SEX!!");
        break;
    }

    sprintf(buf2, " %s '%s'  IDNum: [%5ld], In room [%5d]\r\n",
            (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
            GET_NAME(k), GET_IDNUM(k), world[k->in_room].number);
    send_to_char(strcat(buf, buf2), ch);
    if (IS_MOB(k)) {
        sprintf(buf, "Alias: %s, VNum: [%5d], RNum: [%5d]\r\n",
                k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));
        send_to_char(buf, ch);
    }
    sprintf(buf, "Title: %s\r\n", (k->player.title ? k->player.title : "<None>"));
    send_to_char(buf, ch);

    sprintf(buf, "L-Des: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));
    send_to_char(buf, ch);
    if (!IS_MOB(k))
    {
        sprintf(buf, "Hometown: %d Email: %s   Host: %s\r\n", k->player.hometown, k->player_specials->saved.email, k->desc? k->desc->host:"(Linkless)");
        send_to_char(buf, ch);
    }

    strcpy(buf2, "\r\n");
    sprintf(buf, "Race: &c%s&0	", pc_race_types[(int) GET_RACE(k)]);
    sprintf(buf, "%sClass: ", buf);
    
    if (GET_NUM_OF_CLASS(k) > 1) {
        for (i = 0; i < NUM_CLASSES; i++)
            if (IS_SET(GET_CLASS(k), (1 << i)))
                sprintf(buf, "%s&c%s&0 ", buf, pc_class_types[i]);
    } else
        sprintf(buf, "%s&c%s&0 ", buf, pc_class_types[GET_CLASS_NUM(k)]);
        
    sprintf(buf, "%s   Deity: &c%s (%d)&0", buf, deity_list[GET_DEITY(k)].name, GET_FAITH(k));    
    
    
    strcat(buf, buf2);

    sprintf(buf2, "Lev: [%s%2d%s], XP: [%s%7d%s], Align: [%4d]\r\n",
            CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
            CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
            GET_ALIGNMENT(k));
    strcat(buf, buf2);
    send_to_char(buf, ch);

    if (!IS_NPC(k)) {
        strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
        strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
        buf1[10] = buf2[10] = '\0';

        sprintf(buf, "Created: [%s], Last Logon: [%s], Played [%dh %dm], Age [%d]\r\n",
                buf1, buf2, k->player.time.played / 3600,
                ((k->player.time.played / 3600) % 60), age(k).year);
        send_to_char(buf, ch);
        sprintf(buf, "Loadroom [%d/%d], Speaks: [%d/%d/%d], (PRC[%d]/PPL[%d]/LRN[%d%%])",
                GET_LOADROOM(k), world[GET_LOADROOM(k)].number, GET_TALK(k, 0), GET_TALK(k, 1), GET_TALK(k, 2),
                GET_PRACTICES(k), 2 + wis_app[GET_WIS(k)].bonus, int_app[GET_INT(k)].learn);
        /* . Display OLC zone for immorts . */
        if (GET_LEVEL(k) >= LVL_IMMORT)
            sprintf(buf, "%s, OLC[%d]\r\n", buf, GET_OLC_ZONE(k));
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
    }
    if (IS_NPC(k))
        sprintf(buf, "Str: [%s%d/%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
                "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]",
                CCCYN(ch, C_NRM), GET_STR(k), GET_ADD(k), CCNRM(ch, C_NRM),
                CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
                CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
                CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
                CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
                CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM));
    else
        sprintf(buf, "Str: [%s%d/%d%s]  Int: [%s%d%s/%d]  Wis: [%s%d%s/%d]  "
                "Dex: [%s%d%s/%d]  Con: [%s%d%s/%d]  Cha: [%s%d%s/%d]",
                CCCYN(ch, C_NRM), GET_STR(k), GET_STRR(k), CCNRM(ch, C_NRM),
                CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM), GET_INTR(k),
                CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM), GET_WISR(k),
                CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM), GET_DEXR(k),
                CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM), GET_CONR(k),
                CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM), GET_CHAR(k));
    send_to_char(buf, ch);
    strcpy(buf, "\r\n");
    send_to_char(buf, ch);
    /* if (!IS_NPC(k)) {
    sprintf(buf, "Vitality :[%d/%d+%4.2f]\r\n",
    (int) GET_HIT(k), (int) GET_MAX_HIT(k), hit_gain(k));
    send_to_char(buf, ch);*/
    if (!IS_NPC(k)) {
        sprintf(buf, "Hit :[%d/%d+%.1f]  Mana :[%d/%d+%.1f]  Move :[%d/%d+%.1f]\r\n",
                GET_HIT(k), GET_MAX_HIT(k), hit_gain(k),
                GET_MANA(k), GET_MAX_MANA(k), mana_gain(k),
                GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k));
        send_to_char(buf, ch);
    } else {
        /*sprintf(buf, "Vitality :[%.0f/%.0f]\r\n",
          GET_HIT(k), GET_MAX_HIT(k));
        send_to_char(buf, ch);*/
        sprintf(buf, "Hit p.:[%s%d/%d%s]  Mana p.:[%s%d/%d%s]  Move p.:[%s%d/%d%s]\r\n",
                CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), CCNRM(ch, C_NRM),
                CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), CCNRM(ch, C_NRM),
                CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), CCNRM(ch, C_NRM));
        send_to_char(buf, ch);
    }
    sprintf(buf, "Coins: [%9d], Bank: [%9d] (Total: %d)",
            GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));
    if (!IS_NPC(ch))
        sprintf(buf,"%s QuestP: [%d]\r\n",buf, GET_QUESTPOINTS(k));
    else
        strcat(buf,"\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "AC: [%d/%d], Hitroll: [%2d (%2.0f)], Damroll: [%2d (x%1.2f)], Saving throws: [%d/%d/%d/%d/%d]\r\n",
            GET_AC(k), GET_MAGAC(k),
            k->points.hitroll,dex_app[GET_DEX(k)].reaction,
            k->points.damroll,str_app[GET_STR(k)].todam,
            GET_SAVE(k, 0),
            GET_SAVE(k, 1), GET_SAVE(k, 2), GET_SAVE(k, 3), GET_SAVE(k, 4));
    send_to_char(buf, ch);

    sprinttype(GET_POS(k), position_types, buf2);
    sprintf(buf, "Pos: %s   Fighting: %s", buf2,
            (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

    if (IS_NPC(k)) {
        strcat(buf, "  Attack type: ");
        strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
        sprintf(buf2, " Attacks: %d\r\n", k->mob_specials.attack_num);
        strcat(buf, buf2);
    }
    if (k->desc) {
        sprinttype(k->desc->connected, connected_types, buf2);
        strcat(buf, " Connected: ");
        strcat(buf, buf2);
    }


    if (IS_NPC(k)) {
        strcat(buf, "Default position: ");
        sprinttype((k->mob_specials.default_pos), position_types, buf2);
        strcat(buf, buf2);
    } else {
        sprintf(buf2, ", Idle Timer (in tics) [%d]", k->char_specials.timer);
        strcat(buf, buf2);
    }
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (IS_NPC(k)) {
        sprintbit(MOB_FLAGS(k), action_bits, buf2);
        sprintf(buf, "NPC flags : %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        sprintbit(MOB2_FLAGS(k), action_bits2, buf2);
        sprintf(buf, "%sNPC flags2: %s%s%s\r\n", buf, CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        sprintbit(MOB3_FLAGS(k), action_bits3, buf2);
        sprintf(buf, "%sNPC flags3: %s%s%s\r\n", buf, CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        send_to_char(buf, ch);
    } else {
        sprintbit(PLR_FLAGS(k), player_bits, buf2);
        sprintf(buf, "PLR : %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        sprintbit(PLR2_FLAGS(k), player_bits2, buf2);
        sprintf(buf, "%sPLR2: %s%s%s\r\n", buf, CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        sprintbit(PLR3_FLAGS(k), player_bits3, buf2);
        sprintf(buf, "%sPLR3: %s%s%s\r\n", buf, CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        send_to_char(buf, ch);
        sprintbit(PRF_FLAGS(k), preference_bits, buf2);
        sprintf(buf, "PRF : %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        sprintbit(PRF2_FLAGS(k), preference_bits2, buf2);
        sprintf(buf, "%sPRF2: %s%s%s\r\n", buf, CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
        send_to_char(buf, ch);
    }

    if (IS_MOB(k)) {
        sprintf(buf, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
                (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
                k->mob_specials.damnodice, k->mob_specials.damsizedice);
        send_to_char(buf, ch);
    }
    sprintf(buf, "Carried: weight: %d, items: %d; ",
            IS_CARRYING_W(k), IS_CARRYING_N(k));

    for (i = 0, j = k->carrying; j; j = j->next_content, i++);
    sprintf(buf, "%sItems in: inventory: %d, ", buf, i);

    for (i = 0, i2 = 0; i < NUM_WEARS; i++)
        if (GET_EQ(k, i))
            i2++;
    sprintf(buf2, "eq: %d\r\n", i2);
    strcat(buf, buf2);
    send_to_char(buf, ch);

    sprintf(buf, "Hunger: %d, Thirst: %d, Drunk: %d\r\n",
            GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
    send_to_char(buf, ch);
    if (!IS_NPC(k)) {
        sprintf(buf, "PKed:  %4d    MKed:  %4d     PKs: %4d   MKs: %4d    MKScore:%4d\r\n",
                k->player_specials->saved.killed_by_player,
                k->player_specials->saved.killed_by_mob,
                k->player_specials->saved.killed_player,
                k->player_specials->saved.killed_mob, k->player_specials->saved.mkscore);
    }
    sprintf(buf, "Master is: %s, Followers are:\r\n",
            ((k->master) ? GET_NAME(k->master) : "<none>"));


    for (fol = k->followers; fol; fol = fol->next) {
        sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
        strcat(buf, buf2);
        if (strlen(buf) >= 62) {
            if (fol->next)
                send_to_char(strcat(buf, ",\r\n"), ch);
            else
                send_to_char(strcat(buf, "\r\n"), ch);
            *buf = found = 0;
        }
    }

    /* Showing the bitvector */
    sprintbit(AFF_FLAGS(k), affected_bits, buf2);
    sprintf(buf, "AFF : %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    sprintbit(AFF2_FLAGS(k), affected_bits2, buf2);
    sprintf(buf, "%sAFF2: %s%s%s\r\n", buf, CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    sprintbit(AFF3_FLAGS(k), affected_bits3, buf2);
    sprintf(buf, "%sAFF3: %s%s%s\r\n", buf, CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);

    /* Routine to show what spells a char is affected by */
    if (k->affected) {
        for (aff = k->affected; aff; aff = aff->next) {
            *buf2 = '\0';
            if (aff->duration==-2)
                sprintf(buf, "SPL : (leech) %s%-21s%s ",
                        CCCYN(ch, C_NRM),   spells[aff->type], CCNRM(ch, C_NRM));
            else
                sprintf(buf, "SPL : (%3dhr) %s%-21s%s ", aff->duration ,
                        CCCYN(ch, C_NRM),   spells[aff->type] , CCNRM(ch, C_NRM));

            if (aff->modifier) {
                sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
                strcat(buf, buf2);
            }
            if (aff->bitvector) {
                if (*buf2)
                    strcat(buf, ", sets ");
                else
                    strcat(buf, "sets ");
                sprintbit(aff->bitvector, affected_bits, buf2);
                strcat(buf, buf2);
            }
            if (aff->bitvector2) {
                if (*buf2)
                    strcat(buf, ", sets ");
                else
                    strcat(buf, "sets ");
                sprintbit(aff->bitvector2, affected_bits2, buf2);
                strcat(buf, buf2);
            }
            if (aff->bitvector3) {
                if (*buf2)
                    strcat(buf, ", sets ");
                else
                    strcat(buf, "sets ");
                sprintbit(aff->bitvector3, affected_bits3, buf2);
                strcat(buf, buf2);
            }
            send_to_char(strcat(buf, "\r\n"), ch);
        }
    }
}


ACMD(do_stat)
{
    struct char_data *victim = 0;
    struct obj_data *object = 0;
    struct char_file_u tmp_store;
    int tmp;

    half_chop(argument, buf1, buf2);

    if (!*buf1) {
        send_to_char("Stats on who or what?\r\n", ch);
        return;
    } else if (is_abbrev(buf1, "room")) {
        do_stat_room(ch);
    } else if (is_abbrev(buf1, "mob")) {
        if (!*buf2)
            send_to_char("Stats on which mobile?\r\n", ch);
        else {
            if ((victim = get_char_vis(ch, buf2)))
                do_stat_character(ch, victim);
            else
                send_to_char("No such mobile around.\r\n", ch);
        }
    } else if (is_abbrev(buf1, "player")) {
        if (!*buf2) {
            send_to_char("Stats on which player?\r\n", ch);
        } else {
            if ((victim = get_player_vis(ch, buf2, 0)))
                do_stat_character(ch, victim);
            else
                send_to_char("No such player around.\r\n", ch);
        }
    } else if (is_abbrev(buf1, "file")) {
        if (!*buf2) {
            send_to_char("Stats on which player?\r\n", ch);
        } else {
            CREATE(victim, struct char_data, 1);
            clear_char(victim);
            if (load_char(buf2, &tmp_store) > -1) {
                store_to_char(&tmp_store, victim);
                if (GET_LEVEL(victim) > GET_LEVEL(ch))
                    send_to_char("Sorry, you can't do that.\r\n", ch);
                else
                    do_stat_character(ch, victim);
                free_char(victim);
            } else {
                send_to_char("There is no such player.\r\n", ch);
                DISPOSE(victim);
            }
        }
    } else if (is_abbrev(buf1, "object")) {
        if (!*buf2)
            send_to_char("Stats on which object?\r\n", ch);
        else {
            if ((object = get_obj_vis(ch, buf2)))
                do_stat_object(ch, object);
            else
                send_to_char("No such object around.\r\n", ch);
        }
    } else {
        if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)))
            do_stat_object(ch, object);
        else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)))
            do_stat_object(ch, object);
        else if ((victim = get_char_room_vis(ch, buf1)))
            do_stat_character(ch, victim);
        else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)))
            do_stat_object(ch, object);
        else if ((victim = get_char_vis(ch, buf1)))
            do_stat_character(ch, victim);
        else if ((object = get_obj_vis(ch, buf1)))
            do_stat_object(ch, object);
        else
            send_to_char("Nothing around by that name.\r\n", ch);
    }
}


ACMD(do_shutdown)
{
    extern int circle_shutdown, circle_reboot;
    int temp = 0;

    Crash_save_all();
    if (subcmd != SCMD_SHUTDOWN) {
        send_to_char("If you want to shut something down, say so!\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (!*arg && GET_LEVEL(ch) == LVL_IMPL) {
        sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
        log(buf);
        sprintf(buf, "\007\007Shutdown sequence has been requested by %s\r\n", GET_NAME(ch));
        send_to_all(buf);
        circle_shutdown = (600 RL_SEC);
    } else if (GET_LEVEL(ch) < LVL_IMPL || !str_cmp(arg, "time")) {
        if (circle_shutdown == 0)
            send_to_char(" Lands of Myst is not shutting down.\r\n", ch);
        else if (circle_shutdown == 1)
            send_to_char(" Land of Myst is shutting down NOW!!!\r\n", ch);
        else {
            sprintf(buf, " Lands of Myst is shutting down in %d seconds\r\n", circle_shutdown / PASSES_PER_SEC);
            send_to_char(buf, ch);
        }
    } else if (!str_cmp(arg, "off")) {
        if (circle_shutdown >= 1) {
            sprintf(buf, "(GC) Reboot suspeneded by %s.", GET_NAME(ch));
            log(buf);
            sprintf(buf, "\007\007Shutdown CANCELLED by %s.\r\n", GET_NAME(ch));
            send_to_all(buf);
            circle_shutdown = 0;
        } else
            send_to_char("But the mud isn't shutting down...\r\n", ch);
    } else if (!str_cmp(arg, "quick-reboot")) {
        sprintf(buf, "(GC) Quick-Reboot by %s.", GET_NAME(ch));
        log(buf);
        sprintf(buf, "\007\007Quick-reboot sequence has been requested by %s.\r\n", GET_NAME(ch));
        send_to_all(buf);
        touch("../.fastboot");
        circle_shutdown = circle_reboot = (5 RL_SEC);
    } else if (!str_cmp(arg, "reboot")) {
        sprintf(buf, "(GC) Reboot by %s.", GET_NAME(ch));
        log(buf);
        sprintf(buf, "\007\007Shutdown-reboot sequence has been requested by %s.\r\n", GET_NAME(ch));
        send_to_all(buf);
        touch("../.fastboot");
        circle_shutdown = circle_reboot = (30 RL_SEC);
    } else if (!str_cmp(arg, "die")) {
        sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
        log(buf);
        sprintf(buf, "\007\007FAST\007\007 Shutdown sequence has been requested by %s.\r\nSave at once!\r\n", GET_NAME(ch));
        send_to_all(buf);
        touch("../.killscript");
        circle_shutdown = (10 RL_SEC);
    } else if (!str_cmp(arg, "remake")) {
        sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
        log(buf);
        sprintf(buf, "Code-maintanence sequence has been requested by %s.\r\n", GET_NAME(ch));
        send_to_all(buf);
        touch("../.remake");
        circle_shutdown = (300 RL_SEC);
    } else if (!str_cmp(arg, "pause")) {
        sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
        log(buf);
        sprintf(buf, "Indefinite Shutdown requested by %s.\r\n", GET_NAME(ch));
        send_to_all(buf);
        touch("../pause");
        circle_shutdown = (30 RL_SEC);
    } else if (isdigit(*arg)) {
        temp = atoi(arg);
        if (temp > 3600) {
            temp = 3600;
            send_to_char("Timer set to 3600 seconds.\r\n", ch);
        } else if (temp < 10) {
            temp = 10;
            send_to_char("Timer set to 10 seconds.\r\n", ch);
        }
        sprintf(buf, "(GC) Timed Shutdown by %s (%d seconds).", GET_NAME(ch), temp);
        log(buf);
        sprintf(buf, "\007\007Shutdown in %d seconds requested by %s.\r\n", temp, GET_NAME(ch));
        send_to_all(buf);
        circle_shutdown = (temp RL_SEC);
    } else
        send_to_char("Unknown shutdown option.\r\n", ch);

}


void stop_snooping(struct char_data * ch)
{
    if (!ch->desc->snooping)
        send_to_char("You aren't snooping anyone.\r\n", ch);
    else {
        send_to_char("You stop snooping.\r\n", ch);
        ch->desc->snooping->snoop_by = NULL;
        ch->desc->snooping = NULL;
    }
}


ACMD(do_snoop)
{
    struct char_data *victim, *tch;

    if (!ch->desc)
        return;

    one_argument(argument, arg);

    if (!*arg)
    {
        stop_snooping(ch);

    }
    else if (!(victim = get_char_vis(ch, arg)))
        send_to_char("No such person around.\r\n", ch);
    else if (!victim->desc)
        send_to_char("There's no link.. nothing to snoop.\r\n", ch);
    else if (victim == ch)
        stop_snooping(ch);
    else if (victim->desc->snoop_by) {
        send_to_char("Busy already. Snooped by: ", ch);
        send_to_char(GET_NAME(victim->desc->snoop_by->character), ch);
        send_to_char("\r\n", ch);
    } else if (victim->desc->snooping == ch->desc)
        send_to_char("Don't be stupid.\r\n", ch);
    else {
        if (victim->desc->original)
            tch = victim->desc->original;
        else
            tch = victim;

        if (GET_LEVEL(tch) >= GET_LEVEL(ch)) {
            send_to_char("You can't.\r\n", ch);
            return;
        }
        send_to_char(OK, ch);

        if (ch->desc->snooping)
            ch->desc->snooping->snoop_by = NULL;

        ch->desc->snooping = victim->desc;
        victim->desc->snoop_by = ch->desc;
    }
}


ACMD(do_snooproom)
{
    struct char_data *victim, *tch;

    if (!ch->desc)
        return;

    one_argument(argument, arg);

    if (!*arg)
    {
        if (ch->listening_to) {
            struct char_data *temp;
            REMOVE_FROM_LIST(ch, world[ch->listening_to].listeners, next_listener);
            ch->listening_to = 0;
            send_to_char("You stop room snooping.\r\n", ch);
        }
    }
    else
    {

        sh_int location;

        if ((location = find_target_room(ch, arg)) < 0)
            return;

        ch->next_listener = world[location].listeners;
        world[location].listeners = ch;
        ch->listening_to =location;
        sprintf(buf, "You snoop room %d - %s.\r\n", atoi(arg), world[location].name);
        send_to_char(buf, ch);

    }


}
int ptna []=
{14300, 3062, 8353, 0};


int display_pc2npc_menu(struct descriptor_data *d)
{                              
	int i=0, j=0;
	struct char_data *ch=d->character;
	struct char_data *victim=NULL;
	
	sprintf(buf, "\r\n\r\n\r\n&CLogin as:\r\n&c---------&0\r\n\r\n");
	while (ptna[i])
	{
		for (victim = character_list; victim ; victim = victim->next)
    			if (IS_NPC(victim) && GET_MOB_VNUM(victim)==ptna[i])
    				break;		                                            
		i++;    				
    		if (!victim)
    			continue;         			
		else
		{
			sprintf(buf, "%s&G%d&9)&c %s in %s, level %d&0\r\n", buf, i, CAP(GET_NAME(victim)),zone_table[world[victim->in_room].zone].name, GET_LEVEL(victim));    			
			j=1;
		}
				
	}
	
	if (j)                    
	{	strcat(buf, "\r\n&GR&0) &cReturn to main menu\r\n");
		strcat(buf ,"\r\n&GChoice: &0");
		SEND_TO_Q(buf, d);
		STATE(d)=CON_PC2NPC;
	}
	else
	{
		SEND_TO_Q("There are no free mobs available at the moment, or your level is too low.\r\n", d);
		STATE(d)=CON_MENU;
	}
		
	return 0;
}      
extern char *MENU;//, *MENU2;                
ACMD(do_prompt);
void log_character(struct descriptor_data *d, int mode);
int check_pc2npc(struct descriptor_data *d, char *arg)                
{       
	    int num=atoi(arg);
	    int i=0;
	    struct char_data *victim;
	
	
	if (UPPER(*arg)=='R')
	{
		
		SEND_TO_Q(MENU, d);
            	
		STATE(d)=CON_MENU;
		return 0;
	}
	
	if (!num)
	{
		SEND_TO_Q("That is not a valid choice.\r\n", d);
		display_pc2npc_menu(d);
		return 0;
	}
		
	while (ptna[i])
	{       
		victim=NULL;
		for (victim = character_list; victim ; victim = victim->next)
    			if (IS_NPC(victim) && !victim->desc && GET_MOB_VNUM(victim)==ptna[i])
    				break;		                                            
		i++;    				
		
		if (i==num && victim)
		{                    
		    
		    if (GET_LEVEL(victim)>GET_LEVEL(d->character)-10)
		    {
		    	SEND_TO_Q("\r\nYou need to be at least ten levels above that mob.\r\n", d);	
		    	display_pc2npc_menu(d);
			return 0;
		    }
		    	
	            log_character(d, 2);
	            
        	       if (GET_WAS_IN(d->character) == NOWHERE && d->character->in_room != NOWHERE) {
            		GET_WAS_IN(d->character) = d->character->in_room;                      
            		 
            		  save_char(d->character, d->character->in_room); 
            		 char_from_room(d->character);
            		char_to_room(d->character, 1); 
            	    }
        	 
        	 
        	    d->original = d->character;
		    victim->desc = d;
                    d->character->desc = NULL;   
                    
                 
            		
                    
                    strcpy(victim->player_specials->saved.prompt, d->character->player_specials->saved.prompt);
                    strcpy(victim->player_specials->saved.prompt_combat, d->character->player_specials->saved.prompt_combat);
                    victim->player_specials->saved.pref= d->character->player_specials->saved.pref;
                    victim->player_specials->saved.pref2= d->character->player_specials->saved.pref2;
                    
                    
                    d->character = victim;  
                    
                    do_look(victim, "", 0, 0);                                       
                    send_to_char("Type 'return' to return to your original body.\r\n", victim);
                    return 0;
		}
						
	}       
	
		SEND_TO_Q("That is not a valid choice.\r\n", d);
		display_pc2npc_menu(d);	
		return 0;
	
}

	
ACMD(do_pctonpc)
{
    struct char_data *victim;
    
    for (victim = character_list; victim ; victim = victim->next)
    	if (IS_NPC(victim) && !victim->desc && GET_MOB_VNUM(victim)==ptna[0])
    		break;
    	
    	if (!victim)
    		return;
    
    
         send_to_char("OK.\r\n", ch);
        ch->desc->character = victim;
        ch->desc->original = ch;

        victim->desc = ch->desc;
        ch->desc = NULL;
        /*    victim->player_specials = ch->player_specials;
            victim->player_specials->aliases = ch->player_specials->aliases; */
    
}




ACMD(do_switch)
{
    struct char_data *victim;

    one_argument(argument, arg);

    if (ch->desc->original)
        send_to_char("You're already switched.\r\n", ch);
    else if (!*arg)
        send_to_char("Switch with who?\r\n", ch);
    else if (!(victim = get_char_vis(ch, arg)))
        send_to_char("No such character.\r\n", ch);
    else if (ch == victim)
        send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);
    else if (!IS_NPC(victim))
        send_to_char("Sorry, you can only switch into mobs!\r\n", ch);
    else if (victim->desc)
        send_to_char("You can't do that, the body is already in use!\r\n", ch);
    else if ((GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim))
        send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
    else if (GET_LEVEL(ch) < GET_LEVEL(victim) && GET_LEVEL(ch)!=LVL_IMPL)
        send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
    else {
        send_to_char(OK, ch);

        ch->desc->character = victim;
        ch->desc->original = ch;

        victim->desc = ch->desc;
        ch->desc = NULL;
        /*    victim->player_specials = ch->player_specials;
            victim->player_specials->aliases = ch->player_specials->aliases; */
    }
}


ACMD(do_return)
{
    if (ch->desc && ch->desc->original) {
        send_to_char("You return to your original body.\r\n", ch);

        /* JE 2/22/95 */
        /* if someone switched into your original body, disconnect them */
        if (ch->desc->original->desc)
            close_socket(ch->desc->original->desc);

        ch->desc->character = ch->desc->original;
        ch->desc->original = NULL;

        ch->desc->character->desc = ch->desc;
        ch->desc = NULL;
        
        
        
    }
}



ACMD(do_load)
{
    struct char_data *mob;
    struct obj_data *obj;
    int number, r_num;
    int is_num=0, temp_num;

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2) {
        send_to_char("Usage: load { obj | mob } <name | vnum>\r\n", ch);
        return;
    }
    if ((number = atoi(buf2)) < 0) {
        send_to_char("A NEGATIVE number??\r\n", ch);
        return;
    }



    is_num=isdigit(*buf2);
    if (is_abbrev(buf, "mob")) {
        if(!is_num)
            temp_num=get_vnum_from_name(buf2,ch,USE_MOB);
        else
            temp_num=atoi(buf2);

        if ((temp_num) < 0) {
            send_to_char("A NEGATIVE number??\r\n", ch);
            return;
        }

        if ((r_num = real_mobile(temp_num)) < 0) {
            send_to_char("There is no monster with that number.\r\n", ch);
            return;
        }

        mob = read_mobile(r_num, REAL, world[ch->in_room].zone);
        char_to_room(mob, ch->in_room);

        act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
            0, 0, TO_ROOM);
        act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
        act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    }
    else if (is_abbrev(buf, "obj")) {
        if(!isdigit(*buf2))
            temp_num=get_vnum_from_name(buf2,ch,USE_OBJ);
        else
            temp_num=atoi(buf2);

        if ((r_num = real_object(temp_num)) < 0) {
            send_to_char("What is that?\r\n", ch);
            return;
        }
        if ((r_num = real_object(temp_num)) < 0) {
            send_to_char("There is no object with that number.\r\n", ch);
            return;
        }
        obj = read_object(r_num, REAL, world[ch->in_room].zone, OBJ_PERM);
        act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
        act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
        act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
        obj_to_room(obj, ch->in_room);
        GET_OBJ_DAMAGE(obj)=100;
    } else
        send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);




    /*
        if (is_abbrev(buf, "mob")) {
    	if ((r_num = real_mobile(number)) < 0) {
    	    send_to_char("There is no monster with that number.\r\n", ch);
    	    return;
    	}
    	mob = read_mobile(r_num, REAL, world[ch->in_room].zone);
    	char_to_room(mob, ch->in_room);

    	act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
    	    0, 0, TO_ROOM);
    	act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    	act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
        } else if (is_abbrev(buf, "obj")) {
    	if ((r_num = real_object(number)) < 0) {
    	    send_to_char("There is no object with that number.\r\n", ch);
    	    return;
    	}
    	obj = read_object(r_num, REAL, world[ch->in_room].zone, OBJ_PERM);
    	act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    	act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    	act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    	obj_to_room(obj, ch->in_room);
        } else
    	send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
    	*/
}



ACMD(do_vstat)
{
    struct char_data *mob;
    struct obj_data *obj;
    int number, r_num;

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2 || !isdigit(*buf2)) {
        send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
        return;
    }
    if ((number = atoi(buf2)) < 0) {
        send_to_char("A NEGATIVE number??\r\n", ch);
        return;
    }
    if (is_abbrev(buf, "mob")) {
        if ((r_num = real_mobile(number)) < 0) {
            send_to_char("There is no monster with that number.\r\n", ch);
            return;
        }
        mob = read_mobile(r_num, REAL, world[ch->in_room].zone);
        char_to_room(mob, 3);
        do_stat_character(ch, mob);
        extract_char(mob);
    } else if (is_abbrev(buf, "obj")) {
        if ((r_num = real_object(number)) < 0) {
            send_to_char("There is no object with that number.\r\n", ch);
            return;
        }
        obj = read_object(r_num, REAL, world[ch->in_room].zone, OBJ_PERM);
        do_stat_object(ch, obj);
        extract_obj(obj);
    } else
        send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}




/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
    struct char_data *vict, *next_v;
    struct obj_data *obj, *next_o;

    one_argument(argument, buf);

    if (*buf) {			/* argument supplied. destroy single
           object or char */
        if ((vict = get_char_room_vis(ch, buf))) {
            if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
                send_to_char("Fuuuuuuuuu!\r\n", ch);
                return;
            }
            act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

            if (!IS_NPC(vict)) {
                sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
                mudlog(buf, BRF, LVL_GOD, TRUE);
                if (vict->desc) {
                    close_socket(vict->desc);
                    vict->desc = NULL;
                }
            }
            extract_char(vict);
        } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
            act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
            extract_obj(obj);
        } else {
            send_to_char("Nothing here by that name.\r\n", ch);
            return;
        }

        send_to_char(OK, ch);
    } else {			/* no argument. clean out the room */
        act("$n gestures... You are surrounded by scorching flames!",
            FALSE, ch, 0, 0, TO_ROOM);

        send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

        for (vict = world[ch->in_room].people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (IS_NPC(vict))
                extract_char(vict);
        }
        for (obj = world[ch->in_room].contents; obj; obj = next_o) {
            next_o = obj->next_content;
            extract_obj(obj);
        }
    }
}



ACMD(do_advance)
{
    struct char_data *victim;
    char *name = arg, *level = buf2;
    int newlevel;
    void do_start(struct char_data * ch);

    void gain_exp(struct char_data * ch, int gain);

    two_arguments(argument, name, level);

    if (GET_LEVEL(ch) < LVL_IMPL) {
        send_to_char("You're not godly enough to do that!\r\n", ch);
        return;
    }
    if (*name) {
        if (!(victim = get_char_vis(ch, name))) {
            send_to_char("That player is not here.\r\n", ch);
            return;
        }
    } else {
        send_to_char("Advance who?\r\n", ch);
        return;
    }

    if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
        send_to_char("Maybe that's not such a great idea.\r\n", ch);
        return;
    }
    if (IS_NPC(victim)) {
        send_to_char("NO!  Not on NPC's.\r\n", ch);
        return;
    }
    if (!*level || (newlevel = atoi(level)) <= 0) {
        send_to_char("That's not a level!\r\n", ch);
        return;
    }
    if (newlevel > LVL_IMPL) {
        sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
        send_to_char(buf, ch);
        return;
    }
    if (newlevel > GET_LEVEL(ch)) {
        send_to_char("Yeah, right.\r\n", ch);
        return;
    }
    if (newlevel < GET_LEVEL(victim)) {
        do_start(victim);
        send_to_char("You are momentarily enveloped by darkness!\r\n"
                     "You feel somewhat diminished.\r\n", victim);
        GET_LEVEL(victim) = newlevel;
    } else {
        act("$n makes some strange gestures.\r\n"
            "A strange feeling comes upon you,\r\n"
            "Like a giant hand, light comes down\r\n"
            "from above, grabbing your body, that\r\n"
            "begins to pulse with colored lights\r\n"
            "from inside.\r\r\n\n"
            "Your head seems to be filled with demons\r\n"
            "from another plane as your body dissolves\r\n"
            "to the elements of time and space itself.\r\n"
            "Suddenly a silent explosion of light\r\n"
            "snaps you back to reality.\r\r\n\n"
            "You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
    }

    send_to_char(OK, ch);

    sprintf(buf, "(GC) %s has advanced %s to level %d (from %d)",
            GET_NAME(ch), GET_NAME(victim), newlevel, GET_LEVEL(victim));
    log(buf);
    gain_exp_regardless(victim,
                        //total_exp(newlevel-1)+1 - GET_EXP(victim));
                        (newlevel-GET_LEVEL(victim))*LEVELEXP(victim)+1 - GET_EXP(victim));
    save_char(victim, victim->in_room);
}

void restore_all(char *name)
{
    struct char_data *vict;
    struct descriptor_data *d;
    int i;

    for (d = descriptor_list; d; d = d->next)
        if ((d->connected == CON_PLAYING) && !IS_NPC(d->character)) {
            vict = d->character;
            GET_HIT(vict) = GET_MAX_HIT(vict);
            GET_MANA(vict) = GET_MAX_MANA(vict);
            GET_MOVE(vict) = GET_MAX_MOVE(vict);
            GET_COND(vict, DRUNK) = 0;
            GET_COND(vict, THIRST) = 24;
            GET_COND(vict, FULL) = 24;
            update_pos(vict);

            sprintf(buf, "You have been fully restored by %s.\r\n", name);
            if (!IS_GOD(vict))
            	send_to_char(buf, vict);
            if (PLR_FLAGGED(vict, PLR_CRASH)) {
                Crash_crashsave(vict);
                save_char(vict, vict->in_room);
                REMOVE_BIT(PLR_FLAGS(vict), PLR_CRASH);
            }

        }
}

ACMD(do_restore)
{
    struct char_data *vict;
    struct descriptor_data *d;
    int i;

    one_argument(argument, buf);
    if (!*buf)
        send_to_char("Whom do you wish to restore?\r\n", ch);
    else if (!str_cmp(buf, "all"))
        restore_all(GET_NAME(ch));
    else if (!(vict = get_char_vis(ch, buf)))
        send_to_char(NOPERSON, ch);
    else {
        if (GET_LEVEL(vict) == LVL_IMPL) {
            GET_DAMROLL(vict) = 10;
            GET_HITROLL(vict) = 10;
        }
        GET_HIT(vict) = GET_MAX_HIT(vict);
        GET_MANA(vict) = GET_MAX_MANA(vict);
        GET_MOVE(vict) = GET_MAX_MOVE(vict);
        GET_COND(vict, DRUNK) = 0;
        GET_COND(vict, THIRST) = 24;
        GET_COND(vict, FULL) = 24;

        if ((GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
            for (i = 1; i <= MAX_SKILLS; i++)
                SET_SKILL(vict, i, 100);

            if (GET_LEVEL(vict) >= LVL_GRGOD) {
                vict->real_abils.str_add = 0;
                vict->real_abils.intel = 25;
                vict->real_abils.wis = 25;
                vict->real_abils.dex = 25;
                vict->real_abils.str = 25;
                vict->real_abils.con = 25;
                vict->real_abils.cha = 25;
            }
            vict->aff_abils = vict->real_abils;
        }
        sprintf(buf, "You have been fully restored by %s!", GET_NAME(ch));
        send_to_char(buf, vict);

        update_pos(vict);
        send_to_char(OK, ch);
    }
}


void perform_immort_vis(struct char_data * ch)
{
    void appear(struct char_data * ch);

    if (GET_INVIS_LEV(ch) == 0 && !IS_AFFECTED(ch, AFF_HIDE | AFF_INVISIBLE)) {
        send_to_char("You are already fully visible.\r\n", ch);
        return;
    }
    GET_INVIS_LEV(ch) = 0;
    appear(ch);
    send_to_char("You are now fully visible.\r\n", ch);
}


void perform_immort_invis(struct char_data * ch, int level)
{
    struct char_data *tch;

    if (IS_NPC(ch))
        return;

    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if (tch == ch)
            continue;
        if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
            act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
                tch, TO_VICT);
        if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
            act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
                tch, TO_VICT);
    }

    GET_INVIS_LEV(ch) = level;
    sprintf(buf, "Your invisibility level is %d.\r\n", level);
    send_to_char(buf, ch);
}


ACMD(do_invis)
{
    int level;

    if (IS_NPC(ch)) {
        send_to_char("You can't do that!\r\n", ch);
        return;
    }
    one_argument(argument, arg);
    if (!*arg) {
        if (GET_INVIS_LEV(ch) > 0)
            perform_immort_vis(ch);
        else
            perform_immort_invis(ch, GET_LEVEL(ch));
    } else {
        level = atoi(arg);
        if (level > GET_LEVEL(ch))
            send_to_char("You can't go invisible above your own level.\r\n", ch);
        else if (level < 1)
            perform_immort_vis(ch);
        else
            perform_immort_invis(ch, level);
    }
}


ACMD(do_gecho)
{
    struct descriptor_data *pt;

    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (!*argument)
        send_to_char("That must be a mistake...\r\n", ch);
    else {
        sprintf(buf, "%s\r\n", argument);
        for (pt = descriptor_list; pt; pt = pt->next)
            if (!pt->connected && pt->character && pt->character != ch)
                send_to_char(buf, pt->character);
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else
            send_to_char(buf, ch);
    }
}


ACMD(do_poofset)
{
    char **msg;

    switch (subcmd) {
    case SCMD_POOFIN:
        msg = &(POOFIN(ch));
        break;
    case SCMD_POOFOUT:
        msg = &(POOFOUT(ch));
        break;
    default:
        return;
        break;
    }

    skip_spaces(&argument);

    if (*msg)
        DISPOSE(*msg);

    if (!*argument)
        *msg = NULL;
    else
        *msg = str_dup(argument);

    send_to_char(OK, ch);
}



ACMD(do_dc)
{
    struct descriptor_data *d;
    int num_to_dc;

    one_argument(argument, arg);
    if (!(num_to_dc = atoi(arg))) {
        send_to_char("Usage: DC <connection number> (type USERS for a list)\r\n", ch);
        return;
    }
    for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

    if (!d) {
        send_to_char("No such connection.\r\n", ch);
        return;
    }
    if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
        if (!CAN_SEE(ch, d->character))
            send_to_char("No such connection.\r\n", ch);
        else
            send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
        return;
    }
    /* We used to just close the socket here using close_socket(), but
       various people pointed out this could cause a crash if you're
       closing the person below you on the descriptor list.  Just setting
       to CON_CLOSE leaves things in a massively inconsistent state so I
       had to add this new flag to the descriptor. */
    d->close_me = 1;
    sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) Connection closed by %s.", GET_NAME(ch));
    log(buf);
}



ACMD(do_wizlock)
{
    int value;
    char *when;

    one_argument(argument, arg);
    if (*arg) {
        value = atoi(arg);
        if (value < 0 || value > GET_LEVEL(ch)) {
            send_to_char("Invalid wizlock value.\r\n", ch);
            return;
        }
        restrict1 = value;
        when = "now";
    } else
        when = "currently";

    switch (restrict1) {
    case 0:
        sprintf(buf, "The game is %s completely open.\r\n", when);
        break;
    case 1:
        sprintf(buf, "The game is %s closed to new players.\r\n", when);
        break;
    default:
        sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
                restrict1, when);
        break;
    }
    send_to_char(buf, ch);
}


ACMD(do_date)
{
    char *tmstr;
    time_t mytime;
    int d, h, m;
    extern time_t boot_time;

    if (subcmd == SCMD_DATE)
        mytime = time(0);
    else
        mytime = boot_time;

    tmstr = (char *) asctime(localtime(&mytime));
    *(tmstr + strlen(tmstr) - 1) = '\0';

    if (subcmd == SCMD_DATE)
        sprintf(buf, "Current machine time: %s\r\n", tmstr);
    else {
        mytime = time(0) - boot_time;
        d = mytime / 86400;
        h = (mytime / 3600) % 24;
        m = (mytime / 60) % 60;

        sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
                ((d == 1) ? "" : "s"), h, m);
    }

    send_to_char(buf, ch);
}


struct char_data *is_in_game(int idnum) {
    extern struct descriptor_data *descriptor_list;
    struct descriptor_data *i, *next_i;

    for (i = descriptor_list; i; i = next_i) {
        next_i = i->next;
        if (GET_IDNUM(i->character) == idnum) {
            return i->character;
        }
    }
    return NULL;
}

/* altered from stock to the following:
   last [name] [#]
   last without arguments displays the last 10 entries.
   last with a name only displays the 'stock' last entry.
   last with a number displays that many entries (combines with name)
*/



ACMD(do_last)
{
    struct char_file_u chdata;
    int num_to_list=-1;
    int recs,name=0;
    char offend[30];
    char time[128];
    FILE *fp;
    time_t delta;
    struct last_entry mlast;
    struct char_data *temp;

    extern char *class_abbrevs[];

    if(*argument) {
        skip_spaces(&argument);
        while(*argument) {
            argument=one_argument(argument,buf);
            if (isdigit(*buf)) {
                num_to_list=atoi(buf);
                if(num_to_list <=0 ) {
                    send_to_char("You must specify a number greater than 0\r\n",ch);
                    return;
                }
            } else {
                strncpy(offend,buf,29);
                name=1;
            }
        }
    }
    if(name && num_to_list == -1) {
        if  (load_char(offend, &chdata) < 0) {
            send_to_char("There is no such player.\r\n", ch);
            return;
        }
        sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
                chdata.char_specials_saved.idnum, chdata.level,
                class_abbrevs[MIN(9, (int) chdata.class)],
                chdata.name, chdata.host,
                ctime(&chdata.last_logon));
        send_to_char(buf, ch);
        return;
    }

    if(num_to_list <= 0 || num_to_list >= 100) {
        num_to_list=10;
    }
    if(!(fp=fopen(LAST_FILE,"rb"))) {
        send_to_char("No entries found.\r\n",ch);
        return;
    }
    fseek(fp,0L,SEEK_END);
    recs=ftell(fp)/sizeof(struct last_entry);

    sprintf(buf,"Last log\r\n");
    while(num_to_list > 0 && recs > 0) {
        fseek(fp,-1* (sizeof(struct last_entry)),SEEK_CUR);
        fread(&mlast,sizeof(struct last_entry),1,fp);
        fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);
        if(!name ||(name && !strcasecmp(offend,mlast.username))) {
            sprintf(buf,"%s%10.10s %20.20s ",buf,mlast.username,mlast.hostname);
            sprintf(time,"%s",ctime(&mlast.time));
            time[(strlen(time)-1)]='\0';
            sprintf(buf,"%s%16.16s - ",buf,time);
            if(mlast.close_type == LAST_CRASH) {
                temp=is_in_game(mlast.idnum);
                if(temp && mlast.punique == GET_PREF(temp)) {
                    sprintf(buf,"%sStill Playing  ",buf);
                } else {
                    sprintf(buf,"%sCrash          ",buf);
                }
            } else {
                sprintf(time,"%5.5s",ctime(&mlast.close_time)+11);
                sprintf(buf,"%s%s  ",buf,time);
                delta=mlast.close_time - mlast.time;
                sprintf(buf,"%s(%5.5s) ",buf,asctime(gmtime(&delta))+11);
            }
            sprintf(buf,"%s\r\n",buf);
            num_to_list--;
        }
        recs--;
    }
    fclose(fp);
    page_string(ch->desc,buf,1);

}



/*
ACMD(do_last)
{
    struct char_file_u chdata;
    extern char *class_abbrevs[];

    one_argument(argument, arg);
    if (!*arg) {
	send_to_char("For whom do you wish to search?\r\n", ch);
	return;
    }
    if (load_char(arg, &chdata) < 0) {
	send_to_char("There is no such player.\r\n", ch);
	return;
    }
    if ((chdata.level > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMPL)) {
	send_to_char("You are not sufficiently godly for that!\r\n", ch);
	return;
    }
    sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
	    chdata.char_specials_saved.idnum, chdata.level,
	    class_abbrevs[MIN(9, (int) chdata.class)],
	    chdata.name, chdata.host,
	    ctime(&chdata.last_logon));
    send_to_char(buf, ch);
}

*/
ACMD(do_force)
{
    struct descriptor_data *i, *next_desc;
    struct char_data *vict, *next_force;
    char to_force[MAX_INPUT_LENGTH + 2];

    half_chop(argument, arg, to_force);

    sprintf(buf1, "$n has forced you to '%s'.", to_force);

    if (!*arg || !*to_force)
        send_to_char("Whom do you wish to force do what?\r\n", ch);
    else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
        if (!(vict = get_char_vis(ch, arg)))
            send_to_char(NOPERSON, ch);
        else if ((GET_LEVEL(ch) <= GET_LEVEL(vict)) && (!IS_GOD(ch) || (!IS_NPC(vict) && IS_GOD(vict))))
            send_to_char("No, no, no!\r\n", ch);
        else {
            send_to_char(OK, ch);
            //act(buf1, TRUE, ch, NULL, vict, TO_VICT);
            sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
            mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            command_interpreter(vict, to_force);
        }
    } else if (!str_cmp("room", arg)) {
        send_to_char(OK, ch);
        sprintf(buf, "(GC) %s forced room %d to %s", GET_NAME(ch), world[ch->in_room].number, to_force);
        mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

        for (vict = world[ch->in_room].people; vict; vict = next_force) {
            next_force = vict->next_in_room;
            if (GET_LEVEL(vict) >= GET_LEVEL(ch))
                continue;
            act(buf1, TRUE, ch, NULL, vict, TO_VICT);
            command_interpreter(vict, to_force);
        }
    } else {			/* force all */
        send_to_char(OK, ch);
        sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
        mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

        for (i = descriptor_list; i; i = next_desc) {
            next_desc = i->next;

            if (i->connected || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))
                continue;
            act(buf1, TRUE, ch, NULL, vict, TO_VICT);
            command_interpreter(vict, to_force);
        }
    }
}



ACMD(do_wiznet)
{
    struct descriptor_data *d;
    char emote = FALSE;
    char any = FALSE;
    int level = LVL_IMMORT;

    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (!*argument) {
        send_to_char("Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
                     "       wiznet @<level> *<emotetext> | wiz @\r\n", ch);
        return;
    }
    switch (*argument) {
    case '*':
        emote = TRUE;
    case '#':
        one_argument(argument + 1, buf1);
        if (is_number(buf1)) {
            half_chop(argument + 1, buf1, argument);
            level = MAX(atoi(buf1), LVL_IMMORT);
            if (level > GET_LEVEL(ch)) {
                send_to_char("You can't wizline above your own level.\r\n", ch);
                return;
            }
        } else if (emote)
            argument++;
        break;
    case '@':
        for (d = descriptor_list; d; d = d->next) {
            if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
                    !PRF_FLAGGED(d->character, PRF_NOWIZ) &&
                    (CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LVL_IMPL)) {
                if (!any) {
                    sprintf(buf1, "Gods online:\r\n");
                    any = TRUE;
                }
                sprintf(buf1, "%s  %s", buf1, GET_NAME(d->character));
                if (PLR_FLAGGED(d->character, PLR_WRITING))
                    sprintf(buf1, "%s (Writing)\r\n", buf1);
                else if (PLR_FLAGGED(d->character, PLR_MAILING))
                    sprintf(buf1, "%s (Writing mail)\r\n", buf1);
                else if (PLR_FLAGGED(d->character, PLR_EDITING))
                    sprintf(buf1, "%s (editing)\r\n", buf1);
                else
                    sprintf(buf1, "%s\r\n", buf1);

            }
        }
        any = FALSE;
        for (d = descriptor_list; d; d = d->next) {
            if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
                    PRF_FLAGGED(d->character, PRF_NOWIZ) &&
                    CAN_SEE(ch, d->character)) {
                if (!any) {
                    sprintf(buf1, "%sGods offline:\r\n", buf1);
                    any = TRUE;
                }
                sprintf(buf1, "%s  %s\r\n", buf1, GET_NAME(d->character));
            }
        }
        send_to_char(buf1, ch);
        return;
        break;
    case '\\':
        ++argument;
        break;
    default:
        break;
    }
    if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
        send_to_char("You are offline!\r\n", ch);
        return;
    }
    skip_spaces(&argument);

    if (!*argument) {
        send_to_char("Don't bother the gods like that!\r\n", ch);
        return;
    }
    if (level > LVL_IMMORT) {
        sprintf(buf1, "%s: <%d> %s%s\r\n", GET_NAME(ch), level,
                emote ? "<--- " : "", argument);
        sprintf(buf2, "Someone: <%d> %s%s\r\n", level, emote ? "<--- " : "",
                argument);
    } else {
        sprintf(buf1, "%s: %s%s\r\n", GET_NAME(ch), emote ? "<--- " : "",
                argument);
        sprintf(buf2, "Someone: %s%s\r\n", emote ? "<--- " : "", argument);
    }

    for (d = descriptor_list; d; d = d->next) {
        if ((!d->connected) && (GET_LEVEL(d->character) >= level) &&
                (!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
                (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING | PLR_EDITING))
                && (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
            send_to_char(CCCYN(d->character, C_NRM), d->character);
            if (CAN_SEE(d->character, ch))
                send_to_char(buf1, d->character);
            else
                send_to_char(buf2, d->character);
            send_to_char(CCNRM(d->character, C_NRM), d->character);
        }
    }

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
}


ACMD(do_zreset)
{
    void reset_zone(int zone);

    int i, j;

    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("You must specify a zone.\r\n", ch);
        return;
    }
    if (*arg == '*') {
        for (i = 0; i <= top_of_zone_table; i++)
            reset_zone(i);
        send_to_char("Reset world.\r\n", ch);
        sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
        return;
    } else if (*arg == '.')
        i = world[ch->in_room].zone;
    else {
        j = atoi(arg);
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].number == j)
                break;
    }
    if (i >= 0 && i <= top_of_zone_table) {
        reset_zone(i);
        sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
                zone_table[i].name);
        send_to_char(buf, ch);
        sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
        mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
    } else
        send_to_char("Invalid zone number.\r\n", ch);
}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
    struct char_data *vict;
    long result;
    void roll_real_abils(struct char_data * ch);

    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Yes, but for whom?!?\r\n", ch);
    else if (!(vict = get_char_vis(ch, arg)))
        send_to_char("There is no such player.\r\n", ch);
    else if (IS_NPC(vict))
        send_to_char("You can't do that to a mob!\r\n", ch);
    else if (GET_LEVEL(vict) > GET_LEVEL(ch))
        send_to_char("Hmmm...you'd better not.\r\n", ch);
    else {
        switch (subcmd) {
        case SCMD_REROLL:
            send_to_char("Rerolled...\r\n", ch);
            roll_real_abils(vict);
            sprintf(buf, "(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
            log(buf);
            sprintf(buf, "New stats: Str %d/%d, Int %d, Wis %d, Dex %d, Con %d, Cha %d\r\n",
                    GET_STR(vict), GET_ADD(vict), GET_INT(vict), GET_WIS(vict),
                    GET_DEX(vict), GET_CON(vict), GET_CHA(vict));
            send_to_char(buf, ch);
            break;
        case SCMD_PARDON:
            if (!PLR_FLAGGED(vict, PLR_THIEF | PLR_KILLER)) {
                send_to_char("Your victim is not flagged.\r\n", ch);
                return;
            }
            REMOVE_BIT(PLR_FLAGS(vict), PLR_THIEF | PLR_KILLER);
            send_to_char("Pardoned.\r\n", ch);
            send_to_char("You have been pardoned by the Gods!\r\n", vict);
            sprintf(buf, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
            mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            break;
        case SCMD_NOTITLE:
            result = PLR_TOG_CHK(vict, PLR_NOTITLE);
            sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
                    GET_NAME(vict), GET_NAME(ch));
            mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            strcat(buf, "\r\n");
            send_to_char(buf, ch);
            break;
        case SCMD_SQUELCH:
            result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
            sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
                    GET_NAME(vict), GET_NAME(ch));
            mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            strcat(buf, "\r\n");
            send_to_char(buf, ch);
            break;
        case SCMD_FREEZE:
            if (ch == vict) {
                send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
                return;
            }
            if (PLR_FLAGGED(vict, PLR_FROZEN)) {
                send_to_char("Your victim is already pretty cold.\r\n", ch);
                return;
            }
            SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
            GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
            send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n", vict);
            send_to_char("Frozen.\r\n", ch);
            act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
            sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
            mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            break;
        case SCMD_THAW:
            if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
                send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
                return;
            }
            if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
                sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
                        GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
                send_to_char(buf, ch);
                return;
            }
            sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
            mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
            send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
            send_to_char("Thawed.\r\n", ch);
            act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
            break;
        case SCMD_UNAFFECT:
            if (vict->affected) {
                while (vict->affected)
                    affect_remove(vict, vict->affected);
                send_to_char("There is a brief flash of light!\r\n"
                             "You feel slightly different.\r\n", vict);
                send_to_char("All spells removed.\r\n", ch);
            } else {
                send_to_char("Your victim does not have any affections!\r\n", ch);
                return;
            }
            break;

        case SCMD_TOAD:
            if (ch == vict) {
                send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
                return;
            }
            if (PLR_FLAGGED(vict, PLR_TOAD)) {
                send_to_char("Your victim is already hopping mad.\r\n", ch);
                return;
            }
            SET_BIT(PLR_FLAGS(vict), PLR_TOAD);
            send_to_char("You have pissed off the gods. Zap! You're a toad!\r\nYou have an odd longing for flies...\r\n", vict);
            send_to_char("Cool. You've got a toad.\r\n", ch);
            act("The gods are pissed off and have turned $n into a toad!", FALSE, vict, 0, 0, TO_ROOM);

            if (GET_LEVEL(ch) != LVL_IMPL) {
                sprintf(buf, "(GC) %s has achieved toadhood, courtesy of %s.", GET_NAME(vict), GET_NAME(ch));
                mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            }

            break;
        case SCMD_TOADOFF:
            if (!PLR_FLAGGED(vict, PLR_TOAD)) {
                send_to_char("Sorry, your victim is not a toad at this moment...should he be...?\r\n", ch);
                return;
            }

            if (GET_LEVEL(ch) != LVL_IMPL) {
                sprintf(buf, "(GC) %s returned to original state by %s.", GET_NAME(vict), GET_NAME(ch));
                mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            }

            REMOVE_BIT(PLR_FLAGS(vict), PLR_TOAD);
            send_to_char("The gods have shown you mercy, you wretch.\r\nYour opposable thumbs have returned.\r\n", vict);
            send_to_char("Weeeelllllll...alright. If you wish.\r\n", ch);
            act("$n stops hopping around and returns to human form!", FALSE, vict, 0, 0, TO_ROOM);
            break;

        default:
            log("SYSERR: Unknown subcmd passed to do_wizutil (act.wizard.c)");
            break;
        }
        save_char(vict,vict->in_room);
    }
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, int zone)
{
    sprintf(bufptr, "%s%3d %-28.28s Reset:%3d/%3d (%1d) Top: %5d %3d/%3d/%3d/%3d R:%d\r\n",
            bufptr, zone_table[zone].number, zone_table[zone].name,
            zone_table[zone].age, zone_table[zone].lifespan,
            zone_table[zone].reset_mode, zone_table[zone].top, zone_table[zone].num,  zone_table[zone].min,zone_table[zone].max, zone_table[zone].avg, zone_table[zone].calc_ratio);
}
void print_zone_to_buf_mort(char *bufptr, int zone)
{
    /*  sprintf(bufptr, "%s  %-35.35s by: %-12.12s [lvs:%3d to%3d]\r\n",
    	  bufptr, zone_table[zone].name, zone_table[zone].creator,
    	  zone_table[zone].lvl_low, zone_table[zone].lvl_high);*/
    sprintf(bufptr, "%s  %-35.35s\r\n", bufptr, zone_table[zone].name);
}


char *get_event_name(struct char_data *ch)
{
    sprintf(buf2, "");
    if (GET_WEAR_EVENT(ch))
        sprintf(buf2, "WEAR");
    if (GET_UTIL_EVENT(ch))
        strcat(buf2, "UTIL ");
    if (GET_MOVE_EVENT(ch))
        strcat(buf2, "MOVE ");
    if (GET_SPELL_EVENT(ch))
        strcat(buf2, "SPELL ");
    if (GET_FIGHT_EVENT(ch))
        strcat(buf2, "FIGHT ");
    return buf2;
}

struct ss {
    char ime[100];
    int score;
} ess;
struct ss niz[4000];

int maxniz=0;
void sortniz()
{
    int i,j,k;

    for (i=0;i<maxniz-1;i++)
        for (j=i+1;j<maxniz;j++)
            if (niz[i].score<niz[j].score)
            {
                ess=niz[i];
                niz[i]=niz[j];
                niz[j]=ess;
            }
}



ACMD(do_show)
{
    struct char_file_u vbuf;
    int i, j, k, l, con, lee, num;
    char self = 0;
    struct char_data *vict;
    struct obj_data *obj;
    char field[40], value[40], birth[80];
    extern int buf_switches, buf_largecount, buf_overflows;
    struct leech_type *hjp;
    void show_shops(struct char_data * ch, char *value);

    struct show_struct {
        char *cmd;
        char level;
    } fields[] = {
                     {
                         "nothing", 0
                     },			/* 0 */
                     {
                         "zones", LVL_GOD
                     },			/* 1 */
                     {
                         "player", LVL_GOD
                     },
                     {
                         "rent", LVL_GOD
                     },
                     {
                         "stats", LVL_IMMORT
                     },
                     {
                         "errors", LVL_IMPL
                     },			/* 5 */
                     {
                         "death", LVL_GOD
                     },
                     {
                         "godrooms", LVL_GOD
                     },
                     {
                         "shops", LVL_IMMORT
                     },
                     {
                         "teleport", LVL_GOD
                     },
                     {
                         "areas", 1
                     },			/* 10 */
                     {
                         "status", LVL_GOD
                     },
                     {
                         "pfile", LVL_GOD
                     },
                     {
                         "arena", LVL_GOD
                     },
                     {
                         "houses", LVL_GRGOD
                     },
                     {
                         "traps", LVL_GRGOD
                     },
                     {
                         "events", LVL_GRGOD
                     },
                     {
                         "bestzones", 1
                     },
                     {
                         "worsezone", 1
                     },
                     {
                         "\n", 0
                     }
                 };

    skip_spaces(&argument);

    if (!*argument) {
        strcpy(buf, "Show options:\r\n");
        for (j = 0, i = 1; fields[i].level; i++)
            if (fields[i].level <= GET_LEVEL(ch))
                sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
        return;
    }
    strcpy(arg, two_arguments(argument, field, value));

    for (l = 0; *(fields[l].cmd) != '\n'; l++)
        if (!strncmp(field, fields[l].cmd, strlen(field)))
            break;

    if (GET_LEVEL(ch) < fields[l].level) {
        send_to_char("You are not godly enough for that!\r\n", ch);
        return;
    }
    if (!strcmp(value, "."))
        self = 1;
    buf[0] = '\0';
    switch (l) {
    case 1:			/* zone */
        /* tightened up by JE 4/6/93 */
        if (self) {
            print_zone_to_buf(buf, world[ch->in_room].zone);
        } else if (*value && is_number(value)) {
            for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
            if (i <= top_of_zone_table)
                print_zone_to_buf(buf, i);
            else {
                send_to_char("That is not a valid zone.\r\n", ch);
                return;
            }
        } else {
            for (i = 0; i <= top_of_zone_table; i++)
                print_zone_to_buf(buf, i);
        }
        send_to_char(buf, ch);
        break;
    case 2:			/* player */
        if (!*value) {
            send_to_char("A name would help.\r\n", ch);
            return;
        }
        if (load_char(value, &vbuf) < 0) {
            send_to_char("There is no such player.\r\n", ch);
            return;
        }
        sprintf(buf, "Player: %-12s (%s) [%2d]\r\n", vbuf.name,
                genders[(int) vbuf.sex], vbuf.level);
        sprintf(buf,
                "%sAu: %-8d  Bal: %-8d  Exp: %-8d  Align: %-5d  Lessons: %-3d\r\n",
                buf, vbuf.points.gold, vbuf.points.bank_gold, vbuf.points.exp,
                vbuf.char_specials_saved.alignment,
                vbuf.player_specials_saved.spells_to_learn);
        strcpy(birth, ctime(&vbuf.birth));
        sprintf(buf,
                "%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
                buf, birth, ctime(&vbuf.last_logon), (int) (vbuf.played / 3600),
                (int) (vbuf.played / 60 % 60));
        send_to_char(buf, ch);
        break;
    case 3:
        Crash_listrent(ch, value);
        break;
    case 4:
        i = 0;
        j = 0;
        k = 0;
        con = 0;
        lee=0;
        for (hjp = leech_list; hjp; hjp = hjp->next, lee++);
        for (vict = character_list; vict; vict = vict->next) {
            if (IS_NPC(vict))
                j++;
            else if (CAN_SEE(ch, vict)) {
                i++;
                if (vict->desc)
                    con++;
            }
        }
        for (obj = object_list; obj; obj = obj->next)
            k++;
        sprintf(buf, "Current stats:\r\n");
        sprintf(buf, "%s  %5d players in game  %5d connected\r\n", buf, i, con);
        sprintf(buf, "%s  %5d registered\r\n", buf, top_of_p_table + 1);
        sprintf(buf, "%s  %5d mobiles          %5d prototypes\r\n",
                buf, j, top_of_mobt + 1);
        sprintf(buf, "%s  %5d objects          %5d prototypes\r\n",
                buf, k, top_of_objt + 1);
        sprintf(buf, "%s  %5d rooms            %5d zones\r\n",
                buf, top_of_world + 1, top_of_zone_table + 1);
        sprintf(buf, "%s  %5d elements in leech list\r\n",
                buf, lee);
        sprintf(buf, "%s  %5d large bufs\r\n", buf, buf_largecount);
        sprintf(buf, "%s  %5d buf switches     %5d overflows\r\n", buf,
                buf_switches, buf_overflows);
        sprintf(buf, "%s  %5d chars in queue   %5d objs in queue\r\n", buf,
                cur_qchars, cur_qobjs);
        send_to_char(buf, ch);
        break;
    case 5:
        strcpy(buf, "Errant Rooms in file\r\n--------------------\r\n");
        for (i = 0, k = 0; i <= top_of_world; i++)
            for (j = 0; j < NUM_OF_DIRS; j++)
                if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
                    sprintf(buf, "%s%2d: [%5d] %s (%s)\r\n", buf, ++k, world[i].number,
                            world[i].name, dirs[j]);
        send_to_char(buf, ch);
        strcpy(buf, "\r\nErrant Rooms\r\n------------\r\n");
        for (i = 0, k = 0; i <= top_of_world; i++)
            for (j = 0; j < NUM_OF_DIRS; j++)
                if (world[i].dir_option[j] && (world[world[i].dir_option[j]->to_room].number == 0) && !ROOM_FLAGGED(i, ROOM_ARENA))
                    sprintf(buf, "%s%2d: [%5d] %s (%s)\r\n", buf, ++k, world[i].number,
                            world[i].name, dirs[j]);
        send_to_char(buf, ch);
        break;
    case 6:
        strcpy(buf, "Death Traps\r\n-----------\r\n");
        for (i = 0, j = 0; i <= top_of_world; i++)
            if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
                sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j,
                        world[i].number, world[i].name);
        send_to_char(buf, ch);
        break;
    case 7:
#define GOD_ROOMS_ZONE 12
        strcpy(buf, "Godrooms\r\n--------------------------\r\n");
        for (i = 0, j = 0; i < top_of_world; i++)
            if (ROOM_FLAGGED(i, ROOM_GODROOM))
                sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j, world[i].number, world[i].name);
        send_to_char(buf, ch);
        break;
    case 8:
        show_shops(ch, value);
        break;
    case 9:			/* teleports */
        strcpy(buf, "Teleport Rooms\r\n-----------\r\n");
        /*	for (i = 0, j = 0; i <= top_of_world; i++)
        	    if (world[i].tele != NULL)
        		sprintf(buf, "%s%2d: [%5d] %-24.24s Targ: %5d\r\n", buf, ++j,
        		    world[i].number, world[i].name, world[i].tele->targ);*/
        send_to_char(buf, ch);
        break;
    case 10:
        send_to_char("Available Areas for your Perusing Pleasure:\r\n", ch);
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].number >= 40 || GET_LEVEL(ch) > LVL_IMMORT)
                print_zone_to_buf_mort(buf, i);
        send_to_char(buf, ch);
        break;
    case 11:			/* stats of a player */
        if (!*value) {
            send_to_char("A name would help.\r\n", ch);
            return;
        }
        if (load_char(value, &vbuf) < 0) {
            send_to_char("There is no such player.\r\n", ch);
            return;
        }
        sprintf(buf, "%s is a level %2d %s %s.\r\n", vbuf.name, vbuf.level,
                genders[(int) vbuf.sex], pc_race_types[(int) vbuf.race]);
        sprintf(buf, "%s%s seems %s to you.\r\n",
                buf, vbuf.name,
                align_types[get_alignment_type(vbuf.char_specials_saved.alignment)]);
        /*    sprintf(buf, "%s%s has been killed %d times, and has killed %d times.\r\n",
        	    buf, vbuf.name,
        	    GET_REINCARN(&vbuf), vbuf.player_specials_saved.killed_by_player +
        		vbuf.player_specials_saved.killed_by_mob,
        		vbuf.player_specials_saved.killed_player);*/

        send_to_char(buf, ch);
        break;
    case 12:
        showplay(ch);
        break;
    case 13:
        if (in_arena == ARENA_OFF) {
            sprintf(buf, "The Arena is closed right now.\r\n");
        } else if (in_arena == ARENA_START) {
            sprintf(buf, "Arena will start in %d hour(s)\r\n", time_to_start);
            sprintf(buf, "%sIt will last for %d hour(s)\r\n", buf, game_length);
        } else if (in_arena == ARENA_RUNNING) {
            sprintf(buf, "Arena will end in %d hour(s)\r\n", time_left_in_game);
        }
        send_to_char(buf, ch);
        break;
    case 14:
        hcontrol_list_houses(ch);
        break;
    case 15:
        {
            struct raff_node *raff;
            extern struct raff_node *raff_list;
            send_to_char("Traps:\r\n------\r\n\r\n", ch);
            if (raff_list)
                for (raff=raff_list; raff; raff=raff->next)
                {
                    if (raff->affection==RAFF_TRAP)
                    {
                        sprintf(buf, "[&c%5d&0] - %-35s &c%s&0 set by &c%s&0\r\n", GET_ROOM_VNUM(raff->room), world[raff->room].name, spells[raff->spell], raff->name);
                        send_to_char(buf, ch);
                    }

                }

        }
        break;
    case 16:
        send_to_char("Events:\r\n-------\r\n\r\n", ch);
        for (vict = character_list; vict; vict = vict->next)
            if (IN_EVENT_FULL(vict))
            {
                sprintf(buf, "&c%-25s &w%s&0\r\n", GET_NAME(vict), get_event_name(vict));
                send_to_char(buf, ch);
            }
        break;
    case 17:
        maxniz=0;
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].calc_ratio>100)
            {
                sprintf(niz[maxniz].ime, "%s",zone_table[i].name);
                niz[maxniz].score= zone_table[i].calc_ratio;
                maxniz++;
            }

        if (maxniz)
        {
            sortniz();
            maxniz=MIN(10, maxniz);
            send_to_char("Best rated zones:\r\n\r\n", ch);
            for (num=0;num<maxniz;num++)
            {
                sprintf(buf,"&w%4d&0. &c%-40s  %3d&0\r\n",num+1, niz[num].ime, niz[num].score);
                send_to_char(buf, ch);
            }
        }

        break;
    case 18:
        maxniz=0;
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].calc_ratio<100)
            {
                sprintf(niz[maxniz].ime, "%s",zone_table[i].name);
                niz[maxniz].score= zone_table[i].calc_ratio;
                maxniz++;
            }

        if (maxniz)
        {
            sortniz();

            send_to_char("Worse rated zones:\r\n\r\n", ch);
            for (num=maxniz-1;num>=MAX(0, maxniz-10);num--)
            {
                sprintf(buf,"&w%4d&0. &c%-40s  %3d&0\r\n",maxniz-num, niz[num].ime, niz[num].score);
                send_to_char(buf, ch);
            }
        }

        break;

    default:
        send_to_char("Sorry, I don't understand that.\r\n", ch);
        break;
    }
}


ACMD(do_bwzone)
{

    int i,num;
    switch (subcmd)
    {
    case SCMD_BESTZONE:
        maxniz=0;
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].calc_ratio>100)
            {
                sprintf(niz[maxniz].ime, "%s",zone_table[i].name);
                niz[maxniz].score= zone_table[i].calc_ratio;
                maxniz++;
            }

        if (maxniz)
        {
            sortniz();
            maxniz=MIN(5, maxniz);
            send_to_char("Best rated zones:\r\n\r\n", ch);
            for (num=0;num<maxniz;num++)
            {
                sprintf(buf,"&w%4d&0. &c%-40s&0\r\n",num+1, niz[num].ime);
                send_to_char(buf, ch);
            }
        }

        break;
    case SCMD_WORSEZONE:
        maxniz=0;
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].calc_ratio<100)
            {
                sprintf(niz[maxniz].ime, "%s",zone_table[i].name);
                niz[maxniz].score= zone_table[i].calc_ratio;
                maxniz++;
            }

        if (maxniz)
        {
            sortniz();

            send_to_char("Worse rated zones:\r\n\r\n", ch);
            for (num=maxniz-1;num>=MAX(0, maxniz-5);num--)
            {
                sprintf(buf,"&w%4d&0. &c%-40s&0\r\n",maxniz-num, niz[num].ime);
                send_to_char(buf, ch);
            }
        }
    default: break;
    }

}

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT(flagset, flags); \
	else if (off) REMOVE_BIT(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))

ACMD(do_set)
{
    int i, l;
    struct char_data *vict = NULL;
    struct char_data *cbuf = NULL;
    struct char_file_u tmp_store;
    char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], val_arg[MAX_INPUT_LENGTH];
    int on = 0, off = 0, value = 0;
    char is_file = 0, is_mob = 0, is_player = 0;
    int player_i = 0;
    int parse_class(char arg, int race);
    int parse_race(char arg);

    struct set_struct {
        char *cmd;
        char level;
        char pcnpc;
        char type;
    } fields[] = {
                     {
                         "brief", LVL_GOD, PC, BINARY
                     },			/* 0 */
                     {
                         "invstart", LVL_GOD, PC, BINARY
                     },			/* 1 */
                     {
                         "title", LVL_GOD, PC, MISC
                     },
                     {
                         "nosummon", LVL_GRGOD, PC, BINARY
                     },
                     {
                         "maxhit", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "maxmana", LVL_GRGOD, BOTH, NUMBER
                     },			/* 5 */
                     {
                         "maxmove", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "hit", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "mana", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "move", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "align", LVL_GOD, BOTH, NUMBER
                     },			/* 10 */
                     {
                         "str", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "stradd", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "int", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "wis", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "dex", LVL_GRGOD, BOTH, NUMBER
                     },			/* 15 */
                     {
                         "con", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "sex", LVL_GRGOD, BOTH, MISC
                     },
                     {
                         "ac", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "coins", LVL_GOD, BOTH, NUMBER
                     },
                     {
                         "bank", LVL_GOD, PC, NUMBER
                     },			/* 20 */
                     {
                         "exp", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "hitroll", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "damroll", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "invis", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "nohassle", LVL_GRGOD, PC, BINARY
                     },			/* 25 */
                     {
                         "frozen", LVL_FREEZE, PC, BINARY
                     },
                     {
                         "practices", LVL_GRGOD, PC, NUMBER
                     },
                     {
                         "lessons", LVL_GRGOD, PC, NUMBER
                     },
                     {
                         "drunk", LVL_GRGOD, BOTH, MISC
                     },
                     {
                         "hunger", LVL_GRGOD, BOTH, MISC
                     },			/* 30 */
                     {
                         "thirst", LVL_GRGOD, BOTH, MISC
                     },
                     {
                         "killer", LVL_GOD, PC, BINARY
                     },
                     {
                         "thief", LVL_GOD, PC, BINARY
                     },
                     {
                         "level", LVL_IMPL, BOTH, NUMBER
                     },
                     {
                         "room", LVL_IMPL, BOTH, NUMBER
                     },			/* 35 */
                     {
                         "roomflag", LVL_GRGOD, PC, BINARY
                     },
                     {
                         "siteok", LVL_GRGOD, PC, BINARY
                     },
                     {
                         "deleted", LVL_IMPL, PC, BINARY
                     },
                     {
                         "class", LVL_GRGOD, BOTH, MISC
                     },
                     {
                         "nowizlist", LVL_GOD, PC, BINARY
                     },			/* 40 */
                     {
                         "quest", LVL_GOD, PC, BINARY
                     },
                     {
                         "loadroom", LVL_GRGOD, PC, MISC
                     },
                     {
                         "color", LVL_GOD, PC, BINARY
                     },
                     {
                         "idnum", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "passwd", LVL_IMPL, PC, MISC
                     },			/* 45 */
                     {
                         "nodelete", LVL_GOD, PC, BINARY
                     },
                     {
                         "cha", LVL_GRGOD, BOTH, NUMBER
                     },
                     {
                         "race", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "bounty", LVL_IMPL, PC, BINARY
                     },
                     {
                         "assassin", LVL_IMPL, PC, BINARY
                     },
                     {
                         "druhari", LVL_IMPL, PC, BINARY
                     },
                     {
                         "yllantra", LVL_IMPL, PC, BINARY
                     },
                     {
                         "retire", LVL_IMPL, PC, BINARY
                     },
                     {
                         "red", LVL_IMPL, PC, BINARY
                     },
                     {
                         "blue", LVL_IMPL, PC, BINARY
                     },
                     {
                         "olc", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "qp", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "pks", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "pkk", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "name", LVL_IMPL, PC, MISC
                     },
                     {
                         "clan", LVL_IMPL, PC, NUMBER
                     },
                     {
                         "private", LVL_IMPL, PC, MISC
                     },
                     {
                         "deity", LVL_IMPL, BOTH, MISC
                     },
                     {
                         "faith", LVL_IMPL, BOTH, NUMBER
                     },
                     {
                         "weight", LVL_IMPL, BOTH, NUMBER
                     },
                     {
                         "height", LVL_IMPL, BOTH, NUMBER
                     },
           
                     {
                         "\n", 0, BOTH, MISC
                     }
                 };

    half_chop(argument, name, buf);
    if (!strcmp(name, "file")) {
        is_file = 1;
        half_chop(buf, name, buf);
    } else if (!str_cmp(name, "player")) {
        is_player = 1;
        half_chop(buf, name, buf);
    } else if (!str_cmp(name, "mob")) {
        is_mob = 1;
        half_chop(buf, name, buf);
    }
    half_chop(buf, field, buf);
    strcpy(val_arg, buf);

    if (!*name || !*field) {
        send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
        return;
    }
    if (!is_file) {
        if (is_player) {
            if (!(vict = get_player_vis(ch, name, 0))) {
                send_to_char("There is no such player.\r\n", ch);
                return;
            }
        } else {
            if (!(vict = get_char_vis(ch, name))) {
                send_to_char("There is no such creature.\r\n", ch);
                return;
            }
        }
    } else if (is_file) {
        CREATE(cbuf, struct char_data, 1);
        clear_char(cbuf);
        if ((player_i = load_char(name, &tmp_store)) > -1) {
            store_to_char(&tmp_store, cbuf);
            if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)) {
                free_char(cbuf);
                send_to_char("Sorry, you can't do that.\r\n", ch);
                return;
            }
            vict = cbuf;
        } else {
            DISPOSE(cbuf);
            send_to_char("There is no such player.\r\n", ch);
            return;
        }
    }
    if (GET_LEVEL(ch) != LVL_IMPL) {
        if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
            send_to_char("Maybe that's not such a great idea...\r\n", ch);
            return;
        }
    }
    for (l = 0; *(fields[l].cmd) != '\n'; l++)
        if (!strncmp(field, fields[l].cmd, strlen(field)))
            break;

    if (GET_LEVEL(ch) < fields[l].level) {
        send_to_char("You are not godly enough for that!\r\n", ch);
        return;
    }
    if (IS_NPC(vict) && !(fields[l].pcnpc && PC)) {
        send_to_char("You can't do that to a beast!\r\n", ch);
        return;
    } else if (!IS_NPC(vict) && !(fields[l].pcnpc & PC)) {
        send_to_char("That can only be done to a beast!\r\n", ch);
        return;
    }
    if (fields[l].type == BINARY) {
        if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
            on = 1;
        else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
            off = 1;
        if (!(on || off)) {
            send_to_char("Value must be on or off.\r\n", ch);
            return;
        }
    } else if (fields[l].type == NUMBER) {
        value = atoi(val_arg);
    }
    strcpy(buf, "Okay.");	/* can't use OK macro here 'cause of \r\n */
    switch (l) {
    case 0:
        SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
        break;
    case 1:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
        break;
    case 2:
        set_title(vict, val_arg);
        sprintf(buf, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
        break;
    case 3:
        SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
        on = !on;		/* so output will be correct */
        break;
    case 4:
        GET_HITR(vict)=vict->points.max_hit = GET_MAX_HIT(vict)=RANGE(1, 30000);
        affect_total(vict);
        break;
    case 5:
        GET_MANAR(vict)=vict->points.max_mana = RANGE(1, 30000);
        affect_total(vict);
        break;
    case 6:
        GET_MOVER(vict)=vict->points.max_move = RANGE(1, 30000);
        affect_total(vict);
        break;
    case 7:
        //GET_HIT(vict)=vict->points.hit = RANGE(-9, vict->vitality);
        vict->points.hit = RANGE(-9, vict->points.max_hit);
        affect_total(vict);
        break;
    case 8:
        vict->points.mana = RANGE(0, vict->points.max_mana);
        affect_total(vict);
        break;
    case 9:
        vict->points.move = RANGE(0, vict->points.max_move);
        affect_total(vict);
        break;
    case 10:
        GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
        affect_total(vict);
        break;
    case 11:
        RANGE(3, 25);
        vict->real_abils.str = value;
        vict->real_abils.str_add = 0;
        affect_total(vict);
        break;
    case 12:
        vict->real_abils.str_add = RANGE(0, 100);
        if (value > 0)
            vict->real_abils.str = 18;
        affect_total(vict);
        break;
    case 13:
        RANGE(3, 25);
        vict->real_abils.intel = value;
        affect_total(vict);
        break;
    case 14:
        RANGE(3, 25);
        vict->real_abils.wis = value;
        affect_total(vict);
        break;
    case 15:
        RANGE(3, 25);
        vict->real_abils.dex = value;
        affect_total(vict);
        break;
    case 16:
        RANGE(3, 25);
        vict->real_abils.con = value;
        affect_total(vict);
        break;
    case 17:
        if (!str_cmp(val_arg, "male"))
            vict->player.sex = SEX_MALE;
        else if (!str_cmp(val_arg, "female"))
            vict->player.sex = SEX_FEMALE;
        else if (!str_cmp(val_arg, "neutral"))
            vict->player.sex = SEX_NEUTRAL;
        else {
            send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
            return;
        }
        break;
    case 18:
        vict->points.armor = RANGE(-200, 200);
        affect_total(vict);
        break;
    case 19:
        GET_GOLD(vict) = RANGE(0, 100000000);
        break;
    case 20:
        GET_BANK_GOLD(vict) = RANGE(0, 100000000);
        break;
    case 21:
        vict->points.exp = RANGE(0, 50000000);
        break;
    case 22:
        vict->points.hitroll = RANGE(-200, 200);
        affect_total(vict);
        break;
    case 23:
        vict->points.damroll = RANGE(-200, 200);
        affect_total(vict);
        break;
    case 24:
        if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
            send_to_char("You aren't godly enough for that!\r\n", ch);
            return;
        }
        GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
        break;
    case 25:
        if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
            send_to_char("You aren't godly enough for that!\r\n", ch);
            return;
        }
        SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
        break;
    case 26:
        if (ch == vict) {
            send_to_char("Better not -- could be a long winter!\r\n", ch);
            return;
        }
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
        break;
    case 27:
    case 28:
        GET_PRACTICES(vict) = RANGE(0, 200);
        break;
    case 29:
    case 30:
    case 31:
        if (!str_cmp(val_arg, "off")) {
            GET_COND(vict, (l - 29)) = (char) -1;
            sprintf(buf, "%s's %s now off.", GET_NAME(vict), fields[l].cmd);
        } else if (is_number(val_arg)) {
            value = atoi(val_arg);
            RANGE(0, 72);
            GET_COND(vict, (l - 29)) = (char) value;
            sprintf(buf, "%s's %s set to %d.", GET_NAME(vict), fields[l].cmd,
                    value);
        } else {
            send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
            return;
        }
        break;
    case 32:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
        break;
    case 33:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
        break;
    case 34:
        if ((value > GET_LEVEL(ch) || value > LVL_IMPL) ||
                (GET_LEVEL(vict) == GET_LEVEL(ch) && GET_LEVEL(ch)!=LVL_IMPL)) {
            send_to_char("You can't do that.\r\n", ch);
            return;
        }
        RANGE(0, LVL_IMPL);
        vict->player.level = (byte) value;
        break;
    case 35:
        if ((i = real_room(value)) < 0) {
            send_to_char("No room exists with that number.\r\n", ch);
            return;
        }
        char_from_room(vict);
        char_to_room(vict, i);
        break;
    case 36:
        SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
        break;
    case 37:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
        break;
    case 38:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
        break;
    case 39:
        if ((i = parse_class(*val_arg, GET_RACE(vict))) == CLASS_UNDEFINED) {
            send_to_char("No such class.\r\n", ch);
            return;
        }
        GET_CLASS(vict) = i;
        break;
    case 40:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
        break;
    case 41:
        SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
        break;
    case 42:
        if (!str_cmp(val_arg, "on"))
            SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
        else if (!str_cmp(val_arg, "off"))
            REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
        else {
            if (real_room(i = atoi(val_arg)) > -1) {
                GET_LOADROOM(vict) = i;
                sprintf(buf, "%s will enter at %d.", GET_NAME(vict),
                        GET_LOADROOM(vict));
            } else
                sprintf(buf, "That room does not exist!");
        }
        break;
    case 43:
        SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
        break;
    case 44:
        if (GET_IDNUM(ch) != 1 || !IS_NPC(vict))
            return;
        GET_IDNUM(vict) = value;
        break;
    case 45:
        if (!is_file)
            return;
        /*	if (GET_IDNUM(ch) > 1) {
        	    send_to_char("Please don't use this command, yet.\r\n", ch);
        	    return;
        	}*/
        if (GET_LEVEL(vict) >= LVL_GRGOD) {
            send_to_char("You cannot change that.\r\n", ch);
            return;
        }
        strcpy(tmp_store.pwd, val_arg);
        tmp_store.pwd[MAX_PWD_LENGTH] = '\0';
        strcpy(GET_PASSWD(vict), val_arg);
        *(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';

        sprintf(buf, "Password changed to '%s'.", val_arg);
        break;
    case 46:
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
        break;
    case 47:
        if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
            RANGE(3, 25);
        else
            RANGE(3, 25);
        vict->real_abils.cha = value;
        affect_total(vict);
        break;
    case 48:
        if ((i = parse_race(*val_arg)) == CLASS_UNDEFINED) {
            send_to_char("That is not a valid race.\r\n", ch);
            return;
        }
        if (i<0)
            i=(-i-1);
        GET_RACE(vict) = i;
        break;
    case 49:
        //SET_OR_REMOVE(PRF2_FLAGS(vict), PRF2_BOUNTYHUNT);
        break;
    case 50:
        SET_OR_REMOVE(PRF2_FLAGS(vict), PRF2_ASSASSIN);
        break;
    case 51:
        //	SET_OR_REMOVE(PRF2_FLAGS(vict), PRF2_WAR_DRUHARI);
        break;
    case 52:
        //	SET_OR_REMOVE(PRF2_FLAGS(vict), PRF2_WAR_YLLANTRA);
        break;
    case 53:
        SET_OR_REMOVE(PRF2_FLAGS(vict), PRF2_RETIRED);
        break;
    case 54:
        SET_OR_REMOVE(PRF2_FLAGS(vict), PRF2_ARENA_RED);
        break;
    case 55:
        SET_OR_REMOVE(PRF2_FLAGS(vict), PRF2_ARENA_BLUE);
        break;
    case 56:
        GET_OLC_ZONE(vict) = value;
        break;
    case 57:
        GET_QUESTPOINTS(vict)+=value;
        break;
    case 58:
        vict->player_specials->saved.pkscore=value;
        break;
    case 59:
        vict->player_specials->saved.killed_player=value;
        break;
    case 60:
        if (!is_file)
            return;
        if (GET_IDNUM(ch) > 1) {
            send_to_char("Please don't use this command, yet.\r\n", ch);
            return;
        }
        if (GET_LEVEL(vict) >= LVL_GRGOD) {
            send_to_char("You cannot change that.\r\n", ch);
            return;
        }
        strcpy(tmp_store.name, CAP(val_arg));
        tmp_store.name[20] = '\0';
        strcpy(GET_NAME(vict), CAP(val_arg));
        *(GET_NAME(vict) + 9) = '\0';

        sprintf(buf, "Name changed to '%s'.", CAP(val_arg));
        break;

    case 61:
        switch (value) {
        case 0:GET_CLAN(vict)=CLAN_NEWBIE;break;
        case 1:GET_CLAN(vict)=CLAN_BLUE;break;
        case 2:GET_CLAN(vict)=CLAN_RED;break;
        case 3:GET_CLAN(vict)=CLAN_OUTLAW;break;
        }
        break;
    case 62:
        {
            struct char_data *vic;
            if (!(vic = get_char_vis(ch, val_arg)))
                send_to_char ("Noone by that name here.\r\n", ch);
            else
            {
                GET_PRIVATE(vict)=GET_PRIVATE(vic);
                if (GET_PRIVATE(vict))
                    sprintf(buf, "%s is now on %s's private channel (%d).\r\n", GET_NAME(vict), GET_NAME(vic), GET_PRIVATE(vict));
                else
                    send_to_char("They are not on a private channel.\r\n", ch);
            }
            break;
        }
    case 63:
        i=find_deity_by_name(val_arg);
        if (i==-1)
        {
        	send_to_char("That is not a valid deity.\r\n", ch);
        	return;
        }
        else
        	SET_DEITY(vict,1);
        break;
    case 64:
        SET_FAITH(vict, value);
        break;        
    case 65:
    	GET_WEIGHT(vict)= RANGE(0, 1000);
    	break;
    case 66:
    	GET_HEIGHT(vict)= RANGE(0, 1000);
    	break;
    default:
        sprintf(buf, "Can't set that!");
        break;
    }

    if (fields[l].type == BINARY) {
        sprintf(buf, "%s %s for %s.\r\n", fields[l].cmd, ONOFF(on),
                GET_NAME(vict));
        CAP(buf);
    } else if (fields[l].type == NUMBER) {
        sprintf(buf, "%s's %s set to %d.\r\n", GET_NAME(vict),
                fields[l].cmd, value);
    } else
        strcat(buf, "\r\n");
    send_to_char(CAP(buf), ch);

    if (!is_file && !IS_NPC(vict))
        save_char(vict, vict->in_room);

    if (is_file) {
        char_to_store(vict, &tmp_store);
        fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
        fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
        fflush(player_fl);
        free_char(cbuf);
        send_to_char("Saved in file.\r\n", ch);
    }
}


/*static char *logtypes[] = {
"off", "brief", "normal", "complete", "\n"};

ACMD(do_syslog)
{
  int tp;

  one_argument(argument, arg);

  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
	  (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
  SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}
*/
#define OBJ_NEW(ch) ((ch)->player.obj_buffer)
#define MOB_NEW(ch) ((ch)->player.mob_buf)
#define ZONE_BUF(ch) ((ch)->player.zone_edit)
#define GET_ROOM_SECTOR(room) (world[(room)].sector_type)

ACMD(do_rstat)
{
    /* struct room_data *rm; */
    int number, r_num, room_temp;

    one_argument(argument, buf);

    if (*buf) {
        if (!isdigit(*buf)) {
            send_to_char("Usage: rstat [vnum]\r\n", ch);
            return;
        }
        if ((number = atoi(buf)) < 0) {
            send_to_char("A NEGATIVE number??\r\n", ch);
            return;
        }
        if ((r_num = real_room(number)) < 0) {
            send_to_char("There is no room with that number.\r\n", ch);
            return;
        }
        room_temp = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, r_num);
        do_stat_room(ch);
        char_from_room(ch);
        char_to_room(ch, room_temp);
    } else {
        do_stat_room(ch);
    }
}



ACMD(do_zstat)
{
    int i, zn = -1, pos = 0, none = TRUE;
    FILE *zfile;
    char line[256];
    char arg3[40], arg4[80], arg5[40], arg6[40];

    one_argument(argument, buf);

    if (*buf) {
        if (isdigit(*buf))
            zn = atoi(buf);
        else {
            send_to_char("Usage: zstat [zone number]\r\n", ch);
            return;
        }
    } else
        zn = world[ch->in_room].number / 100;

    if (zn < 0)
        return;

    for (i = 0; i <= top_of_zone_table && zone_table[i].number != zn; i++);
    if (i > top_of_zone_table) {
        send_to_char("That zone doesn't exist.\r\n", ch);
        return;
    }
    if ((zone_table[i].reset_mode < 0) || (zone_table[i].reset_mode > 2))
        zone_table[i].reset_mode = 2;

    sprintf(buf, "%sZone #      : %s%3d       %sName      : %s%s\r\n",
            CCMAG(ch, C_SPR),
            CCWHT(ch, C_SPR), zone_table[i].number, CCMAG(ch, C_SPR),
            CCWHT(ch, C_SPR), zone_table[i].name);
    sprintf(buf, "%s%sMin/Max LVL : %s%3d%s/%s%3d   %sCreator   : %s%s\r\n",
            buf, CCMAG(ch, C_SPR), CCWHT(ch, C_SPR), zone_table[i].lvl_low,
            CCMAG(ch, C_SPR), CCWHT(ch, C_SPR), zone_table[i].lvl_high,
            CCMAG(ch, C_SPR), CCWHT(ch, C_SPR), zone_table[i].creator);
    sprintf(buf, "%s%sBuilders    : %s", buf, CCMAG(ch, C_SPR), CCWHT(ch, C_SPR));

    sprintf(buf1, "%s/%i.zon", ZON_PREFIX, zn);
    if (!(zfile = fopen(buf1, "r"))) {
        send_to_char(CCNRM(ch, C_SPR), ch);
        send_to_char("\r\nError:  unable to open zone file\r\n", ch);
        return;
    }
    do {
        get_line2(zfile, line);
        if (*line == '*') {
            half_chop(line, arg3, arg4);
            half_chop(arg4, arg5, arg6);
            if (is_abbrev(arg5, "Builder")) {
                none = FALSE;
                switch (pos) {
                case 0:
                    sprintf(buf, "%s%s", buf, arg6);
                    pos = 2;
                    break;
                case 1:
                    sprintf(buf, "%s\r\n              %s", buf, arg6);
                    pos = 2;
                    break;
                case 2:
                    sprintf(buf, "%s, %s", buf, arg6);
                    pos = 3;
                    break;
                case 3:
                    sprintf(buf, "%s, %s", buf, arg6);
                    pos = 1;
                    break;
                }
            }
        }
    } while ((*line != '$') && !feof(zfile));
    fclose(zfile);

    if (none)
        sprintf(buf, "%sNONE%s\r\n", buf, CCNRM(ch, C_SPR));
    else
        sprintf(buf, "%s%s\r\n", buf, CCNRM(ch, C_SPR));
    send_to_char(buf, ch);
}

/*
int remove(char * filename);
int rename(char * filename, char * filename2);
LINUX!!!
*/
ACMD(do_zallow)
{
    int zone;
    FILE *old_file, *new_file;
    char *old_fname, *new_fname, line[256];
    char arg1[40], arg2[40];

    half_chop(argument, arg1, arg2);

    if (!*arg1 || !*arg2 || !isdigit(*arg2)) {
        send_to_char("Usage  : zallow <player> <zone>\r\n", ch);
        return;
    }
    *arg1 = UPPER(*arg1);
    zone = atoi(arg2);

    sprintf(buf1, "%s/%i.zon", ZON_PREFIX, zone);
    old_fname = buf1;
    sprintf(buf2, "%s/%i.zon.temp", ZON_PREFIX, zone);
    new_fname = buf2;

    if (!(old_file = fopen(old_fname, "r"))) {
        sprintf(buf, "Error: Could not open %s for read.\r\n", old_fname);
        send_to_char(buf, ch);
        return;
    }
    if (!(new_file = fopen(new_fname, "w"))) {
        sprintf(buf, "Error: Could not open %s for write.\r\n", new_fname);
        send_to_char(buf, ch);
        fclose(old_file);
        return;
    }
    get_line2(old_file, line);
    fprintf(new_file, "%s\n", line);
    get_line2(old_file, line);
    fprintf(new_file, "%s\n", line);
    get_line2(old_file, line);
    fprintf(new_file, "%s\n", line);
    get_line2(old_file, line);
    fprintf(new_file, "%s\n", line);
    get_line2(old_file, line);
    fprintf(new_file, "* Builder  %s\n", arg1);
    fprintf(new_file, "%s\n", line);
    do {
        get_line2(old_file, line);
        fprintf(new_file, "%s\n", line);
    } while (*line != 'S');
    fclose(old_file);
    fclose(new_file);
    remove(old_fname);
    rename(new_fname, old_fname);
    send_to_char("Zone file edited.\r\n", ch);
    sprintf(buf, "olc: %s gives %s zone #%i builder access.", GET_NAME(ch),
            arg1, zone);
    mudlog(buf, BRF, LVL_IMPL, TRUE);
}


ACMD(do_zdeny)
{
    int zone;
    FILE *old_file, *new_file;
    char *old_fname, *new_fname, line[256];
    char arg1[40], arg2[40], arg3[40], arg4[40], arg5[40], arg6[40];

    half_chop(argument, arg1, arg2);

    if (!*arg1 || !*arg2 || !isdigit(*arg2)) {
        send_to_char("Usage  : zdeny <player> <zone>\r\n", ch);
        return;
    }
    *arg1 = UPPER(*arg1);
    zone = atoi(arg2);

    sprintf(buf1, "%s/%i.zon", ZON_PREFIX, zone);
    old_fname = buf1;
    sprintf(buf2, "%s/%i.zon.temp", ZON_PREFIX, zone);
    new_fname = buf2;

    if (!(old_file = fopen(old_fname, "r"))) {
        sprintf(buf, "Error: Could not open %s for read.\r\n", old_fname);
        send_to_char(buf, ch);
        return;
    }
    if (!(new_file = fopen(new_fname, "w"))) {
        sprintf(buf, "Error: Could not open %s for write.\r\n", new_fname);
        send_to_char(buf, ch);
        fclose(old_file);
        return;
    }
    do {
        get_line2(old_file, line);
        if (strstr(line, "* Builder")) {
            half_chop(line, arg3, arg4);
            half_chop(arg4, arg5, arg6);
            if (!isname(arg1, arg6))
                fprintf(new_file, "%s\n", line);
        } else {
            fprintf(new_file, "%s\n", line);
        }
    } while (*line != 'S');
    fclose(old_file);
    fclose(new_file);
    remove(old_fname);
    rename(new_fname, old_fname);
    send_to_char("Zone file edited.\r\n", ch);
    sprintf(buf, "olc: %s removes %s's zone #%i builder access.", GET_NAME(ch),
            arg1, zone);
    mudlog(buf, BRF, LVL_IMPL, TRUE);
}

int showplay(struct char_data * ch)
{
    FILE *fl;
    struct char_file_u player;
    char classname[100];
    char mybuf[200];
    char sexname;
    int num = 0;

    if (!(fl = fopen(PLAYER_FILE, "r+b"))) {
        perror("error opening playerfile");
    }
    for (;;) {
        fread(&player, sizeof(struct char_file_u), 1, fl);
        if (feof(fl)) {
            fclose(fl);
            break;
        }
        switch (player.class) {
        case CLASS_THIEF:
            strcpy(classname, "Th");
            break;
        case CLASS_WARRIOR:
            strcpy(classname, "Wa");
            break;
        case CLASS_MAGIC_USER:
            strcpy(classname, "Mu");
            break;
        case CLASS_CLERIC:
            strcpy(classname, "Cl");
            break;
        case CLASS_DRUID:
            strcpy(classname, "Dr");
            break;
        case CLASS_MONK:
            strcpy(classname, "Mo");
            break;
        default:
            strcpy(classname, "--");
            break;
        }

        switch (player.sex) {
        case SEX_FEMALE:
            sexname = 'F';
            break;
        case SEX_MALE:
            sexname = 'M';
            break;
        case SEX_NEUTRAL:
            sexname = 'N';
            break;
        default:
            sexname = '-';
            break;
        }

        sprintf(mybuf, "%4d. ID: %4ld (%c) [%2d %s] %-10s %30s %9dg\r\n", ++num,
                player.char_specials_saved.idnum, sexname, player.level,
                classname, player.name, player.player_specials_saved.email, player.points.gold+player.points.bank_gold);
        send_to_char(mybuf, ch);
    }
    return 0;
}

ACMD(do_hall)
{
    FILE *fl;
    struct char_file_u player;
    char classname[100];
    char mybuf[200];
    char sexname;
    int score;
    int num = 0;
    maxniz=0;
    Crash_save_all();
    if (!(fl = fopen(PLAYER_FILE, "r+b"))) {
        perror("error opening playerfile");
    }
    while (!feof(fl)) {
        fread(&player, sizeof(struct char_file_u), 1, fl);
        if (player.level<LVL_IMMORT && !IS_SET(player.char_specials_saved.act, PLR_DELETED)) {
            score=(player.player_specials_saved.mkscore+8*player.player_specials_saved.pkscore)/(100-player.level); // player.player_specials_saved.questpoints;
            sprintf(niz[maxniz].ime, "%-12s %-45s %9d\r\n", player.name, player.title, score);
            niz[maxniz].score= score;
            maxniz++;
        }
    }
    fclose(fl);

    if (maxniz)
    {
        sortniz();
        maxniz=MIN(15, maxniz);
        send_to_char("\r\n&Y                           H A L L   O F   F A M E\r\n                           =======================&0\r\n\r\n",ch);
        for (num=0;num<maxniz;num++)
        {
            sprintf(mybuf,"&w%4d&0. &c%s&0",num+1, niz[num].ime);
            send_to_char(mybuf, ch);
        }
        sprintf(mybuf, "\r\nYour Adventure Points: &Y%d&0\r\n\r\n", (ch->player_specials->saved.mkscore+ch->player_specials->saved.pkscore*8)/(100-GET_LEVEL(ch)));
        send_to_char(mybuf,ch);
    }
    else 
    	send_to_char("No info available for the Hall of Fame list.\r\n", ch);
}
extern CHAR_DATA *supermob; 

ACMD(do_top)
{
    FILE *fl;
    struct char_file_u player;
    struct char_data *victim;
    char classname[100];
    char mybuf[500];
    char sexname;
    int num = 0;
    int klasa;
    maxniz=0;
    Crash_save_all();
    if (!(fl = fopen(PLAYER_FILE, "r+b"))) {
        perror("error opening playerfile");
    }

    klasa=GET_CLASS(supermob);
    
    while (!feof(fl)) {

        fread(&player, sizeof(struct char_file_u), 1, fl);
        if (player.level<LVL_IMMORT && !IS_SET(player.char_specials_saved.act, PLR_DELETED)) {
            GET_CLASS(supermob)=player.class;
            sprintf(niz[maxniz].ime, "%-15s  %10s %-15s (%2d)   %3d/%3d/ %d\r\n",
                    player.name, pc_race_types[player.race],pc_class_types[GET_CLASS_NUM(supermob)],  player.level, player.player_specials_saved.killed_by_mob+
                    player.player_specials_saved.killed_by_player, player.player_specials_saved.killed_player, player.player_specials_saved.killed_mob);
            GET_CLASS(supermob)=klasa;
            niz[maxniz].score= player.level;
            maxniz++;}
    }
    
    fclose(fl);

    // maxniz=MIN(15, maxniz);
    if (maxniz)
    {
        sortniz();
        maxniz=MIN(15, maxniz);
        send_to_char("\r\n&Y                             T O P   P L A Y E R S \r\n                             =====================&0\r\n\r\n",ch);

        for (num=0;num<maxniz;num++)
        {
            sprintf(mybuf,"&w%4d&0. &c%s&0",num+1, niz[num].ime);
            send_to_char(mybuf, ch);
        }
    }
    else
    send_to_char("No info available for the Top list.\r\n", ch);
}


ACMD(do_mobtop)
{
    int i, num;
    char mybuf[500];
    maxniz=0;

    for (i=0;i<MAX_MOBKILL;i++)
    {
        if (mobkills[i].killed && mobkills[i].rnum!=65000 && i>100)
        {
            sprintf(buf, (mob_proto[mobkills[i].rnum]).player.short_descr);
            sprintf(niz[maxniz].ime, "%-45s     %5d\r\n",CAP(buf), mobkills[i].killed);
            niz[maxniz].score= mobkills[i].killed;
            maxniz++;
        }
    }
    if (maxniz)
    {
        sortniz();
        maxniz=MIN(15, maxniz);
        send_to_char("\r\n&Y                       T O P   M O B S   K I L L E D \r\n                       =============================&0\r\n\r\n",ch);

        for (num=0;num<maxniz;num++)
        {
            sprintf(mybuf,"&w%4d&0. &c%s&0",num+1, niz[num].ime);
            send_to_char(mybuf, ch);
        }
    }
}



ACMD(do_mobpk)
{
    int i, num;
    char mybuf[500];
    maxniz=0;

    for (i=0;i<MAX_MOBKILL;i++)
    {
        if (mobkills[i].pkills && mobkills[i].rnum!=65000 && i>100)
        {
            sprintf(buf, (mob_proto[mobkills[i].rnum]).player.short_descr);
            sprintf(niz[maxniz].ime, "%-45s     %5d\r\n",CAP(buf), mobkills[i].pkills);
            niz[maxniz].score= mobkills[i].pkills;
            maxniz++;
        }
    }

    if (maxniz)
    {
        sortniz();
        maxniz=MIN(15, maxniz);
        send_to_char("\r\n&Y                               M O B S   P K I L L E R S \r\n                               =========================&0\r\n\r\n",ch);

        for (num=0;num<maxniz;num++)
        {
            sprintf(mybuf,"&w%4d&0. &c%s&0",num+1, niz[num].ime);
            send_to_char(mybuf, ch);
        }
    }
}


void generate_zonekill_ratio()
{
    int i, num, vnum, k, temp;
    char mybuf[500];
    maxniz=0;

    for (i = 0; i <= top_of_zone_table; i++)
        zone_table[i].calc_ratio=100;


    for (i=100;i<MAX_MOBKILL;i++)
    {
        if (mobkills[i].killed && mobkills[i].rnum!=65000)
        {
            sprintf(niz[maxniz].ime, "%d",i/100);
            niz[maxniz].score= mobkills[i].killed;
            maxniz++;
        }
    }
    if (maxniz)
    {
        sortniz();
        temp=MIN(15, maxniz);

        for (num=0;num<temp;num++)
        {
            vnum=atoi(niz[num].ime);
            vnum*=100;

            if (niz[num].score)
                for (i = 0; i <= top_of_zone_table; i++)
                    if ((zone_table[i].number * 100 <= vnum) && (zone_table[i].top >= vnum))
                    {
                        k=10/(num+1);
                        zone_table[i].calc_ratio-=(15+k-num)/2;
                        //	logs("Zone %d (%s) added %d (num=%d)", vnum, zone_table[i].name, 20-num, num);
                        break;
                    }

        }

        temp=MAX(0, maxniz-15);
        for (num=maxniz-1;num>=temp;num--)
        {
            vnum=atoi(niz[num].ime);
            vnum*=100;

            if (niz[num].score)
                for (i = 0; i <= top_of_zone_table; i++)
                    if ((zone_table[i].number * 100 <= vnum) && (zone_table[i].top >= vnum))
                    {
                        k=10/(maxniz-num);
                        zone_table[i].calc_ratio+=(15+k-(maxniz-num)+1);
                        //	logs("Zone %d (%s) added %d (num=%d)", vnum, zone_table[i].name, 20-num, num);
                        break;
                    }

        }
    }

    /*maxniz=0;

    for (i=100;i<MAX_MOBKILL;i++)
        {   			
    	if (mobkills[i].pkills && mobkills[i].rnum!=65000)
    	{	
    	sprintf(niz[maxniz].ime, "%d",i/100);
    	niz[maxniz].score= mobkills[i].pkills;
    	maxniz++;
    	}
        }    
        if (maxniz)
        {
        sortniz();             
        maxniz=MIN(15, maxniz);
           
        for (num=0;num<maxniz;num++)
        {   
        	vnum=atoi(niz[num].ime);
        	vnum*=100;
        	
           if (niz[num].score)
            for (i = 0; i <= top_of_zone_table; i++)
             if ((zone_table[i].number * 100 <= vnum) && (zone_table[i].top >= vnum))
             	{       
             		k=10/(num+1);
             		zone_table[i].calc_ratio+=15+k-num;                		
             	//	logs("Zone %d (%s) added %d (num=%d)", vnum, zone_table[i].name, 20-num, num);
             		break;
             	}
       
        }
        }*/

}




extern struct player_index_element *player_table;
extern int top_of_p_table;

ACMD(do_players)
{ char buf[1000];
    int i, count = 0;

    for (i = 0; i <= top_of_p_table + 1; i++) {
        sprintf(buf, "%s  %-20.20s", buf, (player_table + i)->name);
        count++;
        if (count == 3) {
            count = 0;
            send_to_char(buf, ch);
            send_to_char("\r\n", ch);
            sprintf(buf, "");
        }
    }
}

void do_newbie(struct char_data * vict)
{
    struct obj_data *obj;
    int num=0;
    int give_obj[] = {3043, 3081, 3086, 3015, 3015, 94, -1};

    int i;
    for (i = 0; give_obj[i] != -1; i++) {
        obj = read_object(give_obj[i], VIRTUAL, 0, 1);
        obj_to_char(obj, vict);
    }

    if (IS_MAGIC_USER(vict) || IS_THIEF(vict) || IS_BARD(vict) || IS_NECRON(vict))
        num=3701;
    
    else if (IS_WARRIOR(vict) || IS_DEATHK(vict) || IS_RANGER(vict))
        num=3702;
    else //if (IS_CLERIC(vict) || IS_DRUID(vict))
        num=3700;
    if (num)
    {
        obj = read_object(num, VIRTUAL, 0, 1);
        GET_OBJ_VAL(obj, 0)=1;
        GET_OBJ_VAL(obj, 1)=1;
        GET_OBJ_VAL(obj, 2)=3;
        obj_to_char(obj, vict);
    }
}

ACMD(do_wish)
{
    struct obj_data *obj;
    int give_obj[] = {3043, 3081, 3086, -1};
    /* replace the 4 digit numbers on the line above with your basic eq -1 HAS
     * to  be the end field
     */
    int i;
    return;
    act("Creature appears in the room and gives something to $n.\r\n", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("The Gods smile upon you.\r\nYou have been given some items!\r\n", ch);

    for (i = 0; give_obj[i] != -1; i++) {
        obj = read_object(give_obj[i], VIRTUAL, 0, -1);
        obj_to_char(obj, ch);
    }
}

ACMD(do_rlist)
{
    extern struct room_data *world;
    extern int top_of_world;

    int first, last, nr, found = 0;

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2) {
        send_to_char("Usage: rlist <begining number> <ending number>\r\n", ch);
        return;
    }
    first = atoi(buf);
    last = atoi(buf2);

    if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
        send_to_char("Values must be between 0 and 99999.\r\n", ch);
        return;
    }
    if (first >= last) {
        send_to_char("Second value must be greater than first.\r\n", ch);
        return;
    }
    for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++) {
        if (world[nr].number >= first) {
            sprintf(buf, "%5d. [%5d] (%3d) %s\r\n", ++found,
                    world[nr].number, world[nr].zone,
                    world[nr].name);
            send_to_char(buf, ch);
        }
    }

    if (!found)
        send_to_char("No rooms were found in those parameters.\r\n", ch);
}


ACMD(do_mlist)
{
    extern struct index_data *mob_index;
    extern struct char_data *mob_proto;
    extern int top_of_mobt;

    int first, last, nr, found = 0;
    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2) {
        send_to_char("Usage: mlist <begining number> <ending number>\r\n", ch);
        return;
    }
    first = atoi(buf);
    last = atoi(buf2);

    if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
        send_to_char("Values must be between 0 and 99999.\r\n", ch);
        return;
    }
    if (first >= last) {
        send_to_char("Second value must be greater than first.\r\n", ch);
        return;
    }
    for (nr = 0; nr <= top_of_mobt && (mob_index[nr].virtual <= last); nr++) {
        if (mob_index[nr].virtual >= first) {
            sprintf(buf, "%5d. [%5d] %s\r\n", ++found,
                    mob_index[nr].virtual,
                    mob_proto[nr].player.short_descr);
            send_to_char(buf, ch);
        }
    }

    if (!found)
        send_to_char("No mobiles were found in those parameters.\r\n", ch);
}


ACMD(do_olist)
{
    extern struct index_data *obj_index;
    extern struct obj_data *obj_proto;
    extern int top_of_objt;

    int first, last, nr, found = 0;

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2) {
        send_to_char("Usage: olist <begining number> <ending number>\r\n", ch);
        return;
    }
    first = atoi(buf);
    last = atoi(buf2);

    if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
        send_to_char("Values must be between 0 and 99999.\r\n", ch);
        return;
    }
    if (first >= last) {
        send_to_char("Second value must be greater than first.\r\n", ch);
        return;
    }
    for (nr = 0; nr <= top_of_objt && (obj_index[nr].virtual <= last); nr++) {
        if (obj_index[nr].virtual >= first) {
            sprintf(buf, "%5d. [%5d] %s\r\n", ++found,
                    obj_index[nr].virtual,
                    obj_proto[nr].short_description);
            send_to_char(buf, ch);
        }
    }

    if (!found)
        send_to_char("No objects were found in those parameters.\r\n", ch);
}

ACMD(do_zap)
{
    struct char_data *victim;
    int zap_points;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Who do you want to zap?\r\n", ch);
        return;
    }
    if (!(victim = get_char_vis(ch, arg))) {
        send_to_char("No,no..", ch);
        return;
    }
    if (IS_NPC(victim)) {
        send_to_char("You can't zap a mob!\r\n", ch);
        return;
    }
    if (GET_LEVEL(victim) >= GET_LEVEL(ch)) {
        send_to_char("Probably not a good idea.\r\n", ch);
        return;
    }
    switch (GET_LEVEL(ch)) {
    case LVL_IMMORT:
        zap_points = GET_HIT(victim) / 10;
        break;
    case LVL_GOD:
        zap_points = GET_HIT(victim) / 3;
        break;
    case LVL_GRGOD:
    case LVL_IMPL:
        zap_points = GET_HIT(victim) / 2;
        break;
    default:
        zap_points = 0;
        break;
    }
    send_to_all(
        "The clouds part and a powerful bolt of lightning arcs from the sky\r\nfries the ground");
    sprintf(buf, ". %s is struck down by the unholy wrath of %s!\r\n",
            GET_NAME(victim), GET_NAME(ch));
    send_to_all(buf);
    sprintf(buf, "(GC) %s ZAPPED %s for %i damage.", GET_NAME(ch),
            GET_NAME(victim), zap_points);
    mudlog(buf, NRM, LVL_IMMORT, FALSE);
    GET_HIT(victim) -= zap_points;
}

ACMD(do_test)
{
    time_t tloc, tloc1;
    struct char_data *vict;
    int i, j;
    float levels[100], max=0;
    char testb[1000];
    tloc = time(NULL);
    return;
    for (i=0; i<100;i++)
        levels[i]=0;
    j=0;
    for (vict = character_list; vict; vict = vict->next) {
        levels[GET_LEVEL(vict)]++;
        j++;
    }
    for (i=0; i<100; i++)
        if (levels[i]>max)
            max=levels[i];
    for (i=0; i<70;i++)
        if (levels[i])
        {
            sprintf(buf, "%2d) %s\r\n",i, make_bar(testb, (int) levels[i]*100/max, 75));
            send_to_char(buf, ch);
            if (!(i%10))
                process_output(ch->desc);
        }
    return;


    for (i = 1; i < 1000; i++) {
        if (i % 10)
            zone_update();
        if (i % 8)
            mobile_activity();
        if (i % 2)
            perform_violence();
        if (i % 25)
            point_update();
        if (i % 75) {
            weather_and_time();
            affect_update();
        }
        if (i % 65) {
            start_arena();
            do_game();
        }
    }
    tloc1 = time(NULL);
    sprintf(buf, "TEST Time: %d\r\n", (tloc1) - (tloc));
    log(buf);
}



ACMD(do_copyto)
{

    /* Only works if you have Oasis OLC */
    extern void olc_add_to_save_list(int zone, byte type);

    char buf2[10];
    char buf[80];
    int iroom = 0, rroom = 0;
    struct room_data *room;

    one_argument(argument, buf2);
    /* buf2 is room to copy to */

    CREATE(room, struct room_data, 1);

    iroom = atoi(buf2);
    rroom = real_room(atoi(buf2));
    *room = world[rroom];

    if (!*buf2) {
        send_to_char("This command copies this room's description to target room.\r\nFormat: copyto <room number>\r\n", ch);
        return;
    }
    if (rroom <= 0) {
        sprintf(buf, "There is no room with the number %d.\r\n", iroom);
        send_to_char(buf, ch);
        return;
    }
    /* Main stuff */

    if (world[ch->in_room].description) {
        world[rroom].description = str_dup(world[ch->in_room].description);

        /* Only works if you have Oasis OLC */
        olc_add_to_save_list((iroom / 100), OLC_SAVE_ROOM);

        sprintf(buf, "You copy the description to room %d.\r\n", iroom);
        send_to_char(buf, ch);
    } else
        send_to_char("This room has no description!\r\n", ch);
}

ACMD(do_connect)
{
    /* Only works if you have Oasis OLC */
    extern void olc_add_to_save_list(int zone, byte type);

    char buf2[10];
    char buf3[10];
    char buf[80];
    int iroom = 0, rroom = 0;
    int dir = 0;
    /*  struct room_data *room; */

    two_arguments(argument, buf2, buf3);
    /* buf2 is the direction, buf3 is the room */

    iroom = atoi(buf3);
    rroom = real_room(iroom);

    if (!*buf2) {
        send_to_char("Format: dig <dir> <room number>\r\n", ch);
        return;
    } else if (!*buf3) {
        send_to_char("Format: dig <dir> <room number>\r\n", ch);
        return;
    }
    if (rroom <= 0) {
        sprintf(buf, "There is no room with the number %d", iroom);
        send_to_char(buf, ch);
        return;
    }
    /* Main stuff */
    switch (*buf2) {
    case 'n':
    case 'N':
        dir = NORTH;
        break;
    case 'e':
    case 'E':
        dir = EAST;
        break;
    case 's':
    case 'S':
        dir = SOUTH;
        break;
    case 'w':
    case 'W':
        dir = WEST;
        break;
    case 'u':
    case 'U':
        dir = UP;
        break;
    case 'd':
    case 'D':
        dir = DOWN;
        break;
    }

    CREATE(world[rroom].dir_option[rev_dir[dir]], struct room_direction_data, 1);
    world[rroom].dir_option[rev_dir[dir]]->general_description = NULL;
    world[rroom].dir_option[rev_dir[dir]]->keyword = NULL;
    world[rroom].dir_option[rev_dir[dir]]->to_room = ch->in_room;

    CREATE(world[ch->in_room].dir_option[dir], struct room_direction_data, 1);
    world[ch->in_room].dir_option[dir]->general_description = NULL;
    world[ch->in_room].dir_option[dir]->keyword = NULL;
    world[ch->in_room].dir_option[dir]->to_room = rroom;

    /* Only works if you have Oasis OLC */
    olc_add_to_save_list((iroom / 100), OLC_SAVE_ROOM);

    sprintf(buf, "You make an exit %s to room %d.\r\n", buf2, iroom);
    send_to_char(buf, ch);
}



static char *logtypes[] = {
                              "off", "brief", "normal", "complete", "\n"};

ACMD(do_syslog)
{
    int tp;

    one_argument(argument, arg);

    if (!*arg) {
        tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
              (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
        sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
        send_to_char(buf, ch);
        return;
    }
    if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
        send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
        return;
    }
    REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
    SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

    sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
}


ACMD(do_affects)
{
    char buf[256], buf2[256];
    struct affected_type *aff;
    bool showed = FALSE;
    int notr = 0, i;
    struct obj_data *obj;
    if (IS_IMMORT(ch)) {
        send_to_char("Affected by: ", ch);
        if (AFF_FLAGS(ch)) {
            sprintbit(AFF_FLAGS(ch), affected_bits, buf);
            send_to_char(buf, ch);
            showed = TRUE;
        }
        if (AFF2_FLAGS(ch)) {
            send_to_char("\r\n             ", ch);
            sprintbit(AFF2_FLAGS(ch), affected_bits2, buf);
            send_to_char(buf, ch);
            showed = TRUE;
        }
        if (AFF3_FLAGS(ch)) {
            send_to_char("\r\n             ", ch);
            sprintbit(AFF3_FLAGS(ch), affected_bits3, buf);
            send_to_char(buf, ch);
            showed = TRUE;
        }
        if (showed)
            send_to_char("\r\n", ch);
        else
            send_to_char("Nothing\r\n", ch);
    }
    *buf2 = '\0';
    notr = 0;


    if (IN_ARENA(ch))
    {
        send_to_char("&GTemporary affects:&g\r\n--------&0\r\n", ch);
        notr=1;

        if (BLUE(ch))
            send_to_char(" (&BARENA BLUE TEAM MEMBER&0)\r\n", ch);
        if (RED(ch))
            send_to_char(" (&RARENA RED TEAM MEMBER&0)\r\n", ch);
        if (HAS_RED(ch))
            send_to_char(" (You carry the &RRED FLAG!&0)\r\n", ch);
        if (HAS_BLUE(ch))
            send_to_char(" (You carry the &BBLUE FLAG!&0)\r\n", ch);
    }
    if (ch->affected) {
        maxniz=0;
        for (aff = ch->affected; aff; aff = aff->next) {

            if (aff->duration>-1) {
                if (!notr)
                {
                    send_to_char("&GTemporary affects:&g\r\n------------------&0\r\n", ch);
                    notr=1;
                }
                sprintf(buf2, "(%3dhr)", aff->duration + 1);

                sprintf(buf, " %s %s%-25s%s ", buf2, CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
                if (aff->modifier) {
                    sprintf(buf2, "%+d to %s\r\n", aff->modifier, apply_types[(int) aff->location]);
                } else
                    sprintf(buf2, "\r\n");
                strcat(buf, buf2);
                strcpy(niz[maxniz].ime,buf);
                niz[maxniz].score=aff->duration+1;
                maxniz++;
            }
        }
        sortniz();
        for (i=maxniz-1;i>=0;i--)
            send_to_char(niz[i].ime,ch);
    }



    if (ch->affected) {
        for (aff = ch->affected; aff; aff = aff->next) {

            if (aff->duration==-2) {
                if (notr!=2)
                {
                    send_to_char("\r\n&GAffects:&g\r\n--------&0\r\n", ch);
                    notr=2;
                }
                sprintf(buf2, "(leech)");

                sprintf(buf, " %s %s%-25s%s ", buf2, CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
                if (aff->modifier) {
                    sprintf(buf2, "%+d to %s\r\n", aff->modifier, apply_types[(int) aff->location]);
                } else               
                    sprintf(buf2, "\r\n");

                strcat(buf, buf2);
                send_to_char(buf, ch);
            }
        }
    }

    if (ch->affected) {
        for (aff = ch->affected; aff; aff = aff->next) {

            if (aff->duration==-1) {
                if (notr!=2)
                {
                    send_to_char("\r\n&GOther:&g\r\n-------&0\r\n", ch);
                    notr=2;
                }
                sprintf(buf2, "(other)");

                sprintf(buf, " %s %s%-25s%s ", buf2, CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
                if (aff->modifier) {
                    sprintf(buf2, "%+d to %s\r\n", aff->modifier, apply_types[(int) aff->location]);
                } else
                    sprintf(buf2, "\r\n");
                strcat(buf, buf2);
                send_to_char(buf, ch);
            }
        }
    }

    for (i = 0; i < NUM_WEARS; i++)
        if ((obj=GET_EQ(ch, i))) {
            if (obj->obj_flags.bitvector)
            {
                if (notr!=3)
                {
                    send_to_char("\r\n&GItem abilities:&g\r\n---------------&0\r\n", ch);
                    notr=3;
                }

                send_to_char(" (const) ", ch);
                sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
                sprintf(buf2,"&C%-25s&0 given by %s\r\n", buf, obj->short_description);
                send_to_char(buf2, ch);
            }
            if (obj->obj_flags.bitvector2)
            {
                if (notr!=3)
                {
                    send_to_char("\r\n&GItem abilities:&g\r\n---------------&0\r\n", ch);
                    notr=3;
                }
                send_to_char(" (const) ", ch);
                sprintbit(obj->obj_flags.bitvector2, affected_bits2, buf);
                sprintf(buf2,"&C%-25s&0 given by %s\r\n", buf, obj->short_description);
                send_to_char(buf2, ch);
            }
            if (obj->obj_flags.bitvector3)
            {
                if (notr!=3)
                {
                    send_to_char("\r\n&GItem abilities:&g\r\n---------------&0\r\n", ch);
                    notr=3;
                }
                send_to_char(" (const) ", ch);
                sprintbit(obj->obj_flags.bitvector3, affected_bits3, buf);
                sprintf(buf2,"&C%-25s&0 given by %s\r\n", buf, obj->short_description);
                send_to_char(buf2, ch);
            }
        }

    if (!notr)
        send_to_char("\r\nYou do not feel any affects.\r\n", ch);

}




void list_room_zone  (struct char_data *ch, int whichzone)
{


    int i=0;
    int j=0;
    int first_z_room=0;             /*vnum of first obj in zone list*/
    int last_z_room=0;              /*vnum of last obj in zone list */
    int char_zone = ((GET_ROOM_VNUM(ch->in_room))/100);  /*initialize to current zone    */

    buf[0]='\0';
    if (whichzone > (-1)) char_zone = whichzone;   /*see if specific zone was passed as arg*/

    while (j!= char_zone)
    {
        first_z_room++;
        j=(world[i].number/100);
        i++;



        /*some zones have 100+ rooms but not 100+ items/objs.  To find the needed objs/objects,
         * search backwards for previous zone.*/
        if (j>char_zone)
        {
            if (char_zone>0)
            {
                char_zone--;
                first_z_room=0;
                i=0;
                j=0;
            }
            else
            {
                log("SYSERR:  rlist looked for a zone < 0");
                return;
            }
        }
    }

    first_z_room --;
    last_z_room=first_z_room;


    while (j == char_zone)
    {
        j=(world[i].number/100);
        last_z_room++;
        i++;
    }

    if (first_z_room<0) first_z_room ++;
    for (i=first_z_room;i<last_z_room;i++)
    {
        sprintf(buf+strlen(buf),"[%5d]  %s\r\n",
                world[i].number,
                world[i].name);


    }
    if (strlen(buf)<5) send_to_char ("There are no rooms in the zone you requested.\r\n",ch);
    else page_string (ch->desc,buf,1);
    return;
}


void do_wrlist (struct char_data *ch, char *input)
{

    int i=0;
    if (!(*input)) list_room_zone(ch,(-1));

    else
    {
        skip_spaces(&input);
        if (is_number(input))  list_room_zone(ch,atoi(input));
        else list_room_zone(ch,((GET_ROOM_VNUM(ch->in_room)) / 100));

    }
    return;
}


int get_weapon_dam (struct obj_data *o)
{

    int i = 0;

    for (i=0; i<MAX_OBJ_AFFECT; i++)
        if (o->affected[i].location == APPLY_DAMROLL)
        {
            return (o->affected[i].modifier);
        }
    return (0);
}


int get_armor_ac (struct obj_data *o)
{

    int i = 0;

    for (i=0;i<MAX_OBJ_AFFECT;i++)
        if (o->affected[i].location == APPLY_AC)
        {
            return (o->affected[i].modifier);
        }
    return (0);
}

extern struct obj_data *obj_proto;     /* prototypes for objs		 */
void list_object_zone  (struct char_data *ch, int whichzone)
{


    int i=0;
    int j=0;
    int first_z_obj=0;             /*vnum of first obj in zone list*/
    int last_z_obj=0;              /*vnum of last obj in zone list */
    int char_zone = ((GET_ROOM_VNUM(ch->in_room))/100);  /*initialize to current zone    */

    buf[0]='\0';
    if (whichzone > (-1)) char_zone = whichzone;   /*see if specific zone was passed as arg*/

    while (j!= char_zone)
    {
        first_z_obj++;
        j=(GET_OBJ_VNUM(&obj_proto[i])/100);
        i++;


        /*some zones have 100+ rooms but not 100+ items/objs.  To find the needed objs/objects,
         * search backwards for previous zones w/them.*/
        if (j>char_zone)
        {
            if (char_zone>0)
            {
                char_zone--;
                first_z_obj=0;
                i=0;
                j=0;
            }
            else
            {
                log("SYSERR:  olist looked for a zone < 0");
                return;
            }
        }
    }

    first_z_obj --;
    last_z_obj=first_z_obj;


    while (j == char_zone)
    {
        j=(GET_OBJ_VNUM(&obj_proto[i])/100);
        last_z_obj++;
        i++;
    }

    if (first_z_obj<0) first_z_obj ++;
    for (i=first_z_obj;i<last_z_obj;i++)
    {
        sprintf(buf+strlen(buf),"[%5d] %-32s",
                GET_OBJ_VNUM(&obj_proto[i]),
                obj_proto[i].short_description);

        j=obj_proto[i].obj_flags.type_flag;
        sprintf(buf+strlen(buf)," %s ", item_types[obj_proto[i].obj_flags.type_flag]);
        if ((j == ITEM_WORN) || (j== ITEM_ARMOR))
        {
            sprintf(buf+strlen(buf)," (Armor %d) ",GET_OBJ_VAL(&obj_proto[i],0));
            j=get_armor_ac (&obj_proto[i]);
            if (j!=0) sprintf(buf + strlen(buf), "(Magic armor %d)",j);
        }

        if (j==ITEM_WEAPON || j==ITEM_FIREWEAPON)
        {
            sprintf(buf+strlen(buf),"(Ave dam: %.1f) ",
                    (((GET_OBJ_VAL(&obj_proto[i], 2) + 1) / 2.0) * GET_OBJ_VAL(&obj_proto[i], 1)));
            j  = get_weapon_dam(&obj_proto[i]);
            if (j!=0) sprintf(buf+strlen(buf), " +%d DAM ",j);
        }
        sprintbit(GET_OBJ_WEAR(&obj_proto[i]), wear_bits, buf+strlen(buf));
        sprintf(buf+strlen(buf),"\r\n");
    }
    if (strlen(buf)<5) send_to_char ("There are no objects in the zone you requested.\r\n",ch);
    else page_string (ch->desc,buf,1);
    return;
}


void do_wolist (struct char_data *ch, char *input)
{

    int i=0;
    if (!(*input)) list_object_zone(ch,(-1));

    else
    {
        skip_spaces(&input);
        if (is_number(input))  list_object_zone(ch,atoi(input));
        else list_object_zone(ch,((GET_ROOM_VNUM(ch->in_room)) / 100));

    }
    return;
}




void list_mob_zone  (struct char_data *ch, int whichzone)
{


    int i=0;
    int j=0;
    int first_z_mob=0;             /*vnum of first mob in zone list*/
    int last_z_mob=0;              /*vnum of last mob in zone list */
    int char_zone = ((GET_ROOM_VNUM(ch->in_room))/100);  /*initialize to current zone    */

    buf[0]='\0';
    if (whichzone > (-1)) char_zone = whichzone;   /*see if specific zone was passed as arg*/

    while (j!= char_zone)
    {
        first_z_mob++;
        j=(GET_MOB_VNUM(&mob_proto[i])/100);
        i++;


        /*some zones have 100+ rooms but not 100+ items/mobs.  To find the needed mobs/objects,
         * search backwards for previous zonew/them.*/
        if (j>char_zone)
        {
            if (char_zone>0)
            {
                char_zone--;
                first_z_mob=0;
                i=0;
                j=0;
            }
            else
            {
                log("SYSERR:  mlist looked for a zone < 0");
                return;
            }
        }
    }

    first_z_mob --;
    last_z_mob=first_z_mob;


    while (j == char_zone)
    {
        j=(GET_MOB_VNUM(&mob_proto[i])/100);
        last_z_mob++;
        i++;
    }

    if (first_z_mob<0) first_z_mob ++;
    for (i=first_z_mob;i<last_z_mob;i++)
    {
        sprintf(buf+strlen(buf),"[%5d] %-30s  (level %d)\r\n",
                GET_MOB_VNUM(&mob_proto[i]),
                mob_proto[i].player.short_descr,
                mob_proto[i].player.level);
    }
    if (strlen(buf)<5) send_to_char ("There are no mobs in the zone you requested.\r\n",ch);
    else page_string (ch->desc,buf,1);
    return;
}


void do_wmlist (struct char_data *ch, char *input)
{


    if (!(*input)) list_mob_zone(ch,(-1));

    else
    {
        skip_spaces(&input);
        if (is_number(input))  list_mob_zone(ch,atoi(input));
        else list_mob_zone(ch,((GET_ROOM_VNUM(ch->in_room)) / 100));
    }
    return;
}

extern char *spells[];
void list_wands_staves (struct char_data *ch, char *input)
{

    int type=0;
    int i=0;
    int j=0;
    int k=0;

    skip_spaces(&input);
    switch (input[0])
    {
    case 'T':
    case 't': type = ITEM_STAFF; break;
    case 'W':
    case 'w': type = ITEM_WAND; break;
    default:
        {
            sprintf(buf,"SYSERR:  Default reached in list_scrolls_potions (arg = %s)", input);
            log(buf);
            return;
        }
    }  /*switch...*/

    buf[0]='\0';
    for (i=0;i<top_of_objt;i++)
    {
        j=obj_proto[i].obj_flags.type_flag;  /*look for specific sort of item*/
        if (j == type)  /*found one*/
        {
            sprintf(buf+strlen(buf),"[%5d] %-30s",   /*print vnum, short description*/
                    GET_OBJ_VNUM(&obj_proto[i]),
                    obj_proto[i].short_description);

            /*
                values 0-3:
                Potion, Scroll - up to three spells [values 1-3]
            */

            sprintf(buf+strlen(buf), " Spells: ");
            for (k=1;k<4;k++)
            {
                if ((GET_OBJ_VAL(&obj_proto[i], k)) != (-1))
                    sprintf(buf+strlen(buf), "%s ",
                            spells[GET_OBJ_VAL(&obj_proto[i], k)]);
            }
            sprintf(buf+strlen(buf), "\r\n");
        } /*if j == type*/
    }  /*for i...*/
    page_string (ch->desc, buf, 1);
}



void list_scrolls_potions (struct char_data *ch, char *input)
{

    int type=0;
    int i=0;
    int j=0;
    int k=0;

    skip_spaces(&input);
    switch (input[0])
    {
    case 'S':
    case 's': type = ITEM_SCROLL; break;
    case 'P':
    case 'p': type = ITEM_POTION; break;
    default :
        sprintf(buf,"SYSERR:  Default reached in list_scrolls_potions (arg = %s)", input);
        log (buf);
        return;
    }  /*switch...*/

    buf[0]='\0';
    for (i=0;i<top_of_objt;i++)
    {
        j=obj_proto[i].obj_flags.type_flag;  /*look for specific sort of item*/
        if (j == type)  /*found one*/
        {
            sprintf(buf+strlen(buf),"[%5d] %-20s",   /*print vnum, short description*/
                    GET_OBJ_VNUM(&obj_proto[i]),
                    obj_proto[i].short_description);

            /*
                values 0-3:
                Potion, Scroll - up to three spells [values 1-3]                    
            */

            sprintf(buf+strlen(buf), " Spells: ");
            for (k=1;k<4;k++)
            {
                if ((GET_OBJ_VAL(&obj_proto[i], k)) != (-1))
                    sprintf(buf+strlen(buf), "%s ",
                            spells[GET_OBJ_VAL(&obj_proto[i], k)]);
            }


            sprintf(buf+strlen(buf), "\r\n");

        } /*if j == type*/
    }  /*for i...*/
    page_string (ch->desc, buf, 1);
}



void do_list_wear (struct char_data *ch, char *input)
{

    char j = atoi(input);
    int i=0;


    if (input[0] == '?')
    {
        j=0;
        send_to_char ("Wear positions:\r\n", ch);
        for (i = 0; i < NUM_ITEM_WEARS; i++)
        {
            sprintf(buf, "(%2d) %-20.20s %s",
                    i + 1, wear_bits[i],
                    !(++j % 2) ? "\r\n" : "");
            send_to_char(buf, ch);
        }
        send_to_char("\r\nIf you choose TAKE, you will be shown item that are !Take\r\n",ch);
        return;
    }

    buf[0]='\0';


    j--; /*to be used with NAMES array*/

    if (j==0)  /*Show ony !Take items for this option*/
    {
        for (i=0;i<top_of_objt;i++)   /*cycle through every obj*/
            if (!(CAN_WEAR(&obj_proto[i], (1<<j))))/*check exact bit for requested position*/
            {
                sprintf(buf+strlen(buf),"[%5d] %-32s !TAKE\r\n",
                        GET_OBJ_VNUM(&obj_proto[i]),
                        obj_proto[i].short_description);
            }
        page_string (ch->desc, buf, 1);
        return;
    }

    for (i=0;i<top_of_objt;i++)   /*cycle through every obj*/
    {
        if (CAN_WEAR(&obj_proto[i], (1<<j)))  /*check exact bit for requested position*/
        {
            sprintf(buf+strlen(buf),"[%5d] %-32s ",
                    GET_OBJ_VNUM(&obj_proto[i]),
                    obj_proto[i].short_description);
            sprintf(buf+strlen(buf),"%s\r\n", wear_bits[j]);  /*repeat position*/
        }
    }
    if (!*buf) (send_to_char("There are no items of that type in the object files.c",ch));
    page_string (ch->desc, buf, 1);
}


ACMD (do_wlist)
{
    char *message="Usage: wlist < o | m | s | t | w | p | b | r | # (?)> <zone>\r\nType HELP WLIST for details\r\n";


    two_arguments(argument, buf, buf2);

    if (!*buf)
    {
        send_to_char(message,ch);
        return;
    }




    switch (buf[0])
    {
    case 'O':
    case 'o': do_wolist(ch, buf2); break;
    case 'M':
    case 'm': do_wmlist(ch, buf2); break;
    case 'T':
    case 't':
    case 'W':
    case 'w': list_wands_staves(ch,buf); break;
    case 'P':
    case 'p':
    case 'S':
    case 's': list_scrolls_potions(ch,buf); break;
    case 'R':
    case 'r': do_wrlist(ch,buf2); break;
    case '#': do_list_wear(ch, buf2); break;
    default:
        send_to_char (message, ch);
        break;
    }  /*switch...*/
    return;
}






ACMD(do_chown)
{
    struct char_data *victim;
    struct obj_data *obj;
    char buf2[80];
    char buf3[80];
    int i, k = 0;

    two_arguments(argument, buf2, buf3);

    if (!*buf2)
        send_to_char("Syntax: chown <object> <character>.\r\n", ch);
    else if (!(victim = get_char_vis(ch, buf3)))
        send_to_char("No one by that name here.\r\n", ch);
    else if (victim == ch)
        send_to_char("Are you sure you're feeling ok?\r\n", ch);
    else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
        send_to_char("That's really not such a good idea.\r\n", ch);
    else if (!*buf3)
        send_to_char("Syntax: chown <object> <character>.\r\n", ch);
    else {
        for (i = 0; i < NUM_WEARS; i++) {
            if (GET_EQ(victim, i) && CAN_SEE_OBJ(ch, GET_EQ(victim, i)) &&
                    isname(buf2, GET_EQ(victim, i)->name)) {
                obj_to_char(unequip_char(victim, i), victim);
                k = 1;
            }
        }

        if (!(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
            if (!k && !(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
                sprintf(buf, "%s does not appear to have the %s.\r\n", GET_NAME(victim), buf2);
                send_to_char(buf, ch);
                return;
            }
        }

        act("&n$n makes a magical gesture and $p&n flies from $N to $m.",FALSE,ch,obj,
            victim,TO_NOTVICT);
        act("&n$n makes a magical gesture and $p&n flies away from you to $m.",FALSE,ch,
            obj,victim,TO_VICT);
        act("&nYou make a magical gesture and $p&n flies away from $N to you.",FALSE,ch,
            obj, victim,TO_CHAR);

        obj_from_char(obj);
        obj_to_char(obj, ch);
        save_char(ch, NOWHERE);
        save_char(victim, NOWHERE);
    }

}


ACMD(do_vwear)
{
    one_argument(argument, buf);

    if (!*buf) {
        send_to_char("Usage: vwear <wear position>|<item type>\r\n"
                     "Wear positions are:\r\n"
                     "finger  neck   body   head   legs    feet    hands   face\r\n"
                     "shield  arms   about  waist  wrist   wield   hold    back\r\n"
                     "eyes    ears\r\n"
                     "---------------------------------------------------------\r\n"
                     "Item types are:\r\n"
                     "light   scroll  wand   staff  treasure  armour    table\r\n" /* show types */
                     "potion  worn    other  trash  container liquid \r\n"
                     "key     food    money  pen    boat      fountain\r\n", ch);

        return;
    }
    if (is_abbrev(buf, "finger"))
        vwear_object(ITEM_WEAR_FINGER, ch);
    else if (is_abbrev(buf, "neck"))
        vwear_object(ITEM_WEAR_NECK, ch);
    else if (is_abbrev(buf, "body"))
        vwear_object(ITEM_WEAR_BODY, ch);
    else if (is_abbrev(buf, "head"))
        vwear_object(ITEM_WEAR_HEAD, ch);
    else if (is_abbrev(buf, "legs"))
        vwear_object(ITEM_WEAR_LEGS, ch);
    else if (is_abbrev(buf, "feet"))
        vwear_object(ITEM_WEAR_FEET, ch);
    else if (is_abbrev(buf, "hands"))
        vwear_object(ITEM_WEAR_HANDS, ch);
    else if (is_abbrev(buf, "arms"))
        vwear_object(ITEM_WEAR_ARMS, ch);
    else if (is_abbrev(buf, "shield"))
        vwear_object(ITEM_WEAR_SHIELD, ch);
    else if (is_abbrev(buf, "about body"))
        vwear_object(ITEM_WEAR_ABOUT, ch);
    else if (is_abbrev(buf, "waist"))
        vwear_object(ITEM_WEAR_WAIST, ch);
    else if (is_abbrev(buf, "wrist"))
        vwear_object(ITEM_WEAR_WRIST, ch);
    else if (is_abbrev(buf, "wield"))
        vwear_object(ITEM_WEAR_WIELD, ch);
    else if (is_abbrev(buf, "hold"))
        vwear_object(ITEM_WEAR_HOLD, ch);
    else if (is_abbrev(buf, "back"))
        vwear_object(ITEM_WEAR_BACK, ch);
    else if (is_abbrev(buf, "face"))
        vwear_object(ITEM_WEAR_FACE, ch);
   else if (is_abbrev(buf, "eyes"))
        vwear_object(ITEM_WEAR_EYES, ch);
   else if (is_abbrev(buf, "ears"))
        vwear_object(ITEM_WEAR_EARS, ch);       
    else if (is_abbrev(buf, "light")) /* Start of new vwear- Item types */
        vwear_obj(ITEM_LIGHT, ch);
    else if (is_abbrev(buf, "scroll"))
        vwear_obj(ITEM_SCROLL, ch);
    else if (is_abbrev(buf, "wand"))
        vwear_obj(ITEM_WAND, ch);
    else if (is_abbrev(buf, "staff"))
        vwear_obj(ITEM_STAFF, ch);
    else if (is_abbrev(buf, "treasure"))
        vwear_obj(ITEM_TREASURE, ch);
    else if (is_abbrev(buf, "armor"))
        vwear_obj(ITEM_ARMOR, ch);
    else if (is_abbrev(buf, "potion"))
        vwear_obj(ITEM_POTION, ch);
    else if (is_abbrev(buf, "worn"))
        vwear_obj(ITEM_WORN, ch);
    else if (is_abbrev(buf, "other"))
        vwear_obj(ITEM_OTHER, ch);
    else if (is_abbrev(buf, "trash"))
        vwear_obj(ITEM_TRASH, ch);
    else if (is_abbrev(buf, "container"))
        vwear_obj(ITEM_CONTAINER, ch);
    else if (is_abbrev(buf, "liquid"))
        vwear_obj(ITEM_DRINKCON, ch);
    else if (is_abbrev(buf, "key"))
        vwear_obj(ITEM_KEY, ch);
    else if (is_abbrev(buf, "food"))
        vwear_obj(ITEM_FOOD, ch);
    else if (is_abbrev(buf, "money"))
        vwear_obj(ITEM_MONEY, ch);
    else if (is_abbrev(buf, "pen"))
        vwear_obj(ITEM_PEN, ch);
    else if (is_abbrev(buf, "boat"))
        vwear_obj(ITEM_BOAT, ch);
    else if (is_abbrev(buf, "fountain"))
        vwear_obj(ITEM_FOUNTAIN, ch);
    else if (is_abbrev(buf, "table"))
        vwear_obj(ITEM_TABLE, ch);
    else
        send_to_char("Possible positions are:\r\n"
                     "Wear positions are:\r\n"
                     "finger  neck   body   head   legs    feet    hands   face\r\n"
                     "shield  arms   about  waist  wrist   wield   hold    back\r\n"
                     "eyes    ears\r\n"
                     "---------------------------------------------------------\r\n" /* new types */
                     "Item types are:\r\n"
                     "light   scroll  wand   staff  treasure  armour    table\r\n"
                     "potion  worn    other  trash  container liquid \r\n"
                     "key     food    money  pen    boat      fountain\r\n", ch);
}

int no_more=0;

ACMD(do_guzva)
{
    int limit=5;
    int count, total=0;
    two_arguments(argument, buf, buf2);

    if (!*buf)
    {
        send_to_char("Usage: guzva o|m <number>\r\n",ch);
        return;
    }



    if (!*buf2)
        limit=10;
    else
        limit=atoi(buf2);
    if (limit<5)
        limit=5;
    if (buf[0]=='m')
    {
        int i;
        extern int top_of_world;
        for (i = 0; i < top_of_world; i++)
        {
            count=0;

            if (world[i].people != NULL)
            {
                struct char_data *pom=world[i].people;
                count=1;
                while (pom->next_in_room != NULL)
                {
                    pom=pom->next_in_room;
                    count++;
                }
                if (count>=limit)
                {
                    sprintf(buf, "[%5d] %40s - %3d mobs\r\n", GET_ROOM_VNUM(i), world[i].name, count);
                    send_to_char(buf, ch);
                    total+=count;
                }
            }
        }
        sprintf(buf, "\r\nTotal count %d.\r\n", total);
        send_to_char(buf, ch);
    }
    else if (buf[0]=='o')
    {
        int i;
        extern int top_of_world;
        for (i = 0; i < top_of_world; i++)
        {
            count=0;

            if (world[i].contents != NULL)
            {
                struct obj_data *pom=world[i].contents;
                count=1;
                while (pom->next_content != NULL)
                {
                    pom=pom->next_content;
                    count++;
                }
                if (count>=limit)
                {
                    sprintf(buf, "[%5d] %40s - %3d objs\r\n", GET_ROOM_VNUM(i), world[i].name, count);
                    send_to_char(buf, ch);
                    total+=count;
                }
            }
        }
        sprintf(buf, "\r\nTotal count %d.\r\n", total);
        send_to_char(buf, ch);
    }
    else if (buf[0]=='n')
    {
        no_more=!no_more;
        sprintf(buf, "***** NO MORE IS NOW %d", no_more);
        log(buf);
        send_to_char(buf, ch);
    }
}





ACMD(do_irepair)
{
    struct obj_data *obj;
    argument = two_arguments(argument, buf1, buf2);

    if (!*buf1) {

        send_to_char("Usage: irepair <obj> <new damage percent>\r\n", ch);
        return;
    }
    if (!(obj = get_obj_in_list_vis(ch, buf1, ch->carrying))) {
        sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
        send_to_char(buf, ch);
        return;
    }

    if (!*buf2)
    {
        send_to_char("Set damage to how much?\r\n", ch);
        return;
    }

    GET_OBJ_DAMAGE(obj)=MAX(0, MIN(100, atoi(buf2)));
    send_to_char("Ok\r\n", ch);

}



ACMD(do_areas)
{
    int i, j, k, l;

    one_argument(argument, buf2);

    k=world[ch->in_room].zone;

    strcpy(buf,"&w Num Name                                   Avg (Min-Max)  Entered from&0\r\n --- ----                                   -------------  ------------\r\n");
    if (!(j=atoi(buf2)))
    {
        for (i = 0; i <= top_of_zone_table; i++)

            if (area_info[i].num>4)
            {
                sprintf(buf,"%s%c%3d &c%-39s &y%2d&0 (%2d - %2d)&c  ", buf, k==i?'*':' ',
                        zone_table[i].number, zone_table[i].name, (zone_table[i].avg+(area_info[i].avg-area_info[i].max-area_info[i].min)/(area_info[i].num-2))/2,area_info[i].min,area_info[i].max, zone_table[i].creator );
                 for (j=0;j<top_of_zone_table;j++)
                	if (area_info[i].connected_from[j])
                		sprintf(buf, "%s%s, ", buf, zone_table[j].name);
                //sprintf(buf, "%s%s, ", buf, zone_table[j].name);
                sprintf(buf, "%s&0\r\n", buf);


            }
        strcat (buf, "\r\nUse 'areas <num>' to get detailed information on particular area.\r\n");
    }
    else
    {	for (i = 0; i <= top_of_zone_table; i++)
        if (zone_table[i].number==j && area_info[i].num>4)
        {
            sprintf(buf,"%s%c%3d &c%-39s &y%2d&0 (%2d - %2d)&c  %s&0\r\n", buf, k==i?'*':' ',
                    zone_table[i].number, zone_table[i].name, (zone_table[i].avg+(area_info[i].avg-area_info[i].max-area_info[i].min)/(area_info[i].num-2))/2,area_info[i].min,area_info[i].max, zone_table[i].creator );
//            sprintf(buf, "%s\r\nThis zone is connected with the following zones:\r\n", buf);
            sprintf(buf, "%s\r\nExits to:\r\n", buf);
            for (j=0;j<top_of_zone_table;j++)
                if (area_info[i].connected_to[j])
                    //	sprintf(buf, "%s&c%s&0\r\n", buf, zone_table[j].name);
                    sprintf(buf,"%s%c%3d &c%-39s &y%2d&0 (%2d - %2d)&c  %s&0\r\n", buf, k==i?'*':' ',
                            zone_table[j].number, zone_table[j].name, area_info[j].num>2?(zone_table[j].avg+(area_info[j].avg-area_info[j].max-area_info[j].min)/(area_info[j].num-2))/2:0,area_info[j].min,area_info[j].max, zone_table[j].creator );
            sprintf(buf, "%s\r\nEntered from:\r\n", buf);
            for (j=0;j<top_of_zone_table;j++)
                if (area_info[i].connected_from[j])
                    //	sprintf(buf, "%s&c%s&0\r\n", buf, zone_table[j].name);
                    sprintf(buf,"%s%c%3d &c%-39s &y%2d&0 (%2d - %2d)&c  %s&0\r\n", buf, k==i?'*':' ',
                            zone_table[j].number, zone_table[j].name, area_info[j].num>2?(zone_table[j].avg+(area_info[j].avg-area_info[j].max-area_info[j].min)/(area_info[j].num-2))/2:0,area_info[j].min,area_info[j].max, zone_table[j].creator );
                            
            break;

        }
        if (i>top_of_zone_table)
            strcpy(buf, "No info on that area number.\r\n");
    }


    page_string(ch->desc, buf, 1);
}

ACMD(do_modexp)
{        
	int i=0;
    one_argument(argument, buf);

    if (!*buf) {
    	send_to_char("Usage: modexp <modifier>  (times 10)\n", ch);
    	 ch_printf(ch, "Modexp set to %d.\n", modexp);
    	return;
    	}
                
   modexp=atoi(buf);                
   
   ch_printf(ch, "Modexp set to %d.\n", modexp);
  }                      
  
  
void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
                           int recur);

ACMD(do_showegos)
{
	
    struct obj_data *i, *last=NULL;  
    int j=1;
    
    send_to_char("Hidden egos:\r\n", ch);
    for (i = object_list; i ; i = i->next) 
    	if (IS_SET(GET_OBJ_EXTRA(i), ITEM_HIDDEN_EGO))    	
    		print_object_location(j++, i, ch, TRUE);
    		
    send_to_char("Normal egos:\r\n", ch);    		
    j=1;
    for (i = object_list; i ; i = i->next) 
    	if (IS_SET(GET_OBJ_EXTRA(i), ITEM_EGO) && !IS_SET(GET_OBJ_EXTRA(i), ITEM_HIDDEN_EGO))    	
    		print_object_location(j++, i, ch, TRUE);    		
}
    		
	   