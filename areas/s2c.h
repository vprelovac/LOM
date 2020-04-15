#include "spells.h"
char *room_flags[]=
{
"ROOM_DARK",
"ROOM_DEATH",
"ROOM_NO_MOB",
"ROOM_INDOORS",
"ROOM_LAWFUL",
"ROOM_NEUTRAL",
"ROOM_CHAOTIC",
"ROOM_NO_MAGIC",
"ROOM_TUNNEL",
"ROOM_PRIVATE",
"ROOM_SAFE",
"ROOM_SOLITARY",
"ROOM_PET_SHOP",
"ROOM_NO_RECALL",
"ROOM_DONATION",
"ROOM_NODROPALL",
"ROOM_SILENCE",
"ROOM_LOGSPEECH",
"ROOM_NODROP",
"ROOM_CLANSTOREROOM",
"ROOM_NO_SUMMON",
"ROOM_NO_ASTRAL",
"ROOM_TELEPORT",
"ROOM_TELESHOWDESC",
"ROOM_NOFLOOR",
"ROOM_NOSUPPLICATE",
"ROOM_ARENA",
"ROOM_NOMISSILE",
"ROOM_PROTOTYPE",
};


char *exit_flags[]=
{
"EX_ISDOOR",
"EX_CLOSED",
"EX_LOCKED",
"EX_SECRET",
"EX_SWIM",
"EX_PICKPROOF",
"EX_FLY",
"EX_CLIMB",
"EX_DIG",
"EX_EATKEY",
"EX_NOPASSDOOR",
"EX_HIDDEN",
"EX_PASSAGE",
"EX_PORTAL",
"EX_RES1",
"EX_RES2",
"EX_xCLIMB",
"EX_xENTER",
"EX_xLEAVE",
"EX_xAUTO",
"EX_NOFLEE",
"EX_xSEARCHABLE",
"EX_BASHED",
"EX_BASHPROOF",
"EX_NOMOB",
"EX_WINDOW",
"EX_xLOOK",
};

char *item_types[]=
{

"ITEM_NONE"," ITEM_LIGHT"," ITEM_SCROLL"," ITEM_WAND"," ITEM_STAFF"," ITEM_WEAPON",
"ITEM_FIREWEAPON"," ITEM_MISSILE"," ITEM_TREASURE"," ITEM_ARMOR"," ITEM_POTION",
"ITEM_WORN"," ITEM_FURNITURE"," ITEM_TRASH"," ITEM_OLDTRAP"," ITEM_CONTAINER",
"ITEM_NOTE"," ITEM_DRINK_CON"," ITEM_KEY"," ITEM_FOOD"," ITEM_MONEY"," ITEM_PEN",
"ITEM_BOAT"," ITEM_CORPSE_NPC"," ITEM_CORPSE_PC"," ITEM_FOUNTAIN"," ITEM_PILL",
"ITEM_BLOOD"," ITEM_BLOODSTAIN"," ITEM_SCRAPS"," ITEM_PIPE"," ITEM_HERB_CON",
"ITEM_HERB"," ITEM_INCENSE"," ITEM_FIRE"," ITEM_BOOK"," ITEM_SWITCH"," ITEM_LEVER",
"ITEM_PULLCHAIN"," ITEM_BUTTON"," ITEM_DIAL"," ITEM_RUNE"," ITEM_RUNEPOUCH",
"ITEM_MATCH"," ITEM_TRAP"," ITEM_MAP"," ITEM_PORTAL"," ITEM_PAPER",
"ITEM_TINDER"," ITEM_LOCKPICK"," ITEM_SPIKE"," ITEM_DISEASE"," ITEM_OIL"," ITEM_FUEL",
"ITEM_EMPTY1"," ITEM_EMPTY2"," ITEM_MISSILE_WEAPON"," ITEM_PROJECTILE"," ITEM_QUIVER",
"ITEM_SHOVEL"," ITEM_SALVE"," ITEM_COOK"," ITEM_KEYRING"," ITEM_ODOR"
};

