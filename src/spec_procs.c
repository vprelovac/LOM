/*\
*  Use: implementation of special procedures for mobiles/objects/rooms  *
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
#include <sys/types.h>

#include "auction.h"
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
#include "constants.h"
#include "clan.h"

ACMD(do_say);
ACMD(do_gen_comm);

extern int ffield;
int find_first_step(sh_int src, sh_int target);
extern int clan_loadroom[];

typedef struct {
    char ime[15];
    int tornum;
    int rnum;
    int vnum;
} soba;

typedef struct {
    int v[6],
    vn,
    maxnum,
    vnum_room;
    char *s1,*s2;
} gstruct;

int maxgards=9;
gstruct gardovi[] = {
                        {  3099,  3059,  3067,    3060,    -1,    -1, 4,   6, -1, "GUUAAARDS! Trouble at %s!", "UPHOLD THE LAW! CHAAAAAARGE!"},
                        { 15000, 15003, 15004, 15005, 15006, 15007, 6, 999, -1, "Intruders at %s!", "Protect King Welmar's reign! BANZAII!!"},
                        { 15008, 15009, 15010, 15011, 15012, 15013, 6, 999, -1, "Intruders at %s!", "Protect King Welmar's reign! BANZAII!!"},
                        { 15024, 15025, 15026, 15027, 15028, 15029, 6, 999, -1, "Intruders at %s!", "Protect King Welmar's reign! BANZAII!!"},
                        { 15001, 15020, 15021, 15012, 15003, 15024, 6, 999, -1, "All guards come to %s!", "SLAIN THE INTRUDERS!!"},
                        {  5461,  5462,  5463,  5482,    -1,    -1, 4,   7, -1, "GUAAAARDS! Trouble at %s!", "CHAAAARGE!"},
                        { 10204, 10215,    -1,    -1,    -1,    -1, 2,   6, -1, "GUAAAARDS! Trouble at %s! GUAAAARDS!", "PROTECT THE TOWN OF SOLACE!"},
                        { 600, -1,    -1,    -1,    -1,    -1,      1,   4, -1, "Criminals at %s! GUAAAARDS!", "Protect the order in Ofcol! CHAAAAARGE!!"},
                        { 6659,   6660, 6601,    -1,    -1,    -1,      3,   6, -1, "Civil disorder at %s! Come quickly and help me!", "Protect the city of Sundhaven! CHAAAAARGE!!"}
                    };


int maxsoba = 4;

soba sobe[] = {
                  "midgaard", 3001, -1, -1,
                  "rome", 12067, 12067, 101,
                  "drow city", 5100, 5100, 102,
                  "new thalos", 5616, 5616, 103
              };

int a;

static char *headers[] =
    {
        "the corpse of The ",
        "the corpse of the ",
        "the corpse of an ",
        "the corpse of An ",
        "the corpse of a ",
        "the corpse of A ",
        "the corpse of ",
        "the headless corpse of The ",
        "the headless corpse of the ",
        "the headless corpse of an ",
        "the headless corpse of An ",
        "the headless corpse of a ",
        "the headless corpse of A ",
        "the headless corpse of "
    };


/*   external vars  */
extern char *putanja;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct command_info cmd_info[];

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
void advance_char(struct char_data * ch);

struct social_type {
    char *cmd;
    int next_line;
};

extern int classnumskills[];




/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

int spell_sort_info[MAX_SKILLS + 1];

extern char *spells[];

void sort_spells(void)
{
    int a, b, tmp;

    /* initialize array */
    for (a = 1; a <= MAX_SKILLS; a++)
        spell_sort_info[a] = a;

    /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
    for (a = 1; a <= TOP_SPELL_USED - 1; a++)
        for (b = a + 1; b <= TOP_SPELL_USED; b++)
            if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) {
                tmp = spell_sort_info[a];
                spell_sort_info[a] = spell_sort_info[b];
                spell_sort_info[b] = tmp;
            }
    for (a = TOP_SPELL_USED+1; a <= TOP_SKILL_USED - 1; a++)
        for (b = a + 1; b <= TOP_SKILL_USED; b++)
            if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) {
                tmp = spell_sort_info[a];
                spell_sort_info[a] = spell_sort_info[b];
                spell_sort_info[b] = tmp;
            }
}


char *how_good(int percent)
{
    static char buf[256];

    if (percent == 0)
        strcpy(buf, " (unknown)");
    else if (percent <= 10)
        strcpy(buf, " (awful)");
    else if (percent <= 20)
        strcpy(buf, " (bad)");
    else if (percent <= 25)
        strcpy(buf, " (poor)");
    else if (percent <= 35)
        strcpy(buf, " (below avg)");
    else if (percent <= 45)
        strcpy(buf, " (average)");
    else if (percent <= 55)
        strcpy(buf, " (fair)");
    else if (percent <= 65)
        strcpy(buf, " (good)");
    else if (percent <= 75)
        strcpy(buf, " (very good)");
    else if (percent <= 85)
        strcpy(buf, " (excellent)");
    else if (percent <= 95)
        strcpy(buf, " (superb)");
    else
        strcpy(buf, " (master)");

    return (buf);
}

char *prac_types[] = {
                         "spell",
                         "skill"
                     };

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define PRAC_TYPE	1	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
extern int prac_params[2][NUM_CLASSES];

#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS_NUM_FULL(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS_NUM_FULL(ch)]])

int mag_manacost(struct char_data * ch, int spellnum);


ACMD(do_skills)
{
    extern char *spells[];
    extern struct spell_info_type spell_info[];
    int k, i, j, sortpos, temp, flag;
    char buf_1[100];

    *buf2=0;
    for (k=1; k<LVL_IMMORT; k++)
    {
        flag=0;
        for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
            i = spell_sort_info[sortpos];
            if (strlen(buf2) >= MAX_STRING_LENGTH - 100) {
                strcat(buf2, "**OVERFLOW**\r\n");
                break;
            }
            for (j = 0; j < NUM_CLASSES; j++)
                if (spell_info[i].type && IS_SET(GET_CLASS(ch), (1 << j)) && (k== spell_info[i].min_level[j]) && !IS_COMBAT_SKILL(i)) {
                    if (!IS_SPELL(i))
                    {
                    if (!flag)
                        sprintf(buf, "\r\nLevel %2d:&w %-23s &y%3d%%&0",  spell_info[i].min_level[j], spells[i], GET_SKILL(ch, i));
                    else if (!(flag %2))
                        sprintf(buf, "\r\n          &w%-23s &y%3d%%&0",  spells[i], GET_SKILL(ch, i));
                    else
                        sprintf(buf, "     &w%-23s &y%3d%%&0",   spells[i], GET_SKILL(ch, i));
                    }
                    else{
                    if (!flag)
                        sprintf(buf, "\r\nLevel %2d:&C %-23s &y%3d%%&0",  spell_info[i].min_level[j], spells[i], GET_SKILL(ch, i));
                    else if (!(flag %2))
                        sprintf(buf, "\r\n          &C%-23s &y%3d%%&0",  spells[i], GET_SKILL(ch, i));
                    else
                        sprintf(buf, "     &C%-23s &y%3d%%&0",   spells[i], GET_SKILL(ch, i));
                         }
                    flag++;
                    strcat(buf2, buf);
                }

        }
    }
    strcat(buf2, "\r\n");
    page_string(ch->desc, buf2, 1);
}





void list_learned(struct char_data * ch , struct char_data *vict)
{
    extern char *spells[];
    extern struct spell_info_type spell_info[];
    int i, j, sortpos, temp;
    char buf_1[100];

    sprintf(buf, "&GYou are familiar with following abilities:&0\r\n\r\n");

    strcpy(buf2, buf);
    if (IS_CASTER(ch))
    	strcat(buf2, "Name                Learned                                   Cost    Rnds\r\n----                -------                                   ----    ----\r\n");
    else		
    	strcat(buf2, "Name                Learned\r\n----                -------\r\n");
    
    
    for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
        i = spell_sort_info[sortpos];
        if (strlen(buf2) >= MAX_STRING_LENGTH - 100) {            
            strcat(buf2, "**OVERFLOW**\r\n");
            break;
        }
        //for (j = 0; j < NUM_CLASSES; j++) {
        //   if (IS_SET(GET_CLASS(ch), (1 << j)) && (GET_LEVEL(ch) >= spell_info[i].min_level[j])) {
        if (GET_SKILL(ch, i) > 0 && !IS_COMBAT_SKILL(i) && !IS_TRAP_SKILL(i) && i!=SKILL_ENH_SIGHT && strlen(spells[i])>2) {
            if (IS_SPELL(i)) {
                sprintf(buf, "&C%-22s &y%3d%%&0 [&c%s&0]", spells[i], GET_SKILL(ch, i),make_bar(buf_1, GET_SKILL(ch, i), 30) );
                strcat(buf2, buf);
                if ((temp = mag_manacost(ch, i)))
                    sprintf(buf, "  &C%4d&0", temp);
                else
                    sprintf(buf, "&cn/a&0\r\n");
                strcat(buf2, buf);
                temp = cast_time(ch, i);
                sprintf(buf, "     &C%2.1f&0", temp/(float) PULSE_VIOLENCE);                
                strcat(buf2, buf);
                strcat(buf2, "\r\n");
                
            }
            else
            {
                sprintf(buf, "&w%-22s &y%3d%%&0 [&c%s&0]\r\n", spells[i], GET_SKILL(ch, i),make_bar(buf_1, GET_SKILL(ch, i), 30) );
                strcat(buf2, buf);
            }

        }
        //}
        //}
    }

    if (!vict)
        page_string(ch->desc, buf2, 1);
    else
        page_string(vict->desc, buf2, 1);
}



void list_traps(struct char_data * ch , struct char_data *vict)
{
    extern char *spells[];
    extern struct spell_info_type spell_info[];
    int i, j, sortpos, temp;
    char buf_1[100];




    *buf2=0;
    for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
        i = spell_sort_info[sortpos];
        if (strlen(buf2) >= MAX_STRING_LENGTH - 100) {
            /*if (!vict)
            	page_string(ch->desc, buf2, 1);
            else
            	page_string(vict->desc, buf2, 1);	    

            *buf2=0;	    	*/
            strcat(buf2, "**OVERFLOW**\r\n");
            break;
        }
        for (j = 0; j < NUM_CLASSES; j++) {
            if (IS_SET(GET_CLASS(ch), (1 << j)) && (GET_LEVEL(ch) >= spell_info[i].min_level[j])) {
                if (GET_SKILL(ch, i) > 0 && IS_TRAP_SKILL(i)) {
                    float time;
                    int move;
                    time=(float) MAX(500, 14*((600-3.0*GET_SKILL(ch, i))*(40-GET_DEX(ch)))/10) /1500.0;
                    move=(210-GET_SKILL(ch, i))*(150-4*GET_DEX(ch))/100;
                    sprintf(buf, "&w%-22s&0[&c%s&0] &y%3d%%&0  Move: &c%3d&0 Time: &c%2.1f&0 s\r\n", spells[i], make_bar(buf_1, GET_SKILL(ch, i), 25), GET_SKILL(ch, i), move, time);
                    strcat(buf2, buf);
                    break;
                }
            }
        }
    }
    if (*buf2)
    {
        send_to_char("&CKnown traps:\r\n&c============&0\r\n\r\n", ch);
        if (!vict)
            page_string(ch->desc, buf2, 1);
        else
            page_string(vict->desc, buf2, 1);
    }
    else
        send_to_char("You dont know any traps.\r\n", ch);
}




void list_skills(struct char_data * ch , struct char_data *vict)
{
    extern char *spells[];
    extern struct spell_info_type spell_info[];
    int i, j, sortpos, temp;
    int par=1, b1;
    float base;
    char buf_1[100];

    /*    if (!GET_PRACTICES(ch))
    	strcpy(buf, "You have no practice sessions remaining.\r\n");
        else
    	sprintf(buf, "You have %d practice session%s remaining.\r\n",
    		GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));
    */
    sprintf(buf, "You currently need %d exp per level.\r\nYou are able to learn following abilities:\r\n\r\n", LEVELEXP(ch));


    strcpy(buf2, buf);
    strcat(buf2, "Name                   Level     Gold      Exp add\r\n----                   -----     ----      -------\r\n");

    for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
        i = spell_sort_info[sortpos];
        if (strlen(buf2) >= MAX_STRING_LENGTH - 100) {

            /*if (!vict)
               		page_string(ch->desc, buf2, 1);
               	else
               		page_string(vict->desc, buf2, 1);	    
                
                *buf2=0;	*/
            strcat(buf2, "**OVERFLOW**\r\n");
            break;
        }
        for (j = 0; j < NUM_CLASSES; j++) {
            if (spell_info[i].type && IS_SET(GET_CLASS(ch), (1 << j)) && (GET_LEVEL(ch) >= spell_info[i].min_level[j])) {
                //sprintf(buf, "&w%-24s&c %-18.18s&0 LV: %-2d", spells[i], how_good(GET_SKILL(ch, i)), spell_info[i].min_level[j]);
                if (!GET_SKILL(ch, i))
                {
                    par++;
                    base=(float) TOTAL_MOBNUM/((float) classnumskills[GET_CLASS_NUM(ch)]*(LVL_IMMORT-GET_LEVEL(ch)));
                    base+=5.0*base*(GET_LEVEL(ch)-spell_info[i].min_level[j])/100.0;

                    b1=base*MOB_EXP_BASE;

                    if (IS_SPELL(i)) {
                        sprintf(buf, "%s%-24s %-2d",par%2?"&y":"&c", spells[i], spell_info[i].min_level[j]);
                        strcat(buf2, buf);
                        if ((temp = mag_manacost(ch, i)))
                        {

                            int gold=spell_info[i].min_level[j];
                            gold=MAX(1,gold*gold*gold/25);
                            if (spell_info[i].mana_change == 111)
                                gold=spell_info[i].mana_max*spell_info[i].mana_max;
                            sprintf(buf, "     %5d", gold);

                        }
                        else
                            sprintf(buf, "n/a");
                        strcat(buf2, buf);
                        sprintf(buf,"       %-5d&0\r\n", b1);
                        strcat(buf2, buf);
                        break;
                    } else {
                        int gold=spell_info[i].min_level[j];
                        sprintf(buf, "%s%-24s %-2d",par%2?"&y":"&c", spells[i], spell_info[i].min_level[j]);
                        strcat(buf2, buf);

                        gold=MAX(1,gold*gold*gold/30);
                        sprintf(buf, "     %5d", gold);
                        strcat(buf2, buf);
                        sprintf(buf,"       %-5d&0\r\n", b1);
                        strcat(buf2, buf);
                        break;
                    }


                }
            }
        }
    }

    strcat(buf2, "\r\n\r\nTo train type '&ctrain&0' at your guildmaster.\r\nTo see learned abilitites type '&clearned&0'.\r\nTo see your combat skills type '&ccombat&0'.\r\nTo see list of all skills available to your class type '&cskills&0'.\r\n");
    if (!vict)
        page_string(ch->desc, buf2, 1);
    else
        page_string(vict->desc, buf2, 1);
}

int do_guild(struct char_data * ch, void *me, int cmd, char *argument, int skilltype);

SPECIAL(mageguild)
{
    return do_guild(ch, me, cmd, argument, SKILL_TYPE_MAGIC);
}

SPECIAL(rangerguild)
{
    return do_guild(ch, me, cmd, argument, SKILL_TYPE_RANGER);
}


SPECIAL(clericguild)
{
    return do_guild(ch, me, cmd, argument, SKILL_TYPE_CLERIC);
}

SPECIAL(thiefguild)
{
    return do_guild(ch, me, cmd, argument, SKILL_TYPE_THIEF);
}

SPECIAL(fighterguild)
{
    return do_guild(ch, me, cmd, argument, SKILL_TYPE_FIGHTER);
}

SPECIAL(othersguild)
{
    return do_guild(ch, me, cmd, argument, SKILL_TYPE_ALL);
}

int do_guild(struct char_data * ch, void *me, int cmd, char *argument, int skilltype)
{
    int skill_num, percent, i, maxgain, mingain;
    char bufma[200];
    extern struct spell_info_type spell_info[];
    extern struct int_app_type int_app[36];
    float base;
    int b1;
    int a=0;

    if (IS_NPC(ch) || (!(CMD_IS("practice")) && !(CMD_IS("train"))))
        return 0;

    skip_spaces(&argument);

    /*  if (CMD_IS("advance")) {
        advance_char(ch);
        return 1;
      }*/

    if (CMD_IS("train"))
    {
    	if (!*argument)
    	{
    	        send_to_char("You can train hitpoints, energy or movement.\r\nThe cost is ten adventuring points per one point trained.\r\nYou would also need one gold point per your level to train.\r\n", ch);
        	return 1;
        }
	if (GET_QUESTPOINTS(ch)<10)
	{
			send_to_char("You need at least ten adventuring points to train.\r\n", ch);
			return 1;
	}        
	if (GET_GOLD(ch)<GET_LEVEL(ch))
	{
			ch_printf(ch, "You need at least %d gold coins to train.\r\n", GET_LEVEL(ch));
			return 1;
	}        
        if (isname(argument, "hitpoints"))
        {
        	GET_MAX_HIT(ch)+=1;        
        	GET_HITR(ch)+=1;
        	send_to_char("You train for a while.\r\n", ch);
        	send_to_char("\r\n&c+&0 You gain one hitpoint &c+&0\r\n", ch);
        	GET_QUESTPOINTS(ch)-=10;                                                            
        	GET_GOLD(ch)-=GET_LEVEL(ch);
        	return 1;
        }
        if (isname(argument, "energy"))
        {
        	GET_MAX_MANA(ch)+=1;        
        	GET_MANAR(ch)+=1;
		send_to_char("You train for a while.\r\n", ch);
        	send_to_char("\r\n&c+&0 You gain one energy &c+&0\r\n", ch);
        	GET_QUESTPOINTS(ch)-=10;    
        	GET_GOLD(ch)-=GET_LEVEL(ch);
        	return 1;
        }
        if (isname(argument, "movement"))
        {
        	GET_MAX_MOVE(ch)+=1;        
        	GET_MOVER(ch)+=1;
        	send_to_char("You train for a while.\r\n", ch);
        	send_to_char("\r\n&c+&0 You gain one movement &c+&0\r\n", ch);
        	GET_QUESTPOINTS(ch)-=10;    
        	GET_GOLD(ch)-=GET_LEVEL(ch);
        	return 1;
        }      	        	
        
        send_to_char("Train hitpoints, energy or movement?\r\n", ch);
        return 1;
        
    }
    
    



    if (!*argument) {
        list_skills(ch, NULL);
        return 1;
    }
    if (GET_PRACTICES(ch) <= 0) {
        send_to_char("You do not seem to be able to practice now.\r\n", ch);
        return 1;
    }
    skill_num = find_skill_num(argument);

    if (skill_num==-1)
    {
        send_to_char("I am not familiar with that...\r\n",ch);
        return 1;
    }

    if (skilltype==SKILL_TYPE_RANGER && !IS_RANGER(ch))
    {
        send_to_char("You can't learn that here.\r\n", ch);
        return 1;
    }
    /*    if (!IS_SET(skilltype, spell_info[skill_num].type)) {
        	send_to_char("Sorry, we do not teach that here...\r\n", ch);
    	    return 1;
        }*/
    /*    if (GET_LEVEL(ch) < spell_info[skill_num].min_level[GET_CLASS(ch)])
        {	
        	send_to_char("You can't learn that now.\r\n",ch);
        	return;
        }*/
    if (GET_SKILL(ch, skill_num)>0)
    {
        send_to_char("You already have the basic knowledge in that..\r\n",ch);
        return;
    }
    if (GET_SKILL(ch, skill_num) >= LEARNED(ch)) {
        send_to_char("But you are already master in that area, my friend. I can teach you no more...'\r\n", ch);
    } else if (skill_num > 0) {
        *bufma='\0';
        for (i = 0; i < NUM_CLASSES; i++) {
            if (spell_info[skill_num].type && IS_SET(GET_CLASS(ch), (1 << i)) && GET_LEVEL(ch) >= spell_info[skill_num].min_level[i]) {

                int gold=spell_info[skill_num].min_level[i];
                if (IS_SPELL(skill_num))
                    gold=MAX(1,gold*gold*gold/25);
                else
                    gold=MAX(1,gold*gold*gold/30);

                if (spell_info[skill_num].mana_change == 111)
                    gold=spell_info[skill_num].mana_max*spell_info[skill_num].mana_max;
                if (GET_GOLD(ch)<gold)
                {
                    send_to_char("You do not have enough gold to practice that.\r\n", ch);
                    return;
                };
                sprintf(bufma, "You hand %d gold to %s.\r\n", gold, CAP(GET_NAME((struct char_data *)me)));
                send_to_char(bufma, ch);
                GET_GOLD(ch)-=gold;

                if (IS_SPELL(skill_num))
                {
                    if (!IS_CLERIC(ch))
                    {
                        sprintf(bufma, "%s teaches you how to cast '&c%s&0'.\r\n", CAP(GET_NAME((struct char_data *)me)), spells[skill_num]);
                        send_to_char(bufma, ch);
                        send_to_char("\r\n&c+&0 You have learned a new spell &c+&0\r\n", ch);
                    }
                    else
                    {
                        sprintf(bufma, "%s reveals to you the words of the '&c%s&0' prayer.\r\n", CAP(GET_NAME((struct char_data *)me)), spells[skill_num]);
                        send_to_char(bufma, ch);
                        send_to_char("\r\n&c+&0 You have learned a new prayer &c+&0\r\n", ch);
                    }
                }
                else
                {
                    sprintf(bufma, "%s skillfully shows you the basics of the &c%s&0 skill.\r\n", CAP(GET_NAME((struct char_data *)me)), spells[skill_num]);
                    send_to_char(bufma, ch);
                    send_to_char("\r\n&c+&0 You have learned a new skill &c+&0\r\n", ch);
                }


                percent = 2*GET_INTR(ch)+2*GET_WISR(ch);                
                	
                if (!IS_SPELL(skill_num) && (!IS_CASTER(ch) || IS_RANGER(ch)))
                    percent+=percent/4;
                
                if (FOL_AZ(ch))
                	percent+=10;
    
                percent=MIN(100, percent);
                //if (spell_info[skill_num].mana_change == 111)
                //    percent/=5;
                //		percent += GET_SKILL(ch, skill_num);                
                SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

                if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
                    send_to_char("&CYou are now master in that area.&0\r\n", ch);

                base=(float) TOTAL_MOBNUM/((float) classnumskills[GET_CLASS_NUM(ch)]*(LVL_IMMORT-GET_LEVEL(ch)));
                base+=5.0*base*(GET_LEVEL(ch)-spell_info[skill_num].min_level[i])/100.0;
                GET_MOBRATIO(ch)+=base;

                return 1;
            }
        }
        if (*bufma=='\0')
        {
            send_to_char("You are not ready for that.\r\n",ch);
        }
    } else {
        sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
        send_to_char(buf, ch);
    }
    return 1;
}

