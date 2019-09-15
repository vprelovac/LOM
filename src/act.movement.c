/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <string.h>

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
#include "house.h"
#include "auction.h"
#include "events.h"

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct index_data *mob_index;
extern int rev_dir[];
extern char *dirs[];
extern char *dirs2[];
extern int movement_loss[];
extern int get_carry_cond(struct char_data *ch);
/* external functs */
int special(struct char_data * ch, int cmd, char *arg);
void death_cry(struct char_data * ch);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
void mprog_greet_trigger(struct char_data * ch);
void mprog_entry_trigger(struct char_data * mob);
extern int      top_of_world;   /* In db.c */


#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))) :\
			(IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_PICKPROOF)) : \
			(IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)))

#define DOOR_IS_BASHPROOF(ch, door) ( \
			(IS_SET(EXIT(ch, door)->exit_info, EX_BASHPROOF)))			
#define DOOR_IS_NOPASSDOOR(ch, door) ( \
			(IS_SET(EXIT(ch, door)->exit_info, EX_NOPASSDOOR)))						


#define DOOR_IS_CLOSED(ch, obj, door)	(!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)	(!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)		((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))

/* simple function to determine if char can walk on water */
int has_boat(struct char_data * ch)
{
    struct obj_data *obj;
    int i;

    if (IS_AFFECTED(ch, AFF_WATERWALK))
        return 1;

    if (IS_AFFECTED(ch, AFF_FLYING))
        return 1;


    /* non-wearable boats in inventory will do it */
    for (obj = ch->carrying; obj; obj = obj->next_content)
        if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
            return 1;

    /* and any boat you're wearing will do it too */
    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
            return 1;

    return 0;
}

int can_fly(struct char_data * ch)
{
    struct obj_data *obj;
    int i;

    if (IS_AFFECTED(ch, AFF_FLYING))
        return 1;

    /* non-wearable flying object in inventory will do it */
    /*    for (obj = ch->carrying; obj; obj = obj->next_content)
    	if (GET_OBJ_TYPE(obj) == ITEM_FLIGHT && (find_eq_pos(ch, obj, NULL) < 0))
    	    return 1;

      
        for (i = 0; i < NUM_WEARS; i++)
    	if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_FLIGHT)
    	    return 1;
    */
    return 0;
}

int can_lava(struct char_data * ch)
{

    if (IS_AFFECTED(ch, AFF_PROT_FIRE))
        return 1;

    return 0;
}

int can_artic(struct char_data * ch)
{

    if (AFF2_FLAGGED(ch, AFF2_PROT_COLD))
        return 1;

    return 0;
}

int can_under(struct char_data * ch)
{
    if (IS_AFFECTED(ch, AFF_WATERBREATH))
        return 1;
    return 0;
}


struct move_event_obj
{
    struct char_data *ch;
    int dir, need_specials_check;
    float need_movement;
};

