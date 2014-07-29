/* ************************************************************************
*  File: ego.c                          Part of Lands of Myst MUD         *
*                                                                         *
*  All rights reserved.                                                   *
*                                                                         *
*  Copyright (C) 1996-2002 Vladimir Prelovac                              *
************************************************************************ */ 

/* Ego weapons by Vladimir Prelovac 1999 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "structs.h"
#include "rooms.h"
#include "objs.h"
#include "class.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "house.h"
#include "chatlink.h"
#include "arena.h"
#include "ego.h"
#include "constants.h"
/*
	char name[20];
	int min_lev;
	int item2;
	int loc1, mod1, loc2, mod2, loc3, mod3;
	int aff1, aff2, aff3;	
*/
struct Sego_weapons empty_ego =
    {
        //{"", -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
        "", -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };
            /*
struct Sego_weapons ego_weapon_low[MAX_EGO_WEAPON_LOW] =
    {
        {"(Defender)", 5, 0, APPLY_AC, 1, APPLY_CON, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Close Combat)", 1, 0, APPLY_HITROLL, 1, APPLY_DAMROLL, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Beauty)", 1, 0, APPLY_CHA, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Whirlwind)", 2, 0, APPLY_DEX, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Black Hawk)", 2, 0, APPLY_WIS, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of the Mad Butcher)", 1, 0, APPLY_STR, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of the Mind)", 2, 0, APPLY_INT, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Belangil)", 2, 0, APPLY_CON, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Gilettar)", 3, 0, APPLY_MANA, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Anguirel)", 1, 0, APPLY_MOVE, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Angrist)", 5, 0, APPLY_HIT, 15, APPLY_MOVE, 15, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Lesser Might)", 8, 0, APPLY_STR, 1, APPLY_DAMROLL, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Invader)", 6, 0, APPLY_HITROLL, 3, APPLY_DEX, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Cannae)", 5, 0, APPLY_HIT, 10, APPLY_CON, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Severe Pain)", 5, 0, APPLY_DAMROLL, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Whiplash)", 5, 0, -SKILL_DODGE, 5, APPLY_DEX, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Low Blow)", 2, 0, -SKILL_PARRY, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

struct Sego_weapons ego_weapon_med[MAX_EGO_WEAPON_MED] =
    {
        {"{Skull Smasher}", 7, 0, APPLY_STR, 3, APPLY_DAMROLL, 4, 0, 0, 0, 0, 0, 0, 0, 0},
        {"{Bonebreaker}", 5, 0, APPLY_DAMROLL, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"{Predator}", 7, 0, APPLY_INT, 2, APPLY_STR, 2, APPLY_HITROLL, 6, 0, 0, 0, 0, 0, 0},
        {"{of Gideon}", 4, 0, APPLY_INT, 2, 0, 0, 0, 0, AFF_DETECT_INVIS, 0, 0, 0, 0, 0},
        {"{Aglarang}", 7, 0, APPLY_INT, 2, APPLY_WIS, 2, APPLY_MANA, 50, 0, AFF2_ENH_MANA, 0, 0, 0, 0},
        {"{of Westernesse}", 7, 0, APPLY_STR, 2, APPLY_DEX, 2, APPLY_CON, 2, 0, 0, 0, 0, 0, 0},
        {"{Morgul}", 8, ITEM2_SLAY_GOOD, APPLY_STR, 2, APPLY_DEX, 2, APPLY_INT, 2, 0, AFF2_REGENERATE, 0, 0, 0, 0},
        {"{Mighty Avenger}", 8, ITEM2_SLAY_EVIL, APPLY_STR, 2, APPLY_DEX, 2, APPLY_INT, 2, 0, AFF2_REGENERATE, 0, 0, 0, 0}
    };
struct Sego_weapons ego_weapon_hig[MAX_EGO_WEAPON_HIG]=
    {
        {"", -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    };

              */
