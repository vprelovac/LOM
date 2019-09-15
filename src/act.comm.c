/* ************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "chatlink.h"
#include "clan.h"
/* extern variables */

extern int clan_loadroom[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern void mprog_speech_trigger(char *txt, struct char_data * mob);
char *makedrunk(char *string, struct char_data * ch);

ACMD(do_private_channel)
{
    struct char_data *vict;
    struct descriptor_data *i;

    half_chop(argument, buf, buf2);

    if (subcmd == PRIVATE_HELP) {
        send_to_char("Private Channel (PC) commands\r\n", ch);
        send_to_char("------------------------\r\n", ch);
        send_to_char("popen   - opens your own private channel.\r\n", ch);
        send_to_char("padd    - adds a player to your PC.\r\n", ch);
        send_to_char("premove - remove a player from your PC.\r\n", ch);
        send_to_char("pclose  - closes your private channel.\r\n", ch);
        send_to_char("pwho    - lists all members on the current PC.\r\n", ch);
        send_to_char("poff    - exits you from your current PC.\r\n\r\n", ch);
        send_to_char("  NOTE: If you don't want to be added to another\r\n", ch);
        send_to_char("        player's PC open your own with no players.\r\n", ch);
        send_to_char("\r\nTo talk on the channel use 'private' or '/'\r\n",ch);
    } else if (subcmd == PRIVATE_OPEN) {
        GET_PRIVATE(ch) = GET_IDNUM(ch);
        send_to_char("You have just opened your own Private Channel.\r\n", ch);
    } else if (subcmd == PRIVATE_OFF) {
        GET_PRIVATE(ch) = 0;
        send_to_char("You have just quit any Private Channels.\r\n", ch);
    } else if (subcmd == PRIVATE_CLOSE) {
        GET_PRIVATE(ch) = 0;
        /* now remove all people on the private channel */
        for (i = descriptor_list; i; i = i->next)
            if (!i->connected)
                if ((GET_PRIVATE(i->character) == GET_IDNUM(ch)) &&
                        (ch != i->character)) {
                    GET_PRIVATE(i->character) = 0;
                    sprintf(buf, "%s has just closed their Private Channel.\r\n",
                            GET_NAME(ch));
                    send_to_char(buf, i->character);
                }
        send_to_char("You have just CLOSED your Private Channel.\r\n", ch);
    } else if (subcmd == PRIVATE_WHO) {
        if (GET_PRIVATE(ch) == 0)
            send_to_char("You are not on a private channel\r\n",ch);
        else {
            /* show all people on the private channel */
            send_to_char("Private Channel Members\r\n", ch);
            send_to_char("-----------------------\r\n", ch);
            for (i = descriptor_list; i; i = i->next)
                if (!i->connected)
                    if (GET_PRIVATE(i->character) == GET_PRIVATE(ch)) {
                        sprintf(buf, "%s\r\n", GET_NAME(i->character));
                        send_to_char(buf, ch);
                    }
        }
    } else if (subcmd == PRIVATE_CHECK) {
        /* show all people on the ALL private channels */
        send_to_char("Private Channels\r\n", ch);
        send_to_char("---------------------------------------------\r\n", ch);
        for (i = descriptor_list; i; i = i->next)
            if (!i->connected) {
                sprintf(buf, "[%-5d]  %s\r\n",
                        GET_PRIVATE(i->character), GET_NAME(i->character));
                send_to_char(buf, ch);
            }
    } else if (subcmd == PRIVATE_REMOVE) {
        if (!*buf)
            send_to_char("Who do you wish to add to you private channel?\r\n", ch);
        else if (!(vict = get_char_vis(ch, buf)))
            send_to_char(NOPERSON, ch);
        else if (IS_NPC(vict))
            send_to_char("NPC's cannot be on private channels\r\n", ch);
        else if (GET_PRIVATE(vict) != GET_IDNUM(ch)) {
            sprintf(buf,"%s is NOT on your Private Channel!\r\n",
                    GET_NAME(vict));
            send_to_char(buf, ch);
        } else {
            GET_PRIVATE(vict) = 0;
            sprintf(buf,"You have been REMOVED from %s's Private Channel!\r\n",
                    GET_NAME(ch));
            send_to_char(buf, vict);
            sprintf(buf,"%s has been REMOVED from your Private Channel!\r\n",
                    GET_NAME(vict));
            send_to_char(buf, ch);
        }
    } else if (subcmd == PRIVATE_ADD) {
        if (GET_PRIVATE(ch) != GET_IDNUM(ch))
            send_to_char("You must open your own private channel first\r\n",ch);
        else if (!*buf)
            send_to_char("Who do you wish to add to you private channel?\r\n", ch);
        else if (!(vict = get_char_vis(ch, buf)))
            send_to_char(NOPERSON, ch);
        else if (ch == vict)
            GET_PRIVATE(ch) = GET_IDNUM(ch);
        else if (IS_NPC(vict))
            send_to_char("NPC's cannot be added to private channels\r\n", ch);
        else if (GET_PRIVATE(vict) != 0) {
            sprintf(buf,"%s is already on another private channel!\r\n",
                    GET_NAME(vict));
            send_to_char(buf, ch);
        } else {
            GET_PRIVATE(vict) = GET_IDNUM(ch);
            sprintf(buf,"You have been ADDED to %s's Private Channel!\r\n",
                    GET_NAME(ch));
            send_to_char(buf, vict);
            sprintf(buf,"%s has been ADDED to your Private Channel!\r\n",
                    GET_NAME(vict));
            send_to_char(buf, ch);
        }
    }
}


ACMD(do_say)
{
    skip_spaces(&argument);

    if (!*argument)
        send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
    else {
        argument = makedrunk(argument, ch);
        if (!IS_NPC(ch))
            CAP(argument);

        else if (!(world[ch->in_room].people->next_in_room))
            return;

        sprintf(buf, "$n says, '&c%s&0'", argument);
        MOBTrigger = FALSE;
        //	if (!IS_NPC(ch) || MOB3_FLAGGED(ch, MOB3_CANTALK))

        if (IS_SUPERMOB(ch)) super_silent=0;
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        super_silent=1;
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else {
            sprintf(buf, "You say, '&c%s&0'", argument);
            act(buf, FALSE, ch, 0, argument, TO_CHAR);
        }
        chatperformtoroom(ch, argument);
        mprog_speech_trigger(argument, ch);
        oprog_speech_trigger(argument, ch);
        rprog_speech_trigger(argument, ch);

    }
}

ACMD(do_gsay)
{
    struct char_data *k;
    struct follow_type *f;

    skip_spaces(&argument);

    if (!IS_AFFECTED(ch, AFF_GROUP)) {
        send_to_char("But you are not member of any group!\r\n", ch);
        return;
    }
    if (!*argument)
        send_to_char("Yes, but WHAT do you want to tell your group?\r\n", ch);
    else {
        if (ch->master)
            k = ch->master;
        else
            k = ch;

        argument = makedrunk(argument, ch);
        CAP(argument);
        sprintf(buf, "$n tells the group, '&C%s&0'", argument);

        if (IS_AFFECTED(k, AFF_GROUP) && (k != ch))
            act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
        for (f = k->followers; f; f = f->next)
            if (IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower != ch))
                act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);

        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else {
            sprintf(buf, "You tell the group, '&G%s&0'", argument);
            act(buf, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
        }
    }
}