EVENTFUNC(move_event)
{
    int was_in;
    struct char_data *ch;
    int dir, need_specials_check;
    float need_movement;
    struct move_event_obj *sniff = (struct move_event_obj *) event_obj;
    struct char_data *pom, *pom1;
        char *s;
    ch=sniff->ch;
    dir=sniff->dir;
    need_specials_check=sniff->need_specials_check;
    need_movement=sniff->need_movement;


    if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch)) {
        GET_MOVE(ch) -= need_movement/5;
    }

    was_in = ch->in_room;
    char_from_room(ch);
    //    if (!IS_NPC(ch) && AFF_FLAGGED(ch, AFF_HIDE) && number(1,100)>GET_SKILL(ch, SKILL_MOVE_HIDDEN))
    //      REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);
    char_to_room(ch, world[was_in].dir_option[dir]->to_room);
        
            if (GET_COND(ch, DRUNK) > 5)
            act("$n stumbles in drunkenly, cheering everyone.",FALSE, ch,NULL,NULL,TO_ROOM);
        else
        {
            if (AFF2_FLAGGED(ch, AFF2_HASTE))
                sprintf(buf2, "$n rushes in from %s.", dirs2[rev_dir[dir]]);
            else if (AFF_FLAGGED(ch, AFF_FLYING))
                sprintf(buf2, "$n flies in from %s.", dirs2[rev_dir[dir]]);
            else if (AFF_FLAGGED(ch, AFF_WATERWALK))
                sprintf(buf2, "$n floats in from %s.", dirs2[rev_dir[dir]]);
            else  
            { if (IS_NPC(ch))
            
                sprintf(buf2, "$n arrives from %s.", dirs2[rev_dir[dir]]);
                else
                
                sprintf(buf2, "$n walks in from %s.", dirs2[rev_dir[dir]]);
               }

            if (!IS_NPC(ch) && strcmp(ch->player_specials->saved.walkin, "off"))
            {
                strcpy(buf, ch->player_specials->saved.walkin);
                if ((s=strchr(buf, '#')))
                {
                    *s=0;
                    sprintf(buf2, "$n %s%s%s", buf, dirs[dir], s+1);
                }
                else
                    sprintf(buf2, "$n %s", buf);
            }
            if (!IS_AFFECTED(ch, AFF_SNEAK) || IN_ARENA(ch) || number(1, 100)>GET_SKILL(ch, SKILL_SNEAK))
                act(buf2, TRUE, ch, 0, ((ch->master && need_specials_check==50) ? ch->master : NULL), TO_NOTVICT);

        }

        if (ch->desc != NULL)
            look_at_room(ch, 0);


        if (ROOM_AFFECTED(ch->in_room, RAFF_TRAP) && number(1, 250)>GET_CHA(ch))
        {
            //         	CREF(ch, CHAR_NULL);
            FireRoomTrap(ch);
            //       	CUREF(ch);
            if (DEAD(ch))
            { GET_MOVE_EVENT(ch)=NULL;
        DISPOSE(event_obj);
                return 0;  
        }
        }

        if (ch && ch->in_room>0 && ch->in_room < top_of_world)
            //if (!IS_NPC(ch))
            for (pom=world[ch->in_room].people; pom; pom=pom1)
            {
                pom1=pom->next_in_room;
                if (AFF3_FLAGGED(pom, AFF3_AMBUSH))
                    if (CAN_MURDER(pom, ch) && (!pom->ambush_name || isname(pom->ambush_name, ch->player.name)))	// they are victims
                    {
                        // now check it the victim spots the ambush
                        act("\r\n&GYou charge towards $N!&0\r\n", FALSE, pom, 0, ch, TO_CHAR);
                        act("\r\n&C$n charges towards you!&0\r\n", FALSE, pom, 0, ch, TO_VICT);
                        act("\r\n&c$n ambushes $N!&0\r\n", FALSE, pom, 0, ch, TO_NOTVICT);
                        //		CREF(ch, CHAR_NULL);
                        //		CREF(pom, CHAR_NULL);
                        REMOVE_BIT(AFF3_FLAGS(pom), AFF3_AMBUSH);
                        if (pom->ambush_name)
                            DISPOSE(pom->ambush_name);
                        if (!DEX_CHECK(pom) || (GET_POS(ch)>POS_STUNNED && (MOB_FLAGGED(ch, MOB_AWARE) || (CAN_SEE(ch, pom) && number(1, 101)>GET_SKILL(pom, SKILL_AMBUSH)))))
                        {
                            // they've seen it
                            act("Luckily, you notice $M in time!", FALSE, ch, 0, pom, TO_CHAR);
                            act("$e notices you lunging!", FALSE, ch, 0, pom, TO_VICT);
                            act("$n notices $N in time to ready for combat.", FALSE, ch, 0, pom, TO_NOTVICT);
                            hit(pom, ch, TYPE_UNDEFINED);
                            WAIT_STATE(pom, PULSE_VIOLENCE);
                        }
                        else
                        {
                            hit(pom, ch, SKILL_AMBUSH);
                            WAIT_STATE(pom, 2 * PULSE_VIOLENCE);
                        }
                        //		CUREF(ch);
                        //		CUREF(pom);
                    }
            }

        if (DEAD(ch))   
        {
        	 GET_MOVE_EVENT(ch)=NULL;
        DISPOSE(event_obj);
            return 0;
        }

        if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT) {
            char buf2[200];
            log_death_trap(ch);
            sprintf(buf2, "\r\n&BINFO || &C%s&c hits the death trap.&0\r\n", GET_NAME(ch));
            INFO_OUT(buf2);
            
             GET_MOVE_EVENT(ch)=NULL;
             DISPOSE(event_obj);
            
            die(ch, NULL);
            return 0;
        }
        else if (!DEAD(ch))//&& !IS_NPC(ch))
        {
            mprog_entry_trigger(ch);
            rprog_enter_trigger(ch);
            mprog_greet_trigger(ch);
            oprog_greet_trigger(ch);
        }
    
        
        
    /*    
        
    if ((IN_ARENA(ch) || !IS_AFFECTED(ch, AFF_SNEAK)) {
        if (AFF2_FLAGGED(ch, AFF2_HASTE))
            sprintf(buf2, "$n arrives quickly from %s.", dirs2[rev_dir[dir]]);
        else if (AFF_FLAGGED(ch, AFF_FLYING))
            sprintf(buf2, "$n flies in from %s.", dirs2[rev_dir[dir]]);
        else if (AFF_FLAGGED(ch, AFF_WATERWALK))
            sprintf(buf2, "$n floats in from %s.", dirs2[rev_dir[dir]]);
        else
            sprintf(buf2, "$n walks in from %s.", dirs2[rev_dir[dir]]);
        act(buf2, TRUE, ch, 0, ((ch->master && need_specials_check==50) ? ch->master : NULL), TO_NOTVICT);
    }


    if (ch->desc != NULL)
        look_at_room(ch, 0);
    if (ROOM_AFFECTED(ch->in_room, RAFF_TRAP) && number(1, 250)>GET_CHA(ch))
    {
        //CREF(ch, CHAR_NULL);
        FireRoomTrap(ch);
        //CUREF(ch);
        if (DEAD(ch))
        {
            DISPOSE(event_obj);
            return 0;
        }
    }

    if (ch && ch->in_room>0 && ch->in_room < top_of_world)
        //if (!IS_NPC(ch))
        for (pom=world[ch->in_room].people; pom; pom=pom1)
        {
            pom1=pom->next_in_room;
            if (AFF3_FLAGGED(pom, AFF3_AMBUSH))
                if (CAN_MURDER(pom, ch) && (!pom->ambush_name || isname(pom->ambush_name, ch->player.name)))	// they are victims
                {
                    // now check it the victim spots the ambush
                    act("\r\n&GYou charge towards $N!&0\r\n", FALSE, pom, 0, ch, TO_CHAR);
                    act("\r\n&C$n charges towards you!&0\r\n", FALSE, pom, 0, ch, TO_VICT);
                    act("\r\n&c$n ambushes $N!&0\r\n", FALSE, pom, 0, ch, TO_NOTVICT);
                    //		CREF(ch, CHAR_NULL);
                    //		CREF(pom, CHAR_NULL);
                    REMOVE_BIT(AFF3_FLAGS(pom), AFF3_AMBUSH);
                    if (pom->ambush_name)                        
                            DISPOSE(pom->ambush_name);
                    if (!DEX_CHECK(pom) || (GET_POS(ch)>POS_STUNNED && (MOB_FLAGGED(ch, MOB_AWARE) || (CAN_SEE(ch, pom) && number(1, 101)>GET_SKILL(pom, SKILL_AMBUSH)))))
                    {
                        // they've seen it
                        act("Luckily, you notice $M in time!", FALSE, ch, 0, pom, TO_CHAR);
                        act("$e notices you lunging!", FALSE, ch, 0, pom, TO_VICT);
                        act("$n notices $N in time to ready for combat.", FALSE, ch, 0, pom, TO_NOTVICT);
                        hit(pom, ch, TYPE_UNDEFINED);
                        WAIT_STATE(pom, PULSE_VIOLENCE);
                    }
                    else
                    {
                        hit(pom, ch, SKILL_AMBUSH);
                        WAIT_STATE(pom, PULSE_VIOLENCE);
                    }
                }
                else if (!CAN_MURDER(pom, ch))
                {
                    act("You let $N safely pass.", FALSE, pom, 0, ch, TO_CHAR);
                    return;
                }
        }

    if (DEAD(ch))
    {

        DISPOSE(event_obj);
        return 0;
    }


    if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT) {
        char buf2[200];
        log_death_trap(ch);
        //	death_cry(ch);
        sprintf(buf2, "\r\n&BINFO || &C%s&c hits the death trap.&0\r\n", GET_NAME(ch));
        INFO_OUT(buf2);            
        
        
        
        GET_MOVE_EVENT(ch)=NULL;
        DISPOSE(event_obj);
        die(ch, NULL);

        return 0;
    } else
    {
        mprog_entry_trigger(ch);
        mprog_greet_trigger(ch);
    }                             
    
    */

    GET_MOVE_EVENT(ch)=NULL;
    DISPOSE(event_obj);

    return 0;
}






/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
 */
