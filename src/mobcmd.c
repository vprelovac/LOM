/***************************************************************************
 * MOBProgram ported for CircleMUD 3.0 by Mattias Larsson		   *
 * Traveller@AnotherWorld (ml@eniac.campus.luth.se 4000) 		   *
 **************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "structs.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "events.h"
#include "clan.h"

extern int clan_loadroom[];
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern char *spells[];
extern int      top_of_world;
extern int      pulse;
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;


extern struct index_data *get_mob_index(int vnum);
extern struct index_data *get_obj_index(int vnum);

extern sh_int   find_target_room(struct char_data * ch, char *rawroomstr);

#define bug(x, y) { sprintf(buf2, (x), (y)); progbug(buf2, ch); }

char log_buf[1000];
int global_color;
/*
 * Local functions.
 */

char           *mprog_type_to_name(int type);

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */
/*
char           *mprog_type_to_name(int type)
{
switch (type) {
case IN_FILE_PROG:
  return "in_file_prog";
case ACT_PROG:
  return "act_prog";
case SPEECH_PROG:
  return "speech_prog";
case RAND_PROG:
  return "rand_prog";
case FIGHT_PROG:
  return "fight_prog";
case HITPRCNT_PROG:
  return "hitprcnt_prog";
case DEATH_PROG:
  return "death_prog";
case ENTRY_PROG:
  return "entry_prog";
case GREET_PROG:
  return "greet_prog";
case ALL_GREET_PROG:
  return "all_greet_prog";
case GIVE_PROG:
  return "give_prog";
case BRIBE_PROG:
  return "bribe_prog";
default:
  return "ERROR_PROG";
}
}
  */
int get_color(char *argument)    /* get color code from command string */
{
    char color[MAX_INPUT_LENGTH];
    char *cptr;
    static char const * color_list=
        "_bla_red_dgr_bro_dbl_pur_cya_cha_dch_ora_gre_yel_blu_pin_lbl_whi";
    static char const * blink_list=
        "*bla*red*dgr*bro*dbl*pur*cya*cha*dch*ora*gre*yel*blu*pin*lbl*whi";

    one_argument (argument, color);
    if (color[0]!='_' && color[0]!='*') return 0;
    if ( (cptr = strstr(color_list, color)) )
        return (cptr - color_list) / 4;
    if ( (cptr = strstr(blink_list, color)) )
        return (cptr - blink_list) / 4 + AT_BLINK;
    return 0;
}
char colour_buf[100];
char * set_char_color( sh_int AType)
{

    CHAR_DATA *och;

    {

        if ( AType == 7 )
            strcpy( colour_buf, "\033[0;37m" );
        else
            sprintf(colour_buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
                    (AType > 15 ? "5;" : ""), (AType & 7)+30);
        // sprintf(colour_buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
        // sprintf(colour_buf, "\x1B[%d;%s%dm", (AType & 8) == 8,

        //sprintf(colour_buf, "\027[0;37m", (AType & 8) == 8,
        // sprintf(colour_buf, "&w", (AType & 8) == 8,
        // (AType > 150 ? "5;" : ""), (AType & 7)+30);
        return colour_buf;
    }
    return NULL;
}
char *mprog_type_to_name( int type )
{
    switch ( type )
    {
    case IN_FILE_PROG:      return "in_file_prog";
    case ACT_PROG:          return "act_prog";
    case SPEECH_PROG:       return "speech_prog";
    case RAND_PROG:         return "rand_prog";
    case FIGHT_PROG:        return "fight_prog";
    case HITPRCNT_PROG:     return "hitprcnt_prog";
    case DEATH_PROG:        return "death_prog";
    case ENTRY_PROG:        return "entry_prog";
    case GREET_PROG:        return "greet_prog";
    case ALL_GREET_PROG:    return "all_greet_prog";
    case GIVE_PROG:         return "give_prog";
    case BRIBE_PROG:        return "bribe_prog";
    case HOUR_PROG:		return "hour_prog";
    case TIME_PROG:		return "time_prog";
    case WEAR_PROG:         return "wear_prog";
    case REMOVE_PROG:       return "remove_prog";
    case SAC_PROG :         return "sac_prog";
    case LOOK_PROG:         return "look_prog";
    case EXA_PROG:          return "exa_prog";
    case ZAP_PROG:          return "zap_prog";
    case GET_PROG:          return "get_prog";
    case DROP_PROG:         return "drop_prog";
    case REPAIR_PROG:       return "repair_prog";
    case DAMAGE_PROG:       return "damage_prog";
    case PULL_PROG:         return "pull_prog";
    case PUSH_PROG:         return "push_prog";
    case SCRIPT_PROG:	return "script_prog";
    case SLEEP_PROG:        return "sleep_prog";
    case REST_PROG:         return "rest_prog";
    case LEAVE_PROG:        return "leave_prog";
    case USE_PROG:          return "use_prog";
    case ACTION_PROG:       return "action_prog";
    case RANDIW_PROG:       return "grand_prog";
    default:                return "ERROR_PROG";
    }
}
// SMAUG mobprogs
/* string prefix routine */

