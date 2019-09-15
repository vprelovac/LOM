/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "newquest.h"
/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int pk_allowed;
extern char *spells[];
extern struct dex_app_type dex_app[];
/* extern functions */
void check_killer(struct char_data * ch, struct char_data * victim);
void raw_kill(struct char_data * ch, struct char_data * killer);
bool CAN_MURDER(struct char_data * ch, struct char_data * victim);
void killfire(struct char_data * ch);
void killcold(struct char_data * ch);
void killacid(struct char_data * ch);
extern int not_in_arena(struct char_data *ch);
extern struct sclass class_app[];
extern int      top_of_world;   /* In db.c */
char skillbuf[MAX_STRING_LENGTH];

void improve_skill(struct char_data * ch, int skill, int mod)
{
    int percent = GET_SKILL(ch, skill), j;

    //int newpercent=0;
    if (!ch || DEAD(ch))
        return;
    if (IS_NPC(ch))
        return;
    if (ch->last_skill==skill && GET_LEVEL(ch)>2)
        return;

    if (!GET_SKILL(ch, skill))
        return;				// can not improve something you dont know right ?

    /*
        if (number(1, mod * 10000) > GET_WIS(ch)) 
        {
     	for (j = 0; j < NUM_CLASSES; j++) 
    	    if (IS_SET(GET_CLASS(ch), (1 << j)) && (GET_LEVEL(ch) >= spell_info[skill].min_level[j])) 
    	    {
    	            (GET_LEVEL(ch) >= spell_info[skill].min_level[j])
    	      
    	    }
        }          */
    if (percent >=110)
        return;
    if (percent > 99)
        mod*=10+(percent-100)*3;
    if (FOL_AZ(ch))
    	mod=MAX(1, mod-MAX(1, 25*mod/100));    
    if (IS_SPELL(skill))
    {
        if (number(1, 500+(mod * MAX(3500, GET_SKILL(ch, skill)*100))/4) > GET_INT(ch)*GET_WIS(ch))
            return;
    }
    else
        if (number(1, 500+mod * MAX(3500, 5*GET_SKILL(ch, skill)*GET_SKILL(ch, skill)/4)) > GET_INT(ch)*GET_WIS(ch))
            return;



    percent++;
    //newpercent=1;
    //percent = MIN(98, percent);
    if (number(1, 32)<GET_INT(ch))
        percent++;
        
    
    percent=MIN(110, percent);
    SET_SKILL(ch, skill, percent);
    ch->last_skill=skill;
    //if (newpercent >= 0) {
    if (percent>100)
        sprintf(skillbuf, "\r\n&C* You feel your %s improving beyond imaginable *&0\r\r\n\n", spells[skill]);
    else
        sprintf(skillbuf, "\r\n&C* You feel your %s improving! *&0\r\r\n\n", spells[skill]);
    send_to_char(skillbuf, ch);
    
    
    if (QUESTING(ch) && current_quest!=QUEST_NONE && quest_step==QUEST_ACTION)
    {
        if (current_quest==QUEST_RUSH)
        {   
            percent=MIN(110, percent+10);
            SET_SKILL(ch, skill, percent);            
            sprintf(buf, "we have a winner - %s!", GET_NAME(ch));
            end_current_quest(buf);
        }
    }
        
    
        // }
}



void improve_combat_skill(struct char_data * ch, int skill)
{
    int percent = GET_SKILL(ch, skill);
    int p;
    int diff;

    //int newpercent=0;


    if (!ch || DEAD(ch))
        return;
    if (IS_NPC(ch))
        return;
    if (percent > 99)
        return;

    /*if (ch->last_skill==skill && GET_LEVEL(ch)>2)
       	return;*/                                  
    if (percent>=2*GET_LEVEL(ch)+10)
        return; 	  
        
//    if (!FOL_MUGRAK(ch) && FIGHTING(ch) && (  (100*GET_HIT(ch)/GET_MAX_HIT(ch))>  (!IS_NPC(FIGHTING(ch))? 70:95)  ))
  //  	return;	
  
  
    //if (FIGHTING(ch) && (GET_SKILL(ch, skill)>(GET_SKILL(FIGHTING(ch)+10))))
    	//return;

    //diff=FIGHTING(ch)? GET_LEVEL(ch)-GET_LEVEL(FIGHTING(ch)):0;
    diff=FIGHTING(ch)? (GET_SKILL(ch, skill)-GET_SKILL(FIGHTING(ch), skill)):0;
    diff=MIN(5, MAX(-5, diff/2));
    
    if ((p=number(1, MAX(24000, MAX(30000+diff*1100,diff*1100+GET_SKILL(ch, skill)*800)))) > GET_INT(ch)*GET_WIS(ch)+FOL_URG(ch)?200:0)
    {   
    	
        //    	ch_printf(ch, "number=%d skill=%d\n", p, GET_INTR(ch)*GET_WISR(ch));
        if ((skill==SKILL_PARRY || skill==SKILL_DODGE || skill==SKILL_SHIELD) && CHANCE(75))
        {
        	if ((p=number(1, MAX(24000, MAX(30000+diff*1100,diff*1100+GET_SKILL(ch, skill)*800)))) > GET_INT(ch)*GET_WIS(ch)+FOL_URG(ch)?200:0)
        		return;
	}
        else 
        	return;
    }
    
    

    //if (percent >= (class_app[(GET_CLASS_NUM(ch))].maxcombat))//*GET_LEVEL(ch)/50)
    //return;


    percent++;
    //if (number(1, 60)<GET_INT(ch)+GET_WIS(ch))
    //percent++;
   // ch->last_skill=skill;
    //newpercent=1;
    //  percent = MIN(97, percent);
    SET_SKILL(ch, skill, percent);
    //if (newpercent >= 0) {
    sprintf(skillbuf, "\r\n&C* You feel your %s improving! *&0\r\r\n\n", spells[skill]);
    send_to_char(skillbuf, ch);
    
    
    if (QUESTING(ch) && current_quest!=QUEST_NONE && quest_step==QUEST_ACTION)
    {
        if (current_quest==QUEST_RUSH)
        {   
            percent=MIN(100, percent+4);
            SET_SKILL(ch, skill, percent);            
            sprintf(buf, "we have a winner - %s!", GET_NAME(ch));
            end_current_quest(buf);
        }
    }
    
    
    //}
}




struct combatstand_eo
{
    struct char_data *ch;
};

EVENTFUNC(event_combat_stand)
{
    struct combatstand_eo *cevent=(struct combatstand_eo *) event_obj;
    struct char_data *ch;
    ch=cevent->ch;

    if (!ch || ch->in_room<1 || GET_POS(ch)!=POS_SITTING)
        goto kraj_combat_stand;

    if (FIGHTING(ch))
        GET_POS(ch) = POS_FIGHTING;
    else
        GET_POS(ch) = POS_STANDING;
    if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2))
    {
        act("$n scrambles to $s feet!", 1, ch, 0, 0, TO_ROOM);
        send_to_char("&wYou clamber to your feet.&0\r\n", ch);
    }
    else if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 4))
    {
        act("$n scrambles to $s feet!", 1, ch, 0, 0, TO_ROOM);
        send_to_char("&wYou clamber to your feet.&0\r\n", ch);
    }
    else
    {
        act("$n barely manages to get on $s feet again.", 1, ch, 0, 0,TO_ROOM);
        send_to_char("&wYou barely manage to clamber back to your feet.&0\r\n", ch);
    }

kraj_combat_stand:
    if (cevent)
        free (cevent);
    if (ch)
        GET_UTIL_EVENT(ch)=0;
    return 0;
}


void assign_stand_event(struct char_data *ch, int when)
{
    struct combatstand_eo *cevent;

    CREATE(cevent, struct combatstand_eo, 1);
    cevent->ch=ch;
    when+=(17-GET_DEX(ch));
    when =MAX(6, when);
    if (GET_UTIL_EVENT(ch))
    {
        send_to_char("You are interrupted!\r\n", ch);
        event_cancel(GET_UTIL_EVENT(ch));
    }
    GET_UTIL_EVENT(ch)=event_create(event_combat_stand, cevent, when);
}



ACMD(do_assist)
{
    struct char_data *helpee, *opponent;

    if (FIGHTING(ch)) {
        send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Whom do you wish to assist?\r\n", ch);
    else if (!(helpee = get_char_room_vis(ch, arg)))
        send_to_char(NOPERSON, ch);
    else if (helpee == ch)
        send_to_char("You can't help yourself any more than this!\r\n", ch);
    else {
        opponent = FIGHTING(helpee);
        /*    for (opponent = world[ch->in_room].people;
        	 opponent && (FIGHTING(opponent) != helpee);
        	 opponent = opponent->next_in_room)
        		;*/
        if (!opponent)
            act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
        else if (!CAN_SEE(ch, opponent))
            act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
        else if (!pk_allowed && !IS_NPC(opponent) && !CAN_MURDER(ch, opponent))	/* prevent accidental
            									   pkill */
            act("Use 'murder' if you really want to attack $N.", FALSE,
                ch, 0, opponent, TO_CHAR);
        else {
            send_to_char("&BYou join the fight!&0\r\n", ch);
            act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
            act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
            //hit(ch, opponent, TYPE_UNDEFINED);
            check_fight(ch, opponent);
            
        }
    }
}

ACMD(do_assist);
ACMD(do_hit)
{
    struct char_data *vict;
    struct follow_type *k;


    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Hit who?\r\n", ch);
    else if (!(vict = get_char_room_vis(ch, arg)))
        send_to_char("They don't seem to be here.\r\n", ch);
    else if (vict == ch) {
        send_to_char("You hit yourself...OUCH!.\r\n", ch);
        act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
    } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
        act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
    else {
        if (!CAN_MURDER(ch,vict))
        {
            /*if (GET_LEVEL(vict)<7)
            send_to_char("That person is still a newbie. And you can't slaughter newbies.\r\n",ch);
            else if (GET_LEVEL(ch)<7)
            send_to_char("You are still a newbie. Wait until you reach level 7.\r\n",ch);*/
            send_to_char("You can not attack that person.\r\n", ch);
            return;
        }
        if (!ROOM_FLAGGED(ch->in_room, ROOM_ARENA) || (!IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict)))
            if (!pk_allowed) {
                if (!IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict) && (subcmd != SCMD_MURDER)) {
                    send_to_char("Use 'murder' to hit another person. Be forewarned, you will get a killer flag.\r\n", ch);
                    return;
                }
                if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict) && !CAN_MURDER(ch->master, vict)) {
                    send_to_char("You wimp...\r\n", ch->master);
                    return;	/* you can't order a charmed pet to attack
                     a player */
                }
            }
        if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
            send_to_char("This room has a nice peaceful feeling.\r\n", ch);
            return;
        }
        /*if (SCMD_MURDER) {
            check_killer(ch, vict);
    } */
        if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
            sprintf(buf2, "%s and %s engage in mortal combat!", GET_NAME(ch),
                    GET_NAME(vict));
            sportschan(buf2);
        }
        /*if (GET_LEVEL(ch) < 4 && (GET_LEVEL(vict) > 6)) {
        	send_to_char("You change your mind in the last moment, as you estimate that action suicidal.\r\n", ch);
        	return;
    } */
        if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {

            if (subcmd!=SCMD_CHARGE)
                if (!IS_NPC(ch) && (strcmp("off", ch->player_specials->saved.enrage))) {
                    sprintf(buf, "&G$n screams '%s' and attacks!&0", ch->player_specials->saved.enrage);
                    act(buf, FALSE, ch, 0, 0, TO_ROOM);
                    sprintf(buf, "&GYou scream '%s'&0", ch->player_specials->saved.enrage);
                    act(buf, FALSE, ch, 0, 0, TO_CHAR);
                }
            if (subcmd==SCMD_CHARGE)
            {
                int attacks=1,dual_wield_flag=0;
                if (!GET_SKILL(ch, SKILL_CHARGE))
                {
                    send_to_char("Better leave charging to true warriors. Try 'kill' instead.\r\n", ch);
                    return;
                }
                if (number(1, 111)>GET_SKILL(ch, SKILL_CHARGE))
                {
                    send_to_char("You fail your charge.\r\n", ch);
                    //hit(vict, ch, TYPE_UNDEFINED);
                    check_fight(vict, ch);
                    return;
                }
                if (!IS_NPC(ch) && (strcmp("off", ch->player_specials->saved.enrage))) {
                    sprintf(buf, "&G$n screams '%s' and CHARGES $N!!&0", ch->player_specials->saved.enrage);
                    act(buf, FALSE, ch, 0, vict, TO_NOTVICT);
                    sprintf(buf, "&G$n screams '%s' and CHARGES you!!&0", ch->player_specials->saved.enrage);
                    act(buf, FALSE, ch, 0, vict, TO_VICT);
                    sprintf(buf, "&GYou scream '%s' and CHARGE $N!!&0", ch->player_specials->saved.enrage);
                    act(buf, FALSE, ch, 0, vict, TO_CHAR);
                }
                else  if (!IS_NPC(ch))
                {

                    act("&G$n CHARGES towards $N!!&0", FALSE, ch, 0, vict, TO_NOTVICT);
                    act("&G$n CHARGES towards you!!&0", FALSE, ch, 0, vict, TO_VICT);
                    act("&GYou scream '%s' and CHARGE towards $N!!&0", FALSE, ch, 0, vict, TO_CHAR);
                }
                improve_skill(ch, SKILL_CHARGE, 2);

                //	    	CREF(ch, CHAR_NULL);
                //	    	CREF(vict, CHAR_NULL);
                if ((GET_SKILL(ch, SKILL_SECOND_ATTACK) > 0) && number(1, 135)
                        <= (GET_SKILL(ch, SKILL_SECOND_ATTACK)  + GET_DEX(ch))) {
                    attacks++;
                    improve_skill(ch, SKILL_SECOND_ATTACK, 14);
                    if ((GET_SKILL(ch, SKILL_THIRD_ATTACK) > 0)// && (!IS_NPC(ch) || GET_LEVEL(ch)>=20)
                            && number(1, 140)  <= (GET_SKILL(ch, SKILL_THIRD_ATTACK)  + GET_DEX(ch))) {
                        attacks++;
                        improve_skill(ch, SKILL_THIRD_ATTACK, 8);
                        if ((GET_SKILL(ch, SKILL_FOURTH_ATTACK) > 0)// && (!IS_NPC(ch) || GET_LEVEL(ch)>=30)
                                && number(1, 145) <= (GET_SKILL(ch, SKILL_FOURTH_ATTACK)  + GET_DEX(ch))) {
                            attacks++;
                            improve_skill(ch, SKILL_FOURTH_ATTACK, 4);
                        }
                    }
                }
                if (GET_SKILL(ch, SKILL_DUAL_WIELD) && (GET_EQ(ch, WEAR_DUALWIELD))
                        && ((number(1, 64) <= (GET_SKILL(ch, SKILL_DUAL_WIELD)/4  + GET_DEX(ch))))) {
                    attacks++;
                    improve_skill(ch, SKILL_DUAL_WIELD, 12);
                    dual_wield_flag = 1;
                    if (GET_SKILL(ch, SKILL_SECOND_DUAL) > 0) {
                        if (number(1, 100) <= (GET_SKILL(ch, SKILL_SECOND_DUAL)/4  + GET_DEX(ch))) {
                            attacks++;
                            improve_skill(ch, SKILL_SECOND_DUAL, 6);
                            dual_wield_flag = 2;
                        }
                    }
                }

                while (attacks>0 && !DEAD(ch) && !DEAD(vict))
                {
                    if ((dual_wield_flag > 0))
                    {
                        hit(ch, vict, SKILL_DUAL_WIELD);
                        dual_wield_flag--;
                    }
                    else
                        hit(ch, vict, TYPE_UNDEFINED);
                    attacks--;
                }
                //CUREF(ch);
                //CUREF(vict);
            }
            else
                hit(ch, vict, TYPE_UNDEFINED);
        } else {
            if (FIGHTING(ch)==vict)
                act("You are already fighting $m.", FALSE, ch, 0, vict, TO_CHAR);
            else {
                act("You turn to attack $N!", FALSE, ch, NULL, vict, TO_CHAR);
                act("$n turns to attack $N!", FALSE, ch, NULL, vict, TO_NOTVICT);
                act("$n turns to attack you!", FALSE, ch, NULL, vict, TO_VICT);
                //      stop_fighting(ch);
                FIGHTING(ch)=vict;
                WAIT_STATE(ch, PULSE_VIOLENCE);
            }
        }
    }
}