char *item_flags[]=
{

"ITEM_GLOW"," ITEM_HUM"," ITEM_DARK"," ITEM_LOYAL"," ITEM_EVIL"," ITEM_INVIS"," ITEM_MAGIC",
"ITEM_NODROP"," ITEM_BLESS"," ITEM_ANTI_GOOD"," ITEM_ANTI_EVIL"," ITEM_ANTI_NEUTRAL",
"ITEM_NOREMOVE"," ITEM_INVENTORY"," ITEM_ANTI_MAGE"," ITEM_ANTI_THIEF",
"ITEM_ANTI_WARRIOR"," ITEM_ANTI_CLERIC"," ITEM_ORGANIC"," ITEM_METAL"," ITEM_DONATION",
"ITEM_CLANOBJECT"," ITEM_CLANCORPSE"," ITEM_ANTI_VAMPIRE"," ITEM_ANTI_DRUID",
"ITEM_HIDDEN"," ITEM_POISONED"," ITEM_COVERING"," ITEM_DEATHROT"," ITEM_BURIED",
"ITEM_PROTOTYPE"," ITEM_NOLOCATE"," ITEM_GROUNDROT"," MAX_ITEM_FLAG"
  };





 char *apply_types[]=
 {
       "APPLY_NONE"," APPLY_STR"," APPLY_DEX"," APPLY_INT"," APPLY_WIS"," APPLY_CON",
"APPLY_SEX"," APPLY_CLASS"," APPLY_LEVEL"," APPLY_AGE"," APPLY_HEIGHT"," APPLY_WEIGHT",
"APPLY_MANA"," APPLY_HIT"," APPLY_MOVE"," APPLY_GOLD"," APPLY_EXP"," APPLY_AC",
"APPLY_HITROLL"," APPLY_DAMROLL"," APPLY_SAVING_POISON"," APPLY_SAVING_ROD",
"APPLY_SAVING_PARA"," APPLY_SAVING_BREATH"," APPLY_SAVING_SPELL"," APPLY_CHA",
"APPLY_AFFECT"," APPLY_RESISTANT"," APPLY_IMMUNE"," APPLY_SUSCEPTIBLE",
"APPLY_WEAPONSPELL"," APPLY_LCK"," APPLY_BACKSTAB"," APPLY_PICK"," APPLY_TRACK",
"APPLY_STEAL"," APPLY_SNEAK"," APPLY_HIDE"," APPLY_PALM"," APPLY_DETRAP"," APPLY_DODGE",
"APPLY_PEEK"," APPLY_SCAN"," APPLY_GOUGE"," APPLY_SEARCH"," APPLY_MOUNT"," APPLY_DISARM",
"APPLY_KICK"," APPLY_PARRY"," APPLY_BASH"," APPLY_STUN"," APPLY_PUNCH"," APPLY_CLIMB",
"APPLY_GRIP"," APPLY_SCRIBE"," APPLY_BREW"," APPLY_WEARSPELL"," APPLY_REMOVESPELL",
"APPLY_EMOTION"," APPLY_MENTALSTATE"," APPLY_STRIPSN"," APPLY_REMOVE"," APPLY_DIG",
"APPLY_FULL"," APPLY_THIRST"," APPLY_DRUNK"," APPLY_BLOOD"," APPLY_COOK",
"APPLY_RECURRINGSPELL"," APPLY_CONTAGIOUS"," APPLY_EXT_AFFECT"," APPLY_ODOR",
"APPLY_ROOMFLAG"," APPLY_SECTORTYPE"," APPLY_ROOMLIGHT"," APPLY_TELEVNUM",
"APPLY_TELEDELAY"," MAX_APPLY_TYPE"

 };

