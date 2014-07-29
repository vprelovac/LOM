/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "auction.h"
#include "structs.h"
#include "class.h"
#include "objs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"
#include "clan.h"
#include "comm.h"
#include "deity.h"

void            obj_from_char(struct obj_data * obj);
void            extract_obj(struct obj_data * obj);

struct Shometown {
	char *name, *desc;
	int start;
} hometowns[]=
{
	{"Midgaard", "Ancient city whose time is slowly passing", 3001},
	{"Chiiron" , "Small and peaceful village (experimental)", 35604}
};
#define NUM_HOMETOWNS (sizeof(hometowns)/sizeof(struct Shometown))
	

#define MIN_ATTRIBUTE_AVE 13
/* Names first */

const char     *class_abbrevs[] = {
                                      "Mag",
                                      "Cle",
                                      "Thi",
                                      "War",
                                      "Ran",
                                      "Bar",
                                      "Dru",
                                      "Nec",
                                      "Mon",
                                      "Dkn",
                                      "Pri",                                      
                                      "Dua",
                                      "Tri",
                                      "\n"
                                  };

const char     *pc_class_types[] = {
                                       "Mage",
                                       "Cleric",
                                       "Thief",
                                       "Warrior",
                                       "Ranger",
                                       "Bard",
                                       "Druid",
                                       "Necromancer",
                                       "Monk",
                                       "Death Knight",                                       
                                       "Priest",
                                       "Dual-Class",
                                       "Triple-Class",
                                       "\n"
                                   };

const char     *race_abbrevs[] = {
                                     "HUM",
                                     "ELF",
                                     "GOB",
                                     "DWA",
                                     "HAL",
                                     "GNO",
                                     "DRW",
                                     "OGR",
                                     "TRO",
                                     "ORC",
                                     "ENT",
                                     "DRA",
                                     "NOR",
                                     "UND",
                                     "HMD",
                                     "ANI",
                                     "GIA",
                                     "INS",
                                     "DEM",
                                     "MAG",
                                     "OOZ",
                                     "GOD",
                                     "SMA",
                                     "BIR",
                                     "\n"
                                 };

const char     *pc_race_types[] = {
                                      "Human",
                                      "Elf",
                                      "Goblin",
                                      "Dwarf",
                                      "Halfling",
                                      "Gnome",
                                      "Drow",
                                      "Ogre",
                                      "Troll",
                                      "Orc",
                                      "Ent",
                                      "DragonLord",               /* last PC class */
                                      "Other",
                                      "Undead",
                                      "Humanoid",
                                      "Animal",
                                      "Giant",
                                      "Insectoid",
                                      "Demon",
                                      "Magical",
                                      "Ooze",
                                      "God",
                                      "Small Animal"
                                      "Bird",
                                      "\n"
                                  };

/* The menu for choosing a race in interpreter.c: */
const char     *race_menu1 =
    "\r\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\n"
    "You can choose among the following races. You may see the help on each race by\r\n"
    "typing 'h' before the race number (eg. h3 will give you the help on gnome)\r\n"
    "\r\n"
    "&c0&0) &GHuman&0 (Good)...........&cThe most common race&0\r\n"
    "&c1&0) &GElf&0....................&cIntelligent, wise and agile&0\r\n"
    "&c2&0) &GDwarf&0..................&cShort, stout, wise and temperament creatures&0\r\n"
    "&c3&0) &GGnome&0..................&cSmall, devious, agile and very intelligent&0\r\n"
    "&c4&0) &GEnt&0....................&cVery big and strong tree-like creatures&0\r\n\r\n"
    "&c5&0) &GHuman&0 (Evil)...........&cThe most common race&0\r\n"
    "&c6&0) &GDrow&0...................&cDark Elves&0\r\n"
    "&c7&0) &GTroll&0..................&cBig and tough but not very smart&0\r\n"
    "&c8&0) &GOrc&0....................&cStrong and ugly, faster then Troll&0\r\n"
    "&c9&0) &GGoblin&0.................&cQuick and tough little squirts&0\r\n";

