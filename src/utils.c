/*
*   File: utils.c                                       Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/telnet.h>
#include <netinet/in.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "clan.h"

extern int top_of_zone_table;
extern struct zone_data *zone_table;
extern struct time_data time_info;
extern struct room_data *world;
extern struct char_data *character_list;

unsigned long   my_rand(void);
void            kill_group_spells(struct char_data * victim);


/* creates a random number in interval [from;to] */
int             number(int from, int to)
{
    int             i = 0;
    /* if (from > to) {
         i = from;
         from = to;
         to = i;
     }*/  

    if ( ( to = to - from + 1 ) <= 1 )
        return from;
    return (my_rand() % to) + from;
    /*
    if ( (to-from) < 1 )
        return from;
    return ((my_rand() % (to-from+1)) + from);
          */

    to = MAX(to, 1);
    from = MAX(from, 0);
    i = ((my_rand() % (to - from + 1)) + from);
    return i;
}



/* simulates dice roll */
int             dice(int num, int size)
{
    int             sum = 0;

    if (size <= 0 || num <= 0)
        return 0;

    //  return (number(num, num*size));

    while (num-- > 0)
        sum += ((my_rand() % size) + 1);

    return sum;
}


int             MIN(int a, int b)
{
    return a < b ? a : b;
}


int             MAX(int a, int b)
{
    return a > b ? a : b;
}


unsigned int             UMIN(unsigned int a, unsigned int b)
{
    return a < b ? a : b;
}


unsigned int             UMAX(unsigned int a, unsigned int b)
{
    return a > b ? a : b;
}



/* Create a duplicate of a string */
char           *str_dup(const char *source)
{
    char           *new;

    CREATE(new, char, strlen(source) + 1);
    return (strcpy(new, source));
}



/* str_cmp: a case-insensitive version of strcmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int             str_cmp(char *arg1, char *arg2)
{
    int             chk;
    //			i;

    //    for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    for (; *(arg1) || *(arg2); arg1++, arg2++)
        //        if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
        if ((chk = LOWER(*(arg1)) - LOWER(*(arg2))))
            return (chk<0 ? -1:1);
    /*            if (chk < 0)
                    return (-1);
                else
                    return (1);*/
    return (0);
}


/* strn_cmp: a case-insensitive version of strncmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int             strn_cmp(char *arg1, char *arg2, int n)
{
    int             chk,
    i;

    for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
        if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
            if (chk < 0)
                return (-1);
            else
                return (1);
        }

    return (0);
}


/* log a death trap hit */
void            log_death_trap(struct char_data * ch)
{
    char            buf[150];
    extern struct room_data *world;

    sprintf(buf, "%s hit death trap #%d (%s)", GET_NAME(ch),
            world[ch->in_room].number, world[ch->in_room].name);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
}


/* writes a string to the log */
void            log(char *str)
{
    time_t          ct;
    char           *tmstr;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    fprintf(stderr, "%-15.15s :: %s\n", tmstr + 4, str);
}


/* the "touch" command, essentially. */
int             touch(char *path)
{
    FILE           *fl;

    if (!(fl = fopen(path, "a"))) {
        perror(path);
        return -1;
    } else {
        fclose(fl);
        return 0;
    }
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void            mudlog(char *str, char type, sbyte level, byte file)
{
    char            buf[256];
    extern struct descriptor_data *descriptor_list;
    struct descriptor_data *i;
    char           *tmp,
    tp;
    time_t          ct;

    ct = time(0);
    tmp = asctime(localtime(&ct));

    if (file)
        fprintf(stderr, "%-15.15s :: %s\n", tmp + 4, str);
    if (level < 0)
        return;

    sprintf(buf, "[ %s ]\r\n", str);

    for (i = descriptor_list; i; i = i->next)
        if (i->connected==CON_PLAYING && !PLR_FLAGGED(i->character, PLR_WRITING)) {
            tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
                  (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));

            if ((GET_LEVEL(i->character) >= level) && (tp >= type)) {
                send_to_char(CCGRN(i->character, C_NRM), i->character);
                send_to_char(buf, i->character);
                send_to_char(CCNRM(i->character, C_NRM), i->character);
            }
        }
}



void            sprintbit(ulong vektor, char *names[], char *result)
{
    unsigned long   nr;

    *result = '\0';

    if (vektor < 0) {
        strcpy(result, "SPB_ERROR");
        return;
    }
    for (nr = 0; vektor; vektor >>= 1) {
        if (IS_SET(1, vektor)) {
            if (*names[nr] != '\n') {
                strcat(result, names[nr]);
                strcat(result, " ");
            }
            //  else    strcat(result, "UNDEFINED ");
        }
        if (*names[nr] != '\n')
            nr++;
    }

    if (!*result)
        strcat(result, "NONE ");
}

void            sprinttype(int type, char *names[], char *result)
{
    int             nr;

    for (nr = 0; (*names[nr] != '\n'); nr++);
    if (type < nr)
        strcpy(result, names[type]);
    // else
    //   strcpy(result, "UNDEFINED");
}