int do_simple_move(struct char_data * ch, int dir, int need_specials_check)
{
    int was_in;
    float need_movement;
    struct move_event_obj *sniff;
    struct char_data *pom, *pom1;
    char *s;
    int special(struct char_data * ch, int cmd, char *arg);



    if (DEAD(ch))
        return 0;
    /* Check for special routines (North is 1 in command list, but 0 here)
       Note -- only check if following; this avoids 'double spec-proc' bug */
    if (need_specials_check && special(ch, dir + 1, ""))
        return 0;

    /* charmed? */
    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && ch->in_room == ch->master->in_room) {// && !MOB_FLAGGED(ch->master,MOB_GANG_LEADER)) {
        send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
        if (!number(0,5))
            act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
        return 0;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GODROOM) ||
            ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_GODROOM)) {
        if (GET_LEVEL(ch) < LVL_IMMORT) {
            send_to_char("A strange force prevents you from entering there.\r\n", ch);
            return 0;
        }
    }
    // if this room or the one we're going to needs flight, check for flight */

    if ((SECT(EXIT(ch, dir)->to_room) == SECT_FLYING) && GET_LEVEL(ch) < LVL_IMMORT) {
        if (!can_fly(ch)) {
            send_to_char("You need to be able to fly to go there.\r\n", ch);
            return 0;
        }
    }


    if (((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
            (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM))
            && GET_LEVEL(ch) < LVL_IMMORT) {
        if (!can_fly(ch) && !has_boat(ch)) {
            send_to_char("You need a boat to go there.\r\n", ch);
            return 0;
        }
    }
    // if this room or the one we're going to needs boat/underwater, check  for boat/underwater
    if (((SECT(ch->in_room) == SECT_UNDERWATER) ||
            (SECT(EXIT(ch, dir)->to_room) == SECT_UNDERWATER))
            && GET_LEVEL(ch) < LVL_IMMORT) {
        if (!can_under(ch)) {
            send_to_char("You better know how to breath underwater...\r\n", ch);
        }
    }
    if (((SECT(ch->in_room) == SECT_LAVA) ||
            (SECT(EXIT(ch, dir)->to_room) == SECT_LAVA))
            && GET_LEVEL(ch) < LVL_IMMORT) {
        if (!can_lava(ch))
            SET_BIT(AFF2_FLAGS(ch), AFF2_BURNING);
    }

    if (((SECT(ch->in_room) == SECT_ARCTIC) ||
            (SECT(EXIT(ch, dir)->to_room) == SECT_ARCTIC))
            && GET_LEVEL(ch) < LVL_IMMORT) {
        if (!can_artic(ch))
            SET_BIT(AFF2_FLAGS(ch), AFF2_FREEZING);
    }



    /* move points needed is avg. move loss for src and destination sect
       type */

    if (!IS_NPC(ch))
    {
        need_movement = (float) (movement_loss[SECT(ch->in_room)] +
                                 movement_loss[SECT(EXIT(ch, dir)->to_room)]) /10;
        need_movement*=((float) get_carry_cond(ch)+4.0)/4.0;
        if (IS_AFFECTED(ch, AFF_FLYING))
            need_movement = 1;
        if (IS_AFFECTED(ch, AFF_WATERWALK))
            need_movement =1;
        if (AFF_FLAGGED(ch, AFF_NOTRACK))
            need_movement *= 1.5;
        if (AFF3_FLAGGED(ch, AFF3_WEB))
            need_movement *=4;
        if (AFF2_FLAGGED(ch, AFF2_HASTE))
            need_movement *=0.8;
        if (AFF2_FLAGGED(ch, AFF2_ADRENALIN))
            need_movement *=0.8 ;
        if (AFF3_FLAGGED(ch, AFF3_ENDURE))
            need_movement /=2.0;
        if (AFF_FLAGGED(ch, AFF_SNEAK))
            need_movement *=2.0;  
        if (IN_ARENA(ch))
        	need_movement /=2.0;
        if (AFF2_FLAGGED(ch, AFF2_MOVE_HIDDEN))
        {
            need_movement *=3-GET_SKILL(ch, SKILL_MOVE_HIDDEN)/100;
        }

    }
    else
        need_movement=0;

    if ((GET_MOVE(ch) -need_movement<MOVE_LIMIT(ch)) && !IS_NPC(ch)) {
        if (need_specials_check && ch->master)
            send_to_char("&wYou are too exhausted to follow.&0\r\n", ch);
        else{
            send_to_char("&wYou are too exhausted.&0\r\n", ch);
        }
        return 0;
    }
    if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_ATRIUM)) {
        if (!House_can_enter(ch, world[EXIT(ch, dir)->to_room].number)) {
            send_to_char("That's private property -- no traspassing!\r\n", ch);
            return 0;
        }
    }
    if (!IS_IMMORT(ch) && IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
            world[EXIT(ch, dir)->to_room].people != NULL) {
        send_to_char("There isn't enough room there for more than one person!\r\n", ch);
        return 0;
    }
    if (!IS_IMMORT(ch) && IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_PRIVATE) &&
            world[EXIT(ch, dir)->to_room].people != NULL &&
            (world[EXIT(ch, dir)->to_room].people)->next_in_room != NULL) {
        send_to_char("There isn't enough room there for more than two people!\r\n", ch);
        return 0;
    }

    /* if this room or the one we're going to needs flight, check for
       flight */
    if ((SECT(ch->in_room) == SECT_FLYING) && GET_LEVEL(ch) < LVL_IMMORT) {

        if (!can_fly(ch)) {
            char buf2[200];
            send_to_char("You have plummeted to the ground.\r\n", ch);
            log_death_trap(ch);
            //  death_cry(ch);
            sprintf(buf2, "\r\n&BINFO || &C%s&c plummeted to the ground.&0\r\n", GET_NAME(ch));
            INFO_OUT(buf2);
            //  extract_char(ch);
            die(ch, NULL);
            return 0;
        }
    }
    if (AFF3_FLAGGED(ch, AFF3_WEB))
    {
        if (number(1, 300)>GET_DEX(ch)+GET_STR(ch))
        {
            send_to_char("You attempt to leave but stumble in the web and fall down!\r\n", ch);
            act("$n attempts to leave but stumbles in the web and falls down!", FALSE, ch, 0, 0, TO_ROOM);
            GET_POS(ch)=POS_SITTING;
            return 0;
        }
        else
        {
            send_to_char("You struggle with and somehow manage to move.\r\n", ch);
            act("$n struggles and somehow manages to move.", FALSE, ch, 0, 0, TO_ROOM);
        }
    }

#ifdef EVENT_MOVE   
    if (!IS_NPC(ch) && ((!PRF2_FLAGGED(ch, PRF2_RUNNING) || (ch->master && (need_specials_check==50) && (IS_NPC(ch->master) || !PRF2_FLAGGED(ch->master, PRF2_RUNNING))))))
    {
        int pulse_move=5;
        int room_to=world[ch->in_room].dir_option[dir]->to_room;
        CREATE(sniff, struct move_event_obj, 1);
        sniff->ch=ch;
        sniff->dir=dir;
        sniff->need_specials_check=need_specials_check;
        sniff->need_movement=need_movement;
        if (need_specials_check!=50)
        {
            //if (IS_DARK(room_to) && !CAN_SEE_IN_DARK(ch))
            sprintf(buf, "You depart %s.\r\n", dirs [dir]);
            //else
            //	sprintf(buf, "You depart towards %s.", world[room_to].name);
            send_to_char(buf, ch);
        }
        if (need_specials_check==50)
        {
            //pulse_move++;
            act("$n follows $N.", FALSE, ch, 0 , ch->master , TO_NOTVICT);
        }
        else
        {
            act("$n departs $T.", FALSE, ch, 0 , dirs[dir] , TO_ROOM);
        }
        
        rprog_leave_trigger(ch);
        if (DEAD(ch))
            return 0;
        
        GET_MOVE_EVENT(ch)=event_create(move_event, sniff, pulse_move);
        WAIT_STATE(ch, pulse_move);
        return 1;
    }
    else
