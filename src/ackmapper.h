/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Zepp 1998    *
 *    _/_/_/_/      _/          _/  _/             Version #: 4.3          *
 *   _/      _/      _/_/_/     _/    _/     _/                            *
 *                                                                         *
 *                        http://ackmud.nuc.net/                           *
 *                        zenithar@ackmud.nuc.net                          *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
/*

*
 * Ported to DOTDII MUD (http://www.dotd.com) by Garil - 6-15-99
 */

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
#include "house.h"
#include "arena.h"
#include "constants.h"
#include "vt100c.h"
#include "auction.h"
#include "clan.h"
#include "events.h"
#include "ident.h"

#define MAX_MAP 100
#define MAX_MAP_DIR 10

#define DOOR_LOCKED -1
#define DOOR_CLOSED -2
#define DOOR_OPEN -3
#define DOOR_NS -4
#define DOOR_EW -5
#define DOOR_NWSE -6
#define DOOR_NESW -7
#define DOOR_NULL -8

#define LOS_INITIAL -5
#define MAP_Y           9
struct room_content_type
{
  char string[10];
};

struct room_content_type contents[MAX_MAP][MAX_MAP];

int map[MAX_MAP][MAX_MAP];




struct map_info_type
{
  int sector_type;
  char * display_color;
  char * display_code;
  char * invert_color;
  char * desc;
};


void MapArea2
(struct room_data *room, CHAR_DATA *ch, int x, int y, int min, int max, int line_of_sight, int test_los);

void ShowMap2( CHAR_DATA *ch, int min, int max, int size, int center );
void ShowHalfMap( CHAR_DATA *ch, int min, int max);
void do_printmap(CHAR_DATA *ch, char *argument);
void ShowRoom( CHAR_DATA *ch, int min, int max, int size, int center);
void do_map( CHAR_DATA *ch, char *argument );
void do_smallmap( CHAR_DATA *ch, char *argument );


