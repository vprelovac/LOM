/* *************************************************************************  File: class.h                                         Part of LostLands*
*  Usage: header file for central character contstants f                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* preamble *************************************************************/


#include <sys/types.h>

/* char and mob-related defines *****************************************/

/* PC classes */
#define CLASS_UNDEFINED	  -1
#define CLASS_MAGIC_USER  (1 << 0)	/* 1 */
#define CLASS_CLERIC      (1 << 1)      /* 2 */
#define CLASS_THIEF       (1 << 2)      /* 4 */
#define CLASS_WARRIOR     (1 << 3)      /* 8 */
#define CLASS_RANGER      (1 << 4)      /* 16 */
#define CLASS_BARD        (1 << 5)      /* 32 */
#define CLASS_DRUID       (1 << 6)      /* 64 */
#define CLASS_NECRON      (1 << 7)      /* 128 */
#define CLASS_MONK	  (1 << 8)
#define CLASS_DEATHK      (1 << 9)
#define CLASS_NEWCLERIC   (1 << 10)

/* and more dual/triple definition */
#define CLASS_DUAL	  (1 << 14)	/* 16384 */ 
#define CLASS_TRIPLE	  (1 << 15)	/* 32768 */





#define ANYCLASS	CLASS_MAGIC_USER+CLASS_CLERIC+CLASS_THIEF+CLASS_WARRIOR+CLASS_RANGER+CLASS_BARD+CLASS_DRUID+CLASS_NECRON+CLASS_MONK+CLASS_DEATHK+CLASS_NEWCLERIC+CLASS_DUAL+CLASS_TRIPLE

#define SKILL_TYPE_NONE		    0
#define SKILL_TYPE_G_MAGIC 	(1 << 0)
#define SKILL_TYPE_E_MAGIC 	(1 << 1)
#define SKILL_TYPE_G_CLERIC	(1 << 2)
#define SKILL_TYPE_E_CLERIC	(1 << 3)
#define SKILL_TYPE_FIGHTER	(1 << 4)
#define SKILL_TYPE_THIEF	(1 << 5)
#define SKILL_TYPE_RANGER	(1 << 6)

#define SKILL_TYPE_MAGIC	SKILL_TYPE_G_MAGIC | SKILL_TYPE_E_MAGIC
#define SKILL_TYPE_CLERIC	SKILL_TYPE_G_CLERIC | SKILL_TYPE_E_CLERIC
#define SKILL_TYPE_MUNDANE	SKILL_TYPE_THIEF | SKILL_TYPE_FIGHTER | SKILL_TYPE_RANGER
#define SKILL_TYPE_ALL		SKILL_TYPE_DRUID | SKILL_TYPE_MUNDANE
#define SKILL_TYPE_DRUID        SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC


/* various multi-classes */
#define CLASS_MAG_CLE	  CLASS_MAGIC_USER + CLASS_CLERIC + CLASS_DUAL
#define CLASS_MAG_THI	  CLASS_MAGIC_USER + CLASS_THIEF + CLASS_DUAL
#define CLASS_MAG_FIG     CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_DUAL
#define CLASS_MAG_CLE_FIG CLASS_MAGIC_USER + CLASS_CLERIC + CLASS_WARRIOR + CLASS_TRIPLE
#define CLASS_MAG_THI_FIG CLASS_MAGIC_USER + CLASS_THIEF + CLASS_WARRIOR + CLASS_TRIPLE
#define CLASS_CLE_THI     CLASS_CLERIC + CLASS_THIEF + CLASS_DUAL
#define CLASS_CLE_FIG     CLASS_CLERIC + CLASS_WARRIOR + CLASS_DUAL
#define CLASS_FIG_THI     CLASS_WARRIOR + CLASS_THIEF + CLASS_DUAL

#define NUM_PC_RACE     10

#define RACE_HUMAN	0
#define RACE_ELF	1 
#define RACE_GOBLIN 	2
#define RACE_DWARF	3
#define RACE_HALFLING	4
#define RACE_GNOME	5
#define RACE_DROW       6
#define RACE_OGRE       7
#define RACE_TROLL      8
#define RACE_ORC        9
#define RACE_ENT        10

