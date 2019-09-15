/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include "auction.h"
#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "class.h"
#include "objs.h"
#include "clan.h"
#include "rooms.h"
#include "interpreter.h"

#define READ_TITLE(ch) 	titles1[0].title

extern unsigned int   pulse;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles1[LVL_IMPL + 1];
extern struct room_data *world;
extern int      max_exp_gain;
extern int      max_exp_loss;
extern int      has_boat(struct char_data * ch);
int clim;
/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int             graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

    if (age < 15)
        return (p0);            /* < 15   */
    else if (age <= 29)
        return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));    /* 15..29 */
    else if (age <= 44)
        return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));    /* 30..44 */
    else if (age <= 59)
        return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));    /* 45..59 */
    else if (age <= 79)
        return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));    /* 60..79 */
    else
        return (p6);            /* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
float             mana_gain(struct char_data * ch)
{
    int             gain;

    if (IS_NPC(ch)) {
        /* Neat and fast */

        return(GET_LEVEL(ch));
    } else {
        /*        // gain = graf(age(ch).year, 4, 8, 12, 16, 12, 10, 8);*/

        /* Class calculations */
        //gain = (float) (GET_INTR(ch)+GET_INT(ch))*GET_LEVEL(ch)/48.0+0.5;

        gain=GET_MAX_MANA(ch)/20.0+1;//GET_CONR(ch)*GET_LEVEL(ch)/16.0+0.5;

        gain+=ch->regen;
        
        
        /* Skill/Spell calculations */
        if (IS_SET(AFF2_FLAGS(ch), AFF2_ENH_MANA))
            gain += gain/5; 
            
            
       if ( FOL_VALERIA(ch))
           gain += gain/8+1;      
        /* Position calculations    */
        switch (GET_POS(ch)) {
        case POS_SLEEPING:
            gain *= 3.0;
            if (AFF2_FLAGGED(ch, AFF2_NAP))
                gain *= 1.5;
            break;
        case POS_RESTING:
            gain *= 1.6;       /* Divide by 4 */
            break;
        case POS_SITTING:
            gain *=1.2;   /* Divide by 4 */
            break;    
        case POS_FIGHTING:
            gain *=0.25;   /* Divide by 4 */
            break;    
        }



        /*//  if (IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_WIZARD(ch) || IS_AVATAR(ch))*/
        /*

          if (AFF2_FLAGGED(ch,AFF2_REGENERATE))
             gain<<=1;
        */

        if (AFF2_FLAGGED(ch, AFF2_MEDITATE))
            gain+=gain*GET_SKILL(ch, SKILL_MEDITATE)/70.0;



        if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
            gain /= 16;

        if (IS_AFFECTED(ch, AFF_POISON) && GET_RACE(ch)!=RACE_TROLL)
            gain =GET_INTR(ch)/5;

        return (gain>0.4 ? MAX(gain, 1) : 0);
    }
}



