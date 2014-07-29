/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "structs.h"
#include "class.h"


const char circlemud_version[] = { "Lands of Myst (c) 1996-2002 Vladimir Prelovac\r\n"};

/*
17 - 117
18 - 100
19 - 84
20 - 69
21 - 55
22 - 42
23 - 30
24 - 19
25 - 9
*/

const struct race_app_type race_app[NUM_PC_RACE+1] =
    {//  tot  str int wil dex con cha
        {275, 21, 21, 21, 21, 21, 21},  // HUMAN
        {221, 20, 24, 23, 24, 19, 25},  // ELF
        {333, 19, 19, 18, 25, 19, 18},  // GOBLIN
        {263, 23, 19, 24, 18, 23, 20},  // DWARF
        {000, 00, 00, 00, 00, 00, 00},  // HALFLING
        {306, 18, 25, 21, 22, 18, 21},  // GNOME
        {215, 21, 24, 22, 23, 20, 24},  // DROW
        {000, 00, 00, 00, 00, 00, 00},  // OGRE
        {312, 25, 18, 18, 19, 24, 17},  // TROLL
        {283, 23, 19, 19, 21, 23, 18},  // ORC
        {294, 24, 17, 22, 17, 25, 19}   // ENT
    };

const struct sclass class_app[]=

    {
        100,                                // MAGIC_USER
        100,                                 // CLERIC
        100,                                 // THIEF
        100,                                 // WARRIOR
        100,                                 // RANGER
        100,                                 // BARD
        100,                                 // DRUID
        100,                                 // NECRON
        100,                                 // MONK
        100,                                 // DEATHK
        100                                 // ADVENTURER
    };


int clan_loadroom[4]={
                         3001,
                         25100,
                         25200,
                         6608
                     };


struct Sdeity  deity_list[]=
{
	{"noone", 	DEITY_NONE 	},
	{"Amuron", 	DEITY_AMURON	},
	{"Valeria", 	DEITY_VALERIA   },
	{"Az", 		DEITY_AZ   	},
	{"Skig", 	DEITY_SKIG 	},
	{"Urg", 	DEITY_URG  	},
	{"Bougar", 	DEITY_BOUGAR    },
	{"Mugrak", 	DEITY_MUGRAK    },
	{"Vetiin", 	DEITY_VETIIN   },
	
	{"",-1}
};



/* char channel_bits[] will be read in from a file */
/* char clan_names[] will also be read in from a file */

const char *channel_bits[] = {
                                 "God Channel",
                                 "Radio Free Europa",
                                 "Arena battles",
                                 "4",
                                 "5",
                                 "6",
                                 "7",
                                 "8",
                                 "9",
                                 "10",
                                 "11",
                                 "12",
                                 "13",
                                 "14",
                                 "15",
                                 "16",
                                 "17",
                                 "18",
                                 "19",
                                 "20",
                                 "21",
                                 "22",
                                 "23",
                                 "24",
                                 "25",
                                 "26",
                                 "27",
                                 "28",
                                 "29",
                                 "30",
                                 "31",
                                 "32",
                                 "\n"
                             };

/* cardinal directions */
const char *dirs[] =
    {
        "north",
        "east",
        "south",
        "west",
        "up",
        "down",
        "north-east",
        "north-west",
        "south-east",
        "south-west",
        "somewhere",
        "\n"
    };

const char *dirs2[] =
    {
        "the north",
        "the east",
        "the south",
        "the west",
        "above",
        "below",
        "the north-east",
        "the north-west",
        "the south-east",
        "the south-west",
        "somewhere",
        "\n"
    };

const char *teleport_bits[] = {
                                  "Force Look",
                                  "Timed",
                                  "Random",
                                  "Spin(NOT AVAIL)",
                                  "Has Obj",
                                  "No Obj",
                                  "No MSG",
                                  "\n"
                              };

/* ROOM_x */
const char *room_bits[] = {
                              "DARK",
                              "DEATH",
                              "!MOB",
                              "INDOORS",
                              "PEACEFUL",
                              "SOUNDPROOF",
                              "!TRACK",
                              "!MAGIC",
                              "TUNNEL",
                              "PRIVATE",
                              "GODROOM",
                              "HOUSE (R)",
                              "HCRSH (R)",
                              "ATRIUM(R)",
                              "OLC   (R)",
                              "* (R)",			/* BFS MARK */
                              "MURKY",
                              "COURT",
                              "BROADCAST",
                              "RECEIVE",
                              "ARENA",
                              "RED BASE",
                              "BLUE BASE",
                              "NORECALL",
                              "NOSUMMON",
                              "\n"
                          };


/* EX_x */
const char *exit_bits[] = {
                              "DOOR",
                              "CLOSED",
                              "LOCKED",
                              "PICKPROOF",
                              "HIDDEN",
                              "PASSAGE",
                              "NOMOB",
                              "NOPASSDOOR",
                              "BASHPROOF",
                              "\n"
                          };

/* SECT_ */
const char *sector_types[] = {
                                 "Inside",
                                 "City",
                                 "Field",
                                 "Forest",
                                 "Hills",
                                 "Mountains",
                                 "Water (Swim)",
                                 "Water (No Swim)",
                                 "Underwater",
                                 "In Flight",
                                 "Quicksand",
                                 "Lava",
                                 "\n"
                             };


/* SEX_x */
const char *genders[] =
    {
        "Neutral",
        "Male",
        "Female"
    };

const char *gend_he[] =
    {
        "it",
        "he",
        "she"
    };