const char     *race_menu2 =
    "8) Minotaur\r\n"
    "9) Pixie\r\n";

const char     *race_menu3 =
    "v) Hemnov\r\n"
    "y) Llyran\r\n";

const char     *race_menu4 =
    "w) Hengyokai\r\n";

const char     *race_menu5 =
    "d) DragonLord\r\n";

const char     *race_menu6 =
    "\r\n";

void display_hometowns(struct descriptor_data *d)
{
	int i;
	SEND_TO_Q("\r\n\r\nNow select your home town.\r\n\r\n", d);
	for (i=0;i<NUM_HOMETOWNS;i++)
	{
		sprintf(buf, "%2d) &c%-20s&0  %s\r\n", i+1, hometowns[i].name, hometowns[i].desc);
		SEND_TO_Q(buf, d);
	}                                                 
	SEND_TO_Q("\r\nChoose your hometown: ", d);
}

int             parse_ht(char *arg, struct descriptor_data *d)
{

    int             i = atoi(arg);
    

	if (!i)
		return -1;
	
    sprintf(buf, "OK, setting your hometown to &c%s&0.\r\n", hometowns[i-1].name);
    SEND_TO_Q(buf, d);                                                            
    return hometowns[i-1].start;
 }
		
	
int             parse_race(char arg)
{

    int             i = -500; // KLUDGE
    arg = LOWER(arg);

    switch (arg) {
    case '0':
        i = RACE_HUMAN;
        break;

    case '1':
        i = RACE_ELF;
        break;
    case '2':
        i = RACE_DWARF;
        break;
    case '3':
        i = RACE_GNOME;
        break;
    case '4':
        i = RACE_ENT;
        break;
    case '5':
        i = -RACE_HUMAN-1;
        break;
    case '6':
        i = -RACE_DROW-1;
        break;
    case '7':
        i = -RACE_TROLL-1;
        break;
    case '8':
        i = -RACE_ORC-1;
        break;
    case '9':
        i = -RACE_GOBLIN-1;
        break;
    default:
        break;
    }
    return i;
}

/*" [B]arbarian\r\n"
" [S]amurai\r\n"
" [T]hief                        Avatars available on the final step\r\n"
" [N]inja                        in the path of the Gods.\r\n"
" [W]izard\r\n"*/
const char     *class_menu1 =
    "\r\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\nSelect a class:\r\r\n\n"
    "  [&GW&0]&Garrior&0...............&cAncient fighter trusting nothing but his weapon&0\r\n"
    "  [&GT&0]&Ghief&0.................&cAlways finds a way to get what he wants&0\r\n"
    "  [&GP&0]&Griest&0................&cThe believer, a servant of his deity&0\r\n"
    
    "  [&GM&0]&Gage&0..................&cThe ruler of mystical energies&0\r\n" 
    "  [&GD&0]&Gruid&0.................&cThe protector of Nature elements&0\r\n"
    "  M[&GO&0]&Gnk&0..................&cMaster of the unarmed combat&0\r\n"
    "  [&GR&0]&Ganger&0................&cElite forest trooper&0\r\n"
    "\r\n  Not currently avalable:\r\n"        
    "  [&GN&0]&Gecromancer&0...........&cMaster of the Dark side of Magic&0\r\n"
    "  [&GA&0]&Gssassin&0..............&cBringer of silent death&0\r\n"
    "  [&GB&0]&Gard&0..................&cWanderer and entertainer&0\r\n";