float             hit_gain(struct char_data * ch)
/* Hitpoint gain pr. game hour */
{
    float             gain;

    if (IS_NPC(ch)) {
        gain = GET_MAX_HIT(ch)/(FIGHTING(ch)? 48:12);
        if (MOB_FLAGGED(ch, MOB_FASTREGEN))
            gain *=2;
        if (IS_AFFECTED(ch, AFF_POISON))
            gain /=4;

        return ((int) gain+1);
        /* Neat and fast */
    } else {

        if (FOL_MUGRAK(ch))
        {	
        	if (ch->in_room==real_room(DEATHR))
        		return 0;
        	else if (GET_HIT(ch)<=GET_LEVEL(ch))
        		return 0;
        	else
        		return -(MAX(1, GET_LEVEL(ch)/2));
        }
        //gain = (float) (GET_CONR(ch)+GET_CON(ch))*GET_LEVEL(ch)/48.0+0.5;
        gain=GET_MAX_HIT(ch)/20.0+1;//GET_CONR(ch)*GET_LEVEL(ch)/16.0+0.5;
        gain+=ch->regen;


        /* Position calculations    */

        if (IS_SET(AFF2_FLAGS(ch), AFF2_ENH_HEAL))
            gain += gain/5;

        if (GET_SKILL(ch, SKILL_FASTHEALING))
            gain+=gain*GET_SKILL(ch, SKILL_FASTHEALING)/256;
            
            
        if (AFF2_FLAGGED(ch, AFF2_REGENERATE))
            gain += gain/5;          
           
       if ( FOL_VALERIA(ch))
           gain += gain/8+1;          


        switch (GET_POS(ch)) {
        case POS_SLEEPING:
            gain *= 3.0;

            if (AFF2_FLAGGED(ch, AFF2_NAP))
                gain *= 1.25;
            break;
        case POS_RESTING:
            gain *= 1.5;       /* Divide by 4 */
            break;
        case POS_SITTING:
            gain *=1.3;   /* Divide by 4 */
            break;
        case POS_FIGHTING:
            gain *=0.25;   /* Divide by 4 */
            break;    
        default:
        	break;
        }

        

     
        /*    // If (IS_WARRIOR(ch) || IS_THIEF(ch) || IS_MONK (ch))*/



        if (GET_RACE(ch)==RACE_TROLL)
            gain*=1.33;

        if (AFF2_FLAGGED(ch, AFF2_MEDITATE))
            gain+=gain*GET_SKILL(ch, SKILL_MEDITATE)/150.0;


        /*if (GET_WIS(ch)>16)
            gain*=GET_WIS(ch)/16.0;
        else if (GET_WIS(ch)<11)
            gain*=(GET_WIS(ch)/11.0+1)/2;
*/

        if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
            gain  /=16;

        if (IS_AFFECTED(ch, AFF_POISON) && GET_RACE(ch)!=RACE_TROLL)
            gain =GET_CON(ch)/5;

	
        return (gain>0.4 ? MAX(gain, 1) : 0);
    }
}

float             move_gain(struct char_data * ch)
/* move gain pr. game hour */
{
    int             gain;

    if (IS_NPC(ch)) {
        return (GET_LEVEL(ch));
        /* Neat and fast */
    } else {
        /*//    gain = graf(age(ch).year, 16, 20, 24, 20, 16, 12, 10);*/

        //gain = (GET_DEXR(ch)+GET_DEX(ch))/2+(GET_CONR(ch)+GET_CONR(ch))/4;
        gain = GET_DEX(ch);
        gain+=ch->regen;
        if (IS_SET(AFF2_FLAGS(ch), AFF2_ENH_MOVE))
            gain += gain/5;  
            
            
        if ( FOL_VALERIA(ch))
           gain += gain/8+1;      
        /* Position calculations    */
        switch (GET_POS(ch)) {
        case POS_SLEEPING:
            gain *= 3.0;
            if (AFF2_FLAGGED(ch, AFF2_NAP))
                gain *= 1.5;
            break;
        case POS_RESTING:
            gain *= 1.8;       /* Divide by 4 */
            break;
        case POS_SITTING:
            gain *= 1.5;   /* Divide by 4 */
            break;    
            case POS_FIGHTING:
            gain *=0.25;   /* Divide by 4 */
            break;    
        }



        /* if (AFF2_FLAGGED(ch,AFF2_REGENERATE)) gain>>=1; */

        if (AFF2_FLAGGED(ch, AFF2_MEDITATE))
            gain+=gain*GET_SKILL(ch, SKILL_MEDITATE)/70.0;

        if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
            gain /= 16;

        if (IS_AFFECTED(ch, AFF_POISON) && GET_RACE(ch)!=RACE_TROLL)
            gain =GET_DEXR(ch)/5;

        return (gain>0.4 ? MAX(gain, 1) : 0);
    }
}


void            set_title(struct char_data * ch, char *title)
{
    if (title == NULL)
        title = READ_TITLE(ch);

    if (strlen(title) > MAX_TITLE_LENGTH)
        title[MAX_TITLE_LENGTH] = '\0';

    if (GET_TITLE(ch) != NULL)
        DISPOSE(GET_TITLE(ch));

    GET_TITLE(ch) = str_dup(title);
}


void            check_autowiz(struct char_data * ch)
{
    char            buf[100];
    extern int      use_autowiz;
    extern int      min_wizlist_lev;
    pid_t           getpid(void);

    if (use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT) {
        sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
                WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
        mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
        system(buf);
    }
}

