/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "fight.h"
#include "clan.h"
#include "events.h"
/* external structs */
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
extern int total_ram;
void            killfire(struct char_data * ch);
void            killcold(struct char_data * ch);
void            killacid(struct char_data * ch);

extern void     specext(struct char_data * ch, struct char_data * victim);
extern void     specwarm(struct char_data * ch, struct char_data * victim);
extern void     specclear(struct char_data * ch, struct char_data * victim);
extern void     specstruggle(struct char_data * ch, struct char_data * victim);

void            hunt_victim(struct char_data * ch);

void            mprog_random_trigger(struct char_data * mob);
void            mprog_wordlist_check(char *arg, struct char_data * mob, struct char_data * actor,
                                     struct obj_data * obj, void *vo, int type);
extern int      is_empty(int zone_nr);


void            mobile_activity(void)
{
    register struct char_data *ch, *next_ch,
                *vict,
                *i;

    struct obj_data *obj,
                *best_obj;
    int             door,
    found,
    max,
    dam,
    ie;
    memory_rec     *names;
    struct affected_type af;

    extern int      no_specials;

    ACMD(do_get);
    ACMD(do_wear);
    ACMD(do_wield);
    ACMD(do_say);

for (ch = character_list; ch; ch = next_ch) {
        next_ch = ch->next;
        if (ch->in_room==NOWHERE)
            continue;
        SANITY_CHECK(ch);
        i = ch;
        if (GET_POS(ch)==POS_FIGHTING && !FIGHTING(ch))
        {
            logs("%s is fighting NULL.", GET_NAME(ch));
            GET_POS(ch)=POS_STANDING;
        }



        if (AFF3_FLAGGED(ch, AFF3_PLAGUE))
            for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
                if (vict==ch || AFF3_FLAGGED(ch, AFF3_PLAGUE) || (!IS_NPC(vict) && GET_CLAN(vict)==CLAN_NEWBIE) || number(1, 70)<GET_LEVEL(vict) || !number(0, 2))
                    continue;
                act("You catch a PLAGUE from $n!!", FALSE, ch, 0, vict, TO_VICT);
                act("$N catches a PLAGUE from $n!!", FALSE, ch, 0, vict, TO_NOTVICT);
                act("$N catches a PLAGUE from you!!", FALSE, ch, 0, vict, TO_CHAR);
                af.type = SPELL_PLAGUE;
                af.duration=-1;
                af.bitvector = 0;
                af.bitvector2 = 0;
                af.bitvector3 = AFF3_PLAGUE;
                af.modifier = 0;
                af.location = APPLY_NONE;
                affect_join(vict, &af, FALSE, FALSE, FALSE, FALSE);
            }


        if (AFF2_FLAGGED(i, AFF2_BURNING)) {
            if (SECT(i->in_room) >=SECT_WATER_SWIM && SECT(i->in_room) <=SECT_UNDERWATER ) {
                send_to_char("You are no longer burning.\r\n", i);
                act("$n is no longer burning.", FALSE, i, NULL, NULL, TO_ROOM);
                killfire(i);
            } else     if (!IS_GOD(i)){
                send_to_char("&cYou are BURNING!&0\r\n", i);
                dam =number(GET_MAX_HIT(i)/30, GET_MAX_HIT(i)/20);
                if (AFF_FLAGGED(i, AFF_PROT_FIRE))
                    dam = dam / 4;
                if (GET_RACE(i)==RACE_TROLL)
                    dam*=2;
                if (GET_POS(ch)==POS_SLEEPING)
                    dam*=3;

                    GET_HIT(i) -= dam;
                if (GET_POS(ch)==POS_SLEEPING)
                    raw_awake(ch);
            }
            GET_HIT(ch)=MAX(GET_HIT(ch), -GET_CON(ch)/2+1);
            if (check_kill(ch, "burning to death"))
                continue;
            if (IS_NPC(i) && AWAKE(i))
                specext(i, i);
        }
        if (AFF2_FLAGGED(i, AFF2_ACIDED)) {
            if (SECT(i->in_room) == SECT_UNDERWATER) {
                send_to_char("You feel acid clear of your body.\r\n", i);
                act("$n is no longer covered in acid.", FALSE, i, NULL, NULL, TO_ROOM);
                killacid(i);
            } else                 if (!IS_GOD(i)){
                send_to_char("&cYou are DISSOLVING!&0\r\n", i);
               dam =number(GET_MAX_HIT(i)/25, GET_MAX_HIT(i)/15);
                if (AFF2_FLAGGED(i, AFF2_STONESKIN))
                    dam = dam / 2;
                if (GET_POS(ch)==POS_SLEEPING)
                    dam*=3;
                
                    GET_HIT(i) -= dam;
                if (GET_POS(ch)==POS_SLEEPING)
                    raw_awake(ch);
            }
              GET_HIT(ch)=MAX(GET_HIT(ch), -GET_CON(ch)/2+1);
            if (check_kill(ch, "acid"))
                continue;
            if (IS_NPC(i) && AWAKE(i))
                specclear(i, i);

        }
        if (AFF2_FLAGGED(i, AFF2_FREEZING)) {
            if (SECT(i->in_room) == SECT_LAVA) {
                send_to_char("You are no longer freezing.\r\n", i);
                act("$n is no longer freezing.", FALSE, i, NULL, NULL, TO_ROOM);
                killcold(i);
            } else                 if (!IS_GOD(i)){
                send_to_char("&cYou are FREEZING!&0\r\n", i);
                dam =number(GET_MAX_HIT(i)/30, GET_MAX_HIT(i)/25);
                if (AFF2_FLAGGED(i, AFF2_PROT_COLD))
                    dam = dam / 2;
                if (GET_POS(ch)==POS_SLEEPING)
                    dam*=3;
                if (!IS_GOD(i))
                    GET_HIT(i) -= dam;
                if (GET_POS(ch)==POS_SLEEPING)
                    raw_awake(ch);
            }
            
              GET_HIT(ch)=MAX(GET_HIT(ch), -GET_CON(ch)/2+1);
            if (check_kill(ch, "freezing to death"))
                continue;
            if (IS_NPC(i) && AWAKE(i))
                specwarm(i, i);
        }
        if (AFF3_FLAGGED(i, AFF3_CHOKE)) {
            if (number(1, 200)<GET_WIS(i)) {
                send_to_char("You are relieved as you regain your breath.\r\n", i);
                act("$n regains $s breath.", FALSE, i, NULL, NULL, TO_ROOM);
                killchoke(i);
            } else                 if (!IS_GOD(i)){
                send_to_char("&RYou are CHOKING!&0\r\n", i);
                act("$n is choking!", FALSE, i, 0, 0, TO_ROOM);
                dam =number(GET_MAX_HIT(i)/20, GET_MAX_HIT(i)/15);

                if (GET_POS(ch)==POS_SLEEPING)
                    dam*=3;
                if (!IS_GOD(i))
                    GET_HIT(i) -= dam;
                if (GET_POS(ch)==POS_SLEEPING)
                    raw_awake(ch);
            }                                
              GET_HIT(ch)=MAX(GET_HIT(ch), -GET_CON(ch)/2+1);
            if (check_kill(ch, "choking"))
                continue;
        }
        if (!IS_NPC(i) && (SECT(i->in_room) == SECT_UNDERWATER) &&
                !AFF_FLAGGED(i, AFF_WATERBREATH)) {
            send_to_char("You're gasping for air!!\r\n", i);
            if (!IS_GOD(i))
                GET_HIT(i) -= number(GET_LEVEL(ch), 2*GET_LEVEL(ch));
            if (GET_POS(ch)==POS_SLEEPING)
                raw_awake(ch);
            if (check_kill(ch, "drowning"))
                continue;
        }

        if (!IS_MOB(ch))/*// || FIGHTING(ch) || !AWAKE(ch) || AFF_FLAGGED(ch, AFF_CHARM))*/
            continue;

        
        if (AFF3_FLAGGED(i, AFF3_WEB) && AWAKE(i))
            specstruggle(i, i);
        /* Examine call for special procedure */
        //CREF(ch, CHAR_NULL);
        


        if (ch->in_room!=NOWHERE && MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) {
            if (mob_index[GET_MOB_RNUM(ch)].func == NULL) {
                sprintf(buf, "%s (#%d): Attempting to call non-existing mob func",
                        GET_NAME(ch), GET_MOB_VNUM(ch));
                log(buf);
                REMOVE_BIT(MOB_FLAGS(ch), MOB_SPEC);
            } else {
                if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
                    continue;   /* go to next char */
            }
        }


        if (FIGHTING(ch) || !AWAKE(ch) || DEAD(ch) )
            continue;

        /* Scavenger (picking up objects) */
        if (MOB_FLAGGED(ch, MOB_SCAVENGER)) {
            if (world[ch->in_room].contents && !number(0, 7)) {
                max = 1;
                best_obj = NULL;
                for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
                {
                    if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max) {
                        best_obj = obj;
                        max = GET_OBJ_COST(obj);
                    }
                    //	if (GET_OBJ_RENT(obj)==9999)
                    //		do_get(ch,"all corpse",0,0);


                }
                if (best_obj != NULL) {
                    obj_from_room(best_obj);
                    obj_to_char(best_obj, ch);
                    do_say(ch, "Ummm.. This is nice.", 0, 0);
                    act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
                }
                if (!number(0,2))
                    wear_all_suitable(ch);
            }

        }

        if (DEAD(ch))
            continue;

        if (!(ie=is_empty(world[ch->in_room].zone)))
        {

            if ( HAS_MOB_PROG( ch, SCRIPT_PROG ) )
            {
                mprog_script_trigger( ch );
                continue;
            }

            mprog_random_trigger(ch);

             if (DEAD(ch) || AFF_FLAGGED(ch, AFF_CHARM))
                continue;
                
            if (IS_NPC(ch) && !FIGHTING(ch))
                if ((GET_POS(ch)==POS_RESTING || GET_POS(ch)==POS_SITTING) && GET_DEFAULT_POS(ch)==POS_STANDING)
                    do_stand(ch, "", 0,0);

           

            /* Mob Movement */
            if (!MOB_FLAGGED(ch, MOB_SENTINEL) && GET_POS(ch) == POS_STANDING && (!AFF_FLAGGED(ch, AFF_HOLDED)) &&
                    (!AFF2_FLAGGED(ch, AFF2_PETRIFY)) &&
                    ((door = number(0, 18)) < NUM_OF_DIRS) && CAN_GO(ch, door) &&
                    !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB | ROOM_DEATH) &&
                    (!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
                     (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone))) {
                perform_move(ch, door, 1);
            }
            if (DEAD(ch))
                continue;


            /* Helper Mobs */
            if (MOB_FLAGGED(ch, MOB_HELPER)) {
                found = FALSE;
                for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
                    if (ch != vict && IS_NPC(vict) && FIGHTING(vict) &&
                            !IS_NPC(FIGHTING(vict)) && ch != FIGHTING(vict) && CAN_SEE(ch, vict)) {
                        act("$n jumps to the aid of $N!", FALSE, ch, 0, vict, TO_ROOM);
                        hit(ch, FIGHTING(vict), TYPE_HIT);
                        found = TRUE;
                    }
                }
            }

        }
        if (DEAD(ch) || AFF_FLAGGED(ch, AFF_CHARM))
            continue;


        //if (!ch)
        //	goto mobact_kraj;

        /* MOB Prog foo */
        /* if (IS_NPC(ch) && ch->mpactnum > 0) {
         	
             MPROG_ACT_LIST *tmp_act,
                            *tmp2_act;
             
             for (tmp_act = ch->mpact; tmp_act != NULL; tmp_act = tmp_act->next) {
                 if (!ie) mprog_wordlist_check(tmp_act->buf, ch, tmp_act->ch,
                                      tmp_act->obj, tmp_act->vo, ACT_PROG);
                 //total_ram-=strlen(tmp_act->buf);                
                 DISPOSE(tmp_act->buf);
             }
             for (tmp_act = ch->mpact; tmp_act != NULL; tmp_act = tmp2_act) {
                 tmp2_act = tmp_act->next;                     
                 DISPOSE(tmp_act);
             }
             ch->mpactnum = 0;
             ch->mpact = NULL;
         }
         */


        // mob aggro && memory stuff moved to char_to_room(), handler.c

        /* Hunting Mobs */
        if (HUNTING(ch) && !GET_UTIL_EVENT(ch) && ch->in_room!=((HUNTING(ch))->in_room) && GET_POS(ch)==POS_STANDING)
        {
            struct hunt_eo *hevent;
            CREATE(hevent, struct hunt_eo, 1);
            hevent->ch=ch;
            GET_UTIL_EVENT(ch)=event_create(event_hunt_victim, hevent, 5);    // 5 = 1 sec
           
        }



        /* Add new mobile actions here */

        //mobact_kraj:
        //CUREF(ch);

    }                           /* end for() */
}



