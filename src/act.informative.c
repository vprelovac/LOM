
/* ************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include <time.h>
#include <errno.h>
#include <sys/time.h>

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
#include "constants.h"
#include "clan.h"
#include "vt100c.h"

/* extern variables */
extern int      top_of_world;   /* In db.c */
extern struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];
extern int clan_loadroom[];
extern struct zone_data *zone_table;
extern int             top_of_zone_table;
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles1[LVL_IMPL + 1];
extern struct command_info cmd_info[];
extern struct index_data *mob_index, *obj_index;
extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
extern char *pc_class_types[];
extern char *pc_race_types[];

/*
extern struct str_app_type *str_app;
extern struct dex_app_type *dex_app;
*/
extern unsigned int pulse;
extern char *obj_condition_names[];
extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *buglist;
extern char *idealist;
extern char *typolist;
char *make_a_bar(char *buf, int left, int right, int pos, int len);
char *make_bar(char *buf, int percent, int len);
void autoscan(struct char_data *ch);
extern const char *carry_cond[];

long find_class_bitvector(char arg);
#define WFIELDS 7
char *fields[WFIELDS] = {
                            "small dragon",
                            "full moon",
                            "great flood",
                            "grand dragon",
                            "falling Sun",
                            "lightning sky",
                            "long shadows"
                        };

char *blood_messages[] = {
                             "&0There is no blood here.&0",
                             "&yThere are traces of blood here.&0",
                             "&yYou're standing in some blood.&0",
                             "&yYou're standing in some blood.&0",
                             "&yYou're standing in some blood.&0",
                             "&yThere's a lot of blood all around.&0",
                             "&yThere's a lot of blood all around.&0",
                             //"&rThere's so much blood here it's intoxicating!&0",
                             //"&rHow much more blood can there be in any one room?&0",
                             //"&rSuch carnage. The God of Death is feastin' tonight!&0",
                             "&rYou are splashing around in the blood of the slain!&0",
                             "&rYou are splashing around in the blood of the slain!&0",
                             "&rYou are splashing around in the blood of the slain!&0",
                             //"&rEven the walls are revolted by the death and destruction!&0",
                             //"&rThe Gods should show some mercy and cleanse this horrid room!&0",
                             "&RSo much blood has been shed here, you are drowning in it!&0",
                             "\n"
                         };



int ffield;
int boothigh=0;

int players_online()
{
    struct descriptor_data *d;
    int i=0;

    for (d = descriptor_list; d; d = d->next)
        if ((d->connected == CON_PLAYING) && (!IS_NPC(d->character)))
            i++;
    return i;
}

#define MAX_RAND_MES 3
char *random_messages[MAX_RAND_MES]=
    {
        "A bird dropping lands in front of you, just missing your head.\r\n",
        "The wind gently ruffles your hair.",
        "The wind gently ruffles your hair."
    };


void perform_players(int mode)
{
    struct descriptor_data *d;   
    struct char_data *ch;
    int i=0;

    for (d = descriptor_list; d; d = d->next)
    {
        if ((d->connected == CON_PLAYING) && (!IS_NPC(d->character))) {
            i++;
            if (AFF2_FLAGGED(d->character, AFF2_ROLLING)) {
                ffield++;
                if (ffield == WFIELDS)
                    ffield = 0;
                sprintf(buf, "Sign of the %s slowly passes by...\r\n", fields[ffield]);
                send_to_char(buf, d->character);
            }
            if (AFF2_FLAGGED(d->character, AFF2_BURNING) && IS_WET(d->character))
                specext(d->character, d->character);
                                                
            d->character->pk_timer-=PULSE_PLAYERS;
            ch=d->character;
            
            if (!number(0, 39))
            {
            	SET_FAITH(ch, GET_FAITH(ch)-1);
            	if (FOL_SKIG(ch) && !number(0, 3))
            		SET_FAITH(ch, GET_FAITH(ch)-1);
            	if (GET_FAITH(ch)>0 && CHANCE(GET_FAITH(ch)*GET_FAITH(ch)/1600))
            		SET_FAITH(ch, GET_FAITH(ch)-2);
            	
            	
            	
            	// skigs tax	
		if (FOL_SKIG(ch) && !number(0, 9))            		
		{
			GET_GOLD(ch)=MAX(0, GET_GOLD(ch)-number(1, GET_LEVEL(ch)/2));
			GET_BANK_GOLD(ch)=MAX(0, GET_BANK_GOLD(ch)-number(1, GET_LEVEL(ch)/2));
			send_to_char("&pSkig&0 takes his share of your gold.\r\n", ch);
			
			
		}
		if (FOL_BOUGAR(ch) && !number(0, 9))            		
		{
			GET_EXP(ch)+=(GET_FAITH(ch)-1500);
			send_to_char("&CBougar&0 takes his share of your experience.\r\n", ch);
		}
		
            
           }

        }
        if (i>boothigh) boothigh=i;
    }
}
void rprog_room_time_trigger( room_num nr);
void rprog_room_hour_trigger( room_num nr) ;
void rprog_room_random_trigger( room_num nr)  ;

void perform_rooms()
{
    room_num nr;
    extern int top_of_world;
    

    int isfullhour=!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC));
    for (nr=0;nr<top_of_world;nr++)
        if (world[nr].progtypes)
        {
            if (HAS_ROOM_PROG(&world[nr], RANDIW_PROG))
                rprog_room_random_trigger(nr);
            if (isfullhour && HAS_ROOM_PROG(&world[nr], TIME_PROG))
                rprog_room_time_trigger(nr);
            if (isfullhour && HAS_ROOM_PROG(&world[nr], HOUR_PROG))
                rprog_room_hour_trigger(nr);
        }
}






int number_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for (pdot = argument; *pdot != '\0'; pdot++) {
        if (*pdot == '.') {
            *pdot = '\0';
            number = atoi(argument);
            *pdot = '.';
            strcpy(arg, pdot + 1);
            return number;
        }
    }

    strcpy(arg, argument);
    return 1;
}

char *show_char_cond(int percent)
{

    if (percent >= 99)
        return ("&Wexcellent&0");
    else if (percent >= 90)
        return ("&Bscratched&0");
    else if (percent >= 76)
        return ("&Bbruised&0");
    else if (percent >= 58)
        return ("&Gwounded&0");
    else if (percent >= 42)
        return ("&Ghurt&0");
    else if (percent >= 25)
        return ("&Ypretty hurt&0");
    else if (percent >= 12)
        return ("&Yawful&0");
    else if (percent > 1)
        return ("&Ralmost dead&0");
    else
        return ("&RDYING&0");
}

// mode 11= eq
//extern const char *item_types[];
void show_obj_to_char(struct obj_data * object, struct char_data * ch,
                      int mode, int cr, int num)
{
    bool found, showflag, showflag2, sf3=TRUE;

    *buf = '\0';

    if ((mode <= 0) && object->description) {
        if (!IS_NPC(ch) && (GET_QUESTOBJ(ch) > 0) && (GET_OBJ_VNUM(object) == GET_QUESTOBJ(ch)))
            strcat(buf, "{QUEST} ");
        strcat(buf, object->description);
    } else if (object->short_description && ((mode == 1) || (mode == 11) ||
               (mode == 2) || (mode == 3) || (mode == 4)))
        strcpy(buf, object->short_description);
    else if (mode == 5) {
        if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
            if (object->action_description) {
                strcpy(buf, "There is something written upon it:\r\r\n\n");
                strcat(buf, object->action_description);
                page_string(ch->desc, buf, 1);
            } else
                act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        } else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON) {

            if (IS_SET(GET_OBJ_EXTRA(object), ITEM_EGO))
            {
                if (!IS_SET(GET_OBJ_EXTRA(object), ITEM_HIDDEN_EGO))
                    sprintf(buf, "%s\r\nYou can easily feel it's unnatural power.\r\n\r\n", buf);
                else    if (number(1, 101)<GET_SKILL(ch, SKILL_WARRIORCODE))
                {
                    sprintf(buf, "%s\r\n&cYou have a feeling that it could be really good.&0\r\n\r\n", buf);
                    improve_skill(ch, SKILL_WARRIORCODE, 1);
                }
            }






            sprintf(buf, "%sIt is %s, in %s condition.", buf,strlower(item_types[GET_OBJ_TYPE(object)]), obj_condition_names[GET_OBJ_DAMAGE(object)/10]);
            if (GET_OBJ_TYPE(object)==ITEM_LEVER)
            {

                if (GET_OBJ_VAL(object,0)== TRIG_UP)
                    sprintf(buf, "%s\r\nYou notice that it is in the up position.", buf );
                else
                    sprintf(buf, "%s\r\nYou notice that it is in the down position.", buf );
            }
            else  if (GET_OBJ_TYPE(object)==ITEM_BUTTON)
            {

                if (GET_OBJ_VAL(object,0)== TRIG_UP)
                    sprintf(buf, "%s\r\nYou notice that it is pressed.", buf );
                else
                    sprintf(buf, "%s\r\nYou notice that it is depressed.", buf );
            }


            //strcpy(buf, "You see nothing special...");
            //    mode = 3;
        } else {		/* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
            strcpy(buf, "It looks like a drink container.");
            //  mode = 3;
        }
    }
    else if (mode==6)
    {
        if (IS_SET(GET_OBJ_EXTRA(object), ITEM_EGO))
        {
            if (!IS_SET(GET_OBJ_EXTRA(object), ITEM_HIDDEN_EGO))
                sprintf(buf, "%s\r\nYou can easily feel it's unnatural power.\r\n", buf);
            else    if (number(1, 101)<GET_SKILL(ch, SKILL_WARRIORCODE))
            {
                sprintf(buf, "%s\r\n&cYou have a feeling that it could be really good.&0\r\n", buf);
                improve_skill(ch, SKILL_WARRIORCODE, 1);
            }
        }
        sprintf(buf, "%s\r\nIt is %s, in %s condition.", buf, item_types[GET_OBJ_TYPE(object)], obj_condition_names[GET_OBJ_DAMAGE(object)/10]);
        if (GET_OBJ_TYPE(object)==ITEM_LEVER)
        {

            if (GET_OBJ_VAL(object,0)== TRIG_UP)
                sprintf(buf, "%s\r\nYou notice that it is in the up position.", buf );
            else
                sprintf(buf, "%s\r\nYou notice that it is in the down position.", buf );
        }
        else  if (GET_OBJ_TYPE(object)==ITEM_BUTTON)
        {

            if (GET_OBJ_VAL(object,0)== TRIG_UP)
                sprintf(buf, "%s\r\nYou notice that it is pressed.", buf );
            else
                sprintf(buf, "%s\r\nYou notice that it is depressed.", buf );
        }
    }
    showflag = FALSE;
    showflag2 = FALSE;
    if (mode != 3) {
        found = FALSE;
        if (mode && IS_OBJ_STAT(object, ITEM_ENGRAVED)) {
            strcat(buf, " of ");
            strcat(buf, object->owner_name);
            found = TRUE;
        }
        if (mode!=11)
        {
            if (IS_OBJ_STAT(object, ITEM_HUM)) {
                if (mode!=11)
                    strcat(buf, " (hum)");
                //else
                //strcat(buf, " &y(hum)&0");
                found = TRUE;
            }
            if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
                if (mode!=11)
                    strcat(buf, " (invis)");
                //else
                //strcat(buf, " &w(invis)&0");
                found = TRUE;
            }



            if (IS_OBJ_STAT(object, ITEM_GLOW)) {
                if (mode!=11)
                    strcat(buf, " (soft");
                found = TRUE;
                showflag = TRUE;
            }
            if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {

                if (!showflag)
                    sprintf(buf, "%s (bluish", buf);
                else
                    sprintf(buf, "%s bluish", buf);
                showflag = TRUE;
                showflag2 = TRUE;
                found = TRUE;
            }
            if (IS_OBJ_STAT(object, ITEM_MAGIC) && CAN_SEE_MAGIC(ch)) {
                if (!showflag)
                    sprintf(buf, "%s (green", buf);
                else if (!showflag2)
                    sprintf(buf, "%s green", buf);
                else
                    sprintf(buf, "%s-green", buf);
                showflag = TRUE;
                found = TRUE;
            }
            if (showflag)
                sprintf(buf, "%s glow)", buf);
            if (IS_BLOODY(object)) {
                //if (mode!=11)
                    strcat(buf, " (bloody)");
              //  else
               //     strcat(buf, " &r(bloody)&0");
                found = TRUE;
            }
        }
        if (IS_SET(GET_OBJ_EXTRA2(object), ITEM2_POISONED)) {
           // if (mode==11)
                strcat(buf, " (poisoned)");
          //  else
           //     strcat(buf, " &y(poisoned)&0");
            found = TRUE;
        }
        if (num && mode==1)
        {
            sprintf(buf2,"%4.1f", (float) num*GET_OBJ_WEIGHT(object)/2.0);
            //sprintf(buf1,"%-66s%s%3d  %6s", buf, ((IS_SET(GET_OBJ_EXTRA(object), ITEM_EGO) && !IS_SET(GET_OBJ_EXTRA(object), ITEM_HIDDEN_EGO)) ? "    " : ""), num, buf2);
            //sprintf(buf1,"%-66s%s%3d  %6s", buf, "", num, buf2);
            sprintf(buf1,"%2d  %-66s%6s", num, buf, buf2);
        }
    }
    if (num && mode==1)
        sprintf(buf, "%s", buf1);

    if (mode==11)
        ch_printf(ch, "%-46s ", buf);
    else
    {
        if (cr)
            strcat(buf, "\r\n");
        page_string(ch->desc, buf, 1);
    }
}

/*void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
		           bool show)
{
  struct obj_data *i;
  bool found;

  found = FALSE;
  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}
*/

int get_carry_cond(struct char_data *ch)
{
    //int f;
    //f=(IS_CARRYING_W(ch)<<7)/CAN_CARRY_W(ch);
    //return (3.0*IS_CARRYING_W(ch)/CAN_CARRY_W(ch));
    int a=10*(IS_EQUIP_W(ch)+IS_CARRYING_W(ch))/CAN_CARRY_W(ch);
    if (a>8)
    	return 3;
    else if (a>6)
    	return 2;
    else if (a>3) 
    	return 1; 
    else    
    	return 0;
    
    
    //return (3.0*GET_TOTAL_WEIGHT(ch)/CAN_CARRY_W(ch));
}




void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode, bool show)
{
    struct obj_data *i, *j;
    bool found;
    int num;

    found = FALSE;
    for (i = list; i; i = i->next_content) {
        send_to_char(CCCYN(ch, C_NRM), ch);
        num = 0;
        for (j = list; j != i; j = j->next_content)
            if (j->item_number == NOTHING) {
                if (strcmp(j->short_description, i->short_description) == 0)
                    break;
            }
            else if (strcmp(j->short_description, i->short_description) == 0)
                break;
        if (j != i)
            continue;
        for (j = i; j; j = j->next_content)
            if (strcmp(j->short_description, i->short_description) == 0)
                num++;

        if (CAN_SEE_OBJ(ch, i)) {
            if (mode==-1) {
                send_to_char("\r\n", ch);
                mode=0;
            }
            if (!show  && num!=1) {
                sprintf(buf, "(%2i) ", num);
                send_to_char(buf, ch);

            }
            show_obj_to_char(i, ch, mode, 1, num);

            /*if (!show) {
            if (num==1)
            	send_to_char("\r\n",ch);	    	
            else if (num==2)
            	send_to_char(" &c(you see two of them)&0\r\n", ch);
            else if (num<=6) 	    	
            		send_to_char(" &c(there are few more around)&0\r\n", ch);	    
                   else if (num<=12) 	    
            		send_to_char(" &c(many others can be seen around)&0\r\n", ch);
            else if (num<=24) 	    		
            		send_to_char(" &c(a whole bunch is here)&0\r\n", ch);	    
            else 
            	send_to_char(" &c(dozens can be seen all over the place)&0\r\n", ch);
        } */
            found = TRUE;
        }
        send_to_char(CCNRM(ch, C_NRM), ch);
    }
    if (!found && show)
    {   if (IS_CARRYING_W(ch))
    		send_to_char(" You can't see a thing!\r\n", ch);
        else
        	send_to_char(" Nothing.\r\n", ch);
    }
    if (show && mode==1)
    {
        //sprintf(buf, "\r\n&yGold carried: &Y%d&y coins&0\r\n", GET_GOLD(ch));
       // send_to_char(buf, ch);       
        
	        
        //sprintf(buf, "&cTotal weight: &C%3.1f kg (%3d max)&c   Items:&C%2d (%2d max)&0\r\n", IS_CARRYING_W(ch)/2.0, CAN_CARRY_W(ch)/2, IS_CARRYING_N(ch), CAN_CARRY_N(ch)); 
        sprintf(buf, "\r\n&yItems:&Y%2ld/%2d   &yWeight: &Y%3.1f/%3d kg&0\r\n", IS_CARRYING_N(ch), CAN_CARRY_N(ch), IS_CARRYING_W(ch)/2.0, CAN_CARRY_W(ch)/2); 
        send_to_char(buf, ch);       
        //sprintf(buf, "&cTotal weight: &C%4.1f kg (%s)&0\r\n",(float)  IS_CARRYING_W(ch)/2.0,  carry_cond[MAX(0, MIN(3, ((int) get_carry_cond(ch))))]);
        
    }
}


void diag_char_to_char(struct char_data * i, struct char_data * ch)
{
    int percent;

    if (GET_MAX_HIT(i) > 0)
        percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
    else
        percent = -1;		/* How could MAX_HIT be < 1?? */

    strcpy(buf, PERS(i, ch));
    CAP(buf);

    if (percent >= 100)
        strcat(buf, " is in &Wexcellent&0 condition.\r\n");
    else if (percent >= 85)
        strcat(buf, " is in &Bvery good&0 condition.\r\n");
    else if (percent >= 70)
        strcat(buf, " is in &Bgood&0 condition.\r\n");
    else if (percent >= 55)
        strcat(buf, " looks not so well.\r\n");
    else if (percent >= 35)
        strcat(buf, " looks &Ypretty bad&0.\r\n");
    else if (percent >= 13)
        strcat(buf, " is in &Yawful&0 condition.\r\n");
    else if (percent >= 0)
        strcat(buf, " is &Ralmost dead&0.\r\n");
    else
        strcat(buf, " is &RDYING&0!\r\n");

    send_to_char(buf, ch);
}


void look_at_char(struct char_data * i, struct char_data * ch)
{
    int j, found;
    struct obj_data *tmp_obj;
    int prob;


    if (PLR_FLAGGED(i, PLR_TOAD)) {
        act("You see a toad that looks amazingly like $m.", FALSE, i, 0, ch, TO_VICT);
        return;
    }


    if (i->player.description)
        send_to_char(i->player.description, ch);
    else
        act("You see nothing unusual about $m.", FALSE, i, 0, ch, TO_VICT);



    found = FALSE;
    for (j = 0; !found && j < NUM_WEARS; j++)
        if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
            found = TRUE;

    if (found) {
        send_to_char("\r\n&G", ch);
        act("$n is using:&0", FALSE, i, 0, ch, TO_VICT);
        for (j = 0; j < NUM_WEARS; j++)
            if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
                send_to_char(where[j], ch);
                show_obj_to_char(GET_EQ(i, j), ch, 1, 1, 0);
            }
    }
    if (ch != i && GET_SKILL(ch, SKILL_PEEK)) {
        found = FALSE;
        send_to_char("\r\n&y", ch);
        act("You attempt to peek at $s inventory:&0", FALSE, i, 0, ch, TO_VICT);
        for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
            if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 100) < GET_SKILL(ch, SKILL_PEEK))) {
                show_obj_to_char(tmp_obj, ch, 1, 1, 0);
                found = TRUE;
            }
     
        }

        if (!found)
            send_to_char("You can't see anything.\r\n", ch);
        send_to_char("\r\n", ch);
        if (GET_GOLD(i))
        {
            sprintf(buf, "$e is carrying &c%d&0 gold.", GET_GOLD(i));
            act(buf, FALSE, i, 0, ch, TO_VICT);
        }
        else
            act("$e is carrying &cno&0 gold.", FALSE, i, 0, ch, TO_VICT);
        improve_skill(ch, SKILL_PEEK, 1);

    }
    send_to_char("\r\n", ch);



    if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_MONSTERLORE))
    {
        int k, j, pom;
        sprintf(buf, "You estimate %s as level %d %s.\r\n", GET_NAME(i), GET_LEVEL(i), pc_class_types[GET_CLASS_NUM(i)]);
        k=GET_HIT(i)/100;
        k*=100;
        if (k==0)
            k=50;
        if (GET_SKILL(ch, SKILL_MONSTERLORE)>85)
            sprintf(buf, "%sYou estimate %s hitpoints to %d.\r\n", buf, HSHR(i), k);
        if (IS_NPC(i))
        {
            k=GET_MOB_RNUM(i)/8;
            j=GET_MOB_RNUM(i) % 8;
            pom=GET_KILLED_MOB(ch, k);
            if (!(pom & (1<<j)))
                strcat(buf, "You have never killed this mob.\r\n");
        }
        send_to_char(buf, ch);
        improve_skill(ch, SKILL_MONSTERLORE, 1);
        send_to_char("\r\n", ch);
    }
    diag_char_to_char(i, ch);



}