char           *sprintbitascii(ulong vektor, char *result)
{
    unsigned long   nr;
    int             temp = 0;

    *result = '\0';

    for (nr = 0, temp = 0; nr < 32; nr++) {
        if (IS_SET(vektor, 1 << nr)) {
            if (nr < 26)
                result[temp++] = (char) ('a' + nr);
            else
                result[temp++] = (char) ('A' + nr - 26);
        }
    }
    result[temp] = '\0';
    if (!temp) {
        strcpy(result, "0");
    }
    return result;
}

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
    long            secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs / SECS_PER_REAL_HOUR) % 24;       /* 0..23 hours */
    secs -= SECS_PER_REAL_HOUR * now.hours;

    now.day = (secs / SECS_PER_REAL_DAY);       /* 0..34 days  */
    secs -= SECS_PER_REAL_DAY * now.day;

    now.month = -1;
    now.year = -1;

    return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long            secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs / SECS_PER_MUD_HOUR) % 24;        /* 0..23 hours */
    secs -= SECS_PER_MUD_HOUR * now.hours;

    now.day = (secs / SECS_PER_MUD_DAY) % 35;   /* 0..34 days  */
    secs -= SECS_PER_MUD_DAY * now.day;

    now.month = (secs / SECS_PER_MUD_MONTH) % 17;       /* 0..16 months */
    secs -= SECS_PER_MUD_MONTH * now.month;

    now.year = (secs / SECS_PER_MUD_YEAR);      /* 0..XX? years */

    return now;
}



struct time_info_data age(struct char_data * ch)
{
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0), ch->player.time.birth);

    player_age.year += 17;      /* All players start at 17 */

    return player_age;
}




/*
 * Turn off echoing (specific to telnet client)
 */
void            echo_off(struct descriptor_data * d)
{
    char            off_string[] =
        {
            (char) IAC,
            (char) WILL,
            (char) TELOPT_ECHO,
            (char) 0,
        };

    SEND_TO_Q(off_string, d);
}


/*
 * Turn on echoing (specific to telnet client)
 */
void            echo_on(struct descriptor_data * d)
{
    char            on_string[] =
        {
            (char) IAC,
            (char) WONT,
            (char) TELOPT_ECHO,
            (char) TELOPT_NAOFFD,
            (char) TELOPT_NAOCRD,
            (char) 0,
        };

    SEND_TO_Q(on_string, d);
}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool            circle_follow(struct char_data * ch, struct char_data * victim)
{
    struct char_data *k;

    for (k = victim; k; k = k->master) {
        if (k == ch)
            return TRUE;
    }

    return FALSE;
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void            stop_follower(struct char_data * ch)
{
    void            make_corpse(struct char_data * ch, struct char_data * killer, int type);
    struct follow_type *j,
                *k, *kk;

    assert(ch->master);

if (IS_AFFECTED(ch, AFF_CHARM)) {
        act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
        act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
        act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
        if (affected_by_spell(ch, SPELL_CHARM))
        {
            affect_from_char(ch, SPELL_CHARM);
            leech_from_char(ch, SPELL_CHARM);

        }
    } else if (ch != ch->master) {

        if (!AFF2_FLAGGED(ch, AFF2_STALK))
        {
            act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
            act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
            act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
        }
        else
        {
            act("You stop stalking $N.", FALSE, ch, 0, ch->master, TO_CHAR);
            REMOVE_BIT(AFF2_FLAGS(ch), AFF2_STALK);
        }

    }
    if (ch->master->followers->follower == ch) {        /* Head of
                                                             * follower-list? */
        k = ch->master->followers;
        ch->master->followers = k->next;
        DISPOSE(k);
    } else if (!ch->master->followers->follower)
    {
        ch->master->followers=ch->master->followers->next;
    } else {                    /* locate follower who is not head of list */

        for ( kk=k = ch->master->followers; k && k->next && k->next->follower != ch; kk=k, k = k->next)
        {
            if (!k->follower)
                kk->next=k->next;
        };
        if (k->next && k->next->follower==ch)
        {
            j = k->next;
            k->next = j->next;
            DISPOSE(j);
        }
    }

    if (count_group(ch->master) == 1)
    {
        REMOVE_BIT(AFF_FLAGS(ch->master), AFF_GROUP);
        REMOVE_BIT(AFF3_FLAGS(ch->master), AFF3_GUARD);
        kill_group_spells(ch->master);
    }

    ch->master = NULL;

    if (count_group(ch) == 1)
        REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);

    kill_group_spells(ch);
    REMOVE_BIT(AFF3_FLAGS(ch), AFF3_GUARD);

}