void perform_tell(struct char_data * ch, struct char_data * vict, char *arg)
{
    //    send_to_char(CCRED(vict, C_NRM), vict);
    int i;
    CAP(arg);
    MOBTrigger = FALSE;


    sprintf(buf, "$n tells you, '&C%s&0'", arg);
    act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
    MOBTrigger = TRUE;

    if (!IS_NPC(vict))
    {
        if (vict->last_tells[MAX_TELLS-1])
            DISPOSE(vict->last_tells[MAX_TELLS-1]);
        for (i=MAX_TELLS-1; i>0; i--)
            vict->last_tells[i]=vict->last_tells[i-1];
        sprintf(buf, "%s tells you, '&c%s&0'\r\n", GET_NAME(ch), arg);
        vict->last_tells[0]=str_dup(buf);
    }



    //  send_to_char(CCNRM(vict, C_NRM), vict);

    if (vict->char_specials.timer>3)
    {
        sprintf(buf, "+++ $N has been idle for over a minute +++\r\n");
        act(buf, FALSE, ch, 0, vict, TO_CHAR);
    }
    if (FIGHTING(vict))
    {
        sprintf(buf, "+++ $N is in battle +++\r\n");
        act(buf, FALSE, ch, 0, vict, TO_CHAR);
    }

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
    else {
        //	send_to_char(CCRED(ch, C_CMP), ch);

        sprintf(buf, "You tell $N, '&C%s&0'", arg);
        act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
        //	send_to_char(CCNRM(ch, C_CMP), ch);
    }

    GET_LAST_TELL(vict) = GET_IDNUM(ch);
    chatperform(vict, ch, arg);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
    struct char_data *vict;

    half_chop(argument, buf, buf2);

    if (!*buf || !*buf2)
        send_to_char("Who do you wish to tell what??\r\n", ch);
    else if (!(vict = get_char_vis(ch, buf)) || (IS_NPC(vict) && vict->in_room != ch->in_room))
        send_to_char(NOPERSON, ch);
    else if (ch == vict)
        send_to_char("You try to tell yourself something.\r\n", ch);
    else if (PRF_FLAGGED(ch, PRF_NOTELL))
        send_to_char("You can't tell other people while you have notell on.\r\n", ch);
    else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
        send_to_char("The walls seem to absorb your words.\r\n", ch);
    else if (!IS_NPC(vict) && !vict->desc)	/* linkless */
        act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    else if (PLR_FLAGGED(vict, PLR_WRITING))
        act("$E's writing a message right now, try again later.",
            FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    else if (PLR_FLAGGED(vict, PLR_EDITING))
        act("$E's editing the database right now, try again later.",
            FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    else if (PRF_FLAGGED(vict, PRF_NOTELL) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF))
        act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    else if (PRF2_FLAGGED(vict, PRF2_AFK))
        act("$E is away from $S keyboard right now.  Try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    else {
        argument = makedrunk(buf2, ch);
        perform_tell(ch, vict, argument);
    }
}

ACMD(do_sayto)
{
    struct char_data *vict = NULL;

    half_chop(argument, buf, buf2);

    if (!*buf || !*buf2)
    {
        send_to_char("Who do you wish to say what??\r\n", ch);
        return;
    }
    else if (!(vict = get_char_vis(ch, buf)) || (vict->in_room != ch->in_room))
        send_to_char(NOPERSON, ch);
    if (!IS_NPC(ch))
        CAP(buf2);
    //send_to_char(CCRED(vict, C_NRM), vict);
    sprintf(buf, "$n says to you, '&c%s&0'", buf2);
    if (IS_SUPERMOB(ch)) super_silent=0;
    act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
    super_silent=1;
    //send_to_char(CCNRM(vict, C_NRM), vict);


    //send_to_char(CCRED(ch, C_CMP), ch);
    sprintf(buf, "You say to $N, '&c%s&0'", buf2);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    //send_to_char(CCNRM(ch, C_CMP), ch);

}
ACMD(do_ctell)
{
    struct descriptor_data *i;
    int minlev = 1, c = 0;
    char level_string[6] = "\0\0\0\0\0\0";

    skip_spaces(&argument);

    /*
     * The syntax of ctell for imms is different then for morts
     * mort: ctell <bla bla bla>    imms: ctell <clan_num> <bla bla bla>
     * Imms cannot actually see ctells but they can send them
     */
    if (GET_LEVEL(ch) >= LVL_IMMORT) {
        c = atoi(argument);
        if ((c < 0) || (c >= num_of_clans)) {
            send_to_char("There is no clan with that number.\r\n", ch);
            return;
        }
        while ((*argument != ' ') && (*argument != '\0'))
            argument++;
        while (*argument == ' ')
            argument++;
    } else if ((c = GET_CLAN(ch)) == CLAN_OUTLAW) {
        send_to_char("You can not use this as an Outlaw.\r\n", ch);
        return;
    }
    skip_spaces(&argument);

    if (!*argument) {
        send_to_char("What do you want to clan-tell?\r\n", ch);
        return;
    }
    if (*argument == '#') {
        argument++;
        minlev = atoi(argument);
        if (minlev > clan[c].ranks) {
            send_to_char("No one has a clan rank high enough to hear you!\r\n", ch);
            return;
        }
        while (*argument != ' ')
            argument++;
        while (*argument == ' ')
            argument++;
        sprintf(level_string, "(%d)", minlev);
    }
    CAP(argument);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        sprintf(buf1,"%s",OK);
    else
        sprintf(buf1, "You clan-tell%s, '&G%s&0'\r\n", level_string, argument);
    send_to_char(buf1, ch);

    for (i = descriptor_list; i; i = i->next) {
        if (i->character && i->character->player_specials->saved.clan == c) {
            if (i->character->player_specials->saved.clan_rank >= minlev) {
                if (strcmp(i->character->player.name, ch->player.name)) {
                    sprintf(buf, "%s clan-tells%s, '&C%s&0'\r\n",
                            (INVIS_OK(i->character, ch) ?
                             "Someone" : ch->player.name), level_string, argument);
                    send_to_char(buf, i->character);
                }
            }
        }
    }

    return;
}



ACMD(do_reply)
{
    struct char_data *tch = character_list, *vict;

    skip_spaces(&argument);

    if (GET_LAST_TELL(ch) == NOBODY)
        send_to_char("You have no-one to reply to!\r\n", ch);
    else if (!*argument)
        send_to_char("What do you want to reply?\r\n", ch);
    else {
        /* Make sure the person you're replying to is still playing by
           searching for them.  Note, now last tell is stored as player
           IDnum instead of a pointer, which is much better because it's
           safer, plus will still work if someone logs out and back in
           again. */

        while (tch != NULL && (GET_IDNUM(tch) != GET_LAST_TELL(ch)))
            tch = tch->next;
        if (tch==NULL)
        {
            tch = character_list;
            while (tch != NULL && (IS_NPC(tch) && GET_LAST_TELL(ch)!=-GET_MOB_RNUM(tch)))
                tch = tch->next;
        }
        if (tch == NULL)
            send_to_char("They are no longer playing.\r\n", ch);
        else {
            vict=tch;
            if (!IS_NPC(vict) && !vict->desc)	/* linkless */
                act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
            else if (PLR_FLAGGED(vict, PLR_WRITING))
                act("$E's writing a message right now, try again later.",
                    FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
            else if (PLR_FLAGGED(vict, PLR_EDITING))
                act("$E's editing the database right now, try again later.",
                    FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
            else if (PRF_FLAGGED(vict, PRF_NOTELL) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF))
                act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
            else if (PRF2_FLAGGED(vict, PRF2_AFK))
                act("$E is away from $S keyboard right now.  Try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
            else
            {
                argument = makedrunk(argument, ch);
                perform_tell(ch, tch, argument);
            }
        }
    }
}

ACMD(do_spec_comm)
{
    struct char_data *vict;
    char *action_sing, *action_plur, *action_others;

    if (subcmd == SCMD_WHISPER) {
        action_sing = "whisper to";
        action_plur = "whispers to";
        action_others = "$n whispers something to $N.";
    } else {
        action_sing = "ask";
        action_plur = "asks";
        action_others = "$n asks $N a question.";
    }

    half_chop(argument, buf, buf2);

    if (!*buf || !*buf2) {
        sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
        send_to_char(buf, ch);
    } else if (!(vict = get_char_room_vis(ch, buf)))
        send_to_char(NOPERSON, ch);
    else if (vict == ch)
        send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
    else {
        argument = makedrunk(buf2, ch);
        sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
        act(buf, FALSE, ch, 0, vict, TO_VICT);
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else {
            sprintf(buf, "You %s %s, '&c%s&0'", action_sing, GET_NAME(vict), CAP(buf2));
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }
        act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
    }
}



#define MAX_NOTE_LENGTH 1000	/* arbitrary */

ACMD(do_write)
{
    struct obj_data *paper = 0, *pen = 0;
    char *papername, *penname;

    papername = buf1;
    penname = buf2;

    two_arguments(argument, papername, penname);

    if (!ch->desc)
        return;

    if (!*papername) {		/* nothing was delivered */
        send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
        return;
    }
    if (*penname) {		/* there were two arguments */
        if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
            sprintf(buf, "You have no %s.\r\n", papername);
            send_to_char(buf, ch);
            return;
        }
        if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
            sprintf(buf, "You have no %s.\r\n", penname);
            send_to_char(buf, ch);
            return;
        }
    } else {			/* there was one arg.. let's see what we
           can find */
        if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
            sprintf(buf, "There is no %s in your inventory.\r\n", papername);
            send_to_char(buf, ch);
            return;
        }
        if (GET_OBJ_TYPE(paper) == ITEM_PEN) {	/* oops, a pen.. */
            pen = paper;
            paper = 0;
        } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
            send_to_char("That thing has nothing to do with writing.\r\n", ch);
            return;
        }
        /* One object was found.. now for the other one. */
        if (!GET_EQ(ch, WEAR_HOLD)) {
            sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
                    papername);
            send_to_char(buf, ch);
            return;
        }
        if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
            send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
            return;
        }
        if (pen)
            paper = GET_EQ(ch, WEAR_HOLD);
        else
            pen = GET_EQ(ch, WEAR_HOLD);
    }


    /* ok.. now let's see what kind of stuff we've found */
    if (GET_OBJ_TYPE(pen) != ITEM_PEN)
        act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
    else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
        act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    else if (paper->action_description)
        send_to_char("There's something written on it already.\r\n", ch);
    else {
        /* we can write - hooray! */
        /* this is the PERFECT code example of how to set up: a) the text
           editor with a message already loaed b) the abort buffer if the
           player aborts the message */
        ch->desc->backstr = NULL;
        send_to_char("Write your note.  (/s saves /h for help)\r\n", ch);
        /* ok, here we check for a message ALREADY on the paper */
        if (paper->action_description) {
            /* we str_dup the original text to the descriptors->backstr */
            ch->desc->backstr = str_dup(paper->action_description);
            /* send to the player what was on the paper (cause this is
               already */
            /* loaded into the editor) */
            send_to_char(paper->action_description, ch);
        }
        act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
        /* assign the descriptor's->str the value of the pointer to the
           text */
        /* pointer so that we can reallocate as needed (hopefully that
           made */
        /* sense :>) */
        ch->desc->str = &paper->action_description;
        ch->desc->max_str = MAX_NOTE_LENGTH;
    }
}



