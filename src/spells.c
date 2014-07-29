/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "clan.h"
#include "ego.h"

extern SPECIAL(shop_keeper);
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
//extern struct cha_app_type cha_app[];
extern struct int_app_type int_app[];
extern struct index_data *obj_index;
extern struct index_data *mob_index;
extern char     *pc_class_types[];
extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;

extern int      mini_mud;
extern int      pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern char     weapon_verbs[];
extern int     *max_ac_applys;
extern struct apply_mod_defaults *apmd;
extern char    *spells[];
extern char    *spell_wear_off_msg[];

void gain_ap(struct char_data *ch, int points, char* why);
char            *show_char_cond(int percent);
int           do_mapper(struct char_data *ch, int size , int show_closed, struct obj_data *paper);
void            clearMemory(struct char_data * ch);
void            act(char *str, int i, struct char_data * c, struct obj_data * o,
                    void *vict_obj, int j);

void            damage(struct char_data * ch, struct char_data * victim,
                       int damage, int weapontype, struct obj_data * obj);
void            weather_change(void);
void            weight_change_object(struct obj_data * obj, int weight);
void            add_follower(struct char_data * ch, struct char_data * leader);
int             mag_savingthrow(struct char_data * ch, int type, struct char_data * ataker);
bool            CAN_MURDER(struct char_data * ch, struct char_data * victim);
extern int not_in_arena(struct char_data *ch);
/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{
    int             water;

    void            name_to_drinkcon(struct obj_data * obj, int type);
    void            name_from_drinkcon(struct obj_data * obj);

    if (ch == NULL || obj == NULL)
        return;
    level = MAX(MIN(level, LVL_IMPL), 1);

    if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
        if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
            name_from_drinkcon(obj);
            GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
            name_to_drinkcon(obj, LIQ_SLIME);
        } else {
            water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
            if (water > 0) {
                if (GET_OBJ_VAL(obj, 1) >= 0)
                    name_from_drinkcon(obj);
                GET_OBJ_VAL(obj, 2) = LIQ_WATER;
                GET_OBJ_VAL(obj, 1) += water;
                name_to_drinkcon(obj, LIQ_WATER);
                weight_change_object(obj, water);
                act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
            }
        }
    }
}


ASPELL(spell_astral_projection)
{
    struct obj_data *paper = NULL;
    struct obj_data *next_obj;
    char *paper_name=tar_str;
    char buf[2000];

    int scroll = 0, found = FALSE;
    if (not_in_arena(ch))
        return;

    if (ch==NULL || ch->in_room==NOWHERE)
        return;

    if (!paper_name){
        send_to_char("You need to specify on what should be map drawn.\r\n", ch);
        return;
    }

    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(paper = get_obj_in_list_vis(ch, paper_name,ch->carrying)))
            continue;
        else
            found = TRUE;
    }
    if (found && GET_OBJ_TYPE(paper) != ITEM_NOTE && GET_OBJ_TYPE(paper)!= ITEM_SCROLL) {
        send_to_char("You can't draw on that.\r\n", ch);
        return;
    }
    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",paper_name);
        send_to_char(buf, ch);
        return;
    }


    if (SECT(ch->in_room)==SECT_CITY)
    {
        send_to_char("All this surrounding noise disturb your concentration.\r\n",ch);
        write_to_mapobj(ch, paper,"?????\r\n??*??\r\n?????\r\n");
        return;
    }

    send_to_char("You sink into a deep trans.\r\n", ch);
    act("$n's body dimms for a brief moment.", FALSE, ch, NULL, NULL, TO_ROOM);

    if (!do_mapper(ch, MIN(15, MAX(5, GET_LEVEL(ch)/3)), 0, paper))
        send_to_char("\r\nThe paper was used so many times that you finnaly couldn't understand a single\r\nword you write so you decide to tear it apart.\r\n",ch);
    /*    else
            WAIT_STATE(ch, 2*PULSE_VIOLENCE);*/

}

ASPELL(spell_omni_eye)
{
    struct descriptor_data *d;
    struct char_data *i;
    char bufoe[2000];
    int num = 0, found = 0;

    if (ch==NULL || ch->in_room==NOWHERE)
        return;

    send_to_char("You spread your mind over the lands...\r\n\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
        if (!d->connected) {
            i = (d->original ? d->original : d->character);
            if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) && !AFF3_FLAGGED(i, AFF3_SHROUD)) {
                sprintf(bufoe, "You see %s in %s health being at %s, somewhere in %s.\r\n", GET_NAME(i), show_char_cond(GET_HIT(i)*100/GET_MAX_HIT(i)), world[i->in_room].name, zone_table[world[i->in_room].zone].name);
                send_to_char(bufoe, ch);
            }
        }

}


ASPELL(spell_gust_of_wind)
{

    if (ch==NULL || ch->in_room==NOWHERE || victim == NULL || victim->in_room==NOWHERE)
        return;

    if (IS_SHOPKEEPER(victim) /*MOB_FLAGGED(victim, MOB_SENTINEL)*/ || mag_savingthrow(victim, SAVING_SPELL, ch) || 2*GET_LEVEL(ch)>number(0,(GET_TOTAL_WEIGHT(victim))) )
    {
        act("$n stands firmly on $s place.",FALSE,victim,NULL, NULL, TO_ROOM);
        send_to_char("As wind starts to blow, you shake a little but stand firmly on your place.\r\n",victim);
        return;
    }

    if (!blow_out(victim))
    {
        act("$n barely manages to stand $s place.",FALSE,victim,NULL, NULL, TO_ROOM);
        send_to_char("As wind starts to blow, you barely manage to stand your place.\r\n",victim);
        return;
    }

}


ASPELL(spell_recall)
{
    extern sh_int   r_mortal_start_room;
    extern int clan_loadroom[];

    if (not_in_arena(ch))
        return;

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
        send_to_char("Only way to leave is as a winner or as a looser.", ch);
        return;
    }
    if (victim == NULL)
        victim = ch;

    if (IS_NPC(victim))
        return;

    if ( ROOM_FLAGGED(ch->in_room,ROOM_NORECALL) )
    {
        send_to_char( "For some strange reason... nothing happens.\r\n", ch );
        return;
    }
    if ((ROOM_FLAGGED(ch->in_room,ROOM_GODROOM) || ROOM_FLAGGED(victim->in_room,ROOM_GODROOM)))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }

    act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, real_room(victim->player.hometown>1?victim->player.hometown:clan_loadroom[GET_CLAN(victim)]));
    act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    look_at_room(victim, 0);
}


extern int      top_of_world;
ASPELL(spell_teleport)
{
    int             to_room;
    if (not_in_arena(ch))
        return;


    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
        send_to_char("You can not do that here!\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }
    if ( ROOM_FLAGGED(ch->in_room,ROOM_NORECALL) )
    {
        send_to_char( "For some strange reason... nothing happens.\r\n", ch );
        return;
    }

    do {
        to_room = number(10, top_of_world);
    } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_ARENA | ROOM_NOMAGIC));

    act("$n slowly fades out of existence and is gone.",
        FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, to_room);
    act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
    look_at_room(victim, 0);
}




ASPELL(spell_local_teleport)
{
    int             to_room;
    extern int      top_of_world;

    if (not_in_arena(ch))
        return;

    if (!victim)
        return;

    if (IS_NPC(victim))
    {
        send_to_char("Your magic is unable to local teleport monsters.\r\n",ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
        send_to_char("You can not do that here!\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }
    if ( ROOM_FLAGGED(ch->in_room,ROOM_NORECALL) )
    {
        send_to_char( "For some strange reason... nothing happens.\r\n", ch );
        return;
    }

    if (ch!=victim  && mag_savingthrow(victim, SAVING_SPELL, ch))
    {
        act("$N resists your magic.", FALSE, ch, NULL, victim, TO_CHAR);
        return;
    }


    do {
        to_room = number(0, top_of_world);
    } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_ARENA | ROOM_NOMAGIC) || world[to_room].zone!=world[ch->in_room].zone);

    act("$n slowly fades out of existence and is gone.",
        FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, to_room);
    act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
    look_at_room(victim, 0);
}


#define SUMMON_FAIL "A shimmering portal appears in front of you, but soon it closes.\r\n"
#define SUMMON_FAIL2 "You fail to form a portal.\r\n"

ASPELL(spell_summon)
{

    if (ch == NULL || victim == NULL)
        return;
        
    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) || ROOM_FLAGGED(victim->in_room, ROOM_ARENA)) {
        send_to_char("First let the arena finish!", ch);
        return;
    }

    if (ROOM_FLAGGED(victim->in_room,ROOM_PEACEFUL) || ROOM_FLAGGED(victim->in_room,ROOM_NOMAGIC) || ROOM_FLAGGED(victim->in_room,ROOM_NORECALL)  || ROOM_FLAGGED(victim->in_room,ROOM_NOSUMMON))
    {
        send_to_char("You can't activate the spell on the other side.\r\n",ch);
        return;
    }


    if (ROOM_FLAGGED(ch->in_room,ROOM_PRIVATE) || ROOM_FLAGGED(ch->in_room,ROOM_TUNNEL))
    {
        send_to_char("There is not enough room here..\r\n",ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room,ROOM_NOSUMMON))
    {
        send_to_char( "For some strange reason... nothing happens.\r\n", ch );

        return;
    }

    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM) || ROOM_FLAGGED(victim->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }

    if (ch==victim)
    {
    	send_to_char("Summon yourself? You want to end up like Artur?\r\n", ch);
    	return;
    }
    
    if (FIGHTING(victim))
    {
        send_to_char("You can't seem to get a precise fix on your victim's location.\r\n",ch);
        return;
    }

    /*  if (GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, level + 3)) {
        send_to_char(SUMMON_FAIL, ch);
        return;
      }
    */
    if (GET_LEVEL(ch) < GET_LEVEL(victim)) {
        send_to_char(SUMMON_FAIL2, ch);
        return;
    }

    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN)) {
        act("As the words escape your lips and $N travels\r\n"
            "through time and space towards you, you realize that $E is\r\n"
            "aggressive and might harm you.",
            //, so you wisely send $M back.",
            FALSE, ch, 0, victim, TO_CHAR);
        if ( mag_savingthrow(victim, SAVING_SPELL, ch))
        {
            send_to_char(SUMMON_FAIL,ch);
            return;
        }

    }

    if (!IS_NPC(victim) && (!PRF_FLAGGED(victim, PRF_SUMMONABLE))) {
        if (!tar_str || !(*tar_str) || !isname("mob_sum", tar_str)){
            sprintf(buf, "Someone just tried to summon you to: %s.\r\n"
                    "%s failed because you have summon protection on.\r\n"
                    "Type NOSUMMON to allow other players to summon you.\r\n",
                    world[ch->in_room].name,
                    (ch->player.sex == SEX_MALE) ? "He" : "She");
            send_to_char(buf, victim);

            sprintf(buf, "You failed because %s has summon protection on.\r\n",
                    GET_NAME(victim));
            send_to_char(buf, ch);

            sprintf(buf, "SUMMON: %s failed summoning %s to %s.",
                    GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
            mudlog(buf, BRF, LVL_IMMORT, TRUE);
            return;
        }
    }

    if (MOB_FLAGGED(victim, MOB_NOSUMMON) ||
            (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL, ch))) {
        send_to_char(SUMMON_FAIL, ch);
        return;
    }

    /*
        if (!IS_NPC(victim) && CAN_MURDER(ch, victim) && mag_savingthrow(victim, SAVING_SPELL, ch)) {
            send_to_char(SUMMON_FAIL, ch);
            return;
        }
      */  
    act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

    char_from_room(victim);
    char_to_room(victim, ch->in_room);

    act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
    act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE) )
        hit(victim,ch, TYPE_UNDEFINED);
    if (!IS_NPC(ch))
        look_at_room(victim, 0);
}