/* Called when a character that follows/is followed dies */
void            die_follower(struct char_data * ch)
{
    struct follow_type *j,
                *k;
    struct char_data *vict;

    void            make_corpse(struct char_data * ch, struct char_data * killer, int type);

    if (ch->master)
        stop_follower(ch);

for (k = ch->followers; k; k = j) {
        j = k->next;
        vict = k->follower;
        stop_follower(vict);
        if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_PET)) {
            die_follower(ch);
            act("Without a master, $n perishes!", FALSE, ch, 0, 0, TO_ROOM);
            make_corpse(ch, ch, 0);
            extract_char(ch);
        } else if (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PET)) {
            die_follower(vict);
            act("Without a master, $n perishes!", FALSE, vict, 0, 0, TO_ROOM);
            make_corpse(vict, ch, 0);
            extract_char(vict);
        }
    }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void            add_follower(struct char_data * ch, struct char_data * leader)
{
    struct follow_type *k;
    struct char_data *l=leader;
    char buf[200];

    assert(!ch->master);

    if (leader->master)
        leader = leader->master;

    if (leader->in_room!=ch->in_room)
    {
        sprintf(buf,"%s is already following %s, who is not here.\r\n",GET_NAME(l),GET_NAME(leader));
        send_to_char(buf,ch);
        return;
    }

    /*if (!ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
{
    if (!IS_NPC(ch) && !IS_NPC(leader) && ((GET_CLAN(ch)==CLAN_BLUE && GET_CLAN(leader)==CLAN_RED) || (GET_CLAN(ch)==CLAN_RED && GET_CLAN(leader)==CLAN_BLUE)))
{   
        if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
        {
            send_to_char("You should kill that ugly bastard !!\r\n", ch);
            return;
        }
        send_to_char("Damn, it's your enemy there! Kill! BANZAII!!\r\n",ch);
        hit(ch, leader, TYPE_UNDEFINED);
        return;
}


    if (!IS_NPC(ch) && GET_CLAN(ch)==CLAN_NEWBIE && !IS_NPC(leader) && GET_CLAN(leader)!=CLAN_NEWBIE)
{
        act("You change your mind, as $N is much more experienced than you are. \r\nNever mind, just take your time and enjoy your newbie years.", FALSE, ch, NULL, leader, TO_CHAR);
        return;
}

    if (!IS_NPC(ch) && GET_CLAN(ch)!=CLAN_NEWBIE && !IS_NPC(leader) && GET_CLAN(leader)==CLAN_NEWBIE)
{
        act("You decide not, as $N is still a newbie and you want $M enjoy $S newbie years alone.", FALSE, ch, NULL, leader, TO_CHAR);
        return;
}
}
      */
    //if (leader->master)
    //  leader = leader->master;

    if (leader->in_room!=ch->in_room)
    {
        sprintf(buf,"%s is already following %s, who is not here.\r\n",GET_NAME(l),GET_NAME(leader));
        send_to_char(buf,ch);
        return;
    }

    if (MOB_FLAGGED(ch, MOB_PET) && count_pets(leader)>=NUM_PETS_ALLOWED)
    {
        act("$N doesn't seem to be able of controling $n.",FALSE, ch, NULL, leader, TO_NOTVICT);
        act("$n takes out of your control.",FALSE, ch, NULL, leader, TO_VICT);
        return;
    }

    ch->master = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = leader->followers;
    leader->followers = k;

    if (!AFF2_FLAGGED(ch, AFF2_STALK)){
        if (l==leader)
            act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
        else
            act("You see it's better to follow $N.", FALSE, ch, 0, leader, TO_CHAR);
        if (CAN_SEE(leader, ch))
            act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
        act("$n starts following $N.", TRUE, ch, 0, leader, TO_NOTVICT);

    }
    else
    {
        if (l==leader)
            act("You now stalk $N.", FALSE, ch, 0, leader, TO_CHAR);
        else
            act("You see it's better to follow $N.", FALSE, ch, 0, leader, TO_CHAR);
    }
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int             get_line(FILE * fl, char *buf)
{
    char            temp[256];
    int             lines = 0;

    do {
        lines++;
        fgets(temp, 256, fl);
        if (*temp)
            temp[strlen(temp) - 1] = '\0';
    } while (!feof(fl) && (*temp == '*' || !*temp));

    if (feof(fl))
        return 0;
    else {
        strcpy(buf, temp);
        return lines;
    }
}

int             get_line2(FILE * fl, char *buf)
{
    char            temp[256];
    int             lines = 0;

    do {
        lines++;
        fgets(temp, 256, fl);
        if (*temp)
            temp[strlen(temp) - 1] = '\0';
    } while (!feof(fl) && (!*temp));

    if (feof(fl))
        return 0;
    else {
        strcpy(buf, temp);
        return lines;
    }
}


int             get_filename(char *orig_name, char *filename, int mode)
{
    char           *prefix,
    *middle,
    *suffix,
    *ptr,
    name[64];

    switch (mode) {
    case CRASH_FILE:
        prefix = "plrobjs";
        suffix = "objs";
        break;
    case ETEXT_FILE:
        prefix = "plrtext";
        suffix = "text";
        break;
    case ALIAS_FILE:
        prefix = "plralias";
        suffix = "alias";
        break;
    case QUEST_FILE:
        prefix = "plrquest";
        suffix = "quest";
        break;
    default:
        return 0;
        break;
    }

    if (!*orig_name)
        return 0;

    strcpy(name, orig_name);
    for (ptr = name; *ptr; ptr++)
        *ptr = LOWER(*ptr);

    switch (LOWER(*name)) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
        middle = "A-E";
        break;
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
        middle = "F-J";
        break;
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
        middle = "K-O";
        break;
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
        middle = "P-T";
        break;
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
        middle = "U-Z";
        break;
    default:
        middle = "ZZZ";
        break;
    }
    sprintf(filename, "%s/%s/%s.%s", prefix, middle, name, suffix);
    return 1;
}





