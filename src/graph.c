/* ************************************************************************
*   File: graph.c                                       Part of CircleMUD *
*  Usage: various graph algorithms                                        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


#define TRACK_THROUGH_DOORS 

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "events.h"


/* Externals */
extern int      top_of_world;
extern const char *dirs[];
extern struct room_data *world;

struct bfs_queue_struct {
    sh_int          room;
    char            dir;
    struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = 0,
                    *queue_tail = 0;

/* Utility macros */
#define MARK(room) (SET_BIT(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define UNMARK(room) (REMOVE_BIT(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define IS_MARKED(room) (IS_SET(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define TOROOM(x, y) (world[(x)].dir_option[(y)]->to_room)
#define IS_CLOSED(x, y) (IS_SET(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED))

#ifdef TRACK_THROUGH_DOORS
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!ROOM_FLAGGED(TOROOM(x, y), ROOM_NOTRACK)) && \
			  (!IS_MARKED(TOROOM(x, y))))
#else
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!IS_CLOSED(x, y)) &&		\
			  (!ROOM_FLAGGED(TOROOM(x, y), ROOM_NOTRACK)) && \
			  (!IS_MARKED(TOROOM(x, y))))
#endif

void            bfs_enqueue(sh_int room, char dir)
{
    struct bfs_queue_struct *curr;

    CREATE(curr, struct bfs_queue_struct, 1);
    curr->room = room;
    curr->dir = dir;
    curr->next = 0;

    if (queue_tail) {
        queue_tail->next = curr;
        queue_tail = curr;
    } else
        queue_head = queue_tail = curr;
}


void            bfs_dequeue(void)
{
    struct bfs_queue_struct *curr;

    curr = queue_head;

    if (!(queue_head = queue_head->next))
        queue_tail = 0;
    DISPOSE(curr);
}


void            bfs_clear_queue(void)
{
    while (queue_head)
        bfs_dequeue();
}


/* find_first_step: given a source room and a target room, find the first
   step on the shortest path from the source to the target.

   Intended usage: in mobile_activity, give a mob a dir to go if they're
   tracking another mob or a PC.  Or, a 'track' skill for PCs.
*/

int             find_first_step(sh_int src, sh_int target)
{
    int             curr_dir;
    sh_int          curr_room;

    if (src < 0 || src > top_of_world || target < 0 || target > top_of_world) {
        logs("Illegal value passed to find_first_step (src=%d, target=%d)", src, target);
        return BFS_ERROR;
    }
    if (src == target)
        return BFS_ALREADY_THERE;

    /* clear marks first */
    for (curr_room = 0; curr_room <= top_of_world; curr_room++)
        UNMARK(curr_room);

    MARK(src);

    /* first, enqueue the first steps, saving which direction we're going. */
    for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
        if (VALID_EDGE(src, curr_dir)) {
            MARK(TOROOM(src, curr_dir));
            bfs_enqueue(TOROOM(src, curr_dir), curr_dir);
        }
    /* now, do the classic BFS. */
    while (queue_head) {
        if (queue_head->room == target) {
            curr_dir = queue_head->dir;
            bfs_clear_queue();
            return curr_dir;
        } else {
            for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
                if (VALID_EDGE(queue_head->room, curr_dir)) {
                    MARK(TOROOM(queue_head->room, curr_dir));
                    bfs_enqueue(TOROOM(queue_head->room, curr_dir), queue_head->dir);
                }
            bfs_dequeue();
        }
    }

    return BFS_NO_PATH;
}


/************************************************************************
*  Functions and Commands which use the above fns		        *
************************************************************************/