void list_one_char(struct char_data * i, struct char_data * ch)
{
    bool showflag;
    char *positions[] = {
                            " is lying here, dead.",
                            " is lying here, mortally wounded.",
                            " is lying here, incapacitated.",
                            " is lying here, stunned.",
                            " is sleeping here.",
                            " is resting here.",
                            " is sitting here.",
                            "!FIGHTING!",
                            " is standing here."
                        };

    showflag = FALSE;
    strcpy(buf, "");
    if (IS_NPC(i) && (GET_QUESTMOB(ch) > 0) && (GET_MOB_VNUM(i) == GET_QUESTMOB(ch)))
        strcat(buf, "&W&f{QUEST}&0 ");
        
        if (IS_NPC(i) && !IS_IMMORT(ch))
    {
        int mexp;
        char buf2[20];
        mexp=(MOB_EXP_BASE*GET_RATIO(i)/100);
        mexp=LEVELDIFF(mexp, GET_LEVEL(ch), GET_LEVEL(i));
        mexp=(LEVELEXP(ch)-GET_EXP(ch))/mexp;
        if (mexp>=1) {
            sprintf(buf2, "<%d> ", mexp);
            //strcat(buf, buf2); //todo: this is how many mobs per level
        }
    }    
    if (IN_ARENA(i))
    {
        if (RED(i))
            strcat(buf, "{&RRED&0} ");
        else
            strcat(buf, "{&BBLUE&0} ");
    }
    /* determine aura(s) */
    if (AFF2_FLAGGED(i, AFF2_MIRRORIMAGE) && !IN_ARENA(i))
        strcat(buf, "(Multiple) ");

    if (IS_AFFECTED(i, AFF_SANCTUARY) && !IN_ARENA(i)) {
        strcat(buf, "(&wbright&y");
        showflag = TRUE;
    }
    if (IS_AFFECTED(ch, AFF_DETECT_ALIGN) && !IN_ARENA(ch)) {
        if (showflag)
            strcat(buf, "-");
        else
            strcat(buf, "(");
        if (IS_EVIL(i))
            strcat(buf, "&Rred&y");
        else if (IS_GOOD(i))
            strcat(buf, "&Bblue&y");
        else
            strcat (buf, "&Ggreen&y");
        showflag = TRUE;
    }
    if (showflag)
        strcat(buf, " aura) ");

    showflag = TRUE;


    if (MOB_FLAGGED(i, MOB_ETHEREAL))
        strcat(buf, "<ETHEREAL> ");
    
    if (GET_LEVEL(ch)-GET_LEVEL(i)>5)
    	strcat(buf, "&w");
    else if (GET_LEVEL(ch)-GET_LEVEL(i)>=0)
    	strcat(buf, "&y");
    else if (GET_LEVEL(ch)-GET_LEVEL(i)>=-5)
    	strcat(buf, "&Y");    	
    else 
    	strcat(buf, "&P");    		
    	
    if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
        strcat(buf, i->player.long_descr);
        showflag = FALSE;
    } else if (IS_NPC(i)) {
        strcat(buf, i->player.short_descr);
        CAP(buf);
    } else if (PLR_FLAGGED(i, PLR_TOAD))
        sprintf(buf, "An ugly toad that looks amazingly like %s", GET_NAME(i));

    else
    {

        //	if (!IN_ARENA(i))
        if (!IS_NPC(i) && i->player.long_descr && !FIGHTING(i))
        {
            if (GET_SKILL(i, SKILL_DISGUISE)>4*GET_INT(ch)) {
                strcat(buf, i->player.long_descr);
                showflag = FALSE;
                improve_skill(i, SKILL_DISGUISE, 1);
            }
            else
                sprintf(buf, "%s(disguised) %s %s", buf, i->player.name, GET_TITLE(i));
        }
        else
            sprintf(buf, "%s%s %s", buf, i->player.name, GET_TITLE(i));
        /*   else
           {
               if (RED(i))
                   sprintf(buf, "%sMember of the &RRED Team&0", buf);
               else
                   sprintf(buf, "%sMember of the &BBLUE Team&0", buf);
           }*/
    }
    if (showflag && GET_POS(i) != POS_FIGHTING) {
        if (GET_POS(i) != POS_STANDING) {
            if (!AFF2_FLAGGED(i, AFF2_MEDITATE))
                strcat(buf, positions[(int) GET_POS(i)]);
            else
                strcat(buf, " is here, meditating.");
        }
        else {
            if (IS_AFFECTED(i, AFF_FLYING))
                strcat(buf, " is flying in the air.");
            else if (IS_AFFECTED(i, AFF_WATERWALK))
                strcat(buf, " is floating in the air.");
            else
                strcat(buf, positions[(int) GET_POS(i)]);
        }
    } else {
        if (FIGHTING(i)) {
            strcat(buf, " is here, fighting ");
            if (FIGHTING(i) == ch)
                strcat(buf, "&YYOU&0!");
            else {
                if (i->in_room == FIGHTING(i)->in_room)
                    strcat(buf, PERS(FIGHTING(i), ch));
                else
                    strcat(buf, "someone who has already left");
                strcat(buf, "!");
            }
        }			/* else */
        /* NIL fighting pointer */
        /* strcat(buf, " is here struggling with thin air."); */
    }

    strcat(buf, "&0");
    
    if (showflag)
        strcat(buf, "\r\n");
        
    send_to_char(buf, ch);


    showflag = FALSE;
    strcpy(buf, "  ...$e is ");
    if (IN_ARENA(i))
    {
        if (HAS_RED(i))
            act("&0  ...$e is carrying &RRED FLAG!&0", TRUE, i, 0, ch, TO_VICT);
        if (HAS_BLUE(i))
            act("&0  ...$e is carrying &BBLUE FLAG!&0", TRUE, i, 0, ch, TO_VICT);
        return;
    }
    if (IS_AFFECTED(i, AFF_BLIND))
        act("&0&0  ...$e is groping around blinded!", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_BURNING))
        act("&0  ...$e is covered in flames!", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_FREEZING))
        act("&0  ...$e is freezing!", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_ACIDED))
        act("&0  ...$e is drenched in acid!", TRUE, i, 0, ch, TO_VICT);
    if (AFF3_FLAGGED(i, AFF3_WEB))
        act("&0  ...$e is covered by a sticky web!", TRUE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_PASSDOOR))
        act("&0  ...$s &cform is translucent.&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_BLINK))
        act("&0  ...$s image is &wdisplaced.&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_DEATHDANCE))
        act("&0  ...$e is &Rdeath dancing!&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_BERSERK))
        act("&0  ...$e is &CBERSERK!&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_HASTE))
        act("&0  ...$e is moving very quickly.", TRUE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_DEFLECTION))
        act("&0  ...$e is surrounded by a deflection shield.", TRUE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_FORCE_FIELD))
        act("&0  ...$e is charged with &belectricity!&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_FIRE_SHIELD))
        act("&0  ...$e is &ysurrounded by raging flames!&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF3_FLAGGED(i, AFF3_SOF))
        act("&0  ...$e is protected by &Pshield of faith!&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_SILVER))
        act("&0  ...$e is &Gprotected by a shimmering aura.&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_PETRIFY))
        act("&0  ...$e is &Bpetrified!&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_ULTRA))
        act("&0  ...$e &Ylooks like $e could take on anything!&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_PRISM))
        act("&0  ...$e &Yis surrounded with a prismatic sphere of light.&0", TRUE, i, 0, ch, TO_VICT);
    if (AFF3_FLAGGED(i, AFF3_PLAGUE))
        act("&0  ...$e has PLAGUE!&0", TRUE, i, 0, ch, TO_VICT);

    if (IS_AFFECTED(i, AFF_INVISIBLE)) {
        showflag = TRUE;
        strcat(buf, "invisible");
    }
    if (IS_AFFECTED(i, AFF_HIDE)) {
        if (showflag)
            strcat(buf, " and ");
        else
            showflag = TRUE;
        strcat(buf, "hidden");
    }
    if (!IS_NPC(i) && !i->desc) {
        if (showflag)
            strcat(buf, " and ");
        else
            showflag = TRUE;
        strcat(buf, "linkless");
    }
    if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_WRITING)) {
        if (showflag)
            strcat(buf, " and ");
        else
            showflag = TRUE;
        strcat(buf, "writing");
    }
    if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_EDITING)) {
        if (showflag)
            strcat(buf, " and ");
        else
            showflag = TRUE;
        strcat(buf, "editing");
    }
    if (showflag) {
        strcat(buf, ".");
        act(buf, TRUE, i, 0, ch, TO_VICT);
    }
}



int list_char_to_char(struct char_data * list, struct char_data * ch)
{
    struct char_data *i;
    int hv=0;

    for (i = list; i; i = i->next_in_room)
        if (ch != i) {
            if (CAN_SEE(ch, i) && (!MOB_FLAGGED(i, MOB_ETHEREAL) || GET_LEVEL(ch) > LVL_IMMORT))
            {
                if (!hv)
                    send_to_char("\r\n", ch);
                //send_to_char(CCYEL(ch, C_NRM), ch);
                list_one_char(i, ch);
                hv=1;
            }
            else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) && !MOB_FLAGGED(i, MOB_ETHEREAL))
            {
                if (!hv) send_to_char("\r\n", ch);
                //list_one_char(i, ch);
                hv=1;
                send_to_char("&0You see a &Rpair of glowing eyes&0 looking your way.\r\n", ch);
            }
        }
    return hv;
}



//extern int pulse;
extern struct time_info_data time_info;
void do_auto_dir(struct char_data * ch)
{
    int i,len, door;
    char exs[11][10];
    char nn[30], buf3[1000];
    strcpy(nn,GET_NAME(ch));
    *buf = '\0';
    sprintf(nn,"&c%c&0",LOWER(nn[0]));
    for (door = 0; door < NUM_OF_DIRS; door++)
        if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE && !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
        {
            if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
            {
                if (door<NORTHEAST)
                    sprintf(exs[door],"&G%c &0",UPPER(dirs[door][0]));
                else
                    switch(door)
                    {
                    case NORTHEAST:
                        sprintf(exs[door],"&GNE&0");break;
                    case NORTHWEST:
                        sprintf(exs[door],"&GNW&0");break;
                    case SOUTHEAST:
                        sprintf(exs[door],"&GSE&0");break;
                    case SOUTHWEST:
                        sprintf(exs[door],"&GSW&0");break;
                    default:
                        sprintf(exs[door],"&G??&0");break;
                    }
            }
            else
                sprintf(exs[door],"&r#&0");
        }
        else {
            if (EXIT(ch, door) && IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN) && IS_GOD(ch))
                sprintf (exs[door],"&BO&0");
            else
                sprintf (exs[door],"  ");
        }

    strcpy(buf3, "::::::::::::::::::::::::::");
    sprintf(buf, " &G%-50s&0         %s     %s     %s  %s\r\n",world[ch->in_room].name,exs[NORTHWEST],exs[NORTH],exs[NORTHEAST], exs[UP]);
    sprintf(buf2,"%s", sector_types[world[ch->in_room].sector_type]);
    len=strlen(buf2);
    for (i=0;i<15-len;i++)
        strcat(buf3,":");
    sprintf(buf, "%s::[%s]%s%s[%d:%02d]        %s<---(%s)---> %s\r\n",
            buf, buf2,buf3,(time_info.hours<10?":":""),time_info.hours,((pulse % (SECS_PER_MUD_HOUR*PASSES_PER_SEC))/PASSES_PER_SEC),
            exs[WEST],CAP(nn),exs[EAST]);

    sprintf(buf, "%s %50s         %s     %s     %s  %s\r\n",buf, "",exs[SOUTHWEST],exs[SOUTH],exs[SOUTHEAST], exs[DOWN]);

    send_to_char(buf, ch);
}
void do_auto_dir2(struct char_data * ch)
{
    int i,len, door;
    char exs[6][10];
    char nn[30], buf3[1000];
    strcpy(nn,GET_NAME(ch));
    *buf = '\0';
    sprintf(nn,"&c%c&0",(nn[0]));
    for (door = 0; door < NUM_OF_DIRS; door++)
        if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE)
        {
            if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
               sprintf (exs[door],"-");
            else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
                sprintf(exs[door],"&G%c&0",UPPER(dirs[door][0]));
            else
                sprintf(exs[door],"&r#&0");
        }
        else
            sprintf (exs[door],"-");

    strcpy(buf3, "::::::::::::::::::::::::::");
    sprintf(buf, " &G%-50s&0           %s    %s\r\n",world[ch->in_room].name,exs[4],exs[0]);
    sprintf(buf2,"%s", sector_types[world[ch->in_room].sector_type]);
    len=strlen(buf2);
    for (i=0;i<15-len;i++)
        strcat(buf3,":");
    sprintf(buf, "%s::[%s]%s%s[%d:%02d]        %s <---(%s)---> %s\r\n",
            buf, buf2,buf3,(time_info.hours<10?":":""),time_info.hours,((pulse % (SECS_PER_MUD_HOUR*PASSES_PER_SEC))/PASSES_PER_SEC),
            exs[3],nn,exs[1]);

    sprintf(buf, "%s %50s                %s    %s\r\n",buf, "",exs[2],exs[5]);
    /*
        for (door = 0; door < NUM_OF_DIRS; door++)
    	if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
    	    !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
    	    sprintf(buf, "%s%s ", buf, dirs[door]);;

        sprintf(buf2, "[ Exits: %s]\r\n",
    	    *buf ? buf : "None! ");
    */
    send_to_char(buf, ch);
}
ACMD(do_exits)
{
    int door;
    bool hidden;

    *buf = '\0';

    if (DEAD(ch))
        return;
    if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
        return;
    }
    for (door = 0; door < NUM_OF_DIRS; door++) {
        hidden = FALSE;
        if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE) {
            if (GET_LEVEL(ch) >= LVL_IMMORT) {
                sprintf(buf2, "  %-10s - [%5d] %-15s", dirs[door],
                        world[EXIT(ch, door)->to_room].number,
                        world[EXIT(ch, door)->to_room].name);
                if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
                    sprintf(buf2, "%-30s [CLOSED]", buf2);
                if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
                    strcat(buf2, "[HIDDEN]");
            } else {
                sprintf(buf2, "  %-10s - ", dirs[door]);
                if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
                    strcat(buf2, "&LToo dark to tell&0");
                else {
                    if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
                        strcat(buf2, "&p(hidden)&0 ");
                        hidden = TRUE;
                    }
                    if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)){
                        if (EXIT(ch, door)->keyword){
                            strcat(buf2,EXIT(ch, door)->keyword);
                            strcat(buf2, " ");
                        }
                        strcat(buf2, "&y(closed)&0");
                    }
                    else
                        strcat(buf2, world[EXIT(ch, door)->to_room].name);
                }
            }
            if (hidden)// && !CAN_SEE_HIDDEN(ch))
                strcpy(buf2, "");
            else
                strcat(buf2, "\r\n");
            strcat(buf2, "&0");
            strcat(buf, CAP(buf2));
        }
    }
    //    send_to_char("Obvious exits:\r\n", ch);

    if (*buf)
        send_to_char(buf, ch);
    //    else
    //	send_to_char(" None.\r\n", ch);
}
void show_surrounding(struct char_data *ch);

void print_room_affections(struct char_data *ch)
{       
	if (!ch || DEAD(ch))
		return;
	if (ROOM_AFFECTED(ch->in_room, RAFF_ILLUMINATION))
		send_to_char("&wUnearthly light illuminates this place.&0\r\n", ch);
	if (ROOM_AFFECTED(ch->in_room, RAFF_PEACEFUL))
		send_to_char("&BBluish myst covers the place, making you feel safe.&0\r\n", ch);		
	
	
	
}
ACMD(do_newmap);
void look_at_room(struct char_data * ch, int ignore_brief)
{
    int b=0;

    if (ch->in_room==NOWHERE)
    {
        log("SYSERR: look_at_room, char in NOWHERE");
        return;
    }
    if (!AWAKE(ch))
        return;
    if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
        send_to_char("&wIt is pitch black...&0\r\n", ch);
        return;
    } else if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_char("You see nothing but infinite darkness...\r\n", ch);
        return;
    }

    if (PRF2_FLAGGED(ch, PRF2_AUTOMAP))
        //show_surrounding(ch);
        do_newmap(ch, "9", 0, 0);
    send_to_char("&G\r\n", ch);
    if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
        sprintbit((long) ROOM_FLAGS(ch->in_room), room_bits, buf);
        sprintf(buf2, "[%5d] %s [%s: %s]\r\n", world[ch->in_room].number,
                world[ch->in_room].name, sector_types[world[ch->in_room].sector_type], buf);
        send_to_char(buf2, ch);
        /*	if (world[ch->in_room].tele != NULL)
        	    send_to_char(" Teleport", ch);*/
    };
    //     else
    //	send_to_char(world[ch->in_room].name, ch);
    //    send_to_char("\r\n", ch);

    /* autodir */
    if (PRF_FLAGGED(ch, PRF_AUTODIR) && !PRF_FLAGGED(ch, PRF_ROOMFLAGS))
    {
        do_auto_dir(ch);
        //  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief)
        //    send_to_char("\r\n", ch);
    }
    else if (!PRF_FLAGGED(ch, PRF_ROOMFLAGS)){
        send_to_char(world[ch->in_room].name, ch);
        send_to_char("\r\n", ch);
    };

    send_to_char(CCNRM(ch, C_NRM), ch);
    if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
            ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    {
        send_to_char(world[ch->in_room].description, ch);

        if (ROOM_AFFECTIONS(ch->in_room))
        	print_room_affections(ch);
        if (RM_BLOOD(ch->in_room) > 0 && (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief))
            act(blood_messages[RM_BLOOD(ch->in_room)], FALSE, ch, 0, 0, TO_CHAR);
        
    }

    /* autoexits */
    if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
        do_exits(ch, 0, 0, 0);
    //   else
    //	send_to_char("\r\n", ch);

    /* now list characters & objects */

    if (!list_char_to_char(world[ch->in_room].people, ch))
        b=-1;


    list_obj_to_char(world[ch->in_room].contents, ch, b, FALSE);
    send_to_char(CCNRM(ch, C_NRM), ch);
    if (PRF2_FLAGGED(ch, PRF2_AUTOSCAN) && SECT(ch->in_room) != SECT_CITY)
        autoscan(ch);
}