/*" [1] Mage-Cleric             [2] Mage-Thief            [3] Mage-Warrior\r\n"
" [4] Mage-Cleric-Warrior     [5] Mage-Thief-Warrior     [6] Cleric-Warrior\r\n"
" [7] Warrior-Thief                                     [8] Monk\r\n"
"    Race/Code    :  F B S C D M W T N 1 2 3 4 5 6 7 8\r\n"
"     Human       :  X X X X X X X X X               X\r\n"
"     Drow        :  X     X   X X X   X   X     X    \r\n"
"     Elf         :  X     X X X   X     X X   X   X\r\n"
"     Halfelf     :  X     X X X X X   X X X X X X X\r\n"
"     Dwarf       :  X X   X       X             X X\r\n"
"     Halfling    :  X X       X   X               X\r\n"
"     Gnome       :  X     X   X X X     X X       X X\r\n"
"     Ogre        :  X X   X   X           X       X\r\n"
"     Minotaur    :  X X     X X   X\r\n"
"     Pixie       :  X             X               X\r\n";
*/

const char     *class_menu2 =
    ""
    "";

const char     *class_menu3 =
    ""
    "";

const char     *class_menu4 =
    "";

/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int             parse_class(char arg, int race)
{
    int             retval = CLASS_UNDEFINED;
    arg = LOWER(arg);

    switch (arg) {
    case 'w':
        retval = CLASS_WARRIOR;
        break;
    case 't':
        retval = CLASS_THIEF;
        break;
      
            case 'c':
                retval = CLASS_CLERIC;
                break;
                               
                case 'p':

                retval = CLASS_NEWCLERIC;
                break;
                case 'm':
                retval = CLASS_MAGIC_USER;
                break;
                
            case 'd':
                retval = CLASS_DRUID;
                break;

            
            case 'o':

                retval = CLASS_MONK;
                break;
            case 'r':
                retval = CLASS_RANGER;
                break;
          
    default:
        retval = CLASS_UNDEFINED;
        break;
    }
    return retval;
}

const char     *deity_menu =
    "\r\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\n"
    "Now select your deity. Chose carefully as this decision will have great\r\n"
    "significance to the life of your character. You may see the help on each deity\r\n"
    "by typing 'h' before the deity number (eg. h1 will give you the help on Amuron)"
    "\r\n\r\n"
    "&c1&0) &GAmuron&0.................&cGreater God of souls and death&0\r\n"
    "&c2&0) &GValeria&0................&cGreater Goddess of birth and life, eternal mother&0\r\n"
    "&c3&0) &GAz&0.....................&cGreater God of knowledge&0\r\n\r\n"
    "&c4&0) &GSkig&0...................&cGod of thievery and betrayal&0\r\n"
    "&c5&0) &GUrg&0....................&cGod of battle and bloodshed&0\r\n"
    "&c6&0) &GBougar&0.................&cGod of chaos and destruction&0\r\n"
    "&c7&0) &GMugrak&0.................&cGod of suffering and pain&0\r\n"
    //"&c8&0) &GVetiin&0.................&cGod of hunt and celebrations&0\r\n"
    ;
        
