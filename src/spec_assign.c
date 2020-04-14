/* ************************************************************************
   File: spec_assign.c                                 Part of CircleMUD *
   Usage: Functions to assign function pointers to objs/mobs/rooms        * *

All rights reserved.  See license.doc for complete information.        * *

Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
   CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               * *********************************************************************** */

#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "rooms.h"
#include "class.h"

extern struct room_data *world;
extern int top_of_world;
extern int mini_mud;
extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern struct index_data *obj_index;
extern int             top_of_mobt;

int do_guild(struct char_data * ch, void *me, int cmd, char *argument, int skilltype);

/* functions to perform assignments */

void set_mob_class(int rnum, int klasa)
{
    int i;

    for (i=0;i<top_of_mobt;i++)
        if (mob_proto[i].nr==rnum)
        {
            mob_proto[i].player.class=klasa;
            REMOVE_BIT(mob_proto[i].char_specials.saved.act, MOB_SPEC);
            //        if (mob_proto[i].exp_ratio==100)
            //      	mob_proto[i].exp_ratio=135;
            if (klasa==CLASS_CLERIC && mob_proto[i].player.level>=25)
                SET_BIT(mob_proto[i].char_specials.saved.affected_by, AFF_SANCTUARY);
        }
}

void set_mob_spec(int rnum)
{
    int i;

    for (i=0;i<top_of_mobt;i++)
        if (mob_proto[i].nr==rnum)
        {
            SET_BIT(mob_proto[i].char_specials.saved.act, MOB_SPEC);
        }
}