void look_in_direction(struct char_data * ch, int dir)
{
    room_num temp_room;
    int distance = 0, counter = 0;
    char buf2[127];
    bool show = TRUE;

    distance = 1;

    if (33 <= GET_SKILL(ch, SKILL_ENH_SIGHT)) {
        distance++;
        if (66 <= GET_SKILL(ch, SKILL_ENH_SIGHT)) {
            distance++;
        }
    }
    if (AFF2_FLAGGED(ch, AFF2_FARSEE)) {
        distance = 4;
    }
    distance = MIN(distance, 4);

    if (!EXIT(ch, dir))
        send_to_char("Nothing special there...", ch);
    else {
        if (EXIT(ch, dir)->general_description)
            send_to_char(EXIT(ch, dir)->general_description, ch);
        else if (IS_SET(EXITN((ch)->in_room, dir)->exit_info, EX_HIDDEN))
            send_to_char("It looks like a hidden exit.\r\n", ch);
        else
            send_to_char("You see nothing special.\r\n", ch);

        temp_room = (ch)->in_room;

        do {
            show = TRUE;
            if (IS_SET(EXITN(temp_room, dir)->exit_info, EX_HIDDEN))
                return;
            if (IS_SET(EXITN(temp_room, dir)->exit_info, EX_CLOSED) && EXITN(temp_room, dir)->keyword) {
                sprintf(buf, "The %s is closed.\r\n", fname(EXITN(temp_room, dir)->keyword));
                send_to_char(buf, ch);
                return;
            } else if (show && IS_SET(EXITN(temp_room, dir)->exit_info, EX_ISDOOR) && EXITN(temp_room, dir)->keyword) {
                sprintf(buf, "The %s is open.\r\n", fname(EXITN(temp_room, dir)->keyword));
                send_to_char(buf, ch);
            }
            if (EXITN(temp_room, dir)->to_room == NOWHERE) {
                sprintf(buf, "Incorrect room number found in #%d -> #%d", world[temp_room].number, EXITN(temp_room, dir)->to_room_vnum);
                mudlog(buf, BRF, LVL_IMMORT, TRUE);
                send_to_char("There's something wrong with this room, please report to an IMP.", ch);

                return;
            }
            temp_room = EXITN(temp_room, dir)->to_room;
            counter++;
            if (IS_MURKY(temp_room) && !IS_IMMORT(ch)) {
                send_to_char("Something prevents you from seeing further.\r\n", ch);
                return;
            }
            if (IS_DARK(temp_room) && !CAN_SEE_IN_DARK(ch)) {
                strcpy(buf2, "You see nothing but darkness.");
                show = FALSE;
            } else
                strcpy(buf2, world[temp_room].name);
            sprintf(buf, "\r\n Dist: %d   %-15s", counter, buf2);
            send_to_char(buf, ch);
            if (show && temp_room) {
                send_to_char(CCCYN(ch, C_NRM), ch);
                list_char_to_char(world[temp_room].people, ch);
                send_to_char(CCNRM(ch, C_NRM), ch);
            }
        }
        while (EXITN(temp_room, dir) && (counter < distance));
    }
    send_to_char("\r\n", ch);
}


void look_in_obj(struct char_data * ch, char *arg)
{
    struct obj_data *obj = NULL;
    struct char_data *dummy = NULL;
    int amt, bits;

    if (!*arg)
        send_to_char("Look in what?\r\n", ch);
    else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
                                   FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
        sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
    } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
               (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
               (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
        send_to_char("There's nothing inside that!\r\n", ch);
    else {
        if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
            if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
                send_to_char("It is closed.\r\n", ch);
            else {
                send_to_char(fname(obj->name), ch);
                switch (bits) {
                case FIND_OBJ_INV:
                    send_to_char(" (carried): \r\n", ch);
                    break;
                case FIND_OBJ_ROOM:
                    send_to_char(" (here): \r\n", ch);
                    break;
                case FIND_OBJ_EQUIP:
                    send_to_char(" (used): \r\n", ch);
                    break;
                }

                list_obj_to_char(obj->contains, ch, 2, FALSE);
            }
        } else {		/* item must be a fountain or drink
            			   container */
            if (GET_OBJ_VAL(obj, 1) <= 0)
                send_to_char("It is empty.\r\n", ch);
            else {
                amt = ((GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0));
                sprintf(buf, "It's %sfull of a %s liquid.\r\n", fullness[amt],
                        color_liquid[GET_OBJ_VAL(obj, 2)]);
                send_to_char(buf, ch);
            }
        }
    }
}


char *find_exdesc(char *word, struct extra_descr_data * list)
{
    struct extra_descr_data *i;

    for (i = list; i; i = i->next)
        if (isname(word, i->keyword))
            return (i->description);

    return NULL;
}



#define NUM_DAYS 35
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define SHOW_WIDTH MAP_WIDTH/2
#define MAP_HEIGHT 9
/* Should be the string length and number of the constants below.*/
/*const char * star_map[] =
{
"   W.N     ' .     :. M,N     :  y:.,N    `  ,       B,N      .      .  ",
" W. :.N .      G,N  :M.: .N  :` y.N    .      :     B:   .N       :     ",
"    W:N    G.N:       M:.,N:.:   y`N      ,    c.N           .:    `    ",
"   W.`:N       '. G.N  `  : ::.      y.N      c'N      B.N R., ,N       ",
" W:'  `:N .  G. N    `  :    .y.N:.          ,     B.N      :  R:   . .N",
":' '.   .    G:.N      .'   '   :::.  ,  c.N   :c.N    `        R`.N    ",
"      :       `        `        :. ::. :     '  :        ,   , R.`:N    ",
"  ,       G:.N              `y.N :. ::.c`N      c`.N   '        `      .",
"     ..        G.:N :           .:   c.N:.    .              .          "
};*/
const char * star_map[] =
    {
        "   W.N     ' .     :. M,N     :  y:.,N    `  ,       B,N      .      .  ",
        " W. :.N .      G,N  :M.: .N  :` y.N    .      :     B:   .N       :     ",
        "    W:N    G.N:       M:.,N:.:   y`N      ,    c.N           .:    `    ",
        "   W.`:N       '. G.N  `  : ::.      y.N      c'N      B.N R., ,N       ",
        " W:'  `:N .  G. N    `  :    .y.N:.          ,     B.N      :  R:   . .N",
        ":' '.   .    G:.N      .'   '   :::.  ,  c.N   :c.N    `        R`.N    ",
        "      :       `  .     `        :. ::. :     '  :        ,   , R.`:N    ",
        "  ,       G:.N              `y.N :. ::.c`N      c`.N   '        `      .",
        "     ..        G.:N             .:   c.N:.    .              .          "
    };

const char * cloud_map[] =
    {
        "       _ -                                                              ",
        "      -   '--_                                          _---_           ",
        "      -       --_                                   __--     --         ",
        "       - -  -- --                                 _'           ---__    ",
        "                         =_                     /'                  -   ",
        "                     _--'  '----_               ---__               /   ",
        "                    /          -/                    _             /    ",
        "                  -=     '----/'                      ---__  ___---     ",
        "                  ' '----                                  --           "
    };



/***************************CONSTELLATIONS*******************************
  Lupus     Gigas      Pyx      Enigma   Centaurus    Terken    Raptus
   The       The       The       The       The         The       The  
White Wolf  Giant     Pixie     Sphinx    Centaur      Drow     Raptor
*************************************************************************/	
const char * sun_map[] =
    {
        "\\'|'/",
        "- O -",
        "/.|.\\"
    };
const char * moon_map[] =
    {
        " @@@ ",
        "@@@@@",
        " @@@ "
    };

void look_sky ( CHAR_DATA * ch )
{
    static char buf[MAX_STRING_LENGTH];
    static char buf2[4];
    int starpos, sunpos, moonpos, moonphase, i, linenum;

    send_to_char("You gaze up towards the heavens and see:\n\r",ch);

    sunpos  = (MAP_WIDTH * (24 - time_info.hours) / 24);
    moonpos = (sunpos + time_info.day * MAP_WIDTH / NUM_DAYS) % MAP_WIDTH;
    if ((moonphase = ((((MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) +
                       (MAP_WIDTH/16)) * 8 ) / MAP_WIDTH)
            > 4) moonphase -= 8;
    starpos = (sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS) % MAP_WIDTH;
    /* The left end of the star_map will be straight overhead at midnight during
       month 0 */

    for ( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
    {
        if ((time_info.hours >= 5 && time_info.hours <= 20) &&
                (linenum < 3 || linenum >= 6))
            continue;
        sprintf(buf,"&W|&0");
        for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)
        {

            /*if (weather_info.sky && cloud_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH]!=' ')
               	{
               		sprintf(buf2, "%c", cloud_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH]);
               		strcat(buf, buf2);
               	}        
               else      	*/
            /* plot moon on top of anything else...unless new moon & no eclipse */
            if ((time_info.hours >= 5 && time_info.hours <= 20)  /* daytime? */
                    && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
                    && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
                    && ((sunpos == moonpos && time_info.hours == 12) || moonphase != 0  ) /*no eclipse*/
                    && (moon_map[linenum-3][i+2-moonpos] == '@'))
            {
                if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
                        (moonphase > 0 && i + 2 - moonpos <= moonphase))
                    strcat(buf,"&W@");
                else
                    strcat(buf," ");
            }
            else
                if ((linenum >= 3) && (linenum < 6) && /* nighttime */
                        (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
                        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
                        && (moon_map[linenum-3][i+2-moonpos] == '@'))
                {
                    if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
                            (moonphase > 0 && i + 2 - moonpos <= moonphase))
                        strcat(buf,"&W@");
                    else
                        strcat(buf," ");
                }
                else /* plot sun or stars */
                {
                    if (time_info.hours>=5 && time_info.hours<=20) /* daytime */
                    {
                        if ( i >= sunpos - 2 && i <= sunpos + 2 )
                        {
                            sprintf(buf2,"&Y%c",sun_map[linenum-3][i+2-sunpos]);
                            strcat(buf,buf2);
                        }
                        else
                            strcat(buf," ");
                    }
                    else
                    {
                        switch (star_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH])
                        {
                        default     : strcat(buf," ");    break;
                        case '.'    : strcat(buf,".");    break;
                        case ','    : strcat(buf,",");    break;
                        case ':'    : strcat(buf,":");    break;
                        case '`'    : strcat(buf,"`");    break;
                        case 'R'    : strcat(buf,"&R ");  break;
                        case 'G'    : strcat(buf,"&G ");  break;
                        case 'B'    : strcat(buf,"&B ");  break;
                        case 'W'    : strcat(buf,"&W ");  break;
                        case 'M'    : strcat(buf,"&M ");  break;
                        case 'N'    : strcat(buf,"&w ");  break;
                        case 'y'    : strcat(buf,"&y ");  break;
                        case 'c'    : strcat(buf,"&c ");  break;
                        }
                    }
                }
        }
        strcat(buf,"&W|&0\n\r");
        send_to_char(buf,ch);
    }
}





/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void look_at_target(struct char_data * ch, char *arg)
{
    int bits, found = 0, j;
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL, *found_obj = NULL;
    char *desc;

    if (!*arg) {
        send_to_char("Look at what?\r\n", ch);
        return;
    }
    bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
                        FIND_CHAR_ROOM, ch, &found_char, &found_obj);

    /* Is the target a character? */
    if (found_char != NULL) {
        look_at_char(found_char, ch);
        if (ch != found_char) {
            if (CAN_SEE(found_char, ch))
                act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
            act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
        }
        return;
    }
    /* Does the argument match an extra desc in the room? */
    if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL) {
        send_to_char(desc,ch);
        return;
    }
     /* Does the argument match an extra desc in the char's inventory? */
    for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
        if (CAN_SEE_OBJ(ch, obj))
            if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
                send_to_char(desc,ch);
                found = 1;
            }
    }
   
    /* Does the argument match an extra desc in the char's equipment? */
    for (j = 0; j < NUM_WEARS && !found; j++)
        if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
            if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL) {
                send_to_char(desc,ch);
                found = 1;
            }
  

    /* Does the argument match an extra desc of an object in the room? */
    for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
        if (CAN_SEE_OBJ(ch, obj))
            if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
                send_to_char(desc,ch);
                found = 1;
            }
    if (bits) {			/* If an object was found back in
           generic_find */
        if (!found)
            show_obj_to_char(found_obj, ch, 5, 1, 0);	/* Show no-description */
        else
            show_obj_to_char(found_obj, ch, 6, 1, 0);	/* Find hum, glow etc */
        oprog_examine_trigger( ch, found_obj );
    } else if (!found)
        send_to_char("You do not see that here.\r\n", ch);
}


ACMD(do_look)
{
    static char arg2[MAX_INPUT_LENGTH];
    int look_type;

    if (!ch->desc)
        return;

    if (GET_POS(ch) < POS_SLEEPING)
        send_to_char("You can't see anything but stars!\r\n", ch);
    else if (IS_AFFECTED(ch, AFF_BLIND))
        send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
        send_to_char("&wIt is pitch black...&0\r\n", ch);
        list_char_to_char(world[ch->in_room].people, ch);	/* glowing red eyes */
    } else {
        half_chop(argument, arg, arg2);

        if (subcmd == SCMD_READ) {
            if (!*arg)
                send_to_char("Read what?\r\n", ch);
            else
                look_at_target(ch, arg);
            return;
        }
        if (!*arg)		/* "look" alone, without an argument at
            			   all */
            look_at_room(ch, 1);
        else if (is_abbrev(arg, "in"))
            look_in_obj(ch, arg2);
        /* did the char type 'look <direction>?' */
        else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
            look_in_direction(ch, look_type);
        else if (is_abbrev(arg, "at"))
            look_at_target(ch, arg2);
        else if (!strcmp(arg, "sky"))
        {
            if ( !OUTSIDE(ch) )
            {
                send_to_char( "You can't see the sky indoors.\n\r", ch );
                return;
            }
            else
            {
                look_sky(ch);
                return;
            }
        }
        else
            look_at_target(ch, arg);
    }
}