#define RACE_DRAGON     11

/* NPC races (currently unused - feel free to implement!) */
#define RACE_NORMAL	12
#define RACE_UNDEAD     13
#define RACE_HUMANOID   14
#define RACE_ANIMAL     15
#define RACE_GIANT      16
#define RACE_INSECTOID	17
#define RACE_DEMON	18
#define RACE_MAGICAL    19
#define RACE_OOZE       20   
#define RACE_GOD	21
#define RACE_SMALL_ANIMAL 22
#define RACE_BIRD	23


#define RACE_DARKGNOME  99

#define MAX_NPC_RACE	23

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* Positions */
#define POS_DEAD       0	/* dead			*/
#define POS_MORTALLYW  1	/* mortally wounded	*/
#define POS_INCAP      2	/* incapacitated	*/
#define POS_STUNNED    3	/* stunned		*/
#define POS_SLEEPING   4	/* sleeping		*/
#define POS_RESTING    5	/* resting		*/
#define POS_SITTING    6	/* sitting		*/
#define POS_FIGHTING   7	/* fighting		*/
#define POS_STANDING   8	/* standing		*/
#define POS_CAMPING    9
#define POS_GUARDING   10


/* Player flags: used by char_data.char_specials.act */
#define PLR_KILLER	(1 << 0)   /* Player is a player-killer		*/
#define PLR_THIEF	(1 << 1)   /* Player is a player-thief		*/
#define PLR_FROZEN	(1 << 2)   /* Player is frozen			*/
#define PLR_DONTSET     (1 << 3)   /* Don't EVER set (ISNPC bit)	*/
#define PLR_WRITING	(1 << 4)   /* Player writing (board/mail/olc)	*/
#define PLR_MAILING	(1 << 5)   /* Player is writing mail		*/
#define PLR_CRASH	(1 << 6)   /* Player needs to be crash-saved	*/
#define PLR_SITEOK	(1 << 7)   /* Player has been site-cleared	*/
#define PLR_NOSHOUT	(1 << 8)   /* Player not allowed to shout/goss	*/
#define PLR_NOTITLE	(1 << 9)   /* Player not allowed to set title	*/
#define PLR_DELETED	(1 << 10)  /* Player deleted - space reusable	*/
#define PLR_LOADROOM	(1 << 11)  /* Player uses nonstandard loadroom	*/
#define PLR_NOWIZLIST	(1 << 12)  /* Player shouldn't be on wizlist	*/
#define PLR_NODELETE	(1 << 13)  /* Player shouldn't be deleted	*/
#define PLR_INVSTART	(1 << 14)  /* Player should enter game wizinvis	*/
#define PLR_CRYO	(1 << 15)  /* Player is cryo-saved (purge prog)	*/
#define PLR_EDITING     (1 << 16)  /* Player is zone editing            */
#define PLR_OLEDIT      (1 << 16)  /* PLyer using olc *meditmod*        */
#define PLR_NOSETTITLE  (1 << 17)  /* Won't autoset title when leveled  */
#define PLR_JUSTREIN    (1 << 18)  /* Just reincarnated... don't set    */
#define PLR_QUESTOR     (1 << 19)      
#define PLR_TOAD	(1 << 20)
#define PLR_JUSTDIED	(1 << 21)