void            gain_exp(struct char_data * ch, int gain)
{
    int             is_altered = FALSE;
    int             num_levels = 0;
    char            buf[128];
    int             target_xp;

    if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
        return;

    if (!IS_NPC(ch)) {
        if (gain > 0) {
            GET_EXP(ch) += MIN(max_exp_gain, (int) (gain / GET_NUM_OF_CLASS(ch)));

            if (GET_LEVEL(ch) < (LVL_IMMORT - 1)) {     /* able to gain */
                //target_xp = total_exp(GET_LEVEL(ch));
                target_xp = LEVELEXP(ch);
                if (GET_EXP(ch) >= target_xp) { /* advance one level */
                    GET_LEVEL(ch) += 1;
                    clan[GET_CLAN(ch)].power++;
                    send_to_char("\r\n                     [&W&f  You raise a level!&0  ]\r\n", ch);
                    advance_level(ch);
                    //GET_EXP(ch) = total_exp(GET_LEVEL(ch)-1)+1;
                    GET_EXP(ch) = 1;
                    /*                    if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
                                            set_title(ch, NULL);
                    */
                    sprintf(buf, "\r\n&G[Info]:  &Y%s gained a level.&0\r\n", GET_NAME(ch));
                    INFO_OUT(buf);
                    if (!PRF_FLAGGED(ch, PRF_NOGRATZ))
                        GRATS_OUT(buf, ch);
                }
            } else {            /* immort */
                //target_xp = total_exp(GET_LEVEL(ch));
                target_xp = LEVELEXP(ch);

                if ( /* (GET_REINCARN(ch) >= 4) && */ (GET_EXP(ch) >= target_xp)) {
                    GET_LEVEL(ch) += 1;
                    advance_level(ch);
                    send_to_char("\r\r\n\n                        *** Welcome to Immortality! *** \r\r\n\r\n\n", ch);
                    /*                    if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
                                            set_title(ch, NULL);*/
                    sprintf(buf, "\r\n[Info]:  %s meets Immortality!!!", GET_NAME(ch));
                    INFO_OUT(buf);
                    if (!PRF_FLAGGED(ch, PRF_NOGRATZ))
                        GRATS_OUT(buf, ch);
                    check_autowiz(ch);
                } else {
                    //send_to_char("\r\n *** You must progress further along the Golden Path before reaching Immortality. *** \r\n", ch);
                }
            }
        } else {                /* gain < 0 */
            //        GET_EXP(ch) += MAX(-max_exp_loss, gain);    /* Cap max exp lost per * death */
            if (GET_LEVEL(ch)<LEVEL_NEWBIE)
                gain=0;
            GET_EXP(ch)+=gain;
            //if (GET_EXP(ch) < 0)
            //  GET_EXP(ch) = 0;
        }
    } else {
        GET_EXP(ch) = MAX(0, GET_EXP(ch)+gain);
    }
    return;
}

void            advance_char(struct char_data * ch)
{
    int             target_xp;



    if (GET_LEVEL(ch) < (LVL_IMMORT - 1)) {     /* able to gain */
        //        target_xp = total_exp(GET_LEVEL(ch));
        target_xp = LEVELEXP(ch);
        if (GET_EXP(ch) >= target_xp) { /* advance one level */
            GET_LEVEL(ch) += 1;
            advance_level(ch);
            send_to_char("\r\n                     [ &W&f You raise a level! &0 ]\r\n", ch);
            //            target_xp = total_exp(GET_LEVEL(ch)-1);
            //GET_EXP(ch) = target_xp +1;
            GET_EXP(ch) = 1;
            /*            if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
                            set_title(ch, NULL);*/
        } else
            send_to_char("You are not ready to gain a level yet.\r\n", ch);
    } else {                    /* immort */
        //        target_xp = total_exp(GET_LEVEL(ch));
        target_xp = LEVELEXP(ch);

        if ( /* (GET_REINCARN(ch) >= 8) && */ (GET_EXP(ch) >= target_xp)) {
            GET_LEVEL(ch) += 1;
            advance_level(ch);
            send_to_char("\r\r\n\n                        *** Welcome to Immortality! *** \r\r\n\r\n\n", ch);
            /*            if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
                            set_title(ch, NULL);*/
            check_autowiz(ch);
        } else {
            send_to_char("You must progress further along the Golden Path before reaching Immortality.\r\n", ch);
        }
    }

}