ACMD(do_track)
{
    struct char_data *vict;
    int             dir,
    num;

    if (!GET_SKILL(ch, SKILL_TRACK)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }
    if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))) {
        send_to_char("There are many tracks on the ground. You are confused!\r\n", ch);
        return;
    }
    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Whom are you trying to track?\r\n", ch);
        return;
    }
    if (!(vict = get_char_vis(ch, arg))) {
        send_to_char("No one is around by that name.\r\n", ch);
        return;
    }
    if (world[ch->in_room].zone!=world[vict->in_room].zone)
    {
        act("You are too far from $N to sense any trail.", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }

    if (IS_AFFECTED(vict, AFF_NOTRACK)) {
        send_to_char("You sense no trail.\r\n", ch);
        return;
    }

    /* 101 is a complete failure, no matter what the proficiency. */
    if (number(0, 101) >= GET_SKILL(ch, SKILL_TRACK)) {
        /* Find a random direction. :) */
        int tries = 10;
        do {
            dir = number(0, NUM_OF_DIRS - 1);
        } while (!CAN_GO(ch, dir) && --tries);
        sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
        send_to_char(buf, ch);
        return;
    }


    dir = find_first_step(ch->in_room, vict->in_room);

    switch (dir) {
    case BFS_ERROR:
        send_to_char("Hmm.. something seems to be wrong.\r\n", ch);
        break;
    case BFS_ALREADY_THERE:
        send_to_char("You're already in the same room!!\r\n", ch);
        break;
    case BFS_NO_PATH:
        sprintf(buf, "You can't sense a trail to %s from here.\r\n",
                HMHR(vict));
        send_to_char(buf, ch);
        break;
    default:
        sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
        send_to_char(buf, ch);
        improve_skill(ch, SKILL_TRACK, 2);
        break;
    }
}



#define DISPOSE_HUNT(ch) ({HUNTING(ch)=0;GET_UTIL_EVENT(ch)=0;if (hevent) DISPOSE(hevent);})