void ASSIGNMOB(int mob, SPECIAL(fname))
{
    int rnum;
    SPECIAL(magic_user);
    SPECIAL(cleric);
    SPECIAL(thief);
    SPECIAL(yoda);
    SPECIAL(healer);

    if ((rnum = real_mobile(mob)) >= 0)
    {

        if      (fname==magic_user)
            set_mob_class(rnum, CLASS_MAGIC_USER);
        else if (fname==cleric)
            set_mob_class(rnum, CLASS_CLERIC);
        else if (fname==thief)
        {
            set_mob_class(rnum, CLASS_THIEF);
            set_mob_spec(rnum);
            mob_index[rnum].func = fname;
        }
        else
        {
            mob_index[rnum].func = fname;
            set_mob_spec(rnum);
        }

    }
    else if (!mini_mud) {
        sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
                mob);
        log(buf);
    }
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
    if (real_object(obj) >= 0)
        obj_index[real_object(obj)].func = fname;
    else if (!mini_mud) {
        sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
                obj);
        log(buf);
    }
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
    if (real_room(room) >= 0)
        world[real_room(room)].func = fname;
    else if (!mini_mud) {
        sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%d",
                room);
        log(buf);
    }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
    SPECIAL(postmaster);
    SPECIAL(cityguard);
    SPECIAL(mid_cityguard);
    SPECIAL(receptionist);
    SPECIAL(cryogenicist);
    SPECIAL(guild_guard);
    SPECIAL(mageguild);
    SPECIAL(clericguild);
    SPECIAL(fighterguild);
    SPECIAL(rangerguild);
    SPECIAL(thiefguild);
    SPECIAL(othersguild);
    SPECIAL(fire_dragon);
    SPECIAL(cold_dragon);
    SPECIAL(acid_dragon);
    SPECIAL(gas_dragon);
    SPECIAL(lightning_dragon);
    SPECIAL(undead);
    SPECIAL(puff);
    SPECIAL(fido);
    SPECIAL(janitor);
    SPECIAL(mayor);
    SPECIAL(snake);
    SPECIAL(thief);
    SPECIAL(magic_user);
    SPECIAL(cleric);
    SPECIAL(sailor);
    SPECIAL(engraver);
    SPECIAL(embalmer);
    SPECIAL(newbie_guide);
    SPECIAL(healer);
    SPECIAL(yoda);
    SPECIAL(archer);
    SPECIAL(mob_gang_leader);
    SPECIAL(temple_cleric);
    SPECIAL(drunker);
    SPECIAL(recruiter);
    SPECIAL(dragon);
    SPECIAL(knight);
    SPECIAL(icewizard);
    SPECIAL(grave_undertaker);
    SPECIAL(grave_demilich);
    SPECIAL(grave_ghoul);
    SPECIAL(grave_priest);
    SPECIAL(vampire);
    SPECIAL(wheel_guard);
    SPECIAL(sund_earl);
    SPECIAL(hangman);
    SPECIAL(blinder);
    SPECIAL(silktrader);
    SPECIAL(butcher);
    SPECIAL(idiot);
    SPECIAL(athos);
    SPECIAL(stu);
    SPECIAL(marbles);
    SPECIAL(troll_member);
    SPECIAL(ogre_member);
    SPECIAL(patrolman);

    SPECIAL(repair_shop);
    SPECIAL(salesman);
    SPECIAL(play_war);

    ASSIGNMOB(3069, repair_shop);
    ASSIGNMOB(3070, salesman);
    ASSIGNMOB(3071, play_war);

    //  Gang Leaders
    ASSIGNMOB(100, mob_gang_leader);


    // immortal Zone
    ASSIGNMOB(1, puff);
    ASSIGNMOB(30, receptionist);
    ASSIGNMOB(31, postmaster);
    ASSIGNMOB(32, janitor);
    //    ASSIGNMOB(1200, receptionist);
    ASSIGNMOB(1200, salesman);


    //ASSIGNMOB(1201, postmaster);
    ASSIGNMOB(1201, play_war);
    ASSIGNMOB(1202, janitor);
    //ASSIGNMOB(1202, repair_shop);

    //  Midgaard ZOO
    ASSIGNMOB(8245, wheel_guard); //Abrams
    ASSIGNMOB(8242, embalmer);    //Joe
    ASSIGNMOB(8243, engraver);    //Gruber
    ASSIGNMOB(8208, acid_dragon);

    //  Zmajcheki
    ASSIGNMOB(7040, dragon);

    ASSIGNMOB(11, undead);
    ASSIGNMOB(22, dragon);
    ASSIGNMOB(18603, fire_dragon);


    //  MUD SCHOOL
    ASSIGNMOB(3719, othersguild);
    ASSIGNMOB(3718, sailor);
    ASSIGNMOB(3707, healer);

    //  Midgaard
    ASSIGNMOB(33, drunker);
    ASSIGNMOB(3007, sailor);
    ASSIGNMOB(3066, yoda);
    //ASSIGNMOB(3066, archer);
    ASSIGNMOB(3068, newbie_guide);
    ASSIGNMOB(3005, receptionist);
    ASSIGNMOB(3010, postmaster);
    ASSIGNMOB(3020, othersguild); //mage
    ASSIGNMOB(3021, othersguild); //cleric
    ASSIGNMOB(3022, othersguild); //thief
    ASSIGNMOB(3023, othersguild); //warrior
    ASSIGNMOB(3097, othersguild); //monk
    ASSIGNMOB(3098, othersguild); //druid
    ASSIGNMOB(6000, rangerguild); //ranger (john the lumberjack)

    ASSIGNMOB(3024, guild_guard);
    ASSIGNMOB(3025, guild_guard);
    ASSIGNMOB(3026, guild_guard);
    ASSIGNMOB(3027, guild_guard);
    ASSIGNMOB(3095, guild_guard);
    ASSIGNMOB(3096, guild_guard);

    ASSIGNMOB(3059, mid_cityguard);
    ASSIGNMOB(3060, mid_cityguard);
    ASSIGNMOB(3061, janitor);
    ASSIGNMOB(3062, fido);
    ASSIGNMOB(3067, mid_cityguard);
    ASSIGNMOB(3099, mid_cityguard);
    ASSIGNMOB(3105, mayor);


    //  MORIA
    ASSIGNMOB(4000, snake);
    ASSIGNMOB(4001, snake);
    ASSIGNMOB(4053, snake);
    ASSIGNMOB(4100, magic_user);
    ASSIGNMOB(4102, snake);
    ASSIGNMOB(4103, thief);

    //  Redferne's
    ASSIGNMOB(7900, cityguard);


    //  High Tower Of Sorcery
    ASSIGNMOB(2501, cleric);
    ASSIGNMOB(2504, magic_user);
    ASSIGNMOB(2507, magic_user);
    ASSIGNMOB(2508, magic_user);
    ASSIGNMOB(2510, magic_user);
    ASSIGNMOB(2511, thief);
    ASSIGNMOB(2514, magic_user);
    ASSIGNMOB(2515, magic_user);
    ASSIGNMOB(2516, magic_user);
    ASSIGNMOB(2517, magic_user);
    ASSIGNMOB(2518, magic_user);
    ASSIGNMOB(2520, magic_user);
    ASSIGNMOB(2521, magic_user);
    ASSIGNMOB(2522, magic_user);
    ASSIGNMOB(2523, magic_user);
    ASSIGNMOB(2524, magic_user);
    ASSIGNMOB(2525, magic_user);
    ASSIGNMOB(2526, magic_user);
    ASSIGNMOB(2527, magic_user);
    ASSIGNMOB(2528, magic_user);
    ASSIGNMOB(2529, magic_user);
    ASSIGNMOB(2530, magic_user);
    ASSIGNMOB(2531, magic_user);
    ASSIGNMOB(2532, magic_user);
    ASSIGNMOB(2533, magic_user);
    ASSIGNMOB(2534, magic_user);
    ASSIGNMOB(2536, magic_user);
    ASSIGNMOB(2537, magic_user);
    ASSIGNMOB(2538, magic_user);
    ASSIGNMOB(2540, magic_user);
    ASSIGNMOB(2541, magic_user);
    ASSIGNMOB(2548, magic_user);
    ASSIGNMOB(2549, magic_user);
    ASSIGNMOB(2552, magic_user);
    ASSIGNMOB(2553, magic_user);
    ASSIGNMOB(2554, magic_user);
    ASSIGNMOB(2556, magic_user);
    ASSIGNMOB(2557, magic_user);
    ASSIGNMOB(2559, magic_user);
    ASSIGNMOB(2560, magic_user);
    ASSIGNMOB(2562, magic_user);
    ASSIGNMOB(2564, magic_user);

    //SEWERS
    ASSIGNMOB(7006, snake);
    ASSIGNMOB(7009, magic_user);
    ASSIGNMOB(7200, magic_user);
    ASSIGNMOB(7201, magic_user);
    ASSIGNMOB(7202, magic_user);

    //FOREST
    ASSIGNMOB(6112, dragon);
    ASSIGNMOB(6113, snake);
    ASSIGNMOB(6114, magic_user);
    ASSIGNMOB(6115, magic_user);
    ASSIGNMOB(6116, cleric);
    ASSIGNMOB(6117, magic_user);

    //  ARACHNOS
    ASSIGNMOB(6302, magic_user);
    ASSIGNMOB(6309, magic_user);
    ASSIGNMOB(6312, magic_user);
    ASSIGNMOB(6314, magic_user);
    ASSIGNMOB(6315, magic_user);

    //Desert
    ASSIGNMOB(5004, magic_user);
    ASSIGNMOB(5005, guild_guard);
    ASSIGNMOB(5010, magic_user);
    ASSIGNMOB(5014, magic_user);

    //  Drow City
    ASSIGNMOB(5103, magic_user);
    ASSIGNMOB(5104, magic_user);
    ASSIGNMOB(5107, magic_user);
    ASSIGNMOB(5108, magic_user);

    //  Old Thalos
    ASSIGNMOB(5200, magic_user);
    ASSIGNMOB(5201, magic_user);





    //  DWARVEN KINGDOM
    ASSIGNMOB(6500, cityguard);
    ASSIGNMOB(6502, magic_user);
    ASSIGNMOB(6509, cleric);
    ASSIGNMOB(6516, magic_user);
    ASSIGNMOB(6517, snake);

    //  Snotling Village
    ASSIGNMOB (14305, thief);

    //  Vougon Gnome Village
    ASSIGNMOB (19209, snake);
    ASSIGNMOB (19210, snake);
    ASSIGNMOB (19211, snake);
    ASSIGNMOB (19213, acid_dragon);
    ASSIGNMOB (19214, undead);
    ASSIGNMOB (19215, fire_dragon);

    //  Wyvern's Tower
    ASSIGNMOB(1605, cleric);
    ASSIGNMOB(1609, undead);
    ASSIGNMOB(1614, acid_dragon);
    ASSIGNMOB(1616, dragon);
    ASSIGNMOB(1617, undead);
    ASSIGNMOB(1716, undead);

    //  Dwarven Catacombs
    ASSIGNMOB(2007, undead);
    ASSIGNMOB(2009, undead);
    ASSIGNMOB(2011, magic_user);
    ASSIGNMOB(2013, undead);
    ASSIGNMOB(2014, undead);
    ASSIGNMOB(2016, dragon);
    ASSIGNMOB(2017, snake);


    //  Keep of Mahn-tor
    ASSIGNMOB (2300, undead);
    ASSIGNMOB (2302, acid_dragon);
    ASSIGNMOB (2306, snake);
    ASSIGNMOB (2310, cleric);
    ASSIGNMOB (2313, cold_dragon);
    ASSIGNMOB (2314, acid_dragon);
    ASSIGNMOB (2324, magic_user);
    ASSIGNMOB (2326, undead);
    ASSIGNMOB (2328, magic_user);
    ASSIGNMOB (2329, cleric);
    ASSIGNMOB (2330, cleric);
    ASSIGNMOB (2333, magic_user);

    //  Holy Grove
    ASSIGNMOB (8900, cleric);
    ASSIGNMOB (8901, cleric);
    ASSIGNMOB (8902, cleric);
    ASSIGNMOB (8903, cleric);
    ASSIGNMOB (8908, snake);
    ASSIGNMOB (8909, cleric);
    ASSIGNMOB (8910, cleric);


    //  Highlands
    ASSIGNMOB (11510, magic_user);
    ASSIGNMOB (11511, cleric);
    ASSIGNMOB (11513, dragon);
    ASSIGNMOB (11514, dragon);


    //  Dragon Tower
    ASSIGNMOB (2200, dragon);
    ASSIGNMOB (2203, magic_user);
    ASSIGNMOB (2204, cleric);
    ASSIGNMOB (2205, dragon);
    ASSIGNMOB (2220, dragon);
    ASSIGNMOB (2221, fire_dragon);
    ASSIGNMOB (2222, acid_dragon);
    ASSIGNMOB (2223, cold_dragon);
    ASSIGNMOB (2225, gas_dragon);
    ASSIGNMOB (2226, snake);
    ASSIGNMOB (2227, thief);
    ASSIGNMOB (2240, undead);
    ASSIGNMOB (2241, gas_dragon);
    ASSIGNMOB (2242, thief);
    ASSIGNMOB (2243, dragon);

    //  Chapel Catacombs
    ASSIGNMOB (3400, undead);
    ASSIGNMOB (3401, undead);
    ASSIGNMOB (3402, undead);
    ASSIGNMOB (3403, undead);
    ASSIGNMOB (3404, undead);
    ASSIGNMOB (3405, cleric);
    ASSIGNMOB (3407, undead);
    ASSIGNMOB (3408, undead);
    ASSIGNMOB (3410, undead);
    ASSIGNMOB (3411, undead);
    ASSIGNMOB (3412, magic_user);
    ASSIGNMOB (3414, magic_user);
    ASSIGNMOB (3415, undead);
    ASSIGNMOB (3416, undead);

    //  Tree of Yggdrasil
    ASSIGNMOB (23100, cleric);
    ASSIGNMOB (23101, magic_user);
    ASSIGNMOB (23104, magic_user);
    ASSIGNMOB (23106, magic_user);
    ASSIGNMOB (23107, undead);
    ASSIGNMOB (23108, magic_user);
    ASSIGNMOB (23109, gas_dragon);
    ASSIGNMOB (23110, cleric);
    ASSIGNMOB (23111, thief);
    ASSIGNMOB (23113, dragon);


    //  Shaolin temple
    ASSIGNMOB (20102, cleric);
    ASSIGNMOB (20106, magic_user);
    ASSIGNMOB (20107, snake);
    ASSIGNMOB (20111, snake);
    ASSIGNMOB (20117, thief);

    //  Sundhaven
    ASSIGNMOB(6600, sund_earl);        /* Earl of Sundhaven */
    ASSIGNMOB(6601, cityguard);
    ASSIGNMOB(6602, hangman);
    ASSIGNMOB(6664, postmaster);
    ASSIGNMOB(6656, guild_guard);
    ASSIGNMOB(6655, guild_guard);
    ASSIGNMOB(6658, guild_guard);
    ASSIGNMOB(6657, guild_guard);
    ASSIGNMOB(6666, stu);
    ASSIGNMOB(6606, fido);             /* Smoke rat */
    ASSIGNMOB(6616, othersguild);
    ASSIGNMOB(6619, othersguild);
    ASSIGNMOB(6617, othersguild);
    ASSIGNMOB(6618, othersguild);
    ASSIGNMOB(6659, cityguard);
    ASSIGNMOB(6660, cityguard);
    ASSIGNMOB(6607, thief);
    ASSIGNMOB(6648, butcher);
    ASSIGNMOB(6661, blinder);
    ASSIGNMOB(6637, silktrader);
    ASSIGNMOB(6615, idiot);
    ASSIGNMOB(6653, athos);
    ASSIGNMOB(6625, repair_shop);

    //  Ultima
    ASSIGNMOB (22007, snake);
    ASSIGNMOB (22008, magic_user);
    ASSIGNMOB (22009, magic_user);
    ASSIGNMOB (22010, magic_user);
    ASSIGNMOB (22023, cleric);
    ASSIGNMOB (22024, cleric);
    ASSIGNMOB (22026, cleric);
    ASSIGNMOB (22033, thief);
    ASSIGNMOB (22038, snake);
    ASSIGNMOB (22044, fire_dragon);
    ASSIGNMOB (22049, undead);
    ASSIGNMOB (22050, undead);
    ASSIGNMOB (22051, undead);
    ASSIGNMOB (22052, undead);
    ASSIGNMOB (22054, magic_user);
    ASSIGNMOB (22059, magic_user);
    ASSIGNMOB (22060, magic_user);
    ASSIGNMOB (22062, cleric);


    //  Abandoned Cathedral
    ASSIGNMOB(19605, grave_demilich);
    ASSIGNMOB(19601 , undead);
    ASSIGNMOB(19602 , undead);
    ASSIGNMOB(19606 , undead);
    ASSIGNMOB(19607 , undead);
    ASSIGNMOB(19608 , undead);
    ASSIGNMOB(19609 , undead);
    ASSIGNMOB(19610 , undead);
    ASSIGNMOB(19611 , undead);
    ASSIGNMOB(19612 , undead);
    ASSIGNMOB(19613 , undead);
    ASSIGNMOB(19614 , undead);

    //  Ancalador
    ASSIGNMOB(19500 , undead);
    ASSIGNMOB(19503 , undead);
    ASSIGNMOB(19504 , undead);
    ASSIGNMOB(19505 , undead);
    ASSIGNMOB(19506 , undead);
    ASSIGNMOB(19512 , undead);
    ASSIGNMOB(19501 , snake);
    ASSIGNMOB(19510 , snake);
    ASSIGNMOB(19509 , fire_dragon);

    //  Elven Woods
    ASSIGNMOB(19014, fire_dragon);



    //Ghenna
    ASSIGNMOB(9909, knight);
    ASSIGNMOB(9923, knight);
    ASSIGNMOB(9908, magic_user);
    ASSIGNMOB(9913, magic_user);
    ASSIGNMOB(9989, dragon);
    ASSIGNMOB(9914, magic_user);
    ASSIGNMOB(9907, magic_user);

    // Haunted House
    ASSIGNMOB(17002, undead);
    ASSIGNMOB(17009, undead);
    ASSIGNMOB(17005, gas_dragon);

    // Graveyard
    ASSIGNMOB(10102, grave_undertaker);
    ASSIGNMOB(10103, grave_priest);
    ASSIGNMOB(10105, grave_ghoul);
    ASSIGNMOB(10112, grave_demilich);
    ASSIGNMOB(10100, undead);
    ASSIGNMOB(10107, undead);
    ASSIGNMOB(10110, undead);

    ASSIGNMOB(1918, gas_dragon);
    ASSIGNMOB(1000, magic_user);



    ASSIGNMOB (1902, magic_user);
    ASSIGNMOB (1905, thief);
    ASSIGNMOB (1910, magic_user);
    ASSIGNMOB (1911, magic_user);
    ASSIGNMOB (1912, magic_user);
    ASSIGNMOB (1913, undead);
    ASSIGNMOB (1914, undead);
    ASSIGNMOB (1915, undead);
    ASSIGNMOB (1917, magic_user);
    ASSIGNMOB (1918, magic_user);
    ASSIGNMOB (1919, fire_dragon);

    /*ASSIGNMOB (35000, cleric);
    ASSIGNMOB (35007, cleric);
    ASSIGNMOB (35012, cleric);
    ASSIGNMOB (35013, magic_user);
    ASSIGNMOB (35019, magic_user);
    ASSIGNMOB (35020, acid_dragon);
      */
    ASSIGNMOB (35102,magic_user);
    ASSIGNMOB (35105, magic_user);
    ASSIGNMOB (35114, fido);
    ASSIGNMOB (35122,magic_user);
    ASSIGNMOB (35126, gas_dragon);
    ASSIGNMOB (35129, thief);
    ASSIGNMOB (35132, magic_user);
    ASSIGNMOB (35136, gas_dragon);
    ASSIGNMOB (35137, fire_dragon);
    ASSIGNMOB (35138, cold_dragon);
    ASSIGNMOB (35139, cold_dragon);
    ASSIGNMOB (35140,magic_user);
    //ASSIGNMOB (35142, magic_user); thief bre
    ASSIGNMOB (35143, gas_dragon);
    ASSIGNMOB (35145, thief);
    ASSIGNMOB (35146, cleric);
    ASSIGNMOB (35147, cleric);
    ASSIGNMOB (35148, cold_dragon);
    ASSIGNMOB (35151, magic_user);

    ASSIGNMOB (35205, snake);
    ASSIGNMOB (35262, thief);
    ASSIGNMOB (35284, janitor);

    ASSIGNMOB(35606, othersguild);              
    
  ASSIGNMOB (36000, cleric);
  ASSIGNMOB (36002, snake);
  ASSIGNMOB (36003, magic_user);
  ASSIGNMOB (36004, magic_user);
  ASSIGNMOB (36009, cold_dragon);
  ASSIGNMOB (36010, undead);
  ASSIGNMOB (36011, fire_dragon);
  ASSIGNMOB (36014, acid_dragon);
  ASSIGNMOB (36016, cleric);
  ASSIGNMOB (36019, acid_dragon);
  ASSIGNMOB (36021, magic_user);
  ASSIGNMOB (36022, cleric);
  ASSIGNMOB (36024, cleric);
  ASSIGNMOB (36025, magic_user);
  ASSIGNMOB (36026, snake);
  ASSIGNMOB (36027, acid_dragon);
  ASSIGNMOB (36030, cleric);
  ASSIGNMOB (36031, fido);
  ASSIGNMOB (36032, fido);
  ASSIGNMOB (36033, fido);
  ASSIGNMOB (36034, fido);
  ASSIGNMOB (36035, snake);
  ASSIGNMOB (36036, snake);
  ASSIGNMOB (36041, cleric);
  ASSIGNMOB (36042, thief);
  ASSIGNMOB (36044, magic_user);
  ASSIGNMOB (36045, gas_dragon);
  ASSIGNMOB (36046, gas_dragon);
  ASSIGNMOB (36047, fido);
  ASSIGNMOB (36049, undead);
  ASSIGNMOB (36050, undead);
  ASSIGNMOB (36052, acid_dragon);
  ASSIGNMOB (36053, snake);
  ASSIGNMOB (36059, cleric);
  ASSIGNMOB (36065, gas_dragon);
  ASSIGNMOB (36066, magic_user);
  ASSIGNMOB (36069, fire_dragon);
  ASSIGNMOB (36071, cleric);
  ASSIGNMOB (36074, snake);
  ASSIGNMOB (36083, magic_user);
  ASSIGNMOB (36086, cleric);
  ASSIGNMOB (36088, magic_user);
  ASSIGNMOB (36090, cold_dragon);
  ASSIGNMOB (36091, cold_dragon);
  ASSIGNMOB (36093, cold_dragon);
  ASSIGNMOB (36096, cold_dragon);
  ASSIGNMOB (36099, cold_dragon);
  ASSIGNMOB (36100, magic_user);
  ASSIGNMOB (36101, fire_dragon);
  ASSIGNMOB (36103, fido);
  ASSIGNMOB (36106, cleric);
  ASSIGNMOB (36107, fido);
  ASSIGNMOB (36108, gas_dragon);
  ASSIGNMOB (36253, cleric);
  
  