int             parse_deity(char arg)
{

    int             i = -500; // KLUDGE
    arg = LOWER(arg);

    switch (arg) {
    

    case '1':
        i = DEITY_AMURON;
        break;
    case '2':
        i = DEITY_VALERIA;
        break;
    case '3':
        i = DEITY_AZ;
        break;
    case '4':
        i = DEITY_SKIG;
        break;
    case '5':
        i = DEITY_URG;
        break;
    case '6':
        i = DEITY_BOUGAR;
        break;
    case '7':
        i = DEITY_MUGRAK;
        break;
    /*case '8':
        i = DEITY_VETIIN;
        break;
      */
    default:
        break;
    }
    return i;
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long            find_class_bitvector(char arg)
{
    arg = LOWER(arg);

    switch (arg) {
    case 'm':
        return (1 << 0);
        break;
    case 'c':
        return (1 << 1);
        break;
    case 't':
        return (1 << 2);
        break;
    case 'w':
        return (1 << 3);
        break;
    case 'r':
        return (1 << 4);
        break;
    case 'b':
        return (1 << 5);
        break;
    case 'd':
        return (1 << 6);
        break;
    case 'W':
        return (1 << 7);
        break;
    case 'o':
        return (1 << 8);
        break;
    case 'a':
        return (1 << 9);
        break;
    case 'n':
        return (1 << 10);
        break;
    case 'y':
        return (1 << 14);
        break;
    case 'x':
        return (1 << 15);
    default:
        return 0;
        break;
    }
}

/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 *
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 *
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SPELL	0
#define SKILL	1

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		2  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		3  min percent gain in skill per practice */
/* #define PRAC_TYPE		1  should it say 'spell' or 'skill'?	*/

int             prac_params[2][NUM_CLASSES] = {
            /* MAG    CLE    THI    WAR    RAN    BARD    DRU    WIZ    MON    AVA    NIN    Dual   Triple*/
            {95, 95, 85, 85, 90, 80, 95, 95, 85, 95, 90, 80, 75},       /* learned level */
            {SPELL, SPELL, SKILL, SKILL, SKILL, SKILL, SPELL, SPELL, SKILL, SKILL, SKILL, SKILL, SKILL} /* prac name */
        };

/*
  *   {53,    48,    33,    33,    20,    25,    75,    90,    70,    50}, */ /* 
  * ax per prac */
/*
  *   {10,    10,     8,     8,     5,     6,    10,    15,    10,     8},	*/ /* m
  * in per pac */
/*
};
*/

/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 3017, only MAGIC_USERS are allowed
 * to go south.
 */
int             guild_info[][7] = {
                                      {CLASS_MAGIC_USER, ANYRACE, ANYALIGN, 3017, SCMD_SOUTH, 1, LVL_IMPL},
                                      {CLASS_CLERIC | CLASS_NEWCLERIC, ANYRACE, ANYALIGN, 3004, SCMD_NORTH, 1, LVL_IMPL},
                                      {CLASS_WARRIOR, ANYRACE, ANYALIGN, 3021, SCMD_EAST, 1, LVL_IMPL},
                                      {CLASS_THIEF, ANYRACE, ANYALIGN, 3027, SCMD_EAST, 1, LVL_IMPL},
                                      {CLASS_MONK , ANYRACE, ANYALIGN, 3068, SCMD_NORTH, 1, LVL_IMPL},
                                      {CLASS_DRUID , ANYRACE, ANYALIGN, 3070, SCMD_EAST, 1, LVL_IMPL},
                                      {ANYCLASS, ANYRACE, ANYALIGN, 52, SCMD_NORTH, 7, LVL_IMPL},
                                      {ANYCLASS ,ANYRACE,ANYALIGN,	5065,	SCMD_WEST,1,LVL_IMPL},

                                      {CLASS_THIEF ,ANYRACE,ANYALIGN,	5532,	SCMD_SOUTH,1,LVL_IMPL},
                                      {CLASS_WARRIOR ,ANYRACE,ANYALIGN,	5526,	SCMD_SOUTH,1,LVL_IMPL},
                                      {CLASS_CLERIC,ANYRACE,ANYALIGN,	5512,	SCMD_SOUTH,1,LVL_IMPL},
                                      {CLASS_MAGIC_USER ,ANYRACE,ANYALIGN,	5525,	SCMD_NORTH,1,LVL_IMPL},

                                      {CLASS_WARRIOR,ANYRACE, ANYALIGN, 6757, SCMD_WEST,1,LVL_IMPL},
                                      {CLASS_THIEF,ANYRACE, ANYALIGN, 6740, SCMD_EAST,1,LVL_IMPL},
                                      {CLASS_MAGIC_USER, ANYRACE, ANYALIGN,6616, SCMD_UP,1,LVL_IMPL},
                                      {CLASS_CLERIC,ANYRACE, ANYALIGN, 6661, SCMD_WEST,1,LVL_IMPL},
                                      {-1, -1, -1, -1, -1, 1, LVL_IMPL}
                                  };

/* THAC0 for classes and levels.  (To Hit Armor Class 0) */
/*thac0 for level 0, level 50*/
const int       thaco1[NUM_CLASSES][2] = {
            {20, 9},                    /* MAGE */
            {20, 9},                    /* CLERIC */
            {19, 3},                    /* THIEF */
            {18, 0},                    /* WARRIOR */
            {18, 2},                    /* RANGER */
            {19, 3},                    /* BARD */
            {20, 9},                    /* DRUID */
            {20, 9},                    /* WIZARD */
            {17, 0},                    /* MONK */
            {18, 0},                    /* AVATAR */
            {16, 0},                    /* NINJA */
            {20, 11},                   /* DUAL */
            {20, 13}                    /* TRIPLE */
        };




/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
 * which priority will be given for the best to worst stats.
 */
void            roll_real_abils(struct char_data * ch)
{
    //    void            send_to_char(char *messg, struct char_data * ch);

    int             i,
    j,
    m = 0;
    ubyte           rolls[4];
    ubyte           table[6];
    char            buf[256];

    while (m < MIN_ATTRIBUTE_AVE) {
        for (m = 0, i = 0; i < 6; i++) {
            for (j = 0; j < 4; j++)
                rolls[j] = number(1, 6);
            table[i] = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
                       MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));
            m += table[i];
        }
        m = (int) (m / 6);
    }
    ch->real_abils.str_add = 0;
    ch->real_abils.str = table[0];
    ch->real_abils.dex = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];

    if (IS_DROW(ch)) {
        ch->real_abils.dex += 2;
        ch->real_abils.intel += 1;
        ch->real_abils.con -= 1;
        ch->real_abils.cha -= 2;
    } else if (IS_ELF(ch)) {
        ch->real_abils.intel += 1;
        ch->real_abils.wis += 1;
        ch->real_abils.str -= 1;
        ch->real_abils.con -= 1;
    } else if (IS_HALFLING(ch)) {
        ch->real_abils.dex += 2;
        ch->real_abils.str -= 2;
    } else if (IS_DWARF(ch)) {
        ch->real_abils.con += 1;
        ch->real_abils.wis += 1;
        ch->real_abils.dex -= 1;
        ch->real_abils.cha -= 1;
    } else if (IS_GNOME(ch)) {
        ch->real_abils.str -= 1;
        ch->real_abils.con -= 1;
        ch->real_abils.dex += 2;
    } else if (IS_OGRE(ch)) {
        ch->real_abils.intel -= 3;
        ch->real_abils.wis -= 2;
        ch->real_abils.str += 3;
        ch->real_abils.con += 3;
        ch->real_abils.dex -= 1;
    };

    /* else if (IS_MINOTAUR(ch)) { ch->real_abils.str += 2;
     * ch->real_abils.con += 2; ch->real_abils.wis -= 2; ch->real_abils.cha
     * -= 2; } else if (IS_PIXIE(ch)) { ch->real_abils.intel += 1;
     * ch->real_abils.con -= 1; } else if (IS_LLYRA(ch)) { ch->real_abils.str
     * = 18 + dice(1, 4); ch->real_abils.dex = 18 + dice(1, 6);
     * ch->real_abils.wis = 19 + dice(1, 4); ch->real_abils.con = 17 +
     * dice(1, 3); ch->real_abils.cha = 20 + dice(1, 5); ch->real_abils.intel
     * = 20 + dice(1, 4); } else if (IS_HEMNOV(ch)) { ch->real_abils.str = 18
     * + dice(1, 6); ch->real_abils.dex = 19 + dice(1, 4); ch->real_abils.wis
     * = 18 + dice(1, 4); ch->real_abils.con = 20 + dice(1, 5);
     * ch->real_abils.cha = 19 + dice(1, 6); ch->real_abils.intel = 17 +
     * dice(1, 5); } else if (IS_WEREFORM(ch)) { ch->real_abils.str = 18 +
     * dice(1, 4); ch->real_abils.dex = 19 + dice(1, 4); ch->real_abils.wis =
     * 18 + dice(1, 3); ch->real_abils.con = 20 + dice(1, 5);
     * ch->real_abils.cha = 20 + dice(1, 5); ch->real_abils.intel = 17 +
     * dice(1, 4); } else if (IS_DRAGON(ch)) { ch->real_abils.str = 17 +
     * dice(1, 8); ch->real_abils.dex = 18 + dice(1, 7); ch->real_abils.wis =
     * 17 + dice(1, 8); ch->real_abils.con = 19 + dice(1, 6);
     * ch->real_abils.cha = 18 + dice(1, 7); ch->real_abils.intel = 20 +
     * dice(1, 5); } */
    m = ch->real_abils.str + ch->real_abils.dex + ch->real_abils.con +
        ch->real_abils.intel + ch->real_abils.wis + ch->real_abils.cha;

    ch->aff_abils = ch->real_abils;
    sprintf(buf, "Str: %d  Dex: %d  Con: %d  Int: %d  Wis: %d  Cha: %d\r\n",
            ch->real_abils.str, /* ch->real_abils.str_add, */ ch->real_abils.dex, ch->real_abils.con,
            ch->real_abils.intel, ch->real_abils.wis, ch->real_abils.cha,
            (int) (m / 6.0));
    send_to_char(buf, ch);
}