ACMD(do_examine)
{
    int bits;
    struct char_data *tmp_char;
    struct obj_data *tmp_object, *obj;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Examine what?\r\n", ch);
        return;
    }
    look_at_target(ch, arg);

    if (DEAD(ch))
        return;

    bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
                        FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

    if (tmp_object) {
        if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
                (GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
                (GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
            send_to_char("&GWhen you look inside, you see:&0\r\n", ch);
            look_in_obj(ch, arg);
        }
    } else {
        if ((obj = get_object_in_equip_vis(ch, arg, ch->equipment,(int *) &tmp_char))) {
            send_to_char("&GWhen you look inside, you see:&0\r\n", ch);
            look_in_obj(ch, arg);
        } else if ((obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
            send_to_char("&GWhen you look inside, you see:&0\r\n", ch);
            look_in_obj(ch, arg);
        }
    }
}



ACMD(do_gold)
{
    if (GET_GOLD(ch) == 0)
        send_to_char("You're &wbroke&0!\r\n", ch);
    else if (GET_GOLD(ch) == 1)
        send_to_char("You have one miserable little coin.\r\n", ch);
    else {
        sprintf(buf, "You have &Y%d&0 coins.\r\n", GET_GOLD(ch));
        send_to_char(buf, ch);
    }
}


ACMD(do_victim)
{
    int i;
    char colorbuf[500];
    i = 50;

    STATUS_COLOR(i, colorbuf, ch, C_SPR);
    sprintf(buf, "Monster kills: &B%4d&0            Killed by monster: &R%4d&0\r\n",
            ch->player_specials->saved.killed_mob,
            ch->player_specials->saved.killed_by_mob);

    sprintf(buf, "%sPlayer  kills: &B%4d&0            Killed by players: &R%4d&0 \r\n",
            buf,
            ch->player_specials->saved.killed_player,
            ch->player_specials->saved.killed_by_player);
    sprintf(buf, "%sYour Player Kills score is &G%ld&0.\r\n", buf, ch->player_specials->saved.pkscore);
    sprintf(buf, "%sYour Mob Kills score is &G%ld&0.\r\n", buf, ch->player_specials->saved.mkscore);
    send_to_char(buf, ch);
}

ACMD(do_worthy)
{
    struct time_info_data real_time_passed(time_t t2, time_t t1);
    int i;
    char colorbuf[500];

    i = 100 * GET_HIT(ch) / GET_MAX_HIT(ch);
    STATUS_COLOR(i, colorbuf, ch, C_SPR);
    sprintf(buf,
            "You have %s%d&0/&B%d&0 hit, ",
            colorbuf, GET_HIT(ch), GET_MAX_HIT(ch));

    i = 100 * GET_MANA(ch) / GET_MAX_MANA(ch);
    STATUS_COLOR(i, colorbuf, ch, C_SPR);
    sprintf(buf, "%s%s%d&0/&B%d&0 mana ",
            buf, colorbuf, GET_MANA(ch), GET_MAX_MANA(ch));

    i = 100 * GET_MOVE(ch) / GET_MAX_MOVE(ch);
    STATUS_COLOR(i, colorbuf, ch, C_SPR);
    sprintf(buf, "%sand %s%d&0/&B%d&0 movement points.\r\n",
            buf, colorbuf, GET_MOVE(ch), GET_MAX_MOVE(ch));
    send_to_char(buf, ch);

    sprintf(buf, "You have &G%d&0 coins and ", GET_GOLD(ch));
    /*if (!GET_PRACTICES(ch))
    sprintf(buf, "%sno practice sessions remaining.\r\n", buf);
    else
    sprintf(buf, "%s&G%d&0 practice session%s remaining.\r\n", buf, GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));*/
    sprintf(buf, "%s&G%d&0 adventuring points.\r\n", buf, GET_QUESTPOINTS(ch));
    /*if IS_QUESTOR
    (ch)
     sprintf(buf, "%s You have &W%d&0 hours remaining to complete your quest.\r\n", buf, ch->player_specials->saved.questcdown);
    else {
    if (ch->player_specials->saved.questnext > 0)
     sprintf(buf, "%s You may take a quest in &W%d&0 hours.\r\n", buf, ch->player_specials->saved.questnext);
    else
     sprintf(buf, "%s You are allowed to take a quest.\r\n", buf);
}*/
    if (!IS_NPC(ch)) {
        if (GET_LEVEL(ch) < (LVL_IMMORT - 1))
            sprintf(buf, "%sYou need &C%d&0 exp to reach your next level.\r\n", buf,
                    //			total_exp(GET_LEVEL(ch))-GET_EXP(ch));
                    LEVELEXP(ch)-GET_EXP(ch));
    }
    send_to_char(buf, ch);
}
char tsbuf[6][20];

char *textstat(int r, int t, int m, int num)
{
    *tsbuf[num]=0;
    if (t==m)
        strcpy(tsbuf[num], "&CMAX&0");
    else if (t>r)
        sprintf(tsbuf[num], "&c+%2d&0", t-r);
    else if (t==r)
        strcpy(tsbuf[num], "&0Normal&0");
    else
        sprintf(tsbuf[num], "&c-%2d&0", r-t);
    return (tsbuf[num]);
}


char *armor_desc (int n)
{
    if (n<20)
        strcpy(buf1, "poor");
    else if (n<40)
        strcpy(buf1, "fair");
    else if (n<60)
        strcpy(buf1, "good");
    else if (n<80)
        strcpy(buf1, "very good");
    else if (n<100)
        strcpy(buf1, "excellent");
    else
        strcpy(buf1, "dragon");
    return (buf1);
}

ACMD(do_score)
{
    struct time_info_data playing_time;
    struct time_info_data real_time_passed(time_t t2, time_t t1);
    int i;
    char *styles[3]=
        {"normal","evasive","aggresive"};
    int clan_num;
    char colorbuf[500], clanb[50]="Newbie";
    char regenb[10], vitb[20];
    char buf1[40];


    send_to_char("*****************************************************************************\r\n**********************************&G S C O R E &0********************************\r\n*****************************************************************************\r\n", ch);

    sprintf(buf, "&c%s %s, %d year old %s %s, level %d.&0",
            GET_NAME(ch), GET_TITLE(ch), GET_AGE(ch), pc_race_types[(int) GET_RACE(ch)], pc_class_types[GET_CLASS_NUM(ch)], GET_LEVEL(ch));//, clan[GET_CLAN(ch)].name);

    if ((age(ch).month == 0) && (age(ch).day == 0))
        strcat(buf, "\r\nIt's your birthday!\r\r\n\n");
    else
        strcat(buf, "\r\r\n\n");
    /*
       sprintf(regenb,"%.1f", hit_gain(ch));
       //sprintf(regenb,"%-5s", regenb);
       sprintf(vitb,"%d (%d)", (int) GET_HIT(ch),(int)  GET_MAX_HIT(ch));
       //sprintf(vitb,"%-11s", vitb);
       
      //  i = 100 * GET_HIT(ch) / GET_MAX_HIT(ch);
    //    STATUS_COLOR(i, colorbuf, ch, C_CMP);
        sprintf(buf, "%sVitality: &c%-11s&0    Armor Bonus: &c%-4d&0              Hitroll: &c%2d&0\r\n",
          buf, vitb, (int) GET_AC(ch)/20, ch->points.hitroll);

        i = 100 * GET_MANA(ch) / GET_MAX_MANA(ch);
        STATUS_COLOR(i, colorbuf, ch, C_CMP);                           
       sprintf(buf, "%s   Regen: &c%-5s&0           Experience: &c%-15d&0   Damroll: &c%2d&0\r\n\r\n",
          buf, regenb, GET_EXP(ch), ch->points.damroll);
        i = 100 * GET_MOVE(ch) / GET_MAX_MOVE(ch);
        
        STATUS_COLOR(i, colorbuf, ch, C_CMP);         
       
        send_to_char(buf, ch);
    */

    
    i = 100 * GET_HIT(ch) / GET_MAX_HIT(ch);
    STATUS_COLOR(i, colorbuf, ch, C_CMP);
    //sprintf(buf, "%sHit : %s%4d&0/&G%4d&0 (&c%c%-3d&0)      Exp to level: &c%-15d&0Hitroll: &c%2d&0\r\n",
    sprintf(buf, "%sHit   : %s%4d&0/&G%4d&0 (&G%c%-3d&0)      Exp to level: &G%-3.2f%%&0           Hitroll: &G%2d&0\r\n",
            //buf, colorbuf, GET_HIT(ch), GET_MAX_HIT(ch), (hit_gain(ch)>0? '+' : '-'), abs(hit_gain(ch)),  total_exp(GET_LEVEL(ch))-GET_EXP(ch), ch->points.hitroll/*,((str_app[STRENGTH_APPLY_INDEX(ch)].tohit + dex_app[GET_DEX(ch)].reaction)>=0 ? "+": ""), str_app[STRENGTH_APPLY_INDEX(ch)].tohit + dex_app[GET_DEX(ch)].reaction*/);
            buf, colorbuf, GET_HIT(ch), GET_MAX_HIT(ch), (hit_gain(ch)>0? '+' : '-'), abs(hit_gain(ch)),  100.0*GET_EXP(ch)/LEVELEXP(ch), ch->points.hitroll/*,((str_app[STRENGTH_APPLY_INDEX(ch)].tohit + dex_app[GET_DEX(ch)].reaction)>=0 ? "+": ""), str_app[STRENGTH_APPLY_INDEX(ch)].tohit + dex_app[GET_DEX(ch)].reaction*/);

    i = 100 * GET_MANA(ch) / (GET_MAX_MANA(ch)?GET_MAX_MANA(ch):1);
    STATUS_COLOR(i, colorbuf, ch, C_CMP);
    sprintf(buf, "%sEnergy: %s%4d&0/&G%4d&0 (&G%c%-3d&0)  Adventure points: &G%-7d&0          Damroll: &G%2d&0\r\n",
            buf, colorbuf, GET_MANA(ch), GET_MAX_MANA(ch),(mana_gain(ch)>=0? '+' : '-'), abs(mana_gain(ch)),GET_QUESTPOINTS(ch), ch->points.damroll/*, ((str_app[STRENGTH_APPLY_INDEX(ch)].todam>=0) ? "+":""),str_app[STRENGTH_APPLY_INDEX(ch)].todam*/);

    i = 100 * GET_MOVE(ch) / (GET_MAX_MOVE(ch)?GET_MAX_MOVE(ch):1);

    STATUS_COLOR(i, colorbuf, ch, C_CMP);  

    sprintf(buf1, "%d+%d", GET_GOLD(ch), GET_BANK_GOLD(ch));
    sprintf(buf, "%sMove  : %s%4d&0/&G%4d&0 (&G%c%-3d&0)         Gold+Bank: &G%-12s&0 Armor class:&G%3d/10&0\r\n",
            buf, colorbuf, GET_MOVE(ch), GET_MAX_MOVE(ch), (move_gain(ch)>0? '+' : '-'), abs(move_gain(ch)), buf1,GET_AC(ch));
    //send_to_char(buf, ch);    
    
    sprintf(buf, "%s                                     Faith: &G%-3d%%&0        Magic resist:&G%3d&0\r\n\r\n",
            buf, 100*(GET_FAITH(ch)+400)/800, GET_MAGAC(ch));            
    send_to_char(buf, ch);


    if (!IS_NPC(ch))
        /*sprintf(buf,
        "Strength    : %-10s    Intelligence: %-10s      Dexterity: %-10s\r\nConstitution: %-10s      Will Power: %-10s       Charisma: %-10s\r\n",
        textstat(GET_STRR(ch), GET_STR(ch), race_app[GET_RACE(ch)].str, 0),
        textstat(GET_INTR(ch), GET_INT(ch), race_app[GET_RACE(ch)].intel, 1),
        textstat(GET_DEXR(ch), GET_DEX(ch), race_app[GET_RACE(ch)].dex, 2),
        textstat(GET_CONR(ch), GET_CON(ch), race_app[GET_RACE(ch)].con, 3),
        textstat(GET_WISR(ch), GET_WIS(ch), race_app[GET_RACE(ch)].wis, 4),
        textstat(GET_CHAR(ch), GET_CHA(ch), race_app[GET_RACE(ch)].cha, 5));*/
        sprintf(buf,
                "Strength    : &G%2d&0 (%2d)    Intelligence: &G%2d&0 (%2d)    Dexterity: &G%2d&0 (%2d)\r\nConstitution: &G%2d&0 (%2d)      Will Power: &G%2d&0 (%2d)     Charisma: &G%2d&0 (%2d)\r\n",
                GET_STR(ch), race_app[GET_RACE(ch)].str,
                GET_INT(ch), race_app[GET_RACE(ch)].intel,
                GET_DEX(ch), race_app[GET_RACE(ch)].dex,
                GET_CON(ch), race_app[GET_RACE(ch)].con,
                GET_WIS(ch), race_app[GET_RACE(ch)].wis,
                GET_CHA(ch), race_app[GET_RACE(ch)].cha);

    else
        sprintf(buf,
                "Strength    : &c%2d&0         Intelligence: &c%2d&0           Dexterity: &c%2d&0\r\nConstitution: &c%2d&0               Wisdom: &c%2d&0            Charisma: &c%2d&0\r\n",
                GET_STR(ch), GET_INT(ch), GET_DEX(ch), GET_CON(ch),
                GET_WIS(ch), GET_CHA(ch));
    



    sprintf(buf, "%s*****************************************************************************\r\n", buf);
    //sprintf(buf, "%sYou are known as &c%s %s&0\r\n", buf, GET_NAME(ch), GET_TITLE(ch));

    /*  if (GET_LEVEL(ch) >= 10)
    	sprintf(buf,"%s (%d)\r\n", buf, GET_ALIGNMENT(ch));
    */

    sprintf(buf, "%sYou are %d cm tall and weigh %d kg.\r\n", buf, GET_HEIGHT(ch), ( GET_WEIGHT(ch))/2);


    if (!IS_NPC(ch)) {
        /*	sprintf(buf, "%sYou need &c%d&0 exp to reach your next level.\r\n", buf,
        	total_exp(GET_LEVEL(ch))-GET_EXP(ch));*/

        /*if (!GET_PRACTICES(ch))
            sprintf(buf, "%sYou have no practice sessions and ", buf);
        else
            sprintf(buf, "%sYou have &c%d&0 practice session%s and ", buf, GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));*/
        //sprintf(buf, "%sYou have gained &c%d&0 adventuring points so far.\r\n", buf, GET_QUESTPOINTS(ch));

        //sprintf(buf, "%sYou carry &c%d&0 coins, and have &c%d&0 gold stored in bank.\r\n", buf, GET_GOLD(ch), GET_BANK_GOLD(ch));
        playing_time = real_time_passed((time(0) - ch->player.time.logon) +
                                        ch->player.time.played, 0);
        sprintf(buf, "%sYou have been playing for %d days and %d hours.\r\n",
                buf, playing_time.day, playing_time.hours);
    }
    //	strcat(buf, get_mood(ch));
    send_to_char(buf, ch);
    
    if (GET_DEITY(ch))
    {                                                                                   
    if (GET_FAITH(ch)<-350)
    	ch_printf(ch, "Your faith in %s is catastrophic!&0\r\n", DEITY_NAME(ch));
    else if (GET_FAITH(ch)<-250)
    	ch_printf(ch, "Your faith is not worthy of %s's name!&0\r\n", DEITY_NAME(ch));
    else if (GET_FAITH(ch)<-100)
    	ch_printf(ch, "Your faith in %s is questionable.&0\r\n", DEITY_NAME(ch));
    else if (GET_FAITH(ch)<-50)
    	ch_printf(ch, "Your faith in %s is low.&0\r\n", DEITY_NAME(ch));    	
    else if (GET_FAITH(ch)>350)
    	ch_printf(ch, "Your are a true believer of %s.&0\r\n", DEITY_NAME(ch));    
    else if (GET_FAITH(ch)>250)
    	ch_printf(ch, "Your faith in %s is exceptional!&0\r\n", DEITY_NAME(ch));
    else if (GET_FAITH(ch)>100)
    	ch_printf(ch, "Your faith in %s is strong.&0\r\n", DEITY_NAME(ch));
    else if (GET_FAITH(ch)>50)
    	ch_printf(ch, "Your faith in %s is good.&0\r\n", DEITY_NAME(ch));    
    else 
    	ch_printf(ch, "Your faith in %s is normal.\r\n", DEITY_NAME(ch));
    if (FOL_VALERIA(ch))
    	send_to_char("You are under Valeria's protection.\r\n", ch);
    }
    
    sprintf(buf, "You are %s.\r\n",carry_cond[MAX(0, MIN(3, ((int) get_carry_cond(ch))))]);
    send_to_char(buf, ch);
    
    
    sprintf(buf,"Your fighting style is %s.\r\n",styles[GET_STYLE(ch)]);
    send_to_char(buf,ch);

#ifdef EVENT_MOVE    
    sprintf(buf, "You are %s when you move.\r\n", (PRF2_FLAGGED(ch, PRF2_RUNNING) ? "running" : "walking"));
    send_to_char(buf, ch);
#endif
    if (QUESTING(ch))
        send_to_char("You are currently a part of a quest.\r\n", ch);

    strcpy(buf, "You are ");
    switch (GET_POS(ch)) {
    case POS_DEAD:
        sprintf(buf, "%s%sDEAD!%s\r\n", buf, CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
        break;
    case POS_MORTALLYW:
        sprintf(buf, "%s%smortally wounded!%s  You should seek help!\r\n",
                buf, CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
        break;
    case POS_INCAP:
        strcat(buf, "incapacitated, slowly fading away...\r\n");
        break;
    case POS_STUNNED:
        strcat(buf, "stunned!  You can't move!\r\n");
        break;
    case POS_SLEEPING:
        if (!AFF2_FLAGGED(ch, AFF2_MEDITATE))
            strcat(buf, "sleeping.\r\n");
        else
            strcat(buf, "meditating.\r\n");
        break;
    case POS_RESTING:
        strcat(buf, "resting.\r\n");
        break;
    case POS_SITTING:
        strcat(buf, "sitting.\r\n");
        break;
    case POS_FIGHTING:
        if (FIGHTING(ch))
            sprintf(buf, "%sfighting %s!\r\n", buf, PERS(FIGHTING(ch), ch));
        else
            strcat(buf, "fighting thin air.\r\n");
        break;
    case POS_STANDING:
        if (IS_AFFECTED(ch, AFF_FLYING))
            strcat(buf, "flying.\r\n");
        else if (IS_AFFECTED(ch, AFF_WATERWALK))
            strcat(buf, "floating.\r\n");
        else
            strcat(buf, "standing.\r\n");
        break;
    default:
        strcat(buf, "floating.\r\n");
        break;
    }

    if (!IS_NPC(ch) && ch->player.long_descr)
    {
        sprintf(buf2, "You are disguised as: %s", ch->player.long_descr);
        strcat(buf, buf2);
    }

    if (AFF3_FLAGGED(ch, AFF3_QUAD))
        strcat(buf, "&W&fYou are dealing out QUAD damage.&0\r\n");

    if (GET_COND(ch, DRUNK) > 0)
        strcat(buf, "You are drunk.\r\n");

    if (GET_COND(ch, FULL) == 0)
        strcat(buf, "&wYou are very hungry.&0\r\n");

    if (GET_COND(ch, THIRST) == 0)
        strcat(buf, "&wYou are very thirsty.&0\r\n");

    if (IS_AFFECTED(ch, AFF_SNEAK))
        strcat(buf, "You are sneaking.\r\n");

    if (IS_AFFECTED(ch, AFF_HIDE))
        strcat(buf, "&bYou are hidden.&0\r\n");


    if (IS_AFFECTED(ch, AFF_BLIND))
        strcat(buf, "You have been blinded!\r\n");

    if (IS_AFFECTED(ch, AFF_INVISIBLE))
        strcat(buf, "&yYou are invisible.&0\r\n");

    if (IS_AFFECTED(ch, AFF_DETECT_INVIS))
        strcat(buf, "You are sensitive to the presence of invisible things.\r\n");

    if (IS_AFFECTED(ch, AFF_SANCTUARY))
        strcat(buf, "You are protected by &BSanctuary.&0\r\n");


    if (IS_AFFECTED(ch, AFF_POISON))
        strcat(buf, "You are poisoned!\r\n");

    if (IS_AFFECTED(ch, AFF_CHARM))
        strcat(buf, "You have been charmed!\r\n");

    /*  if (affected_by_spell(ch, SPELL_ARMOR))
        strcat(buf, "You feel protected.\r\n");
    */
    if (IS_AFFECTED(ch, AFF_INFRAVISION))
        strcat(buf, "Your eyes are glowing red.\r\n");

    if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
        strcat(buf, "You are summonable by other players.\r\n");

    if (PRF2_FLAGGED(ch, PRF2_CONCEAL))
        strcat(buf, "You are concealing your actions.\r\n");

    if (PRF2_FLAGGED(ch, PRF2_TUMBLE))
        strcat(buf, "You are using tumble in combat.\r\n");

    if (ch->guarding)
    {
        sprintf(buf1, "You are guarding %s.\r\n", GET_NAME(ch->guarding));
        strcat(buf, buf1);
    }
    
    

    send_to_char(buf, ch);
}

ACMD(do_inventory)
{
    
    //send_to_char("\r\n&PItem                                                              Num  Weight&0\r\n", ch);
    //send_to_char("&p----                                                              ---  ------&0\r\n", ch);
    
    send_to_char("\r\n&PNum Item                                                              Weight&0\r\n", ch);
    send_to_char("&p--- ----                                                              ------&0\r\n", ch);
    
    list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


ACMD(do_equipment)
{
    int i, found = 0, j;

    /*send_to_char("&G                                +++ Equipment +++&0\r\n", ch);
    send_to_char("                                -----------------\r\n\r\n", ch);
      */
    //send_to_char("Body Location          Item\r\n----------------------------\r\n", ch);

    send_to_char("\r\n&PBody Location        Item                                           Condition\r\n&p---- --------        ----                                           ---------&0\r\n", ch);

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i)) {
            if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
                ch_printf(ch, "%-25s",where[i]);
                show_obj_to_char(GET_EQ(ch, i), ch, 11, 0, 0);
                j=MAX(0, (GET_OBJ_DAMAGE(GET_EQ(ch, i)))/10);
                ch_printf(ch, "%s\r\n", obj_condition_names[j]);
            } else {
                ch_printf(ch, "%-25s",where[i]);
                ch_printf(ch, "%-46s unknown\r\n", "Something");
            }

            found = TRUE;
        }
    }
    if (IN_ARENA(ch))
    {
        if (HAS_RED(ch))
        {
            send_to_char("<&Rworn as flag&0>       &RRED FLAG&0\r\n", ch);
            found=1;
        }
        if (HAS_BLUE(ch))
        {
            send_to_char("<&Bworn as flag&0>       &BBLUE FLAG&0\r\n", ch);
            found=1;
        }
    }
    if (!found) {
        send_to_char(" Nothing.\r\n", ch);
    }

}


//extern int pulse;
ACMD(do_time)
{
    char *suf;
    int weekday, day, min;
    extern struct time_info_data time_info;
    min=(pulse % (SECS_PER_MUD_HOUR*PASSES_PER_SEC))/PASSES_PER_SEC;
    sprintf(buf, "It is %d:%02d o'clock, on ",
            time_info.hours ,min);

    /* 35 days in a month */
    weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

    strcat(buf, weekdays[weekday]);
    strcat(buf, ".\r\n");
    send_to_char(buf, ch);

    day = time_info.day + 1;	/* day in [1..35] */

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

    sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
            day, suf, month_name[(int) time_info.month], time_info.year);

    send_to_char(buf, ch);
}


ACMD(do_weather)
{
    static char *sky_look[] = {
                                  "cloudless",
                                  "cloudy",
                                  "rainy",
                                  "lit by flashes of lightning"};

    if (OUTSIDE(ch)) {
        sprintf(buf, "&BThe sky is %s and %s.&0\r\n", sky_look[weather_info.sky],
                (weather_info.change >= 0 ? "you feel a warm wind from south" :
                 "your foot tells you bad weather is due"));
        send_to_char(buf, ch);
        send_to_char("\r\nYou can also type 'look sky' to see the current starmap.\r\n", ch);
    } else
        send_to_char("&wYou have no feeling about the weather at all.&0\r\n", ch);
}


ACMD(do_index)
{
    int i;
    int row=0;
    int minlen;
    int all=0;
    extern int top_of_helpt_skills;
    extern struct help_index_element *help_index_skills;
    extern int top_of_helpt;
    extern struct help_index_element *help_index;

    if(!ch->desc)
        return;

    skip_spaces(&argument);
    *buf=0;

    if(!*argument)
    {
        send_to_char("USAGE: index <letter|phrase> <all>\r\n",ch);
        return;
    }
    if (!strcmp("all", argument))
        all=1;


    minlen=strlen(argument);

    strcat(buf, "&cTOPICS:&0\r\n");
    for(i=0;i<top_of_helpt;i++)
    {
        if(!strn_cmp(argument,help_index[i].keyword,minlen) || all)
        {
            row++;
            sprintf(buf+strlen(buf),"|%-23.23s |",help_index[i].keyword);
            if((row%3)==0)
                strcat(buf,"\r\n");
        }
    }

    strcat(buf, "\r\n\r\n&cSPELLS and SKILLS:&0\r\n");
    for(i=0;i<top_of_helpt_skills;i++)
    {
        if(!strn_cmp(argument,help_index_skills[i].keyword,minlen) || all)
        {
            row++;
            sprintf(buf+strlen(buf),"|%-23.23s |",help_index_skills[i].keyword);
            if((row%3)==0)
                strcat(buf,"\r\n");
        }

    }
    strcat(buf, "\r\n");
    if(ch->desc)
        page_string(ch->desc,buf,1);

}


void send_desc_help(struct descriptor_data *ch, char *argument)
{
    extern int top_of_helpt;
    extern struct help_index_element *help_index;
    extern FILE *help_fl;
    extern char *help;
    int i;
    extern char *helps[1000];

    int chk, bot, top, mid, minlen;

    
    skip_spaces(&argument);

    
    if (!help_index) {
        SEND_TO_Q("No help available.\r\n", ch);
        return;
    }
    bot = 0;
    top = top_of_helpt;
    minlen = strlen(argument);
    for (;;) {
        mid = (bot + top) >> 1;
        if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {

            while ((mid > 0) && (!(chk = strn_cmp(argument, help_index[mid - 1].keyword, minlen))))        
                mid--;
            page_string(ch, helps[help_index[mid].pos], 0);
            return;
        } else if (bot >= top) {            
            return;
        } else if (chk > 0)
            bot = ++mid;
        else
            top = --mid;
    }
}


void do_soundexhelp(struct char_data *ch, char *argument)
{
    extern int top_of_helpt;
    extern struct help_index_element *help_index;
    extern int top_of_helpt_skills;
    extern struct help_index_element *help_index_skills;
       
    extern FILE *help_fl;
    extern char *help;
    int i;
    extern char *helps[1000];
    extern char *helps_skills[1000];
    
    char *s1k, *s2k;    
    int best=-1, sm, ipos=-1;

    int chk, bot, top, mid, minlen;

    if (!ch->desc)
        return;

    skip_spaces(&argument);

    if (!*argument) {
        page_string(ch->desc, help, 0);
        return;
    }
    if (!help_index) {
        send_to_char("No help available.\r\n", ch);
        return;
    }
    bot = 0;
    top = top_of_helpt;
    
    s1k=GetSoundexKey(strupper(argument));
    strcpy(buf, s1k);
    minlen = strlen(argument);
    for (i=0;i<top_of_helpt;i++) {
        s2k=GetSoundexKey(help_index[i].keyword);
        if ((sm=SoundexMatch(buf, s2k))>best)
        {
        	best=sm;
        	ipos=i;
        	if (sm==100)
        		break;
        }
        
   }   
   if (best==100)                       
   {
       ch_printf(ch,"&wNo help on '%s' found. Similar word found:&0\r\n\r\n",argument);        
       page_string(ch->desc, helps[help_index[ipos].pos], 0);                         
   }
   else
   {
   	
   	 for (i=0;i<top_of_helpt_skills;i++) {
        s2k=GetSoundexKey(help_index_skills[i].keyword);
        if ((sm=SoundexMatch(buf, s2k))>best)
           {
        	best=sm;
        	ipos=-i-1;
        	if (sm==100)
        		break;
          }
        
        } 
        if (ipos<0)
 		i=find_skill_num(help_index_skills[-ipos-1].keyword);        
 	else 
 		i=-1;
        if (best>=80 && (i==-1 || CAN_PRACTICE(ch, i) || GET_SKILL(ch, i)))                       
   	{
	       	ch_printf(ch,"&wNo help on '%s' found. Similar word found:&0\r\n\r\n",argument);        
       		if (ipos<0)
       			page_string(ch->desc, helps_skills[help_index_skills[-ipos-1].pos], 0);                         
       		else
       			page_string(ch->desc, helps[help_index[ipos].pos], 0);                         
   	}
   	else
   	{
   	    char buf[MAX_INPUT_LENGTH];
            sprintf(buf, "HELP:  %s attempted '%s'", GET_NAME(ch), argument);
            mudlog(buf, NRM, LVL_GOD, TRUE);
            send_to_char("You receive no answer from &cthe Great Tome of Knowledge&0.\r\n", ch);
           }
   }
}


