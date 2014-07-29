/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "auction.h"
#include "arena.h"
#include "structs.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "screen.h"
#include "fight.h"
#include "clan.h"
#include "newquest.h"
#include "events.h"



//#define EVENT_COMBAT
#define ITEM_LEFT_CHANCE 85

#define IS_HIT(i) ((i>=TYPE_HIT) && (i<=TYPE_STAB))
#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) <= TYPE_MISSILE))
#define IS_BACKSTAB(type) ((type==SKILL_BACKSTAB || type==SKILL_DUAL_BACKSTAB || type==SKILL_CIRCLE || type==SKILL_DUAL_CIRCLE ))
/* Structures */
struct char_data *combat_list = NULL;   /* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern CHAR_DATA *supermob;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern struct title_type titles1[LVL_IMPL + 1];
extern int      pk_allowed;     /* see config.c */
extern int      auto_save;      /* see config.c */
extern int      max_exp_gain;   /* see config.c */
extern char    *dirs[];         /* see constants.c */
extern char    *dirs2[];        /* see constants.c */
extern int      rev_dir[];      /* see constants.c */
extern struct char_data *mob_proto;
extern sh_int   r_mortal_start_room;
extern char    *DEATH_MESSG;
extern void     improve_skill(struct char_data * ch, int skill, int mod);
extern void     improve_combat_skill(struct char_data * ch, int skill);
extern char    *spells[];
extern int arena_red, arena_blue;
extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
extern int modexp;
/* External procedures */
char           *fread_action(FILE * fl, int nr);
char           *fread_string(FILE * fl, char *error);
void            stop_follower(struct char_data * ch);
int get_carry_cond(struct char_data *ch);
ACMD(do_flee);
void            hit(struct char_data * ch, struct char_data * victim, int type);
void            forget(struct char_data * ch, struct char_data * victim);
void            remember(struct char_data * ch, struct char_data * victim);
int             ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
void            mprog_hitprcnt_trigger(struct char_data * mob, struct char_data * ch);
void            mprog_death_trigger(struct char_data * mob, struct char_data * killer);
void            mprog_fight_trigger(struct char_data * mob, struct char_data * ch);
void check_add_topdam(char *ch, char *vict, int dam, int skill, int what, int kl);
void fight_mage(struct char_data *ch);
void fight_cleric(struct char_data *ch);
void wear_all_suitable(struct char_data *ch);
extern int get_obj_resistance( OBJ_DATA *obj );
extern struct zone_data *zone_table;   /* zone table			 */
extern int             top_of_zone_table;  /* top element of zone tab	 */
extern int             top_of_world;       /* ref to top element of world	 */
extern int damage_obj( CHAR_DATA *ch, OBJ_DATA *obj, int percent );
extern struct mob_kill_info mobkills[];
extern int current_quest, quest_step;
/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
    {
        {"hit", "hits"},            /* 0 */
        {"sting", "stings"},
        {"whip", "whips"},
        {"slash", "slashes"},
        {"bite", "bites"},
        {"bludgeon", "bludgeons"},  /* 5 */
        {"crush", "crushes"},
        {"pound", "pounds"},
        {"claw", "claws"},
        {"maul", "mauls"},
        {"thrash", "thrashes"},     /* 10 */
        {"pierce", "pierces"},
        {"blast", "blasts"},
        {"punch", "punches"},
        {"stab", "stabs"},
        {"hit", "hits"}        /* 15 */
    };


#define NUM_OF_FUN_PARTS 21
struct fun_body_piece {
    int number;          /* this parts number */
    char name[40];       /* names of this part */
    int  nname;          /* some parts you couldn't trace to an owner*/
    char sdesc[128];     /* short desc: that of inventory  */
    char rdesc[128];     /* room desc: when on ground */
    int  take;           /* some body parts don't transfer well */
    char actout[128];    /* what people in room see upon death, using
    act()*/
    char actkil[128];    /* what the killer sees upon dismemberment, using
    act() */
};

struct fun_body_piece parts[NUM_OF_FUN_PARTS] = {
            {0,"eyeball eye",1,"the eye of %s","The eyeball of %s is lying here.",
                1,"$n's attack knocks an eye out of $N!",
                "Your attack knocks an eye out of $N!"},
            {1,"liver",1,"the liver of %s","%s's liver is lying here.",
             1,"$n's cruel attack blows $N's liver out!",
             "Your cruel attack blows $N's liver out!"},
            {2,"arm",1,"one of %s's arms","%s'arm is lying here on the ground.",
             1,"$n's removes $N's arm!",
             "You remove $N's arm!"},
            {3,"bowels",1,"%s's bowels","Ick. %s's bowels are lying here.",
             1,"$n debowels $N!",
             "You debowl $N!"},
            {4,"tush butt rear ass",1,"%s's rear end",
             "Some one cut of %s's butt and left it here.",
             1,"$n laughs as he severs $N's rear end!",
             "You laugh as you sever $N's rear end!"},
            {5,"right leg",1,"%s's right leg","%s's right leg is here.",
             1,"$N gracefully cuts his leg off! $n chortles merrily!",
             "You watch in awe as $N cuts his leg off!"},
            {6,"left leg",1,"the left leg of %s","The left leg of %s is lying here.",
             1,"$n's screams and strikes $N leg off at the hip!",
             "With a scream of rage, you strike $N's leg off!"},
            {7,"head",1,"%s's ugly head","%s's head is lying here, staring at you.",
             1,"$n severs $N's in a move composed of speed and grace!",
             "With speed and grace, you sever $N's head!"},
            {8,"thumb",1,"%s's thumb","One of %s's thumbs is lying here.",
             1,"$n's attack severs a thumb from $N!",
             "Your attack severs a thumb from $N!"},
            {9,"finger",1,"%s's finger","One of %s fingers is lying here.",
             1,"$n's attack severs a finger from $N!",
             "Your attack severs a finger from $N!"},
            {10,"stomach",1,"%s's stomach","%s lost his stomach here.",
             1,"With animal force, $n tears $N's stomach out!",
             "With animal force, you tear $N's stomach out!"},
            {11,"heart",1,"the once beating heart of %s",
             "%s's once beating heart lies here.",
             1,"$n's uses pure strength to eviscirate $N!",
             "Your depend on your fierce strength, and eviscerate $N!"},
            {12,"spine",1,"the spine of %s","The spine of %s is lying here.",
             1,"$n's attack shatters $N's spine!",
             "Your attack shatters $N's spine!"},
            {13,"intestine",0,"An icky pile of intestines",
             "An icky pile of intestines is here - colon and all.",
             0,"$n hits so hard, that $N pukes up his intestines !",
             "You hit $N so hard that he pukes up his intestines!"},
            {14,"puke vomit",0,"chunky vomit","Some one upchucked on the floor here.",
             0,"$N throws up all over!",
             "$N throws up all over you!"},
            {15,"pool blood",0,"A pool of blood","Blood has formed a pool on the \
             ground.",
             0,"$N bleeds horrendously!",
             "$N bleeds horrendously!"},
            {16,"riblet",1,"a meaty %s riblet","A meaty riblet from %s is lying \
             here.",
             1,"$n's explodes $N's chest with a barrage of attacks!",
             "Your cause $N's chest to explode from a barrage of attacks!"},
            {17,"nose",1,"%s's nose","%s lost his nose here.",
             1,"$n cackles gleefuly as he removes $N's nose!",
             "You cackle as you sever $N's nose!"},
            {18,"ear",1,"%s's ear","%'s bloody severed ear is here.",
             1,"$n's grabs $N's ear and rips it off!",
             "Your rip off $N's ear!"},
            {19,"brain",1,"the jiggly brain of %s","The squishy brain of %s is here.",
             1,"$n shatters $N's skull, knocking the brain on the ground!",
             "You shatter $N's skull, knocking the brain on the ground!"},
            {20,"lung",1,"a bloody lung from %s","A blood soaked lung from %s.",
             1,"$N screams his last as $n removes a lung!",
             "$N's screams are cut short as you remove a lung!"}
        };

void make_fun_body_pieces(struct char_data *ch, struct char_data *killer)
{
    struct obj_data *piece;
    int i;
    extern int max_npc_corpse_time;

    /*lets check and see if we even GET body parts eh - i mean, they're
      fun, but it wouldn't be quite as fun if they were always there!*/

    if (number(1,5) < 4)
        return;

    /*Then Horray! We's got parts! */
    /* But which part? */
    i=number(0,NUM_OF_FUN_PARTS-1);   /* 20 pieces should be okay*/
    piece = create_obj();

    /*now, everything we have should be in the structures neh?*/
    /*name first*/
    piece->name=str_dup(parts[i].name);
    /*then lets see about the descs */
    if (parts[i].nname) {
        sprintf(buf2, parts[i].sdesc, GET_NAME(ch));
        piece->short_description = str_dup(buf2);
        sprintf(buf2, parts[i].rdesc, GET_NAME(ch));
        piece->description = str_dup(buf2);
    }
    else {
        piece->short_description=str_dup(parts[i].sdesc);
        piece->description = str_dup(parts[i].rdesc);
    }
    /*well, now we know how it looks, lets see if we wanna take it.*/
    if (parts[i].take) {
        GET_OBJ_WEAR(piece) = ITEM_WEAR_TAKE;
    }
    /*  and lets see how it got here in the first place neh? */
    act(parts[i].actout,FALSE,killer,0,ch,TO_ROOM);
    act(parts[i].actkil,FALSE,killer,0,ch,TO_CHAR);

    /* setup the rest of the stats any object needs */
    piece->item_number = NOTHING;
    piece->in_room = NOWHERE;
    GET_OBJ_TYPE(piece) = ITEM_CONTAINER;
    GET_OBJ_VAL(piece, 0) = 0;   /* You can't store stuff in a corpse */
    GET_OBJ_VAL(piece, 3) = 1;   /* corpse identifier */
    GET_OBJ_EXTRA(piece) = ITEM_NODONATE;
    GET_OBJ_WEIGHT(piece) = 1;
    GET_OBJ_RENT(piece) = 1;

    /* Note - you may have some trouble with corpse decay here -
    improper settings WILL cause the mud to crash if you do not correcly
    decay.  Right now, the pieces are setup as a corpse, so if you made any
    changes to your corpse identifiers, fix it above. */

    GET_OBJ_TIMER(piece) = max_npc_corpse_time;

    /* and thats all folks! */
    obj_to_room(piece, ch->in_room);
}


struct char_data *get_fightning_with(struct char_data *ch)
{
    struct char_data *pom;

    if (DEAD(ch))
        return;
    for (pom=world[ch->in_room].people; pom; pom=pom->next_in_room)
        if (FIGHTING(pom)==ch)
            return pom;
    return NULL;
}



extern         char           *act_string(char *orig, struct char_data * ch, struct obj_data * obj,
            void *vict_obj, struct char_data * to);

void send_miss_message(struct char_data *ch, struct char_data *victim, char *message)
{
    struct char_data *pom;

    if (DEAD(ch))
        return;
    for (pom=world[ch->in_room].people; pom; pom=pom->next_in_room)
        if (!IS_NPC(pom) && pom!=ch && pom!=victim && AWAKE(pom) && ((!PRF2_FLAGGED(pom, PRF2_NOMISSE) && !is_same_group(pom, ch)) || (!PRF2_FLAGGED(pom, PRF2_NOMISSF) && is_same_group(pom, ch))))
            send_to_char(act_string(message, ch, NULL, (void *) victim, pom), pom);
}

void send_dam_message(struct char_data *ch, struct char_data *victim, char *message)
{
    struct char_data *pom;

    if (DEAD(ch))
        return;
    for (pom=world[ch->in_room].people; pom; pom=pom->next_in_room)
        if (!IS_NPC(pom) && pom!=ch && pom!=victim && AWAKE(pom))
        {
            if (is_same_group(ch, pom))
            {
                send_to_char(CCCYN(ch, C_CMP), pom);
                send_to_char(act_string(message, ch, NULL, (void *) victim, pom), pom);
                send_to_char(CCNRM(ch, C_CMP), pom);
            }
            else
                send_to_char(act_string(message, ch, NULL, (void *) victim, pom), pom);

        }

}



void npc_brag(struct char_data *ch, struct char_data *vict)
{

    char brag[256]={"       "};
    if (GET_LEVEL(ch)<5)
        return;
    switch (number(0, 10)) {
    case 0:
        sprintf(brag, "%s was just too easy to kill! Hahaha...", GET_NAME(vict));
        break;
    case 1:
        sprintf(brag, "%s was a tasty dinner, now who's for desert? Muhaha!", GET_NAME(vict));
        break;
    case 2:
        sprintf(brag, "%s is now in need of some exp...Muhaha!", GET_NAME(vict));
        break;
    case 3:
        sprintf(brag, "%s needs a hospital now. Muhaha!",GET_NAME(vict));
        break;
    case 4:
        sprintf(brag, "Hey %s!  Come back, you dropped your corpse! Muahaha", GET_NAME(vict));
        break;
    case 5:
        sprintf(brag, "%s is a punk, hits like a swampfly. Bah.", GET_NAME(vict));
        break;
    case 6:
        sprintf(brag, "%s, your life force has just run out...Muahaha!", GET_NAME(vict));
        break;
    case 7:
        sprintf(brag, "Hey %s!  Come back, you dropped your corpse! Muahaha", GET_NAME(vict));
        break;
    case 8:
        sprintf(brag, "I think %s wears pink chainmail.  Fights like a girl!  Muhaha!", GET_NAME(vict));
        break;
    case 9:
        sprintf(brag, "Go and play with your toys %s! Muahaha!", GET_NAME(vict));
        break;
    case 10:
        sprintf(brag, "Hey %s! Don't come back before you learn how to fight.", GET_NAME(vict));
        break;
    }

    do_gen_comm(ch, brag, SCMD_GOSSIP, 0);
}


void remove_combat_affects(struct char_data *ch)
{
    if (!ch)
        return;

    if (AFF3_FLAGGED(ch, AFF3_TEMP_BLIND) && number(0, 3))
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_TEMP_BLIND);
    if (AFF3_FLAGGED(ch, AFF3_DISTRACT))
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_DISTRACT);
        
    
}



/* The Fight related routines */

void change_style(struct char_data *ch, struct char_data *victim)
{

    if (!ch)
        return;

    if (IS_CASTERMOB(ch))//MAGIC_USER(ch) || IS_CLERIC(ch))
    {
        if ((GET_HIT(ch)*100/GET_MAX_HIT(ch)) < (GET_HIT(victim)*100/GET_MAX_HIT(victim))-10)
        {
            if (GET_STYLE(ch)!=1)
                act("&cYou notice that $n becomes more defensive.&0", FALSE, ch, 0, 0, TO_ROOM);
            SET_STYLE(ch,1);

        }

        else if ((GET_HIT(ch)*100/GET_MAX_HIT(ch)) > (GET_HIT(victim)*100/GET_MAX_HIT(victim))+50)
        {
            if (GET_STYLE(ch)!=1)
                act("&cYou notice that $n becomes more aggressive.&0", FALSE, ch, 0, 0, TO_ROOM);
            SET_STYLE(ch,2);

        }
    }
    else
        if (GET_LEVEL(ch)>3)
        {
            /*if (((GET_HIT(ch)*100/GET_MAX_HIT(ch)) < (GET_HIT(victim)*100/GET_MAX_HIT(victim))-30) && (GET_HIT(ch)*100/GET_MAX_HIT(ch)>50) && !IS_CASTER(victim))

        {
              //  if (GET_STYLE(ch)!=1)
                    //act("&cYou notice that $n becomes more defensive.&0", FALSE, ch, 0, 0, TO_ROOM);
               // GET_STYLE(ch)=1;
                if (GET_STYLE(ch)!=2)
                    act("&cYou notice that $n becomes more aggressive.&0", FALSE, ch, 0, 0, TO_ROOM);
                GET_STYLE(ch)=2;

        }
              */
            if ((GET_HIT(ch)*100/GET_MAX_HIT(ch)) > (GET_HIT(victim)*100/GET_MAX_HIT(victim))+20)
            {
                if (GET_STYLE(ch)!=2)
                    act("&cYou notice that $n becomes more aggressive.&0", FALSE, ch, 0, 0, TO_ROOM);
                SET_STYLE(ch,2);

            }
            else if (GET_LEVEL(ch)>5 && GET_LEVEL(ch)>GET_LEVEL(victim) && GET_STYLE(ch)!=2 && !number(0, MAX(1, 8-GET_LEVEL(ch)+GET_LEVEL(victim))))
            {
                SET_STYLE(ch,2);
                act("&cYou notice that $n becomes more aggressive.&0", FALSE, ch, 0, 0, TO_ROOM);
            }

        }
}

int blow_out(struct char_data *ch)
{
    int i,dir, was_in;
    char buf2[100];
    struct char_data *tch, *next_tch;

    if (ch==NULL || ch->in_room<1)
        return 0;

    for (i = 0; i < 6; i++) {
        dir = number(0, NUM_OF_DIRS - 1);
        if (CAN_GO(ch, dir)) {
           if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_DEATH))
           continue;

            if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
                    world[EXIT(ch, dir)->to_room].people != NULL)
                continue;

            if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_PRIVATE) &&
                    world[EXIT(ch, dir)->to_room].people != NULL &&
                    (world[EXIT(ch, dir)->to_room].people)->next_in_room != NULL)
                continue;

            if (((SECT(ch->in_room) == SECT_LAVA) ||
                    (SECT(EXIT(ch, dir)->to_room) == SECT_LAVA))
                    && GET_LEVEL(ch) < LVL_IMMORT)
                if (!can_lava(ch))
                    SET_BIT(AFF2_FLAGS(ch), AFF2_BURNING);

            if (((SECT(ch->in_room) == SECT_ARCTIC) ||
                    (SECT(EXIT(ch, dir)->to_room) == SECT_ARCTIC))
                    && GET_LEVEL(ch) < LVL_IMMORT)
                if (!can_artic(ch))
                    SET_BIT(AFF2_FLAGS(ch), AFF2_FREEZING);

            if (FIGHTING(ch)) {
                for (tch = world[ch->in_room].people; tch; tch = next_tch) {
                    next_tch = tch->next_in_room;
                    if (FIGHTING(tch)==ch)
                    {
                        if (IS_NPC(tch) && !HUNTING(tch))
                            HUNTING(tch)=ch;
                        stop_fighting(tch);
                    }
                }
                stop_fighting(ch);
            }

            sprintf(buf2, "$n is &Gblown away&0 to the &G%s&0!!&0", dirs[dir]);
            act(buf2, TRUE, ch, 0, 0, TO_ROOM);

            send_to_char("You are &Gblown away!!&0\r\n\r\n",ch);
            GET_HIT(ch)-=number(GET_TOTAL_WEIGHT(ch)/3,GET_TOTAL_WEIGHT(ch)/2);
            was_in = ch->in_room;
            SET_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
            char_from_room(ch);
            char_to_room(ch, world[was_in].dir_option[dir]->to_room);
            REMOVE_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
            GET_POS(ch)=POS_SITTING;

            if (ch->desc != NULL)
                look_at_room(ch, 0);

            sprintf(buf2, "$n flies in here, blown away from the %s!", dirs2[rev_dir[dir]]);
            act(buf2, TRUE, ch, 0, 0, TO_ROOM);
            if ((SECT(ch->in_room) == SECT_FLYING) && GET_LEVEL(ch) < LVL_IMMORT) {
                if (!can_fly(ch)) {
                    char buf2[200];
                    send_to_char("You have plummeted to the ground.\r\n", ch);
                    log_death_trap(ch);
                    //death_cry(ch);
                    sprintf(buf2, "\r\n&BINFO || &C%s&c plummeted to the ground.&0\r\n", GET_NAME(ch));
                    INFO_OUT(buf2);
                    //extract_char(ch);
                    die(ch, NULL);
                    return 1;
                }
            }

            sprintf(buf2,"breaking %s neck",HSHR(ch));
            check_kill(ch,buf2);
            return 1;
        }
    }
    return 0;
}

void            mob_ai(struct char_data * ch)
{
    struct char_data *opponent,
                *opp = NULL;
    int             maxh = 90000, prob;
    if (!ch || !FIGHTING(ch) || DEAD(ch))
        return;

    if (100*GET_HIT(FIGHTING(ch))/GET_MAX_HIT(FIGHTING(ch))>30)
{
        prob=number(1, 10);


        if (prob<4)
        {
            for (opponent = world[ch->in_room].people; opponent; opponent = opponent->next_in_room)
                if ((FIGHTING(opponent) == ch) && CAN_SEE(ch, opponent)  && (100*GET_HIT(opponent)/GET_MAX_HIT(opponent) < maxh) && !IS_NPC(opponent)) {
                    maxh = 100*GET_HIT(opponent)/GET_MAX_HIT(opponent);
                    opp = opponent;
                }
        }
        else
        {
            for (opponent = world[ch->in_room].people; opponent; opponent = opponent->next_in_room)
                if ((FIGHTING(opponent) == ch) && CAN_SEE(ch, opponent) && (GET_LEVEL(opponent) < maxh) && !IS_NPC(opponent)) {
                    maxh = GET_LEVEL(opponent);
                    opp = opponent;
                }
        }

    }
    if (opp && opp != ch && opp != FIGHTING(ch)) {
        act("You turn to attack $N!", FALSE, ch, NULL, opp, TO_CHAR);
        act("$n turns to attack $N!", FALSE, ch, NULL, opp, TO_NOTVICT);
        act("$n turns to attack you!", FALSE, ch, NULL, opp, TO_VICT);
        stop_fighting(ch);
        set_fighting(ch, opp);
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}




void            side_kick(struct char_data * ch)
{
    struct char_data *opponent,
                *opp = NULL;
    int             maxh = 90000;
    if (!FIGHTING(ch) || DEAD(ch))
        return;

    if (!number(0,2))
{
        for (opponent = world[ch->in_room].people; opponent; opponent = opponent->next_in_room)
            if (((FIGHTING(opponent) == ch) || (CAN_SEE(ch, opponent) && number(1, 100)>50 && is_same_group(opponent, FIGHTING(ch)))) && (GET_HIT(opponent) < maxh)) {
                maxh = GET_HIT(opponent);
                opp = opponent;
            }
    }
    else
    {
        for (opponent = world[ch->in_room].people; opponent; opponent = opponent->next_in_room)
            if (((FIGHTING(opponent) == ch) || (CAN_SEE(ch, opponent) && number(1, 100)>50 && is_same_group(opponent, FIGHTING(ch)))) && (GET_LEVEL(opponent) < maxh)) {
                maxh = GET_LEVEL(opponent);
                opp = opponent;
            }
    }
    if (opp && opp != ch && opp != FIGHTING(ch)) {

        act("You &clunge&0 at $N!", FALSE, ch, NULL, opp, TO_CHAR);
        act("$n &clunges&0 at $N!!", FALSE, ch, NULL, opp, TO_NOTVICT);
        act("$n &Clunges&0 at you!!!", FALSE, ch, NULL, opp, TO_VICT);
        improve_skill(ch, SKILL_LUNGE, 7);
        hit(ch, opp, TYPE_UNDEFINED);
        //if(number(1, 351)<GET_SKILL(ch, SKILL_SECOND_ATTACK)+GET_DEX(ch))
        //	hit(ch, opp, TYPE_UNDEFINED);
    }
}


int             check_kill(struct char_data * victim, char whatk[100])
{
    char            bufm[500];
    update_pos(victim);

    switch (GET_POS(victim)) {
    case POS_MORTALLYW:
        act("$n is mortally wounded, and will die soon, if not aided.",
            TRUE, victim, 0, 0, TO_ROOM);
        act("You are mortally wounded, and will die soon, if not aided.",
            FALSE, victim, 0, 0, TO_CHAR);
        break;
    case POS_INCAP:
        act("$n is incapacitated and will slowly die, if not aided.",
            TRUE, victim, 0, 0, TO_ROOM);
        act("You are incapacitated and will slowly die, if not aided.",
            FALSE, victim, 0, 0, TO_CHAR);
        break;
    case POS_STUNNED:
        act("$n is stunned, but will probably regain conscience again.",
            TRUE, victim, 0, 0, TO_ROOM);
        act("You're stunned, but will probably regain conscience again.",
            FALSE, victim, 0, 0, TO_CHAR);
        break;
    case POS_DEAD:
        //act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
        death_cry(victim);
        act("You are dead! &GDEAD!&0", FALSE, victim, 0, 0, TO_CHAR);
        break;
    default:                    /* >= POS_SLEEPING */
        //if (GET_HIT(victim) < (GET_MAX_HIT(victim) /4)) {
        //  act("You wish that your wounds would stop &RBLEEDING&0 that much!",
        //    FALSE, victim, 0, 0, TO_CHAR);
        //}
        break;
    }

    if (GET_POS(victim) == POS_DEAD) {
        sprintf(bufm, "%s [%d] killed by %s at %s (%d)", GET_NAME(victim), GET_LEVEL(victim),
                whatk,
                world[victim->in_room].name, world[victim->in_room].number);
        log(bufm);
        if (!IS_NPC(victim)) {
            sprintf(bufm, "\r\n&BINFO || &Y%s killed by %s.&0\r\n", GET_NAME(victim), whatk);
            INFO_OUT(bufm);
        }
        die(victim, NULL);
        return 1;
    }
    return 0;
}

void            appear(struct char_data * ch)
{
    if (affected_by_spell(ch, SPELL_INVISIBLE))
    {
        leech_from_char(ch, SPELL_INVISIBLE);
        affect_from_char(ch, SPELL_INVISIBLE);
    }

    if (affected_by_spell(ch, SKILL_MOVE_HIDDEN))
        affect_from_char(ch, SKILL_MOVE_HIDDEN);
    if (affected_by_spell(ch, SKILL_HIDE))
        affect_from_char(ch, SKILL_HIDE);

    REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE | AFF_HIDE);


    if (GET_LEVEL(ch) < LVL_IMMORT)
        act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
    else
        act("You feel a strange presence as $n appears, seemingly from nowhere.",
            FALSE, ch, 0, 0, TO_ROOM);
}