EVENTFUNC(event_hunt_victim)
{
    ACMD(do_say);
    extern struct char_data *character_list;
    char            buf[256];

    int             dir;
    int            found;
    struct char_data *tmp;
    struct hunt_eo *hevent=(struct hunt_eo *) event_obj;
    struct char_data *ch, *vict;


    ch=hevent->ch;



    if (!ch || !HUNTING(ch) || !GET_UTIL_EVENT(ch))
        goto hevent_kraj;

    if (ch->in_room<1 || !world[ch->in_room].people || IS_SHOPKEEPER(ch))//(MOB_FLAGGED(ch, MOB_SENTINEL) && !MOB_FLAGGED(ch, MOB_ASSASSIN)))
        goto hevent_kraj;



    if (FIGHTING(ch) || GET_POS(ch)!=POS_STANDING)
    {
        if (!number(0, 30))
            goto hevent_kraj;
        else
            return 50;
    }

    /* make sure the char still exists */
    for (found = 0, tmp = character_list; tmp && !found; tmp = tmp->next)
        if (HUNTING(ch) == tmp)
            found = 1;

    if (!found) {

        send_to_char("It seems that you have lost your victim's trail.\r\n", ch);
        if (MOB_FLAGGED(ch, MOB_ASSASSIN)) {
            if (HIRED_BY(ch))
            {char bb[100];
                sprintf(bb,"%s I lost the trail. I'm going back..", GET_NAME(HIRED_BY(ch)));
                do_tell(ch,bb,0,0);
            }

            act("$n dissapears in a hurry.",FALSE, ch, NULL, NULL, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(8245));
            HIRED_BY(ch)=NULL;
        }
        goto hevent_kraj;
    }
    if (ch->in_room == HUNTING(ch)->in_room)
    {
        if (IS_NPC(ch) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
            sprintf(buf, "There you are!");
            if (GET_LEVEL(ch)>7)
                do_say(ch, buf, 0, 0);
            vict=HUNTING(ch);
            DISPOSE_HUNT(ch);
            HUNTING(ch)=vict;
            hit(ch, vict, TYPE_UNDEFINED);
            goto hevent_kraj1;
        }


        if (!IS_NPC(ch))
        {
            act("You have found $N.", FALSE, ch, NULL, HUNTING(ch), TO_CHAR);
            goto hevent_kraj;
        }
    }

    if (IS_AFFECTED(HUNTING(ch), AFF_NOTRACK)) {
        send_to_char("It seems that you have lost your victim's trail.\r\n", ch);
        goto hevent_kraj;
    }

    if (!IS_NPC(ch) &&  world[ch->in_room].zone!=world[HUNTING(ch)->in_room].zone)
    {
        send_to_char("Many tracks mangle here and you are unable to proceed.\r\n", ch);
        goto hevent_kraj;
    }

    if (!IS_NPC(ch))
    {
        if (number(0, 180) >= GET_SKILL(ch, SKILL_TRACK) + GET_SKILL(ch, SKILL_HUNT)) {
            do {
                dir = number(0, NUM_OF_DIRS - 1);
            } while (!CAN_GO(ch, dir));
            //sprintf(buf, "You continue hunting %s.\r\n", GET_NAME(HUNTING(ch)));
            //send_to_char(buf, ch);

            //        CREF(ch, CHAR_NULL);
            improve_skill(ch, SKILL_TRACK, 8);
            improve_skill(ch, SKILL_HUNT, 8);
            
            perform_move(ch, dir, 1);     
            //      CUREF(ch);
            if (DEAD(ch))
                return 0;
            return 5;
        }
    }

    if (IS_NPC(ch) && IS_CLERIC(ch))
    {
        call_magic(ch, HUNTING(ch), 0, SPELL_SUMMON, GET_LEVEL(ch), CAST_SPELL, "mob_sum");
        if (ch->in_room==HUNTING(ch)->in_room)
        {
            do_say(ch, "You can run, but not from me!", 0, 0);
            vict=HUNTING(ch);
            DISPOSE_HUNT(ch);
            HUNTING(ch)=vict;
            hit(ch, vict, TYPE_UNDEFINED);

        }

        goto hevent_kraj1;
    }

    dir = find_first_step(ch->in_room, HUNTING(ch)->in_room);

    if (dir!=BFS_ALREADY_THERE && dir < 0) {
        send_to_char("Many tracks mangle here and you are unable to proceed.\r\n", ch);
        if (IS_NPC(ch) && number(0, 30))
            return 50;
        if (MOB_FLAGGED(ch, MOB_ASSASSIN)) {
            if (HIRED_BY(ch))
            {
                char bb[100];
                sprintf(bb,"%s I lost the trail. I am going back..", GET_NAME(HIRED_BY(ch)));
                do_tell(ch,bb,0,0);
            }
            act("$n dissapears in a hurry.",FALSE, ch, NULL, NULL, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(8245));
            HIRED_BY(ch)=NULL;
        }

        goto hevent_kraj;
    } else {

        if (dir!=BFS_ALREADY_THERE && (!IS_NPC(ch) || (!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
                            (world[EXIT(ch, dir)->to_room].zone == world[ch->in_room].zone)))) {
            //CREF(ch, CHAR_NULL);
            improve_skill(ch, SKILL_TRACK, 8);
            improve_skill(ch, SKILL_HUNT, 8);
            
            perform_move(ch, dir, 1);          }
        //CUREF(ch);
        if (DEAD(ch))
            return 0;
        if (HUNTING(ch) && ch->in_room == HUNTING(ch)->in_room) {
            if (IS_NPC(ch) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
                sprintf(buf, "There you are!");
                if (GET_LEVEL(ch)>7)
                    do_say(ch, buf, 0, 0);
                vict=HUNTING(ch);
                DISPOSE_HUNT(ch);
                HUNTING(ch)=vict;
                hit(ch, vict, TYPE_UNDEFINED);
                goto hevent_kraj1;
            }

            if (!IS_NPC(ch))
            {
                act("&GYou have found $N.&0", FALSE, ch, NULL, HUNTING(ch), TO_CHAR);
                goto hevent_kraj;
            }
        }
        return 5;
    }
    return 5;
hevent_kraj:
    DISPOSE_HUNT(ch);
hevent_kraj1:
    return 0;
}