struct Sego_weapons ego_weapon[MAX_EGO_WEAPON] =
    {
        {"(Defender)", 1, 0, APPLY_AC, 1, APPLY_CON, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Close Combat)", 1, 0, APPLY_HITROLL, 1, APPLY_DAMROLL, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {"(of Beauty)", 1, 0, APPLY_CHA, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Whirlwind)", 1, 0, APPLY_DEX, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Black Hawk)", 1, 0, APPLY_WIS, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of the Mad Butcher)", 1, 0, APPLY_STR, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of the Mind)", 1, 0, APPLY_INT, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Belangil)", 1, 0, APPLY_CON, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Gilettar)", 2, 0, APPLY_MANA, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Anguirel)", 1, 0, APPLY_MOVE, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Angrist)", 3, 0, APPLY_HIT, 15, APPLY_MOVE, 15, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Lesser Might)", 3, 0, APPLY_STR, 1, APPLY_DAMROLL, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Invader)", 3, 0, APPLY_HITROLL, 3, APPLY_DEX, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(Cannae)", 2, 0, APPLY_HIT, 10, APPLY_CON, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Severe Pain)", 5, 0, APPLY_DAMROLL, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {"(Whiplash)", 2, 0, -SKILL_DODGE, 5, APPLY_DEX, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"(of Low Blow)", 1, 0, -SKILL_PARRY, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

        {"{Thunderstriker}", 7, 0, APPLY_CON, 2, 0, 0, 0, 0, 0, 0, 0, SPELL_LIGHTNING_BOLT, 15, 0},        
        {"{Skull Smasher}", 7, 0, APPLY_STR, 3, APPLY_DAMROLL, 4, 0, 0, 0, 0, 0, 0, 0, 1},
        {"{Bonebreaker}", 6, 0, APPLY_DAMROLL, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {"{Predator}", 6, 0, APPLY_INT, 2, APPLY_STR, 2, APPLY_HITROLL, 6, 0, 0, 0, 0, 0, 0},
        {"{of Gideon}", 5, 0, APPLY_INT, 2, 0, 0, 0, 0, AFF_DETECT_INVIS, 0, 0, 0, 0, 0},
        {"{Aglarang}", 6, 0, APPLY_INT, 2, APPLY_WIS, 2, APPLY_MANA, 50, 0, AFF2_ENH_MANA, 0, 0, 0, 0},
        {"{of Westernesse}", 6, 0, APPLY_STR, 2, APPLY_DEX, 2, APPLY_CON, 2, 0, 0, 0, 0, 0, 0},
        {"{Morgul}", 7, ITEM2_SLAY_GOOD, APPLY_STR, 2, APPLY_DEX, 2, APPLY_INT, 2, 0, AFF2_REGENERATE, 0, 0, 0, 2},
        {"{Mighty Avenger}", 7, ITEM2_SLAY_EVIL, APPLY_STR, 2, APPLY_DEX, 2, APPLY_INT, 2, 0, AFF2_REGENERATE, 0, 0, 0, 2},
        
        
         {"^of Ultimate Might^", 10, 0, APPLY_STR, 4, APPLY_DAMROLL, 5, 0, 0, 0, 0, 0, 0, 0, 2},
         {"^Slayer^", 10, 0, 0, 0, APPLY_DAMROLL, 3, APPLY_HITROLL, 3, 0, 0, 0, 0, 0, 3},
         {"^Devastator^", 8, 0, 0, 0, APPLY_DAMROLL, 7, 0, 0, 0, 0, 0, 0, 0, 2},
         {"^Mjollner^", 8, 0, APPLY_STR, 2, 0, 0, 0, 0, 0, 0, 0, SPELL_FIREBALL, 15, 1},
         {"^Soul Searer^", 9, 0, APPLY_CON, 2, APPLY_DEX, 2, APPLY_STR, 2, 0, 0, 0, SPELL_ACID_BLAST, 15, 2},                 
         {"^Mage Slayer^", 9, 0, APPLY_AC, 10, 0, 0, 0, 0, 0, 0, 0, SPELL_DISPEL_MAGIC, 10, 1},  
         {"^Conqueror^", 10, 0, APPLY_HIT, 100, APPLY_MOVE, 100, APPLY_MANA, 100, 0, 0, 0, 0, 0, 1}
  };       
         
          /*
struct Sego_weapons ego_armor_low[MAX_EGO_ARMOR_LOW]=
    {
        {"(of Eastwood)", 1 , 0,APPLY_AC, 1,0,0,0,0,0,0,0, 0, 0, 10},
        {"(of Lesser Shielding)", 4 , 0,APPLY_AC, 2,APPLY_CON,1,0,0,0,0,0, 0, 0, 15},
        {"(Balancer)", 3 , 0, APPLY_AC, 1,APPLY_DEX,1,0,0,0,0,0, 0, 0, 10},
        {"(of Gordion)", 6 , 0, APPLY_AC, 2, APPLY_STR, 1,0,0,0,0,0, 0, 0, 10},
        {"(of Enlightment)", 5 , 0, APPLY_AC, 1, APPLY_INT, 1, APPLY_WIS,1,0,0,0, 0, 0, 10},
        {"(Deliveer)", 7 , 0, APPLY_AC, 1, APPLY_HITROLL, 2, APPLY_DAMROLL,1,0,0,0, 0, 0, 10}
    };
      
struct Sego_weapons ego_armor_med[MAX_EGO_ARMOR_MED]=
    {
        {"{Ehr Rhee}", 8 , 0, APPLY_AC, 3, 0,0,0,0,0,AFF2_ENH_HEAL,0, 0, 0, 20},
        {"{WindWalker}", 5 , 0, APPLY_AC, 2, APPLY_DEX,2,0,0,0,0,0, 0, 0, 15},
        {"{Guardian}", 8 , 0, APPLY_AC, 3, APPLY_CON,2,0,0,0,AFF2_STONESKIN,0, 0, 0, 25},
        {"{Sentinel}", 4 , 0, APPLY_AC, 3, APPLY_CHAR_WEIGHT,60,0,0,0,0,0, 0, 0, 15},
        {"{of the Shadows}", 7 , 0, APPLY_AC, 3, 0,0,0,0,AFF_INFRAVISION ,0,0, 0, 0, 15}
        {"{of Endurance}", 7 , 0, APPLY_AC, 3, 0,0,0,0,0,AFF2_ENH_MOVE,0, 0, 0, 25}, 
        {"{of Quatis}", 6 , 0, APPLY_AC, 4, 0,0,0,0,0,AFF2_ENH_MANA,0, 0, 0, 10},
        {"{of Gord}", 9 , 0, APPLY_AC, 2, APPLY_HITROLL, 3, APPLY_DAMROLL,3,0,0,0, 0, 0, 15},
        {"{of Ka-Gord}", 10 , 0, APPLY_AC, 3, APPLY_HITROLL, 4, APPLY_DAMROLL,4,0,0,0, 0, 0, 20}
    };

struct Sego_weapons ego_armor_hig[MAX_EGO_ARMOR_HIG]=
    {
        {"[of Destiny]", 3 , 0, APPLY_AC, 5, 0,0,0,0,0,0,0, 0, 0, 25},
        {"", -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    };
        */