#endif
    {
        //need_movement*=(float) 1.5;




        if (GET_COND(ch, DRUNK) > 5)
            act("$n stumbles off drunkenly on $s way $T.",	FALSE,  ch,NULL,dirs[dir],TO_ROOM);
        else
        {
            if (AFF2_FLAGGED(ch, AFF2_HASTE))
            {
                sprintf(buf2, "$n quickly leaves %s.", dirs[dir]);
                sprintf(buf1, "You quickly leave %s.\r\n", dirs[dir]);
            }
            else if (AFF_FLAGGED(ch, AFF_FLYING))
            {
                sprintf(buf2, "$n flies %s.", dirs[dir]);
                sprintf(buf1, "You fly %s.\r\n", dirs[dir]);
            }
            else if (AFF_FLAGGED(ch, AFF_WATERWALK))
            {
                sprintf(buf2, "$n floats %s.", dirs[dir]);
                sprintf(buf1, "You float %s.\r\n", dirs[dir]);
            }
            else if (IS_NPC(ch) && HUNTING(ch))
            {
                sprintf(buf2, "$n runs %s in a hurry.", dirs[dir]);
                sprintf(buf1, "You run %s in a hurry.\r\n", dirs[dir]);
            }
            else
            {
                sprintf(buf2, "$n leaves %s.", dirs[dir]);
                sprintf(buf1, "You leave %s.\r\n", dirs[dir]);
            }

            if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch))
                GET_MOVE(ch) -= need_movement;



            if (!IS_NPC(ch) && strcmp(ch->player_specials->saved.walkout, "off"))
            {
                strcpy(buf, ch->player_specials->saved.walkout);
                if ((s=strchr(buf, '#')))
                {
                    *s=0;
                    sprintf(buf2, "$n %s%s%s", buf, dirs[dir], s+1);
                }
                else
                    sprintf(buf2, "$n %s", buf);
            }


            if (need_specials_check!=50)
                send_to_char(buf1,ch);
            if (AFF2_FLAGGED(ch, AFF2_MOVE_HIDDEN))
            {
                if ((number(1, 101)>GET_SKILL(ch, SKILL_MOVE_HIDDEN) && number(1, 101)>GET_SKILL(ch, SKILL_HIDE)))
                {
                    affect_from_char(ch, SKILL_MOVE_HIDDEN);
                    send_to_char("&cYou fail to move hidden.&0\r\n", ch);
                }
                else
                    improve_skill(ch, SKILL_MOVE_HIDDEN, 12);
            }

            if (!IS_AFFECTED(ch, AFF_SNEAK) || IN_ARENA(ch) || number(1, 100)>GET_SKILL(ch, SKILL_SNEAK))
                act(buf2, TRUE, ch, 0, 0, TO_ROOM);
            else if (AFF_FLAGGED(ch, AFF_SNEAK))
                improve_skill(ch, SKILL_SNEAK, 12);
        }

        was_in = ch->in_room;

        rprog_leave_trigger(ch);
        if (DEAD(ch))
            return 0;


        char_from_room(ch);
        //    if (!IS_NPC(ch) && AFF_FLAGGED(ch, AFF_HIDE) && number(1,100)>GET_SKILL(ch, SKILL_MOVE_HIDDEN))
        //      REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);
        char_to_room(ch, world[was_in].dir_option[dir]->to_room);


        if (GET_COND(ch, DRUNK) > 5)
            act("$n stumbles in drunkenly, cheering everyone.",FALSE, ch,NULL,NULL,TO_ROOM);
        else
        {
            if (AFF2_FLAGGED(ch, AFF2_HASTE))
                sprintf(buf2, "$n rushes in from %s.", dirs2[rev_dir[dir]]);
            else if (AFF_FLAGGED(ch, AFF_FLYING))
                sprintf(buf2, "$n flies in from %s.", dirs2[rev_dir[dir]]);
            else if (AFF_FLAGGED(ch, AFF_WATERWALK))
                sprintf(buf2, "$n floats in from %s.", dirs2[rev_dir[dir]]);
            else
                sprintf(buf2, "$n arrives from %s.", dirs2[rev_dir[dir]]);

            if (!IS_NPC(ch) && strcmp(ch->player_specials->saved.walkin, "off"))
            {
                strcpy(buf, ch->player_specials->saved.walkin);
                if ((s=strchr(buf, '#')))
                {
                    *s=0;
                    sprintf(buf2, "$n %s%s%s", buf, dirs[dir], s+1);
                }
                else
                    sprintf(buf2, "$n %s", buf);
            }
            if (!IS_AFFECTED(ch, AFF_SNEAK) || IN_ARENA(ch) || number(1, 100)>GET_SKILL(ch, SKILL_SNEAK))
                act(buf2, TRUE, ch, 0, ((ch->master && need_specials_check==50) ? ch->master : NULL), TO_NOTVICT);

        }

        if (ch->desc != NULL)
            look_at_room(ch, 0);


        if (ROOM_AFFECTED(ch->in_room, RAFF_TRAP) && number(1, 250)>GET_CHA(ch))
        {
            //         	CREF(ch, CHAR_NULL);
            FireRoomTrap(ch);
            //       	CUREF(ch);
            if (DEAD(ch))
                return 0;
        }

        if (ch && ch->in_room>0 && ch->in_room < top_of_world)
            //if (!IS_NPC(ch))
            for (pom=world[ch->in_room].people; pom; pom=pom1)
            {
                pom1=pom->next_in_room;
                if (AFF3_FLAGGED(pom, AFF3_AMBUSH))
                    if (CAN_MURDER(pom, ch) && (!pom->ambush_name || isname(pom->ambush_name, ch->player.name)))	// they are victims
                    {
                        // now check it the victim spots the ambush
                        act("\r\n&GYou charge towards $N!&0\r\n", FALSE, pom, 0, ch, TO_CHAR);
                        act("\r\n&C$n charges towards you!&0\r\n", FALSE, pom, 0, ch, TO_VICT);
                        act("\r\n&c$n ambushes $N!&0\r\n", FALSE, pom, 0, ch, TO_NOTVICT);
                        //		CREF(ch, CHAR_NULL);
                        //		CREF(pom, CHAR_NULL);
                        REMOVE_BIT(AFF3_FLAGS(pom), AFF3_AMBUSH);
                        if (pom->ambush_name)
                            DISPOSE(pom->ambush_name);
                        if (!DEX_CHECK(pom) || (GET_POS(ch)>POS_STUNNED && (MOB_FLAGGED(ch, MOB_AWARE) || (CAN_SEE(ch, pom) && number(1, 101)>GET_SKILL(pom, SKILL_AMBUSH)))))
                        {
                            // they've seen it
                            act("Luckily, you notice $M in time!", FALSE, ch, 0, pom, TO_CHAR);
                            act("$e notices you lunging!", FALSE, ch, 0, pom, TO_VICT);
                            act("$n notices $N in time to ready for combat.", FALSE, ch, 0, pom, TO_NOTVICT);
                            hit(pom, ch, TYPE_UNDEFINED);
                            WAIT_STATE(pom, PULSE_VIOLENCE);
                        }
                        else
                        {
                            hit(pom, ch, SKILL_AMBUSH);
                            WAIT_STATE(pom, 2 * PULSE_VIOLENCE);
                        }
                        //		CUREF(ch);
                        //		CUREF(pom);
                    }
            }

        if (DEAD(ch))
            return 0;

        if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT) {
            char buf2[200];
            log_death_trap(ch);
            //	death_cry(ch);
            sprintf(buf2, "\r\n&BINFO || &C%s&c hits the death trap.&0\r\n", GET_NAME(ch));
            INFO_OUT(buf2);
            //GET_QUESTPOINTS(ch)-=2*GET_LEVEL(ch);
            //    char_from_room(ch);
            //  char_to_room(ch, real_room(3001));

            //extract_char(ch);
            die(ch, NULL);
            return 0;
        }
        else if (!DEAD(ch))//&& !IS_NPC(ch))
        {
            mprog_entry_trigger(ch);
            rprog_enter_trigger(ch);
            mprog_greet_trigger(ch);
            oprog_greet_trigger(ch);
        }
        return 1;
    };
}