#define SUMMON_FAIL1 "You see an unclear vision of some distant place.\r\n"

ASPELL(spell_clairvoyance)
{
    int             wasin;

    if (ch == NULL || victim == NULL)
        return;

    if (AFF3_FLAGGED(victim, AFF3_SHROUD) || mag_savingthrow(victim, SAVING_SPELL, ch) || (ROOM_FLAGGED(victim->in_room, ROOM_PRIVATE | ROOM_DEATH))) {
        send_to_char(SUMMON_FAIL1, ch);
        return;
    }
    wasin = ch->in_room;
    SET_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    look_at_room(ch, 1);
    char_from_room(ch);
    REMOVE_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
    char_to_room(ch, wasin);
}

ASPELL(spell_transport)
{
    if (ch == NULL || obj == NULL)
        return;

    if (!OUTSIDE(ch)) {
        send_to_char("You must be outdoors to make it succeed.\r\n", ch);
        return;
    }
    if (number(1, 101) > GET_SKILL(ch, SPELL_TRANSPORT)) {
        act("$p mysticly flies up and then falls back down.", FALSE, ch, obj, NULL, TO_CHAR);
        act("$p mysticly flies up and then falls back down.", FALSE, ch, obj, NULL, TO_ROOM);
        obj_from_char(obj);
        obj_to_room(obj, ch->in_room);
        return;
    }
    act("$p mysticly flies up and loses from the sight.", FALSE, ch, obj, NULL, TO_CHAR);
    act("$p mysticly flies up and loses from the sight.", FALSE, ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    global_no_timer=1;
    obj_to_room(obj, real_room(VOIDR));
    global_no_timer=0;
}

ASPELL(spell_retransport)
{
    struct obj_data *obj1,
                *next_obj;
    int             dotmode,
    found = 0;
    char            name[MAX_INPUT_LENGTH];
    strcpy(name, tar_str);

    if (ch == NULL)
        return;

if (!(obj1 = get_obj_in_list_vis(ch, name, world[real_room(VOIDR)].contents))) {
        sprintf(buf, "You can not feel the presence of %s.\r\n", name);
        send_to_char(buf, ch);
    } else {

        obj_from_room(obj1);
        obj_to_room(obj1, ch->in_room);
        act("$p mysteriously materialises and falls down.", FALSE, ch, obj, NULL, TO_CHAR);
        act("$p mysteriously materialises and falls down.", FALSE, ch, obj, NULL, TO_ROOM);
    }
}

ASPELL(spell_locate_object)
{
    struct obj_data *i, *last=NULL;
    char            name[MAX_INPUT_LENGTH];
    int             j;

    strcpy(name, tar_str);
    //    strcpy(name, fname(obj->name));
    j = level >> 1;

    for (i = object_list; i && (j > 0); i = i->next) {
        if (!isname(name, i->name))
            continue;

        if (last!=NULL && GET_OBJ_VNUM(last)==GET_OBJ_VNUM(i) && last->in_room==i->in_room)
            continue;


        if (i->carried_by)
            sprintf(buf, "%s is being carried by %s.\r\n",
                    i->short_description, PERS(i->carried_by, ch));
        else if (i->in_room != NOWHERE)
            sprintf(buf, "%s is somewhere in %s.\r\n", i->short_description,
                    zone_table[world[i->in_room].zone].name);
        else if (i->in_obj)
            sprintf(buf, "%s is in %s.\r\n", i->short_description,
                    i->in_obj->short_description);
        else if (i->worn_by)
            sprintf(buf, "%s is being worn by %s.\r\n",
                    i->short_description, PERS(i->worn_by, ch));
        else
            sprintf(buf, "%s's location is uncertain.\r\n",
                    i->short_description);

        CAP(buf);
        send_to_char(buf, ch);
        last=i;
        j--;
    }

    if (j == level >> 1)
        send_to_char("You sense nothing.\r\n", ch);
}

ASPELL(spell_locate_creature)
{
    struct char_data *i, *last=NULL;
    char            name[MAX_INPUT_LENGTH];
    int             j;

    strcpy(name, tar_str);
    j = level >> 1;
    for (i = character_list; i && (j > 0); i = i->next) {
        if (!IS_NPC(i))
            continue;
        if (!isname(name, i->player.name))
            continue;

        if (last!=NULL && GET_MOB_VNUM(last)==GET_MOB_VNUM(i) &&  last->in_room==i->in_room)
            continue;

        if (i->in_room != NOWHERE)
            sprintf(buf, "%s is in %s.\r\n", (i->player.short_descr ? i->player.short_descr : i->player.name),
                    zone_table[world[i->in_room].zone].name);
        else
            sprintf(buf, "%s's location is uncertain.\r\n",
                    (i->player.short_descr ? i->player.short_descr : i->player.name));

        CAP(buf);
        send_to_char(buf, ch);
        last=i;
        j--;
    }

    if (j == level >> 1)
        send_to_char("You sense nothing.\r\n", ch);
}



ASPELL(spell_charm)
{
    struct affected_type af;
    int type;

    type = (!strcmp(tar_str, "nospell")? 0 : 1);

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
        send_to_char("You can't do that in arena!\r\n", ch);
        return;
    }
    if (victim == NULL || ch == NULL)
        return;
    if (!CAN_MURDER(ch, victim))
        return;
    if (count_pets(ch)>=NUM_PETS_ALLOWED)
    {
        send_to_char(MSG_TOO_MANY_PETS, ch);
        return;
    }
    if (GET_CHA(ch) +GET_WIS(ch)< GET_CHA(victim)+GET_WIS(victim))
        send_to_char("Seems your victim has stronger character then you thought.\r\n", ch);
    else if (victim == ch)
        send_to_char("You like yourself even better!\r\n", ch);
    else if (IS_AFFECTED(victim, AFF_SANCTUARY))
        send_to_char("Your victim is protected by Sanctuary!\r\n", ch);
    else if (IS_NPC(victim) && mob_index[GET_MOB_RNUM(victim)].func == shop_keeper)
        send_to_char("Your victim resists!\r\n", ch);
    else if (MOB_FLAGGED(victim, MOB_NOCHARM))
        send_to_char("Your victim resists!\r\n", ch);
    else if (IS_AFFECTED(ch, AFF_CHARM))
        send_to_char("You can't have any followers of your own!\r\n", ch);
    else if (IS_AFFECTED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
        send_to_char("You fail.\r\n", ch);
    /* player charming another player - no legal reason for this */
    else if (!IS_NPC(victim))
        send_to_char("You fail - shouldn't be doing it anyway.\r\n", ch);
    else if (circle_follow(victim, ch))
        send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
    else if (mag_savingthrow(victim, SAVING_PARA, ch))
        send_to_char("Your victim barely resists!\r\n", ch);
    else {
        if (victim->master)
            stop_follower(victim);
        act("$N falls under your influence.", FALSE, ch, NULL, victim, TO_CHAR);
        act("$N falls under $n's influence.", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
        add_follower(victim, ch);
        perform_group(ch, victim);

        af.type = 0;
        af.bitvector = 0;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.type = SPELL_CHARM;

        af.duration =type? -2 : 24 + GET_CHA(ch) - GET_CHA(victim);

        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);
        if (type)
            add_leech(ch, victim, SPELL_CHARM, mag_manacost(ch, SPELL_CHARM));

        if (IS_NPC(victim)) {
            REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);
           // REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
        }
    }
}

extern char *obj_condition_names[];