/* POS_x */
const char *position_types[] = {
                                   "Dead",
                                   "Mortally wounded",
                                   "Incapacitated",
                                   "Stunned",
                                   "Sleeping",
                                   "Resting",
                                   "Sitting",
                                   "Fighting",
                                   "Standing",
                                   "Camping",
                                   "Guarding",
                                   "\n"
                               };


/* PLR_x */
const char *player_bits[] = {
                                "KILLER",
                                "THIEF",
                                "FROZEN",
                                "DONTSET",
                                "WRITING",
                                "MAILING",
                                "CSH",
                                "SITEOK",
                                "NOSHOUT",
                                "NOTITLE",
                                "DELETED",
                                "LOADRM",
                                "!WIZL",
                                "!DEL",
                                "INVST",
                                "CRYO",
                                "EDITING",
                                "NOAUTOTITLE",
                                "JUSTREIN",
                                "QUESTOR",
                                "TOAD",
                                "JUST_DIED",
                                "\n"
                            };

const char *player_bits2[] = {
                                 "CLAN BIT1",
                                 "CLAN BIT2",
                                 "CLAN BIT3",
                                 "CLAN LEAD",
                                 "\n"
                             };

const char *player_bits3[] = {
                                 "\n"
                             };

/* MOB_x */
const char *action_bits[] = {
                                "SPEC",
                                "SENTINEL",
                                "SCAVENGER",
                                "ISNPC",
                                "AWARE",
                                "AGGR",
                                "STAY-ZONE",
                                "WIMPY",
                                "AGGR_EVIL",
                                "AGGR_GOOD",
                                "AGGR_NEUTRAL",
                                "MEMORY",
                                "HELPER",
                                "!CHARM",
                                "!SUMMN",
                                "!SLEEP",
                                "!BASH",
                                "!BLIND",
                                "!Corpse",
                                "PET",
                                "ETHEREAL",
                                "FAST_REGEN",
                                "!FLEE",
                                "ASSASSIN",
                                "HOLDED",
                                "QUESTM",
                                "MONK",
                                "GANG_LEADER",
                                "GANG",
                                "SHOP_KEEPER",
                                "CAN_TALK",
                                "SECRATIVE",
                                "\n"
                            };


const char *action_bits2[] = {
                                 "NO_BURN",
                                 "MORE_BURN",
                                 "NO_FREEZE",
                                 "MORE_FREEZE",
                                 "NO_ACID",
                                 "MORE_ACID",
                                 "CAN_BURN",
                                 "CAN_FREEZE",
                                 "CAN_ACID",
                                 "GAZE_PETRIF",
                                 "\n"
                             };

const char *action_bits3[] = {
                                 "CAN_TALK",
                                 "CAN'T_FLEE",
                                 "\n"
                             };

/* PRF_x */
const char *preference_bits[] = {
                                    "BRIEF",
                                    "COMPACT",
                                    "DEAF",
                                    "!TELL",
                                    "DHP",
                                    "DMANA",
                                    "DMOVE",
                                    "A_EXIT",
                                    "!HASS",
                                    "QUEST",
                                    "SUMN",
                                    "!REP",
                                    "LIGHT",
                                    "C1",
                                    "C2",
                                    "!WIZ",
                                    "L1",
                                    "L2",
                                    "!AUC",
                                    "!GOS",
                                    "!GTZ",
                                    "RMFLG",
                                    "!CLAN",
                                    "!OOC",
                                    "!WAR",
                                    "!ARENA",
                                    "A_DIR",
                                    "A_SAC",
                                    "SPARE1",
                                    "SPARE2",
                                    "SPARE3",
                                    "A_SAVE",
                                    "\n"
                                };

const char *preference_bits2[] = {
                                     "RNCRN1",
                                     "RNCRN2",
                                     "RNCRN3",
                                     "WAR_DRUHARI",
                                     "WAR_YLLANTRA",
                                     "RETIRED",
                                     "ARENA_RED",
                                     "ARENA_BLUE",
                                     "AFK",
                                     "AUTOMAP",
                                     "ASSASSIN",
                                     "NOQUIT",
                                     "A_LOOT",
                                     "NOINFO",
                                     "A_SPLIT",
                                     "A_GRAT",
                                     "S1",
                                     "CONCEAL",
                                     "TUMBLE",
                                     "!MISS_E",
                                     "S5",
                                     "S6",
                                     "REG",
                                     "A_ASSIST",
                                     "ALERT",
                                     "NOWHO",
                                     "!MISS_F",
                                     "AUTO_SCAN",
                                     "LINEWRAP",
                                     "RUNNING",
                                     "DISP_DAM",
                                     "DISP_DESC",
                                     "\n"
                                 };

/* AFF_x */
const char *affected_bits[] =
    {
        "BLIND",
        "INVISIBILITY",
        "DET-ALIGN",
        "DET-INVIS",
        "DET-MAGIC",
        "SENSE LIFE",
        "WATERWALK",
        "SANCTUARY",
        "GROUP",
        "CURSE",
        "INFRAVISION",
        "POISON",
        "PROT-EVIL",
        "PROT-GOOD",
        "SLEEP",
        "!TRACK",
        "HOLDED",
        "SHIELD",
        "SNEAK",
        "HIDE",
        "DEATHDANCE",
        "CHARM",
        "FLYING",
        "H2O BREATH",
        "PROT FIRE",
        "EQUINOX",
        "PASSDOOR",
        "DEFLECTION",
        "FORCE FIELD",
        "FIRE SHIELD",
        "HOLY AURA",
        "TARGET",
        "\n"
    };