int do_sailor(struct char_data * ch, void *me, int cmd, char *argument)
/*SPECIAL(trainer)*/
{
    char buf[256], buf1[256];
    bool str, dex, con, intel, wis, cha;
    int pom;
    if (IS_NPC(ch) || !CMD_IS("train"))
        return 0;
    if (!IS_NPC(ch) && CMD_IS("train"))
        do_say((struct char_data *) me, "Sorry, I am retired.",0,0);
    return 1;
    str = (ch->real_abils.str < 25);
    dex = (ch->real_abils.dex < 25);
    con = (ch->real_abils.con < 25);
    intel = (ch->real_abils.intel < 25);
    wis = (ch->real_abils.wis < 25);
    cha = (ch->real_abils.cha < 25);
    skip_spaces(&argument);
    if (!(*argument)) {
        sprintf(buf, "It will cost you 5 practices and %d gold to train.\r\n", 100 * GET_LEVEL(ch) * GET_LEVEL(ch));
        send_to_char(buf, ch);
        send_to_char("You can increase one of your attributes or h/m/v points.\r\r\n\n", ch);
        send_to_char("You can train:\r\n", ch);
        sprintf(buf, "    hit mana move  ");
        if (str)
            strcat(buf, " str");
        if (dex)
            strcat(buf, " dex");
        if (con)
            strcat(buf, " con");
        if (intel)
            strcat(buf, " int");
        if (wis)
            strcat(buf, " wis");
        if (cha)
            strcat(buf, " cha");
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
        return 1;
    }
    if (GET_PRACTICES(ch) < 5) {
        send_to_char("You do not have enough practices to train right now.\r\n", ch);
        return 1;
    }
    if (GET_GOLD(ch) < 100 * GET_LEVEL(ch) * GET_LEVEL(ch)) {
        send_to_char("You do not have enough gold to train right now.\r\n", ch);
        return 1;
    }
    pom = 0;
    if (!strcmp(argument, "hit")) {
        pom = 1;
        GET_MAX_HIT(ch) += number(5, 10);
    }
    if (!strcmp(argument, "mana")) {
        pom = 1;
        GET_MAX_MANA(ch) += number(5, 10);
    }
    if (!strcmp(argument, "move")) {
        pom = 1;
        GET_MAX_MOVE(ch) += number(5, 25);
    }
    if (!strcmp(argument, "str")) {
        pom = 1;
        if (str)
            ch->real_abils.str += 1;
        else
            pom = 2;
    }
    if (!strcmp(argument, "dex")) {
        pom = 1;
        if (dex)
            ch->real_abils.dex += 1;
        else
            pom = 2;
    }
    if (!strcmp(argument, "wis")) {
        pom = 1;
        if (wis)
            ch->real_abils.wis += 1;
        else
            pom = 2;
    }
    if (!strcmp(argument, "con")) {
        pom = 1;
        if (con)
            ch->real_abils.con += 1;
        else
            pom = 2;
    }
    if (!strcmp(argument, "cha")) {
        pom = 1;
        if (cha)
            ch->real_abils.cha += 1;
        else
            pom = 2;
    }
    if (!strcmp(argument, "int")) {
        pom = 1;
        if (intel)
            ch->real_abils.intel += 1;
        else
            pom = 2;
    }
    if (pom == 0) {
        send_to_char("You can't train that.\r\n", ch);
        return 1;
    }
    if (pom == 2) {
        send_to_char("You are already at maximum.\r\n", ch);
        return 1;
    }
    GET_PRACTICES(ch) -= 5;
    GET_GOLD(ch) -= 100 * GET_LEVEL(ch) * GET_LEVEL(ch);
    affect_total(ch);
    send_to_char("You train for a while...\r\n", ch);
    return 1;
}

SPECIAL(sailor)
{
    return do_sailor(ch, me, cmd, argument);
}

SPECIAL(dump)
{
    struct obj_data *k;
    int value = 0;

    ACMD(do_drop);
    char *fname(char *namelist);

    for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
        act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
        extract_obj(k);
    }

    if (!CMD_IS("drop"))
        return 0;

    do_drop(ch, argument, cmd, 0);

    for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
        act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
        value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
        extract_obj(k);
    }

    if (value) {
        act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

        if (GET_LEVEL(ch) < 3 && value>5)
            gain_exp(ch, value);
        else
            GET_GOLD(ch) += value;
    }
    return 1;
}


SPECIAL(mayor)
{
    ACMD(do_gen_door);

    static char open_path[] =
        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static char close_path[] =
        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static char *path;
    static int index;
    static bool move = FALSE;

    if (!move) {
        if (time_info.hours == 6) {
            move = TRUE;
            path = open_path;
            index = 0;
        } else if (time_info.hours == 20) {
            move = TRUE;
            path = close_path;
            index = 0;
        }
    }
    if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
            (GET_POS(ch) == POS_FIGHTING))
        return FALSE;

    switch (path[index]) {
    case '0':
    case '1':
    case '2':
    case '3':
        perform_move(ch, path[index] - '0', 1);
        break;

    case 'W':
        GET_POS(ch) = POS_STANDING;
        act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'S':
        GET_POS(ch) = POS_SLEEPING;
        act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'a':
        act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'b':
        act("$n says 'What a view!  I must get something done about that dump!'",
            FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'c':
        act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
            FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'd':
        act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'e':
        act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'E':
        act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'O':
        do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
        do_gen_door(ch, "gate", 0, SCMD_OPEN);
        break;

    case 'C':
        do_gen_door(ch, "gate", 0, SCMD_CLOSE);
        do_gen_door(ch, "gate", 0, SCMD_LOCK);
        break;

    case '.':
        move = FALSE;
        break;

    }

    index++;
    return FALSE;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */

void npc_steal(struct char_data * ch, struct char_data * victim)
{
    int gold;

    if (IS_NPC(victim))
        return;
    if (GET_LEVEL(victim) >= LVL_IMMORT)
        return;

    if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
        act("You discover that $n has $s hands in your money pouch.", FALSE, ch, 0, victim, TO_VICT);
        act("$n tries to steal some coins from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
        /* Steal some gold coins */
        gold = (int) ((GET_GOLD(victim) * number(1, 20)) / 100);
        if (gold > 0) {
            GET_GOLD(ch) += gold;
            GET_GOLD(victim) -= gold;
        }
    }
}

SPECIAL(icewizard)
{

    struct char_data *tch, *vict;
    int low_on_hits = 10000;

    if (cmd)
        return (FALSE);

    /* Find out who has the lowest hitpoints and burn his ass off */
    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if (!IS_NPC(tch))
            if (GET_HIT(tch) < low_on_hits) {
                low_on_hits = GET_HIT(tch);
                vict = tch;
            }
    }

    act("$n screams 'Bonjour! you tiny, little looser!!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n looks at $N", 1, ch, 0, vict, TO_NOTVICT);
    act("$n looks at YOU!", 1, ch, 0, vict, TO_VICT);
    cast_spell(ch, vict, 0, SPELL_CONE_OF_COLD, 0);
    return TRUE;
}





SPECIAL(snake)
{
    if (cmd)
        return FALSE;

    if (GET_POS(ch) != POS_FIGHTING)
        return FALSE;

    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) && !number(0, 7)) {
        act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
        act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
        call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL, 0);
        return TRUE;
    }
    return FALSE;
}


SPECIAL(thief)
{
    struct char_data *cons;

    if (cmd)
        return FALSE;

    if (GET_POS(ch) != POS_STANDING)
        return FALSE;

    for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
        if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && CAN_SEE(ch, cons) && (!number(0, 3))) {
            npc_steal(ch, cons);
            return TRUE;
        }
    return FALSE;
}


void fight_warrior(struct char_data *ch)
{
    struct char_data *vict;
    int num, spell=-1, min_level;

    if (!ch || ch->in_room==NOWHERE || !FIGHTING(ch))
        return;


    vict=FIGHTING(ch);
    if (GET_RACE(ch)!=RACE_ANIMAL && GET_RACE(ch)!=RACE_SMALL_ANIMAL)
    {
        if (GET_LEVEL(ch)>=36 && !number(0, 8))
            do_melee(ch,"", 0, 0);
        else if (GET_LEVEL(ch)>=33 && GET_EQ(ch, WEAR_HEAD) && !number(0, 1))
            do_headbutt(ch, "", 0, 0);
        else if (GET_LEVEL(ch)>=32 && !number(0, MAX(50, 160-2*GET_LEVEL(ch))))
            do_berserk(ch, "", 0, 0);
        else if (GET_LEVEL(ch)>=27 && !number(0, 6))
            do_disarm(ch, "", 0, 0);
        else if (GET_LEVEL(ch)>=5 && GET_EQ(ch, WEAR_SHIELD) && !number(0, 1))
            do_bash(ch, "", 0, 0);
        else if (GET_LEVEL(ch)>=10 && !number(0, 1))
            do_kick(ch, "", 0, 0);
    }
    else if (number(0, 2) && GET_LEVEL(ch)>=LEVEL_NEWBIE)
        hit(ch, vict, TYPE_BITE);
}


void fight_thief(struct char_data *ch)
{
    struct char_data *vict;
    int num, spell=-1, min_level;

    if (!ch || ch->in_room==NOWHERE || !FIGHTING(ch))
        return;


    vict=FIGHTING(ch);

    if (GET_LEVEL(ch)>=22 && !number(0, 3))
        do_dirtkick(ch,"", 0, 0);
    else if (GET_LEVEL(ch)>=16 && number(0, 2))
        do_circle(ch, "", 0, 0);
    else if (GET_LEVEL(ch)>=10 && !number(0, 1))
        do_trip(ch, "", 0, 0);

}


void fight_mage(struct char_data *ch)
{
    struct char_data *vict;
    int num, spell=-1, min_level;

    if (!ch || ch->in_room==NOWHERE)
        return;

    //if (!FIGHTING(ch))
    {
        //	if (GET_HIT(ch)<GET_MAX_HIT(ch))
        if (GET_LEVEL(ch)>=25 && GET_HIT(ch)>3*GET_MAX_HIT(ch)/5)
        {   
            again:
            switch (number(1, 6)) {
            case 1: spell=SPELL_BLINK;break;
            case 2: spell=SPELL_MIRROR_IMAGE;break;
            case 3: spell=SPELL_STONESKIN;break;
            case 4: spell=SPELL_FIRE_SHIELD;break;
            case 5: spell=SPELL_INTESIFY;break;
            case 6: spell=SPELL_HASTE;break;
            }
            if (!affected_by_spell(ch, spell))
            {
            	sprintf(buf, " '%s' ", spells[spell]);
    		do_cast(ch, buf,0,0);
                /*cast_spell(ch, ch, NULL, spell, 0);
                WAIT_STATE(ch, PULSE_SPELL);*/
                return;
            }
            else if (!number(0, 1))
            	goto again;
        }

        //return;
    }


    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch && !number(0, 4))
            break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
        vict = FIGHTING(ch);






    switch (number(3*GET_LEVEL(ch)/4,GET_LEVEL(ch))) {
    case 0:
        break;
    case 1:
    case 2:
        spell = SPELL_MAGIC_MISSILE;
        break;
    case 3:
    case 4:
        spell = SPELL_CHILL_TOUCH;
        break;
    case 5:
    case 6:
        spell = SPELL_BURNING_HANDS;
        break;
    case 7:
    case 8:
        spell = SPELL_SHOCKING_GRASP;
        break;
    case 9:
    case 10:
        spell = SPELL_LIGHTNING_BOLT;
        break;
    case 11:
    case 12:
    case 13:
    case 14:
        spell = SPELL_COLOR_SPRAY;
        break;
    default:
        if (GET_LEVEL(ch)>40 && !number(0,3))
            spell= SPELL_DISINTIGRATE;
        else if (GET_LEVEL(ch)>30 && !number(0,2))
            spell= SPELL_MINUTE_METEOR;
        else if (GET_LEVEL(ch)>25 && !number(0,1))
            spell= SPELL_ACID_BLAST;
        else
            spell= SPELL_FIREBALL;
        break;
    }          
     sprintf(buf, " '%s' ", spells[spell]);
    do_cast(ch, buf,0,0);
    //cast_spell(ch, vict, NULL, spell, 0);
    //WAIT_STATE(ch, PULSE_SPELL);
}


void fight_cleric(struct char_data *ch)
{
    struct char_data *vict;
    int num, spell=-1, min_level;
    bool self=FALSE;

    if (!ch || ch->in_room==NOWHERE || !FIGHTING(ch))
        return;

    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch && !number(0, 4))
            break;

    if (vict == NULL)
        vict = FIGHTING(ch);

	if (GET_LEVEL(ch)>=25 && GET_HIT(ch)>3*GET_MAX_HIT(ch)/4)
        {   
            again:
            switch (number(1, 4)) {
            case 1: spell=PRAYER_SHIELD_OF_FAITH;break;
            case 2: if (GET_LEVEL(ch)>=40)
            		spell=PRAYER_DIVINE_P;
            	else if (GET_LEVEL(ch)>=30)
            		spell=PRAYER_GREATER_P;
            		else
            		spell=PRAYER_MAJOR_P;
            		break;
            case 3: spell=SPELL_FORCE_FIELD;break;            
            case 4: spell=SPELL_ADRENALIN;break;        
            		
            		
            }
            if (!affected_by_spell(ch, spell))
            {
            	sprintf(buf, " '%s' ", spells[spell]);
    		do_cast(ch, buf,0,SCMD_PRAY);
                /*cast_spell(ch, ch, NULL, spell, 0);
                WAIT_STATE(ch, PULSE_SPELL);*/
                return;
            }
            else if (!number(0, 1))
            	goto again;
        }


    switch (number(3*GET_LEVEL(ch)/4,GET_LEVEL(ch))) {
    case 0:
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        spell = SPELL_CURE_LIGHT;
        break;
    case 5:
    case 6:
    case 7:
        spell = SPELL_CURE_SERIOUS;
        break;
    case 8:
    case 9:
    	spell = SPELL_CURE_CRITIC;
    case 11:
    case 12:
    case 13:
        spell = PRAYER_PUNISHMENT;
        break;
    default:
        if (GET_LEVEL(ch)>33 && GET_HIT(ch)<GET_MAX_HIT(ch)/2 && !number(0,1))
            spell=SPELL_POWER_HEAL;
        else if (GET_LEVEL(ch)>=27 && !number(0, 1))
    	{
    		if (IS_GOOD(ch) && IS_EVIL(vict))
    			spell=PRAYER_SMITE_EVIL;
    		else	if (IS_GOOD(vict) && IS_EVIL(ch))
    			spell=PRAYER_SMITE_GOOD;
    		else        
    			spell=PRAYER_JUDGMENT; 
    	}        
        else if (GET_LEVEL(ch)>17 && !number(0,1))
        {            
            spell=SPELL_SPIRITUAL_HAMMER;
        }
        else if (GET_LEVEL(ch)>17 && !number(0,1))
        {
            
            spell=SPELL_FLAMESTRIKE;
        }
        else if (GET_LEVEL(ch)>14 && !number(0,1))
        {
            
            spell=SPELL_HARM;
        }
        else if (GET_LEVEL(ch)>=13 && !number(0, 1))
    	{
    		if (IS_GOOD(ch) && IS_EVIL(vict))
    			spell=PRAYER_DISPEL_EVIL;
    		else	if (IS_GOOD(vict) && IS_EVIL(ch))
    			spell=PRAYER_DISPEL_GOOD;        
    		else spell=PRAYER_PUNISHMENT;
    	}
        else if (!number(0,1))
            spell=SPELL_HEAL;
    }

    if (spell!=-1)
    {   
    	
    	if (FOL_MUGRAK(ch))
    		sprintf(buf, " '%s' %s", spells[spell], self?"":(vict?GET_NAME(vict):""));
    	else                                          
    		sprintf(buf, " '%s' ", spells[spell]);
        	
        	do_cast(ch, buf,0,SCMD_PRAY);
        	
        
        }
        
   
}



SPECIAL(magic_user)
{
    struct char_data *vict;
    int num, spell, min_level;
    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;

    if (number(0, 200) > MIN(50, 2 * GET_LEVEL(ch)))
        return TRUE;

    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch && !number(0, 4))
            break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
        vict = FIGHTING(ch);

    for (;;) {
        num = number(0, 16);
        switch (num) {
        case 0:
            min_level = 0;
            spell = SPELL_MAGIC_MISSILE;
            break;
        case 1:
            min_level = 3;
            spell = SPELL_CHILL_TOUCH;
            break;
        case 2:
            min_level = 5;
            spell = SPELL_BURNING_HANDS;
            break;
        case 3:
            min_level = 8;
            spell = SPELL_SHOCKING_GRASP;
            break;
        case 4:
            min_level = 11;
            spell = SPELL_LIGHTNING_BOLT;
            break;
        case 5:
            min_level = 12;
            spell = SPELL_COLOR_SPRAY;
            break;
        case 6:
            min_level = 13;
            spell = SPELL_ENERGY_DRAIN;
            break;
        case 7:
            min_level = 20;
            spell = SPELL_BLINK;
            break;
        case 8:
        case 9:
            min_level = 15;
            spell = SPELL_DISPEL_MAGIC;
            break;
        case 10:
        case 11:
            min_level = 30;
            spell = SPELL_MINUTE_METEOR;
        default:
            min_level = 17;
            spell = SPELL_FIREBALL;
            break;
        }

        if (GET_LEVEL(ch) >= min_level)
            break;
    }

    if (spell != SPELL_BLINK)
        cast_spell(ch, vict, NULL, spell, 0);
    else
        cast_spell(ch, NULL, NULL, SPELL_BLINK, 0);
    return TRUE;

}

void dodragon(struct char_data * ch, int spellnum)
{
    struct char_data *vict;

    vict = FIGHTING(ch);
    if (!number(0, 4))
        cast_spell(ch, vict, NULL, spellnum, 0);
}


SPECIAL(dragon)
{
    int num;
    struct char_data *vict;
    int freqmod = 0;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;


    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if ((FIGHTING(vict) == ch) && !number(0, 4))
            break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
        vict = FIGHTING(ch);

    /* as mobs typically range from level 1 to level 120 on wintermute,
       the algorithm below gives a cast chance of 1 in 8 to 14 pulses.
       this, of course, is still limited by saves and the right conditions.
       this is an attempt to make one proc apply to all casters.
       change the freqmod variable (-2 minimum) to adjust the cast chance. */

    if (!number(0, 3)) {
        switch (number(1,3)) {
        case 1:
            act("$n opens $s mouth and BREATHES FIRE in your direction!'", 1, ch, 0, 0, TO_ROOM);
            call_magic(ch, vict,0, SPELL_FIREBALL, GET_LEVEL(ch), CAST_SPELL, "");
            //    cast_spell(ch, vict, NULL, SPELL_FIRE_BREATH, 0);
            break;
        case 2:
            act("$n BLASTS you with a COLUMN OF FLAME!'", 1, ch, 0, 0, TO_ROOM);
            call_magic(ch, vict,0, SPELL_FLAMESTRIKE, GET_LEVEL(ch), CAST_SPELL, "");
            //  cast_spell(ch, vict, NULL, SPELL_FLAMESTRIKE, 0);
            break;
        case 3:
            act("$n uses his massive weight to SHAKE THE GROUND!'", 1, ch, 0, 0, TO_ROOM);
            call_magic(ch, vict,0, SPELL_EARTHQUAKE, GET_LEVEL(ch), CAST_SPELL, "");
            //    cast_spell(ch, vict, NULL, SPELL_EARTHQUAKE, 0);
            break;
        }
        return;
    }


    num = number(1, GET_LEVEL(ch));
    if (num < 10)
        dodragon(ch, SPELL_FROST_BREATH);
    else if (num < 18)
        dodragon(ch, SPELL_GAS_BREATH);
    else if (num < 27)
        dodragon(ch, SPELL_FIRE_BREATH);
    else if (num < 36)
        dodragon(ch, SPELL_LIGHTNING_BREATH);
    else
        dodragon(ch, SPELL_ACID_BREATH);
    return TRUE;
}

SPECIAL(gas_dragon)
{
    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;
    dodragon(ch, SPELL_GAS_BREATH);
    return TRUE;
}

SPECIAL(lightning_dragon)
{
    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;
    dodragon(ch, SPELL_LIGHTNING_BREATH);
    return TRUE;
}
SPECIAL(fire_dragon)
{
    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;
    dodragon(ch, SPELL_FIRE_BREATH);
    return TRUE;
}

SPECIAL(cold_dragon)
{
    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;
    dodragon(ch, SPELL_FROST_BREATH);
    return TRUE;
}

SPECIAL(acid_dragon)
{
    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;
    dodragon(ch, SPELL_ACID_BREATH);
    return TRUE;
}



SPECIAL(knight)
{
    struct char_data *vict;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;

    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch && !number(0, 4))
            break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
        vict = FIGHTING(ch);

    if (number(0, 2) && cast_spell(ch, vict, NULL, SPELL_BLADEBARRIER, 0));

    return TRUE;

}


SPECIAL(cleric)
{
    struct char_data *vict;
    int min_level, num, spell;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;

    if (number(0, 200) > MIN(50, 2 * GET_LEVEL(ch)))
        return TRUE;

    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch && !number(0, 4))
            break;

    if (vict == NULL)
        vict = FIGHTING(ch);

    for (;;) {
        num = number(1, 11);
        switch (num) {
        case 1:
            min_level = 0;
            spell = SPELL_BLINDNESS;
            break;
        case 2:
            min_level = 7;
            spell = SPELL_EARTHQUAKE;
            break;
        case 3:
            min_level = 12;
            spell = SPELL_CURSE;
            break;
        case 4:
            min_level = 15;
            spell = SPELL_HARM;
            break;
        case 5:
            min_level = 30;
            spell = SPELL_HOLY_WORD;
            break;
        case 6:
            min_level = 2;
            spell = SPELL_CURE_LIGHT;
            vict = NULL;
            break;
        case 7:
            min_level = 12;
            spell = SPELL_CURE_CRITIC;
            vict = NULL;
            break;
        case 8:
            min_level = 17;
            spell = SPELL_HEAL;
            break;
        default:
            min_level = 20;
            spell = SPELL_DISPEL_MAGIC;
            break;
        }

        if (GET_LEVEL(ch) >= min_level)
            break;
    }

    cast_spell(ch, vict, NULL, spell, 0);
    return TRUE;
}

SPECIAL(vampire)
{
    struct char_data *vict;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;

    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch && !number(0, 4))
            break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
        vict = FIGHTING(ch);

    if ((GET_LEVEL(ch) > 17) && (number(0, 9) == 0))
        cast_spell(ch, vict, NULL, SPELL_HARM, 0);

    if ((GET_LEVEL(ch) > 7) && (number(0, 7) == 0))
        if (!IS_EVIL(ch))
            cast_spell(ch, vict, NULL, SPELL_CURSE, 0);
        else
            cast_spell(ch, vict, NULL, SPELL_POISON, 0);


    if ((GET_LEVEL(ch) > 12) && (number(0, 11) == 0)) {
        if (IS_EVIL(ch))
            cast_spell(ch, vict, NULL, SPELL_DISPEL_GOOD, 0);
        else if (IS_GOOD(ch))
            cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL, 0);
    }
    return TRUE;

}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

char *denybuf = "$N tells you, 'Sorry, you do not belong there.' and pushes you.\r\n";
char *denybuf2 = "$N tells $n, 'Sorry, you do not belong there.' and pushes $m.";
char *immortbuf = "$N announces your arrival and commands everyone to bow.\r\n";
char *immortbuf2 = "$N announces the arrival of $n and commands everyone to bow.";
char *racebuf[] = {
                      "$N tells you that only Humans are allowed beyond this point.\r\n",
                      "$N tells you that only Elves are allowed beyond this point.\r\n",
                      "$N tells you that only HalfElves are allowed beyond this point.\r\n",
                      "$N tells you that only Dwarves are allowed beyond this point.\r\n",
                      "$N tells you that only Halflings are allowed beyond this point.\r\n",
                      "$N tells you that only Gnomes are allowed beyond this point.\r\n",
                      "$N tells you that only Hemnov are allowed beyond this point.\r\n"
                      "$N tells you that only Llyran are allowed beyond this point.\r\n"
                      "$N tells you that only Minotaurs are allowed beyond this point.\r\n"
                      "$N tells you that only Pixies are allowed beyond this point.\r\n"
                      "\n"
                  };