void            gain_exp_regardless(struct char_data * ch, int gain)
{
    int             is_altered = FALSE;
    int             num_levels = 0;
    int saveexp;

    GET_EXP(ch) += (int) (gain / GET_NUM_OF_CLASS(ch));
    if (GET_EXP(ch) < 0)
        GET_EXP(ch) = 0;

    saveexp=GET_EXP(ch);
    if (!IS_NPC(ch)) {
        while (GET_LEVEL(ch) < LVL_IMPL &&
                //GET_EXP(ch) >= total_exp(GET_LEVEL(ch))) {
                GET_EXP(ch) >= LEVELEXP(ch)) {
            GET_LEVEL(ch) += 1;
            num_levels++;
            saveexp-=LEVELEXP(ch);
            advance_level(ch);
            GET_EXP(ch)=saveexp;
            is_altered = TRUE;
        }

        if (is_altered) {
            if (num_levels == 1)
                send_to_char("You rise a level!\r\n", ch);
            else {
                sprintf(buf, "You rise %d levels!\r\n", num_levels);
                send_to_char(buf, ch);
            }
            /*            if (!PLR_FLAGGED(ch, PLR_NOSETTITLE))
                            set_title(ch, NULL);*/
            check_autowiz(ch);
        }
    }
}


void            gain_condition(struct char_data * ch, int condition, int value)
{
    bool            intoxicated;

    if (GET_COND(ch, condition) <= -1)  /* No change */
        return;

    intoxicated = (GET_COND(ch, DRUNK) > 0);

    if (value>0 || !IS_GOD(ch))
        GET_COND(ch, condition) += value;
    
    if (value>0)
    {
    	value=MAX(1, value-(GET_CON(ch)-18));
    }    

    GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
    GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

    if (PLR_FLAGGED(ch, PLR_WRITING | PLR_EDITING))
        return;
    if (GET_COND(ch, condition) && value<0)
    {
        if (condition==FULL && !FIGHTING(ch))
        {
            if (GET_COND(ch, condition)==4)
                send_to_char("Your hunger gets unbearable.\r\n", ch);
            else if (GET_COND(ch, condition)==6)
                send_to_char("You could really use some food.\r\n", ch);
            else if (GET_COND(ch, condition)==8)
                send_to_char("You are getting very hungry.\r\n", ch);
            else if (GET_COND(ch, condition)==14)
                send_to_char("You are a bit hungry.\r\n", ch);
        }
        else
            if (condition==THIRST && !FIGHTING(ch))
            {
                if (GET_COND(ch, condition)==3)
                    send_to_char("You are almost dehydrated.\r\n", ch);
                else if (GET_COND(ch, condition)==6)
                    send_to_char("You could really use some water.\r\n", ch);
                else if (GET_COND(ch, condition)==9)
                    send_to_char("You are getting thirsty.\r\n", ch);
                else if (GET_COND(ch, condition)==15)
                    send_to_char("Your mouth is a little dry.\r\n", ch);

            }
    }
    else if (value < 0 && !IS_GOD(ch) && ch->in_room>1)
        switch (condition) {
        case FULL:
            if (!FIGHTING(ch))
                send_to_char("&wYour stomach makes faint rumbling sounds.&0\r\n", ch);
            //if (GET_HIT(ch)>(4*GET_MAX_HIT(ch)/100))// && GET_HIT(ch)>1)
            GET_HIT(ch)-=MAX(1, 4*GET_MAX_HIT(ch)/100);
            return;
        case THIRST:
            if (!FIGHTING(ch))
                send_to_char("&wYou are completly dehydrated.&0\r\n", ch);
            ///if (GET_HIT(ch)>8*GET_MAX_HIT(ch)/100 )//&& GET_HIT(ch)>1)
            GET_HIT(ch)-=MAX(1, 8*GET_MAX_HIT(ch)/100);
            return;
        case DRUNK:
            if (intoxicated)
                send_to_char("&cYou are now sober.&0\r\n", ch);
            return;
        default:
            break;
        }


}