bool            CAN_MURDER(struct char_data * ch, struct char_data * victim)
{
    if (IS_NPC(ch) || IS_NPC(victim))
        return TRUE;
    if (IN_ARENA(ch)) {
        if (((RED(ch) && BLUE(victim)) || (BLUE(ch) && RED(victim))))
            return TRUE;
        else return FALSE;
    }

    if (is_same_group(ch,victim))
        return FALSE;
    //    if (GET_CLAN(ch)==GET_CLAN(victim) && (GET_CLAN(ch)!=CLAN_OUTLAW || GET_CLAN(victim)!=CLAN_OUTLAW))
    //      return FALSE;
    // if (GET_LEVEL(victim)<CLAN_ENTRY_LEVEL || GET_LEVEL(ch)<CLAN_ENTRY_LEVEL)
    // 	return FALSE;
    //if (GET_ALIGNMENT(ch)==GET_ALIGNMENT(victim) && GET_ALIGNMENT(ch)>-500)
      //  return FALSE;
    if (ch==victim)
        return FALSE;
    //if ((GET_LEVEL(ch)>(GET_LEVEL(victim)+10)) || (GET_LEVEL(ch)<(GET_LEVEL(victim)-10)))
    //return FALSE;

    return TRUE;
    //if (IS_NPC(ch) || IS_NPC(victim))
    //  return TRUE;
    //if (ch == victim)
    //  return TRUE;


}

bool            CAN_DAMAGE(struct char_data * ch, struct char_data * vic)
{
    /*   struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
       int affectlevel = 0;

        	affectlevel:   0  == none
            affectlevel:   1  == silver
            affectlevel:   2  == plus 1
            affectlevel:   3  == plus 2
            affectlevel:   4  == plus 3
            affectlevel:   5  == plus 4
            affectlevel:   6  == plus 5

       if (IS_MONK(ch)) {
            affectlevel = MAX(0, lvD3(ch) - 4);
       }
       else if (wielded) {
            if (IS_OBJ_STAT(wielded, ITEM_PLUSFIVE))
                    affectlevel = 6;
            else if (IS_OBJ_STAT(wielded, ITEM_PLUSFOUR))
                    affectlevel = 5;
            else if (IS_OBJ_STAT(wielded, ITEM_PLUSTHREE))
                    affectlevel = 4;
            else if (IS_OBJ_STAT(wielded, ITEM_PLUSTWO))
                    affectlevel = 3;
            else if (IS_OBJ_STAT(wielded, ITEM_PLUSONE))
                    affectlevel = 2;
            else if (IS_OBJ_STAT(wielded, ITEM_SILVER))
                    affectlevel = 1;
            if (IS_SAMURAI(ch) || IS_NINJA(ch))
                    affectlevel += MAX(0, lvD10(ch) - 2);
       } else if (IS_MOB(ch)) {
            if (AFF_FLAGGED(ch, AFF_PLUSFIVE))
                    affectlevel = 6;
            else if (AFF_FLAGGED(ch, AFF_PLUSFOUR))
                    affectlevel = 5;
            else if (AFF_FLAGGED(ch, AFF_PLUSFOUR))
                    affectlevel = 4;
            else if (AFF_FLAGGED(ch, AFF_PLUSTHREE))
                    affectlevel = 3;
            else if (AFF_FLAGGED(ch, AFF_PLUSTWO))
                    affectlevel = 2;
            else if (AFF_FLAGGED(ch, AFF_PLUSONE))
                    affectlevel = 1;
       }

       if (AFF_FLAGGED(vic, AFF_PLUSFIVE))
            affectlevel -= 6;
       else if (AFF_FLAGGED(vic, AFF_PLUSFOUR))
            affectlevel -= 5;
       else if (AFF_FLAGGED(vic, AFF_PLUSTHREE))
            affectlevel -= 4;
       else if (AFF_FLAGGED(vic, AFF_PLUSTWO))
            affectlevel -= 3;
       else if (AFF_FLAGGED(vic, AFF_PLUSONE))
            affectlevel -= 2;
       else if (AFF_FLAGGED(vic, AFF_SILVER))
            affectlevel -= 1;
    */
    return (!AFF_FLAGGED(vic, AFF_SILVER));
}


int             GET_REINCARN(struct char_data * ch)
{
    int             temp = 0;

    if (PRF2_FLAGGED(ch, PRF2_REINCARN1))
        temp += 1;
    if (PRF2_FLAGGED(ch, PRF2_REINCARN2))
        temp += 2;
    if (PRF2_FLAGGED(ch, PRF2_REINCARN3))
        temp += 4;

    return (temp + 1);
}