bool            str_prefix(const char *astr, const char *bstr)
{
    if (!astr) {
        log("Strn_cmp: null astr.");
        return TRUE;
    }
    if (!bstr) {
        log("Strn_cmp: null astr.");
        return TRUE;
    }
    for (; *astr; astr++, bstr++) {
        if (LOWER(*astr) != LOWER(*bstr))
            return TRUE;
    }
    return FALSE;
}

/* prints the argument to all the rooms aroud the mobile */



/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MUDprograms which are set.
 */

ACMD(do_mpstat)
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    CHAR_DATA  *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "MProg stat whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_vis (ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) )
    {
        send_to_char( "Only Mobiles can have MobPrograms!\n\r", ch);
        return;
    }

    if ( !mob_index[victim->nr].progtypes )
    {
        send_to_char( "That Mobile has no Programs set.\n\r", ch);
        return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               victim->player.name, GET_MOB_VNUM(victim));

    ch_printf( ch, "Short description: %s.\n\rLong  description: %s",
               victim->player.short_descr,
               victim->player.long_descr[0] != '\0' ?
               victim->player.long_descr : "(none).\n\r" );

    ch_printf( ch, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d\n\r",
               victim->points.hit,         victim->points.max_hit,
               victim->points.mana,        victim->points.max_mana,
               victim->points.move,        victim->points.max_move
             );

    ch_printf( ch,
               "Lev: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\n\r",
               GET_LEVEL(victim),       GET_CLASS_NUM(victim),        GET_ALIGNMENT(victim),
               GET_AC( victim ),    GET_GOLD(victim),         GET_EXP(victim) );

    for ( mprg = mob_index[victim->nr].mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->type ),
                   mprg->arglist,
                   mprg->comlist );
    return;
}


ACMD(do_opstat)
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    OBJ_DATA   *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "OProg stat what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_vis( ch, arg ) ) == NULL )
    {
        send_to_char( "You cannot find that.\n\r", ch );
        return;
    }

    if ( !obj_index[obj->item_number].progtypes )
    {
        send_to_char( "That object has no programs set.\n\r", ch);
        return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               obj->name, GET_OBJ_VNUM(obj) );

    ch_printf( ch, "Short description: %s.\n\r",
               obj->short_description );

    for ( mprg = obj_index[obj->item_number].mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->type ),
                   mprg->arglist,
                   mprg->comlist );

    return;

}


ACMD(do_rpstat)
{
    MPROG_DATA *mprg;

    if ( !world[ch->in_room].progtypes )
    {
        send_to_char( "This room has no programs set.\n\r", ch);
        return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               world[ch->in_room].name, world[ch->in_room].number );

    for ( mprg = world[ch->in_room].mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->type ),
                   mprg->arglist,
                   mprg->comlist );
    return;
}



ACMD (do_mpasupress)
{
    logs("Call of mpasupress command by %s", GET_NAME(ch));
    return;
}