/* Player flags: used by char_data.char_specials.act */
#define PLR2_CLAN1       (1 << 0)   /* Clan bit 1                        */
#define PLR2_CLAN2       (1 << 1)   /* Clan bit 2                        */
#define PLR2_CLAN3       (1 << 2)   /* Clan bit 3                        */
#define PLR2_CLANLEADER  (1 << 3)   /* Clan Leader                       */

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         (1 << 0)  /* Mob has a callable spec-proc	*/
#define MOB_SENTINEL     (1 << 1)  /* Mob should not move		*/
#define MOB_SCAVENGER    (1 << 2)  /* Mob picks up stuff on the ground	*/
#define MOB_ISNPC        (1 << 3)  /* (R) Automatically set on all Mobs	*/
#define MOB_AWARE	 (1 << 4)  /* Mob can't be backstabbed		*/
#define MOB_AGGRESSIVE   (1 << 5)  /* Mob hits players in the room	*/
#define MOB_STAY_ZONE    (1 << 6)  /* Mob shouldn't wander out of zone	*/
#define MOB_WIMPY        (1 << 7)  /* Mob flees if severely injured	*/
#define MOB_AGGR_EVIL	 (1 << 8)  /* auto attack evil PC's		*/
#define MOB_AGGR_GOOD	 (1 << 9)  /* auto attack good PC's		*/
#define MOB_AGGR_NEUTRAL (1 << 10) /* auto attack neutral PC's		*/
#define MOB_MEMORY	 (1 << 11) /* remember attackers if attacked	*/
#define MOB_HELPER	 (1 << 12) /* attack PCs fighting other NPCs	*/
#define MOB_NOCHARM	 (1 << 13) /* Mob can't be charmed		*/
#define MOB_NOSUMMON	 (1 << 14) /* Mob can't be summoned		*/
#define MOB_NOSLEEP	 (1 << 15) /* Mob can't be slept		*/
#define MOB_NOBASH	 (1 << 16) /* Mob can't be bashed (e.g. trees)	*/
#define MOB_NOBLIND	 (1 << 17) /* Mob can't be blinded		*/
#define MOB_NO_CORPSE	 (1 << 18) /* Doesn't leave a corpse		*/
#define MOB_PET		 (1 << 19) /* Mob is a pet		        */
#define MOB_ETHEREAL	 (1 << 20) /* Mob is invisible			*/
#define MOB_FASTREGEN    (1 << 21) /* Mob regens hp faster than normal  */
#define MOB_CANT_FLEE	 (1 << 22) /* Prevents from fleeing			*/
#define MOB_ASSASSIN     (1 << 23)
#define MOB_HOLDED	 (1 << 24)
#define MOB_QUESTMASTER  (1 << 25)
#define MOB_MONK         (1 << 26)
#define MOB_GANG_LEADER  (1 << 27) // unused
#define MOB_GANG	 (1 << 28)
#define MOB_SHOP	 (1 << 29)
#define MOB_TALK	 (1 << 30)
#define MOB_SECRETIVE	 (1 << 31)   // actions are not seen

/* Mobile flags: used by char_data.char_specials.act2 */
#define MOB2_NOBURN       (1 << 0) /* Mob can't be burned               */
#define MOB2_MOREBURN     (1 << 1) /* Mob more susceptable to burning   */
#define MOB2_NOFREEZE     (1 << 2) /* Mob can't be frozen		*/
#define MOB2_MOREFREEZE   (1 << 3) /* Mob more susceptable to freezing  */
#define MOB2_NOACID       (1 << 4) /* Mob can't be acided 		*/
#define MOB2_MOREACID     (1 << 5) /* Mob more susceptable to acid      */
#define MOB2_CANBURN      (1 << 6) /* Mob's touch causes burning        */
#define MOB2_CANFREEZE    (1 << 7) /* Mob's touch causes freezing       */
#define MOB2_CANACID      (1 << 8) /* Mob's touch causes acidburning    */
#define MOB2_GAZEPETRIFY  (1 << 9) /* Mob's gaze can petrify		*/
#define MOB2_NOWEAR       (1 << 10) /* can not wear eq		*/

/* Mobile flags: used by char_data.char_specials.act3 */
#define MOB3_CANTALK	  (1 << 0) /* Mob can speak			*/
#define MOB3_CANT_FLEE    (1 << 1) /* Prevents attackers from fleeing   */