void            ADD_REINCARN(struct char_data * ch)
{
    if (GET_REINCARN(ch) >= 8)
        return;

    if (!PRF2_FLAGGED(ch, PRF2_REINCARN1))
        SET_BIT(PRF2_FLAGS(ch), PRF2_REINCARN1);
    else {
        REMOVE_BIT(PRF2_FLAGS(ch), PRF2_REINCARN1);
        if (!PRF2_FLAGGED(ch, PRF2_REINCARN2))
            SET_BIT(PRF2_FLAGS(ch), PRF2_REINCARN2);
        else {
            REMOVE_BIT(PRF2_FLAGS(ch), PRF2_REINCARN2);
            SET_BIT(PRF2_FLAGS(ch), PRF2_REINCARN3);
        }
    }
}

int             get_alignment_type(int temp)
{
    if (temp <= -950)
        return 0;
    else if (temp <= -750)
        return 1;
    else if (temp <= -500)
        return 2;
    else if (temp <= -333)
        return 3;
    else if (temp <= -200)
        return 4;
    else if (temp < 200)
        return 5;
    else if (temp < 333)
        return 6;
    else if (temp < 500)
        return 7;
    else if (temp < 750)
        return 8;
    else if (temp < 950)
        return 9;
    else
        return 10;
}

int             interpolate(int level, int value_00, int value_47)
{
    return value_00 + level * (value_47 - value_00) / 50;
}

/* string manipulation fucntion originally by Darren Wilson */
/* (wilson@shark.cc.cc.ca.us) improved and bug fixed by Chris (zero@cnw.com) */
/* completely re-written again by M. Scott 10/15/96 (scottm@workcommn.net), */
/* substitute appearances of 'pattern' with 'replacement' in string */
/* and return the # of replacements */
int             replace_str(char **string, char *pattern, char *replacement, int rep_all,
                            int max_size)
{
    char           *replace_buffer = NULL;
    char           *flow,
    *jetsam,
    temp;
    int             len,
    i;

    if ((strlen(*string) - strlen(pattern)) + strlen(replacement) > max_size)
        return -1;

    CREATE(replace_buffer, char, max_size);
    i = 0;
    jetsam = *string;
    flow = *string;
    *replace_buffer = '\0';
    if (rep_all) {
        while ((flow = (char *) strstr(flow, pattern)) != NULL) {
            i++;
            temp = *flow;
            *flow = '\0';
            if ((strlen(replace_buffer) + strlen(jetsam) + strlen(replacement)) > max_size) {
                i = -1;
                break;
            }
            strcat(replace_buffer, jetsam);
            strcat(replace_buffer, replacement);
            *flow = temp;
            flow += strlen(pattern);
            jetsam = flow;
        }
        strcat(replace_buffer, jetsam);
    } else {
        if ((flow = (char *) strstr(*string, pattern)) != NULL) {
            i++;
            flow += strlen(pattern);
            len = ((char *) flow - (char *) *string) - strlen(pattern);

            strncpy(replace_buffer, *string, len);
            strcat(replace_buffer, replacement);
            strcat(replace_buffer, flow);
        }
    }
    if (i == 0)
        return 0;
    if (i > 0) {
        RECREATE(*string, char, strlen(replace_buffer) + 3);
        strcpy(*string, replace_buffer);
    }
    DISPOSE(replace_buffer);
    return i;
}


/* re-formats message type formatted char * */
/* (for strings edited with d->str) (mostly olc and mail)     */
void            format_text(char **ptr_string, int mode, struct descriptor_data * d, int maxlen)
{
    int             total_chars,
    cap_next = TRUE,
               cap_next_next = FALSE;
    char           *flow,
    *start = NULL,
             temp;
    /* warning: do not edit messages with max_str's of over this value */
    char            formated[MAX_STRING_LENGTH];

    flow = *ptr_string;
    if (!flow)
        return;

    if (IS_SET(mode, FORMAT_INDENT)) {
        strcpy(formated, "   ");
        total_chars = 3;
    } else {
        *formated = '\0';
        total_chars = 0;
    }

    while (*flow != '\0') {
        while ((*flow == '\n') ||
                (*flow == '\r') ||
                (*flow == '\f') ||
                (*flow == '\t') ||
                (*flow == '\v') ||
                (*flow == ' '))
            flow++;

        if (*flow != '\0') {

            start = flow++;
            while ((*flow != '\0') &&
                    (*flow != '\n') &&
                    (*flow != '\r') &&
                    (*flow != '\f') &&
                    (*flow != '\t') &&
                    (*flow != '\v') &&
                    (*flow != ' ') &&
                    (*flow != '.') &&
                    (*flow != '?') &&
                    (*flow != '!'))
                flow++;

            if (cap_next_next) {
                cap_next_next = FALSE;
                cap_next = TRUE;
            }
            /* this is so that if we stopped on a sentance .. we move off the
             * sentance delim. */
            while ((*flow == '.') || (*flow == '!') || (*flow == '?')) {
                cap_next_next = TRUE;
                flow++;
            }

            temp = *flow;
            *flow = '\0';

            if ((total_chars + strlen(start) + 1) > 79) {
                strcat(formated, "\r\n");
                total_chars = 0;
            }
            if (!cap_next) {
                if (total_chars > 0) {
                    strcat(formated, " ");
                    total_chars++;
                }
            } else {
                cap_next = FALSE;
                *start = UPPER(*start);
            }

            total_chars += strlen(start);
            strcat(formated, start);

            *flow = temp;
        }
        if (cap_next_next) {
            if ((total_chars + 3) > 79) {
                strcat(formated, "\r\n");
                total_chars = 0;
            } else {
                strcat(formated, "  ");
                total_chars += 2;
            }
        }
    }
    strcat(formated, "\r\n");

    if (strlen(formated) > maxlen)
        formated[maxlen] = '\0';
    RECREATE(*ptr_string, char, MIN(maxlen, strlen(formated) + 3));
    strcpy(*ptr_string, formated);
}