/* Some initializations for characters, including initial skills */
void            do_start(struct char_data * ch)
{
    void            advance_level(struct char_data * ch);
    struct obj_data *obj;

    int             gold = 0;
    int             divis = GET_NUM_OF_CLASS(ch);

    GET_LEVEL(ch) = 1;
    GET_EXP(ch) = 1;

    GET_ALIGNMENT(ch) = GET_REAL_ALIGNMENT(ch);
    WRATH(ch) = 0;
    //    WRATHOBJ(ch) = NULL;
    set_title(ch, NULL);
    GET_HITR(ch)=ch->points.max_hit=GET_MAX_HIT(ch) = 1;
    GET_MANAR(ch)=ch->points.max_mana = 1;
    GET_MOVER(ch)=ch->points.max_move = GET_DEX(ch)*8;

    GET_TERM(ch) = 0;
    GET_TERM_SIZE(ch) = 24;
    while (ch->carrying) {      /* clear out inventory, just in case */
        obj = ch->carrying;
        obj_from_char(obj);
        extract_obj(obj);
    }

    SET_SKILL(ch, SKILL_DODGE, 1);
    SET_SKILL(ch, SKILL_PARRY, 1);
    SET_SKILL(ch, SKILL_SHIELD, 1);

    SET_SKILL(ch, SKILL_HIT, 1);
    SET_SKILL(ch, SKILL_SLASH, 1);
    SET_SKILL(ch, SKILL_POUND, 1);
    SET_SKILL(ch, SKILL_PIERCE, 1);

    //SET_SKILL(ch, SKILL_SWIMMING, 1);
    //SET_SKILL(ch, SKILL_RIDING, 1);


    GET_GOLD(ch) = number(7, 9);
    GET_BANK_GOLD(ch) = 0;
    GET_PRACTICES(ch) = 15;
    advance_level(ch);

    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);;
    GET_MOVE(ch) = GET_MAX_MOVE(ch);

    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, FULL) = 24;
    GET_COND(ch, DRUNK) = 0;

    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

    GET_CLAN(ch) = 0;
    ch->player_specials->saved.clan_expelled=0;




}