ACMD(do_page)
{
    struct descriptor_data *d;
    struct char_data *vict;

    half_chop(argument, arg, buf2);

    if (IS_NPC(ch))
        send_to_char("Monsters can't page.. go away.\r\n", ch);
    else if (!*arg)
        send_to_char("Whom do you wish to page?\r\n", ch);
    else {
        sprintf(buf, "\007\007*%s pages: &W%s&0 *\r\n", GET_NAME(ch), buf2);
        if (!str_cmp(arg, "all")) {
            if (GET_LEVEL(ch) > LVL_GOD) {
                for (d = descriptor_list; d; d = d->next)
                    if (!d->connected && d->character)
                        act(buf, FALSE, ch, 0, d->character, TO_VICT);
            } else
                send_to_char("You will never be godly enough to do that!\r\n", ch);
            return;
        }
        if ((vict = get_char_vis(ch, arg)) != NULL) {
            if (PRF2_FLAGGED(vict, PRF2_AFK) && (GET_LEVEL(ch) < LVL_IMPL))
                act("Hmmm... $E's away from $S keyboard right now.... Try again later.",
                    FALSE, ch, 0, vict, TO_CHAR);
            else
                act(buf, FALSE, ch, 0, vict, TO_VICT);
            if (PRF_FLAGGED(ch, PRF_NOREPEAT))
                send_to_char(OK, ch);
            else
                act(buf, FALSE, ch, 0, vict, TO_CHAR);
            return;
        } else
            send_to_char("There is no such person in the game!\r\n", ch);
    }
}