bool            is_same_group(struct char_data * ach, struct char_data * bch)
{
    if (ach == bch)
        return 1;
    if (!IS_AFFECTED(ach, AFF_GROUP) || !IS_AFFECTED(bch, AFF_GROUP))
        return 0;
    if (ach->master != NULL)
        ach = ach->master;
    if (bch->master != NULL)
        bch = bch->master;
    return (ach == bch);
}

int             count_group(struct char_data * ch)
{
    struct char_data *k;
    struct follow_type *f;
    int             i = 1;

    if (ch != NULL) {
        if (!(k = ch->master))
            k = ch;

        for (f = k->followers; f; f = f->next)
            if (f->follower && IS_AFFECTED(f->follower, AFF_GROUP))
                i++;
    }
    return i;
}

int             count_pets(struct char_data * ch)
{
    struct char_data *k;
    struct follow_type *f;
    int             i = 0;

    if (ch != NULL) {
        // if (!(k = ch->master))
        k = ch;
        for (f = k->followers; f; f = f->next)
            if (f->follower && IS_NPC(f->follower) && (IS_AFFECTED(f->follower, AFF_GROUP) || MOB_FLAGGED(f->follower,MOB_PET)))
                i++;
    }
    return i;
}

/*
int             exp_this_level(int lev)
{
    return (MOB_EXP(lev) * MOBS_PER_LEVEL(lev));
}

int             total_exp(int lev)
{
    int             i,
                    sum = 0;

    for (i = 1; i <= lev; i++)
        sum += exp_this_level(i);

    return (sum);
}

int             power(int n)
{
    int             i;
    float           res;

    res = 26;
    for (i = 1; i <= n; i++)
        res *= 1.17;
    return ((int) res);
}
*/

int             level_power(int base, int n)
{
    int             i;
    float           res, con;

    res = 1.0;
    con=n>0? 1.23 : 1.27;
    for (i = 1; i <= abs(n); i++)
        res *= con;
    if (n<0)
        res=1.0/res;
    res=res*base;
    return ((int) res);
}



int align_damage(struct char_data *ch,int dam)
{

    int i=GET_ALIGNMENT(ch),j=0;

    if (GET_REAL_ALIGNMENT(ch)==0)
    {
        if (i<-667)
            j=-20;
        else if (i<-333)
            j=-10;
        else if (i>667)
            j=-20;
        else if (i>333)
            j=-10;
        else if (i<65 && i>-65)
            j=10;
    }
    else if (GET_REAL_ALIGNMENT(ch)<0)
    {
        if (i>667)
            j=-40;
        else if (i>333)
            j=-30;
        else if (i>0)
            j=-20;
        else if (i>-333)
            j=-10;
        else if (i<933)
            j=10;
    }
    else
    {
        if (i<-665)
            j=-40;
        else if (i<-333)
            j=-30;
        else if (i<0)
            j=-20;
        else if (i<333)
            j=-10;
        else if (i>933)
            j=10;
    }

    return (dam+=dam*j/100);
}


char *mood_str[]={
                     "You feel terrible.\r\n",
                     "You don't feel very well.\r\n",
                     "You feel disturbed.\r\n",
                     "You feel somewhat uneasy.\r\n",
                     "You feel good.\r\n",
                     "You feel great!\r\n"
                 };

char *get_mood(struct char_data *ch)
{

    int i=GET_ALIGNMENT(ch),j=0;

    if (GET_REAL_ALIGNMENT(ch)==0)
    {
        if (i<-667)
            j=-20;
        else if (i<-333)
            j=-10;
        else if (i>667)
            j=-20;
        else if (i>333)
            j=-10;
        else if (i<65 && i>-65)
            j=10;
    }
    else if (GET_REAL_ALIGNMENT(ch)<0)
    {
        if (i>667)
            j=-40;
        else if (i>333)
            j=-30;
        else if (i>0)
            j=-20;
        else if (i>-333)
            j=-10;
        else if (i<933)
            j=10;
    }
    else
    {
        if (i<-667)
            j=-40;
        else if (i<-333)
            j=-30;
        else if (i<0)
            j=-20;
        else if (i<333)
            j=-10;
        else if (i>933)
            j=10;
    }

    return (mood_str[j/10+4]);
}