/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void            advance_level(struct char_data * ch)
{
    int             add_hp = 0,
                             add_mana = 0,
                                        add_move = 0,
                                                   pracs=0,
                                                         i;
    int             divis = GET_NUM_OF_CLASS(ch);
    int             pr = 0;
    extern struct wis_app_type wis_app[];
    extern struct con_app_type con_app[];

    add_hp = con_app[GET_CON(ch)].hitp;

    add_move += number(GET_DEX(ch)/3, GET_DEX(ch) / 2);
    add_mana += number((GET_INT(ch) + GET_WIS(ch)) / 6, (GET_INT(ch) +GET_WIS(ch)) / 4);

    if (IS_MAGIC_USER(ch)) {
        add_hp += (int) (number(5, 7) / divis);
    }
    if (IS_CLERIC(ch)) {
        add_hp += (number(6, 8) / divis);
    }
    if (IS_THIEF(ch)) {
        add_hp += (number(7, 9) / divis);
        add_mana = 0;
    }
    if (IS_WARRIOR(ch)) {
        add_hp += (number(9, 11) / divis);
        add_mana = 0;
    }
    if (IS_MONK(ch)) {
        add_hp += (number(7, 9) / divis);
        add_mana = number(0, GET_INT(ch)/6);
    }
    if (IS_DRUID(ch)) {
        add_hp += (number(6, 9) / divis);

    }
    if (IS_RANGER(ch)) {
        add_hp += (number(8, 10) / divis);
        add_mana=number(2*(GET_INT(ch) + GET_WIS(ch)) / 19, 2*(GET_INT(ch) +GET_WIS(ch)) / 13);
    }
    
    add_mana =0;
    add_hp=GET_CONR(ch)-2+number(0,4);
    //  add_hp+=GET_CONR(ch)*GET_CONR(ch)/40		// 11=3 16=6 21=11 25=15
    add_mana=GET_INTR(ch)-2+number(0,4);
    //   add_move=GET_DEXR(ch)/4-1+number(0,2);
    if (IS_WARRIOR(ch) || IS_THIEF(ch) || IS_MONK(ch))
        add_mana=number(1,2);
    GET_MAX_HIT(ch) += MAX(1, add_hp);
    // ch->points.max_move += MAX(1, add_move);
    ch->points.max_mana += MAX(0, add_mana);
    GET_MANAR(ch)+=add_mana;
    GET_HITR(ch)+=add_hp;
    ch->points.max_hit=GET_MAX_HIT(ch);
    //GET_MOVER(ch)+=add_move;

    //    pracs=2 + wis_app[GET_WIS(ch)].bonus;
    //  GET_PRACTICES(ch) += pracs;
    if (GET_LEVEL(ch) >= LVL_IMMORT) {
        for (i = 0; i < 3; i++)
            GET_COND(ch, i) = -1;
        SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
    }


    sprintf(buf, "%s advanced to level %d.", GET_NAME(ch), GET_LEVEL(ch));
    mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    /*    if (add_mana!=0)
            sprintf(buf,"You gain &G%d&0 hit, &G%d&0 mana, &G%d&0 move points and &G%d&0 practice sessions.\r\n", add_hp, add_mana,  add_move, pracs);
        else
            sprintf(buf,"You gain &G%d&0 hit, &G%d&0 move points and &G%d&0 practice sessions.\r\n", add_hp, add_move, pracs);
        send_to_char(buf,  ch);	    
    */
    /*if (GET_LEVEL(ch)==CLAN_ENTRY_LEVEL)
{   
        send_to_char("\r\n&GWelcome to the real world!&0\r\nYou have become an OUTLAW! You'll experience even more adventure and dangers,\r\nwill be able to teach other player a lesson and show them who is the boss after\r\na while. You can also join some of the clans now as well (see help on 'clan').\r\nGood luck and enjoy it!\r\n", ch);
        GET_CLAN(ch)=CLAN_OUTLAW;
}*/  
    affect_total(ch);
    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    GET_COND(ch, DRUNK) = 0;
    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, FULL) = 24;
    save_char(ch, ch->in_room);

}