ACMD(do_spellhelp)
{
    extern int top_of_helpt_skills;
    extern struct help_index_element *help_index_skills;
    extern FILE *help_fl;
    extern char *help;
    int i;
    extern char *helps_skills[1000];
    int chk, bot, top, mid, minlen;

    if (!ch->desc)
        return;
    skip_spaces(&argument);
    if (!*argument) {
        page_string(ch->desc, help, 0);
        return;
    }
    if (!help_index_skills) {
        send_to_char("You receive no answer from &cthe Great Tome of Knowledge&0.\r\n", ch);
        return;
    }
    bot = 0;
    top = top_of_helpt_skills;
    minlen = strlen(argument);
    for (;;) {
        mid = (bot + top) >> 1;
        if (!(chk = strn_cmp(argument, help_index_skills[mid].keyword, minlen))) {

            //  while ((mid > 0) && (!(chk = is_abbrev_multi(argument, help_index_skills[mid - 1].keyword))))
            while ((mid > 0) && (!(chk = strn_cmp(argument, help_index_skills[mid - 1].keyword, minlen))))
                mid--;
            i=find_skill_num(argument);
            if (i!=-1 && (CAN_PRACTICE(ch, i) || GET_SKILL(ch, i)))
            {
                send_to_char("&cThe Great Tome of Knowledge responds&0: \r\n\r\n", ch);
                page_string(ch->desc, helps_skills[help_index_skills[mid].pos], 0);
            }
            else if (i!=-1)
            {
                send_to_char("&cThe Great Tome of Knowledge responds&0: \r\n\r\n", ch);
                send_to_char("You are not ready to receive knowledge in that topic.\r\n", ch);
            }
            else
                send_to_char("You receive no answer from &cthe Great Tome of Knowledge&0.\r\n", ch);
            return;
        } else if (bot >= top) {
           
            do_soundexhelp(ch, argument);
            return;
        } else if (chk > 0)
            bot = ++mid;
        else
            top = --mid;
    }
}




ACMD(do_help)
{
    extern int top_of_helpt;
    extern struct help_index_element *help_index;
    extern FILE *help_fl;
    extern char *help;
    int i;
    extern char *helps[1000];

    int chk, bot, top, mid, minlen;

    if (!ch->desc)
        return;

    skip_spaces(&argument);

    if (!*argument) {
        page_string(ch->desc, help, 0);
        return;
    }
    if (!help_index) {
        send_to_char("No help available.\r\n", ch);
        return;
    }
    bot = 0;
    top = top_of_helpt;
    minlen = strlen(argument);
    for (;;) {
        mid = (bot + top) >> 1;
        if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {

            while ((mid > 0) && (!(chk = strn_cmp(argument, help_index[mid - 1].keyword, minlen))))
                // while ((mid > 0) && (!(chk = is_abbrev_multi(argument, help_index[mid - 1].keyword))))
                mid--;
            page_string(ch->desc, helps[help_index[mid].pos], 0);
            return;
        } else if (bot >= top) {
            do_spellhelp(ch, argument, 0, 0);
            return;
        } else if (chk > 0)
            bot = ++mid;
        else
            top = --mid;
    }
}

void write_who_file()
{
    struct descriptor_data *d;
    struct char_data *tch;
    int num_can_see=0;
    FILE *fl;
    time_t          ct;
    char           *tmstr;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    fl=fopen("/u/mud/.plan","w");
    if (!fl) return;
    fprintf(fl,"\n\n\nLands of Myst MUD, 147.91.1.129 5000\n----------------------------------\n\n");

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected)
            continue;

        if (d->original)
            tch = d->original;
        else if (!(tch = d->character))
            continue;
        if (IS_GOD(tch))
            continue;

        num_can_see++;
        sprintf(buf1, "%s %s", GET_NAME(tch), GET_TITLE(tch));
        if (!PRF2_FLAGGED(tch, PRF2_NOWHO))
            sprintf(buf, "[%s %2d %s] %-45.45s",
                    RACE_ABBR(tch), GET_LEVEL(tch), CLASS_ABBR(tch), buf1);
        else
            sprintf(buf, "[--- -- ---] %-45.45s",buf1);

        sprintf(buf, "%s [%s]", buf,clan[GET_CLAN(tch)].name);

        if (QUESTING(tch))
            strcat(buf, "(Q)");
        else if (IS_AFFECTED(tch, AFF_INVISIBLE))
            strcat(buf, "(I)");
        if (GET_INVIS_LEV(tch)) {
            sprintf(buf1, "(i%d)", GET_INVIS_LEV(tch));
            strcat(buf, buf1);
        }

        if (PRF2_FLAGGED(tch, PRF2_AFK))
            strcat(buf, " (AFK)");
        if (PLR_FLAGGED(tch, PLR_MAILING))
            strcat(buf, " (mailing)");
        else if (PLR_FLAGGED(tch, PLR_WRITING))
            strcat(buf, " (writing)");
        else if (PLR_FLAGGED(tch, PLR_EDITING))
            strcat(buf, " (editing)");
        if (PRF_FLAGGED(tch, PRF_DEAF))
            strcat(buf, " (deaf)");
        if (PRF_FLAGGED(tch, PRF_NOTELL))
            strcat(buf, " notell");
        fputs(buf, fl);
        fputs("\n",fl);
    }				/* end of for */

    fprintf(fl,"\nGenerated: %-15.15s\n\n", tmstr+4);
    fclose(fl);
}




#define WHO_FORMAT \
"format: who  [-n name] [-c classlist] [-s] [-q] [-r] [-z]\r\n"

ACMD(do_who)
{
    struct descriptor_data *d;
    struct char_data *tch;
    char name_search[MAX_INPUT_LENGTH];
    char mode;
    int i, k,low = 0, itot=0,high = LVL_IMPL, localwho = 0, questwho = 0;
    int showclass = 0, short_list = 0, outlaws = 0, num_can_see = 0, tot=0;
    int who_room = 0;
    int clan_num;
    char babf[1000];
    char *tmstr;
    time_t mytime, current_time;
    extern struct timeval curr_time;
    extern time_t boot_time_real;
    extern time_t boot_time;
    mytime = boot_time;

    gettimeofday( &curr_time, NULL );
    current_time = curr_time.tv_sec;
    sprintf_minutes(buf2, current_time-boot_time_real);



    i=players_online();
    if (i>boothigh)
        boothigh=i;
    tmstr = (char *) asctime(localtime(&mytime));
    *(tmstr + strlen(tmstr) - 1) = '\0';

    skip_spaces(&argument);
    strcpy(buf, argument);
    name_search[0] = '\0';

    while (*buf) {
        half_chop(buf, arg, buf1);
        /*	if (isdigit(*arg)) {
        	    sscanf(arg, "%d-%d", &low, &high);
        	    strcpy(buf, buf1);
        	} else*/ if (*arg == '-') {
            mode = *(arg + 1);	/* just in case; we destroy arg in the
              switch */
            switch (mode) {
            case 'o':
            case 'k':
                outlaws = 1;
                strcpy(buf, buf1);
                break;
            case 'z':
                localwho = 1;
                strcpy(buf, buf1);
                break;
                /*	    case 's':
                		short_list = 1;
                		strcpy(buf, buf1);
                		break;*/
            case 'q':
                questwho = 1;
                strcpy(buf, buf1);
                break;
            case 'l':
                half_chop(buf1, arg, buf);
                sscanf(arg, "%d-%d", &low, &high);
                break;
            case 'n':
                half_chop(buf1, name_search, buf);
                break;
            case 'r':
                who_room = 1;
                strcpy(buf, buf1);
                break;
            case 'c':
                half_chop(buf1, arg, buf);
                for (i = 0; i < strlen(arg); i++)
                    showclass |= find_class_bitvector(arg[i]);
                break;
            default:
                send_to_char(WHO_FORMAT, ch);
                return;
                break;
            }			/* end of switch */

        } else {		/* endif */
            send_to_char(WHO_FORMAT, ch);
            return;
        }
    }				/* end while (parser) */

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected)
            continue;

        if (d->original)
            tch = d->original;
        else if (!(tch = d->character))
            continue;
        if (IS_IMMORT(tch) && CAN_SEE(ch, tch))// && (tch->in_room!=1 || (IS_IMMORT(ch) && CAN_SEE(ch, tch))))
        {
            itot++;
            num_can_see++;



            if (itot==1)
                send_to_char("[&RImmortals&0]\r\n-----------\r\n&0", ch);

            sprintf(buf1, "%s %s", GET_NAME(tch), GET_TITLE(tch));
            switch (GET_LEVEL(tch))
            {
            	case LVL_IMMORT:sprintf(buf, "[&y  Avatar  &0] %-45.45s", buf1);break;
            	case LVL_GOD:sprintf(buf, "[&Y  BUILDER &0] %-45.45s", buf1);break;
            	case LVL_GRGOD:sprintf(buf, "[&C   DEITY  &0] %-45.45s", buf1);break;
		case LVL_IMPL:sprintf(buf, "[&P   IMPL   &0] %-45.45s", buf1);break;
	     }
            	
            	
            if (!PRF2_FLAGGED(tch, PRF2_NOWHO) || GET_LEVEL(ch)>=GET_LEVEL(tch));
                //sprintf(buf, "[&c%s %2d %s&0] %-45.45s", RACE_ABBR(tch), GET_LEVEL(tch), CLASS_ABBR(tch), buf1);
                
            else
                sprintf(buf, "%s[&L----------&0] %-45.45s",
                        (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),buf1);

            // sprintf(buf, "%s [%s]", buf,clan[GET_CLAN(tch)].name);

            if (QUESTING(tch))
                strcat(buf, "(Q)");
            else if (IS_AFFECTED(tch, AFF_INVISIBLE))
                strcat(buf, "(I)");
            if (GET_INVIS_LEV(tch)) {
                sprintf(buf1, "(i%d)", GET_INVIS_LEV(tch));
                strcat(buf, buf1);
            }

            if (PRF2_FLAGGED(tch, PRF2_AFK))
                strcat(buf, " (AFK)");
            if (PLR_FLAGGED(tch, PLR_MAILING))
                strcat(buf, " (mailing)");
            else if (PLR_FLAGGED(tch, PLR_WRITING))
                strcat(buf, " (writing)");
            else if (PLR_FLAGGED(tch, PLR_EDITING))
                strcat(buf, " (editing)");
            if (PRF_FLAGGED(tch, PRF_DEAF))
                strcat(buf, " (deaf)");
            if (PRF_FLAGGED(tch, PRF_NOTELL))
                strcat(buf, " (notell)");
            if (GET_LEVEL(tch) >= LVL_IMMORT)
                strcat(buf, CCNRM(ch, C_SPR));
            strcat(buf, "\r\n");
            send_to_char(buf, ch);
        }
    }

    if (itot)
        send_to_char("\r\n",ch);









    send_to_char("[&GMortals&0]\r\n---------\r\n&0", ch);

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected)
            continue;

        if (d->original)
            tch = d->original;
        else if (!(tch = d->character))
            continue;
        if (IS_IMMORT(tch))// || (tch->in_room==1 && !IS_IMMORT(ch) && !CAN_SEE(ch, tch)))
            continue;
        tot++;
        if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
                !strstr(GET_TITLE(tch), name_search))
            continue;
        if ((GET_LEVEL(tch) < low || GET_LEVEL(tch) > high) )
            continue;
        if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
                !PLR_FLAGGED(tch, PLR_THIEF))
            continue;
        if (questwho && !QUESTING(tch))
            continue;
        if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
            continue;
        if (who_room && (tch->in_room != ch->in_room))
            continue;
        if (showclass && !(showclass & GET_CLASS(tch)))
            continue;

        {
            if (!IS_IMMORT(tch) && CAN_SEE(ch, tch))
                num_can_see++;

            if (CAN_SEE(ch, tch))
                sprintf(buf1, "%s %s", GET_NAME(tch), GET_TITLE(tch));
            else
                sprintf(buf1, "(Someone)", GET_NAME(tch), GET_TITLE(tch));

            if ((!PRF2_FLAGGED(tch, PRF2_NOWHO) || GET_LEVEL(ch)>=GET_LEVEL(tch)) && CAN_SEE(ch, tch))
                sprintf(buf, "%s&0[&c%s %2d %s&0] %-45.45s&0",
                        (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),
                        RACE_ABBR(tch), GET_LEVEL(tch), CLASS_ABBR(tch), buf1);
            else
                sprintf(buf, "%s[&L--- -- ---&0] %-45.45s&0",
                        (GET_LEVEL(tch) >= LVL_IMMORT ? CCYEL(ch, C_SPR) : ""),buf1);

            sprintf(buf, "%s", buf);

            if (CAN_SEE(ch, tch))
            {
                if (QUESTING(tch))
                    strcat(buf, "(Q)");
                else if (IS_AFFECTED(tch, AFF_INVISIBLE))
                    strcat(buf, "(I)");
                if (GET_INVIS_LEV(tch)) {
                    sprintf(buf1, "(i%d)", GET_INVIS_LEV(tch));
                    strcat(buf, buf1);
                }

                if (PRF2_FLAGGED(tch, PRF2_AFK))
                    strcat(buf, " (AFK)");
                if (PLR_FLAGGED(tch, PLR_MAILING))
                    strcat(buf, " (mailing)");
                else if (PLR_FLAGGED(tch, PLR_WRITING))
                    strcat(buf, " (writing)");
                else if (PLR_FLAGGED(tch, PLR_EDITING))
                    strcat(buf, " (editing)");
                if (PRF_FLAGGED(tch, PRF_DEAF))
                    strcat(buf, " (deaf)");
                if (PRF_FLAGGED(tch, PRF_NOTELL))
                    strcat(buf, " (notell)");
            }
            if (GET_LEVEL(tch) >= LVL_IMMORT)
                strcat(buf, CCNRM(ch, C_SPR));

            strcat(buf, "\r\n");
            send_to_char(buf, ch);
        }			/* endif shortlist */
    }				/* end of for */
    if (short_list && (num_can_see % 4))
        send_to_char("\r\n", ch);
    if (num_can_see == 0)
        sprintf(buf, "\r\nNo-one at all!\r\n");
    else
    {
        /*if (num_can_see == 1)
        sprintf(buf, "\r\nOne fearsome character displayed, &w%d&0 total detected in game.\r\n", tot+itot);
        else
        sprintf(buf, "\r\n&w%d&0 characters visible to you, &w%d&0 total detected in game.\r\n", num_can_see, tot+itot);
          */
        sprintf(buf, "\r\nTotal of &w%d&0 players detected in game.\r\n", tot+itot);
    }
    send_to_char(buf, ch);
    //sprintf(buf, "Boot time high was %d players.\r\nLast reboot was at %s.\r\n", boothigh, tmstr);
    sprintf(buf, "Boot time high was &w%d&0 players.\r\nThe portal to LOM has been open for %s.\r\n", boothigh, buf2);
    send_to_char(buf, ch);
}


#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

ACMD(do_users)
{
    extern char *connected_types[];
    char line[200], line2[220], idletime[10], classname[20];
    char state[30], *timeptr, *format, mode;
    char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
    struct char_data *tch;
    struct descriptor_data *d;
    int low = 0, high = LVL_IMPL, i, num_can_see = 0;
    int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;

    host_search[0] = name_search[0] = '\0';

    //send_to_char("Command temporarly disabled.\r\n", ch);
    //return;

    strcpy(buf, argument);
    while (*buf) {
        half_chop(buf, arg, buf1);
        if (*arg == '-') {
            mode = *(arg + 1);	/* just in case; we destroy arg in the
              switch */
            switch (mode) {
            case 'o':
            case 'k':
                outlaws = 1;
                playing = 1;
                strcpy(buf, buf1);
                break;
            case 'p':
                playing = 1;
                strcpy(buf, buf1);
                break;
            case 'd':
                deadweight = 1;
                strcpy(buf, buf1);
                break;
            case 'l':
                playing = 1;
                half_chop(buf1, arg, buf);
                sscanf(arg, "%d-%d", &low, &high);
                break;
            case 'n':
                playing = 1;
                half_chop(buf1, name_search, buf);
                break;
            case 'h':
                playing = 1;
                half_chop(buf1, host_search, buf);
                break;
            case 'c':
                playing = 1;
                half_chop(buf1, arg, buf);
                for (i = 0; i < strlen(arg); i++)
                    showclass |= find_class_bitvector(arg[i]);
                break;
            default:
                send_to_char(USERS_FORMAT, ch);
                return;
                break;
            }			/* end of switch */

        } else {		/* endif */
            send_to_char(USERS_FORMAT, ch);
            return;
        }
    }				/* end while (parser) */
    strcpy(line,
           "Num Name         State          Idl Login@   Site [Email]\r\n");
    strcat(line,
           "--- ------------ -------------- --- -------- ------------------------------------\r\n");
    send_to_char(line, ch);

    one_argument(argument, arg);

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected && playing)
            continue;
        if (!d->connected && deadweight)
            continue;
        if (!d->connected) {
            if (d->original)
                tch = d->original;
            else if (!(tch = d->character))
                continue;

            if (*host_search && !strstr(d->host, host_search))
                continue;
            if (*name_search && str_cmp(GET_NAME(tch), name_search))
                continue;
            if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
                continue;
            if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
                    !PLR_FLAGGED(tch, PLR_THIEF))
                continue;
            if (showclass && !(showclass & GET_CLASS(tch)))
                continue;
            if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
                continue;
            /*
            	    if (d->original)
            		sprintf(classname, "%2d %s", GET_LEVEL(d->original),
            			CLASS_ABBR(d->original));
            	    else
            		sprintf(classname, "%2d %s", GET_LEVEL(d->character),
            			CLASS_ABBR(d->character));*/
        } else
            strcpy(classname, "     -     ");

        timeptr = asctime(localtime(&d->login_time));
        timeptr += 11;
        *(timeptr + 8) = '\0';

        if (!d->connected && d->original)
            strcpy(state, "Switched");
        else
            //if (d->connected==CON_PLAYING)
                strcpy(state, connected_types[d->connected]);
                //strcpy(state, "-");

        if (d->character && !d->connected && GET_LEVEL(d->character) < LVL_GOD)
            sprintf(idletime, "%3d", d->character->char_specials.timer *
                    SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
        else
            strcpy(idletime, "N/A");

        format = "%3d %-12s %-12s   %-3s %-8s ";

        if (d->character && d->character->player.name) {
            if (d->original)
                sprintf(line, format, d->desc_num,
                        d->original->player.name, state, idletime, timeptr);
            else
                sprintf(line, format, d->desc_num,
                        d->character->player.name, state, idletime, timeptr);
        } else
            sprintf(line, format, d->desc_num, "   -   ", 
                    state, idletime, timeptr);

        if (*d->host)
            sprintf(line + strlen(line), "%s [%s]\r\n", d->host,  d->character?d->character->player_specials->saved.email:"");
        else
            sprintf(line+strlen(line), "Unknown [%s]\r\n",  d->character? d->character->player_specials->saved.email:"");

        if (d->connected) {
            sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
            strcpy(line, line2);
        }
        if (d->connected || (!d->connected && CAN_SEE(ch, d->character))) {
            send_to_char(line, ch);
            num_can_see++;
        }
    }

    sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
    send_to_char(line, ch);
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
    extern char circlemud_version[];

    switch (subcmd) {
    case SCMD_CREDITS:
        page_string(ch->desc, credits, 0);
        break;
    case SCMD_NEWS:
        page_string(ch->desc, news, 0);
        break;
    case SCMD_INFO:
        page_string(ch->desc, info, 0);
        break;
    case SCMD_WIZLIST:
        page_string(ch->desc, wizlist, 0);
        break;
    case SCMD_IMMLIST:
        page_string(ch->desc, immlist, 0);
        break;
    case SCMD_HANDBOOK:
        page_string(ch->desc, handbook, 0);
        break;
    case SCMD_BUGLIST:
        file_to_string_alloc(BUGLIST_FILE, &buglist);
        page_string(ch->desc, buglist, 0);
        break;
    case SCMD_IDEALIST:
        file_to_string_alloc(IDEALIST_FILE, &idealist);
        page_string(ch->desc, idealist, 0);
        break;
    case SCMD_TYPOLIST:
        file_to_string_alloc(TYPOLIST_FILE, &typolist);
        page_string(ch->desc, typolist, 0);
        break;
    case SCMD_POLICIES:
        page_string(ch->desc, policies, 0);
        break;
    case SCMD_MOTD:
        page_string(ch->desc, motd, 0);
        break;
    case SCMD_IMOTD:
        page_string(ch->desc, imotd, 0);
        break;
    case SCMD_CLEAR:
        send_to_char("\033[H\033[J", ch);
        if (GET_TERM(ch) == VT100)
            InitScreen(ch);
        break;
    case SCMD_VERSION:
        //send_to_char(circlemud_version, ch);
        send_to_char("\r\n\r\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\nLands of Myst (c) 1996-2002 Vladimir Prelovac\r\n", ch);
        ch_printf(ch, "\r\nCompiled at %s, %s\r\n",  __TIME__, __DATE__);
        send_to_char("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n", ch);
        
        
        break;
    case SCMD_WHOAMI:
        send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch);
        break;
    default:
        return;
        break;
    }
}


