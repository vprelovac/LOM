/* ************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* preamble *************************************************************/


#include <sys/types.h>


#define EVENT_MOVE
#define SUPERMOB	50
#define MAX_TELLS 	10

#define DEFAULT_PROMPT "%hH %mE %vV %e> "
#define DEFAULT_COMBAT_PROMPT "%hH %mE %vV [%O: %d]> "

#define SUB_EVENT	9999 // subcmd for events

typedef struct	char_data		CHAR_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct  mob_prog_act_list 	MPROG_ACT_LIST;
typedef struct  mob_prog_data	 	MPROG_DATA;
typedef struct  index_data 		OBJ_INDEX_DATA;
typedef struct  index_data 		MOB_INDEX_DATA;
typedef struct  room_data 		ROOM_INDEX_DATA;
typedef struct	room_direction_data	EXIT_DATA;
typedef struct	extracted_char_data	EXTRACT_CHAR_DATA;
typedef struct	extracted_obj_data	EXTRACT_OBJ_DATA;

/* number of queues to use (reduces enqueue cost) */
#define NUM_EVENT_QUEUES    50

#define LEVEL_NEWBIE 5
#define OBJ_PERM 111
#define MAX_MOBKILL 100000
#define MAX_ROOMS_IN_MUD 1500  // bytes ; x8 rooms
#define MAX_MOBS_IN_MUD 1000  // bytes ; x8 mobs
#define MAX_CHAR_EVENTS 10

#define ITEM_BURIED 	(1<<28)
#define WRATH_CLOUD     11
#define OBJ_HEAD 	98
#define OBJ_SPRING	97

#define NOWHERE    -1    /* nil reference for room-database	*/
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/
#define ANYRACE    -1
#define ANYALIGN   -1

#define FORMAT_INDENT (1<<0)
#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING	 0		/* Playing - Nominal state	*/
#define CON_CLOSE	 1		/* Disconnecting		*/
#define CON_GET_NAME	 2		/* By what name ..?		*/
#define CON_NAME_CNFRM	 3		/* Did I get that right, x?	*/
#define CON_PASSWORD	 4		/* Password:			*/
#define CON_NEWPASSWD	 5		/* Give me a password for x	*/
#define CON_CNFPASSWD	 6		/* Please retype password:	*/
#define CON_QSEX	 7		/* Sex?				*/
#define CON_QCLASS	 8		/* Class?			*/
#define CON_RMOTD	 9		/* PRESS RETURN after MOTD	*/
#define CON_MENU	 10		/* Your choice: (main menu)	*/
#define CON_EXDESC	 11		/* Enter a new description:	*/
#define CON_CHPWD_GETOLD 12		/* Changing passwd: get old	*/
#define CON_CHPWD_GETNEW 13		/* Changing passwd: get new	*/
#define CON_CHPWD_VRFY   14		/* Verify new password		*/
#define CON_DELCNF1	 15		/* Delete confirmation 1	*/
#define CON_DELCNF2	 16		/* Delete confirmation 2	*/
#define CON_QATTRIB      17             /* query attributes 		*/
#define CON_QRACE        18		/* query race 			*/
#define CON_IEDITo	 19		/* OLC modes 			*/
#define CON_REDITo	 20
#define CON_ZEDITo	 21
#define CON_REINCNF1	 22
#define CON_REINCNF2     23
#define CON_MEDITo	 24
#define CON_RS           25
#define CON_CS           26
#define CON_DWAIT        27
#define CON_QSTAT        28


#define CON_QSTATS        29
#define CON_QSTATW        30
#define CON_QSTATI        31
#define CON_QSTATD        32
#define CON_QSTATC        33
#define CON_OEDIT	 34		/*. OLC mode - object edit     .*/
#define CON_REDIT	 35		/*. OLC mode - room edit       .*/
#define CON_ZEDIT	 36		/*. OLC mode - zone info edit  .*/
#define CON_MEDIT	 37		/*. OLC mode - mobile edit     .*/
#define CON_SEDIT	 38		/*. OLC mode - shop edit       .*/
#define CON_QALIGN   39

#define CON_QNAMEPOL     40
#define CON_QEMAIL       41
#define CON_GET_NAME1    42
#define CON_QCOLOR       43
#define CON_QDD          44
#define CON_IDCONING	 45		/* waiting for ident connection */
#define CON_IDCONED	 46		/* ident connection complete    */
#define CON_IDREADING	 47		/* waiting to read ident sock   */
#define CON_IDREAD	 48		/* ident results read           */
#define CON_ASKNAME	 49		/* Ask user for name            */
#define CON_ASKTOWN	 50
#define CON_DEITY	 51
#define CON_ASKDEITY	 52
#define CON_PC2NPC	 53