ACMD(do_gemote);
/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
    extern int level_can_shout;
    extern int holler_move_cost;
    struct descriptor_data *i;
    char color_on[24];
    byte gmote=0;

    /* Array of flags which must _not_ be set in order for comm to be
       heard */
    static int channels[] = {
                                0,
                                PRF_DEAF,
                                PRF_NOGOSS,
                                PRF_NOAUCT,
                                PRF_NOGRATZ,
                                PRF_NOCHAT,
                                PRF_NOWAR,
                                PRF_NOTEAM,
                                PRF_NOCLAN,
                                0
                            };

    /* com_msgs: [0] Message if you can't perform the action because of
       noshout [1] name of the action [2] message if you're not on the
       channel [3] a color string. */
    static char *com_msgs[][4] = {
                                     {"You cannot holler!!\r\n",
                                      "holler",
                                      "",
                                      KYEL},

                                     {"You cannot shout!!\r\n",
                                      "shout",
                                      "Turn off your noshout flag first!\r\n",
                                      KYEL},

                                     {"You cannot gossip!!\r\n",
                                      "gossip",
                                      "You must have gossip channel turned on first!\r\n",
                                      KYEL},

                                     {"You cannot auction!!\r\n",
                                      "auction",
                                      "You must have auction channel turned on first!\r\n",
                                      KMAG},

                                     {"You cannot congratulate!\r\n",
                                      "congrat",
                                      "You must have grats channel turned on first!\r\n",
                                      KGRN},

                                     {"You cannot speak on ooc channel!\r\n",
                                      "ooc",
                                      "You aren't even on ooc channel!\r\n",
                                      "&P"},

                                     {"You cannot speak on ooc channel.\r\n",
                                      "ooc",
                                      "You aren't even on the ooc channel\r\n",
                                      KMAG},

                                     {"You cannot broadcast on the arena channel.\r\n",
                                      "broadcast",
                                      "You aren't even on the arena channel.\r\n",
                                      KBLU},

                                     {"You cannot broadcast on any clan channel.\r\n",
                                      "clan-say",
                                      "You aren't even on a clan channel.\r\n",
                                      KGRN},

                                     {"You cannot talk on the Private Channel!\r\n",
                                      "private-say",
                                      "You aren't even on the channel!\r\n",
                                      KYEL}
                                 };


    /* to keep pets, etc from being ordered to shout */
    if (!ch->desc && IS_NPC(ch) && AFF_FLAGGED(ch, AFF_GROUP))
        return;

    if (DEAD(ch))
        return;
    if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
        send_to_char(com_msgs[subcmd][0], ch);
        return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) {
        send_to_char("The walls seem to absorb your words.\r\n", ch);
        return;
    }
    /* level_can_shout defined in config.c */
    if (GET_LEVEL(ch) < level_can_shout) {
        sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
                level_can_shout, com_msgs[subcmd][1]);
        send_to_char(buf1, ch);
        return;
    }
    /* make sure the char is on the channel */
    if (PRF_FLAGGED(ch, channels[subcmd])) {
        send_to_char(com_msgs[subcmd][2], ch);
        return;
    }

    if (subcmd == SCMD_PRIVATE && GET_PRIVATE(ch) == 0) {
        send_to_char("You are not on a private channel!", ch);
        return;
    }

    /* skip leading spaces */
    skip_spaces(&argument);


    if (subcmd == SCMD_GOSSIP && (*argument == '@' || *argument=='#')) {
        subcmd = SCMD_GOSSIP;
        if (*argument=='@')
            gmote = 1;
        else
            gmote = 2;
        argument+=1;
    }

    /* make sure that there is something there to say! */
    if (!*argument) {
        sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT?\r\n",
                com_msgs[subcmd][1], com_msgs[subcmd][1]);
        send_to_char(buf1, ch);
        return;
    }


    if (gmote) {
        if (gmote==1)
            do_gemote(ch, argument, 0, 0);
        else
            do_gemote(ch, argument, 0, 1);
        return;
    }

    if (subcmd == SCMD_HOLLER) {
        if (GET_MOVE(ch) < MAX(2, GET_MAX_HIT(ch)/10)) {
            send_to_char("You're too exhausted to holler.\r\n", ch);
            return;
        } else
            GET_MOVE(ch) -= MAX(2, GET_MAX_HIT(ch)/10);
    }
    if (subcmd == SCMD_BROADCAST && !(RED(ch) || BLUE(ch)))
    {
        send_to_char("You're not on one of the teams at the arena.\r\n", ch);
        return;
    }
    if (subcmd == SCMD_CLAN && GET_CLAN(ch) <= 0) {
        send_to_char("You're not in a clan.\r\n", ch);
        return;
    }
    argument = makedrunk(argument, ch);
    CAP(argument);
    /* set up the color on code */
    strcpy(color_on, com_msgs[subcmd][3]);

    /* first, set up strings to be given to the communicator */
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
    else {
        if (subcmd == SCMD_CHAT)
            sprintf(buf1, "&P[OOC] You : &c'%s'&0", argument);
        else   if (subcmd != SCMD_BROADCAST)
            sprintf(buf1, "You %s, &c'%s'&0", com_msgs[subcmd][1], argument);
        else
            sprintf(buf1, "You tell your team, &c'%s'&0", argument);

        act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
    }
    if (subcmd == SCMD_CHAT)
        sprintf(buf, "[OOC] $n : &c'%s'&0", argument);
    else if (subcmd != SCMD_BROADCAST)
        sprintf(buf, "$n %ss, &c'%s'&0", com_msgs[subcmd][1], argument);
    else
        sprintf(buf, "$n tells the team, &c'%s'&0",  argument);

    /* now send all the strings out */
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected && i != ch->desc && i->character &&
                !PRF_FLAGGED(i->character, channels[subcmd]) &&
                !PLR_FLAGGED(i->character, PLR_WRITING) &&
                !PLR_FLAGGED(i->character, PLR_EDITING) &&
                !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

            if (subcmd == SCMD_SHOUT &&
                    ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
                     GET_POS(i->character) < POS_RESTING))
                continue;
            //if (subcmd == SCMD_GOSSIP && ((ENEMIES(ch, i->character)) &&
            //!IS_IMMORT(i->character)))
            //	continue;
            if (subcmd == SCMD_BROADCAST && !((RED(ch) &&
                                               RED(i->character)) ||
                                              (BLUE(ch) &&
                                               BLUE(i->character))))
                continue;
            if (subcmd == SCMD_CLAN && GET_CLAN(ch) != GET_CLAN(i->character))
                continue;


            if (subcmd == SCMD_PRIVATE && (GET_PRIVATE(ch) != GET_PRIVATE(i->character)))
                continue;

            if (COLOR_LEV(i->character) >= C_NRM)
                send_to_char(color_on, i->character);
            act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
            if (COLOR_LEV(i->character) >= C_NRM)
                send_to_char(KNRM, i->character);
        }
    }
}