ASPELL(spell_identify)
{
    int             i;
    int             found;

    struct time_info_data age(struct char_data * ch);

    extern char    *spells[];

    extern char    *item_types[];
    extern char    *extra_bits[];
    extern char    *extra_bits2[];
    extern char    *apply_types[];
    extern char    *affected_bits[];
    extern char    *affected_bits2[];
    extern char    *affected_bits3[];
    struct Sego_weapons pom;

    if (obj) {
        if (IS_SET(GET_OBJ_EXTRA(obj), ITEM_HIDDEN_EGO))      //ego not yet finished
        {
            pom=make_ego(obj, GET_OBJ_LEVEL(obj));
            if (pom.min_lev==-1)
            {
                log("SCREWED EGO WEAPON!");
                send_to_char("Something is weird with this object. Report it to immortal.\r\n", ch);
                return;
            }
            if (check_existing_ego(pom.name, GET_OBJ_TYPE(obj))>=0)
            {
                if (GET_LEVEL(ch)<GET_OBJ_LEVEL(obj)+15)
                    gain_ap(ch, MAX(1, GET_OBJ_LEVEL(obj)+pom.min_lev)+1, "revealing an ego item");
            }
            else
            {
                gain_ap(ch, (GET_OBJ_LEVEL(obj)+pom.min_lev)*2, "revealing a *new* ego item");
                write_ego_to_file(pom, GET_OBJ_TYPE(obj));
            }

        }
        //send_to_char("You feel informed:\r\n", ch);
        sprintf(buf, "Object '&c%s&0', Item type: &c", obj->short_description);
        sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
        if (GET_OBJ_TYPE(obj)==ITEM_CONTAINER)
            sprintf(buf2,"%s (holds %d)", buf2, GET_OBJ_VAL(obj, 0));
	if (GET_OBJ_TYPE(obj)==ITEM_WEAPON)
            sprintf(buf2,"%s", spells[GET_OBJ_VAL(obj, 3)+TYPE_HIT]);            
        strcat(buf, buf2);
        strcat(buf, "&0\r\n");
        send_to_char(buf, ch);
                    
       if (CAN_WEAR(obj, ITEM_WEAR_TAKE))
       {
        send_to_char("Can be worn on: &c", ch);
    	sprintbit(obj->obj_flags.wear_flags, wear_bits, buf);
    	ch_printf(ch, "%s&0\r\n", buf); 
	}

        if (obj->bound_spell)
        {
            sprintf(buf, "Item is enhanced with a bound spell: &c%s at level %d&0\r\n", spells[obj->bound_spell], obj->bound_spell_level);
            send_to_char(buf, ch);
        }
        if (obj->obj_flags.bitvector || obj->obj_flags.bitvector2 || obj->obj_flags.bitvector3) {
            send_to_char("Item will give you following permament abilities: &c ", ch);
            if (obj->obj_flags.bitvector) {
                sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
                send_to_char(buf, ch);}
            if (obj->obj_flags.bitvector2) {
                sprintbit(obj->obj_flags.bitvector2, affected_bits2, buf);
                send_to_char(buf, ch);}
            if (obj->obj_flags.bitvector3) {
                sprintbit(obj->obj_flags.bitvector3, affected_bits3, buf);
                send_to_char(buf, ch);}
            send_to_char("&0\r\n", ch);
        }

        send_to_char("Item is:&c ", ch);
        sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
        send_to_char(buf, ch);
        if (GET_OBJ_EXTRA2(obj))
        {
            sprintbit(GET_OBJ_EXTRA2(obj), extra_bits2, buf);
            strcat(buf, "&0\r\n");
            send_to_char(buf, ch);
        }
        else
            send_to_char("&0\r\n", ch);

        sprintf(buf, "Weight: %3.1f kg, Value: &y%d&0 gold coins\r\n",
                (float) GET_OBJ_WEIGHT(obj)/2.0, GET_OBJ_COST(obj));
        send_to_char(buf, ch);

        ch_printf(ch, "Item is in %s condition.\r\n", obj_condition_names[GET_OBJ_DAMAGE(obj)/10]);
        switch (GET_OBJ_TYPE(obj)) {
        case ITEM_SCROLL:
        case ITEM_POTION:
            sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);

            if (GET_OBJ_VAL(obj, 1) >= 1)
                sprintf(buf, "%s &P%s&0", buf, spells[GET_OBJ_VAL(obj, 1)]);
            if (GET_OBJ_VAL(obj, 2) >= 1)
                sprintf(buf, "%s &P%s&0", buf, spells[GET_OBJ_VAL(obj, 2)]);
            if (GET_OBJ_VAL(obj, 3) >= 1)
                sprintf(buf, "%s &P%s&0", buf, spells[GET_OBJ_VAL(obj, 3)]);
            sprintf(buf, "%s (at level %d)\r\n", buf,GET_OBJ_VAL(obj, 0));
            send_to_char(buf, ch);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);
            sprintf(buf, "%s &P%s&0 (at level %d)\r\n", buf, spells[GET_OBJ_VAL(obj, 3)],GET_OBJ_VAL(obj, 0));
            sprintf(buf, "%sIt has %d maximum charge%s and %d remaining.\r\n", buf,
                    GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
                    GET_OBJ_VAL(obj, 2));
            send_to_char(buf, ch);
            break;
        case ITEM_WEAPON:
            sprintf(buf, "Weapon potential damage is &c%dd%d+%d&0", GET_OBJ_VAL(obj, 1),
                    GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 0));
            sprintf(buf, "%s (average damage of &c%.1f&0)\r\n", buf,
                    (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1) + GET_OBJ_VAL(obj, 0)));
            send_to_char(buf, ch);
            if (GET_OBJ_DAMAGE(obj)<90)
                ch_printf(ch, "Due to condition, average damage reduced to &c%.1f&0\r\n", AVE_DAM(obj)*GET_OBJ_DAMAGE(obj)/100.0);
            break;
        case ITEM_FIREWEAPON:
            sprintf(buf, "Weapon damage is &c%dd%d+%d&0", GET_OBJ_VAL(obj, 1),
                    GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 0));
            sprintf(buf, "%s (average damage of &c%.1f&0)\r\n", buf,
                    (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1) + GET_OBJ_VAL(obj, 0)));
            send_to_char(buf, ch);
            if (GET_OBJ_DAMAGE(obj)<90)
                ch_printf(ch, "Due to condition, average damage reduced to &c%.1f&0\r\n", AVE_DAM(obj)*GET_OBJ_DAMAGE(obj)/100.0);
            send_to_char("If arrow is fired this damage is added to arrow's final damage.\r\n", ch);
            break;
        case ITEM_MISSILE:
            sprintf(buf, "Arrow damage is &c%dd%d&0", GET_OBJ_VAL(obj, 1),
                    GET_OBJ_VAL(obj, 2));
            sprintf(buf, "%s and the maximum firing distance is &c%d&0\r\n", buf,
                    GET_OBJ_VAL(obj, 0));
            send_to_char(buf, ch);
            break;
        case ITEM_ARMOR:
            if (GET_OBJ_VAL(obj, 0) == GET_OBJ_VAL(obj, 1))
                sprintf(buf, "Armor class: &c%d&0\r\n", GET_OBJ_VAL(obj, 0));
            else
                sprintf(buf, "Armor class: &c%d&0 (%d)\r\n", GET_OBJ_VAL(obj, 0), GET_OBJ_VAL(obj, 1));
            send_to_char(buf, ch);

            break;
        }
        found = FALSE;
        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].location != APPLY_LEVEL) &&
                    (obj->affected[i].modifier != 0)) {
                if (!found) {
                    send_to_char("Can affect you as :\r\n", ch);
                    found = TRUE;
                }
                if (obj->affected[i].location>=0)
                    sprinttype(obj->affected[i].location, apply_types, buf2);
                else
                    sprintf(buf2, "SKILL '%s'", spells[-obj->affected[i].location]);
                sprintf(buf, "   Affects: &c%s by %c%d&0\r\n", buf2, obj->affected[i].modifier>0 ? '+':'-', abs(obj->affected[i].modifier));
                send_to_char(buf, ch);
            }
        }
    } else if (victim) {        /* victim */
        sprintf(buf, "Name: &c%s&0 Class: &c%s&0 Deity: &c%s&0\r\n", GET_NAME(victim), pc_class_types[GET_CLASS_NUM(victim)], DEITY_NAME(victim));
        send_to_char(buf, ch);
        if (!IS_NPC(victim)) {
            sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
                    GET_NAME(victim), age(victim).year, age(victim).month,
                    age(victim).day, age(victim).hours);
            send_to_char(buf, ch);
        }
        *buf='\0';
        if (!IS_NPC(victim))
            sprintf(buf, "Height %d cm, Weight %d kg (%d total)\r\n",
                    GET_HEIGHT(victim), GET_WEIGHT(victim)/2+(GET_COND(ch, FULL) ? 0 : -1), GET_TOTAL_WEIGHT(victim)/2+(GET_COND(victim, FULL) ? 0 : -1));
        /*        sprintf(buf, "%sLevel: %d, Vitality: %d\r\n", buf,
                        GET_LEVEL(victim), GET_HIT(victim));*/
        sprintf(buf, "%sLevel: &c%d&0, Hits: &c%d&0, Mana: &c%d&0\r\n", buf,
                GET_LEVEL(victim), GET_HIT(victim), GET_MANA(victim));
        sprintf(buf, "%sArmor Class: &c%d&0/10 Magic Resistance: &c%d&0\r\nHitroll: &c%d&0, Damroll: &c%d&0\r\n", buf,
                GET_AC(victim), GET_MAGAC(victim), GET_HITROLL(victim), GET_DAMROLL(victim));
        sprintf(buf, "%sStr: &c%d&0, Int: &c%d&0, Will: &c%d&0, Dex: &c%d&0, Con: &c%d&0, Cha: &c%d&0\r\n",
                buf, GET_STR(victim), GET_INT(victim),
                GET_WIS(victim), GET_DEX(victim), GET_CON(victim), GET_CHA(victim));
        sprintf(buf, "%sSaves vs Spells/Breaths/Items: &c%d&0/&c%d&0/&c%d&0\r\n", buf, GET_SAVE(victim, SAVING_SPELL), GET_SAVE(victim, SAVING_BREATH),GET_SAVE(victim, SAVING_ROD));
        if (!IS_NPC(victim))
            sprintf(buf, "%sCan carry %3.1f and can wield %3.1f kg.\r\n", buf,
                    (float) CAN_CARRY_W(victim)/2.0, (float) CAN_WIELD_W(victim)/2.0);
        send_to_char(buf, ch);

    }
}



