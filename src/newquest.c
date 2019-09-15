/* ************************************************************************
*  File: newquest.c                          Part of Lands of Myst MUD    *
*                                                                         *
*  All rights reserved.                                                   *
*                                                                         *
*  Copyright (C) 1996-2002 Vladimir Prelovac                              *
************************************************************************ */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "structs.h"
#include "class.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "shop.h"
#include "rooms.h"
#include "spells.h"
#include "newquest.h"
#include "auction.h"

SPECIAL(shop_keeper);

extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct object_data *object_list;
extern struct object_data *obj_proto;
extern struct char_data *mob_proto;
extern struct zone_data *zone_table;
extern int      top_of_mobt;
extern int      top_of_world;
extern struct room_data *world;
extern int players_online();
extern struct descriptor_data *descriptor_list;

int    real_mobile(int virtual);

int current_quest=QUEST_NONE, quest_step=0, quest_time_left=0, last_quest_timer=0, quest_join=0;

struct SQuests
{
    int chance;
    char name[50];
    int (*questfunc )(int code);
    int code;
    int time;

} quests[]=
    {
        { 0, "bronze mask"		, quest_mask	, QUEST_MASK_B	, 10 },
        { 0, "amber mask"		, quest_mask	, QUEST_MASK_A	, 10 },
        { 0, "mask of ancients"		, quest_mask	, QUEST_MASK_O	, 10 },
        { 7, "gift of fury"		, quest_gift	, QUEST_GIFT	, 10 },
        { 0, "berserker"		, quest_berserk	, QUEST_BERSERK	, 10 },
        { 4, "rush for wisdom"		, quest_rush	, QUEST_RUSH	, 10 },
        { 0, "bloodhunt!"		, quest_blood	, QUEST_BLOOD	, 10 },
        { 0, "treasure hunt"		, quest_treasure, QUEST_TREASURE, 10 },
        { 0, "path to glory"		, quest_path	, QUEST_PATH	, 10 },
        { 0, "chase the chicken"	, quest_chase	, QUEST_CHASE	, 10 },
        { 0, "nirvana"			, quest_nirvana	, QUEST_NIRVANA , 10 }
    };

#define NUM_QUESTS (sizeof(quests)/sizeof(quests[0]))


void            questchan(char *ar)
{
    struct descriptor_data *i;
    sprintf(buf1, "\r\n&w[QUEST]:&0 %s\r\n", ar);

    /* now send all the strings out */
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected && i->character &&
                QUESTING(i->character) &&
                !PLR_FLAGGED(i->character, PLR_WRITING) &&
                !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {
            send_to_char(buf1, i->character);
        }
    }
    *buf1 = '\0';

}

void quest_help()
{
	switch (current_quest)
	{
		case QUEST_GIFT: sprintf(buf1, "&w[QUEST]:&0 The winner of Gift of Fury quest is the first player to slay\r\n         a monster of higher level then his/hers. The winner is awarded\r\n         with double the normal experience and adventure points.\r\n");break;
		case QUEST_RUSH: sprintf(buf1, "&w[QUEST]:&0 The winner of Rush for Wisdom quest is the first player to  \r\n         improve a skill. Award is that skill being improved much better.\r\n");break;
		default: return; 
	}
	
	INFO_OUT(buf1);
}
		

int quest_mask(int code)
{
return 0;
}

int quest_gift(int code)
{
    current_quest=QUEST_GIFT;
    quest_step=QUEST_JOIN;
    quest_join=QUEST_JOIN_TIME;
    quest_time_left=quests[code].time;
    INFO_OUT("\r\n&w[QUEST]:&0 &cGIFT OF FURY quest is about to start!&0\r\n");
    return 0;
}

int quest_berserk(int code)
{
    return 0;
}

int quest_rush	(int code)
{
    current_quest=QUEST_RUSH;
    quest_step=QUEST_JOIN;
    quest_join=QUEST_JOIN_TIME;
    quest_time_left=quests[code].time;
    INFO_OUT("\r\n&w[QUEST]:&0 &cRUSH FOR WISDOM quest is about to start!&0\r\n");
    return 0;
}

int quest_blood	(int code)
{
return 0;
}

int quest_treasure(int code)
{
return 0;
}

int quest_path	(int code)
{
return 0;
}

int quest_chase	(int code)
{
return 0;
}

int quest_nirvana (int code)
{
return 0;
}


int generate_autoquest()
{
    int q, k;
	    
    if (current_quest!=QUEST_NONE || (players_online()<MIN_PLAYERS_BEFORE_QUEST))
        return 0;
    
    k=number(0, NUM_QUESTS-1);
    while (number(1, 10)>quests[k].chance)
    	k=number(0, NUM_QUESTS-1);
    
    quests[k].questfunc(quests[k].code);
    return 0;
}