ACMD(do_kill)
{
    struct char_data *vict;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Kill who?\r\n", ch);
    } else {
        if (!(vict = get_char_room_vis(ch, arg)))
            send_to_char("They aren't here.\r\n", ch);
        else if (ch == vict)
            send_to_char("Your mother would be so sad...\r\n", ch);
        else {
            /*if (GET_LEVEL(ch) < 4 && (GET_LEVEL(vict) > 6)) {
            send_to_char("You change your mind in the last moment, as you estimate that action suicidal.\r\n", ch);
            return;
        } */
            do_hit(ch, argument, cmd, subcmd);
        }
    }
    return;
}

ACMD(do_disarm)
{
    struct obj_data *obj, *wielded;
    struct char_data *vict;
    int type, type2;
    one_argument(argument, buf);
    if (not_in_arena(ch))
        return;

    if (!GET_SKILL(ch, SKILL_DISARM))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }


    if (GET_MOVE(ch)<2)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, buf))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Disarm who?\r\n", ch);
            return;
        }
    }
    type=TYPE_HIT;
    type2=TYPE_HIT;
    wielded=GET_EQ(ch, WEAR_WIELD);
    if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
        type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
    wielded=GET_EQ(vict, WEAR_WIELD);
    if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
        type2 = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;

    if (vict == ch) {
        send_to_char("Try removing your weapon instead.\r\n", ch);
        return;
    } else if (!pk_allowed && !IS_NPC(vict) && !IS_NPC(ch) && !CAN_MURDER(ch, vict)) {
        send_to_char("That would be seen as an act of aggression!\r\n", ch);
        return;
    } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict)) {
        send_to_char("The thought of disarming your master seems revolting to you.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    } else if (!(obj = GET_EQ(vict, WEAR_WIELD)))
    {
        act("$N is unarmed!", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }
    else 	    //if ((number(1, 100)<GET_SKILL(ch, type)+GET_SKILL(ch, SKILL_DISARM)/4) && (number(1, 100)>GET_SKILL(vict, type2)-GET_SKILL(ch, SKILL_DISARM)/4))
        if ((number (1, 201) > GET_SKILL(ch, SKILL_DISARM)+GET_SKILL(ch, type)) || (number(1, 250) < GET_SKILL(vict, SKILL_GRIP)+GET_SKILL(vict, type2)))
            //	     (number(1, 201) > GET_SKILL(ch, SKILL_DISARM) / 2 + GET_LEVEL(ch) - GET_LEVEL(vict)))
        {
            act("You fail to disarm $N!", FALSE, ch, 0, vict, TO_CHAR);
            act("$n tries to disarm you, but you hold your weapon firm!", FALSE, ch, obj, vict, TO_VICT);
            if (GET_SKILL(ch, SKILL_GRIP))
                improve_skill(vict, SKILL_GRIP, 1);
        }

        else {
            improve_skill(ch, SKILL_DISARM, 2);
            obj_to_char(unequip_char(vict, WEAR_WIELD), vict);
            act("&GYou skillfully DISARM your enemy!&0", FALSE, ch, 0, 0, TO_CHAR);
            act("&GYour weapon flies out of your hand as $N DISARMS you!&0", FALSE, vict, obj, ch, TO_CHAR);
            act("$n skillfully DISARMS $N!&0", FALSE, ch, obj, vict, TO_NOTVICT);


            if ((obj = GET_EQ(vict, WEAR_DUALWIELD))) {
                act("You flip $p to your other hand.", FALSE, vict, obj, NULL, TO_CHAR);
                act("$n flips $p to $s other hand.", FALSE, vict, obj, NULL, TO_ROOM);
                obj_to_char(unequip_char(vict, WEAR_DUALWIELD), vict);
                obj_from_char(obj);
                equip_char(vict, obj, WEAR_WIELD);
            }
            if (IS_NPC(vict))
                WAIT_STATE(vict, 2*PULSE_VIOLENCE);
        }
    GET_MOVE(ch)-=2;
    if (!FIGHTING(ch))
        check_fight(ch, vict);
    WAIT_STATE(ch, PULSE_VIOLENCE);

}


ACMD(do_slay)
{
    struct char_data *vict, *victim;
    char arg2[MAX_STRING_LENGTH];
    if ((GET_LEVEL(ch) < LVL_IMPL) || IS_NPC(ch)) {
        if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
            send_to_char("This room has a nice peaceful feeling.\r\n", ch);
            return;
        }
        do_hit(ch, argument, cmd, subcmd);
        return;
    }
    two_arguments(argument, arg, arg2);

    if (!*arg) {
        send_to_char("Slay who <immolate | shatter | demon | slit | pounce | dog> ?\r\n", ch);
    } else {
        if (!(vict = get_char_room_vis(ch, arg)))
            send_to_char("They aren't here.\r\n", ch);
        else if (ch == vict)
            send_to_char("Your mother would be so sad...\r\n", ch);
        else {
            victim=vict;

            if ( isname( arg2, "immolate" ) )
            {
                act( "Your fireball turns $N into a blazing inferno.",  FALSE, ch, NULL, victim, TO_CHAR    );
                act( "$n releases a searing fireball in your direction.",FALSE,  ch, NULL, victim, TO_VICT    );
                act( "$n points at $N, who bursts into a flaming inferno.",FALSE,   ch, NULL, victim, TO_NOTVICT );
            }

            else if ( isname( arg2, "shatter" ) )
            {
                act( "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  FALSE, ch, NULL, victim, TO_CHAR    );
                act( "$n freezes you with a glance and shatters your frozen body into tiny shards.", FALSE, ch, NULL, victim, TO_VICT    );
                act( "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  FALSE, ch, NULL, victim, TO_NOTVICT );
            }

            else if ( isname( arg2, "demon" ) )
            {
                act("You gesture, and a slavering demon appears.  With a horrible grin, the\r\nfoul creature turns on $N, who screams in panic before being eaten alive.",  FALSE, ch, NULL, victim, TO_CHAR );
                act("$n gestures, and a slavering demon appears.  The foul creature turns on\r\nyou with a horrible grin.   You scream in panic before being eaten alive.", FALSE,  ch, NULL, victim, TO_VICT );
                act("$n gestures, and a slavering demon appears.  With a horrible grin, the\r\nfoul creature turns on $N, who screams in panic before being eaten alive.",  FALSE, ch, NULL, victim, TO_NOTVICT );

            }

            else if ( isname( arg2, "pounce" ) )
            {
                act( "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...", FALSE,  ch, NULL, victim, TO_CHAR );
                act( "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...",FALSE,  ch, NULL, victim, TO_VICT );
                act( "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.", FALSE,  ch, NULL, victim, TO_NOTVICT );
            }

            else if ( isname( arg2, "slit" ) )
            {
                act( "You calmly slit $N's throat.",FALSE,  ch, NULL, victim, TO_CHAR );
                act( "$n reaches out with a clawed finger and calmly slits your throat.", FALSE, ch, NULL, victim, TO_VICT );
                act( "$n calmly slits $N's throat.", FALSE, ch, NULL, victim, TO_NOTVICT );
            }

            else if ( isname( arg2, "dog" ) )
            {
                act("You order your dogs to rip $N to shreds.", FALSE, ch, NULL, victim, TO_CHAR );
                act("$n orders $s dogs to rip you apart.",FALSE,  ch, NULL, victim, TO_VICT );
                act("$n orders $s dogs to rip $N to shreds.", FALSE, ch, NULL, victim, TO_NOTVICT );
            }
            else
            {
                act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
                act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
                act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
            }
            raw_kill(vict, ch);
        }
    }
}





ACMD(do_backstab);
struct bstab_eo
{
    struct char_data *ch;

};

EVENTFUNC(event_bstab)
{
    struct bstab_eo *cevent=(struct bstab_eo *) event_obj;
    struct char_data *ch, *vict;


    ch=cevent->ch;


    if (!ch || DEAD(ch))
    {
    	
        ch->current_target=NULL;
    if (cevent)
        free (cevent);
    if (ch)
        GET_UTIL_EVENT(ch)=NULL;
    return 0;
    }

    vict=ch->current_target;
    if (vict)
        REMOVE_BIT(AFF_FLAGS(vict), AFF_TARGET);

    do_backstab(ch, "", 0, SUB_EVENT);


kraj_bstab:
    ch->current_target=NULL;
    if (cevent)
        free (cevent);
    if (ch)
        GET_UTIL_EVENT(ch)=NULL;
    return 0;
}


void assign_bstab_event(struct char_data *ch, struct char_data *victim, int when)
{
    struct bstab_eo *cevent;

    CREATE(cevent, struct bstab_eo, 1);
    cevent->ch=ch;
    ch->current_target=victim;
    SET_BIT(AFF_FLAGS(victim), AFF_TARGET);
    GET_UTIL_EVENT(ch)=event_create(event_bstab, cevent, when);
}



ACMD(do_backstab)
{
    struct char_data *vict;
    struct obj_data *obj;
    int percent, prob;


    if (GET_SKILL(ch, SKILL_BACKSTAB) == 0) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (IN_EVENT(ch) && subcmd!=SUB_EVENT)
    {
        send_to_char("You are already doing something!\r\n", ch);
        return;
    }



    if (subcmd==SUB_EVENT)
    {
        vict=ch->current_target;
        if (!vict || ch->in_room!=vict->in_room || GET_POS(ch)<POS_FIGHTING)
        {
            send_to_char("You abort the backstab.\r\n", ch);
            return;
        }

    }
    else
    {
        one_argument(argument, buf);
        if (!(vict = get_char_room_vis(ch, buf))) {
            send_to_char("Whom do you wish to backstab?\r\n", ch);
            return;
        }
    }


    if (vict == ch) {
        send_to_char("How can you sneak up on yourself?\r\n", ch);
        return;
    }
    if (!(obj = GET_EQ(ch, WEAR_WIELD))) {
        send_to_char("You need to wield a piercing weapon to make it a success.\r\n", ch);
        return;
    }
    if (GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT) {
        send_to_char("Only piercing weapons can be used for backstabing.\r\n", ch);
        return;
    }
    if (FIGHTING(vict)) {
        send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You can't backstab that person.\r\n", ch);
        return;
    }
    if (AWAKE(vict) && GET_HIT(vict) < GET_MAX_HIT(vict)/2) {
        act("$N is badly hurt and alert ... you can't sneak up.", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }



    if (subcmd!=SUB_EVENT)
    {
        send_to_char("You begin to move silently to the back of your victim.\r\n", ch);
        act("$n sneaks behind $N's back.", FALSE, ch, 0, vict, TO_NOTVICT);
        if (CAN_SEE(vict, ch))
            act("$n sneaks behind you.", FALSE, ch, 0, vict, TO_VICT);
        assign_bstab_event(ch, vict, 2*PULSE_VIOLENCE/3);
       // WAIT_STATE(ch, 2*PULSE_VIOLENCE/3);
        return;
    }


    if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$n notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        //WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }


    percent = number(1, 111);	/* 105% is a complete failure */
    prob = GET_SKILL(ch, SKILL_BACKSTAB) +5*((GET_SKILL(ch, TYPE_PIERCE)-5)/2- (GET_OBJ_LEVEL(obj)>50?50:GET_OBJ_LEVEL(obj))) + 2 * (GET_DEX(ch) - GET_INT(vict));


    if (!AWAKE(vict) || (percent <= prob) || (!CAN_SEE(vict, ch) && !INT_CHECK(vict))) {
        if (GET_SKILL(ch, SKILL_DUAL_BACKSTAB) && GET_EQ(ch, WEAR_DUALWIELD) && number(1,111)<GET_SKILL(ch, SKILL_DUAL_BACKSTAB) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_DUALWIELD), 3) == (TYPE_PIERCE - TYPE_HIT)))
        {
            hit(ch, vict, SKILL_DUAL_BACKSTAB);
            improve_skill(ch, SKILL_DUAL_BACKSTAB, 2);
        }
        else
            hit(ch, vict, SKILL_BACKSTAB);
        improve_skill(ch, SKILL_BACKSTAB, 2);
    }
    else
        damage(ch, vict, 0, SKILL_BACKSTAB, obj);
    WAIT_STATE(ch, 1 RL_SEC);
}

ACMD(do_circle)
{
    struct char_data *vict;
    struct obj_data *obj;
    int percent, prob;

    if (GET_SKILL(ch, SKILL_CIRCLE) == 0) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    one_argument(argument, buf);

    if (!FIGHTING(ch)) {
        send_to_char("You must be fighting in order to do that.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<4)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, buf)))
        vict = FIGHTING(ch);

    if (vict == ch) {
        send_to_char("You spin around in a circle!\r\n", ch);
        return;
    }
    if (!(obj = GET_EQ(ch, WEAR_WIELD))) {
        send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
        return;
    }
    if (GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT) {
        send_to_char("You must wield a piercing weapon!\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }
    if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You can't circle a person under the protection of Mark of Neutrality.\r\n", ch);
        return;
    }
    percent = number(1, 111);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_CIRCLE) + 4 * (GET_DEX(ch) - GET_INT(vict));
    GET_MOVE(ch)-=4;
    if ((percent > prob || !DEX_CHECK(ch))) {
        damage(ch, vict, 0, SKILL_CIRCLE, obj);
        WAIT_STATE(ch, (2+ (PRF2_FLAGGED(ch, PRF2_TUMBLE)?1:0))*PULSE_VIOLENCE);
    } else {


        if (GET_SKILL(ch, SKILL_DUAL_CIRCLE) && GET_EQ(ch, WEAR_DUALWIELD) && number(1,101)<GET_SKILL(ch, SKILL_DUAL_CIRCLE) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_DUALWIELD), 3) == (TYPE_PIERCE - TYPE_HIT)))
        {
            hit(ch, vict, SKILL_DUAL_CIRCLE);
            improve_skill(ch, SKILL_DUAL_CIRCLE, 3);
        }
        else
        {
            hit(ch, vict, SKILL_CIRCLE);
            improve_skill(ch, SKILL_CIRCLE, 4);
        }
        WAIT_STATE(ch, PULSE_VIOLENCE*(2+ (PRF2_FLAGGED(ch, PRF2_TUMBLE)?1:0)));
    }
}