void            load_messages(void)
{
    FILE           *fl;
    int             i,
    type;
    struct message_type *messages;
    char            chk[128],
    pom[200];

    if (!(fl = fopen(MESS_FILE, "r"))) {
        sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
        perror(buf2);
        exit(1);
    }
    for (i = 0; i < MAX_MESSAGES; i++) {
        fight_messages[i].a_type = 0;
        fight_messages[i].number_of_attacks = 0;
        fight_messages[i].msg = 0;
    }


    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
        fgets(chk, 128, fl);

    while (*chk == 'M') {
        fgets(chk, 128, fl);
        sscanf(chk, " %d\n", &type);
        for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
                (fight_messages[i].a_type); i++);
        if (i >= MAX_MESSAGES) {
            fprintf(stderr, "Too many combat messages.  Increase MAX_MESSAGES and recompile.");
            exit(1);
        }
        CREATE(messages, struct message_type, 1);
        fight_messages[i].number_of_attacks++;
        fight_messages[i].a_type = type;
        messages->next = fight_messages[i].msg;
        fight_messages[i].msg = messages;
        //sprintf(pom, "%d  type: %d, num : %d", i, type, fight_messages[i].number_of_attacks);
        // if (type==300)
        //log(pom);

        messages->die_msg.attacker_msg = fread_action(fl, i);
        messages->die_msg.victim_msg = fread_action(fl, i);
        messages->die_msg.room_msg = fread_action(fl, i);
        messages->miss_msg.attacker_msg = fread_action(fl, i);
        messages->miss_msg.victim_msg = fread_action(fl, i);
        messages->miss_msg.room_msg = fread_action(fl, i);
        messages->hit_msg.attacker_msg = fread_action(fl, i);
        messages->hit_msg.victim_msg = fread_action(fl, i);
        messages->hit_msg.room_msg = fread_action(fl, i);
        messages->god_msg.attacker_msg = fread_action(fl, i);
        messages->god_msg.victim_msg = fread_action(fl, i);
        messages->god_msg.room_msg = fread_action(fl, i);
        fgets(chk, 128, fl);
        while (!feof(fl) && (*chk == '\n' || *chk == '*'))
            fgets(chk, 128, fl);
    }

    fclose(fl);
}


void            update_pos(struct char_data * victim)
{

    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
        return;
    else if (GET_HIT(victim) > 0)
        GET_POS(victim) = POS_STANDING;
    else if (GET_HIT(victim) <= -GET_CON(victim)/2)
        GET_POS(victim) = POS_DEAD;
    else if (GET_HIT(victim) <= -GET_CON(victim)/3)
        GET_POS(victim) = POS_MORTALLYW;
    else if (GET_HIT(victim) <= -2)
        GET_POS(victim) = POS_INCAP;
    else
        GET_POS(victim) = POS_STUNNED;
}


void            check_killer(struct char_data * ch, struct char_data * vict)
{
    char            buf[256];

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
        return;
    if (PLR_FLAGGED(ch, PLR_KILLER) || CAN_MURDER(ch, vict))
        return;
    else {
        SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
        sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.",
                GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name);
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n", ch);
    }
}


struct fight_event_data {
    struct char_data *ch, *vict;
};

int fight_delay(struct char_data *ch)
{
    int next_hit;

    if (!ch || ch->in_room<1 || !FIGHTING(ch) || !GET_FIGHT_EVENT(ch))
        return 0;

    next_hit =  MAX(1, PULSE_VIOLENCE*20/MAX(1, GET_SKILL(ch, GET_EQ(ch, WEAR_WIELD)? GET_SKILL(ch, GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3)+TYPE_HIT): GET_SKILL(ch, SKILL_HIT))));
    return next_hit;
}


EVENTFUNC(fight_event)
{
    struct char_data *ch = ((struct fight_event_data *)event_obj)->ch;
    struct char_data *vict = ((struct fight_event_data *)event_obj)->vict;
    int next_hit;
    struct event *pom;


    if (!FIGHTING(ch) || ch->in_room != vict->in_room || ch->in_room<1) {
        stop_fighting(ch);
        return 0;
    }

    if (FIGHTING(ch)!=vict)
    {
        log("Screw up");
        stop_fighting(ch);
        return 0;
    }
    //pom=GET_FIGHT_EVENT(ch);
    //GET_FIGHT_EVENT(ch)=NULL;


    if (IS_NPC(ch)) {
        if (GET_MOB_WAIT(ch) > 0) {
            GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
            return PULSE_VIOLENCE;
        }
        GET_MOB_WAIT(ch) = 0;
        if (GET_POS(ch) < POS_FIGHTING) {
            GET_POS(ch) = POS_FIGHTING;
            act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
        }
    }

    if (GET_POS(ch) < POS_FIGHTING) {
        send_to_char("You can't fight while sitting!!\r\n", ch);
        return PULSE_VIOLENCE;
    }

    //  CREF(ch, CHAR_NULL);
    hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    if (DEAD(ch))
        return 0;

    if (ch && MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
        (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");

    next_hit = fight_delay(ch);


    //if (next_hit>0 && ch)
    //	GET_FIGHT_EVENT(ch)=pom;
    //CUREF(ch);
    if (next_hit<=0)
        free (event_obj);
    return next_hit;
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void            set_fighting(struct char_data * ch, struct char_data * vict)
{
    struct follow_type *k;

    struct fight_event_data *fed;

    if (ch == vict)
        return;

    assert(!FIGHTING(ch));
    assert(!GET_FIGHT_EVENT(ch));


    if (IS_SUPERMOB(ch) || IS_SUPERMOB(vict))
    {
        logs("SYSERR: set_fight %s is trying to enter fight with %s in room %d", GET_NAME(ch), GET_NAME(vict), GET_ROOM_VNUM(ch->in_room));
        return;
    }
    /*    if (FIGHTING(ch))
        {char bufg[100];
        int i=0,j=5;
           sprintf(bufg,"FATAL: %s with %s (already with %s)", GET_NAME(ch), GET_NAME(vict), GET_NAME(FIGHTING(ch)));
        log(bufg);
        abort();
        }
    */
    ch->next_fighting = combat_list;
    combat_list = ch;

    if (GET_POS(ch)==POS_SLEEPING)
    {
        send_to_char( "You are suddenly awakened by the sharp feeling of pain!\r\n", ch );

    }

    if (IS_AFFECTED(ch, AFF_SLEEP))
    {
        affect_from_char(ch, SPELL_SLEEP);
        leech_from_char(ch, SPELL_SLEEP);
    }

    if (AFF2_FLAGGED(ch, AFF2_NAP)) {
        affect_from_char(ch, SPELL_NAP);
        leech_from_char(ch, SPELL_NAP);
    }
    if (!IN_ARENA(ch))
    ch->pk_timer=MAX(30*PASSES_PER_SEC, ch->pk_timer);



    if (FOL_URG(ch))
    {
        if (!AFF2_FLAGGED(ch, AFF2_BERSERK))
        {
        	struct affected_type af;

            af.bitvector = 0;
            af.bitvector2 = AFF2_BERSERK;
            af.bitvector3 = 0;
            af.type = SKILL_BERSERK;
            af.duration = -1;
            af.location = APPLY_HITROLL;
            af.modifier = GET_LEVEL(ch)/4+2;
            affect_to_char(ch, &af);

            af.modifier = GET_LEVEL(ch)/4+2;
            af.location = APPLY_DAMROLL;
            affect_to_char(ch, &af);

            send_to_char("&WYou have gone BERSERK!&0\r\n", ch);
            act("$n has gone &WBERSERK!&0", FALSE, ch, NULL, NULL, TO_ROOM);

        }
    }










#ifdef EVENT_COMBAT    
    CREATE(fed, struct fight_event_data, 1);
    fed->ch = ch;
    fed->vict = vict;
    GET_FIGHT_EVENT(ch) = event_create(fight_event, fed, 1);
#endif



    FIGHTING(ch) = vict;
    //act("You engage in combat with $N.", FALSE, ch, 0, vict, TO_CHAR);

    //if (GET_POS(ch)==POS_RESTING || GET_POS(ch)==POS_SITTING)
    //act("$n stands up.", FALSE, ch, 0, 0, TO_ROOM);

    GET_POS(ch) = POS_FIGHTING;
    if (AFF_FLAGGED(ch, AFF_GROUP)) {
        for (k = ch->followers; k; k = k->next)
            if ((k->follower->in_room == ch->in_room) && !FIGHTING(k->follower) && vict != k->follower && (GET_POS(k->follower) > POS_SLEEPING) &&
                    ((AFF_FLAGGED(k->follower, AFF_GROUP) && PRF2_FLAGGED(k->follower, PRF2_AUTOASSIST))
                     ||
                     (IS_NPC(k->follower) && AFF_FLAGGED(k->follower, AFF_CHARM) && AFF_FLAGGED(k->follower, AFF_GROUP) &&
                      (PRF2_FLAGGED(ch, PRF2_AUTOASSIST) || (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_GANG_LEADER)))))) {
                act("$N assists you!", FALSE, ch, NULL, k->follower, TO_CHAR);
                act("$N assists $n.", FALSE, ch, NULL, k->follower, TO_NOTVICT);
                send_to_char("&GYou jump to the fight!&0\r\n", k->follower);
                set_fighting(k->follower, vict);
            }
        if (ch->master && MOB_FLAGGED(ch->master, MOB_GANG_LEADER) && ch->in_room==ch->master->in_room && !FIGHTING(ch->master) && (GET_POS(ch->master) > POS_SLEEPING)) {
            act("$N assists you!", FALSE, ch, NULL, ch->master, TO_CHAR);
            act("$N assists $n.", FALSE, ch, NULL, ch->master, TO_NOTVICT);
            send_to_char("&GYou jump to the fight!&0\r\n", ch->master);
            set_fighting(ch->master, vict);
        }
    }
}



/* remove a char from the list of fighting chars */
void            stop_fighting(struct char_data * ch)
{
    struct char_data *temp;

    if (ch == next_combat_list)
        next_combat_list = ch->next_fighting;

    REMOVE_FROM_LIST(ch, combat_list, next_fighting);
    ch->next_fighting = NULL;

    if (GET_FIGHT_EVENT(ch))
        //if (event_is_queued(GET_FIGHT_EVENT(ch)))
        event_cancel(GET_FIGHT_EVENT(ch));
    GET_FIGHT_EVENT(ch) = NULL;

    FIGHTING(ch) = NULL;
    GET_POS(ch) = POS_STANDING;
    if (AFF2_FLAGGED(ch, AFF2_BERSERK))
    {
        affect_from_char(ch, SKILL_BERSERK);
        send_to_char("You shake off the madness.\r\n", ch);
    }
    //if (IS_NPC(ch))
    //  GET_STYLE(ch)=0;
    remove_combat_affects(ch);
    if (FOL_MUGRAK(ch))
    	GET_HIT(ch)=MIN(GET_HIT(ch), GET_MAX_HIT(ch));
    update_pos(ch);
    if (IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_PET) && GET_POS(ch)>=POS_RESTING)
    	 GET_HIT(ch) = MAX(1, MIN(GET_HIT(ch) + hit_gain(ch), GET_MAX_HIT(ch))); 
    	 
    if (!IN_ARENA(ch))
    ch->pk_timer=MAX(30*PASSES_PER_SEC, ch->pk_timer);

}


void            make_corpse(struct char_data * ch, struct char_data * killer, int type)
{
    struct obj_data *corpse,
                *o,
                *next_obj,
                *head;
    struct obj_data *money;
    int             i;
    extern int      max_npc_corpse_time,
        max_pc_corpse_time;

    struct obj_data *create_money(int amount);

    if (DEAD(ch))
{
        logs("make_corpse: char dead before corpse", GET_NAME(ch));
        return;
    }
    corpse = create_obj();
    corpse->item_number = 0;
    corpse->in_room = NOWHERE;
    corpse->name = str_dup("corpse");

    if (!strstr(GET_NAME(ch), "he corpse") && !strstr(GET_NAME(ch), "he headless corpse")) {
        if (!type)
            sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
        else
            sprintf(buf2, "The headless corpse of %s is lying here.", GET_NAME(ch));
        corpse->description = str_dup(buf2);
        if (!type)
            sprintf(buf2, "the corpse of %s", GET_NAME(ch));
        else
            sprintf(buf2, "the headless corpse of %s", GET_NAME(ch));
        corpse->short_description = str_dup(buf2);
    } else {
        if (!type)
            sprintf(buf2, "%s is lying here.", GET_NAME(ch));
        else
            sprintf(buf2, "%s is lying here.", GET_NAME(ch));
        CAP(buf2);
        corpse->description = str_dup(buf2);
        if (!type)
            sprintf(buf2, "%s", GET_NAME(ch));
        else
            sprintf(buf2, "%s", GET_NAME(ch));
        corpse->short_description = str_dup(buf2);
    }
    corpse->orig_zone = world[ch->in_room].zone;
    GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
    GET_OBJ_WEAR(corpse) = ITEM_WEAR_TAKE;
    GET_OBJ_EXTRA(corpse) = ITEM_NODONATE;
    GET_OBJ_VAL(corpse, 0) = 0; /* You can't store stuff in a corpse */
    GET_OBJ_VAL(corpse, 2) = GET_LEVEL(ch); // level of the victim
    GET_OBJ_VAL(corpse, 3) = 1; /* corpse identifier */
    GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
    GET_OBJ_RENT(corpse) = 9999;
    GET_OBJ_COST(corpse) = 0;
    GET_OBJ_LEVEL(corpse)=GET_LEVEL(ch);


    corpse->attack_verb=str_dup(GET_NAME(ch));
    if (killer)
        corpse->action_description=str_dup(GET_NAME(killer));
    else
        corpse->action_description=str_dup("unknown cause");

    corpse->orig_value=0;
    if (IS_NPC(ch))
        GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
    else
    {
        if (!IN_ARENA(ch))
            GET_OBJ_TIMER(corpse) = max_pc_corpse_time;
        else
            GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
            
        corpse->orig_value=MAX(0, (GET_FAITH(ch)+120)/70);    // time in little ticks the corpse is protected by the deity
        if (corpse->orig_value)
        	SET_BIT((corpse)->obj_flags.extra_flags,ITEM_GLOW);
    }

    if (!IN_ARENA(ch) && type!=2)
    {

        /* transfer character's inventory to the corpse, but only if NPC is killed */
        if (IS_NPC(ch))
        {
            corpse->contains = ch->carrying;
            for (o = corpse->contains; o != NULL; o = o->next_content)
                o->in_obj = corpse;
            object_list_new_owner(corpse, NULL);

            /* transfer character's equipment to the corpse */
            for (i = 0; i < NUM_WEARS; i++)
                if (GET_EQ(ch, i))
                    obj_to_obj(unequip_char(ch, i), corpse);

            /* transfer gold */
            if (GET_GOLD(ch) > 0) {
                /* following 'if' clause added to fix gold duplication loophole */
                if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
                    money = create_money(GET_GOLD(ch));
                    obj_to_obj(money, corpse);
                }
                GET_GOLD(ch) = 0;
            }
            ch->carrying = NULL;
            IS_CARRYING_N(ch) = 0;
            IS_CARRYING_W(ch) = 0;
        }
        else  /* players die differently */
        {
            SET_BIT(PLR_FLAGS(ch), PLR_JUSTDIED);
            corpse->contains = ch->carrying;
            for (o = corpse->contains; o != NULL; o = o->next_content)
                o->in_obj = corpse;
            object_list_new_owner(corpse, NULL);
            if (!FOL_AMURON(ch) || (killer && !IS_NPC(killer)))
            for (i = 0; i < NUM_WEARS; i++)
                if (GET_EQ(ch, i))
                {
                    if (number(1, 100)>=ITEM_LEFT_CHANCE || IS_NPC(ch))
                        obj_to_obj(unequip_char(ch, i), corpse);
                    else if (GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_CONTAINER && number(1, 100)>=ITEM_LEFT_CHANCE)
                        obj_to_obj(unequip_char(ch, i), corpse);
                }

            /* transfer gold */
            if (GET_GOLD(ch) > 0) {
                /* following 'if' clause added to fix gold duplication loophole */
                if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
                    money = create_money(GET_GOLD(ch));
                    obj_to_obj(money, corpse);
                }
                GET_GOLD(ch) = 0;
            }
        }


    }
    if (type==1) {
        head = read_object(OBJ_HEAD, VIRTUAL, 0, GET_LEVEL(ch));
        sprintf(buf2, "the head of %s", GET_NAME(ch));
        head->short_description = str_dup(buf2);
        sprintf(buf2, "The head of %s is lying here.", GET_NAME(ch));
        head->description = str_dup(buf2);
        obj_to_room(head, ch->in_room);
        if (IN_ARENA(ch))
            GET_OBJ_TIMER(head) = max_npc_corpse_time;
    }
    else if (type==2)
    {
        head = read_object(22, VIRTUAL, 0, GET_LEVEL(ch)/3);
        sprintf(buf2, "the ashes of %s", GET_NAME(ch));
        head->short_description = str_dup(buf2);
        sprintf(buf2, "The ashes of what once used to be %s are spread all over.", GET_NAME(ch));
        head->description = str_dup(buf2);
        obj_to_room(head, ch->in_room);
    }
    if (killer && IS_NPC(ch) && MOB_FLAGGED(ch, MOB_NO_CORPSE) && (PRF_FLAGGED(killer, PRF_AUTOSAC) || FOL_URG(killer)) && type!=2) {
        global_no_timer=1;
        obj_to_room(corpse, ch->in_room);
        global_no_timer=0;

        if (!IS_NPC(killer) && PRF2_FLAGGED(killer, PRF2_AUTOLOOT) && ch != killer && !IN_ARENA(killer)) {
            do_get(killer, "all corpse", 0, 0);
        }
        if (!IS_NPC(killer))
        	do_sac(killer, "corpse", 0, 0);
    } else if (type!=2) { 
    	global_no_timer=1;
        obj_to_room(corpse, ch->in_room);
        global_no_timer=0;
        if (killer && !IS_NPC(killer) && PRF2_FLAGGED(killer, PRF2_AUTOLOOT) && ch != killer && !IN_ARENA(killer)) {
            do_get(killer, "all corpse", 0, 0);
        }
    }


}

/* When ch kills victim */
void            change_alignment(struct char_data * ch, struct char_data * victim)
{
    int factor=32;
    if (GET_REAL_ALIGNMENT(ch)==0)
        factor=48;
    //   GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) / 32;
    if (GET_REAL_ALIGNMENT(ch)==1000 && IS_GOOD(victim))
        factor=16;
    GET_ALIGNMENT(ch) = MAX(-1000, MIN(1000, GET_ALIGNMENT(ch)-(GET_ALIGNMENT(victim) / factor)));
}


char dcbuf[100];
int            death_cry(struct char_data * ch)
{
    int             door,
    was_in, i;
    int res=0;

    switch (number(0, 9)) {
    case 0:
        act("&GACK, $n falls dead to the ground.&0", FALSE, ch,0,0,TO_ROOM);
        res=1;
        break;
    case 1:
        act("&GYou freeze as $n wails a death cry.&0", FALSE, ch,0,0,TO_ROOM);
        break;
    case 2:
        act("&GYour blood becomes cold as $n dies.&0", FALSE, ch,0,0,TO_ROOM);
        break;
    case 3:
        act("&GYour heart races as you see $n fall to the ground... dead.&0", FALSE, ch,0,0,TO_ROOM);
        break;
    case 4:
        act("&GBlood spurts out of $n as $e hits the ground... dead.&0", FALSE, ch,0,0,TO_ROOM);
        res=1;
        break;
    case 5:
        act("&GDeath carries $n to the afterlife!&0",FALSE,ch,0,0,TO_ROOM);
        break;
    default:
        act("&GYour blood freezes as you hear $n's death cry.&0", FALSE, ch, 0, 0, TO_ROOM);
        break;
    }
    was_in = ch->in_room;

    for (door = 0; door < NUM_OF_DIRS; door++) {
        if (CAN_GO(ch, door)) {
            ch->in_room = world[was_in].dir_option[door]->to_room;
            switch (number(0, 13)) {
            case 0:
                act("You hear a loud thud, nearby!",FALSE,ch,0,0,TO_ROOM);
                break;
            case 1:
                act("Death cackles with glee... nearby!!", FALSE, ch,0,0,TO_ROOM);
                break;
            case 2:
                act("What was that...a death scream?", FALSE, ch,0,0,TO_ROOM);
                break;
            case 3:
                act("You hear death claiming another victim nearby.", FALSE, ch,0,0,TO_ROOM);
                break;
            case 4:
                act("Nearby...a body falls to the ground!", FALSE, ch,0,0,TO_ROOM);
                break;
            case 5:
                act("You hear a loud thud, nearby!",FALSE,ch,0,0,TO_ROOM);
                break;
            case 6:
                act("You hear a fierce struggle, nearby.",FALSE,ch,0,0,TO_ROOM);
                break;
            default:
                sprintf(dcbuf,"Your blood freezes as you hear someone's death cry coming from the %s.",dirs[rev_dir[door]]);
                act(dcbuf, FALSE, ch, 0, 0, TO_ROOM);
                break;
            }
            ch->in_room = was_in;
        }
    }
    return res;
}