/*  
  ASSIGNMOB (37500, cleric);
  ASSIGNMOB (37501, fido);
  ASSIGNMOB (37502, cleric);
  ASSIGNMOB (37503, fido);
  ASSIGNMOB (37506, undead);
  ASSIGNMOB (37507, janitor);
  ASSIGNMOB (37508, fido);
  ASSIGNMOB (37509, snake);
  ASSIGNMOB (37510, janitor);
  ASSIGNMOB (37512, undead);
  ASSIGNMOB (37513, magic_user);
  ASSIGNMOB (37514, gas_dragon);
  ASSIGNMOB (37515, cleric);
  ASSIGNMOB (37516, cleric);
  ASSIGNMOB (37517, magic_user);
  ASSIGNMOB (37518, cleric);
  ASSIGNMOB (37519, magic_user);
  ASSIGNMOB (37520, cleric);
  ASSIGNMOB (37521, magic_user);
  ASSIGNMOB (37524, fire_dragon);
  ASSIGNMOB (37530, dragon);
  ASSIGNMOB (37532, fido);
  ASSIGNMOB (37533, cold_dragon);
  ASSIGNMOB (37534, magic_user);
  ASSIGNMOB (37535, cleric);
  ASSIGNMOB (37536, fido);
  ASSIGNMOB (37537, fido);
  ASSIGNMOB (37540, acid_dragon);
  ASSIGNMOB (37541, fido);
  ASSIGNMOB (37542, snake);
  ASSIGNMOB (37543, fido);
  ASSIGNMOB (37544, snake);
  ASSIGNMOB (37546, fido);
  ASSIGNMOB (37547, fido);
  ASSIGNMOB (37551, fire_dragon);
  ASSIGNMOB (37557, fido);
  ASSIGNMOB (37558, magic_user);
  ASSIGNMOB (37562, cleric);
  ASSIGNMOB (37564, magic_user);
  ASSIGNMOB (37565, thief);
  ASSIGNMOB (37566, janitor);
  ASSIGNMOB (37567, fido);
  ASSIGNMOB (37568, snake);
  ASSIGNMOB (37569, janitor);
  ASSIGNMOB (37570, janitor);
  ASSIGNMOB (37571, fido);
  ASSIGNMOB (37572, snake);
  ASSIGNMOB (37573, janitor);
  ASSIGNMOB (37574, gas_dragon);
  ASSIGNMOB (37575, cleric);

*/
  ASSIGNMOB (40006, cleric);
  ASSIGNMOB (40010, magic_user);
  ASSIGNMOB (40015, fido);
  ASSIGNMOB (40020, snake);
  ASSIGNMOB (40039, acid_dragon);
  ASSIGNMOB (40202, cold_dragon);
  ASSIGNMOB (40206, fido);
  ASSIGNMOB (40215, cold_dragon);
  ASSIGNMOB (40216, cold_dragon);
  ASSIGNMOB (40217, fido);
  ASSIGNMOB (40219, magic_user);
  ASSIGNMOB (40220, cleric);
  ASSIGNMOB (40221, magic_user);
  ASSIGNMOB (40225, fido);
  ASSIGNMOB (40226, cold_dragon);
  ASSIGNMOB (40235, cleric);
  ASSIGNMOB (40236, cleric);
  ASSIGNMOB (40303, fire_dragon);
  ASSIGNMOB (40305, fido);
  ASSIGNMOB (40307, magic_user);
  ASSIGNMOB (40308, thief);
  ASSIGNMOB (40309, fido);
  ASSIGNMOB (40310, thief);
  ASSIGNMOB (40313, magic_user);
  ASSIGNMOB (40315, fido);
  ASSIGNMOB (40322, fire_dragon);
  ASSIGNMOB (40328, cleric);
  ASSIGNMOB (40329, cleric);
  ASSIGNMOB (40330, gas_dragon);
  ASSIGNMOB (40333, fido);
  ASSIGNMOB (40335, undead);
  ASSIGNMOB (40337, fire_dragon);
  ASSIGNMOB (40347, fido);
  ASSIGNMOB (40349, magic_user);
  ASSIGNMOB (40351, snake);
  ASSIGNMOB (40352, snake);
  ASSIGNMOB (40353, cleric);
  ASSIGNMOB (40354, thief);
  ASSIGNMOB (40356, fido);
  ASSIGNMOB (40360, fido);
  ASSIGNMOB (40361, fire_dragon);
  ASSIGNMOB (40364, fido);
  ASSIGNMOB (40365, fire_dragon);
  ASSIGNMOB (40366, fido);
  ASSIGNMOB (40367, magic_user);
  ASSIGNMOB (40369, cleric);
  ASSIGNMOB (40371, fire_dragon);
  ASSIGNMOB (40374, snake);
  ASSIGNMOB (40375, magic_user);
  ASSIGNMOB (40378, magic_user);
  ASSIGNMOB (40498, magic_user);