ACMD(do_order)
{
    char name[100], message[256];
    char buf[500];
    bool found = FALSE;
    int org_room;
    struct char_data *vict;
    struct follow_type *k, *pom;

    half_chop(argument, name, message);

    if (!*name || !*message)
        send_to_char("Order who to do what?\r\n", ch);
    else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
        send_to_char("That person isn't here.\r\n", ch);
    else if (ch == vict)
        send_to_char("You obviously suffer from shitzofrenia.\r\n", ch);
    else {
        if (IS_AFFECTED(ch, AFF_CHARM)) {
            send_to_char("Your superior would not aprove you giving orders.\r\n", ch);
            return;
        }
        if (vict) {
            sprintf(buf, "$N orders you to '%s'", message);
            act(buf, FALSE, vict, 0, ch, TO_CHAR);
            act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

            if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
                act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
            else if (!FIGHTING(vict) || GET_MOB_WAIT(vict)<=0) {
                GET_MOB_WAIT(vict)=0;
                command_interpreter(vict, message);
            }
            else if (GET_MOB_WAIT(vict)>0)
                found=TRUE;
        } else {		/* This is order "followers" */
            if (!IS_NPC(ch))
                act("$n issues the order to $s followers.", FALSE, ch, 0, vict, TO_ROOM);

            org_room = ch->in_room;

            for (k = ch->followers; k;k=pom) {
            	pom=k->next;
                if (org_room == k->follower->in_room) {
                    if (IS_AFFECTED(k->follower, AFF_CHARM) && (!FIGHTING(k->follower) || GET_MOB_WAIT(k->follower)<=0)) {
                        found = TRUE;
                        GET_MOB_WAIT(k->follower)=0;
                        command_interpreter(k->follower, message);
                    }
                    else if (GET_MOB_WAIT(k->follower)>0)
                        found=TRUE;
                }        

            }
            if (!found)
                send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
        }
    }
}


ACMD(do_flee)
{
    int i, attempt, loss;
    struct char_data *opp;
    ulong flags=PRF2_FLAGS(ch);
    struct obj_data *obj;


    if (DEAD(ch))
        return;
    //loss = exp_this_level(GET_LEVEL(ch))/100;
    //loss=ch->dam_exp;
    loss=550*GET_HIT(ch)*GET_HIT(ch)/(GET_MAX_HIT(ch)*GET_MAX_HIT(ch))+50;
    if (not_in_arena(ch))
        return;

    if (AFF3_FLAGGED(ch, AFF3_FLEEING))
    {
        if (!FIGHTING(ch))
            REMOVE_BIT(AFF3_FLAGS(ch), AFF3_FLEEING);
        return;
    }

    if ((IS_NPC(ch) && GET_MOB_WAIT(ch)) || (!IS_NPC(ch) && CHECK_WAIT(ch))) {
        send_to_char("You cannot flee now!\r\n", ch);
        return;
    }
    if (GET_POS(ch) < POS_FIGHTING) {
        send_to_char("You are in pretty bad shape, unable to flee!\r\n", ch);
        return;
    }
    if (!IS_NPC(ch) && GET_MOVE(ch)<MOVE_LIMIT(ch)) {
        send_to_char("You are too exhausted to even move!\r\n", ch);
        return;
    }

    if (!FIGHTING(ch) && subcmd!=99)
    {
        send_to_char("You aren't even in a fight!\r\n",ch);
        return;
    }
    if (IS_AFFECTED(ch, AFF_HOLDED)) {
        send_to_char("You are being mysteriously holded!\r\n", ch);
        return;
    }
    if (AFF2_FLAGGED(ch, AFF2_PETRIFY)) {
        send_to_char("You are petrified! You can't move!\r\n", ch);
        return;
    }
    if (AFF3_FLAGGED(ch, AFF3_WEB))
        if (number(1, 200)>GET_DEX(ch)+GET_STR(ch))
        {
            send_to_char("You stumble in the web and fall to the ground!\r\n", ch);
            act("$n attempts to flee but stumbles in the web and falls down!", FALSE, ch, 0, 0, TO_ROOM);
            GET_POS(ch)=POS_SITTING;
            assign_stand_event(ch, PULSE_VIOLENCE);
            return;
        }

    if (AFF2_FLAGGED(ch, AFF2_BERSERK)) {
        send_to_char("You can't flee, you're BERSERK!\r\n", ch);
        return;
    }
    if (FIGHTING(ch) && MOB_FLAGGED(FIGHTING(ch), MOB_CANT_FLEE)) {
        act("$N grabs you and prevents you from fleeing!!", FALSE, ch, 0, FIGHTING(ch), TO_CHAR);
        act("$N grabs $n and prevents $m from fleeing!!", FALSE, ch, 0, FIGHTING(ch), TO_ROOM);
        return;
    }

    act("$n attempts to flee!!\r\n", TRUE, ch, 0, 0, TO_ROOM);
    SET_BIT(AFF3_FLAGS(ch), AFF3_FLEEING);

    if (FIGHTING(ch) && FIGHTING(FIGHTING(ch))==ch)
        if (DEX_CHECK(FIGHTING(ch)))
        {
            if (!(!IS_NPC(ch) && (SKILL_CHECK(ch, SKILL_RETREAT) || SKILL_CHECK(ch, SKILL_ESCAPE))))
            {
                opp=FIGHTING(ch);
                act("You use the opportunity for an extra hit to $S back.", FALSE, opp, 0, ch, TO_CHAR);
                send_to_char("You hear a swing behind you.\r\n", ch);


                //CREF(ch, CHAR_NULL);

                if (IS_THIEF(opp) && GET_EQ(opp, WEAR_WIELD) && (GET_OBJ_VAL(GET_EQ(opp, WEAR_WIELD), 3) == TYPE_PIERCE - TYPE_HIT))// && GET_SKILL(ch, SKILL_TRIP)>number(1, 111))
                {

                    if (SKILL_CHECK(opp, SKILL_BACKSTAB))
                        hit(opp,ch, SKILL_BACKSTAB);
                    else
                        hit(opp, ch, TYPE_UNDEFINED);
                }
                else
                    hit(opp, ch, TYPE_UNDEFINED);
                //CUREF(ch);
                REMOVE_BIT(AFF3_FLAGS(ch), AFF3_FLEEING);
                if (DEAD(ch))
                    return;
            }
        }


    REMOVE_BIT(AFF3_FLAGS(ch), AFF3_FLEEING);
    if (GET_POS(ch)<=POS_SITTING)
        return;


    if (!AFF3_FLAGGED(ch, AFF3_STRONG_MIND) && (number(1,111)>GET_SKILL(ch, SKILL_RETREAT) && number(1,111)>GET_SKILL(ch, SKILL_ESCAPE)))
    {
        for (i = 0; i < 12; i++) {
            attempt = number(0, NUM_OF_DIRS - 1);	/* Select a random
            					   direction */
            if (CAN_GO(ch, attempt) &&
                    !IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH) && (!(IS_SET(EXIT(ch, attempt)->exit_info, EX_NOMOB) && IS_NPC(ch)))) {

                if (number(1, 101) > 5 * GET_DEX(ch) && !SKILL_CHECK(ch, SKILL_GRIP)) {
                    if ((obj = GET_EQ(ch, WEAR_WIELD)) && !IS_OBJ_STAT(obj, ITEM_NODROP)) {
                        act("&CYou throw $p in panic!!!&0\r\n", FALSE, ch, obj, 0, TO_CHAR);
                        act("&c$n throws $p in panic!!!&0", FALSE, ch, obj, 0, TO_ROOM);
                        obj_to_room(unequip_char(ch, WEAR_WIELD), ch->in_room);
                        if ((obj = GET_EQ(ch, WEAR_DUALWIELD))) {
                            obj_to_char(unequip_char(ch, WEAR_DUALWIELD), ch);
                            obj_from_char(obj);
                            equip_char(ch, obj, WEAR_WIELD);
                        }
                    }
                }
                else
                    if (GET_SKILL(ch, SKILL_GRIP))
                        improve_skill(ch, SKILL_GRIP, 2);
                SET_BIT(PRF2_FLAGS(ch), PRF2_RUNNING);

                /* */
                if (do_simple_move(ch, attempt, TRUE)) {
                    send_to_char("\r\nYou flee head over heels.\r\n", ch);
                    gain_exp(ch, -loss);
                    if (GET_HIT(ch)>GET_MAX_HIT(ch)/2)
                        ch->dam_exp=0;
                    //		if (FIGHTING(FIGHTING(ch))==ch)
                    //			stop_fighting(FIGHTING(ch));
                    stop_fighting(ch);
                }
                PRF2_FLAGS(ch)=flags;
                return;
            }
        }
    }
    else
    {
        for (i = 0; i < 36; i++) {
            attempt = number(0, NUM_OF_DIRS - 1);	/* Select a random
            					   direction */
            if (CAN_GO(ch, attempt) &&
                    !IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH)) {
                SET_BIT(PRF2_FLAGS(ch), PRF2_RUNNING);
                if (do_simple_move(ch, attempt, TRUE)) {
                    send_to_char("\r\n&GYou skillfully retreat from the battle.&0\r\n", ch);
                    if (!AFF3_FLAGGED(ch, AFF3_STRONG_MIND))
                    {
                        improve_skill(ch, SKILL_ESCAPE, 2);
                        improve_skill(ch, SKILL_RETREAT, 2);
                    }
                    if (GET_HIT(ch)>GET_MAX_HIT(ch)/2)
                        ch->dam_exp=0;
                    //		if (FIGHTING(FIGHTING(ch))==ch)
                    //			stop_fighting(FIGHTING(ch));
                    stop_fighting(ch);
                    gain_exp(ch, -loss/2);
                    PRF2_FLAGS(ch)=flags;
                    return;
                }
                PRF2_FLAGS(ch)=flags;
            }
        }
    }
    send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}