void            raw_kill(struct char_data * ch, struct char_data * killer)
{
    int             bh = 0;

#ifdef EVENT_COMBAT    
    if (GET_FIGHT_EVENT(ch))
#else
    if (FIGHTING(ch))
#endif
        stop_fighting(ch);
    if (GET_HIT(ch)==-334)
        bh=2; // disintigrate
    if (AFF2_FLAGGED(ch, AFF2_BEHEAD))
        bh = 1;
    REMOVE_BIT(AFF2_FLAGS(ch), AFF2_BEHEAD);
    REMOVE_BIT(AFF2_FLAGS(ch), AFF2_STALK);



    if (IN_ARENA(ch))
    {
        struct obj_data *flag;
        if (HAS_RED(ch))
        {
            REMOVE_BIT(AFF3_FLAGS(ch), AFF3_HAS_RED);
            flag = read_object(FLAG_RED, VIRTUAL, 0, 0);
            global_no_timer=1;
            obj_to_room(flag, ch->in_room);
            global_no_timer=0;
        }
        if (HAS_BLUE(ch))
        {
            REMOVE_BIT(AFF3_FLAGS(ch), AFF3_HAS_BLUE);
            flag = read_object(FLAG_BLUE, VIRTUAL, 0, 0);
            global_no_timer=1;
            obj_to_room(flag, ch->in_room);
            global_no_timer=0;
        }
    }
    else if (!IS_NPC(ch) && GET_LEVEL(ch)>LEVEL_NEWBIE && !FOL_AZ(ch))
    {
        if (CHANCE(GET_SKILL(ch, SKILL_DODGE)))
            SET_SKILL(ch, SKILL_DODGE, MAX(1, GET_SKILL(ch, SKILL_DODGE)-1));
        if (CHANCE(GET_SKILL(ch, SKILL_PARRY)))
            SET_SKILL(ch, SKILL_PARRY, MAX(1, GET_SKILL(ch, SKILL_PARRY)-1));
        if (CHANCE(GET_SKILL(ch, SKILL_SHIELD)))
            SET_SKILL(ch, SKILL_SHIELD, MAX(1, GET_SKILL(ch, SKILL_SHIELD)-1));
        if (CHANCE(GET_SKILL(ch, SKILL_HIT)))
            SET_SKILL(ch, SKILL_HIT, MAX(1, GET_SKILL(ch, SKILL_HIT)-1));
        if (CHANCE(GET_SKILL(ch, SKILL_PIERCE)))
            SET_SKILL(ch, SKILL_PIERCE, MAX(1, GET_SKILL(ch, SKILL_PIERCE)-1));
        if (CHANCE(GET_SKILL(ch, SKILL_SLASH)))
            SET_SKILL(ch, SKILL_SLASH, MAX(1, GET_SKILL(ch, SKILL_SLASH)-1));
        if (CHANCE(GET_SKILL(ch, SKILL_POUND)))
            SET_SKILL(ch, SKILL_POUND, MAX(1, GET_SKILL(ch, SKILL_POUND)-1));

    }


    /* death_cry(ch); */


    if (ch->in_room)
        RM_BLOOD(ch->in_room) = MIN(RM_BLOOD(ch->in_room) + 1, 10);  // add blood to the room

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
        make_corpse(ch, killer, bh);
        extract_char(ch);
        if (IS_NPC(ch)) {
            act("$n's body glows brightly for an instant and then slowly fades away"
                ,FALSE, ch, 0, 0, TO_ROOM);
        } else
            act("$n is magically whisked away.", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, arena_room());
        send_to_char("\r\nYou are refreshed and ready to start over...\r\n", ch);
        act("$n magically appears, looking like $e had a bad day.", FALSE, ch, 0, 0, TO_ROOM);
        GET_HIT(ch) = GET_MAX_HIT(ch);
        //GET_MANA(ch) = GET_MAX_MANA(ch);
        //GET_MOVE(ch) = GET_MAX_MOVE(ch);
        GET_POS(ch) = POS_STANDING;

    } else {
        if (killer) {
            if (ch!=killer)
                mprog_death_trigger(ch, killer);
            rprog_death_trigger( killer, ch);


            if (IS_MOB(ch) && (PRF_FLAGGED(killer, PRF_AUTOSAC) || FOL_URG(killer)) && (ch->in_room == killer->in_room)) {
                SET_BIT(MOB_FLAGS(ch), MOB_NO_CORPSE);
            }
        }
        make_corpse(ch, killer, bh);
        /*if (killer &&  (number(1, 70)<GET_LEVEL(killer)) && IS_NPC(killer) && !MOB_FLAGGED(killer, MOB_PET))
    {
        do_get(killer,"all corpse",0,0);
        wear_all_suitable(killer);
    } */
        extract_char(ch);
    }
}

int grmaxlev(struct char_data *ch)
{
    int maxlev=GET_LEVEL(ch);
    struct follow_type *f;
    struct char_data *k, *tch, *next_tch;

    if (!(k = ch->master) && !AFF_FLAGGED(ch, AFF_GROUP))
        return GET_LEVEL(ch);
    else    {
        k=ch->master?ch->master:ch;
        for (f = k->followers; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_GROUP))
                maxlev=MAX(maxlev, GET_LEVEL(f->follower));
        return maxlev;
    }
}


void            die(struct char_data * ch, struct char_data * killer)
{

    if (!ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {





        if (!IS_NPC(ch)) {
            if (killer && (!IS_NPC(killer) || (killer->master && !IS_NPC(killer->master))) && killer!=ch)
            {
                //gain_exp(ch, GET_LEVEL(killer)>GET_LEVEL(ch)+10 ? 0 : -(exp_this_level(GET_LEVEL(ch)) / PLAYERS_PER_LEVEL));
                gain_exp(ch, LEVELDIFF(-LEVELEXP(ch) / PLAYERS_PER_LEVEL, grmaxlev(killer), GET_LEVEL(ch))); // mora obrnuto ovde jer sto je on veci nivo manje se gubi
            }
            else
            {
                //gain_exp(ch, -(exp_this_level(GET_LEVEL(ch)) / 4));
                int loss;
                if (FOL_AMURON(ch) && FAITH_STRONG(ch))
                    loss=-LEVELEXP(ch) / 5;
                else if (FOL_BOUGAR(ch))
                    loss=-LEVELEXP(ch) / 3;
                else
                    loss=-LEVELEXP(ch) / 4;
                gain_exp(ch, loss);
            }
            REMOVE_BIT(PLR_FLAGS(ch), PLR_KILLER | PLR_THIEF);
        }
    }


    raw_kill(ch, killer);

}

void            perform_group_gain(struct char_data * ch, int base,
                                   struct char_data * victim)
{
    int             share;
    int i, vnum, ratio=100;
    int dbl=0;      
    int meg;


    if (GET_POS(ch)<=POS_SLEEPING)
        return;

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
        ch->dam_exp = 0;
        return;
    }

    if (IS_NPC(ch))
    {   
    	 if (IS_AFFECTED(ch, AFF_CHARM))
        	base/=4;
        base=LEVELDIFF(base, grmaxlev(ch), GET_LEVEL(victim));
       
        GET_EXP(ch)+=MIN(max_exp_gain, MAX(0, base));        
        return;
    }
    //if (!IS_NPC(ch))  {
    /*if (GET_LEVEL(victim)<GET_LEVEL(ch))
        base+=8*base*(GET_LEVEL(victim)-GET_LEVEL(ch))/100;
    else
        base+=8*base*(GET_LEVEL(victim)-GET_LEVEL(ch))/100;*/

    if (IS_NPC(victim))
    {
        vnum=GET_MOB_VNUM(victim)/100;			// this is bonus for zones
        vnum*=100;

        for (i = 0; i <= top_of_zone_table; i++)
            if ((zone_table[i].number * 100 <= vnum) && (zone_table[i].top >= vnum))
            {
                vnum=GET_ROOM_VNUM(ch->in_room)/100;
                vnum*=100;
                if ((zone_table[i].number * 100 <= vnum) && (zone_table[i].top >= vnum))
                    ratio=zone_table[i].calc_ratio;
                break;
            }

        ratio=MAX(75, MIN(125, ratio));
        base=base*ratio/100;

        // base+=GET_EXP(victim);
    }

    if (ch->dam_exp<0)
    {
        dbl=1;
        ch->dam_exp=-ch->dam_exp;
    }
    base+=MIN(MOB_EXP_BASE/10, (ch->dam_exp)*(MOB_EXP_BASE/20));


    //     ne dobijes expa ako nisi ucestvovao u fajtu
    if (!(ch->dam_exp))
     base=MIN(2, base);
     
     
     // ako je kill izveden sa distance onda samo pola expa
     if (!FIGHTING(ch))
     	base/=2;     

    base=LEVELDIFF(base, grmaxlev(ch), GET_LEVEL(victim));
    
    

    if (GET_RACE(ch)==RACE_HUMAN)
        base=(11*base)/10;
        
        //nema potreba jer sad kastuju kroz event (do_cast()...)
    //if (IS_NPC(victim) && !IS_WARRIOR(victim))//MOB_FLAGGED(victim, MOB_SPEC))
      //  base=(120*base)/100;

    if (IS_NPC(victim))
    {
        int plev=GET_LEVEL(victim);
        plev=plev*plev/8+plev;
        base=MIN(140*base/100, ((GET_AC(victim))*15/plev+85)*base/100);

        if (GET_EQ(victim, WEAR_WIELD))
            base=110*base/100;
    }

    if (QUESTING(ch) && current_quest!=QUEST_NONE && quest_step==QUEST_ACTION)
    {
        if (current_quest==QUEST_GIFT && GET_LEVEL(victim)>GET_LEVEL(ch))
        {
            base*=4;
            sprintf(buf, "we have a winner - %s!", GET_NAME(ch));
            end_current_quest(buf);
        }
    }

    if (IS_NPC(victim) && GET_EXP(victim)>0)
        base+=LEVELDIFF(GET_EXP(victim), grmaxlev(ch), GET_LEVEL(victim));

    if (dbl)
        base*=2;
        
        

     base=modexp*base/10;
     
     		if (FOL_AMURON(ch) && FAITH_STRONG(ch))
                    meg=LEVELEXP(ch) / 10;
                else if (FOL_BOUGAR(ch))
                    meg=LEVELEXP(ch) / 6;
                else
                    meg=LEVELEXP(ch) / 8;


    share = MIN(meg, MAX(0, base));
    ch->dam_exp = 0;	//


    if (share==0)
    {
        send_to_char("\r\n&c*Yawn* That was a boring kill, wasn't it?&0\r\n", ch);
    }
    else if (share > 1) {
        sprintf(buf2, "\r\nYou receive your share of experience -- &c%d points.&0\r\n", share);
        send_to_char(buf2, ch);
    } else
        send_to_char("\r\nYou receive your share of experience -- one measly little point!\r\n", ch);

    gain_exp(ch, share);
    //change_alignment(ch, victim);
}


void            group_gain(struct char_data * ch, struct char_data * victim)
{
    int             tot_lev,
    tot_members,
    tot_pl = 0,
             maxlev=-1;
    unsigned int base;
    struct char_data *k;
    struct follow_type *f;

    if (!AFF_FLAGGED(ch, AFF_GROUP)) {



        if (IS_NPC(victim))
            //perform_group_gain(ch,(MOB_EXP(GET_LEVEL(victim)) + GET_EXP(victim)/2)*GET_RATIO(victim)/100, victim);
            perform_group_gain(ch,(MOB_EXP_BASE)*GET_RATIO(victim)/100, victim);
        else
        {
            int loss;

            if (FOL_AMURON(ch) && FAITH_STRONG(victim))
                loss=LEVELEXP(victim) / 10;
            else if (FOL_BOUGAR(victim))
                loss=LEVELEXP(victim) / 6;
            else
                loss=LEVELEXP(victim) / 8;

            if (!IS_NPC(ch) || (ch->master && !IS_NPC(ch->master)) && GET_LEVEL(victim)>LEVEL_NEWBIE)
                perform_group_gain(ch, GET_EXP(victim)>0? MIN(GET_EXP(victim),LEVELEXP(victim)/PLAYERS_PER_LEVEL):0, victim);
            else if (GET_LEVEL(victim)>LEVEL_NEWBIE)
                perform_group_gain(ch, (GET_EXP(victim)>0 ? MIN(GET_EXP(victim),loss):0), victim);
        }
    } else {

        if (!(k = ch->master))
            k = ch;
        tot_lev = 0;

        if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
            tot_members = 1;
            tot_lev = GET_LEVEL(k);
        } else
            tot_members = 0;

        for (f = k->followers; f; f = f->next)
            if (IS_AFFECTED(f->follower, AFF_GROUP)) {
                tot_lev += GET_LEVEL(f->follower);
                tot_members++;

            }
        if (IS_NPC(victim))
        {
            base=MOB_EXP_BASE;//MOB_EXP(GET_LEVEL(victim));
            base+=GET_EXP(victim)/2;
            base*=GET_RATIO(victim);
            base*=(10 + tot_members);
            base/=(11 * 100 * tot_lev);

        }
        else
        {
            if (!IS_NPC(ch) || (ch->master && !IS_NPC(ch->master)))
                base=(LEVELEXP(victim)/PLAYERS_PER_LEVEL)*(10 + tot_members) / (11 * tot_lev);
            else
                base=(LEVELEXP(victim)/4)*(10 + tot_members) / (11 * tot_lev);
        }
        if (!IS_NPC(k) && GET_SKILL(k, SKILL_LEADERSHIP) && k->in_room==ch->in_room)
        {
            base=base+(base*(GET_SKILL(k, SKILL_LEADERSHIP)+2*GET_CHA(k))/1000);  // valjda max 1.18 expa :P
            improve_skill(k, SKILL_LEADERSHIP, MAX(1, GET_LEVEL(k)-GET_LEVEL(victim)+5));
        }
        if (IS_AFFECTED(k, AFF_GROUP) && k->in_room == ch->in_room)
            perform_group_gain(k, base * GET_LEVEL(k), victim);

        for (f = k->followers; f; f = f->next)
            if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
                perform_group_gain(f->follower, base * GET_LEVEL(f->follower), victim);
    }

    if (PLR_FLAGGED(ch, PLR_QUESTOR) && IS_NPC(victim)) {
        if (GET_QUESTMOB(ch) == GET_MOB_VNUM(victim)) {
            send_to_char("&WYou have almost completed your QUEST!&0\r\n", ch);
            send_to_char("&WReturn to the questmaster before your time runs out!&0\r\n", ch);
            GET_QUESTMOB(ch) = -1;
        }
    }
}


char           *replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
    static char     buf[256];
    char           *cp;

    cp = buf;

    for (; *str; str++) {
        if (*str == '#') {
            switch (*(++str)) {
            case 'W':
                for (; *weapon_plural; *(cp++) = *(weapon_plural++));
                break;
            case 'w':
                for (; *weapon_singular; *(cp++) = *(weapon_singular++));
                break;
            default:
                *(cp++) = '#';
                break;
            }
        } else
            *(cp++) = *str;

        *cp = 0;
    }                           /* For */

    return (buf);
}
/*
static struct dam_weapon_type {
    char           *to_room;
    char           *to_char;
    char           *to_victim;
}               dam_weapons[] =

{

    
    {       
    	"$n's #w injures $N.",    // 5
        "Your #w &Pinjures&c $N.",    // 5
        "$n's #w injures you.",    // 5
    },
    {
    	"$n's #w wounds $N.",    // 5
        "Your #w &Pwounds&c $N.",    // 5
        "$n's #w wounds you.",    // 5
    },
    {                                   
    	"$n's #w maims $N.",   // 11
        "Your #w &Pmaims&c $N.",   // 11
        "$n's #w maims you.",   // 11
    },
    {
        "$n's #w decimates $N.", // 25
        "Your #w &Pdecimates&c $N.", // 25
        "$n's #w decimates you.", // 25
    },
    {
    	"$n's #w devastates $N.",  //17
        "Your #w &Pdevastates&c $N.",  //17
        "$n's #w devastates you.",  //17
    },
       
    {
    	"$n's #w DISMEMBERS $N.", //34
        "Your #w &PDISMEMBERS&c $N.", //34
        "$n's #w DISMEMBERS you!", //34
    },
    {
    	"$n's #w DEMOLISHES $N.", //34
        "Your #w &PDEMOLISHES&c $N.", //34
        "$n's #w DEMOLISHES you!", //34
    },

    {
    	"$n's #w * DISEMBOWELS * $N!", //34
        "Your #w &P* DISEMBOWELS *&c $N!", //34
        "$n's #w * DISEMBOWELS * you!!", //34
    },

    {
    	"$n's #w ** EVISCARATES ** $N's body!!", // 44
        "Your #w &P** EVISCARATES **&c $N's body!!", // 44
        "$n's #w ** EVISCARATES ** your body!!!", // 44
    },
    

    {
    	"$n's #w utterly *** ANNIHILATES *** $N!!!",
        "Your #w utterly &P*** ANNIHILATES ***&c $N!!!",
        "$n's #w utterly *** ANNIHILATES *** YOU!!!!",
    },
    {                      
    	"$n's #w does truly &W>>>> UNSPEAKABLE <<<<&0 things to $N!!!!",
        "Your #w does truly &W>>>> UNSPEAKABLE <<<<&0 things to $N!!!!",
        "$n's #w does truly &W>>>> UNSPEAKABLE <<<<&0 things to YOU!!!!!",
    }
};
  */
static struct dam_weapon_type {
    char           *to_room;
    char           *to_char;
    char           *to_victim;
}               dam_weapons[] =
    {

        /* use #w for singular (i.e. "slash") and #W for plural (i.e.
         * "slashes") */

        {
            "$n tries to #w $N, but misses.",	/* 0: 0     */
            "You try to #w $N, but miss.",
            "$n tries to #w you, but misses."
        },

        {
            "$n's #w tickles $N.",	/* 1: 1..2  */
            "Your #w tickles $N.",
            "$n's #w tickles you."
        },

        {
            "$n's #w scratches $N.",		/* 2: 3..4  */
            "Your #w scratches $N.",
            "$n's #w scratches you."
        },


        {
            "$n's #w grazes $N.", // 4: 7..10
            "Your #w grazes $N.",
            "$n's #w grazes you."
        },

        {
            "$n's #w injures $N.", // 4: 7..10
            "Your #w injures $N.",
            "$n's #w injures you."
        },
        {
            "$n's #w wounds $N.", // 4: 7..10
            "Your #w wounds $N.",
            "$n's #w wounds you."
        },

        {
            "$n's #w maims $N.", // 4: 7..10
            "Your #w maims $N.",
            "$n's #w maims you."
        },

        {
            "$n's #w decimates $N.",        // 14: 100..150
            "Your #w decimates $N.",
            "$n's #w decimates you."
        },

        {
            "$n's #w devastates $N.",        // 14: 100..150
            "Your #w devastates $N.",
            "$n's #w devastates you."
        },

        {
            "$n's #w MUTILATES $N.",  // 11: 36..44
            "Your #w MUTILATES $N.",
            "$n's #w MUTILATES you."
        },

        {
            "$n's #w DISEMBOWELS $N.",        // 14: 100..150
            "Your #w DISEMBOWELS $N.",
            "$n's #w DISEMBOWELS you."
        },

        {
            "$n's #w DISMEMBERS $N.",        // 14: 100..150
            "Your #w DISMEMBERS $N.",
            "$n's #w DISMEMBERS you."
        },


        {
            "$n's #w MASSACRES $N.",        // 14: 100..150
            "Your #w MASSACRES $N.",
            "$n's #w MASSACRES you."
        },

        {
            "$n's #w MANGLES $N!",        // 13: 50..100
            "Your #w MANGLES $N!",
            "$n's #w MANGLES you!"
        },


        {
            "$n's #w * PULVERIZES * $N!",        // 14: 100..150
            "Your #w * PULVERIZES * $N!",
            "$n's #w * PULVERIZES * you!"
        },

        {
            "$n's #w ** DEMOLISHES ** $N!",        // 13: 50..100
            "Your #w ** DEMOLISHES ** $N!",
            "$n's #w ** DEMOLISHES ** you!"
        },
        {
            "$n's #w *** DEVASTATES *** $N!!",        // 14: 100..150
            "Your #w *** DEVASTATES *** $N!!",
            "$n's #w *** DEVASTATES *** you!!"
        },

        {
            "$n's #w === OBLITERATES === $N!!",        // 14: 100..150
            "Your #w === OBLITERATES === $N!!",
            "$n's #w === OBLITERATES === you!!"
        },
        {
            "$n's #w >>> ANNIHILATES <<< $N!!",
            "Your #w >>> ANNIHILATES <<< $N!!",
            "$n's #w >>> ANNIHILATES <<< YOU!!",
        },
        {
            "$n's #w <<< ERADICATES >>> $N!!", // 44
            "Your #w <<< ERADICATES >>> $N!!", // 44
            "$n's #w <<< ERADICATES >>> YOU!!!", // 44
        },
        {
            "$n's mighty #w does ^^^ UNSPEAKABLE ^^^ things to $N!!!!",  // 16: > 250
            "Your mighty #w does ^^^ UNSPEAKABLE ^^^ things to $N!!!!",
            "$n's mighty #w does ^^^ UNSPEAKABLE ^^^ things to YOU!!!!",
        }



    };


void            damspell_message(int dam, struct char_data * ch, struct char_data * victim,
                                 int w_type)
{
    char           *buf;
    int             msgnum;


    if (ch==supermob)
        return;

    if (dam && (w_type != SPELL_POISON)) {
        if (dam == 0)
            msgnum = 0;
        else if (dam <= 1)
            msgnum = 1;
        else if (dam <= 3)
            msgnum = 2;
        else if (dam <= 7)
            msgnum = 3;
        else if (dam <= 12)
            msgnum = 4;
        else if (dam <= 20)
            msgnum = 5;
        else if (dam <= 30)
            msgnum = 6;
        else if (dam <= 40)
            msgnum = 7;
        else if (dam <= 50)
            msgnum = 8;
        else if (dam <= 65)
            msgnum = 9;
        else if (dam <= 80)
            msgnum = 10;
        else if (dam <= 100)
            msgnum = 11;
        else if (dam <= 130)
            msgnum = 12;
        else if (dam <= 170)
            msgnum = 13;
        else if (dam <= 220)
            msgnum = 14;
        else if (dam <= 300)
            msgnum = 15;
        else if (dam <= 400)
            msgnum = 16;
        else if (dam <= 500)
            msgnum = 17;
        else if (dam <= 600)
            msgnum = 18;
        else if (dam <= 800)
            msgnum = 19;
        else
            msgnum = 20;



        buf = replace_string(dam_weapons[msgnum].to_room, spells[w_type], spells[w_type]);
        if (dam!=0)
            //act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
            send_dam_message(ch, victim, buf);
        else
            send_miss_message(ch, victim, buf);


        /* damage message to damager */
        send_to_char(CCCYN(ch, C_CMP), ch);
        buf = replace_string(dam_weapons[msgnum].to_char, spells[w_type], spells[w_type]);
        if (!IS_NPC(ch))// && ((dam!=0) || (!IS_NPC(ch) && !PRF2_FLAGGED(ch, PRF2_NOSPAM))))
        {
            if (dam!=0 && PRF2_FLAGGED(ch, PRF2_DISPDAM))
                sprintf(buf, "%s &y(%d)&0", buf, dam);
            if (dam || SEEMISS_F(ch))
                act(buf, FALSE, ch, NULL, victim, TO_CHAR);
        }
        send_to_char(CCNRM(ch, C_CMP), ch);

        send_to_char(CCYEL(victim, C_CMP), victim);
        buf = replace_string(dam_weapons[msgnum].to_victim, spells[w_type], spells[w_type]);
        //if ((dam!=0) || (!IS_NPC(victim) && !PRF2_FLAGGED(victim, PRF2_NOSPAM)))
        if (dam || SEEMISS_E(victim))
            act(buf, FALSE, ch, NULL, victim, TO_VICT);

        send_to_char(CCNRM(victim, C_CMP), victim);
    }
}