ASPELL(spell_enchant_weapon)
{
    int             i;
    int             free1,
    free2;
    free1 = -1;
    free2 = -1;

    if (ch == NULL || obj == NULL)
        return;

    if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
            !IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {

        for (i = 0; i < MAX_OBJ_AFFECT; i++)
            if ((obj->affected[i].location == APPLY_HITROLL) || (obj->affected[i].location==APPLY_DAMROLL)) {
                if (free1 < 0)
                    free1 = i;
                else if (free2 < 0)
                    free2 = i;
            }
        for (i = 0; i < MAX_OBJ_AFFECT; i++)
            if (!obj->affected[i].location) {
                if (free1 < 0)
                    free1 = i;
                else if (free2 < 0)
                    free2 = i;
            }

        if (free1 < 0)
            free1 = 0;
        if (free2 < 0)
            free2 = 1;
        if (free1==free2)
            free2++;
        if (free2>=MAX_OBJ_AFFECT)
            free2=0;
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODISP_MAG);

        obj->affected[free1].location = APPLY_HITROLL;
        obj->affected[free1].modifier = level / 10 - 1 + number(0, 2)+(level > 49 ? level - 49 :0);
        if (GET_SKILL(ch, SKILL_GREATER_ENCH))
            obj->affected[free1].modifier+=obj->affected[free1].modifier*number(1,GET_SKILL(ch, SKILL_GREATER_ENCH))/160;

        obj->affected[free2].location = APPLY_DAMROLL;
        obj->affected[free2].modifier = level / 10 - 1 + number(0, 2)+(level>49 ? level-49 : 0);
        if (GET_SKILL(ch, SKILL_GREATER_ENCH))
            obj->affected[free2].modifier+=obj->affected[free2].modifier*number(1,GET_SKILL(ch, SKILL_GREATER_ENCH))/160;

        if (IS_GOOD(ch)) {
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
            act("$p glows &bblue&0.", FALSE, ch, obj, 0, TO_CHAR);
        } else if (IS_EVIL(ch)) {
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
            act("$p glows &rred&0.", FALSE, ch, obj, 0, TO_CHAR);
        } else {
            act("$p glows &yyellow&0.", FALSE, ch, obj, 0, TO_CHAR);
        }
        /*        	    {
        	    char buftr[1000];
            	    sprintf(buftr, "%s (enchanted)", obj->short_description);
        	    obj->short_description = str_dup(buftr);
        	}*/
    } else
        act("You fail.", FALSE, ch, obj, 0, TO_CHAR);
}


ASPELL(spell_enchant_armor)
{
    int             i;
    int             free1,
    free2;
    char bea[1000];
    free1 = -1;
    free2 = -1;
    if (ch == NULL || obj == NULL)
        return;

    if ((GET_OBJ_TYPE(obj) == ITEM_ARMOR) &&
            !IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {
        for (i = 0; i < MAX_OBJ_AFFECT; i++)
            if ((obj->affected[i].location== APPLY_AC) || (obj->affected[i].location == APPLY_SAVING_SPELL)) {
                if (free1 < 0)
                    free1 = i;
                else if (free2 < 0)
                    free2 = i;
            }

        for (i = 0; i < MAX_OBJ_AFFECT; i++)
        {
            if (!(obj->affected[i].location)) {
                if (free1 < 0)
                    free1 = i;
                else if (free2 < 0)
                    free2 = i;
            }
        }
        if (free1 < 0)
            free1 = 0;
        if (free2 < 0)
            free2 = 1;
        if (free1==free2)
            free2++;
        if (free2>=MAX_OBJ_AFFECT)
            free2=0;

        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODISP_MAG);
        obj->affected[free1].location = APPLY_AC;
        obj->affected[free1].modifier = (level / 8 - 1 + number(0, 2))-(level>49 ? level-49:0);
        if (GET_SKILL(ch, SKILL_GREATER_ENCH))
            obj->affected[free1].modifier+=obj->affected[free1].modifier*number(1,GET_SKILL(ch, SKILL_GREATER_ENCH))/110;

        obj->affected[free2].location = APPLY_SAVING_SPELL;
        obj->affected[free2].modifier = -((level / 10) - 1 + number(0, 2))-(level>49 ? level-49:0);
        if (GET_SKILL(ch, SKILL_GREATER_ENCH))
            obj->affected[free2].modifier+=obj->affected[free2].modifier*number(1,GET_SKILL(ch, SKILL_GREATER_ENCH))/160;

        if (IS_GOOD(ch)) {
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
            act("$p glows &bblue&0.", FALSE, ch, obj, 0, TO_CHAR);
        } else if (IS_EVIL(ch)) {
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
            act("$p glows &rred&0.", FALSE, ch, obj, 0, TO_CHAR);
        } else {
            act("$p glows &yyellow&0.", FALSE, ch, obj, 0, TO_CHAR);
        }
        /*            {
        	    char buftr[1000];
            	    sprintf(buftr, "%s (enchanted)", obj->short_description);
        	    obj->short_description = str_dup(buftr);
        	}*/
    } else
        act("You fail.", FALSE, ch, obj, 0, TO_CHAR);
}

ASPELL(spell_dispel_magic)
{
    if (ch == NULL || victim == NULL)
        return;

    if (!CAN_MURDER(ch,victim))
        // || (!IS_NPC(ch) && (        /* !IS_NPC(victim) || */
        //              (IS_AFFECTED(victim, AFF_GROUP) && (ch == victim->master)))))
        /* Defensive spell - remove ALL effects */
    {
        if (victim->affected)
            while (victim->affected)
            {leech_from_char(victim, victim->affected->type);
                affect_remove(victim, victim->affected);
            }

        if (victim == ch) {
            act("You have removed magic effects from yourself.", FALSE,
                ch, NULL, NULL, TO_CHAR);
            act("$n has removed magic effects from $mself.", FALSE,
                ch, NULL, NULL, TO_ROOM);
        } else {
            act("You have removed magic effects from $N.", FALSE,
                ch, NULL, victim, TO_CHAR);
            act("$n has removed magic effects from you.", FALSE,
                ch, NULL, victim, TO_VICT);
            act("$n has removed magic effects from $N.", FALSE,
                ch, NULL, victim, TO_NOTVICT);
        }
        return;
    } else
        /* Offensive spell - enforced by multi_hit whether succeeds or fails */
    {
        if (!IS_NPC(victim) && !pk_allowed && !CAN_MURDER(ch, victim)) {
            send_to_char("Mark of neutrality protects your victim.\r\n", ch);
            return;
        }
        if (victim->affected)
            while (victim->affected) {
                if (!mag_savingthrow(victim, SAVING_SPELL, ch)) {
                    if (*spell_wear_off_msg[victim->affected->type]) {
                        send_to_char(spell_wear_off_msg[victim->affected->type], victim);
                        send_to_char("\r\n", victim);
                    }
                    act("$n is no longer affected by $T.", FALSE,
                        victim, NULL, spells[victim->affected->type], TO_ROOM);
                    leech_from_char(victim, victim->affected->type);
                    affect_remove(victim, victim->affected);
                    break;
                }
            }
        /* ALWAYS give a shot at removing sanctuary */
        if (IS_AFFECTED(victim, AFF_SANCTUARY)
                && (!mag_savingthrow(victim, SAVING_SPELL, ch)) && (!mag_savingthrow(victim, SAVING_SPELL, ch))) {
            REMOVE_BIT(AFF_FLAGS(victim), AFF_SANCTUARY);
            affect_from_char(victim, SPELL_SANCTUARY);
            leech_from_char(victim, SPELL_SANCTUARY);
            send_to_char("The white aura around your body fades.\r\n",
                         victim);
            act("The white aura around $n's body fades.", FALSE,
                victim, NULL, NULL, TO_ROOM);
        }
        if (ch != victim)
        {
            hit(victim, ch, TYPE_UNDEFINED);
            hit(victim, ch, TYPE_UNDEFINED);
        }
    }
    return;
}


ASPELL(spell_detect_poison)
{
    if (victim) {
        if (victim == ch) {
            if (IS_AFFECTED(victim, AFF_POISON))
                send_to_char("You can sense poison in your blood.\r\n", ch);
            else
                send_to_char("You feel healthy.\r\n", ch);
        } else {
            if (IS_AFFECTED(victim, AFF_POISON))
                act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
            else
                act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
        }
    }
    if (obj) {
        switch (GET_OBJ_TYPE(obj)) {
        case ITEM_DRINKCON:
        case ITEM_FOUNTAIN:
        case ITEM_FOOD:
            if (GET_OBJ_VAL(obj, 3))
                act("You sense that $p has been contaminated.", FALSE, ch, obj, 0, TO_CHAR);
            else
                act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
                    TO_CHAR);
            break;
        default:
            send_to_char("You sense that it should not be consumed.\r\n", ch);
        }
    }
}

ASPELL(spell_peace)
{
    struct char_data *temp;
    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
        send_to_char("PEACE is unknown word in arena!", ch);
        return;
    }
   // if (number(1, 25) <= GET_CHA(ch)) {
        //act("$n makes a prayer.", TRUE, ch, 0, 0, TO_ROOM);
        //act("You make your prayer.", FALSE, ch, 0, 0, TO_CHAR);

        for (temp = world[ch->in_room].people; temp; temp = temp->next_in_room)
            if (FIGHTING(temp) && (GET_LEVEL(temp) < GET_LEVEL(ch))) {
                stop_fighting(temp);
                if (IS_NPC(temp)) {
                    clearMemory(temp);
                }
                if (ch != temp) {
                    act("$n stops fighting.", TRUE, temp, 0, 0, TO_ROOM);
                    act("You stop fighting.", TRUE, temp, 0, 0, TO_CHAR);
                }
            }
    //} else
      //  send_to_char("You fail.\r\n", ch);
           if (FIGHTING(ch))
           	stop_fighting(ch);
    return;
}

