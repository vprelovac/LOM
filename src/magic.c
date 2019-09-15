/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
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

#define SINFO spell_info[spellnum]

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
//extern struct cha_app_type cha_app[];
extern struct int_app_type int_app[];
extern struct index_data *obj_index;

extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
extern sh_int   r_mortal_start_room;
extern int      mini_mud;
extern int      pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern char     weapon_verbs[];
extern int     *max_ac_applys;
extern struct apply_mod_defaults *apmd;
extern char    *spell_wear_off_msg[];

void            clearMemory(struct char_data * ch);
void            act(char *str, int i, struct char_data * c, struct obj_data * o,
                    void *vict_obj, int j);

void            damage(struct char_data * ch, struct char_data * victim,
                       int damage, int weapontype, struct obj_data * obj);

void            weight_change_object(struct obj_data * obj, int weight);
void            add_follower(struct char_data * ch, struct char_data * leader);
int             dice(int number, int size);
extern struct spell_info_type spell_info[];

bool            CAN_MURDER(struct char_data * ch, struct char_data * victim);

struct char_data *read_mobile(int, int, int);
extern CHAR_DATA *supermob;

/*
 * Saving throws for:
 * MCTWBSDWN
 *   PARA, ROD, PETRI, BREATH, SPELL
 *     Levels 0-40
 */
const int       saving_throws1[NUM_CLASSES][2] = {
            {65, 16},                   /* MAGE */
            {68, 25},                   /* CLERIC */
            {70, 30},                   /* THIEF */
            {79, 25},                   /* WARRIOR */
            {70, 22},                   /* BARBARIAN */
            {75, 25},                   /* SAMURAI */
            {66, 20},                   /* DRUID */
            {66, 20},                   /* WIZARD */
            {70, 23},                   /* MONK */
            {75, 20},                   /* AVATAR */
            {75, 20},                   /* NINJA */
            {75, 20},                   /* DUAL */
            {75, 20}                    /* TRIPLE */
        };

const byte      saving_throws[NUM_CLASSES][5][41];

void process_data()
{
    struct descriptor_data *d, *next_d;
    for (d = descriptor_list; d; d = next_d) {
        next_d = d->next;
        /*//      if (FD_ISSET(d->descriptor, &output_set) && *(d->output))*/
        if (*(d->output)) {// && FD_ISSET(d->descriptor, &output_set)) {
            /* Output for this player is ready */

            if (d && process_output(d) < 0)
                close_socket(d);
            else
                d->has_prompt = 1;
        }
    }
}

void            kill_group_spells(struct char_data * victim)
{
    if (AFF2_FLAGGED(victim, AFF2_BATTLECRY))
    {
        affect_from_char(victim, SKILL_BATTLECRY);
        REMOVE_BIT(AFF2_FLAGS(victim), AFF2_BATTLECRY);
        send_to_char("Your battle morale diminishes.\r\n", victim);
    }


    if (AFF3_FLAGGED(victim, AFF3_GR_ARMOR))
    {
        affect_from_char(victim, SPELL_ARMOR);
        leech_from_char(victim, SPELL_ARMOR);
        REMOVE_BIT(AFF3_FLAGS(victim), AFF3_GR_ARMOR);
        if (*spell_wear_off_msg[SPELL_ARMOR]) {
            send_to_char(spell_wear_off_msg[SPELL_ARMOR], victim);
            send_to_char("\r\n", victim);
        }
    }

    if (AFF3_FLAGGED(victim, AFF3_GR_INVIS))
    {
        affect_from_char(victim, SPELL_INVISIBLE);
        leech_from_char(victim, SPELL_INVISIBLE);
        REMOVE_BIT(AFF3_FLAGS(victim), AFF3_GR_INVIS);
        if (*spell_wear_off_msg[SPELL_INVISIBLE]) {
            send_to_char(spell_wear_off_msg[SPELL_INVISIBLE], victim);
            send_to_char("\r\n", victim);
        }

    }

    if (AFF3_FLAGGED(victim, AFF3_GR_FLY))
    {
        affect_from_char(victim, SPELL_FLY);
        leech_from_char(victim, SPELL_FLY);
        REMOVE_BIT(AFF3_FLAGS(victim), AFF3_GR_FLY);
        if (*spell_wear_off_msg[SPELL_FLY]) {
            send_to_char(spell_wear_off_msg[SPELL_FLY], victim);
            send_to_char("\r\n", victim);
        }

    }

    if (AFF3_FLAGGED(victim, AFF3_GR_WATER))
    {
        affect_from_char(victim, SPELL_WATERBREATH);
        leech_from_char(victim, SPELL_WATERBREATH);
        REMOVE_BIT(AFF3_FLAGS(victim), AFF3_GR_WATER);
        if (*spell_wear_off_msg[SPELL_WATERBREATH]) {
            send_to_char(spell_wear_off_msg[SPELL_WATERBREATH], victim);
            send_to_char("\r\n", victim);
        }

    }

    if (AFF3_FLAGGED(victim, AFF3_GR_DEF))
    {
        affect_from_char(victim, SPELL_DEFLECTION);
        leech_from_char(victim, SPELL_DEFLECTION);
        REMOVE_BIT(AFF3_FLAGS(victim), AFF3_GR_DEF);
        if (*spell_wear_off_msg[SPELL_DEFLECTION]) {
            send_to_char(spell_wear_off_msg[SPELL_DEFLECTION], victim);
            send_to_char("\r\n", victim);
        }

    }

    if (AFF3_FLAGGED(victim, AFF3_GR_FIRE))
    {
        affect_from_char(victim, SPELL_FIRE_SHIELD);
        leech_from_char(victim, SPELL_FIRE_SHIELD);
        REMOVE_BIT(AFF3_FLAGS(victim), AFF3_GR_FIRE);
        if (*spell_wear_off_msg[SPELL_FIRE_SHIELD]) {
            send_to_char(spell_wear_off_msg[SPELL_FIRE_SHIELD], victim);
            send_to_char("\r\n", victim);
        }

    }

    if (AFF3_FLAGGED(victim, AFF3_GR_FORCE))
    {
        affect_from_char(victim, SPELL_FORCE_FIELD);
        leech_from_char(victim, SPELL_FORCE_FIELD);
        REMOVE_BIT(AFF3_FLAGS(victim), AFF3_GR_FORCE);
        if (*spell_wear_off_msg[SPELL_FORCE_FIELD]) {
            send_to_char(spell_wear_off_msg[SPELL_FORCE_FIELD], victim);
            send_to_char("\r\n", victim);
        }

    }

}

void            killfire(struct char_data * victim)
{
    affect_from_char(victim, SPELL_FIREBALL);
    affect_from_char(victim, SPELL_FIRESTORM);
    affect_from_char(victim, SPELL_FLAMESTRIKE);
    affect_from_char(victim, SPELL_FIRE_BREATH);
    affect_from_char(victim, SPELL_BURN);
    affect_from_char(victim, SPELL_FLAME_ARROW);
    REMOVE_BIT(AFF2_FLAGS(victim), AFF2_BURNING);
}

void            killcold(struct char_data * victim)
{
    affect_from_char(victim, SPELL_COLD_ARROW);
    affect_from_char(victim, SPELL_CONE_OF_COLD);
    affect_from_char(victim, SPELL_FROST_BREATH);
    affect_from_char(victim, SPELL_FREEZE);
    REMOVE_BIT(AFF2_FLAGS(victim), AFF2_FREEZING);
}


void            killacid(struct char_data * victim)
{
    affect_from_char(victim, SPELL_ACID_BREATH);
    affect_from_char(victim, SPELL_ACID_BLAST);
    affect_from_char(victim, SPELL_ACID);
    REMOVE_BIT(AFF2_FLAGS(victim), AFF2_ACIDED);
}


void            killchoke(struct char_data * victim)
{
    affect_from_char(victim, SPELL_CHOKE);
    REMOVE_BIT(AFF3_FLAGS(victim), AFF3_CHOKE);
}

int             ok_kill(struct char_data * ch, struct char_data * victim)
{
    if (!CAN_MURDER(ch, victim) && ch!=victim) {
        act("The Gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
        return FALSE;
    }
    return TRUE;
}

int             mag_savingthrow(struct char_data * ch, int type, struct char_data * ataker)
{
    int             save;

    /* negative apply_saving_throw values make saving throws better! */

    /*if (IS_NPC(ch))
        save = interpolate(GET_LEVEL(ch), saving_throws1[CLASS_WARRIOR][0], saving_throws1[CLASS_WARRIOR][1]);
    else
        save = interpolate(GET_LEVEL(ch), saving_throws1[(int) GET_CLASS_NUM_FULL(ch)][0], saving_throws1[(int) GET_CLASS_NUM_FULL(ch)][1]);

    save += GET_LEVEL(ataker) - GET_LEVEL(ch);*/

    //  save=50+(GET_WIS(ataker)-GET_WIS(ch))*8;
    if (ataker==supermob)
        return FALSE;
    //save=50+(GET_WIS(ataker)-GET_WIS(ch))*8;

    save=110-GET_WIS(ch)*GET_WIS(ch)/5;
    save += GET_SAVE(ch, type);
    save+=GET_WIS(ataker)*GET_WIS(ataker)/20;
    if (IS_DWARF(ch))
        save-=15;
    else if (IS_ORC(ch))
        save+=15;
    if (save < 3)
        save = 3;
    if (save > 97)
        save = 97;
    /* throwing a 0 is always a failure */
    return (save < number(0, 99));
}


/* affect_update: called from comm.c (causes spells to wear off) */
void            affect_update(void)
{
    static struct affected_type *af,
                *next;
    static struct char_data *i;
    extern struct raff_node *raff_list;
    struct raff_node *raff, *next_raff, *temp;

for (i = character_list; i; i = i->next) {

        /*    if (AFF_FLAGGED(i, AFF_VISITING)) {
                if ((i->howlong) > 1)
                    (i->howlong)--;
                if (i->howlong == 2)
                    send_to_char("You feel your bond to this place fading.\r\n", i);
                if (i->howlong == 1) {
                    if (i->wasbefore < 2)
                        i->wasbefore = r_mortal_start_room;
                    send_to_char("A shimmering portal suddenly appears above you and sucks you in.\r\r\n\n", i);
                    act("A shimmering portal suddenly appears above $n and sucks $m in.", FALSE, i, NULL, NULL, TO_ROOM);
                    if (FIGHTING(i) && (FIGHTING(FIGHTING(i)) == i))
                        stop_fighting(FIGHTING(i));
                    if (FIGHTING(i))
                        stop_fighting(i);
                    char_from_room(i);
                    char_to_room(i, i->wasbefore);
                    act("A shimmering portal opens and throws $n out.", TRUE, i, 0, 0, TO_ROOM);
                    look_at_room(i, 0);
                    REMOVE_BIT(AFF_FLAGS(i), AFF_VISITING);
                    i->howlong = 0;
                    i->wasbefore = 0;
                }
                if (i->howlong < 2 || i->howlong > 40) {
                    REMOVE_BIT(AFF_FLAGS(i), AFF_VISITING);
                    i->howlong = 0;
                    i->wasbefore = 0;
                }
            }*/
        for (af = i->affected; af; af = next) {
            next = af->next;
            if (af->duration >= 1)
                af->duration--;
            //else if (af->duration == -1)        /* No action */
            //af->duration = -1;      /* GODs only! unlimited */
            else if (!af->duration)  {
                if (IS_SPELL(af->type))
                    if (!af->next || (af->next->type != af->type) ||
                            (af->next->duration > 0))
                        if (*spell_wear_off_msg[af->type]) {
                            send_to_char(spell_wear_off_msg[af->type], i);
                            send_to_char("\r\n", i);
                        }
                affect_remove(i, af);
            }
        }
    }
    /* update the room affections */
    for (raff = raff_list; raff; raff = next_raff) {
        next_raff = raff->next;

        if (raff->timer>=1)
            raff->timer--;

        if (raff->timer == 0) {
            /* this room affection has expired */
            if (IS_SPELL(raff->spell))
            {
                send_to_room(spell_wear_off_msg[raff->spell],raff->room);
                send_to_room("\r\n", raff->room);
            }

            /* remove the affection */
            REMOVE_BIT(world[(int)raff->room].room_affections,
                       raff->affection);
            REMOVE_FROM_LIST(raff, raff_list, next)
            DISPOSE(raff);
        }
    }
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int             mag_materials(struct char_data * ch, int item0, int item1, int item2,
                              int extract, int verbose)
{
    struct obj_data *tobj;
    struct obj_data *obj0 = NULL;
    struct obj_data *obj1 = NULL;
    struct obj_data *obj2 = NULL;

    for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
        if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
            obj0 = tobj;
            item0 = -1;
        } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
            obj1 = tobj;
            item1 = -1;
        } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
            obj2 = tobj;
            item2 = -1;
        }
    }
    if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
        if (verbose) {
            switch (number(0, 2)) {
            case 0:
                send_to_char("A wart sprouts on your nose.\r\n", ch);
                break;
            case 1:
                send_to_char("Your hair falls out in clumps.\r\n", ch);
                break;
            case 2:
                send_to_char("A huge corn develops on your big toe.\r\n", ch);
                break;
            }
        }
        return (FALSE);
    }
    if (extract) {
        if (item0 < 0) {
            obj_from_char(obj0);
            extract_obj(obj0);
        }
        if (item1 < 0) {
            obj_from_char(obj1);
            extract_obj(obj1);
        }
        if (item2 < 0) {
            obj_from_char(obj2);
            extract_obj(obj2);
        }
    }
    if (verbose) {
        send_to_char("A puff of smoke rises from your pack.\r\n", ch);
        act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
    }
    return (TRUE);
}

/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 */