/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       (1 << 0)  /* Room descs won't normally be shown	*/
#define PRF_COMPACT     (1 << 1)  /* No extra CRLF pair before prompts	*/
#define PRF_DEAF	(1 << 2)  /* Can't hear shouts			*/
#define PRF_NOTELL	(1 << 3)  /* Can't receive tells		*/
#define PRF_DISPHP	(1 << 4)  /* Display hit points in prompt	*/
#define PRF_DISPMANA	(1 << 5)  /* Display mana points in prompt	*/
#define PRF_DISPMOVE	(1 << 6)  /* Display move points in prompt	*/
#define PRF_AUTOEXIT	(1 << 7)  /* Display exits in a room		*/
#define PRF_NOHASSLE	(1 << 8)  /* Aggr mobs won't attack		*/
#define PRF_QUEST	(1 << 9)  /* On quest				*/
#define PRF_SUMMONABLE	(1 << 10) /* Can be summoned			*/
#define PRF_NOREPEAT	(1 << 11) /* No repetition of comm commands	*/
#define PRF_HOLYLIGHT	(1 << 12) /* Can see in dark			*/
#define PRF_COLOR_1	(1 << 13) /* Color (low bit)			*/
#define PRF_COLOR_2	(1 << 14) /* Color (high bit)			*/
#define PRF_NOWIZ	(1 << 15) /* Can't hear wizline			*/
#define PRF_LOG1	(1 << 16) /* On-line System Log (low bit)	*/
#define PRF_LOG2	(1 << 17) /* On-line System Log (high bit)	*/
#define PRF_NOAUCT	(1 << 18) /* Can't hear auction channel		*/
#define PRF_NOGOSS	(1 << 19) /* Can't hear gossip channel		*/
#define PRF_NOGRATZ	(1 << 20) /* Can't hear grats channel		*/
#define PRF_ROOMFLAGS	(1 << 21) /* Can see room flags (ROOM_x)	*/
#define PRF_NOCLAN	(1 << 22) /* Can't hear clan channel            */
#define PRF_NOCHAT      (1 << 23) /* Can't hear chat channel            */
#define PRF_NOWAR       (1 << 24) /* Hears War channel                  */
#define PRF_NOTEAM	(1 << 25) /* Can't hear arena channel		*/
#define PRF_AUTODIR	(1 << 26) /* auto directions			*/
#define PRF_AUTOSAC	(1 << 27) /* auto sac corpses			*/
#define PRF_SPARE1  	(1 << 28) /* shows amount of gold		*/
#define PRF_SPARE2	(1 << 29) /* shows amount of xp 'til next level */
#define PRF_SPARE3 	(1 << 30) /* shows amount enemy is damaged	*/
#define PRF_AUTOSAVE	(1 << 31) /* shows amount enemy is damaged	*/

