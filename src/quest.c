/* ************************************************************************
*   File: quest.c				   A utility to CircleMUD *
*  Usage: writing/reading player's quest bits                             *
*                                                                         *
*  Code done by Billy H. Chan  				   		  *
*                                                                         *
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

int             real_mobile(int virtual);

#define IS_QUESTOR(ch)     (PLR_FLAGGED(ch, PLR_QUESTOR))

/* Object vnums for Quest Rewards */
/* on Reimsmud just use item vnums in the godroom range: 1200's */
#define QUEST_ITEM0 25000
#define QUEST_ITEM1 25001
#define QUEST_ITEM2 25002
#define QUEST_ITEM3 25003
#define QUEST_ITEM4 25004
#define QUEST_ITEM5 25005
#define QUEST_ITEM6 25006
#define QUEST_ITEM7 25016
#define QUEST_ITEM8 25017
#define QUEST_ITEM9 25018
#define QUEST_ITEM10 25019
#define QUEST_ITEM11 25020
#define QUEST_ITEM12 25021
#define QUEST_ITEM13 25022

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST_FIRST 25007
#define QUEST_OBJQUEST_LAST  25015

#define GET_QUESTGIVER(ch) get_char_num(real_mobile(GET_QUESTG(ch)))
/* Local functions */

void            generate_quest(struct char_data * ch, struct char_data * questman);
void            quest_update(void);
int             chance(int num);
void            send_to_char(char *messg, struct char_data * ch);
struct char_data *get_char_vis(struct char_data * ch, char *arg1);
extern int current_quest;

struct obj_data *create_object(int vnum, int dummy)
{
    int             r_num;
    struct obj_data *tobj;

    if ((r_num = real_object(vnum)) < 0)
        tobj = NULL;
    else
        tobj = read_object(r_num, REAL, 0, dummy+1);
    return (tobj);
}


/* CHANCE function. I use this everywhere in my code, very handy :> */

int             chance(int num)
{
    if (number(1, 100) <= num)
        return 1;
    else
        return 0;
}

/* The main quest function */