void            mag_damage(int level, struct char_data * ch, struct char_data * victim,
                           int spellnum, int savetype)
{
    int             is_mage = 0,
                              is_cleric = 0;
    int             dam = 0;
    struct affected_type af;


    if (victim == NULL || ch == NULL)
        return;

    if (DEAD(victim) || DEAD(ch))
        return;


    af.type = spellnum;
    af.bitvector = 0;
    af.bitvector2 = 0;
    af.bitvector3 = 0;
    af.modifier = 0;
    af.location = APPLY_NONE;

    is_mage = (IS_MAGIC_USER(ch) || IS_NECRON(ch));
    is_cleric = (IS_CLERIC(ch) || IS_DRUID(ch));

    switch (spellnum) {
        /* Mostly mages */
    case PRAYER_PUNISHMENT:
        dam = dice(level, 5);
        break;

    case SPELL_MAGIC_MISSILE:
        dam = dice(1, 8) + lvD8(ch);
        if (AFF_FLAGGED(victim, AFF_SHIELD)) {
            send_to_char("Your mystic shield absorbs the magic!\r\n", victim);
            send_to_char("Your magic mysticly disappears!\r\n", ch);
            dam = 0;
            return;
        }
        break;
    case SPELL_CHILL_TOUCH:     /* chill touch also has an affect */
        if (AFF_FLAGGED(victim, AFF_SHIELD) && !number(0, 4)) {
            send_to_char("Your mystic shield absorbs the magic!\r\n", victim);
            send_to_char("Your magic mysticly disappears!\r\n", ch);
            dam = 0;
            return;
        }
        dam = dice(2, 4) + 3 + lvD8(ch);
        break;
    case SPELL_BURNING_HANDS:
        if (AFF_FLAGGED(victim, AFF_SHIELD) && !number(0, 7)) {
            send_to_char("Your mystic shield absorbs the magic!\r\n", victim);
            send_to_char("Your magic mysticly disappears!\r\n", ch);
            dam = 0;
            return;
        }
        dam = dice(2, 4) + MIN(20,level);
        break;
    case SPELL_SHOCKING_GRASP:
        dam = dice(level, 4);
        break;

    case SPELL_LIGHTNING_BOLT:
        dam = dice(level, 5);
        if (AFF2_FLAGGED(victim, AFF2_PROT_LIGHTNING))
            dam -= number(1, dam / 2);
        break;

    case SPELL_COLOR_SPRAY:
        dam = dice(level, 6);
        break;

    case SPELL_CONJURING_BEAM:
        
        
        dam=(50*MIN(level, 3*GET_LEVEL(victim))/(3*GET_LEVEL(victim)))*GET_MAX_HIT(victim)/100;
        //dam = MIN(3000, MAX(1000, dice(level, 20)));
        //if (!IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL, ch))
          //  dam /= 2;
        if (AFF_FLAGGED(victim, AFF_DEATHDANCE))
            dam /= 2;
        if (victim==ch) dam=3000;
        break;

    case SPELL_TORNADO:
        dam=number(level, 2*level);
        break;

    case SPELL_FIREBALL:
        dam = dice(level, 7);
        if (AFF_FLAGGED(victim, AFF_PROT_FIRE))
            dam -= dam / 2;
        if (IS_WET(victim))
            dam -= dam / 2;
        break;

    case SPELL_ACID_BLAST:
        dam = dice(2*level, 4)+level;
        break;
        
    case PRAYER_JUDGMENT:
        //dam = dice(2*level, 4);
        dam=dice(level, 9);
        if (IS_EVIL(ch))
        {
            if (IS_GOOD(victim))
                dam+=dam/3;
            else
                dam-=dam/3;
        }
        else {
            if (IS_EVIL(victim))
                dam+=dam/3;
            else
                dam-=dam/3;
        }
        break;
        /* Mostly clerics */
    case PRAYER_DISPEL_EVIL:
    case SPELL_DISPEL_EVIL:
        dam=dice(level, 7);
        //if (mag_savingthrow(victim, savetype, ch))
        //dam/=2;
        if (IS_EVIL(ch)) {
            victim = ch;
            GET_HIT(ch) = dice(1, 4);
            send_to_char("You lift high your unholy symbol against yourself.\r\n",ch);
            act("$n lifts high $s unholy symbol against $mself.", FALSE, ch, 0, victim, TO_ROOM);
            return;
        } else if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
            act("The Gods stand for $N and protect $m.", FALSE, ch, 0, victim, TO_CHAR);
            dam = 0;
            return;
        }
        break;
        
    case PRAYER_SMITE_EVIL:    
        //dam = dice(2*level, 4);        
        dam=dice(level, 14);
        //if (mag_savingthrow(victim, savetype, ch))
        //dam/=2;
        if (IS_EVIL(ch)) {
            victim = ch;
            GET_HIT(ch) = dice(1, 4);
            send_to_char("You lift high your unholy symbol against yourself.\r\n",ch);
            act("$n lifts high $s unholy symbol against $mself.", FALSE, ch, 0, victim, TO_ROOM);
            return;
        } else if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
            act("The Gods stand for $N and protect $m.", FALSE, ch, 0, victim, TO_CHAR);
            dam = 0;
            return;
        }
        break;     
        
        
    case PRAYER_SMITE_GOOD:    
        //dam = dice(2*level, 4);        
        dam=dice(level, 14);
        //if (mag_savingthrow(victim, savetype, ch))
        //dam/=2;
        if (IS_GOOD(ch)) {
            victim = ch;
            GET_HIT(ch) = dice(1, 4);
            send_to_char("You lift high your holy symbol against yourself.\r\n",ch);
            act("$n lifts high $s holy symbol against $mself.", FALSE, ch, 0, victim, TO_ROOM);
            return;
        } else if (IS_EVIL(victim) || IS_NEUTRAL(victim)) {
            act("The Gods stand for $N and protect $m.", FALSE, ch, 0, victim, TO_CHAR);
            dam = 0;
            return;
        }
        break;     
        
    case PRAYER_DISPEL_GOOD:    
    case SPELL_DISPEL_GOOD:
        dam=dice(level, 7);
        //if (mag_savingthrow(victim, savetype, ch))
          //  dam/=2;
        if (IS_GOOD(ch)) {
            victim = ch;
            GET_HIT(ch) = dice(1, 4);
            act("You lift high your holy symbol against yourself.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n lifts high $s holy symbol against $mself.", FALSE, ch, 0, victim, TO_ROOM);
            return;
        } else if (IS_EVIL(victim) || IS_NEUTRAL(victim)) {
            act("The Gods stand for $N and protect $m.", FALSE, ch, 0, victim, TO_CHAR);
            dam = 0;
            return;
        }
        break;

    case SPELL_CALL_LIGHTNING:
        if (!OUTSIDE(ch))
        {
            send_to_char("You couldn't call the lightings here!\r\n",ch);
            return;
        };

        if (!(weather_info.sky == SKY_LIGHTNING)) {
            act("You see no lightnings in the sky.", FALSE, ch, 0, victim, TO_CHAR);
            return;
        };

        dam = dice(level, 5)+level;
        if (AFF2_FLAGGED(victim, AFF2_PROT_LIGHTNING))
            dam -= number(1, dam / 2);
        break;


    case SPELL_ARROW_RAIN:
        dam=dice(level*2, 2);
        break;

    case PRAYER_SPIRITUAL_HAMMER:
    case SPELL_SPIRITUAL_HAMMER:
        //dam=dice(level*2, 3);
        dam=dice(level, 9);
        break;


    case SPELL_HARM:
        if (GET_HIT(victim)<8*level)
            dam=GET_HIT(victim)-dice(1,4);
        else
            dam=dice(level, 4)+level;
        break;

    case SPELL_ENERGY_DRAIN:
        dam = dice(level, 5);
        break;

    case SPELL_COLD_ARROW:
        dam = dice(8, level/2)+level;
        if (AFF2_FLAGGED(victim, AFF2_PROT_COLD))
            dam -= number(1, dam / 2);
        break;


    case SPELL_FLAME_ARROW:
        dam = dice(12, level/2)+2*level;
        if (AFF_FLAGGED(victim, AFF_PROT_FIRE))
            dam -= dam / 2;
        if (IS_WET(victim))
            dam -= dam / 2;
        break;

        /* Area spells */
    case SPELL_MINUTE_METEOR:
        dam = dice(level, 8) + level;
        break;

    case SPELL_CONE_OF_COLD:
        dam = dice(20, 5) + 3*level;
        if (AFF2_FLAGGED(victim, AFF2_PROT_COLD))
            dam -= number(1, dam / 2);
        break;
    case SPELL_AREA_LIGHTNING:
        dam = dice(12, level / 2);
        if (AFF2_FLAGGED(victim, AFF2_PROT_LIGHTNING))
            dam -= number(1, dam / 2);
        break;

    case SPELL_EARTHQUAKE:
        dam = dice(level, 2) + level;
        break;

    case SPELL_FIRESTORM:
        dam = lvD2(ch) * dice(2, 7) + 2*level;
        if (AFF_FLAGGED(victim, AFF_PROT_FIRE))
            dam -= dam / 2;
        if (IS_WET(victim))
            dam -= dam / 2;
        break;

    case SPELL_FLAMESTRIKE:
        dam = dice(10, 10) + 2 * level;
        if (AFF_FLAGGED(victim, AFF_PROT_FIRE))
            dam -= dam / 2;
        if (IS_WET(victim))
            dam -= dam / 2;
        break;

    case SPELL_POWER_OF_NATURE:
        dam = dice(lvD2(ch), lvD2(ch)) + MAX(level, 5 * (level - GET_LEVEL(victim)));
        break;

    case SPELL_HOLY_WORD:
        dam = dice(level, 7);
        if (GET_ALIGNMENT(ch) ==1000)
            dam += 2 * level;
        break;

    case SPELL_FIRE_BREATH:
        dam = dice(15, 10) + dice(5, level);
        if (AFF_FLAGGED(victim, AFF_PROT_FIRE))
            dam -= dam / 2;
        if (IS_WET(victim))
            dam -= dam / 2;
        if(number(0,50)>GET_LEVEL(ch)){
            if (!mag_savingthrow(victim, SAVING_BREATH, ch)) {
                struct obj_data *burn=NULL;
                for(burn=victim->carrying ;
                        burn &&
                        (burn->obj_flags.type_flag!=ITEM_SCROLL) &&
                        (burn->obj_flags.type_flag!=ITEM_WAND) &&
                        (burn->obj_flags.type_flag!=ITEM_STAFF) &&
                        (burn->obj_flags.type_flag!=ITEM_NOTE)
                        ;
                        burn=burn->next_content);
                if(burn){ /* if we found one thing to burn its gone */
                    act("&RCaught by the fire, your $p burns to ashes!&0",FALSE, victim, burn, 0, TO_CHAR);
                    extract_obj(burn);
                }
            }
        }
        break;

    case SPELL_GAS_BREATH:
        dam = dice(12, 10) + dice(4, level);
        if (!mag_savingthrow(victim, SAVING_BREATH, ch)) {
            apply_poison(victim, 1);

            act("$n starts choking on the toxic fumes!!", FALSE, victim, 0, 0,TO_ROOM);
            send_to_char("You start choking on the toxic fumes!\r\n", victim);
        }
        if (!mag_savingthrow(victim, SAVING_BREATH, ch) && (!affected_by_spell(victim, SPELL_BLINDNESS))) {
            af.type      = SPELL_BLINDNESS;

            af.bitvector = AFF_BLIND;
            af.bitvector2=0;
            af.bitvector3=0;
            af.duration  = 0;
            affect_to_char(victim, &af);

            act("$n seems to have been blinded by the gas!", FALSE, victim, 0, 0, TO_ROOM);
            send_to_char("You have been blinded by the gas!\r\n", victim);
        }

        break;

    case SPELL_FROST_BREATH:
        dam = dice(8, 10) + level;
        if (AFF2_FLAGGED(victim, AFF2_PROT_COLD))
            dam -= number(1, dam / 2);
        if (!mag_savingthrow(victim, SAVING_BREATH, ch)) {
            af.type      = SPELL_CHILL_TOUCH;
            af.duration  = 1;
            af.modifier  = -1;
            af.location  = APPLY_STR;
            af.bitvector = 0;
            af.bitvector2=0;
            af.bitvector3=0;
            affect_join(victim, &af, TRUE, FALSE, FALSE, FALSE);
            send_to_char("The intense cold saps your strength.\n\r", victim);
        }

        if(number(0,50)<GET_LEVEL(ch))
        {
            if (!mag_savingthrow(victim, SAVING_BREATH, ch))
            {  struct obj_data *frozen=NULL;
                for(frozen=victim->carrying ;
                        frozen && (frozen->obj_flags.type_flag!=ITEM_DRINKCON) &&
                        (frozen->obj_flags.type_flag!=ITEM_FOOD) &&
                        (frozen->obj_flags.type_flag!=ITEM_POTION);
                        frozen=frozen->next_content);
                if(frozen)
                {
                    act("$p &Wfreezes&0 into ice and breaks!",FALSE,victim,frozen,0,TO_CHAR);
                    extract_obj(frozen);
                }
            }
        }


        break;

    case SPELL_ACID_BREATH:
        dam = dice(20, 12) + dice(7, level);
        if(number(0,50)<GET_LEVEL(ch))
        {
            if (!mag_savingthrow(victim, SAVING_BREATH, ch))
            {  int damaged;
                for(damaged = 0; damaged<NUM_WEARS &&
                        (victim->equipment[damaged]) &&
                        (victim->equipment[damaged]->obj_flags.type_flag==ITEM_ARMOR) &&
                        (victim->equipment[damaged]->obj_flags.value[0]>0) &&
                        (number(0,2)==0) ; damaged++)
                {
                    act("&YAcid corrodes your $p!&0", FALSE,victim,victim->equipment[damaged],0,TO_CHAR);
                    GET_AC(victim)-=apply_ac(victim,damaged);
                    ch->equipment[damaged]->obj_flags.value[0]-=number(1,7);
                    GET_AC(victim)+=apply_ac(victim,damaged);
                    ch->equipment[damaged]->obj_flags.cost = 0;
                }
            }
        }
        break;

    case SPELL_LIGHTNING_BREATH:
        dam = dice(18, 10) + dice(6, level);
        if (AFF2_FLAGGED(victim, AFF2_PROT_LIGHTNING))
            dam -= number(1, dam / 2);
        break;

    case SPELL_BLADEBARRIER:
        dam = dice(8, 8) + level;
        break;

    case SPELL_SUNRAY:
    case PRAYER_BLINDING_LIGHT:
        return;
        break;
    }                           /* switch(spellnum) */

    //sprintf(buf, "dam: %d\r\n", dam);
    //send_to_char(buf, ch);
    /* divide damage by two if victim makes his saving throw */
    if (!IS_NPC(ch)) {
        if (AFF_FLAGGED(ch, AFF_EQUINOX))
            dam += number(dam / 2, dam * 2);

    }
    //dam+=GET_DAMROLL(ch);
    //    dam = (dam * GET_INT(ch)) / 18;
    // if (mag_savingthrow(victim, savetype, ch))
    //   dam /= 2;
    /*if (IS_ORC(victim))
        dam=115*dam/100;
      */
    //sprintf(buf, "dam: %d\r\n", dam);
    //send_to_char(buf, ch);


    /* and finally, inflict the damage */
    //    CREF(victim, CHAR_NULL);
    if (!CAN_MURDER(ch, victim))
        act("The Gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
    else
        damage(ch, victim, dam, spellnum, NULL);
    //  CUREF(victim);
    if (spellnum==SPELL_TORNADO && !DEAD(victim) && !DEAD(ch))
    {
        struct char_data * caster=ch, * cvict=victim;
        struct obj_data *ovict=NULL;
        char *tar_str=NULL;
        MANUAL_SPELL(spell_gust_of_wind);
    }
}

/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

int fort_spells[]={
                      SPELL_ARMOR,
                      SPELL_SANCTUARY,
                      SPELL_SHIELD,
                      SPELL_STONESKIN,
                      SPELL_BARKSKIN,
                      SPELL_DEFLECTION,
                      SPELL_MIRROR_IMAGE,
                      SPELL_BLINK,
                      SPELL_FIRE_SHIELD,
                      SPELL_FORCE_FIELD,
                      SPELL_SHILLELAGH,
                      SPELL_VITALITY,
                      SPELL_MAGICAL_PROTECTION,
                      SPELL_RUNIC_SHELTER,
                      SPELL_AGILITY,
                      SPELL_INTELLIGIZE,
                      SPELL_ENERGY_CONTAINMENT,
                      SPELL_HOLY_ARMOR,
                      -1
                  };