/* AFF2_x */
const char *affected_bits2[] =
    {
        "MIRROR IMAGE",
        "STONESKIN",
        "FARSEE",
        "ENH HEAL",
        "ENH MANA",
        "ENH MOVE",
        "HELD",
        "GUARDED",
        "BURNING",
        "FREEZING",
        "ACIDED",
        "PROT COLD",
        "BLINK",
        "HASTE",
        "ADRENALIN",
        "BERSERK",
        "PROT LIGHTNING",
        "RESIST_MAGIC",
        "WRATH_E",
        "NAP",
        "REGENERATION",
        "PETRIFY",
        "ULTRA",
        "PRISM",
        "BEHEAD",
        "ROLLING",
        "KATA",
        "MEDITATE",
        "BATTLECRY",
        "STALK",
        "MOVE HIDDEN",
        "PROTECTION",
        "\n"
    };

/* AFF_x */
const char *affected_bits3[] =
    {
        "PASSDOOR",
        "GR_ARMOR",
        "GR_INVIS",
        "GR_FLY",
        "GR_WATER",
        "GR_DEF",
        "GR_FORCE",
        "GR_FIRE",
        "QUAD",
        "STRONG_MIND",
        "GUARD",
        "SLOW",
        "WEB",
        "INTESIFY",
        "PLAGUE",
        "SHAMROCK",
        "SHELTER",
        "SHROUD",
        "RED",
        "BLUE",
        "HAS RED",
        "HAS BLUE",
        "HOG",
        "ENLIGHTMENT",
        "DARKENING",
        "ENDURE",
        "TEMP_BLIND",
        "DISTRACT",
        "FLEEING",
        "AMBUSH",
        "CHOKING",
        "SHIELD_OF_FAITH",
        "\n"
    };


/* CON_x */
const char *connected_types[] = {
                                    "Playing",			// 0
                                    "Disconnecting",
                                    "Get name",
                                    "Confirm name",
                                    "Get password",
                                    "Get new PW",
                                    "Confirm new PW",
                                    "Select sex",
                                    "Select class",
                                    "Reading MOTD",
                                    "Main Menu",                    // 10
                                    "Get descript.",
                                    "Changing PW 1",
                                    "Changing PW 2",
                                    "Changing PW 3",
                                    "Self-Delete 1",
                                    "Self-Delete 2",
                                    "Get Attrib",
                                    "Get Race",
                                    "I-EDITING",
                                    "R-EDITING",                         // 20
                                    "Z-EDITING"
                                    "Reincarn 1",
                                    "Reincarn 2",
                                    "M-EDITING",
                                    "Race select",
                                    "Class select",
                                    "1",
                                    "2",
                                    "3",
                                    "4",
                                    "5",                                      //31
                                    "6",
                                    "7",
                                    "Object edit",
                                    "Room edit",
                                    "Zone edit",
                                    "Mobile edit",
                                    "Shop edit",
                                    "Q Align",
                                    "Q Namepol",
                                    "Q Email",                                    //41
                                    "Q Name1",
                                    "Q Color",
                                    "Q Double delete",
                                    "Ident conning",
                                    "Ident conned",
                                    "Ident reading",
                                    "Ident read",
                                    "Asking name",
                                    "Town Select",
                                    "Deity select",
                                    "Deity confirm",
                                    "\n"
                                };


/* WEAR_x - for eq list */
const char *where[] = {
                          "<&yused as light&0>      ",
                          "<&cworn on finger&0>     ",
                          "<&cworn on finger&0>     ",
                          "<&cworn around neck&0>   ",
                          "<&cworn around neck&0>   ",
                          "<&cworn on body&0>       ",
                          "<&cworn on head&0>       ",
                          "<&cworn on legs&0>       ",
                          "<&cworn on feet&0>       ",
                          "<&cworn on hands&0>      ",
                          "<&cworn on arms&0>       ",
                          "<&cworn as a shield&0>   ",
                          "<&cworn about body&0>    ",
                          "<&cworn about waist&0>   ",
                          "<&cworn around wrist&0>  ",
                          "<&cworn around wrist&0>  ",
                          "<&wwielded&0>            ",
                          "<&wheld&0>               ",
                          "<&wdual-wielded&0>       ",
                          "<&cworn on back&0>       ",
                          "<&cworn on face&0>       ",
                          "<&cworn on ears&0>       ",
                          "<&cworn on eyes&0>       "
                      };


/* WEAR_x - for stat */
const char *equipment_types[] = {
                                    "Used as light",
                                    "Worn on right finger",
                                    "Worn on left finger",
                                    "First worn around Neck",
                                    "Second worn around Neck",
                                    "Worn on body",
                                    "Worn on head",
                                    "Worn on legs",
                                    "Worn on feet",
                                    "Worn on hands",
                                    "Worn on arms",
                                    "Worn as a shield",
                                    "Worn about body",
                                    "Worn around waist",
                                    "Worn around right wrist",
                                    "Worn around left wrist",
                                    "Wielded",
                                    "Held",
                                    "Dual-Wielded",
                                    "Worn on the back",
                                    "Worn on face",
                                    "Worn on ears",
                                    "Worn on eyes",
                                    "\n"
                                };