/* message for doing damage with a weapon */
void            dam_message(int dam, struct char_data * ch, struct char_data * victim,
                            int w_type, struct obj_data * obj)
{
    char           *buf;
    int             msgnum, t;

    if (ch==supermob)
        return;
    w_type -= TYPE_HIT;         /* Change to base of table with text */

    if (dam == 0)
        msgnum = 0;
    else if (dam <= 1)
        msgnum = 1;
    else if (dam <= 3)
        msgnum = 2;
    else if (dam <= 7)
        msgnum = 3;
    else if (dam <= 12)
        msgnum = 4;
    else if (dam <= 20)
        msgnum = 5;
    else if (dam <= 30)
        msgnum = 6;
    else if (dam <= 40)
        msgnum = 7;
    else if (dam <= 50)
        msgnum = 8;
    else if (dam <= 65)
        msgnum = 9;
    else if (dam <= 80)
        msgnum = 10;
    else if (dam <= 100)
        msgnum = 11;
    else if (dam <= 130)
        msgnum = 12;
    else if (dam <= 170)
        msgnum = 13;
    else if (dam <= 220)
        msgnum = 14;
    else if (dam <= 300)
        msgnum = 15;
    else if (dam <= 400)
        msgnum = 16;
    else if (dam <= 500)
        msgnum = 17;
    else if (dam <= 600)
        msgnum = 18;
    else if (dam <= 800)
        msgnum = 19;
    else
        msgnum = 20;


    //if (dam == 0)
    //msgnum = 0;
    //t=dam;
    //dam=dam*100/MAX(1,GET_HIT(victim));
    /*
             if (dam <= 1)
            msgnum = 1;
        else if (dam <= 2)
            msgnum = 2;
        else if (dam <= 4)
            msgnum = 3;
        else if (dam <= 6)
            msgnum = 4;
        else if (dam <= 8)
            msgnum = 5;
        else if (dam <= 10)
            msgnum = 6;
        else if (dam <= 15)
            msgnum = 7;
        else if (dam <= 20)
            msgnum = 8;
        else if (dam <= 26)
            msgnum = 9;
        else if (dam <= 33)
            msgnum = 10;
        else if (dam <= 40)
            msgnum = 11;
        else if (dam <= 48)
            msgnum = 12;
        else if (dam <= 56)
            msgnum = 13;
        else if (dam <= 65)
            msgnum = 14;
        else if (dam <= 75)
            msgnum = 15;
        else if (dam <= 95)
            msgnum = 16;
        else
            msgnum = 17;
    */
    /*
    if (dam == 0)		msgnum = 0;
      else if (dam <= 2)    msgnum = 1;
      else if (dam <= 4)    msgnum = 2;
      else if (dam <= 7)    msgnum = 3;
      else if (dam <= 11)   msgnum = 4;
      else if (dam <= 16)   msgnum = 5;
      else if (dam <= 22)   msgnum = 6;
      else if (dam <= 29)   msgnum = 7;
      else if (dam <= 40)   msgnum = 8; 
      else			msgnum = 9;

    */

    /* damage message to onlookers */

    buf = replace_string(dam_weapons[msgnum].to_room,
                         attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
    if (dam)
        //act((buf), FALSE, ch, obj, victim, TO_NOTVICT);
        send_dam_message(ch, victim, buf);
    else
        send_miss_message(ch, victim, buf);
    /* damage message to damager */
    send_to_char(CCCYN(ch, C_CMP), ch);
    buf = replace_string(dam_weapons[msgnum].to_char,
                         attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
    //    if (GET_LEVEL(ch) >= LVL_GOD)
    //      sprintf(buf, "%s\r\nYou do %s%d%s points of damage.", buf, CCRED(ch, C_CMP), dam, CCYEL(ch, C_CMP));
    if (ch->desc)// && ((dam!=0) || (!IS_NPC(ch) && !PRF2_FLAGGED(ch, PRF2_NOSPAM))))
    {
        if (dam!=0 && PRF2_FLAGGED(ch, PRF2_DISPDAM))
            sprintf(buf, "%s &y(%d)&0", buf, dam);
        if (dam || SEEMISS_F(victim))
            act((buf), FALSE, ch, obj, victim, TO_CHAR);
    }
    send_to_char(CCNRM(ch, C_CMP), ch);

    /* damage message to damagee */
    send_to_char(CCYEL(victim, C_CMP), victim);
    buf = replace_string(dam_weapons[msgnum].to_victim,
                         attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
    /*    if (GET_LEVEL(victim) >= LVL_GOD)
            sprintf(buf, "%s  You got hit for %s%d%s points of damage.", buf, CCYEL(victim, C_CMP), dam, CCRED(victim, C_CMP));*/
    //if (dam!=0 ||  (!IS_NPC(victim) && !PRF2_FLAGGED(victim, PRF2_NOSPAM)))
    if (dam || SEEMISS_E(victim))
        act((buf), FALSE, ch, obj, victim, TO_VICT | TO_SLEEP);
    send_to_char(CCNRM(victim, C_CMP), victim);
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int             skill_message(int dam, struct char_data * ch, struct char_data * vict,
                              int attacktype, struct obj_data * weap)
{
    char            buf[256];
    int             i,
    j,
    nr;
    struct message_type *msg;


    if (ch==supermob)
        return;
    for (i = 0; i < MAX_MESSAGES; i++) {
        if (fight_messages[i].a_type == attacktype) {
            nr = number(1, fight_messages[i].number_of_attacks);
            //sprintf(buf, "nr: %d\r\n", nr);
            //send_to_char(buf, ch);
            //nr/=100;
            for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
                msg = msg->next;

            if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
                if (msg->god_msg.attacker_msg)
                    act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
                if (msg->god_msg.victim_msg)
                    act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
                if (msg->god_msg.room_msg)
                    act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
            } else if (dam != 0) {
                if (GET_POS(vict) == POS_DEAD) {
                    if (msg->die_msg.attacker_msg) {
                        send_to_char("&G", ch);
                        act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
                        send_to_char(CCNRM(ch, C_CMP), ch);
                    }
                    if (msg->die_msg.victim_msg) {
                        send_to_char("&R", vict);
                        act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
                        send_to_char(CCNRM(vict, C_CMP), vict);
                    }
                    if (msg->die_msg.room_msg)
                        act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
                } else {
                    if (msg->hit_msg.attacker_msg) {
                        send_to_char(CCCYN(ch, C_CMP), ch);
                        strcpy(buf, msg->hit_msg.attacker_msg);
                        //if (GET_LEVEL(ch) >= LVL_GOD && attacktype != 399)
                        //  sprintf(buf, "%s\r\nYou do %d damage.", buf,  dam);
                        if (PRF2_FLAGGED(ch, PRF2_DISPDAM) && IS_HIT(attacktype))
                            sprintf(buf, "%s &y(%d)&0", buf, dam);
                        
                        if (!PRF2_FLAGGED(ch, PRF2_DISPSDESC) || IS_HIT(attacktype))
                        	act(buf, FALSE, ch, weap, vict, TO_CHAR); 
                        send_to_char(CCNRM(ch, C_CMP), ch);
                    }

                    if (msg->hit_msg.victim_msg) {
                        send_to_char(CCYEL(ch, C_CMP), vict);
                        strcpy(buf, msg->hit_msg.victim_msg);
                        if (GET_LEVEL(vict) >= LVL_GOD && attacktype != 399)
                            sprintf(buf, "%s\r\nYou take %d damage.", buf, dam);
                        if (!PRF2_FLAGGED(vict, PRF2_DISPSDESC) || IS_HIT(attacktype))                        
                        act(buf, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
                        send_to_char(CCNRM(vict, C_CMP), vict);
                    }
                    if (msg->hit_msg.room_msg)
                        act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
                    if (!IS_HIT(attacktype))
                        damspell_message(dam, ch, vict, attacktype);
                }
            } else if (ch != vict) {    /* Dam == 0 */
                if (msg->miss_msg.attacker_msg) {// && SEEMISS_F(ch)){// && !IS_NPC(ch) && (!PRF2_FLAGGED(ch, PRF2_NOSPAM) || IS_SPELL(attacktype))) {
                    //                   send_to_char(CCYEL(ch, C_CMP), ch);
                    act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
                    //                    send_to_char(CCNRM(ch, C_CMP), ch);
                }
                if (msg->miss_msg.victim_msg) {// && SEEMISS_E(ch)){// && !IS_NPC(vict) && !PRF2_FLAGGED(vict, PRF2_NOSPAM)) {
                    //                    send_to_char(CCRED(vict, C_CMP), vict);
                    act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
                    //                    send_to_char(CCNRM(vict, C_CMP), vict);
                }
                if (msg->miss_msg.room_msg)
                    act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
                //send_miss_message( ch, vict,msg->miss_msg.room_msg);
            }
            return 1;
        }
    }
    if (IS_SPELL(attacktype) || attacktype==SKILL_MELEE)
        damspell_message(dam, ch, vict, attacktype);
    return 0;
}

void            check_fight(struct char_data * ch, struct char_data * victim)
{
    if (IS_SUPERMOB(ch) || IS_SUPERMOB(victim))
    {
        logs("SYSERR: check_fight %s is trying to enter fight with %s in room %d", GET_NAME(ch), GET_NAME(victim), GET_ROOM_VNUM(ch->in_room));
        return;
    }
    if (DEAD(ch) || DEAD(victim))
        return;
        
    if (ch->in_room!=victim->in_room)
    	return;	
    	
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    	return;
    
    if (victim != ch) {
        if (!(FIGHTING(ch)) && (GET_POS(ch) > POS_STUNNED)) {
            /*            // if ((attacktype >= 0))*/
            set_fighting(ch, victim);

            if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
                    !number(0, 5) && IS_AFFECTED(victim, AFF_CHARM) &&
                    (victim->master->in_room == ch->in_room)) {
                if (FIGHTING(ch))
                    stop_fighting(ch);
                hit(ch, victim->master, TYPE_UNDEFINED);
                return;
            }
        }
        if (!FIGHTING(victim) && /* attacktype >= 0 && */ (GET_POS(victim) > POS_STUNNED)) {

            set_fighting(victim, ch);
            if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
                    (GET_LEVEL(ch) < LVL_IMMORT))
                remember(victim, ch);

        }
    }
    if (victim->master == ch)
        stop_follower(victim);

    if (IS_AFFECTED(ch, AFF_INVISIBLE | AFF_HIDE))
        appear(ch);
    if (IS_AFFECTED(ch, AFF_FLYING))
        do_land(ch,"", 0 ,0);
}

void            damage(struct char_data * ch, struct char_data * victim, int dam,
                       int attacktype, struct obj_data * obj)
{
    int             exp, backdam=dam, prob;
    int             local_gold = 0, tdam, i, j, pom, mc	, orig_dam, dameq, percent;
    int 	    adam=0, plev=0;


    char            local_buf[256], lb1[20];

    bool            freeflag = FALSE, critic=FALSE;
    float fdam=dam, tdam1;
    struct obj_data *damobj;

    if (!ch || !victim || ch->in_room<2 || victim->in_room<2)
        return;

    if (GET_POS(victim) <= POS_DEAD) {
        logs("SYSERR: %s attempted to damage a corpse %s (%d)", GET_NAME(ch), GET_NAME(victim), attacktype);
        die(victim, ch);
        return;                 /* -je, 7/7/92 */
    }

    if (DEAD(ch) || DEAD(victim))
    {
        log("fight.c Corpse(s) engaged in fight");
        return;
    }

    if (IS_SUPERMOB(ch) || IS_SUPERMOB(victim))
    {
        logs("SYSERR: damage() %s is trying to enter fight with %s in room %d using %s", GET_NAME(ch), GET_NAME(victim), GET_ROOM_VNUM(ch->in_room), spells[attacktype]);
        return;
    }

    /* shopkeeper protection */
    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (AFF2_FLAGGED(ch, AFF2_PETRIFY) && !(attacktype == SKILL_BEHEAD))
        return;

    /*if (victim->master
    && ch!=victim->master
    && IS_SKILL(attacktype)
    && (AFF3_FLAGGED(victim->master, AFF3_GUARD))
    && (victim->in_room==victim->master->in_room)
    && ((GET_SKILL(victim->master, SKILL_GUARD)+4*GET_DEX(victim->master))>number(1,300)))
{
        //if (GET_MOVE(victim->master)<5*PERCENT(victim->master))
            //send_to_char("You are too tired to guard others.\r\n", ch);             
        
        if (GET_MOVE(victim->master)>2)
        {
            improve_skill(victim->master, SKILL_GUARD, 6);
            GET_MOVE(victim->master)-=2;
            switch (number(1, 3))
            {
                case 1: act("$n manages to guard you from a blow!", FALSE, victim->master, 0, victim, TO_VICT);
                        act("You rush down and take a blow for $N.", FALSE, victim->master, 0, victim, TO_CHAR);
                        act("$n manages to guard &N from a blow!", FALSE, victim->master, 0, victim, TO_NOTVICT);
                        break;
                case 2: act("$n runs down just in time to take a blow for you!", FALSE, victim->master, 0, victim, TO_VICT);
                        act("You quickly move and guard $N form a blow.", FALSE, victim->master, 0, victim, TO_CHAR);
                        act("$n runs down just in time to take a blow for $N!", FALSE, victim->master, 0, victim, TO_NOTVICT);
                        break;
                case 3: act("With a quick jump, $n appears in front of you, taking a hit!", FALSE, victim->master, 0, victim, TO_VICT);
                        act("You jump in front of $N saving $M from a hit.", FALSE, victim->master, 0, victim, TO_CHAR);
                        act("$n jumps in front of $N, taking a blow!", FALSE, victim->master, 0, victim, TO_NOTVICT);
                        break;
            }
            victim=victim->master;
        }
}
    */




    if (victim != ch) {
        if (!(FIGHTING(ch)) && (GET_POS(ch) > POS_STUNNED) && ch->in_room == victim->in_room) {
            if ((attacktype >= 0))
                set_fighting(ch, victim);

            if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
                    !number(0, 5) && IS_AFFECTED(victim, AFF_CHARM) &&
                    (victim->master->in_room == ch->in_room)) {
                if (FIGHTING(ch))
                    stop_fighting(ch);
                hit(ch, victim->master, TYPE_UNDEFINED);
                return;
            }
        }
        if (!FIGHTING(victim) && (attacktype >= 0) && (GET_POS(victim) > POS_STUNNED) && (ch->in_room == victim->in_room)) {
            set_fighting(victim, ch);
            if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
                    (GET_LEVEL(ch) < LVL_IMMORT))
                remember(victim, ch);
        }
    }
    if (victim->master == ch)
        stop_follower(victim);

    if (IS_AFFECTED(ch, AFF_INVISIBLE | AFF_HIDE))
        appear(ch);




    /*  if (!pk_allowed) {
        check_killer(ch, victim);
    */

    //sprintf(buf, "Pass #2: %d\r\n",dam);
    // send_to_char(buf, ch);


    orig_dam=dam;


    if (dam!=-334 && attacktype!=SPELL_POISON && attacktype!=SKILL_BEHEAD && attacktype!=SKILL_CUTTHROAT) {
        prob=IS_SPELL(attacktype) ? 101 : (obj ? (IS_OBJ_STAT(obj, ITEM_NODROP)?151:IS_OBJ_STAT(obj, ITEM_BLESS)?81:101):101);
        /*if (number(1, 101)<GET_SKILL(ch, SKILL_CRITICAL_HIT))
        	prob=25;
        if (GET_SKILL(ch, SKILL_FEINT) && (number(1, 201)<GET_SKILL(ch, SKILL_FEINT)+2*GET_INT(ch)+2*GET_DEX(ch)))
        	prob=25;	
        if (attacktype==SKILL_BACKSTAB &&  number(1, 101)<GET_SKILL(ch, SKILL_ASSASSINATE))
        	prob=8;*/

        /*if (GET_SKILL(ch, SKILL_FEINT) && (number(1, 1000)<GET_SKILL(ch, SKILL_FEINT)+2*GET_INT(ch)+2*GET_DEX(ch)))
    {
            		sprintf(buf, "Your &cfeint&0 tricks %s, there goes &wCRITICAL&0 hit...\r\n", GET_NAME(victim));
            		improve_skill(ch, SKILL_FEINT, 1);
            		dam+=dam/2;
    }
            	else	
        	*/

        if (dam >0 && !IS_NPC(ch) && !number(0, prob) && INT_CHECK(ch) && !INT_CHECK(victim) && number(1, 101)<GET_SKILL(ch, attacktype))
        {
            critic=TRUE;

            improve_skill(ch, SKILL_CRITICAL_HIT, 2);
            if (IS_SKILL(attacktype))
            {

                switch (number(1, 4))
                {
                case 1: sprintf(buf, "With a little luck on your side, you place a &wCRITICAL&0 hit to %s...\r\n...", GET_NAME(victim));break;
                case 2: sprintf(buf, "You focus yourself and go for a &wCRITICAL&0 hit...\r\n...");break;
                case 3: sprintf(buf, "You skillfully place a &wCRITICAL&0 hit to %s...\r\n...", GET_NAME(victim));break;
                case 4: sprintf(buf, "You find a weak spot and place a &wCRITICAL&0 hit...\r\n...");break;
                }


            }
            else if (IS_SPELL(attacktype))
                sprintf(buf, "Your %s produces a &wCRITICAL&0 effect...\r\n...", SORP(ch));
            if (attacktype!=TYPE_MISSILE)
            {
                send_to_char(buf, ch);
                act("$n places a CRITICAL hit.", FALSE, ch, 0,0,  TO_ROOM);
            }
            dam*=2;
        }
        else if (dam >0 && IS_NPC(ch) && !number(0, prob) && INT_CHECK(ch) && !INT_CHECK(victim) && number(1, 101)<GET_SKILL(ch, attacktype))
        {

            //act("&y", FALSE, ch, 0, 0, TO_ROOM);
            if (IS_SKILL(attacktype))
            {
                mc=number((GET_LEVEL(ch)>5?1:2), 4);
                switch (mc)
                {
                case 1: act("$n laughs out, '&cDodge this!&0'...", FALSE, ch, 0, victim, TO_VICT); break;
                case 2: act("$n uses a weakness in your flank and charges forward...", FALSE, ch, 0, victim, TO_VICT); break;
                case 3: act("You are amazed as $n manages to place a CRITICAL hit to you...", FALSE, ch, 0, victim, TO_VICT); break;
                case 4: act("$n makes a feint and you fall to it...", FALSE, ch, 0, victim, TO_VICT); break;
                }
            }
            else if (IS_SPELL(attacktype))
                act("$n's spell produces a CRITICAL effect...&0", FALSE, ch, 0, victim, TO_VICT);
            dam*=2;
            critic=TRUE;
        }
        
        
        if (critic && (GET_SKILL(victim, SKILL_SIXTHSENSE)>number(1,111)) && DEX_CHECK(victim))
        {
        	critic=FALSE;
        	improve_skill(victim, SKILL_SIXTHSENSE, 1);
        }

        //sprintf(buf, "Pass #3: %d\r\n",dam);
        //send_to_char(buf, ch);




        fdam=dam;
        tdam1=1;
        if (IS_SKILL(attacktype))//!=TYPE_SUFFERING)
        {

            if (IS_NPC(victim))
            {	
            	plev=GET_LEVEL(victim);
            	adam=MIN(GET_AC(victim), 9*(4*plev*plev/23+3*plev)/8);
            }
            else
            	adam=GET_AC(victim);
            	
	    fdam-=(float) adam/10.0;            	
            
            //fdam-=(float) GET_AC(victim)/10.0;

            dameq  = number(WEAR_LIGHT, NUM_WEARS-1);
            if (dameq <5 && dameq>8)
                dameq  = number(WEAR_LIGHT, NUM_WEARS-1);
            
            damobj = GET_EQ(victim, dameq);
            if ( damobj )
            {   
            	
                dameq=get_obj_resistance(damobj);
                tdam=MIN(4, dam/dameq);
                if (attacktype==SKILL_MISSILE)
                    damage_obj(victim, damobj, number(10, 15));
                if ( !number(0, 6-tdam) && dam >= dameq && !number(0, 3 +(IS_ORGANIC(damobj)?-1:0)+(IS_METAL(damobj)?1:0)))
                {
                    percent=MIN(40, MAX(1, (dam-dameq)*5/dameq));
                    if (IS_OBJ_STAT(damobj, ITEM_NODROP))
                        percent*=2;
                    else if (IS_OBJ_STAT(damobj, ITEM_BLESS))
                        percent/=2;
                    if (!CANBE_ARMOR(damobj))
            		percent=number(1,2);
                    damage_obj(victim, damobj, percent);
                }

                //fdam *= 0.95;  /* add a bonus for having something to block the blow */
            }
            // else
            //  fdam *= 1.05;  /* add penalty for bare skin! */

        }
        else if (IS_SPELL(attacktype))
            fdam-=GET_MAGAC(victim);
            
        
        //if (fdam<1 && orig_dam)
        	//fdam=MAX(100, MIN(orig_dam*100, GET_MAX_HIT(ch)))/100.0;		//minimum damage (1% max_hit)
        if (orig_dam)
        	if (fdam<(orig_dam/7.0))        	
        		fdam=(float) orig_dam/7.0;
        		//fdam=(float) MAX((int) fdam*100, orig_dam*10)/100.0 // armor sprecava 90% damage maximum
                                                          
                                                          
                                                          

        if (AFF_FLAGGED(victim, AFF_EQUINOX))
            fdam += number(0, 2*fdam);
        if (AFF2_FLAGGED(ch, AFF2_ULTRA))
            fdam *= 2;

        if (AFF3_FLAGGED(ch, AFF3_QUAD))
            fdam *= 4;

        //

        //if (!IS_NPC(ch))
        if (GET_SKILL(ch, SKILL_ENH_DAMAGE) && IS_WEAPON(attacktype) ) {
            fdam += fdam * (GET_SKILL(ch, SKILL_ENH_DAMAGE))  /500.0;  // poslednji bastion and is now gone *smirk*
            improve_skill(ch, SKILL_ENH_DAMAGE, 30);
        }

        if (IS_NPC(victim) && GET_SKILL(ch, SKILL_TACTICS) && (IS_SKILL(attacktype)) && (IS_NPC(ch) || IS_WEAPON(attacktype) )) {
            i=GET_MOB_RNUM(victim)/8;
            j=GET_MOB_RNUM(victim) % 8;
            pom=GET_KILLED_MOB(ch, i);
            if ((pom & (1<<j)))
            {
                fdam += fdam * (GET_SKILL(ch, SKILL_TACTICS)) /400.0;
                improve_skill(ch, SKILL_TACTICS, 20);
            }
        }




        if (AFF_FLAGGED(ch, AFF_DEATHDANCE))
        {
            fdam += fdam / 2;
            //if (IS_NPC(ch) && AFF_FLAGGED(victim, AFF_CHARM))
              //  fdam*=2;
        }




        if (AFF3_FLAGGED(ch, AFF3_INTESIFY) && IS_SPELL(attacktype))
            fdam *= 1.2;

        //sprintf(buf, "Pass #4: %.0f\r\n",fdam);
        //  send_to_char(buf, ch);

        /* Position  sitting  x 1.33 6*/
        /* Position  resting  x 1.66 5*/
        /* Position  sleeping x 2.00 4*/
        /* Position  stunned  x 2.33 3*/
        /* Position  incap    x 2.66 2*/
        /* Position  mortally x 3.00 1*/

        if (GET_POS(victim) < POS_FIGHTING && IS_SKILL(attacktype))
            fdam += fdam*(POS_FIGHTING - GET_POS(victim)) / 6;

        if (IS_SKILL(attacktype))//!=TYPE_SUFFERING)
        {

            if (IS_WEAPON(attacktype) || IS_BACKSTAB(attacktype))
                fdam*=str_app[GET_STR(ch)].todam;
          //  else if (IS_BACKSTAB(attacktype))
            //    fdam*=(str_app[GET_STR(ch)].todam+1)/2.0;

            if (GET_STYLE(ch)){
                if (GET_STYLE(ch)==2 && number(1,100)<GET_SKILL(ch, SKILL_AGGRESIVE))
                {
                    fdam*=1.1;
                    improve_skill(ch, SKILL_AGGRESIVE, 40);
                }
            }
            if (GET_STYLE(victim)){
                if (GET_STYLE(victim)==1 && number(1,100)<GET_SKILL(ch, SKILL_EVASIVE))
                {
                    fdam*=0.95;
                    improve_skill(victim, SKILL_EVASIVE, 40);
                }

            }
        }
        //sprintf(buf, "Pass #5: %f\r\n", fdam);
        //send_to_char(buf, ch);

            //if (AFF2_FLAGGED(victim, AFF2_STONESKIN) && obj && (GET_OBJ_VAL(obj, 3)== TYPE_PIERCE - TYPE_HIT))
//              fdam-=fdam/5;

        if (AFF3_FLAGGED(ch, AFF3_HOG) && IS_HIT(attacktype))
        {
            if (number(1, 100)<((GET_WIS(ch)*GET_WIS(ch))/50))
            {
                fdam*=dice(3,3);
                send_to_char("&WYou suddenly feel holy presence within you, giving you God-like strength ...&0\r\n****  ", ch);
            }
        }

        //    if (!IS_NPC(ch))
        //      dam = align_damage(ch, dam);
        //    else
        //  	if ((GET_LEVEL(ch)-GET_LEVEL(victim))>0)
        //dam+=(GET_LEVEL(ch)-GET_LEVEL(victim))*dam/64;


        //sprintf(buf, "(dam= %.0f)  %d attacktype=%d", fdam,attacktype);
        //send_to_char(buf,ch);
        //send_to_char(buf, victim);


        if (attacktype==SPELL_ENERGY_DRAIN)
            GET_HIT(ch)=MIN(GET_MAX_HIT(ch), GET_HIT(ch)+fdam/2.0);






        if (AFF3_FLAGGED(victim, AFF3_SHELTER) && IS_SPELL(attacktype))
            fdam -= fdam/4.0;

        if (AFF2_FLAGGED(victim, AFF2_RESISTMAGIC) && IS_SPELL(attacktype) && number(1, 111)<GET_SKILL(victim, SKILL_RESISTANCE))
        {
            fdam -= fdam/3.0;
            improve_skill(victim, SKILL_RESISTANCE, 2);
        }

        if (AFF2_FLAGGED(victim, AFF2_BERSERK) && IS_SPELL(attacktype))
        {
            fdam -= fdam/5.0;
        }


        if (IS_AFFECTED(victim, AFF_SILVER))
            fdam -= number(0, (int) fdam);

        //if (IS_AFFECTED(victim, AFF_SANCTUARY) && fdam >= 2)
        //  fdam /= 2;               /* 1/2 damage when sanctuary */
        if (FOL_VALERIA(victim))
            fdam-=MIN(10, fdam/5.0);


        if (AFF2_FLAGGED(victim, AFF2_PROTECTION))
            fdam-=MAX((victim->protection)/3.0, fdam*(victim->protection)/100);

	
	// impact of CON on damage	
	 fdam-=fdam*((float) GET_CON(ch)/18.0-1.0)/2.6;



        if (AFF2_FLAGGED(victim, AFF2_PRISM))
            fdam = 0;

        if (attacktype==SKILL_BACKSTAB && fdam>0)
        {
            if (number(1, 151)<GET_SKILL(ch, SKILL_ASSASSINATE))
            {
                act("&cYou do extra damage to $N!&0", FALSE, ch, 0, victim, TO_CHAR);
                fdam*=(float) ((float) (FOL_SKIG(ch)? 7.2 : 6.7)+(float) (GET_LEVEL(ch)-45)/3);
                improve_skill(ch, SKILL_ASSASSINATE, 2);
            }
            else
                fdam*=(float) ((float) (FOL_SKIG(ch)? 2.5 : 2.0)+(float) GET_LEVEL(ch)/15.0);
        }
        else if (attacktype==SKILL_CIRCLE)
            fdam*=(float) ((float) (FOL_SKIG(ch)? 2.5 : 2.0)+(float) GET_LEVEL(ch)/15.0)/2.0;



        if (obj && IS_SET(GET_OBJ_EXTRA2(obj), ITEM2_POISONED) && IS_SKILL(attacktype) && fdam>0)
        {
            if (!IS_AFFECTED(victim, AFF_POISON))
            {
                struct affected_type af;
                act("\r\n&cYou are poisoned!!&0", FALSE, ch, obj, victim, TO_VICT);
                act("\r\n&yYou poisoned $N!&0",FALSE, ch, 0, victim,TO_CHAR);
                act("$N is poisoned!",FALSE, ch,0,victim,TO_NOTVICT);

                apply_poison(victim, 0);

                //affect_to_char(victim, &af);
                fdam*=1.02+(float) GET_SKILL(ch, SKILL_ENVENOM)/800.0;
                improve_skill(ch, SKILL_ENVENOM, 2);

            }
            REMOVE_BIT(GET_OBJ_EXTRA2(obj), ITEM2_POISONED);
        }
        if (AFF2_FLAGGED(ch, AFF2_BERSERK) && fdam>1)
            improve_skill(ch, SKILL_BERSERK, 6);
            
            
            if (!IS_NPC(victim) && !IS_NPC(ch) && ch!=victim)
            	fdam*=0.8;	// ovo radimo da bi pc to pc fight trajao duze
            	

        
        //if (fdam<1 && orig_dam)
        //	fdam=MAX(10, GET_LEVEL(ch))/10.0;		//minimum one damage always    
        	
        dam=(int) (fdam+0.51); 




        dam = MAX(dam, 0);
        
        

        /* You can't damage an immortal! */
        if (!IS_NPC(victim) && IS_GOD(victim))
            dam=0;        
            
        if (FOL_MUGRAK(ch) && dam>=2)        
        	GET_HIT(ch)+=MIN(GET_HIT(victim), (dam+1)/3);
        	


        if (!IS_NPC(ch) && !IS_IMMORT(ch) && dam>0 )//&& GET_POS(victim)>POS_STUNNED)
            check_add_topdam(GET_NAME(ch), GET_NAME(victim), dam, attacktype, 1, GET_CLASS_NUM(ch));
        if (!IS_NPC(victim) && dam>0)// && GET_POS(victim)>POS_STUNNED)
            check_add_topdam(GET_NAME(victim), GET_NAME(ch), dam, attacktype, 2, GET_CLASS_NUM(victim));

        if (dam>3000)
        {

            sprintf(buf, "DAMAGE: %d by %s  (%s)", dam, GET_NAME(ch), IS_SPELL(attacktype) || IS_SKILL(attacktype) ? spells[attacktype]: "other");
            if (dam>3000)
                log(buf);
            dam=3000;
        }

    }
    /* skill_message sends a message from the messages file in lib/misc.
     * dam_message just sends a generic "You hit $n extremely hard.".
     * skill_message is preferable to dam_message because it is more
     * descriptive.
     * ACKS
     
     
     * If we are _not_ attacking with a weapon (i.e. a spell), always use
     * skill_message. If we are attacking with a weapon: If this is a miss or
     * a death blow, send a skill_message if one exists; if not, default to a
     * dam_message. Otherwise, always send a dam_message. */

    /*    if (!obj) {
            exp = MAX(39, MIN(56, attacktype + 40));
            obj = read_object(exp, VIRTUAL, 0, 0);        
            obj_to_room(obj, 0);
            exp = 0;
            freeflag = TRUE;
        }*/
    tdam=dam;//*100/MAX(1,GET_MAX_HIT(victim));



    if (dam!=-334)
    {
        GET_HIT(victim) -= dam;        // srce muda

        if (!ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
            if (ch != victim && GET_POS(victim)>POS_STUNNED)
                ch->dam_exp += dam;
    }
    else
    {
        GET_HIT(victim)=-334;
        if (!ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
            if (ch != victim)
                ch->dam_exp += GET_MAX_HIT(victim);
    }
    if ((attacktype == SKILL_BEHEAD) && dam > 0)
        GET_HIT(victim) = -100;


    update_pos(victim);

    if (GET_POS(ch)==POS_DEAD && FOL_VALERIA(ch) && FAITH_STRONG(ch) && !number(0, 9))
    {
        send_to_char("\r\n&BYou sense divine presence of your deity.&0\r\n\r\n", ch);
        act("&BBluish light surrounds $n.", FALSE, ch, 0, 0, TO_ROOM);
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        update_pos(ch);
    }


    if ((attacktype==SKILL_BACKSTAB || attacktype==SKILL_DUAL_BACKSTAB) && IS_NPC(victim))// && !IS_NPC(ch))
    {
        SET_BIT(MOB_FLAGS(victim), MOB_MEMORY);
        if ((GET_LEVEL(ch) < LVL_IMMORT))
            remember(victim, ch);
    }
    /*if (attacktype==SKILL_TRIP)
{
    	sprintf(buf, "dam: %d tdam:%d tdam1: %d\r\n", dam, tdam, tdam1);
    	send_to_char(buf, ch);
}
    */
    //sprintf(buf, "Pass #7: %d\r\n", dam);
    //   send_to_char(buf, ch);

    if (tdam1!=-888) //Armor absorbed damage
    {
        //if (dam>0 && tdam==0)
        //tdam=1;

        if (orig_dam && !dam && !IS_IMMORT(victim)) 	// damage was absorbed in the way
        {
            if (IS_WEAPON(attacktype))
                sprintf(buf, "Your %s deals no damage to $N.", attack_hit_text[attacktype-TYPE_HIT].singular);
            else
                sprintf(buf, "Your %s deals no damage to $N.", spells[attacktype]);
            act(buf, FALSE, ch, 0,  victim, TO_CHAR);
            if (IS_WEAPON(attacktype))
                sprintf(buf, "$n's %s deals no damage to you.", attack_hit_text[attacktype-TYPE_HIT].singular);
            else
                sprintf(buf, "$n's %s deals no damage to you.", spells[attacktype]);
            act(buf, FALSE, ch, 0,  victim, TO_VICT);
        }

        else if (!IS_WEAPON(attacktype))
            skill_message(tdam, ch, victim, attacktype, obj);
        else {
            if (GET_POS(victim) == POS_DEAD || dam == 0) {
                if (!skill_message(tdam, ch, victim, attacktype, obj))
                    dam_message(tdam, ch, victim, attacktype, obj);
            } else
                dam_message(tdam, ch, victim, attacktype, obj);
        }
        if (IS_WEAPON(attacktype) && obj && dam>(GET_MAX_HIT(victim)/4) && !number(0, 30))
        {
            SET_BIT(GET_OBJ_EXTRA2(obj), ITEM2_BLOODY);
            act("&RBlood spurts out from $N's wound!&0", FALSE, ch, obj, victim, TO_CHAR);
            act("&RBlood spurts out from $N's wound!&0", FALSE, ch, obj, victim, TO_NOTVICT);
            act("&RBlood spurts out from your wound!&0", FALSE, ch, obj, victim, TO_VICT);
        }
    }

    if (freeflag) {
        extract_obj(obj);
        freeflag = FALSE;
    }



    if (attacktype==SKILL_CIRCLE && IS_NPC(victim) && GET_POS(victim)>POS_STUNNED && FIGHTING(victim) && FIGHTING(victim)!=ch)
    {
        act("$n leaps to attack $N!", FALSE, victim, NULL, ch, TO_NOTVICT);
        act("$n leaps to attack you!", FALSE,  victim, NULL, ch, TO_VICT);
        stop_fighting(victim);;
        set_fighting(victim, ch);
        //FIGHTING(victim)=ch;
    }




    /* Use send_to_char -- act() doesn't send message if you are DEAD. */
    switch (GET_POS(victim)) {
    case POS_MORTALLYW:
        if (dam>0)
        {
            act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
            send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
        }
        break;
    case POS_INCAP:
        if (dam>0)
        {
            act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
            send_to_char("You are incapacitated and will slowly die, if not aided.\r\n", victim);
        }
        break;
    case POS_STUNNED:
        if (dam>0)
        {
            act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
            send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
        }
        break;
    case POS_DEAD:
        if (ROOM_FLAGGED(victim->in_room, ROOM_ARENA)) {
            //act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
            if (death_cry(victim) && attacktype!=SPELL_POISON)  // no blood from poison damage
            {
                for (i = 0; i < NUM_WEARS; i++) {
                    if (GET_EQ(ch, i) && !number(0, 5+NUM_WEARS-i))
                    {
                        SET_BIT(GET_OBJ_EXTRA2(GET_EQ(ch, i)), ITEM2_BLOODY);
                        break;
                    }
                }
            }

            if (RED(ch))
            {
                sprintf(buf2, "&RRED Team&0 scores a kill (%s killed %s)!", GET_NAME(ch), GET_NAME(victim));
                sportschan(buf2);
                arena_red++;
            }
            else if (BLUE(ch))
            {
                sprintf(buf2, "&BBLUE Team&0 scores a kill (%s killed %s)!", GET_NAME(ch), GET_NAME(victim));
                sportschan(buf2);
                arena_blue++;
            }

        } else {
            //act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
            if (death_cry(victim)  && attacktype!=SPELL_POISON)
            {
                int ffound=2;
                while (ffound>0)
                {
                    i=number(0, NUM_WEARS);
                    if (GET_EQ(ch, i) && !number(0, 5+NUM_WEARS-i))
                    {
                        SET_BIT(GET_OBJ_EXTRA2(GET_EQ(ch, i)), ITEM2_BLOODY);
                        ffound=0;
                    }
                    ffound--;
                }
            }
            if (attacktype==SPELL_DISINTIGRATE)
                GET_HIT(victim)=-334;
        }
        break;

    default:                    /* >= POSITION SLEEPING */
        if (dam > (GET_MAX_HIT(victim) / 4))
            send_to_char("&RThat really did HURT!&0\r\n",victim);
        else if (/*!MOB_FLAGGED(victim, MOB_SENTINEL) && */ !IS_SHOPKEEPER(victim) && attacktype!=SKILL_BACKSTAB && attacktype!=SKILL_CIRCLE && attacktype!=TYPE_MISSILE && dam>GET_TOTAL_WEIGHT(victim)/2 && !number(0, 20))
        {
            blow_out(victim);
            return;
        }
        else {
            if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 4)) {
                sprintf(buf2, "You wish that everything would stop &RHURTING&0 so much!\r\n");
                send_to_char(buf2, victim);
                if (GET_RACE(victim)==RACE_TROLL && !number(0, 7))
                {
                    act("&cA burst of acid sprays from one of your wounds onto $N!&0", FALSE, victim, 0, ch, TO_CHAR);
                    act("&cA burst of acid sprays from $N's wound hitting you!&0", FALSE, ch, 0, victim, TO_CHAR);
                    act("&cA burst of acid sprays from $N's wound hitting $n!&0", FALSE, ch, 0, victim, TO_NOTVICT);
                    GET_HIT(ch)-=MIN(GET_HIT(ch)-1, number(GET_LEVEL(victim), 2*GET_LEVEL(victim))+10);
                    /*sprintf(buf2, "a burst of %s's acid blood", GET_NAME(victim));
                    if (check_kill(ch, buf2))
                        return;*/
                }

                if (!AFF3_FLAGGED(victim, AFF3_FLEEING) && MOB_FLAGGED(victim, MOB_WIMPY) && (ch != victim) && GET_MOB_WAIT(victim)<=0)
                {
                    //CREF(victim, CHAR_NULL);
                    do_flee(victim, "", 0, 0);
                    // CUREF(victim);
                    if (DEAD(victim))
                        return;
                }
            }
            if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) && !AFF3_FLAGGED(victim, AFF3_FLEEING) &&
                    GET_HIT(victim) < GET_WIMP_LEV(victim) && GET_HIT(victim)>0 && !CHECK_WAIT(victim) && victim->in_room!=NOWHERE && GET_POS(victim)>POS_SITTING) {
                send_to_char("&cYou wimp out, and attempt to flee!&0\r\n", victim);
                //CREF(victim, CHAR_NULL);
                do_flee(victim, "", 0, 0);
                //CUREF(victim);
                if (DEAD(victim))
                    return;
            }
        }
        break;
    }

    /* Help out poor linkless people who are attacked */
    if (!IS_NPC(victim) && !(victim->desc) && victim->in_room!=NOWHERE && !AFF3_FLAGGED(victim, AFF3_FLEEING) ) {
        //CREF(victim, CHAR_NULL);
        do_flee(victim, "", 0, 0);
        //CUREF(victim);
        if (DEAD(victim))
            return;
        if (!FIGHTING(victim)) {
            act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
            GET_WAS_IN(victim) = victim->in_room;
            char_from_room(victim);
            char_to_room(victim, 1);
        }
    }
    /* stop someone from fighting if they're stunned or worse */
    if ((GET_POS(victim) <= POS_STUNNED) && (FIGHTING(victim) != NULL) && victim->in_room!=NOWHERE)
        stop_fighting(victim);

    /* Uh oh.  Victim died. */
    if (GET_POS(victim) == POS_DEAD && victim->in_room!=NOWHERE) {

        if (IS_NPC(victim)) {
            /*            sprintf(buf2, "Mob #%d %s (%d) killed by %s (%d)", GET_MOB_VNUM(victim), GET_NAME(victim), GET_LEVEL(victim), GET_NAME(ch), GET_LEVEL(ch));
                        mudlog(buf2, BRF, LVL_IMMORT, TRUE);*/

            if (!IS_NPC(ch))
            {
                i=GET_MOB_RNUM(victim)/8;
                j=GET_MOB_RNUM(victim) % 8;
                pom=GET_KILLED_MOB(ch, i);
                if (!(pom & (1<<j)))
                {
                    pom=pom | (1<<j);
                    GET_KILLED_MOB(ch, i)=pom;
                    send_to_char("+ You gain adventure points +\r\n", ch);
                    GET_QUESTPOINTS(ch)+=MAX(1, GET_LEVEL(victim)/2+2*(GET_LEVEL(victim)-GET_LEVEL(ch)));
                    if (FOL_AZ(ch))
                    {
                        send_to_char("Az rewards your progress.\r\n", ch);
                        ch->dam_exp=-ch->dam_exp-1;
                    }
                }

                ch->player_specials->saved.killed_mob++;
                ch->player_specials->saved.mkscore += GET_LEVEL(victim);//MAX(1, (GET_LEVEL(victim) + MAX(0, (GET_LEVEL(ch) - GET_LEVEL(victim)) * 2)) / 5);
                mobkills[GET_MOB_VNUM(victim)].killed++;
            }
        }
        if (!IS_NPC(victim)) {
            if (!ROOM_FLAGGED(victim->in_room, ROOM_ARENA)) {
                sprintf(buf2, "%s [%d] killed by %s [%d] at %s (%d)", GET_NAME(victim), GET_LEVEL(victim),
                        GET_NAME(ch), GET_LEVEL(ch),
                        world[victim->in_room].name, world[victim->in_room].number);
                if (IS_NPC(ch) || IS_MOB(ch))
                {
                    victim->player_specials->saved.killed_by_mob++;
                    npc_brag(ch, victim);
                    mobkills[GET_MOB_VNUM(ch)].pkills++;
                }
                else {
                    if (ch != victim) {
                        char buf5[2000];
                        victim->player_specials->saved.killed_by_player++;
                        ch->player_specials->saved.killed_player++;
                        ch->player_specials->saved.pkscore += GET_LEVEL(victim);
                        sprintf(buf5, "PKILL: %s", buf2);
                        log(buf5);
                    }
                }

                mudlog(buf2, BRF, LVL_IMMORT, TRUE);
                if (MOB_FLAGGED(ch, MOB_MEMORY))
                    forget(ch, victim);
                if (attacktype == TYPE_SUFFERING && IS_AFFECTED(victim, AFF_POISON))
                    sprintf(buf2, "\r\n&BINFO || &R%s killed by poison.&0\r\n", GET_NAME(victim));
                else if (attacktype == TYPE_SUFFERING)
                    sprintf(buf2, "\r\n&BINFO || &R%s suffers and slowly bleeds to death.&0\r\n", GET_NAME(victim));
                else if (attacktype == SPELL_POISON)
                    sprintf(buf2, "\r\n&BINFO || &R%s killed by poison.&0\r\n", GET_NAME(victim));
                else
                {
                    if (!IS_NPC(ch) && !IS_NPC(victim) && GET_CLAN(ch)!=CLAN_NEWBIE)
                    {
                        sprintf(buf2, "\r\n&WPKILL || &C%s killed %s.&0\r\n",  GET_NAME(ch),GET_NAME(victim));
                        clan[GET_CLAN(ch)].kills++; clan[GET_CLAN(ch)].score+=GET_LEVEL(victim);
                        save_clans();
                    }
                    else
                        sprintf(buf2, "\r\n&BINFO || &C%s killed by %s&0.\r\n", GET_NAME(victim), GET_NAME(ch));

                }
                INFO_OUT(buf2);
            }
        }
        if (!IS_NPC(ch) && ch != victim) {
            local_gold = GET_GOLD(victim);
            sprintf(local_buf, "%ld", (long) local_gold);
                                                         
	if (!IN_ARENA(ch))                                                         
            ch->pk_timer=ch->pk_timer=MAX(ch->pk_timer, IS_NPC(victim)?30*PASSES_PER_SEC:300*PASSES_PER_SEC);
        }
        if (IS_NPC(victim) || (victim->desc && (ch != victim)))
        {
            if (attacktype==SPELL_RAGNAROK)
            {
                GET_EXP(victim)+=ch->dam_exp;
                ch->dam_exp=0;
            }
            if (attacktype == TYPE_SUFFERING)
            {
                struct char_data *pom;
                pom=get_fightning_with(victim);
                if (pom)
                    group_gain(pom, victim);
            }
            else
                group_gain(ch, victim);
        }


        if ((MOB_FLAGGED(ch, MOB_ASSASSIN)) && (HUNTING(ch) == victim)) {
            if (HIRED_BY(ch))
                act("$n dissapears in a hurry.", FALSE, ch, NULL, NULL, TO_ROOM);
            {char bb[100];
                sprintf(bb,"%s I have succeeded.", GET_NAME(HIRED_BY(ch)));
                do_tell(ch,bb,0,0);
            }
            char_from_room(ch);
            char_to_room(ch, real_room(8245));
            HIRED_BY(ch)=NULL;
            HUNTING(ch)=0;
            if (GET_UTIL_EVENT(ch))
                event_cancel(GET_UTIL_EVENT(ch));
            GET_UTIL_EVENT(ch)=0;
        }
        if (MOB_FLAGGED(victim, MOB_ASSASSIN) && HIRED_BY(victim))
            send_to_char("Your assassin has failed.\r\n", HIRED_BY(victim));
        die(victim, ch);
        /*        if (!IS_NPC(ch) && PRF2_FLAGGED(ch, PRF2_AUTOLOOT) && ch != victim && !IN_ARENA(ch) && local_gold>0) {
        	    do_get(ch, "coin corpse", 0, 0);
                } */
        /* If Autoloot enabled, get all corpse */
        /*        if (!IS_NPC(ch) && PRF2_FLAGGED(ch, PRF2_AUTOLOOT) && !PRF_FLAGGED(ch, PRF_AUTOSAC) && ch != victim && !IN_ARENA(ch)) {
                    do_get(ch, "all corpse", 0, 0);
                }
        */	        
        /* If Autoloot AND AutoSplit AND we got money, split with group */
        if (IS_AFFECTED(ch, AFF_GROUP) && (local_gold > 1) &&
                PRF2_FLAGGED(ch, PRF2_AUTOSPLIT) && PRF2_FLAGGED(ch, PRF2_AUTOLOOT)/* && (!(PRF_FLAGGED(ch, PRF_AUTOSAC) || FOL_URG(ch)))*/ && ch != victim) {
            do_split(ch, local_buf, 0, 0);
        }
    }
}