int end_current_quest(char *reason)
{
    struct descriptor_data *d;
    int i=0;

    sprintf (buf2, "Quest is over (%s).", reason);
    questchan (buf2);
    current_quest=QUEST_NONE;
    quest_step=0;


    for (d = descriptor_list; d; d = d->next)
        if ((d->connected == CON_PLAYING) && (!IS_NPC(d->character)) && QUESTING(d->character))
            REMOVE_BIT(PRF_FLAGS(d->character), PRF_QUEST);

    return 1;
}

int find_quest_by_name(char *name)
{
    int i;

    for (i=0; i<NUM_QUESTS; i++)
        if (isname(name, quests[i].name))
            break;

    return (i!=NUM_QUESTS? i : 0);
}

ACMD(do_newquest)
{
    int i;
    two_arguments(argument, buf, buf2);

    if (isname(buf, "end"))
    {    	
        if (current_quest==QUEST_NONE)
            send_to_char("There is no active quest right now.\r\n", ch);
        else
            end_current_quest("by the immortal intervention");
            return;
    }
    if (isname(buf, "prolong"))
    {
        if (current_quest==QUEST_NONE || quest_step!=QUEST_ACTION)
            send_to_char("There is no active quest right now.\r\n", ch);
        else
        {
            quest_time_left++;
            questchan("Quest time prolonged by immortal ways..");
        }
        return;
    }

    else if (isname(buf, "start"))
    {
        if (current_quest!=QUEST_NONE)
            send_to_char("There is an active quest already. You have to STOP it first.\r\n", ch);
        else
        {
            i=find_quest_by_name(buf2);
            if (!i)
                send_to_char("There is no such quest.\r\n", ch);
            else
                quests[i].questfunc(quests[i].code);
        }
        return;
    }
    if (isname(buf, "auto"))
    {    	
    	if (current_quest!=QUEST_NONE)
            send_to_char("There is an active quest already. You have to STOP it first.\r\n", ch);
        else
        {
        	generate_autoquest();
        }
    }
    else
        send_to_char("Usage: iquest start <name> [time] | end | prolong | auto\r\n", ch);
}


int count_questors()
{
    struct descriptor_data *d;
    int i=0;

    for (d = descriptor_list; d; d = d->next)
        if ((d->connected == CON_PLAYING) && (!IS_NPC(d->character)) && QUESTING(d->character))
            i++;
    return i;
}


void init_new_quest()
{
    int i,j,k=10000;

    while (--k>0)
    {
        i=number(0, NUM_QUESTS-1);
        if (number(1, 10)<=quests[i].time)
            k=0;
    }
    if (i>=0 && i<NUM_QUESTS)
        quests[i].questfunc(quests[i].code);
    else
        log("Error in init_new_quest()!");
}



int newquest_update()
{
    if (current_quest!=QUEST_NONE && quest_step==QUEST_JOIN)
    {
        quest_join--;
        if (quest_join==QUEST_JOIN_TIME-1)
        {
		sprintf(buf, "&w[QUEST]:&0 Type 'quest join' to join up for this quest.\r\n");
		INFO_OUT(buf);
	}         	
        
        else if (quest_join==QUEST_JOIN_TIME-2)
        {
		quest_help();
	}         	
        if (quest_join<=0)
        {
            if (count_questors()>=MIN_PLAYERS_FOR_QUEST)
            {
                questchan("&CQUEST BEGINS N O W !!!&0");
                quest_step=QUEST_ACTION;
            }
            else
                end_current_quest("not enough adventurers");
        }
    }
    else if (current_quest!=QUEST_NONE && quest_step==QUEST_ACTION)
    {
        quest_time_left--;
        if (quest_time_left<=0)
            end_current_quest("time is up!");
        else if (count_questors()<1)
            end_current_quest("all of the adventurers have left");
    }
    else
        if (current_quest==QUEST_NONE && last_quest_timer>QUEST_REPEAT && players_online()>MIN_PLAYERS_BEFORE_QUEST)
            init_new_quest();
    return 0;
}



ACMD(do_newquest_players)
{
    char    arg2[MAX_STRING_LENGTH],
    arg1[MAX_STRING_LENGTH],
    buf[MAX_STRING_LENGTH];

    two_arguments(argument, arg1, arg2);
    if ( !*arg1)
    {
        send_to_char("QUEST commands: JOIN.\r\n", ch);
        send_to_char("For more information, type 'HELP QUEST'.\r\n", ch);
        return;
    }
    if (isname(arg1, "join")) {
        if (QUESTING(ch))

            send_to_char("You are already signed up for a quest.\r\n", ch);

        else
        {
            SET_BIT(PRF_FLAGS(ch), PRF_QUEST);
            send_to_char("OK.\r\n", ch);
            sprintf(buf, "\r\n&w[QUEST]:&0 %s joined up the quest!", GET_NAME(ch));
             
            INFO_OUT(buf);
        }
    }
}