int perform_move(struct char_data * ch, int dir, int need_specials_check)
{
    int was_in;
    struct follow_type *k, *next;

    if (ch == NULL || ch->in_room==NOWHERE || dir < 0 || dir >= NUM_OF_DIRS) //dead man dont walk
        return 0;


    if (GET_COND(ch, DRUNK)>0)
    {
        /* Uh oh, another drunk Frenchman on the loose! :) */
        if (GET_COND(ch, DRUNK) +5 > number(1, 100))
        {
            act("You feel a little drunk.. not to mention kind of lost..",
                FALSE, ch,NULL,NULL,TO_CHAR);
            act("$n looks a little drunk.. not to mention kind of lost..",
                FALSE, ch,NULL,NULL,TO_ROOM);
            dir = number(0,5);
        }
        /*  else
          {
            act("You feel a little.. drunk..",FALSE, ch,NULL,NULL,TO_CHAR);
            act("$n looks a little.. drunk..",FALSE, ch,NULL,NULL,TO_ROOM);
          }
          */      
    }

    if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
    {

        if (GET_COND(ch, DRUNK)>0)
        {
            if (number(1, 100)<GET_COND(ch, DRUNK)+10)
            {
                act("You drunkenly slam face-first into the 'exit' on your way $T.",FALSE, ch,NULL,dirs[dir],TO_CHAR);
                act("$n drunkenly slams face-first into the 'exit' on $s way $T.",
                    FALSE, ch,NULL,dirs[dir],TO_ROOM);
                GET_HIT(ch)-=number(1, 3);

            }
            else
            {
                if (GET_COND(ch, DRUNK) > 10)
                {
                    act("You stumble about aimlessly and fall down drunk.",
                        FALSE, ch,NULL,dirs[dir],TO_CHAR);
                    act("$n stumbles about aimlessly and falls down drunk.",
                        FALSE, ch,NULL,dirs[dir],TO_ROOM);
                    GET_POS(ch) = POS_RESTING;
                }
                else
                {
                    act("You almost go $T, but suddenly realize that there's no exit there.",FALSE, ch,NULL,dirs[dir],TO_CHAR);
                    act("$n looks like $e's about to go $T, but suddenly stops short and looks confused.",FALSE, ch,NULL,dirs[dir],TO_ROOM);
                }
            }
        }
        else
        {
            act("You almost go $T, but suddenly realize that there's no exit there.",FALSE, ch,NULL,dirs[dir],TO_CHAR);
            act("$n looks like $e's about to go $T, but suddenly stops short and looks confused.",FALSE, ch,NULL,dirs[dir],TO_ROOM);
        }
    }

    //send_to_char("Alas, you cannot go that way...\r\n", ch);

    else if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) && (!AFF_FLAGGED(ch, AFF_PASSDOOR) || DOOR_IS_NOPASSDOOR(ch, dir))) {
        if (EXIT(ch, dir)->keyword) {
            sprintf(buf2, "The %s seems to be closed.", fname(EXIT(ch, dir)->keyword));
            send_to_char(buf2, ch);
        } else
            send_to_char("It seems to be closed.", ch);                                                             
    }
    else if (IS_SET(EXIT(ch, dir)->exit_info, EX_NOMOB) && IS_NPC(ch))
    {
        send_to_char("Mobs are not allowed in there.", ch);

	}
	else if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT && (number(1, 111)<GET_SKILL(ch, SKILL_SPOT_TRAPS) || number(1, 111)<GET_SKILL(ch, SKILL_SIXTHSENSE) ) && INT_CHECK(ch) && DEX_CHECK(ch)) {
		send_to_char("&wYou change your mind in the last moment as you sense mortal danger there!&0\r\n", ch);
		act("$n almost goes $T but changes $s mind in the last moment.",FALSE, ch,NULL,dirs[dir],TO_ROOM);
		improve_skill(ch, SKILL_SPOT_TRAPS, 1);
		improve_skill(ch, SKILL_SIXTHSENSE, 1);
	
	}
 else {
        if (!ch->followers)
            return (do_simple_move(ch, dir, need_specials_check));

        was_in = ch->in_room;
        if (!do_simple_move(ch, dir, need_specials_check+70))
            return 0;
        if (DEAD(ch)) return 0;
        for (k = ch->followers; k; k = next) {
            next = k->next;
            if (k->follower && (was_in == k->follower->in_room) &&
                    (GET_POS(k->follower) >= POS_STANDING)) {
                if (!AFF2_FLAGGED(k->follower, AFF2_STALK))
                {
                    sprintf(buf, "You follow %s %s to %s.\r\n", GET_NAME(ch), dirs[dir], world[EXIT(k->follower, dir)->to_room].name);
                    send_to_char(buf, k->follower);
                    //act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
                }
                else
                    act("You quitely stalk $N.", FALSE, k->follower, 0, ch, TO_CHAR);
                if (perform_move(k->follower, dir, 50))
                    if (k->follower && !AFF2_FLAGGED(k->follower, AFF2_STALK)) act("$n follows you.", FALSE, k->follower, 0, ch, TO_VICT);
            }
        }

        return 1;
    }

    return 0;
}


ACMD(do_move)
{
    /* This is basically a mapping of cmd numbers to perform_move indices.
       It cannot be done in perform_move because perform_move is called by
       other functions which do not require the remapping. */
    int nr_times, loop;

    if (IN_EVENT(ch) && !GET_WEAR_EVENT(ch))
    {
        send_to_char("You are already performing an action.\r\n", ch);
        return;
    }

    if (!*argument)
        nr_times = 1;
    if (*argument) {
        if (atoi(argument) <= 0)
            nr_times = 1;
        else
            nr_times = atoi(argument);
    }
#ifdef EVENT_MOVE    
    if (nr_times>1  && !PRF2_FLAGGED(ch, PRF2_RUNNING))
    {
        send_to_char("You can speedwalk only while in running mode!\r\n", ch);
        return;
    }
#endif
    if (nr_times <= 5) {
        for (loop = 0; loop < nr_times; loop++)
            if (!FIGHTING(ch)) perform_move(ch, subcmd - 1, 0);
        send_to_char("\r\n", ch);
    } else
        send_to_char("Please limit your speedwalking to 5 moves per direction.\r\n", ch);
}