struct Sego_weapons ego_armor[MAX_EGO_ARMOR]=       
{
        {"(of Eastwood)", 1 , 0,APPLY_AC, 1,0,0,0,0,0,0,0, 0, 0, 10},
        {"(of Lesser Shielding)", 2 , 0,APPLY_AC, 2,APPLY_CON,1,0,0,0,0,0, 0, 0, 15},
        {"(Balancer)", 2 , 0, APPLY_AC, 1,APPLY_DEX,1,0,0,0,0,0, 0, 0, 10},
        {"(of Gordion)", 2 , 0, APPLY_AC, 2, APPLY_STR, 1,0,0,0,0,0, 0, 0, 10},
        {"(of Enlightment)", 3 , 0, APPLY_AC, 1, APPLY_INT, 1, APPLY_WIS,1,0,0,0, 0, 0, 10},
        {"(Deliveer)", 3 , 0, APPLY_AC, 1, APPLY_HITROLL, 2, APPLY_DAMROLL,1,0,0,0, 0, 0, 10},
        
        {"{Ehr Rhee}", 5 , 0, APPLY_AC, 3, 0,0,0,0,0,AFF2_ENH_HEAL,0, 0, 0, 15},
        {"{WindWalker}", 4 , 0, APPLY_AC, 2, APPLY_DEX,2,0,0,0,0,0, 0, 0, 15},
        {"{Guardian}", 6 , 0, APPLY_AC, 3, APPLY_CON,2,0,0,0,AFF2_STONESKIN,0, 0, 0, 20},
        {"{Sentinel}", 4 , 0, APPLY_AC, 3, APPLY_CHAR_WEIGHT,60,0,0,0,0,0, 0, 0, 15},
        {"{of the Shadows}", 6 , 0, APPLY_AC, 3, 0,0,0,0,AFF_INFRAVISION ,0,0, 0, 0, 15},
        {"{of Endurance}", 5 , 0, APPLY_AC, 3, 0,0,0,0,0,AFF2_ENH_MOVE,0, 0, 0, 20}, 
        {"{of Quatis}", 5 , 0, APPLY_AC, 4, 0,0,0,0,0,AFF2_ENH_MANA,0, 0, 0, 10},
        {"{Nord}", 6 , 0, APPLY_AC, 2, APPLY_HITROLL, 3, APPLY_DAMROLL,3,0,0,0, 0, 0, 15},
        {"{Kah-Nord}", 7 , 0, APPLY_AC, 3, APPLY_HITROLL, 4, APPLY_DAMROLL,4,0,0,0, 0, 0, 20}, 
               