/* ITEM_x (ordinal object types) */
const char *item_types[] = {
                               "UNDEFINED",
                               "LIGHT",
                               "SCROLL",
                               "WAND",
                               "STAFF",
                               "WEAPON",
                               "FIREWEAPON",
                               "ARROW",
                               "TREASURE",
                               "ARMOR",
                               "POTION",
                               "WORN",
                               "UNUSUAL",
                               "TRASH",
                               "TRAP",
                               "CONTAINER",
                               "PAPER",
                               "LIQ CONTAINER",
                               "KEY",
                               "FOOD",
                               "MONEY",
                               "PEN",
                               "BOAT",
                               "FOUNTAIN",
                               "RED_FLAG",
                               "BLUE_FLAG",
                               "TABLE",
                               "T CARD",
                               "CLOUD",
                               "TROPHY",
                               "PORTAL",
                               "LEVER",
                               "BUTTON",
                               "\n"
                           };


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
                              "TAKE",
                              "FINGER",
                              "NECK",
                              "BODY",
                              "HEAD",
                              "LEGS",
                              "FEET",
                              "HANDS",
                              "ARMS",
                              "SHIELD",
                              "ABOUT",
                              "WAIST",
                              "WRIST",
                              "WIELD",
                              "HOLD",
                              "BACK",
                              "FACE",
                              "EARS",
                              "EYES",
                              "\n"
                          };


/* ITEM_x (extra bits) */
const char *extra_bits[] = {
                               "GLOW",
                               "HUM",
                               "!RENT",
                               "!DONATE",
                               "!INVIS",
                               "INVISIBLE",
                               "MAGIC",
                               "!DROP",
                               "BLESS",
                               "!GOOD",
                               "!EVIL",
                               "!NEUTRAL",
                               "!MAGE",
                               "!CLERIC",
                               "!THIEF",
                               "!WARRIOR",
                               "!SELL",
                               "RETURNING",
                               "!REMOVE",
                               "ENGRAVED",
                               "2-Handed",
                               "AUTOENGRAVE",
                               "METAL",
                               "!DRUID",
                               "!DISP_MAG",
                               "ORGANIC",
                               "!RANGER",
                               "LABEL",
                               "BURIED",
                               "EGO",
                               "!MONK",
                               "HIDDEN_EGO",
                               "\n"
                           };

const char *extra_bits2[] = {
                                "BURRIED",
                                "freeOVE",
                                "freeAVED",
                                "freended",
                                "freeENGRAVE",
                                "freeRGE",
                                "freeRGE",
                                "FROM_SHOP",
                                "SLAY_EVIL",
                                "SLAY_GOOD",
                                "BLOODY",
                                "POISONED",
                                "!HAMMER",
                                "!FORGE",
                                "\n"
                            };

/* APPLY_x */
const char *apply_types[] = {
                                "NONE",
                                "STR",
                                "DEX",
                                "INT",
                                "WILL POWER",
                                "CON",
                                "CHA",
                                "CLASS",
                                "LEVEL (111 for perm)",
                                "AGE",
                                "CHAR_WEIGHT",
                                "CHAR_HEIGHT",
                                "ENERGY",
                                "HIT",
                                "MOVE",
                                "GOLD",
                                "EXP",
                                "MAGIC_RESIST",
                                "HITROLL",
                                "DAMROLL",
                                "SAVING_PARA",
                                "SAVING_ROD",
                                "SAVING_PETRI",
                                "SAVING_BREATH",
                                "SAVING_SPELL",
                                "REGENERATION",
                                "*** SKILL/SPELL: Type name ***",
                                "\n"
                            };


/* CONT_x */
const char *container_bits[] = {
                                   "CLOSEABLE",
                                   "PICKPROOF",
                                   "CLOSED",
                                   "LOCKED",
                                   "\n",
                               };


/* LIQ_x */
const char *drinks[] =
    {
        "water",
        "beer",
        "wine",
        "ale",
        "dark ale",
        "whisky",
        "lemonade",
        "firebreather",
        "local speciality",
        "slime mold juice",
        "milk",
        "tea",
        "coke",
        "blood",
        "salt water",
        "clear water",
        "\n"
    };


/* other constants for liquids ******************************************/


/* one-word alias for each drink */
const char *drinknames[] =
    {
        "water",
        "beer",
        "wine",
        "ale",
        "ale",
        "whisky",
        "lemonade",
        "firebreather",
        "local",
        "juice",
        "milk",
        "tea",
        "coke",
        "blood",
        "salt",
        "water",
        "\n"
    };


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int drink_aff[][3] = {
                               {0, 1, 10},
                               {3, 2, 5},
                               {5, 2, 5},
                               {2, 2, 5},
                               {1, 2, 5},
                               {6, 1, 4},
                               {0, 1, 8},
                               {10, 0, 0},
                               {3, 3, 3},
                               {0, 4, -8},
                               {0, 3, 6},
                               {0, 1, 8},
                               {0, 1, 6},
                               {0, 2, -1},
                               {0, 1, -2},
                               {0, 0, 13}
                           };


/* color of the various drinks */
const char *color_liquid[] =
    {
        "clear",
        "brown",
        "clear",
        "brown",
        "dark",
        "golden",
        "red",
        "green",
        "clear",
        "light green",
        "white",
        "brown",
        "black",
        "red",
        "clear",
        "crystal clear"
    };


/* level of fullness for drink containers */
const char *fullness[] =
    {
        "less than half ",
        "about half ",
        "more than half ",
        ""
    };

const char *carry_cond[]=
    {
        "unburdened",
        "lightly burdened",
        "burdened",
        "heavily burdened"
    };