ASPELL(spell_relocate)
{
    if (ch == NULL || victim == NULL)
        return;

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) || ROOM_FLAGGED(ch->in_room, ROOM_PRIVATE)) {
        send_to_char("You can not do that here!\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(victim->in_room,ROOM_PRIVATE) || ROOM_FLAGGED(victim->in_room,ROOM_TUNNEL))
    {
        send_to_char("There is not enough room there to open a portal.\r\n",ch);
        return;
    }

    if (ROOM_FLAGGED(victim->in_room,ROOM_PEACEFUL) || ROOM_FLAGGED(victim->in_room,ROOM_NOMAGIC) || ROOM_FLAGGED(victim->in_room,ROOM_ARENA) || ROOM_FLAGGED(victim->in_room,ROOM_NOSUMMON))
    {
        send_to_char("You can't activate the portal on the other side.\r\n",ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room,ROOM_NOSUMMON) || ROOM_FLAGGED(ch->in_room,ROOM_NORECALL))
    {
        send_to_char( "For some strange reason... nothing happens.\r\n", ch );

        return;
    }
    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM) || ROOM_FLAGGED(victim->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }


    if (GET_LEVEL(ch) < GET_LEVEL(victim)) {
        send_to_char(SUMMON_FAIL2, ch);
        return;
    }

    if (MOB_FLAGGED(victim, MOB_NOSUMMON)) {
        send_to_char(SUMMON_FAIL, ch);
        return;
    }

    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
        act("As the words escape your lips and a rift travels\r\n"
            "through time and space toward $N, you realize that $E is\r\n"
            "aggressive and might harm you.",
            FALSE, ch, 0, victim, TO_CHAR);
        //        return;
    }
    if (!IS_NPC(victim) && (!PRF_FLAGGED(victim, PRF_SUMMONABLE) && !CAN_MURDER(ch, victim))) {
        sprintf(buf, "%s just tried to use the dimension walk to you.\r\n"
                "Type NOSUMMON to allow other players to do that.\r\n",
                GET_NAME(ch));
        send_to_char(buf, victim);

        sprintf(buf, "You failed because %s has summon protection on.\r\n",
                GET_NAME(victim));
        send_to_char(buf, ch);

        sprintf(buf, "%s failed relocating to %s at %s.",
                GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        return;
    }
    if (!ROOM_FLAGGED(victim->in_room, ROOM_ARENA)) {
        if (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL, ch)) {
            send_to_char(SUMMON_FAIL, ch);
            return;
        }
        if (!IS_NPC(victim) && CAN_MURDER(ch, victim) && mag_savingthrow(victim, SAVING_SPELL, ch)) {
            send_to_char(SUMMON_FAIL, ch);
            return;
        }
        act("$n opens a portal and steps through it.", TRUE, ch, 0, 0, TO_ROOM);
        act("You open a portal and step through.\r\n", FALSE, ch, 0, 0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, victim->in_room);
        act("The portal opens and $n steps out.", TRUE, ch, 0, 0, TO_ROOM);


        look_at_room(ch, 0);
        if (MOB_FLAGGED(victim, MOB_AGGRESSIVE) )
            hit(victim,ch, TYPE_UNDEFINED);
    } else
        send_to_char("Apply to arena if you want to go there!", ch);
}

/*
ASPELL(spell_visitation)
{
    if (ch == NULL || victim == NULL)
        return;

    if (ch == victim)
        return;

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) || ROOM_FLAGGED(ch->in_room, ROOM_PRIVATE)) {
        send_to_char("You can not do that here!\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(victim->in_room,ROOM_PRIVATE) || ROOM_FLAGGED(victim->in_room,ROOM_TUNNEL))
    {
        send_to_char("There is not enough room there to open a portal.\r\n",ch);
        return;
    }

    if (ROOM_FLAGGED(victim->in_room,ROOM_PEACEFUL) || ROOM_FLAGGED(victim->in_room,ROOM_NOMAGIC))
    {
        send_to_char("You can't activate the portal on the other side.\r\n",ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM) || ROOM_FLAGGED(victim->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }


    if (GET_LEVEL(ch) < GET_LEVEL(victim)) {
        send_to_char(SUMMON2_FAIL, ch);
        return;
    }
    if (GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, level + 3)) {
        send_to_char(SUMMON_FAIL, ch);
        return;
    }
    if (MOB_FLAGGED(victim, MOB_NOSUMMON)) {
        send_to_char(SUMMON_FAIL, ch);
        return;
    }
    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
        act("As the words escape your lips and a rift travels\r\n"
            "through time and space toward $N, you realize that $E is\r\n"
            "aggressive and might harm you.",
            FALSE, ch, 0, victim, TO_CHAR);
//        return;
    }
    if (!IS_NPC(victim) && (!PRF_FLAGGED(victim, PRF_SUMMONABLE) && !CAN_MURDER(ch, victim))) {
        sprintf(buf, "%s just tried to travel to you.\r\n"
                "Type NOSUMMON to allow other players to travel to you.\r\n",
                GET_NAME(ch));
        send_to_char(buf, victim);

        sprintf(buf, "You failed because %s has summon protection on.\r\n",
                GET_NAME(victim));
        send_to_char(buf, ch);

        sprintf(buf, "%s failed visiting to %s at %s.",
                GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        return;
    }
    if (!ROOM_FLAGGED(victim->in_room, ROOM_ARENA)) {
        if (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL, ch)) {
            send_to_char(SUMMON_FAIL, ch);
            return;
        }
        if (!IS_NPC(victim) && CAN_MURDER(ch, victim) && mag_savingthrow(victim, SAVING_SPELL, ch)) {
            send_to_char(SUMMON_FAIL, ch);
            return;
        }
        SET_BIT(AFF_FLAGS(ch), AFF_VISITING);
        ch->howlong = MAX(4, GET_LEVEL(ch) / 8);
        ch->wasbefore = ch->in_room;

        act("$n opens a portal and steps through it.", TRUE, ch, 0, 0, TO_ROOM);
        act("You open a portal and step through.\r\n", FALSE, ch, 0, 0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, victim->in_room);
        act("The portal opens and $n steps out.", TRUE, ch, 0, 0, TO_ROOM);
        look_at_room(ch, 0);
            if (MOB_FLAGGED(victim, MOB_AGGRESSIVE) )
        hit(victim,ch, TYPE_UNDEFINED);
    } else
        send_to_char("Apply to arena if you want to go there!", ch);
}
*/

ASPELL(spell_charge)
{

    if (!ch || !victim)
        return;
    if (victim) {

        if (!CAN_MURDER(ch, victim)) {
            send_to_char("The Gods protect your victim.\r\n", ch);
            return;
        }
        if (GET_MANA(ch)>50)
        {
            GET_MANA(ch)/=2;
            GET_HIT(victim)=MAX(GET_MAX_HIT(victim)/8, GET_HIT(victim)-GET_MANA(ch)*GET_LEVEL(ch)/25);
            act("$n uses $s magical powers to CHARGE through $N!", FALSE, ch, 0, victim, TO_NOTVICT);
            act("You let your magical powers CHARGE through $N!", FALSE, ch, 0, victim, TO_CHAR);
            act("$n uses $s magical powers to CHARGE through you!", FALSE, ch, 0, victim, TO_VICT);
            hit(ch, victim,TYPE_UNDEFINED);
        }
        else
            send_to_char("You don't have enough mana!\r\n",ch);
    }
}

ASPELL(spell_disintigrate)
{
    struct obj_data *foolz_objs, *head;

    int             save,
    i;

    if (ch == NULL)
        return;
    if (not_in_arena(ch))
        return;
    if (obj) {
        /* Used on my mud if (GET_OBJ_EXTRA(obj) == ITEM_IMMORT &&
         * GET_LEVEL(ch) < LVL_IMMORT) { send_to_char("Your mortal magic
         * fails.\r\n", ch); return; } */
        switch (GET_OBJ_TYPE(obj)) {
        case ITEM_LIGHT:
            save = 19;
            break;
        case ITEM_SCROLL:
            save = 20;
            break;
        case ITEM_STAFF:
            /* case ITEM_ROD: */
        case ITEM_WAND:
            save = 19;
            break;
        case ITEM_WEAPON:
            save = 15;
            break;
        case ITEM_MISSILE:
            save = 20;
            break;
        case ITEM_ARMOR:
            save = 16;
            break;
        case ITEM_WORN:
            save = 18;
            break;
            /* case ITEM_SPELLBOOK: save = 15; break; case ITEM_PORTAL: save
             * = 13; break; */
        default:
            save = 18;
            break;
        }
        /* Save modified by affect on weapons..this is kinda based on +5 or
         * so being high */
        if (GET_OBJ_EXTRA(obj) == ITEM_MAGIC && GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
            for (i = 0; i < MAX_OBJ_AFFECT; i++) {
                if (obj->affected[i].location == APPLY_DAMROLL)
                    save -= obj->affected[i].modifier;
            }
        }

        /* A bonus for ac affecting items also */
        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].location == APPLY_AC)
                save -= obj->affected[i].modifier / 10;
        }
        if (number(1, 20) < save) {
            act("$n claps his hands and disintegrates $p!", FALSE, ch, obj, 0, TO_NOTVICT);
            act("You clap your hands and disintegrate $p!", FALSE, ch, obj, 0, TO_CHAR);
            act("$p vanishes in a puff of smoke!", FALSE, ch, obj, 0, TO_CHAR);
            act("$p vanishes in a puff of smoke!", FALSE, ch, obj, 0, TO_ROOM);
            extract_obj(obj);
            return;
        } else {
            act("You fail.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        }
    }
    if (victim) {

        if (!CAN_MURDER(ch, victim)) {
            send_to_char("The Gods protect your victim.\r\n", ch);
            return;
        }

        if (GET_LEVEL(ch)>GET_LEVEL(victim) && !mag_savingthrow(victim, SAVING_SPELL, ch) && number(0, 225)<((GET_LEVEL(ch)-GET_LEVEL(victim))*(GET_LEVEL(ch)-GET_LEVEL(victim))))
        {
            act("You exclaim loudly, 'Ashes to ashes, dust to dust!'", FALSE, ch, 0, 0, TO_CHAR);
            act("$n exclaims loudly, 'Ashes to ashes, dust to dust!'", FALSE, ch, 0, 0, TO_ROOM);
            damage(ch, victim, -334, SPELL_DISINTIGRATE, 0);
        }
        else
            damage(ch, victim, dice(6, GET_LEVEL(ch)*3), SPELL_DISINTIGRATE, 0);

        /*        act("$n conjures beam of energy and uses it to DISINTIGRATE $N!", FALSE, ch, 0, victim, TO_NOTVICT);
                act("You conjure beam of energy and use it to DISINTIGRATE $N!", FALSE, ch, 0, victim, TO_CHAR);
                act("$n conjures a beam of energy and uses it to DISINTIGRATE you!", FALSE, ch, 0, victim, TO_VICT);

                if ((GET_LEVEL(ch) - GET_LEVEL(victim)) > 10) {
                    if (!mag_savingthrow(victim, SAVING_SPELL, ch)) {
                        GET_HIT(victim) = -100;
                        act("$N suffers UNDESCRIBABLE pain!", FALSE, ch, 0, victim, TO_ROOM);
                        act("You feel mighty as $N twinches in UNDISCRIBABLE pain.", FALSE, ch, 0, victim, TO_CHAR);
                    } else
                        GET_HIT(victim) = GET_HIT(victim)*2/3;

                } else
                    GET_HIT(victim) -= dice(GET_LEVEL(ch)*3, 4);*/
    }
}


