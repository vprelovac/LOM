/* ************************************************************************
*   File: weather.c                                     Part of CircleMUD *
*  Usage: functions handling time and the weather                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <string.h>

#include "auction.h"
#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "fight.h"

extern struct time_info_data time_info;
extern struct char_data *character_list;        /* lightning */
extern struct room_data *world;

void            weather_and_time(int mode);
void            another_hour(int mode);
void            weather_change(void);

int last_struck=-1;
void            beware_lightning(void)
{
    int             dam = 0;
    struct char_data *victim = NULL,
                                           *temp = NULL;
    char            buf[256];
    struct descriptor_data *i;
    int d,j=0;



    if (!(weather_info.sky == SKY_LIGHTNING))
        return;                 /* go away if its not even stormy!   */

    if (number(0, 7))
        return;                 /* Bolt only 15% of time  */

if (number(1, 100) > 60) {  /* nobody targeted 60% */
        send_to_outdoor("You hear the clap of distant thunder.\r\n");
        return;
    }


    for (victim = character_list; victim; victim = victim->next, j++)
    {
        if (world[victim->in_room].zone==last_struck)
            if (!number(0, 20))
                break;
    }

    if (!victim)
    {
        d=number(0, j-1);
        j=0;
        for (victim = character_list; victim, j<d; victim = victim->next, j++);

        if (!OUTSIDE(victim) ||  (AFF2_FLAGGED(victim, AFF2_PROT_LIGHTNING)))
        {	if (!number(0, 2))
            last_struck=world[victim->in_room].zone;
            return;
        }


    }
    if (!OUTSIDE(victim) ||  (AFF2_FLAGGED(victim, AFF2_PROT_LIGHTNING)))
        return;




    dam = number(1, GET_MAX_HIT(victim));
    dam = MIN(dam, 1500);
    dam = MAX(dam, 150);
    if ((GET_LEVEL(victim) >= LVL_IMMORT) && !IS_NPC(victim))
        /* You can't damage an immortal! */
        dam = 0;
    GET_HIT(victim) -= dam;
    send_to_outdoor("*BOOM* You hear the clap of thunder.\r\n");
    act("&CKAZAK! A struck of lightning hits $n!  You hear a sick sizzle.&0\r\n",
        TRUE, victim, 0, 0, TO_ROOM);
    act("&WKAZAK! A struck of lightning hits you !!! OUUUUUCH!&0\r\n",
        FALSE, victim, 0, 0, TO_CHAR);

    if (dam > (GET_MAX_HIT(victim) /4))
        act("&RThat Really did HURT!&0", FALSE, victim, 0, 0, TO_CHAR);

    last_struck=world[victim->in_room].zone;
    for (i = descriptor_list; i; i = i->next)
        if (!i->connected  && i->character &&
                !PLR_FLAGGED(i->character, PLR_WRITING) &&
                !PLR_FLAGGED(i->character, PLR_EDITING) &&
                !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

            if (world[victim->in_room].zone == world[i->character->in_room].zone)
                if (victim->in_room != i->character->in_room)
                    send_to_char("\r\n&cKAZAK! A lightning bolt hits nearby!&0\r\n", i->character);
        }

    check_kill(victim, "thunderstorm");


    for (victim = character_list; victim; victim = victim->next, j++)
    {
        if (FOL_BOUGAR(victim) && (!number(0, 3)))
        {
            struct char_data *tch, *koga=NULL;
            int s=0;

            for (tch=world[victim->in_room].people;tch; tch=tch->next, s++)
                if (!FOL_BOUGAR(tch) && !number(0, s))
                    koga=tch;


            if (koga)
            {
                victim=koga;
                dam = number(1, GET_MAX_HIT(victim));
                dam = MIN(dam, 1500);
                dam = MAX(dam, 150);
                if ((GET_LEVEL(victim) >= LVL_IMMORT) && !IS_NPC(victim))
                    /* You can't damage an immortal! */
                    dam = 0;
                GET_HIT(victim) -= dam;

                act("&CKAZAK! A struck of lightning hits $n!  You hear a sick sizzle.&0\r\n",
                    TRUE, victim, 0, 0, TO_ROOM);
                act("&WKAZAK! A struck of lightning hits you !!! OUUUUUCH!&0\r\n",
                    FALSE, victim, 0, 0, TO_CHAR);

                if (dam > (GET_MAX_HIT(victim) /4))
                    act("&RThat Really did HURT!&0", FALSE, victim, 0, 0, TO_CHAR);

                last_struck=world[victim->in_room].zone;
                for (i = descriptor_list; i; i = i->next)
                    if (!i->connected  && i->character &&
                            !PLR_FLAGGED(i->character, PLR_WRITING) &&
                            !PLR_FLAGGED(i->character, PLR_EDITING) &&
                            !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

                        if (world[victim->in_room].zone == world[i->character->in_room].zone)
                            if (victim->in_room != i->character->in_room)
                                send_to_char("\r\n&cKAZAK! Another lightning bolt hits nearby!&0\r\n", i->character);
                    }

                check_kill(victim, "thunderstorm");
            }


        }
    }
}