/* defined constants **********************************************/
#define LVL_IMPL	54
#define LVL_GRGOD	53
#define LVL_GOD		52
#define LVL_IMMORT	51

#define LVL_FREEZE	LVL_GRGOD



#define NUM_OF_DIRS	11	/* number of directions in a room (nsewud) */
#define NUM_WEARS	23	/* # of eq positions see objs.h */
#define NUM_CLASSES	13	/* # of PC classes see class.h */
#define NUM_RACES	20	/* # of PC races see class.h */

#define RENT_COST	200  /* flat rate for rent */

#define LAVA_DAMAGE   12   /* damage for going in Lava */
#define UNWAT_DAMAGE	10   /* damage for going underwater */
#define CRIT_DAMAGE     10   /* damage for being crit hit */

#define OPT_USEC	200000	/* 5 passes per second */
#define PASSES_PER_SEC	(1000000 / OPT_USEC)
#define RL_SEC		* PASSES_PER_SEC

#define PULSE_ZONE      (30 RL_SEC)
#define PULSE_MOBILE    (5 RL_SEC)
#define PULSE_VIOLENCE  (12)
#define PULSE_SPELL     (12)
#define PULSE_PLAYERS   (5 RL_SEC)

/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (12 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH       96          /* Max length of prompt        */
#define GARBAGE_SPACE		32          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE		1024        /* Static output buffer size   */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE	   (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

#define HISTORY_SIZE		5	/* Keep last 5 commands. */
#define MAX_STRING_LENGTH	16384
#define MAX_INPUT_LENGTH	256	/* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH	512	/* Max size of *raw* input */
#define MAX_MESSAGES		800
#define MAX_NAME_LENGTH		10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH		10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH	40  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH		50  /* Used in char_file_u *DO*NOT*CHANGE* */
#define EXDSCR_LENGTH		240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE		3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS		500 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT2		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT3		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT		9 /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define IDENT_LENGTH		8

#define OBJ_NEXT          1
#define OBJ_NEXTCONTENT   2
#define OBJ_NULL          3


#define CHAR_NEXT         1
#define CHAR_NEXTROOM     2
#define CHAR_NULL         3


/***********************************************************************
 * Structures                                                          *
 **********************************************************************/

typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed short int	sh_int;
typedef unsigned short int	ush_int;
typedef char			bool;
typedef char			byte;

typedef int	room_num;
typedef int	obj_num;
typedef int	obj_rnum;
typedef int	obj_vnum;
#ifndef AFRO
typedef unsigned long ulong;
#endif



struct Sarea_info
{
    int avg, max, min, num;
    byte connected_to[500];
    byte connected_from[500];
};

struct Sarea_info area_info[500];



/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
    char	*keyword;                 /* Keyword in look/examine          */
    char	*description;             /* What to see                      */
    struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/
struct obj_ref_type
{
    bool inuse;
    struct obj_ref_type *next;
    struct obj_data **var;
    int type;                    /* OBJ_xxxx */
};
struct char_ref_type
{
    bool inuse;
    struct char_ref_type *next;
    struct char_data **var;
    int type;
};



/* object flags; used in obj_data */
struct obj_flag_data {
    int	value[4];	/* Values of the item (see list)    */
    int  type_flag;	/* Type of item			    */
    ulong	wear_flags;	/* Where you can wear it	    */
    ulong	extra_flags;	/* If it hums, glows, etc.	    */
    ulong	extra_flags2;	/* If it hums, glows, etc.	    */
    int	weight;		/* Weigt what else                  */
    int	cost;		/* Value when sold (gp.)            */
    int	cost_per_day;	/* Cost to keep pr. real day        */
    int	timer;		/* Timer for object                 */
    byte data;
    ulong	bitvector;	/* To set chars bits                */
    ulong	bitvector2;	/* To set chars bits                */
    ulong	bitvector3;	/* To set chars bits                */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
    int location;      /* Which ability to change (APPLY_XXX) */
    int modifier;     /* How much it changes by              */
};


/* ================== Memory Structure for Objects ================== */
struct obj_data {
    obj_num item_number;		/* Where in data-base			*/
    room_num in_room;		/* In what room -1 when conta/carr	*/

    struct obj_flag_data obj_flags;/* Object information               */
    struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

    char	*name;                    /* Title of object :get etc.        */
    char	*description;		  /* When in room                     */
    char	*short_description;       /* when worn/carry/in cont.         */
    char	*action_description;      /* What to write when used          */
    char *attack_verb;		  /* supplimentary attack verb        */
    struct extra_descr_data *ex_description; /* extra descriptions     */
    struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
    struct char_data *worn_by;	  /* Worn by?			      */
    char owner_name[20];		  /* for Engraved items		      */
    sh_int worn_on;		  /* Worn where?		      */
    sh_int orig_zone;		  /* Origination Zone		      */
    int bound_spell, bound_spell_level, bound_spell_timer;
    int damage;
    int orig_value;	// this is spare spot for now

    struct obj_data *in_obj;       /* In what object NULL when none    */
    struct obj_data *contains;     /* Contains objects                 */

    struct obj_data *next_content; /* For 'contains' lists             */
    struct obj_data *next;         /* For the object list              */
    MPROG_ACT_LIST *	mpact;		/* mudprogs */
    int			mpactnum;	/* mudprogs */
    sh_int		mpscriptpos;
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files		   */
struct obj_file_elem {
    obj_num item_number;
    sh_int locate;  /* that's the (1+)wear-location (when equipped) or
         (20+)index in obj file (if it's in a container) BK */
    int	value[4];
    long	extra_flags;
    long	extra_flags2;
    int	weight;
    int cost;
    int	timer;
    int damage;
    int orig_value;	// this is spare spot for now
    int data;	// level
    long	bitvector;
    long	bitvector2;
    long	bitvector3;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
    int bound_spell, bound_spell_level, bound_spell_timer;
    char owner_name[20];
    char name[100];
    char description[100];
    char short_description[100];
};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
    int	time;
    int	rentcode;
    int	net_cost_per_diem;
    int	gold;
    int	account;
    int	nitems;
    int	spare0;
    int	spare1;
    int	spare2;
    int	spare3;
    int	spare4;
    int	spare5;
    int	spare6;
    int	spare7;
};
/* ======================================================================= */


/* room-related structures ************************************************/


struct room_direction_data {
    char	*general_description;       /* When look DIR.			*/

    char	*keyword;		/* for open/close			*/

    sh_int exit_info;		/* Exit info				*/
    obj_num key;			/* Key's number (-1 for no key)		*/
    room_num to_room;		/* Where direction leads (NOWHERE)	*/
    room_num to_room_vnum;       /* the vnum of the room. Used for OLC   */
};


struct teleport_data {
    int time;
    int targ;
    long mask;
    int cnt;
    int obj;
};

/* Broadcast Info, if it's a broad   */
struct broadcast_data {
    int channel;  /* number from 0 to 32 */
    int targ1;
    int targ2;
};

/* ================== Memory Structure for room ======================= */
struct room_data {
    room_num number;		/* Rooms number	(vnum)		      */
    sh_int zone;                 /* Room zone (for resetting)          */
    ush_int	sector_type;            /* sector type (move/hide)            */
    char	*name;                  /* Rooms name 'You are ...'           */
    char	*description;           /* Shown when entered                 */
    struct extra_descr_data *ex_description; /* for examine/look       */
    struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
    int room_flags;		/* DEATH,DARK ... etc                 */

    //   struct teleport_data *tele;  /* Teleport Info, if it's a teleport  */
    //   struct broadcast_data *broad; /* Broadcast Info, if it's a broad   */

    byte light;                  /* Number of lightsources in room     */
    byte blood;
    SPECIAL(*func);

    struct obj_data *contents;   /* List of items in room              */
    struct char_data *people;    /* List of NPC / PC in room           */
    long  room_affections;    /* bitvector for spells/skills */
    struct char_data *listeners; // who else is listening
    MPROG_ACT_LIST *	mpact;		    /* mudprogs */
    int			mpactnum;	    /* mudprogs */
    sh_int		mpscriptpos;
    int  progtypes;  /* program types for MOBProg		*/
    MPROG_DATA *	mudprogs;	    /* mudprogs */

};
/* ====================================================================== */

/* char-related structures ************************************************/
struct class_type1
{
    int thac0_0;
    int thac0_50;
};

/* memory structure for characters */
struct memory_rec_struct {
    long	id;
    struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;

/* MOBProgram foo */

/* Mob program structures */
struct  act_prog_data
{
    struct act_prog_data *next;
    void *vo;
};

struct mob_prog_act_list {
    struct mob_prog_act_list *next;
    char *buf;
    struct char_data *ch;
    struct obj_data *obj;
    void *vo;
};


struct mob_prog_data {
    struct mob_prog_data *next;
    int type;
    bool	 triggered;
    int	 resetdelay;
    char *arglist;
    char *comlist;
};



extern bool MOBTrigger;
/*
#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024
  */

#define ACT_PROG           (1<<0)
#define SPEECH_PROG        (1<<1)
#define RAND_PROG          (1<<2)
#define FIGHT_PROG         (1<<3)
#define DEATH_PROG         (1<<4)
#define HITPRCNT_PROG      (1<<5)
#define ENTRY_PROG         (1<<6)
#define GREET_PROG         (1<<7)
#define ALL_GREET_PROG     (1<<8)
#define GIVE_PROG          (1<<9)
#define BRIBE_PROG         (1<<10)
#define HOUR_PROG         (1<<11)
#define TIME_PROG         (1<<12)
#define WEAR_PROG         (1<<13)
#define REMOVE_PROG        (1<<14)
#define SAC_PROG         (1<<15)
#define LOOK_PROG         (1<<16)
#define EXA_PROG         (1<<17)
#define ZAP_PROG         (1<<18)
#define GET_PROG         (1<<19)
#define DROP_PROG         (1<<20)
#define DAMAGE_PROG        (1<<21)
#define REPAIR_PROG        (1<<22)
#define PULL_PROG         (1<<23)
#define PUSH_PROG         (1<<24)
#define SLEEP_PROG         (1<<25)
#define REST_PROG         (1<<26)
#define LEAVE_PROG         (1<<27)
#define SCRIPT_PROG        (1<<28)
#define USE_PROG         (1<<29)
#define RANDIW_PROG        (1<<30)
#define ACTION_PROG      (1<<31)



#define ERROR_PROG        -1
#define IN_FILE_PROG      -2
/*
typedef enum
{
  ACT_PROG, SPEECH_PROG, RAND_PROG, FIGHT_PROG, DEATH_PROG, HITPRCNT_PROG, 
  ENTRY_PROG, GREET_PROG, ALL_GREET_PROG, GIVE_PROG, BRIBE_PROG, HOUR_PROG, 
  TIME_PROG, WEAR_PROG, REMOVE_PROG, SAC_PROG, LOOK_PROG, EXA_PROG, ZAP_PROG, 
  GET_PROG, DROP_PROG, DAMAGE_PROG, REPAIR_PROG, RANDIW_PROG, SPEECHIW_PROG, 
  PULL_PROG, PUSH_PROG, SLEEP_PROG, REST_PROG, LEAVE_PROG, SCRIPT_PROG,
  USE_PROG
} prog_types;
  */
/*
 * For backwards compatability
 */
#define RDEATH_PROG DEATH_PROG
#define ENTER_PROG  ENTRY_PROG
#define RFIGHT_PROG FIGHT_PROG
#define RGREET_PROG GREET_PROG
#define OGREET_PROG GREET_PROG


// SMAUG mobprogs

/* end of MOBProg foo */

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
    int hours, day, month;
    sh_int year;
};

/* These data contain information about a players time data */
struct time_data {
    time_t birth;    /* This represents the characters age                */
    time_t logon;    /* Time of the last logon (used to calculate played) */
    int	played;     /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
    char	passwd[MAX_PWD_LENGTH+1]; /* character's password      */
    char	*name;	       /* PC / NPC s name (kill ...  )         */
    char	*short_descr;  /* for NPC 'actions'                    */
    char	*long_descr;   /* for 'look'			       */
    char	*description;  /* Extra descriptions                   */
    char	*title;        /* PC / NPC's title                     */
    byte sex;           /* PC / NPC's sex                       */
    int  class;         /* PC / NPC's class		       */
    byte level;         /* PC / NPC's level                     */
    int race;	       /* PC / NPC's race		       */
    int	hometown;      /* PC s Hometown (zone)                 */
    struct time_data time;  /* PC's AGE in days                 */
    sh_int weight;       /* PC / NPC's weight                    */
    sh_int height;       /* PC / NPC's height                    */
    int zone_edit;      /* PC Zone being edited *reditmod*      */
    struct obj_data *obj_buffer;/*PC object edit buffer*oeditmod*/
    struct char_data *mob_buf;/*PC mob edit buffer     *meditmod*/
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
    int str;
    int str_add;      /* 000 - 100 if strength 18             */
    int intel;
    int wis;
    int dex;
    int con;
    int cha;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
    int mana;
    int max_mana;     /* Max move for PC/NPC			   */
    int hit;
    int max_hit;      /* Max hit for PC/NPC                      */
    int move;
    int max_move;     /* Max move for PC/NPC                     */

    int hitr;
    int mover;
    int manar;

    int armor;        /* Internal -100..100, external -10..10 AC */
    int magic_armor;
    int	gold;           /* Money carried                           */
    int	bank_gold;	/* Gold the char has in a bank account	   */
    int	exp;            /* The experience of the player            */

    int hitroll;       /* Any bonus or penalty to the hit roll    */
    int damroll;       /* Any bonus or penalty to the damage roll */
};


/*
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
    int	alignment;		/* +-3000 for alignments                */
    int  real_alignment;
    long	idnum;			/* player's idnum; -1 for mobiles	*/
    long	act;			/* act flag for NPC's; player flag for PC's */
    long	act2;			/* act flag for NPC's; player flag for PC's */
    long	act3;			/* act flag for NPC's; player flag for PC's */

    long	affected_by;		/* Bitvector for spells/skills affected by */
    long	affected_by2;		/* Bitvector for spells/skills affected by */
    long	affected_by3;		/* Bitvector for spells/skills affected by */
    sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
    struct char_data *fighting;	/* Opponent				*/
    struct char_data *hunting;	/* Char hunted by this char		*/

    struct char_data *hired_by;
    byte position;		/* Standing, fighting, sleeping, etc.	*/
    struct obj_data *wrathobj;
    sh_int	carry_weight;		/* Carried weight			*/
    sh_int	equip_weight;		/* equip weight			*/
    sh_int carry_items;		/* Number of items carried		*/
    int	timer;			/* Timer for update			*/

    struct char_special_data_saved saved; /* constants saved in plrfile	*/
    struct char_data *betted_on;
    int bet_amt;
    room_num wasin;
};


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
    ubyte skills[MAX_SKILLS+4];	/* array of skills plus skill 0		*/
    int PADDING0;		/* How many can you learn yet this level*/
    bool talks[MAX_TONGUE];	/* PC s Tongues 0 for NPC		*/
    int	wimp_level;		/* Below this # of hit points, flee!	*/
    int freeze_level;		/* Level of god who froze char, if any	*/
    sh_int invis_level;		/* level of invisibility		*/
    room_num load_room;		/* Which room to place char in		*/
    ulong	pref;			/* preference flags for PC's.		*/
    ulong	pref2;			/* preference flags for PC's.		*/
    int bad_pws;		/* number of bad password attemps	*/
    int conditions[3];         /* Drunk, full, thirsty			*/
    byte rooms_visited[MAX_ROOMS_IN_MUD];
    byte mobs_killed[MAX_MOBS_IN_MUD];


    /* spares below for future expansion.  You can change the names from
       'sparen' to something meaningful, but don't change the order.  */

    int mirror_images;
    int clan_rank;
    int term;
    int size;
    int clan_expelled;
    int linewrap;
    int spells_to_learn;		/* How many can you learn yet this level*/
    int olc_zone;
    int wrath;
    int questpoints;
    int questgiver;
    int questmob;
    int questitem;
    int questcdown;
    int questnext;
    int clan;
    int honor;
    int killed_mob;
    int killed_player;
    int killed_by_player;
    int killed_by_mob;
    int inn_num;			/* room vnum of the inn */
    long	channel;		/* 32 broadcasting channels */
    long	mkscore;
    long pkscore;
    long	spare19; // NE DIRAJ  style!
    long	spare20; // a je aggresivnes
    float mobratio;
    float freef1;
    float freef2;
    float freef3;
    int sold_in_tellas;;
    char enrage[40];
    char prompt[100];
    char prompt_combat[100];
    char walkin[80];
    char walkout[80];
    char grats[50];
    char email[60];
    int deity;
    int faith;
    char free_room[112];
};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
    struct player_special_data_saved saved;

    char	*poofin;		/* Description on arrival of a god.     */
    char	*poofout;		/* Description upon a god's exit.       */
    struct alias *aliases;	/* Character's aliases			*/
    struct quest *quests;	/* Character's questbits		*/
    long last_tell;		/* idnum of last tell from		*/
    void *last_olc_targ;		/* olc control				*/
    int last_olc_mode;		/* olc control				*/
    char *group_name;
    int priv;		// private channel ID
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
    byte last_direction;     /* The last direction the monster went     */
    int	attack_type;        /* The Attack Type Bitvector for NPC's     */
    byte default_pos;        /* Default position for NPC                */
    memory_rec *memory;	    /* List of attackers to remember	       */
    byte damnodice;          /* The number of damage dice's	       */
    byte damsizedice;        /* The size of the damage dice's           */
    byte attack_num;	    /* The number of attacks		       */
    int wait_state;	    /* Wait state for bashed mobs	       */
    sh_int orig_zone;	    /* Origination Zone 		       */
    char *brag;
    int deity;
    int style;
};


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
    sh_int type;          /* The type of spell that caused this      */
    sh_int duration;      /* For how long its effects will last      */
    int modifier;       /* This is added to apropriate ability     */
    int location;        /* Tells which ability to change(APPLY_XXX)*/
    long	bitvector;       /* Tells which bits to set (AFF_XXX)       */
    long	bitvector2;       /* Tells which bits to set (AFF2_XXX)       */
    long	bitvector3;       /* Tells which bits to set (AFF3_XXX)       */

    struct affected_type *next;
};