ASPELL(spell_fear)
{
    struct char_data *target = (struct char_data *) victim;
    struct char_data *next_target;
    int             rooms_to_flee = 0;

    if (ch == NULL)
        return;

    send_to_char("You radiate an aura of fear into the room!\r\n", ch);
    act("$n is surrounded by an aura of fear!", TRUE, ch, 0, 0, TO_ROOM);

    for (target = world[ch->in_room].people; target; target = next_target) {
        next_target = target->next_in_room;

        if (target == NULL)
            return;

        if (target == ch)
            continue;

        if (!CAN_MURDER(ch, target) || !CAN_SEE(ch, target))
            continue;

        if (mag_savingthrow(target, SAVING_PARA, ch)) {
            if (IS_NPC(target) && GET_LEVEL(target) >= GET_LEVEL(ch))
                hit(target, ch, TYPE_UNDEFINED);
        } else
            do_flee(target, "", 0, 99);
    }
}

ASPELL(spell_improve_stat)
{
    struct affected_type af;
    char           *arc_name = tar_str;
    int             i = 0,
                        max = 25;
    struct char_data *target;
    struct char_data *next_target;
    if (ch == NULL)
        return;
    if (arc_name == NULL) {
        send_to_char("What stat do you want to improve?\r\n", ch);
        return;
    }
    if (PRF2_FLAGGED(ch, PRF2_S1)) {
        send_to_char("You have allready used this powerful spell once.\r\n", ch);
        return;
    }
    for (target = world[ch->in_room].people; target; target = next_target) {
        next_target = target->next_in_room;

        if (target == NULL) {
            send_to_char("There is nobody here you can improve.\r\n", ch);
            return;
        }
        if (target == ch)
            continue;

        if (IS_NPC(target))
            continue;

        if (!str_cmp(arc_name, "str")) {
            GET_STRR(target) = GET_STRR(target) + 1;
            if (GET_STR(target) == 26)
                GET_STRR(target) = GET_STRR(target) - 1;
        } else if (!str_cmp(arc_name, "con")) {
            GET_CONR(target) = GET_CONR(target) + 1;
            if (GET_CON(target) == 26)
                GET_CONR(target) = GET_CONR(target) - 1;
        } else if (!str_cmp(arc_name, "will")) {
            GET_WISR(target) = GET_WISR(target) + 1;
            if (GET_WIS(target) == 26)
                GET_WISR(target) = GET_WISR(target) - 1;
        } else if (!str_cmp(arc_name, "int")) {
            GET_INTR(target) = GET_INTR(target) + 1;
            if (GET_INT(target) == 26)
                GET_INTR(target) = GET_INTR(target) - 1;
        } else if (!str_cmp(arc_name, "dex")) {
            GET_DEXR(target) = GET_DEXR(target) + 1;
            if (GET_DEX(target) == 26)
                GET_DEXR(target) = GET_DEXR(target) - 1;
        } else if (!str_cmp(arc_name, "cha")) {
            GET_CHAR(target) = GET_CHAR(target) + 1;
            if (GET_CHA(target) == 26)
                GET_CHAR(target) = GET_CHAR(target) - 1;
        } else {
            send_to_char("What stat do you want to improve?\r\n", ch);
            return;
        }

        act("$n begins to shiver as he grabs $N's hands.", FALSE, ch, 0, target, TO_NOTVICT);
        act("You grab $N's hands and begin to shiver as you call upon the Gods.", FALSE, ch, 0, target, TO_CHAR);
        act("$n grabs your hands and begin to shiver as you close your eyes.", FALSE, ch, 0, target, TO_VICT);
        act("The strike of lightning hits both $n and $N leaving them paralyzed\r\nfor a moment.", FALSE, ch, 0, target, TO_NOTVICT);
        act("Strike of lightning hits you with full impact!", FALSE, ch, 0, target, TO_CHAR);
        act("Strike of lightning hits you with full impact!", FALSE, ch, 0, target, TO_VICT);
        act("They fall on the ground, totaly exhausted.", FALSE, ch, 0, target, TO_NOTVICT);
        act("Uuuhhhr... pain...", FALSE, ch, 0, target, TO_CHAR);
        act("You weaken and fall on the ground, totaly exhausted.", FALSE, ch, 0, target, TO_CHAR);
        act("Uuuhhhr... pain...", FALSE, ch, 0, target, TO_VICT);
        act("You weaken and fall on the ground, totaly exhausted.", FALSE, ch, 0, target, TO_VICT);

        //        GET_MANA(ch) = 1;
        GET_HIT(ch) = 1;
        //      GET_MOVE(ch) = 1;
        af.type = 0;
        af.bitvector = 0;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.type = SPELL_IMPROVE_STAT;
        af.duration = 5;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_BLIND;
        affect_to_char(ch, &af);
        af.type = SPELL_IMPROVE_STAT;
        af.duration = 5;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_SLEEP;
        affect_to_char(ch, &af);
        GET_POS(ch) = POS_SLEEPING;
        SET_BIT(PRF2_FLAGS(ch), PRF2_S1);
        save_char(ch, ch->in_room);
        //GET_MANA(target) = 1;
        GET_HIT(target) = 1;
        //GET_MOVE(target) = 1;
        af.type = 0;
        af.bitvector = 0;
        af.bitvector2 = 0;
        af.bitvector3 = 0;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.type = SPELL_IMPROVE_STAT;
        af.duration = 5;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_BLIND;
        affect_to_char(target, &af);
        af.type = SPELL_IMPROVE_STAT;
        af.duration = 5;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_SLEEP;
        affect_to_char(target, &af);
        GET_POS(target) = POS_SLEEPING;
        save_char(target, ch->in_room);
        return;
    }
    send_to_char("There is nobody here you can improve.\r\n", ch);

}