int find_door(struct char_data * ch, char *type, char *dir, char *cmdname)
{
    int door;

    if (*dir) {			/* a direction was specified */
        if ((door = search_block(dir, dirs, FALSE)) == -1) {	/* Partial Match */
            send_to_char("That's not a direction.\r\n", ch);
            return -1;
        }
        if (EXIT(ch, door))
            if (EXIT(ch, door)->keyword)
                if (isname(type, EXIT(ch, door)->keyword))
                    return door;
                else {
                    sprintf(buf2, "I see no %s there.\r\n", type);
                    send_to_char(buf2, ch);
                    return -1;
                } else
                return door;
        else {
            send_to_char("I really don't see how you can close anything there.\r\n", ch);
            return -1;
        }
    } else {			/* try to locate the keyword */
        if (!*type) {
            sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
            send_to_char(buf2, ch);
            return -1;
        }
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXIT(ch, door))
                if (EXIT(ch, door)->keyword)
                    if (isname(type, EXIT(ch, door)->keyword))
                        return door;
        if ((door = search_block(type, dirs, FALSE)) == -1) {	/* Partial Match */
            sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
            send_to_char(buf2, ch);
            return -1;
        }
        if (EXIT(ch, door))
            return door;

        sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
        send_to_char(buf2, ch);
        return -1;
    }
}


EXIT_DATA * find_door2(struct char_data * ch, char *dir, char *type)
{
    int door;


    if (*dir) {			/* a direction was specified */
        if ((door = search_block(dir, dirs, FALSE)) == -1) {	/* Partial Match */
            return NULL;
        }
        return EXIT(ch, door);

    } else {			/* try to locate the keyword */
        if (!*type) {
            return NULL;
        }
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXIT(ch, door))
                if (EXIT(ch, door)->keyword)
                    if (isname(type, EXIT(ch, door)->keyword))
                        return EXIT(ch, door);
        return NULL;
    }

}


int has_key(struct char_data * ch, int key)
{
    struct obj_data *o;

    for (o = ch->carrying; o; o = o->next_content)
        if (GET_OBJ_VNUM(o) == key)
            return 1;

    if (GET_EQ(ch, WEAR_HOLD))
        if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
            return 1;

    return 0;
}



#define NEED_OPEN	1
#define NEED_CLOSED	2
#define NEED_UNLOCKED	4
#define NEED_LOCKED	8

char *cmd_door[] =
    {
        "open",
        "close",
        "unlock",
        "lock",
        "pick",
        "bash"
    };

const int flags_door[] =
    {
        NEED_CLOSED | NEED_UNLOCKED,
        NEED_OPEN,
        NEED_CLOSED | NEED_LOCKED,
        NEED_CLOSED | NEED_UNLOCKED,
        NEED_CLOSED | NEED_LOCKED,
        NEED_CLOSED,
    };


#define OPEN_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data * ch, struct obj_data * obj, int door, int scmd)
{
    int other_room = 0;
    struct room_direction_data *back = 0;
    char tempbuf[1000];

    if (DEAD(ch))
        return;
    sprintf(buf, "$n %ss ", cmd_door[scmd]);
    if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
        if ((back = world[other_room].dir_option[rev_dir[door]]))
            if (back->to_room != ch->in_room)
                back = 0;

    switch (scmd) {
    case SCMD_OPEN:
    case SCMD_CLOSE:
        OPEN_DOOR(ch->in_room, obj, door);
        if (back)
            OPEN_DOOR(other_room, obj, rev_dir[door]);
        sprintf(buf1, "You reach out for %s%s and %s it%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
                (EXIT(ch, door)->keyword ? "$F" : "door"), cmd_door[scmd], (obj) ? "" : (scmd==SCMD_CLOSE ? "" : ", making exit to the "), (obj) ? "" : (scmd==SCMD_CLOSE ? "" : dirs[door]));
        //sprintf(tempbuf, "$n opens %s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :(EXIT(ch, door)->keyword ? "$F" : "door"));
        if (!(obj) || (obj->in_room != NOWHERE))
        {

            act(buf1, FALSE, ch, obj, obj ?NULL : EXIT(ch, door)->keyword, TO_CHAR);
            //act(tempbuf, FALSE, ch, obj, obj ? NULL : EXIT(ch, door)->keyword, TO_ROOM);
        }
        //send_to_char(OK, ch);
        break;
    case SCMD_UNLOCK:
    case SCMD_LOCK:
        LOCK_DOOR(ch->in_room, obj, door);
        if (back)
            LOCK_DOOR(other_room, obj, rev_dir[door]);
        send_to_char("*Click*\r\n", ch);
        break;
    case SCMD_PICK:
        LOCK_DOOR(ch->in_room, obj, door);
        if (back)
            LOCK_DOOR(other_room, obj, rev_dir[door]);
        send_to_char("&wThe lock yields to your skills.&0\r\n", ch);
        strcpy(buf, "$n skillfully picks the lock on ");
        improve_skill(ch, SKILL_PICK_LOCK, 1);
        break;
    case SCMD_DOORBASH:
        OPEN_DOOR(ch->in_room, obj, door);
        if (back)
            OPEN_DOOR(other_room, obj, rev_dir[door]);
        //send_to_char("You bash it open with a loud SMACK!\r\n", ch);
        //strcpy(buf, "$n skillfully picks the lock on ");
        break;
    }

    if (scmd!=SCMD_DOORBASH)
    {/* Notify the room */
        sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
                (EXIT(ch, door)->keyword ? "$F" : "door"));
        if (!(obj) || (obj->in_room != NOWHERE))
            act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);
    }
    else
    {
        sprintf(buf, "$n slams into the %s, and it bursts open with a loud *SMACK* !", EXIT(ch, door)->keyword?EXIT(ch, door)->keyword:"door");
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        sprintf(buf, "You slam into the %s, and it bursts open with a loud *SMACK* !\n\r", EXIT(ch, door)->keyword?EXIT(ch, door)->keyword:"door");
        send_to_char(buf, ch);
    }
    if (DEAD(ch))
        return;

    /* Notify the other room */
    if (((scmd == SCMD_OPEN) || (scmd == SCMD_CLOSE)) && (back)) {
        sprintf(buf, "You notice that the %s is %s%s from the other side.\r\n",
                (back->keyword ? fname(back->keyword) : "door"), cmd_door[scmd],
                (scmd == SCMD_CLOSE) ? "d" : "ed");
        send_to_room(buf, EXIT(ch, door)->to_room);
    }
    else if ((scmd == SCMD_DOORBASH) && (back)) {
        sprintf(buf, "With a loud thud, the %s is bashed open from the other side!\r\n",(back->keyword ? fname(back->keyword) : "door"));
        send_to_room(buf, EXIT(ch, door)->to_room);
    }


}


