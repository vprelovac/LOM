/* ************************************************************************
*  File: trap.c                          Part of Lands of Myst MUD        *
*                                                                         *
*  All rights reserved.                                                   *
*                                                                         *
*  Copyright (C) 1996-2002 Vladimir Prelovac                              *
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
extern struct raff_node *raff_list;
void TrapAff(struct char_data *ch, int trap);
extern struct dex_app_type dex_app[];



struct raff_node *find_raff_by_spell(int room, int spell)
{
    struct raff_node *raff;


    if (raff_list)
        for (raff=raff_list; raff; raff=raff->next)
        {
            if ((raff->room==room) && (raff->spell==spell))
                return raff;
        }
    return NULL;
}


struct raff_node *find_raff_by_aff(int room, int aff)
{
    struct raff_node *raff;


    if (raff_list)
        for (raff=raff_list; raff; raff=raff->next)
        {
            if ((raff->room==room) && (raff->affection==aff))
                return raff;
        }
    return NULL;
}



int get_trap_by_name(char *name)
{

    if (is_abbrev(name, "poisonous dart trap"))
        return SKILL_TRAP_POISON;
    else if (is_abbrev(name, "dart"))
        return SKILL_TRAP_DART;
    else if (is_abbrev(name, "rope trap"))
        return SKILL_TRAP_ROPE;
    else if (is_abbrev(name, "horror trap"))
        return SKILL_TRAP_HORROR;
    else if (is_abbrev(name, "fire trap"))
        return SKILL_TRAP_FIRE;
    else if (is_abbrev(name, "teleport trap"))
        return SKILL_TRAP_TELEPORT;
    else if (is_abbrev(name, "local teleport trap"))
        return SKILL_TRAP_LOCAL_TELEPORT;
    else if (is_abbrev(name, "blind trap"))
        return SKILL_TRAP_BLIND;
    else if (is_abbrev(name, "stun trap"))
        return SKILL_TRAP_STUN;
    else if (is_abbrev(name, "acid trap"))
        return SKILL_TRAP_ACID;
    else if (is_abbrev(name, "web trap"))
        return SKILL_TRAP_WEB;
    else if (is_abbrev(name, "sleep trap"))
        return SKILL_TRAP_SLEEP;
    else if (is_abbrev(name, "caltrops trap"))
        return SKILL_TRAP_CALTROPS;
    else if (is_abbrev(name, "doom trap"))
        return SKILL_TRAP_DOOM;
    else if (is_abbrev(name, "gas trap"))
        return SKILL_TRAP_GAS;
    else
        return 0;
}



void remove_trap(struct raff_node *raff)
{
    struct raff_node *next_raff, *temp;


    REMOVE_BIT(world[(int)raff->room].room_affections,
               raff->affection);
    REMOVE_FROM_LIST(raff, raff_list, next)
    DISPOSE(raff);
}

int do_settrap( struct char_data *ch, int trap)
{
    struct raff_node *raff;

    int duration;
    if (!ch || !IS_TRAP_SKILL(trap))
        return 0;

    switch (trap)
    {
    case SKILL_TRAP_DART:
    case SKILL_TRAP_POISON:
    case SKILL_TRAP_FIRE:
    case SKILL_TRAP_LOCAL_TELEPORT:
    case SKILL_TRAP_BLIND:
    case SKILL_TRAP_STUN:
    case SKILL_TRAP_ACID:
    case SKILL_TRAP_WEB:
    case SKILL_TRAP_SLEEP:
    case SKILL_TRAP_TELEPORT:
    case SKILL_TRAP_ROPE:
    case SKILL_TRAP_CALTROPS:
    case SKILL_TRAP_DOOM:
    case SKILL_TRAP_GAS:
        duration=DUR_TRIGGER_TRAP;
        break;
    case SKILL_TRAP_HORROR:
        duration=MAX(1, GET_SKILL(ch, SKILL_TRAP_HORROR)/30);
        break;
    default:
        logs("ERROR: do_settrap(), invalid trap - %d.", trap);
        return 0;
        break;
    }
    CREATE(raff, struct raff_node, 1);
    raff->room = ch->in_room;
    raff->timer = duration;
    raff->affection = RAFF_TRAP;
    raff->spell = trap;
    raff->ch=ch;
    raff->next = raff_list;
    raff->special_data=GET_LEVEL(ch);
    raff->deity=GET_DEITY(ch);
    strcpy(raff->name, GET_NAME(ch));
    raff_list = raff;


    SET_BIT(ROOM_AFFECTIONS(raff->room), RAFF_TRAP);

    return 1;
}


void TrapDamage( struct char_data *ch, int trap, int level, int deity)
{
    int dam;

    switch (trap)
    {
    case SKILL_TRAP_DART:
        dam=number(30, 60);
        break;
    case SKILL_TRAP_POISON:
        dam=number(50, 80);
        break;
    case SKILL_TRAP_FIRE:
        //dam=number(80, 140);
        dam=dice(level, 6);
        break;
    case SKILL_TRAP_ACID:
        //dam=number(220, 300);
        dam=number(level, 12)+level;
        break;
    case SKILL_TRAP_CALTROPS:
        //dam=number(40, 70);
        dam=dice(level, 4);
        break;
    case SKILL_TRAP_DOOM:
        dam=dice(level, 15);
        //dam=number(350, 400);
        break;
    default:
        dam=10;
    }
    
    if (deity==DEITY_SKIG)
    	dam+=dam/5;
    	
    if (!IS_GOD(ch))
        GET_HIT(ch)-=dam;

    if (check_kill(ch, spells[trap]))
        return;

    if (IS_NPC(ch))
        do_action(ch, "", find_command("curse"), 0);
}

void TrapAff(struct char_data *ch, int trap)
{
    struct affected_type af;


    af.bitvector = 0;
    af.bitvector2 = 0;
    af.bitvector3 = 0;
    af.type = 0;
    af.location=APPLY_NONE;
    af.modifier=0;
    af.duration = 0;

    switch (trap)
    {
    case SKILL_TRAP_CALTROPS:
        af.location = APPLY_DEX;
        af.modifier = -3;
        af.duration = 2;
        af.type=SKILL_TRAP_CALTROPS;
        GET_MOVE(ch)=MAX(0, GET_MOVE(ch)-100);
        send_to_char("\r\nYour feet are bleeding!\r\n", ch);
        break;
    case SKILL_TRAP_GAS:
        if (IS_AFFECTED(ch, AFF_POISON) && AFF3_FLAGGED(ch, AFF3_CHOKE))
            return;
        af.bitvector=AFF_POISON;
        af.bitvector3 = AFF3_CHOKE;
        af.location = APPLY_STR;
        af.duration = 1;
        af.modifier = -2;
        af.type=SPELL_POISON;
        act("\r\nYou are CHOKING!.",FALSE,ch,0,0,TO_CHAR);
        act("$n suddenly looks very, very sick.",FALSE,ch,0,0,TO_ROOM);
        break;
    case SKILL_TRAP_POISON:
        if (IS_AFFECTED(ch, AFF_POISON))
            return;

        act("\r\nYou feel very sick.",FALSE,ch,0,0,TO_CHAR);
        act("$n suddenly looks very sick.",FALSE,ch,0,0,TO_ROOM);
        apply_poison(ch, 1);
        break;
    case SKILL_TRAP_SLEEP:
        if ((IS_NPC(ch) && (MOB_FLAGGED(ch, MOB_AWARE) || MOB_FLAGGED(ch, MOB_NOSLEEP))) ||
                ((number(1, 35)<GET_COURAGE(ch)))) {
            send_to_char("You somehow manage to recover and stay on your feet.\r\n",ch);
            act("$n shakes his head, and manages to recover.", FALSE,ch,NULL,0,TO_ROOM);
            return;
        }
        af.bitvector=AFF_SLEEP;
        af.duration = number(5, 9);
        af.type=SPELL_SLEEP;
        GET_POS(ch)=POS_SLEEPING;
        send_to_char("You are sent to sleep.\r\n", ch);
        break;
    case SKILL_TRAP_BLIND:
        af.bitvector=AFF_BLIND;
        af.duration = number(2, 4);
        af.type=SPELL_BLINDNESS;
        send_to_char("\r\nYou can't see a damned thing!\r\n", ch);
        break;
    case SKILL_TRAP_STUN:
        if (number(1, 342)<GET_DEX(ch)*GET_DEX(ch)/10)
        {
            send_to_char("You somehow manage to recover and stay on your feet.\r\n",ch);
            act("$n shakes his head, and manages to recover.", FALSE,ch,NULL,0,TO_ROOM);
            return;
        }
        send_to_char("\r\nYou are STUNNED!\r\n", ch);
        act("$n is stunned.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch)=POS_STUNNED;
        return;
        break;
    case SKILL_TRAP_ROPE:
        if (number(1, 171)<GET_DEX(ch)*GET_DEX(ch)/13)
        {
            send_to_char("You manage to keep your balance.\r\n", ch);
            act("$n manages to keep $s balance.\r\n", FALSE, ch, 0, 0, TO_ROOM);
            if (IS_NPC(ch))
                do_action(ch, "", find_command("mumble"), 0);
            return;
        }
        send_to_char("\r\nYou fall to the ground.\r\n", ch);
        act("$n falls to the ground.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch)=POS_SITTING;
        return;
        break;
    case SKILL_TRAP_WEB:

        if (number(1, 342)<GET_DEX(ch)*GET_DEX(ch)/10)
        {
            send_to_char("...You manage to jump away and avoid it.\r\n", ch);
            act("$n manages to jump away and avoid the sticky web.", FALSE, ch, 0, 0, TO_ROOM);
            return;
        }
        af.location = APPLY_DEX;
        af.modifier = -4;
        af.duration = 2;
        af.bitvector3 = AFF3_WEB;
        af.type=SPELL_WEB;
        send_to_char("\r\n...It falls all over you! And its sticky, YUCK!\r\n", ch);
        act("$n is covered in sticky web!", FALSE, ch, 0, 0 , TO_ROOM);
        break;
    case SKILL_TRAP_FIRE:
        af.duration = 1;
        af.bitvector2 = AFF2_BURNING;
        af.type=SPELL_BURN;
        if (!AFF2_FLAGGED(ch, AFF2_BURNING)) {
            send_to_char("\r\nYou are engulfed in flames!", ch);
            act("$n is engulfed in flames!", FALSE, ch, 0, 0, TO_ROOM);
        }
        else
            return;
        break;
    case SKILL_TRAP_ACID:
        af.duration = 1;
        af.bitvector2 = AFF2_ACIDED;
        af.type=SPELL_ACID;
        if (!AFF2_FLAGGED(ch, AFF2_ACIDED)) {
            send_to_char("\r\nYou are drenched in acid!", ch);
            act("$n is drenched in acid!", FALSE, ch, 0, 0, TO_ROOM);
        }
        else
            return;
        break;

    }
    if (trap!=SKILL_TRAP_POISON)
        affect_to_char(ch, &af);
}


void TrapHorror(struct char_data *ch, int level)
{
    int prob;

    my_srandp(GET_ROOM_VNUM(ch->in_room));
    prob=number(1, 30);
    my_srand(rand());

    if (!WIL_CHECK(ch) || GET_LEVEL(ch)<level-10)     // trap works
    {
        act("You scream in panic!", FALSE, ch, 0, 0, TO_CHAR);
        act("$n screams in panic!", FALSE, ch, 0, 0, TO_ROOM);
        do_flee(ch, "", 0, 99);
    }
    else
    {
        act("'So what?', you shrug.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n seems not to be terrified by a horror sight.", FALSE, ch, 0, 0, TO_ROOM);
        if (IS_NPC(ch))
            do_action(ch, "", find_command("shrug"), 0);
    }
}




void TrapTeleport(struct char_data *ch)
{
    int to_room,try = 0;
    extern int top_of_world;


    if (mag_savingthrow(ch,SAVING_SPELL, 0) && mag_savingthrow(ch,SAVING_SPELL, 0)) {
        send_to_char("\r\nYou feel strange, but the effect fades.\n\r",ch);
        return;
    }

    do {
        to_room = number(5, top_of_world);
    } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_ARENA | ROOM_NOMAGIC));

    act("$n slowly fades out of existence and is gone.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);
    if (IS_NPC(ch))
        do_action(ch, "", find_command("curse"), 0);
}

void TrapLocalTeleport(struct char_data *ch)
{
    int to_room,try = 0;
    extern int top_of_world;



    if (mag_savingthrow(ch,SAVING_SPELL, 0) && mag_savingthrow(ch,SAVING_SPELL, 0)) {
        send_to_char("\r\nYou feel strange, but the effect fades.\n\r",ch);
        return;
    }

    do {
        to_room = number(5, top_of_world);
    } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_ARENA | ROOM_NOMAGIC) || world[to_room].zone!=world[ch->in_room].zone);

    act("$n slowly fades out of existence and is gone.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);
    if (IS_NPC(ch))
        do_action(ch, "", find_command("curse"), 0);
}




int spot_trap(struct char_data *ch, int room)
{

    struct raff_node *raff;
    int trap;
    int prob;

    if (!ch || ch->in_room<1 || !ROOM_AFFECTED(room, RAFF_TRAP))
        return 0;


    raff=find_raff_by_aff(room, RAFF_TRAP);

    if (!raff)
    {
        logs("ERROR: ROOM TRAP without trap");
        return 1;
    }


    trap=raff->spell;
    my_srandp(GET_ROOM_VNUM(room)+trap);
    prob=number(1, 101);
    my_srand(rand());

    if (PRF_FLAGGED(ch, PRF_NOHASSLE) || raff->ch==ch || ((GET_LEVEL(ch)>=(raff->special_data)) && ((prob<10+GET_SKILL(ch, SKILL_SPOT_TRAPS)) || (prob<10+GET_SKILL(ch, SKILL_SIXTHSENSE)))))
    {
        improve_skill(ch, SKILL_SPOT_TRAPS, 1);   
        improve_skill(ch, SKILL_SIXTHSENSE, 1);
        sprintf(buf, "\r\nYou spot a &c%s&0 here!\r\n",spells[trap]);
        send_to_char(buf, ch);
        return 1;
    }
    return 0;

}



void FireRoomTrap(struct char_data *ch)
{

    struct raff_node *raff;
    int trap;
    int prob;

    if (!ch || ch->in_room<1 || !ROOM_AFFECTED(ch->in_room, RAFF_TRAP))
        return;


    raff=find_raff_by_aff(ch->in_room, RAFF_TRAP);

    if (!raff)
    {
        logs("ERROR: ROOM TRAP without trap");
        return;
    }

    trap=raff->spell;
    if (PRF_FLAGGED(ch, PRF_NOHASSLE) )
    {

        sprintf(buf, "You jump away from the &c%s&0.\r\n",spells[trap]);
        send_to_char(buf, ch);
        return;
    }

    if (spot_trap(ch, ch->in_room))
        return;

    if (trap!=SKILL_TRAP_ROPE && trap!=SKILL_TRAP_CALTROPS)
    {
        act("&wYou hear a strange noise..\r\n&0", TRUE, ch, 0, 0, TO_ROOM);
        act("&wYou hear a strange noise..\r\n&0", TRUE, ch, 0, 0, TO_CHAR);
    }

    prob=20+dex_app[GET_DEX(ch)].reaction;
    if (number(1, 401)>prob)
        TriggerTrap(ch, trap, raff->special_data, raff->deity);
    else
    {
        act("&GThe $T triggers but you manage to avoid it in the last moment!&0", FALSE, ch, 0, spells[trap], TO_CHAR);
        act("The $T triggers but $n manages to avoid it in the last moment!", FALSE, ch, 0, spells[trap], TO_ROOM);
    }


    if (raff->timer==DUR_TRIGGER_TRAP)
        remove_trap(raff);

}


int TriggerTrap( struct char_data *ch, int trap, int level, int deity)
{

    char to_char[100], to_room[100];
    char buf[132];
    int dam=FALSE, aff=FALSE;

    if (!ch || ch->in_room<1 || !IS_TRAP_SKILL(trap))
        return 0;


    *to_char=0;
    *to_room=0;

    switch(trap) {
    case SKILL_TRAP_DART:
        strcpy(to_char,"You are &cstabed&0 by a small dart!");
        strcpy(to_room,"$n is &cstabed&0 by a small dart!");
        dam=TRUE;
        break;
    case SKILL_TRAP_FIRE:
        strcpy(to_char,"&RYou are searead by scorching flames!!!&0");
        strcpy(to_char,"$n is &Rssearead&0 by scorching flames!!");
        dam=TRUE;
        aff=TRUE;
        break;
    case SKILL_TRAP_ACID:
        strcpy(to_char,"&CAcid spills all over you! You are corroded&0!!!");
        strcpy(to_room,"Acid spills all over $n, $e is &Ccorroded&0!!");
        dam=TRUE;
        aff=TRUE;
        break;
    case SKILL_TRAP_POISON:
        strcpy(to_char,"You are &ystabed&0 by a small dart!!");
        strcpy(to_room,"$n is &ystabed&0 by a small dart!");
        aff=TRUE;
        dam=TRUE;
        break;
    case SKILL_TRAP_GAS:
        strcpy(to_char,"You see a cloud of black gas.");
        strcpy(to_room,"A cloud of black gas shrouds $n.");
        aff=TRUE;
        break;
    case SKILL_TRAP_BLIND:
        strcpy(to_char,"&bYou are blinded by a flash of bright light!&0");
        strcpy(to_room,"A flash of bright light blinds $n!");
        aff=TRUE;
        break;
    case SKILL_TRAP_SLEEP:
        strcpy(to_char,"&YYou are hit in the head by something really BIG!!!&0");
        strcpy(to_room,"$n is &Yknocked out&0 by a sleep trap!!!");
        aff=TRUE;
        break;
    case SKILL_TRAP_STUN:
        strcpy(to_char,"&PYou are almost knocked out a stun trap!!!&0");
        strcpy(to_room,"$n is &Palmost knocked out&0 to the floor by a stun trap!!");
        aff=TRUE;
        break;
    case SKILL_TRAP_ROPE:
        strcpy(to_char,"You &ctrip over&0 a hidden rope.");
        strcpy(to_room,"$n &ctrips over&0 a hidden rope.");
        aff=TRUE;
        break;
    case SKILL_TRAP_WEB:
        strcpy(to_char,"&WThe web is launched from a trap and heading right towards you...&0");
        strcpy(to_room,"$n has triggered a &Wweb&0 trap!");
        aff=TRUE;
        break;
    case SKILL_TRAP_TELEPORT:
        strcpy(to_char,"&yYou are engulfed in strange mist...&0");
        strcpy(to_room,"$n is engulfed in strange mist.");
        break;
    case SKILL_TRAP_LOCAL_TELEPORT:
        strcpy(to_char,"You are engulfed in &bbluish&0 mist...");
        strcpy(to_room,"$n is engulfed in &bblusih&0 mist.");
        break;
    case SKILL_TRAP_HORROR:
        strcpy(to_char,"&wSomething really ugly and scarry pops up in front of your head! &WBOOO!!!&0");
        strcpy(to_room,"Something really ugly and scarry pops up in front of $n's head!");
        break;
    case SKILL_TRAP_CALTROPS:
        strcpy(to_char,"&cYou step onto dozen of caltrops laid on the floor! Ouch!&0");
        strcpy(to_room,"$n screams in pain as $e steps onto caltrops!");
        aff=TRUE;
        dam=TRUE;
        break;
    case SKILL_TRAP_DOOM:
        strcpy(to_char,"Oh, no... it sounds like a &WDOOM trap&0!!!");
        strcpy(to_room,"$n steps into a doom trap!");
        break;

    }


    if (IS_AFFECTED(ch, AFF_INVISIBLE | AFF_HIDE))
        appear(ch);
    if (IS_AFFECTED(ch, AFF_FLYING))
        do_land(ch,"", 0 ,0);

    if (*to_room)
        act(to_room, TRUE, ch, 0,0, TO_ROOM);
    if (*to_char)
        act(to_char, TRUE, ch, 0, 0, TO_CHAR);

    if (aff)
        TrapAff(ch, trap);
    if (dam)
        TrapDamage(ch, trap, level, deity);
    if (trap == SKILL_TRAP_TELEPORT)
        TrapTeleport(ch);
    if (trap == SKILL_TRAP_LOCAL_TELEPORT)
        TrapLocalTeleport(ch);
    if (trap == SKILL_TRAP_HORROR)
        TrapHorror(ch, level);
    if (trap==SKILL_TRAP_DOOM)
    {
        TrapAff(ch, SKILL_TRAP_CALTROPS);
        TrapAff(ch, SKILL_TRAP_GAS);
        TrapAff(ch, SKILL_TRAP_ACID);
        TrapDamage(ch, SKILL_TRAP_DOOM, level, deity);

    }
    return 1;
}






struct  trap_event_obj
{
    struct char_data *ch;
    int trap, room, move;
};

EVENTFUNC(make_trap_event)
{

    struct trap_event_obj *tevent=(struct trap_event_obj *) event_obj;
    struct char_data *ch;
    int trap, room, prob;


    ch=tevent->ch;
    trap=tevent->trap;
    room=tevent->room;
    GET_MOVE(ch)-=tevent->move;


    GET_UTIL_EVENT(ch)=0;
    DISPOSE(event_obj);


    if (ch->in_room != room)
    {
        send_to_char("You are unable to finish the trap here.\r\n", ch);
        return 0;
    }


    my_srandp(GET_ROOM_VNUM(ch->in_room)+trap);
    prob=number(1, 101);
    my_srand(rand());

    if (GET_SKILL(ch, trap)<prob)	// fail
    {

        if (number(1, 35)>GET_DEX(ch))
        {
            send_to_char("Uh oh. You make a slight mistake while puting it all together.\r\n", ch);
            TriggerTrap(ch, trap, GET_LEVEL(ch), GET_DEITY(ch));
        }
        else
        {
            sprintf(buf, "You were unable to set a %s here.\r\n", spells[trap]);
            send_to_char(buf, ch);
        }
    }
    else
    {
        if (do_settrap(ch, trap))
        {
            sprintf(buf, "&GYou successfully assemble a %s here.&0\r\n", spells[trap]);
            act("$n builds a trap here.\r\n", FALSE, ch, 0, spells[trap], TO_ROOM);
            improve_skill(ch, trap, 1);
        }
        else
            sprintf(buf, "For some reason, you are unable to place a trap here.\r\n");
        send_to_char(buf, ch);
    }
    return 0;
}


ACMD(do_trap)
{
    int trap, prob, timep, move;
    struct trap_event_obj *tevent;
    one_argument(argument, buf);

    if (!*buf)
    {
        send_to_char("Set what trap?\r\n", ch);
        return;
    }
    if (!(trap=get_trap_by_name(buf)))
    {
        send_to_char("You do not know that trap.\r\n", ch);
        return;
    }

    if (!GET_SKILL(ch, trap))
    {
        send_to_char("Better leave the traps to those who know.\r\n", ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("This room has a nice peaceful feeling.\r\n", ch);
        return;
    }

    if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
            (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
            (world[IN_ROOM(ch)].sector_type == SECT_FLYING))
    {
        send_to_char("How do you think to set trap here??\r\n", ch);
        return;
    }

    if (IS_THIEF(ch) && SECT(ch->in_room)==SECT_FOREST)
    {
        send_to_char("You can not set traps in the forest.\r\n", ch);
        return;
    }
    if (IS_RANGER(ch) && SECT(ch->in_room)==SECT_CITY)
    {
        send_to_char("You can not set traps in the city.\r\n", ch);
        return;
    }

    move=(210-GET_SKILL(ch, trap))*(150-4*GET_DEX(ch))/100;
    if (GET_MOVE(ch)<move)
    {
        send_to_char("You do not have enough movement.\r\n", ch);
        return;
    }

    if (IN_EVENT(ch))
    {
        send_to_char("You are already performing an action.\r\n", ch);
        return;
    }


    sprintf(buf, "You start assembling the %s.\r\n\r\n", spells[trap]);
    send_to_char(buf, ch);
    act("$n starts building a trap.", FALSE, ch, 0, 0, TO_ROOM);

    if (ROOM_AFFECTED(ch->in_room, RAFF_TRAP))
    {
        send_to_char("You activated an old trap!\r\n", ch);
        act("$n activates an old trap!", FALSE, ch,0, 0, TO_ROOM);
        FireRoomTrap(ch);
        return;
    }



    CREATE(tevent, struct trap_event_obj, 1);
    tevent->ch=ch;
    tevent->room=ch->in_room;
    tevent->trap=trap;
    tevent->move=move;

    timep=MAX(5, 14*((600-3*GET_SKILL(ch, trap))*(40-GET_DEX(ch))/300)/10);

    GET_UTIL_EVENT(ch)=event_create(make_trap_event, tevent, timep-1);    // 5 = 1 sec
    WAIT_STATE(ch, timep);
}





ACMD(do_disable)

{
    int trap, prob,  move;
    struct raff_node *raff;

    one_argument(argument, buf);

    if (!*buf)
    {
        send_to_char("Disable what trap?\r\n", ch);
        return;
    }
    if (!(trap=get_trap_by_name(buf)))
    {
        send_to_char("There is no such trap.\r\n", ch);
        return;
    }

    if (!ROOM_AFFECTED(ch->in_room, RAFF_TRAP))
    {
        send_to_char("Yea, no problem. But there is no such trap here.\r\nn", ch);
        act("$n likes to disable traps. If only $e could find one first.", FALSE, ch, 0, 0, TO_ROOM);
        return;
    }

    raff=find_raff_by_aff(ch->in_room, RAFF_TRAP);

    if (trap!=raff->spell)
    {
        send_to_char("You find a trap, but then you realize you are disabling it the wrong way!\r\n", ch);
        FireRoomTrap(ch);
        return;
    }

    my_srandp(GET_ROOM_VNUM(ch->in_room)+raff->spell);
    prob=number(1, 141);
    my_srand(rand());


    if (prob<GET_DEX(ch)+GET_INT(ch)+(IS_THIEF(ch)? GET_SKILL(ch, raff->spell):0) )
    {
        send_to_char("<Click>\r\n", ch);
        act("$n disables a trap.", FALSE, ch, 0, 0, TO_ROOM);
        remove_trap(raff);
    }
    else
    {
        send_to_char("<CLICK> (Uh, oh)\r\n", ch);
        FireRoomTrap(ch);
    }
}