/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
const struct str_app_type str_app[26] =
    {
        {0.3, 0.20, 0, 0},		/* 0  */
        {0.4, 0.30, 3, 1},		/* 1  */
        {0.5, 0.30, 3, 2},
        {0.38, 0.38, 10, 3},		/* 3  */
        {0.43, 0.43, 25, 4},
        {0.48, 0.48, 55, 5},		/* 5  */
        {0.53, 0.53, 80, 6},
        {0.58, 0.58, 90, 7},
        {0.63, 0.63, 100, 8},
        {0.68, 0.68, 110, 9},
        {0.73, 0.73, 120, 10},		/* 10  */
        {0.78, 0.78, 130, 11},
        {0.82, 0.82, 140, 12},
        {0.86, 0.86, 150, 13},		/* 13  */
        {0.90, 0.90, 160, 15},
        {0.94, 0.95, 170, 17},		/* 15  */
        {1.00, 1.00, 190, 19},
        {1.05, 1.06, 220, 21},
        {1.10, 1.12, 250, 25},		/* 18  */
        {1.15, 1.18, 290, 30},
        {1.20, 1.24, 340, 35},		/* 20  */
        {1.26, 1.30, 400, 40},
        {1.32, 1.37, 460, 45},
        {1.38, 1.45, 520, 50},
        {1.46, 1.54, 580, 55},
        {1.55, 1.64, 650, 65}		/* 25   */
    };


/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[26] = {
            {-99, -99, -90, -99, -60},
            {-90, -90, -60, -90, -50},
            {-80, -80, -40, -80, -45},
            {-70, -70, -30, -70, -40},
            {-60, -60, -30, -60, -35},
            {-50, -50, -20, -50, -30},
            {-40, -40, -20, -40, -25},
            {-30, -30, -15, -30, -20},
            {-20, -20, -15, -20, -15},
            {-15, -10, -10, -20, -10},
            {-10, -5, -10, -15, -5},	/* 10 */
            {-5, 0, -5, -10, 0},
            {0, 0, 0, -5, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 5, 0, 0, 0},
            {5, 10, 0, 5, 5},
            {10, 15, 5, 10, 10},
            {15, 20, 10, 15, 15},
            {15, 20, 10, 15, 15},	/* 20 */
            {20, 25, 10, 15, 20},
            {20, 25, 15, 20, 20},
            {25, 25, 15, 20, 20},
            {25, 30, 15, 25, 25},
            {25, 30, 15, 25, 25}
        };



/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[36] = {
                                   1,				/* 0 */
                                   2,				/* 1 */
                                   2,
                                   2,
                                   2,
                                   2,				/* 5 */
                                   2,
                                   2,
                                   3,
                                   3,
                                   3,				/* 10 */
                                   3,
                                   3,
                                   3,
                                   4,
                                   4,				/* 15 */
                                   4,
                                   4,
                                   4,
                                   4,
                                   4,				/* 20 */
                                   5,
                                   5,
                                   5,
                                   5,
                                   5,				/* 25 */
                                   5,
                                   5,
                                   5,
                                   5,
                                   5,				/* 30 */
                                   5,
                                   5,
                                   5,
                                   5,
                                   5				/* 35 */
                               };


/* [dex] apply (all) */
struct dex_app_type dex_app[26] = {
                                      {-80, -7, 6},
                                      {-70, -6, 6},
                                      {-60, -5, 5},
                                      {-50, -4, 4},		/* 3 */
                                      {-48, -3, 3},
                                      {-44, -2, 2},
                                      {-40, -1, 1},
                                      {-36, 0, 0},
                                      {-32, 0, 0},
                                      {-28, 0, 0},
                                      {-24, 0, 0},			/* 10 */
                                      {-20, 0, 0},
                                      {-16, 0, 0},
                                      {-12, 0,  0},
                                      {-8, 1, -1},
                                      {-4, 1, -1},
                                      {0 , 1, -2},			// 16
                                      {5, 2, -2},
                                      {10, 3, -3},			/* 18 */
                                      {15, 3, -3},
                                      {20, 3, -4},           /* 20 */
                                      {26, 4, -5},
                                      {32, 4, -5},
                                      {38, 5, -6},
                                      {46, 6, -7},
                                      {55, 7, -8}            /* 25 */
                                  };



/* [con] apply (all) */
const struct con_app_type con_app[26] =
    {
        {-4, 20},			/* 0 */
        {-3, 25},			/* 1 */
        {-2, 30},
        {-2, 35},			/* 3 */
        {-2, 40},
        {-1, 45},			/* 5 */
        {-1, 50},
        {-1, 55},
        {0, 60},
        {0, 65},
        {0, 70},			/* 10 */
        {1, 75},
        {2, 80},
        {3, 85},
        {4, 88},
        {5, 90},			/* 15 */
        {6, 95},
        {7, 97},
        {8, 99},			/* 18 */
        {9, 99},
        {10, 99},			/* 20 */
        {11, 99},
        {13, 99},
        {15, 99},
        {17, 99},
        {20, 99}			/* 25 */
    };



/* [int] apply (all) */
const struct int_app_type int_app[26] =
    {
        {0},			/* 0 */
        {1},			/* 1 */
        {2},
        {3},			/* 3 */
        {4},
        {5},			/* 5 */
        {6},
        {7},
        {8},
        {9},
        {10},			/* 10 */
        {11},
        {12},
        {13},
        {14},
        {15},			/* 15 */
        {16},
        {17},
        {18},			/* 18 */
        {19},
        {20},			/* 20 */
        {22},
        {24},
        {26},
        {28},
        {31}			/* 25 */
    };