/* Mob Memory Routines */

/* make ch remember victim */
void            remember(struct char_data * ch, struct char_data * victim)
{
    memory_rec     *tmp;
    bool            present = FALSE;

    if (!IS_NPC(ch) || IS_NPC(victim))
        return;

    for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
        if (tmp->id == GET_IDNUM(victim))
            present = TRUE;

    if (!present) {
        CREATE(tmp, memory_rec, 1);
        tmp->next = MEMORY(ch);
        tmp->id = GET_IDNUM(victim);
        MEMORY(ch) = tmp;
    }
}


/* make ch forget victim */
void            forget(struct char_data * ch, struct char_data * victim)
{
    memory_rec     *curr,
    *prev = NULL;

    if (!(curr = MEMORY(ch)))
        return;

    while (curr && curr->id != GET_IDNUM(victim)) {
        prev = curr;
        curr = curr->next;
    }

    if (!curr)
        return;                 /* person wasn't there at all. */

    if (curr == MEMORY(ch))
        MEMORY(ch) = curr->next;
    else
        prev->next = curr->next;

    DISPOSE(curr);
}


/* erase ch's memory */
void            clearMemory(struct char_data * ch)
{
    memory_rec     *curr,
    *next;

    curr = MEMORY(ch);

    while (curr) {
        next = curr->next;
        DISPOSE(curr);
        curr = next;
    }

    MEMORY(ch) = NULL;
}
