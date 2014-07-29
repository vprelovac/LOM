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

/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define DEATHR	99
#define VOIDR    3

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NORTHEAST      6
#define NORTHWEST      7
#define SOUTHEAST      8
#define SOUTHWEST      9
#define SOMEWHERE      10


#define DIR_NORTH          0
#define DIR_EAST           1
#define DIR_SOUTH          2
#define DIR_WEST           3
#define DIR_UP             4
#define DIR_DOWN           5
#define DIR_NORTHEAST      6
#define DIR_NORTHWEST      7
#define DIR_SOUTHEAST      8
#define DIR_SOUTHWEST      9
#define DIR_SOMEWHERE      10


/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK		(1 << 0)   /* Dark			*/
#define ROOM_DEATH		(1 << 1)   /* Death trap		*/
#define ROOM_NOMOB		(1 << 2)   /* MOBs not allowed		*/
#define ROOM_INDOORS		(1 << 3)   /* Indoors			*/
#define ROOM_PEACEFUL		(1 << 4)   /* Violence not allowed	*/
#define ROOM_SOUNDPROOF		(1 << 5)   /* Shouts, gossip blocked	*/
#define ROOM_NOTRACK		(1 << 6)   /* Track won't go through	*/
#define ROOM_NOMAGIC		(1 << 7)   /* Magic not allowed		*/
#define ROOM_TUNNEL		(1 << 8)   /* room for only 1 pers	*/
#define ROOM_PRIVATE		(1 << 9)   /* Can't teleport in		*/
#define ROOM_GODROOM		(1 << 10)  /* LVL_GOD+ only allowed	*/
#define ROOM_HOUSE		(1 << 11)  /* (R) Room is a house	*/
#define ROOM_HOUSE_CRASH	(1 << 12)  /* (R) House needs saving	*/
#define ROOM_ATRIUM		(1 << 13)  /* (R) The door to a house	*/
#define ROOM_OLC		(1 << 14)  /* (R) Modifyable/!compress	*/
#define ROOM_BFS_MARK		(1 << 15)  /* (R) breath-first srch mrk	*/
#define ROOM_MURKY              (1 << 16)  /* Room can't be looked into */
#define ROOM_COURT		(1 << 17)  /* Room is a court room      */
#define ROOM_BROADCAST          (1 << 18)  /* Room broadcasts actions   */
#define ROOM_RECEIVER           (1 << 19)  /* Room receives             */
#define ROOM_ARENA 		(1 << 20)  /* Room is arena 		*/
#define ROOM_RED_BASE 		(1 << 21)  /* Room is arena 		*/
#define ROOM_BLUE_BASE 		(1 << 22)  /* Room is arena 		*/
#define ROOM_NORECALL  		(1 << 23)  /* Room is norecall */
#define ROOM_NOSUMMON  		(1 << 24)  /* Room is nosummon*/

/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR		(1 << 0)   /* Exit is a door		*/
#define EX_CLOSED		(1 << 1)   /* The door is closed	*/
#define EX_LOCKED		(1 << 2)   /* The door is locked	*/
#define EX_PICKPROOF		(1 << 3)   /* Lock can't be picked	*/
#define EX_HIDDEN		(1 << 4)   /* door is hidden		*/
#define EX_PASSAGE		(1 << 5)   /* door is passage		*/
#define EX_NOMOB  		(1 << 6)   /* door is passage		*/
#define EX_NOPASSDOOR		(1 << 7)   /* door is passage		*/
#define EX_BASHPROOF		(1 << 8)   /* door is passage		*/

/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0		   /* Indoors			*/
#define SECT_CITY            1		   /* In a city			*/
#define SECT_FIELD           2		   /* In a field		*/
#define SECT_FOREST          3		   /* In a forest		*/
#define SECT_HILLS           4		   /* In the hills		*/
#define SECT_MOUNTAIN        5		   /* On a mountain		*/
#define SECT_WATER_SWIM      6		   /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7		   /* Water - need a boat	*/
#define SECT_UNDERWATER	     8		   /* Underwater		*/
#define SECT_FLYING	     9		   /* Wheee!			*/
#define SECT_QUICKSAND	    10		   /* Loses HP and costs mana   */
#define SECT_LAVA	    11	           /* Loses HP and causes burn  */
#define SECT_ARCTIC         12             /* Loses HP and causes freeze*/

#define PULSE_TELEPORT (20 RL_SEC)
#define TELE_LOOK	(1 << 0)
#define TELE_COUNT	(1 << 1)
#define TELE_RANDOM	(1 << 2)
#define TELE_SPIN	(1 << 3)
#define TELE_OBJ	(1 << 4)
#define TELE_NOOBJ	(1 << 5)
#define TELE_NOMSG	(1 << 6)

/* Sun state for weather_data */
#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


#define RAFF_FOG        (1 << 0)
#define RAFF_TRAP       (1 << 1)
#define RAFF_ILLUMINATION (1 << 2)
#define RAFF_PEACEFUL (1 << 3)


#define DUR_TRIGGER_TRAP 	-1