ACMD(do_hunt)
{
    struct char_data *vict;
    int             dir,
    num;
    struct hunt_eo *hevent;

    if (!GET_SKILL(ch, SKILL_HUNT)) {
        send_to_char("You have no idea how.\r\n", ch);
        return;
    }



    if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))) {
        send_to_char("There are many tracks on the ground. You are confused!\r\n", ch);
        return;
    }



    skip_spaces(&argument);
    //one_argument(argument, arg);
    if (!*argument) {
        send_to_char("Use hunt <victim_name> to start hunting, or 'hunt !' to abort.\r\n", ch);
        return;
    }

    if (*argument=='!')
    {
        if (HUNTING(ch))
        {
            act("You stop hunting $N.", FALSE, ch, NULL, HUNTING(ch), TO_CHAR);
            HUNTING(ch)=0;
            event_cancel(GET_UTIL_EVENT(ch));
            GET_UTIL_EVENT(ch)=0;
        }
        else
            send_to_char("You are not hunting anyone.\r\n", ch);
        return;
    }


    if (IN_EVENT(ch))
    {
        send_to_char("You are already performing an action.\r\n", ch);
        return;
    }


    if (!(vict = get_char_vis(ch, argument))) {
        send_to_char("No one is around by that name.\r\n", ch);
        return;
    }

    if (world[ch->in_room].zone!=world[vict->in_room].zone)
    {
        act("You are too far from the $N to sense any trail.", FALSE, ch, NULL, vict, TO_CHAR);
        return;
    }

    if (IS_AFFECTED(vict, AFF_NOTRACK)) {
        send_to_char("You sense no trail.\r\n", ch);
        return;
    }
    dir = find_first_step(ch->in_room, vict->in_room);

    switch (dir) {
    case BFS_ERROR:
        send_to_char("Hmm.. something seems to be wrong.\r\n", ch);
        break;
    case BFS_ALREADY_THERE:
        send_to_char("You're already in the same room!!\r\n", ch);
        break;
    case BFS_NO_PATH:
        sprintf(buf, "You can't sense a trail to %s from here.\r\n",
                HMHR(vict));
        send_to_char(buf, ch);
        break;
    default:
        act("You start hunting $N.", FALSE, ch, NULL, vict, TO_CHAR);
        HUNTING(ch)=vict;
        break;
    }



    CREATE(hevent, struct hunt_eo, 1);
    hevent->ch=ch;
    GET_UTIL_EVENT(ch)=event_create(event_hunt_victim, hevent, 5);    // 5 = 1 sec


}