ACMD(do_bash)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);
    if (!GET_SKILL(ch, SKILL_BASH)) {
        send_to_char("You'd better learn how to do it first.\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Bash who?\r\n", ch);
            return;
        }
    }
    if (GET_MOVE(ch)<4)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }


    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (GET_POS(vict) == POS_SITTING) {
        send_to_char("Your victim is already on the ground!\r\n", ch);
        return;
    }
    if (!(obj = GET_EQ(ch, WEAR_SHIELD))) {
        send_to_char("You need to wear a shield to even try that.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }                            
    
    if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
            (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {
        send_to_char("You can not try that here!\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    if (IS_AFFECTED(vict, AFF_WATERWALK) || IS_AFFECTED(vict, AFF_FLYING))
    {
        send_to_char("That person is flying in the air.\r\n",ch);
        return;
    }


    percent = number(1, 116);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_BASH) +  2 * (GET_STR(ch) - GET_DEX(vict))+GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_SHIELD))/2+(GET_TOTAL_WEIGHT(ch)-GET_TOTAL_WEIGHT(vict))/20+(GET_SKILL(ch, SKILL_SHIELDMASTERY)-40)/5;

    if (MOB_FLAGGED(vict, MOB_NOBASH))
        percent = 150;

    GET_MOVE(ch)-=4;

    if (percent > prob || IS_GOD(vict) || !STR_CHECK(ch)) { 
    	if (!DEX_CHECK(ch) || !STR_CHECK(ch))
    	{
        	WAIT_STATE(ch, PULSE_VIOLENCE);
        	assign_stand_event(ch, PULSE_VIOLENCE*2);
        	act("&cAs $N avoids your bash, you topple over and fall to the ground!!&0", FALSE, ch, 0, vict, TO_CHAR);
        	act("You dodge a bash from $n who loses $s balance and falls!!", FALSE, ch, 0, vict, TO_VICT);
        	act("$N avoids being bashed by $n who loses $s balance and falls!!",  FALSE, ch, 0, vict, TO_NOTVICT);        
        	check_fight(ch, vict);
        	GET_POS(ch) = POS_SITTING;
        }
        else                                   
        {
        	WAIT_STATE(ch, PULSE_VIOLENCE);
        	check_fight(ch, vict);
        	act("$N avoids your bash but you maintain balance.", FALSE, ch, 0, vict, TO_CHAR);
        	act("You dodge a bash from $n.", FALSE, ch, 0, vict, TO_VICT);
        	act("$N avoids a bash from $n.",  FALSE, ch, 0, vict, TO_NOTVICT);        
        }
        
    } else {
        WAIT_STATE(ch, 5*PULSE_VIOLENCE/2);
        WAIT_STATE(vict, 3*PULSE_VIOLENCE/4);
        check_fight(ch, vict);
        GET_POS(vict) = POS_SITTING;
        assign_stand_event(vict, PULSE_VIOLENCE);
        act("&cYour bash at $N sends $M sprawling!!&0", FALSE, ch, 0, vict, TO_CHAR);
        act("$n sends you sprawling with a powerful bash!!", FALSE, ch, 0, vict, TO_VICT);
        act("$n sends $N sprawling with a powerful bash!!", FALSE, ch, 0, vict, TO_NOTVICT);
        //damage(ch, vict, number(1, GET_LEVEL(ch))+GET_DAMROLL(ch), SKILL_BASH, obj);
        improve_skill(ch, SKILL_BASH, 5);
        improve_skill(ch, SKILL_SHIELDMASTERY, 5);
    }

}


ACMD(do_rescue)
{
    struct char_data *vict=NULL, *tmp_ch;
    int percent, prob;

    one_argument(argument, arg);
    if (AFF2_FLAGGED(ch, AFF2_BERSERK)) {
        send_to_char("You can't rescue anyone, you're BERSERK!\r\n", ch);
        return;
    }


    if (!*arg && ch->guarding && ch->guarding->in_room==ch->in_room)
        vict=ch->guarding;
    else
        vict = get_char_room_vis(ch, arg);

    if (!vict) {
        send_to_char("Whom do you want to rescue?\r\n", ch);
        return;
    }
    if (vict == ch) {
        send_to_char("What about fleeing instead?\r\n", ch);
        return;
    }
    if (FIGHTING(ch) == vict) {
        send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
        return;
    }
    if (GET_MOVE(ch)<5)
    {
        send_to_char("You are too exhausted too even try rescuing.\r\n", ch);
        return;
    }




    for (tmp_ch = world[ch->in_room].people; tmp_ch &&
            (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

    if (!tmp_ch) {
        if (subcmd!=SCMD_AUTORESCUE)
            act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }
    if (!GET_SKILL(ch, SKILL_RESCUE))
        send_to_char("You have no idea how to rescue someone.", ch);
    else {
        percent = number(1, 111);	/* 101% is a complete failure */
        prob = GET_SKILL(ch, SKILL_RESCUE);

        if (percent > prob) {
            send_to_char("You fail the rescue!\r\n", ch);
            return;
        }
        improve_skill(ch, SKILL_RESCUE, 1);
        if (subcmd!=SCMD_AUTORESCUE)
            send_to_char("&cBanzai!  To the rescue...&0\r\n", ch);
        else
            act("&cYou jump to rescue $N!&0", FALSE, ch, 0, vict, TO_CHAR);

        act("&cYou are rescued by $N!&0", FALSE, vict, 0, ch, TO_CHAR);
        act("&c$n heroically rescues $N!&0", FALSE, ch, 0, vict, TO_NOTVICT);

        if (FIGHTING(tmp_ch))
            FIGHTING(tmp_ch)=ch;
        else
            set_fighting(tmp_ch, ch);;

        GET_MOVE(ch)-=5;


        if (FIGHTING(ch))
            FIGHTING(ch)=tmp_ch;
        else
            set_fighting(ch, tmp_ch);

    }

}

ACMD(do_kick)
{
    struct char_data *vict;
    int percent, prob, dir, wasin;
    struct obj_data *obj;
    int dam;

    if (!GET_SKILL(ch, SKILL_KICK)) {
        send_to_char("You are not familiar with that skill.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }
    /*if (!(obj = GET_EQ(ch, WEAR_FEET))) {
    send_to_char("You need some kind of footwear to make it a success.\r\n", ch);
    return;
} */

    obj = GET_EQ(ch, WEAR_FEET);
    if (GET_MOVE(ch)<1)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }



    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Kick who?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }
    percent = number(1, 111);	/* 101% is a complete
    					   failure */
    prob = GET_SKILL(ch, SKILL_KICK);
    GET_MOVE(ch)-=1;

    dam=number(1, 5)+GET_DAMROLL(ch)+GET_LEVEL(ch)/3;
    if (obj && (GET_OBJ_TYPE(obj) == ITEM_WEAPON))
        dam += dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2)) + GET_OBJ_VAL(obj, 0);

    dam+=32*GET_SKILL(ch, SKILL_KICKFLIP)/100;

    if (percent > prob || IS_GOD(vict)) {
        damage(ch, vict, 0, SKILL_KICK, obj);
    } else {
        if (number(1, 256)>GET_SKILL(ch, SKILL_KICKFLIP) || !STR_CHECK(ch) || (MOB_FLAGGED(vict, MOB_NOBASH)))
        {
            damage(ch, vict, dam, SKILL_KICK, obj);
            improve_skill(ch, SKILL_KICK, 5);
        }
        else if (!number(0, 9))
        {
            damage(ch, vict, 2*dam, SKILL_KICK, obj);
            improve_skill(ch, SKILL_KICK, 5);
            improve_skill(ch, SKILL_KICKFLIP, 3);
            dir = number(0,3);
            if(!DEAD(vict) && !DEAD(ch) && CAN_GO(vict,dir)){
                wasin=vict->in_room;
                char_from_room(vict);
                char_to_room(vict, world[wasin].dir_option[dir]->to_room);
                sprintf(buf, "Your massive kick sends $N sprawling %s, into the next room!", dirs[dir]);
                act(buf, TRUE,ch,0,vict,TO_CHAR);
                sprintf(buf, "$n's massive kick sends $N sprawling %s, into the next room!", dirs[dir]);
                act(buf, TRUE,ch,0,vict,TO_NOTVICT);
                act("$n comes carriering in from the next room!", TRUE,vict,0,0, TO_NOTVICT);
                act("$n's kick sends you flying into the next room!", TRUE,ch,0,vict,TO_VICT);
                if(FIGHTING(vict))
                    stop_fighting(FIGHTING(vict));
                stop_fighting(vict);
                GET_POS(vict)=POS_SITTING;
                assign_stand_event(vict, PULSE_VIOLENCE);
                WAIT_STATE(vict,PULSE_VIOLENCE);
                if (vict->desc != NULL)
                    look_at_room(vict, 0);
                send_to_char("\r\nYou see the stars flying around your head.\r\n", vict);
                if (ROOM_AFFECTED(vict->in_room, RAFF_TRAP) && number(1, 250)>GET_CHA(vict))
                {
                    FireRoomTrap(vict);
                    if (DEAD(vict))
                        return;
                }

            }

        }
        else
        {
            GET_POS(vict) = POS_SITTING;
            WAIT_STATE(vict, PULSE_VIOLENCE/2);
            assign_stand_event(vict, PULSE_VIOLENCE);
            damage(ch, vict, 1.3*dam, SKILL_KICKFLIP, obj);
            improve_skill(ch, SKILL_KICK, 5);
            improve_skill(ch, SKILL_KICKFLIP, 3);
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_behead)
{
    struct char_data *vict;
    struct obj_data *obj, *obj1 = NULL, *obj2 = NULL;
    int percent, prob;

    if (!GET_SKILL(ch, SKILL_BEHEAD)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }
    if (not_in_arena(ch))
        return;
    one_argument(argument, buf);

    if (!(vict = get_char_room_vis(ch, buf))) {
        send_to_char("Behead who?\r\n", ch);
        return;
    }
    if (strstr(GET_NAME(vict), "headless")) {
        send_to_char("You can't behead a headless creature!\r\n", ch);
        return;
    }
    if (vict == ch) {
        send_to_char("Better ask someone else to do it...\r\n", ch);
        return;
    }
    if (IS_IMMORT(vict))
    {
    	send_to_char("That doesn't make much sense.\r\n", ch);
        return;
    }
    
    obj1 = GET_EQ(ch, WEAR_WIELD);
    obj2 = GET_EQ(ch, WEAR_DUALWIELD);
    if (!(obj1 || obj2)) {
        send_to_char("You need to be wielding an appropriate weapon.\r\n", ch);
        return;
    }
    obj = obj1;
    if (GET_OBJ_VAL(obj1, 3) != TYPE_SLASH - TYPE_HIT) {
        if (!obj2) {
            send_to_char("You need to be wielding an appropriate type of weapon.\r\n", ch);
            return;
        }
        if ((GET_OBJ_VAL(obj2, 3) != TYPE_SLASH - TYPE_HIT)) {
            send_to_char("You need to be wielding an appropriate type of weapon.\r\n", ch);
            return;
        }
        obj = obj2;
    }
    if (FIGHTING(vict)) {
        send_to_char("You can't behead a fighting person -- they're too alert!\r\n", ch);
        return;
    }
    if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }
    if (IS_AFFECTED(vict, AFF_WATERWALK) || IS_AFFECTED(vict, AFF_FLYING))
    {
        send_to_char("That person is flying in the air.\r\n",ch);
        return;
    }


    if (!CAN_MURDER(ch, vict)) {
        send_to_char("Gods protect that person.\r\n", ch);
        return;
    }
    percent = number(1, 240);
    if (GET_LEVEL(vict)>50) percent=300;
    if (!IS_NPC(vict))
        percent+=GET_LEVEL(ch);
    prob = GET_SKILL(ch, SKILL_BEHEAD)/2+2*(GET_DEX(ch) - GET_DEX(vict))+8*(GET_LEVEL(ch) - GET_LEVEL(vict));


    if ((AWAKE(vict) && (percent > prob)) || !IS_NPC(vict))
        damage(ch, vict, 0, SKILL_BEHEAD, obj);
    else {
        SET_BIT(AFF2_FLAGS(vict), AFF2_BEHEAD);
        damage(ch, vict, GET_MAX_HIT(vict)*4, SKILL_BEHEAD, obj);
        improve_skill(ch, SKILL_BEHEAD, 1);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

ACMD(do_fist_of_doom)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    if (!GET_SKILL(ch, SKILL_FIST_OF_DOOM)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("Fist of doom who?\r\n", ch);
        return;
    }
    if (FIGHTING(vict)) {
        send_to_char("Your victim is to aware to let you do that!", ch);
        return;
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("Gods protect that person.\r\n", ch);
        return;
    }
    if (number(0, 85) > GET_SKILL(ch, SKILL_FIST_OF_DOOM) / 2 + 2*(GET_LEVEL(ch) - GET_LEVEL(vict))) {
        damage(ch, vict, 0, SKILL_FIST_OF_DOOM, 0);
    } else {
        damage(ch, vict, MIN(1000, MAX(MAX(5*GET_LEVEL(ch), GET_MAX_HIT(vict)/8), 40 * (GET_LEVEL(ch) - GET_LEVEL(vict)))), SKILL_FIST_OF_DOOM, 0);
        improve_skill(ch, SKILL_FIST_OF_DOOM, 5);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_berserk)
{
    struct affected_type af;

    /* Don't allow charmed mobs to do this, check player's level */
    if ((IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
            || (!IS_NPC(ch)
                && !GET_SKILL(ch, SKILL_BERSERK))) {
        send_to_char("You're not enough of a warrior to go Berserk.\r\n",
                     ch);
        return;
    }
    if (!FIGHTING(ch)) {
        send_to_char("You aren't fighting anyone.\r\n", ch);
        return;
    }
    if (AFF2_FLAGGED(ch, AFF2_BERSERK))
        return;


    send_to_char("The taste of blood begins to drive you crazy!\r\n",ch);

    if (GET_SKILL(ch, SKILL_BERSERK) > number(1, 111)) {
        
        af.bitvector = 0;
        af.bitvector2 = AFF2_BERSERK;
        af.bitvector3 = 0;
        af.type = SKILL_BERSERK;
        af.duration = -1;
        af.location = APPLY_HITROLL;
        af.modifier = GET_LEVEL(ch)/5+2;
        affect_to_char(ch, &af);

	af.modifier = GET_LEVEL(ch)/5+2;
        af.location = APPLY_DAMROLL;
        affect_to_char(ch, &af);

        send_to_char("&WYou have gone BERSERK!&0\r\n", ch);
        act("$n has gone &WBERSERK!&0", FALSE, ch, NULL, NULL, TO_ROOM);
        WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
        return;
    }
    send_to_char("You shake off the madness.\r\n", ch);

    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void specext(struct char_data * ch, struct char_data * victim)
{
    int num = 4000;
    if (IS_NPC(ch))
        num = 2000;
    if (IS_WET(victim) || IS_GOD(victim))
        num = 100;
    if (number(1, num) > MAX(300, ((GET_DEX(ch) - 10) * 100))) {
        if (ch == victim) {
            send_to_char("You try to put out the flames on your body!\r\n", ch);
            act("$n panicly tries to put out the flames on $s body!", FALSE, ch, NULL, NULL, TO_ROOM);
        } else {
            act("You try to put out the flames on $N's body!", FALSE, ch, NULL, victim, TO_CHAR);
            act("$n tries to put out the flames on $N's body!", FALSE, ch, NULL, victim, TO_NOTVICT);
            act("$n tries to put out the flames on your body!", FALSE, ch, NULL, victim, TO_VICT);
        }
    } else {
        if (ch == victim) {
            if (IS_WET(victim)) {
                send_to_char("The rain extinguishes the flames on your body.\r\n", ch);
                act("The rain extinguishes flames on $n's body.", FALSE, ch, NULL, NULL, TO_ROOM);
            } else {
                send_to_char("&BYou relax as you manage to extinguish yourself.&0\r\n", ch);
                act("$n relaxes as $e manages to extinguish $mself.", FALSE, ch, NULL, NULL, TO_ROOM);
            }
        } else {
            if (IS_WET(victim)) {
                act("$N gets wet and you easily extinguish $M.", FALSE, ch, NULL, victim, TO_CHAR);
                act("$n easily extinguishes $N's wet body.", FALSE, ch, NULL, victim, TO_NOTVICT);
                act("$n easily extinguishes you as you got wet.", FALSE, ch, NULL, victim, TO_VICT);
            } else {
                act("&BYou manage to extinguish $N.&0", FALSE, ch, NULL, victim, TO_CHAR);
                act("&B$n manages to extinguish $N.&0", FALSE, ch, NULL, victim, TO_NOTVICT);
                act("&B$n manages to extinguish you.&0", FALSE, ch, NULL, victim, TO_VICT);
            }
        }
        killfire(victim);
    }
    WAIT_STATE(ch, 1 RL_SEC);
}

ACMD(do_extinguish)
{
    struct char_data *vict;
    int percent, prob;

    one_argument(argument, arg);
    if (*arg) {
        if (!(vict = get_char_room_vis(ch, arg))) {
            send_to_char("Extinguish whom?\r\n", ch);
            return;
        }
        if (!AFF2_FLAGGED(vict, AFF2_BURNING)) {
            act("$N is not burning.", FALSE, ch, NULL, vict, TO_CHAR);
            return;
        }
    } else {
        vict = ch;
        if (!AFF2_FLAGGED(vict, AFF2_BURNING)) {
            send_to_char("You are not burning.\r\n", ch);
            return;
        }
    }
    specext(ch, vict);
    return;
}

void specwarm(struct char_data * ch, struct char_data * victim)
{
    int num = 4000;
    if (IS_NPC(ch))
        num = 2000;
    if (IS_GOD(victim))
        num=100;
    if (number(1, num) > MAX(300, ((GET_DEX(ch) - 10) * 100))) {
        if (ch == victim) {
            send_to_char("You try to warm up yourself but you end up rubbing your hands together.\r\n", ch);
            act("$n tries to warm up $mself but ends up rubbing $s hands together.", FALSE, ch, NULL, NULL, TO_ROOM);
        } else {
            act("You warm $N up a little.", FALSE, ch, NULL, victim, TO_CHAR);
            act("$n warms $N up a little.", FALSE, ch, NULL, victim, TO_NOTVICT);
            act("$n warms you up a little.", FALSE, ch, NULL, victim, TO_VICT);
        }
    } else {
        if (ch == victim) {
            send_to_char("&BYou feel warm and easy again.&0\r\n", ch);
            act("$n feels warm and easy again.", FALSE, ch, NULL, NULL, TO_ROOM);
        } else {
            act("&BYou warm $N up.&0", FALSE, ch, NULL, victim, TO_CHAR);
            act("&B$n warms $N up.&0", FALSE, ch, NULL, victim, TO_NOTVICT);
            act("&B$n warms you up.&0", FALSE, ch, NULL, victim, TO_VICT);
        }
        killcold(victim);
    }
    WAIT_STATE(ch, 1 RL_SEC);
}

ACMD(do_warm)
{
    struct char_data *vict;
    int percent, prob;

    one_argument(argument, arg);
    if (*arg) {
        if (!(vict = get_char_room_vis(ch, arg))) {
            send_to_char("Warm up whom?\r\n", ch);
            return;
        }
        if (!AFF2_FLAGGED(vict, AFF2_FREEZING)) {
            act("$N is not freezing.", FALSE, ch, NULL, vict, TO_CHAR);
            return;
        }
    } else {
        vict = ch;
        if (!AFF2_FLAGGED(vict, AFF2_FREEZING)) {
            send_to_char("You are not freezing.\r\n", ch);
            return;
        }
    }
    specwarm(ch, vict);
    return;
}

void specclear(struct char_data * ch, struct char_data * victim)
{
    int num = 4000;
    if (IS_NPC(ch))
        num = 2000;
    if (IS_GOD(victim))
        num=100;
    if (number(1, num) > MAX(300, ((GET_DEX(ch) - 10) * 100))) {
        if (ch == victim) {
            send_to_char("You clear some acid off your body.\r\n", ch);
            act("$n clears some acid off $s body.", FALSE, ch, NULL, NULL, TO_ROOM);
        } else {
            act("You clear some acid off $N's body.", FALSE, ch, NULL, victim, TO_CHAR);
            act("$n clears some acid off $N's body.", FALSE, ch, NULL, victim, TO_NOTVICT);
            act("$n clears some acid off your body.", FALSE, ch, NULL, victim, TO_VICT);
        }
    } else {
        if (ch == victim) {
            send_to_char("&BYou clear last drops of the damn thing.&0\r\n", ch);
            act("$n clears last drops of acid off $s body.", FALSE, ch, NULL, NULL, TO_ROOM);
        } else {
            act("&BYou clear last drops of acid off $N's body.&0", FALSE, ch, NULL, victim, TO_CHAR);
            act("&B$n clears last drops of acid off $N's body.&0", FALSE, ch, NULL, victim, TO_NOTVICT);
            act("&B$n clears last drops of acid off your body.&0", FALSE, ch, NULL, victim, TO_VICT);
        }
        killacid(victim);
    }
    WAIT_STATE(ch, 1 RL_SEC);
}

ACMD(do_clear)
{
    struct char_data *vict;
    int percent, prob;

    one_argument(argument, arg);
    if (*arg) {
        if (!(vict = get_char_room_vis(ch, arg))) {
            send_to_char("Clear whom?\r\n", ch);
            return;
        }
        if (!AFF2_FLAGGED(vict, AFF2_ACIDED)) {
            act("$N is not covered in acid.", FALSE, ch, NULL, vict, TO_CHAR);
            return;
        }
    } else {
        vict = ch;
        if (!AFF2_FLAGGED(vict, AFF2_ACIDED)) {
            send_to_char("You are not covered in acid.\r\n", ch);
            return;
        }
    }
    specclear(ch, vict);
    return;
}







void specstruggle(struct char_data * ch, struct char_data * victim)
{
    int num = 5000;
    if (IS_NPC(ch))
        num = 3500;
    if (number(1, num) > MAX(300, ((GET_STR(ch) - 10) * 90))) {
        send_to_char("You hopelessly struggle with the web.\r\n", ch);
        act("$n hopelessly struggles with the web.", FALSE, ch, NULL, NULL, TO_ROOM);
    } else {
        send_to_char("&BYou tear the web apart!&0\r\n", ch);
        act("$n tears the web apart!", FALSE, ch, NULL, NULL, TO_ROOM);
        affect_from_char(ch, SPELL_WEB);
    }
    WAIT_STATE(ch, 1 RL_SEC);
}

ACMD(do_struggle)
{
    struct char_data *vict;
    int percent, prob;

    vict=ch;
    if (!AFF3_FLAGGED(vict, AFF3_WEB)) {
        send_to_char("You are not covered by a web.\r\n", ch);
        return;
    }
    specstruggle(ch, vict);
    return;
}




ACMD(do_first_aid)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("First aid who?\r\n", ch);
        return;
    }
    if (vict == ch) {
        send_to_char("You cannot do that!\r\n", ch);
        return;
    }
    //    prob = GET_SKILL(ch, SKILL_FIRST_AID);

    if (number(1, 111) <= prob) {
        GET_HIT(vict) += number(3, 10);
        GET_HIT(vict) = MAX(GET_MAX_HIT(vict), GET_HIT(vict));
        act("You do first aid on $N.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n skillfully heals some of your wounds.", FALSE, ch, 0, vict, TO_VICT);
        act("$n jumps in and aid $N.", FALSE, ch, 0, vict, TO_NOTVICT);
    } else {
        GET_HIT(vict) -= number(3, 10);
        act("You try to first aid $N but you failed. Oh, no!.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n tries to aid you but he only made things worse!", FALSE, ch, 0, vict, TO_VICT);
        act("$n tried to aid but only made thing worse $N.", FALSE, ch, 0, vict, TO_NOTVICT);
    }

}


static char *fire_msg[] = {
                              "You are not holding something to throw.\r\n",
                              "You must wield the correct weapon.\r\n",
                              "You are not holding the right arrow for that weapon.\r\n",
                              "You must specify a correct direction.\r\n",
                              "You pull your arm back and throw $p.",
                              "You take aim and fire $p.",
                              "With herculean might, $n throws $p.",
                              "$n takes aim and fires $p.",
                              "Losing its momentum, $p lands to the ground.",
                              "Miraculously, $p returns to $n.",
                              "Miraculously, $p returns to you.",
                              "Making a turn, $p returns."
                              "\n"
                          };

extern char *dirs[];

void fire_in_direction(struct char_data * ch, struct obj_data * obj, int look_type, int distance, char *name);

ACMD(do_fire)
{
    static char arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj = GET_EQ(ch, WEAR_WIELD);
    int look_type, distance,  fnd=0,dam = 0,
                                   diceroll,att, def;             
    struct char_data *victim;

    two_arguments(argument, arg, arg2);
    
    
    if (subcmd == SCMD_QUICKTHROW) {
/*    	if (!GET_SKILL(ch, SKILL_THROW_DAGGER))
    	{
    		send_to_char("You don't even know how to do that.\r\n", ch);
    		return;    	
	}
	else*/ if (!(victim=FIGHTING(ch)))
	{
		send_to_char("You must be fighting in order to do that.\r\n", ch);
		return;
	}              
	if (*arg)
	{
		if ((obj = get_obj_in_list_vis(ch, arg, ch->carrying)))		
			fnd=1;
	}
	else if (GET_SKILL(ch, SKILL_THROW_DAGGER))
		for (obj = ch->carrying; obj; obj = obj->next_content) 
        		if (GET_OBJ_TYPE(obj)==ITEM_WEAPON && GET_OBJ_VAL(obj, 3) == TYPE_PIERCE - TYPE_HIT)
        		{
        			fnd=1;
        			break;
        		}
        if (!fnd)
        {
		send_to_char("You don't have that in your inventory.\r\n", ch);
		return;
	}               
	
	// now finnaly, throw the dagger
          
                             
    att=GET_SKILL(ch, SKILL_THROW_DAGGER)/2+GET_DEX(ch);
    if (GET_OBJ_TYPE(obj)==ITEM_WEAPON && GET_OBJ_VAL(obj, 3) == TYPE_PIERCE - TYPE_HIT)
    	att+=GET_SKILL(ch, SKILL_THROW_DAGGER)/2;
    def=GET_DEX(ch)*3;    
    if (GET_RACE(ch)==RACE_ELF || GET_RACE(ch)==RACE_DROW)
        att+=20;
    
    act("You throw $p at $N.", FALSE, ch, obj, victim, TO_CHAR);
    act("$n throws $p at you.", FALSE, ch, obj, victim, TO_VICT);
    act("$n throws $p at $N.", FALSE, ch, obj, victim, TO_NOTVICT);
    
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
    diceroll = number(1, 100);
    if (((att-def)+100)/2<diceroll)
	{
		act("You miss.", FALSE, ch, 0, victim, TO_CHAR);        
		act("$e misses.", FALSE, ch, 0, victim, TO_VICT);		
	}
    else if (AFF2_FLAGGED(victim, AFF2_MIRRORIMAGE) && (number(1, 40) > GET_INT(ch)) && (number(1, 40) <= GET_INT(victim))) {
        act("One of $N's false images dissipate and is instantly replaced!", FALSE, ch, 0, victim, TO_CHAR);
        act("One of your images takes the $p $n throwed at you.", FALSE, ch, obj, victim, TO_VICT);  
    } else {        
        dam=0;                                                                          
        //if (GET_SKILL(ch, SKILL_THROW_DAGGER) && GET_OBJ_TYPE(obj)==ITEM_WEAPON && GET_OBJ_VAL(obj, 3) == TYPE_PIERCE - TYPE_HIT)
            dam+=GET_DAMROLL(ch);                                                          
            if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
        	{
			dam += (GET_OBJ_VAL(obj, 0) + dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2)));
        	}
        	else
            		dam += number(GET_OBJ_WEIGHT(obj) / 4, GET_OBJ_WEIGHT(obj) / 2);  
            
                
          
            
            dam*=str_app[GET_STR(ch)].todam;

 	   dam = MAX(1, dam);          /* at least 1 hp damage min per hit */
 	   damage(ch, victim, dam, SKILL_MISSILE, obj); 
 	   improve_skill(ch, SKILL_THROW_DAGGER, 2); 	   
    }
 
	WAIT_STATE(ch, PULSE_VIOLENCE); 	
	return;

    }
    
    if (!*arg || (look_type = search_block(arg, dirs, FALSE)) < 0) {
        send_to_char(fire_msg[3], ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("Divine intervention prevents you from doing that.\r\n", ch);
        return;
    }
    if (subcmd == SCMD_THROW) {
        obj = GET_EQ(ch, WEAR_DUALWIELD);
        if (!obj && !GET_EQ(ch, WEAR_WIELD)) {
            if (!GET_EQ(ch, WEAR_HOLD)) {
                send_to_char(fire_msg[0], ch);
                return;
            } else
                obj = unequip_char(ch, WEAR_HOLD);
        } else if (!obj) {
            obj = unequip_char(ch, WEAR_WIELD);
        } else {
            obj = unequip_char(ch, WEAR_DUALWIELD);
        }
        distance = 1;
        if (GET_OBJ_TYPE(obj)==ITEM_MISSILE)
            GET_OBJ_VAL(obj, 3)=0;
    } else {			/* fire, not throw */
        obj=GET_EQ(ch, WEAR_WIELD);
        if (!obj || GET_OBJ_TYPE(obj) != ITEM_FIREWEAPON) {
            send_to_char(fire_msg[1], ch);
            return;
        }
        if (!GET_EQ(ch, WEAR_HOLD) || GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) != ITEM_MISSILE
                // || GET_OBJ_VAL(obj, 3) != GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3)
           ) {
            send_to_char(fire_msg[2], ch);
            return;
        }
        if (number(1, 121)>GET_SKILL(ch, SKILL_ARCHERY)+GET_INT(ch)+GET_DEX(ch)+2*(GET_LEVEL(ch)-GET_OBJ_LEVEL(obj)))
        {
            act("As you try to aim, $p slips away.", FALSE, ch, GET_EQ(ch, WEAR_HOLD), 0, TO_CHAR);
            return;
        }
        dam=dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2))+GET_OBJ_VAL(obj, 0);
        dam=MAX(1, dam);
        obj = unequip_char(ch, WEAR_HOLD);
        GET_OBJ_VAL(obj, 3)=dam;
        distance = GET_OBJ_VAL(obj, 0);
        //if (*arg2)
        //  distance = MAX(1, MIN(atoi(arg2), distance));
        distance = MIN(distance, 10);	/* Max firing distance is 8 */
    }
    act(fire_msg[4 + subcmd], FALSE, ch, obj, 0, TO_CHAR);
    act(fire_msg[6 + subcmd], FALSE, ch, obj, 0, TO_ROOM);
    improve_skill(ch, SKILL_ARCHERY, 1);
    improve_skill(ch, SKILL_THROW_DAGGER, 1);
    fire_in_direction(ch, obj, look_type, distance, arg2);
}