extern int      thaco[NUM_CLASSES][LVL_IMPL + 1];
extern int      thaco1[NUM_CLASSES][2];
extern byte     backstab_mult[];



struct def_messages {
    char           *to_char;
    char           *to_victim;
}               defm[] =
    {

        {
            "$N barely dodges your attack.",
            "You barely dodge $n's attack."
        },
        {
            "$N dodges your attack.",
            "You dodge $n's attack."
        },
        {
            "$N easily dodges your attack.",
            "You easily dodge $n's attack."
        },
        {
            "$N barely parries your attack.",
            "You barely parry $n's attack."
        },
        {
            "$N parries your attack.",
            "You parry $n's attack."
        },
        {
            "$N easily parries your attack.",
            "You easily parry $n's attack."
        },
        {
            "$N barely blocks your attack.",
            "Your barely block $n's attack."
        },
        {
            "$N blocks your attack with $S shield.",
            "You block $n's attack with your shield."
        },
        {
            "$N easilly blocks your attack.",
            "You easilly block $n's attack."
        }
    };



// the heart of the combat system, check_hit tests if a person hits the enemy in combat
int check_hit(struct char_data * ch, struct char_data * victim, int type, struct obj_data *wielded)
{
    int tp, what,  chance, res;
    int mdex=0;
    float att1, def1,  att, def, koef;
    float pom, pom1, pom2;
    float dexk;
    int gc=0;

    if (!ch || !victim)
        return 0;


    if (GET_POS(victim)<=POS_SLEEPING)
        return 1;                         // stunned on the ground or worse - always hit

    if (AFF3_FLAGGED(victim, AFF3_FLEEING))
        return 1;                    // always hit fleeing chars


    if (!wielded)
        type=TYPE_HIT;

    // now form a basic attack roll. get skill with weapon, add double the hitroll bonus
    // and deduct drunkness if any
    att=GET_SKILL(ch, type)+2*GET_HITROLL(ch)-2*GET_COND(ch, DRUNK);

    // if a charater is wielding a weapon test his skill vs. weapons level
    if (wielded)
    {
        att1= ((GET_SKILL(ch, type)-5)/2-(MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(wielded))));
        if (att1>0)
            att1=att1*att1/5;
        else
            att1=-(att1*att1/5);

        att+=MAX(-300, MIN(50, (int) (10.0*att1)));        
    }

    // if char is doing something (reciting a scroll, wearing eq) deduct from his attack
    if (IN_EVENT(ch))
        att-=number(30, 40);


    def=-1;
    tp=0;


    if (GET_RACE(ch)==RACE_TROLL && type==TYPE_POUND)   // trolls hit better with pounding weapons
        att+=10;
    else if (GET_RACE(ch)==RACE_ORC && type==TYPE_SLASH)	// orcs slash better
        att+=10;
    else if (GET_RACE(ch)==RACE_DWARF && world[IN_ROOM(ch)].sector_type == SECT_INSIDE) // dwarves fight better undergorund or inside
        att+=10;


    if (IS_AFFECTED(ch, AFF_BLIND) || AFF3_FLAGGED(ch, AFF3_TEMP_BLIND))	// test if char is blinded
    {
        if (number(1, 101)<GET_SKILL(ch, SKILL_BLINDFIGHTING))
            improve_skill(ch, SKILL_BLINDFIGHTING, 1);
        else
            att-=number(35, 55);
    }

    // effect of encumberance on hitting the enemy
    att-=50*((IS_EQUIP_W(ch)+IS_CARRYING_W(ch))/CAN_CARRY_W(ch))*((IS_EQUIP_W(ch)+IS_CARRYING_W(ch))/CAN_CARRY_W(ch));


    // ok we have the attack roll. now determine how the victim will defend
    //   1..450  - dodge
    // 450..800  - parry
    // 800..1000 - shield
    what=number(1,1000);

    // if victim knows shield mastery, its more likely it will use a shield
    if (GET_EQ(victim, WEAR_SHIELD) && number(1, 400)<GET_SKILL(victim, SKILL_SHIELDMASTERY))
        what=900;

    // if victim knows dodge mastery its more likely it will try to dodge
    if (number(1, 800)<GET_SKILL(victim, SKILL_DODGEMASTERY))
        what=100;

    // people tend to dodge more if unarmed
    if (!GET_EQ(victim, WEAR_WIELD) && !number(0, 7))
        what=100;
    if (!GET_EQ(victim, WEAR_SHIELD) && !number(0, 7))
        what=100;


    if (what<=400) {      // ok lets dodge

        def=GET_SKILL(victim, SKILL_DODGE);	// add basic skill
        if (GET_RACE(victim)==RACE_ELF || GET_RACE(victim)==RACE_DROW)
            def+=10;	// elves have innate dodge ability
        def+=MAX(1, (GET_SKILL(victim, SKILL_DODGEMASTERY)-40)/2);
        if (!IS_NPC(victim) && PRF2_FLAGGED(victim, PRF2_TUMBLE))
            def+=GET_SKILL(victim, SKILL_TUMBLE)/5.0;
        tp=0;
    }
    else if (what<=750)  // parry
    {
        if (GET_EQ(victim, WEAR_WIELD) && !GET_SPELL_EVENT(victim))  // if have something to parry with
        {
            def1=GET_SKILL(victim, SKILL_PARRY)+GET_SKILL(victim, GET_OBJ_VAL(GET_EQ(victim, WEAR_WIELD),3)+TYPE_HIT)/2.0;
            att1=GET_SKILL(ch,type)/2.0;

            def=def1-att1+10;
            tp=1;
        }  
        else if IS_MONK(victim)
        {
       	    def1=GET_SKILL(victim, SKILL_PARRY)+GET_SKILL(victim, TYPE_HIT)/2.0;
            att1=GET_SKILL(ch,type)/2.0;

            def=def1-att1+10;
            tp=1;
        }
        else 	// else try to dodge with half the success
        {
            def=GET_SKILL(victim, SKILL_DODGE);
            if (GET_RACE(victim)==RACE_ELF || GET_RACE(victim)==RACE_DROW)
                def+=10;		// ovo prebaciti u skill na startu?
            def+=MAX(1, (GET_SKILL(victim, SKILL_DODGEMASTERY)-40)/2);
            if (!IS_NPC(victim) && PRF2_FLAGGED(victim, PRF2_TUMBLE))
                def+=GET_SKILL(victim, SKILL_TUMBLE)/5.0;

            def=3*def/4-10;	// but with a penalty
            tp=0;
        }

    }
    else {		// shield blcok
        if (GET_EQ(victim, WEAR_SHIELD) && !GET_SPELL_EVENT(victim))	// if have a shield
        {
            def=GET_SKILL(victim, SKILL_SHIELD)+20;
            def+=(2*GET_SKILL(victim, SKILL_SHIELDMASTERY)/3);
            tp=2;
        }
        else if (GET_EQ(victim, WEAR_WIELD) && !GET_SPELL_EVENT(victim))	// else try to parry
        {
            def1=GET_SKILL(victim, SKILL_PARRY)+GET_SKILL(victim, GET_OBJ_VAL(GET_EQ(victim, WEAR_WIELD),3)+TYPE_HIT)/2.0;
            att1=GET_SKILL(ch,type)/2.0;

            def=def1-att1+10;
            def=3*def/4-10;	// with a penalty
            tp=1;
        }
         else if IS_MONK(victim)
        {
       	    def1=GET_SKILL(victim, SKILL_PARRY)+GET_SKILL(victim, TYPE_HIT)/2.0;
            att1=GET_SKILL(ch,type)/2.0;

            def=def1-att1+10;
            tp=1;
        }
        else
        {       // or worse case try to dodge
            def=GET_SKILL(victim, SKILL_DODGE);
            if (GET_RACE(victim)==RACE_ELF || GET_RACE(victim)==RACE_DROW)
                def+=10;		// ovo prebaciti u skill na startu?
            def+=MAX(1, (GET_SKILL(victim, SKILL_DODGEMASTERY)-40)/2);
            if (!IS_NPC(victim) && PRF2_FLAGGED(victim, PRF2_TUMBLE))
                def+=GET_SKILL(victim, SKILL_TUMBLE)/5.0;

            def=3*def/5-15;	// with even some more penaly
            tp=0;
        }

    }
    // if busy doing something, deduct more
    if (IN_EVENT(victim))
        def-=number(30, 40);


    if (GET_POS(victim)<POS_FIGHTING)	// on the ground?
    {
        if (number(1, 126)>GET_SKILL(victim, SKILL_GROUNDCONTROL)+GET_DEX(victim))	// deduct from defense
            def-=number(30, 40);
        else
        {   	// unless ground control skill rolls in

            def+=GET_SKILL(victim, SKILL_GROUNDCONTROL)/3.0;
            gc=1;
        }
    }

    if (IS_AFFECTED(victim, AFF_BLIND) || AFF3_FLAGGED(victim, AFF3_TEMP_BLIND)) // blinded?
    {
        if (number(1, 101)<GET_SKILL(victim, SKILL_BLINDFIGHTING))
            improve_skill(victim, SKILL_BLINDFIGHTING, 1);
        else
            def-=number(30, 50);
    }

    def+=GET_SKILL(victim, SKILL_ARTOFWAR)/4.0;	// art of war adds defense bonus

    def-=MAX(0, num_fighting(victim)-1)*10; 	// overhelmed by enemies

    if (AFF2_FLAGGED(victim, AFF2_BERSERK))	// berserk people care less for safety
        def-=number(40, 50);


    // encumberance effect to defense roll
    def-=50*((IS_EQUIP_W(victim)+IS_CARRYING_W(victim))/CAN_CARRY_W(victim))*((IS_EQUIP_W(victim)+IS_CARRYING_W(victim))/CAN_CARRY_W(victim));


    chance=number(1, 100);

    /*sprintf(buf,"%s hits: %s att= %.0f   def= %.0f   f(att, def)= %0.f   chance= %d\r\n", GET_NAME(ch), (tp==2?"shield":(tp==1?"parry":"dodge")), att, def, ((att-def)+100)/2, chance);
    send_to_char(buf, ch);
    send_to_char(buf, victim);
             */ 

    // normalize results
    koef=100-MIN(att, def);
    att+=koef;
    def+=koef;

    // adjust results by DEX factor
    mdex=18-MIN(GET_DEX(ch), GET_DEX(victim));
    dexk=(((float) (GET_DEX(ch)+mdex)/(float) (GET_DEX(victim)+mdex)));
    def/=dexk;

    // the more you are hurt, worser you fight
    pom1=100.0*((float) GET_HIT(ch)/GET_MAX_HIT(ch));
    pom2=100.0*((float) GET_HIT(victim)/GET_MAX_HIT(victim));
    pom=1.0+(pom2-pom1)/256.0;

    def*=pom;


    pom=MAX(abs(att), abs(def));

    pom/=100.0;
    att=(float) att/pom;
    def=(float) def/pom;


    /*sprintf(buf,"%s hits: %s att= %.0f   def= %.0f   f(att, def)= %0.f   chance= %d\r\n", GET_NAME(ch), (tp==2?"shield":(tp==1?"parry":"dodge")), att, def, ((att-def)+100)/2, chance);
    send_to_char(buf, ch);
    send_to_char(buf, victim);
             */    


    what=tp;


    if (chance <3)
        res=1;  // 2% chance of a lucky hit
    else if (chance>98)
        res=0;  // 2% chance of fumble
    else {

        att1=((att-def)+100)/2.0;
        koef=(float) att1/3.34;
        // bonuses for style
        if (GET_STYLE(ch)==2 && number(1,111)<GET_SKILL(ch, SKILL_AGGRESIVE))
            att1+=koef*0.75;
        else if (GET_STYLE(ch)==1)
            att1-=koef;
        if (GET_STYLE(victim)==2)
            att1+=koef;
        else if (GET_STYLE(victim)==1 && number(1,111)<GET_SKILL(victim, SKILL_EVASIVE))
            att1-=koef;

        def1=chance;
        if (att1>=def1)
        {
            res=1;
            improve_combat_skill(ch, type);
        }
        else
        {                                                         

            if (GET_SKILL(ch, SKILL_FEINT) && (number(1, 1250)<GET_SKILL(ch, SKILL_FEINT)+2*GET_INT(ch)+2*GET_DEX(ch)))
            {
            	chance=number(1, 100);
            	if (att1>=chance)
        	{               			 
        		send_to_char("You make a feint!\r\n",ch);
        		improve_skill(ch, SKILL_FEINT, 2);    	            		
            		return 1;
        	}
            	
            }
            
            if (def1-att1<10) // barely
                res=0;
            else if (def1-att1<60) // easily
                res=1;
            else
                res=2;

            if (gc)
                improve_skill(victim, SKILL_GROUNDCONTROL, 1);
            if (GET_SKILL(victim, SKILL_ARTOFWAR))
                improve_skill(victim, SKILL_ARTOFWAR, 15);
            improve_combat_skill(victim, SKILL_DODGE+what);
            if (tp==2 && GET_SKILL(victim, SKILL_SHIELDMASTERY))
                improve_skill(victim, SKILL_SHIELDMASTERY, 7);
            if (tp==0 && GET_SKILL(victim, SKILL_DODGEMASTERY))
                improve_skill(victim, SKILL_DODGEMASTERY, 15);
            if (SEEMISS_F(ch))
                act(defm[what*3+res].to_char, FALSE, ch, 0, victim, TO_CHAR);
            if (SEEMISS_E(victim))
                act(defm[what*3+res].to_victim, FALSE, ch, 0, victim, TO_VICT);

            if (tp==1  && GET_EQ(victim, WEAR_WIELD) && !number(0, 30+(IS_ORGANIC(GET_EQ(ch, WEAR_WIELD))?-15:0)+(IS_METAL(GET_EQ(ch, WEAR_WIELD))?15:0)+GET_OBJ_DAMAGE(GET_EQ(victim, WEAR_WIELD))))
                damage_obj(ch, GET_EQ(ch, WEAR_WIELD), 1);
            else if (tp==2 && !number(0,30+GET_OBJ_DAMAGE(GET_EQ(victim, WEAR_SHIELD))))
                damage_obj(victim, GET_EQ(ch, WEAR_SHIELD), 1);

            //if parry, check for counterstrike
            if ((tp==1) && GET_EQ(victim, WEAR_WIELD) && GET_SKILL(victim, SKILL_COUNTERSTRIKE) && number(1, 350)<GET_SKILL(victim, SKILL_COUNTERSTRIKE)+GET_DEX(victim))
            {

                ch_printf(ch, "%s counterstrikes!\r\n", CAP(GET_NAME(victim)));
                send_to_char("You counterstrike!\r\n", victim);
                improve_skill(victim, SKILL_COUNTERSTRIKE, 5);
                hit(victim, FIGHTING(victim), TYPE_UNDEFINED);
            }
            res=-1;
        }
    }
    return res;
}