void            check_idling(struct char_data * ch)
{
    extern int      free_rent;
    void            Crash_rentsave(struct char_data * ch, int cost);
    if (++(ch->char_specials.timer) > 20) {
        if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
            GET_WAS_IN(ch) = ch->in_room;
            if (FIGHTING(ch)) {
                return;
                /*	stop_fighting(FIGHTING(ch));
                        stop_fighting(ch);*/
            }
            act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
            leech_from_char(ch, -123454321);
            ch->mana_leech=0;
            save_char(ch, ch->in_room);
            Crash_crashsave(ch);
            char_from_room(ch);
            char_to_room(ch, 1);
        } else if (ch->char_specials.timer > 60) {
            /*      if (ch->in_room != NOWHERE)
            //      char_from_room(ch);
            //      char_to_room(ch, 1);*/
            add_llog_entry(ch,LAST_IDLEOUT);  
             if (ch->desc)
                close_socket(ch->desc);
            /*            //save_char(ch, GET_WAS_IN(ch));*/
            ch->desc = NULL;
            
            if (free_rent)
                Crash_rentsave(ch, 0);
            else
                Crash_idlesave(ch);
            sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
            mudlog(buf, CMP, LVL_GOD, TRUE);
            extract_char(ch);   
           
        }
    }
}

void            update_char_objects(struct char_data * ch); /* handler.c */
void            extract_obj(struct obj_data * obj); /* handler.c */



extern struct index_data *obj_index; 
void            point_update(void)
{
    struct char_data *rch,
                *i,
                *next_char, *ch, *vict;
    struct obj_data *j,
                *next_thing,
                *jj,
                *next_thing2;
    bool found;
    int             new_room;
    int robj_spring=real_object(OBJ_SPRING);
    int             dam;
    int isfullhour=0;
    memory_rec *names;
    /* characters */
    isfullhour=!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC));