void perform_mortal_where(struct char_data * ch, char *arg)
{
    register struct char_data *i;
    register struct descriptor_data *d;

    if (!*arg) {
        send_to_char("Players in your Zone\r\n--------------------\r\n", ch);
        for (d = descriptor_list; d; d = d->next)
            if (!d->connected) {
                i = (d->original ? d->original : d->character);
                if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) &&
                        (world[ch->in_room].zone == world[i->in_room].zone) && GET_ALIGNMENT(ch)==GET_ALIGNMENT(i)) {
                    sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
                    send_to_char(buf, ch);
                }
            }
    } else {			/* print only FIRST char, not all. */
        for (i = character_list; i; i = i->next)
            if (world[i->in_room].zone == world[ch->in_room].zone && CAN_SEE(ch, i) &&
                    (i->in_room != NOWHERE) && isname(arg, i->player.name) && GET_ALIGNMENT(ch)==GET_ALIGNMENT(i)) {
                sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
                send_to_char(buf, ch);
                return;
            }
        send_to_char("No-one around by that name.\r\n", ch);
    }
}


void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
                           int recur)
{
    if (num > 0)
        sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
    else
        sprintf(buf, "%33s", " - ");

    if (obj->in_room > NOWHERE) {
        sprintf(buf + strlen(buf), "[%5d] %s\r\n",
                world[obj->in_room].number, world[obj->in_room].name);
        send_to_char(buf, ch);
    } else if (obj->carried_by) {
        sprintf(buf + strlen(buf), "carried by %s.\r\n",
                PERS(obj->carried_by, ch));
        send_to_char(buf, ch);
    } else if (obj->worn_by) {
        sprintf(buf + strlen(buf), "worn by %s.\r\n",
                PERS(obj->worn_by, ch));
        send_to_char(buf, ch);
    } else if (obj->in_obj) {
        sprintf(buf + strlen(buf), "inside %s%s.\r\n",
                obj->in_obj->short_description, (recur ? ", which is" : " "));
        send_to_char(buf, ch);
        if (recur)
            print_object_location(0, obj->in_obj, ch, recur);
    } else {
        sprintf(buf + strlen(buf), "in an unknown location.\r\n");
        send_to_char(buf, ch);
    }
}



void perform_immort_where(struct char_data * ch, char *arg)
{
    register struct char_data *i;
    register struct obj_data *k;
    struct descriptor_data *d;
    int num = 0, found = 0;

    if (!*arg) {
        send_to_char("Players\r\n-------\r\n", ch);
        for (d = descriptor_list; d; d = d->next)
            if (!d->connected) {
                i = (d->original ? d->original : d->character);
                if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
                    if (d->original)
                        sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
                                GET_NAME(i), world[d->character->in_room].number,
                                world[d->character->in_room].name, GET_NAME(d->character));
                    else
                        sprintf(buf, "[%2d %3s] %-10s - [%5d] %s, HP: %d\r\n", GET_LEVEL(i), CLASS_ABBR(i), GET_NAME(i),
                                world[i->in_room].number, world[i->in_room].name, (int) (GET_HIT(i)*100/GET_MAX_HIT(i)));
                    send_to_char(buf, ch);
                }
            }
    } else {
        for (i = character_list; i; i = i->next)
            if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
                found = 1;
                sprintf(buf, "[%5d] %-25s - [%5d] %s\r\n", GET_MOB_VNUM(i), GET_NAME(i),
                        world[i->in_room].number, world[i->in_room].name);
                send_to_char(buf, ch);
            }
        for (num = 0, k = object_list; k; k = k->next)
            if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name) &&
                    (!k->carried_by || CAN_SEE(ch, k->carried_by))) {
                found = 1;
                print_object_location(++num, k, ch, TRUE);
            }
        if (!found)
            send_to_char("Couldn't find any such thing.\r\n", ch);
    }
}



ACMD(do_where)
{
    one_argument(argument, arg);

    if (GET_LEVEL(ch) >= LVL_IMMORT)
        perform_immort_where(ch, arg);
    //else send_to_char("You can not use this command, mortal.\r\n",ch);
    else
        perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
    /*int i, j = GET_LEVEL(ch);

    if (IS_NPC(ch)) {
    send_to_char("No levels for mobs available.\r\n", ch);
    return;
}
    *buf = '\0';

    for (i = j; i < MIN(LVL_IMMORT, j + 10); i++) {
    sprintf(buf + strlen(buf), "[%2d] %8d-%-8d", i,
    total_exp(i-1)+1, total_exp(i));

    strcat(buf, "\r\n");
}
    send_to_char(buf, ch);*/
}

char* diff_txt[]={
                     "Much Lower",
                     "Lower",
                     "Same",
                     "Greater",
                     "Much Greater"
                 }               ;
int get_diff(int diff)
{
    int a=0;
    if (diff<=-5)
        a=0;
    else if (diff<0)
        a=1;
    else if (diff==0)
        a=2;
    else if (diff>=5)
        a=4;
    else if (diff>0)
        a=3;
    return a;
}


ACMD(do_consider)
{
    char *por[] = {
                      "&BVastly inferior to your&0\r\n",
                      "&BWay out worser than yours&0\r\n",
                      "&GMuch worser than yours&0\r\n",
                      "&GPretty worser than yours&0\r\n",
                      "&CSlightly worser than yours&0\r\n",
                      "&CAbout the same&0\r\n",
                      "&CSlightly better than yours&0\r\n",
                      "&YPretty better than yours&0\r\n",
                      "&YMuch better than yours&0\r\n",
                      "&RWay out better than yours&0\r\n",
                      "&R&fVastly superior to your&0\r\n",
                  };
    int dane;
    struct char_data *victim;
    int diff, diffu, maxhit1, maxhit2, hit1, hit2, t1, t2;
    struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
    struct obj_data *wieldedv;
    int  dam = 0, damv = 0,  bonus=0, adam,plev;


    extern struct str_app_type str_app[];
    extern struct dex_app_type dex_app[];



    one_argument(argument, buf);

    if (!(victim = get_char_room_vis(ch, buf))) {
        send_to_char("Consider killing who?\r\n", ch);
        return;
    }
    if (victim == ch) {
        send_to_char("Easy!  Very easy indeed!\r\n", ch);
        return;
    }
    /*  if (!IS_NPC(victim)) {
        send_to_char("Would you like to borrow a cross and a shovel?\r\n", ch);
        return;
      }
     @maybe
     */
    if (IS_IMMORT(ch)) {
        send_to_char("Not worthy bothering...\r\n", ch);
        return;
    }
    wieldedv = GET_EQ(victim, WEAR_WIELD);

    dane = 0;//GET_SKILL(ch, SKILL_CONSIDER) > 60;
    /*  sprintf(buf,"Considering %s.\r\n",GET_NAME(victim));*/
    /*  send_to_char("Considering ",ch);
      send_to_char(GET_NAME(victim),ch);
      send_to_char(buf,ch);*/
    /* diffu = 0;
     diff = GET_LEVEL(victim) - GET_LEVEL(ch);
     diffu += diff * 15;
     if (dane) {
    send_to_char("LEVEL        : ", ch);
    if (diff <= -13)
      send_to_char(por[0], ch);
    else if (diff <= -8)
      send_to_char(por[1], ch);
    else if (diff <= -5)
      send_to_char(por[2], ch);
    else if (diff <= -2)
      send_to_char(por[3], ch);
    else if (diff <= -1)
      send_to_char(por[4], ch);
    else if (diff >= 13)
      send_to_char(por[10], ch);
    else if (diff >= 8)
      send_to_char(por[9], ch);
    else if (diff >= 5)
      send_to_char(por[8], ch);
    else if (diff >= 2)
      send_to_char(por[7], ch);
    else if (diff >= 1)
      send_to_char(por[6], ch);
    else
      send_to_char(por[5], ch);
     }                          
      */
    hit1=GET_HIT(victim);
    //if (AFF_FLAGGED(victim, AFF_SANCTUARY))
    //	hit1+=GET_HIT(victim);
    maxhit1=GET_MAX_HIT(victim);
    //if (AFF_FLAGGED(victim, AFF_SANCTUARY))
    //	maxhit1+=GET_MAX_HIT(victim);

    t1=-3 +3*hit1/maxhit1;
    t1=0;

    hit2=GET_HIT(ch);
    // if (AFF_FLAGGED(ch, AFF_SANCTUARY))
    // 	hit2+=GET_HIT(ch);
    maxhit2=GET_MAX_HIT(ch);
    //if (AFF_FLAGGED(ch, AFF_SANCTUARY))
    //	maxhit2=GET_MAX_HIT(ch);

    t2=-3 +3*hit2/maxhit2;    
    t2=0;

    if (IS_NPC(victim))
    {
        if (IS_CASTERMOB(victim))
            bonus+=2;
        if (GET_EQ(victim, WEAR_WIELD))
            bonus+=1;                          
	plev=GET_LEVEL(victim);
        adam=MIN(GET_AC(victim), 3*(plev*plev/6+3*plev)/2);        
        if (GET_AC(victim)>adam)
            bonus+=1;
        
        bonus=MIN(3, bonus);
    }
    diff=GET_LEVEL(victim)+t1+bonus-(GET_LEVEL(ch)+t2);

    /*
        	
        diff = (GET_HIT(victim) - GET_HIT(ch))/GET_LEVEL(ch);
        if (AFF_FLAGGED(ch, AFF_SANCTUARY))
        	diff-=GET_HIT(ch);
        if (AFF_FLAGGED(victim, AFF_SANCTUARY))
        	diff+=GET_HIT(victim);
        	
        diffu += diff;
        if (dane) {
    	send_to_char("HEALTH       : ", ch);
    	if (diff <= -40)
    	    send_to_char(por[0], ch);
    	else if (diff <= -25)
    	    send_to_char(por[1], ch);
    	else if (diff <= -15)
    	    send_to_char(por[2], ch);
    	else if (diff <= -8)
    	    send_to_char(por[3], ch);
    	else if (diff <= -3)
    	    send_to_char(por[4], ch);
    	else if (diff >= 40)
    	    send_to_char(por[10], ch);
    	else if (diff >= 25)
    	    send_to_char(por[9], ch);
    	else if (diff >= 15)
    	    send_to_char(por[8], ch);
    	else if (diff >= 8)
    	    send_to_char(por[7], ch);
    	else if (diff >= 3)
    	    send_to_char(por[6], ch);
    	else
    	    send_to_char(por[5], ch);
        }
        dam = GET_DAMROLL(ch);

        if ((dam == 0) && (IS_NPC(ch)))
    	dam += GET_LEVEL(ch) / 5;

        
        if (wielded && (GET_OBJ_TYPE(wielded) == ITEM_WEAPON || GET_OBJ_TYPE(wielded) == ITEM_FIREWEAPON))
    	dam += (GET_OBJ_VAL(wielded, 1) * ((GET_OBJ_VAL(wielded, 2) + 1) / 2)) + GET_OBJ_VAL(wielded, 0);
        else {
    	if (IS_NPC(ch)) {
    	    int pdam;    
    	    pdam = (GET_LEVEL(ch) / 2 + GET_LEVEL(ch)*4/3 ) / 2+1;
    	    dam += pdam;
    	} else if (IS_MONK(ch)) {
    	    dam +=GET_LEVEL(ch);
    	} else
    	    dam += 1;
        }
        dam *= str_app[STRENGTH_APPLY_INDEX(ch)].todam;
        damv = GET_DAMROLL(victim);

        if ((damv == 0) && (IS_NPC(victim)))
    	damv += GET_LEVEL(victim) / 5;
        
        if (wieldedv && (GET_OBJ_TYPE(wieldedv) == ITEM_WEAPON || GET_OBJ_TYPE(wieldedv) == ITEM_FIREWEAPON))
    	{
    	damv += (GET_OBJ_VAL(wieldedv, 1) * ((GET_OBJ_VAL(wieldedv, 2) + 1) / 2)) + GET_OBJ_VAL(wieldedv, 0);
    	if (IS_NPC(victim)) {
    	    int pdam;
    	    pdam = (GET_LEVEL(victim) / 2 + GET_LEVEL(victim)*4/3) / 2+1;
    	    damv += pdam;
    	    }
    	    }
        else {
    	if (IS_NPC(victim)) {
    	    int pdam;
    	    pdam = (GET_LEVEL(victim) / 2 + GET_LEVEL(victim)*4/3) / 2+1;
    	    damv += pdam;
    	} else if (IS_MONK(victim)) {
    	    damv += GET_LEVEL(victim);
    	} else
    	    damv += 1;	
        }
    damv *= str_app[STRENGTH_APPLY_INDEX(victim)].todam;
    if (IS_NPC(victim) && !IS_WARRIOR(victim))
       damv*=1.5;
       if (IS_CASTER(ch))
       	dam*=1.4;

        diff = damv - dam;
        diffu += diff * 5;
        if (dane) {
    	send_to_char("BATTLE POWER : ", ch);
    	if (diff <= -25)
    	    send_to_char(por[0], ch);
    	else if (diff <= -15)
    	    send_to_char(por[1], ch);
    	else if (diff <= -10)
    	    send_to_char(por[2], ch);
    	else if (diff <= -5)
    	    send_to_char(por[3], ch);
    	else if (diff <= -2)
    	    send_to_char(por[4], ch);
    	else if (diff >= 25)
    	    send_to_char(por[10], ch);
    	else if (diff >= 15)
    	    send_to_char(por[9], ch);
    	else if (diff >= 10)
    	    send_to_char(por[8], ch);
    	else if (diff >= 5)
    	    send_to_char(por[7], ch);
    	else if (diff >= 2)
    	    send_to_char(por[6], ch);
    	else
    	    send_to_char(por[5], ch);

    	
        }
                            */
    /*diff = GET_AC(victim)/3 - (GET_AC(ch)/20;
    diffu += diff * 25;
    if (dane) {
    send_to_char("ARMOR        : ", ch);
    if (diff <= -25)
     send_to_char(por[0], ch);
    else if (diff <= -15)
     send_to_char(por[1], ch);
    else if (diff <= -10)
     send_to_char(por[2], ch);
    else if (diff <= -5)
     send_to_char(por[3], ch);
    else if (diff <= -2)
     send_to_char(por[4], ch);
    else if (diff >= 25)
     send_to_char(por[10], ch);
    else if (diff >= 15)
     send_to_char(por[9], ch);
    else if (diff >= 10)
     send_to_char(por[8], ch);
    else if (diff >= 5)
     send_to_char(por[7], ch);
    else if (diff >= 2)
     send_to_char(por[6], ch);
    else
     send_to_char(por[5], ch);
    send_to_char("------------------------------\r\n", ch);
}


    diff = diffu;
    */

    if (GET_SKILL(ch, SKILL_WARRIORCODE))
    {

        send_to_char("             You          Opponent\r\n             ---          --------\r\n", ch);
        if (GET_SKILL(ch, SKILL_WARRIORCODE)<95)
        {
            ch_printf(ch, "LEVEL:       %2d           %s\r\n", GET_LEVEL(ch), diff_txt[get_diff(GET_LEVEL(victim)-GET_LEVEL(ch))]);
            ch_printf(ch, "STR:         %2d           %s\r\n", GET_STR(ch), diff_txt[get_diff(GET_STR(victim)-GET_STR(ch))]);
            ch_printf(ch, "DEX:         %2d           %s\r\n", GET_DEX(ch), diff_txt[get_diff(GET_DEX(victim)-GET_DEX(ch))]);
            ch_printf(ch, "CON:         %2d           %s\r\n\r\n", GET_CON(ch), diff_txt[get_diff(GET_CON(victim)-GET_CON(ch))]);
        }
        else{
            ch_printf(ch, "LEVEL:       %2d           %d\r\n", GET_LEVEL(ch), GET_LEVEL(victim));
            ch_printf(ch, "STR:         %2d           %d\r\n", GET_STR(ch), GET_STR(victim));
            ch_printf(ch, "DEX:         %2d           %d\r\n", GET_DEX(ch), GET_DEX(victim));
            ch_printf(ch, "CON:         %2d           %d\r\n\r\n", GET_CON(ch), GET_CON(victim));
        }
        improve_skill(ch, SKILL_WARRIORCODE, 5);
    }
    if (diff <= -20)
        send_to_char("Now where did that chicken go?\r\n", ch);
    else if (diff <= -15)
        send_to_char("You could do it blind!\r\n", ch);
    else if (diff <= -10)
        send_to_char("Say the word and its history...\n\r", ch);
    else if (diff <= -5)
        send_to_char("Pretty easy.\r\n", ch);
    else if (diff <= -3)
        send_to_char("Easy.\r\n", ch);
    else if (diff <= -1)
        send_to_char("Fairly easy.\r\n", ch);
    else if (diff == 0)
        send_to_char("The perfect match!\r\n", ch);
    else if (diff <= 1)
        send_to_char("You would need some luck!\r\n", ch);
    else if (diff <= 2)
        send_to_char("You would need a lot of luck!\r\n", ch);
    else if (diff <= 3)
        send_to_char("You would need a hell of luck and great equipment!\r\n", ch);
    else if (diff <= 5)
        send_to_char("Do you feel *lucky*, punk?\r\n", ch);
    else if (diff <= 7)
        send_to_char("Bring some friends!\r\n", ch);
    else if (diff <= 10)
        send_to_char("Bring *LOTS* of friends!\r\n", ch);
    else if (diff <=20)
        send_to_char("You ARE mad!\r\n", ch);
    else send_to_char("Death slowly waves it's scythe..\r\n", ch);




}



ACMD(do_diagnose)
{
    struct char_data *vict;

    one_argument(argument, buf);

    if (*buf) {
        if (!(vict = get_char_room_vis(ch, buf))) {
            send_to_char(NOPERSON, ch);
            return;
        } else
            diag_char_to_char(vict, ch);
    } else {
        if (FIGHTING(ch))
            diag_char_to_char(FIGHTING(ch), ch);
        else
            send_to_char("Diagnose who?\r\n", ch);
    }
}


static char *ctypes[] = {
                            "off", "ON", "ON", "ON", "\n"};