bool range_hit(struct char_data * ch, struct char_data * victim, struct obj_data * obj);

bool fire_at_char(struct char_data * ch, struct char_data * list, struct obj_data * obj, int dir, char *name);

void fire_in_direction(struct char_data * ch, struct obj_data * obj, int dir, int distance, char *name)
{
    room_num temp_room = ch->in_room;
    struct char_data *vict;
    bool contin = TRUE;		/* Has missile hit someone yet? (true =
       no) */

    while (contin && EXITN(temp_room, dir) && (distance-- > 0) &&
            !IS_SET(EXITN(temp_room, dir)->exit_info, EX_CLOSED)) {
        temp_room = EXITN(temp_room, dir)->to_room;
        if (temp_room==NOWHERE)
            return;
        else if (ROOM_FLAGGED(temp_room, ROOM_MURKY))
            distance--;
        else
            contin = fire_at_char(ch, world[temp_room].people, obj, dir, name);
    }
    if (IS_OBJ_STAT(obj, ITEM_RETURNING)) {
        act(fire_msg[10], FALSE, ch, obj, 0, TO_CHAR);
        act(fire_msg[9], FALSE, ch, obj, 0, TO_ROOM);
        if (temp_room!=ch->in_room  && (vict = world[temp_room].people)) {
            act(fire_msg[11], FALSE, vict, obj, vict, TO_VICT);
            act(fire_msg[11], FALSE, vict, obj, vict, TO_NOTVICT);
        }
        obj_to_char(obj, ch);
    } else {
        if ((vict = world[temp_room].people)) {
            act(fire_msg[8], FALSE, vict, obj, 0, TO_CHAR);
            act(fire_msg[8], FALSE, vict, obj, 0, TO_ROOM);
        }
        obj_to_room(obj, temp_room);
        if (GET_OBJ_TYPE(obj)==ITEM_MISSILE)
            GET_OBJ_VAL(obj, 3)=0;
    }
    WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_trip)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);
    if (!GET_SKILL(ch, SKILL_TRIP)) {
        send_to_char("You'd better learn how to do it first.\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Trip who?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (GET_POS(vict) == POS_SITTING) {
        send_to_char("Your victim is already on the ground!\r\n", ch);
        return;
    }
    if (GET_MOVE(ch)<2)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }



    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }       
    
     if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
            (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {
        send_to_char("You can not try that here!\r\n", ch);
        return;
    }
    if (IS_AFFECTED(vict, AFF_WATERWALK) || IS_AFFECTED(vict, AFF_FLYING))
    {
        send_to_char("That person is flying in the air.\r\n",ch);
        return;
    }

    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    percent = number(1, 111);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_TRIP) +  2 * (GET_DEX(ch) - GET_DEX(vict))-GET_TOTAL_WEIGHT(vict)/10;

    if (MOB_FLAGGED(vict, MOB_NOBASH))
        percent = 150;


    GET_MOVE(ch)-=2;

    if (percent > prob || !DEX_CHECK(ch)) {
        WAIT_STATE(ch, 2*PULSE_VIOLENCE);

        act("$N manages to avoid your trip attempt.", FALSE, ch, 0, vict, TO_CHAR);
        act("You manage to dodge $n's attempt to trip you.", FALSE, ch, 0, vict, TO_VICT);
        act("$N quickly avoids trip from $n.", FALSE, ch, 0, vict, TO_NOTVICT);

    } else {
        WAIT_STATE(ch, 5*PULSE_VIOLENCE/2);
        WAIT_STATE(vict, 3*PULSE_VIOLENCE/4);
        assign_stand_event(vict, PULSE_VIOLENCE);
        GET_POS(vict) = POS_SITTING;
        act("&cYour sweeping foot takes $N's feet out from under $M!&0", FALSE, ch, 0, vict, TO_CHAR);
        act("$n's sweeping trip takes your feet out from under you!", FALSE, ch, 0, vict, TO_VICT);
        act("$n's sweeping trip takes $N's feet right out from under $M!", FALSE, ch, 0, vict, TO_NOTVICT);

        improve_skill(ch, SKILL_TRIP, 5);
    }
    check_fight(ch, vict);
}