void            hit(struct char_data * ch, struct char_data * victim, int type)
{
    struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
    int             w_type,
    victim_ac,
    calc_thaco,
    dam = 0, dam2=0,
                  diceroll;
    int             pdam, res;
    char buflog[1000];

    if (!ch || !victim)
        return;

    if (DEAD(ch) || DEAD(victim))
    {
        log("fight.c Corpse(s) engaged in fight");
        return;
    }


    if (ch->in_room != victim->in_room) {
        if (FIGHTING(ch) && FIGHTING(ch) == victim)
            stop_fighting(ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
        return;
    }

    if (GET_POS(ch)<POS_FIGHTING && !FIGHTING(ch))
        return;


    if (!FIGHTING(victim) && !FIGHTING(ch) && GET_POS(victim)==POS_STANDING && ((number(1, 111)<GET_SKILL(victim, SKILL_ALERTNESS)) || (number(1, 111)<GET_SKILL(victim, SKILL_SIXTHSENSE)))  && DEX_CHECK(victim)   )
    {
        act("You jump away as $N leaps to attack you.", FALSE, victim, 0, ch, TO_CHAR);
        act("$n jumps away as $N leaps to attack $m.", FALSE, victim, 0, ch, TO_NOTVICT);
        act("$n jumps away as you leap to attack $m.", FALSE, victim, 0, ch, TO_VICT);
        improve_skill(victim, SKILL_ALERTNESS, 1);
        improve_skill(victim, SKILL_SIXTHSENSE, 1);
        check_fight(victim, ch);
        return;
    }





    if (type == SKILL_DUAL_WIELD) {
        wielded = GET_EQ(ch, WEAR_DUALWIELD);
        type = TYPE_UNDEFINED;
    }

    if (type == SKILL_DUAL_BACKSTAB || type == SKILL_DUAL_CIRCLE) {
        wielded = GET_EQ(ch, WEAR_DUALWIELD);
    }

    if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
        w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
    else if (wielded && GET_OBJ_TYPE(wielded) == ITEM_FIREWEAPON)
        w_type = TYPE_POUND;  /* fireweapons bludgeon in hand-to-hand */
    else {
        if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
            w_type = ch->mob_specials.attack_type + TYPE_HIT;
        else
            w_type = TYPE_HIT;
    }

    if (IS_NPC(ch) && !FIGHTING(ch) && !FIGHTING(victim) && IS_THIEF(ch) && GET_EQ(ch, WEAR_WIELD) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) == TYPE_PIERCE - TYPE_HIT))
        type=SKILL_BACKSTAB;

    res=1;
    if (IS_WEAPON(w_type) && !IS_BACKSTAB(type))
        res=check_hit(ch, victim, w_type, wielded);

    if (res==-1)
    {
        check_fight(ch, victim);
        return;
    }


    if (!res)
    {
        if (type == SKILL_BACKSTAB || type == SKILL_DUAL_BACKSTAB)
            damage(ch, victim, 0, SKILL_BACKSTAB, wielded);
        else if (type == SKILL_CIRCLE || type==SKILL_DUAL_CIRCLE)
            damage(ch, victim, 0, SKILL_CIRCLE, wielded);
        else
            damage(ch, victim, 0, w_type, wielded);
    } else if (AFF_FLAGGED(victim, AFF_FORCE_FIELD)  &&
               (!number(0, 6))) {
        act("&BYou are repulsed as you enter $N's force field!&0", FALSE, ch, 0, victim, TO_CHAR);
        act("$N's force field repulses $n!", FALSE, ch, 0, victim, TO_NOTVICT);
        act("&BYour force field repulses $n!&0", FALSE, ch, 0, victim, TO_VICT);
        //    if (number(1, 4)>2)
        damage(victim, ch, dice(5, GET_LEVEL(victim)), SPELL_FORCE_FIELD, NULL);
        //  else
        //    check_fight(ch, victim);
        return;  
     } else if (AFF3_FLAGGED(victim, AFF3_SOF)  && CHANCE(GET_FAITH(victim)/20)) {
        act("&PYou are repulsed by $N's shield of faith!&0", FALSE, ch, 0, victim, TO_CHAR);
        act("$N's shield of faith repulses $n!", FALSE, ch, 0, victim, TO_NOTVICT);
        act("&PYour shield of faith repulses $n!&0", FALSE, ch, 0, victim, TO_VICT);
        //    if (number(1, 4)>2)
        //damage(victim, ch, dice(5, GET_LEVEL(victim)), PRAYER_SHIELD_OF_FAITH, NULL);
        //  else
        //    check_fight(ch, victim);
        return;    
    } else if (AFF_FLAGGED(victim, AFF_FIRE_SHIELD) &&  (!number(0, 7)))
    {
        act("&RYou are repulsed as you enter $N's fire shield!&0", FALSE, ch, 0, victim, TO_CHAR);
        act("$n is repulsed by $N's fire shield!", FALSE, ch, 0, victim, TO_NOTVICT);
        act("&RYour fire shield repulses $n!&0", FALSE, ch, 0, victim, TO_VICT);
        SET_BIT(AFF2_FLAGS(ch), AFF2_BURNING);
        //if (number(1, 4)>2)
        damage(victim, ch, dice(7, GET_LEVEL(victim)), SPELL_FIRE_SHIELD, NULL);
        //else
        //check_fight(ch, victim);
        return;
    } else if (AFF2_FLAGGED(victim, AFF2_BLINK) && type!=SKILL_BACKSTAB && type!=SKILL_DUAL_BACKSTAB && type!=SKILL_AMBUSH &&
               (number(1, 620) <= GET_LEVEL(victim) + GET_INT(victim) + GET_DEX(victim))) {
        act("$N blinks away from your attack.", FALSE, ch, 0, victim, TO_CHAR);
        act("$N blinks away from $n's attack.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("You blink out of $n's way.", FALSE, ch, 0, victim, TO_VICT);
        //      improve_skill(victim, SPELL_BLINK, 15);
        check_fight(ch, victim);
        return;
    } else if (AFF2_FLAGGED(victim, AFF2_MIRRORIMAGE) &&
               (number(1, 42) > GET_INT(ch)) && (number(1, 64) <= GET_INT(victim))) {
        act("One of $N's false images dissipates and is instantly replaced!", FALSE, ch, 0, victim, TO_CHAR);
        act("One of $N's false images dissipates and is instantly replaced!", FALSE, ch, 0, victim, TO_NOTVICT);
        act("One of your images takes the blow from $n and is replaced by another image.", FALSE, ch, 0, victim, TO_VICT);
        //        improve_skill(victim, SPELL_MIRROR_IMAGE, 15);
        check_fight(ch, victim);
        return;
    } else if (type!=SKILL_BACKSTAB && type!=SKILL_DUAL_BACKSTAB &&  PRF2_FLAGGED(victim, PRF2_TUMBLE) &&
               (number(1, 450) <= GET_SKILL(victim, SKILL_TUMBLE)) && DEX_CHECK(victim) && GET_POS(victim)==POS_FIGHTING) {
        act("$N tumbles to the side, avoiding your blow.", FALSE, ch, 0, victim, TO_CHAR);
        act("$N tumbles to the side, avoiding $n's blow.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("You tumble to the side, avoiding $n's blow.", FALSE, ch, 0, victim, TO_VICT);
        improve_skill(victim, SKILL_TUMBLE, 8);
        check_fight(ch, victim);
        return;

    } else if (type!=SKILL_BACKSTAB && type!=SKILL_DUAL_BACKSTAB && type!=SKILL_AMBUSH  &&
               (number(1, 800) <= GET_SKILL(victim, SKILL_DUCK)+(GET_HEIGHT(ch)-GET_HEIGHT(victim))/2) && DEX_CHECK(victim) && GET_POS(victim)==POS_FIGHTING) {
        act("$N ducks under your blow.", FALSE, ch, 0, victim, TO_CHAR);
        act("$N ducks under $n's blow.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("You duck under $n's blow.", FALSE, ch, 0, victim, TO_VICT);
        improve_skill(victim, SKILL_DUCK, 8);
        check_fight(ch, victim);
        return;
    }
    else if (IS_HIT(w_type) && w_type!=TYPE_PIERCE && w_type!= TYPE_HIT && GET_POS(victim)>POS_SITTING &&
             (number(1, 700) <= GET_SKILL(victim, SKILL_JUMP)) && DEX_CHECK(victim) && GET_POS(victim)==POS_FIGHTING) {
        act("$N jumps over your swing.", FALSE, ch, 0, victim, TO_CHAR);
        act("$N jumps over $n's swing.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("You jump over $n's swing.", FALSE, ch, 0, victim, TO_VICT);
        improve_skill(victim, SKILL_JUMP, 7);
        check_fight(ch, victim);
        return;

    } else {
        /* okay, we know the guy has been hit.  now calculate damage. */

        dam=0;



        if (wielded && !IS_MONK(ch) && (GET_OBJ_TYPE(wielded) == ITEM_WEAPON || GET_OBJ_TYPE(wielded) == ITEM_FIREWEAPON)) {
            dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0);
            if (AFF3_FLAGGED(ch, AFF3_ENLIGHTMENT))
                dam +=	(dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0))/2;
            if (AFF3_FLAGGED(ch, AFF3_DARKENING))
                dam -=	(dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0))/2;
            //  if (IS_NPC(ch)) {
            //  dam+= number(2*GET_LEVEL(ch)/3, GET_LEVEL(ch))/2;

            //  }
            dam=dam*GET_OBJ_DAMAGE(wielded)/100;
        } else {
            /*if (IS_NPC(ch) && !IS_MONK(ch)) {
                dam+= number(2*GET_LEVEL(ch)/3, GET_LEVEL(ch))+1;
        } else if (IS_MONK(ch)) {
                dam += number(MAX(0, GET_LEVEL(ch)-10), GET_LEVEL(ch) + GET_LEVEL(ch)/4)+1;
                if (wielded && (GET_OBJ_TYPE(wielded) == ITEM_WEAPON))
                    dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0);
        }
            else
                dam +=MAX( number(1, 4), (GET_SKILL(ch, SKILL_HIT)-number(2, 12))/3);      */
            dam += number(MAX(0, 17*((GET_SKILL(ch, SKILL_HIT)-5)/2)/24), ((GET_SKILL(ch, SKILL_HIT)-5)/2))+number(1,3);
            /*  if (wielded && (GET_OBJ_TYPE(wielded) == ITEM_WEAPON))
                  dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0);*/
        }
        dam += GET_DAMROLL(ch);


        if (type == SKILL_DUAL_BACKSTAB || type == SKILL_DUAL_CIRCLE || type==SKILL_AMBUSH) {

            if (type!=SKILL_AMBUSH)
                wielded = GET_EQ(ch, WEAR_WIELD);
            else
                wielded = GET_EQ(ch, WEAR_DUALWIELD);

            dam2=0;

            if (wielded && !IS_MONK(ch) && (GET_OBJ_TYPE(wielded) == ITEM_WEAPON || GET_OBJ_TYPE(wielded) == ITEM_FIREWEAPON)) {
                dam2 += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0);
                if (AFF3_FLAGGED(ch, AFF3_ENLIGHTMENT))
                    dam2 +=	(dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0))/2;
                if (AFF3_FLAGGED(ch, AFF3_DARKENING))
                    dam2 -=	(dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + GET_OBJ_VAL(wielded, 0))/2;
                if (IS_NPC(ch)) {
                    dam2+= number(2*GET_LEVEL(ch)/3, GET_LEVEL(ch))/2;
                }
                dam2=dam2*GET_OBJ_DAMAGE(wielded)/100;
            } else
                if (type!=SKILL_AMBUSH)
                    log("SYSERR: should never see this, fight.c hit()");


            dam2 += GET_DAMROLL(ch);
            if (type==SKILL_AMBUSH)
                wielded = GET_EQ(ch, WEAR_WIELD);
            else
                wielded = GET_EQ(ch, WEAR_DUALWIELD);

        }





        //        sprintf(buf, "Initial dam: %d\r\n", dam);
        //      send_to_char(buf, ch);


        if (AFF2_FLAGGED(victim, AFF2_NAP))
            dam += dam / 2;
        if (AFF2_FLAGGED(victim, AFF2_NAP))
            dam2 += dam2 / 2;
        
        dam = MAX(1, dam);      /* at least 0 hp damage min per hit */
        dam*=res;

        dam2 = MAX(1, dam2);      /* at least 0 hp damage min per hit */
        dam2*=res;


        if (AFF_FLAGGED(victim, AFF_DEFLECTION) &&
                (!number(0, 6))) {
            act("You hit $N's deflection shield. Ouch!", FALSE, ch, 0, victim, TO_CHAR);
            act("$n hits $N's deflection shield.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("$n hits your deflection shield.", FALSE, ch, 0, victim, TO_VICT);
            damage(victim, ch, dam, SPELL_DEFLECTION, wielded);
            return;
        }
        if (type == SKILL_DUAL_BACKSTAB) {
            //            dam *= 2 + GET_LEVEL(ch) / 15;
            //          dam2 *=2 + GET_LEVEL(ch) / 15;
            //            CREF(victim, CHAR_NULL);
            //          CREF(ch, CHAR_NULL);
            damage(ch, victim, dam2, SKILL_BACKSTAB, GET_EQ(ch, WEAR_WIELD));
            if (!DEAD(ch) && !DEAD(victim))
                damage(ch, victim, dam, SKILL_BACKSTAB, wielded);
            //        CUREF(victim);
            //      CUREF(ch);
        } else if (type == SKILL_BACKSTAB) {
            //        dam *= 2 + GET_LEVEL(ch) / 15;
            damage(ch, victim, dam, SKILL_BACKSTAB, wielded);
        }
        else if (type == SKILL_DUAL_CIRCLE) {
            //      dam *=MAX(2, (2 + GET_LEVEL(ch) / 15)/2);
            //    dam2 *=MAX(2, (2 + GET_LEVEL(ch) / 15)/2);
            //            CREF(victim, CHAR_NULL);
            //          CREF(ch, CHAR_NULL);
            damage(ch, victim, dam2, SKILL_CIRCLE, GET_EQ(ch, WEAR_WIELD));
            if (!DEAD(ch) && !DEAD(victim))
                damage(ch, victim, dam, SKILL_CIRCLE, wielded);
            //        CUREF(victim);
            //      CUREF(ch);
        }
        else if (type == SKILL_CIRCLE) {
            //  dam *=MAX(2, (2 + GET_LEVEL(ch) / 15)/2);
            damage(ch, victim, dam, SKILL_CIRCLE, wielded);
        } else if (type == SKILL_AMBUSH) {
            int percent, prob;

            if (GET_SKILL(ch, SKILL_BACKSTAB) && wielded && (GET_OBJ_VAL(wielded, 3)==TYPE_PIERCE-TYPE_HIT))
            {
                dam+=dam/3;
                prob = GET_SKILL(ch, SKILL_BACKSTAB) +5*((GET_SKILL(ch, TYPE_PIERCE)-5)/2-(MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(wielded)))) + 2 * (GET_DEX(ch) - GET_INT(victim));
                percent=number(1, 111);

                if (percent > prob)
                    damage(ch, victim, dam, w_type, wielded);
                else {
                    //    		dam  *= 2 + GET_LEVEL(ch) / 15;
                    //  		dam2 *= 2 + GET_LEVEL(ch) / 15;
                    if (GET_SKILL(ch, SKILL_DUAL_BACKSTAB) && GET_EQ(ch, WEAR_DUALWIELD) && number(1,111)<GET_SKILL(ch, SKILL_DUAL_BACKSTAB) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_DUALWIELD), 3) == (TYPE_PIERCE - TYPE_HIT)))
                    {
                        //            				CREF(victim, CHAR_NULL);
                        //          				CREF(ch, CHAR_NULL);
                        dam2+=dam2/3;
                        damage(ch, victim, dam2, SKILL_BACKSTAB, GET_EQ(ch, WEAR_WIELD));
                        if (!DEAD(ch) && !DEAD(victim))
                            damage(ch, victim, dam, SKILL_BACKSTAB, GET_EQ(ch, WEAR_DUALWIELD));
                        //        				CUREF(victim);
                        //      				CUREF(ch);
                    }
                    else
                        damage(ch, victim, dam, SKILL_BACKSTAB, GET_EQ(ch, WEAR_WIELD));
                }
            }
            else
            {
                dam*=2;
                if (wielded)
                {
                    //    				CREF(victim, CHAR_NULL);
                    //          			CREF(ch, CHAR_NULL);
                    damage(ch, victim, dam, w_type, wielded);
                    if (!DEAD(ch) && !DEAD(victim) && (wielded=GET_EQ(ch, WEAR_DUALWIELD)) && number(1, 101)<GET_SKILL(ch, SKILL_DUAL_WIELD))
                    {
                        w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
                        dam2*=2;
                        damage(ch, victim, dam2, w_type, wielded);
                    }
                    //			CUREF(victim);
                    //  			CUREF(ch);
                }
                else
                    damage(ch, victim, dam, w_type, wielded);
            }
            if (!DEAD(victim))
                WAIT_STATE(victim, PULSE_VIOLENCE);
        } else if (type == SKILL_POWER_BLOW) {
            damage(ch, victim, dam*6, SKILL_POWER_BLOW, wielded);
        } else if (type == SKILL_ELBOW_SWING) {
            damage(ch, victim, 3*dam, SKILL_ELBOW_SWING, wielded);
        } else if (type == SKILL_KNEE_THRUST) {
            damage(ch, victim, 4*dam, SKILL_KNEE_THRUST, wielded);
        } else if (type == SKILL_QUIVER_PALM) {
            damage(ch, victim, 2*dam, SKILL_QUIVER_PALM, wielded);
        } else
            damage(ch, victim, dam, w_type, wielded);
    }
}