/* Preference flags: used by char_data.player_specials.pref2 */
#define PRF2_REINCARN1  (1 << 0)  /* Reincarned (low bit)		*/
#define PRF2_REINCARN2  (1 << 1)  /* Reincarned (mid bit)		*/
#define PRF2_REINCARN3  (1 << 2)  /* Reincarned (high bit)              */
#define PRF2_HAS_RED (1 << 3) /* Registered to fight for Druhari    */
#define PRF2_HAS_BLUE (1 << 4)/* Registered to fight for Yllantra   */
#define PRF2_RETIRED    (1 << 5)  /* No longer want to be in the war    */
#define PRF2_ARENA_RED  (1 << 6)  /* Red side in Arena			*/
#define PRF2_ARENA_BLUE (1 << 7)  /* Blue side in Arena                 */
#define PRF2_AFK	(1 << 8)  /* Away from Keyboard			*/
#define PRF2_AUTOMAP    (1 << 9)  /* automap on			*/
#define PRF2_ASSASSIN   (1 << 10) /* Assassin				*/
#define PRF2_NOQUIT     (1 << 11) /* Can't accidentally quit            */
#define PRF2_AUTOLOOT   (1 << 12) /* Autoloot				*/
#define PRF2_NOINFO      (1 << 13)
#define PRF2_AUTOSPLIT   (1<<14) 
#define PRF2_AUTOGRAT    (1<<15)
#define PRF2_S1          (1<<16)	// improve stat
#define PRF2_CONCEAL	 (1<<17)
#define PRF2_TUMBLE      (1<<18)
#define PRF2_NOMISSE	 (1<<19)
#define PRF2_S5		 (1<<20)
#define PRF2_S6		 (1<<21)
#define PRF2_REG	 (1<<22)
#define PRF2_AUTOASSIST  (1<<23)
#define PRF2_NOALERT	 (1<<24)
#define PRF2_NOWHO       (1<<25)
#define PRF2_NOMISSF     (1<<26)
#define PRF2_AUTOSCAN    (1<<27)
#define PRF2_LINEWRAP    (1<<28)
#define PRF2_RUNNING     (1<<29)
#define PRF2_DISPDAM     (1<<30)
#define PRF2_DISPSDESC   (1<<31)

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_BLIND             (1 << 0)	   /* (R) Char is blind		*/
#define AFF_INVISIBLE         (1 << 1)	   /* Char is invisible		*/
#define AFF_DETECT_ALIGN      (1 << 2)	   /* Char is sensitive to align*/
#define AFF_DETECT_INVIS      (1 << 3)	   /* Char can see invis chars  */
#define AFF_DETECT_MAGIC      (1 << 4)	   /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        (1 << 5)	   /* Char can sense hidden life*/
#define AFF_WATERWALK	      (1 << 6)	   /* Char can walk on water	*/
#define AFF_SANCTUARY         (1 << 7)	   /* Char protected by sanct.	*/
#define AFF_GROUP             (1 << 8)	   /* (R) Char is grouped	*/
#define AFF_CURSE             (1 << 9)	   /* Char is cursed		*/
#define AFF_INFRAVISION       (1 << 10)	   /* Char can see in dark	*/
#define AFF_POISON            (1 << 11)	   /* (R) Char is poisoned	*/
#define AFF_PROTECT_EVIL      (1 << 12)	   /* Char protected from evil  */
#define AFF_PROTECT_GOOD      (1 << 13)	   /* Char protected from good  */
#define AFF_SLEEP             (1 << 14)	   /* (R) Char magically asleep	*/
#define AFF_NOTRACK	      (1 << 15)	   /* Char can't be tracked	*/
#define AFF_HOLDED	      (1 << 16)	   /* Room for future expansion	*/
#define AFF_SHIELD	      (1 << 17)	   /* Char has shield spell	*/
#define AFF_SNEAK             (1 << 18)	   /* Char can move quietly	*/
#define AFF_HIDE              (1 << 19)	   /* Char is hidden		*/
#define AFF_DEATHDANCE	      (1 << 20)	   /* Char is Death Dancing	*/
#define AFF_CHARM             (1 << 21)	   /* Char is charmed		*/
#define AFF_FLYING            (1 << 22)    /* Char is Flying            */
#define AFF_WATERBREATH       (1 << 23)    /* Char can breath water     */
#define AFF_PROT_FIRE	      (1 << 24)    /* Char protected from fire  */
#define AFF_EQUINOX           (1 << 25)    /* Char needs +1 to damage   */
#define AFF_PASSDOOR          (1 << 26)    /* Char needs +2 to damage   */
#define AFF_DEFLECTION        (1 << 27)    /* Char needs +3 to damage   */
#define AFF_FORCE_FIELD       (1 << 28)    /* Char needs +4 to damage   */
#define AFF_FIRE_SHIELD       (1 << 29)    /* Char needs +5 to damage   */
#define AFF_SILVER            (1 << 30)    /* Char needs silver to dam  */
#define AFF_TARGET            (1 << 31)    /* Char is target of an event */



