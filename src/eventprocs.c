#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


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



struct test_event_obj
{
    struct char_data *ch, *victim;
};


EVENTFUNC(test_event)
{
    struct test_event_obj *seo = (struct test_event_obj *) event_obj;
    struct char_data *ch, *victim;

    ch = seo->ch;
    victim = seo->victim;


    send_to_char("I am OK boss!\r\n", ch);
    return 5;
}

ACMD(do_bigboss)
{
    struct char_data *vict;
    struct test_event_obj *sniff;
    CREATE(sniff, struct test_event_obj, 1);

    one_argument(argument, arg);

    if (!*arg)
        return;
    else
        if (!(vict = get_char_room_vis(ch, arg))) {
            send_to_char("Noone around by that name.\r\n", ch);
            return;
        }
    act("$n claps $s legs together and reports to you.", FALSE, vict, 0, ch, TO_VICT);
    sniff->ch=ch;
    sniff->victim=vict;

    GET_UTIL_EVENT(ch)=event_create(test_event, sniff, 5);
    GET_UTIL_EVENT(vict)=GET_UTIL_EVENT(ch);
}



/* the event object for the sniff event */
struct sniff_event_obj {
    struct char_data *ch;
    byte severity;
};


EVENTFUNC(sniff_event)
{
    struct sniff_event_obj *seo = (struct sniff_event_obj *) event_obj;
    struct char_data *ch, *victim;

    ch = seo->ch;

    GET_UTIL_EVENT(ch) = NULL;


    act("$n sniffs loudly.", FALSE, ch, NULL, NULL, TO_ROOM);

    act("You sniff.", FALSE, ch, NULL, NULL, TO_CHAR);

    if (--seo->severity <= 0) {
        /* we're done with sniffing */
        DISPOSE(event_obj);
        return 0;
    }
    else
        return 8;
}


ACMD(do_bigboss1)
{
    struct sniff_event_obj *sniff;

    CREATE(sniff, struct sniff_event_obj, 1);
    sniff->ch = ch;
    sniff->severity = 5;

    GET_UTIL_EVENT(ch) = event_create(sniff_event, sniff, 8);

    send_to_char(OK, ch);
}