ACMD(do_mpasound)
{

    room_num        was_in_room;
    int             door;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    if (argument[0] == '\0') {
        bug("Mpasound - No argument: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if ( (global_color = get_color(argument)) )
    {
        argument = one_argument ( argument, buf1 );

        sprintf(buf, "%s%s&0", set_char_color(global_color), argument);
        strcpy(argument, buf);

    }
    //one_argument(argument, arg);
    skip_spaces(&argument);

    was_in_room = ch->in_room;
    for (door = 0; door  <  NUM_OF_DIRS; door++) {
        struct room_direction_data *pexit;

        if ((pexit = world[was_in_room].dir_option[door]) != NULL
                && pexit->to_room != NOWHERE
                && pexit->to_room != was_in_room) {
            ch->in_room = pexit->to_room;
            MOBTrigger = FALSE;
            if (IS_SUPERMOB(ch)) super_silent=0;
            act(argument, FALSE, ch, NULL, NULL, TO_ROOM);
            super_silent=1;
        }
    }

    ch->in_room = was_in_room;
    return;

}

/* lets the mobile kill any player or mobile without murder*/

ACMD(do_mpkill)
{
    char            arg[MAX_INPUT_LENGTH];
    struct char_data *victim;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (arg[0] == '\0') {
        bug("MpKill - no argument: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if ((victim = get_char_room_vis(ch, arg)) == NULL) {
        bug("MpKill - Victim not in room: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if (victim == ch) {
        bug("MpKill - Bad victim to attack: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
        bug("MpKill - Charmed mob attacking master: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if (ch->char_specials.position == POS_FIGHTING) {
        bug("MpKill - Already fighting: vnum %d",
            mob_index[ch->nr].virtual);
        return;
    }
    hit(ch, victim, -1);
    return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */

ACMD(do_mpjunk)
{
    char            arg[MAX_INPUT_LENGTH];
    int             pos;
    struct obj_data *obj;
    struct obj_data *obj_next;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (arg[0] == '\0') {
        bug("Mpjunk - No argument: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if (str_cmp(arg, "all") && str_prefix("all.", arg)) {
        if ((obj = get_object_in_equip_vis(ch, arg, ch->equipment, &pos)) != NULL) {
            unequip_char(ch, pos);
            extract_obj(obj);
            return;
        }
        if ((obj = get_obj_in_list_vis(ch, arg, ch->carrying)) != NULL)
            extract_obj(obj);
        return;
    } else {
        for (obj = ch->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            if (arg[3] == '\0' || isname(arg + 4, obj->name)) {
                extract_obj(obj);
            }
        }
        while ((obj = get_object_in_equip_vis(ch, arg, ch->equipment, &pos)) != NULL) {
            unequip_char(ch, pos);
            extract_obj(obj);
        }
    }
    return;
}

/* prints the message to everyone in the room other than the mob and victim */

ACMD(do_mpechoaround)
{
    char            arg[MAX_INPUT_LENGTH];
    struct char_data *victim;
    char           *p;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }

    argument = one_argument( argument, arg );
    if (arg[0] == '\0') {
        bug("Mpechoaround - No argument:  vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if (!(victim = get_char_room_vis(ch, arg))) {
        bug("Mpechoaround - victim does not exist: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if ( (global_color = get_color(argument)) )
    {
        argument = one_argument ( argument, buf1 );

        sprintf(buf, "%s%s&0", set_char_color(global_color), argument);
        strcpy(argument, buf);

    }
    if (IS_SUPERMOB(ch)) super_silent=0;
    act(argument, FALSE, ch, NULL, victim, TO_NOTVICT);
    super_silent=1;
    return;
}


ACMD(do_mpsoundaround)
{
    char            arg[MAX_INPUT_LENGTH];
    struct char_data *victim;
    char           *p;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    p = one_argument(argument, arg);
    while (isspace(*p))
        p++;                    /* skip over leading space */

    if (arg[0] == '\0') {
        bug("Mpsoundaround - No argument:  vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if (!(victim = get_char_room_vis(ch, arg))) {
        bug("Mpsoundaround - victim does not exist: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    sprintf(buf, "!!SOUND(%s)", p);
    if (IS_SUPERMOB(ch)) super_silent=0;
    act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
    super_silent=1;
    logs("Call of mpasoundaround command by %s", GET_NAME(ch));
    return;
}

ACMD(do_mpsoundat)
{
    logs("SYSERR:Call of mpasoundat command by %s", GET_NAME(ch));
    return;
}

ACMD(do_mpsound)
{
	sprintf(buf, "SYSERR: Call of unsupported mudprog command '%s' by %s", cmd_info[cmd].command, GET_NAME(ch));
    progbug(buf, ch);
    return;

}
/* prints the message to only the victim */

ACMD(do_mpechoat)
{
    char            arg[MAX_INPUT_LENGTH];
    struct char_data *victim;
    char           *p;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }

    argument = one_argument(argument, arg);


    if (arg[0] == '\0') {
        bug("Mpechoat - No argument:  vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if (!(victim = get_char_room_vis(ch, arg))) {
        bug("Mpechoat - victim does not exist: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }

    if ( (global_color = get_color(argument)) )
    {
        argument = one_argument ( argument, buf1 );

        sprintf(buf, "%s%s&0", set_char_color(global_color), argument);
        strcpy(argument, buf);

    }
    if (IS_SUPERMOB(ch)) super_silent=0;
    act(argument, FALSE, ch, NULL, victim, TO_VICT);
    super_silent=1;
    return;
}

/* prints the message to the room at large */

ACMD(do_mpecho)
{
    char           *p;
    char       arg1 [MAX_INPUT_LENGTH];


    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    if (argument[0] == '\0') {
        bug("Mpecho - called w/o argument: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if ( (global_color = get_color(argument)) )
    {
        argument = one_argument ( argument, buf1 );

        sprintf(buf, "%s%s\033[0;37m", set_char_color(global_color), argument);
        strcpy(argument, buf);

    }
    p = argument;
    while (isspace(*p))
        p++;

    if (IS_SUPERMOB(ch)) super_silent=0;
    act(p, FALSE, ch, NULL, NULL, TO_ROOM);
    super_silent=1;
    return;

}

/* lets the mobile load an item or mobile.  All items
are loaded into  tttttttttttttttttttttttttttttttttiuinventory.  you can specify a level with
the load object portion as well. */

ACMD(do_mpmload)
{
    char            arg[MAX_INPUT_LENGTH];
    struct index_data *pMobIndex;
    struct char_data *victim;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (arg[0] == '\0' || !is_number(arg)) {
        bug("Mpmload - Bad vnum as arg: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if ((pMobIndex = get_mob_index(atoi(arg))) == NULL) {
        bug("Mpmload - Bad mob vnum: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    victim = read_mobile(atoi(arg), VIRTUAL, world[ch->in_room].zone);
    char_to_room(victim, ch->in_room);
    return;
}

ACMD(do_mpoload)
{
    char            arg1[MAX_INPUT_LENGTH];
    char arg2[ MAX_INPUT_LENGTH ];
    struct index_data *pObjIndex;
    struct obj_data *obj;
    int level=0;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg1);
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0' || !is_number(arg1)) {
        bug("Mpoload - Bad syntax: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }

    if ( arg2[0] == '\0' )
        level = IS_SUPERMOB(ch) ? ch->protection : GET_LEVEL( ch );
    else
    {
        /*
         * New feature from Alander.
         */
        if ( !is_number( arg2 ) )
        {
            progbug( "Mpoload - Bad level syntax", ch );
            return;
        }
        level = atoi( arg2 );
        if ( level < 0 || level > GET_LEVEL( ch ) +3)
        {
            progbug( "Mpoload - Bad level ", ch );
            return;
        }

    }
    if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL) {
        bug("Mpoload - Bad vnum arg: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    obj = read_object(atoi(arg1), VIRTUAL, world[ch->in_room].zone, level);
    if (obj == NULL)
        return;
    if (CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
        obj_to_char(obj, ch);
    } else {
        obj_to_room(obj, ch->in_room);
    }

    return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MOBprogram
   otherwise ugly stuff will happen */

ACMD(do_mppurge)
{
    struct char_data *victim;
    struct obj_data *obj;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    one_argument(argument, arg);

    if (arg[0] == '\0') {
        /* 'purge' */
        struct char_data *vnext;
        struct obj_data *obj_next;

        for (victim = world[ch->in_room].people; victim != NULL; victim = vnext) {
            vnext = victim->next_in_room;
            if (IS_NPC(victim) && victim != ch)
                extract_char(victim);
        }

        for (obj = world[ch->in_room].contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            extract_obj(obj);
        }

        return;
    }
    if (!(victim = get_char_room_vis(ch, arg))) {
        if ((obj = get_obj_vis(ch, arg))) {
            extract_obj(obj);
        } else {
            bug("Mppurge - Bad argument: vnum %d.", mob_index[ch->nr].virtual);
        }
        return;
    }
    if (!IS_NPC(victim)) {
        bug("Mppurge - Purging a PC: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }

    if ( IS_NPC( victim ) && GET_MOB_VNUM(victim) == SUPERMOB )
    {
        progbug( "Mppurge: trying to purge supermob", ch );
        return;
    }
    if ( victim == ch )
    {
        progbug( "Mppurge - Trying to purge oneself (WARNING)", ch );
        //	return;
    }

    extract_char(victim);
    return;
}


/* lets the mobile goto any location it wishes that is not private */

ACMD(do_mpgoto)
{
    char            arg[MAX_INPUT_LENGTH];
    sh_int          location;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    one_argument(argument, arg);
    if (arg[0] == '\0') {
        bug("Mpgoto - No argument: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if ((location = find_target_room(ch, arg)) < 0) {
        bug("Mpgoto - No such location: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if (FIGHTING(ch) != NULL)
        stop_fighting(ch);

    char_from_room(ch);
    char_to_room(ch, location);

    return;
}

/* lets the mobile do a command at another location. Very useful */

ACMD(do_mpat)
{
    char            arg[MAX_INPUT_LENGTH];
    sh_int          location;
    sh_int          original;
    /*    struct char_data       *wch; */

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        bug("Mpat - Bad argument: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if ((location = find_target_room(ch, arg)) < 0) {
        bug("Mpat - No such location: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    original = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, argument);

    /* See if 'ch' still exists before continuing! Handles 'at XXXX quit'
     * case. */
    if (ch->in_room == location) {
        char_from_room(ch);
        char_to_room(ch, original);
    }
    return;
}

/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */

ACMD(do_mptransfer)
{
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];
    char            arg3[MAX_INPUT_LENGTH];
    sh_int          location;
    struct descriptor_data *d;
    struct char_data *victim, *nextinroom;
    ACMD(do_trans);

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0') {
        bug("Mptransfer - Bad syntax: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    /* if (!str_cmp(arg1, "all")) {
         for (d = descriptor_list; d != NULL; d = d->next) {
             if (d->connected == CON_PLAYING
                 && d->character != ch
                 && d->character->in_room != NOWHERE
                 && CAN_SEE(ch, d->character)) {
                 char            buf[MAX_STRING_LENGTH];
                 sprintf(buf, "%s %s", d->character->player.name, arg2);
                 do_trans(ch, buf, cmd, 0);
             }
         }
         return;
     }
     */
    /* Put in the variable nextinroom to make this work right. -Narn */
    if ( !str_cmp( arg1, "all" ) )
    {
        for ( victim = world[ch->in_room].people; victim; victim = nextinroom )
        {
            nextinroom = victim->next_in_room;
            if ( victim != ch
                    &&   CAN_SEE( ch, victim ) )
            {
                sprintf( buf, "%s %s", victim->player.name, arg2 );
                do_mptransfer( ch, buf, cmd, 0);
            }
        }
        return;
    }
    /* This will only transfer PC's in the area not Mobs --Shaddai */
    if (!str_cmp(arg1, "area")) {
        for (d = descriptor_list; d != NULL; d = d->next) {
            if (d->connected == CON_PLAYING
                    && d->character != ch
                    && d->character->in_room != NOWHERE
                    && CAN_SEE(ch, d->character)
                    && world[ch->in_room].zone==world[d->character->in_room].zone) {
                char            buf[MAX_STRING_LENGTH];
                sprintf(buf, "%s %s", d->character->player.name, arg2);
                do_trans(ch, buf, cmd, 0);
            }
        }
        return;
    }



    /* Thanks to Grodyn for the optional location parameter. */
    if (arg2[0] == '\0') {
        location = ch->in_room;
    } else {
        if ((location = find_target_room(ch, arg2)) < 0) {
            bug("Mptransfer - No such location: vnum %d.",
                mob_index[ch->nr].virtual);
            return;
        }
        /*if (IS_SET(world[location].room_flags, ROOM_PRIVATE)) {
            bug("Mptransfer - Private room: vnum %d.",
                mob_index[ch->nr].virtual);
            return;
        } */
    }

    if ((victim = get_char_vis(ch, arg1)) == NULL) {
        bug("Mptransfer - No such person: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if (victim->in_room == 0) {
        bug("Mptransfer - Victim in Limbo: vnum %d.",
            mob_index[ch->nr].virtual);
        return;
    }
    if (FIGHTING(victim) != NULL)
        stop_fighting(victim);

    if (*arg3=='1' && !IS_NPC(victim))
        location=real_room(victim->player.hometown>1?victim->player.hometown:clan_loadroom[GET_CLAN(victim)]);

    char_from_room(victim);
    char_to_room(victim, location);

    return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

ACMD(do_mpforce)
{
    char            arg[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        bug("Mpforce - Bad syntax: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if (!str_cmp(arg, "all")) {
        struct descriptor_data *i;
        struct char_data *vch;

        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected &&
                    i->character->in_room == ch->in_room) {
                vch = i->character;
                if (GET_LEVEL(vch) < GET_LEVEL(ch) && CAN_SEE(ch, vch)) {
                    command_interpreter(vch, argument);
                }
            }
        }
    } else {
        struct char_data *victim;

        if ((victim = get_char_room_vis(ch, arg)) == NULL) {
            bug("Mpforce - No such victim: vnum %d.",
                mob_index[ch->nr].virtual);
            return;
        }
        if (victim == ch) {
            bug("Mpforce - Forcing oneself: vnum %d.",
                mob_index[ch->nr].virtual);
            return;
        }
        command_interpreter(victim, argument);
    }

    return;
}





// new stuff




ACMD(do_mpechozone)
{
    char       arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    struct char_data *victim;
    sh_int     color;
    char *p;


    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc)
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' ) {
        progbug( "Mpechozone - called w/o argument", ch );
        return;
    }

    if ( (global_color = get_color(argument)) )
    {
        argument = one_argument ( argument, buf1 );

        sprintf(buf, "%s%s&0", set_char_color(global_color), argument);
        strcpy(argument, buf);

    }
    p=one_argument(argument, arg1);


    if ((victim = get_char_room_vis(ch, arg1))==NULL || strlen(arg1)!=strlen(GET_NAME(victim)))
    {
        p=argument;
        victim=NULL;
    }



    while (isspace(*p))
        p++;
    for ( vch = character_list; vch; vch = vch_next )
    {
        vch_next = vch->next;
        if ( world[vch->in_room].zone == world[ch->in_room].zone
                &&  !IS_NPC(vch)
                &&   AWAKE(vch)
                && vch!=victim )
        {         if (IS_SUPERMOB(ch)) super_silent=0;
            act( p, FALSE, vch, NULL, NULL, TO_CHAR );
            super_silent=1;
        }
    }
}




/*
 *  Haus' toys follow:
 */

/*
 * syntax:  mppractice victim spell_name max%
 *
 */
ACMD(do_mp_practice)
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int sn, max, tmp, adept;
    char *skill_name;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc)
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Mppractice: bad syntax", ch );
        progbug( "Mppractice - Bad syntax", ch );
        return;
    }

    if ( ( victim = get_char_room_vis( ch, arg1 ) ) == NULL )
    {
        send_to_char("Mppractice: Student not in room? Invis?", ch);
        progbug( "Mppractice: Invalid student not in room", ch );
        return;
    }

    if ( ( sn = find_skill_num( arg2 ) ) < 0 )
    {
        send_to_char("Mppractice: Invalid spell/skill name", ch);
        progbug( "Mppractice: Invalid spell/skill name", ch );
        return;
    }


    if(IS_NPC(victim))
    {
        send_to_char("Mppractice: Can't train a mob", ch);
        progbug("Mppractice: Can't train a mob", ch );
        return;
    }

    skill_name = spells[sn];

    max = atoi( arg3 );
    if( (max<0) || (max>100) )
    {
        sprintf( log_buf, "mp_practice: Invalid maxpercent: %d", max );
        send_to_char( log_buf, ch);
        progbug( log_buf, ch );
        return;
    }

    /*if(victim->level < skill_table[sn]->skill_level[victim->class] )
{
    sprintf(buf,"$n attempts to tutor you in %s, but it's beyond your comprehension.",skill_name);
    act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
    return;
} */

    /* adept is how high the player can learn it */
    /* adept = class_table[ch->class]->skill_adept; */
    adept = max;

    if ( GET_SKILL(victim, sn)>max)
    {
        sprintf(buf,"$n shows some knowledge of %s, but yours is clearly superior.",spells[sn]);
        act(buf, FALSE, ch, NULL, victim, TO_VICT );
        return;
    }


    /* past here, victim learns something */

    sprintf(buf,"$N demonstrates %s to you.  You feel more learned in this subject.", spells[sn]);
    act( buf, FALSE, victim, NULL, ch, TO_CHAR );

    SET_SKILL(victim, sn, max);


    return;

}



ACMD(do_mpscatter)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    int low_vnum, high_vnum, rvnum, prob;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' ) {
        send_to_char( "Mpscatter whom?\n\r", ch );
        progbug( "Mpscatter: invalid (nonexistent?) argument", ch );
        return;
    }
    if ( ( victim = get_char_room_vis( ch, arg1 ) ) == NULL ) {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpscatter: victim not in room", ch );
        return;
    }
    if ( IS_IMMORT( victim ) && GET_LEVEL( victim ) >= GET_LEVEL( ch ) ) {
        send_to_char( "You haven't the power to succeed against this victim.\n\r", ch );
        progbug( "Mpscatter: victim level too high", ch );
        return;
    }
    if (arg2[0] == '\0') {
        send_to_char( "You must specify a low vnum.\n\r", ch );
        progbug( "Mpscatter:  missing low vnum", ch );
        return;
    }
    if (argument[0] == '\0') {
        send_to_char( "You must specify a high vnum.\n\r", ch );
        progbug( "Mpscatter:  missing high vnum", ch );
        return;
    }
    low_vnum = atoi( arg2 ); high_vnum = atoi( argument );
    if ( low_vnum < 1 || high_vnum < low_vnum || low_vnum > high_vnum || low_vnum == high_vnum || high_vnum > 250000 ) {
        send_to_char( "Invalid range.\n\r", ch );
        progbug( "Mpscatter:  invalid range", ch );
        return;
    }
    prob=-1;
    while (prob<0) {
        rvnum = number( low_vnum, high_vnum );
        prob=   real_room(rvnum);
        if (prob==-1)
        {
            progbug( "Mpscatter:  invalid range", ch );
            return;
        }
    }
    if ( FIGHTING(victim) ) stop_fighting( victim);
    char_from_room( victim );
    char_to_room( victim, prob);
    GET_POS(victim) = POS_RESTING;
    look_at_room( victim, 0);
    return;
}




/*
 * syntax: mpslay (character)
 */
ACMD(do_mp_slay)
{
    char arg1[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc)
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpslay whom?\n\r", ch );
        progbug( "Mpslay: invalid (nonexistent?) argument", ch );
        return;
    }

    if ( ( victim = get_char_room_vis( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpslay: victim not in room", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You try to slay yourself.  You fail.\n\r", ch );
        progbug( "Mpslay: trying to slay self", ch );
        return;
    }

    if ( IS_NPC( victim ) && GET_MOB_VNUM(victim)==SUPERMOB)
    {
        send_to_char( "You cannot slay supermob!\n\r", ch );
        progbug( "Mpslay: trying to slay supermob", ch );
        return;
    }

    if( GET_LEVEL(victim) < LVL_IMMORT)
    {
        act(  "You slay $M in cold blood!",  FALSE, ch, NULL, victim, TO_CHAR);
        act(  "$n slays you in cold blood!", FALSE, ch, NULL, victim, TO_VICT);
        act(  "$n slays $N in cold blood!",  FALSE, ch, NULL, victim, TO_NOTVICT);
        raw_kill( victim, ch );        
        if (!IS_NPC(victim))
                gain_exp(victim, -LEVELEXP(victim) / 10);
    }
    else
    {
        act(  "You attempt to slay $M and fail!", FALSE, ch, NULL, victim, TO_CHAR);
        act(  "$n attempts to slay you.  What a kneebiter!",FALSE, ch, NULL, victim, TO_VICT);
        act(  "$n attempts to slay $N.  Needless to say $e fails.",FALSE,  ch, NULL, victim, TO_NOTVICT);
    }
    return;
}



/*
 * syntax: mpdamage (character) (#hps)
 */
ACMD(do_mp_damage)
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *nextinroom;
    int dam;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc)
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpdamage whom?\n\r", ch );
        progbug( "Mpdamage: invalid argument1", ch );
        return;
    }
    /* Am I asking for trouble here or what?  But I need it. -- Blodkai */
    if ( !str_cmp( arg1, "all" ) )
    {
        for ( victim = world[ch->in_room].people; victim; victim = nextinroom )
        {
            nextinroom = victim->next_in_room;
            if ( victim != ch
                    &&   CAN_SEE( ch, victim ) ) /* Could go either way */
            {
                sprintf( buf, "'%s' %s", GET_NAME(victim), arg2 );
                do_mp_damage( ch, buf , cmd, subcmd);
            }
        }
        return;
    }
    if ( arg2[0] == '\0' )
    {
        send_to_char( "mpdamage inflict how many hps?\n\r", ch );
        progbug( "Mpdamage: invalid argument2", ch );
        return;
    }
    if ( ( victim = get_char_room_vis( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpdamage: victim not in room", ch );
        return;
    }
    if ( victim == ch )
    {
        send_to_char( "You can't mpdamage yourself.\n\r", ch );
        progbug( "Mpdamage: trying to damage self", ch );
        return;
    }
    dam = atoi(arg2);
    if( (dam<0) || (dam>32000) )
    {
        send_to_char( "Mpdamage how much?\n\r", ch );
        progbug( "Mpdamage: invalid (nonexistent?) argument", ch );
        return;
    }
    /* this is kinda begging for trouble        */
    /*
     * Note from Thoric to whoever put this in...
     * Wouldn't it be better to call damage(ch, ch, dam, dt)?
     * I hate redundant code
     */
    if (!IS_GOD(victim))
        GET_HIT(victim)-=dam;
    check_kill(victim, GET_NAME(ch));
    return;
}





/*
 * syntax: mprestore (character) (#hps)                Gorog
 */
ACMD(do_mp_restore)
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int hp;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "mprestore whom?\n\r", ch );
        progbug( "Mprestore: invalid argument1", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "mprestore how many hps?\n\r", ch );
        progbug( "Mprestore: invalid argument2", ch );
        return;
    }

    if ( ( victim = get_char_room_vis( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mprestore: victim not in room", ch );
        return;
    }

    hp = atoi(arg2);

    if( (hp<0) || (hp>32000) )
    {
        send_to_char( "Mprestore how much?\n\r", ch );
        progbug( "Mprestore: invalid (nonexistent?) argument", ch );
        return;
    }
    GET_HIT(victim)=MIN(GET_MAX_HIT(victim), GET_HIT(victim)+hp);
}



/*
 * Does nothing.  Used for scripts.
 */
ACMD(do_mpnothing)
{
    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    return;
}


/*
 *   Sends a message to sleeping character.  Should be fun
 *    with room sleep_progs
 *
 */
ACMD(do_mpdream)
{
    char arg1[MAX_STRING_LENGTH];
    CHAR_DATA *vict;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc)
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }


    argument = one_argument( argument, arg1 );

    if (  (vict =get_char_vis(ch, arg1)) == NULL )
    {
        progbug( "Mpdream: No such character", ch );
        return;
    }

    if( GET_POS(vict) <= POS_SLEEPING)
    {
        send_to_char(argument, vict);
        send_to_char("\n\r",   vict);
    }
    return;
}



ACMD(do_mpdelay)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int delay;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    if ( !*arg ) {
        send_to_char( "Delay for how many rounds?n\r", ch );
        progbug( "Mpdelay: no duration specified", ch );
        return;
    }
    if ( !( victim = get_char_room_vis( ch, arg ) ) ) {
        send_to_char( "They aren't here.\n\r", ch );
        progbug( "Mpdelay: target not in room", ch );
        return;
    }
    if ( IS_IMMORT( victim ) ) {
        send_to_char( "Not against immortals.\n\r", ch );
        progbug( "Mpdelay: target is immortal", ch );
        return;
    }
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) ) {
        send_to_char( "Delay them for how many rounds?\n\r", ch );
        progbug( "Mpdelay: invalid (nonexistant?) argument", ch );
        return;
    }
    delay = atoi( arg );
    if ( delay < 1 || delay > 30 ) {
        send_to_char( "Argument out of range.\n\r", ch );
        progbug( "Mpdelay:  argument out of range (1 to 30)", ch );
        return;
    }
    WAIT_STATE( victim, delay * PULSE_VIOLENCE );
    send_to_char( "Mpdelay applied.\n\r", ch );
    return;
}

ACMD(do_mppeace)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    if ( !*arg ) {
        send_to_char( "Who do you want to mppeace?\n\r", ch );
        progbug( "Mppeace: invalid (nonexistent?) argument", ch );
        return;
    }
    if ( !str_cmp( arg, "all" ) ) {
        for ( rch=world[ch->in_room].people; rch; rch=rch->next_in_room ) {
            if ( FIGHTING(rch)) {
                stop_fighting( rch);
                do_sit( rch, "" );
            }


        }
        send_to_char( "Ok.\n\r", ch );
        return;
    }
    if ( ( victim = get_char_room_vis( ch, arg ) ) == NULL ) {
        send_to_char( "They must be in the room.n\r", ch );
        progbug( "Mppeace: target not in room", ch );
        return;
    }
    if ( FIGHTING(victim))
        stop_fighting( victim);
    send_to_char( "Ok.\n\r", ch );
    return;
}

/*
 * Syntax mp_open_passage x y z
 *
 * opens a 1-way passage from room x to room y in direction z
 *
 *  won't mess with existing exits
 */
ACMD(do_mp_open_passage)
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    room_num targetRoom, fromRoom;
    int targetRoomVnum, fromRoomVnum, exit_num;
    EXIT_DATA *pexit;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if( !is_number(arg1) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    fromRoomVnum = atoi(arg1);
    if(  (fromRoom = real_room( fromRoomVnum ) )  ==-1)
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if( !is_number(arg2) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    targetRoomVnum = atoi(arg2);
    if(  (targetRoom = real_room( targetRoomVnum ) )  ==-1)
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if( !is_number(arg3) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    exit_num = atoi(arg3);
    if( (exit_num < 0) || (exit_num >= NUM_OF_DIRS) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if( (pexit = EXITN( fromRoom, exit_num )) != NULL )
    {
        if( !IS_SET( pexit->exit_info, EX_PASSAGE) )
            return;
        progbug( "MpOpenPassage - Exit exists", ch );
        return;
    }

    pexit = make_exit( fromRoom, targetRoom, exit_num );
    pexit->keyword 		= NULL;
    pexit->general_description		= NULL;
    pexit->key     		= -1;
    pexit->exit_info		= EX_PASSAGE;

    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM ); */

    return;
}

/*
 * Syntax mp_close_passage x y 
 *
 * closes a passage in room x leading in direction y
 *
 * the exit must have EX_PASSAGE set
 */
ACMD(do_mp_close_passage)
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    room_num fromRoom;
    int fromRoomVnum, exit_num;
    EXIT_DATA *pexit;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        progbug( "MpClosePassage - Bad syntax null arguments", ch );
        return;
    }

    if( !is_number(arg1) )
    {
        progbug( "MpClosePassage - Bad syntax not number arg1", ch );
        return;
    }

    fromRoomVnum = atoi(arg1);
    if(  (fromRoom = real_room( fromRoomVnum ) )  ==-1)
    {
        progbug( "MpClosePassage - Bad syntax no such room arg1", ch );
        return;
    }

    if( !is_number(arg2) )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }

    exit_num = atoi(arg2);
    if( (exit_num < 0) || (exit_num >= NUM_OF_DIRS) )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }

    if( ( pexit = EXITN(fromRoom, exit_num) ) == NULL )
    {
        return;    /* already closed, ignore...  so rand_progs */
        /*                            can close without spam */
    }

    if( !IS_SET( pexit->exit_info, EX_PASSAGE) )
    {
        progbug( "MpClosePassage - Exit not a passage", ch );
        return;
    }

    extract_exit( fromRoom, pexit ,exit_num);

    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_ROOM ); */

    return;
}


extern struct char_data *supermob;
extern int global_newsupermob;
struct wait_eo
{
    struct char_data *ch;
    room_num room;

};

EVENTFUNC(event_wait)
{
    struct wait_eo *cevent=(struct wait_eo *) event_obj;
    struct char_data *ch;
    char *pom, *pom1, cmd[1000];
    struct char_data *temp;
    room_num room;




    ch=cevent->ch;
    room=cevent->room;

    if (cevent)
        free (cevent);
    if (ch)
        GET_WAIT_EVENT(ch)=NULL;

    if (!ch || DEAD(ch))
        return 0;

    if (IS_SUPERMOB(ch))
    {
        char_from_room( ch );
        char_to_room( ch, room);
    }
    pom1=ch->wait_buffer;
    while (pom1 && (pom=strchr(pom1,'\n')) && !GET_WAIT_EVENT(ch))
    {
        *pom=0;
        command_interpreter(ch, pom1);
        strcpy(ch->wait_buffer, pom+1);
        pom1=ch->wait_buffer;
    }

    if (!GET_WAIT_EVENT(ch))
    {
        //STRFREE(ch->wait_buffer);
        if (IS_SUPERMOB(ch))
        {
            char_from_room(ch);
            REMOVE_FROM_LIST(ch, character_list, next);
            free_char(ch);
        }
    }
    return 0;
}


void assign_wait_event(struct char_data *ch, int when)
{
    struct wait_eo *cevent;

    CREATE(cevent, struct wait_eo, 1);
    cevent->ch=ch;
    cevent->room=ch->in_room;
    GET_WAIT_EVENT(ch)=event_create(event_wait, cevent, when);
    if (!ch->wait_buffer)
    {
        CREATE(ch->wait_buffer, char, MAX_STRING_LENGTH);

    }
}

extern int global_wait;
ACMD(do_mpwait)
{
    char            arg[MAX_INPUT_LENGTH];
    CHAR_DATA *pom;
    int i;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg);

    if (!(i=atoi(arg))){
        bug("Mpwait - Bad syntax: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }

    if (IS_SUPERMOB(ch))
    {
        global_newsupermob=1;
    }

    global_wait=i;
    //    assign_wait_event(ch, i RL_SEC);

    return;
}


ACMD(do_mpwaitstate)
{
    char            arg[MAX_INPUT_LENGTH];
    struct char_data *vch;
    int ws;

    if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc) {
        send_to_char("Huh?\r\n", ch);
        return;
    }
    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
        bug("Mpwaitstate - Bad syntax: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if (!(ws=atoi(argument))){
        bug("Mpwaitstate - Bad syntax: vnum %d.", mob_index[ch->nr].virtual);
        return;
    }
    if (!str_cmp(arg, "all")) {
        struct char_data *vch;

        for (vch=world[ch->in_room].people;vch;vch=vch->next) {
            if (!IS_NPC(vch))

                WAIT_STATE(vch, ws RL_SEC);
        }
    } else {
        struct char_data *victim;

        if ((victim = get_char_room_vis(ch, arg)) == NULL) {
            bug("Mpwaitstate - No such victim: vnum %d.",
                mob_index[ch->nr].virtual);
            return;
        }
        if (!IS_NPC(victim))
            WAIT_STATE(victim, ws RL_SEC);

    }

    return;
}