ACMD(do_stun)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;
    
    
    one_argument(argument, arg);
    if (!GET_SKILL(ch, SKILL_STUN)) {
        send_to_char("You'd better learn how to do it first.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("Stun who?\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    if (AWAKE(vict) && GET_HIT(vict) < GET_MAX_HIT(vict)/2 && CAN_SEE(vict, ch)) {
        act("$N is watching you suspiciously ... you can't sneak up.", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }

    if (FIGHTING(vict))
    {
        act("$N is to alert now!", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }                  
    
    if (GET_POS(vict)<=POS_STUNNED)
    {
    	send_to_char("They are already stunned.\r\n", ch);
    	return;
    }
    
    

    if (MOB_FLAGGED(vict, MOB_AWARE) && GET_LEVEL(vict)>GET_LEVEL(ch) && AWAKE(vict)) {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE );
        return;
    } 


    prob=GET_SKILL(ch, SKILL_STUN);
    if (AFF2_FLAGGED(vict, AFF2_HASTE) || AFF2_FLAGGED(vict, AFF2_ADRENALIN))
        prob/=2;

    if (number(1,101)<prob && (DEX_CHECK(ch) || !CAN_SEE(vict, ch) || !INT_CHECK(vict)))
    {
        act("&CYou sneak up and squeeze a nerve on $N's neck. You STUN $M!&0", FALSE, ch, NULL, vict, TO_CHAR);
        act("$n sneaks up and squeezes a nerve on $N's neck.", FALSE, ch, NULL, vict, TO_NOTVICT);
        act("$n sneaks up and squeezes a nerve on your neck. \r\n&CYou are STUNNED!&0", FALSE, ch, NULL, vict, TO_VICT);
        GET_POS(vict)=POS_STUNNED;
        WAIT_STATE(vict, PULSE_VIOLENCE);
        //WAIT_STATE(ch, PULSE_VIOLENCE);
        improve_skill(ch, SKILL_STUN, 2);
    }
    else
    {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);        
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        //WAIT_STATE(ch, PULSE_VIOLENCE );
    }
}

ACMD(do_archirage)
{
    struct char_data *tch, *next_tch;
    int flag=0;
    int kk=0;
    struct affected_type af;


    if (!GET_SKILL(ch, SKILL_ARCHIRAGE))
    {
        send_to_char("Go play with your toys and leave this to true warriors.\r\n", ch);
        return;
    }

    if (FIGHTING(ch))
    {
        send_to_char("You can not concentrate enough!\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
        send_to_char("This room is too peaceful for such an uncivilized behaviour.\r\n", ch);
        return;
    }


    if (number(1,111)>GET_SKILL(ch, SKILL_ARCHIRAGE))
    {
        kk=1;
    }

    for (tch = world[ch->in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (tch == ch)
            continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
            continue;
        if (!CAN_MURDER(ch, tch))
            continue;
        if (is_same_group(ch, tch))
            continue;
        if (!CAN_SEE(ch, tch) && number(0, 2))
            continue;
        if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
            continue;
        if (kk==0 && flag==0)
        {
            send_to_char("&GGROOOOOOOOOOAAARRRR!!!&0\r\n\r\n", ch);
            flag=1;
            improve_skill(ch, SKILL_ARCHIRAGE, 1);
        }
        if (kk==1 && flag==0)
        {
            send_to_char("You fail.\r\n", ch);
            flag=1;
        }
        if (kk==0)
        {
            hit(ch, tch, TYPE_UNDEFINED);
            if (ch && tch  && !DEAD(ch) && tch->in_room==ch->in_room && number(1,150)>GET_SKILL(ch, SKILL_ARCHIRAGE))
                hit(ch, tch, TYPE_UNDEFINED);
        }
        else
            hit(tch, ch, TYPE_UNDEFINED);
    }
}



ACMD(do_whirlwind)
{
    struct char_data *tch, *next_tch;
    int flag=0;
    int kk=0;
    struct affected_type af;


    if (!GET_SKILL(ch, SKILL_WHIRLWIND))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }


    if (GET_MOVE(ch)<10)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
        send_to_char("This room is too peaceful.\r\n", ch);
        return;
    }

    if (!GET_EQ(ch, WEAR_WIELD) || !GET_EQ(ch, WEAR_DUALWIELD)) {
        send_to_char("You need to wield weapons in both of your hands for balance.\r\n", ch);
        return;
    }

    GET_MOVE(ch)-=10;

    if (number(1,151)>(GET_SKILL(ch, SKILL_WHIRLWIND)+2*GET_DEX(ch)))
    {
        send_to_char("You loose your balance and tumble to the ground!\r\n", ch);
        act("Attempting to do a whirlwind, $n looses $s balance and tumbles to the ground.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        assign_stand_event(ch, PULSE_VIOLENCE*2);
        return;
    }


    for (tch = world[ch->in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (tch == ch)
            continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
            continue;
        if (!CAN_MURDER(ch, tch))
            continue;
        if (is_same_group(ch, tch))
            continue;
        if (!CAN_SEE(ch, tch) && number(0, 2))
            continue;
        if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
            continue;
        if (flag==0)
        {
            send_to_char("&wYou spin around in a deadly whirlwind!&0\r\n\r\n", ch);
            act("$n spins around, forming a deadly whirlwind!", FALSE, ch, 0, 0, TO_ROOM);
            flag=1;
            improve_skill(ch, SKILL_WHIRLWIND, 1);
        }

        //        CREF(tch, CHAR_NULL);
        hit(ch, tch, TYPE_UNDEFINED);
        if (ch && tch && !DEAD(ch) && tch->in_room==ch->in_room) hit(ch, tch, SKILL_DUAL_WIELD);
        //      CUREF(tch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE* 2);
}

ACMD(do_spin_kick)
{
    struct char_data *tch, *next_tch;
    int flag=0;
    int kk=0;
    struct affected_type af;


    if (!GET_SKILL(ch, SKILL_SPIN_KICK))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (!FIGHTING(ch))
    {
        send_to_char("You aren't even in a fight.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<6)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
        send_to_char("This room is too peaceful.\r\n", ch);
        return;
    }


    GET_MOVE(ch)-=6;

    if (number(1,151)>(GET_SKILL(ch, SKILL_SPIN_KICK)+2*GET_DEX(ch)))
    {
        send_to_char("You loose your balance and tumble to the ground!\r\n", ch);
        act("Attempting to perform a spin kick, $n looses $s balance and tumbles to the ground.\r\n", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        assign_stand_event(ch, PULSE_VIOLENCE*2);
        return;
    }


    for (tch = world[ch->in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (tch == ch)
            continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
            continue;
        if (!CAN_MURDER(ch, tch))
            continue;
        if (is_same_group(ch, tch))
            continue;
        if (!CAN_SEE(ch, tch))
            continue;
        if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
            continue;
        if (!(FIGHTING(tch)==ch))
            continue;
        if (flag==0)
        {
            send_to_char("&wYou jump up and perform a deadly spin kick!&0\r\n", ch);
            act("$n jumps up and performs a deadly spin kick!", FALSE, ch, 0, 0, TO_ROOM);
            flag=1;
            improve_skill(ch, SKILL_SPIN_KICK, 5);
        }
        //        CREF(tch, CHAR_NULL);
        damage(ch, tch, 3*GET_LEVEL(ch)+number(GET_DAMROLL(ch), GET_DAMROLL(ch)*3), SKILL_SPIN_KICK, NULL);
        if (!DEAD(ch) && !DEAD(tch) && number(0, 55)>GET_LEVEL(tch))
        {
            send_to_char("The blow takes you right to the ground!\r\n", tch);
            act("The blow takes $n to the ground!", FALSE, tch, 0, 0, TO_ROOM);
            WAIT_STATE(tch, PULSE_VIOLENCE/2);
            GET_POS(tch) = POS_SITTING;
            assign_stand_event(tch, PULSE_VIOLENCE);
        }
        //      CUREF(tch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE* 2);
}


ACMD(do_gouge)
{
    struct char_data *vict;
    struct obj_data *obj, *head;
    int percent, prob;
    struct affected_type af, af2;

    if (GET_SKILL(ch, SKILL_GOUGE) == 0) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    one_argument(argument, buf);

    if (!(vict = get_char_room_vis(ch, buf))) {
        send_to_char("Eye gouge whom?\r\n", ch);
        return;
    }
    if (vict == ch) {
        send_to_char("Funny fellow.\r\n", ch);
        return;
    }
    
    if (!(obj = GET_EQ(ch, WEAR_WIELD))) {
        send_to_char("You need to wield a piercing weapon to make it a success.\r\n", ch);
        return;
    }
    if (GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT) {
        send_to_char("Only piercing weapons can be used for this.\r\n", ch);
        return;
    }
    
    if (IS_IMMORT(vict))
    {
    	send_to_char("That doesn't make much sense.\r\n", ch);
        return;
    }
    if (FIGHTING(vict)) {
        send_to_char("You can't gouge eyes to a fighting person -- they're too alert!\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }
    if (affected_by_spell(vict, SPELL_BLINDNESS))
    {
        act("$E is already lurking around blinded!", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }
    if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }

    /*    if (!IS_NPC(vict)) {
    	send_to_char("You can't gouge eyes to other players.\r\n", ch);
    	return;
        }*/

    if (AWAKE(vict) && GET_HIT(vict) < GET_MAX_HIT(vict)/2 && CAN_SEE(vict, ch)) {
        act("$N is watching you suspiciously ... you can't sneak up.", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }


    percent = number(1, 111);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_GOUGE) - dex_app[GET_DEX(vict)].reaction;

    if (AWAKE(vict) && (percent > prob || !DEX_CHECK(ch)))
    {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }
    else {
        af.bitvector = 0;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        af.type = SPELL_BLINDNESS;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.duration =-1;
        af.bitvector = AFF_BLIND;
        affect_to_char(vict, &af);
        improve_skill(ch, SKILL_GOUGE, 1);
        act("You sneak up, and use $p to gouge $N's eyes out!!!", FALSE, ch, GET_EQ(ch, WEAR_WIELD), vict, TO_CHAR);
        act("$n sneaks up, and uses $p to gouge $N's eyes out!!!", FALSE, ch, GET_EQ(ch, WEAR_WIELD), vict, TO_NOTVICT);
        act("$n is BLIND!", FALSE, vict, 0, 0, TO_ROOM);
        head = read_object(21, VIRTUAL, 0, GET_LEVEL(vict)/3);
        sprintf(buf2, "an eye of %s", GET_NAME(vict));
        head->short_description = str_dup(buf2);
        sprintf(buf2, "An eye of %s is lying here.", GET_NAME(vict));
        head->description = str_dup(buf2);
        obj_to_room(head, ch->in_room);
    }
}


ACMD(do_buddha_finger)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_BUDDHA_FINGER)) {
        send_to_char("You'd better learn how to do it first.\r\n", ch);
        return;
    }

    if (GET_MANA(ch)<MOVE_LIMIT(ch))
    {
        send_to_char("You are too exhausted to even try that now!\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Buddha finger whom?\r\n", ch);
            return;
        }
    }

    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    if (number(1, 111)>GET_SKILL(ch, SKILL_BUDDHA_FINGER))
        damage(ch, vict, 0, SKILL_BUDDHA_FINGER, NULL);
    else {

        
        damage(ch, vict, GET_MANA(ch), SKILL_BUDDHA_FINGER, NULL);
        
        improve_skill(ch, SKILL_BUDDHA_FINGER, 1);
        
        //GET_MOVE(ch)/=2;
        GET_MANA(ch)=0;
    }
}


ACMD(do_melee)
{
    struct char_data *tch, *next_tch;
    int flag=0;
    int kk=0;
    struct affected_type af;
    int prob=0;  
    int dam=0;


    if (!GET_SKILL(ch, SKILL_MELEE))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (!FIGHTING(ch))
    {
        send_to_char("You aren't even in a fight.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<25)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
        send_to_char("This room is too peaceful for such an uncivilized behaviour.\r\n", ch);
        return;
    }

    GET_MOVE(ch)-=25;
    send_to_char("&wYou spread your arms and rush forward, sweeping everything!&0\r\n\r\n", ch);
    act("$n spreads $s arms and rushes forward, sweeping everything!", FALSE, ch, 0, 0, TO_ROOM);
    /*if (number(1, 161)<GET_SKILL(ch, SKILL_MELEE)+GET_STR(ch)+GET_DEX(ch))
    {
        
        prob=1;
    } */

    for (tch = world[ch->in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (tch == ch)
            continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
            continue;
        if (!CAN_MURDER(ch, tch))
            continue;
        if (is_same_group(ch, tch) || MOB_FLAGGED(tch, MOB_NOBASH))
            continue;
        if (!CAN_SEE(ch, tch) )
            continue;
        if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
            continue;
        if (!(FIGHTING(tch)==ch) && !(IS_AFFECTED(tch, AFF_WATERWALK) || IS_AFFECTED(tch, AFF_FLYING)))
            continue;
        if (number(1, 111)<GET_SKILL(ch, SKILL_MELEE)+2 * (GET_STR(ch) - GET_DEX(tch))+(GET_TOTAL_WEIGHT(ch)-GET_TOTAL_WEIGHT(tch))/20){
		
            WAIT_STATE(tch, PULSE_VIOLENCE);
            if (tch) {
                send_to_char("The blow sends you to the ground!\r\n", tch);
                act("$n is smashed to the ground!", FALSE, tch, 0, 0, TO_ROOM);
                GET_POS(tch) = POS_SITTING;
                WAIT_STATE(tch, PULSE_VIOLENCE/2);
                assign_stand_event(tch, PULSE_VIOLENCE);
            } 
             
            dam=GET_DAMROLL(ch)+2*GET_STR(ch)+GET_TOTAL_WEIGHT(ch)/32+number(1, 10);
            improve_skill(ch, SKILL_MELEE, 3);
            damage(ch, tch, dam, SKILL_MELEE, NULL);
	    check_fight(ch, tch);            
            
        }
        else
        {
            act("$N manages to evade you.", FALSE, ch, 0, tch, TO_CHAR);
            act("You manage to evade $n.", FALSE, ch, 0, tch, TO_VICT);
            act("$N manages to evade $n.", FALSE, ch, 0, tch, TO_NOTVICT);
        }

    }
    WAIT_STATE(ch, 5*PULSE_VIOLENCE /2);
}

ACMD(do_elbow)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    if (!GET_SKILL(ch, SKILL_ELBOW2)) {
        send_to_char("You are not familiar with that skill.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Elbow whom?\r\n", ch);
            return;
        }
    }


    if (GET_MOVE(ch)<2)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }


    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }
    percent = number(1, 111);
    prob = GET_SKILL(ch, SKILL_ELBOW2)+GET_LEVEL(ch)-GET_LEVEL(vict);
    GET_MOVE(ch)-=2;
    if (percent > prob) {
        damage(ch, vict, 0, SKILL_ELBOW2, NULL);
    } else {
        damage(ch, vict, 2*GET_DAMROLL(ch)+number(GET_LEVEL(ch), 2*GET_LEVEL(ch)), SKILL_ELBOW2, NULL);
        improve_skill(ch, SKILL_ELBOW2, 5);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_knee)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    if (!GET_SKILL(ch, SKILL_KNEE2)) {
        send_to_char("You are not familiar with that skill.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Knee thrust whom?\r\n", ch);
            return;
        }
    }

    if (GET_MOVE(ch)<3)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }
    percent = number(1, 111);
    prob = GET_SKILL(ch, SKILL_KNEE2)+GET_LEVEL(ch)-GET_LEVEL(vict);

    GET_MOVE(ch)-=3;
    if (percent > prob) {
        damage(ch, vict, 0, SKILL_KNEE2, NULL);
    } else {
        damage(ch, vict, 3*GET_DAMROLL(ch)+number(GET_LEVEL(ch), 3*GET_LEVEL(ch)), SKILL_KNEE2, NULL);
        improve_skill(ch, SKILL_KNEE2, 5);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_sweepkick)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    if (!GET_SKILL(ch, SKILL_SWEEPKICK)) {
        send_to_char("You are not familiar with that skill.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Sweepkick whom?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }


    if (IS_AFFECTED(vict, AFF_WATERWALK) || IS_AFFECTED(vict, AFF_FLYING))
    {
        send_to_char("That person is flying in the air.\r\n",ch);
        return;
    }


    if (GET_POS(vict) == POS_SITTING) {
        send_to_char("Your victim is already on the ground!\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<4)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    percent =number(1, 111);

    prob = GET_SKILL(ch, SKILL_SWEEPKICK) + 4 * (GET_DEX(ch) - GET_DEX(vict))-dex_app[GET_DEX(ch)].reaction;
    GET_MOVE(ch)-=4;
    if (percent > prob) {
        damage(ch, vict, 0, SKILL_SWEEPKICK, NULL);
        GET_POS(ch) = POS_SITTING;
        WAIT_STATE(ch, PULSE_VIOLENCE/2);
        assign_stand_event(ch, PULSE_VIOLENCE);
    } else {
        WAIT_STATE(vict, PULSE_VIOLENCE/2);
        GET_POS(vict) = POS_SITTING;
        assign_stand_event(vict, PULSE_VIOLENCE);
        damage(ch, vict, 4*GET_DAMROLL(ch)+number(1, GET_LEVEL(ch)), SKILL_SWEEPKICK, NULL);
        improve_skill(ch, SKILL_SWEEPKICK, 5);
        WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }

}


ACMD(do_combo)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    if (!GET_SKILL(ch, SKILL_COMBO)) {
        send_to_char("You are not familiar with that skill.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Combo whom?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<8)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    percent =number(1, 150);

    prob = GET_SKILL(ch, SKILL_COMBO) + 5 * (GET_DEX(ch) - GET_DEX(vict));
    GET_MOVE(ch)-=8;
    if (percent > prob) {
        damage(ch, vict, 0, SKILL_SWEEPKICK, NULL);
        GET_POS(ch) = POS_SITTING;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        assign_stand_event(ch, PULSE_VIOLENCE*2);
    } else {
        //    CREF(vict, CHAR_NULL);
        if (number(1, 111)<GET_SKILL(ch, SKILL_SWEEPKICK)) {
            damage(ch, vict, 4*GET_DAMROLL(ch)+number(1, GET_LEVEL(ch)), SKILL_SWEEPKICK, NULL);
            improve_skill(ch, SKILL_SWEEPKICK, 5);
            if (!DEAD(vict))
            {
                GET_POS(vict) = POS_SITTING;
                WAIT_STATE(vict, PULSE_VIOLENCE/2);
                assign_stand_event(vict, PULSE_VIOLENCE);
            }
        }
        if (!DEAD(vict) && vict->in_room==ch->in_room && (number(1, 111)<GET_SKILL(ch, SKILL_ELBOW2)))
        {
            damage(ch, vict, 2*GET_DAMROLL(ch)+number(GET_LEVEL(ch), 2*GET_LEVEL(ch)), SKILL_ELBOW2, NULL);
            improve_skill(ch, SKILL_ELBOW2, 5);
        }
        if (!DEAD(vict) && vict->in_room==ch->in_room && (number(1, 111)<GET_SKILL(ch, SKILL_KNEE2)))
        {
            damage(ch, vict, 3*GET_DAMROLL(ch)+number(GET_LEVEL(ch), 3*GET_LEVEL(ch)), SKILL_KNEE2, NULL);
            improve_skill(ch, SKILL_KNEE2, 5);

        }
        //  CUREF(vict);
        improve_skill(ch, SKILL_COMBO, 5);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_scare)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    if (!GET_SKILL(ch, SKILL_SCARE)) {
        send_to_char("You scare yourself.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        {
            send_to_char("Scare whom?\r\n", ch);
            return;
        }
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }


    if (GET_MOVE(ch)<10)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Scare whom?\r\n", ch);
            return;
        }
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You only manage to look silly.\r\n", ch);
        return;
    }
    percent = number(1, 111);
    if (FIGHTING(vict))
        percent+=25;
    prob = GET_SKILL(ch, SKILL_SCARE);
    if (IS_SHOPKEEPER(vict))//MOB_FLAGGED(vict, MOB_SPEC))
        prob=0;
    act("You suddenly jump towards $N waving your hands and screaming madly.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n suddenly jumps towards you waving $s hands and screaming madly.", FALSE, ch, 0, vict, TO_VICT);
    act("$n suddenly jumps towards $N waving $s hands and screaming madly.", FALSE, ch, 0, vict, TO_NOTVICT);

    GET_MOVE(ch)-=10;
    if (percent > prob || (WIL_CHECK(vict) && WIL_CHECK(vict))) {
        if (!number(0, 4) && IS_NPC(vict))
        {
            act("$n exclaims, 'Scaring me? Take this punk!'", FALSE, vict, 0, 0, TO_ROOM);
            hit(vict, ch, TYPE_UNDEFINED);
        }
        else if (!number(0, 3) && IS_NPC(vict))
        {
            act("$N falls down laughing.", FALSE, ch, 0, vict, TO_CHAR);
            act("$N falls down laughing.", FALSE, ch, 0, vict, TO_NOTVICT);
        }
        else if (!number(0, 2))
        {
            act("$N sighs.", FALSE, ch, 0, vict, TO_CHAR);
            act("$N sighs.", FALSE, ch, 0, vict, TO_NOTVICT);
        }
        else if (!number(0, 1))
        {
            act("$N rises an eyebrow.", FALSE, ch, 0, vict, TO_CHAR);
            act("$N rises an eyebrow.", FALSE, ch, 0, vict, TO_NOTVICT);
        }
        return;
    } else
    {

        act("$n screams in panic!", FALSE, vict, 0, 0, TO_ROOM);
        do_flee(vict, "", 0, 99);
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);
}



ACMD(do_kickflip)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;
    int dam=0;
    one_argument(argument, arg);


    if (!GET_SKILL(ch, SKILL_KICKFLIP)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    if (!(obj = GET_EQ(ch, WEAR_FEET))) {
        send_to_char("You need some kind of footwear to make it a success.\r\n", ch);
        return;
    }

    if (GET_MOVE(ch)<2)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Kickflip who?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }

    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    percent = number(1, 111);	/* 109% is a complete failure */
    prob = GET_SKILL(ch, SKILL_KICKFLIP)  +  3*(GET_DEX(ch) - GET_DEX(vict)) - GET_TOTAL_WEIGHT(vict)/8;

    if (MOB_FLAGGED(vict, MOB_NOBASH))
        percent = 131;


    GET_MOVE(ch)-=2;

    dam=GET_DAMROLL(ch);
    if (obj && (GET_OBJ_TYPE(obj) == ITEM_WEAPON))
        dam += dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2)) + GET_OBJ_VAL(obj, 0)+dice(3, GET_LEVEL(ch));

    if (percent > prob || !STR_CHECK(ch)) {
        GET_POS(ch) = POS_SITTING;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        assign_stand_event(ch, PULSE_VIOLENCE*2);
        damage(ch, vict, 0, SKILL_KICKFLIP, obj);
        GET_HIT(ch) -= 5;
    } else {
        GET_POS(vict) = POS_SITTING;
        WAIT_STATE(vict, PULSE_VIOLENCE/2);
        WAIT_STATE(ch, 2*PULSE_VIOLENCE);
        assign_stand_event(vict, PULSE_VIOLENCE);
        damage(ch, vict, dam, SKILL_KICKFLIP, obj);
        improve_skill(ch, SKILL_KICKFLIP, 5);
    }


}



ACMD(do_knockout)
{
    struct char_data *vict;
    int percent, prob;
    struct affected_type af;

    one_argument(argument, buf);

    if (!GET_SKILL(ch, SKILL_KNOCKOUT)) {
        send_to_char("You don't know how!\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }


    if (!(vict = get_char_room_vis(ch, buf))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Knockout who?\r\n", ch);
            return;
        }
    }


    if (!GET_EQ(ch, WEAR_WIELD)) {
        send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
        return;
    }

    if (FIGHTING(vict)) {
        send_to_char("You can't knock out a fighting person -- they're too alert!\r\n", ch);
        return;
    }
    if (!AWAKE(vict)) {
        send_to_char("Ther are already knocked down!\r\n", ch);
        return;
    }



    if (vict == ch) {
        send_to_char("Well, OK. *WHAAAP*\r\n", ch);
        act("$n knocks $mself down, gee.", FALSE, ch, 0, 0, TO_ROOM);
        af.bitvector = AFF_SLEEP;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        af.type = SPELL_SLEEP;
        af.location=0;
        af.modifier=0;
        af.duration = 0;
        affect_to_char(vict, &af);
        GET_POS(vict)=POS_SLEEPING;


        return;
    }


    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }


    if (MOB_FLAGGED(vict, MOB_AWARE) || MOB_FLAGGED(vict, MOB_NOSLEEP)) {
        act("You notice $N raising $s weapon at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you raising your weapon!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N raising $s weapon at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        //hit(vict, ch, TYPE_UNDEFINED);
        check_fight(vict, ch);
        return;
    }

    percent = number(1, 111);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_KNOCKOUT);
    if (percent<prob && (!WIL_CHECK(vict) || !INT_CHECK(vict)))
    {
        act("You are knocked out when $N hits you upside your head.", FALSE, vict, 0, vict, TO_CHAR);
        act("$n sees stars, and slumps over, knocked out.", FALSE, vict, 0, ch, TO_VICT);
        act("$n sees stars, and slumps over, knocked out, after $N brains $m.", FALSE, vict, 0, ch, TO_NOTVICT);

        /*af.bitvector = AFF_SLEEP;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        af.type = SPELL_SLEEP;
        af.duration = 0;	
        af.location=0;
        af.modifier=0;
        affect_to_char(vict, &af);    */
        GET_POS(vict)=POS_STUNNED;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;

    }

    act("You notice $N raising $s weapon at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you raising your weapon!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N raising $s weapon at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    //hit( vict, ch, TYPE_UNDEFINED);
    check_fight(vict, ch);
    WAIT_STATE(ch, PULSE_VIOLENCE);


}




ACMD(do_gut)
{
    struct char_data *vict;
    int percent, prob, healthpercent;
    struct obj_data *piece;

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_GUT)) {
        send_to_char("You don't know how!\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Gut who?\r\n", ch);
            return;
        }
    }

    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }

    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }
    if (!GET_EQ(ch, WEAR_WIELD)) {
        send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
        return;
    }
    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT) {
        send_to_char("Only slashing weapons can be used for gutting.\r\n", ch);
        return;
    }
    percent = number(1, 111);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_GUT);

    if (GET_MAX_HIT(vict) > 0)
        healthpercent = (100 * GET_HIT(vict)) / GET_MAX_HIT(vict);
    else
        healthpercent = -1;

    if (healthpercent >= 6+GET_SKILL(ch, SKILL_GUT)/10) {
        send_to_char("They are not hurt enough for you to attempt that.\r\n", ch);
        //hit(vict, ch, TYPE_UNDEFINED);
        WAIT_STATE(ch, PULSE_VIOLENCE );
        return;
    }

    if (percent > prob) {
        sprintf(buf, "Even in %s's bad state, they manage to avoid your wild slash.\r\n", GET_NAME(vict));
        send_to_char(buf, ch);
        send_to_char("You avoid a wild slash at your midsection.\r\n", vict);
    } else {

        /* EWWWW */
        GET_HIT(vict) = -3;

        improve_skill(ch, SKILL_GUT, 1);
        act("&cYou gut $N!&0", FALSE, ch, 0, vict, TO_CHAR);
        act("$N guts you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$n brutally guts $N!", FALSE, ch, 0, vict, TO_NOTVICT);

        act("$n looks down in horror as $s intestines spill out!", FALSE, vict, 0, 0, TO_ROOM);

        piece = read_object(21, VIRTUAL, 0, GET_LEVEL(vict));
        sprintf(buf, "An icky pile of intestines once belonging to %s", GET_NAME(vict));
        piece->name = str_dup("intestine");
        piece->short_description = str_dup(buf);
        sprintf(buf2, "An eye of %s is lying here.", GET_NAME(vict));
        piece->description = str_dup("An icky pile of intestines is here - colon and all.");



        obj_to_room(piece, ch->in_room);


        update_pos(vict);
        if ((FIGHTING(ch)==vict) && (FIGHTING(vict)==ch))
        {
            stop_fighting(ch);
            stop_fighting(vict);
            /*act("\r\nYou stare at $N, laughing mercilessly! MUHAHAHA!!!", FALSE, ch, 0, vict, TO_CHAR);
            act("$n stares at $N, laughing mercilessly!!!", FALSE, ch, 0, vict, TO_NOTVICT);
            act("$n stares at you, laughing mercilessly!!!", FALSE, ch, 0, vict, TO_VICT);*/
        }
        else
            hit(ch, vict, TYPE_UNDEFINED);
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }

}