ASPELL(spell_arcane)
{

    const char     *arc[5][2][11] = {
                                        {                       /* Ten Thousands */
                                            {"ba", "bo", "di", "fo", "gu", "hy", "ja", "ke", "li", "mu", "\n"},
                                            {"ny", "cu", "qe", "ri", "so", "tu", "vy", "wa", "xe", "zi", "\n"},
                                        },
                                        {                       /* Thousands */
                                            {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "\n"},
                                            {"n", "p", "q", "r", "s", "t", "v", "w", "x", "z", "\n"},
                                        },
                                        {                       /* Hundreds */
                                            {"ab", "ae", "ai", "ao", "au", "ay", "ea", "ec", "ei", "eo", "\n"},
                                            {"oi", "of", "ou", "oy", "ac", "ec", "ed", "ug", "uz", "uq", "\n"},
                                        },
                                        {                       /* Tens */
                                            {"z", "c", "x", "f", "w", "h", "v", "k", "s", "m", "\n"},
                                            {"n", "b", "q", "d", "k", "t", "j", "g", "r", "p", "\n"},
                                        },
                                        {                       /* Ones */
                                            {"aq", "as", "av", "au", "an", "ef", "eg", "et", "en", "ep", "\n"},
                                            {"re", "ru", "us", "ut", "at", "ur", "yr", "yr", "er", "ea", "\n"},
                                        }
                                    };

    int             roomno,
    tmp,
    tenthousands,
    thousands,
    hundreds,
    tens,
    ones;

    if (ch == NULL)
        return;
    if (not_in_arena(ch))
        return;
    if (ROOM_FLAGGED(ch->in_room,ROOM_PRIVATE))
    {
        send_to_char("There seem to be some disturbance in the eter here.\r\n",ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }


    roomno = world[ch->in_room].number;

    ones = roomno % 10;         /* 12345 = 5    */
    tmp = roomno / 10;          /* 12345 = 1234 */
    tens = tmp % 10;            /* 1234  = 4    */
    tmp = tmp / 10;             /* 1234  = 123  */
    hundreds = tmp % 10;        /* 123   = 3    */
    tmp = tmp / 10;             /* 123   = 12   */
    thousands = tmp % 10;       /* 12    = 2    */
    if (tmp > 0)
        tenthousands = tmp / 10;/* 12    = 1    */
    else
        tenthousands = 0;

    sprintf(buf, "You feel a strange sensation in your body, as the wheel of time stops.\r\n");
    send_to_char(buf, ch);
    sprintf(buf, "Strange sounds can be heard through the mists of time.\r\n");
    send_to_char(buf, ch);
    sprintf(buf, "An old man tells you : 'The Arcane name for this place is %s%s%s%s%s'.\r\n",
            arc[0][number(0, 1)][tenthousands], arc[1][number(0, 1)][thousands],
            arc[2][number(0, 1)][hundreds], arc[3][number(0, 1)][tens],
            arc[4][number(0, 1)][ones]);
    send_to_char(buf, ch);

}

ASPELL(spell_portal)
{

    static char    *arc[5][2][11] = {
                                        {
                                            {"ba", "bo", "di", "fo", "gu", "hy", "ja", "ke", "li", "mu", "\n"},
                                            {"ny", "cu", "qe", "ri", "so", "tu", "vy", "wa", "xe", "zi", "\n"},
                                        },
                                        {
                                            {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "\n"},
                                            {"n", "p", "q", "r", "s", "t", "v", "w", "x", "z", "\n"},
                                        },
                                        {
                                            {"ab", "ae", "ai", "ao", "au", "ay", "ea", "ec", "ei", "eo", "\n"},
                                            {"oi", "of", "ou", "oy", "ac", "ec", "ed", "ug", "uz", "uq", "\n"},
                                        },
                                        {
                                            {"z", "c", "x", "f", "w", "h", "v", "k", "s", "m", "\n"},
                                            {"n", "b", "q", "d", "k", "t", "j", "g", "r", "p", "\n"},
                                        },
                                        {
                                            {"aq", "as", "av", "au", "an", "ef", "eg", "et", "en", "ep", "\n"},
                                            {"re", "ru", "us", "ut", "at", "ur", "yr", "yr", "er", "ea", "\n"},
                                        }
                                    };

    char           *arc_name = tar_str;
    char           *tmp = 0;
    int             arc_found = 0;
    int             nr,
    icount;
    sh_int          location;

    int             dicechance = 0;
    extern int      top_of_world;

    location = 0;
    if (not_in_arena(ch))
        return;

    if (ch == NULL)
        return;

    /* It takes *SOME* kind of Intelligence to portal!! */
    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room,ROOM_NORECALL))
    {
        send_to_char( "For some strange reason... nothing happens.\r\n", ch );

        return;
    }

    if (GET_INT(ch) < 7) {
        send_to_char("Sorry, you're too stupid to portal anywhere", ch);
        return;
    }
    if (arc_name == NULL) {
        send_to_char("Portal to where?", ch);
        return;
    }
    for (nr = 0; nr < 5; nr++) {
        arc_found = FALSE;
        icount = 0;
        for (icount = 0; icount < 9; icount++) {
            if (!arc_found) {
                tmp = arc[nr][0][icount];
                if (!strncmp(arc_name, tmp, strlen(tmp))) {
                    arc_found = TRUE;
                    arc_name = arc_name + strlen(tmp);
                    location = location * 10 + icount;
                }
                if (!arc_found) {
                    tmp = arc[nr][1][icount];
                    if (!strncmp(arc_name, tmp, strlen(tmp))) {
                        arc_found = TRUE;
                        arc_name = arc_name + strlen(tmp);
                        location = location * 10 + icount;
                    }
                }
            }
        }
        /* here we have tried all 20 chars, and that means.. If still
         * !arc_found = total fail */
        /* They still can be saved by their Intelligence, the higher
         * intelligence, then less */
        if (!arc_found) {
            dicechance = GET_INT(ch) - number((GET_INT(ch) - 7), GET_INT(ch));
            switch (dicechance) {
            case 0:             /* Full Score Nothing happens! */
                send_to_char("You feel strangely relieved, like something terrible just was avoided.\r\n", ch);
                return;
            case 1:             /* Minor Mishap! Give a lame text or push the
                                     * character somewhere */
                send_to_char("Your concentration is lost.\r\n", ch);
                return;
            case 7:             /* TOTAL Fail!! Let's send the bugger off to
                                     * somewhere <g> */
                send_to_char("You feel a disturbance in the magic aura, and your portal fails.\r\n", ch);
                do {
                    location = number(1, top_of_world);
                } while (ROOM_FLAGGED(location, ROOM_TUNNEL | ROOM_PRIVATE | ROOM_DEATH | ROOM_GODROOM | ROOM_ARENA | ROOM_NOSUMMON));
                act("$n fails a portal terribly, and is sent off to somewhere.", FALSE, ch, 0, 0, TO_ROOM);
                char_from_room(ch);
                char_to_room(ch, location);
                act("The portal opens, and $n tumbles out of it, clearly confused.", FALSE, ch, 0, 0, TO_ROOM);
                look_at_room(ch, 0);
                return;
            default:            /* Hum, just do something text based */
                send_to_char("Your portal fails.\r\n", ch);
                return;
            }
        }
    }                           /* End decode roomno */
    /* At this point all have been successful! now, let's make magic */
    /* The portal spell takes a persons int and wisdom, and uses them for a saving throw */
    /* Beware it *IS* possible to die from this spell!!! (should happen once out of 1000 or so */
    /*

      if (ROOM_FLAGGED(location, ROOM_TUNNEL | ROOM_PRIVATE | ROOM_DEATH | ROOM_GODROOM | ROOM_ARENA)) {
       act("You feel a disturbance in the magic aura, and your portal fails.", FALSE, 0, 0, 0, TO_CHAR);
       do {
       location = number(0, top_of_world);
       } while (ROOM_FLAGGED(location, ROOM_PRIVATE | ROOM_DEATH | ROOM_GODROOM | ROOM_ARENA));
       if (location!=NOWHERE) {
       act("$n fails a portal terribly, and is sent off to somewhere.", FALSE, ch, 0, 0, TO_ROOM);
       char_from_room(ch);
       char_to_room(ch, location);
       act("The portal opens, and $n tumbles into the room, clearly confused.", FALSE, ch, 0, 0, TO_ROOM);
       look_at_room(ch, 0);
       }
       return;
     }
    */
    act("You feel a strange shimmerning in the magic aura as you open the portal.", FALSE, 0, 0, 0, TO_CHAR);
    act("$n opens a portal, steps into it and vanishes.", FALSE, ch, 0, 0, TO_ROOM);
    if (location<=0)
        location=3001;
    char_from_room(ch);
    char_to_room(ch, real_room(location));

    act("The portal opens, and $n steps out of it.", FALSE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);
}

ASPELL(spell_create_spring)
{
    struct obj_data *spring = '\0';
    int             spring_num = 97;
    *buf = '\0';

    if (GET_SKILL(ch, SPELL_CREATE_SPRING) == 0) {
        send_to_char("That spell is unfamiliar to you.\r\n", ch);
        return;
    }
    if ((SECT(ch->in_room) == SECT_INSIDE) || (SECT(ch->in_room) == SECT_CITY)) {
        send_to_char("You cannot create a spring here!\r\n", ch);
        return;
    }
    if ((SECT(ch->in_room) == SECT_WATER_SWIM) || (SECT(ch->in_room) == SECT_WATER_NOSWIM)) {
        send_to_char("How can you create a spring in water?\r\n", ch);
        return;
    }
    if (SECT(ch->in_room) == SECT_UNDERWATER) {
        send_to_char("You cannot create a spring underwater!\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
        send_to_char("An unforseen force prevents you from casting spells.\r\n", ch);
        return;
    }
    spring = read_object(spring_num, VIRTUAL, 0, 0);
    GET_OBJ_TIMER(spring) = MAX(2, lvD5(ch));   /* you may want to reflect
                                                 * exp or level on */
    /* this value here, maybe 2 + GET_LEVEL(ch) */
    obj_to_room(spring, ch->in_room);
    sprintf(buf, "You rub your hands together and a small spring pops out!\r\n");
    /* You may want a message sent to the room as well */
    send_to_char(buf, ch);
    act("$n intones mystical phrases and a small spring pops out!", FALSE, ch, 0, 0, TO_ROOM);
    return;
}

ASPELL(spell_control_weather)
{
    char           *arc_name = tar_str;
    if (ch == NULL)
        return;
    if (arc_name == NULL) {
        send_to_char("Do you want it to get better or worse?\r\n", ch);
        return;
    }
    if (!str_cmp(arc_name, "better"))
        weather_info.change += dice(level / 3, 4);
    else if (!str_cmp(arc_name, "worse"))
        weather_info.change -= dice(level / 3, 4);
    else {
        send_to_char("Do you want it to get better or worse?\r\n", ch);
        return;
    }

    send_to_char("You rise your hands towards the sky and loudly intone mystical phrases.\r\n", ch);
    act("$n looks up the sky and intones mystical phrases.", FALSE, ch, 0, 0, TO_ROOM);
    weather_change();
    WAIT_STATE(ch, 2 RL_SEC);
    return;

}
#define PORTAL_OBJ  20   /* the vnum of the portal object */

ASPELL(spell_portal2)
{
    struct obj_data *portal, *tportal;
    struct extra_descr_data *new_descr, *new_tdescr;
    int to_room;
    if (not_in_arena(ch))
        return;

    if (ch == NULL || victim == NULL) return;
    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) ||
            ROOM_FLAGGED(victim->in_room, ROOM_ARENA)) {
        send_to_char("First let the games finish!", ch);
        return;
    }

    if (ROOM_FLAGGED(victim->in_room,ROOM_PEACEFUL) ||
            ROOM_FLAGGED(victim->in_room,ROOM_NOMAGIC) || ROOM_FLAGGED(victim->in_room,ROOM_NOSUMMON))
    {
        send_to_char("You can't activate the portal on the other side.\r\n",ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room,ROOM_NOSUMMON) || ROOM_FLAGGED(ch->in_room,ROOM_NORECALL))
    {
        send_to_char( "For some strange reason... nothing happens.\r\n", ch );

        return;
    }

    if (ROOM_FLAGGED(ch->in_room,ROOM_PRIVATE) ||
            ROOM_FLAGGED(ch->in_room,ROOM_TUNNEL))
    {
        send_to_char("There is not enough room here to open a portal.\r\n",ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room,ROOM_GODROOM) ||
            ROOM_FLAGGED(victim->in_room,ROOM_GODROOM))
    {
        send_to_char("You feel a strange buzzing in your head, 'Don't do this!'\r\n",ch);
        return;
    }


    if (FIGHTING(victim))
    {
        send_to_char("You can't seem to get a precise fix on your victim's location.\r\n",ch);
        return;
    }

    if (!IS_NPC(victim) && (!PRF_FLAGGED(victim, PRF_SUMMONABLE))) {
        sprintf(buf, "Someone just tried to open a portal to you.\r\n"
                "%s failed because you have summon protection on.\r\n"
                "Type NOSUMMON to allow other players to do that.\r\n",
                (ch->player.sex == SEX_MALE) ? "He" : "She");
        send_to_char(buf, victim);

        sprintf(buf, "You failed because %s has summon protection on.\r\n",
                GET_NAME(victim));
        send_to_char(buf, ch);

        sprintf(buf, "SUMMON: %s failed portal %s to %s.",
                GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        return;
    }

    if (MOB_FLAGGED(victim, MOB_NOSUMMON) ||
            (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL, ch))) {
        send_to_char(SUMMON_FAIL, ch);
        return;
    }


    /* create the portal */
    portal = read_object(PORTAL_OBJ, VIRTUAL,0, 0);
    if (!IS_NPC(victim))
        to_room=victim->in_room;
    else
        do {
            to_room = number(0, top_of_world);
        } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_ARENA) || world[to_room].zone!=world[victim->in_room].zone);


    GET_OBJ_VAL(portal, 0) = to_room;
    GET_OBJ_TIMER(portal) = (int) (GET_LEVEL(ch) / 10);
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup("portal gate gateway");
    sprintf(buf, "Through the mists of the portal, you can faintly see %s.", world[to_room].name);
    new_descr->description = str_dup(buf);
    new_descr->next = portal->ex_description;
    portal->ex_description = new_descr;        
    global_no_timer=1;
    obj_to_room(portal, ch->in_room);
    global_no_timer=0;
    act("$n conjures a magical portal out of thin air.",
        TRUE, ch, 0, 0, TO_ROOM);
    act("You conjure a magic portal out of thin air.",
        TRUE, ch, 0, 0, TO_CHAR);
    /* create the portal at the other end */
    tportal = read_object(PORTAL_OBJ, VIRTUAL,0, 0);
    GET_OBJ_VAL(tportal, 0) = ch->in_room;
    GET_OBJ_TIMER(tportal) = (int) (GET_LEVEL(ch) / 10);
    CREATE(new_tdescr, struct extra_descr_data, 1);
    new_tdescr->keyword = str_dup("portal gate gateway");
    sprintf(buf, "Through the mists of the portal, you can faintly see %s.", world[ch->in_room].name);
    new_tdescr->description = str_dup(buf);
    new_tdescr->next = tportal->ex_description;
    tportal->ex_description = new_tdescr;
    obj_to_room(tportal, victim->in_room);
    act("A glowing portal appears out of thin air.",
        TRUE, victim, 0, 0, TO_ROOM);
    act("A glowing portal opens here for you.",
        TRUE, victim, 0, 0, TO_CHAR);
}