int ok_pick(struct char_data * ch, int keynum, int pickproof, int scmd, int seed, int dir)
{
    int percent;

    my_srandp(keynum);
    percent = number(1, 131);
    my_srandp(rand());
    percent+=number(1, 101);
    percent/=2;
    if (scmd == SCMD_PICK) {
        if (keynum < 0)
            send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
        else if (pickproof)
            send_to_char("You attempt to pick it, to no avail.\r\n", ch);
        else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK))
            send_to_char("You fail to pick the lock.\r\n", ch);
        else
            return (1);
        return (0);
    }
    if (scmd == SCMD_DOORBASH) {
        if (keynum < 0)
            //   send_to_char("Odd - you can't seem to bash it.\r\n", ch);
            percent=number(1, 25);

        {
            if (dir == UP) {
                send_to_char("Are you crazy, you can't door bash UPWARDS!\n\r",ch);
                return 0;
            }
            //if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
            sprintf(buf, "$n charges %swards.", dirs[dir]);
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            sprintf(buf, "You charge %swards.\n\r", dirs[dir]);
            send_to_char(buf, ch);
            if (pickproof || DOOR_IS_BASHPROOF(ch, dir) || percent > GET_SKILL(ch, SKILL_DOORBASH)+number(1, 2*GET_STR(ch))-25)
            {
                sprintf(buf, "You slam against the %s with no effect.\n\r", EXIT(ch, dir)->keyword);
                send_to_char(buf, ch);
                send_to_char("OUCH!  That REALLY Hurt!\n\r", ch);
                sprintf(buf, "$n crashes against the %s with no effect.\n\r", EXIT(ch, dir)->keyword);
                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                if (!IS_IMMORT(ch))
                    GET_HIT(ch)-=GET_MAX_HIT(ch)/number(8,11);
                check_kill(ch, "attempting to bash a door");
                return (0);
            }
            else
                return (1);
        }

    }

    return (1);
}



ACMD(do_gen_door)
{
    int door = -1, keynum, kk;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    skip_spaces(&argument);
    if (!*argument) {
        sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
        send_to_char(CAP(buf), ch);
        return;
    }
    two_arguments(argument, type, dir);

    kk=generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj);
    if (!kk || (GET_OBJ_TYPE(obj)!=ITEM_CONTAINER))
    {
        if (kk)
            obj=NULL;
        door = find_door(ch, type, dir, cmd_door[subcmd]);
    }

    if ((obj) || (door >= 0)) {
        keynum = DOOR_KEY(ch, obj, door);
        if (!(DOOR_IS_OPENABLE(ch, obj, door)))
            act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
        else if (!DOOR_IS_OPEN(ch, obj, door) &&
                 IS_SET(flags_door[subcmd], NEED_OPEN))
            send_to_char("But it's already closed!\r\n", ch);
        else if (!DOOR_IS_CLOSED(ch, obj, door) &&
                 IS_SET(flags_door[subcmd], NEED_CLOSED))
            send_to_char("But it's currently open!\r\n", ch);
        else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
                 IS_SET(flags_door[subcmd], NEED_LOCKED))
            send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
        else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
                 IS_SET(flags_door[subcmd], NEED_UNLOCKED))
            send_to_char("It seems to be locked.\r\n", ch);
        else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_GOD) &&
                 ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
            send_to_char("You don't seem to have the proper key.\r\n", ch);
        else if ((subcmd == SCMD_DOORBASH) && door<0)
            send_to_char("Try bashing doors instead.\r\n", ch);
        else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd, (obj==NULL ? world[ch->in_room].number : GET_OBJ_VNUM(obj)), door))
            do_doorcmd(ch, obj, door, subcmd);
    }
    return;
}



ACMD(do_enter)
{
    int door;
    struct obj_data *obj = NULL;


    one_argument(argument, buf);

    if (*buf) {			/* an argument was supplied, search for
           door keyword */

        if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
            if (CAN_SEE_OBJ(ch, obj)) {
                if (GET_OBJ_TYPE(obj) == ITEM_PORTAL) {
                    if (GET_OBJ_VAL(obj, 0) != NOWHERE) {
                        send_to_char("You enter the portal.\r\n\r\n", ch);
                        act("$n enters the portal.", FALSE, ch, 0 , 0, TO_ROOM);
                        char_from_room(ch);
                        char_to_room(ch, GET_OBJ_VAL(obj, 0));
                        act("$n steps out of the portal.", FALSE, ch, 0 , 0, TO_ROOM);
                    } else if (real_room(GET_OBJ_VAL(obj, 1)) != NOWHERE) {
                        char_from_room(ch);
                        char_to_room(ch, real_room(GET_OBJ_VAL(obj, 1)));
                    }
                    look_at_room(ch, 1);
                    return;
                }
            }
        }

        for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXIT(ch, door))
                if (EXIT(ch, door)->keyword)
                    if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
                        perform_move(ch, door, 1);
                        return;
                    }
        sprintf(buf2, "There is no %s here.\r\n", buf);
        send_to_char(buf2, ch);
    } else if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
        send_to_char("You are already indoors.\r\n", ch);
    else {
        /* try to locate an entrance */
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXIT(ch, door))
                if (EXIT(ch, door)->to_room != NOWHERE)
                    if ((!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) || (AFF_FLAGGED(ch, AFF_PASSDOOR) && !DOOR_IS_NOPASSDOOR(ch, door))) &&
                            IS_SET(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
                        perform_move(ch, door, 1);
                        return;
                    }
        send_to_char("You can't seem to find anything to enter.\r\n", ch);
    }
}