/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int             invalid_class(struct char_data * ch, struct obj_data * obj)
{
    long            temp_class = ~0;

    if (IS_MOB(ch))
        return 0;
    if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
        REMOVE_BIT(temp_class, CLASS_DUAL | CLASS_TRIPLE);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER))
            REMOVE_BIT(temp_class, CLASS_MAGIC_USER);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC))
            REMOVE_BIT(temp_class, CLASS_CLERIC );
        if (IS_OBJ_STAT(obj, ITEM_ANTI_DRUID))
            REMOVE_BIT(temp_class, CLASS_DRUID );
        if (IS_OBJ_STAT(obj, ITEM_ANTI_RANGER))
            REMOVE_BIT(temp_class, CLASS_RANGER);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_MONK))
            REMOVE_BIT(temp_class, CLASS_MONK);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF))
            REMOVE_BIT(temp_class, CLASS_THIEF);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR))
            REMOVE_BIT(temp_class, CLASS_WARRIOR);
        if IS_SET
        (GET_CLASS(ch), temp_class)
            return 0;
        else
            return 1;
    } else {
        temp_class = 0;
        if (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER))
            SET_BIT(temp_class, CLASS_MAGIC_USER);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC))
            SET_BIT(temp_class, CLASS_CLERIC);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_DRUID))
            REMOVE_BIT(temp_class, CLASS_DRUID );
        if (IS_OBJ_STAT(obj, ITEM_ANTI_RANGER))
            REMOVE_BIT(temp_class, CLASS_RANGER );
        if (IS_OBJ_STAT(obj, ITEM_ANTI_MONK))
            REMOVE_BIT(temp_class, CLASS_MONK);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF))
            SET_BIT(temp_class, CLASS_THIEF);
        if (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR))
            SET_BIT(temp_class, CLASS_WARRIOR);
        if (GET_CLASS(ch) & temp_class)
            return 1;
        else
            return 0;
    }
}