void            weather_and_time(int mode)
{
    another_hour(mode);
    if (mode)
        weather_change();
}


void            another_hour(int mode)
{
    time_info.hours++;

    if (mode) {
        switch (time_info.hours) {
        case 5:
            weather_info.sunlight = SUN_RISE;
            send_to_outdoor("&yThe sun rises in the east.&0\r\n");
            break;
        case 6:
            weather_info.sunlight = SUN_LIGHT;
            send_to_outdoor("&YThe day has begun.&0\r\n");
            break;
        case 20:
            weather_info.sunlight = SUN_SET;
            send_to_outdoor("&pThe sun slowly disappears in the west.&0\r\n");
            break;
        case 21:
            weather_info.sunlight = SUN_DARK;
            send_to_outdoor("&PThe night has begun.&0\r\n");
            break;
        default:
            break;
        }
    }
    if (time_info.hours > 23) { /* Changed by HHS due to bug ??? */
        time_info.hours -= 24;
        time_info.day++;

        if (time_info.day > 34) {
            time_info.day = 0;
            time_info.month++;

            if (time_info.month > 16) {
                time_info.month = 0;
                time_info.year++;
            }
        }
    }
}


void            weather_change(void)
{
    int             diff,
    change;
    if ((time_info.month >= 9) && (time_info.month <= 16))
        diff = (weather_info.pressure > 985 ? -2 : 2);
    else
        diff = (weather_info.pressure > 1015 ? -2 : 2);

    weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

    weather_info.change = MIN(weather_info.change, 12);
    weather_info.change = MAX(weather_info.change, -12);

    weather_info.pressure += weather_info.change;

    weather_info.pressure = MIN(weather_info.pressure, 1040);
    weather_info.pressure = MAX(weather_info.pressure, 960);

    change = 0;

    switch (weather_info.sky) {
    case SKY_CLOUDLESS:
        if (weather_info.pressure < 990)
            change = 1;
        else if (weather_info.pressure < 1010)
            if (dice(1, 4) == 1)
                change = 1;
        break;
    case SKY_CLOUDY:
        if (weather_info.pressure < 970)
            change = 2;
        else if (weather_info.pressure < 990)
            if (dice(1, 4) == 1)
                change = 2;
            else
                change = 0;
        else if (weather_info.pressure > 1030)
            if (dice(1, 4) == 1)
                change = 3;

        break;
    case SKY_RAINING:
        if (weather_info.pressure < 970)
            if (dice(1, 4) == 1)
                change = 4;
            else
                change = 0;
        else if (weather_info.pressure > 1030)
            change = 5;
        else if (weather_info.pressure > 1010)
            if (dice(1, 4) == 1)
                change = 5;

        break;
    case SKY_LIGHTNING:
        if (weather_info.pressure > 1010)
            change = 6;
        else if (weather_info.pressure > 990)
            if (dice(1, 4) == 1)
                change = 6;

        break;
    default:
        change = 0;
        weather_info.sky = SKY_CLOUDLESS;
        break;
    }

    switch (change) {
    case 0:
        break;
    case 1:
        send_to_outdoor("&bThe sky starts to get cloudy.&0\r\n");
        weather_info.sky = SKY_CLOUDY;
        break;
    case 2:
        send_to_outdoor("&BIt starts to rain.&0\r\n");
        weather_info.sky = SKY_RAINING;
        break;
    case 3:
        send_to_outdoor("&wThe clouds disappear.&0\r\n");
        weather_info.sky = SKY_CLOUDLESS;
        break;
    case 4:
        send_to_outdoor("&WLightning starts to show in the sky.&0\r\n");
        weather_info.sky = SKY_LIGHTNING;
        break;
    case 5:
        send_to_outdoor("&cThe rain stops.&0\r\n");
        weather_info.sky = SKY_CLOUDY;
        break;
    case 6:
        send_to_outdoor("&cThe lightning stops.&0\r\n");
        weather_info.sky = SKY_RAINING;
        last_struck=-1;
        break;
    default:
        break;
    }
}