ACMD(do_leave)
{
    int door;

    if (!IS_SET(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
        send_to_char("You are outside.. where do you want to go?\r\n", ch);
    else {
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXIT(ch, door))
                if (EXIT(ch, door)->to_room != NOWHERE)
                    if ((!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) || (AFF_FLAGGED(ch, AFF_PASSDOOR) && !DOOR_IS_NOPASSDOOR(ch, door))) &&
                            !IS_SET(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
                        perform_move(ch, door, 1);
                        return;
                    }
        send_to_char("I see no obvious exits to the outside.\r\n", ch);
    }
}


ACMD(do_stand)
{
    switch (GET_POS(ch)) {
    case POS_STANDING:
        act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
        break;
    case POS_SITTING:
        // if (!IS_NPC(ch) && FIGHTING(ch) && CHECK_WAIT(ch)<=0)
        //  return;
        act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = FIGHTING(ch) ? POS_FIGHTING : POS_STANDING;
        break;
    case POS_RESTING:
        act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_STANDING;
        break;
    case POS_SLEEPING:
        act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
        break;
    case POS_FIGHTING:
        act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
        break;
    default:
        act("You stop floating around, and put your feet on the ground.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and puts $s feet on the ground.",
            TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_STANDING;
        break;
    }
    ch->sleep_timer=0;
}


ACMD(do_sit)
{
    switch (GET_POS(ch)) {
    case POS_STANDING:
        act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        break;
    case POS_SITTING:
        send_to_char("You're sitting already.\r\n", ch);
        break;
    case POS_RESTING:
        act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        break;
    case POS_SLEEPING:
        act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
        break;
    case POS_FIGHTING:
        act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
        break;
    default:
        act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        break;
    }
    rprog_rest_trigger( ch );
}
char *restmsgs[][2] = {
                          {"You rest for a while.\r\n",
                           "$n sits down and rests."},
                          {"It's too noisy and disturbing around here to rest.\r\n",
                           NULL},
                          {"You lie down and rest.\r\n",
                           "$n lies down and rest."},
                          {"You curl down underneath a nearby tree and rest.\r\n",
                           "$n curls down underneath a nearby tree and rests."},
                          {"You rest yourself.\r\n",
                           "$n sits down and rests."},
                          {"You sit on a rock and rest your tired bones.\r\n",
                           "$n rests on a rock."},
                          {"You can't rest in the water!\r\n",
                           NULL},
                          {"You can't rest in the water!\r\n",
                           NULL},
                          {"You can't rest in the water!\r\n",
                           NULL},
                          {"You can't rest in the air!\r\n",
                           NULL},
                          /* always keep this as the last message */
                          {"If you see this please tell a god.\r\n", NULL}
                      };


ACMD(do_rest)
{

    switch (GET_POS(ch)) {
    case POS_STANDING:
    case POS_SITTING:
        /*	if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
        	(world[IN_ROOM(ch)].sector_type == SECT_CITY) ||
        	(world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
        	(world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
        	(world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {	
        	if (restmsgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
        	    send_to_char(restmsgs[world[IN_ROOM(ch)].sector_type][0], ch);
        	if (restmsgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
        	    act(restmsgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL, TO_ROOM);
        	return;
                }
                if (restmsgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
        	send_to_char(restmsgs[world[IN_ROOM(ch)].sector_type][0], ch);
                if (restmsgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
        	act(restmsgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL, TO_ROOM);
          */    
        send_to_char("You rest your tired bones.\r\n", ch);
        act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_RESTING;
        ch->sleep_timer=0;
        break;

    case POS_RESTING:
        act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
        break;
    case POS_SLEEPING:
        act("You are sleeping now!", FALSE, ch, 0, 0, TO_CHAR);
        break;
    case POS_FIGHTING:
        act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
        break;
    default:
        act("You stop floating around, and stop to rest your tired bones.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_RESTING;
        break;
    }
    rprog_rest_trigger( ch );

}


ACMD(do_sleep)
{
    /*if (GET_POS(ch)==POS_SLEEPING)
    	send_to_char("You are already sound asleep!\r\n", ch);
    else if (GET_POS(ch)==POS_RESTING)
    send_to_char("Wait and you will eventually fall into sleep.\r\n",ch);
    else
     send_to_char("You have to 'rest' first.\r\n", ch);
     return;*/

    switch (GET_POS(ch)) {
    case POS_STANDING:
    case POS_SITTING:
    case POS_RESTING:
        send_to_char("You go to sleep.\r\n", ch);
        act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);

        GET_POS(ch) = POS_SLEEPING;
        break;
    case POS_SLEEPING:
        send_to_char("You are already sound asleep.\r\n", ch);
        break;
    case POS_FIGHTING:
        send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
        break;
    default:
        act("You stop floating around, and lie down to sleep.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and lie down to sleep.",
            TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SLEEPING;
        break;
    }
    rprog_sleep_trigger( ch );
}


ACMD(do_wake)
{
    struct char_data *vict;
    int self = 0;

    one_argument(argument, arg);
    if (*arg) {
        if (GET_POS(ch) == POS_SLEEPING)
            send_to_char("You can't wake people up if you're asleep yourself!\r\n", ch);
        else if ((vict = get_char_room_vis(ch, arg)) == NULL)
            send_to_char(NOPERSON, ch);
        else if (vict == ch)
            self = 1;
        else if (GET_POS(vict) > POS_SLEEPING)
            act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
        else if (GET_POS(vict) < POS_STUNNED)
            act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
        else if (GET_POS(vict) != POS_SLEEPING)
            act("$E is not sleeping.", FALSE, ch, 0, vict, TO_CHAR);
        else if (IS_AFFECTED(vict, AFF_SLEEP))
            act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
        else {
            act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
            act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
            GET_POS(vict) = POS_STANDING;
            look_at_room(vict, 0);
        }
        if (!self)
            return;
    }
    if (IS_AFFECTED(ch, AFF_SLEEP))
        send_to_char("You can't wake up!\r\n", ch);
    else if (GET_POS(ch) > POS_SLEEPING)
        send_to_char("You are already awake...\r\n", ch);
    else if (GET_POS(ch) == POS_SLEEPING) {
        //if (!IS_NPC(ch) && IS_GOD(ch))
        //	send_to_char("You'll wake up when you are fully rested.\r\n", ch);
        //else
        {
            act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_STANDING;
            look_at_room(ch, 0);
        }
    }
}


ACMD(do_follow)
{
    struct char_data *leader;

    void stop_follower(struct char_data * ch);
    void add_follower(struct char_data * ch, struct char_data * leader);

    one_argument(argument, buf);

    if (*buf) {
        if (!(leader = get_char_room_vis(ch, buf))) {
            send_to_char(NOPERSON, ch);
            return;
        }
    } else {
        send_to_char("Whom do you wish to follow?\r\n", ch);
        return;
    }

    if (ch->master == leader) {
        act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
        return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
        act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
    } else {			/* Not Charmed follow person */
        if (leader == ch) {
            if (!ch->master) {
                send_to_char("You are already following yourself.\r\n", ch);
                return;
            }
            stop_follower(ch);
        } else {
            if (circle_follow(ch, leader)) {
                act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
                return;
            }
            if (FOL_SKIG(ch))
            {
            	if (!number(0, 2))
            	{    
    			if (GET_EQ(ch, WEAR_WIELD) && GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) == TYPE_PIERCE - TYPE_HIT)
            			do_backstab(ch, buf, 0, 0);
            		else 
            			do_hit(ch, buf, 0 , 0);  
            		return;
            	}
            }
            
            if (ch->master)
                stop_follower(ch);
            add_follower(ch, leader);
        }
    }
}

ACMD(do_stalk)
{
    struct char_data *leader;

    void stop_follower(struct char_data * ch);
    void add_follower(struct char_data * ch, struct char_data * leader);

    one_argument(argument, buf);

    if (*buf) {
        if (!(leader = get_char_room_vis(ch, buf))) {
            send_to_char(NOPERSON, ch);
            return;
        }
    } else {
        send_to_char("Whom do you wish to stalk?\r\n", ch);
        return;
    }

    if (number(1,101)>GET_SKILL(ch, SKILL_STALK))
    {
        act("You fail and now prettend you were just looking at $M.", FALSE, ch, NULL, leader, TO_CHAR);
        act("$n looks at you.", FALSE, ch, NULL, leader, TO_VICT);
        return;
    }
    if (ch->master == leader) {
        if (AFF2_FLAGGED(ch, AFF2_STALK))
            act("You are already stalking $M.", FALSE, ch, 0, leader, TO_CHAR);
        else
        {
            act("You now stalk $M.", FALSE, ch, 0, leader, TO_CHAR);
            SET_BIT(AFF2_FLAGS(ch), AFF2_STALK);
        }
        return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
        act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
    } else {			/* Not Charmed follow person */
        if (leader == ch) {
            if (!AFF2_FLAGGED(ch, AFF2_STALK))
                send_to_char("You can't do that.\r\n", ch);
            else
                if (ch->master) stop_follower(ch);
            REMOVE_BIT(AFF2_FLAGS(ch), AFF2_STALK);
            return;
        } else {
            if (circle_follow(ch, leader)) {
                act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
                return;
            }
            if (ch->master)
                stop_follower(ch);
            SET_BIT(AFF2_FLAGS(ch), AFF2_STALK);
            add_follower(ch, leader);
        }
    }
}