/*    
    
      ASSIGNMOB (36433, fido);
  ASSIGNMOB (36434, janitor);
  ASSIGNMOB (36435, fido);
  ASSIGNMOB (36436, fido);
  ASSIGNMOB (36437, fido);
  ASSIGNMOB (36438, fido);
  ASSIGNMOB (36439, fido);
  ASSIGNMOB (36440, cityguard);
  ASSIGNMOB (36441, cold_dragon);
  ASSIGNMOB (36442, cold_dragon);
  ASSIGNMOB (36443, janitor);
  ASSIGNMOB (36444, janitor);
  ASSIGNMOB (36445, cold_dragon);
  ASSIGNMOB (36446, thief);
  ASSIGNMOB (36447, thief);
  ASSIGNMOB (36448, thief);
  ASSIGNMOB (36449, cold_dragon);
  ASSIGNMOB (36450, janitor);
  ASSIGNMOB (36451, thief);
  ASSIGNMOB (36452, cityguard);
  ASSIGNMOB (36453, cityguard);
  ASSIGNMOB (36454, cityguard);
  ASSIGNMOB (36455, cityguard);
  ASSIGNMOB (36456, fido);
  ASSIGNMOB (36457, cityguard);
  ASSIGNMOB (36458, cold_dragon);
  ASSIGNMOB (36459, cold_dragon);
  ASSIGNMOB (36460, cold_dragon);
  ASSIGNMOB (36461, cityguard);
  ASSIGNMOB (36462, cityguard);
  ASSIGNMOB (36463, cityguard);
  ASSIGNMOB (36464, cityguard);
  ASSIGNMOB (36465, cityguard);
  ASSIGNMOB (36466, cityguard);
  ASSIGNMOB (36467, cityguard);
  ASSIGNMOB (36468, magic_user);
  ASSIGNMOB (36469, cityguard);
  ASSIGNMOB (36470, cityguard);
  ASSIGNMOB (36471, fido);
  ASSIGNMOB (36472, fido);
  ASSIGNMOB (36473, fido);
  ASSIGNMOB (36474, magic_user);
  ASSIGNMOB (36475, fido);
  ASSIGNMOB (36476, cityguard);
  ASSIGNMOB (36477, cityguard);
  ASSIGNMOB (36478, cityguard);
  ASSIGNMOB (36479, cityguard);
  */
    
    //  Elemental Canyon
      ASSIGNMOB (9202, cleric);
      ASSIGNMOB (9204, dragon);
      ASSIGNMOB (9207, dragon);
      ASSIGNMOB (9208, cleric);
      ASSIGNMOB (9209, fire_dragon);
      ASSIGNMOB (9210, gas_dragon);
      ASSIGNMOB (9211, lightning_dragon);
      ASSIGNMOB (9212, acid_dragon);
      ASSIGNMOB (9219, acid_dragon);
      ASSIGNMOB (9223, lightning_dragon);
      ASSIGNMOB (9224, thief);
      ASSIGNMOB (9225, thief);
      ASSIGNMOB (9226, fire_dragon);
      ASSIGNMOB (9227, lightning_dragon);
      ASSIGNMOB (9228, cold_dragon);
      ASSIGNMOB (9230, gas_dragon);
      ASSIGNMOB (9231, magic_user);
      ASSIGNMOB (9232, cleric);
      ASSIGNMOB (9233, magic_user);
      ASSIGNMOB (9234, thief);
      ASSIGNMOB (9235, dragon);
      ASSIGNMOB (9237, dragon);
      ASSIGNMOB (9238, magic_user);

    //  Crystalmir Lake
      ASSIGNMOB(10001, cleric);
      ASSIGNMOB(10013, thief);
      ASSIGNMOB(10008, gas_dragon);

    //  Solace
      ASSIGNMOB(10201, magic_user);
      ASSIGNMOB(10203, magic_user);
      ASSIGNMOB(10223, magic_user);
      ASSIGNMOB(10224, magic_user);
      ASSIGNMOB(10226, magic_user);
      ASSIGNMOB(10233, magic_user);
      ASSIGNMOB(10243, magic_user);
      ASSIGNMOB(10217, thief);
      ASSIGNMOB(10234, thief);
      ASSIGNMOB(10229, thief);
      ASSIGNMOB(10232, thief);
      ASSIGNMOB(10202, cleric);
      ASSIGNMOB(10208, cleric);
      ASSIGNMOB(10211, cleric);
      ASSIGNMOB(10236, cleric);
      ASSIGNMOB(10212, undead);
      ASSIGNMOB(10213, undead);
      ASSIGNMOB(10214, undead);
      ASSIGNMOB(10242, undead);
      ASSIGNMOB(10204, cityguard);
      ASSIGNMOB(10205, cityguard);
      ASSIGNMOB(10215, cityguard);
      ASSIGNMOB(10218, cityguard);
      ASSIGNMOB(10216, snake);


    //  King Welmar's Castle (not covered in castle.c)
      ASSIGNMOB(15015, thief);
      ASSIGNMOB(15032, magic_user);
      assign_kings_castle();

    //  New Thalos
      ASSIGNMOB(5404, receptionist);
      ASSIGNMOB(5421, magic_user);
      ASSIGNMOB(5422, magic_user);
      ASSIGNMOB(5423, magic_user);
      ASSIGNMOB(5424, magic_user);
      ASSIGNMOB(5425, magic_user);
      ASSIGNMOB(5426, magic_user);
      ASSIGNMOB(5427, magic_user);
      ASSIGNMOB(5428, magic_user);
      ASSIGNMOB(5434, cityguard);
      ASSIGNMOB(5440, magic_user);
      ASSIGNMOB(5455, magic_user);
      ASSIGNMOB(5461, cityguard);
      ASSIGNMOB(5462, cityguard);
      ASSIGNMOB(5463, cityguard);
      ASSIGNMOB(5482, cityguard);


      ASSIGNMOB(5400, othersguild); //thief
      ASSIGNMOB(5401, othersguild); //warrior
      ASSIGNMOB(5402, othersguild); //monk
      ASSIGNMOB(5403, othersguild); //druid

      ASSIGNMOB(5456, guild_guard);
      ASSIGNMOB(5457, guild_guard);
      ASSIGNMOB(5458, guild_guard);
      ASSIGNMOB(5459, guild_guard);


    //  PYRAMID
      ASSIGNMOB(5300, snake);
      ASSIGNMOB(5301, snake);
      ASSIGNMOB(5304, thief);
      ASSIGNMOB(5305, thief);
      ASSIGNMOB(5309, fire_dragon);
      ASSIGNMOB(5311, magic_user);
      ASSIGNMOB(5313, cleric);
      ASSIGNMOB(5314, cleric);
      ASSIGNMOB(5315, cleric);
      ASSIGNMOB(5316, cleric);
      ASSIGNMOB(5317, magic_user);

     ASSIGNMOB(1518, dragon);
      
      //ROME
      ASSIGNMOB(12009, magic_user);
      ASSIGNMOB(12018, cityguard);
      ASSIGNMOB(12020, magic_user);
      ASSIGNMOB(12021, cityguard);
      ASSIGNMOB(12025, magic_user);
      ASSIGNMOB(12030, magic_user);
      ASSIGNMOB(12031, magic_user);
      ASSIGNMOB(12032, magic_user);
      
      //  Dwarven Daycare
      ASSIGNMOB (19800, thief);
      ASSIGNMOB (19801, thief);
      ASSIGNMOB (19802, thief);
      ASSIGNMOB (19803, thief);
      ASSIGNMOB (19804, thief);
      ASSIGNMOB (19805, thief);
      ASSIGNMOB (19806, cleric);
      ASSIGNMOB (19808, thief);
      
      


      

    //  CIRCUS
      ASSIGNMOB(4408, thief);
      ASSIGNMOB(4451, thief);

    //  Smurf Village
      ASSIGNMOB (19101, thief);
      ASSIGNMOB (19102, thief);
      ASSIGNMOB (19103, thief);
      ASSIGNMOB (19104, thief);
      ASSIGNMOB (19105, thief);
      ASSIGNMOB (19106, thief);
      ASSIGNMOB (19107, thief);
      ASSIGNMOB (19108, thief);
      ASSIGNMOB (19109, thief);
      ASSIGNMOB (19112, snake);

    //  Land of the Fire Newts
      ASSIGNMOB (2913, cleric);
      ASSIGNMOB (2914, fire_dragon);

    //  Shire
      ASSIGNMOB (1100, magic_user);
      ASSIGNMOB (1110, cityguard);
      ASSIGNMOB (1111, cityguard);
      ASSIGNMOB (1112, cityguard);
      ASSIGNMOB (1116, magic_user);
      ASSIGNMOB (1117, magic_user);
      ASSIGNMOB (1121, cityguard);
      ASSIGNMOB (1122, thief);
      ASSIGNMOB (1132, cityguard);
      
      
    //  Little Haven
      ASSIGNMOB (1804, cold_dragon);
      ASSIGNMOB (1805, lightning_dragon);
      ASSIGNMOB (1806, magic_user);
      ASSIGNMOB (1807, cold_dragon);
      ASSIGNMOB (1809, cleric);
      
    //  Murky Land
      ASSIGNMOB (10700, magic_user);
      ASSIGNMOB (10704, thief);
      ASSIGNMOB (10705, snake);
      ASSIGNMOB (10708, undead);
      ASSIGNMOB (10709, undead);
      ASSIGNMOB (10710, thief);
      ASSIGNMOB (10712, magic_user);
      ASSIGNMOB (10718, thief);
      ASSIGNMOB (10720, magic_user);
      ASSIGNMOB (10724, undead);
      ASSIGNMOB (10725, undead);
      ASSIGNMOB (10727, magic_user);
      ASSIGNMOB (10732, undead);
      ASSIGNMOB (10733, undead);
      ASSIGNMOB (10734, acid_dragon);
      ASSIGNMOB (10735, magic_user);
      ASSIGNMOB (10736, cleric);
      ASSIGNMOB (10737, magic_user);
      ASSIGNMOB (10738, magic_user);
      ASSIGNMOB (10740, undead);
      ASSIGNMOB (10741, undead);
      ASSIGNMOB (10742, undead);
      ASSIGNMOB (10743, undead);
      ASSIGNMOB (10744, undead);
      ASSIGNMOB (10745, undead);
      
    //  Olympus
      ASSIGNMOB (19301, cleric);
      ASSIGNMOB (19302, fire_dragon);
      ASSIGNMOB (19304, magic_user);
      ASSIGNMOB (19305, magic_user);
      ASSIGNMOB (19307, magic_user);
      ASSIGNMOB (19308, lightning_dragon);
      ASSIGNMOB (19309, cityguard);
      ASSIGNMOB (19314, dragon);
      ASSIGNMOB (19319, dragon);
      ASSIGNMOB (19320, fire_dragon);
      ASSIGNMOB (19321, cleric);
      ASSIGNMOB (19322, snake);
      ASSIGNMOB (19326, magic_user);

    //  Ofcol
      ASSIGNMOB (350, magic_user);
      ASSIGNMOB (600, cityguard);
      ASSIGNMOB (601, cleric);
      ASSIGNMOB (602, cleric);
      ASSIGNMOB (603, magic_user);
      ASSIGNMOB (621, janitor);
      ASSIGNMOB (623, cityguard);
      ASSIGNMOB (628, temple_cleric);
      ASSIGNMOB (629, cityguard);
      ASSIGNMOB (630, dragon);
      ASSIGNMOB (631, dragon);
      ASSIGNMOB (632, magic_user);
      ASSIGNMOB (633, dragon);
      ASSIGNMOB (634, cityguard);

    //  Valley of the Elves
      ASSIGNMOB (7807, cleric);
      ASSIGNMOB (7808, magic_user);
      ASSIGNMOB (7809, fire_dragon);
      ASSIGNMOB (7811, magic_user);  
      
      
        ASSIGNMOB (17301, cityguard);
    ASSIGNMOB (17303, cityguard);
    ASSIGNMOB (17306, cityguard);
    ASSIGNMOB (17312, cityguard);
    ASSIGNMOB (17313, magic_user);
    ASSIGNMOB (17314, magic_user);
    ASSIGNMOB (17316, lightning_dragon);
    ASSIGNMOB (17318, thief);
    ASSIGNMOB (17337, fire_dragon);     

    // Gangland
      ASSIGNMOB(2101, ogre_member);
      ASSIGNMOB(2102, troll_member);    
      ASSIGNMOB(2106, patrolman);

    
}