ACMD(do_color)
{
    int tp;

    if (IS_NPC(ch))
        return;

    //    one_argument(argument, arg);

    if (!(*argument)) {
        send_to_char("Usage: color { off | on }\r\n", ch);
        sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
        send_to_char(buf, ch);
        return;
    }
    /*
    if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char("Usage: color { off | on }\r\n", ch);
    return;
}*/

    tp=0;
    if (isname("off",argument))
    {tp=0;
        REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
    }
    else if (isname("on",argument))
    {tp=3;
        SET_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
    }
    sprintf(buf, "&RYour &Gcolor &Bis &Ynow &P%s.&0\r\n",
            ctypes[tp]);
    send_to_char(buf, ch);
}


ACMD(do_toggle)
{

    if (IS_NPC(ch))
        return;

    sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

    sprintf(buf,
            "\r\n&PStrings\r\n&p-------&0\r\nWarcry        : &w%s&0\r\n"
            "Grats         : &w%s&0\r\n"
            "Walkin        : &w%s&0\r\n"
            "Walkout       : &w%s&0\r\n"
            "Prompt        : &w%s&0\r\n"
            "Combat prompt : &w%s&0\r\n\r\n\r\n"
            "           &PChannels                  Flags                 Actions&p\r\n"
            "           --------                  -----                 -------&0\r\n"
            "         Gossip:&c %-3s&0    "
            //"     Quest Flag:&c %-3s&0    "
            " Summon Protect:&c %-3s&0    "
            "      Autosplit:&c %-3s&0\r\n"

            "        Auction:&c %-3s&0    "
            " Visible on who:&c %-3s&0    "
            "        Autosac:&c %-3s&0\r\n"

            "           Info:&c %-3s&0    "
            "     Brief Mode:&c %-3s&0    "
            "        Autodir:&c %-3s&0\r\n"

            "            OOC:&c %-3s&0    "
            "   Compact Mode:&c %-3s&0    "
            "       Autoexit:&c %-3s&0\r\n"

            "          Arena:&c %-3s&0    "
            "    Repeat comm:&c %-3s&0    "
            "       Autoloot:&c %-3s&0\r\n"

            "          Quest:&c %-3s&0    "
            "     ANSI Color:&c %-3s&0    "
            "     Autoassist:&c %-3s&0\r\n"

            "          Tells:&c %-3s&0    "
            "        Wimp HP:&c %-3s&0    "
            "       Autograt:&c %-3s&0\r\n"

            "         Shouts:&c %-3s&0    "
            " Display damage:&c %-3s&0    "
            "        Automap:&c %-3s&0\r\n"

            "                        "
            "Friendly misses:&c %-3s&0    "
            "       Autosave:&c %-3s&0\r\n"

            "                        "
            "   Enemy misses:&c %-3s&0    "
            "                         \r\n"
            
            "                        "
            "     Skill desc:&c %-3s&0    "
            "                         \r\n"


            "\r\nSee 'help toggle' for more information.\r\n",

            ch->player_specials->saved.enrage,
            ch->player_specials->saved.grats,
            ch->player_specials->saved.walkin,
            ch->player_specials->saved.walkout,
            ch->player_specials->saved.prompt,
            ch->player_specials->saved.prompt_combat,


            ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
            //ONOFF(IS_QUESTOR(ch)),
            ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),
            ONOFF(PRF2_FLAGGED(ch, PRF2_AUTOSPLIT)),

            ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
            ONOFF(!PRF2_FLAGGED(ch, PRF2_NOWHO)),
            ONOFF(PRF_FLAGGED(ch, PRF_AUTOSAC)),

            ONOFF(!PRF2_FLAGGED(ch, PRF2_NOINFO)),
            ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
            ONOFF(PRF_FLAGGED(ch, PRF_AUTODIR)),

            ONOFF(!PRF_FLAGGED(ch, PRF_NOCHAT)),
            ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
            ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),

            ONOFF(!PRF_FLAGGED(ch, PRF_NOTEAM)),
            ONOFF(!PRF_FLAGGED(ch, PRF_NOREPEAT)),
            ONOFF(PRF2_FLAGGED(ch, PRF2_AUTOLOOT)),

            ONOFF(PRF_FLAGGED(ch, PRF_QUEST)),
            ctypes[COLOR_LEV(ch)],
            ONOFF(PRF2_FLAGGED(ch, PRF2_AUTOASSIST)),

            ONOFF(!PRF_FLAGGED(ch, PRF_NOTELL)),
            buf2,
            ONOFF(PRF2_FLAGGED(ch, PRF2_AUTOGRAT)),

            ONOFF(!PRF_FLAGGED(ch, PRF_DEAF)),

            ONOFF(PRF2_FLAGGED(ch, PRF2_DISPDAM)),
            ONOFF(PRF2_FLAGGED(ch, PRF2_AUTOMAP)),
            ONOFF(!PRF2_FLAGGED(ch, PRF2_NOMISSF)),
            ONOFF(PRF_FLAGGED(ch, PRF_AUTOSAVE)),
            ONOFF(!PRF2_FLAGGED(ch, PRF2_NOMISSE)),
            ONOFF(!PRF2_FLAGGED(ch, PRF2_DISPSDESC))
           );

    send_to_char(buf, ch);
}

struct sort_struct {
    int sort_pos;
    int is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;

void sort_commands(void)
{
    int a, b, tmp;
    char buf[128];

    ACMD(do_action);

    num_of_cmds = 0;

    /* first, count commands (num_of_commands is actually one greater than
       the number of commands; it includes the '\n'. */
    while (*cmd_info[num_of_cmds].command != '\n')
        num_of_cmds++;

    sprintf(buf, "The number of commands counted was: %d", num_of_cmds);
    log(buf);

    /* create data array */
    CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

    /* initialize it */
    for (a = 1; a < num_of_cmds; a++) {
        cmd_sort_info[a].sort_pos = a;
        cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
    }

    /* the infernal special case */
    cmd_sort_info[find_command("insult")].is_social = TRUE;

    /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
    for (a = 1; a < num_of_cmds - 1; a++)
        for (b = a + 1; b < num_of_cmds; b++)
            if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
                       cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
                tmp = cmd_sort_info[a].sort_pos;
                cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
                cmd_sort_info[b].sort_pos = tmp;
            }
}



ACMD(do_commands)
{
    int no, i, cmd_num, j=-1;
    int wizhelp = 0, socials = 0;
    struct char_data *vict;

    one_argument(argument, arg);

    if (*arg) {
        if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
            send_to_char("Who is that?\r\n", ch);
            return;
        }
        if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
            send_to_char("You can't see the commands of people above your level.\r\n", ch);
            return;
        }
    } else
        vict = ch;

    if (subcmd == SCMD_SOCIALS)
        socials = 1;
    else if (subcmd == SCMD_WIZHELP)
        wizhelp = 1;

    sprintf(buf, "The following %s%s are available to %s:\r\n",
            wizhelp ? "privileged " : "",
            socials ? "socials" : "commands",
            vict == ch ? "you" : GET_NAME(vict));

    /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
    for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
        i = cmd_sort_info[cmd_num].sort_pos;
        if (cmd_info[i].minimum_level != -2 &&
                GET_LEVEL(vict) >= cmd_info[i].minimum_level &&
                (cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
                (wizhelp || socials == cmd_sort_info[i].is_social)) {
            j=find_skill_num(cmd_info[i].command);
            if (j==-1 || GET_SKILL(ch, j))
            {
                sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
                if (!(no % 7))
                    strcat(buf, "\r\n");
                no++;
            }
        }
    }

    strcat(buf, "\r\n");
    send_to_char(buf, ch);
}

/* functions and macros for 'scan' command */
void list_scanned_chars(struct char_data * list, struct char_data * ch,
                        int distance, int door)
{
    const char *how_far[] = {
                                "Close by",
                                "Not too far",
                                "A ways off",
                                "far off to the"
                            };

    struct char_data *i;
    int count = 0;
    int first=1;
    *buf = '\0';

    /* this loop is a quick, easy way to help make a grammatical sentence
       (i.e., "You see x, x, y, and z." with commas, "and", etc.) */

    for (i = list; i; i = i->next_in_room)
        /* put any other conditions for scanning someone in this if statement -
           i.e., if (CAN_SEE(ch, i) && condition2 && condition3) or whatever */
        if (CAN_SEE(ch, i) && !IS_AFFECTED(ch, AFF_BLIND) &&  (IS_LIGHT((i)->in_room) || CAN_SEE_IN_DARK(ch)) && INVIS_OK(ch,i))
            count++;

    if (!count)
        return;

    for (i = list; i; i = i->next_in_room) {

        /* make sure to add changes to the if statement above to this one also, using
           or's to join them.. i.e.,
           if (!CAN_SEE(ch, i) || !condition2 || !condition3) */

        if ( !(!IS_AFFECTED(ch, AFF_BLIND) &&  (IS_LIGHT((i)->in_room) || CAN_SEE_IN_DARK(ch)) && INVIS_OK(ch,i)))
            continue;
        if (!CAN_SEE(ch, i))
            continue;
        /*    if (!IN_ARENA(ch))
            {
        	if (!*buf)
        	    sprintf(buf, "&c%s %s&0 you see &w%s&0", how_far[distance], dirs[door], GET_NAME(i));
        	else
        	    sprintf(buf, "%s&w%s&0", buf, GET_NAME(i));
            }
            else
            {
        	if (!*buf)
        	    sprintf(buf, "&c%s %s&0 you see &w%s&0", how_far[distance], dirs[door], (RED(i) ? "&RRED Team&0 member" : "&BBLUE Team&0 member"));
        	else
          	    sprintf(buf, "%s&w%s&0", buf, (RED(i) ? "&RRED Team&0 member" : "&BBLUE Team&0 member"));
            }*/


        if (!*buf)
        {
            if (distance==-1)	// Here
                sprintf(buf1,  "%-10s      ", "Here");
            else if (!distance)
                sprintf(buf1,  "%-10s      ", dirs[door]);
            else
                sprintf(buf1,  "%-10s (%d)  ", dirs[door], distance+1);
            *buf1=UPPER(*buf1);
            ch_printf(ch, "&w%s&0", buf1);
        }
    
    *buf2=0;    
    if (GET_LEVEL(ch)-GET_LEVEL(i)>5)
    	strcpy(buf2, "&w");
    else if (GET_LEVEL(ch)-GET_LEVEL(i)>=0)
    	strcpy(buf2, "&y");
    else if (GET_LEVEL(ch)-GET_LEVEL(i)>=-5)
    	strcpy(buf2, "&Y");    	
    else 
    	strcpy(buf2, "&P");    		
        
        
        sprintf(buf1, "%s%s", buf2, GET_NAME(i));
        
        if (first)
        {
            *buf1=UPPER(*buf1);
            first=0;
        }

        if (IS_NPC(i))
        {
            if (!IN_ARENA(ch))
                //sprintf(buf, "%s&c%s&0", buf, buf1);
                sprintf(buf, "%s%s&0", buf, buf1);
            else
                sprintf(buf, "%s%s%s member&0", buf, (RED(i) ? "&RRED Team&0" : "&BBLUE Team&0"), buf2);
                
        }
        else if (!IN_ARENA(ch))
//            sprintf(buf, "%s&C%s&0", buf, buf1);
            sprintf(buf, "%s%s&0", buf, buf1);
        else
            //sprintf(buf, "%s&C%s&0", buf, (RED(i) ? "&RRED Team&0 member" : "&BBLUE Team&0 member"));
            sprintf(buf, "%s%s%s member&0", buf, (RED(i) ? "&RRED Team&0" : "&BBLUE Team&0"), buf2);


        if (--count > 1)
            strcat(buf, ", ");
        else if (count == 1)
            strcat(buf, " and ");
        else {
            strcat(buf, "\r\n");
        }


    }
    *buf=UPPER(*buf);
    send_to_char(buf, ch);
}




ACMD(do_scan)
{
    /* >scan You quickly scan the area. You see John, a large horse and
       Frank close by north. You see a small rabbit a ways off south. You
       see a huge dragon and a griffon far off to the west.

    */
    int door;

    *buf = '\0';

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
        return;
    }
    /* may want to add more restrictions here, too */
    send_to_char("You quickly scan the area.\r\n\r\n", ch);
    send_to_char("Where           Scanned\r\n-----           -------\r\n", ch);

    act("$n quickly scans the area.", FALSE, ch, 0, 0, TO_ROOM);

    list_scanned_chars(world[ch->in_room].people, ch, -1, 0);
    for (door = 0; door < NUM_OF_DIRS; door++)	/* don't scan up/down */
        if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
                !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) && !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
            if (world[EXIT(ch, door)->to_room].people) {
                list_scanned_chars(world[EXIT(ch, door)->to_room].people, ch, 0, door);
            } if ((GET_SKILL(ch, SKILL_ENH_SIGHT) > 33) && _2ND_EXIT(ch, door) && _2ND_EXIT(ch, door)->to_room !=
                    NOWHERE && !IS_SET(_2ND_EXIT(ch, door)->exit_info, EX_CLOSED) && !IS_SET(_2ND_EXIT(ch, door)->exit_info, EX_HIDDEN)) {
                /* check the second room away */
                if (world[_2ND_EXIT(ch, door)->to_room].people) {
                    list_scanned_chars(world[_2ND_EXIT(ch, door)->to_room].people, ch, 1, door);
                } if ((GET_SKILL(ch, SKILL_ENH_SIGHT) > 66) && _3RD_EXIT(ch, door) && _3RD_EXIT(ch, door)->to_room !=
                        NOWHERE && !IS_SET(_3RD_EXIT(ch, door)->exit_info, EX_CLOSED) && !IS_SET(_3RD_EXIT(ch, door)->exit_info, EX_HIDDEN)) {
                    /* check the third room */
                    if (world[_3RD_EXIT(ch, door)->to_room].people) {
                        list_scanned_chars(world[_3RD_EXIT(ch, door)->to_room].people, ch, 2, door);
                    }
                }
            }
        }
}


int list_autoscanned_chars(struct char_data * list, struct char_data * ch,
                           int door, int hv)
{
    struct char_data *i;
    int count = 0;
    *buf = '\0';


    for (i = list; i; i = i->next_in_room)
        if (CAN_SEE(ch, i) && !IS_AFFECTED(ch, AFF_BLIND) &&  (IS_LIGHT((i)->in_room) || CAN_SEE_IN_DARK(ch)) && INVIS_OK(ch,i))
            count++;

    if (!count)
        return 0;
    if (!hv)
        send_to_char("\r\n",ch);
    for (i = list; i; i = i->next_in_room) {

        if (!CAN_SEE(ch, i)  || !(!IS_AFFECTED(ch, AFF_BLIND) &&  (IS_LIGHT((i)->in_room) || CAN_SEE_IN_DARK(ch)) && INVIS_OK(ch,i)))
            continue;
        if (!IN_ARENA(ch))
	{
		
	*buf1=0;    
    if (GET_LEVEL(ch)-GET_LEVEL(i)>5)
    	strcpy(buf1, "&w");
    else if (GET_LEVEL(ch)-GET_LEVEL(i)>=0)
    	strcpy(buf1, "&y");
    else if (GET_LEVEL(ch)-GET_LEVEL(i)>=-5)
    	strcpy(buf1, "&Y");    	
    else 
    	strcpy(buf1, "&P");    		
    		
		
            if (!*buf)
                //sprintf(buf, "  %-10s : &y%s&0", dirs[door], GET_NAME(i));
                sprintf(buf, "  %-10s : %s%s&0", dirs[door], buf1,GET_NAME(i));
            else
                //sprintf(buf, "%s&y%s&0", buf, GET_NAME(i));
                sprintf(buf, "%s%s%s&0", buf,  buf1, GET_NAME(i));
        }
        else
        {                                             
        	
            if (!*buf)
                //sprintf(buf, "  %-10s : %s", dirs[door],    (RED(i) ? "&RRED Team&0 member" : "&BBLUE Team&0 member"));
                sprintf(buf, "  %-10s : %s %smember&0", dirs[door],    (RED(i) ? "&RRED Team&0" : "&BBLUE Team&0"), buf1);
            else
                sprintf(buf, "%s%s%s member&0", buf,(RED(i) ? "&RRED Team&0" : "&BBLUE Team&0"), buf1);
        }

        if (--count > 0)
            strcat(buf, ", ");
        else
            strcat(buf, "\r\n");

    }

    send_to_char(buf, ch);
    return 1;
}

void autoscan(struct char_data *ch)
{
    int door, hv=0;

    for (door = 0; door < NUM_OF_DIRS ; door++)	/* don't scan up/down */
        if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
                !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) && !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
            if (world[EXIT(ch, door)->to_room].people)
                hv=list_autoscanned_chars(world[EXIT(ch, door)->to_room].people, ch, door, hv);

}


ACMD(do_auto)
{
    argument = one_argument(argument, buf);
    if (*buf == 'a') {
        SET_BIT(PRF_FLAGS(ch), PRF_AUTOEXIT | PRF_AUTODIR | PRF_AUTOSAC | PRF_AUTOSAVE);
        SET_BIT(PRF2_FLAGS(ch), PRF2_AUTOSPLIT | PRF2_AUTOLOOT | PRF2_AUTOGRAT | PRF2_AUTOASSIST | PRF2_AUTOSCAN | PRF2_AUTOMAP);
        REMOVE_BIT(PLR_FLAGS(ch), PLR_NOSETTITLE);
    }
    if (*buf == 'n') {
        REMOVE_BIT(PRF_FLAGS(ch), PRF_AUTOEXIT | PRF_AUTODIR | PRF_AUTOSAC | PRF_AUTOSAVE);
        REMOVE_BIT(PRF2_FLAGS(ch), PRF2_AUTOSPLIT | PRF2_AUTOLOOT | PRF2_AUTOGRAT | PRF2_AUTOASSIST | PRF2_AUTOSCAN | PRF2_AUTOMAP);
        SET_BIT(PLR_FLAGS(ch), PLR_NOSETTITLE);
    }
    send_to_char(PRF_FLAGGED(ch, PRF_AUTOEXIT)
                 ? "[&C+ AUTOEXIT&0  ] You'll see the list of visible exits.\r\n"
                 : "[&c- autoexit&0  ] You don't see exits.\r\n"
                 ,ch);

    send_to_char(PRF_FLAGGED(ch, PRF_AUTODIR)
                 ? "[&C+ AUTODIR&0   ] You get brief list of visible exits.\r\n"
                 : "[&c- autodir&0   ] You don't get brief list of exits.\r\n"
                 ,ch);

    send_to_char(PRF2_FLAGGED(ch, PRF2_AUTOLOOT)
                 ? "[&C+ AUTOLOOT&0  ] You auto-loot corpses.\r\n"
                 : "[&c- autoloot&0  ] You don't auto-loot corpses.\r\n"
                 ,ch);

    send_to_char(PRF_FLAGGED(ch, PRF_AUTOSAC)
                 ? "[&C+ AUTOSAC&0   ] You auto-sacrifice corpses.\r\n"
                 : "[&c- autosac&0   ] You don't auto-sacrifice corpses.\r\n"
                 ,ch);

    send_to_char(PRF2_FLAGGED(ch, PRF2_AUTOSPLIT)
                 ? "[&C+ AUTOSPLIT&0 ] You automatically split coins among your group members.\r\n"
                 : "[&c- autosplit&0 ] You don't automatically split coins.\r\n"
                 ,ch);


    send_to_char(PRF2_FLAGGED(ch, PRF2_AUTOASSIST)
                 ? "[&C+ AUTOASSIST&0] You auto-assist the leader. Your pets will auto-assist you.\r\n"
                 : "[&c- autoassist&0] You don't auto-assist. Your pets will not auto-assist you.\r\n"
                 ,ch);


    /*	send_to_char(!(  PLR_FLAGGED( ch, PLR_NOSETTITLE)   )
    	    ? "[+ AUTOTITLE ] Title will change when advancing a level.\r\n"
    	    : "[- autotitle ] Title won't change when advancing a level.\r\n"
    	    , ch );
    */
    send_to_char(PRF2_FLAGGED(ch, PRF2_AUTOGRAT)
                 ? "[&C+ AUTOGRAT&0  ] You automatically congratulate players when they level.\r\n"
                 : "[&c- autograt&0  ] You don't automatically congratulate.\r\n"
                 ,ch);

    send_to_char(PRF2_FLAGGED(ch, PRF2_AUTOSCAN)
                 ? "[&C+ AUTOSCAN&0  ] You automatically scan when you move.\r\n"
                 : "[&c- autoscan&0  ] You don't automatically scan when you move.\r\n"
                 ,ch);

    send_to_char(PRF2_FLAGGED(ch, PRF2_AUTOMAP)
                 ? "[&C+ AUTOMAP&0   ] You see the automap.\r\n"
                 : "[&c- automap&0   ] You don't see automap.\r\n"
                 ,ch);

    send_to_char(PRF_FLAGGED(ch, PRF_AUTOSAVE)
                 ? "[&C+ AUTOSAVE&0  ] Your character is autosaved regulary.\r\n"
                 : "[&c- autosave&0  ] Your character isn't autosaved.\r\n"
                 ,ch);


    send_to_char("\r\n&wAUTO all,  AUTO none&0\r\n", ch);
}