const struct wis_app_type wis_app[26] =
    {
        {-3},			/* 0 */
        {-3},			/* 1 */
        {-2},
        {-2},			/* 3 */
        {-2},
        {-1},			/* 5 */
        {-1},
        {-1},
        {0},
        {0},
        {0},			/* 10 */
        {1},
        {1},
        {2},
        {2},
        {3},			/* 15 */
        {3},
        {4},
        {5},			/* 18 */
        {6},
        {7},			/* 20 */
        {8},
        {9},
        {10},
        {11},
        {12}			/* 25 */
    };




const char *spell_wear_off_msg[] = {
                                       "RESERVED DB.C",		/* 0 */
                                       "You feel less protected.",	/* 1 */
                                       "This is a bug. Please report this ->Teleport!",
                                       "You feel less righteous.",
                                       "You feel a cloak of blindness disolve.",
                                       "This is a bug. Please report this ->Burning Hands!",		/* 5 */
                                       "This is a bug. Please report this ->Call Lightning",
                                       "You feel more self-confident.",
                                       "You feel your strength return.",
                                       "This is a bug. Please report this ->Clone!",
                                       "This is a bug. Please report this ->Color Spray!",		/* 10 */
                                       "This is a bug. Please report this ->Control Weather!",
                                       "This is a bug. Please report this ->Create Food!",
                                       "This is a bug. Please report this ->Create Water!",
                                       "This is a bug. Please report this ->Cure Blind!",
                                       "This is a bug. Please report this ->Cure Critic!",		/* 15 */
                                       "This is a bug. Please report this ->Cure Light!",
                                       "You feel more optimistic.",
                                       "You feel less aware.",
                                       "Your eyes stop tingling.",
                                       "You can no more detect magic auras.",	/* 20 */
                                       "You can no more detect poison.",
                                       "This is a bug. Please report this ->Dispel Evil!",
                                       "This is a bug. Please report this ->Earthquake!",
                                       "This is a bug. Please report this ->Enchant Weapon!",
                                       "This is a bug. Please report this ->Energy Drain!",		/* 25 */
                                       "You are no longer covered in flames.",
                                       "This is a bug. Please report this ->Harm!",
                                       "This is a bug. Please report this ->Heal!",
                                       "You feel yourself exposed.",
                                       "This is a bug. Please report this ->Lightning Bolt!",		/* 30 */
                                       "This is a bug. Please report this ->Locate object!",
                                       "This is a bug. Please report this ->Magic Missile!",
                                       "You feel less sick.",
                                       "You feel less protected.",
                                       "This is a bug. Please report this ->Remove Curse!",		/* 35 */
                                       "The white aura around your body fades.",
                                       "This is a bug. Please report this ->Shocking Grasp!",
                                       "You don't feel sleepy anymore.",
                                       "You feel weaker.",
                                       "This is a bug. Please report this ->Summon!",			/* 40 */
                                       "This is a bug. Please report this ->Ventriloquate!",
                                       "This is a bug. Please report this ->Word of Recall!",
                                       "This is a bug. Please report this ->Remove Poison!",
                                       "You feel less aware of your suroundings.",
                                       "This is a bug. Please report this ->Animate Dead!",		/* 45 */
                                       "This is a bug. Please report this ->Dispel Good!",
                                       "This is a bug. Please report this ->Group Armor!",
                                       "This is a bug. Please report this ->Group Heal!",
                                       "This is a bug. Please report this ->Group Recall!",
                                       "Your night vision seems to fade.",	/* 50 */
                                       "Your feet seem less boyant.",
                                       "This is a bug. Please report this ->Relocate!",
                                       "This is a bug. Please report this ->Peace!",
                                       "You can no longer fly.",
                                       "You feel your movements speeding up to normal again.",
                                       "You feel less insulated.",
                                       "Your gills disappear and you return to normal.",
                                       "This is a bug. Please report this ->Group Fly!",
                                       "This is a bug. Please report this ->Group Invis!",
                                       "This is a bug. Please report this ->Ressurection!",		/* 60*/
                                       "You feel a warm again.",
                                       "You are no longer covered in flames.",
                                       "This is a bug. Please report this ->Cure serious!",
                                       "This is a bug. Please report this ->Minute Meteor!",
                                       "This is a bug. Please report this ->Area lightning!",
                                       "You feel less determined.",
                                       "You feel your mystic shield dwindle off.",
                                       "You feel yourself becoming solid again.",
                                       "You feel your strength returning.",
                                       "Your skin becomes supple again.",	/* 70*/
                                       "You feel warm again.",
                                       "You feel normal again.",
                                       "You feel your skin become soft again.",
                                       "This is a bug. Please report this ->MATERIALIZE.",
                                       "This is a bug. Please report this ->faerie fog!",
                                       "You feel less endurable.",
                                       "This is a bug. Please report this ->Conj Elemental!",
                                       "This is a bug. Please report this ->Gate!",
                                       "You feel restful.",
                                       "You are no longer protected by a deflection shield.",
                                       "This is a bug. Please report this ->GROUP DEFL!",
                                       "This is a bug. Please report this ->SUMM ANC DRAG!",
                                       "Your courage returns!",
                                       "You are no longer outlined by a pale light.",
                                       "Your body regenerates at normal rate again.",
                                       "You regain your senses.",
                                       "You feel the acid finnaly vaporize.",
                                       "Your images recombine into one.",
                                       "You stop blinking.",
                                       "You start to slow down.",	/* 90*/
                                       "This is a bug. Please report this ->DISPEL MAGIC!",
                                       "You feel your life force return to you.",
                                       "You feel your shell of warm dissipate.",
                                       "This is a bug. Please report this ->DISINTIGRATE!",
                                       "You are no coverd in flames.",
                                       "You feel your blood purify.",
                                       "You finally landed.",
                                       "This is a bug. Please report this ->HOLD MONSTER!",
                                       "This is a bug. Please report this ->HOLY WORD!",
                                       "This is a bug. Please report this ->LOCATE CRAETURE!",	/* 100*/
                                       "This is a bug. Please report this ->IDENTIFY!",
                                       "This is a bug. Please report this ->prism",
                                       "You are no longer protected by a fire shield.",
                                       "This is a bug. Please report this ->GR FIRE SHIELD!",
                                       "You are no longer proteced by a force field.",
                                       "This is a bug. Please report this ->GR force field!",
                                       "You feel your health return to normal.",
                                       "Your link to mana returns to normal.",
                                       "You feel your stamina return to normal.",
                                       "This is a bug. Please report this ->ASTRAL WALK!",		/* 110*/
                                       "You shake of the madness.",
                                       "This is a bug. Please report this ->Power of nature!",
                                       "You are no longer covered in flames.",
                                       "This is a bug. Please report this ->Restore!",
                                       "This is a bug. Please report this ->Synost",
                                       "You become to feel normal again. Was that a dream?",
                                       "Your aura weakens!",
                                       "You feel sober.",
                                       "This is a bug. Please report this ->Trasnport",		/* 120*/
                                       "This is a bug. Please report this ->Retrasnport",
                                       "This is a bug. Please report this ->Arcane!",
                                       "This is a bug. Please report this ->Portal!",
                                       "This is a bug. Please report this ->Enchant armor!",
                                       "This is a bug. Please report this ->Create spring!",
                                       "You no longer feel safe from lightning.",
                                       "This is a bug. Please report this ->heling touch",
                                       "You no longer feel wrath of the Earth upon you.",
                                       "This is a bug. Please report this ->clairvoyance!",
                                       "This is a bug. Please report this ->ray of disr",
                                       "This is a bug. Please report this ->ressurection",
                                       "You are back to normal hitpoints again.",
                                       "Your prismatic sphere fades away.",
                                       "You are no longer dealing out ultra-damage.",
                                       "You can move again.",
                                       "This is a bug. Please report this ->CREATE_RAIN!",
                                       "This is a bug. Please report this ->AREA_EARTHQUAKE!",
                                       "This is a bug. Please report this ->GROUP_POWER_HEAL!",
                                       "You feel much weaker.",
                                       "This is a bug. Please report this ->RAGNAROK!",
                                       "This is a bug. Please report this ->TORNADO!",
                                       "This is a bug. Please report this ->ASTRAL_PROJECTION",
                                       "This is a bug. Please report this ->OMNI_EYE",
                                       "You don't feel so charming anymore.",
                                       "This is a bug. Please report this ->HOLY_TOUCH",
                                       "This is a bug. Please report this ->LOCAL_TELEPORT",
                                       /*"",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",*/
                                       "You feel less intelligent now. Urgh!",
                                       "This is a bug. Please report this ->RECHARGE",
                                       "This is a bug. Please report this ->free",
                                       "Your shroud of obscurement dwiddles off.",
                                       "This is a bug. Please report this ->mass sleep",
                                       "You manage to somehow tear off the web.",
                                       "You feel less protected from magic.",
                                       "Your runic shelter wears off.",
                                       "This is a bug. Please report this ->sunray",
                                       "You feel less depressed.",
                                       "You are no longer affected by shillelagh.",
                                       "You feel your intelligence return.",
                                       "You don't feel so lucky anymore.",
                                       "This is a bug. Please report this ->spiritual hammer",
                                       "Your are no longer under benediction.",
                                       "You no longer feel divine.",
                                       "Your spells are back to normal power.",
                                       "The shroud of darkness slowly fades.",
                                       "This is a bug. Please report this ->restoration",
                                       "You don't feel so vital anymore.",
                                       "This is a bug. Please report this ->baptize",
                                       "This is a bug. Please report this ->retrieve corpse",
                                       "You are cured! The plague is gone!",
                                       "This is a bug. Please report this ->cure plague",
                                       "This is a bug. Please report this ->atonement",
                                       "You are not so agile anymore.",
                                       "The energy globe collapses and explodes.",
                                       "Your holy armor wears off.",
                                       "You no longer feel holy presence within.",
                                       "You no longer feel enlighted.",
                                       "You shake off the dark feelings.",
                                       "You regain your breath.",
                                       "This is a bug. Please report this ->cure",
                                       "You feel less protected.",
                                       "You feel less protected.",
                                       "This is a bug. Please report this ->heal",
                                       "You feel less protected.",
                                       "You feel less protected.",
                                       "This is a bug. Please report this ->invigorate",
                                       "You feel less protected.",
                                       "This is a bug. Please report this ->invigorate all",
                                       "This is a bug. Please report this ->miracle",
                                       "You are no longer protected by Santcuary.",
                                       "This is a bug. Please report this ->punishment",
                                       "This is a bug. Please report this ->dispel evil",
                                       "This is a bug. Please report this ->spiritual hammer",
                                       "This is a bug. Please report this ->judgment",
                                       "This is a bug. Please report this ->smite evil",
                                       "This is a bug. Please report this ->Hammer of justice",
                                       "This is a bug. Please report this ->rapture",
                                       "This is a bug. Please report this ->wog",
                                       "This is a bug. Please report this ->divine i",
                                       "You no longer sense alignment.",
                                       "This is a bug. Please report this ->quench",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "",
                                       "This is a bug. Please report this ->sate",
                                       "Illumination slowly fades out.",
                                       "You are no longer sensible to hidden life forms.",
                                       "This is a bug. Please report this ->recall",
                                       "This is a bug. Please report this ->peace",
                                       "This is a bug. Please report this ->summon",
                                       "This is a bug. Please report this ->vot",
                                       "This is a bug. Please report this ->visions",
                                       "This is a bug. Please report this ->rejuvenate",
                                       "You feel your vision return.",
                                       "You no longer feel inspired.",
                                       "You no longer feel safe here.",
                                       "This is a bug. Please report this ->revelation",
                                       "This is a bug. Please report this ->guiding light",
                                       "You no longer feel divine presence in this room.",
                                       "This is a bug. Please report this ->retr corpse",
                                       "This is a bug. Please report this ->true seeing",
                                       "This is a bug. Please report this ->summon avatar",
                                       "This is a bug. Please report this ->ressurection",
                                       "This is a bug. Please report this ->wish",
                                       "You feel less righteous.",		// 241
                                        "This is a bug. Please report this ->dispel good",
                                         "This is a bug. Please report this ->smite good",
                                       "You no longer feel protected by shield of faith."  
                                   };