void            hunt_victim(struct char_data * ch)
{
    ACMD(do_say);
    extern struct char_data *character_list;
    char            buf[256];

    int             dir;
    int            found;
    struct char_data *tmp;

    if (!ch || !HUNTING(ch))
        return;

    if (ch->in_room==NOWHERE || IS_SHOPKEEPER(ch))//(MOB_FLAGGED(ch, MOB_SENTINEL) && !MOB_FLAGGED(ch, MOB_ASSASSIN)))
    {
        HUNTING(ch)=0;
        return;
    }


    SANITY_CHECK(ch);

    if (FIGHTING(ch))
        return;
    if (!AWAKE(ch))
        return;
    /* make sure the char still exists */
    for (found = 0, tmp = character_list; tmp && !found; tmp = tmp->next)
        if (HUNTING(ch) == tmp)
            found = 1;

    if (!found) {
        HUNTING(ch) = 0;
        send_to_char("It seems that you have lost your victim's trail.\r\n", ch);
        if (MOB_FLAGGED(ch, MOB_ASSASSIN)) {
            if (HIRED_BY(ch))
            {char bb[100];
                sprintf(bb,"%s I lost the trail. I'm going back..", GET_NAME(HIRED_BY(ch)));
                do_tell(ch,bb,0,0);
            }

            act("$n dissapears in a hurry.",FALSE, ch, NULL, NULL, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(8245));
            HIRED_BY(ch)=NULL;
        }
        return;
    }
    if (ch->in_room == HUNTING(ch)->in_room)
    {
        if (IS_NPC(ch) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
            sprintf(buf, "There you are!");
            do_say(ch, buf, 0, 0);
            hit(ch, HUNTING(ch), TYPE_UNDEFINED);
            return;
        }


        if (!IS_NPC(ch))
        {
            act("You have found $N.", FALSE, ch, NULL, HUNTING(ch), TO_CHAR);
            HUNTING(ch)=0;
        }
    }

    if (IS_AFFECTED(HUNTING(ch), AFF_NOTRACK)) {
        send_to_char("It seems that you have lost your victim's trail.\r\n", ch);
        HUNTING(ch)=0;
        return;
    }

    if (!IS_NPC(ch) &&  world[ch->in_room].zone!=world[HUNTING(ch)->in_room].zone)
    {
        send_to_char("Many tracks mangle here and you are unable to proceed.\r\n", ch);
        HUNTING(ch)=0;
        return;
    }

    if (!IS_NPC(ch))
    {
        if (number(0, 180) >= GET_SKILL(ch, SKILL_TRACK) + GET_SKILL(ch, SKILL_HUNT)) {
            do {
                dir = number(0, NUM_OF_DIRS - 1);
            } while (!CAN_GO(ch, dir));
            //sprintf(buf, "You continue hunting %s.\r\n", GET_NAME(HUNTING(ch)));
            //send_to_char(buf, ch);

            perform_move(ch, dir, 1);
            return;
        }
    }

    if (IS_NPC(ch) && IS_CLERIC(ch))
    {
        call_magic(ch, HUNTING(ch), 0, SPELL_SUMMON, GET_LEVEL(ch), CAST_SPELL, "mob_sum");
        if (ch->in_room==HUNTING(ch)->in_room)
        {
            do_say(ch, "You can run, but you can't hide!!!", 0, 0);
            hit(ch, HUNTING(ch), TYPE_UNDEFINED);
        }


        return;
    }

    dir = find_first_step(ch->in_room, HUNTING(ch)->in_room);

    if (dir!=BFS_ALREADY_THERE && dir < 0) {
        send_to_char("Many tracks mangle here and you are unable to proceed.\r\n", ch);
        HUNTING(ch) = 0;
        if (MOB_FLAGGED(ch, MOB_ASSASSIN)) {
            if (HIRED_BY(ch))
            {
                char bb[100];
                sprintf(bb,"%s I lost the trail. I am going back..", GET_NAME(HIRED_BY(ch)));
                do_tell(ch,bb,0,0);
            }
            act("$n dissapears in a hurry.",FALSE, ch, NULL, NULL, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, real_room(8245));
            HIRED_BY(ch)=NULL;
        }

        return;
    } else {

        if (dir!=BFS_ALREADY_THERE && (!IS_NPC(ch) || (!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
                            (world[EXIT(ch, dir)->to_room].zone == world[ch->in_room].zone))))
            perform_move(ch, dir, 1);
        if (HUNTING(ch) && ch->in_room == HUNTING(ch)->in_room) {
            if (IS_NPC(ch) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
                sprintf(buf, "There you are!");
                do_say(ch, buf, 0, 0);
                hit(ch, HUNTING(ch), TYPE_UNDEFINED);
                return;
            }

            if (!IS_NPC(ch))
            {
                act("&GYou have found $N.&0", FALSE, ch, NULL, HUNTING(ch), TO_CHAR);
                HUNTING(ch)=0;
            }
        }
        return;
    }
}