/* Affect bits: used in char_data.char_specials.saved.affected_by2 */
#define AFF2_MIRRORIMAGE      (1 << 0)     /* Char affected by mirImg   */
#define AFF2_STONESKIN        (1 << 1)     /* Char affected by StnSkin  */
#define AFF2_FARSEE           (1 << 2)     /* Char affected by Farsee   */
#define AFF2_ENH_HEAL	      (1 << 3)     /* Char affected by Enh Heal */
#define AFF2_ENH_MANA         (1 << 4)     /* Char affected by Enh Mana */
#define AFF2_ENH_MOVE         (1 << 5)     /* Char affected by Enh Move */
#define AFF2_HOLDPERSON       (1 << 6)     /* Char affected by Hold Per */
#define AFF2_GUARDED          (1 << 7)     /* Char is guarded by someone*/
#define AFF2_BURNING          (1 << 8)     /* Char is on fire           */
#define AFF2_FREEZING         (1 << 9)     /* Char is freezing          */
#define AFF2_ACIDED           (1 << 10)    /* Char is covered with acid */
#define AFF2_PROT_COLD	      (1 << 11)    /* Char is prot. from cold   */
#define AFF2_BLINK            (1 << 12)    /* Char is displaced         */
#define AFF2_HASTE	      (1 << 13)    /* Char is hasted		*/
#define AFF2_ADRENALIN        (1 << 14)
#define AFF2_BERSERK	      (1 << 15)
#define AFF2_PROT_LIGHTNING   (1 << 16)
#define AFF2_RESISTMAGIC      (1 << 17)    /* Char has 30% magic resistance */
#define AFF2_WRATHE	      (1 << 18)
#define AFF2_NAP	      (1 << 19)
#define AFF2_REGENERATE	      (1 << 20)
#define AFF2_PETRIFY	      (1 << 21)
#define AFF2_ULTRA	      (1 << 22)
#define AFF2_PRISM	      (1 << 23)
#define AFF2_BEHEAD           (1 << 24)
#define AFF2_ROLLING	      (1 << 25)
#define AFF2_KATA             (1 << 26)
#define AFF2_MEDITATE         (1 << 27)
#define AFF2_BATTLECRY        (1 << 28)
#define AFF2_STALK            (1 << 29)
#define AFF2_MOVE_HIDDEN      (1 << 30)

#define AFF2_PROTECTION       (1 << 31)

/* Affect bits: used in char_data.char_specials.saved.affected_by3 */
#define AFF3_PASSDOOR      (1 << 0)     /* Char walks through doors  */
#define AFF3_GR_ARMOR      (1 << 1)
#define AFF3_GR_INVIS      (1 << 2)
#define AFF3_GR_FLY        (1 << 3)
#define AFF3_GR_WATER      (1 << 4)
#define AFF3_GR_DEF        (1 << 5)
#define AFF3_GR_FORCE      (1 << 6)
#define AFF3_GR_FIRE       (1 << 7)
#define AFF3_QUAD          (1 << 8)
#define AFF3_STRONG_MIND   (1 << 9)
#define AFF3_GUARD         (1 << 10)
#define AFF3_SLOW          (1 << 11)
#define AFF3_WEB           (1 << 12)
#define AFF3_INTESIFY      (1 << 13)
#define AFF3_PLAGUE        (1 << 14)
#define AFF3_SHAMROCK      (1 << 15)
#define AFF3_SHELTER       (1 << 16)
#define AFF3_SHROUD        (1 << 17)
#define AFF3_RED           (1 << 18)
#define AFF3_BLUE          (1 << 19)
#define AFF3_HAS_RED       (1 << 20)
#define AFF3_HAS_BLUE      (1 << 21)
#define AFF3_HOG           (1 << 22)
#define AFF3_ENLIGHTMENT           (1 << 23)
#define AFF3_DARKENING           (1 << 24)
#define AFF3_ENDURE        (1 << 25)

//some combat affects
#define AFF3_TEMP_BLIND    (1 << 26)
#define AFF3_DISTRACT      (1 << 27)

#define AFF3_FLEEING       (1 << 28)
#define AFF3_AMBUSH        (1 << 29)
#define AFF3_CHOKE         (1 << 30)

#define AFF3_SOF           (1 << 31)


/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