const char *npc_class_types[] = {
                                    "Normal",
                                    "Undead",
                                    "\n"
                                };


const char *align_types[] = {
                                "Evil Incarnate",
                                "Demonic",
                                "Evil",
                                "Nasty",
                                "Naughty",
                                "Neutral",
                                "Nice",
                                "Helpful",
                                "Good",
                                "Saintly",
                                "God's child",
                                "\n"
                            };

const int rev_dir[] =
    {
        2,
        3,
        0,
        1,
        5,
        4,
        9,
        8,
        7,
        6,
        10
    };


const int movement_loss[] =
    {
        8,				/* Inside     */
        6,				/* City       */
        10,				/* Field      */
        15,				/* Forest     */
        25,				/* Hills      */
        35,				/* Mountains  */
        40,				/* Swimming   */
        40,				/* Unswimable */
        50,				/* Underwater */
        5,				/* Flight     */
        70,				/* quicksand  */
        50,				/* lava	      */
        40				/* artic	      */
    };


const char *weekdays[7] = {
                              "the Day of the Moon",
                              "the Day of Lightning",
                              "the Day of the Deception",
                              "the Day of Thunder",
                              "the Day of Freedom",
                              "the day of the Great Gods",
                              "the Day of the Sun"};


const char *month_name[17] = {
                                 "Month of Winter",		/* 0 */
                                 "Month of the Winter Wolf",
                                 "Month of the Frost Giant",
                                 "Month of the Old Forces",
                                 "Month of the Grand Struggle",
                                 "Month of the Spring",
                                 "Month of Nature",
                                 "Month of Futility",
                                 "Month of the Dragon",
                                 "Month of the Sun",
                                 "Month of the Heat",
                                 "Month of the Battle",
                                 "Month of the Dark Shades",
                                 "Month of the Shadows",
                                 "Month of the Long Shadows",
                                 "Month of the Ancient Darkness",
                                 "Month of the Great Evil"
                             };