int smaug_spells[]={

SPELL_ACID_BLAST,              70,
SPELL_ACID_BREATH,            200,
SPELL_SENSE_LIFE,              102,   // alertness
SPELL_ANIMATE_DEAD,           231,
SPELL_RUNIC_SHELTER,        224,        // antimagic shell
SPELL_WATERBREATH,            236,
SPELL_ARMOR,                    1,
SPELL_ASTRAL_WALK,             90,
-1,           342,
-1,      343,
-1,             310,
-1,             301,
-1,        303,
SPELL_PROT_FIRE,              216,
SPELL_PROT_FIRE,              215,
SPELL_BLESS,                    3,
SPELL_BLINDNESS,                4,
SPELL_BURNING_HANDS,            5,
SPELL_CALL_LIGHTNING,           6,
SPELL_MAGIC_MISSILE,          63,   // cause
SPELL_MAGIC_MISSILE,             62,   // cause
SPELL_MAGIC_MISSILE,           64,   //cause
-1,          313,
-1,          302,
-1,              82,
-1,         105,
SPELL_CHARM,             7,
SPELL_CHILL_TOUCH,              8,
SPELL_COLOR_SPRAY,            10,
SPELL_INFRAVISION,         57,
SPELL_CONTROL_WEATHER,         11,
SPELL_CREATE_FOOD,                   100,
SPELL_FIREBALL,             85,
SPELL_CREATE_FOOD,             12,
SPELL_CREATE_SPRING,           80,
-1,          101,
SPELL_CREATE_WATER,            13,
SPELL_CURE_BLIND,          14,
SPELL_CURE_CRITIC,           15,
SPELL_CURE_LIGHT,              16,
SPELL_REMOVE_POISON,             43,
SPELL_CURE_SERIOUS,            61,
SPELL_CURSE,                   17,
SPELL_STONESKIN,              210,
SPELL_DETECT_ALIGN,             18,
SPELL_SENSE_LIFE,           44,
SPELL_DETECT_INVIS,            19,
SPELL_DETECT_MAGIC,            20,
SPELL_DETECT_POISON,           21,
-1,            86,
SPELL_DISPEL_EVIL,             22,
SPELL_DISPEL_MAGIC,            59,
-1,             305,
SPELL_DIVINITY,               112,
SPELL_STONESKIN,             212,
SPELL_INTELLIGIZE,             227,
SPELL_SLEEP,                  233,
SPELL_EARTHQUAKE,              23,
SPELL_BARKSKIN,        207,
SPELL_BEAUTIFY,           226,
SPELL_ENCHANT_WEAPON,          24,
SPELL_ENERGY_DRAIN,            25,
SPELL_FIREBALL,          312,
SPELL_FIREBALL,        218,
SPELL_FIREBALL,        217,
SPELL_FIREBALL,            340,
SPELL_FAERIE_FIRE,             72,
SPELL_FAERIE_FOG,              73,
-1,               222,
-1,                103,
SPELL_FIREBALL,                26,
SPELL_FIRE_SHIELD,              88,
SPELL_FIRE_BREATH,            201,
SPELL_FLAMESTRIKE,             65,
-1,                   60,
SPELL_WATERWALK,                  292,
SPELL_FLY,                     56,
SPELL_FROST_BREATH,           202,
-1,          304,
SPELL_GAS_BREATH,             203,
SPELL_TELEPORT,                    83,
SPELL_PROT_LIGHTNING,              104,
SPELL_FIREBALL,          307,
SPELL_HARM,                    27,
SPELL_HEAL,                    28,
-1,           298,
-1,         341,
SPELL_BLESS,          111,
SPELL_COLD_ARROW,               299,
SPELL_SHIELD,              221,
SPELL_IDENTIFY,                53,
SPELL_INFRAVISION,             77,
SPELL_CURE_CRITIC,           213,
SPELL_INVISIBLE,                   29,
SPELL_STRENGTH,        39,
-1,                  234,
SPELL_DETECT_ALIGN,          58,
-1,               109,
SPELL_LIGHTNING_BOLT,          30,
SPELL_LIGHTNING_BREATH,       204,
SPELL_LOCATE_OBJECT,           31,
SPELL_MAGIC_MISSILE,           32,
-1,        311,
SPELL_INVISIBLE,              69,
SPELL_WRAITHFORM,               290,
SPELL_BURNING_HANDS,      114,
-1,                297,
SPELL_WRAITHFORM,               74,
-1,             291,
SPELL_POISON,                  33,
-1,              294,
SPELL_PORTAL,                 220,
SPELL_ARMOR,              34,
-1,          314,
SPELL_STONESKIN,              211,
SPELL_RECHARGE,               229,
SPELL_SURCEASE,                 81,
SPELL_REMOVE_CURSE,            35,
-1,           230,
-1,             87,
-1,                95,
SPELL_BARKSKIN,             106,
SPELL_HEAL,            113,
SPELL_RESTORE_MANA,            84,
SPELL_DIVINITY,         345,
-1,               228,
SPELL_SANCTUARY,               36,
-1,        296,
-1,                    91,
-1,             225,
SPELL_SHIELD,                  67,
SPELL_SHOCKING_GRASP,          51,
SPELL_ARMOR,             89,
SPELL_SLEEP,                   38,
SPELL_AGILITY,                  205,
SPELL_FLY,           293,
-1,   344,
-1,        309,
-1,         306,
SPELL_ACID_BLAST,           295,
SPELL_LIGHTNING_BOLT,        115,
SPELL_STONESKIN,              66,
SPELL_COLOR_SPRAY,        308,
SPELL_SUMMON,                  40,
-1,              209,
SPELL_TELEPORT,                 2,
-1,                300,
SPELL_TELEPORT,              219,
SPELL_REGENERATE,         206,
SPELL_SENSE_LIFE,             235,
SPELL_FAERIE_FIRE,        110,
-1,               208,
-1,           41,
SPELL_WEAKEN,                  68,
-1,            214,
SPELL_WORD_OF_RECALL,          42,
-1,       246,

};



 char *	const	npc_race	[] =
{
"human", "elf", "dwarf", "halfling", "pixie", "vampire", "half-ogre",
"half-orc", "half-troll", "half-elf", "gith", "drow", "sea-elf",
"lizardman", "gnome", "r5", "r6", "r7", "r8", "troll",
"ant", "ape", "baboon", "bat", "bear", "bee",
"beetle", "boar", "bugbear", "cat", "dog", "dragon", "ferret", "fly",
"gargoyle", "gelatin", "ghoul", "gnoll", "gnome", "goblin", "golem",
"gorgon", "harpy", "hobgoblin", "kobold", "lizardman", "locust",
"lycanthrope", "minotaur", "mold", "mule", "neanderthal", "ooze", "orc",
"rat", "rustmonster", "shadow", "shapeshifter", "shrew", "shrieker",
"skeleton", "slime", "snake", "spider", "stirge", "thoul", "troglodyte",
"undead", "wight", "wolf", "worm", "zombie", "bovine", "canine", "feline",
"porcine", "mammal", "rodent", "avis", "reptile", "amphibian", "fish",
"crustacean", "insect", "spirit", "magical", "horse", "animal", "humanoid",
"monster", "god"
};

 char *mob_aff[]=
 {
"AFF_BLIND"," AFF_INVISIBLE"," AFF_DETECT_EVIL"," AFF_DETECT_INVIS",
"  AFF_DETECT_MAGIC"," AFF_DETECT_HIDDEN"," AFF_HOLD"," AFF_SANCTUARY",
"  AFF_FAERIE_FIRE"," AFF_INFRARED"," AFF_CURSE"," AFF_FLAMING"," AFF_POISON",
"  AFF_PROTECT"," AFF_PARALYSIS"," AFF_SNEAK"," AFF_HIDE"," AFF_SLEEP"," AFF_CHARM",
"  AFF_FLYING"," AFF_PASS_DOOR"," AFF_FLOATING"," AFF_TRUESIGHT"," AFF_DETECTTRAPS",
"  AFF_SCRYING"," AFF_FIRESHIELD"," AFF_SHOCKSHIELD"," AFF_HAUS1"," AFF_ICESHIELD",
"  AFF_POSSESS"," AFF_BERSERK"," AFF_AQUA_BREATH"," AFF_RECURRINGSPELL",
"  AFF_CONTAGIOUS"," MAX_AFFECTED_BY "
};  

#define NUM_SPEC 34
char *spec_names[]=
{
	"spec_breath_any","dragon",	
"spec_breath_acid"	,"acid_dragon",
"spec_breath_fire"	,"fire_dragon",
"spec_breath_frost"	,"cold_dragon",
"spec_breath_gas"	,"gas_dragon",
"spec_breath_lightning" ,"lightning_dragon",
"spec_cast_adept"	,"cleric",
"spec_cast_cleric"	,"cleric",
"spec_cast_mage"	,"magic_user",
"spec_cast_undead"	,"undead",
"spec_executioner"	,"executioner1",
"spec_fido"		,"fido",
"spec_guard"		,"cityguard",
"spec_janitor"		,"janitor",
"spec_mayor"		,"mayor1",
"spec_poison"		,"snake",
"spec_thief"		,"thief"
};