        {"^of North^", 8 , 0, APPLY_AC, 5, APPLY_STR, 3,0,0,0,0,0,0, 0,  25},    
        {"^of South^", 8 , 0, APPLY_AC, 5, APPLY_INT, 2,APPLY_WIS, 2,0,0,0,0, 0,  25},    
        {"^of East^", 8, 0, APPLY_AC, 5, APPLY_CON, 3,0,0,0,0,0,0, 0,  25},    
        {"^of West^", 8 , 0, APPLY_AC, 5, APPLY_DEX, 3,0,0,0,0,0,0, 0,  25},            
        {"^Healer^", 9 , 0, APPLY_AC, 5, 0,0,0,0,0,AFF2_REGENERATE,0, PRAYER_CURE, 20, 20}    
       
        
        
};

void init_ego_pom(struct Sego_weapons ego[], int max)
{
    int i;
    strcpy(buf, "&c");

    for (i=0; i<max; i++)
    {
        strcat(ego[i].name, "&0");
        strcat(buf, ego[i].name);
        strcpy(ego[i].name, buf);
    }
}

void init_ego()
{
  /*  init_ego_pom(ego_weapon_low, MAX_EGO_WEAPON_LOW);
    init_ego_pom(ego_weapon_med, MAX_EGO_WEAPON_MED);
    init_ego_pom(ego_weapon_hig, MAX_EGO_WEAPON_HIG);
    init_ego_pom(ego_armor_low, MAX_EGO_ARMOR_LOW);
    init_ego_pom(ego_armor_med, MAX_EGO_ARMOR_MED);
    init_ego_pom(ego_armor_hig, MAX_EGO_ARMOR_HIG);*/
    init_ego_pom(ego_weapon, MAX_EGO_WEAPON);
    init_ego_pom(ego_armor, MAX_EGO_ARMOR);
    
}

int find_spot (struct obj_data *obj, int loc)
{

    int i, free=-1;
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
        if (obj->affected[i].location == APPLY_NONE || obj->affected[i].location == loc)
        {
            free = i;
            break;
        }
    return (free > -1 ? free : 0);
}

void assign_ego( struct obj_data *obj, struct Sego_weapons ego)
{
    int last_spot=-1, spot;

    if (ego.min_lev==-1)
        return;


    REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_HIDDEN_EGO);
    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_EGO);

    // inscribe name
    sprintf(buf, "%s %s", obj->short_description, ego.name);
    obj->short_description = str_dup(buf);

    if (ego.item2)
        SET_BIT(GET_OBJ_EXTRA2(obj), ego.item2);

    if (ego.loc1)
    {
        spot=find_spot(obj, ego.loc1);
        if (!spot)
        {
            spot=last_spot+1;
            last_spot=spot;
        }
        obj->affected[spot].location=ego.loc1;
        obj->affected[spot].modifier+=ego.mod1;
    }
    if (ego.loc2)
    {
        spot=find_spot(obj, ego.loc2);
        if (!spot)
        {
            spot=last_spot+1;
            last_spot=spot;
        }
        obj->affected[spot].location=ego.loc2;
        obj->affected[spot].modifier+=ego.mod2;
    }
    if (ego.loc3)
    {
        spot=find_spot(obj, ego.loc3);
        if (!spot)
        {
            spot=last_spot+1;
            last_spot=spot;
        }
        obj->affected[spot].location=ego.loc3;
        obj->affected[spot].modifier+=ego.mod3;
    }

    SET_BIT(obj->obj_flags.bitvector,ego.aff1);
    SET_BIT(obj->obj_flags.bitvector2,ego.aff2);
    SET_BIT(obj->obj_flags.bitvector3,ego.aff3); 
    
    if (ego.enh) {
    	if (GET_OBJ_TYPE(obj)==ITEM_WEAPON)
    		GET_OBJ_VAL(obj, 1)= GET_OBJ_VAL(obj, 1)+ego.enh;
    	else
    		{                          
    			int i;
    			i=MAX(1, ego.enh*GET_OBJ_VAL(obj, 1)/100);
    			GET_OBJ_VAL(obj, 1)=GET_OBJ_VAL(obj, 1)+i;
    			GET_OBJ_VAL(obj, 0)=GET_OBJ_VAL(obj, 0)+i;
    		}
    	} 
    obj->bound_spell=ego.spell;
    obj->bound_spell_timer=ego.timer;
    obj->bound_spell_level=GET_OBJ_LEVEL(obj);
    
    GET_OBJ_COST(obj)*=MAX(2, ego.min_lev/2);

}