struct leech_type {
    int type;
    int mana;
    struct char_data *from, *to;
    struct leech_type *next;
};
struct leech_type *leech_list;       /* leech list  */
#define SPELL_ALL -3492345
#define SPELL_EXTRACT -3492346

/* Structure used for chars following other chars */
struct follow_type {
    struct char_data *follower;
    struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== */
struct char_data {
    int pfilepos;			 /* playerfile pos		  */
    sh_int nr;                            /* Mob's rnum			  */
    room_num in_room;                     /* Location (real room number)	  */
    room_num was_in_room;		 /* location for linkdead people  */
    room_num wasbefore;  		 /* was in before spell */
    int howlong;				 /* how much longer*/
    sh_int exp_ratio;
    byte regen;

    struct char_player_data player;       /* Normal data                   */
    struct char_ability_data real_abils;	 /* Abilities without modifiers   */
    struct char_ability_data aff_abils;	 /* Abils with spells/stones/etc  */
    struct char_point_data points;        /* Points                        */
    struct char_special_data char_specials;	/* PC/NPC specials	  */
    struct player_special_data *player_specials; /* PC specials		  */
    struct mob_special_data mob_specials;	/* NPC specials		  */

    struct affected_type *affected;       /* affected by what spells       */
    struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

    struct obj_data *carrying;            /* Head of list                  */
    struct descriptor_data *desc;         /* NULL for mobiles              */