for (i = character_list; i; i = next_char) {
        next_char = i->next;

        SANITY_CHECK(i);
        update_pos(i);

        if (AFF3_FLAGGED(i, AFF3_PLAGUE))
        {
            if (!IS_NPC(i))
            {
                GET_HIT(i)-=GET_MAX_HIT(i)/16;
                send_to_char("You feel awfuly sick.\r\n", i);
            }
            else
            {
                GET_HIT(i)-=10;
                if (!number(0, 6))
                {
                    affect_from_char(i, SPELL_PLAGUE);
                    act("$n looks much healthier now.", FALSE, i, 0, 0, TO_ROOM);
                }
            }
            if (check_kill(i, "plague"))
                continue;
        }
        if (AFF2_FLAGGED(i, AFF2_PRISM) && number(0,54)<GET_LEVEL(i))
            affect_from_char(i, SPELL_PRISMATIC_SPHERE);
        if (AFF2_FLAGGED(i, AFF2_ULTRA) && number(0,54)<GET_LEVEL(i))
            affect_from_char(i, SPELL_ULTRA_DAMAGE);
        if (AFF2_FLAGGED(i, AFF2_PETRIFY) && number(0,54)<GET_LEVEL(i))
            affect_from_char(i, SPELL_PETRIFY);
        if (!IS_NPC(i) && (i->desc)) {
        
         if (!IS_GOD(i) && (!number(0,(GET_COND(i, FULL) ? 6:3)))) {
            if (GET_RACE(i)==RACE_TROLL)
            {
                gain_condition(i, FULL, -2);
                //gain_condition(i, FULL, -1);
            }
            else if ((GET_RACE(i)==RACE_ELF || GET_RACE(i)==RACE_DROW) && number(0,100)>50)
                gain_condition(i, FULL, -1);
            else
                gain_condition(i, FULL, -1); 
                
            

            if (check_kill(i, "starvation"))
                continue;
        }
        if ((!number(0,(GET_COND(i, THIRST) ? 15:6))))
        {
            gain_condition(i, THIRST, -1);
            if (check_kill(i, "dehydration"))
                continue;
        } 
        if (!number(0, 2))
        	gain_condition(i, DRUNK, -1-(number(0, 25)<GET_CONR(i)?1:0));
        }

        //        CREF(i, CHAR_NULL);
        if (GET_POS(i) >= POS_STUNNED) {

            if (IS_AFFECTED(i, AFF_POISON))
            {

                if (number(1, 85+(FIGHTING(i)?10:-25))<GET_CON(i)*GET_CON(i)/23)
                {
                    send_to_char("Your body resists the poison.\r\n", i);
                    affect_from_char(i, SPELL_POISON);
                }
                else if (GET_RACE(i)!=RACE_TROLL)
                {
                    damage(i, i, number(GET_LEVEL(i),2*GET_LEVEL(i)), SPELL_POISON, 0);
               	    if (DEAD(i))
                    	continue; 
                }
            }

            if (!AFF_FLAGGED(i, AFF_DEATHDANCE) && !AFF2_FLAGGED(i, AFF2_WRATHE) && !AFF3_FLAGGED(i, AFF3_PLAGUE)) {

                clim=GET_HIT(i)<GET_MAX_HIT(i);
                GET_HIT(i) = MAX(1, MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i)));
                if (!IS_NPC(i) && GET_POS(i)==POS_SLEEPING && GET_SKILL(i, SKILL_FASTHEALING))
                    improve_skill(i, SKILL_FASTHEALING, 8);
                if (!IS_NPC(i) && clim && GET_HIT(i)==GET_MAX_HIT(i))
                    send_to_char("You are at max hit points now.\r\n", i);
                /*    if (GET_POS(i)==POS_SLEEPING)
                    {
                    	if ((GET_HIT(i)==GET_MAX_HIT(i)) && !IS_AFFECTED(i, AFF_SLEEP))
                    	{
                    		send_to_char("You awaken and rise up.\r\n", i);
                    		act("$n awakens and stretches up.", TRUE, i, 0, 0, TO_ROOM);
                GET_POS(i) = POS_STANDING;				
                i->sleep_timer=0;
            }
                else 
            {
                if (i->sleep_timer>0)
                i->sleep_timer++;
                if (i->sleep_timer>7 && number(1, 100)<20)
            {
                send_to_char("You dream about distant lands and great adventures.\r\n",i);
                i->sleep_timer=-1;
            }
            }			
                if ((number(1,30)>GET_WIS(i)))
            {
                switch (number(1, 9))
            {
                case 1:
                case 2:
                case 3:
                case 4:	act("$n snores loudly.", FALSE, i, 0, 0, TO_ROOM);break;
                case 5: 
                case 6: act("$n turns around, making $mself more comfortable.", FALSE, i, 0, 0, TO_ROOM);break;
                case 7:
                case 8: act("$n tumbles over.", FALSE, i, 0, 0, TO_ROOM);break;
                default: act("$n farts.", FALSE, i, 0, 0, TO_ROOM);break;
            }
            }
            }
                else if (GET_POS(i)==POS_RESTING)
            {

                if (((GET_COND(i, FULL) == 0) || (GET_COND(i, THIRST) == 0)))
            {
                if (GET_COND(ch, FULL) == 0)
            {
                gain_condition(i, FULL, -1);
                if (check_kill(i, "starvation"))
                continue;
            }
                if (GET_COND(ch, THIRST) == 0)
                gain_condition(i, THIRST, -1);
                if (check_kill(i, "dehydration"))
                continue;
            }			 	
                else if (GET_HIT(i)<GET_MAX_HIT(i))
            {       
                i->sleep_timer++;
                switch (i->sleep_timer)
            {
                case 1: send_to_char("You relax yourself into a comfortable position.\r\n", i);break;
                //case 2: send_to_char("You feel sleepy..\r\n", i);break;
                case 2: send_to_char("You slowly fade away to the dreamland...\r\n", i);
                act("$n falls to sleep.", FALSE, i, 0, 0, TO_ROOM);
                GET_POS(i)=POS_SLEEPING;
                break;
                default:send_to_char("Please report a bug with sleep timer.\r\n", i);break;			
            }
            }
            }		*/
                clim=GET_MANA(i)<GET_MAX_MANA(i);
                GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
                if (!IS_NPC(i) && clim && GET_MANA(i)==GET_MAX_MANA(i))
                    send_to_char("You are at max energy now.\r\n", i);
                GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
                update_pos(i);
            }
        } else if (GET_POS(i) == POS_INCAP)
            damage(i, i, 1, TYPE_SUFFERING, 0);
        else if (GET_POS(i) == POS_MORTALLYW)
            damage(i, i, 2, TYPE_SUFFERING, 0);
        if (GET_POS(i)==POS_SLEEPING)
            rprog_sleep_trigger(i);
        if (DEAD(i))
            continue;


        if (IS_NPC(i) && IS_CASTERMOB(i) && !FIGHTING(i))
        {
            if (GET_HIT(i)<GET_MAX_HIT(i))
            {
                if (!affected_by_spell(i, SPELL_ARMOR))
                    cast_spell(i, i, 0, SPELL_FORTRESS, 0);
                if (IS_CLERIC(i))
                    cast_spell(i,i, 0, SPELL_HEAL, 0);
            }
        }


        if (!IS_GOD(i) && (SECT(i->in_room) == SECT_FLYING) && !can_fly(i)) {
            char buf2[200];
            send_to_char("You have plummeted to the ground.\r\n", i);
            log_death_trap(i);
            //death_cry(i);
            sprintf(buf2, "\r\n&BINFO || &C%s plummeted to the ground.&0\r\n", CAP(GET_NAME(i)));
            INFO_OUT(buf2);
            //extract_char(i);
            die(i, NULL);
            //                CUREF(i);
            continue;

        }
        if (!IS_NPC(i)) {
            //    if (GET_POS(i)==POS_SLEEPING)
            //    send_to_char("A gentle voice whispers to you, 'Tick-tack'\r\n", i);
            update_char_objects(i);
            if (GET_LEVEL(i) < LVL_GOD && !IN_ARENA(i))
                check_idling(i);

            if (DEAD(i))
                continue;
            if (GET_LEVEL(i) < LVL_IMMORT) {
                if (SECT(i->in_room) == SECT_WATER_NOSWIM && !has_boat(i)) {
                    act("$n thrashes about in the water straining to stay afloat.", FALSE,
                        i, 0, 0, TO_ROOM);
                    send_to_char("You are drowning!\r\n", i);
                    GET_HIT(i) -= GET_MAX_HIT(i)/4;
                    if (check_kill(i, "drowning"))
                        continue;
                }


            }
        }
        //CUREF(i);
        SANITY_CHECK(i);

        if (i && i->in_room!=NOWHERE && !ROOM_FLAGGED(i->in_room, ROOM_PEACEFUL)){
            ch=i;
            /* Aggressive Mobs */
            if (MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN) && !FIGHTING(ch)) {
                found = FALSE;
                for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
                    if (IS_NPC(vict) || !CAN_SEE(ch, vict) || GET_POS(ch)<=POS_SLEEPING ||  PRF_FLAGGED(vict, PRF_NOHASSLE))
                        continue;
                    if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
                        continue;
                    if (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) ||
                            (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vict)) ||
                            (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vict)) ||
                            (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vict))) {
                        hit(ch, vict, TYPE_UNDEFINED);
                        found = TRUE;
                    }
                }
            }

            if (DEAD(ch))
                continue;
            /* Mob Memory */
            if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch) && !FIGHTING(ch) && GET_POS(ch)>=POS_RESTING) {
                found = FALSE;
                for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
                    if (IS_NPC(vict) || !CAN_SEE(ch, vict) || GET_POS(ch)<=POS_SLEEPING || PRF_FLAGGED(vict, PRF_NOHASSLE))
                        continue;
                    for (names = MEMORY(ch); names && !found; names = names->next)
                        if (names->id == GET_IDNUM(vict)) {
                            found = TRUE;
                            act("'Hey!  You're the bloody fiend!!', exclaims $n.",
                                FALSE, ch, 0, 0, TO_ROOM);
                            hit(ch, vict, TYPE_UNDEFINED);
                        }
                }
            }
        }

        if (DEAD(i))
            continue;

        if(!IS_NPC(i))
            rprog_random_trigger( i );

        if (isfullhour)
        {
            mprog_hour_trigger(i);
            mprog_time_trigger(i);

        }

        //continue;
        //clear_ref:
        //CUREF(i);

    }


    /* objects */
    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;

        /* If this object is in water. */
        /*        if (j->in_room != NOWHERE && (SECT(j->in_room) == SECT_WATER_NOSWIM ||
                                              SECT(j->in_room) == SECT_WATER_SWIM)) {
                  
                    if (GET_OBJ_TYPE(j) != ITEM_BOAT && number(0, GET_OBJ_WEIGHT(j)) > 0) {
                        act("$p sinks into the murky depths.", FALSE, 0, j, 0, TO_ROOM);
                        extract_obj(j);
                        continue;
                    } else
                        act("$p floats unsteadily in the area.", FALSE, 0, j, 0, TO_ROOM);
                }*/
        /* check for falling objects */
        if (j->in_room && SECT(j->in_room) == SECT_FLYING &&
                (GET_OBJ_WEAR(j) & ITEM_WEAR_TAKE) &&
                world[j->in_room].dir_option[5] &&
                world[j->in_room].dir_option[5]->to_room) {
            new_room = world[j->in_room].dir_option[5]->to_room;
            if ((rch = world[j->in_room].people) != NULL) {
                act("$p falls away.", FALSE, rch, j, NULL, TO_ROOM);
                act("$p falls away.", FALSE, rch, j, NULL, TO_CHAR);
            }
            obj_from_room(j);             obj_to_room(j, new_room);
            if ((rch = world[j->in_room].people) != NULL) {
                act("$p falls in from above.", FALSE, rch, j, NULL, TO_ROOM);
                act("$p falls in from above.", FALSE, rch, j, NULL, TO_CHAR);
            }
        }

        if (GET_OBJ_TYPE(j) == ITEM_PORTAL) {
            if (GET_OBJ_TIMER(j) > 0)
                GET_OBJ_TIMER(j)--;
            if (!GET_OBJ_TIMER(j) && (j->in_room != NOWHERE) && (world[j->in_room].people)) {
                act("A glowing portal closes and fades from existance.",
                    TRUE, world[j->in_room].people, j, 0, TO_ROOM);
                act("A glowing portal closes and fades from existance.",
                    TRUE, world[j->in_room].people, j, 0, TO_CHAR);
                extract_obj(j);
                continue;
            }
        }

        if (GET_OBJ_RNUM(j) == robj_spring) {
            if (GET_OBJ_TIMER(j) > 0)
                GET_OBJ_TIMER(j)--;
            if (GET_OBJ_TIMER(j) == 1) {
                if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
                    act("$p starts to slow down.", FALSE, world[j->in_room].people, j, 0, TO_ROOM);
                    act("$p starts to slow down.", FALSE, world[j->in_room].people, j, 0, TO_CHAR);
                }
            }
            if (GET_OBJ_TIMER(j) == 0) {
                if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
                    act("$p has stopped flowing!", FALSE, world[j->in_room].people, j, 0, TO_ROOM);
                    act("$p has stopped flowing!", FALSE, world[j->in_room].people, j, 0, TO_CHAR);
                }
                extract_obj(j);
                continue;
            }
        }
        /* If this is a corpse */
        if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER) && GET_OBJ_VAL(j, 3)) {
        	if (j->orig_value)	// time corpse is protected by deity
        	{
        		j->orig_value--;
        		if (!j->orig_value)
        		{
        			if (j->carried_by)
                 		   act("$p stops glowing.", FALSE, j->carried_by, j, 0, TO_CHAR);
                		else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
                 	   		act("$p stops glowing.", TRUE, world[j->in_room].people, j, 0, TO_ROOM);
                    			act("$p stops glowing.", TRUE, world[j->in_room].people, j, 0, TO_CHAR);
                    		}
                    		REMOVE_BIT((j)->obj_flags.extra_flags,ITEM_GLOW);
                	}
        	}		
        	
            if (GET_OBJ_TIMER(j) > 0)
                GET_OBJ_TIMER(j)--;
            if (GET_OBJ_TIMER(j)<=0) {
                if (j->carried_by)
                    act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
                else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
                    act("A quivering horde of maggots consumes $p.", TRUE, world[j->in_room].people, j, 0, TO_ROOM);
                    act("A quivering horde of maggots consumes $p.", TRUE, world[j->in_room].people, j, 0, TO_CHAR);
                }
                for (jj = j->contains; jj; jj = next_thing2) {
                    next_thing2 = jj->next_content;
                    obj_from_obj(jj);
                    if (j->in_obj)
                        obj_to_obj(jj, j->in_obj);
                    else if (j->carried_by)
                        obj_to_room(jj, j->carried_by->in_room);
                    else if (j->in_room != NOWHERE)
                        obj_to_room(jj, j->in_room);
                    else
                        assert(FALSE);
                }
                extract_obj(j);
                continue;
            }
        }
        else if (GET_OBJ_TIMER(j) && j->in_room!=NOWHERE)
        {
            GET_OBJ_TIMER(j)--;
            if (GET_OBJ_TIMER(j)<=0)
            {
                act("$p vanishes out of existance.", TRUE, world[j->in_room].people, j, 0, TO_ROOM);
                act("$p vanishes out of existance.", TRUE, world[j->in_room].people, j, 0, TO_CHAR);

                extract_obj(j);
                continue;
            }
        }
    
    if (HAS_OBJ_PROG(j, RAND_PROG)) 
    	oprog_random_trigger(j);
    }
    

}