int find_ego_by_lev(struct Sego_weapons *ego, int maxego, int lev)
{
    int i, t=14;
    if (maxego==1)
        return 0;

    while (t)
    {
        i=number(0, maxego-1);
        //if ((abs(ego[i].min_lev-lev/2)<=number(0, 10)))
        if ((ego[i].min_lev<=number(1, 10)) && (ego[i].min_lev*2<number(1, lev)) && (ego[i].min_lev*5>=number(0, lev)))
            return (i);
        t--;
    } 
   
    return (number(0, maxego-1));
}

struct Sego_weapons make_ego(struct obj_data *obj, int lev)
{
    int i, j, k;
    struct Sego_weapons pom;

    if (lev<1 || lev > 100)
    {
        sprintf(buf, "Illegal obj level (%d) passed to make_ego()\n", lev);
        log(buf);
        return empty_ego;
    }

    if (GET_OBJ_TYPE(obj)==ITEM_ARMOR)
    {
        
        k=find_ego_by_lev(ego_armor, MAX_EGO_ARMOR, lev);              
        pom=ego_armor[k];        
        
/*        
        if (lev < 16)
        {
            k=find_ego_by_lev(ego_armor_low, MAX_EGO_ARMOR_LOW, lev);
            pom=ego_armor_low[k];
        }
        else if (lev < 32 )
        {
            k=find_ego_by_lev(ego_armor_med, MAX_EGO_ARMOR_MED, lev-16);
            pom=ego_armor_med[k];
        }
        else
        {
            k=find_ego_by_lev(ego_armor_hig, MAX_EGO_ARMOR_HIG, lev-32);
            pom=ego_armor_hig[k];
        }*/

    }
    else if (GET_OBJ_TYPE(obj)==ITEM_WEAPON || GET_OBJ_TYPE(obj)==ITEM_FIREWEAPON)
    {
    	    k=find_ego_by_lev(ego_weapon, MAX_EGO_WEAPON, lev);    	    
            pom=ego_weapon[k];
            /*
        if (lev < 16)
        {
            k=find_ego_by_lev(ego_weapon_low, MAX_EGO_WEAPON_LOW, lev);
            pom=ego_weapon_low[k];
        }
        else if (lev < 32 )
        {
            k=find_ego_by_lev(ego_weapon_med, MAX_EGO_WEAPON_MED, lev-16);
            pom=ego_weapon_med[k];
        }
        else
        {
            k=find_ego_by_lev(ego_weapon_hig, MAX_EGO_WEAPON_HIG, lev-32);
            pom=ego_weapon_hig[k];
        }     */
    }
    else
        return empty_ego;
    if (pom.min_lev!=-1)
        assign_ego(obj, pom);
    return (pom);
}


int check_existing_ego(char *name, int type)
{
    char *filename;
    FILE *f=NULL;
    char buff[100000];
    int size;

    if (type==ITEM_ARMOR)
        f=fopen(EGO_ARMORS_FILE, "r+t");
    else if (type==ITEM_WEAPON || type==ITEM_FIREWEAPON)
        f=fopen(EGO_WEAPONS_FILE, "r+t");

    else
        return -1;

    if (f==NULL)
    {
        log("ERROR OPENING EGO FILE: PROBABLY OPENING FOR THE FIRST TIME");
        return 	-1;
    }

    fseek(f, 0, SEEK_END);
    size=ftell(f);
    if (!size)
    {
    	fclose(f);
        return -1;
    }
    if (size>99999)
    {
    	   log("ERROR OPENING EGO FILE: EGO FILE SIZE EXCEEDS BUFFER SIZE");
    	   fclose(f);
        return 	-1;
    }
    fseek(f, 0,0);
    fread(buff, size, 1, f);
    fclose(f);
    return (strstr(buff, name)?1:-1);
}