    struct char_data *next_in_room;     /* For room->people - list         */
    struct char_data *next;             /* For either monster or ppl-list  */
    struct char_data *next_fighting;    /* For fighting list               */

    struct follow_type *followers;        /* List of chars followers       */
    struct char_data *master;             /* Who is char following?        */
    MPROG_ACT_LIST *mpact;
    int mpactnum;
    sh_int		mpscriptpos;
    int dam_exp;        // combat exp for damage
    int mana_leech;    //  mana leeched
    int sleep_timer;    // for sleep phases
    int last_skill;    //last skill improved

    struct event *char_events[MAX_CHAR_EVENTS];
    long pref;
    char *hostname;			/* hostname copy */
    char *ambush_name;                // baja who we ambush
    struct char_data *next_listener; // next in line...
    int listening_to;
    struct char_data *guarding;      // if guarding someone
    struct char_data *current_target;	// current target for skills (backstab etc.)
    struct char_data *watch;
    char *last_tells[MAX_TELLS];

    char *wait_buffer;
    int protection;


    int pk_timer;                   
    int last_zone;

};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct char_file_u {
    /* char_player_data */
    char	name[20+1];
    char	description[EXDSCR_LENGTH];
    char	title[MAX_TITLE_LENGTH+1];
    byte sex;
    int class;
    byte level;
    byte race;
    int hometown;
    time_t birth;   /* Time of birth of character     */
    int	played;    /* Number of secs played in total */
    ubyte weight;
    ubyte height;

    char	pwd[MAX_PWD_LENGTH+1];    /* character's password */

    struct char_special_data_saved char_specials_saved;
    struct player_special_data_saved player_specials_saved;
    struct char_ability_data abilities;
    struct char_point_data points;
    struct affected_type affected[MAX_AFFECT];

    time_t last_logon;		/* Time (in secs) of last logon */
    char host[HOST_LENGTH+1];	/* host of last logon */
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
    char	*text;
    int aliased;
    struct txt_block *next;
};


struct txt_q {
    struct txt_block *head;
    struct txt_block *tail;
};


struct descriptor_data {
    int	descriptor;		/* file descriptor for socket		*/
    int ident_sock;
    ush_int peer_port;
    char	host[HOST_LENGTH+1];	/* hostname				*/
    int close_me;               /* flag: this desc. should be closed    */
    int	bad_pws;		/* number of bad pw attemps this login	*/
    int	connected;		/* mode of 'connectedness'		*/
    int	wait;			/* wait for how many loops		*/
    int	desc_num;		/* unique num assigned to desc		*/
    time_t login_time;		/* when the person connected		*/
    char	*showstr_head;		/* for paging through texts		*/
    char	*showstr_point;		/*		-			*/
    int  showstr_count;		/* number of pages to page through	*/
    int  showstr_page;		/* which page are we currently showing?	*/
    char	**str;			/* for the modify-str system		*/
    char *backstr;
    char **history;		/* History of commands, for ! mostly.	*/
    int	history_pos;		/* Circular array position.		*/
    char p1[600],p2[600],p3[600]; /*display 1 , saved prompts */
    int	max_str;		/*		-			*/
    long	mail_to;		/* name for mail system			*/
    int	has_prompt;		/* control of prompt-printing		*/
    char	inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input		*/
    char	last_input[MAX_INPUT_LENGTH]; /* the last input			*/
    char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
    char *output;		/* ptr to the current output buffer	*/
    int idle_tics;
    int  bufptr;			/* ptr to end of current output		*/
    int	bufspace;		/* space left in the output buffer	*/
    struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
    struct txt_q input;		/* q of unprocessed input		*/
    struct char_data *character;	/* linked to char			*/
    struct char_data *original;	/* original char if switched		*/
    struct descriptor_data *snooping; /* Who is this char snooping	*/
    struct descriptor_data *snoop_by; /* And who is snooping this char	*/
    struct descriptor_data *next; /* link to next descriptor		*/
    int edit_mode;                /* editing sub mode */
    int edit_number;              /* virtual num of thing being edited */
    int edit_number2;             /* misc number for editing */
    int edit_zone;                /* which zone object is part of      */
    void **misc_data;              /* misc data, usually for extra data crap */
    int stat_points;
    struct olc_data *olc;
};


/* other miscellaneous structures ***************************************/


struct msg_type {
    char	*attacker_msg;  /* message to attacker */
    char	*victim_msg;    /* message to victim   */
    char	*room_msg;      /* message to room     */
};


struct message_type {
    struct msg_type die_msg;	/* messages when death			*/
    struct msg_type miss_msg;	/* messages when miss			*/
    struct msg_type hit_msg;	/* messages when hit			*/
    struct msg_type god_msg;	/* messages when hit on god		*/
    struct message_type *next;	/* to next messages of this kind.	*/
};


struct message_list {
    int	a_type;			/* Attack type				*/
    int	number_of_attacks;	/* How many attack messages to chose from. */
    struct message_type *msg;	/* List of messages.			*/
};


struct dex_skill_type {
    sh_int p_pocket;
    sh_int p_locks;
    sh_int traps;
    sh_int sneak;
    sh_int hide;
};


struct dex_app_type {
    float reaction;
    sh_int miss_att;
    sh_int defensive;
};


struct str_app_type {
    float tohit;    /* To Hit (THAC0) Bonus/Penalty        */
    float todam;    /* Damage Bonus/Penalty                */
    long int carry_w;  /* Maximum weight that can be carrried */
    sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
    int bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
    int learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
    sh_int hitp;
    sh_int shock;
};

struct race_app_type {
    int points;
    int str;
    int intel;
    int wis;
    int dex;
    int con;
    int cha;
};


struct sclass {
    int maxcombat;
};

struct weather_data {
    int	pressure;	/* How is the pressure ( Mb ) */
    int	change;	/* How fast and what way does it change. */
    int	sky;	/* How is the sky. */
    int	sunlight;	/* And how much sun. */
};


struct title_type {
    char	*title;
    int	exp;
};


/* element in monster and object index-tables   */
struct index_data {
    int	virtual;    /* virtual number of this mob/obj           */
    int	*number;   /* number of existing units of this mob/obj	*/
    int  progtypes;  /* program types for MOBProg		*/
    MPROG_DATA *mudprogs; /* programs for MOBProg		*/
    SPECIAL(*func);
};


struct current_info {
    int room_vnum;
    int direction;
    int percent;
};

struct time_write {
    int year, month, day;
};


struct raff_node {
    room_num room;        /* location in the world[] array of the room */
    int      timer;       /* how many ticks this affection lasts */
    long     affection;   /* which affection does this room have */
    long     affection2;   /* which affection does this room have , part 2*/
    int      spell;       /* the spell number */
    int special_data;     /* space for special data */
    int deity;
    struct char_data *ch; /* the baja who gave it */
    char name[30];