SPECIAL(guild_guard)
{
    int i;
    extern int guild_info[][7];
    struct char_data *guard = (struct char_data *) me;

    if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND)) {
        return FALSE;
    }
    for (i = 0; guild_info[i][0] != -1; i++) {
        if (IS_NPC(ch))
        { char buf[100];
            if (world[ch->in_room].number!=guild_info[i][3])
                continue;
            //sprintf(buf,"i:%d, cmd:%d, where:$s,g_w:%d, g_c:%d\n",i,cmd,world[ch->in_room].number, guild_info[i][3], guild_info[i][4]);
            //send_to_char(buf,ch);
            if (cmd == guild_info[i][4])
                return TRUE;
            else
                return FALSE;
        }
        if (GET_LEVEL(ch) >= LVL_IMMORT && cmd == guild_info[i][4] &&
                world[ch->in_room].number == guild_info[i][3]) {
            act(immortbuf, FALSE, ch, 0, guard, TO_CHAR);
            act(immortbuf2, FALSE, ch, 0, guard, TO_ROOM);
            return FALSE;
        }
        /* Check CLASS */
        if (guild_info[i][0] != ANYCLASS) {
            if (!IS_SET(GET_CLASS(ch), guild_info[i][0]))
                if (world[ch->in_room].number == guild_info[i][3])
                    if (cmd == guild_info[i][4]) {
                        act(denybuf, FALSE, ch, 0, guard, TO_CHAR);
                        act(denybuf2, FALSE, ch, 0, guard, TO_ROOM);
                        return TRUE;
                    }
        }
        /*        if (guild_info[i][1] != ANYRACE) {
              if ( !(GET_RACE(ch) == guild_info[i][1]) &&
          	   (world[ch->in_room].number == guild_info[i][3]) &&
        	   (cmd == guild_info[i][4]) ) {
                act(racebuf[(guild_info[i][1])], FALSE, ch, 0, guard, TO_CHAR);
                act(denybuf2, FALSE, ch, 0, guard, TO_ROOM);
                return TRUE;
              }
            }
            if (guild_info[i][5] > GET_LEVEL(ch) &&
            	   (world[ch->in_room].number == guild_info[i][3]) &&
        	   (cmd == guild_info[i][4]) ) {
        	act(denybuf2, FALSE, ch, 0, guard, TO_CHAR);
        	return TRUE;
            }
            if (guild_info[i][6] < GET_LEVEL(ch) &&
            	   (world[ch->in_room].number == guild_info[i][3]) &&
        	   (cmd == guild_info[i][4]) ) {
        	act(denybuf2, FALSE, ch, 0, guard, TO_CHAR);
        	return TRUE;
            }*/
    }

    return FALSE;

}



SPECIAL(puff)
{

    if (cmd)
        return (0);

    switch (number(0, 60)) {
    case 0:
        do_say(ch, "My god!  It's full of stars!", 0, 0);
        return (1);
    case 1:
        do_say(ch, "How'd all those fish get up here?", 0, 0);
        return (1);
    case 2:
        do_say(ch, "I'm a very female dragon.", 0, 0);
        return (1);
    case 3:
        do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
        return (1);
    default:
        return (0);
    }
}



SPECIAL(fido)
{

    struct obj_data *i, *temp, *next_obj;

    if (cmd || !AWAKE(ch))
        return (FALSE);

    for (i = world[ch->in_room].contents; i; i = i->next_content) {
        if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) {
            act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
            for (temp = i->contains; temp; temp = next_obj) {
                next_obj = temp->next_content;
                obj_from_obj(temp);
                obj_to_room(temp, ch->in_room);
            }
            extract_obj(i);
            return (TRUE);
        }
    }
    return (FALSE);
}



SPECIAL(janitor)
{
    struct obj_data *i;

    if (cmd || !AWAKE(ch))
        return (FALSE);

    for (i = world[ch->in_room].contents; i; i = i->next_content) {
        if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
            continue;
        /*	if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
        	//    continue;*/
        act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
        obj_from_room(i);
        obj_to_char(i, ch);
        return TRUE;
    }

    return FALSE;
}

void call_guards(struct char_data *ch, struct char_data *victim)
{
    struct char_data *mob, *next_ch;
    int a,ind=-1,i=0,j=0, numm=0;
    int v1=-1, v2=-1,maxnum=-1, vnum_room=-1;
    char bufcg[200];
    a=GET_MOB_VNUM(ch);
    for (i=0; i<maxgards;i++)
        for (j=0;j<gardovi[i].vn;j++)
            if (a==gardovi[i].v[j])
            {
                maxnum=gardovi[i].maxnum;
                vnum_room=gardovi[i].vnum_room;
                ind=i;
                break;
            }

    if (ind==-1)
        return;

    i=0;
    if (maxnum!=999)
        for (mob = character_list; mob; mob = next_ch) {
            next_ch = mob->next;a=GET_MOB_VNUM(mob);
            for (j=0;j<gardovi[ind].vn;j++)
                if (a==gardovi[ind].v[j] && (HUNTING(mob)==victim || FIGHTING(mob)==victim))
                    numm++;
        }
    if (numm<maxnum)
        for (mob = character_list; mob && numm<maxnum; mob = next_ch) {
            next_ch = mob->next;a=GET_MOB_VNUM(mob);
            for (j=0;j<gardovi[ind].vn;j++)
                if (a==gardovi[ind].v[j] && !IS_SHOPKEEPER(mob)/*!MOB_FLAGGED(mob, MOB_SENTINEL)*/ && !(HUNTING(mob)==victim) && !(FIGHTING(mob)==victim))
                {
                    numm++;i++;
                    HUNTING(mob)=victim;
                    break;
                }
        }


    if (i>0)
    {
        sprintf(bufcg,gardovi[ind].s1,world[ch->in_room].name);
        do_gen_comm(ch,bufcg,0,SCMD_SHOUT);
        if (vnum_room!=-1)
        {
            for (j=0;j<gardovi[ind].vn;j++)
            {
                mob = read_mobile(gardovi[ind].v[j], VIRTUAL, world[ch->in_room].zone);
                char_to_room(mob, real_room(vnum_room));
            }
        }
    }
    else if (!number(0,10))
    {
        sprintf(bufcg,gardovi[ind].s2);
        do_gen_comm(ch,bufcg,0,SCMD_SHOUT);
    }
}

SPECIAL(mid_cityguard)
{
    struct char_data *tch, *evil;
    int max_evil,dir;

    if (cmd || !AWAKE(ch))
        return FALSE;
    if (FIGHTING(ch)) {
        if (!number(0,9))
            act("$n screams 'You're gonna get it now!'", FALSE, ch, 0, 0, TO_ROOM);
        else if (!number(0,9))
            act("$n screams 'How dare you fight a guard!'", FALSE, ch, 0, 0, TO_ROOM);
        if (GET_HIT(ch)<=GET_MAX_HIT(ch)/2)
            call_guards(ch,FIGHTING(ch));
        return FALSE;
    }
    /*    if (world[ch->in_room].zone!=ch->mob_specials.orig_zone && !FIGHTING(ch) && !HUNTING(ch))
        {
      //          do_say(ch, "I don't want to live this kind of life anymore!", 0,0);
        //        act("$n comits a suicide just in front of your eyes!!",FALSE, ch, NULL, NULL, TO_ROOM);
    //            make_corpse(ch, ch, 0);
                extract_char(ch);
                return TRUE;
        }    */

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
        return(FALSE);
    max_evil = 300;
    evil = 0;
    /*
        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    	if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_KILLER)) {
    	    act("$n screams 'HEY!!!  You're one of those wanted killers!!!'", FALSE, ch, 0, 0, TO_ROOM);
    	    hit(ch, tch, TYPE_UNDEFINED);
    	    return (TRUE);
    	}
        }

        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    	if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_THIEF)) {
    	    act("$n screams 'HEY!!!  You're one of the wanted outlaws!'", FALSE, ch, 0, 0, TO_ROOM);
    	    hit(ch, tch, TYPE_UNDEFINED);
    	    return (TRUE);
    	}
        }
    */
    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if (ch==tch)
            continue;
        if (MOB_FLAGGED(tch, MOB_AGGRESSIVE))
        {
            evil=tch;
            break;
        }
        if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
            if ((GET_ALIGNMENT(tch) < max_evil) &&
                    (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
                max_evil = GET_ALIGNMENT(tch);
                evil = tch;
            }
        }
    }

    if (evil) {
        if (MOB_FLAGGED(evil, MOB_AGGRESSIVE)) {
            act("$n screams '&YPROTECT THE ORDER!  BANZAI!  CHAAAAAAAARGE!&0'", FALSE, ch, 0, 0, TO_ROOM);
            hit(ch, evil, TYPE_UNDEFINED);
            return (TRUE);
        }

        if ((GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
            act("$n screams '&YPROTECT THE INNOCENT!  BANZAI!  CHAAAAAAAARGE!&0'", FALSE, ch, 0, 0, TO_ROOM);
            hit(ch, evil, TYPE_UNDEFINED);
            return (TRUE);
        }
    }
    return (FALSE);
}


SPECIAL(cityguard)
{
    struct char_data *tch, *evil;
    int max_evil,dir;

    if (cmd || !AWAKE(ch))
        return FALSE;
    if (FIGHTING(ch)) {
        if (!number(0,9))
            act("$n screams 'You're gonna get it now!'", FALSE, ch, 0, 0, TO_ROOM);
        else if (!number(0,9))
            act("$n screams 'How dare you fight a guard!'", FALSE, ch, 0, 0, TO_ROOM);
        if (GET_HIT(ch)<=GET_MAX_HIT(ch)*2/3)
            call_guards(ch,FIGHTING(ch));
        return FALSE;
    }

    /*   if (world[ch->in_room].zone!=ch->mob_specials.orig_zone && !FIGHTING(ch) && !HUNTING(ch))
       {
               do_say(ch, "I don't want to live this kind of life anymore!", 0,0);
               act("$n comits a suicide just in front of your eyes!!",FALSE, ch, NULL, NULL, TO_ROOM);
               make_corpse(ch, ch, 0);
               extract_char(ch);
               return TRUE;
       }    */


    max_evil = 300;
    evil = 0;
    /*
        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    	if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_KILLER)) {
    	    act("$n screams 'HEY!!!  You're one of those wanted killers!!!'", FALSE, ch, 0, 0, TO_ROOM);
    	    hit(ch, tch, TYPE_UNDEFINED);
    	    return (TRUE);
    	}
        }

        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    	if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_THIEF)) {
    	    act("$n screams 'HEY!!!  You're one of the wanted outlaws!'", FALSE, ch, 0, 0, TO_ROOM);
    	    hit(ch, tch, TYPE_UNDEFINED);
    	    return (TRUE);
    	}
        }
    */
    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
            if ((GET_ALIGNMENT(tch) < max_evil) &&
                    (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
                max_evil = GET_ALIGNMENT(tch);
                evil = tch;
            }
        }
    }

    if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
        act("&Y$n screams '&YPROTECT THE INNOCENT!  BANZAI!  CHAAAAAAAARGE!&0'", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, evil, TYPE_UNDEFINED);
        return (TRUE);
    }
    return (FALSE);
}






SPECIAL(pet_shops)
{
    char buf[MAX_STRING_LENGTH], pet_name[256];
    int pet_room;
    struct char_data *pet;

    pet_room = ch->in_room + 1;

    if (CMD_IS("list")) {
        send_to_char("Available pets are:\r\n", ch);
        for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
            sprintf(buf, "%8d - %s\r\n", 3 * GET_LEVEL(pet), GET_NAME(pet));
            send_to_char(buf, ch);
        }
        return (TRUE);
    } else if (CMD_IS("buy")) {

        if (count_pets(ch)>=NUM_PETS_ALLOWED)
        {
            send_to_char(MSG_TOO_MANY_PETS,ch);
            return TRUE;
        }
        argument = one_argument(argument, buf);
        argument = one_argument(argument, pet_name);

        if (!(pet = get_char_room(buf, pet_room))) {
            send_to_char("There is no such pet!\r\n", ch);
            return (TRUE);
        }
        if (!IS_NPC(pet))
        {
            send_to_char("Hehe, very funny. Better report that to immortal.\r\n", ch);
            return;
        }
        if (GET_GOLD(ch) < (GET_LEVEL(pet) * 3)) {
            send_to_char("You don't have enough coins!\r\n", ch);
            return (TRUE);
        }
        if (GET_LEVEL(ch) < GET_LEVEL(pet)) {
            send_to_char("You are not qualified to be that pet's master!\r\n", ch);
            return (TRUE);
        }
        GET_GOLD(ch) -= GET_LEVEL(pet) * 3;

        pet = read_mobile(GET_MOB_RNUM(pet), REAL, world[pet_room].zone);
        GET_EXP(pet) = 0;
        SET_BIT(AFF_FLAGS(pet), AFF_CHARM);
        SET_BIT(MOB_FLAGS(pet), MOB_PET);

        if (*pet_name) {
            sprintf(buf, "%s %s", pet->player.name, pet_name);
            /* DISPOSE(pet->player.name); don't free the prototype! */
            pet->player.name = str_dup(buf);

            sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
                    pet->player.description, pet_name);
            /* DISPOSE(pet->player.description); don't free the prototype! */
            pet->player.description = str_dup(buf);
        }
        char_to_room(pet, ch->in_room);
        add_follower(pet, ch);
        perform_group(ch,pet);

        /* Be certain that pets can't get/carry/use/wield/wear items */
        IS_CARRYING_W(pet) = 1000;
        IS_CARRYING_N(pet) = 100;

        send_to_char("May you enjoy your pet.\r\n", ch);
        act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

        return 1;
    }
    /* All commands except list and buy */
    return 0;
}

SPECIAL(engraver)
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data *obj;
    struct char_data *mee=(struct char_data *) me;

    if (!cmd && !FIGHTING(mee)) {
        if (number(1, 60) < 10) {
            do_say(mee,"Just type list to get my prices.",0,0);
            return 1;
        }
        return 0;
    }
    if (CMD_IS("list")) {
        do_say(mee,"Engraving an item makes that item permanently yours.",0,0);
        do_say(mee, "It costs 1000 coins to engrave an item.",0,0);
        do_say(mee,"Of course, you can unengrave an item, but that costs five times\r\n more.",0,0);
        return (TRUE);
    } else if (CMD_IS("engrave")) {

        argument = one_argument(argument, buf);

        if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
            send_to_char("You don't have that!\r\n", ch);
            return (TRUE);
        }
        if (IS_OBJ_STAT(obj, ITEM_ENGRAVED)) {
            send_to_char("That item's already engraved!\r\n", ch);
            return (TRUE);
        }
        if (IS_OBJ_STAT(obj, ITEM_AUTOENGRAVE)) {
            send_to_char("You can't engrave that!\r\n", ch);
            return (TRUE);
        }
        if (GET_GOLD(ch) < 1000) {
            send_to_char("You don't have enough coins!\r\n", ch);
            return (TRUE);
        }
        GET_GOLD(ch) -= 1000;

        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ENGRAVED);
        strcpy(obj->owner_name, GET_NAME(ch));
        act("$n engraves $p.", FALSE, mee, obj, 0, TO_ROOM);
        do_say(mee,"There you go, enjoy!!",0,0);
        return 1;
    } else if (CMD_IS("unengrave")) {

        argument = one_argument(argument, buf);

        if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
            send_to_char("You don't have that!\r\n", ch);
            return (TRUE);
        }
        if (!IS_OBJ_STAT(obj, ITEM_ENGRAVED)) {
            send_to_char("That item isn't engraved!\r\n", ch);
            return (TRUE);
        }
        if (GET_GOLD(ch) < 5000) {
            send_to_char("You don't have enough coins!\r\n", ch);
            return (TRUE);
        }
        GET_GOLD(ch) -= 5000;
        /*	if (IS_OBJ_STAT(obj, ITEM_AUTOENGRAVE)) {
        	    send_to_char("You can't unengrave that!\r\n", ch);
        	    return (TRUE);
        	}
        	if (strcmp(obj->owner_name, GET_NAME(ch)) != 0) {
        	    send_to_char("Excuse me, but that item isn't yours to unengrave!\r\n", ch);
        	    return (TRUE);
        	}
        */
        REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_ENGRAVED);
        strcpy(obj->owner_name, "");
        act("$n unengraves $p.", FALSE, mee, obj, 0, TO_ROOM);
        do_say(mee,"There you go, it's unengraved now!!",0,0);

        return 1;
    }
    return 0;
}


SPECIAL(recruiter)
{
    if (CMD_IS("register")) {
        if (GET_LEVEL(ch) < 10) {
            send_to_char("You are not ready yet for the war. Wait till you reach 10-th level.\r\n", ch);
            return 1;
        }
        if (!PRF2_FLAGGED(ch, PRF2_REG)) {
            SET_BIT(PRF2_FLAGS(ch), PRF2_REG);
            send_to_char("Recruiter tells you 'Yes, sir! Welcome to the club!'\r\n", ch);
            do_action((struct char_data *) me, GET_NAME(ch), find_command("shake"), 0);
            sprintf(buf, "[Info]:  %s just registred for WAR! Hail ho!\r\n", GET_NAME(ch));
            INFO_OUT(buf);
        } else
            send_to_char("You are allready registred!\r\n", ch);
        save_char(ch, ch->in_room);
        return 1;
    }
    if (cmd || (FIGHTING((struct char_data *) me)))
        return (0);

    switch (number(0, 18)) {
    case 0:
    case 1:
    case 2:
        act("$n says 'Join the war!'", FALSE, ch, 0, 0, TO_ROOM);
        return (1);
    case 3:
    case 4:
    case 5:
        act("$n says 'Register for the war here!'", FALSE, ch, 0, 0, TO_ROOM);
        return (1);
    case 6:
    case 7:
    case 8:
        act("$n says 'Sign up now!  Join the cause!'", FALSE, ch, 0, 0, TO_ROOM);
        return (1);
    case 9:
    case 10:
    case 11:
        act("$n says 'Fight, fight, fight!  Register now!'", FALSE, ch, 0, 0, TO_ROOM);
        return (1);
    case 12:
    case 13:
    case 14:
        act("$n says 'Be, all that you can be... now, how does that go again?'", FALSE, ch, 0, 0, TO_ROOM);
        return (1);
    default:
        do_action(ch, "", find_command("grin"), 0);
        return (0);
    }
}

SPECIAL(bounty_reg)
{
    if (CMD_IS("register")) {
        /*if (PRF2_FLAGGED(ch, PRF2_BOUNTYHUNT)) {
            send_to_char("But, you are already a registered bounty hunter!\r\n", ch);
            return 1;
    }
        if (GET_NUM_OF_CLASS(ch) != 1) {
            send_to_char("Sorry, you can not register to be a bounty hunter.\r\n", ch);
            return 1;
    }
        if (PRF2_FLAGGED(ch, PRF2_ASSASSIN)) {
            send_to_char("HA! You are more a fool than I once thought.\r\n", ch);
            return 1;
    }
        send_to_char("Welcome, Bounty Hunter.  Now go get some Assassins!\r\n", ch);
        SET_BIT(PRF2_FLAGS(ch), PRF2_BOUNTYHUNT);
        return 1;
           */}
    return 0;
}

SPECIAL(assass_reg)
{
    if (CMD_IS("register")) {
        if (PRF2_FLAGGED(ch, PRF2_ASSASSIN)) {
            send_to_char("You are already an assassin!\r\n", ch);
            return 1;
        }
        if (GET_NUM_OF_CLASS(ch) != 1) {
            send_to_char("Sorry, I don't know what you mean, multi-class slime!\r\n", ch);
            return 1;
        }
        if (!IS_THIEF(ch)) {
            send_to_char("HA! You are more a fool than I once thought.\r\n", ch);
            return 1;
        }
        send_to_char("Welcome to the Assassin's Circle!\r\n", ch);
        SET_BIT(PRF2_FLAGS(ch), PRF2_ASSASSIN);
        return 1;
    }
    return 0;
}

/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(bank)
{
    int amount;

    if (CMD_IS("balance")) {
        if (GET_BANK_GOLD(ch) > 0)
            sprintf(buf, "Your current balance is %d coins.\r\n",
                    GET_BANK_GOLD(ch));
        else
            sprintf(buf, "You currently have no money deposited.\r\n");
        send_to_char(buf, ch);
        return 1;
    } else if (CMD_IS("deposit")) {
        if (strstr(argument, "all"))
            amount = GET_GOLD(ch);
        else if ((amount = atoi(argument)) <= 0) {
            send_to_char("How much do you want to deposit?\r\n", ch);
            return 1;
        }
        if (GET_GOLD(ch) < amount) {
            send_to_char("You don't have that many coins!\r\n", ch);
            return 1;
        }
        GET_GOLD(ch) -= amount;
        GET_BANK_GOLD(ch) += amount;
        if (amount > 0) {
            sprintf(buf, "You deposit %d coins.\r\n", amount);
            send_to_char(buf, ch);
            act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
        } else
            send_to_char("You have no coins with you.\r\n", ch);
        return 1;
    } else if (CMD_IS("withdraw")) {
        if (strstr(argument, "all"))
            amount = GET_BANK_GOLD(ch);
        else if ((amount = atoi(argument)) <= 0) {
            send_to_char("How much do you want to withdraw?\r\n", ch);
            return 1;
        }
        if (GET_BANK_GOLD(ch) < amount) {
            send_to_char("You don't have that many coins deposited!\r\n", ch);
            return 1;
        }
        GET_GOLD(ch) += amount;
        GET_BANK_GOLD(ch) -= amount;
        if (amount > 0) {
            sprintf(buf, "You withdraw %d coins.\r\n", amount);
            send_to_char(buf, ch);
            act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
        } else
            send_to_char("You have no gold in the bank.\r\n", ch);
        return 1;
    } else
        return 0;
}