ACMD(do_qcomm)
{
    struct descriptor_data *i;

    if (!PRF_FLAGGED(ch, PRF_QUEST)) {
        send_to_char("You aren't even part of the quest!\r\n", ch);
        return;
    }
    skip_spaces(&argument);

    if (!*argument) {
        sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
                CMD_NAME);
        CAP(buf);
        send_to_char(buf, ch);
    } else {
        argument = makedrunk(argument, ch);
        CAP(argument);
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else {
            if (subcmd == SCMD_QSAY)
                sprintf(buf, "You quest-say, '&p%s&0'", argument);
            else
                strcpy(buf, argument);
            act(buf, FALSE, ch, 0, argument, TO_CHAR);
        }

        if (subcmd == SCMD_QSAY)
            sprintf(buf, "$n quest-says, '&P%s&0'", argument);
        else
            strcpy(buf, argument);

        for (i = descriptor_list; i; i = i->next)
            if (!i->connected && i != ch->desc &&
                    PRF_FLAGGED(i->character, PRF_QUEST))
                act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
    }
}

void chatperform(struct char_data * ch, struct char_data * victim, char *msg)
{
    char *reply, *s, *t;
    extern ACMD(do_echo);

    return;    // uncomment to disable eliza chatter


    if (!IS_NPC(ch) || !MOB_FLAGGED(ch, MOB_TALK) || (victim != NULL && IS_NPC(victim)))
        return;			/* ignores ch who are PCs and victims who
    		   are NPCs */
    /*if (!MOB_FLAGGED(ch, MOB_TALK))
{
    	act("$n ignores you.",FALSE,ch,NULL,victim,TO_VICT);
    	return;
}*/
    //    reply = dochat(GET_NAME(ch), msg, victim ? GET_NAME(victim) : "you");

    if (reply) {
        switch (reply[0]) {
        case '\0':		/* null msg */
            break;
        case '"':		/* do say */
            do_say(ch, reply + 1, 0, 0);
            break;
        case ':':		/* do emote */
            do_echo(ch, reply + 1, 0, SCMD_EMOTE);
            break;
        case '!':		/* execute command thru interpreter */
            s = reply + 1;

            /* RK: changed to perform a list of commands seperated by ';' */

            while (*s != '\0') {
                t = strchr(s, ';');

                if (t == NULL) {
                    command_interpreter(ch, s);
                    break;
                } else {
                    *t = '\0';
                    command_interpreter(ch, s);
                    s = t + 1;
                }
            }
            break;
        default:		/* is a say or tell */
            if (victim == NULL)
                do_say(ch, reply, 0, 0);
            else {		/* do a tell  (this is always the case
                  here) */
                char buf[MAX_STRING_LENGTH];
                sprintf(buf, "$N tells you, '&C%s&0'", CAP(reply));
                act(buf, FALSE, victim, NULL, ch, TO_CHAR);
                GET_LAST_TELL(victim) = -GET_MOB_RNUM(ch);
            }
        }
    }
}


/* ch here is the PC saying the phrase
 */
void chatperformtoroom(struct char_data * ch, char *txt)
{
    struct char_data *vch;

    if (IS_NPC(ch) || DEAD(ch))
        return;			/* we dont want NPCs trigger'ing events */

    for (vch = world[ch->in_room].people; vch != NULL; vch = vch->next_in_room) {
        /* RK: contained once a check for mobprogs see stuff in Original
           dir */
        if (IS_NPC(vch) && !(mob_index[vch->nr].progtypes & SPEECH_PROG) && AWAKE(vch))
            chatperform(vch, ch, txt);
    }
}