/* assign special procedures to objects */
void assign_objects(void)
{
    SPECIAL(bank);
    SPECIAL(gen_board);
    SPECIAL(pop_dispenser);
    SPECIAL(table);
    SPECIAL(tbox);
    SPECIAL(marbles);
    SPECIAL(wand_of_wonder);

    ASSIGNOBJ(100, tbox);
    ASSIGNOBJ(87, table);
    ASSIGNOBJ(91, pop_dispenser);
    ASSIGNOBJ(3096, gen_board);   /* social board */
    ASSIGNOBJ(3097, gen_board);	/* freeze board */
    ASSIGNOBJ(3098, gen_board);	/* immortal board */
    ASSIGNOBJ(3099, gen_board);	/* mortal board */

    ASSIGNOBJ(3034, bank);	/* atm */
    ASSIGNOBJ(36, bank);	/* cashcard */


    ASSIGNOBJ(6612, bank);
    ASSIGNOBJ(6647, marbles);
    ASSIGNOBJ(6709, marbles);

}

/* assign special procedures to rooms */
void assign_rooms(void)
{
    extern int dts_are_dumps;
    int i;

    SPECIAL(dump);
    SPECIAL(pet_shops);
    SPECIAL(pray_for_items);
    SPECIAL(bounty_reg);
    SPECIAL(assass_reg);
    SPECIAL(assassin);
    SPECIAL(troom);
    SPECIAL(map_room);
    SPECIAL(wheel_of_destiny);
    SPECIAL(mud_school);
    SPECIAL(current_proc);
    /* ASSIGNROOM(4088, war_reg); ASSIGNROOM(4194, engraver);
       ASSIGNROOM(4202, bounty_reg); ASSIGNROOM(4118, assass_reg); */
    ASSIGNROOM(3030, dump);
    ASSIGNROOM(3031, pet_shops);
    ASSIGNROOM(25100, troom);
    ASSIGNROOM(25200, troom);
    ASSIGNROOM(3072, troom);
    ASSIGNROOM(3073, map_room);
    ASSIGNROOM(3700, mud_school);

    ASSIGNROOM(8245, assassin);
    ASSIGNROOM(8247, wheel_of_destiny);

    if (dts_are_dumps)
        for (i = 0; i < top_of_world; i++)
            if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
                world[i].func = dump;
}