int             invalid_race(struct char_data * ch, struct obj_data * obj)
{
    return 0;
}

/* Names of class/levels and exp required for each level */
const struct title_type titles1[LVL_IMPL + 1] = {
            {"didn't set the title yet...", 0},
            {"the Swordpupil", 1},
            {"the Recruit", 2000},
            {"the Sentry", 4000},
            {"the Fighter", 8000},
            {"the Soldier", 16000},
            {"the Warrior", 32000},
            {"the Veteran", 64000},
            {"the Swordsman", 125000},
            {"the Fencer", 250000},
            {"the Combatant", 500000},
            {"the Hero", 750000},
            {"the Myrmidon", 1000000},
            {"the Swashbuckler", 1250000},
            {"the Mercenary", 1500000},
            {"the Swordmaster", 1850000},
            {"the Lieutenant", 2200000},
            {"the Champion", 2550000},
            {"the Dragoon", 2900000},
            {"the Cavalier", 3250000},
            {"the Knight", 3600000},
            {"the Knight (21)", 3900000},
            {"the Knight (22)", 4200000},
            {"the Knight (23)", 4500000},
            {"the Knight (24)", 4800000},
            {"the Knight (25)", 5150000},
            {"the Knight (26)", 5500000},
            {"the Knight (27)", 5950000},
            {"the Knight (28)", 6400000},
            {"the Knight (29)", 6850000},
            {"the Knight (30)", 7400000},
            {"the Immortal Fighter", 8000000},
            {"the Extirpator", 9000000},
            {"the Deity of War", 9500000},
            {"the Implementor", 11000000},
            {"the Mercenary", 13000000},
            {"the Swordmaster", 15000000},
            {"the Lieutenant", 18500000},
            {"the Champion", 22000000},
            {"the Dragoon", 25500000},
            {"the Cavalier", 29000000},
            {"the Knight", 33000000},
            {"the Knight (21)", 37000000},
            {"the Knight (22)", 42000000},
            {"the Knight (23)", 45000000},
            {"the Knight (24)", 48000000},
            {"the Knight (25)", 51500000},
            {"the Knight (26)", 55000000},
            {"the Knight (27)", 59500000},
            {"the Knight (28)", 64000000},
            {"the Knight (29)", 68500000},
            {"the Knight (30)", 74000000},
            {"the Immortal Fighter", 80000000},
            {"the Extirpator", 90000000},
            {"the Deity of War", 95000000}
        };