ASPELL(spell_fearie_fog)
{
    struct char_data *target = (struct char_data *) victim;
    struct char_data *next_target;
    struct obj_data *cont;
    int             rooms_to_flee = 0;

    if (ch == NULL)
        return;

    send_to_char("A cloud of faerie fog slowly spreads around.\r\n", ch);
    act("$n makes a move and a cloud of faerie fog slowly spreads around.", TRUE, ch, 0, 0, TO_ROOM);

    for (target = world[ch->in_room].people; target; target = next_target) {
        next_target = target->next_in_room;

        if (target == NULL)
            return;

        if (target == ch)
            continue;

        if (GET_LEVEL(target) >= GET_LEVEL(ch))
            continue;

        if (AFF_FLAGGED(target, AFF_INVISIBLE)) {
            if (affected_by_spell(target, SPELL_INVISIBLE))
            {
                affect_from_char(target, SPELL_INVISIBLE);
                leech_from_char(target, SPELL_INVISIBLE);
            }
            REMOVE_BIT(AFF_FLAGS(target), AFF_INVISIBLE);

            act("$n slowly fades into existence.", FALSE, target, 0, 0, TO_ROOM);
            send_to_char("You slowly fade into existance.\r\n", target);
        }
    }
}


ASPELL(spell_eagle_eye)
{

    if (ch == NULL || victim == NULL) return;
    if (GET_MOVE(ch)<100)
    {
        send_to_char("You are too tired to try this now.\r\n", ch);
        return;
    }
    if (victim->in_room != NOWHERE && !mag_savingthrow(victim, SAVING_SPELL, ch) && !AFF3_FLAGGED(victim, AFF3_SHROUD))
        sprintf(buf, "You manage to relax into comfortable position and concentrate.\r\nEye of the eagle tells you that %s is somewhere in %s.\r\n", GET_NAME(victim),
                zone_table[world[victim->in_room].zone].name);
    else
        sprintf(buf, SUMMON_FAIL1);

    send_to_char(buf, ch);

    GET_MOVE(ch)-=100;
}


ASPELL(spell_retrieve_corpse)
{
    struct obj_data *i, *last=NULL;
    char            name[MAX_INPUT_LENGTH];
    int             j;

    for (i = object_list; i ; i = i->next) {
        if (GET_OBJ_RENT(i)!=9999 || strcmp(GET_NAME(ch), i->attack_verb))
            continue;

        if (i->carried_by)
            sprintf(buf, "%s is being carried by %s!\r\n",
                    i->short_description, PERS(i->carried_by, ch));
        else if (i->in_obj)
            sprintf(buf, "%s is not on the ground.\r\n", i->short_description);
        else if (i->in_room==NOWHERE)
            sprintf(buf, "%s's location is uncertain.\r\n",
                    i->short_description);
        else
        {

            obj_from_room(i);
            obj_to_room(i, ch->in_room);
            act("$p mysteriously materialises in front of you.", FALSE, ch, i, NULL, TO_CHAR);
            act("$p mysteriously materialises here.", FALSE, ch, i, NULL, TO_ROOM);
            return;
        }

        CAP(buf);
        send_to_char(buf, ch);
        return;
    }

    send_to_char("You couldn't find it!\r\n", ch);
}

ASPELL(spell_recharge)
{
    int restored_charges = 0, explode = 0;

    if (ch == NULL || obj == NULL)
        return;


    if (GET_OBJ_TYPE(obj) == ITEM_WAND && GET_OBJ_VNUM(obj)!=5) {
        if (GET_OBJ_VAL(obj, 2) <1)
        {
            send_to_char("There is no magic left in the wand.\r\n", ch);
            return;
        }
        if (GET_OBJ_VAL(obj, 2) < GET_OBJ_VAL(obj, 1)) {
            send_to_char("You attempt to recharge the wand.\r\n", ch);
            restored_charges = number(1, 5);
            GET_OBJ_VAL(obj, 2) += restored_charges;
            if (GET_OBJ_VAL(obj, 2) > GET_OBJ_VAL(obj, 1)) {
                send_to_char("The wand is overcharged and explodes!\r\n", ch);
                sprintf(buf, "%s overcharges %s and it explodes!\r\n",
                        GET_NAME(ch), obj->name);
                act(buf, TRUE, ch, 0, 0, TO_ROOM);
                explode = dice(GET_OBJ_VAL(obj, 2), 2);
                GET_HIT(ch) -= explode;
                update_pos(ch);
                extract_obj(obj);
                check_kill(ch, "recharging the wand");
                return;
            }
            else {
                sprintf(buf, "You restore %d charges to the wand.\r\n", restored_charges);
                send_to_char(buf, ch);
                return;
            }
        }
        else {
            send_to_char("That item is already at full charges!\r\n", ch);
            return;
        }
    }
    else if (GET_OBJ_TYPE(obj) == ITEM_STAFF) {
        if (GET_OBJ_VAL(obj, 2) <1)
        {
            send_to_char("There is no magic left in the staff.\r\n", ch);
            return;
        }
        if (GET_OBJ_VAL(obj, 2) < GET_OBJ_VAL(obj, 1)) {
            send_to_char("You attempt to recharge the staff.\r\n", ch);
            restored_charges = number(1, 3);
            GET_OBJ_VAL(obj, 2) += restored_charges;
            if (GET_OBJ_VAL(obj, 2) > GET_OBJ_VAL(obj, 1)) {
                send_to_char("The staff is overcharged and explodes!\r\n", ch);
                sprintf(buf, "%s overcharges %s and it explodes!\r\n",
                        GET_NAME(ch), obj->name);
                act(buf, TRUE, ch, 0, 0, TO_ROOM);
                explode = dice(GET_OBJ_VAL(obj, 2), 3);
                GET_HIT(ch) -= explode;
                update_pos(ch);
                extract_obj(obj);
                check_kill(ch, "recharging the staff");
                return;
            }
            else {
                sprintf(buf, "You restore %d charges to the staff.\r\n", restored_charges);
                send_to_char(buf, ch);
                return;
            }
        }
        else {
            send_to_char("That item is already at full charges!\r\n", ch);
            return;
        }
    }
    else
        send_to_char("You can't recharge that.\r\n", ch);
}


ASPELL(spell_visions)
{
    int             wasin, j;
    struct obj_data *tmp_obj;

    if (ch == NULL || victim == NULL)
        return;

    if (AFF3_FLAGGED(victim, AFF3_SHROUD) || (GET_LEVEL(ch)<GET_LEVEL(victim)-5) || (ROOM_FLAGGED(victim->in_room, ROOM_PRIVATE | ROOM_DEATH))) {
        send_to_char(SUMMON_FAIL1, ch);
        return;
    }
    wasin = ch->in_room;
    SET_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    look_at_room(ch, 1);
    char_from_room(ch);    
    char_to_room(ch, wasin);
    REMOVE_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
    
   
        act("\r\n&cEquipment:&0", FALSE, victim, 0, ch, TO_VICT);
         for ( j = 0; j < NUM_WEARS; j++)
            if (GET_EQ(victim, j) && CAN_SEE_OBJ(victim, GET_EQ(victim, j))) {
                send_to_char(where[j], ch);
                show_obj_to_char(GET_EQ(victim, j), ch, 1, 1, 0);
            }
   
        act("\r\n&cInventory:&0", FALSE, victim, 0, ch, TO_VICT);
        for (tmp_obj = victim->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
            if (CAN_SEE_OBJ(victim, tmp_obj))
                show_obj_to_char(tmp_obj, ch, 1, 1, 0);
        
}