const char *rmode[] = {
                          "Never resets",
                          "Resets only when deserted",
                          "Always resets",
                          "\n"
                      };


/* Definitions necessary for MobProg support in OasisOLC */
const char *mobprog_types[] = {
                                  "ACT",
                                  "SPEECH",
                                  "RAND",
                                  "FIGHT",
                                  "DEATH",
                                  "HITPRCNT",
                                  "ENTRY",
                                  "GREET",
                                  "ALL_GREET",
                                  "GIVE",
                                  "BRIBE",
                                  "HOUR",
                                  "TIME",
                                  "WEAR",
                                  "REMOVE",
                                  "SAC",
                                  "LOOK",
                                  "EXA",
                                  "ZAP",
                                  "GET",
                                  "DROP",
                                  "DAMAGE",
                                  "REPAIR",
                                  "PULL",
                                  "PUSH",
                                  "SLEEP",
                                  "REST",
                                  "LEAVE",
                                  "SCRIPT",
                                  "USE",
                                  "RANDIW",
                                  "ACTION",
                                  "\n"
                              };


const char *room_affections[] = {
                                    "FOG",
                                    "TRAP",
                                    "ILLUMINATED",
                                    "\n"
                                };




int get_otype( char *type )
{
    int x;

    for ( x = 0; x < (sizeof(item_types) / sizeof(item_types[0])); x++ )
        if ( !str_cmp(type, item_types[x]) )
            return x;
    return -1;
}

int get_aflag( char *flag )
{
    int x;

    for ( x = 0; x < (sizeof(affected_bits) / sizeof(affected_bits[0])); x++ )
        if ( !str_cmp(flag, affected_bits[x]) )
            return x;
    return -1;
}


char *obj_condition_names[]=
    {

        "&Rruined&0",
        "&rdamaged&0",
        "&rpoor&0",
        "&yvery worn&0",
        "&yworn&0",
        "&yfair&0",
        "&cscratched&0",
        "&cgood&0",
        "&cvery good&0",
        "&wexcellent&0",
        "&wexcellent&0",
    };
char *obj_condition_names_mono[]=
    {
        "ruined",
        "damaged",
        "poor",
        "very worn",
        "worn",
        "fair",
        "scratched",
        "good",
        "very good",
        "excellent",
        "excellent",
    };