    struct raff_node *next; /* link to the next node */
};




struct queue {
    struct q_element *head[NUM_EVENT_QUEUES], *tail[NUM_EVENT_QUEUES];
};

struct q_element {
    void *data;
    long key;
    struct q_element *prev, *next;
};



#if	!defined(BERR)
#define BERR	 255
#endif


struct	extracted_char_data
{
    EXTRACT_CHAR_DATA *	next;
    CHAR_DATA *		ch;
};

struct	extracted_obj_data
{
    EXTRACT_OBJ_DATA *	next;
    OBJ_DATA *		obj;
};


/* 32bit bitvector defines */
#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

#define AT_BLACK	    0
#define AT_BLOOD	    1
#define AT_DGREEN           2
#define AT_ORANGE	    3
#define AT_DBLUE	    4
#define AT_PURPLE	    5
#define AT_CYAN	  	    6
#define AT_GREY		    7
#define AT_DGREY	    8
#define AT_RED		    9
#define AT_GREEN	   10
#define AT_YELLOW	   11
#define AT_BLUE		   12
#define AT_PINK		   13
#define AT_LBLUE	   14
#define AT_WHITE	   15
#define AT_BLINK	   16
#define AT_PLAIN	   AT_GREY
#define AT_ACTION	   AT_GREY
#define AT_SAY		   AT_LBLUE
#define AT_GOSSIP	   AT_LBLUE
#define AT_YELL	           AT_WHITE
#define AT_TELL		   AT_WHITE
#define AT_WHISPER	   AT_WHITE
#define AT_HIT		   AT_WHITE
#define AT_HITME	   AT_YELLOW
#define AT_IMMORT	   AT_YELLOW
#define AT_HURT		   AT_RED
#define AT_FALLING	   AT_WHITE + AT_BLINK
#define AT_DANGER	   AT_RED + AT_BLINK
#define AT_MAGIC	   AT_BLUE
#define AT_CONSIDER	   AT_GREY
#define AT_REPORT	   AT_GREY
#define AT_POISON	   AT_GREEN
#define AT_SOCIAL	   AT_CYAN
#define AT_DYING	   AT_YELLOW
#define AT_DEAD		   AT_RED
#define AT_SKILL	   AT_GREEN
#define AT_CARNAGE	   AT_BLOOD
#define AT_DAMAGE	   AT_WHITE
#define AT_FLEE		   AT_YELLOW
#define AT_RMNAME	   AT_WHITE
#define AT_RMDESC	   AT_YELLOW
#define AT_OBJECT	   AT_GREEN
#define AT_PERSON	   AT_PINK
#define AT_LIST		   AT_BLUE
#define AT_BYE		   AT_GREEN
#define AT_GOLD		   AT_YELLOW
#define AT_GTELL	   AT_BLUE
#define AT_NOTE		   AT_GREEN
#define AT_HUNGRY	   AT_ORANGE
#define AT_THIRSTY	   AT_BLUE
#define	AT_FIRE		   AT_RED
#define AT_SOBER	   AT_WHITE
#define AT_WEAROFF	   AT_YELLOW
#define AT_EXITS	   AT_WHITE
#define AT_SCORE	   AT_LBLUE
#define AT_RESET	   AT_DGREEN
#define AT_LOG		   AT_PURPLE
#define AT_DIEMSG	   AT_WHITE
#define AT_WARTALK         AT_RED
#define AT_RACETALK	   AT_DGREEN
#define AT_IGNORE	   AT_GREEN
#define AT_DIVIDER	   AT_PLAIN
#define AT_MORPH           AT_GREY


/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02

#define DEITY_NONE 	0
#define DEITY_AMURON	1
#define DEITY_VALERIA   2
#define DEITY_AZ   	3
#define DEITY_SKIG 	4
#define DEITY_URG  	5
#define DEITY_BOUGAR    6
#define DEITY_MUGRAK    7
#define DEITY_VETIIN    8

#define MAX_DEITY	8
struct Sdeity {
	char *name;
	int id;
};
	