char *make_bar(char *bufmb, int percent, int len)
{
    int i;

    percent=MAX(0, percent);
    for (i=0;i<len;i++)
        bufmb[i]=':';
    for (i=0;i<percent*len/100;i++)
        bufmb[i]='#';
    if (percent>0)
        bufmb[0]='#';
    bufmb[len]='\0';
    return bufmb;
}


char *make_a_bar(char *bufmb, int left, int right, int pos, int len)
{
    int i,ll, rr;

    for (i=0;i<len;i++)
        bufmb[i]=':';

    if (pos<0)
    {
        ll=pos*len/(abs(left)+abs(right))+len/2;
        rr=len/2;
    }
    else
    {
        rr=pos*len/(abs(left)+abs(right))+len/2;
        ll=len/2;
    }

    for (i=ll;i<=rr;i++)
        bufmb[i]='#';

    bufmb[len]='\0';
    return bufmb;
}


int get_zone_rnum(int vnum)
{ int j=-1,i;
    for (i = 0; i <= top_of_zone_table; i++)
        if (zone_table[i].number == vnum)
        {
            j=i;
            break;
        }
    return j;
}

char clanb[20];

char* get_clan_name(struct char_data *ch)
{
    switch GET_CLAN(ch) {
    case CLAN_NEWBIE:
        strcpy(clanb, "Newbie");
        break;
    case CLAN_BLUE:
        strcpy(clanb, "Blue Dragon");
        break;
    case CLAN_RED:
        strcpy(clanb, "Nightsnake");
        break;
    case CLAN_OUTLAW:
        strcpy(clanb, "Outlaw");
        break;
    }
    return clanb;
}


int bti(int bv)
{
    int i,j;

    for (j = 0; j < NUM_CLASSES; j++)
        if (IS_SET(bv, (1 << j)))
            return j;
    return 0;
}

char xbuf2[MAX_STRING_LENGTH];
char xbuf[MAX_STRING_LENGTH];
char linewrap_buffer[MAX_STRING_LENGTH];

char *linewrap(char *str, int max)
{
    char *tmp;
    int i, lline = 0, curr = 0;
    bool spec_code = FALSE;

    xbuf[0] = xbuf2[0] = '\0';
    i = max - 1;

    if (strlen(str) < i || max<40)
        return (str);

    for (tmp = str; *tmp; tmp++) {
        if (*tmp == '\x1B' && !spec_code)
            spec_code = TRUE;
        if (*tmp == ' ') {
            spec_code = FALSE;
            if (lline > i) {
                sprintf(xbuf, "%s\r\n%s ", xbuf, xbuf2);
                lline = 0;
                curr = 0;
                xbuf2[0] = '\0';
            } else {
                sprintf(xbuf, "%s%s ", xbuf, xbuf2);
                lline++;
                curr = 0;
                xbuf2[0] = '\0';
            }
        } else if (*tmp == '\r') {
            spec_code = FALSE;
            if (lline > (i + 1))
                sprintf(xbuf, "%s\r\n%s", xbuf, xbuf2);
            else
                sprintf(xbuf, "%s%s\r", xbuf, xbuf2);
            lline = 0;
            curr = 0;
            xbuf2[0] = '\0';
        } else {
            xbuf2[curr] = *tmp;
            xbuf2[curr + 1] = '\0';
            curr++;
            if (!spec_code)
                lline++;
        }
        if (*tmp == 'm' && spec_code)
            spec_code = FALSE;
    }
    if (lline > i)
        sprintf(xbuf, "%s\r\n%s", xbuf, xbuf2);
    else
        sprintf(xbuf, "%s%s", xbuf, xbuf2);
    return ((xbuf));
}

void sprintf_minutes( char* tmp, const time_t time )
{
    int days    = time/(60*60*24);
    int hours   = (time/3600)%24;
    int minutes = (time/60)%60;

    tmp[0] = '\0';

    if( days > 0 )
        sprintf( tmp, "%d day%s ", days, days == 1 ? "" : "s" );

    if( hours > 0 )
        sprintf( tmp+strlen(tmp), "%d hour%s ", hours, hours == 1 ? "" : "s" );

    sprintf( tmp+strlen(tmp), "%d minute%s", minutes, minutes == 1 ? "" : "s" );
}



int get_other_armor(struct char_data *victim)
{
    int sum=0;
    if (GET_EQ(victim, WEAR_BODY))
        sum+=GET_OBJ_VAL(GET_EQ(victim, WEAR_BODY), 0);
    if (GET_EQ(victim, WEAR_HEAD))
        sum+=GET_OBJ_VAL(GET_EQ(victim, WEAR_HEAD), 0);
    if (GET_EQ(victim, WEAR_LEGS))
        sum+=GET_OBJ_VAL(GET_EQ(victim, WEAR_LEGS), 0);
    if (GET_EQ(victim, WEAR_HEAD))
        sum+=GET_OBJ_VAL(GET_EQ(victim, WEAR_ARMS), 0);
    return GET_AC(victim)-sum;
}