SPECIAL(embalmer)
{

    struct obj_data *obj;
    char loc_buf[MAX_STRING_LENGTH];
    char loc_buf2[MAX_STRING_LENGTH];
    int i = 0, len = 0, total_len = 0;
    int done = FALSE;
    struct obj_data *bag;

    *loc_buf='\0';
    *loc_buf2='\0';
    if (!cmd && !FIGHTING((struct char_data *) me)) {
        if (number(1, 60) < 10) {
            act("$n says 'Anyone have a corpse for me? I make them "
                "into bags for a \r\nreasonable amount of money! Just type embalm and you'll get a brand new bag!'", FALSE, me, 0, 0, TO_ROOM);
            return 1;
        }
        return 0;
    }


    if (CMD_IS("embalm")) {
        argument = one_argument(argument, buf);
        if (TRUE) {
            obj = get_obj_in_list_vis(ch, "corpse", ch->carrying);
            if (!obj) {
                act("$N says to you 'Sorry. You don't seem to have"
                    " any corpses.'", FALSE, ch, 0, me, TO_CHAR);
                return TRUE;
            }
            if (GET_GOLD(ch) < 50) {
                act("$N says to you, 'Sorry. You don't have"
                    " enough gold for my services.'", FALSE, ch, 0, me, TO_CHAR);
                return TRUE;
            } else {
                for (i = 0; (i < 7) || (!done); i++) {
                    len = strlen(headers[i]);
                    if (memcmp(obj->short_description, headers[i], len) == 0) {

                        if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
                            act("$N says to you, 'Sorry. That's not"
                                " a proper corpse.'", FALSE, ch, 0, me, TO_CHAR);
                            return (TRUE);
                        }
                        bag = read_object(96, VIRTUAL, 0, GET_OBJ_VAL(obj, 2)/3);
                        bag->in_room = NOWHERE;

                        total_len = strlen(obj->short_description);
                        strncpy(loc_buf, obj->short_description + len,
                                total_len - len);
                        sprintf(loc_buf2, "bag skin");
                        bag->name = str_dup(loc_buf2);
                        sprintf(loc_buf2, "A bag made of %s's skin lies"
                                " here.", obj->attack_verb);
                        bag->description = str_dup(loc_buf2);
                        sprintf(loc_buf2, "a bag made of %s's skin", obj->attack_verb);
                        bag->short_description = str_dup(loc_buf2);
                        GET_OBJ_VAL(bag, 0) = 150 + number(1, GET_LEVEL(ch));
                        GET_OBJ_RENT(bag) = 200;
                        GET_OBJ_TIMER(bag) = -1;
                        GET_GOLD(ch) -= 50;
                        done = TRUE;
                        extract_obj(obj);
                        obj_to_char(bag, ch);
                        act("$N says to you 'Done! Enjoy your new bag.'",
                            FALSE, ch, 0, me, TO_CHAR);
                        return 1;
                    }
                }
                act("$N says to you 'Sorry, I can't make a bag from"
                    "this.'", FALSE, ch, 0, me, TO_CHAR);
                return 1;
            }
        }
        return 0;
    }

    return 0;
}


SPECIAL(yoda)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    struct char_data *vict;
    struct price_info {
        short int number;
        char name[25];
        short int price;
    } prices[] = {
                     /* Spell Num (defined)	Name shown	  Price  */
                     {
                         SPELL_BLESS,         "bless            ", 5
                     },
                     /*{
                         SPELL_HEAL,          "heal             ", 12
                 },
                 {
                         SPELL_SANCTUARY,     "sanctuary        ", 15
                 },*/
                     {
                         SPELL_CURE_CRITIC,   "critic (cure)    ", 6
                     },
                     {
                         SPELL_CURE_DRUNK,    "drunk (cure)     ", 2
                     },
                     {
                         SPELL_REMOVE_POISON, "poison (cure)    ", 3
                     },
                     {
                         SPELL_CURE_BLIND,    "blindness (cure) ", 3
                     },
                     {
                         SPELL_CURE_PLAGUE,   "plague (cure)    ", 9
                     },

                     /* The next line must be last, add new spells above. */
                     {
                         -1, "\r\n", -1
                     }
                 };

    /* NOTE:  In interpreter.c, you must define a command called 'heal' for this
       spec_proc to work.  Just define it as do_not_here, and the mob will take
       care of the rest.  (If you don't know what this means, look in interpreter.c
       for a clue.)
    */


    if (CMD_IS("heal")) {
        argument = one_argument(argument, buf);

        if (GET_POS(ch) == POS_FIGHTING)
            return TRUE;

        if (*buf) {
            for (i = 0; prices[i].number > SPELL_RESERVED_DBC; i++) {
                if (isname(buf, prices[i].name))
                    if (GET_GOLD(ch) <prices[i].price*GET_LEVEL(ch)) {
                        act("$n tells you, 'My friend, you don't have enough gold for that spell.'",
                            FALSE, (struct char_data *) me, 0, ch, TO_VICT);
                        return TRUE;
                    } else {
                        act("$N gives $n some money.",
                            FALSE, (struct char_data *) me, 0, ch, TO_NOTVICT);
                        sprintf(buf, "You give &c%s&0 %d coins.\r\n",
                                GET_NAME((struct char_data *) me),  prices[i].price*GET_LEVEL(ch));
                        send_to_char(buf, ch);
                        GET_GOLD(ch) -=  prices[i].price*GET_LEVEL(ch);
                        GET_GOLD((struct char_data *) me) +=   prices[i].price*GET_LEVEL(ch);

                        cast_spell((struct char_data *) me, ch, NULL, prices[i].number, 0);
                        return TRUE;

                    }
            }
            act("$n tells you, 'I do not know of that spell!"
                "  Type 'heal' for a list.'", FALSE, (struct char_data *) me,
                0, ch, TO_VICT);

            return TRUE;
        } else {
            act("$n tells you, 'Here is a listing of the prices for my services.'",
                FALSE, (struct char_data *) me, 0, ch, TO_VICT);
            for (i = 0; prices[i].number > SPELL_RESERVED_DBC; i++) {
                sprintf(buf, "&Y%s&c%d&0\r\n", prices[i].name,  prices[i].price*GET_LEVEL(ch));
                send_to_char(buf, ch);
            }
            return TRUE;
        }
    }
    if (!cmd && !FIGHTING((struct char_data *) me)) {
        int a;
        a = number(1, 20);
        switch (a) {
        case 2:
            act("$n says 'Mmmmmmrrrr..... Ooooo.... Tula-ha-nog!'", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 8:
            act("$n reaches down and scratches himself. 'Mmmrrm..'", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 12:
            act("$n gets a piece of meat and eats it.", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 17:
            act("$n says 'I'll catch that dragon one day...'", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 19:
            act("$n jumps and says 'A-hah!'", FALSE, me, 0, 0, TO_ROOM);
            break;
        default:
            break;
        }
    }
    if (cmd)
        return FALSE;

    /* pseudo-randomly choose someone in the room */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (!number(0, 2))
            break;

    /* change the level at the end of the next line to control free spells */
    if (vict == NULL || IS_NPC(vict) || !CAN_SEE(ch, vict))
        return FALSE;

    switch (number(1, 20)) {
    case 1:
        cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT, 0);
        break;
    case 2:
        cast_spell(ch, vict, NULL, SPELL_BLESS, 0);
        break;
    case 3:
        cast_spell(ch, vict, NULL, SPELL_CURE_SERIOUS, 0);
        break;
    case 4:
        cast_spell(ch, vict, NULL, SPELL_CURE_SERIOUS, 0);
        break;
    case 5:
        cast_spell(ch, vict, NULL, SPELL_BLESS, 0);
        break;
    case 6:
        cast_spell(ch, vict, NULL, SPELL_CURE_CRITIC, 0);
        break;
    case 7:
        cast_spell(ch, vict, NULL, SPELL_CREATE_FOOD, 0);
        break;
    case 8:
        cast_spell(ch, vict, NULL, SPELL_REMOVE_POISON, 0);
        break;
    case 9:
        cast_spell(ch, vict, NULL, SPELL_CURE_CRITIC, 0);
        break;
    case 10:
        cast_spell(ch, vict, NULL, SPELL_HEAL, 0);
        break;
    case 11:
        if (IS_CASTER(vict))
            cast_spell(ch, vict, NULL, SPELL_RESTORE_MANA, 0);
        break;
    case 12:
        if (!number(0, 2)) {
            act("$n utters the words, 'energizer'.", TRUE, ch, 0, vict, TO_ROOM);
            act("You feel invigorated!", FALSE, ch, 0, vict, TO_VICT);
            //GET_MANA(vict) = GET_MAX_MANA(vict);
            GET_HIT(vict) = GET_MAX_HIT(vict);
            //GET_MOVE(vict) = GET_MAX_MOVE(vict);
        }
        break;
    }
    return TRUE;
}

SPECIAL(healer)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    struct char_data *vict;
    struct price_info {
        short int number;
        char name[25];
        short int price;
    } prices[] = {
                     /* Spell Num (defined)	Name shown	  Price  */
                     {
                         SPELL_BLESS, "bless          ", 1000
                     },
                     {
                         SPELL_REMOVE_POISON, "remove poison  ", 5000
                     },
                     {
                         SPELL_CURE_BLIND, "cure blindness ", 5000
                     },
                     {
                         SPELL_CURE_CRITIC, "cure critic    ", 3000
                     },
                     {
                         SPELL_SANCTUARY, "sanctuary      ", 10000
                     },
                     {
                         SPELL_HEAL, "heal           ", 10000
                     },

                     /* The next line must be last, add new spells above. */
                     {
                         -1, "\r\n", -1
                     }
                 };

    if (GET_POS(ch) == POS_FIGHTING)
        return TRUE;

    if (!cmd && !FIGHTING((struct char_data *) me)) {
        int a;
        a = number(1, 20);
        switch (a) {
        case 2:
            act("$n says 'Let there be light! ... Or was it...?'", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 8:
            act("$n reaches down and scratches himself. He looks very happy now", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 12:
            act("$n says 'How are you doing?'", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 17:
            act("$n says 'I wonder what do I have for lunch today...'", FALSE, me, 0, 0, TO_ROOM);
            break;
        default:
            break;

        }
    }
    if (cmd)
        return FALSE;

    /* pseudo-randomly choose someone in the room */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (!number(0, 2))
            break;

    /* change the level at the end of the next line to control free spells */
    if (vict == NULL || IS_NPC(vict) || (GET_LEVEL(vict) > 10))
        return FALSE;

    switch (number(1, 15)) {
    case 1:
        cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT, 0);
        break;
    case 2:
        cast_spell(ch, vict, NULL, SPELL_BLESS, 0);
        break;
    case 3:
        cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT, 0);
        break;
    case 4:
        cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT, 0);
        break;
    case 5:
        cast_spell(ch, vict, NULL, SPELL_BLESS, 0);
        break;
    case 6:
        cast_spell(ch, vict, NULL, SPELL_CURE_CRITIC, 0);
        break;
    case 7:
        cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT, 0);
        break;
    case 8:
        cast_spell(ch, vict, NULL, SPELL_CREATE_FOOD, 0);
        break;
    case 9:
        cast_spell(ch, vict, NULL, SPELL_CURE_SERIOUS, 0);
        break;
    case 10:
        if (!number(0, 5)) {
            act("$n utters the words, 'energizer'.", TRUE, ch, 0, vict, TO_ROOM);
            act("You feel invigorated!", FALSE, ch, 0, vict, TO_VICT);
            //GET_MANA(vict) = GET_MAX_MANA(vict);
            GET_HIT(vict) = GET_MAX_HIT(vict);
            //GET_MOVE(vict) = GET_MAX_MOVE(vict);
        }
        break;
    }
    return TRUE;
}