ACMD(do_autoquest)
{
    char            arg2[MAX_STRING_LENGTH],
    arg1[MAX_STRING_LENGTH],
    buf[MAX_STRING_LENGTH];
    struct char_data *questman = NULL;
    struct obj_data *obj = NULL,
                                       *obj_next = NULL;
    struct obj_data *questinfoobj = NULL;
    struct char_data *questinfo = NULL;
    two_arguments(argument, arg1, arg2);
    if (!arg1 || !*arg1)
{
        send_to_char("QUEST commands: JOIN POINTS INFO TIME REQUEST COMPLETE LIST BUY IDENTIFY.\r\n", ch);
        send_to_char("For more information, type 'HELP QUEST'.\r\n", ch);
        return;
    }
    
   if (isname(arg1, "join")) {
        if (QUESTING(ch))

            send_to_char("You are already signed up for a quest.\r\n", ch);

        else if (current_quest==QUEST_NONE)
        {
        	send_to_char("There is no quest running at this time.\r\n", ch);
        	
	}
	else if (quest_step==QUEST_ACTION)
        {
        	send_to_char("The quest already begun, you can not join now!\r\n", ch);
        	
	}
	else 
        {
            SET_BIT(PRF_FLAGS(ch), PRF_QUEST);
            //send_to_char("OK.\r\n", ch);
            sprintf(buf, "%s signed up for a quest!", GET_NAME(ch));
            questchan(buf);
            
        }
        return;
    }
    if (isname(arg1, "info")) {
        if (IS_QUESTOR(ch)) {
            if ((GET_QUESTMOB(ch) == -1) &&
                    ((questman = GET_QUESTGIVER(ch)) != NULL)) {
                sprintf(buf, "Your quest is ALMOST complete!\r\nGet back to %s before your time runs out!\r\n",
                        GET_NAME(GET_QUESTGIVER(ch)));
                send_to_char(buf, ch);
            } else if (GET_QUESTOBJ(ch) > 0) {
                questinfoobj = get_obj_num(real_object(GET_QUESTOBJ(ch)));
                if (questinfoobj != NULL) {
                    sprintf(buf, "&cYou are on a quest to recover &C%s!&0\r\n", questinfoobj->name);
                    send_to_char(buf, ch);
                } else
                {
                    send_to_char("You aren't currently on a quest.\r\n", ch);
                    if (IS_QUESTOR(ch))
                    {
                        REMOVE_BIT(PLR_FLAGS(ch), PLR_QUESTOR);
                        GET_QUESTG(ch) = 0;
                        GET_COUNTDOWN(ch) = 0;
                        GET_QUESTMOB(ch) = 0;
                        GET_QUESTOBJ(ch) = 0;
                    }

                }
                return;
            } else if (GET_QUESTMOB(ch) > 0) {
                questinfo = get_char_num(real_mobile(GET_QUESTMOB(ch)));
                if (questinfo != NULL) {
                    sprintf(buf, "&cYou are on a quest to slay &C%s!&0\r\n",
                            GET_NAME(questinfo));
                    send_to_char(buf, ch);
                } else
                {
                    send_to_char("You aren't currently on a quest.\r\n", ch);
                    if (IS_QUESTOR(ch))
                    {
                        REMOVE_BIT(PLR_FLAGS(ch), PLR_QUESTOR);
                        GET_QUESTG(ch) = 0;
                        GET_COUNTDOWN(ch) = 0;
                        GET_QUESTMOB(ch) = 0;
                        GET_QUESTOBJ(ch) = 0;
                    }
                }
                return;
            }
        } else
            send_to_char("You aren't currently on a quest.\r\n", ch);
        return;
    }
    if (isname(arg1, "points")) {
        sprintf(buf, "You have %d quest points.\r\n", GET_QUESTPOINTS(ch));
        send_to_char(buf, ch);
        return;
    } else if (isname(arg1, "time")) {
        if (!IS_QUESTOR(ch)) {
            send_to_char("You aren't currently on a quest.\r\n", ch);
            if (GET_NEXTQUEST(ch) > 1) {
                sprintf(buf, "There are %d hours remaining until you can go on another quest.\r\n",
                        GET_NEXTQUEST(ch));
                send_to_char(buf, ch);
            } else if (GET_NEXTQUEST(ch) == 1) {
                sprintf(buf, "There is less than one hour remaining until you can go on another quest.\r\n");
                send_to_char(buf, ch);
            }
        } else if (GET_COUNTDOWN(ch) > 0) {
            sprintf(buf, "Time left for current quest: %d hours\r\n",
                    GET_COUNTDOWN(ch));
            send_to_char(buf, ch);
        }
        return;
    }
    /* checks for a mob flagged MOB_QUESTMASTER */
    for (questman = world[ch->in_room].people; questman;
            questman = questman->next_in_room) {
        if (!IS_NPC(questman))
            continue;
        if (MOB_FLAGGED(questman, MOB_QUESTMASTER))
            break;
    }

    if (questman == NULL ) {
        send_to_char("You can't do that here.\r\n", ch);
        return;
    }
    if (FIGHTING(questman) != NULL) {
        send_to_char("Wait until the fighting stops.\r\n", ch);
        return;
    }
    GET_QUESTG(ch) = GET_MOB_VNUM(questman);

    /* And, of course, you will need to change the following lines for YOUR
       quest item information. Quest items on Moongate are unbalanced, very
       very nice items, and no one has one yet, because it takes awhile to
       build up quest points :> Make the item worth their while. */

    if (isname(arg1, "list")) {
        act("$n asks $N for a list of quest items.", FALSE,
            ch, NULL, questman, TO_ROOM);
        act("You ask $N for a list of quest items.", FALSE,
            ch, NULL, questman, TO_CHAR);
        sprintf(buf, "Current Quest Items available for Purchase:\r\n"
                "&G1500qp&0.........&CThe Sword of Myst\r\n"
                "&G1500qp&0.........&CThe Staff of Valaya\r\n"
                "&G1300qp&0.........&GTopuz 'Skullcracker'\r\n"
                "&G1100qp&0.........&GDagger 'Quickdeath'\r\n"
                "&G850qp&0..........&GHelmet of Myst\r\n"
                "&G800qp&0..........&GAmulet of Myst\r\n"
                "&G750qp&0..........&GShield of Myst\r\n"
                "&G650qp&0..........&YGauntlets 'Dragonslayer'\r\n"
                "&G600qp&0..........&YBoots of Miranda\r\n"
                "&G550qp&0..........&YCloak of replenishment\r\n"
             //   "&G500qp&0..........&Y5,000,000 gold pieces\r\n"
                //"&G500qp&0..........&Y10 Practices\r\n"
                "&G450qp&0..........&YTitanium leggings\r\n"
                "&G400qp&0..........&YOgre shoulder pads\r\n"
                "&G300qp&0..........&YEvening mask&0\r\n"
                "&G250qp&0..........&YDecanter of Endless Water&0\r\n\r\n"
                "To identify an item, type 'quest identify <item>'. Cost is (your_level) AP.\r\n"
                "To buy an item, type 'quest buy <item>' (ie. quest buy boots)\r\n");
        send_to_char(buf, ch);
        return;




    } else if (isname(arg1, "identify")) {
        if (arg2[0] == '\0') {
            send_to_char("To identify an item, type 'quest identify <item>'.\r\n", ch);
            return;
        }
        if (isname(arg2, "amulet")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM2, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }

        } else if (isname(arg2, "dagger quickdeath")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM12, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }

        } else if (isname(arg2, "staff valaya")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM13, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }

        } else if (isname(arg2, "topuz skullcracker")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM11, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }

        } else if (isname(arg2, "cloak replenishment")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM7, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "titanium leggings")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM8, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "gauntlets dragonslayer")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM9, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "boots miranda")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM10, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }

        } else if (isname(arg2, "shield")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM1, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "sword")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM4, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman, buf,0, 0);
                return;
            }
        } else if (isname(arg2, "decanter endless water")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM0, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);

                return;
            }
        } else if (isname(arg2, "helmet")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM3, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "mask evening")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM5, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "ogre shoulder pads")) {
            if (GET_QUESTPOINTS(ch) >= GET_LEVEL(ch)) {
                GET_QUESTPOINTS(ch) -= GET_LEVEL(ch);
                obj = create_object(QUEST_ITEM6, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }
        } else {
            sprintf(buf, "I can't identify that for you, %s.", GET_NAME(ch));
            do_say( questman,buf, 0, 0);
            return;
        }
        if (obj != NULL) {

            struct char_data * caster=ch, * cvict=NULL;
            struct obj_data *ovict=obj;
            char *tar_str=NULL;
            int level=GET_LEVEL(ch);
            MANUAL_SPELL(spell_identify);
            extract_obj(obj);
        }
        return;


    } else if (isname(arg1, "buy")) {
        if (arg2[0] == '\0') {
            send_to_char("To buy an item, type 'quest buy <item>'.\r\n", ch);
            return;
        }
        if (isname(arg2, "amulet")) {
            if (GET_QUESTPOINTS(ch) >= 800) {
                GET_QUESTPOINTS(ch) -= 800;
                obj = create_object(QUEST_ITEM2, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }

        } else if (isname(arg2, "dagger quickdeath")) {
            if (GET_QUESTPOINTS(ch) >= 1100) {
                GET_QUESTPOINTS(ch) -= 1100;
                obj = create_object(QUEST_ITEM12, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }

        } else if (isname(arg2, "staff valaya")) {
            if (GET_QUESTPOINTS(ch) >= 1500) {
                GET_QUESTPOINTS(ch) -= 1500;
                obj = create_object(QUEST_ITEM13, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }



        } else if (isname(arg2, "topuz skullcracker")) {
            if (GET_QUESTPOINTS(ch) >= 1300) {
                GET_QUESTPOINTS(ch) -= 1300;
                obj = create_object(QUEST_ITEM11, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf,0,0);
                return;
            }

        } else if (isname(arg2, "cloak replenishment")) {
            if (GET_QUESTPOINTS(ch) >= 550) {
                GET_QUESTPOINTS(ch) -= 550;
                obj = create_object(QUEST_ITEM7, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "titanium leggings")) {
            if (GET_QUESTPOINTS(ch) >= 450) {
                GET_QUESTPOINTS(ch) -= 450;
                obj = create_object(QUEST_ITEM8, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "gauntlets dragonslayer")) {
            if (GET_QUESTPOINTS(ch) >= 650) {
                GET_QUESTPOINTS(ch) -= 650;
                obj = create_object(QUEST_ITEM9, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "boots miranda")) {
            if (GET_QUESTPOINTS(ch) >= 600) {
                GET_QUESTPOINTS(ch) -= 600;
                obj = create_object(QUEST_ITEM10, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }

        } else if (isname(arg2, "shield")) {
            if (GET_QUESTPOINTS(ch) >= 750) {
                GET_QUESTPOINTS(ch) -= 750;
                obj = create_object(QUEST_ITEM1, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "sword")) {
            if (GET_QUESTPOINTS(ch) >= 1500) {
                GET_QUESTPOINTS(ch) -= 1500;
                obj = create_object(QUEST_ITEM4, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say(questman, buf,0, 0);
                return;
            }
        } else if (isname(arg2, "decanter endless water")) {
            if (GET_QUESTPOINTS(ch) >= 250) {
                GET_QUESTPOINTS(ch) -= 250;
                obj = create_object(QUEST_ITEM0, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);

                return;
            }
        } else if (isname(arg2, "helmet")) {
            if (GET_QUESTPOINTS(ch) >= 850) {
                GET_QUESTPOINTS(ch) -= 850;
                obj = create_object(QUEST_ITEM3, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "mask evening")) {
            if (GET_QUESTPOINTS(ch) >= 300) {
                GET_QUESTPOINTS(ch) -= 300;
                obj = create_object(QUEST_ITEM5, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "ogre shoulder pads")) {
            if (GET_QUESTPOINTS(ch) >= 400) {
                GET_QUESTPOINTS(ch) -= 400;
                obj = create_object(QUEST_ITEM6, GET_LEVEL(ch));
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }

        /*} else if (isname(arg2, "practices pracs prac practice")) {
            if (GET_QUESTPOINTS(ch) >= 500) {
                GET_QUESTPOINTS(ch) -= 500;
                GET_PRACTICES(ch) += 10;
                act("$N gives 10 practices to $n.", FALSE,
                    ch, NULL, questman, TO_ROOM);
                act("$N gives you 10 practices.", FALSE,
                    ch, NULL, questman, TO_CHAR);
                return;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }
        } else if (isname(arg2, "gold gp")) {
            if (GET_QUESTPOINTS(ch) >= 500) {
                GET_QUESTPOINTS(ch) -= 500;
                GET_BANK_GOLD(ch) += 5000000;
                act("$N gives 5,000,000 gold pieces to $n.", FALSE,
                    ch, NULL, questman, TO_ROOM);
                act("$N has 5,000,000 in gold transfered from $s Swiss account to your balance.",
                    FALSE, ch, NULL, questman, TO_CHAR);
                return;
            } else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", GET_NAME(ch));
                do_say( questman,buf, 0, 0);
                return;
            }*/
        } else {
            sprintf(buf, "I don't have that item, %s.", GET_NAME(ch));
            do_say( questman,buf, 0, 0);
            return;
        }
        if (obj != NULL) {
            act("$N gives $p to $n.", FALSE, ch, obj, questman, TO_ROOM);
            act("$N gives you $p.", FALSE, ch, obj, questman, TO_CHAR);
            obj_to_char(obj, ch);
        }
        return;
    } else if (isname(arg1, "request")) {
        act("$n asks $N for a quest.", FALSE, ch, NULL, questman, TO_ROOM);
        act("You ask $N for a quest.", FALSE, ch, NULL, questman, TO_CHAR);
        if (IS_QUESTOR(ch)) {
            do_say(questman, "But you're already on a quest!",0,0);
            return;
        }
        if (GET_NEXTQUEST(ch) > 0 && GET_LEVEL(ch) < LVL_GOD) {
            sprintf(buf, "You're very brave, %s, but let someone else have a chance.", GET_NAME(ch));
            do_say(questman,buf,0,0);
            do_say(questman, "Come back later.",0,0);
            return;
        }
        /*	sprintf(buf, "Thank you, brave %s!",GET_NAME(ch));
        //                act(buf, FALSE, questman, 0, 0, TO_ROOM);*/
        generate_quest(ch, questman);

        if (GET_QUESTMOB(ch) > 0 || GET_QUESTOBJ(ch) > 0) {
            int             td,
            th;
            GET_COUNTDOWN(ch) = number(35, 40);
            SET_BIT(PLR_FLAGS(ch), PLR_QUESTOR);
            sprintf(buf, "You have %d hours to complete this quest.",
                    GET_COUNTDOWN(ch));
            do_say( questman,buf, 0, 0);
            sprintf(buf, "May the gods go with you!");
            do_say( questman,buf, 0, 0);
        }
        save_char(ch, ch->in_room);
        return;
    } else if (isname(arg1, "complete")) {
        act("$n informs $N $e has completed $s quest.", FALSE, ch, NULL, questman, TO_ROOM);
        act("You inform $N you have completed $s quest.", FALSE, ch, NULL, questman, TO_CHAR);
        if (GET_QUESTG(ch) != GET_MOB_VNUM(questman)) {
            sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
            do_say( questman,buf, 0, 0);
            return;
        }
        if (IS_QUESTOR(ch)) {
            if (GET_QUESTMOB(ch) == -1 && GET_COUNTDOWN(ch) > 0) {
                int             reward,
                pointreward,
                pracreward;

                reward = number(GET_LEVEL(ch), 2*GET_LEVEL(ch));
                pointreward = number(GET_LEVEL(ch)/2,GET_LEVEL(ch));
                if (GET_COUNTDOWN(ch)>33)
                    pointreward=GET_LEVEL(ch)/2+1;
                sprintf(buf, "&GCongratulations on completing your quest!&0");
                do_say( questman,buf, 0, 0);
                sprintf(buf, "&cAs a reward, I am giving you &C%d&c quest points, and &C%d&c gold.&0", pointreward, reward);
                do_say( questman,buf, 0, 0);
                GET_GOLD(ch) += reward;
                reward=LEVELEXP(ch)/number(20, 50);
                sprintf(buf, "&cI shall also give you &C%d&c experience points for being so brave!&0", reward);
                do_say( questman,buf, 0, 0);
                gain_exp(ch, reward);
                /*if (chance(10)) {
                    pracreward = number(1, 3);
                    sprintf(buf, "&cAnd you gain &B%d&c practice session(s) !!!&0\r\n", pracreward);
                    send_to_char(buf, ch);
                    GET_PRACTICES(ch) += pracreward;
                }*/
                REMOVE_BIT(PLR_FLAGS(ch), PLR_QUESTOR);
                GET_QUESTG(ch) = 0;
                GET_COUNTDOWN(ch) = 0;
                GET_QUESTMOB(ch) = 0;
                GET_QUESTOBJ(ch) = 0;
                GET_NEXTQUEST(ch) = 20;

                GET_QUESTPOINTS(ch) += pointreward;
                save_char(ch, ch->in_room);
                return;
            } else if (GET_QUESTOBJ(ch) > 0 && GET_COUNTDOWN(ch) > 0) {
                bool            obj_found = FALSE;

                for (obj = ch->carrying; obj != NULL; obj = obj_next) {
                    obj_next = obj->next_content;

                    if (obj != NULL && GET_OBJ_VNUM(obj) == GET_QUESTOBJ(ch)) {
                        obj_found = TRUE;
                        break;
                    }
                }
                if (obj_found == TRUE) {
                    int             reward,
                    pointreward,
                    pracreward;

                    //reward = number(GET_LEVEL(ch), GET_LEVEL(ch)*2);
                    //pointreward = number(GET_LEVEL(ch),MIN(60,2* GET_LEVEL(ch)));
                    
                    reward = number(3*GET_LEVEL(ch)/2, 5*GET_LEVEL(ch)/2);
                    pointreward = number(3*GET_LEVEL(ch)/4,3*GET_LEVEL(ch)/2);
                    if (GET_COUNTDOWN(ch)>33)
                    	pointreward=GET_LEVEL(ch)/2+1;
                    	                    
                    act("You hand $p to $N.", FALSE,
                        ch, obj, questman, TO_CHAR);
                    act("$n hands $p to $N.", FALSE,
                        ch, obj, questman, TO_ROOM);

                    sprintf(buf, "&GCongratulations on completing your quest!&0");
                    do_say( questman,buf, 0, 0);
                    sprintf(buf, "&cAs a reward, I am giving you &C%d&c quest points, and &C%d&c gold.&0", pointreward, reward);
                    do_say( questman,buf, 0, 0);
                    GET_GOLD(ch) += reward;
                    reward=LEVELEXP(ch)/number(18, 40);
                    sprintf(buf, "&cI shall also give you &C%d&c experience points for such a courage!&0", reward);
                    do_say( questman,buf, 0, 0);
                    gain_exp(ch, reward);
                    /*if (chance(12)) {
                        pracreward = number(1, 3);
                        sprintf(buf, "And you gain &B%d&0 practice session(s) !!!\r\n", pracreward);
                        send_to_char(buf, ch);
                        GET_PRACTICES(ch) += pracreward;
                    }*/
                    REMOVE_BIT(PLR_FLAGS(ch), PLR_QUESTOR);
                    GET_QUESTG(ch) = 0;
                    GET_COUNTDOWN(ch) = 0;
                    GET_QUESTMOB(ch) = 0;
                    GET_QUESTOBJ(ch) = 0;
                    GET_NEXTQUEST(ch) = 19;
                    GET_QUESTPOINTS(ch) += pointreward;
                    extract_obj(obj);
                    return;
                } else {
                    do_say( questman,"You haven't completed the quest yet, but there is still time!", 0, 0);
                    return;
                }
                return;
            } else if ((GET_QUESTMOB(ch) > 0 || GET_QUESTOBJ(ch) > 0) && GET_COUNTDOWN(ch) > 0) {
                do_say( questman,"You haven't completed the quest yet, but there is still time!", 0, 0);
                return;
            }
        }
        if (GET_NEXTQUEST(ch) > 0)
            sprintf(buf, "But you didn't complete your quest in time!");
        else
            sprintf(buf, "You have to REQUEST a quest first, %s.", GET_NAME(ch));
        do_say( questman,buf, 0, 0);
        return;
    }
        send_to_char("\r\nQUEST commands: JOIN POINTS INFO TIME REQUEST COMPLETE LIST BUY IDENTIFY.\r\n", ch);
        send_to_char("For more information, type 'HELP QUEST'.\r\n", ch);
        return;
    return;
}


int not_suitable(int n)
{
    
    //int nzon[]={44,83,192,35,40,21,143,191,66,-1};
    int nzon[]={31,40,60,83,35,143,314,352,192,186,-1};
    int i,j;

    i=0;
    while (nzon[i]!=-1 && n!=nzon[i])
        i++;
    return (nzon[i]==-1? 1:0);
}



void            generate_quest(struct char_data * ch, struct char_data * questman)
{
    char  buf[MAX_STRING_LENGTH];
    struct char_data *victim = NULL;
    struct char_data *vsearch = NULL,
                                            *tch = NULL;
    struct obj_data *questitem;
    int             level_diff,
    r_num,
    mcounter,
    i,
    found;

for (mcounter = 0; mcounter < 100; mcounter++) {
        r_num = number(1, top_of_mobt - 1);
        if ((vsearch = read_mobile(r_num, REAL, 0)) != NULL) {
            level_diff = GET_LEVEL(vsearch) - GET_LEVEL(ch);
            if (GET_LEVEL(ch) < LVL_IMMORT)
                if (level_diff > 1 || level_diff < 0 || ROOM_FLAGGED(vsearch->in_room,ROOM_PEACEFUL) || mob_index[GET_MOB_RNUM(vsearch)].func == shop_keeper || (GET_LEVEL(ch)<8 && not_suitable(GET_MOB_VNUM(vsearch)/100))) {
                    char_to_room(vsearch, real_room(4));
                    extract_char(vsearch);
                    vsearch = NULL;
                }
            if (vsearch != NULL)
                break;
        }
    }

    if (GET_LEVEL(ch) < LVL_IMMORT)     /* imm 100% */
        if (chance(10) && vsearch != NULL) {    /* chance that a quest will
                                                     * be generated */
            char_to_room(vsearch, real_room(4));
            extract_char(vsearch);
            vsearch = NULL;
        }
    if (vsearch != NULL) {
        found = 0;
        for (i = 0; i < top_of_world; i++) {
            for (tch = world[i].people; tch; tch = tch->next_in_room) {
                if (GET_MOB_VNUM(tch) == GET_MOB_VNUM(vsearch)) {
                    if (!ROOM_FLAGGED(tch->in_room, ROOM_PEACEFUL)) {
                        victim = tch;found = 1;
                        break;}
                }
            }
            if (found) {
                char_to_room(vsearch, real_room(4));
                extract_char(vsearch);
                vsearch = NULL;
                break;
            }
        }
    }
    if (victim == NULL) {
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say( questman,buf, 0, 0);
        sprintf(buf, "Try again later.");
        do_say( questman,buf, 0, 0);
        GET_QUESTMOB(ch)=0;
        GET_QUESTOBJ(ch)=0;
        GET_NEXTQUEST(ch) = 10;
        return;
    }
    if (chance(40)) {           /* prefer item quests myself */
        int             objvnum;
        objvnum= number(QUEST_OBJQUEST_FIRST, QUEST_OBJQUEST_LAST);
        questitem = create_object(objvnum, GET_LEVEL(ch));
        if (questitem == NULL) {
            log("questitem does not EXIST!!");
            send_to_char("Error: questitem does not exist! please notify the imms\r\n", ch);
            return;
        }
        obj_to_char(questitem, victim);

        GET_QUESTOBJ(ch) = GET_OBJ_VNUM(questitem);
        sprintf(buf, "I've heard that %s, an extremly rare item, has been stolen from the museum!",
                questitem->short_description);
        do_say( questman,buf, 0, 0);
        /* I changed my area names so that they have just the name of the
         * area and none of the level stuff. You may want to comment these
         * next two lines. - Vassago */
        sprintf(buf, "Look in the general area of %s for %s!",
                zone_table[world[victim->in_room].zone].name,
                world[victim->in_room].name);
        do_say( questman,buf, 0, 0);
        return;
    }
    /* Quest to kill a mob */
    else {
        switch (number(0, 1)) {
        case 0:
            sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",
                    GET_NAME(victim));
            do_say( questman,buf, 0, 0);
            sprintf(buf, "This threat must be eliminated!");
            do_say( questman,buf, 0, 0);
            break;

        case 1:
            sprintf(buf, "Rune's most heinous criminal, disguised as %s, has escaped from the dungeon!",
                    GET_NAME(victim));
            do_say( questman,buf, 0, 0);
            sprintf(buf, "Since the escape, %s has murdered %d civilians!",
                    GET_NAME(victim), number(2, 20));
            do_say( questman,buf, 0, 0);
            break;
        }
        if (world[victim->in_room].name != NULL) {
            sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",
                    GET_NAME(victim), world[victim->in_room].name);
            do_say( questman,buf, 0, 0);

            /* I changed my area names so that they have just the name of the
             * area and none of the level stuff. You may want to comment
             * these next two lines. - Vassago */

            sprintf(buf, "That location is in the general area of %s.",
                    zone_table[world[victim->in_room].zone].name);
            do_say( questman,buf, 0, 0);
        }
        GET_QUESTMOB(ch) = GET_MOB_VNUM(victim);
    }
    return;
}

/* Called from update_handler() by pulse_area */

void            quest_update(void)
{
    struct char_data *ch,
                *ch_next;

for (ch = character_list; ch; ch = ch_next) {
        ch_next = ch->next;

        if (IS_NPC(ch))
            continue;

        if (GET_NEXTQUEST(ch) > 0) {
            GET_NEXTQUEST(ch)--;

            if (GET_NEXTQUEST(ch) == 0) {
                send_to_char("You may now quest again.\r\n", ch);
                return;
            }
        } else if (IS_QUESTOR(ch)) {
            if (--GET_COUNTDOWN(ch) <= 0) {
                int             num;
                GET_NEXTQUEST(ch) = 5;
                sprintf(buf, "You have run out of time for your quest!\r\nSorry, you lose some quest points.\r\n");
                send_to_char(buf, ch);
                GET_QUESTPOINTS(ch) -= number(1, GET_LEVEL(ch));
                REMOVE_BIT(PLR_FLAGS(ch), PLR_QUESTOR);
                GET_QUESTG(ch) = 0;
                GET_COUNTDOWN(ch) = 0;
                GET_QUESTMOB(ch) = 0;
            }
            if (GET_COUNTDOWN(ch) > 0 && GET_COUNTDOWN(ch) < 6) {
                send_to_char("Better hurry, you're almost out of time for your quest!\r\n", ch);
                return;
            }
        }
    }
    return;
}



void            free_quest(struct quest * q)
{
    DISPOSE(q);
}

void            write_quests(struct char_data * ch)
{
    FILE           *file;
    char            fn[127];
    struct quest   *temp;

    get_filename(GET_NAME(ch), fn, QUEST_FILE);
    unlink(fn);
    if (!GET_QUESTS(ch))
        return;

    file = fopen(fn, "wt");

    temp = GET_QUESTS(ch);

    while (temp) {
        fwrite(temp, sizeof(struct quest), 1, file);
        temp = temp->next;
    }
    fflush(file);
    fclose(file);
}

void            read_quests(struct char_data * ch)
{
    FILE           *file;
    char            fn[127];
    struct quest   *temp,
                *temp2;

    get_filename(GET_NAME(ch), fn, QUEST_FILE);

    file = fopen(fn, "r");

    if (!file)
        return;

    CREATE(GET_QUESTS(ch), struct quest, 1);
    temp = GET_QUESTS(ch);

do {
        fread(temp, sizeof(struct quest), 1, file);
        if (!feof(file)) {
            CREATE(temp2, struct quest, 1);
            temp->next = temp2;
            temp = temp->next;
        }
    } while (!feof(file));

    fclose(file);
}

struct quest   *find_quest(struct char_data * ch, int zone)
{
    struct quest   *previous,
                *temp,
                *next = GET_QUESTS(ch);
    bool            contloop = TRUE;

    previous = NULL;

while ((next != NULL) && contloop) {
        if (next->zone < zone) {
            previous = next;
            next = next->next;
        } else if (next->zone == zone)
            return next;
        else
            contloop = FALSE;
    }
    CREATE(temp, struct quest, 1);
    temp->zone = zone;
    temp->firstrow = 0;
    temp->secondrow = 0;
    temp->next = next;
    if (previous)
        previous->next = temp;
    else
        GET_QUESTS(ch) = temp;
    return temp;
}

questrow        get_quest_bits(struct char_data * ch, int zone, int row, int offset, int length)
{
    struct quest   *q;
    questrow        temp,
    accum = 0;
    int             i = 0;

    q = find_quest(ch, zone);

    if (row >= 2)
        temp = q->secondrow;
    else
        temp = q->firstrow;

    while (i < length) {
        if (IS_SET(temp, 1 << (offset + i)))
            SET_BIT(accum, 1 << i);
        i++;
    }
    return accum;
}

void            change_quest_bits(struct char_data * ch, int zone, int row, int offset, int length, int value)
{
    struct quest   *q;
    questrow        loc;
    int             i = 0;

    if (offset < 0)
        offset = 0;
    if (offset > 31)
        offset = 31;
    while ((offset + length) > 32) {
        length--;
    }
    q = find_quest(ch, zone);

    if (row >= 2)
        loc = q->secondrow;
    else
        loc = q->firstrow;

    while (i < length) {
        if (IS_SET(value, 1 << i))
            SET_BIT(loc, 1 << (offset + i));
        else
            REMOVE_BIT(loc, 1 << (offset + i));
        i++;
    }
    if (row >= 2)
        q->secondrow = loc;
    else
        q->firstrow = loc;
}

void            add_quest_bits(struct char_data * ch, int zone, int row, int offset, int length, int value)
{
    struct quest   *q;
    questrow        loc;

    if (offset < 0)
        offset = 0;
    if (offset > 31)
        offset = 31;
    while ((offset + length) >= 32)
        length--;

    q = find_quest(ch, zone);

    if (row >= 2)
        loc = q->secondrow;
    else
        loc = q->firstrow;

    if (value >= (1 << (length + 1)))
        value -= (1 << (length + 1));

    loc += (value << offset);

    if (row >= 2)
        q->secondrow = loc;
    else
        q->firstrow = loc;
}


ACMD(do_showquest)
{
    int             zone,
    i,
    j;
    struct char_data *victim;
    questrow        temp;
    char            buf[256],
    arg1[256],
    arg2[256];

    if (!*argument) {
        strcpy(buf, "Showquest usage: showquest <player> <zone>\r\n");
        send_to_char(buf, ch);
        return;
    }
    strcpy(buf, two_arguments(argument, arg1, arg2));
    if (!*arg2) {
        strcpy(buf, "Showquest usage: showquest <player> <zone>\r\n");
        send_to_char(buf, ch);
        return;
    }
    victim = get_char_vis(ch, arg1);

    zone = atoi(arg2);

    sprintf(buf, "%-35.35s01234567890123456789012345678901\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    for (j = 1; j <= 2; j++) {
        temp = get_quest_bits(victim, zone, j, 0, 32);
        sprintf(buf, "Questbits for Zone %d in Row %d is: ", zone, j);
        sprintf(buf, "%-35.35s", buf);
        for (i = 0; i < 32; i++) {
            if (temp & 1)
                sprintf(buf, "%s1", buf);
            else
                sprintf(buf, "%s0", buf);
            temp = temp >> 1;
        }
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
    }
}

ACMD(do_setquest)
{
    int             zone,
    offset,
    length,
    row;
    long            value;
    struct char_data *victim;
    questrow        temp;
    char            buf[256],
    arg1[256];

    if (!*argument) {
        if (subcmd == SCMD_SET_QUEST_BIT)
            strcpy(buf, "setquest usage: setquest <player> <zone> <row> <offset> <length> <value>\r\n");
        else
            strcpy(buf, "addquest usage: addquest <player> <zone> <row> <offset> <length> <value>\r\n");
        send_to_char(buf, ch);
        return;
    }
    half_chop(argument, arg1, argument);
    victim = get_char_vis(ch, arg1);

    half_chop(argument, arg1, argument);
    zone = atoi(arg1);
    if (!*arg1 || (zone < 0) || (zone > 500)) {
        strcpy(buf, "zone must be a number between 0 and 500");
        send_to_char(buf, ch);
        return;
    }
    half_chop(argument, arg1, argument);
    row = atoi(arg1);
    if (!*arg1 || (row < 0) || (row > 2)) {
        strcpy(buf, "row must be a number between 1 and 2");
        send_to_char(buf, ch);
        return;
    }
    half_chop(argument, arg1, argument);
    offset = atoi(arg1);
    if (!*arg1 || (offset < 0) || (offset > 31)) {
        strcpy(buf, "offset must be a number between 0 and 31");
        send_to_char(buf, ch);
        return;
    }
    half_chop(argument, arg1, argument);
    length = atoi(arg1);
    if (!*arg1 || (length < 0) || (length > 32)) {
        strcpy(buf, "length must be a number between 1 and 32");
        send_to_char(buf, ch);
        return;
    }
    if (length + offset > 32) {
        strcpy(buf, "length and offset must not exceed 32");
        send_to_char(buf, ch);
        return;
    }
    half_chop(argument, arg1, argument);
    if (!*arg1) {
        strcpy(buf, "a value must be given");
        send_to_char(buf, ch);
        return;
    }
    value = atol(arg1);

    if (subcmd != SCMD_SET_QUEST_BIT) {
        temp = get_quest_bits(victim, zone, row, offset, length);
        if (value >= 0)
            value += temp;
        else
            value = temp & ~value;
    }
    change_quest_bits(victim, zone, row, offset, length, value);
    send_to_char("Quest bit has been modified.\r\n", ch);
}



void gain_ap(struct char_data *ch, int points, char* why)
{
    sprintf(buf, "\r\n++&c You gain adventuring points for %s &0++\r\n\r\n", why);
    send_to_char(buf, ch);
    GET_QUESTPOINTS(ch)+=points;
    if (FOL_AZ(ch))
    {
      	send_to_char("Az rewards your progress.\r\n", ch);
    	gain_exp(ch, 100);
    }
}