int valid_armor_pc[]={ 1, 2, 3, 4, 8, 9, 11, 12, 13, 14, 15,  19, 20};
int get_random_armor_pc(struct char_data *victim)
{
    int i;
    i=valid_armor_pc[number(0,12)];
    if (GET_EQ(victim, i))
        return GET_OBJ_VAL(GET_EQ(victim, i), 0);
    else
        return 0;
}

int valid_armor_npc[]={ 1, 2, 3, 4, 8, 9, 11, 12, 13, 14, 15,  19, 20, 5, 6, 7, 10};
int get_random_armor_npc(struct char_data *victim)
{
    int i;
    i=valid_armor_npc[number(0,16)];
    if (GET_EQ(victim, i))
        return GET_OBJ_VAL(GET_EQ(victim, i), 0);
    else
        return 0;
}



char *strlower( const char *str )
{
    static char strlow[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strlow[i] = LOWER(str[i]);
    strlow[i] = '\0';
    return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
    static char strup[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strup[i] = UPPER(str[i]);
    strup[i] = '\0';
    return strup;
}


int fuzper(int n, int per)
{
    int i,j;
    i=2*n*per/100;
    j=number (0, i);
    return n-i/2+j;
}


int is_guarded(struct char_data *ch)
{
    struct char_data *mob;

    if (!ch)
    {
        log("SYSERR: null pointer passed to is_guarding");
        return 0;
    }

    for (mob = character_list; mob; mob = mob->next)
    {
        if (mob->guarding==ch)
            return 1;
    }
    return 0;
}


int num_fighting(struct char_data *ch)
{

    struct char_data *tmp_ch;
    int num=0;

    for (tmp_ch = world[ch->in_room].people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
        if (FIGHTING(tmp_ch)==ch)
            num++;

    return num;
}

void raw_awake(struct char_data *ch)
{
    if (GET_POS(ch)!=POS_SLEEPING)
        return;

    if (IS_AFFECTED(ch, AFF_SLEEP))
    {
        affect_from_char(ch, SPELL_SLEEP);
        leech_from_char(ch, SPELL_SLEEP);
    }

    if (AFF2_FLAGGED(ch, AFF2_NAP)) {
        affect_from_char(ch, SPELL_NAP);
        leech_from_char(ch, SPELL_NAP);
    }
    send_to_char("You awaken.\r\n", ch);
    act("$n awakes with a painful grimace on $s face.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch)=POS_STANDING;
}



char pomgetst[10000];
char* getst(char *s,int num,char raz)
{

    int i,j=strlen(s), cnt=0;
    i=0;
    *pomgetst=0;
    if (*s)
    {
        while (s[i]==raz)
            i++;

        if (num==1)
        {
            while((i<j) && (s[i]!=raz))
            {
                pomgetst[cnt++]=s[i];
                i++;
            }
        }
        else
        {
            i--;
            while ((num>1) && (i<j))
            {
                i++;
                if (s[i]==raz)
                {
                    while (s[i]==raz)
                        i++;
                    num--;
                }

            }
            if (num==1)
            {
                while((i<j) && (s[i]!=raz))
                {
                    pomgetst[cnt++]=s[i];
                    i++;
                }

            }
        }
    }
    pomgetst[cnt]=0;
    return pomgetst;
}


char *trim (char *str)
{
    char *ibuf, *obuf;

    if (str)
    {
        for (ibuf = obuf = str; *ibuf; )
        {
            while (*ibuf && (isspace (*ibuf)))
                ibuf++;
            if (*ibuf && (obuf != str))
                *(obuf++) = ' ';
            while (*ibuf && (!isspace (*ibuf)))
                *(obuf++) = *(ibuf++);
        }
        *obuf = 0;
    }
    return (str);
}


char cap_buf[MAX_STRING_LENGTH];
char *CAP(char *txt)
{
    //char c=*txt;
    //c=UPPER(c);

    *txt = UPPER(*txt);

    //strcpy(cap_buf, txt);
    return (txt);
}



int is_abbrev_multi(char *arg, char *help)
{
    int index = 0, ok;
    char *temp, *temp2;
    char first[256], first2[256];


    if (is_abbrev(arg, help))
        return 1   ;

    ok = 1;
    temp = any_one_arg(help, first);
    temp2 = any_one_arg(arg, first2);
    while (*first && *first2 && ok) {
        if (!is_abbrev(first2, first))
            ok = 0;
        temp = any_one_arg(temp, first);
        temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
        return 1;

    return 0;

}


int find_deity_by_name(char *name)
{
	int i;
	
	for (i=0;i<=MAX_DEITY;i++)	
		if (isname(name,deity_list[i].name))
			return i;
	
	return -1;
}
	