ACMD(do_compare)
{
    struct obj_data *obj1, *obj2;
    char *msg;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    float value1, value2;
    int ii;
    int mode;
    struct char_data *tmp_char;
    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0') {
        send_to_char("Compare what to what?\r\n", ch);
        return;
    }
    mode = generic_find(arg1, FIND_OBJ_INV, ch, &tmp_char, &obj1);
    if (!obj1) {
        send_to_char("You do not have that in your inventory.\r\n", ch);
        return;
    }
    if (arg2[0] == '\0') {
        /* for (obj2 = ch->carrying; obj2; obj2 = obj2->next_content) */
        for (ii = 0; ii < NUM_WEARS; ii++) {
            obj2 = GET_EQ(ch, ii);
            if (obj2 && CAN_SEE_OBJ(ch, obj2)
                    && obj1->obj_flags.type_flag == obj2->obj_flags.type_flag
                    && (obj1->obj_flags.wear_flags & obj2->obj_flags.wear_flags & ~ITEM_WEAR_TAKE) != 0)
                break;
        }

        if (!obj2) {
            send_to_char("You aren't wearing anything comparable.\r\n", ch);
            return;
        }
    } else {
        mode = generic_find(arg2, FIND_OBJ_INV, ch, &tmp_char, &obj2);
        if (!obj2) {
            send_to_char("You do not have the second item in your inventory.\r\n", ch);
            return;
        }
        /*	if (!obj2)
        	{
        	    char new_arg2 [ MAX_INPUT_LENGTH ];
        	    int  number;

        	    number = number_argument( arg2, arg2 );
        	    if ( number > 1 )  number--;
        	    sprintf( new_arg2, "%d.%s", number, arg2 );
        	    mode = generic_find(new_arg2, FIND_OBJ_EQUIP, ch, &tmp_char, &obj2);
        	    if ( !(obj2) )
        	    {
        		send_to_char( "You do not have that item.\r\n", ch );
        		return;
        	    }
        	
        	    if ( ( obj1->obj_flags.wear_flags & obj2->obj_flags.wear_flags & ~ITEM_WEAR_TAKE ) == 0 )
        	    {
        		send_to_char( "They are not comparable items.\r\n", ch );
        		return;
        	    }

        	}*/
    }

    if ((obj1->obj_flags.wear_flags & obj2->obj_flags.wear_flags & ~ITEM_WEAR_TAKE) == 0) {
        send_to_char("They are not comparable items.\r\n", ch);
        return;
    }
    msg = NULL;
    value1 = 0;
    value2 = 0;

    if (obj1 == obj2) {
        msg = "You compare $p to itself. Hmmm...It looks about the same!";
    } else if (obj1->obj_flags.type_flag != obj2->obj_flags.type_flag) {
        msg = "You can't compare $p and $P.";
    } else {
        switch (obj1->obj_flags.type_flag) {
        default:
            msg = "You are unable to compare $p and $P.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->obj_flags.value[0];
            value2 = obj2->obj_flags.value[0];
            break;

        case ITEM_WEAPON:
            value1 = (float) (obj1->obj_flags.value[1] * ((float) (obj1->obj_flags.value[2] + 1) / 2) + obj1->obj_flags.value[0]);
            value2 = (float) (obj2->obj_flags.value[1] * ((float) (obj2->obj_flags.value[2] + 1) / 2) + obj2->obj_flags.value[0]);
            break;
        }
    }

    if (!msg) {
        /*	if (find_eq_pos(ch, obj2, 0) > 0)
                {
                         if ( value1 == value2 )
                             msg = "$p and $P (equipped) look about the same.";
                   else if ( value1  > value2 )
                             msg = "$p looks better than $P (equipped).";
                    else
                             msg = "$p looks worse than $P (equipped).";
                }
                else*/
        {
            if (value1 == value2)
                msg = "$p and $P look &cabout the same&0.";
            if (value1 > value2)
                msg = "$p looks &Bbetter&0 than $P.";
            if (value2 > value1)
                msg = "$p looks &Rworse&0 than $P.";
        }

    }
    act(msg, FALSE, ch, obj1, obj2, TO_CHAR);
    return;
}


ACMD(do_gwho)
{
    struct descriptor_data *i;
    struct char_data *person;
    struct follow_type *f;
    int count = 0, gcnt = 0;
    char buf[MAX_STRING_LENGTH * 2], tbuf[MAX_STRING_LENGTH], pbuf[MAX_STRING_LENGTH];

    sprintf(buf, "&GFearsome Groups\r\n&g===============\r\n&0");

    /* go through the descriptor list */
    for (i = descriptor_list; i; i = i->next) {
        /* find everyone who is a master  */
        if (!i->connected) {
            person = (i->original ? i->original : i->character);
            *pbuf = '\0';
            /* list the master and the group name */
            if (!person->master && IS_AFFECTED(person, AFF_GROUP)) {
                if (person->player_specials->group_name && CAN_SEE(ch, person)) {

                    sprintf(pbuf, "\r\n&B-->&0      &Y%s&0\r\n&c[%s] %s&0\r\n", person->player_specials->group_name
                            ,CLASS_ABBR(person), GET_NAME(person));
                    /* list the members that ch can see */
                    count = 0;
                    for (f = person->followers; f; f = f->next) {
                        if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower)) {
                            count++;
                            if (CAN_SEE(ch, f->follower) && strlen(GET_NAME(f->follower)) > 1) {
                                sprintf(tbuf, "&c[%s] %s&0\r\n", CLASS_ABBR(f->follower), GET_NAME(f->follower));
                                strcat(pbuf, tbuf);
                            } else {
                                sprintf(tbuf, "      &cSomeone&0\r\n");
                                strcat(pbuf, tbuf);
                            }
                        }
                    }
                    /* if there are no group members, then remove the
                       group title */
                    if (count < 1) {
                        if (person->player_specials->group_name)
                            DISPOSE(person->player_specials->group_name);
                        person->player_specials->group_name = 0;
                    } else {
                        gcnt++;
                        strcat(buf, pbuf);
                    }

                }
            }
        }
    }
    if (!gcnt)
        strcat(buf, "\r\nNone.\r\n");
    page_string(ch->desc, buf, 1);
}

ACMD(do_name)
{
    int count;
    struct follow_type *f;

    if (ch->master || !IS_AFFECTED(ch, AFF_GROUP)) {
        send_to_char("You aren't the master of a group.\r\n", ch);
        return;
    }
    /* check to see at least 2 pcs in group      */
    for (count = 0, f = ch->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower)) {
            count++;
        }
    }
    if (count < 1) {
        send_to_char("You can't have a group name with just one player!\r\n", ch);
        return;
    }
    /* free the old ch->specials.group_name           */
    if (ch->player_specials->group_name) {
        DISPOSE(ch->player_specials->group_name);
        ch->player_specials->group_name = 0;
    }
    /* set ch->specials.group_name to the argument    */
    for (; *argument == ' '; argument++);
    send_to_char("\r\nSetting your group name to: &C", ch);
    send_to_char(argument, ch);
    send_to_char("&0\r\n", ch);
    ch->player_specials->group_name = str_dup(argument);
    for (f = ch->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower)) {
            send_to_char("\r\nSetting your group name to: &C", f->follower);
            send_to_char(argument, f->follower);
            send_to_char("&0\r\n", f->follower);
            if (f->follower->player_specials->group_name)
                DISPOSE(f->follower->player_specials->group_name);
            f->follower->player_specials->group_name = str_dup(argument);
        }
    }

}


ACMD (do_mood)
{
    send_to_char(get_mood(ch),ch);
}

ACMD(do_graph)
{
    char bufg[1000]="\0";
    char buf_1[80],buf_2[80],buf_3[80],buf_4[80], buf_5[80];
    sprintf(bufg, "&Y%s, %d year old %s %s, level %d\r\n&y----------------------------------------------------------------------------&0\r\n\r\n",
            GET_NAME(ch),
            GET_AGE(ch),
            pc_race_types[(int) GET_RACE(ch)],
            pc_class_types[GET_CLASS_NUM(ch)],
            GET_LEVEL(ch),        GET_NAME(ch),
            GET_AGE(ch),
            pc_race_types[(int) GET_RACE(ch)],
            pc_class_types[GET_CLASS_NUM(ch)],
            GET_LEVEL(ch));
    sprintf(bufg,"%s&B%s &0Hit      Evil &G%s&0 Good\r\n&B%s&0 Mana\r\n&B%s&0 Move      Exp &C%s&0\r\n",
            bufg,
            make_bar(buf_1, GET_HIT(ch)*100/(GET_MAX_HIT(ch)==0? 0.1 :GET_MAX_HIT(ch)) ,28),
            make_a_bar(buf_5, -1000, 1000, GET_ALIGNMENT(ch), 28),
            make_bar( buf_2, GET_MANA(ch)*100/(GET_MAX_MANA(ch)==0 ? 0.1 : GET_MAX_MANA(ch)),28),
            make_bar( buf_3, GET_MOVE(ch)*100/(GET_MAX_MOVE(ch)==0 ? 0.1 : GET_MAX_MOVE(ch)),28),
            //make_bar( buf_4, (GET_EXP(ch)-total_exp(GET_LEVEL(ch)-1))*100/(total_exp(GET_LEVEL(ch))-total_exp(GET_LEVEL(ch)-1)),28) );
            make_bar( buf_4, GET_EXP(ch)*100/LEVELEXP(ch),28) );
    send_to_char(bufg, ch);
}


ACMD(do_zone)
{
    char bufg[1000]="\0";

    sprintf(bufg,"You are currently somewhere in the &w%s.&0\r\n", zone_table[world[ch->in_room].zone].name);
    send_to_char(bufg,ch);
}



int att_bonus(struct char_data *ch)
{
    int type=TYPE_HIT;
    float att, att1, i, j;
    struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);


    if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
        type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
    else if (wielded && GET_OBJ_TYPE(wielded) == ITEM_FIREWEAPON)
        type = TYPE_POUND;

    att=GET_SKILL(ch, type)+2*GET_HITROLL(ch)-2*GET_COND(ch, DRUNK);
    if (wielded)                   
    {
        att1= ((GET_SKILL(ch, type)-5)/2-(MIN(LVL_IMMORT-1, GET_OBJ_LEVEL(wielded))));
        if (att1>0)
            att1=att1*att1/5;
        else
            att1=-(att1*att1/5);

        att+=MAX(-300, MIN(50, (int) (10.0*att1)));   
     }   
    att-=50*((IS_EQUIP_W(ch)+IS_CARRYING_W(ch))/CAN_CARRY_W(ch))*((IS_EQUIP_W(ch)+IS_CARRYING_W(ch))/CAN_CARRY_W(ch));

    if (GET_RACE(ch)==RACE_TROLL && type==TYPE_POUND)   // umesto ovoga bi trebao bonus po nastanku
        att+=10;
    else if (GET_RACE(ch)==RACE_ORC && type==TYPE_SLASH)
        att+=10;
    else if (GET_RACE(ch)==RACE_DWARF && world[IN_ROOM(ch)].sector_type == SECT_INSIDE)
        att+=10;
    
    att+=9*(GET_DEX(ch)-16.0);
    	
   /* if (GET_STYLE(ch)==2)
        att+=20*GET_SKILL(ch, SKILL_AGGRESIVE)/100.0;
    else if (GET_STYLE(ch)==1 )
          att-=20*GET_SKILL(ch, SKILL_EVASIVE)/100.0; */
    return (int) (att/5.0);
}


extern struct sclass class_app[];
ACMD(do_common)
{
    char buf_1[80],buf_2[80],buf_3[80],buf_4[80], buf_5[80],buf_6[80],buf_7[80], buf_00[80],buf_01[80];
    sprintf(buf, "&CCombat skills:\r\n&c==============&0\r\n\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n&w%18s&0: [&c%s&0] &y%2d%%&0\r\n",
            spells[SKILL_AGGRESIVE], make_bar(buf_00, GET_SKILL(ch, SKILL_AGGRESIVE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33), GET_SKILL(ch, SKILL_AGGRESIVE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_EVASIVE], make_bar(buf_01, GET_SKILL(ch, SKILL_EVASIVE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33),GET_SKILL(ch, SKILL_EVASIVE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_DODGE], make_bar(buf_1, GET_SKILL(ch, SKILL_DODGE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33), GET_SKILL(ch, SKILL_DODGE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_PARRY], make_bar(buf_2, GET_SKILL(ch, SKILL_PARRY)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33),GET_SKILL(ch, SKILL_PARRY)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_SHIELD], make_bar(buf_3, GET_SKILL(ch, SKILL_SHIELD)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33),GET_SKILL(ch, SKILL_SHIELD)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_HIT], make_bar(buf_4, GET_SKILL(ch, SKILL_HIT)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33),GET_SKILL(ch, SKILL_HIT)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_SLASH], make_bar(buf_5, GET_SKILL(ch, SKILL_SLASH)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33),GET_SKILL(ch, SKILL_SLASH)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_POUND], make_bar(buf_6, GET_SKILL(ch, SKILL_POUND)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33),GET_SKILL(ch, SKILL_POUND)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,
            spells[SKILL_PIERCE], make_bar(buf_7, GET_SKILL(ch, SKILL_PIERCE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat,33),GET_SKILL(ch, SKILL_PIERCE)*100/class_app[(GET_CLASS_NUM(ch))].maxcombat);
    send_to_char(buf, ch);

    sprintf(buf, "\r\n&wCalculated attack bonus: &y%d&0\r\n", att_bonus(ch));
    send_to_char(buf, ch);
    /*sprintf(buf,"\r\nCommon skills:\r\n\r\n%25s %3d\r\n%25s %3d\r\n",
    spells[SKILL_RIDING], GET_SKILL(ch, SKILL_RIDING),
    spells[SKILL_SWIMMING], GET_SKILL(ch, SKILL_SWIMMING));
    send_to_char(buf, ch);*/
}


ACMD(do_leech)
{
    int num;
    argument = one_argument(argument, buf);
    if (!*buf)
        show_leech(ch);
    else if (isname("off", buf))
    {
        argument = one_argument(argument, buf);
        if ( (is_number(buf) || isname("all", buf)))
        {
            num=(is_number(buf)? atoi(buf):123454321);
            leech_from_char(ch, -num);
        }
        else
            send_to_char("Use 'leech off <num>' tu cut the mana supply for spell <num>.\r\nOr you can use 'leech off all'.\r\n", ch);

    }
    else
        send_to_char("Use 'leech off <num>' tu cut the mana supply for spell <num>.\r\nOr you can use 'leech off all'.", ch);

}
char *acdesc[]=
    {
        "no armor",
        "barely armored",
        "lightly armored",
        "medium armored",
        "fairly well armored",
        "well armored",
        "quite well armored",
        "extremely well armored",
        "DRAGON-like armor"
    };

ACMD(do_armor)
{
    int a1, a2, a3, a4;
    struct obj_data *obj;
    a1=a2=a3=a4=0;
    /*
    if ((obj=GET_EQ(ch, WEAR_HEAD)))
{
    	if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    		a1=MAX(1,MIN(8,GET_OBJ_VAL(obj, 0)/2));
}

    if ((obj=GET_EQ(ch, WEAR_ARMS)))
{
    	if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    		a2=MAX(1,MIN(8,GET_OBJ_VAL(obj, 0)/2));
}
    if ((obj=GET_EQ(ch, WEAR_BODY)))
{
    	if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    		a3=MAX(1,MIN(8,GET_OBJ_VAL(obj, 0)/2));
}
    if ((obj=GET_EQ(ch, WEAR_LEGS)))
{
    	if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    		a4=MAX(1,MIN(8,GET_OBJ_VAL(obj, 0)/2));
}

    send_to_char("&CArmor:\r\n&c======&0\r\n\r\n", ch);
    sprintf(buf,"\r\n%4s: [&c%s&0]\r\n%4s: [&c%s&0]\r\n%4s: [&c%s&0]\r\n%4s: [&c%s&0]\r\n\r\nTotal Armor Protection: &C%s&0\r\n",
    "Head", acdesc[a1],
    "Arms", acdesc[a2],
    "Body", acdesc[a3],
    "Legs", acdesc[a4], armor_desc(GET_AC(ch)));
    send_to_char(buf, ch);                      */

    ch_printf(ch, "\r\nPhysical armor: &c%d&0\r\nMagical armor : &c%d&0\r\n", GET_AC(ch), GET_MAGAC(ch));

}


ACMD(do_replay)
{
    int i;
    *buf=0;
    for (i=0;i<MAX_TELLS;i++)
        if (ch->last_tells[i])
            sprintf(buf, "%s%s\r\n", buf, ch->last_tells[i]);
    page_string(ch->desc, buf, 1);
}



extern struct s_topdam topdam[10], tophurt[10];
struct s_topdam topdam_class[10][10], tophurt_class[10][10];

extern void save_topdam();
extern void save_tophurt();

ACMD(do_topdam)
{
    int i;
    struct s_topdam *pom;
    int klasa=-1;
    argument = one_argument(argument, buf);
    if (!*buf)
    {
        send_to_char("\r\n&Y                             T O P   D A M A G E S \r\n                             =====================&0\r\n\r\n\r\n",ch);
        pom=topdam;
    }
    else
    {
        if (isname(buf, "warrior"))
            klasa=3;
        else if (isname(buf, "thief"))
            klasa=2;
        else if (isname(buf, "mage"))
            klasa=0;
        else if (isname(buf, "priest"))
            klasa=10;
        else if (isname(buf, "druid"))
            klasa=6;
        else if (isname(buf, "ranger"))
            klasa=4;
        else if (isname(buf, "monk"))
            klasa=8;
        
        
        if (klasa==-1)
        {
            send_to_char("That's not a valid class.\r\n", ch);
            return;
        }
        ch_printf(ch, "\r\n&Y                             T O P   D A M A G E S \r\n                             =====================\r\n                                     %s&0\r\n\r\n\r\n",pc_class_types[klasa]);
        pom=topdam_class[klasa];
    }




    for (i=0;i<10;i++)
    {
        if (!strlen(pom[i].ch) || !pom[i].dam)
            break;
        sprintf(buf, "  &w%2d.&0 &c%-15s&0  &c%4d&0  (%s using %s)\r\n", i+1, pom[i].ch,  pom[i].dam,pom[i].vict, spells[pom[i].skill]);
        send_to_char(buf, ch);
        if (i==2)
            send_to_char ("\r\n", ch);
    }

    save_topdam();

}

ACMD(do_tophurt)
{
    int i;
    struct s_topdam *pom;
    int klasa=-1;
    argument = one_argument(argument, buf);
    if (!*buf)
    {
        send_to_char("\r\n&Y                                T O P   H U R T\r\n                                ===============&0\r\n\r\n\r\n",ch);
        pom=tophurt;
    }
    else
    {
        if (isname(buf, "warrior"))
            klasa=3;
        else if (isname(buf, "thief"))
            klasa=2;
        else if (isname(buf, "mage"))
            klasa=0;
        else if (isname(buf, "priest"))
            klasa=10;
        else if (isname(buf, "druid"))
            klasa=6;
        else if (isname(buf, "ranger"))
            klasa=4;
        else if (isname(buf, "monk"))
            klasa=8;
        
        if (klasa==-1)
        {
            send_to_char("That's not a valid class.\r\n", ch);
            return;
        }
        ch_printf(ch, "\r\n&Y                                T O P   H U R T\r\n                                ===============\r\n                                     %s&0\r\n\r\n\r\n",pc_class_types[klasa]);
        pom=tophurt_class[klasa];
    }

    for (i=0;i<10;i++)
    {
        if (!strlen(pom[i].ch) || !pom[i].dam)
            break;
        sprintf(buf, "  &w%2d.&0 &c%-15s&0  &c%4d&0  (%s using %s)\r\n", i+1, pom[i].ch,  pom[i].dam,pom[i].vict, spells[pom[i].skill]);
        send_to_char(buf, ch);
        if (i==2)
            send_to_char ("\r\n", ch);
    }

    save_tophurt();
}