ACMD(do_taunt)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    if (!GET_SKILL(ch, SKILL_TAUNT)) {
        send_to_char("You taunt yourself.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        {
            send_to_char("Taunt whom?\r\n", ch);
            return;
        }
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }


    if (GET_MOVE(ch)<20)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {

        send_to_char("Taunt whom?\r\n", ch);
        return;

    }


    if (!IS_NPC(vict)) {
        send_to_char("You only manage to look silly.\r\n", ch);
        return;
    }

    if (FIGHTING(vict)) {
        send_to_char("You can not taunt a fighting person.\r\n", ch);
        return;
    }

    percent = number(1, 111);
    prob = GET_SKILL(ch, SKILL_TAUNT);
    //if (MOB_FLAGGED(vict, MOB_SENTINEL))
    //  prob=0;

    GET_MOVE(ch)-=20;
    if (percent > prob || IS_SHOPKEEPER(vict) /*MOB_FLAGGED(vict, MOB_SPEC)*/ || (GET_LEVEL(vict)>GET_LEVEL(ch)-5 && (WIL_CHECK(vict)))) {
        if (!number(0, 4) && CAN_SEE(vict, ch))
        {
            act("$n exclaims, 'Taunt me? Take this!'", FALSE, vict, 0, 0, TO_ROOM);
            hit(vict, ch, TYPE_UNDEFINED);

        }
        else
            act("$N seems to be ignoring you.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    } else
    {

        struct char_data *opponent,
                    *opp = NULL;
        SET_BIT(MOB_FLAGS(vict), MOB_AGGRESSIVE);
        act("You taunt $N.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n taunts $N.", FALSE, ch, 0, vict, TO_NOTVICT);
        act("$n taunts you.", FALSE, ch, 0, vict, TO_VICT);

        for (opponent = world[ch->in_room].people; opponent; opponent = opponent->next_in_room)
            if (CAN_SEE(vict, opponent) && !IS_SHOPKEEPER(opponent) /*!MOB_FLAGGED(opponent, MOB_SPEC)*/ && (opponent!=vict) && number(1, 100)>50)
                opp = opponent;
        if (opp)
        {
            act("$n growls in rage!", FALSE, vict, 0, 0, TO_ROOM);
            hit(vict, opp, TYPE_UNDEFINED);
        }

    }
    WAIT_STATE(ch, PULSE_VIOLENCE);
}


ACMD(do_dirtkick)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;
    int dam=0;
    one_argument(argument, arg);


    if (!GET_SKILL(ch, SKILL_DIRTKICK)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    /*if (!(obj = GET_EQ(ch, WEAR_FEET))) {
    	send_to_char("You need some kind of footwear to make it a success.\r\n", ch);
    	return;
        }
      */
    if (GET_MOVE(ch)<2)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Dirtkick who?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }

    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
            (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {
        send_to_char("There is no dirt in here!\r\n", ch);
        return;
    }

    percent = number(1, 111);	/* 109% is a complete failure */
    prob = GET_SKILL(ch, SKILL_DIRTKICK);

    if (MOB_FLAGGED(vict, MOB_NOBLIND))// || MOB_FLAGGED(vict, MOB_AWARE))
        percent = 161;


    GET_MOVE(ch)-=2;

//    dam=GET_DAMROLL(ch)+dice(7, 7);

    if (percent > prob || !DEX_CHECK(ch)) {
        //GET_POS(ch) = POS_SITTING;
        //damage(ch, vict, 0, SKILL_DIRTKICK, obj);
        act("$N skillfully avoids your dirtkick.", FALSE, ch, 0, vict, TO_CHAR);
        act("You skillfully avoid $n's dirtkick.", FALSE, ch, 0, vict, TO_VICT);
        act("$N skillfully avoids $n's dirtkick.", FALSE, ch, 0, vict, TO_NOTVICT);

        //GET_HIT(ch) -= 5;
        //assign_stand_event(ch, PULSE_VIOLENCE*2);

    } else {
        SET_BIT(AFF3_FLAGS(vict), AFF3_TEMP_BLIND);
        WAIT_STATE(vict, PULSE_VIOLENCE);
        act("&cYou send some dirt right into $N's eyes!&0", FALSE, ch, 0, vict, TO_CHAR);
        act("$n sends some dirt right into your eyes!", FALSE, ch, 0, vict, TO_VICT);
        act("$n sends some dirt right into $N's eyes!", FALSE, ch, 0, vict, TO_NOTVICT);
        //damage(ch, vict, dam, SKILL_DIRTKICK, obj);
        improve_skill(ch, SKILL_DIRTKICK, 3);
    }
    WAIT_STATE(ch, 2*PULSE_VIOLENCE);
    check_fight(ch, vict);

}



ACMD(do_ambush)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;
    int dam=0;
    one_argument(argument, arg);


    if (!GET_SKILL(ch, SKILL_AMBUSH)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }


    if (GET_MOVE(ch)<30)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }


    /*
      if (!CAN_MURDER(ch, vict)) {
    	send_to_char("You are not allowed to attack that person.\r\n", ch);
    	return;
        }
      */
    if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER)) {
        send_to_char("You can hardly ambush anyone here!\r\n", ch);
        return;
    }

    if (IS_THIEF(ch) && SECT(ch->in_room)==SECT_FOREST)
    {
        send_to_char("You can not make ambushes in the forest.\r\n", ch);
        return;
    }
    if (IS_RANGER(ch) && SECT(ch->in_room)==SECT_CITY)
    {
        send_to_char("You can not make ambushes in the city.\r\n", ch);
        return;
    }


    //my_srandp(GET_ROOM_VNUM(ch->in_room));
    percent = number(1, 111);
    //my_srand(time(0));

    prob = GET_SKILL(ch, SKILL_AMBUSH);
    GET_MOVE(ch)-=30;
    if (percent>prob)
    {
        send_to_char("You fail.\r\n", ch);
    }
    else
    {
        if (!*arg)
        {
            send_to_char("&pYou hide in the shadows.&0\r\n", ch);
            if (ch->ambush_name)
                DISPOSE(ch->ambush_name);
            ch->ambush_name=NULL;
        }

        else
        {
            sprintf(buf, "Setting ambush to anyone matching '%s'&0\r\n", arg);
            send_to_char(buf, ch);
            send_to_char("&pYou hide in the shadows.&0\r\n", ch);
            
            if (ch->ambush_name)
                DISPOSE(ch->ambush_name);
            ch->ambush_name=str_dup(arg);
        }
        SET_BIT(AFF3_FLAGS(ch), AFF3_AMBUSH);
      improve_skill(ch, SKILL_AMBUSH, 1);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE );
}