void            mag_affects(int level, struct char_data * ch, struct char_data * victim,
                            int spellnum, int savetype)
{
    struct affected_type af,
                af2, af3;
    int             is_mage = FALSE;
    int             is_cleric = FALSE;
    int             accum_affect = FALSE;
    int             accum_duration = FALSE;
    char           *to_vict = NULL;
    char           *to_char = NULL;
    char           *to_room = NULL;
    int             nof = 1;
    int             ftc = 0, i, j;

    if (victim == NULL || ch == NULL)
        return;
    if (DEAD(victim) || DEAD(ch))
        return;


    is_mage = (IS_MAGIC_USER(ch) || IS_NECRON(ch));
    is_cleric = (IS_CLERIC(ch) || IS_DRUID(ch));




    af.type = spellnum;
    af.bitvector = 0;
    af.bitvector2 = 0;
    af.bitvector3 = 0;
    af.modifier = 0;
    af.location = APPLY_NONE;

    af2.type = spellnum;
    af2.bitvector = 0;
    af2.bitvector2 = 0;
    af2.bitvector3 = 0;
    af2.modifier = 0;
    af2.location = APPLY_NONE;

    af3.type = spellnum;
    af3.bitvector = 0;
    af3.bitvector2 = 0;
    af3.bitvector3 = 0;
    af3.modifier = 0;
    af3.location = APPLY_NONE;


switch (spellnum) {

    case SPELL_FORTRESS:
        /*j=0;
        while (fort_spells[j]!=-1)
    {
            for (i = 0; i < NUM_CLASSES; i++)
        	if (IS_SET(GET_CLASS(ch), 1 << i) && (spell_info[fort_spells[j]].min_level[i] <= GET_LEVEL(ch)-5) &&
                GET_SKILL(ch, fort_spells[j])>85)
            mag_affects(GET_LEVEL(ch), ch, victim, fort_spells[j], SPELL_FORTRESS);
            j++;
    }*/
        return;

    case PRAYER_MINOR_P:
        if (ch!=victim && !GET_SKILL(ch, PRAYER_PROTECTION))
        {
            send_to_char("Your prayer is not answered.\r\n", ch);
            return;
        }
        if (victim->protection>MIN(GET_LEVEL(ch), 9))
        {
            send_to_char("Target is imbued by more powerful protection already.\r\n", ch);
            return;
        }

        af.bitvector2=AFF2_PROTECTION;
        af.duration =-2;// 12;
        victim->protection=MIN(GET_LEVEL(ch), 9);
        to_room= "$n briefly glows.";

        to_vict = "You feel someone protecting you.";
        break;

    case PRAYER_PROTECTION:
        if (ch!=victim && !GET_SKILL(ch, PRAYER_MAJOR_P))
        {
            send_to_char("Your prayer is not answered.\r\n", ch);
            return;
        }
        if (victim->protection>MIN(GET_LEVEL(ch), 19))
        {
            send_to_char("Target is imbued by more powerful protection already.\r\n", ch);
            return;
        }
        affect_from_char(victim, PRAYER_MINOR_P);

        af.bitvector2=AFF2_PROTECTION;
        af.duration =-2;// 12;
        victim->protection=MIN(GET_LEVEL(ch), 19);
        to_room= "$n briefly glows.";
        to_vict = "You feel someone protecting you.";
        break;
    case PRAYER_MAJOR_P:
        if (ch!=victim && !GET_SKILL(ch, PRAYER_GREATER_P))
        {
            send_to_char("Your prayer is not answered.\r\n", ch);
            return;
        }
        if (victim->protection>MIN(GET_LEVEL(ch), 29))
        {
            send_to_char("Target is imbued by more powerful protection already.\r\n", ch);
            return;
        }
        affect_from_char(victim, PRAYER_MINOR_P);
        affect_from_char(victim, PRAYER_PROTECTION);
        af.bitvector2=AFF2_PROTECTION;
        af.duration =-2;// 12;
        victim->protection=MIN(GET_LEVEL(ch), 29);
        to_room= "$n briefly glows.";
        to_vict = "You feel someone protecting you.";
        break;

    case PRAYER_GREATER_P:
        if (ch!=victim && !GET_SKILL(ch, PRAYER_DIVINE_P))
        {
            send_to_char("Your prayer is not answered.\r\n", ch);
            return;
        }
        if (victim->protection>MIN(GET_LEVEL(ch), 39))
        {
            send_to_char("Target is imbued by more powerful protection already.\r\n", ch);
            return;
        }
        affect_from_char(victim, PRAYER_MINOR_P);
        affect_from_char(victim, PRAYER_PROTECTION);
        affect_from_char(victim, PRAYER_MAJOR_P);
        af.bitvector2=AFF2_PROTECTION;
        af.duration =-2;// 12;
        victim->protection=MIN(GET_LEVEL(ch), 39);
        to_room= "$n briefly glows.";
        to_vict = "You feel someone protecting you.";
        break;

    case PRAYER_DIVINE_P:
        if (ch!=victim && !GET_SKILL(ch, PRAYER_SANCTUARY))
        {
            send_to_char("Your prayer is not answered.\r\n", ch);
            return;
        }
        if (victim->protection>MIN(GET_LEVEL(ch), 49))
        {
            send_to_char("Target is imbued by more powerful protection already.\r\n", ch);
            return;
        }
        affect_from_char(victim, PRAYER_MINOR_P);
        affect_from_char(victim, PRAYER_PROTECTION);
        affect_from_char(victim, PRAYER_MAJOR_P);
        affect_from_char(victim, PRAYER_GREATER_P);
        af.bitvector2=AFF2_PROTECTION;
        af.duration =-2;// 12;
        victim->protection=MIN(GET_LEVEL(ch), 49);
        to_room= "$n briefly glows.";
        to_vict = "You feel someone protecting you.";
        break;



    case SPELL_WEAKEN:
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_STR;
        if (mag_savingthrow(victim, savetype, ch))
            af.duration = 2;
        else
            af.duration = 4;
        af.modifier = MIN(-1, -level/10);
        //accum_duration = TRUE;
        to_room= "$n seems weaker!";
        to_vict = "You feel weaker!";
        break;


    case SPELL_STRONG_MIND:
        af.duration=-2;//level/2;
        af.bitvector3=AFF3_STRONG_MIND;
        to_room="$n looks more determined.";
        to_vict="You are more determined now.";
        break;

    case SPELL_CHILL_TOUCH:
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_STR;
        if (mag_savingthrow(victim, savetype, ch))
            af.duration = 1;
        else
            af.duration = 3;
        af.modifier = -1;
        accum_duration = TRUE;
        to_vict = "You feel your strength wither!.";
        break;

    case SPELL_ARMOR:
        af.location = APPLY_AC;
        af.modifier = 1 + lvD5(ch);
        af.duration =-2;// 12;

        //accum_duration = TRUE;
        to_room= "$n briefly glows.";
        to_vict = "You feel someone protecting you.";
        break;

    case SPELL_HOLY_ARMOR:
        af.location = APPLY_AC;
        af.modifier = +20;
        af.duration =-2;// 8;
        //accum_duration = TRUE;
        to_room= "$n is surrounded by a holy armor.";
        to_vict = "You are surrounded by a holy armor.";
        break;

    case SPELL_FAERIE_FIRE:
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_AC;
        af.modifier = -2 - lvD5(ch);
        af.duration = -2;
        //accum_duration = TRUE;
        to_room= "Pale glowing light outlines $n!";
        to_vict = "You feel a pale light form around you!";
        break;


    case SPELL_QUAD_DAMAGE:
        af.location = 0;
        af.modifier = 0;
        af.duration = 1;
        af.bitvector3 = AFF3_QUAD;
        to_room= "$n goes wild!";
        to_vict = "You feel like doing &W&f*** D A M A G E ***&0";
        break;

    case SPELL_GIVE_LIFE:
        af.location = APPLY_HIT;
        af.modifier = level * 4;
        af.duration = level / 100;
        break;

    case SPELL_ENDURANCE:
        af.location = APPLY_MOVE;
        af.modifier = 100;
        af.duration =-2;// level/2;
        to_room="$n seems more endurable now.";
        to_vict="You feel more endurable.";
        break;


    case SPELL_PETRIFY:
        af.location = APPLY_AC;
        af.modifier = +50;
        af.duration = 0;
        af.bitvector2 = AFF2_PETRIFY;
        break;

    case SPELL_ULTRA_DAMAGE:
        af.duration = 0;
        af.bitvector2 = AFF2_ULTRA;
        break;

    case SPELL_PRISMATIC_SPHERE:
        af.duration = 0;
        af.bitvector2 = AFF2_PRISM;
        break;

    case SPELL_EQUINOX:
        if (AFF_FLAGGED(ch, AFF_EQUINOX)) {
            send_to_char("Your mind is already influenced by Equinox!\r\n", ch);
            return;
        }
        if ((number(1, 100) > 50) && !GET_EQ(ch, WEAR_HEAD)) {
            af.bitvector = AFF_EQUINOX;
            af.duration =-2;// GET_WIS(ch) / 8;
            to_vict = "You connect your mind through the Universe... Nothing can describe the feeling.";
            to_room = "$n starts glowing like a star!";
            //GET_MANA(ch) = GET_MAX_MANA(ch);
            GET_HIT(ch) = GET_MAX_HIT(ch);
            //GET_MOVE(ch) = GET_MAX_MOVE(ch);
            break;
        } else {
            send_to_char("You feel something went wrong. VERY wrong! AAAAAAAAAAAAAAARGH!\r\n", ch);
            act("$n covers his head as some invisible force completely DISINTIGRATES $m!", FALSE, ch, NULL, NULL, TO_ROOM);
            GET_HIT(ch) -= number(GET_MAX_HIT(ch) / 3, GET_MAX_HIT(ch));
            //GET_MOVE(ch) = number(1, GET_DEX(ch));
            GET_POS(ch)=POS_SLEEPING;
            check_kill(ch, "equinox");
            return;
        }

    case SPELL_BLESS:
        af.location = APPLY_HITROLL;
        af.modifier = 1+level/10 + ((ch == victim)?2:0);
        af.duration =-2;// 5 + level/8;

        af2.location = APPLY_SAVING_SPELL;
        af2.modifier = -1-level / 10 - ((ch == victim)?5:0);
        af2.duration =-2;// 5 + level/8;

        to_room= "$n glows blue.";
        to_vict = "You feel righteous.";
        break;

    case PRAYER_BLESSING:

        af.location = APPLY_HITROLL;
        af.modifier = 1+level/10+((ch == victim)?2:0);
        af.duration =-2;// 5 + level/8;

        if (GET_DEITY(ch)==GET_DEITY(victim))
        {
            af3.location = APPLY_DAMROLL;
            af3.modifier = 1+level/15;
            af3.duration =-2;// 5 + level/8;
        }


        af2.location = APPLY_SAVING_SPELL;
        af2.modifier = -1-level / 10 - ((ch == victim)?5:0);
        af2.duration =-2;// 5 + level/8;

        to_room= "$n glows blue.";
        to_vict = "You feel righteous.";
        break;
        
        
    case PRAYER_INSPIRATION:

        af.location = APPLY_HIT;
        af.modifier = level*2;
        af.duration =1;// 5 + level/8;

        af3.location = APPLY_MANA;
        af3.modifier = level*2;
        af3.duration =1;// 5 + level/8;
        

        af2.location = APPLY_MOVE;
        af2.modifier = level;
        af2.duration =1;// 5 + level/8;

        to_room= "$n breathes with new inspiration.";
        to_vict = "You feel inspired!";
        break;
    

    case SPELL_BENEDICTION:
        af.location = APPLY_HITROLL;
        af.modifier = 5;
        af.duration =-2;// 10;
        af2.location = APPLY_SAVING_SPELL;
        af2.modifier = -5;
        af2.duration =-2;// 10;

        to_room= "$n glows bright blue.";
        to_vict = "You feel much more righteous.";
        break;

    case SPELL_HOG:
        af.duration = 1;
        af.bitvector3=AFF3_HOG;
        to_room= "A bright oreol forms around $n.";
        to_vict = "Glowing light apperas in the skies above you.";
        break;


    case SPELL_VITALITY:
        af.location = APPLY_HIT;
        af.modifier = GET_MAX_HIT(ch)/6;
        af.duration = -2;

        to_room= "$n floods with life.";
        to_vict = "You feel your life energy flooding in!";
        break;


    case SPELL_SUNRAY:
    case PRAYER_BLINDING_LIGHT:
    case SPELL_BLINDNESS:
        if (MOB_FLAGGED(victim, MOB_NOBLIND) || mag_savingthrow(victim, savetype, ch)) {
            act("$n barely manages to close $s eyes in time.", FALSE, victim, 0, 0, TO_ROOM);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.type=SPELL_BLINDNESS;
        af.bitvector = AFF_BLIND;
        af.duration = 0;


        to_room = "$n seems to be blinded!";
        to_vict = "You have been blinded!";
        break;


    case SPELL_HOLD_MONSTER:
        if (!IS_NPC(victim)) {
            send_to_char("You can not cast this spell upon other players!", ch);
            return;
        };
        if (CHANCE(4 * GET_CHA(ch)) && (MOB_FLAGGED(victim, MOB_HOLDED)
                                        || mag_savingthrow(victim, savetype, ch))) {
            send_to_char("You fail.\r\n", ch);
            return;
        }
        af.duration = lvD8(ch);
        af.bitvector = AFF_HOLDED;
        af2.location = APPLY_AC;
        af2.modifier = -20;
        af2.duration = -2;
        send_to_char("You close your eyes and deeply concentrate.\r\n", ch);
        to_room = "$N mummbles few words and deeply concentrates.";
        to_vict = "You have been holded!";
        break;

    case SPELL_CURSE:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_HITROLL;
        af.duration =-2;// 1 + (level / 2);
        af.modifier = -lvD6(ch);
        af.bitvector = AFF_CURSE;

        af2.location = APPLY_SAVING_SPELL;
        af2.duration =-2;// 1 + (level /4);
        af2.modifier = +lvD6(ch);
        af2.bitvector = AFF_CURSE;

        //accum_duration = TRUE;
        to_room = "$n briefly glows red!";
        to_vict = "You feel very uncomfortable.";
        break;



    case SPELL_WRATH_OF_EARTH:
        if (!ok_kill(ch, victim))
            return;
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        af.duration =-2;// MAX(0, 6-GET_LEVEL(victim)/ 10);
        af.bitvector2 = AFF2_WRATHE;
        //accum_duration = FALSE;
        to_room = "$n suddenly looks depressed!";
        to_vict = "You suddenly feel very different!";
        break;
        /*case SPELL_WRATH_OF_GOD:

        if (!ok_kill(ch,victim)) return;
        if (AFF2_FLAGGED(victim,AFF2_WRATH))
    {send_to_char("Your victim is allready under the wrath of the Gods!\r\n",ch);
        return;
    }
    {
        struct obj_data *tmpo;
        WRATH(victim)=1;
        tmpo=read_object(WRATH_CLOUD,VIRTUAL,0);
        obj_to_room(tmpo,victim->in_room);
        WRATHOBJ(victim)=tmpo;


            af.duration = 1;
            af.bitvector2 = AFF2_WRATH;

            to_room = "A small, dark cloud materialises itself above $n.";
            to_vict = "A small, dark cloud starts floating above you.";
          }

            break;
        */
    case SPELL_GUINESS:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_HITROLL;
        af.duration = 0;
        af.modifier = -4;

        af2.location = APPLY_DAMROLL;
        af2.duration = 0;
        af2.modifier = -4;

        //accum_duration = FALSE;
        to_char = "You grin as you fill $N's blood with guinness!";
        to_room = "$n suddenly looks very disturbed.";
        to_vict = "You almost faint as your blood suddenly fills with guinness!";
        if (!IS_NPC(victim))
            GET_COND(victim, DRUNK) = 20;
        break;

    case SPELL_BEAUTIFY:
        af.duration =-2;// level;
        af.location = APPLY_CHA;
        af.modifier = 3;
        //accum_duration = FALSE;
        if (!IS_NPC(ch))
            {   if (!number(0,3))
                to_room = "$n grabs a comb and fixes $s hair.";
            else
                to_room = "$n looks so charming now.";
        }
        else
            to_room = "$n now thinks $e is so charming.";
        to_vict = "You feel so charming.";
        break;

    case PRAYER_SENSE_ALIGN:
    case SPELL_DETECT_ALIGN:
        af.duration =-2;// 14 + level;
        af.bitvector = AFF_DETECT_ALIGN;
        //accum_duration = FALSE;
        to_room = "$n blinks $s eyes few times.";
        to_vict = "Your eyes briefly glow white.";
        break;

    case SPELL_DETECT_INVIS:
        af.duration =-2;// 14 + level;
        af.bitvector = AFF_DETECT_INVIS;
        //accum_duration = FALSE;
        to_room = "$n's eyes tingle.";
        to_vict = "Your eyes tingle.";
        break;

    case SPELL_DETECT_MAGIC:
        af.duration =-2;// 14+level;
        af.bitvector = AFF_DETECT_MAGIC;
        //accum_duration = TRUE;
        to_room = "$n's eyes briefly glow green.";
        to_vict = "Your eyes briefly glow green.";
        break;

    case SPELL_INFRAVISION:
        af.duration =-2;// 4+level;
        af.bitvector = AFF_INFRAVISION;
        //accum_duration = TRUE;
        to_vict = "Your eyes glow red.";
        to_room = "$n's eyes glow red.";
        break;

    case SPELL_INVISIBLE:
        if (!victim)
            victim = ch;

        af.duration =-2;// 3 + (level / 4);
        af.bitvector = AFF_INVISIBLE;
        //accum_duration = TRUE;
        to_vict = "You vanish.";
        to_room = "$n slowly fades out of existence.";
        break;

    case SPELL_MAGICAL_PROTECTION:
        af.duration =-2;// 10;
        af.modifier = -10;
        af.location = APPLY_SAVING_SPELL;
        to_vict = "You feel protected from the forces of magic.";
        to_room = "Air around $n shimmers.";
        break;

    case SPELL_SHROUD_OF_OBSCUREMENT:
        af.duration =-2;// 10;
        af.bitvector3 = AFF3_SHROUD;
        to_vict = "You form an obscuring shroud around you.";
        to_room = "$n forms an obscuring shroud.";
        break;

    case SPELL_POISON:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;

        //accum_duration = TRUE;
        to_vict = "&cYou feel very sick!&0";
        to_room = "$n gets violently ill!";
        break;

    case SPELL_SANCTUARY:
        af.duration =-2;// 2+(level / 10);
        af.bitvector = AFF_SANCTUARY;
        to_vict = "A white aura momentarily surrounds you.";
        to_room = "$n is surrounded by a white aura.";
        break;

    case SPELL_ENLIGHTMENT:
        af.duration = 1;
        af.bitvector3 = AFF3_ENLIGHTMENT;
        to_vict = "Millions of bright particles enlighten you.";
        to_room = "$n is enlighted by millions of bright particles.";
        break;

    case SPELL_DARKENING:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.duration = 1;
        af.bitvector3 = AFF3_DARKENING;
        to_vict = "You feel your combat senses fading.";
        to_room = "Darkeness surrounds $n.";
        break;

    case SPELL_SLEEP:
        if (!ok_kill(ch, victim))
            return;
        if (level<GET_LEVEL(victim) || MOB_FLAGGED(victim, MOB_NOSLEEP)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (mag_savingthrow(victim, savetype, ch)) {
            act("$N yawns.", FALSE,ch,NULL,victim,TO_CHAR);
            return;
        }
        af.duration = 3;
        af.bitvector = AFF_SLEEP;

        if (GET_POS(victim) > POS_SLEEPING) {
            act("You feel very sleepy... zzzzzzz......", FALSE, victim, 0, 0, TO_CHAR);
            act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);

            if (FIGHTING(victim))
                stop_fighting(victim);
            GET_POS(victim) = POS_SLEEPING;
        }
        break;

    case SPELL_STRENGTH:
        /* if (GET_STR(victim)>17) { send_to_char(NOEFFECT, ch); return; } */
        af.location = APPLY_STR;
        af.duration =-2;// (level / 4) + 6;
        af.modifier = 1 + (level >= 32);
        //accum_duration = TRUE;
        accum_affect = FALSE;
        to_vict = "You feel stronger!";
        to_room ="$n buldges $s muscles.";
        break;

    case SPELL_INTELLIGIZE:
        af.location = APPLY_INT;
        af.duration =-2;// 10;
        af.modifier = 5;
        to_vict = "You feel yout intellect rising!";
        to_room ="$n suddenly looks more intelligent!";
        break;

    case SPELL_ENERGY_CONTAINMENT:
        af.location = APPLY_HIT;
        af.duration = 4;
        af.modifier = 150;
        to_vict = "You wave your hands and form an energy containment cloud over you.";
        to_room ="An energy containment cloud appears over $n.";
        break;


    case SPELL_GIANT_STRENGTH:
        af.location = APPLY_STR;
        af.duration =-2;// (level >> 3) + 6;
        af.modifier = 3;
        //accum_duration = FALSE;
        accum_affect = FALSE;
        to_vict = "You feel much stronger!";
        to_room ="$n buldges $s muscles with a wide grin.";
        break;
    case PRAYER_SENSE_LIFE:
    case SPELL_SENSE_LIFE:
        af.duration =-2;// level/2+2;
        af.bitvector = AFF_SENSE_LIFE;
        //accum_duration = TRUE;
        to_vict = "You feel your awareness improve.";
        to_room = "$n carefully turns around few times.";
        break;

    case SPELL_WATERWALK:
        af.duration =-2;// MAX(5, level/4)+2;
        af.bitvector = AFF_WATERWALK;
        //accum_duration = TRUE;
        if (!IS_NPC(ch))
            to_room = "$n crosses $s hands and starts to float in the air.";
        to_vict = "You feel webbing between your toes.";
        break;

    case SPELL_FLY:
        af.duration =-2;// 4 + level;
        af.bitvector = AFF_FLYING;
        //accum_duration = FALSE;
        to_vict = "You rise into the currents of air.";
        to_room = "$n slowly lifts $s feet into the air.";
        break;

        /*    case SPELL_LEVITATE:
                af.duration = MAX(5,level/4)+2;
                af.bitvector = AFF_WATERWALK;
                af.modifier = 5;
                af.location = APPLY_AC;
                accum_duration = TRUE;
                to_vict = "You start to float off the ground.";
                to_room = "$n begins to float off the ground.";
                break;
        */
    case SPELL_PROT_FIRE:
        af.duration =-2;// number(1, level)+3;
        af.bitvector = AFF_PROT_FIRE;
        //accum_duration = FALSE;
        to_room = "$n flashes yellow.";
        to_vict = "You feel a shell of insulation form around your body.";
        break;

    case SPELL_PROT_COLD:
        af.duration = -2;//number(1, level)+3;
        af.bitvector2 = AFF2_PROT_COLD;
        //accum_duration = FALSE;
        to_room = "$n flashes white.";
        to_vict = "You feel a shell of warmth form around your body.";
        break;

    case SPELL_PROT_LIGHTNING:
        af.duration = -2;//number(1, level)+3;
        af.bitvector2 = AFF2_PROT_LIGHTNING;
        //accum_duration = FALSE;
        to_room = "$n flashes blue.";
        to_vict = "You feel a shell of energy form around your body.";
        break;

    case SPELL_WATERBREATH:
        af.duration = -2;//24;
        af.bitvector = AFF_WATERBREATH;
        //accum_duration = FALSE;
        to_vict = "It feels as if you just sprouted some gills.";
        to_room = "$n can now breathe underwater.";
        break;

    case SPELL_CONE_OF_COLD:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_FREEZING;
        ftc = 1;
        if (!AFF2_FLAGGED(victim, AFF2_FREEZING)) {
            to_vict = "You are consumed with coldness!";
            to_room = "$n starts shivering from coldness!";
        }
        break;

    case SPELL_COLD_ARROW:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_FREEZING;
        ftc = 1;
        if (!AFF2_FLAGGED(victim, AFF2_FREEZING)) {
            to_vict = "You are consumed with coldness!";
            to_room = "$n starts shivering from coldness!";
        }
        break;

    case SPELL_FLAME_ARROW:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_BURNING;
        ftc = 2;
        if (!AFF2_FLAGGED(victim, AFF2_BURNING)) {
            to_vict = "You are engulfed in flames!";
            to_room = "$n is engulfed in flames!";
        }
        break;

    case SPELL_BARKSKIN:
        af.location = APPLY_AC;
        af.modifier = 4;
        af.duration =-2;// 3+lvD6(ch);
        to_vict = "You feel your skin hardening.";
        to_room = "$n's skin suddenly hardens.";
        break;

    case SPELL_STONESKIN:
        af.location = APPLY_AC;
        af.modifier = 6;
        af.duration = -2;//3+lvD6(ch);
        af.bitvector2 = AFF2_STONESKIN;
        to_room = "$n's skin turns into stone!";
        to_vict = "You feel your skin turning into stone!";

        break;

    case SPELL_MIRROR_IMAGE:
        af.duration = -2;//8;
        af.bitvector2 = AFF2_MIRRORIMAGE;
        to_vict = "Your image breaks up!";
        to_room = "$n breaks up into many images!!!";
        break;

    case SPELL_BLINK:
        af.duration =-2;// 8;
        af.bitvector2 = AFF2_BLINK;
        to_vict = "You quickly shift a few feet away. Undetected.";
        to_room = "You see $n shift a few feet away.";
        break;

    case SPELL_SHIELD:
        af.duration =-2;// 2+lvD4(ch);
        af.bitvector = AFF_SHIELD;
        to_vict = "You form a mystic shield around your body.";
        to_room = "Mystic shield forms around $n.";
        af.location = APPLY_AC;
        af.modifier = 4+lvD4(ch);
        break;

    case SPELL_DEFLECTION:
        af.duration =-2;//lvD8(ch)+2;
        af.bitvector = AFF_DEFLECTION;
        to_vict = "Deflection shield forms all around you.";
        to_room = "Deflection shield forms all around $n.";
        break;

    case SPELL_FORCE_FIELD:
        af.duration =-2;// lvD8(ch)+2;
        af.bitvector = AFF_FORCE_FIELD;
        to_vict = "Thin force field forms all around you.";
        to_room = "Force field forms all around $n.";
        break;  
        

    case SPELL_FIRE_SHIELD:
        af.duration =-2;// lvD8(ch)+2;
        af.bitvector = AFF_FIRE_SHIELD;
        to_vict = "Blazing shield of fire forms all around you.";
        to_room = "Shield of fire forms all around $n.";
        break;
     
     case PRAYER_SHIELD_OF_FAITH:
        af.duration =-2;// lvD8(ch)+2;
        af.bitvector3 = AFF3_SOF;
        if (GET_FAITH(ch)<200)
        {
        	send_to_char("Nothing happens.\r\n", ch);
        	return;
        }
        to_vict = "You are granted the shield of faith.";
        to_room = "$n is granted the shield of faith.";
        break;
    
    case SPELL_NAP:
        if ((GET_POS(victim) == POS_SLEEPING) || (GET_POS(victim) == POS_FIGHTING))
            return;
        if (GET_POS(victim) > POS_SLEEPING) {
            af.duration = 0;
            af.bitvector2 = AFF2_NAP;
            af.bitvector = AFF_SLEEP;
            send_to_char("You yawn... Zzzzzzzz...\r\n", victim);
            to_room = "$n goes to sleep.";
            GET_POS(victim) = POS_SLEEPING;
        }
        break;

    case SPELL_REGENERATE:
        af.duration =-2;// 1+lvD8(ch);
        af.bitvector2 = AFF2_REGENERATE;
        to_room ="$n suddenly becomes very vivid.";
        to_vict = "You feel your body cells regenerating faster.";
        break;

    case SPELL_DEATHDANCE:
        if (ch!=victim && !is_same_group(ch, victim))
        {
            send_to_char("This spell works only on yourself or members of your group.\n", ch);
            return;
        }

        af.duration = 0;
        af.bitvector = AFF_DEATHDANCE;
        to_vict = "You feel your life take on a whole new meaning....";
        to_room = "A wave of death dances forth from $n!";
        break;

    case SPELL_WRAITHFORM:
        af.duration =-2;// 2 + lvD8(ch);
        af.bitvector = AFF_PASSDOOR;
        to_vict = "You turn translucent!";
        to_room = "$n turns translucent!";
        break;

    case SPELL_FIRESTORM:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_BURNING;
        ftc = 2;
        if (!AFF2_FLAGGED(victim, AFF2_BURNING)) {
            to_vict = "You are engulfed in flames!";
            to_room = "$n is engulfed in flames!";
        }
        break;

    case SPELL_FLAMESTRIKE:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_BURNING;
        ftc = 2;
        if (!AFF2_FLAGGED(victim, AFF2_BURNING)) {
            to_vict = "You are engulfed in flames!";
            to_room = "$n is engulfed in flames!";
        }
        break;

    case SPELL_FIREBALL:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_BURNING;
        ftc = 2;
        if (!AFF2_FLAGGED(victim, AFF2_BURNING)) {
            to_vict = "You are engulfed in flames!";
            to_room = "$n is engulfed in flames!";
        }
        break;


    case SPELL_FIRE_BREATH:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_BURNING;
        ftc = 2;
        if (!AFF2_FLAGGED(victim, AFF2_BURNING)) {
            to_vict = "You are engulfed in flames!";
            to_room = "$n is engulfed in flames!";
        }
        break;

    case SPELL_FROST_BREATH:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_FREEZING;
        ftc = 1;
        if (!AFF2_FLAGGED(victim, AFF2_FREEZING)) {
            to_vict = "You are consumed with coldness!";
            to_room = "$n starts shivering from coldness!";
        }
        break;

    case SPELL_ACID_BREATH:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_ACIDED;
        if (!AFF2_FLAGGED(victim, AFF2_ACIDED)) {
            to_vict = "You are drenched in acid.";
            to_room = "$n is drenched in acid!";
        }
        break;

    case SPELL_ACID_BLAST:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_ACIDED;
        if (!AFF2_FLAGGED(victim, AFF2_ACIDED)) {
            to_vict = "You are drenched in acid.";
            to_room = "$n is drenched in acid!";
        }
        break;

    case SPELL_BURN:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_BURNING;
        ftc = 2;
        if (!AFF2_FLAGGED(victim, AFF2_BURNING)) {
            to_vict = "You are engulfed in flames!";
            to_room = "$n is engulfed in flames!";
        }
        break;

    case SPELL_FREEZE:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_FREEZING;
        ftc = 1;
        if (!AFF2_FLAGGED(victim, AFF2_FREEZING)) {
            to_vict = "You are consumed with coldness!";
            to_room = "$n starts shivering from coldness!";
        }
        break;

    case SPELL_ACID:
        nof = 0;
        af.duration = 1;
        accum_duration = TRUE;
        af.bitvector2 = AFF2_ACIDED;
        if (!AFF2_FLAGGED(victim, AFF2_ACIDED)) {
            to_vict = "You are drenched in acid.";
            to_room = "$n is drenched in acid!";
        }
        break;


    case SPELL_ENH_HEAL:
        af.duration = -2;//24;
        af.bitvector2 = AFF2_ENH_HEAL;
        to_room = "$n briefly shines blue.";
        to_vict = "Your body starts healing faster.";
        break;

    case SPELL_ENH_MANA:
        af.duration =-2;// 24;
        af.bitvector2 = AFF2_ENH_MANA;
        to_room = "$n briefly shines green.";
        to_vict = "You feel much more intuned to the flow of mana.";
        break;

    case SPELL_ENH_MOVE:
        af.duration =-2;// 24;
        af.bitvector2 = AFF2_ENH_MOVE;
        to_room = "$n briefly shines yellow.";
        to_vict = "You feel much more restful.";
        break;

    case SPELL_HASTE:
        af.duration =-2;// 2+lvD8(ch);
        af.bitvector2 = AFF2_HASTE;
        to_vict = "You speed up your movements.";
        to_room = "$n starts moving faster.";
        affect_from_char(victim, SPELL_SLOW);
        break;

    case SPELL_SLOW:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_DEX;
        af.modifier = -3;
        af.duration =-2;// 2;
        af.bitvector3 = AFF3_SLOW;
        to_vict = "Your movements start slowing down!";
        to_room = "$n starts slowing down!";
        affect_from_char(victim, SPELL_HASTE);
        affect_from_char(victim, SPELL_ADRENALIN);
        break;


    case SPELL_WEB:
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_DEX;
        af.modifier = -4;
        af.duration = 2;
        af.bitvector3 = AFF3_WEB;
        to_vict = "You are covered by a sticky web!";
        to_room = "$n is covered by a sticky web!";
        break;

    case SPELL_AGILITY:
        af.location = APPLY_DEX;
        af.modifier = 3;
        af.duration =-2;// 3+level/10;
        to_vict = "You feel very agile!";
        to_room = "$n becomes lot more agile!";
        break;


    case SPELL_SHAMROCK:
        af.duration = -2;//10;
        af.bitvector3 = AFF3_SHAMROCK;
        to_vict = "Pleasent feelings overcome your body.";
        to_room = "$n seems to be very happy now.";
        break;

    case SPELL_RUNIC_SHELTER:
        af.duration =-2;// 5;
        af.bitvector3 = AFF3_SHELTER;
        af.location=APPLY_AC;
        af.modifier=10;
        to_vict = "You form a magical shelter around your body.\r\nYou feel safe from magic.";
        to_room = "$n forms a magical shelter around $s body.";
        break;


    case SPELL_PLAGUE:
        if (mag_savingthrow(victim, savetype, ch) || IS_SHOPKEEPER(victim)) {//MOB_FLAGGED(victim, MOB_SENTINEL)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.bitvector3 = AFF3_PLAGUE;
        af.duration = -1;
        to_vict = "You suddenly feel very, very, very bad. You have PLAGUE!";
        to_room = "$n has PLAGUE!!! Get out!";
        break;

    case SPELL_FEEBLEMIND:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_INT;
        af.modifier = number(4, 6)-GET_INT(victim);
        af.duration = 0;
        to_vict = "You are feeling your intellect wither!";
        to_room = "$n turns around confused, wondering what happened to $m.";
        break;

    case SPELL_MELLON_COLLIE:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_HITROLL;
        af.modifier = -5;
        af.duration = -2;//4;
        af2.location = APPLY_DEX;
        af2.modifier = -3;
        af2.duration = -2;//4;
        af3.location = APPLY_SAVING_SPELL;
        af3.modifier = 10;
        af3.duration =-2;// 4;
        to_vict = "You suddenly start feeling depressed.";
        to_room = "$n sighs very loudly in a depressing manner.";
        break;

    case SPELL_SHROUD_OF_DARKNESS:
        if (mag_savingthrow(victim, savetype, ch)) {
            send_to_char(NOEFFECT, ch);
            return;
        }
        if (!ok_kill(ch, victim))
            return;
        af.location = APPLY_DEX;
        af.modifier = -2;
        af.duration = -2;//3;
        af2.location = APPLY_INT;
        af2.modifier = -2;
        af2.duration = -2;//3;
        af3.location = APPLY_HITROLL;
        af3.modifier = -(GET_HITROLL(victim)/6);
        af3.duration = -2;//3;
        to_vict = "A shroud of darkness forms above your head!\r\nYou suddenly don't feel very well.";
        to_room = "A shroud of darkness forms above $n's head!";
        break;

    case SPELL_DIVINITY:
        af.location = APPLY_REGENERATION;
        af.modifier = 15;
        af.duration = -2;//4;
        af2.location = APPLY_DAMROLL;
        af2.modifier = 8;
        af2.duration = -2;//4;
        af3.location = APPLY_HITROLL;
        af3.modifier = 8;
        af3.duration = -2;//4;
        to_vict = "You are embued with a divine light coming seemingly from nowhere!";
        to_room =  "$n is embued with a divine light coming seemingly from nowhere!";
        break;

    case SPELL_SHILLELAGH:
        af.location = APPLY_REGENERATION;
        af.modifier = 4+level/10;
        af.duration = -2;//10;
        to_vict = "You are bathed in a bright rainbow of colors!";
        to_room =  "$n is bathed in a bright rainbow of colors!";
        break;

    case SPELL_INTESIFY:
        af.duration =-2;// 0;
        af.bitvector3 = AFF3_INTESIFY;
        to_room = "$n suddenely flashes with a white light.";
        to_vict = "You flash with a bright white light.";
        break;

    case SPELL_ADRENALIN:
        af.location=APPLY_DEX;
        af.modifier=1;
        af.duration =-2;// 2+lvD8(ch);
        af.bitvector2 = AFF2_ADRENALIN;
        to_room = "$n is adrenalinized!";
        to_vict = "You feel your blood rushing!";
        affect_from_char(victim, SPELL_SLOW);
        break;


    case SPELL_AURA:
        af.duration =-2;// 0;
        af.bitvector = AFF_SILVER;
        to_vict = "You quickly form a shimmering field around your body.";
        to_room = "$n forms a shimmering field around $s body.";
        break;


        /* end of spells */
    }

    if (savetype==SPELL_FORTRESS) {
        accum_affect = FALSE;
        accum_duration = FALSE;
        af.duration=3+lvD4(ch);
        af2.duration=3+lvD4(ch);
        af3.duration=3+lvD4(ch);
    }




    /* If this is a mob that has this affect set in its mob file, do not
     * perform the affect.  This prevents people from un-sancting mobs by
     * sancting them and waiting for it to fade, for example. */
    if (nof && IS_NPC(victim) && (IS_AFFECTED(victim, af.bitvector | af2.bitvector |af3.bitvector) ||
                                  IS_SET(AFF2_FLAGS(victim), af.bitvector2 | af2.bitvector2 |af3.bitvector2) ||
                                  IS_SET(AFF3_FLAGS(victim), af.bitvector3 | af2.bitvector3|af3.bitvector3)) &&
            !affected_by_spell(victim, spellnum)) {
        if (savetype!=SPELL_FORTRESS) send_to_char(NOEFFECT, ch);
        return;
    }

    if (ftc == 1)
        killfire(victim);
    if (ftc == 2)
        killcold(victim);

    if (savetype==SAVING_ROD && af.duration==-2)
    {af.duration=level/10+1;
        af2.duration=level/10+1;
        af3.duration=level/10+1;
    }
    if (IS_SUPERMOB(ch))
    {
        af.duration=number(1, 4);
        af2.duration=af.duration;
        af3.duration=af.duration;
    }

    /* If the victim is already affected by this spell, and the spell does
         * not have an accumulative effect, then fail the spell. */

    if (affected_by_spell(victim, spellnum) && !(accum_duration || accum_affect)) {
        if (savetype!=SPELL_FORTRESS) send_to_char(NOEFFECT, ch);
        return;
    }
    //accum_duration = FALSE;
    if (spellnum!=SPELL_POISON)
    {
        affect_join(victim, &af, accum_duration, FALSE, accum_affect, FALSE);
        if (af2.bitvector || af2.location)
            affect_join(victim, &af2, accum_duration, FALSE, accum_affect, FALSE);
        if (af3.bitvector || af3.location)
            affect_join(victim, &af3, accum_duration, FALSE, accum_affect, FALSE);

        if (af.duration==-2)
            //add_leech(ch, victim, spellnum, (savetype==SAVETYPE_GROUP? mag_manacost(ch, spellnum)/2:mag_manacost(ch, spellnum)));
            add_leech(ch, victim, spellnum,(savetype>=0?mag_manacost(ch, spellnum):-savetype));
    }
    else
        apply_poison(victim, 2);

    if (spellnum==SPELL_DEATHDANCE)
        GET_HIT(victim)=MIN(GET_HIT(victim), GET_MAX_HIT(victim)/2);

    if (to_char != NULL)
        act(to_char, FALSE, ch, 0, victim, TO_CHAR);
    if (to_vict != NULL)
        act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
    if (to_room != NULL)
        act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */

void            perform_mag_groups(int level, struct char_data * ch,
                                   struct char_data * tch, int spellnum, int savetype)
{

    if (ch==tch || can_cast(ch, spellnum))
    {

        savetype=-mag_manacost(ch, spellnum);
        if (ch!=tch)
            GET_MANA(ch)-=mag_manacost(ch, spellnum);

        switch (spellnum) {

        case SPELL_GROUP_POWER_HEAL:
            mag_points(level, ch, tch, SPELL_POWER_HEAL, savetype);

            break;
        case SPELL_GROUP_HEAL:

            mag_points(level, ch, tch, SPELL_HEAL, savetype);
            break;
        case SPELL_HEALING_TOUCH:
            mag_points(level, ch, tch, SPELL_HEALING_TOUCH, savetype);
            break;
        case SPELL_SURCEASE:
            mag_points(level, ch, tch, SPELL_SURCEASE, savetype);
            break;
            /*    case SPELL_GROUP_ARMOR:
                    mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
                    SET_BIT(AFF3_FLAGS(tch),AFF3_GR_ARMOR);
                    break;*/
        case SPELL_GROUP_RECALL:
            spell_recall(level, ch, tch, NULL, 0);
            break;
            /*    case SPELL_GROUP_FLY:
                    mag_affects(level, ch, tch, SPELL_FLY, savetype);
                    SET_BIT(AFF3_FLAGS(tch),AFF3_GR_FLY);
                    break;*/
        case SPELL_GROUP_INVIS:
            mag_affects(level, ch, tch, SPELL_INVISIBLE, savetype);
            SET_BIT(AFF3_FLAGS(tch),AFF3_GR_INVIS);
            break;
            /*    case SPELL_GROUP_WATBREATH:
                    mag_affects(level, ch, tch, SPELL_WATERBREATH, savetype);
                    SET_BIT(AFF3_FLAGS(tch),AFF3_GR_WATER);
                    break;
                case SPELL_FEAST_ALL:
                    mag_affects(level, ch, tch, SPELL_CREATE_FOOD, savetype);
                    break;*/
        case SPELL_NAP:
            mag_affects(level, ch, tch, SPELL_NAP, savetype);
            break;
        case PRAYER_INSPIRATION:
            mag_affects(level, ch, tch, PRAYER_INSPIRATION, savetype);
            break;    
        case SPELL_GROUP_FORCE_FIELD:
            mag_affects(level, ch, tch, SPELL_FORCE_FIELD, savetype);
            SET_BIT(AFF3_FLAGS(tch),AFF3_GR_FORCE);
            break;
        case SPELL_GROUP_FIRE_SHIELD:
            mag_affects(level, ch, tch, SPELL_FIRE_SHIELD, savetype);
            SET_BIT(AFF3_FLAGS(tch),AFF3_GR_FIRE);
            break;
        case SPELL_GROUP_DEFLECTION:
            mag_affects(level, ch, tch, SPELL_DEFLECTION, savetype);
            SET_BIT(AFF3_FLAGS(tch),AFF3_GR_DEF);
            break;
        }
    }
}


void apply_poison(struct char_data *ch, int duration)
{
    struct affected_type af;

    if (IS_TROLL(ch))
    {
        send_to_char("You realize that it has no affect on you.\r\n", ch);
        return;
    }

    af.type = SPELL_POISON;
    af.duration = duration;
    af.modifier = -1;
    af.location = APPLY_STR;
    af.bitvector = AFF_POISON;
    af.bitvector2 = 0;
    af.bitvector3 = 0;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
    af.modifier = -1;
    af.location = APPLY_CON;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
}




/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void            mag_groups(int level, struct char_data * ch, int spellnum, int savetype)
{
    struct char_data *tch,
                *k;
    struct follow_type *f,
                *f_next;

    if (ch == NULL || DEAD(ch))
        return;

    if (!IS_AFFECTED(ch, AFF_GROUP) && spellnum!=SPELL_HEALING_TOUCH && spellnum!=SPELL_SURCEASE)
        return;
    if (ch->master != NULL)
        k = ch->master;
    else
        k = ch;
for (f = k->followers; f; f = f_next) {
        f_next = f->next;
        tch = f->follower;
        if (tch->in_room != ch->in_room)
            continue;
        if (!IS_AFFECTED(tch, AFF_GROUP))
            continue;
        if (ch == tch)
            continue;
        perform_mag_groups(level, ch, tch, spellnum, savetype);
    }

    if ((k != ch) && IS_AFFECTED(k, AFF_GROUP))
        perform_mag_groups(level, ch, k, spellnum, savetype);
    perform_mag_groups(level, ch, ch, spellnum, savetype);
}
extern char    *dirs[];
extern int      rev_dir[];

void            damage_scanned_chars(struct char_data * list, struct char_data * ch,
                                     int distance, int door, int level, int spellnum)
{
    struct char_data *tch;
    int             count = 0;
    int             dam = 0;
    char            mybuf[200];
    if (!list)
        return;

    sprintf(mybuf, "The crack in the ground comes from the %s.", dirs[rev_dir[door]]);
    act(mybuf, FALSE, list, NULL, NULL, TO_ROOM);
    act(mybuf, FALSE, list, NULL, NULL, TO_CHAR);

    for (tch = list; tch; tch = tch->next_in_room) {
        if (tch == ch)
            continue;
        if (ROOM_FLAGGED(tch->in_room, ROOM_PEACEFUL))
            continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
            continue;
        if (!CAN_MURDER(ch, tch) || is_same_group(ch, tch))
            continue;
        if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
            continue;


        dam = dice(5, level);
        if (AFF_FLAGGED(tch, AFF_FLYING) || AFF_FLAGGED(tch, AFF_WATERWALK))
            continue;

        if (IS_NPC(tch) && IS_SHOPKEEPER(tch))//MOB_FLAGGED(tch, MOB_SENTINEL))
            dam = 0;
        if (dam) {
            act("$n falls down, hurting $mself!", FALSE, tch, NULL, NULL, TO_ROOM);
            send_to_char("You fall down, hurting yourself!\r\n", tch);
            damage(ch, tch, MIN(1000, level + number(1, level)), spellnum, NULL);
        }
        if (IS_NPC(tch) && GET_POS(ch) > POS_STUNNED) {
            SET_BIT(MOB_FLAGS(tch), MOB_MEMORY);
            remember(tch, ch);
            if (!IS_SHOPKEEPER(tch))//!MOB_FLAGGED(tch, MOB_SENTINEL))
                HUNTING(tch) = ch;
        }
    }

}



void            mag_surrounds(int level, struct char_data * ch, int spellnum)
{
    int             door;
    char            mybuf[200];

    for (door = 0; door < NUM_OF_DIRS - 2; door++)      /* don't scan up/down */
        if (EXIT(ch, door) && (EXIT(ch, door)->to_room != NOWHERE) &&
                (SECT(EXIT(ch, door)->to_room) < 6) && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
            sprintf(mybuf, "The earth trembles and shivers!\r\nCrack opens in the ground leaving to the %s.", dirs[door]);
            act(mybuf, FALSE, ch, NULL, NULL, TO_ROOM);
            act(mybuf, FALSE, ch, NULL, NULL, TO_CHAR);
            if (world[EXIT(ch, door)->to_room].people)
                damage_scanned_chars(world[EXIT(ch, door)->to_room].people, ch, 0, door, level, spellnum);
            if (_2ND_EXIT(ch, door) && _2ND_EXIT(ch, door)->to_room !=
                    NOWHERE && (SECT(_2ND_EXIT(ch, door)->to_room) < 6) && !IS_SET(_2ND_EXIT(ch, door)->exit_info, EX_CLOSED)) {
                /* check the second room away */
                if (world[_2ND_EXIT(ch, door)->to_room].people)
                    damage_scanned_chars(world[_2ND_EXIT(ch, door)->to_room].people, ch, 1, door, level, spellnum);
                if (_3RD_EXIT(ch, door) && _3RD_EXIT(ch, door)->to_room !=
                        NOWHERE && (SECT(_3RD_EXIT(ch, door)->to_room) < 6) && !IS_SET(_3RD_EXIT(ch, door)->exit_info, EX_CLOSED)) {
                    /* check the third room */
                    if (world[_3RD_EXIT(ch, door)->to_room].people)
                        damage_scanned_chars(world[_3RD_EXIT(ch, door)->to_room].people, ch, 2, door, level, spellnum);
                }
            }
        }
}


void            mag_ragnarok(struct char_data * ch, int level, int spellnum)
{
    struct char_data *tch,
                *i,
                *next_char;
    int sum=0;
for (i = character_list; i; i = next_char) {
        next_char = i->next;

        if (world[ch->in_room].zone == world[i->in_room].zone) {
            tch = i;
            if (tch == ch)
                continue;
            if (ROOM_FLAGGED(tch->in_room, ROOM_PEACEFUL))
                continue;
            if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
                continue;
            if (!CAN_MURDER(ch, tch))
                continue;
            if (is_same_group(ch, tch))
                continue;
            if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
                continue;
            if (IS_NPC(tch) && IS_SHOPKEEPER(tch))//MOB_FLAGGED(tch, MOB_SENTINEL))
                continue;
            if (mag_savingthrow(tch, SAVING_SPELL, ch))
                continue;

            if (!IS_NPC(tch))
                send_to_char("You kneel in pain as something unexplainable completly destroys you! AAAAAAAAAARGH!\r\n", tch);
            damage(ch, tch, 7 * level, spellnum, NULL);
            if ((++sum)>10)
            {
                process_data();
                sum=0;
            }
        }

    }
}







void            perform_mag_casters(int level, struct char_data * ch,
                                    struct char_data * victim, struct obj_data *obj, int spellnum, int savetype)
{
    char            bufma[200];
    switch (spellnum) {
    case SPELL_CONJURING_BEAM:
        if (GET_HIT(victim)<2*GET_MAX_HIT(victim)/3)
        {
            send_to_char("Your victim is under two-thirds of it's hitpoints.\r\n", ch);
            return;
        }
        if (ROOM_FLAGGED(victim->in_room, ROOM_PRIVATE))
        {
            send_to_char("Room your target is in, is PRIVATE!\r\n", ch);
            return;
        }
        if (ROOM_FLAGGED(victim->in_room, ROOM_GODROOM))
            victim=ch;
        act("$n makes a gesture and conjures a flashing ball of light.", FALSE, ch, NULL, victim, TO_ROOM);
        act("You make a gesture and conjure a flashing ball of light.", FALSE, ch, NULL, victim, TO_CHAR);
        sprintf(bufma, "holler Saramon gito tulma %s!", GET_NAME(victim));
        command_interpreter(ch, bufma);
        do_action(ch, "", find_command("grin"), 0);
        act("Ball of light flies up.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Ball of light flies up.", FALSE, ch, NULL, NULL, TO_CHAR);
        act("\r\nBlue ray of cosmic energy strikes $n!", FALSE, victim, NULL, NULL, TO_NOTVICT);
        act("\r\nBlue ray of cosmic energy strikes you!", FALSE, victim, NULL, NULL, TO_VICT);
        mag_damage(level, ch, victim, spellnum, savetype);
        break;
    case SPELL_RESSURECTION:
        act("$n steps back and rises $s hands.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You step back and rise your hands.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler Acronomicon tendera latus suprema %s!", GET_NAME(victim));
        command_interpreter(ch, bufma);
        act("\r\n$n briefly flashes red then blue.", FALSE, victim, NULL, NULL, TO_NOTVICT);
        mag_points(level, ch, victim, spellnum, savetype);
        send_to_char("\r\nYou begin a whole new life...\r\n", victim);
        do_action(victim, "", find_command("grin"), 0);
        break;
    case SPELL_GIVE_LIFE:
        act("$n's eyes briefly glow white.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your eyes briefly glow white.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler Homenus flacum %s, solstim donarum!", GET_NAME(victim));
        command_interpreter(ch, bufma);
        act("$n opens $s hands wide.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You open your hands wide.", FALSE, ch, NULL, NULL, TO_CHAR);
        act("\r\nPuff of blue smoke surrounds $n.", FALSE, victim, NULL, NULL, TO_VICT);
        mag_affects(level, ch, victim, spellnum, savetype);
        send_to_char("\r\nYou feel like a hero!\r\n", victim);
        break;
    case SPELL_PRISMATIC_SPHERE:
        act("$n's eyes briefly glow blue.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your eyes briefly glow blue.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler Mingram sortum spasmis %s!", GET_NAME(victim));
        command_interpreter(ch, bufma);
        act("$n opens $s hands wide.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You open your hands wide.", FALSE, ch, NULL, NULL, TO_CHAR);
        act("\r\nPrismatic sphere of light surrounds $n!", FALSE, victim, NULL, NULL, TO_VICT);
        mag_affects(level, ch, victim, spellnum, savetype);
        send_to_char("\r\nPrismatic sphere of light forms around you!\r\n", victim);
        do_action(victim, "", find_command("grin"), 0);
        break;
    case SPELL_ULTRA_DAMAGE:
        act("$n's points $s finger up.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your point your finger up.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler Holacum %s, ptatis vigro analem!", GET_NAME(victim));
        command_interpreter(ch, bufma);
        act("$n rises $s hands.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You rise your hands.", FALSE, ch, NULL, NULL, TO_CHAR);
        act("\r\n$n starts looking dangerous. VERY dangerous...", FALSE, victim, NULL, NULL, TO_VICT);
        mag_affects(level, ch, victim, spellnum, savetype);
        send_to_char("\r\nYou feel invincible!\r\n", victim);
        do_action(victim, "", find_command("grin"), 0);
        break;
    case SPELL_PETRIFY:
        act("$n's looks up and hollers some words.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your look up and holler some words.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler Makne skamus %s noste lapum!", GET_NAME(victim));
        command_interpreter(ch, bufma);
        act("$n spreads $s hands.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You spread your hands.", FALSE, ch, NULL, NULL, TO_CHAR);
        do_action(ch, "", find_command("grin"), 0);
        act("\r\n$n turns into STONE!", FALSE, victim, NULL, NULL, TO_VICT);
        mag_affects(level, ch, victim, spellnum, savetype);
        send_to_char("\r\nYou are petrified! You can't move!\r\n", victim);
        break;
    case SPELL_MATERIALIZE:
        if (not_in_arena(ch))
            return;
        if (IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC))
        {
            send_to_char("It's magic aura is too strong\r\n!", ch);
            return;
        }
        if (obj->carried_by || obj->worn_by || obj->in_obj)
        {
            send_to_char("It must be lying on the ground!\r\n", ch);
            return;
        }
        if (ROOM_FLAGGED(obj->in_room, ROOM_PRIVATE))
        {
            send_to_char("Room that object is in, is PRIVATE!\r\n", ch);
            return;
        }
        act("$n's looks up and hollers some words.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your look up and holler some words.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler Apperamus, apperamus hovdelum!");
        command_interpreter(ch, bufma);
        act("$n spreads $s hands.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You spread your hands.", FALSE, ch, NULL, NULL, TO_CHAR);
        do_action(ch, "", find_command("giggle"), 0);

        obj_from_room(obj);
        obj_to_room(obj, ch->in_room);
        act("\r\n$p suddenly materializes!", FALSE, NULL, obj, NULL, TO_VICT);
        break;
    case SPELL_AREA_EARTHQUAKE:
        act("$n's throws some dust in the air.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your throw some dust in the air.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "shout Ormen terra motis!");
        command_interpreter(ch, bufma);
        act("\r\nThe dust falls down...", FALSE, ch, NULL, NULL, TO_ROOM);
        act("\r\nThe dust falls down...", FALSE, ch, NULL, NULL, TO_CHAR);
        mag_surrounds(level, ch, spellnum);
        break;
    case SPELL_CREATE_RAIN:
        if ((weather_info.sky == SKY_RAINING) || (weather_info.sky == SKY_LIGHTNING)) {
            send_to_char("The weather is already bad.\r\n", ch);
            break;
        }
        act("$n's throws some water high in the air.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your throw some water high in the air.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler Kalakhum pfandra!");
        command_interpreter(ch, bufma);
        act("\r\nThe weather changes...", FALSE, ch, NULL, NULL, TO_ROOM);
        act("\r\nThe weather changes...", FALSE, ch, NULL, NULL, TO_CHAR);
        while ((weather_info.sky != SKY_RAINING) && (weather_info.sky != SKY_LIGHTNING)) {
            weather_info.change -= 50;
            weather_change();
        }
        break;
    case SPELL_RAGNAROK:
        act("$n's starts waving $s hands like a lunatic.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your start waving your hands like a lunatic.", FALSE, ch, NULL, NULL, TO_CHAR);
        act("$n makes a feather and drops in the air.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You make a feather and drop it in the air.", FALSE, ch, NULL, NULL, TO_CHAR);
        sprintf(bufma, "holler RAAAAAAAAA-GNAAAAAAAA-ROOOOOOOOK!");
        command_interpreter(ch, bufma);
        act("\r\nThe feather slowly falls down...\r\n", FALSE, ch, NULL, NULL, TO_ROOM);
        act("\r\nThe feather slowly falls down...\r\n", FALSE, ch, NULL, NULL, TO_CHAR);
        mag_ragnarok(ch, level, spellnum);
        break;
    case SPELL_HOLY_TOUCH:
        //if (GET_EXP(victim)>total_exp(GET_LEVEL(victim)-1))
        if (GET_EXP(victim)>=0)
        {
            send_to_char("As you start the cast you realize ther is no need for that.\r\n", ch);
            return;
        }
        act("$n looks $N with a wide grin.", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("You look $N with a wide smile.", FALSE, ch, NULL, victim, TO_CHAR);
        sprintf(bufma, "shout Be fundrel %s, nostrus ami!", GET_NAME(victim));
        command_interpreter(ch, bufma);
        act("$n spreads $s hands.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You spread your hands.", FALSE, ch, NULL, NULL, TO_CHAR);
        act("\r\n$n is shaken by a mysterious force!", FALSE, victim, NULL, NULL, TO_VICT);
        send_to_char("\r\nYou suddenly feel... A very nice feeling indeed.\r\n", victim);
        act("A wide grin crosses $n's face.", FALSE, victim, NULL, NULL, TO_ROOM);
        //GET_EXP(victim)=total_exp(GET_LEVEL(victim)-1);
        GET_EXP(victim)=0;
        break;
    }

}

void            mag_casters(int level, struct char_data * ch, struct char_data * victim, struct obj_data *obj,int spellnum, int savetype)
{
    struct char_data *tch,
                *k;
    struct follow_type *f,
                *f_next;
    int             totlev = 0;
    int             mana;

    if (ch == NULL || DEAD(ch))
        return;         
    
    send_to_char("Ritual spells are disabled for balancing reason, until further notice.\r\n", ch);
    return;    

    if (!IS_AFFECTED(ch, AFF_GROUP))
        return;
    if (ch->master != NULL)
        k = ch->master;
    else
        k = ch;
if (k != ch && (IS_AFFECTED(k, AFF_GROUP)) && (GET_CLASS_NUM(k)==GET_CLASS_NUM(ch)) && (GET_LEVEL(k) >= spell_info[spellnum].mana_max)) {
        tch = k;
        totlev += GET_LEVEL(tch);
        mana = hasenough(tch, spellnum);
        GET_MANA(tch) = MAX(0, MIN(GET_MAX_MANA(tch), GET_MANA(tch) - mana));
        act("You connect your mind with $N, placing your energy at $s disposal.", FALSE, tch, NULL, ch, TO_CHAR);
        act("$n sinks into deep thoughts, placing $s energy to $N's disposal.", FALSE, tch, NULL, ch, TO_NOTVICT);
        act("$n connects $s mind with you, placing $s energy to your disposal.", FALSE, tch, NULL, ch, TO_VICT);
    }
    for (f = k->followers; f; f = f_next) {
        f_next = f->next;
        tch = f->follower;
        if (tch->in_room != ch->in_room)
            continue;
        if (!IS_AFFECTED(tch, AFF_GROUP))
            continue;
        if (ch == tch)
            continue;
        if (!(GET_CLASS_NUM(tch)==GET_CLASS_NUM(ch)))
            continue;
        if (GET_LEVEL(tch) < spell_info[spellnum].mana_max)
            continue;
        totlev += GET_LEVEL(tch);
        mana = hasenough(tch, spellnum);
        GET_MANA(tch) = MAX(0, MIN(GET_MAX_MANA(tch), GET_MANA(tch) - mana));
        act("You connect your mind with $N, placing your energy at $s disposal.", FALSE, tch, NULL, ch, TO_CHAR);
        act("$n sinks into deep thoughts, placing $s energy to $N's disposal.", FALSE, tch, NULL, ch, TO_NOTVICT);
        act("$n connects $s mind with you, placing $s energy to your disposal.", FALSE, tch, NULL, ch, TO_VICT);
    }

    totlev += GET_LEVEL(ch);
    perform_mag_casters(totlev, ch, victim, obj, spellnum, savetype);

}


/*
 * mass spells affect every creature in the room except the caster.
 *
 * No spells of this class currently implemented as of Circle 3.0.
 */

void            mag_masses(int level, struct char_data * ch, int spellnum, int savetype)
{
    struct char_data *tch,
                *tch_next;

for (tch = world[ch->in_room].people; tch; tch = tch_next) {
        tch_next = tch->next_in_room;
        if (tch == ch || !CAN_MURDER(ch, tch) || !CAN_SEE(ch, tch))
            continue;

        switch (spellnum) {
        case SPELL_MASS_CHARM:
            spell_charm(level, ch, tch, NULL, "");
            break;
        case SPELL_MASS_SLEEP:
            mag_affects(GET_LEVEL(ch), ch, tch, SPELL_SLEEP, 1);
            break;
        }
    }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/


void            mag_areas(int level, struct char_data * ch, int spellnum, int savetype)
{
    struct char_data *tch,
                *next_tch;
    char           *to_char = NULL;
    char           *to_room = NULL;
    int 	sum=0;

    if (ch == NULL || DEAD(ch))
        return;

    /* to add spells to this fn, just add the message here plus an entry in
     * mag_damage for the damaging part of the spell. */
switch (spellnum) {
    case SPELL_EARTHQUAKE:
        to_char = "You gesture and the earth begins to shake all around you!";
        to_room = "$n gracefully gestures and the earth begins to shake violently!";
        break;
    case SPELL_FIRESTORM:
        to_room = "$n waves $s hands and conjures a heating firestorm!";
        to_char = "You make a small gesture and powers of fire obey you!";
        break;
    case SPELL_FLAMESTRIKE:
        to_room = "$n smirks and conjures a flaming pillar!";
        to_char = "You smirk and conjure a flaming pillar!";
        break;
    case SPELL_MINUTE_METEOR:
        to_room = "$n looks up and conjures a million of tiny meteors!!";
        to_char = "You look up and conjure a millions of tiny meteors!!";
        break;
    case SPELL_POWER_OF_NATURE:
        to_room = "$n rises $s head and calls upon some unknown forces!";
        to_char = "You rise your head and call upon the Powers of Nature!";
        break;
    case SPELL_HOLY_WORD:
        to_room = "$n looks up and calmly utters some words...";
        to_char = "You look up and calmly utter some words...";
        break;
    case SPELL_FIRE_BREATH:
        to_room = "With a draconian roar, $n breaths fire into the room!";
        to_char = "With a draconian roar, you toast everything in the room!";
        break;
    case SPELL_GAS_BREATH:
        to_room = "With a draconian roar, $n fills the room with deadly vapors!";
        to_char = "You cast a deadly spell, filling the room with vapors from hell!";
        break;
    case SPELL_FROST_BREATH:
        to_room = "With a mighty roar, $n freezes everyone with a breath of ice!";
        to_char = "You roar, filling the room with a deadly stream of ice!";
        break;
    case SPELL_ACID_BREATH:
        to_room = "Spittle stream forth from the mouth of $n, eroding everything!";
        to_char = "You breath acid into the room!";
        break;
    case SPELL_LIGHTNING_BREATH:
        to_room = "$n roars, spraying bolts of electricity everywhere!";
        to_char = "You open your mouth and generate electricity!";
        break;
    case SPELL_CONE_OF_COLD:
        to_room = "$n sniffs and launches a deadly cone of super-cooled ice!";
        to_char = "You sniff as you conjure a cone of super-cooled ice!";
        break;
    case SPELL_TORNADO:
        to_room = "$n waves $s hands and conjures a mighty tornado!";
        to_char = "You wave your hands and conjure a might tornado!";
        break;
    case SPELL_ARROW_RAIN:
        to_room = "$n creates rain of wooden arrows!";
        to_char = "You create a rain of wooden arrows!";
        break;
    case SPELL_SUNRAY:
        to_room = "$n opens $s hands and forms an intense beam of light!";
        to_char = "You open your hands and form an intense beam of light!";
        break;
                
    case PRAYER_BLINDING_LIGHT:
        to_room = "As $n spreads $s hands, beam of intense light fills the room!";
        to_char = "As you spreads out your hands, beam of intense light fills the room!";
        break;    

    case SPELL_AREA_LIGHTNING:
        if (!OUTSIDE(ch))
        {
            send_to_char("You couldn't call the lightings in here!\r\n",ch);
            return;
        };

        if (!(weather_info.sky == SKY_LIGHTNING)) {
            send_to_char("You see no lightnings in the sky.\r\n",ch);
            return;
        };

        /*    to_room ="With an almost fural scream, $n conjures a chain of lightning!";
            to_char ="With a raging scream you conjure a chain of lightning toward your enemies!";*/
        break;
    case SPELL_BLADEBARRIER:
        to_room = "$n laughs and puts up a BLADE BARRIER!";
        to_char = "You concentrate on swords.  Swords. SWORDS!!!";
        break;
    }
    if (to_char != NULL)
        act(to_char, FALSE, ch, 0, 0, TO_CHAR);
    if (to_room != NULL)
        act(to_room, FALSE, ch, 0, 0, TO_ROOM);


    for (tch = world[ch->in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;

        /* The skips: 1: the caster 2: immortals 3: if no pk on this mud,
         * skips over all players 4: pets (charmed NPCs) players can only hit
         * players in CRIMEOK rooms 4) players can only hit charmed mobs in
         * CRIMEOK rooms */

        if (tch == ch)
            continue;
        if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
            continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
            continue;
        if (!CAN_MURDER(ch, tch))
            continue;
        if (is_same_group(ch, tch))
            continue;
        if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM) && tch->master && !IS_NPC(tch->master) && !CAN_MURDER(ch, tch->master))
            continue;
        if ((spellnum == SPELL_EARTHQUAKE) && (AFF_FLAGGED(ch, AFF_FLYING) || AFF_FLAGGED(ch, AFF_WATERWALK)))
            continue;

        //        CREF(tch, CHAR_NULL);
        mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
        if (IS_SET(SINFO.routines, MAG_AFFECTS) && !DEAD(tch) && !DEAD(ch))
            mag_affects(GET_LEVEL(ch), ch, tch, spellnum, 1);
        //      CUREF(tch);
        if (tch && spellnum == SPELL_HOLY_WORD) {
            if (FIGHTING(tch)) {
                stop_fighting(tch);
                if (IS_NPC(tch)) {
                    clearMemory(tch);
                }
                if (ch != tch) {
                    act("$n suffers punishment!", TRUE, tch, 0, 0, TO_ROOM);
                    act("$n calmly sits down, astonished.", TRUE, tch, 0, 0, TO_ROOM);
                    act("You suffer punishment!", TRUE, tch, 0, 0, TO_CHAR);
                    act("You calmly sit down, astonished.", TRUE, tch, 0, 0, TO_CHAR);
                    GET_POS(tch) = POS_SITTING;
                }
            }
            stop_fighting(ch);
        }
        if ((++sum)>10)
        {
            process_data();
            sum=0;
        }
    }
}                         
void            mob_ai(struct char_data * ch);

void            mag_points(int level, struct char_data * ch, struct char_data * victim,
                           int spellnum, int savetype)
{
    int             hit = 0;
    int             move = 0;
    char           *to_vict = NULL;
    char           *to_char = NULL;
    char           *to_room = NULL;
    if (victim == NULL || ch==NULL)
        return;
    if (DEAD(victim) || DEAD(ch))
        return;

    switch (spellnum) {
    case PRAYER_REJUVENATE:
        GET_COND(victim, THIRST) = 24;
        GET_COND(victim, FULL) = 24;
        move=GET_MAX_MOVE(victim);
        if (GET_COND(victim, DRUNK)<5)
        {
            to_room="$n looks fresh and fit.";
            send_to_char("You feel fresh and fit.\r\n", victim);
        }
        else
        {
            to_room="$n looks fresh and fit, although a little drunk.";
            send_to_char("You feel fresh and fit, although a little drunk.\r\n", victim);
        }

        break;
    case PRAYER_CURE:
        hit=dice(level, 4)+3;
        if (GET_DEITY(ch)!=GET_DEITY(victim))
            hit = 2*hit/3;

        if (FOL_MUGRAK(ch) || FOL_MUGRAK(victim)) {
            send_to_char("You feel sharp pain.\r\n", victim);
            to_room="$n grimaces in pain.";
        }
        else
        {
            send_to_char("You feel better.\r\n", victim);
            to_room="$n looks better.";
        }

        if (level>5 && GET_COND(victim, DRUNK))
        {
            GET_COND(victim, DRUNK)=0;
            send_to_char("You now feel sober.\r\n", victim);
            to_room="$n looks better.";
        }

        break;
    case PRAYER_QUENCH:
    case SPELL_CREATE_WATER:
        if (IS_NPC(victim))
            return;
        GET_COND(victim, THIRST) = 24;
        send_to_char("Your thirst has been quenched.\r\n", victim);
        to_room="$n pats $s belly.";
        break;

    case PRAYER_SATE:
    case SPELL_CREATE_FOOD:
        if (IS_NPC(victim))
            return;
        GET_COND(victim, FULL) = 24;
        send_to_char("You feel full.\r\n", victim);
        to_room="$n pats $s belly.";
        break;

    case SPELL_CURE_DRUNK:
        GET_COND(victim, DRUNK) = 0;
        send_to_char("You now feel sober.\r\n", victim);
        to_room="$n looks less drunk now.";
        break;
    case SPELL_SYNOST:
        if (GET_MANA(ch) >= (GET_MAX_MANA(ch) / 5)) {
            if (GET_HIT(victim) == GET_MAX_HIT(victim)) {
                send_to_char("Your victim is at full hit points.\r\n", ch);
                return;
            }
            GET_MANA(ch) -= GET_MAX_MANA(ch) / 5;
            GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + GET_MAX_MANA(ch) / 8);
            send_to_char("Your let your mana slowly travel through the eter.\r\n", ch);
            send_to_char("You feel strange as energy pumps in, apparently from nowhere.\r\n", victim);
        } else
            send_to_char("You don't have enough mana.\r\n", ch);
        break;
    case SPELL_RESTORE:
        GET_COND(victim, DRUNK) = 0;
        GET_COND(victim, THIRST) = 24;
        GET_COND(victim, FULL) = 24;
        move=dice(level, 4);
        affect_from_char(victim, SPELL_BLINDNESS);
        to_room="$n looks realy fresh and fit now.";
        send_to_char("You feel realy fresh and fit.\r\n", victim);
        break;
    case SPELL_CURE_LIGHT:
        hit = dice(1, 8) + level / 3;
        send_to_char("You feel better.\r\n", victim);
        to_room="$n looks better.";
        break;
    case SPELL_CURE_SERIOUS:
        hit = dice(2, 8) + level / 2;
        send_to_char("You feel much better!\r\n", victim);
        to_room="$n looks much better.";
        break;
    case SPELL_CURE_CRITIC:
        hit = dice(3, 8) + level;
        send_to_char("You feel a lot better!\r\n", victim);
        to_room="$n looks a lot better.";
        break;
    case SPELL_HEALING_TOUCH:
        hit = MIN(40, dice(2, 10) + level);
        send_to_char("You feel a lot better!\r\n", victim);
        to_room="$n looks a lot better.";
        break;
    case SPELL_SURCEASE:
        hit = MIN(40, dice(4, 6) + level/2);
        move=hit;
        send_to_char("You feel better and fresher.\r\n", victim);
        to_room="$n looks better and fresher.";
        break;
    case PRAYER_HEAL:
    case SPELL_HEAL:
        if (ch==victim)
            hit = 100;
        else if (GET_DEITY(ch)==GET_DEITY(victim))
            hit = 75;
        else
            hit = 50;


        if (FOL_MUGRAK(ch) || FOL_MUGRAK(victim)) {
            send_to_char("You feel unearthly pain.\r\n", victim);
            to_room="$n suffers great pains.";
        }
        else
        {
            send_to_char("A warm feeling floods your body.\r\n", victim);
            to_room="$n looks much healthier.";
        }
        break;

    case SPELL_POWER_HEAL:
        hit = 300;
        if (FOL_MUGRAK(ch) || FOL_MUGRAK(victim)) {
            send_to_char("You feel supreme manifestation of pain.\r\n", victim);
            to_room="$n screams in pains!";
        }
        else
        {
            send_to_char("A supreme warm feeling floods your body.\r\n", victim);
            to_room="A wide grin crosses $n's face.";
        }



        break;

    case SPELL_RESTORE_MANA:
        GET_MANA(victim)=MIN(GET_MAX_MANA(victim), GET_MANA(victim)+100);
        to_char="You feel your mana energy flooding in!";
        break;

    case SPELL_BAPTIZE:
        GET_ALIGNMENT(victim)=GET_ALIGNMENT(ch);
        act("You put your hand on $N's head and baptize $M.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n puts $s hand on your head and baptizes you.", FALSE, ch, 0, victim, TO_VICT);
        act("$n puts $s hand on $N's head and baptizes $M.", FALSE, ch, 0, victim, TO_NOTVICT);
        return;
        break;

    case SPELL_ATONEMENT:
        GET_ALIGNMENT(ch)=GET_REAL_ALIGNMENT(ch);
        act("The air around you shimmers for a moment.\r\nYou feel justified!", FALSE, ch, 0, 0, TO_CHAR);
        act("$n flashes white.", FALSE, ch, 0, 0, TO_ROOM);
        return;
        break;


    case SPELL_RESTORATION:
        hit=GET_MAX_HIT(victim);
        send_to_char("A divine energy floods your whole body.\r\nYou have been fully restored!", victim);
        to_room="$n fully restores $mself.";
        break;

    case SPELL_RESSURECTION:
        hit = level * 10;
        //GET_MANA(victim) = GET_MAX_MANA(victim);
        //GET_MOVE(victim) = GET_MAX_MOVE(victim);
        break;

    }

    if (FOL_MUGRAK(ch) || FOL_MUGRAK(victim))
    {
        hit=-hit;
        if (ch!=victim)
            check_fight(ch, victim);
    }

    if (to_char != NULL)
        act(to_char, FALSE, ch, 0, 0, TO_CHAR);
    if (to_room != NULL)
        act(to_room, FALSE, victim, 0, 0, TO_ROOM);

    if (FIGHTING(victim) && is_same_group(ch, victim))
    {
        int how_good, exp;
        how_good=100*GET_HIT(victim)/GET_MAX_HIT(victim);
        if (how_good>80)
            exp=hit/2;
        else if (how_good>20)
            exp=hit*2;
        else
            exp=hit;
        if (ch==victim)
            exp/=2;
        ch->dam_exp+=MIN(exp, GET_MAX_HIT(victim)-GET_HIT(victim));
    }                     
    
    if (FIGHTING(victim))	   
    	if (IS_NPC(FIGHTING(victim)) && FIGHTING(FIGHTING(victim))!=victim && CHANCE(GET_LEVEL(FIGHTING(victim))))     	// chance for a mob to turn and attack the healer
    	{                                                                                        
    		struct char_data *mob=FIGHTING(victim);
    		act("You turn to attack $N!", FALSE, mob, NULL, ch, TO_CHAR);
 	       	act("$n turns to attack $N!", FALSE, mob, NULL, ch, TO_NOTVICT);
        	act("$n turns to attack you!", FALSE, mob, NULL, ch, TO_VICT);
        	stop_fighting(mob);
        	set_fighting(mob, ch);
        	WAIT_STATE(ch, PULSE_VIOLENCE);
	}    		


    if (hit)
        GET_HIT(victim) = MAX(1, MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit));
    if (move)
        GET_MOVE(victim) = MAX(1, MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move));
    update_pos(victim);
}


void            mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
                              int spellnum, int type)
{
    int             spell = 0;
    char           *to_vict = NULL,
                              *to_room = NULL;

    if (victim == NULL || ch==NULL)
        return;

    if (DEAD(victim) || DEAD(ch))
        return;
    switch (spellnum) {
    case PRAYER_CURE:
        if (level>20 && affected_by_spell(victim, SPELL_PLAGUE))
        {
            to_vict = "You are cured from the plague!";
            to_room = "$n is cured from plague!";
            affect_from_char(victim, SPELL_PLAGUE);
            act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
            act(to_room, TRUE, victim, 0, ch, TO_ROOM);
        }
        if (level>15 && affected_by_spell(victim, SPELL_BLINDNESS))
        {
            to_vict = "Your vision returns!";
            to_room = "There's a momentary gleam in $n's eyes.";
            affect_from_char(victim, SPELL_BLINDNESS);
            act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
            act(to_room, TRUE, victim, 0, ch, TO_ROOM);
        }
        if (level>10 && affected_by_spell(victim, SPELL_POISON))
        {
            to_vict = "You feel your blood purified.";
            to_room = "$n stops shivering.";
            affect_from_char(victim, SPELL_POISON);
            act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
            act(to_room, TRUE, victim, 0, ch, TO_ROOM);
        }
        return;
        break;

    case SPELL_CURE_BLIND:
        spell = SPELL_BLINDNESS;
        to_vict = "Your vision returns!";
        to_room = "There's a momentary gleam in $n's eyes.";
        break;

    case SPELL_HEAL:
        return;
    case SPELL_REMOVE_POISON:
        spell = SPELL_POISON;
        to_vict = "You feel your blood purified.";
        to_room = "$n looks better.";
        break;
    case SPELL_REMOVE_CURSE:
        spell = SPELL_CURSE;
        to_vict = "You don't feel so unlucky anymore.";
        to_room = "$n looks happier.";
        break;

    case SPELL_CURE_PLAGUE:
        spell = SPELL_PLAGUE;
        to_vict = "You are cured! What a relief!";
        to_room = "$n is cured from plague!";
        break;

    default:
        sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_unaffects", spellnum);
        log(buf);
        return;
        break;
    }

    if (!affected_by_spell(victim, spell)) {
        send_to_char(NOEFFECT, ch);
        return;
    }
    affect_from_char(victim, spell);
    if (to_vict != NULL)
        act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
    if (to_room != NULL)
        act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


void            mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
                               int spellnum, int savetype)
{
    char           *to_char = NULL;
    char           *to_room = NULL;
    int             i;
    if (obj == NULL || PURGED(obj))
        return;

    switch (spellnum) {
    case SPELL_BLESS:
        if (!IS_OBJ_STAT(obj, ITEM_BLESS) &&
                (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch))) {
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BLESS);
            to_char = "$p glows briefly white.";
        }
        break;
    case SPELL_CURSE:
        if (!IS_OBJ_STAT(obj, ITEM_NODROP)) {
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);
            if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
                GET_OBJ_VAL(obj, 2)--;
            to_char = "$p briefly glows red.";
        }
        break;
    case SPELL_DISPEL_MAGIC:
        if (IS_OBJ_STAT(obj, ITEM_MAGIC) && (!IS_OBJ_STAT(obj, ITEM_NODISP_MAG))) {
            REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
            for (i = 0; i < MAX_OBJ_AFFECT; i++)
                if (obj->affected[i].location != APPLY_NONE)
                    if (number(1, 130) > level) {
                        obj->affected[i].location = APPLY_NONE;
                        obj->affected[i].modifier = 0;
                    }
            to_char = "All the magic $p ever had is gone.";
            to_room = "All the magic $p ever had is gone.";
        } else
            to_char = "You fail.";
        break;
    case SPELL_INVISIBLE:
        if (!IS_OBJ_STAT(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
            SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
            to_char = "$p vanishes.";
        }
        break;
    case SPELL_POISON:
        if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
                (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
                (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
            GET_OBJ_VAL(obj, 3) = 1;
            to_char = "$p steams briefly.";
        }
        break;
    case SPELL_REMOVE_CURSE:
        if (IS_OBJ_STAT(obj, ITEM_NODROP)) {// || IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
            //REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NOREMOVE);
            if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
                GET_OBJ_VAL(obj, 2)++;
            to_char = "$p briefly glows blue.";
        }
        break;
    case SPELL_REMOVE_POISON:
        if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
                (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
                (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
            GET_OBJ_VAL(obj, 3) = 0;
            to_char = "$p steams briefly.";
        }
        break;
    }

    if (to_char == NULL)
        send_to_char(NOEFFECT, ch);
    else
        act(to_char, TRUE, ch, obj, 0, TO_CHAR);

    if (to_room != NULL)
        act(to_room, TRUE, ch, obj, 0, TO_ROOM);
    else if (to_char != NULL)
        act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}


void            mag_creations(int level, struct char_data * ch, int spellnum)
{
    struct obj_data *tobj;
    int             z;

    if (ch == NULL || DEAD(ch))
        return;
    level = MAX(MIN(level, LVL_IMPL), 1);

    switch (spellnum) {
    case SPELL_CREATE_FOOD:
        z = 10;
        break;
    case SPELL_CREATE_WATER:
    default:
        send_to_char("Spell unimplemented, it would seem.\r\n", ch);
        return;
        break;
    }

    if (!(tobj = read_object(z, VIRTUAL, 0, 0))) {
        send_to_char("I seem to have goofed.\r\n", ch);
        sprintf(buf, "SYSERR: spell_creations, spell %d, obj %d: obj not found",
                spellnum, z);
        log(buf);
        return;
    }
    obj_to_char(tobj, ch);
    act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
}



/*
  Every spell which summons/gates/conjours a mob comes through here.

  None of these spells are currently implemented in Circle 3.0; these
  were taken as examples from the JediMUD code.  Summons can be used
  for spells like clone, ariel servant, etc.
*/

static char    *mag_summon_msgs[] = {
                                        "\r\n",
                                        "$n makes a strange magical gesture; you feel a strong breeze!\r\n",
                                        "$n animates a corpse!\r\n",
                                        "$N appears in a cloud of thick blue smoke!\r\n",
                                        "$N appears in a cloud of thick green smoke!\r\n",
                                        "$N appears in a cloud of thick red smoke!\r\n",
                                        "$N disappears in a thick black cloud!\r\n",
                                        "You suddenly hear a clap of huge wings behind you.\r\n",
                                        "As $n makes a strange magical gesture, you feel a searing heat.\r\n",
                                        "As $n makes a strange magical gesture, you feel a sudden chill.\r\n",
                                        "You suddenly hear a loud growl behind you.\r\n",
                                        "$N clones $mself!\r\n",
                                        "$n rises from the dead!\r\n"
                                    };

#define MOB_MONSUM_I		21      /* lesser level */
#define MOB_MONSUM_II		10      /* regular */
#define MOB_MONSUM_III		10      /* greater */
#define MOB_MONSUM_IV		273     /* dragon  */
#define MOB_MONSUM_V		22      /* ancient */
#define MOB_GATE_I		10
#define MOB_GATE_II		10
#define MOB_GATE_III		10
#define MOB_ELEMENTAL_BASE	20
#define MOB_CLONE		10
#define MOB_ZOMBIE		11
#define MOB_AERIALSERVANT	19


static char    *mag_summon_fail_msgs[] = {
            "\r\n",
            "There are no such creatures.\r\n",
            "Uh oh...\r\n",
            "Oh dear.\r\n",
            "You fail.\r\n",
            "The elements resist!\r\n",
            "You felt a small disturbance in flows of the magic.\r\n",
            "There is no corpse!\r\n"
        };

extern int not_in_arena(struct char_data *ch);

void            mag_summons(int level, struct char_data * ch, struct obj_data * obj,
                            int spellnum, int savetype)
{
    struct char_data *mob;
    static struct affected_type af;
    struct obj_data *tobj,
                *next_obj;
    int             pfail = 0;
    int             msg = 0,
                          fmsg = 0;
    int             mob_num = 0;
    int             hitp = 0;
    int             change_mob = 0;
    int             handle_corpse = 0;
    int nowear=0;
    char            mob_name[256];
    char            mob_sdes[256];

    if (ch == NULL || DEAD(ch))
        return;
    if (not_in_arena(ch))
        return;

switch (spellnum) {
    case SPELL_ANIMATE_DEAD:
        if ((obj == NULL) || (GET_OBJ_TYPE(obj) != ITEM_CONTAINER) ||
                (!GET_OBJ_VAL(obj, 3))) {
            act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        if ( GET_OBJ_TYPE(obj)==ITEM_CONTAINER && (GET_OBJ_VAL(obj, 3)==1) && obj->orig_value && strcmp(GET_NAME(ch), obj->attack_verb))
        {
    		send_to_char("This corpse is protected by divine powers.\r\n", ch);
    		return;
        }
        handle_corpse = 1;
        change_mob = 1;
        msg = 12;
        mob_num = MOB_ZOMBIE;
        pfail = MIN(50, GET_LEVEL(ch));
        hitp = GET_LEVEL(ch) * (number(7, 9));
        strcpy(mob_name, (obj)->short_description);
        strcpy(mob_sdes, ((obj)->short_description));
        break;
    case SPELL_MONSUM_I:
        msg = 4;
        mob_num = MOB_MONSUM_I;
        pfail = MIN(50, GET_LEVEL(ch));
        change_mob = 0;
        hitp = GET_LEVEL(ch) * (number(8, 10));
        break;
    case SPELL_MONSUM_V:
        msg = 7;
        mob_num = MOB_MONSUM_V;
        pfail = MIN(50, GET_LEVEL(ch) + 10);
        change_mob = 0;
        hitp = (GET_LEVEL(ch) - 1 + number(0, 3)) * (number(14, 16));
        nowear=1;
        break;
    case SPELL_CONJ_ELEMENTAL:
        msg = 10;
        mob_num = MOB_ELEMENTAL_BASE;
        pfail = MIN(50, GET_LEVEL(ch));
        change_mob = 0;
        hitp = GET_LEVEL(ch) * (number(11, 13));
        break;
    case SPELL_CLONE:
        if (IS_NPC(ch)) return;
        mob_num = MOB_CLONE;
        pfail = MIN(50, GET_LEVEL(ch));
        strcpy(mob_name, GET_NAME(ch));
        strcpy(mob_sdes, GET_NAME(ch));
        change_mob = 1;
        msg = 11;
        break;
    default:
        return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM)) {
        send_to_char("You are too giddy to have any followers!\r\n", ch);
        return;
    }
    if (number(0, 150) > GET_SKILL(ch, spellnum) + MAX(1, (GET_LEVEL(ch) / 2 + GET_CHA(ch) + GET_INT(ch) + GET_WIS(ch)) / 2)) {
        send_to_char(mag_summon_fail_msgs[6], ch);
        return;
    }
    if (count_pets(ch)>=NUM_PETS_ALLOWED)
    {
        send_to_char("Spell failed beacause you had mob followers.\r\n", ch);
        return;
    }

    if (!(mob = read_mobile(real_mobile(mob_num), REAL, world[ch->in_room].zone))) {
        send_to_char("I seem to have goofed.\r\n", ch);
        sprintf(buf, "SYSERR: spell_summons, spell %d, mob %d: mob not found",
                spellnum, mob_num);
        log(buf);
        return;
    }
    char_to_room(mob, ch->in_room);
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
    IS_EQUIP_W(mob) = 0;
    af.type = 0;
    af.bitvector = 0;
    af.bitvector2 = 0;
    af.bitvector3 = 0;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.type = SPELL_CHARM;
    af.duration =-2;   // (GET_INT(ch) + GET_CHA(ch) + GET_WIS(ch) + GET_LEVEL(ch)) / 2;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);
    GET_LEVEL(mob) = MAX(GET_LEVEL(mob), GET_LEVEL(ch)-10);
    if (spellnum == SPELL_CLONE) {
        GET_CLASS(mob)=GET_CLASS(ch);
        GET_LEVEL(mob)=GET_LEVEL(ch);
        GET_MAX_HIT(mob) = GET_MAX_HIT(ch);
        //GET_MAX_MOVE(mob) = GET_MAX_MOVE(ch);
        //GET_MAX_MANA(mob) = GET_MAX_MANA(ch);
        GET_HIT(mob) = GET_MAX_HIT(ch);
        //GET_MANA(mob) = GET_MAX_MANA(ch);
        //GET_MOVE(mob) = GET_MAX_MOVE(ch);
        GET_ALIGNMENT(mob) = GET_ALIGNMENT(ch);
        GET_AC(mob) = 100;
        GET_HITROLL(mob) = 0;
        GET_DAMROLL(mob) = 0;
        GET_STR(mob)=GET_STR(ch);
        GET_DEX(mob)=GET_DEX(ch);
        GET_CON(mob)=GET_CON(ch);
        GET_WIS(mob)=GET_WIS(ch);
        GET_INT(mob)=GET_INT(ch);
        GET_CHA(mob)=GET_CHA(ch);
    }
    SET_BIT(MOB_FLAGS(mob), MOB_PET);
    GET_RATIO(mob)=1;
    GET_HITROLL(mob)=MAX(GET_HITROLL(mob), GET_LEVEL(mob)-5);
    act(mag_summon_msgs[msg], FALSE, mob, 0, mob, TO_ROOM);
    add_leech(ch, mob, spellnum, mag_manacost(ch, spellnum));
    add_follower(mob, ch);
    perform_group(ch, mob);
    if (spellnum!=SPELL_MONSUM_V && spellnum!=SPELL_CLONE && is_same_group(ch, mob))
        act("$n says, 'How may I serve you, master?'", FALSE, mob, NULL, NULL, TO_ROOM);
    
    if (nowear)
    	SET_BIT(MOB2_FLAGS(mob), MOB2_NOWEAR);


    if (change_mob) {
        mob->player.name = str_dup(mob_name);
        mob->player.short_descr = str_dup(mob_sdes);
        CAP(mob_sdes);
        sprintf(mob_sdes, "%s is standing here.\r\n", mob_sdes);
        mob->player.long_descr = str_dup(mob_sdes);
        if (spellnum == SPELL_CLONE)
            mob->player.description = str_dup(ch->player.long_descr ? ch->player.long_descr : mob->player.description);
        else
            mob->player.description = str_dup(mob->player.description);
    }
    if (handle_corpse) {
        for (tobj = obj->contains; tobj; tobj = next_obj) {
            next_obj = tobj->next_content;
            obj_from_obj(tobj);
            obj_to_char(tobj, mob);
        }
        extract_obj(obj);
    }
}



void mag_room(int level, struct char_data * ch, int spellnum)
{
    long aff; /* what affection */
    int ticks; /* how many ticks this spell lasts */
    char *to_char = NULL;
    char *to_room = NULL;
    struct raff_node *raff;

    extern struct raff_node *raff_list;

    aff = ticks =0;

    if (ch == NULL || DEAD(ch))
        return;
    level = MAX(MIN(level, LVL_IMPL), 1);

    switch (spellnum) {
        /*	case SPELL_WALL_OF_FOG:
        		to_char = "You create a fog out of nowhere.";
        		to_room = "$n creates a fog out of nowhere.";
        		aff = RAFF_FOG;
        		ticks = 1; // this spell lasts one hour
        		break;
          */
    case PRAYER_ILLUMINATION:
        to_char = "Your surroundings illuminates with an unearthly light.";
        to_room = "Your surroundings illuminates with an unearthly light.";
        aff = RAFF_ILLUMINATION;
        ticks = MAX(0, GET_FAITH(ch)/20+1);
        break;
        
    case PRAYER_SPIRITUAL_ASYLUM:
        to_char = "&BBluish myst covers the place, and you suddenly feel safe here.&0";
        to_room = "&BBluish myst covers the place, and you suddenly feel safe here.&0";
        aff = RAFF_PEACEFUL;
        ticks = MAX(0, GET_FAITH(ch)/80);
        break;    
        /* add more room spells here */

    default:
        sprintf(buf, "SYSERR: unknown spellnum %d " \
                "passed to mag_room", spellnum);
        log(buf);
        break;
    }

    /* create, initialize, and link a room-affection node */
    CREATE(raff, struct raff_node, 1);
    raff->room = ch->in_room;
    raff->timer = ticks;
    raff->affection = aff;
    raff->spell = spellnum;
    raff->ch=ch;
    raff->next = raff_list;
    raff->special_data=GET_LEVEL(ch);
    strcpy(raff->name, GET_NAME(ch));
    raff_list = raff;


    /* set the affection */
    SET_BIT(ROOM_AFFECTIONS(raff->room), aff);
    /*if (to_char == NULL)
        send_to_char(NOEFFECT, ch);
    else
        act(to_char, TRUE, ch, 0, 0, TO_CHAR);
      */
    if (to_room != NULL)
        act(to_room, TRUE, ch, 0, 0, TO_ROOM);
    if (to_char != NULL)
        act(to_char, TRUE, ch, 0, 0, TO_CHAR);
}