/* Perform Monk Technique -Erocl 03/21/2000 */
void monk_technique(struct char_data *ch)
{
  int technique_roll=0, technique_damage=0, tornado_count=0, tornado_damage=0, i=0;
  struct affected_type af1, af2;
            
  if (GET_CLASS(ch) == CLASS_MONK && !GET_EQ(ch, WEAR_WIELD) &&
               !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_HOLD) && !GET_EQ(ch, WEAR_HANDS))
                //&&                (GET_LEVEL(FIGHTING(ch)) < LVL_IMMORT))
              
    {
      if(dice(1,100) <= 25)  /* Perform technique 25% of time */
      {
        technique_roll = GET_LEVEL(ch)+(3*number(1,GET_LEVEL(ch)));
            
        if(dice(1,4) == 1) /* Perform kick technique */
        {
          if(technique_roll <= 50) /* Lotus Kick */
          {
            technique_damage = (dice(1,100)+5)*MAX(GET_LEVEL(ch)/50,1);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            act("$N strikes you with a brilliant Lotus Kick!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"You strike $n with a brilliant Lotus Kick! (%d)", technique_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N strikes $n with a brilliant Lotus Kick!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 100)  /* Tornado Kick */
          {
            technique_damage = 0;
            act("$n leaps of the ground beginning a brutal spinning kick!", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("You leap off the ground into a brutal spinning kick!\r\n",ch);
              
            tornado_count = (dice(1,10)*GET_LEVEL(ch)/500)+1; /* Calculate number of kicks */
            for (i=0;i<tornado_count;i++)
                tornado_damage += (dice(1,100)+5)*MAX(GET_LEVEL(ch)/50,1);

              act("...$N rotates and strikes you with a brutal Tornado Kick!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              sprintf(buf,"...Your Tornado Kick connects with great force! (%d)\r\n", tornado_damage);
              send_to_char(buf,ch);
              act("...$N rotates and strikes $n with a brutal Tornado Kick!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
              technique_damage += tornado_damage;
            }
          
          else if(technique_roll <= 150) /* Crescent Kick */
          {
            technique_damage = 150 + dice(1,GET_LEVEL(ch)) + GET_HITROLL(ch);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            act("$N strikes you with a lightning quick Crescent Kick!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"You strike $n with a lightning quick Crescent Kick! (%d)", technique_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N strikes $n with a lightning quick Crescent Kick!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);

            if(!MOB_FLAGGED(FIGHTING(ch),MOB_NOBLIND) && !IS_AFFECTED(FIGHTING(ch), AFF_BLIND))
            {
              af1.location = APPLY_HITROLL;
              af1.modifier = -4;
              af1.duration = 2;
              af1.bitvector = AFF_BLIND;
              affect_join(FIGHTING(ch), &af1, FALSE, FALSE, FALSE, FALSE);
              
              af2.location = APPLY_AC;
              af2.modifier = 40;
              af2.duration = 2;
              af2.bitvector = AFF_BLIND;
              affect_join(FIGHTING(ch), &af2, FALSE, FALSE, FALSE, FALSE);
        
              act("You have been blinded!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              act("$n seems to be blinded!", TRUE, FIGHTING(ch), 0, ch, TO_ROOM);
            }
          }
          else /* Butterfly Kick */
          {
            tornado_damage = 200 + 2*GET_HITROLL(ch) + dice(1,100);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;
            
            act("$N kicks the wind out of you!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"Your left foot knocks the wind out of $n! (%d)", tornado_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);  
            act("$N kicks the wind out of $n!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
            
            technique_damage = tornado_damage;
            
            tornado_damage = 200 + 2*GET_HITROLL(ch) + dice(1,100);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;
             
            act("$N catches you off-guard with $S right foot!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"Your right foot catches $n off-guard! (%d)", tornado_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N connects with $S second foot catching $n off-guard!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
              
            technique_damage += tornado_damage;

            if(dice(1,2) == 1) /* Affect victim with bash */
            {
              act("You topple to the ground from the force of $N's Butterfly Kick!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              sprintf(buf,"$n topples over from the force of your Butterfly Kick! (%d)", technique_damage);
              act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
              act("$n topples over from the force of $N Butterfly Kick!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
           
              GET_POS(FIGHTING(ch)) = POS_SITTING;
              WAIT_STATE(FIGHTING(ch), PULSE_VIOLENCE);
            }
            
          }
        }
        else /* Perform palm technique */
        {
          if(technique_roll <= 25) /* Iron Fist */
          {  
            technique_damage = (dice(1,100)+5)*(MAX(1,GET_LEVEL(ch)/30));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            act("$N's Iron Fist enlightens you with pain!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"Your Iron Fist enlightens $n with pain! (%d)", technique_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$n grimaces in pain from $N's Iron Fist!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 50) /* Dragon Claw */
          {
            technique_damage = ((GET_HITROLL(ch) * dice(1,GET_LEVEL(ch)))/50) + GET_HITROLL(ch);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
             
            act("$N rips into you with a wicked Dragon Claw!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"Your Dragon Claw brutally rakes $n! (%d)", technique_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N rips into $n with a wicked Dragon Claw!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 75) /* Red Buddha Fist */
          { 
            tornado_damage = dice(1,GET_LEVEL(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;
            
            act("$N touches you with a glowing hand!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"Your glowing palm warms against $n! (%d)", tornado_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N touches $n with a glowing hand!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
            
            technique_damage = tornado_damage;
             
            tornado_damage = dice(GET_LEVEL(ch),3);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;

            act("$N's palms suddenly bursts into flames!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"Flames suddenly errupt from your palm as your Red Buddha Fist connects! (%d)", tornado_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("Flames errupt from $N's palm!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
             
            technique_damage += tornado_damage;
          }
          else if(technique_roll <= 100) /* Devil's Claw */ 
          {
            technique_damage = dice(3,GET_LEVEL(ch)) + (3*GET_DEX(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
             
            act("You feel your life draining as $N's Devil's Claw rakes you!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"You usurp life from $n with your Devil's Claw! (%d)", technique_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$n looks horrified as $N claws their life away!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
         
            if(GET_HIT(ch) < GET_MAX_HIT(ch))
            {
              act("...Your wounds mend as you drain $n's precious life force!", FALSE, FIGHTING(ch), 0, ch, TO_VICT);
              GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + technique_damage/10);
            }
          } 
          else if(technique_roll <= 125) /* Eye of the Phoenix */
          {
            technique_damage = dice(10,GET_WIS(ch)) + dice(4,GET_HITROLL(ch)) + (GET_LEVEL(ch)*2);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
          
            send_to_char("Your hands glow as you channel your spiritual energy to them!\r\n",ch);
            sprintf(buf,"You release the Eye of the Phoenix to punish $n! (%d)", technique_damage);
            act("$N punishes you with a glowing palm-strike!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N punishes $n with a glowing palm-strike!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 150) /* Seven Star Earthquake Fist */
          {
            technique_damage = GET_HITROLL(ch) + dice(4,GET_LEVEL(ch)) + dice(7,GET_STR(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            sprintf(buf,"You unleash a flurry of Seven Strikes upon $n! (%d)", technique_damage);
            act("$N batters you with a flurry of attacks!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N batters $n with a flurry of attacks!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
            
            if(dice(1,10) == 1)
            {
              act("$n falls over, overwhelmed by your ferocious assault!", FALSE, FIGHTING(ch), 0, ch, TO_VICT);
              act("You fall over, overwhelmed by $N's ferocious assault!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              act("$n falls over, overwhelmed by $N's ferocious assault!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
             
              GET_POS(FIGHTING(ch)) = POS_SITTING; 
              WAIT_STATE(FIGHTING(ch), PULSE_VIOLENCE);
              
            }
          }
          else if(technique_roll <= 175) /* Heavenly Thunder Palm */
          {
            technique_damage = dice(4,GET_HITROLL(ch)) + (dice(1,GET_DEX(ch))*dice(1,GET_WIS(ch))*dice(1,GET_STR(ch)));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
           
            send_to_char("Your eyes close as you prepare your Heavenly Thunder Palm!\r\n",ch);
            sprintf(buf,"You electrify $n with a stream of spiritual energy! (%d)", technique_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("$N electrifies you with a crackling palm-strike!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act("$N electrifies $n with a crackling palm-strike!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }  
          else /* The Fabled Quivering Palm */
          {
            technique_damage = dice(2, MAX(1,GET_WIS(ch)-10));
            technique_damage *= dice(2, MAX(1,GET_DEX(ch)-10));
            technique_damage *= dice(1, MAX(1,GET_STR(ch)-14));
            technique_damage *= dice(1, MAX(1,GET_CON(ch)-14));
            technique_damage += dice(10, GET_LEVEL(ch));
            technique_damage += dice(10, GET_HITROLL(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)  
              technique_damage /= 2;
            
            send_to_char("Your hand begins to quiver violently as you prepare to strike!\r\n",ch);
            sprintf(buf,"You unlease the fabled Quivering Palm upon $n! (%d)", technique_damage);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("Your jaw drops as $N unleashes the fabled Quivering Palm upon you!", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act("Your jaw drops as $N unleashes the fabled Quivering Palm upon $n!", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
            
        }
            
        GET_HIT(FIGHTING(ch)) -= technique_damage; /* do damage the illegal way since we're not using damage types */
        damage(ch, FIGHTING(ch), 0, -1, NULL); /* update position and such the legal way */
      }
    }
}





void            perform_violence(void)
{
    int             type;
    int ii;
    struct char_data *ch, *pom, *vict;
    extern struct index_data *mob_index;
    struct obj_data *obj;
    unsigned int    attacks,
    dual_wield_flag;
    int             skpb,
    skes,
    skkt,
    skqp, bound=0;
    float zamah=0;
#ifdef EVENT_COMBAT    
    return;		// fight events now
#endif



    for (ch = combat_list; ch; ch = next_combat_list) {
        if (ch!=NULL) {
            next_combat_list = ch->next_fighting;
            type = 0;
            attacks = 1;
            dual_wield_flag = 0;
            skpb = 0;
            skes = 0;
            skkt = 0;
            skqp = 0;
            if (AFF2_FLAGGED(ch, AFF2_PETRIFY))
                continue;

            //if (IN_EVENT(ch))
            //  continue;

            if (IS_NPC(ch))
                attacks = MAX(1, ch->mob_specials.attack_num);
            else if (GET_MOVE(ch)<1)
            {
                send_to_char("You are too exhausted to fight back.\r\n",ch);
                continue;
            }
            /* PC */
            
            if (GET_SPELL_EVENT(ch))
            	continue;		// no hitting while casting/praying
            if (IS_MONK(ch)) {
            	 /* Check if monk and determine whether or not to perform technique */
        	//	monk_technique(ch);
        		
                //if (number(1, 300) < (3*GET_LEVEL(ch)+2*GET_DEX(ch)))
                //  attacks++;
                if ((GET_SKILL(ch, SKILL_POWER_BLOW) > 0)
                        && (number(1, 860) <= (GET_SKILL(ch, SKILL_POWER_BLOW)
                                               + 6*(GET_DEX(ch)+GET_INT(ch)-32))
                           )) {
                    attacks++;
                    improve_skill(ch, SKILL_POWER_BLOW, 4);
                    skpb = 1;
                }
                if ((GET_SKILL(ch, SKILL_QUIVER_PALM) > 0)
                        && (number(1, 642) <= (GET_SKILL(ch, SKILL_QUIVER_PALM)
                                               + 11 * (GET_DEX(ch) - 16)))) {
                    improve_skill(ch, SKILL_QUIVER_PALM, 8);
                    attacks++;
                    skqp = 1;
                }
                if ((GET_SKILL(ch, SKILL_KNEE_THRUST) > 0)
                        && (number(1, 853) <= (GET_SKILL(ch, SKILL_KNEE_THRUST)
                                               + 11 * (GET_DEX(ch) - 16)))) {
                    improve_skill(ch, SKILL_KNEE_THRUST, 5);
                    attacks++;
                    skkt = 1;
                }
                if ((GET_SKILL(ch, SKILL_ELBOW_SWING) > 0)
                        && (number(1, 723) <= (GET_SKILL(ch, SKILL_ELBOW_SWING)
                                               + 11 * (GET_DEX(ch) - 16)))) {
                    improve_skill(ch, SKILL_ELBOW_SWING, 6);
                    attacks++;
                    skes = 1;
                }
            } else {
                if ((GET_SKILL(ch, SKILL_SECOND_ATTACK) > 0) && number(1, 111)
                        <= (GET_SKILL(ch, SKILL_SECOND_ATTACK) )) {
                    attacks++;
                    improve_skill(ch, SKILL_SECOND_ATTACK, 18);
                    
                }   
                if ((GET_SKILL(ch, SKILL_THIRD_ATTACK) > 0)// && (!IS_NPC(ch) || GET_LEVEL(ch)>=20)
                            && number(1, 126)  <= (GET_SKILL(ch, SKILL_THIRD_ATTACK) )) {
                        attacks++;
                        improve_skill(ch, SKILL_THIRD_ATTACK, 9 );
                     
                    }
                if ((GET_SKILL(ch, SKILL_FOURTH_ATTACK) > 0)// && (!IS_NPC(ch) || GET_LEVEL(ch)>=30)
                                && number(1, 141) <= (GET_SKILL(ch, SKILL_FOURTH_ATTACK)  )) {
                            attacks++;
                            improve_skill(ch, SKILL_FOURTH_ATTACK, 5);
                }
                if (GET_SKILL(ch, SKILL_DUAL_WIELD) && (GET_EQ(ch, WEAR_DUALWIELD))
                        && ((number(1, 116) <= (GET_SKILL(ch, SKILL_DUAL_WIELD) )))) {
                    attacks++;
                    improve_skill(ch, SKILL_DUAL_WIELD, 14);
                    dual_wield_flag = 1;
                    if (GET_SKILL(ch, SKILL_SECOND_DUAL) > 0) {
                        if (number(1, 126) <= (GET_SKILL(ch, SKILL_SECOND_DUAL) )) {
                            attacks++;
                            improve_skill(ch, SKILL_SECOND_DUAL, 8);
                            dual_wield_flag ++;
                        }                              
                    }          
                    if (GET_SKILL(ch, SKILL_AMBIDEX) > 0) {
                        if (number(1, 136) <= (GET_SKILL(ch, SKILL_AMBIDEX))) {
                            attacks++;
                            improve_skill(ch, SKILL_AMBIDEX, 4);
                            dual_wield_flag ++;
                        }
                    }
                }
            }

            if (AFF_FLAGGED(ch, AFF_DEATHDANCE) && (IS_NPC(ch) || !AFF2_FLAGGED(ch, AFF2_HASTE)))
                attacks += number(1, GET_LEVEL(ch) / 16);
            if (AFF2_FLAGGED(ch, AFF2_ADRENALIN))
                attacks++;
            if (AFF2_FLAGGED(ch, AFF2_HASTE))
                attacks ++;
            if (AFF3_FLAGGED(ch, AFF3_SLOW))
                attacks --;
            if (IS_CASTERMOB(ch) && !number(0, 1))
                attacks=MAX(1, attacks-1);

            // if (IS_NPC(ch))// && GET_LEVEL(ch)>19)
            //  attacks += (GET_LEVEL(ch)-10)/20;    // kludge
            if (AFF2_FLAGGED(ch, AFF2_BERSERK))
                attacks++;
            attacks = MIN(7, attacks);
            if (IS_NPC(ch)) {
                /*if (GET_POS(ch) < POS_FIGHTING) {
                    if (GET_MOB_WAIT(ch)<=0) {
                        if (FIGHTING(ch))
                        	GET_POS(ch) = POS_FIGHTING;
                        else	
                        	GET_POS(ch) = POS_STANDING;
                        GET_MOB_WAIT(ch) = 0;
                        if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2))
                             act("$n quickly scrambles to $s feet!", 1, ch, 0, 0, TO_ROOM);
                        else if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 6))
                             act("$n scrambles to $s feet!", 1, ch, 0, 0, TO_ROOM);
                        else
                        	act("$n barely manages to get on $s feet again.", 1, ch, 0, 0,TO_ROOM);
                        //act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
                    }
                    else {
                        GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
                        continue;
                    }    
                    
            } else*/

                if (GET_MOB_WAIT(ch)<=0 && ch->carrying)// &&  !number(0, 2))
                    wear_all_suitable(ch);

                if (GET_MOB_WAIT(ch)>0)
                    GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
            } else if (GET_POS(ch) < POS_FIGHTING) {
                if (GET_POS(ch)==POS_SLEEPING)
                    stop_fighting(ch);    	// magicaly asleep doing fight

                continue;
                /*
                   if (!CHECK_WAIT(ch)) {   
                   	if (FIGHTING(ch))
                       	GET_POS(ch) = POS_FIGHTING;
                       else	
                           	GET_POS(ch) = POS_STANDING;	
                       if (!GET_SKILL(ch, SKILL_SPRING_LEAP) || (number(1, 125)>(GET_SKILL(ch, SKILL_SPRING_LEAP)+2*GET_DEX(ch))))
                       {
                           if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2))
                                act("$n quickly scrambles to $s feet!", 1, ch, 0, 0, TO_ROOM);
                           else if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 6))
                                act("$n scrambles to $s feet!", 1, ch, 0, 0, TO_ROOM);
                           else
                           	act("$n barely manages to get on $s feet again.", 1, ch, 0, 0,TO_ROOM);
                           send_to_char("&wYou clamber to your feet.&0\r\n", ch);
                       }
                       else
                       {
                           act("$n spring leaps to $s feet and charges forward!", TRUE, ch, 0, 0, TO_ROOM);
                           send_to_char("&wYou spring leap to your feet and charge forward!&0\r\n", ch);
                           attacks++;                    
                       }                          
                       
                   } else {
                       //send_to_char("You can't fight while sitting!!\r\n", ch); 
                       continue;
                   }*/
            }
            //        CREF(ch, CHAR_NULL);
            if (GET_POS(ch) < POS_FIGHTING)
            {
                if (GET_POS(ch)==POS_SLEEPING)	// magicaly asleep doing fight
                    stop_fighting(ch);
                continue;
            }
            for (ii = 0; ii < NUM_WEARS; ii++)
                if ((obj=GET_EQ(ch, ii)) && obj->bound_spell)
                    if (!number(0, obj->bound_spell_timer))
                    {
                        //        			CREF(FIGHTING(ch), CHAR_NULL);
                        
                        
                        
                        act("&wYour $p humms and vibrates as it unleashes it's magic!&0", FALSE, ch, obj, 0, TO_CHAR);
                        act("&wYou see $n's $p humming and vibrating as it unleashes it's magcis!&0", FALSE, ch, obj, 0, TO_ROOM);
                        call_magic(ch, ((spell_info[obj->bound_spell].violent)? FIGHTING(ch) : ch), 0, obj->bound_spell, obj->bound_spell_level, CAST_BOUND_SPELL, 0);
                        if (!DEAD(ch) && FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room)
                            stop_fighting(ch);
                        //CUREF(FIGHTING(ch));
                        break;
                    }
            if (DEAD(ch))
                continue;

            if (AFF3_FLAGGED(ch, AFF3_DISTRACT))
                attacks--;

            if (ch && IS_NPC(ch) && FIGHTING(ch) && GET_MOB_WAIT(ch)<=0 && GET_POS(FIGHTING(ch)) == POS_FIGHTING)// &&  !number(0, 2))
            {

                change_style(ch, FIGHTING(ch));
                if (ch->carrying)
                    wear_all_suitable(ch);
                if ((MOB_FLAGGED(ch, MOB_MEMORY) || MOB_FLAGGED(ch, MOB_AWARE)) && INT_CHECK(ch))
                    mob_ai(ch);
                if (IS_MAGIC_USER(ch))
                    fight_mage(ch);
                else if (IS_CLERIC(ch))
                    fight_cleric(ch);
                else if (IS_THIEF(ch))
                    fight_thief(ch);
                else if (IS_WARRIOR(ch))// && !MOB_FLAGGED(ch, MOB_PET))
                    fight_warrior(ch);
            }
            if (DEAD(ch))
                continue;
                
                if (FOL_SKIG(ch))
                {
                    if (FAITH_STRONG(ch) && (vict=FIGHTING(ch)))
                    {

                        if (!number(0, 11000/GET_FAITH(ch)) && GET_POS(vict)==POS_FIGHTING)
                        {

                            act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_ROOM);
                            act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_CHAR);

                            act("Your feet are suddennly swept away, you fall to the ground!", FALSE, vict, 0, 0, TO_CHAR);
                            act("$n's feet are swept away right out from under $m!", FALSE, vict, 0, 0, TO_ROOM);
                            WAIT_STATE(vict, 3*PULSE_VIOLENCE/4);
                            assign_stand_event(vict, PULSE_VIOLENCE);
                            GET_POS(vict) = POS_SITTING;
                            
                        }
                        else if (!number(0, 11000/GET_FAITH(ch)) && GET_POS(vict)==POS_FIGHTING)
                        {
                            act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_ROOM);
                            act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_CHAR);

                            act("Someone sends some dirt right into your eyes!", FALSE, vict, 0, 0, TO_CHAR);
                            act("Someone sends some dirt right into $n's eyes!", FALSE, vict, 0,0 , TO_ROOM);
                            SET_BIT(AFF3_FLAGS(vict), AFF3_TEMP_BLIND);
                            WAIT_STATE(vict, PULSE_VIOLENCE);                                                            
                        }
                    }
                    else if (FAITH_QUESTIONABLE(ch))
                    {
                        vict=ch;
                        if (!number(0, 11000/(-GET_FAITH(ch)))  && GET_POS(vict)==POS_FIGHTING)
                        {
                            act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_ROOM);
                            act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_CHAR);

                            act("Your feet are suddennly swept away, you fall to the ground!", FALSE, vict, 0, 0, TO_CHAR);
                            act("$n's feet are swept away right out from under $m!", FALSE, vict, 0, 0, TO_ROOM);
                            WAIT_STATE(vict, 3*PULSE_VIOLENCE/4);
                            assign_stand_event(vict, PULSE_VIOLENCE);
                            GET_POS(vict) = POS_SITTING;
                            
                        }
                        else if (!number(0, 11000/(-GET_FAITH(ch))))
                        {   
			    act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_ROOM);
                            act("You hear &PSkig's&0 insane laughter!&0", FALSE, ch, 0, 0, TO_CHAR);

                            act("Someone sends some dirt right into your eyes!", FALSE, vict, 0, 0, TO_CHAR);
                            act("Someone sends some dirt right into $n's eyes!", FALSE, vict, 0,0 , TO_ROOM);                        	
                            SET_BIT(AFF3_FLAGS(vict), AFF3_TEMP_BLIND);
                            WAIT_STATE(vict, PULSE_VIOLENCE);

                            
                        }
                    }

                }

                if (FOL_BOUGAR(ch) && (vict=FIGHTING(ch)))
                {

                    if (GET_FAITH(ch)>0 && !number(0, (2800+2800*GET_HIT(ch)/GET_MAX_HIT(ch))/GET_FAITH(ch)))
                    {
                        if (num_fighting(ch)>1)
                        { CHAR_DATA *tch, *next_tch;
                            int dam=0;

			    send_to_char("&CComing from nowhere, dozens of meteorites crush your foes!&0\r\n", ch);
			    act("&CComing from nowhere, dozens of meteorites fall!&0\r\n", FALSE, ch, 0, 0, TO_ROOM);
			    
			    
                            for (tch = world[ch->in_room].people; tch; tch = next_tch) {
                                next_tch = tch->next_in_room;

                                /* The skips: 1: the caster 2: immortals 3: if no pk on this mud,
                                 * skips over all players 4: pets (charmed NPCs) players can only hit
                                 * players in CRIMEOK rooms 4) players can only hit charmed mobs in
                                 * CRIMEOK rooms */

                                if (tch == ch)
                                    continue;

                                if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
                                    continue;
                                if (!CAN_MURDER(ch, tch))
                                    continue;
                                if (is_same_group(ch, tch))
                                    continue;
                                if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
                                    continue;

                                //dam=dice(GET_LEVEL(ch), 11)+2*GET_LEVEL(ch); 
                                
                                dam=(16+21*GET_FAITH(ch)/400)*GET_MAX_HIT(tch)/100;
				
				send_to_char("&CYou are CRUSHED!&0\r\n", tch);
				act("$n is &CCRUSHED!&0", FALSE, tch, 0, 0, TO_ROOM);
                                GET_HIT(tch)=MAX(0, GET_HIT(tch)-dam);
                            }
                        }
                        else
                        {                                                          
                        	int dam;
                        	dam=dam=(16+21*GET_FAITH(ch)/400)*GET_MAX_HIT(vict)/100;
                            //GET_HIT(vict)=MAX(0, GET_HIT(vict)-dice(GET_LEVEL(ch), 15));
                            GET_HIT(vict)=MAX(0, GET_HIT(vict)-dam);
                            send_to_char("&wYou are struck by heavenly BOLT of LIGHTNING!!&0\r\n", vict);
                            act("$n is suddenly struck by a &CBOLT of LIGHTNING!!&0", FALSE, vict, 0, 0, TO_ROOM);
                        }
                    }
                }
    
                
                
            while (attacks-- > 0 && ch && !DEAD(ch) && FIGHTING(ch) && GET_POS(ch)==POS_FIGHTING) {
                if (!IS_NPC(ch))
                {
                    zamah=1;
                    zamah+=get_carry_cond(ch)-1;
                    if (AFF_FLAGGED(ch, AFF2_HASTE))
                        zamah+=1;
                    if (AFF2_FLAGGED(ch, AFF2_ADRENALIN))
                        zamah+=1;
                    if (AFF3_FLAGGED(ch, AFF3_WEB))
                        zamah+=2;
                    if (AFF3_FLAGGED(ch, AFF3_ENDURE))
                        zamah /=2;
                    if (AFF2_FLAGGED(ch, AFF2_BERSERK))
                        zamah+=3;
                    if (!IS_GOD(ch))
                        GET_MOVE(ch)-=zamah;

                }
                if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) {
                    stop_fighting(ch);
                    continue;
                }

                // ovdje ubaciti guard check
                if (AFF2_FLAGGED(FIGHTING(ch), AFF2_GUARDED))
                {
                    struct char_data *tch, *next_tch;
                    pom=FIGHTING(ch);

                    for (tch = world[ch->in_room].people; tch; tch = next_tch) {
                        next_tch = tch->next_in_room;
                        if (tch->guarding != pom)
                            continue;

                        if (tch==ch)	// guy you are guarding is the guy you are beating up
                        {
                            REMOVE_BIT(AFF3_FLAGS(ch), AFF3_GUARD);
                            ch->guarding=NULL;
                            if (!is_guarded(pom))
                                REMOVE_BIT(AFF2_FLAGS(pom), AFF2_GUARDED);
                            continue;
                        }

                        if (!AFF2_FLAGGED(tch, AFF2_BERSERK) && (GET_SKILL(tch, SKILL_GUARD)+2*GET_DEX(tch)-MAX(0,num_fighting(tch)-2)*15>number(1, 150)))
                        {
                            improve_skill(ch, SKILL_GUARD, 2);
                            do_rescue(tch, "", 0, SCMD_AUTORESCUE);
                        }

                    }
                }





                if ((dual_wield_flag > 0) && ch) {
                    hit(ch, FIGHTING(ch), SKILL_DUAL_WIELD);
                    dual_wield_flag--;
                } else if (skqp > 0 && ch) {
                    hit(ch, FIGHTING(ch), SKILL_QUIVER_PALM);
                    skqp = 0;

                } else if (skes > 0 && ch) {
                    hit(ch, FIGHTING(ch), SKILL_ELBOW_SWING);
                    skes = 0;

                } else if (skkt > 0 && ch) {
                    hit(ch, FIGHTING(ch), SKILL_KNEE_THRUST);
                    skkt = 0;

                } else if (skpb > 0 && ch) {
                    hit(ch, FIGHTING(ch), SKILL_POWER_BLOW);
                    skpb = 0;

                } else if (ch)
                    hit(ch, FIGHTING(ch), TYPE_UNDEFINED);

            }
            if (!DEAD(ch) && GET_SKILL(ch, SKILL_LUNGE) && (number(1,110) < GET_SKILL(ch, SKILL_LUNGE)))
                side_kick(ch);

            if (!DEAD(ch) && MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
                (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");

            if (!DEAD(ch) && FIGHTING(ch) && IS_THIEF(ch) && GET_SKILL(ch, SKILL_DISTRACT)>number(1, 451))
            {
                SET_BIT(AFF3_FLAGS(FIGHTING(ch)), AFF3_DISTRACT);
                improve_skill(ch, SKILL_DISTRACT, 10);
                act("&cYou -distract- $N's attention!&0", FALSE, ch, 0, FIGHTING(ch), TO_CHAR);
                act("$n -distracts- you for a moment!&0", FALSE, ch, 0, FIGHTING(ch), TO_VICT);
                act("$n -distracts- $N's attention!", FALSE, ch, 0, FIGHTING(ch), TO_NOTVICT);
            }

            remove_combat_affects(ch);	// such as temp blind
            
            if (DEAD(ch) || (FIGHTING(ch) && DEAD(FIGHTING(ch))))
                continue;


            if (FIGHTING(ch) && (ch->in_room==((FIGHTING(ch))->in_room)))
            {
                rprog_rfight_trigger( ch );
                mprog_hitprcnt_trigger(ch, FIGHTING(ch));
                mprog_fight_trigger(ch, FIGHTING(ch));
            }

            //CUREF(ch);
        }
        else
            return; //ch == NULL !!!
    }
}

bool            range_hit(struct char_data * ch, struct char_data * victim, struct obj_data * obj)
{
    int             w_type = 315,
                             victim_ac,
                             calc_thaco,
                             dam = 0,
                                   diceroll,att, def;
    extern int      thaco[NUM_CLASSES][LVL_IMPL + 1];
    extern int      thaco1[NUM_CLASSES][2];




    att=GET_SKILL(ch, SKILL_ARCHERY)+3*GET_INT(ch)+GET_SKILL(ch, SKILL_THROW_DAGGER);
    def=GET_DEX(ch)*2;
    if (GET_POS(victim) == POS_FIGHTING)
        def*=1.5;
    if (GET_RACE(ch)==RACE_ELF || GET_RACE(ch)==RACE_DROW)
        att+=30;

    diceroll = number(1, 100);
    if (((att-def)+100)/2<diceroll)
        return FALSE;
    else if (AFF2_FLAGGED(victim, AFF2_MIRRORIMAGE) && (number(1, 40) > GET_INT(ch)) && (number(1, 40) <= GET_INT(victim))) {
        act("One of $N's false images dissipate and is instantly replaced!", FALSE, ch, 0, victim, TO_CHAR);
        act("One of your images takes the $p $n launched at you.", FALSE, ch, obj, victim, TO_VICT);
        return FALSE;
    } else if (IS_NPC(victim) && IS_SHOPKEEPER(victim)) {// MOB_FLAGGED(victim, MOB_SPEC)) {
        //act("A strange wall of force protects $N from your $p.", FALSE, ch, obj, victim, TO_CHAR);
        return FALSE;
    } else {
        /* okay, we know the guy has been hit.  now calculate damage. */
        dam=0;
        dam=GET_DAMROLL(ch);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
        {
            dam += (GET_OBJ_VAL(obj, 0) + dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2)));
            if (GET_SKILL(ch, SKILL_THROW_DAGGER) && (GET_OBJ_VAL(obj, 3) == TYPE_PIERCE - TYPE_HIT))
             dam+=dam*GET_SKILL(ch, SKILL_THROW_DAGGER)/100;
        } 
        else if (GET_OBJ_TYPE(obj) == ITEM_MISSILE) {
             dam += dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2))+GET_OBJ_VAL(obj, 3);

             if (GET_SKILL(ch, SKILL_ARCHERY))
                 dam+=dam*GET_SKILL(ch, SKILL_ARCHERY)/100;

         }        
        else
            dam += number(GET_OBJ_WEIGHT(obj) / 4, GET_OBJ_WEIGHT(obj) / 2);
            
            
        dam*=str_app[GET_STR(ch)].todam;

        /* if (GET_OBJ_TYPE(obj) == ITEM_MISSILE) {
             dam += dice(GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2))+GET_OBJ_VAL(obj, 3);

             if (GET_SKILL(ch, SKILL_ARCHERY))
                 dam+=dam*GET_SKILL(ch, SKILL_ARCHERY)/100;

         }
         if (GET_SKILL(ch, SKILL_THROW_DAGGER) && (GET_OBJ_VAL(obj, 3) == TYPE_PIERCE - TYPE_HIT))
             dam+=dam*GET_SKILL(ch, SKILL_THROW_DAGGER)/100;
          */

    }
    dam = MAX(1, dam);          /* at least 1 hp damage min per hit */
    damage(ch, victim, dam, w_type, obj);
    return TRUE;
}

bool            fire_at_char(struct char_data * ch, struct char_data * list, struct obj_data * obj, int dir, char *name)
{
    struct char_data *vict = list, *target=NULL;

    char            msgbuf[256];
    if (vict)
    {
        sprintf(msgbuf, "$p flew in from the %s!", dirs2[rev_dir[dir]]);
        act(msgbuf, FALSE, vict, obj, 0, TO_ROOM);
        act(msgbuf, FALSE, vict, obj, 0, TO_CHAR);

    }

    if (!(*name) && vict && !vict->next_in_room)
        target=vict;
    else
        target=get_char_room_vis_in_room(ch, name, (vict!=NULL ? vict->in_room : 0 ));

    //if (!target)
    //send_to_char("No one by that name here.\r\n", ch);


    if (!number(0, 1) && number(1, 101)>GET_SKILL(ch, SKILL_ARCHERY) + GET_SKILL(ch, SKILL_THROW_DAGGER))  // we missed them
        target=NULL;

    if (target)
    {
        vict=target;
        if (ROOM_FLAGGED(vict->in_room, ROOM_PEACEFUL) ||
                (!CAN_MURDER(ch, vict)))
        {
            send_to_char("A strange wall of force protects you from being hit.\r\n", vict);
            return;
        }
        else if (range_hit(ch, vict, obj)) {
            if (vict)
                if (IS_NPC(vict) && GET_POS(vict) > POS_STUNNED) {
                    SET_BIT(MOB_FLAGS(vict), MOB_MEMORY);
                    remember(vict, ch);
                    if (!IS_SHOPKEEPER(vict))//!MOB_FLAGGED(vict, MOB_SENTINEL))
                        HUNTING(vict) = ch;
                }

            return FALSE;
        }
        else
            vict=list;
    }

    while (vict) {              /* while there's a victim */


        if (ROOM_FLAGGED(vict->in_room, ROOM_PEACEFUL) ||
                (!CAN_MURDER(ch, vict)))
            send_to_char("A strange wall of force protects you from being hit.\r\n", vict);
        else if (vict!= target && !number(0, 2) && range_hit(ch, vict, obj)) {
            if (vict) {
                if (IS_NPC(vict) && GET_POS(ch) > POS_STUNNED) {
                    SET_BIT(MOB_FLAGS(vict), MOB_MEMORY);
                    remember(vict, ch);
                    if (!IS_SHOPKEEPER(vict))//!MOB_FLAGGED(vict, MOB_SENTINEL))
                        HUNTING(vict) = ch;
                }
            }
            return FALSE;
        }
        // send_to_char("It misses you.\r\n", vict);
        vict = vict->next_in_room;
    }
    if (vict)
    {
        sprintf(msgbuf, "\r\nLuckily, it doesn't hit anyone.");
        act(msgbuf, FALSE, vict, obj, 0, TO_ROOM);
        act(msgbuf, FALSE, vict, obj, 0, TO_CHAR);

    }
    return TRUE;                /* missed everyone */
}