ACMD(do_daggerdance)
{
    struct char_data *tch, *next_tch;
    int flag=0;
    int kk=0, old;
    struct affected_type af;


    if (!GET_SKILL(ch, SKILL_DANCEOFDAGGERS))
    {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }


    if (GET_MOVE(ch)<10)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
        send_to_char("This room is too peaceful.\r\n", ch);
        return;
    }

    if (!GET_EQ(ch, WEAR_WIELD) || !GET_EQ(ch, WEAR_DUALWIELD)) {
        send_to_char("You need to wield daggers in both of your hands for balance.\r\n", ch);
        return;
    }

    if ((GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)  || (GET_OBJ_VAL(GET_EQ(ch, WEAR_DUALWIELD), 3) != TYPE_PIERCE - TYPE_HIT))
    {
        send_to_char("You need to wield two daggers!.\r\n", ch);
        return;
    }


    GET_MOVE(ch)-=10;

    if (number(1,101)>GET_SKILL(ch, SKILL_DANCEOFDAGGERS))
    {
        if  (!DEX_CHECK(ch))
        {
            send_to_char("You loose your balance and fall to the ground!\r\n", ch);
            act("$n tries to perform something. but looses $s balance and falls to the ground.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            WAIT_STATE(ch, PULSE_VIOLENCE);
            assign_stand_event(ch, PULSE_VIOLENCE*2);
        }
        else
        {
            send_to_char("You fail but manage to maintain balance.\r\n", ch);
            WAIT_STATE(ch, PULSE_VIOLENCE);
        }

        return;
    }


    for (tch = world[ch->in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (tch == ch)
            continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
            continue;
        if (!CAN_MURDER(ch, tch))
            continue;
        if (is_same_group(ch, tch))
            continue;
        if (!CAN_SEE(ch, tch) && number(0, 2))
            continue;
        if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
            continue;
        if (flag==0)
        {
            send_to_char("&wYour start your deadly dagger dance!!&0\r\n\r\n", ch);
            act("$n performs a deadly dance of daggers!!", FALSE, ch, 0, 0, TO_ROOM);
            flag=1;
            improve_skill(ch, SKILL_DANCEOFDAGGERS, 2);
        }

        //        CREF(tch, CHAR_NULL);

        hit(ch, tch, TYPE_UNDEFINED);
        while (number(1, 301)<GET_SKILL(ch, TYPE_PIERCE) && !DEAD(ch) && !DEAD(tch) && tch->in_room==ch->in_room && GET_MOVE(ch)>7)
        {
            hit(ch, tch, TYPE_UNDEFINED);
            GET_MOVE(ch)-=8;
        }
        if (!DEAD(ch) && !DEAD(tch) && tch->in_room==ch->in_room)
            hit(ch, tch, SKILL_DUAL_WIELD);
        if (!DEAD(ch) && !DEAD(tch) && tch->in_room==ch->in_room)
            while (number(1, 401)<GET_SKILL(ch, TYPE_PIERCE) && !DEAD(ch) && !DEAD(tch) && tch->in_room==ch->in_room && GET_MOVE(ch)>7)
            {
                hit(ch, tch, SKILL_DUAL_WIELD);
                GET_MOVE(ch)-=8;
            }
        //      CUREF(tch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}



ACMD(do_headbutt)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;
    int dam;

    if (!GET_SKILL(ch, SKILL_HEADBUTT)) {
        send_to_char("You are not familiar with that skill.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }
    if (!GET_EQ(ch, WEAR_HEAD)) {
        send_to_char("Are you nuts ?  Wear a helmet!\r\n", ch);
        return;
    }
    obj = GET_EQ(ch, WEAR_HEAD);

    if (GET_MOVE(ch)<4)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }



    one_argument(argument, arg);

    if (!(vict = get_char_room_vis(ch, arg))) {
        if (FIGHTING(ch)) {
            vict = FIGHTING(ch);
        } else {
            send_to_char("Headbutt who?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }
    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }
    percent = number(1, 111);	/* 101% is a complete
    					   failure */
    prob = GET_SKILL(ch, SKILL_HEADBUTT);
    GET_MOVE(ch)-=4;

    dam=2*GET_DAMROLL(ch)+dice(12, 13);
    dam+=32*GET_SKILL(ch, SKILL_HEADBUTT)/100;
//    if (GET_SKILL(ch, SKILL_HEADBUTT)>number(1, 1000))
  //      dam+=dam/2;
    if (obj && (GET_OBJ_TYPE(obj) == ITEM_WEAPON))
        dam += dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2)) + GET_OBJ_VAL(obj, 0);

    if (percent > prob || IS_GOD(vict)) {
        damage(ch, vict, 0, SKILL_HEADBUTT, obj);
    } else {
        damage(ch, vict, dam, SKILL_HEADBUTT, obj);
        improve_skill(ch, SKILL_HEADBUTT, 5);
    }
    WAIT_STATE(ch, 3*PULSE_VIOLENCE );
}







ACMD(do_lodge)
{
    struct char_data *vict, *tmp_ch;
    int percent, prob;

    one_argument(argument, arg);
    if (!GET_SKILL(ch, SKILL_LODGE))
    {
        send_to_char("You are not skilled enough to try that.\r\n", ch);
        return;
    }
    tmp_ch=FIGHTING(ch);
    if (!tmp_ch)
    {
        send_to_char("You aren't fighting anyone.\r\n", ch);
        return;
    }
    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("Lodge behind who?\r\n", ch);
        return;
    }
    if (vict == ch) {
        send_to_char("What about fleeing instead?\r\n", ch);
        return;
    }

    if (FIGHTING(ch) == vict) {
        send_to_char("That can hardly be performed.\r\n", ch);
        return;
    }

    if (!is_same_group(ch, vict))
    {
        send_to_char("You can lodge only behind your group members.\r\n", ch);
        return;
    }
    if (GET_MOVE(ch)<5)
    {
        send_to_char("You are too exhausted too even try that now.\r\n", ch);
        return;
    }



    {
        percent = number(1, 111);	/* 101% is a complete failure */
        prob = GET_SKILL(ch, SKILL_LODGE);

        if (percent > prob) {
            send_to_char("You fail to lodge.\r\n", ch);
            return;
        }
        improve_skill(ch, SKILL_LODGE, 1);
        send_to_char("Banzai! To safety...\r\n", ch);
        act("$n jumps behind $N!", FALSE, ch, 0, vict, TO_NOTVICT);
        act("You attack $N!", FALSE, tmp_ch, 0, vict, TO_CHAR);

        if (FIGHTING(tmp_ch))
            stop_fighting(tmp_ch);
        GET_MOVE(ch)-=5;
        set_fighting(tmp_ch, vict);

        WAIT_STATE(ch, PULSE_VIOLENCE);
    }

}

ACMD(do_cutthroat)
{
    struct char_data *vict;
    int percent, prob;
    struct obj_data *obj;

    struct affected_type af;

    one_argument(argument, arg);
    if (!GET_SKILL(ch, SKILL_CUTTHROAT)) {
        send_to_char("You'd better learn how to do it first.\r\n", ch);
        return;
    }

    if (!(obj = GET_EQ(ch, WEAR_WIELD))) {
        send_to_char("You need to wield a piercing weapon to make it a success.\r\n", ch);
        return;
    }
    if (GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT) {
        send_to_char("Only piercing weapons can be used for backstabing.\r\n", ch);
        return;
    }

    if (!(vict = get_char_room_vis(ch, arg))) {
        send_to_char("Cutthroat who?\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("Aren't we funny today...\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    if (!CAN_MURDER(ch, vict)) {
        send_to_char("You are not allowed to attack that person.\r\n", ch);
        return;
    }

    if (AWAKE(vict) && GET_HIT(vict) < GET_MAX_HIT(vict)/2 && CAN_SEE(vict, ch)) {
        act("$N is watching you suspiciously ... you can't sneak up.", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }

    if (FIGHTING(vict))
    {
        act("$N is to alert now!", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }

    if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        hit(vict, ch, TYPE_UNDEFINED);
        WAIT_STATE(ch, PULSE_VIOLENCE );
        return;
    }


    prob=GET_SKILL(ch, SKILL_CUTTHROAT);

    if (number(1,101)<prob && (DEX_CHECK(ch) || (!CAN_SEE(vict, ch) && !INT_CHECK(vict))))
    {
        act("&CYou sneak up and with a swift move cutthroat $N!&0", FALSE, ch, NULL, vict, TO_CHAR);
        act("$n sneaks up and cutthroats $N making a lot of blood!", FALSE, ch, NULL, vict, TO_NOTVICT);
        act("$n sneaks up and cutthroats YOU! Ahhhggghhh!", FALSE, ch, NULL, vict, TO_VICT);
//        RM_BLOOD(ch->in_room) =  MIN(RM_BLOOD(ch->in_room)+1, 10);
        world[ch->in_room].blood= MIN(RM_BLOOD(ch->in_room)+1, 10);
         

        improve_skill(ch, SKILL_CUTTHROAT, 2);
        if (GET_LEVEL(ch)-GET_LEVEL(vict)>10 && IS_NPC(vict))
        {
            damage(ch, vict, GET_MAX_HIT(vict)*4, SKILL_CUTTHROAT, obj);
        }
        else {
            GET_HIT(vict)-=dice(GET_SKILL(ch, SKILL_CUTTHROAT)/2, 4);
            af.bitvector = 0;
            af.bitvector2 = 0;
            af.bitvector3 = AFF3_CHOKE;
            af.type = SPELL_CHOKE;
            af.duration = 1;
            af.location = APPLY_NONE;
            af.modifier = 0;
            affect_to_char(vict, &af);
            WAIT_STATE(vict, 2*PULSE_VIOLENCE);
            GET_HIT(vict)=MAX(-10-dice(1, 5), GET_HIT(vict));
        }
        WAIT_STATE(ch, PULSE_VIOLENCE );
        check_fight(ch, vict);
    }
    else
    {
        act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
        act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
        act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
        hit(vict, ch, TYPE_UNDEFINED);
        WAIT_STATE(ch, PULSE_VIOLENCE );
    }
}