SPECIAL(drunker)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    struct char_data *vict;


    if (GET_POS(ch) == POS_FIGHTING)
        return FALSE;

    if (!cmd && !FIGHTING((struct char_data *) me)) {
        int a;
        a = number(1, 15);
        switch (a) {
        case 1:
            act("$n says 'Gosh. What a night...'", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 2:
        case 3:
            do_action(ch, "", find_command("tackle"), 0);
            break;
        case 5:
        case 6:
        case 7:
            do_action(ch, "", find_command("hic"), 0);
            break;
        case 8:
        case 9:
            do_action(ch, "", find_command("laugh"), 0);
            break;
        case 10:
            act("$n sings 'Nancy, Nancy, Nancy Whiskey...Ho!'", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 11:
            act("$n says 'Want some ?'.", FALSE, me, 0, 0, TO_ROOM);
            break;
        case 12:
            act("$n tries to make a turn but hits a wall.", FALSE, me, 0, 0, TO_ROOM);
            break;
        default:
            do_action(ch, "", find_command("wave"), 0);

        }
    }
    if (cmd)
        return FALSE;

    /* pseudo-randomly choose someone in the room */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (number(1, 10) > 5)
            break;

    /* change the level at the end of the next line to control free spells */
    if (vict == NULL || IS_NPC(vict) || GET_LEVEL(ch) < 5)
        return FALSE;

    a = number(1, 25);
    if ((a > GET_CON(vict)) && (GET_COND(vict, DRUNK) < 15)) {
        do_action(ch, GET_NAME(vict), find_command("pat"), 0);
        /*  act("He asks you to join him for a drink or two. You just couldn't refuse.",FALSE,(struct char_data *)me,vict,0,TO_VICT);
          act("$n and $N joined forces. Poor bottle...",FALSE,(struct char_data *)me,vict,0,TO_ROOM);*/
        send_to_char("Shane asks you to join him for a drink. You just couldn't refuse..\r\n", vict);
        GET_COND(vict, DRUNK) = 21;
        send_to_char("You feel drunk. Hic!\r\n", vict);
    }
    return TRUE;
}

SPECIAL(newbie_guide)
{


    /* static char tour_path[] =
     "WAAA2E3J1230D22G032K011110I22M033212L030014R530A00001HBC32222Z.";*/
    /* The above is the path that the guide takes. The numbers are the   */
    /* Directions in which he moves                                      */

    /* If you find your guide is getting lost, it's probably because you */
    /* don't have the newbie area included in stock bpl11.  To fix it,   */
    /* use this pat instead:                                            */

    static char tour_path[] =
        "WAAAS2E3J1230D22G032K011110I22M033212L0330P232Q011000HBZ.";


    static char *path;
    static int indexx;
    static bool movee = FALSE;

    if (!movee) {
        if (time_info.hours == 1) {	/* Tour starts at 1 am */
            movee = TRUE;
            path = tour_path;
            indexx = 0;
        } else if (time_info.hours == 13) {	/* And at 12 pm */
            movee = TRUE;
            path = tour_path;
            indexx = 0;
        } else if (time_info.hours == 7) {	/* And at 12 pm */
            movee = TRUE;
            path = tour_path;
            indexx = 0;
        } else if (time_info.hours == 19) {	/* And at 12 pm */
            movee = TRUE;
            path = tour_path;
            indexx = 0;
        }

    }
    if (cmd || !movee || (GET_POS(ch) < POS_STANDING) ||
            (GET_POS(ch) == POS_FIGHTING))
        return FALSE;

    switch (path[indexx]) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
        perform_move(ch, path[indexx] - '0', 1);
        break;
    case 'W':
        GET_POS(ch) = POS_STANDING;
        act("$n stands up and announces 'The tour is going to start soon!'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'S':
        act("$n says, 'Our healer, Yoda, is up from here.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'I':
        act("$n says, 'This is the weapon shop. Type 'list' to see what is sold here.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'Z':
        act("$n sits and rests for his next journey.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_RESTING;
        break;
    case 'M':
        act("$n says, 'This is the enterence to the warrior guild.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'L':
        act("$n says, 'This is the enterence to the thief guild.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'K':
        act("$n says, 'This is the enterence to the mage guild.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'J':
        act("$n says, 'This is the enterence to the cleric guild.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'Q':
        act("$n says, 'This is the enterence to the druid guild.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'P':
        act("$n says, 'This is the enterence to the monk guild.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'H':
        act("$n says, 'Right now, you may find it usefull to type 'wear all''", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'A':
        act("$n says, 'Newbies!! Type 'FOLLOW GUIDE' for a guided tour'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'B':
        act("$n says, 'This is the end of the tour. Please, type 'follow self''", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'C':
        act("$n says, 'Now have fun out there, and be careful!'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'D':
        act("$n says, 'This is our dear friend the baker, to buy bread from him\r\n type 'buy bread''", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'E':
        act("$n says, 'This is the Fountain, to drink from it \r\ntype 'drink fountain''", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'F':
        act("$n says, 'This is our dear friend Wally, he will sell you water\r\n type 'LIST' to see a list of what he has.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 'G':
        act("$n says, 'This is the Armorer, he makes armor, type LIST to see what\r\n he has to sell'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case '.':
        movee = FALSE;
        break;
    case 'R':
        act("$n says, 'This is the RECEPTION, in this MUD, you must RENT.'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n says, 'To see how much your rent will cost, type 'OFFER'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n says, 'To rent, type RENT.'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    }
    indexx++;
    return FALSE;
}

/* This special procedure makes a mob into a 'rent-a-cleric', who sells spells
   by the sea shore... uuh, maybe not.  Anyway, the mob will also cast certain
   spells on low-level characters in the room for free.
   By:  Wyatt Bode	Date:  April, 1996
*/

#define NUM_ARCHERS      2	/* # of rooms archers can shoot from */
#define NUM_TARGETS      3	/* # of rooms an archer can shoot at */
#define HIT_CHANCE       40	/* accuracy 30% chance to hit */
#define ARCHER_NUM_DICE  2	/* archer damage dice */
#define ARCHER_SIZE_DICE 5	/* archer does 2d5 each hit */

SPECIAL(archer)
{
    struct char_data *targ;
    int i, j, k;
    int damage;

    sh_int to_from_rooms[NUM_ARCHERS][NUM_TARGETS + 1] = {
                /* archer room     target room #1     #2       #3 */
                {3040, 3052, 6092, -1},	/* archer room #1 */
                {3041, 3053, 3503, -1}	/* room #2 */
            };

    char *mssgs[] = {
                        "You feel a sharp pain in your side as an arrow finds its mark!",
                        "You hear a dull thud as an arrow pierces $n!",
                        "An arrow whistles by your ear, barely missing you!",
                        "An arrow narrowly misses $n!"
                    };

    if (cmd)
        return FALSE;

    if (GET_POS(ch) != POS_STANDING)
        return FALSE;

    for (i = 0; i < NUM_ARCHERS; i++) {
        if (real_room(to_from_rooms[i][0]) == ch->in_room) {

            for (j = 1; j <= NUM_TARGETS; j++) {
                if ((k = real_room(to_from_rooms[i][j])) >= 0) {

                    for (targ = world[k].people; targ; targ = targ->next_in_room) {

                        if (!IS_NPC(targ) && (GET_LEVEL(targ) < LVL_IMMORT)
                           ) {
                            if (number(1, 100) <= HIT_CHANCE) {
                                act(mssgs[0], FALSE, ch, 0, targ, TO_VICT);
                                act(mssgs[1], FALSE, targ, 0, 0, TO_ROOM);
                                damage = number(5, 50);
                                GET_HIT(targ) -= damage + (number(1, 5));
                                /* these above numbers can be changed for
                                   different damage levels. */
                                update_pos(targ);
                                check_kill(targ, "an arrow");

                                return TRUE;
                            } else {
                                act(mssgs[2], FALSE, ch, 0, targ, TO_VICT);
                                act(mssgs[3], FALSE, targ,0,0, TO_ROOM);
                                return TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
    return FALSE;
}

#define ASSASSIN_PRICE(assassin) (GET_LEVEL(assassin) * GET_LEVEL(assassin) * 1500)

SPECIAL(assassin)
{
    char victim_name[256], buf[MAX_STRING_LENGTH], assassin_name[256];
    int assassin_room;
    struct char_data *assassin, *vict;

    assassin_room = ch->in_room;

    if (CMD_IS("list")) {
        send_to_char("Available assassins are:\r\n", ch);
        for (assassin = world[assassin_room].people; assassin; assassin = assassin->
                next_in_room) {
            if (IS_NPC(assassin)) {
                sprintf(buf, "%8d - %s\r\n", ASSASSIN_PRICE(assassin), GET_NAME(assassin));
                send_to_char(buf, ch);
            }
        }
        return (TRUE);
    } else if (CMD_IS("hire")) {

        argument = one_argument(argument, buf);
        argument = one_argument(argument, victim_name);

        if (!(assassin = get_char_room(buf, assassin_room))) {
            send_to_char("There is nobody called that!\r\n", ch);
            return (TRUE);
        }
        if (GET_GOLD(ch) < ASSASSIN_PRICE(assassin)) {
            send_to_char("You don't have enough gold!\r\n", ch);
            return (TRUE);
        }
        if (!*victim_name) {
            send_to_char("You need to specify a victim!\r\n", ch);
            return 1;
        }
        if ((vict = get_player_vis(assassin, victim_name, 0))) {
            if (GET_LEVEL(vict)<CLAN_ENTRY_LEVEL)
            {
                do_say(assassin, "No, I can't do it.",0,0);
                return 1;
            }
            GET_GOLD(ch) -= ASSASSIN_PRICE(assassin);
            GET_GOLD(assassin)+=ASSASSIN_PRICE(assassin);
            //	    assassin = read_mobile(GET_MOB_RNUM(assassin), REAL, 0);
            //	    char_to_room(assassin, ch->in_room);
            //	    SET_BIT(MOB_FLAGS(assassin), MOB_MEMORY);
            SET_BIT(MOB_FLAGS(assassin), MOB_ASSASSIN);
            //	    remember(assassin, vict);
            HUNTING(assassin) = vict;
            HIRED_BY(assassin)=ch;
            act("$n counts the money.",FALSE, assassin, 0,0,TO_ROOM);
            act("$n hires $N for a job.", FALSE, ch, 0, assassin, TO_ROOM);
            do_say(assassin,"You will be contacted if I succeed or not.",0,0);
            return 1;
        } else {
            send_to_char("The assassin can't find the victim!\r\n", ch);
            return 1;
        }
    }
    return 0;
}


SPECIAL(pop_dispenser)
{
    struct obj_data *obj = me, *drink;
    int give_coke = 92;		/* Vnum of the can of coke */
    if (CMD_IS("list")) {
        send_to_char("To buy a drink, type 'buy drink'.\r\n", ch);
        return (TRUE);
    } else if (CMD_IS("buy")) {
        if (GET_GOLD(ch) < 25) {
            send_to_char("You don't have enough gold!\r\n", ch);
            return (TRUE);
        } else {
            drink = read_object(give_coke, VIRTUAL, 0, 0);
            obj_to_char(drink, ch);
            send_to_char("You insert your money into the machine.\r\n", ch);
            GET_GOLD(ch) -= 25;	/* coke costs 25 gold */
            act("$n gets a pop can from $p.", FALSE, ch, obj, 0, TO_ROOM);
            send_to_char("You get a pop can from the machine.\r\n", ch);
        }
        return 1;
    }
    return 0;
}

SPECIAL(table)
{
    char red[80];
    char *tmstr;
    int len;
    FILE *tf;
    char *suf;
    int weekday, day;
    extern struct time_info_data time_info;
    int rm;
    char it[50];
    rm = world[ch->in_room].number;
    sprintf(it, "%s%d", TABLE_DIR, rm);
    if (CMD_IS("look")) {
        argument = one_argument(argument, buf);
        if (isname(buf, "table")) {

            if (tf = fopen(it, "r")) {
                while (fgets(red, 80, tf))
                    send_to_char(red, ch);
                fclose(tf);
                return (1);
            } else
                send_to_char("There is something strange about this table. Please report the BUG.\r\n", ch);
        }
        return (0);
    }
    if (CMD_IS("carve")) {

        if (tf = fopen(it, "r")) {
            if ((fgetc(tf) != '-')) {
                send_to_char("You are not the first one to come here!\r\n", ch);
                fclose(tf);
                return (1);
            }
            fclose(tf);
        }
        tf = fopen(it, "w");
        fprintf(tf, "Some letters are carved into the table:\r\n");

        fputs("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n", tf);

        sprintf(buf,
                " This area was first discovered by %s %s\r\n", (AFF_FLAGGED(ch, AFF_GROUP) ? ch->player_specials->group_name: GET_NAME(ch)), (AFF_FLAGGED(ch, AFF_GROUP) ? "":GET_TITLE(ch)));
        buf[80] = '\0';
        fputs(buf, tf);

        act("$n carves some letters in the table.", TRUE, ch, 0, 0, TO_ROOM);
        if (*argument) {
            argument[80] = '\0';
            //	    sprintf(buf, "\r\n");
            //	    fputs(CAP(buf), tf);
            fputs(argument, tf);
        }
        day = time_info.day + 1;/* day in [1..35] */
        if (day == 1)
            suf = "st";
        else if (day == 2)
            suf = "nd";
        else if (day == 3)
            suf = "rd";
        else if (day < 20)
            suf = "th";
        else if ((day % 10) == 1)
            suf = "st";
        else if ((day % 10) == 2)
            suf = "nd";
        else if ((day % 10) == 3)
            suf = "rd";
        else
            suf = "th";
        sprintf(buf, "on %d%s Day of the %s, Year %d.\r\n",
                day, suf, month_name[(int) time_info.month], time_info.year);
        fputs("\r\r\n\n Written ", tf);
        fputs(buf, tf);
        fputs("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n", tf);
        fclose(tf);
        send_to_char("You carve some letters in the table.\r\n", ch);
        return (1);
    }
    return (0);
}

SPECIAL(tbox)
{
    int i;
    struct obj_data *obj;
    if (CMD_IS("pull")) {
        argument = one_argument(argument, buf);
        if (!*buf) {
            send_to_char("Pull what?\r\n", ch);
            return (1);
        }
        if (!strstr("card", buf)) {
            send_to_char("There are only teleport cards in the box.\r\n", ch);
            return (1);
        }
        for (i = 0; i < maxsoba; i++)
            if (real_room(sobe[i].rnum) == ch->in_room)
                break;

        if (i >= maxsoba) {
            send_to_char("It seems that vandals stole all of the cards.\r\n", ch);
            return (1);
        }
        obj = read_object(sobe[i].vnum, VIRTUAL, 0, 0);
        obj_to_char(obj, ch);
        send_to_char("You pull out the shiny teleport card.\r\n", ch);
        return (1);
    }
    return 0;
}

SPECIAL(troom)
{

    int i=-1, mode;
    char buf[80], buf1[80];
    struct char_data *tmp_char;
    struct obj_data *obj1;
    if (CMD_IS("insert")) {
        argument = one_argument(argument, buf);
        if (!*buf) {
            send_to_char("What card?\r\n", ch);
            return (1);
        }
        /*if  (!strcmp(buf,"rome"))*/
        if (isname(buf, "midgaard"))
            i=0;
        else
        {
            mode = generic_find(buf, FIND_OBJ_INV, ch, &tmp_char, &obj1);
            if (!obj1) {
                send_to_char("You don't have that card.\r\n", ch);
                return (1);
            }
            if (GET_OBJ_TYPE(obj1) != ITEM_TCARD) {
                send_to_char("That isn't teleport card!\r\n", ch);
                return 1;
            }
            buf[15] = '\0';
            for (i = 0; i < maxsoba; i++) {
                if (isname(buf, sobe[i].ime))
                    break;
            }
            if (i >= maxsoba) {
                send_to_char("Usage: insert <name>  eg. insert midgaard.\r\n", ch);
                return (1);
            }
        }
        if (i==-1)
        {log("Serious screw up in troom, spec_procs.c");
            return 0;
        }
        else
        {
            if (i!=0)
                send_to_char("You carefully insert your card and sit in the cabin.\r\n", ch);
            else
                send_to_char("You sit in the cabin and press a red button.\r\n", ch);
            send_to_char("You feel strong vibrations as flash of light blinds you for a moment.\r\n\r\n", ch);
            act("$n enters the cabin.\r\nA brief flash of light comes from the cabin.",FALSE, ch, NULL, NULL, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(sobe[i].tornum));
            look_at_room(ch, 0);
            act("$n arrives, apparently from nowhere.", FALSE, ch, NULL, NULL, TO_ROOM);
            return (1);
        }

    }
    return (0);
}

SPECIAL(grave_undertaker)
{
    if (cmd)
        return FALSE;
    if (!AWAKE(ch))
        return (FALSE);


    do_get(ch, "corpse", 0);
    return FALSE;
}


SPECIAL(grave_demilich)
{
    struct char_data *victim;

    if (cmd || !AWAKE(ch) || !FIGHTING(ch))
        return FALSE;

    for (victim = world[ch->in_room].people; victim; victim = victim->next_in_room)
        if ((!IS_NPC(victim)) && (FIGHTING(victim) == ch))
            /* person is a player who is fighting demlich */
            if (GET_LEVEL(victim) >= number(0, 75))
                /* demilich wants high level victims */
                if (GET_LEVEL(victim) >= number(0, 75))
                    /* high level more likely to resist */
                {
                    act("$n tried to capture your soul.", FALSE, ch, 0, victim, TO_VICT);
                    act("$n gives $N a icy cold stare.", FALSE, ch, 0, victim, TO_NOTVICT);
                    return FALSE;
                } else {
                    act("$n sucks you into one of his gems.", FALSE, ch, 0, victim, TO_VICT);
                    act("$N disappears into one of $n's eyes.", FALSE, ch, 0, victim, TO_NOTVICT);
                    act("You trap $N in one of your gems.", FALSE, ch, 0, victim, TO_CHAR);
                    send_to_char("Your soul is trapped within the demilich.\r\n", victim);
                    send_to_char("Slowly you feel your life-force drain away...\r\n", victim);
                    GET_HIT(victim) = 1;
                    //GET_MOVE(victim) = 0;
                    //GET_MANA(victim) = 0;
                    char_from_room(victim);
                    
                    char_to_room(victim, real_room(victim->player.hometown>1?victim->player.hometown:clan_loadroom[GET_CLAN(ch)]));
                    send_to_char(
                        "\r\nAs you flow you fill devastating pain all over your body.\r\n", victim);
                    /*look_at_room(victim,0);*/
                    return TRUE;
                }

    return FALSE;
}


SPECIAL(grave_ghoul)
{
    if (!FIGHTING(ch) || cmd)
        return (FALSE);

    if (!number(0, 5)) {
        act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
        act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
        cast_spell(ch, FIGHTING(ch), NULL, SPELL_CURSE, 0);
        cast_spell(ch, FIGHTING(ch), NULL, SPELL_POISON, 0);
        do_action(ch, GET_NAME(FIGHTING(ch)), find_command("cackle"), 0);
        return (TRUE);
    }
    return (FALSE);
}

SPECIAL(undead)
{
    struct char_data *vict;
    int min_level, num, spell;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
        return FALSE;


    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch && !number(0, 4))
            break;

    if (vict == NULL)
        return FALSE;

    for (;;) {
        num = number(1, 8);
        switch (num) {
        case 1:
            min_level = 9;
            spell = SPELL_BLINDNESS;
            break;
        case 2:
            min_level = 3;
            spell = SPELL_WEAKEN;
            break;
        case 3:
            min_level = 0;
            spell = SPELL_CURSE;
            break;
        case 4:
            min_level = 18;
            spell = SPELL_HARM;
            break;
        case 5:
            min_level = 6;
            spell = SPELL_CHILL_TOUCH;
            break;
        case 6:
            min_level = 11;
            spell = SPELL_POISON;
            break;
        case 7:
            min_level = 0;
            spell = SPELL_CURSE;
            break;
        case 8:
            min_level = 0;
            spell = SPELL_LOCAL_TELEPORT;
            break;
        }
        if (GET_LEVEL(ch) >= min_level)
            break;
    }

    cast_spell(ch, vict, NULL, spell, 0);
    return TRUE;
}

SPECIAL(grave_priest)
{
    struct char_data *vict;
    char buf[MAX_STRING_LENGTH];
    long num_in_room = 0, vict_num;


    if (number(0, 3) || !AWAKE((struct char_data *) me) || cmd)
        return (FALSE);
    if (FIGHTING((struct char_data *) me)) {
        do_say((struct char_data *) me, "You are commiting blasphemy!", 0, 0);
        return (FALSE);
    }
    /*
      for (vict = world[((struct char_data *) me)->in_room].people; vict; vict = vict->next_in_room)
        if (!IS_NPC(vict) && CAN_SEE((struct char_data *) me, vict)) num_in_room++;

      if (!num_in_room) return (FALSE);

      vict_num = number(1,num_in_room);

      for (vict = world[((struct char_data *) me)->in_room].people; vict; vict = vict->next_in_room) {
        if (!IS_NPC(vict) && CAN_SEE((struct char_data *) me, vict)) vict_num--;
        if (!vict_num && !IS_NPC(vict)) break;
      }
      if (!vict) return (FALSE);*/
    for (vict = world[((struct char_data *) me)->in_room].people; vict; vict = vict->next_in_room)
        if (!number(0, 2))
            break;

    /* change the level at the end of the next line to control free spells */
    if (vict == NULL || IS_NPC(vict))
        return FALSE;



    if (IS_GOOD((struct char_data *) me) && IS_GOOD(vict)) {
        sprintf(buf, "You, %s, are blessed.", GET_NAME(vict));
        do_say((struct char_data *) me, buf, 0, 0);
        cast_spell((struct char_data *) me, vict, NULL, SPELL_BLESS, 0);
    } else if (IS_EVIL((struct char_data *) me) && IS_EVIL(vict)) {
        act("$n grins and says, 'You, $N, are truly wretched.'", FALSE, (struct char_data *) me,
            0, vict, TO_NOTVICT);
        act("$n grins and says, 'You, $N, are truly wretched.'", FALSE, (struct char_data *) me,
            0, vict, TO_VICT);
    } else if (IS_NEUTRAL((struct char_data *) me) && IS_NEUTRAL(vict)) {
        sprintf(buf, "You, %s, follow the True Path.", GET_NAME(vict));
        do_say((struct char_data *) me, buf, 0, 0);
    } else if (IS_NEUTRAL(vict) || IS_NEUTRAL((struct char_data *) me)) {
        sprintf(buf, "%s, it is not too late for you to mend your ways.",
                GET_NAME(vict));
        do_say((struct char_data *) me, buf, 0, 0);
    } else {
        sprintf(buf, "Blasphemy!  %s, your presence will stain this temple no "
                "more!", GET_NAME(vict));
        do_say((struct char_data *) me, buf, 0, 0);
    }
    return (FALSE);
}

SPECIAL(temple_cleric)
{
    struct char_data *vict;
    struct char_data *hitme = NULL;
    static int this_hour;
    float temp1 = 1;
    float temp2 = 1;

    if (cmd)
        return FALSE;

    if (time_info.hours != 0) {

        this_hour = time_info.hours;

        for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
            if (IS_AFFECTED(vict, AFF_POISON))
                hitme = vict;
        }
        if (hitme != NULL) {
            cast_spell(ch, hitme, NULL, SPELL_REMOVE_POISON, 0);
            return TRUE;
        }
        for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
            if (IS_AFFECTED(vict, AFF_BLIND))
                hitme = vict;
        }
        if (hitme != NULL) {
            cast_spell(ch, hitme, NULL, SPELL_CURE_BLIND, 0);
            return TRUE;
        }
        for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
            temp1 = GET_HIT(vict) / GET_MAX_HIT(vict);
            if (temp1 < temp2) {
                temp2 = temp1;
                hitme = vict;
            }
        }
        if (hitme != NULL) {
            cast_spell(ch, hitme, NULL, SPELL_HEAL, 0);
            return TRUE;
        }
    }
    return 0;
}



#define num_gpaths 2
static int go_path[num_gpaths] = {3521, 324};
static int gangdest = 0;
static int gang_in_fight=0;

SPECIAL(shop_keeper);


SPECIAL(mob_gang_leader)
{

    ACMD(do_gen_door);
    int dir,i=0,j,g;
    struct char_data *tch;


    if (cmd || !AWAKE(ch))
        return FALSE;
    if (FIGHTING(ch)) {
        gang_in_fight=1;
        if (!number(0,9))
            act("$n screams 'AAAARGH! Fight, fight, fight!!!'", FALSE, ch, 0, 0, TO_ROOM);
        else if (!number(0,9))
            act("$n screams 'Come get some puny weasle!'", FALSE, ch, 0, 0, TO_ROOM);
        return FALSE;
    }

    if (gang_in_fight==1)
    {
        do_get(ch,"all all.corpse",0,0);
        do_order(ch,"fol get all all.corpse",0,0);
        do_order(ch,"fol get all",0,0);
        gang_in_fight=0;
    }
    if (!ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
            if (ch!=tch && CAN_SEE(ch, tch) &&  !is_same_group(ch,tch) && mob_index[GET_MOB_RNUM(tch)].func != shop_keeper)
            {
                i++;
                if (!IS_AFFECTED(ch, AFF_GROUP))
                    j=10;
                else
                    j=number(0,23);
                if (j<10 && !MOB_FLAGGED(tch,MOB_SPEC) && (GET_LEVEL(ch)>GET_LEVEL(tch)) && GET_HIT(ch)>(GET_MAX_HIT(ch)/2) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
                {
                    if (!IS_NPC(tch) && GET_LEVEL(tch)<11)
                        return TRUE;
                    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
                        return TRUE;
                    act("$n says, 'Lets kick some more ass!'",FALSE, ch,NULL,tch, TO_ROOM);
                    hit(ch, tch, TYPE_UNDEFINED);
                    return (TRUE);
                }
                else if (j<17 && !MOB_FLAGGED(tch,MOB_SPEC) && !MOB_FLAGGED(tch, MOB_AGGRESSIVE) && GET_LEVEL(tch)>5 && count_group(ch)<=6 && IS_NPC(tch))
                {
                    do_fame(ch,tch->player.name,0,0);
                }
                else if (j==20)
                {
                    act("$n says, 'I think $N's mama was almost as ugly as $E is.'",FALSE, ch,NULL,tch, TO_ROOM);
                    act("$n pukes.",FALSE,ch,NULL,NULL,TO_ROOM);
                    return (FALSE);
                }
                else if (j==17)
                {
                    act("$n says, 'I bet $N still wets $S bed.'",FALSE, ch,NULL,tch, TO_ROOM);
                    act("$n falls down laughing.",FALSE,ch,NULL,NULL,TO_ROOM);
                    return (FALSE);
                }
                else if (j==18)
                {
                    act("$n says, 'I could beat $N with one hand!'",FALSE, ch,NULL,tch, TO_ROOM);
                    act("$n pukes.",FALSE,ch,NULL,NULL,TO_ROOM);
                    return (FALSE);
                }
                else if (j==19)
                {
                    act("$n grins evilly.",FALSE,ch,NULL,NULL,TO_ROOM);
                    break;
                }
            }


    if (number(0,i+1)==0)
    {
        if (!number(0,5))
            if (count_group(ch)>2)
            {
                char buf[100];
                struct char_data *k=ch;
                struct follow_type *f;
                do_say(ch, "Switch equipment!",0,0);
                for (f = ch->followers; f; f = f->next)
                    if (IS_AFFECTED(f->follower, AFF_GROUP))
                    {
                        if (IS_CARRYING_N(k)>0)
                        {
                            sprintf(buf,"all %s",f->follower->player.name);
                            do_give(k,buf,0,0);
                            wear_all_suitable(f->follower);
                            /*					do_wear(f->follower,"all",0,0);
                            					if (!GET_EQ(f->follower,WEAR_WIELD))
                            						do_wield(f->follower,"all",0,0);
                            */
                        }
                        k=f->follower;
                    }
                sprintf(buf,"all %s",ch->player.name);
                do_give(k,buf,0,0);
            }

        dir = find_first_step(ch->in_room, real_room(go_path[gangdest]));

        switch (dir) {
        case BFS_ERROR:
            send_to_char("Hmm.. something seems to be wrong.\r\n", ch);
            gangdest = 1;
            do_ungroup(ch,"",0,0);
            act("$n leaves in a hurry.",FALSE, ch, 0,0,TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(go_path[0]));
            break;
        case BFS_ALREADY_THERE:
            send_to_char("You're already in the same room!!\r\n", ch);
            do_action(ch, "", find_command("grin"), 0);
            gangdest++;
            if (gangdest == num_gpaths)
                gangdest = 0;
            break;
        case BFS_NO_PATH:
            sprintf(buf, "You can't sense a trail from here.\r\n");
            send_to_char(buf, ch);
            do_ungroup(ch,"",0,0);
            act("$n leaves in a hurry.",FALSE, ch, 0,0,TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(go_path[0]));
            gangdest=1;
            /*
            		gangdest++;
            	if (gangdest == num_gpaths)
            	    gangdest = 0;
            		do_action(ch, "", find_command("puke"), 0);
            		return FALSE;*/
            break;
        default:
            sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
            send_to_char(buf, ch);
            perform_move(ch, dir, 1);
            break;
        }
    }

    return TRUE;
}


char *cnames[2] =
    {
        "Blue Dragon",
        "Nightsnake"
    };

SPECIAL(mob_hogar)
{

    ACMD(do_gen_door);
    int dir,i=0,j,g;
    struct char_data *tch;


    if (cmd || !AWAKE(ch))
        return FALSE;
    if (FIGHTING(ch)) {
        gang_in_fight=1;
        if (!number(0,8))
            act("$n screams 'GROWL! GROOOOWL!'", FALSE, ch, 0, 0, TO_ROOM);
        return FALSE;
    }

    if (gang_in_fight==1)
    {
        do_get(ch,"all all.corpse",0,0);
        do_order(ch,"fol get all all.corpse",0,0);
        do_order(ch,"fol get all",0,0);
        gang_in_fight=0;
    }
    if (!ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
            if (ch!=tch && CAN_SEE(ch, tch) &&  !is_same_group(ch,tch) && mob_index[GET_MOB_RNUM(tch)].func != shop_keeper)
            {
                i++;
                if (!IS_AFFECTED(ch, AFF_GROUP))
                    j=10;
                else
                    j=number(0,23);
                if (j<10 && !MOB_FLAGGED(tch,MOB_SPEC) && (GET_LEVEL(ch)>GET_LEVEL(tch)) && GET_HIT(ch)>(GET_MAX_HIT(ch)/2) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
                {
                    if (!IS_NPC(tch) && GET_LEVEL(tch)<25)
                        return TRUE;
                    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
                        return TRUE;
                    act("$n says, 'I need a good fight!'",FALSE, ch,NULL,tch, TO_ROOM);
                    hit(ch, tch, TYPE_UNDEFINED);
                    return (TRUE);
                }
                else if (j<17 && !MOB_FLAGGED(tch,MOB_SPEC) && !MOB_FLAGGED(tch, MOB_AGGRESSIVE) && GET_LEVEL(tch)>5 && count_group(ch)<=6 && IS_NPC(tch))
                {
                    do_fame(ch,tch->player.name,0,0);
                }
                else if (j==20)
                {
                    act("$n says, 'I killed ancient dragon in the Dragon Tower.'",FALSE, ch,NULL,tch, TO_ROOM);
                    act("$n pukes.",FALSE,ch,NULL,NULL,TO_ROOM);
                    return (FALSE);
                }
                else if (j==17)
                {
                    act("$n says, 'Some time ago I met this ugly goblin. Almost as ugly as $N is!'",FALSE, ch,NULL,tch, TO_ROOM);
                    act("$n falls down laughing.",FALSE,ch,NULL,NULL,TO_ROOM);
                    return (FALSE);
                }
                else if (j==18)
                {
                    act("$n says, 'What are you looking at?'",FALSE, ch,NULL,tch, TO_ROOM);
                    act("$n peers at you.",FALSE,ch,NULL,tch,TO_VICT);
                    return (FALSE);
                }
                else if (j==19)
                {
                    act("$n grins evilly.",FALSE,ch,NULL,NULL,TO_ROOM);
                    break;
                }
            }


    if (number(0,i+1)==0)
    {
        if (!number(0,5))
            if (count_group(ch)>2)
            {
                char buf[100];
                struct char_data *k=ch;
                struct follow_type *f;
                do_say(ch, "Here!",0,0);
                for (f = ch->followers; f; f = f->next)
                    if (IS_AFFECTED(f->follower, AFF_GROUP))
                    {
                        if (IS_CARRYING_N(k)>0)
                        {
                            sprintf(buf,"all %s",f->follower->player.name);
                            do_give(k,buf,0,0);
                            wear_all_suitable(f->follower);
                            /*					do_wear(f->follower,"all",0,0);
                            					if (!GET_EQ(f->follower,WEAR_WIELD))
                            						do_wield(f->follower,"all",0,0);
                            */
                        }
                        k=f->follower;
                    }
                sprintf(buf,"all %s",ch->player.name);
                do_give(k,buf,0,0);
            }

        dir = find_first_step(ch->in_room, real_room(go_path[gangdest]));

        switch (dir) {
        case BFS_ERROR:
            send_to_char("Hmm.. something seems to be wrong.\r\n", ch);
            gangdest = 1;
            do_ungroup(ch,"",0,0);
            act("$n leaves in a hurry.",FALSE, ch, 0,0,TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(go_path[0]));
            break;
        case BFS_ALREADY_THERE:
            send_to_char("You're already in the same room!!\r\n", ch);
            do_action(ch, "", find_command("grin"), 0);
            gangdest++;
            if (gangdest == num_gpaths)
                gangdest = 0;
            break;
        case BFS_NO_PATH:
            sprintf(buf, "You can't sense a trail from here.\r\n");
            send_to_char(buf, ch);
            do_ungroup(ch,"",0,0);
            act("$n leaves in a hurry.",FALSE, ch, 0,0,TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(go_path[0]));
            gangdest=1;
            /*
            		gangdest++;
            	if (gangdest == num_gpaths)
            	    gangdest = 0;
            		do_action(ch, "", find_command("puke"), 0);
            		return FALSE;*/
            break;
        default:
            sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
            send_to_char(buf, ch);
            perform_move(ch, dir, 1);
            break;
        }
    }

    return TRUE;
}






extern int             num_of_clans;
extern struct clan_rec clan[MAX_CLANS];


SPECIAL(wheel_of_destiny)
{
    static int rolling = 0, cnt = 0;
    char buf[500];

    if (!(CMD_IS("roll") || CMD_IS("stop")))
        return FALSE;

    if (CMD_IS("roll")) {
        if (rolling)
            send_to_char("You give a wheel a little push.\r\n", ch);
        else {
            send_to_char("You strain your muscles and manage to slowly roll the heavy wheel.\r\nYou feel like you have given life to the ancient signs carved deep into it.\r\n", ch);
            rolling = 1;
            SET_BIT(AFF2_FLAGS(ch), AFF2_ROLLING);
            cnt = 0;
            ffield = number(0, 6);
        }
    }
    if (CMD_IS("stop")) {
        if (!rolling)
            send_to_char("You have to roll the wheel first.\r\n", ch);
        else {
            int cl=CLAN_BLUE;
            char bufcl[100];
            if (clan[CLAN_BLUE].power>clan[CLAN_RED].power) cl=CLAN_RED;
            rolling = 0;
            REMOVE_BIT(AFF2_FLAGS(ch), AFF2_ROLLING);
            send_to_char("You stop the wheel.\r\nDim light surrounds you as some unseen force lifts you in the air.\r\r\n\n", ch);
            sprintf(buf, "A voice booms out, 'Welcome &y%s&0, to the mighty clan of the &C%s&0!'\r\n", GET_NAME(ch), clan[cl].name);
            send_to_char(buf, ch);
            do_clan_enroll(ch, cl);
            send_to_char(".\r\n.\r\n.\r\nSuddenly you find yourself in a very friendly place...\r\n",ch);
            char_from_room(ch);
            char_to_room(ch, real_room(ch->player.hometown>1?ch->player.hometown:clan_loadroom[GET_CLAN(ch)]));
            save_char(ch,ch->in_room);
            send_to_char("You may now request to have your own house. See help on 'houses'.\r\n", ch);
            sprintf(bufcl, "\r\n&B{INFO}: &Y%s joins the clan of &C&f%s&0.\r\n", GET_NAME(ch), clan[GET_CLAN(ch)].name);
            INFO_OUT(bufcl);

        }
    }
    return TRUE;
}

char *denymsg[] = {
                      "$n tells you, 'I shall not let you in there til you are level 7.'",
                      "$n tells you, 'You can not go in there anymore.' and blocks your way.",
                      "$N tells you that only HalfElves are allowed beyond this point.\r\n",
                      "$N tells you that only Dwarves are allowed beyond this point.\r\n",
                      "$N tells you that only Halflings are allowed beyond this point.\r\n",
                      "$N tells you that only Gnomes are allowed beyond this point.\r\n",
                      "$N tells  you that only Hemnov are allowed beyond this point.\r\n"
                      "$N tells you that only Llyran are allowed beyond this point.\r\n"
                      "$N tells you that only Minotaurs are allowed beyond this point.\r\n"
                      "$N tells you that only Pixies are allowed beyond this point.\r\n"
                      "\n"
                  };

SPECIAL(wheel_guard)
{
    struct char_data *guard = (struct char_data *) me;

    if (!IS_MOVE(cmd) && !CMD_IS("expell"))
        return FALSE;

    if (IS_NPC(ch))
        if (cmd == SCMD_NORTH)
            return TRUE;
        else
            return FALSE;

    if (CMD_IS("expell"))
    {   char bufcl[100];

        if (GET_CLAN(ch)!=CLAN_BLUE && GET_CLAN(ch)!=CLAN_RED)
        {
            send_to_char("Abrams tells you, 'You are not a memeber of any respectable clan.'\r\n", ch);
            return TRUE;
        }
        clan[GET_CLAN(ch)].members--;
        clan[GET_CLAN(ch)].power -= GET_LEVEL(ch);
        GET_CLAN(ch)=CLAN_OUTLAW;
        clan[GET_CLAN(ch)].members++;
        clan[GET_CLAN(ch)].power += GET_LEVEL(ch);
        GET_CLAN_RANK(ch) = 0;
        IS_EXPELLED(ch)=1;
        send_to_char("Abrams gets a small book and writes something down.\r\n\r\nAbrams tells you, 'You are now an Outlaw. Good luck, you'll sure need it.'\r\n",ch);
        sprintf(bufcl, "\r\n{INFO}: %s becomes an Outlaw.\r\n", GET_NAME(ch));
        INFO_OUT(bufcl);
        save_char(ch, ch->in_room);
        return TRUE;
    }
    if (GET_LEVEL(ch) >= LVL_IMMORT && cmd == SCMD_NORTH &&
            world[ch->in_room].number == 8246) {
        return FALSE;
    }
    if (CLAN_ENTRY_LEVEL > GET_LEVEL(ch) && (world[ch->in_room].number == 8246) && (cmd == SCMD_NORTH)) {
        act(denymsg[0], FALSE, guard, 0, ch, TO_VICT);
        return TRUE;
    }

    if (IS_EXPELLED(ch) && (world[ch->in_room].number == 8246) && (cmd == SCMD_NORTH)) {
        act(denymsg[1], FALSE, guard, 0, ch, TO_VICT);
        return TRUE;
    }

    if ((GET_CLAN(ch)==CLAN_BLUE || GET_CLAN(ch)==CLAN_RED) && (world[ch->in_room].number == 8246) && (cmd == SCMD_NORTH)) {
        act(denymsg[1], FALSE, guard, 0, ch, TO_VICT);
        return TRUE;
    }

    if (GET_QUESTPOINTS(ch)<=0  && (world[ch->in_room].number == 8246) && (cmd == SCMD_NORTH))
    {
        do_say(guard, "I can't let you in so unexperienced. Go seek Willie, and try doing a quest.\r\n",0,0);
        return TRUE;
    }
    return FALSE;
}

#define MAP_COST 150
extern FILE           *player_fl;

SPECIAL(map_room)
{
    struct obj_data *k;
    struct char_file_u vbuf;
    int pn;
    int value = 0, i=0, n=0, nn;
    FILE *f;
    char mybuf[10000];
    char bline[200];
    char pname[100];

    if (!CMD_IS("list") && !CMD_IS("buy"))
        return 0;

    if (CMD_IS("list"))
    {
        f=fopen("/u/mud/mud/lom/lib/maps/index","r");
        if (!f) return 0;
        sprintf(mybuf,"Available maps:\r\n---------------\r\n\r\n");
        while (!feof(f))
        {
            fgets(bline, 200, f);
            i++;
            sprintf(mybuf,"%s%3d. %s",mybuf,i, bline);
        }
        fclose(f);
        send_to_char(mybuf, ch);
        send_to_char("\r\n",ch);
        return 1;
    }
    if (CMD_IS("buy"))
    {
        if (GET_GOLD(ch)<MAP_COST)
        {
            send_to_char("You do not have enough gold to buy a map.\r\n",ch);
            return 1;
        }
        skip_spaces(&argument);
        if (!(*argument)) {
            send_to_char("By what map?\r\n", ch);
            return 1;
        }
        if (is_number(argument))
            n=atoi(argument);
        else
        {
            send_to_char("You must use numerical argument. For example 'buy 2'.\r\n",ch);
            return(1);
        }
        f=fopen("/u/mud/mud/lom/lib/maps/index","r");
        if (!f) return 0;
        nn=n;
        while (!feof(f) && n)
        {
            fgets(bline, 200, f);
            n--;
            if (!n)
                break;
        }
        if (n)
        {
            send_to_char("No map under that number.\r\n",ch);
            return 1;
        }
        fclose(f);
        sprintf(bline,"/u/mud/mud/lom/lib/maps/%d",nn);
        f=fopen(bline,"r");
        k=read_object(79, VIRTUAL, 0, 0);
        fgets(bline, 200, f);
        k->name=str_dup(bline);

        k->ex_description->keyword=str_dup(bline);
        fgets(bline, 200, f);
        bline[strlen(bline)-1]='\0';
        k->short_description=str_dup(bline);
        sprintf(mybuf,"MAPS: %s", bline);
        log(mybuf);
        *mybuf='\0';
        while (!feof(f))
        {
            fgets(bline, 200, f);
            sprintf(mybuf,"%s%s",mybuf,bline);
        }
        fclose(f);
        k->ex_description->description=str_dup(mybuf);
        obj_to_char(k, ch);
        GET_GOLD(ch)-=MAP_COST;
        act("You buy $p.", FALSE, ch, k, NULL, TO_CHAR);
        act("$n buys $p.", FALSE, ch, k, NULL, TO_ROOM);

        return 1;
    }

    return 0;
}


/*
 *  File: sund_procs.c                          Part of Exile MUD
 *  
 *  Special procedures for the mobs and objects of Sundhaven.
 *
 *  Exile MUD is based on CircleMUD, Copyright (C) 1993, 1994.
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.       
 *
 */


/*  Mercy's Procs for the Town of Sundhaven  */


SPECIAL(silktrader)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;

    if (world[ch->in_room].sector_type == SECT_CITY)
        switch (number(0, 30)) {
        case 0:
            act("$n eyes a passing woman.", FALSE, ch, 0, 0,TO_ROOM);
            do_say(ch, "Come, m'lady, and have a look at this precious silk!", 0, 0);
            return(1);
        case 1:
            act("$n says to you, 'Wouldn't you look lovely in this!'", FALSE, ch, 0, 0,TO_ROOM);
            act("$n shows you a gown of indigo silk.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 2:
            act("$n holds a pair of silk gloves up for you to inspect.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 3:
            act("$n cries out, 'Have at this fine silk from exotic corners of the world you will likely never see!", FALSE, ch, 0, 0,TO_ROOM);
            act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 4:
            do_say(ch, "Step forward, my pretty locals!", 0, 0);
            return(1);
        case 5:
            act("$n shades his eyes with his hand.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 6:
            do_say(ch, "Have you ever seen an ogre in a silken gown?", 0, 0);
            do_say(ch, "I didn't *think* so!", 0, 0);
            act("$n throws his head back and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 7:
            act("$n hands you a glass of wine.", FALSE, ch, 0, 0,TO_ROOM);
            do_say(ch, "Come, have a seat and view my wares.", 0, 0);
            return(1);
        case 8:
            act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
            act("$n shakes his head sadly.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 9:
            act("$n fiddles with some maps.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 10:
            do_say(ch, "Here here! Beggars and nobles alike come forward and make your bids!", 0, 0);
            return(1);
        case 11:
            do_say(ch, "I am in this bourgeois hamlet for a limited time only!", 0, 0);
            act("$n swirls some wine in a glass.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        }

    if (world[ch->in_room].sector_type != SECT_CITY)
        switch (number(0, 20)) {
        case 0:
            do_say(ch, "Ah! Fellow travellers! Come have a look at the finest silk this side of the infamous Ched Razimtheth!", 0, 0);
            return(1);
        case 1:
            act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
            do_say(ch, "You are feebly attired for the danger that lies ahead.", 0, 0);
            do_say(ch, "Silk is the way to go.", 0, 0);
            act("$n smiles warmly.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 2:
            do_say(ch, "Worthy adventurers, hear my call!", 0, 0);
            return(1);
        case 3:
            act("$n adjusts his cloak.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 4:
            act("$n says to you, 'Certain doom awaits you, therefore shall you die in silk.'", FALSE, ch, 0, 0,TO_ROOM);
            act("$n bows respectfully.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 5:
            do_say(ch, "Can you direct me to the nearest tavern?", 0, 0);
            return(1);
        case 6:
            do_say(ch, "Heard the latest ogre joke?", 0, 0);
            act("$n snickers to himself.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 7:
            do_say(ch, "What ho, traveller! Rest your legs here for a spell and peruse the latest in fashion!", 0, 0);
            return(1);
        case 8:
            do_say(ch, "Beware ye, traveller, lest ye come to live in Exile!", 0, 0);
            act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);
        case 9:
            act("$n touches your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
            do_say(ch, "A word of advice. Beware of any ale labled 'mushroom' or 'pumpkin'.", 0, 0);
            act("$n shivers uncomfortably.", FALSE, ch, 0, 0,TO_ROOM);
            return(1);

        }
    return(0);
}


SPECIAL(athos)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;
    switch (number(0, 20)) {
    case 0:
        act("$n gazes into his wine gloomily.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 1:
        act("$n grimaces.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 2:
        act("$n asks you, 'Have you seen the lady, pale and fair, with a heart of stone?'", FALSE, ch, 0, 0,TO_ROOM);
        do_say(ch, "That monster will be the death of us all.", 0, 0);
        return(1);
    case 3:
        do_say(ch, "God save the King!", 0, 0);
        return(1);
    case 4:
        do_say(ch, "All for one and .. one for...", 0, 0);
        act("$n drowns himself in a swig of wine.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 5:
        act("$n looks up with a philosophical air.", FALSE, ch, 0, 0,TO_ROOM);
        do_say(ch, "Women - God's eternal punishment on man.", 0, 0);
        return(1);
    case 6:
        act("$n downs his glass and leans heavily on the oaken table.", FALSE, ch, 0, 0,TO_ROOM);
        do_say(ch, "You know, we would best band together and wrestle the monstrous woman from her lair and home!", 0, 0);
        return(1);
    default: return(FALSE);
        break; }
    return(0);
}



SPECIAL(hangman)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;

    switch (number(0, 15)) {
    case 0:
        act("$n whirls his noose like a lasso and it lands neatly around your neck.", FALSE, ch, 0, 0,TO_ROOM);
        do_say(ch, "You're next, you ugly rogue!", 0, 0);
        do_say(ch, "Just kidding.", 0, 0);
        act("$n pats you on your head.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 1:
        do_say(ch, "I was conceived in Exile and have been integrated into society!", 0, 0);
        do_say(ch, "Muahaha!", 0, 0);
        return(1);
    case 2:
        do_say(ch, "Anyone have a butterknife I can borrow?", 0, 0);
        return(1);
    case 3:
        act("$n suddenly pulls a lever.", FALSE, ch, 0, 0,TO_ROOM);
        act("With the flash of light on metal a giant guillotine comes crashing down!", FALSE, ch, 0, 0,TO_ROOM);
        act("A head drops to the ground from the platform.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n looks up and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n shouts, 'Next!'", FALSE, ch, 0, 0, TO_ROOM);
        return(1);
    case 4:
        act("$n whistles a local tune.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    default:
        return(FALSE);
        break;
    }
    return(0);
}



SPECIAL(butcher)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;

    switch (number(0, 40)) {
    case 0:
        do_say(ch, "I need a Union.", 0, 0);
        act("$n glares angrily.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n rummages about for an axe.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 1:
        act("$n gnaws on a toothpick.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 2:
        act("$n runs a finger along the edge of a giant meat cleaver.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 3:
        do_say(ch, "Pork for sale!", 0, 0);
        return(1);
    case 4:
        act("$n whispers to you, 'I've got some great damage eq in the back room. Wanna see?'", FALSE, ch, 0, 0,TO_ROOM);
        act("$n throws back his head and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 5:
        act("$n yawns.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 6:
        act("$n throws an arm around the headless body of an ogre and asks to have his picture taken.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 7:
        act("$n listlessly grabs a cleaver and hurls it into the wall behind your head.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 8:
        act("$n juggles some fingers.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 9:
        act("$n eyes your limbs.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n chuckles.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 10:
        do_say(ch, "Hi, Alice.", 0, 0);
        return(1);
    case 11:
        do_say(ch, "Everyone looks like food to me these days.", 0, 0);
        act("$n sighs loudly.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 12:
        act("$n throws up his head and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n shouts, 'Bring out your dead!'", FALSE, ch, 0, 0, TO_ROOM);
        return(1);
    case 13:
        do_say(ch, "The worms crawl in, the worms crawl out..", 0, 0);
        return(1);
    case 14:
        act("$n sings 'Brave, brave Sir Patton...'", FALSE, ch, 0, 0,TO_ROOM);
        act("$n whistles a tune.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 15:
        do_say(ch, "Get Lurch to bring me over a case and I'll sport you a year's supply of grilled ogre.", 0, 0);
        return(1);
    default: return(FALSE);
        break; }
    return(0);
}



SPECIAL(stu)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;

    switch (number(0, 60)) {
    case 0:
        do_say(ch, "I'm so damn cool, I'm too cool to hang out with myself!", 0, 0);
        break;
    case 1:
        do_say(ch, "I'm really the NICEST guy you ever MEET!", 0, 0);
        break;
    case 2:
        do_say(ch, "Follow me for exp, gold and lessons in ADVANCED C!", 0, 0);
        break;
    case 3:
        do_say(ch, "Mind if I upload 200 megs of pregnant XXX gifs with no descriptions to your bbs?", 0, 0);
        break;
    case 4:
        do_say(ch, "Sex? No way! I'd rather jog 20 miles!", 0, 0);
        break;
    case 5:
        do_say(ch, "I'll take you OUT!!   ...tomorrow", 0, 0);
        break;
    case 6:
        do_say(ch, "I invented Mud you know...", 0, 0);
        break;
    case 7:
        do_say(ch, "Can I have a cup of water?", 0, 0);
        break;
    case 8:
        do_say(ch, "I'll be jogging down ventnor ave in 10 minutes if you want some!", 0, 0);
        break;
    case 9:
        do_say(ch, "Just let me pull a few strings and I'll get ya a site, they love me! - doesnt everyone?", 0, 0);
        break;
    case 10:
        do_say(ch, "Pssst! Someone tell Mercy to sport me some levels.", 0, 0);
        act("$n nudges you with his elbow.", FALSE, ch, 0, 0,TO_ROOM);
        break;
    case 11:
        do_say(ch, "Edgar! Buddy! Let's group and hack some ogres to tiny quivering bits!", 0, 0);
        break;
    case 12:
        act("$n tells you, 'Skylar has bad taste in women!'", FALSE, ch, 0, 0,TO_ROOM);
        act("$n screams in terror!", FALSE, ch, 0, 0,TO_ROOM);
        do_flee(ch, 0, 0, 99);
        break;
    case 13:
        if (number(0, 32767)<10){
            act("$n whispers to you, 'Dude! If you fucking say 'argle bargle' to the glowing fido he'll raise you a level!'", FALSE, ch, 0, 0,TO_ROOM);
            act("$n flexes.", FALSE, ch, 0, 0,TO_ROOM);}
        return(1);
    default:
        return(FALSE);
        break;
        return(1);
    }
    return 0;
}


SPECIAL(sund_earl)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;
    switch (number(0, 20)) {
    case 0:
        do_say(ch, "Lovely weather today.", 0, 0);
        return(1);
    case 1:
        act("$n practices a lunge with an imaginary foe.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 2:
        do_say(ch, "Hot performance at the gallows tonight.", 0, 0);
        act("$n winks suggestively.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 3:
        do_say(ch, "Must remember to up the taxes at my convenience.", 0, 0);
        return(1);
    case 4:
        do_say(ch, "Sundhaven is impermeable to the enemy!", 0, 0);
        act("$n growls menacingly.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 5:
        do_say(ch, "Decadence is the credence of the abominable.", 0, 0);
        return(1);
    case 6:
        do_say(ch, "I look at you and get a wonderful sense of impending doom.", 0, 0);
        act("$n chortles merrily.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 7:
        act("$n touches his goatee ponderously.", FALSE, ch, 0, 0,TO_ROOM);
        return(1);
    case 8:
        do_say(ch, "It's Mexican Madness night at Maynards!", 0, 0);
        act("$n bounces around.", FALSE, ch, 0, 0, TO_ROOM);
        return(1);
    default: return(FALSE);
        break;
        return(0);
    }
}


SPECIAL(blinder)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;

    if (GET_POS(ch) != POS_FIGHTING)
        return FALSE;

    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
            (number(0, 100)+GET_LEVEL(ch) >= 50)) {
        act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
        act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, FIGHTING(ch), TO_VICT);
        act("$n's frayed cloak blows as he points at $N.", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
        act("$n's frayed cloak blows as he aims a bony finger at you.", 1, ch, 0, FIGHTING(ch), TO_VICT);
        act("A flash of pale fire explodes in $N's face!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
        act("A flash of pale fire explodes in your face!", 1, ch, 0, FIGHTING(ch), TO_VICT);
        call_magic(ch, FIGHTING(ch), 0, SPELL_BLINDNESS, GET_LEVEL(ch), CAST_SPELL,NULL);
        return TRUE;
    }
    return FALSE;
}


SPECIAL(idiot)
{
    if (cmd || (GET_POS(ch) < POS_SLEEPING) || (GET_POS(ch) == POS_FIGHTING))
        return 0;

    switch (number(0, 40)) {
    case 0:
        do_say(ch, "even if idiot = god", 0, 0);
        do_say(ch, "and Stu = idiot", 0, 0);
        do_say(ch, "Stu could still not = god.", 0, 0);
        act("$n smiles.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 1:
        act("$n balances a newbie sword on his head.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 2:
        act("$n doesn't think you could stand up to him in a duel.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 3:
        do_say(ch, "Rome really was built in a day.", 0, 0);
        act("$n snickers.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 4:
        act("$n flips over and walks around on his hands.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 5:
        act("$n cartwheels around the room.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 6:
        do_say(ch, "How many ogres does it take to screw in a light bulb?", 0, 0);
        act("$n stops and whaps himself upside the head.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 7:
        do_say(ch, "Uh huh. Uh huh huh.", 0, 0);
        return TRUE;
    case 8:
        act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n whistles quietly.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 9:
        act("$n taps out a tune on your forehead.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 10:
        act("$n has a battle of wits with himself and comes out unharmed.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 11:
        do_say(ch, "All this and I am just a number.", 0, 0);
        act("$n cries on your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
        return TRUE;
    case 12:
        do_say(ch, "A certain hunchback I know dresses very similar to you, very similar...", 0, 0);
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}


SPECIAL(marbles)
{
    struct obj_data *tobj = me;

    if (tobj->in_room == NOWHERE)
        return 0;

    if (CMD_IS("north") || CMD_IS("south") || CMD_IS("east") || CMD_IS("west") ||
            CMD_IS("up") || CMD_IS("down")) {
        if (AFF_FLAGGED(ch, AFF_FLYING))
            return 0;
        if (!DEX_CHECK(ch)) {
            act("You slip on $p and fall.", FALSE, ch, tobj, 0, TO_CHAR);
            act("$n slips on $p and falls.", FALSE, ch, tobj, 0, TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            return 1;
        }
        else {
            act("You slip on $p, but manage to retain your balance.", FALSE, ch, tobj, 0, TO_CHAR);
            act("$n slips on $p, but manages to retain $s balance.", FALSE, ch, tobj, 0, TO_ROOM);
        }
    }
    return 0;
}

#define MOB_VNUM_PATROLMAN         2106
#define GROUP_VNUM_TROLLS          2102
#define GROUP_VNUM_OGRES           2101

SPECIAL(troll_member)
{
    struct char_data *vch, *victim = NULL;
    int count = 0;
    char *message;

    if (!IS_NPC(ch) || cmd)
        return (FALSE);

    if (!ch || !AWAKE(ch) || ch->in_room == NOWHERE
            ||  IS_AFFECTED(ch,AFF_CHARM) || FIGHTING(ch))
        return FALSE;

    /* find an ogre to beat up */
    for (vch = world[ch->in_room].people;  vch != NULL;  vch = vch->next_in_room)
    {
        if (ch == vch || !CAN_SEE(ch, vch))
            continue;

        if (IS_NPC(vch) && GET_MOB_VNUM(vch) == MOB_VNUM_PATROLMAN)
            return FALSE;

        if ((IS_NPC(vch) && GET_MOB_VNUM(vch) == GROUP_VNUM_OGRES) || (!IS_NPC(ch) && GET_RACE(vch)==RACE_OGRE))
        {
            victim = vch;
            break;
        }

    }

    if (victim == NULL)
        return FALSE;

    /* say something, then raise hell */
    switch (number(0,6))
    {
    default:  message = NULL; 	break;
    case 0:	message = "$n yells 'I've been looking for you, punk!'";
        break;
    case 1: message = "With a scream of rage, $n attacks $N.";
        break;
    case 2: message =
            "$n says 'What's slimy Ogre trash like you doing around here?'";
        break;
    case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
        break;
    case 4: message = "$n says 'There's no cops to save you this time!'";
        break;
    case 5: message = "$n says 'Time to join your brother, spud.'";
        break;
    case 6: message = "$n says 'Let's rock.'";
        break;
    }

    if (message != NULL)
        act(message,FALSE, ch,NULL,victim,TO_ROOM);
    hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

SPECIAL(ogre_member)
{
    struct char_data *vch, *victim = NULL;
    int count = 0;
    char *message;

    if (!IS_NPC(ch) || cmd)
        return (FALSE);

    if (!ch || !AWAKE(ch) || ch->in_room == NOWHERE
            ||  IS_AFFECTED(ch,AFF_CHARM) || FIGHTING(ch))
        return FALSE;

    /* find an ogre to beat up */
    for (vch = world[ch->in_room].people;  vch != NULL;  vch = vch->next_in_room)
    {
        if (ch == vch || !CAN_SEE(ch, vch))
            continue;

        if (GET_MOB_VNUM(vch) == MOB_VNUM_PATROLMAN)
            return FALSE;

        if ((IS_NPC(vch) && GET_MOB_VNUM(vch) == GROUP_VNUM_TROLLS) || (!IS_NPC(vch) && GET_RACE(vch)==RACE_TROLL))
        {
            victim = vch;
            break;
        }

    }

    if (victim == NULL)
        return FALSE;

    /* say something, then raise hell */
    switch (number(0,6))
    {
    default: message = NULL;	break;
    case 0: message = "$n yells 'I've been looking for you, punk!'";
        break;
    case 1: message = "With a scream of rage, $n attacks $N.'";
        break;
    case 2: message =
            "$n says 'What's Troll filth like you doing around here?'";
        break;
    case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
        break;
    case 4: message = "$n says 'There's no cops to save you this time!'";
        break;
    case 5: message = "$n says 'Time to join your brother, spud.'";
        break;
    case 6: message = "$n says 'Let's rock.'";
        break;
    }

    if (message != NULL)
        act(message,FALSE, ch,NULL,victim,TO_ROOM);
    hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
}

SPECIAL(patrolman)
{
    struct char_data *vch,*victim = NULL;
    struct obj_data *obj;
    char *message;
    int count = 0;
    if (!IS_NPC(ch) || cmd)
        return (FALSE);

    if (!AWAKE(ch) ||  ch->in_room == NOWHERE
            ||  IS_AFFECTED(ch,AFF_CHARM) || FIGHTING(ch))
        return FALSE;

    /* look for a fight in the room */
    for (vch = world[ch->in_room].people; vch != NULL; vch = vch->next_in_room)
    {
        if (ch == vch || !CAN_SEE(ch, vch))
            continue;

        if (FIGHTING(vch))  /* break it up! */
        {
            if (number(0,count) == 0)
                victim = ((GET_LEVEL(vch) > GET_LEVEL(FIGHTING(vch))) ? vch : vch->char_specials.fighting);
            count++;
        }
    }

    if (victim == NULL || (IS_NPC(victim) && GET_MOB_VNUM(victim) == GET_MOB_VNUM(ch)))
        return FALSE;

    act("$n blows on his whistle. ***WHEEEEEEEEEEEET***",FALSE, ch,NULL, NULL,TO_ROOM);


    switch (number(0,6))
    {
    default:	message = NULL;		break;
    case 0:	message = "$n yells 'All roit! All roit! break it up!'";
        break;
    case 1: message =
            "$n says 'Society's to blame, but what's a bloke to do?'";
        break;
    case 2: message =
            "$n mumbles 'bloody kids will be the death of us all.'";
        break;
    case 3: message = "$n shouts 'Stop that! Stop that!' and attacks.";
        break;
    case 4: message = "$n pulls out his billy and goes to work.";
        break;
    case 5: message =
            "$n sighs in resignation and proceeds to break up the fight.";
        break;
    case 6: message = "$n says 'Settle down, you hooligans!'";
        break;
    }

    if (message != NULL)
        act(message,FALSE ,ch,NULL,NULL,TO_ROOM);

    hit(ch,victim,TYPE_UNDEFINED);

    return TRUE;
}


SPECIAL(mud_school)
{

    if (!IS_MOVE(cmd)) {
        return FALSE;
    }

    if (cmd!=SCMD_UP && GET_LEVEL(ch)>15)
    {
        send_to_char("Naah.. Go back to the real world. You do not need to be here anymore.\r\n", ch);
        return TRUE;
    }
    return FALSE;
}






/* Add new rooms above the { -1, -1, -1 } line.  That must always be last. */
struct current_info current[] = {
                                    /*  Room  Direction Percent
                                      -------------------------  */
                                    { 3001, SOUTH,    34 },
                                    { -1, -1, -1 }
                                };


SPECIAL(current_proc)
{

    extern struct current_info current[];
    int i, found, perm_num, new_room;

    found = FALSE;
    perm_num = 0;

    if(!cmd)
        return FALSE;


    for(i=0; current[i].room_vnum != -1;i++)
        if(ch->in_room == real_room(current[i].room_vnum)) {
            perm_num = i;
            found = TRUE;
            break;
        }

    if(found)
        if(number(0,100) < current[perm_num].percent) {
            sprintf(buf, "The strong current carries you %s!\r\n",
                    dirs[current[perm_num].direction]);
            send_to_char(buf, ch);
            sprintf(buf, "$n is taken %s by the rough current!",
                    dirs[current[perm_num].direction]);
            act(buf, FALSE, ch, 0, 0, TO_NOTVICT);

            /* You can use your favorite way to record errors here. */

            if(!EXIT(ch, current[perm_num].direction)) {
                send_to_char("Error in this room.  Please report this! ERROR 1\r\n", ch);
                return FALSE;
            }

            if(EXIT(ch, current[perm_num].direction)->to_room == NOWHERE) {
                send_to_char("Error in this room.  Please report this! ERROR 2\r\n", ch);
                return FALSE;
            }

            /* Here we want to use char_from_room / char_to_room instead of a
            do_simple_move
               because the current should take them no matter if they have a
            boat, no
               movement points left, etc. */
            new_room = EXIT(ch, current[perm_num].direction)->to_room;
            char_from_room(ch);
            char_to_room(ch, new_room);
            act("$n is swept into the room by the rough current!", FALSE, ch,0, 0, TO_NOTVICT);
        }
    return FALSE;
}



SPECIAL(wand_of_wonder)
{
    struct descriptor_data *d;
    struct obj_data *wand = (struct obj_data *) me, *sobj;
    int i, k;
    struct char_data *tch = NULL, *next_tch, *mob;
    struct obj_data *tobj = NULL;

    void die(struct char_data * ch);
    void death_cry(struct char_data * ch);

    /* this part emulates the do_use bit for a reason */

    if(!CMD_IS("use") || !(wand->worn_by == ch) ||
            GET_EQ(ch,WEAR_HOLD) != wand) {  /*overkill i guess*/
        return FALSE;
    }

    half_chop(argument, arg, buf);
    if (!*arg) {
        sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
        send_to_char(buf2, ch);
        return TRUE;
    }

    if (!isname(arg, wand->name)) {
        sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg),arg);
        send_to_char(buf2, ch);
        return TRUE;
    }
    /* we have the wand, now we need a target */
    one_argument(buf, arg);

    k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                     FIND_OBJ_EQUIP, ch, &tch, &tobj);

    if (k == FIND_CHAR_ROOM) {
        if (tch == ch) {
            act("You point $p at yourself.", FALSE, ch, wand, 0, TO_CHAR);
            act("$n points $p at $mself.", FALSE, ch, wand, 0, TO_ROOM);
        } else {
            act("You point $p at $N.", FALSE, ch, wand, tch, TO_CHAR);
            act("$n points $p at $N.", TRUE, ch, wand, tch, TO_ROOM);
        }
    } else if (tobj != NULL) {
        act("You point $p at  $P.", FALSE, ch, wand, tobj, TO_CHAR);
        act("$n points $p at $P.", TRUE, ch, wand, tobj, TO_ROOM);
    } else {
        act("At what should $p be pointed?", FALSE, ch, wand, NULL, TO_CHAR);
        return TRUE;
    }

    /* now, we'll make life simple on us, at least for WoW I...
           assume that there are no object focused spells */

    if(tobj) {
        send_to_char("That seems to have no effect!\r\n",ch);
        return TRUE;
    }
    if (GET_OBJ_VAL(wand, 2) <= 0) {
        act("It seems powerless.", FALSE, ch, wand, 0, TO_CHAR);
        act("Nothing seems to happen.", FALSE, ch, wand, 0, TO_ROOM);
        return TRUE;
    }
    GET_OBJ_VAL(wand, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);

    i=number(0,100);
    if(i <= 10) {  /* slow target for a few rounds */
        act("A numbing blue glow encases $N - slowing him down!", FALSE, ch,
            0, tch, TO_ROOM);
        act("A numbing blue glow encases you - slowing you down!", FALSE, ch,
            0, tch, TO_VICT);
        WAIT_STATE(tch,PULSE_VIOLENCE*10); /* less you have a slow spell */
    } else if (i >= 11 && i <= 18) {    /* delude caster?..ouch */
        k=number(1,19);
        switch(k) {
        case 1:
            act("A numbing blue glow encases $N - slowing him down!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 2:
            act("$N turns into a large grey elephant!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 3:
            act("A fierce gust of wind hits $N!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 4:
            act("Your wand releases a poisonous stinking cloud!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 5:
            act("The heavens release a torrent of rain, blurring your visibility, and making movement difficult!"          , FALSE, ch, 0 , tch, TO_CHAR);
            break;
        case 6:
            k=number(1,3);
            switch(k) {
            case 1:
                send_to_char("You have summoned a rhino!\r\n",ch);
                break;
            case 2:
                send_to_char("You have summoned an elephant!\r\n",ch);
                break;
            case 3:
                send_to_char("You have summoned a dangerous mouse!\r\n",ch);
                break;

            }
            break;
        case 7:
            act("$N is hit by three consecutive lighting bolts!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 8:
            act("A stream of 600 large butterflies pours forth from the $p -Blinding everyone in the room!",FALSE,ch,wand,tch,TO_CHAR);
            break;
        case 9:
            act("$N grows to double normal size!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 10:
            act("The entire area becomes dark!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 11:
            act("The plants and grasses in the area surge forth in unparalled growth!",
                FALSE,ch,0,tch,TO_CHAR);
            break;
        case 12:
            act("$N fades from sight!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 13:
            act("You shrink to 1/12'th your height!!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 14:
            act("Your fireball hits $N squarely in the chest!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 15:
            act("You turn invisible!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 16:
            act("$N begins to sprout leaves from his body!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 17:
            act("A stream of gemstones is expelled from the $p, striking all those in its path!",
                FALSE, ch, wand, tch, TO_CHAR);
            break;
        case 18:
            act("Shimmering colors dance and play in the area - blinding all in the area!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        case 19:
            act("$n is turned to stone!!!", FALSE,
                ch, 0, tch, TO_CHAR);
            break;
        }
        return TRUE;
    } else if (i >= 19 && i <= 25) { /* gust of wind */
        act("A fierce gust of wind hits $N!", FALSE,
            ch, 0, tch, TO_ROOM);
        act("A fierce gust of wind hits you!", FALSE,
            ch, 0, tch, TO_VICT);

        k = dice(5, 8) + 5;
        /* i'd use damage, but ... i don't want any more messages */
        if(GET_HIT(tch) > k) {
            GET_HIT(tch) = GET_HIT(tch) - k;
        } else {
            die(tch);
        }
        return TRUE;
    } else if (i >=26 && i <= 30) { /* stinking cloud. ish. */
        act("$n's $p releases a poisonous stinking cloud!", FALSE,
            ch, wand, tch, TO_ROOM);
        act("Your $p releases a poisonous stinking cloud!", FALSE,
            ch, wand, tch, TO_CHAR);
        for (tch = world[ch->in_room].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            call_magic(ch, tch, 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL, 0);
        }
        return TRUE;
    } else if (i >= 31 && i <=33) {
        act("The heavens release a torrent of rain, blurring your visibility, and making movement difficult!"
            , FALSE, ch, 0 , tch, TO_ROOM);
        act("The heavens release a torrent of rain, blurring your visibility, and making movement difficult!"
            , FALSE, ch, 0 , tch, TO_CHAR);

        for (tch = world[ch->in_room].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            call_magic(ch, tch, 0, SPELL_BLINDNESS, 3, CAST_SPELL, 0);
            GET_MOVE(tch) = GET_MOVE(tch)/2;
        }
        return TRUE;

    } else if (i >=34 && i <=36) { /* conjure a mob */

        /* 2 choices... load an existing mob, or make a new one */
        /* rough, but i'll make a puff mob, but alter stats, easier */
        /* so, I guess, PUFF must exist. (vn 1) */
        k=number(1,4);

        mob = read_mobile(1, VIRTUAL, world[ch->in_room].zone);
        IS_CARRYING_W(mob) = 0;
        IS_CARRYING_N(mob) = 0;
        SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
        add_follower(mob, ch);
        /*DISPOSE(GET_NAME(mob));
        DISPOSE(mob->player.long_descr);
        DISPOSE(mob->player.description);
        DISPOSE(mob->player.short_descr);
          */
        if(k==1) { /* rhino */
            SET_NAME(mob,str_dup("rhino"));
            mob->player.short_descr=str_dup("rhino");
            mob->player.long_descr=str_dup("A wild looking rhino");
            mob->player.description=str_dup("The beast looks quite dangerous....");
            GET_LEVEL(mob)=20;
            GET_MAX_HIT(mob)=350;
            GET_HIT(mob)=350;
            GET_MAX_MOVE(mob)=350;
            GET_MOVE(mob)=350;
            GET_AC(mob)=0;
            mob->mob_specials.damnodice=4;
            mob->mob_specials.damsizedice=10;
        } else if (k ==2) {
            SET_NAME(mob, str_dup("elephant"));
            mob->player.short_descr=str_dup("elephant");
            mob->player.long_descr=str_dup("A fierce bull elephant");
            mob->player.description=str_dup("This raging bull elephant looks really pissed off!");
            GET_LEVEL(mob)=15;
            GET_MAX_HIT(mob)=650;
            GET_HIT(mob)=650;
            GET_MAX_MOVE(mob)=350;
            GET_MOVE(mob)=350;
            GET_AC(mob)=20;
            mob->mob_specials.damnodice=2;
            mob->mob_specials.damsizedice=15;
        } else {
            SET_NAME(mob, str_dup("mouse"));
            mob->player.short_descr=str_dup("mouse");
            mob->player.long_descr=str_dup("A tiny squeaking mouse");
            mob->player.description=str_dup("The mouse looks more interested in running than fighting!");
            GET_LEVEL(mob)=2;
            GET_MAX_HIT(mob)=20;
            GET_HIT(mob)=20;
            GET_MAX_MOVE(mob)=90;
            GET_MOVE(mob)=90;
            GET_AC(mob)=100;
            mob->mob_specials.damnodice=2;
            mob->mob_specials.damsizedice=1;
        }
        char_to_room(mob,ch->in_room);
        return TRUE;
    } else if (i >= 37 && i <= 46) {
        act("$N is hit by three consecutive lighting bolts!", FALSE,
            ch, 0, tch, TO_ROOM);
        act("You've been hit by three consecutive lighting bolts!",
            FALSE,ch,0,tch,TO_VICT);
        call_magic(ch, tch, 0, SPELL_LIGHTNING_BOLT, 30, CAST_SPELL, 0);
        if(tch && GET_HIT(tch) > 0)
            call_magic(ch, tch, 0, SPELL_LIGHTNING_BOLT, 30, CAST_SPELL, 0);
        if(tch && GET_HIT(tch) > 0)
            call_magic(ch, tch, 0, SPELL_LIGHTNING_BOLT, 30, CAST_SPELL, 0);
        return TRUE;

    } else if(i >= 47 && i <= 49) {
        act("A stream of 600 large butterflies pours forth from the $p - blinding everyone in the room!",FALSE,ch,wand,tch,TO_ROOM);
        for (tch = world[ch->in_room].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            call_magic(ch, tch, 0, SPELL_BLINDNESS, 3, CAST_SPELL, 0);
        }
        return TRUE;

    } else if (i >= 50 && i <= 53) {
        act("$N suddenly doubles in size!", FALSE,
            ch, 0, tch, TO_ROOM);
        act("You suddenly double in size!",
            FALSE,ch,0,tch,TO_VICT);
        call_magic(ch, tch, 0, SPELL_STRENGTH, 20, CAST_SPELL, 0);
        call_magic(ch, tch, 0, SPELL_ARMOR, 20, CAST_SPELL, 0);
        return TRUE;

    } else if (i >= 54 && i <= 58) {
        act("All your lights suddenly go out!", FALSE,ch,0,tch,TO_ROOM);
        world[ch->in_room].light = 0;
        if (!ROOM_FLAGGED(ch->in_room, ROOM_DARK)) {
            SET_BIT(ROOM_FLAGS(ch->in_room), ROOM_DARK);
        }
        for (tch = world[ch->in_room].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if (GET_EQ(ch, WEAR_LIGHT) != NULL)
                if (GET_OBJ_TYPE(GET_EQ(tch, WEAR_LIGHT)) == ITEM_LIGHT)
                    GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2) = 0;
        }
        return TRUE;

    } else if (i >= 59 && i <= 62) {
        act("All the vegetation in the area suddenly doubles - no quadruples in size!", FALSE,ch,0,tch,TO_ROOM);
        world[ch->in_room].sector_type=SECT_FOREST;
        return TRUE;
    } else if (i >= 63 && i <= 65) {
        act("$N fades from sight!", FALSE,
            ch, 0, tch, TO_ROOM);
        act("You fade from sight!", FALSE,
            ch, 0, tch, TO_VICT);
        call_magic(ch, tch, 0, SPELL_INVISIBLE, 20, CAST_SPELL, 0);
        return TRUE;
    } else if (i >= 66 && i <= 69) {
        act("$n suddenly shrinks to 1/12 of his normal size!", FALSE,
            ch, 0, tch, TO_ROOM);
        act("You suddenly shrink to 1/12 of your normal size!", FALSE,
            ch, 0, tch, TO_CHAR);
        call_magic(ch, ch, 0, SPELL_CHILL_TOUCH, 25, CAST_SPELL, 0);
        return TRUE;
    } else if (i >= 70 && i <= 79) {
        act("$N is engulfed in a nasty fireball!", FALSE, ch, 0, tch, TO_ROOM);
        act("You are engulfed in a nasty fireball!", FALSE, ch, 0 , tch, TO_VICT);
        call_magic(ch, tch, 0 , SPELL_FIREBALL, 30, CAST_SPELL, 0);
        return TRUE;
    } else if (i >= 80 && i <= 84) {
        act("You turn invisible!", FALSE,
            ch, 0, tch, TO_CHAR);
        act("$n turns invisibile!",FALSE,ch,0,tch,TO_ROOM);
        call_magic(ch,ch,0,SPELL_INVISIBLE,10,CAST_SPELL, 0);
        return TRUE;
    } else if (i >= 85 && i <= 87) {
        act("How odd - $N starts to sprout leaves and thorny bristles!", FALSE, ch, 0, tch, TO_ROOM);
        act("You start to sprout leaves and thorny bristles!", FALSE, ch,0, tch, TO_VICT);
        call_magic(ch,tch,0,SPELL_ARMOR,10,CAST_SPELL, 0);
        return TRUE;
    } else if (i >= 88 && i <= 90) {
        act("A spray of well cut gems sprays forth from the $p, striking all in the room!",
            FALSE, ch, 0 , tch, TO_ROOM);
        for (tch = world[ch->in_room].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if(tch != ch) {
                GET_HIT(tch) = GET_HIT(tch) - number(0,40);
            }
            if(GET_HIT(tch) <= 0) {
                die(tch);
            }
        }
        for(k=0;k!=40;k++) {
            sobj=create_obj();

            sobj->item_number = NOTHING;
            sobj->in_room = NOWHERE;
            sobj->name = str_dup("gem");
            sobj->description = str_dup("a glittering gem is lying here");
            sobj->short_description = str_dup("a glittering gem");
            GET_OBJ_TYPE(sobj) = ITEM_TREASURE;
            GET_OBJ_WEAR(sobj) = ITEM_WEAR_TAKE & ITEM_WEAR_HOLD;
            GET_OBJ_COST(sobj) = 1 + number(0,10);
            GET_OBJ_WEIGHT(sobj) = 0;
            obj_to_room(sobj,ch->in_room);
        }
        return TRUE;
    } else if (i >= 91 && i <= 97) {
        act("An amazing field of shimmering colors plays over the entire area!", FALSE,
            ch, 0, tch, TO_ROOM);
        /* lets blind EVERYONE in the zone cept for user. I'm feeling pissy.*/
        for (d = descriptor_list; d; d = d->next) {
            if(!IS_NPC(d->character) && (world[d->character->in_room].zone ==
                                         world[ch->in_room].zone) && (d->character != ch)) {
                send_to_char("A bright flash of color blinds you!\r\n",d->character);
                call_magic(ch,d->character,0,SPELL_BLINDNESS,10,CAST_SPELL, 0);
            }
        }
        return TRUE;
    } else {
        act("$N has been turned to STONE!",FALSE,ch,0,tch,TO_ROOM);
        act("You have been turned to stone! What suckage!", FALSE, ch,0,tch,
            TO_VICT);
        for (k = 0; k < NUM_WEARS; k++) {
            if (GET_EQ(ch, k)) {
                unequip_char(tch, k);
            }
        }

        /* delete all items he has */
        for(sobj=tch->carrying;sobj;sobj=sobj->next_content) {
            extract_obj(sobj);
        }

        /* kill the sorry bastard now */
        if (!IS_NPC(tch))
            REMOVE_BIT(PLR_FLAGS(tch), PLR_KILLER | PLR_THIEF);
        if (FIGHTING(tch))
            stop_fighting(tch);
        while (tch->affected)
            affect_remove(tch, tch->affected);
        death_cry(tch);

        /* build a monmument to the dead, pathetic slob */

        sobj=create_obj();
        sobj->item_number = NOTHING;
        sobj->in_room = NOWHERE;
        sobj->name = str_dup("stone");

        sprintf(buf2, "The statue of %s stands here.", GET_NAME(tch));
        sobj->description = str_dup(buf2);

        sprintf(buf2, "the statue of %s", GET_NAME(tch));
        sobj->short_description = str_dup(buf2);
        GET_OBJ_TYPE(sobj)= ITEM_TRASH;
        GET_OBJ_WEAR(sobj)= ITEM_WEAR_TAKE;
        GET_OBJ_EXTRA(sobj)= ITEM_NODONATE;
        GET_OBJ_WEIGHT(sobj) =  GET_TOTAL_WEIGHT(tch);

        /* thats good for now */
        obj_to_room(sobj, tch->in_room);

        extract_char(tch);
        return TRUE;
    }
    return TRUE;
}




SPECIAL(repair_shop)
{

    struct obj_data *obj;
    char loc_buf[MAX_STRING_LENGTH];
    char loc_buf2[MAX_STRING_LENGTH];
    int i = 0, len = 0, total_len = 0;
    int done = FALSE;
    int cost;
    struct obj_data *bag;

    *loc_buf='\0';
    *loc_buf2='\0';
    if (!cmd && !FIGHTING((struct char_data *) me)) {
        if (number(1, 80) < 10) {
            command_interpreter(me, "say All kinds of repairs for a reasonable amount of money!");
            command_interpreter(me, "say I can make old things good as new!");
            return 1;
        }
        return 0;
    }

    if (CMD_IS("repair")) {
        argument = one_argument(argument, buf);
        //skip_spaces(buf);
        if (!strcmp("all", buf))
        {
            int sum=0;
            for (i = 0; i < NUM_WEARS; i++)
                if ((obj=GET_EQ(ch, i)) && GET_OBJ_DAMAGE(obj)!=100) {
                    cost=MAX(1, (110-GET_OBJ_DAMAGE(obj))*GET_OBJ_COST(obj)/100);
                    if (GET_GOLD(ch) < cost) {
                        sprintf(buf, "$N says to you, 'You can't afford to repair $p.'");
                        act(buf, FALSE, ch, obj, me, TO_CHAR);
                        continue;
                    }
                    GET_GOLD(ch) -= cost;
                    sum+=cost;
                    GET_OBJ_DAMAGE(obj)=100;
                    if (GET_OBJ_TYPE(obj)==ITEM_ARMOR)
                    {
                        GET_AC(ch)+=GET_OBJ_VAL(obj, 1)-GET_OBJ_VAL(obj, 0);
                        GET_OBJ_VAL(obj, 0)=GET_OBJ_VAL(obj, 1);
                    }
                }
            if (sum>0)
            {
                act("$N starts repairing stuff.",  FALSE, ch, 0, me, TO_ROOM);
                sprintf(buf2, "Everything is repaired now, %s.", GET_NAME(ch));
                do_say(me, buf2,0,0);
                sprintf(buf2, "It will cost you %d coins.", sum);
                do_say(me, buf2,0,0);
            }
            else
            {
                do_say(me, "There is nothing I can repair there.",0,0);
            }
            affect_total(ch);
            return 1;
        }
        else if (TRUE) {
            obj = get_obj_in_list_vis(ch, buf, ch->carrying);
            if (!obj) {
                act("$N says to you 'Sorry. You don't seem to have"
                    " that.'", FALSE, ch, 0, me, TO_CHAR);
                return TRUE;
            }
            if (GET_OBJ_DAMAGE(obj)==100)
            {
                act("$N says to you, 'That is in no need for repair.", FALSE, ch, 0, me, TO_CHAR);
                return TRUE;
            }
            cost=MAX(1, (110-GET_OBJ_DAMAGE(obj))*GET_OBJ_COST(obj)/100);

            if (GET_GOLD(ch) < cost) {
                sprintf(buf, "$N says to you, 'Sorry. To fully repair that, you will need %d gold coins.'", cost);
                act(buf, FALSE, ch, 0, me, TO_CHAR);
                return TRUE;
            } else {
                act("You hand $p to $N, who nods $S head, and immediately starts working on it.",  FALSE, ch, obj, me, TO_CHAR);
                act("$N starts repairing something.",  FALSE, ch, obj, me, TO_ROOM);
                GET_GOLD(ch) -= cost;
                sprintf(buf, "After some time $N hands you $p and says, 'It's good as new! That'll be %d gold coins.'", cost);
                act(buf, FALSE, ch, obj, me, TO_CHAR);
                //act("$N says to you 'Here you go. Its good as new again.'",FALSE, ch, 0, me, TO_CHAR);
                GET_OBJ_DAMAGE(obj)=100;
                if (GET_OBJ_TYPE(obj)==ITEM_ARMOR)
                {
                    GET_AC(ch)+=GET_OBJ_VAL(obj,1)-GET_OBJ_VAL(obj, 0);
                    GET_OBJ_VAL(obj, 0)=GET_OBJ_VAL(obj, 1);
                }

                //affect_total(ch);
                return 1;
            }
        }
    }
    return 0;
}





extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern obj_rnum top_of_objt;
extern struct descriptor_data *descriptor_list;

extern FILE    *player_fl;

ACMD(do_say);

struct tella_data
{
    obj_num vnum;

    int sell_price;
    char seller_name[50];
    int seller_id;
    int level;
    int	value[4];
    long	extra_flags;
    long	extra_flags2;
    int	weight;
    int cost;
    int	timer;
    int damage;
    int orig_value;	// this is spare spot for now
    byte data;
    long	bitvector;
    long	bitvector2;
    long	bitvector3;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
    int bound_spell, bound_spell_level, bound_spell_timer;
    char owner_name[20];
    char name[100];
    char description[100];
    char short_description[100];
    int spare1;
    int spare2;
    int spare3;
};
extern char *obj_condition_names[];
void custom_identify(struct char_data *ch, struct obj_data *obj)
{
    int found, i;

    sprintf(buf, "Object '&c%s&0', Item type: &c", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    if (GET_OBJ_TYPE(obj)==ITEM_CONTAINER)
        sprintf(buf2,"%s (holds %d)", buf2, GET_OBJ_VAL(obj, 0));
    strcat(buf, buf2);
    strcat(buf, "&0\r\n");
    send_to_char(buf, ch);

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

}

SPECIAL(salesman)
{
    FILE           *fl;
    char            file_name[100];
    int nsell=0;
    struct char_data *customer;
    struct obj_data *obj;
    struct tella_data item,
                temp,
                *list = NULL;
    int             number,
    nr,level=0, j,
             found = FALSE;
    char            arg[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];
    obj_rnum        rnum = NOTHING;
    struct char_file_u temp_u;
    struct descriptor_data *d;

    customer = ch;
    ch = (struct char_data *) me;

    if (!cmd || (
                !CMD_IS("list") &&
                !CMD_IS("sell") &&
                !CMD_IS("info") &&
                !CMD_IS("buy")) )
{
        if (!FIGHTING(ch)) {
            if (dice(1, 8)==1)
            {
                do_say(ch, "Howdie, howdie folks! Type 'list' to see what fine goods I have available for you!", 0, 0);
                return FALSE;
            }
            else
                return FALSE;
        }
        else
            return FALSE;
    }


    sprintf(file_name, "salesman/%d", GET_MOB_VNUM(ch));

    switch (LOWER(*cmd_info[cmd].command)) {
    case 'l':
        number = 0;

        if (!(fl = fopen(file_name, "rb"))) {
            do_say(ch, "Well I have nothing... Yet!", 0, 0);
            do_say(ch, "Maybe YOU would like me to sell something for you? (type 'sell <item> <price>')", 0,0);
            return (TRUE);
        }

        if (!*argument)
            level=0;
        else
        {
            one_argument(argument, arg);
            level = atoi(arg);

            /*if ((level = atoi(arg))  0) {
                send_to_char("Positive number please.\r\n", customer);
                return TRUE;
        } */
        }

        do_say(ch, "I have following items for sale.", 0, 0);
        send_to_char("\n&wNo. Item                                             Asking Price        Sellers Name&0\r\n", customer);
        send_to_char("&0--- ----                                             ------------        ------------&0\r\n", customer);

        while (!feof(fl)) {
            fread(&item, sizeof(struct tella_data), 1, fl);
            if (feof(fl))
                break;

            number++;
            if ((rnum = find_obj_rnum(item.vnum)) == NOTHING)
                continue;


            if (level)
            {
                if (level<0 && item.level>-level)
                    continue;
                else if (level>0 && item.level<level)
                    continue;
            }

            sprintf(buf, "%-4d&c%-50s&0%-20d%s\r\n", number, item.short_description, item.sell_price,
                    item.seller_name);
            send_to_char(buf, customer);
        }
        fclose(fl);

        send_to_char("\r\nType 'list <level>/<-level>' to list only items of level higher/lower then specified.\r\nType 'info <number>' to get more information on item.\r\nType 'buy' or 'sell' to make bussiness with me.\r\n", customer);
        return (TRUE);
    case 'i':
        number = 0;

        if (!(fl = fopen(file_name, "rb"))) {
            do_say(ch, "There is currently nothing for sale here.", 0, 0);
            return (TRUE);
        }
        if (!*argument) {
            send_to_char("Specify item number please.\r\n", customer);
            return (TRUE);
        } else {
            one_argument(argument, arg);
            if ((number = atoi(arg)) < 0) {
                send_to_char("Positive number please.\r\n", customer);
                return TRUE;
            }
            if (GET_GOLD(customer) < 50) {
                do_say(ch, "You need 50 gold to get that information.", 0, 0);
                return (TRUE);
            }
            nr = 0;

            while (!feof(fl)) {
                fread(&item, sizeof(struct tella_data), 1, fl);
                if (feof(fl))
                    break;

                if (++nr == number)
                    break;
            }

            if (feof(fl)) {
                do_say(ch, "I don't have that many items in stock!", 0, 0);
                fclose(fl);
                return (TRUE);
            }
            obj = read_object(item.vnum, VIRTUAL, 0, 0);
            GET_OBJ_VAL(obj, 0) = item.value[0];
            GET_OBJ_VAL(obj, 1) = item.value[1];
            GET_OBJ_VAL(obj, 2) = item.value[2];
            GET_OBJ_VAL(obj, 3) = item.value[3];
            GET_OBJ_EXTRA(obj) = item.extra_flags;
            GET_OBJ_EXTRA2(obj) = item.extra_flags2;
            GET_OBJ_DATA(obj)=item.data;
            GET_OBJ_RENT(obj) = item.weight;
            GET_OBJ_COST(obj) = item.cost;
            GET_OBJ_TIMER(obj) = item.timer;
            GET_OBJ_SPELL(obj)=item.bound_spell;
            GET_OBJ_SPELL_LEVEL(obj)=item.bound_spell_level;
            GET_OBJ_SPELL_TIMER(obj)=item.bound_spell_timer;
            GET_OBJ_DAMAGE(obj)=item.damage;
            GET_OBJ_ORIG(obj)=item.orig_value;
            obj->obj_flags.bitvector = item.bitvector;
            obj->obj_flags.bitvector2 = item.bitvector2;
            obj->obj_flags.bitvector3 = item.bitvector3;
            strcpy(obj->owner_name, item.owner_name);
            obj->name = str_dup(item.name);
            obj->description = str_dup(item.description);
            obj->short_description = str_dup(item.short_description);
            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                obj->affected[j] = item.affected[j];

            GET_GOLD(customer) -= 50;
            act("$n takes your gold and begins to tell you about $p.", FALSE, ch, obj, customer, TO_VICT);
            act("$n takes $N's gold and begins to tell $M about $p.", FALSE, ch, obj, customer, TO_NOTVICT);
            //call_magic(customer, 0, obj, SPELL_IDENTIFY, 50, CAST_SPELL, "");
            custom_identify(customer, obj);
            fclose(fl);
            extract_obj(obj);
            return (TRUE);
        }
        return (TRUE);
    case 's':
        two_arguments(argument, arg, arg2);

        if (!*arg || !*arg2) {
            send_to_char("Syntax: sell <item> <price> \r\n", customer);
            return (TRUE);
        }

        nsell=0;
        if ((fl = fopen(file_name, "rb"))) {

            // check how many items already selling for that customer
            while (!feof(fl)) {
                fread(&item, sizeof(struct tella_data), 1, fl);
                if (feof(fl))
                    break;
                if (!strcmp(GET_NAME(customer), item.seller_name))
                    nsell++;

            }
            fclose(fl);
        }

        if (nsell>=3)
        {
            do_say(ch, "I am already having enough of your things on the sale.", 0, 0);
            return (TRUE);
        }



        if (!(obj = get_obj_in_list_vis(customer, arg, customer->carrying))) {
            do_say(ch, "You havn't even got that!", 0, 0);
            return (TRUE);
        }    
        
        if (Crash_is_unrentable(obj)) {
        	    do_say(ch, "I won't accept that for sale!", 0, 0);
            return (TRUE);
        }    
        
        if ((number = atoi(arg2)) < 0) {
            do_say(ch, "Not a negative price please!", 0, 0);
            return (TRUE);
        }
        if (number<10) {
            do_say(ch, "You have to make it at least 10 coins.", 0, 0);
            return (TRUE);
        }



        memset(&item, 0, sizeof(struct tella_data));

        item.value[0] = GET_OBJ_VAL(obj, 0);
        item.value[1] = GET_OBJ_VAL(obj, 1);
        item.value[2] = GET_OBJ_VAL(obj, 2);
        item.value[3] = GET_OBJ_VAL(obj, 3);
        item.extra_flags = GET_OBJ_EXTRA(obj);
        item.extra_flags2 = GET_OBJ_EXTRA2(obj);
        item.data=GET_OBJ_DATA(obj);
        item.cost=GET_OBJ_COST(obj);
        item.weight = GET_OBJ_RENT(obj);
        item.timer = GET_OBJ_TIMER(obj);
        item.bitvector = obj->obj_flags.bitvector;
        item.bitvector2 = obj->obj_flags.bitvector2;
        item.bitvector3 = obj->obj_flags.bitvector3;
        item.bound_spell=GET_OBJ_SPELL(obj);
        item.bound_spell_level=GET_OBJ_SPELL_LEVEL(obj);
        item.bound_spell_timer=GET_OBJ_SPELL_TIMER(obj);
        item.damage=GET_OBJ_DAMAGE(obj);
        item.orig_value=GET_OBJ_ORIG(obj);
        item.level=GET_OBJ_LEVEL(obj);

        if (obj->name)
            strncpy(item.name,obj->name ,100);
        if (obj->description)
            strncpy(item.description,obj->description,100);
        if (obj->short_description)
            strncpy(item.short_description, obj->short_description,100);

        for (j = 0; j < MAX_OBJ_AFFECT; j++)
            item.affected[j] = obj->affected[j];
        strcpy(item.owner_name, obj->owner_name);

        item.sell_price = number;
        item.seller_id = GET_IDNUM(customer);
        sprintf(item.seller_name, GET_NAME(customer));
        item.vnum = GET_OBJ_VNUM(obj);


        number = 0;
        if (!(fl = fopen(file_name, "rb")))
            number = 0;
        else {
            while (!feof(fl)) {
                fread(&temp, sizeof(struct tella_data), 1, fl);
                if (feof(fl))
                    break;
                number++;
            }
            CREATE(list, struct tella_data, number);
            rewind(fl);
            number = 0;
            while (!feof(fl)) {
                fread(&temp, sizeof(struct tella_data), 1, fl);
                if (feof(fl))
                    break;
                memcpy(&list[number++], &temp, sizeof(struct tella_data));
            }
            fclose(fl);
        }
        if (!(fl = fopen(file_name, "wb"))) {
            do_say(ch, "Sorry I don't accept anything now.", 0, 0);
            log("SYSERR: Can't open file for writing in salesman - sell.");
            if (list)
                free(list);
            return (TRUE);
        }
        if (number)
            for (j = 0; j < number; j++)
                fwrite(&list[j], sizeof(struct tella_data), 1, fl);
        if (list)
            free(list);
        fwrite(&item, sizeof(struct tella_data), 1, fl);
        fflush(fl);
        fclose(fl);
        do_say(ch, "Ok I'll keep hold of that one for you, I'll take 10% of the profit mind!", 0, 0);
        act("You hand $n $p.", FALSE, ch, obj, customer, TO_VICT);
        act("$N hands $n $p.", FALSE, ch, obj, customer, TO_NOTVICT);
        obj_from_char(obj);

        extract_obj(obj);
        return (TRUE);
    case 'b':
        number = 0;
        if (!*argument) {
            send_to_char("Syntax: buy  <item number>\r\n", customer);
            return (TRUE);
        }
        one_argument(argument, arg);
        if (!isdigit(arg[0])) {
            do_say(ch, "Please use the number of the object for reference, not the name.", 0, 0);
            return (TRUE);
        } else if ((nr = atoi(arg)) < 0) {
            do_say(ch, "A positive number please!", 0, 0);
            return (TRUE);
        }
        if (!(fl = fopen(file_name, "rb"))) {
            do_say(ch, "I don't have anything for sale right now.", 0, 0);
            return (TRUE);
        } else {
            while (!feof(fl)) {
                fread(&temp, sizeof(struct tella_data), 1, fl);
                if (feof(fl))
                    break;
                number++;
            }
        } if (nr > number) {
            do_say(ch, "I don't think i have that many items in my store!", 0, 0);
            fclose(fl);
            return (TRUE);
        } CREATE(list, struct tella_data, number);
        rewind(fl);
        number = 0;
        while (!feof(fl)) {
            fread(&temp, sizeof(struct tella_data), 1, fl);
            if (feof(fl))
                break;
            memcpy(&list[number++], &temp, sizeof(struct tella_data));
        } fclose(fl);
        if (GET_GOLD(customer) < list[nr - 1].sell_price && GET_IDNUM(customer) != list[nr - 1].seller_id) {
            do_say(ch, "Hah, you're too poor go away!", 0, 0);
            return (TRUE);
        } memcpy(&item, &list[nr - 1], sizeof(struct tella_data));
        if (!(fl = fopen(file_name, "wb"))) {
            do_say(ch, "Sorry my door is jammed into the store room.", 0, 0);
            log("SYSERR: Can't open file for writing in salesman - sell.");
            return (TRUE);
        } for (j = 0;j < number;j++)
            if (j != nr - 1)
                fwrite(&list[j], sizeof(struct tella_data), 1, fl);
        fflush(fl);
        fclose(fl);
        obj = read_object(item.vnum, VIRTUAL, 0, 0);
        GET_OBJ_VAL(obj, 0) = item.value[0];
        GET_OBJ_VAL(obj, 1) = item.value[1];
        GET_OBJ_VAL(obj, 2) = item.value[2];
        GET_OBJ_VAL(obj, 3) = item.value[3];
        GET_OBJ_EXTRA(obj) = item.extra_flags;
        GET_OBJ_EXTRA2(obj) = item.extra_flags2;
        GET_OBJ_DATA(obj)=item.data;
        GET_OBJ_RENT(obj) = item.weight;
        GET_OBJ_COST(obj) = item.cost;
        GET_OBJ_TIMER(obj) = item.timer;
        GET_OBJ_SPELL(obj)=item.bound_spell;
        GET_OBJ_SPELL_LEVEL(obj)=item.bound_spell_level;
        GET_OBJ_SPELL_TIMER(obj)=item.bound_spell_timer;
        GET_OBJ_DAMAGE(obj)=item.damage;
        GET_OBJ_ORIG(obj)=item.orig_value;
        obj->obj_flags.bitvector = item.bitvector;
        obj->obj_flags.bitvector2 = item.bitvector2;
        obj->obj_flags.bitvector3 = item.bitvector3;
        strcpy(obj->owner_name, item.owner_name);
        obj->name = str_dup(item.name);
        obj->description = str_dup(item.description);
        obj->short_description = str_dup(item.short_description);
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
            obj->affected[j] = item.affected[j];
        if (GET_IDNUM(customer) != item.seller_id) {
            GET_GOLD(customer) -= list[nr - 1].sell_price;
            do_say(ch, "Good deal!", 0, 0);
            act("$n hands you $p.", FALSE, ch, obj, customer, TO_VICT);
            act("$n hands $N $p.", FALSE, ch, obj, customer, TO_NOTVICT);
            found = FALSE;
            for (d = descriptor_list;                d;                d = d->next) {
                if (STATE(d) != CON_PLAYING)
                    continue;
                if (GET_IDNUM(d->character) == item.seller_id) {
                    found = TRUE;

                    sprintf(buf, "A man runs in and informs you that $N has bought $p for %d coins.\r\n", item.sell_price);
                    sprintf(buf + strlen(buf), "He also says you received %d gold from the sale which has been deposited in your bank account.", ((item.sell_price *9) /10));
                    act(buf, FALSE, d->character, obj, customer, TO_CHAR);
                    act("A man runs in and talks to $n.", FALSE, d->character, 0, 0, TO_ROOM);
                    send_to_room("The man turns and is gone as fast as he entered.\r\n", d->character->in_room);
                    GET_BANK_GOLD(d->character) += ((item.sell_price / 10) * 9);
                }
            } if (!found) {
                number = 0;
                if (!(fl = fopen(PLAYER_FILE, "r+b")))
                    do_say(ch, "Well, I can't seem to find the sellers accounts, ah well, their loss.", 0, 0);
                else
                    while (!feof(fl)) {
                        fread(&temp_u, sizeof(struct char_file_u), 1, fl);
                        if (temp_u.char_specials_saved.idnum == item.seller_id) {
                            act("$n deposits some money into a jar.", FALSE, ch, 0, 0, TO_ROOM);
                            temp_u.points.bank_gold += ((item.sell_price *9) /10);
                            temp_u.player_specials_saved.sold_in_tellas += 1;
                            fseek(player_fl, number * sizeof(struct char_file_u), SEEK_SET);
                            fwrite(&temp_u, sizeof(struct char_file_u), 1, player_fl);
                            fflush(player_fl);
                            break;
                        } number++;
                    }
            }
            GET_GOLD(ch)+=item.sell_price/10;
        } else {
            do_say(ch, "Here, you don't want to sell it anymore? Have it back then!", 0, 0);
            act("$n hands you $p.", FALSE, ch, obj, customer, TO_VICT);
            act("$n hands $N $p.", FALSE, ch, obj, customer, TO_NOTVICT);
        }
        obj_to_char(obj, customer);
        save_char(customer, customer->in_room);
        free(list);
        return (TRUE);
    } return (FALSE);
}

obj_rnum      find_obj_rnum(obj_vnum vnum)
{
    int             i;
    for (i = 0;i < top_of_objt;i++)
        if (obj_index[i].virtual == vnum)
            return i;
    return NOTHING;
}


SPECIAL(play_war)
{
    int  wager;
    int  player_card;
    int  dealer_card;
    char buf[128];
    static char *cards[] =
        {"One", "Two", "Three", "Four", "Five", "Six", "Seven",
         "Eight", "Nine", "Ten", "Prince", "Knight", "Wizard"};
    static char *suits[] =
        {"Wands", "Daggers", "Wings", "Fire"};

    if (!cmd && !FIGHTING((struct char_data *) me)) {
        if (number(1, 80) < 10) {
            command_interpreter(me, "say Come and play a card game with me! Type 'gamble' to start!");
            return 1;
        }
        return 0;
    }



    if (!CMD_IS("gamble"))
        return 0;

    one_argument(argument, arg); /* Get the arg (amount to wager.) */

    if (!*arg) {
        do_say((struct char_data *) me, "So you wanna play War? It's simple, look. You take one card, I take one card.", 0, 0);
        do_say((struct char_data *) me, "Who gets the stronger card wins. And that's all.", 0, 0);
        do_say((struct char_data *) me, "Now, how much money we start with? (type 'gamble <money>')", 0, 0);
        return 1;
    } else wager = atoi(arg) ;

    if(wager <= 0){
        send_to_char("Very funny..\r\n", ch);
        return 1;
    } else if (wager > GET_GOLD(ch)){
        send_to_char("You don't have that much gold to wager.\r\n", ch);
        return 1;
    } else {
        /*  Okay - gamble away! */
        player_card=number(0, 12);
        dealer_card=MIN(12, number(0, 12)+1);
        if (wager<GET_GOLD(ch)/20)
            dealer_card=MAX(0, dealer_card-2);
        else if (wager<GET_GOLD(ch)/10)
            dealer_card=MAX(0, dealer_card-1);
        else if (wager>GET_GOLD(ch)/2)
            dealer_card=MIN(12, dealer_card+1);

        sprintf(buf, "You are dealt a %s of %s.\r\n"
                "The dealer turns up a %s of %s.\r\n",
                cards[player_card], suits[number(0, 3)],
                cards[dealer_card], suits[number(0, 3)]);
        send_to_char(buf, ch);
        if(player_card > dealer_card){
            /* You win! */
            sprintf(buf, "You win!  The dealer hands you %d gold coins.\r\n", wager);
            act("$n makes a wager and wins!", FALSE, ch, 0, 0, TO_ROOM);
            send_to_char(buf, ch);
            GET_GOLD(ch) += wager;
        } else if (dealer_card > player_card) {
            /* You lose */
            sprintf(buf, "You lose your wager of %d coins.\r\n"
                    "The dealer takes your gold.\r\n", wager);
            act("$n makes a wager and loses.", FALSE, ch, 0, 0, TO_ROOM);
            send_to_char(buf, ch);
            GET_GOLD(ch) -= wager;
        } else {
            /* WAR! */
            while (player_card==dealer_card) {
                send_to_char("&CWar!&0\r\n", ch);
                player_card=number(0, 12);
                dealer_card=number(0, 12);
                sprintf(buf, "You are dealt a %s of %s.\r\n"
                        "The dealer turns of a %s of %s.\r\n",
                        cards[player_card], suits[number(0, 3)],
                        cards[dealer_card], suits[number(0, 3)]);
                send_to_char(buf, ch);
            }
            if(player_card > dealer_card){
                /* You win! */
                sprintf(buf, "You win!  The dealer hands you %d gold "
                        "coins.\r\n", wager);
                act("$n makes a wager and wins!", FALSE, ch, 0, 0, TO_ROOM);
                send_to_char(buf, ch);
                GET_GOLD(ch) += wager;
            } else if (dealer_card > player_card) {
                /* You lose */
                sprintf(buf, "You lose your wager of %d coins.\r\n"
                        "The dealer takes your gold.\r\n", wager);
                act("$n makes a wager and loses.", FALSE, ch, 0, 0, TO_ROOM);
                send_to_char(buf, ch);
                GET_GOLD(ch) -= wager;
            }
        }
        return 1;
    }
}