void write_ego_to_file(struct Sego_weapons ego, int type)
{
    char buf1[50];
    int k;
    FILE *f;

    if (type==ITEM_ARMOR)
        f=fopen(EGO_ARMORS_FILE, "a+t");
    else if (type==ITEM_WEAPON || type==ITEM_FIREWEAPON)
        f=fopen(EGO_WEAPONS_FILE, "a+t");
    else
        return;

    fprintf(f, "\n&c%s&0\n", ego.name);
    if (ego.loc1)
    {
        if (ego.loc1>0)
            sprinttype(ego.loc1, apply_types, buf1);
        else
            sprintf(buf1, "skill '%s'", spells[-ego.loc1]);
        fprintf(f, "* %s by %d", buf1, ego.mod1);
    }
    if (ego.loc2)
    {
        if (ego.loc2>0)
            sprinttype(ego.loc2, apply_types, buf1);
        else
            sprintf(buf1, "skill '%s'", spells[-ego.loc2]);
        fprintf(f, ", %s by %d", buf1, ego.mod2);
    }
    if (ego.loc3)
    {
        if (ego.loc3>0)
            sprinttype(ego.loc3, apply_types, buf1);
        else
            sprintf(buf1, "skill '%s'", spells[-ego.loc3]);
        fprintf(f, ", %s by %d", buf1, ego.mod3);
    }
    fprintf(f, "\n");

    if (ego.item2)
    {
        sprintbit(ego.item2, extra_bits2, buf1);
        fprintf(f, "* %s\n", buf1);
    }
    k=0;
    if (ego.aff1)
    {
        sprintbit(ego.aff1, affected_bits, buf1);
        fprintf(f, "* %s", buf1);
        k=1;
    }
    if (ego.aff2)
    {
        sprintbit(ego.aff2, affected_bits2, buf1);
        if (k)
            fprintf(f, ", %s", buf1);
        else
            fprintf(f, "* %s", buf1);
        k=1;
    }
    if (ego.aff3)
    {
        sprintbit(ego.aff3, affected_bits3, buf1);
        if (k)
            fprintf(f, ", %s", buf1);
        else
            fprintf(f, "* %s", buf1);

}

	
    if (k)
        fprintf(f, "\n");
        
        if (ego.spell)                                 
		fprintf(f, "* Casts %s\n", spells[ego.spell]);
		
	if (ego.enh)
	{
		if (type==ITEM_ARMOR)
			fprintf(f, "* Enhances item armor by %d%%\n", ego.enh);
		else
			fprintf(f, "* Enhances weapon damage\n");
	}
	fflush(f);
    	fclose(f);

}
  char buff[65000];
ACMD(do_egoknown)
{

    FILE *f=NULL;
   
    int size;

    one_argument(argument, arg);
    if (!*arg)
    {
        send_to_char("You must supply an argument ('weapons' or 'armors').\r\n", ch);
        return;
    }
    if (isname(arg, "weapons"))
    {
        send_to_char("Known ego weapons: \r\n", ch);
        f=fopen(EGO_WEAPONS_FILE, "r+t");
        if (f==NULL)
        {
            send_to_char("\r\n  &CNone so far!&0 \r\n", ch);
            return;
        }
        fseek(f, 0, SEEK_END);
        size=ftell(f);
        if (!size)
            return ;
        fseek(f, 0,0);
        fread(buff, size, 1, f);
        fclose(f);
        buff[size-1]=0;
        page_string(ch->desc,buff, 0);
    }
    else if (isname(arg, "armors"))
    {
        send_to_char("Known ego armors: \r\n", ch);
        f=fopen(EGO_ARMORS_FILE, "r+t");
        if (f==NULL)
        {
            send_to_char("\r\n  &CNone so far!&0 \r\n", ch);
            return;
        }
        fseek(f, 0, SEEK_END);
        size=ftell(f);
        if (!size)
            return ;
        fseek(f, 0,0);
        fread(buff, size, 1, f);
        fclose(f);
        buff[size-1]=0;
        page_string(ch->desc,buff, 0);
    }
    else
        send_to_char("You must supply an argument ('weapons' or 'armors').\r\n", ch);
}
