/* ************************************************************************
*   File: modify.c                                      Part of CircleMUD *
*  Usage: Run-time modification of game variables                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "class.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "spells.h"
#include "mail.h"
#include "boards.h"
#include "olc.h"

void            show_string(struct descriptor_data * d, char *input);

/* action modes for parse_action */
#define PARSE_FORMAT		0
#define PARSE_REPLACE		1
#define PARSE_HELP		2
#define PARSE_DELETE		3
#define PARSE_INSERT		4
#define PARSE_LIST_NORM		5
#define PARSE_LIST_NUM		6
#define PARSE_EDIT		7

char           *string_fields[] =
    {
        "name",
        "short",
        "long",
        "description",
        "title",
        "delete-description",
        "\n"
    };


/* maximum length for text field x+1 */
int             length[] =
    {
        15,
        60,
        256,
        240,
        60
    };


/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */
/* handle some editor commands */
void            parse_action(int command, char *string, struct descriptor_data * d)
{
    int             indent = 0,
                             rep_all = 0,
                                       flags = 0,
                                               total_len,
                                               replaced;
    register int    j = 0;
    int             i,
    line_low,
    line_high;
    char           *s,
    *t,
    temp;

    switch (command) {
    case PARSE_HELP:
        sprintf(buf,
                "Editor command formats: /<letter>\r\r\n\n"
                "/a         -  aborts editor\r\n"
                "/c         -  clears buffer\r\n"
                "/d#        -  deletes a line #\r\n"
                "/e# <text> -  changes the line at # with <text>\r\n"
                "/f         -  formats text\r\n"
                "/fi        -  indented formatting of text\r\n"
                "/h         -  list text editor commands\r\n"
                "/i# <text> -  inserts <text> before line #\r\n"
                "/l         -  lists buffer\r\n"
                "/n         -  lists buffer with line numbers\r\n"
                "/r 'a' 'b' -  replace 1st occurance of text <a> in buffer with text <b>\r\n"
                "/ra 'a' 'b'-  replace all occurances of text <a> within buffer with text <b>\r\n"
                "              usage: /r[a] 'pattern' 'replacement'\r\n"
                "/s         -  saves text\r\n");
        SEND_TO_Q(buf, d);
        break;
    case PARSE_FORMAT:
        while (isalpha(string[j]) && j < 2) {
            switch (string[j]) {
            case 'i':
                if (!indent) {
                    indent = 1;
                    flags += FORMAT_INDENT;
                }
                break;
            default:
                break;
            }
            j++;
        }
        format_text(d->str, flags, d, d->max_str);
        sprintf(buf, "Text formatted with%s indent.\r\n", (indent ? "" : "out"));
        SEND_TO_Q(buf, d);
        break;
    case PARSE_REPLACE:
        while (isalpha(string[j]) && j < 2) {
            switch (string[j]) {
            case 'a':
                if (!indent)
                    rep_all = 1;

                break;
            default:
                break;
            }
            j++;
        }
        s = strtok(string, "'");
        if (s == NULL) {
            SEND_TO_Q("Invalid format.\r\n", d);
            return;
        }
        s = strtok(NULL, "'");
        if (s == NULL) {
            SEND_TO_Q("Target string must be enclosed in single quotes.\r\n", d);
            return;
        }
        t = strtok(NULL, "'");
        if (t == NULL) {
            SEND_TO_Q("No replacement string.\r\n", d);
            return;
        }
        t = strtok(NULL, "'");
        if (t == NULL) {
            SEND_TO_Q("Replacement string must be enclosed in single quotes.\r\n", d);
            return;
        }
        total_len = ((strlen(t) - strlen(s)) + strlen(*d->str));
        if (total_len <= d->max_str) {
            if ((replaced = replace_str(d->str, s, t, rep_all, d->max_str)) > 0) {
                sprintf(buf, "Replaced %d occurance%sof '%s' with '%s'.\r\n", replaced, ((replaced != 1) ? "s " : " "), s, t);
                SEND_TO_Q(buf, d);
            } else if (replaced == 0) {
                sprintf(buf, "String '%s' not found.\r\n", s);
                SEND_TO_Q(buf, d);
            } else {
                SEND_TO_Q("ERROR: Replacement string causes buffer overflow, aborted replace.\r\n", d);
            }
        } else
            SEND_TO_Q("Not enough space left in buffer.\r\n", d);
        break;
    case PARSE_DELETE:
        switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
        case 0:
            SEND_TO_Q("You must specify a line number or range to delete.\r\n", d);
            return;
        case 1:
            line_high = line_low;
            break;
        case 2:
            if (line_high < line_low) {
                SEND_TO_Q("That range is invalid.\r\n", d);
                return;
            }
            break;
        }

        i = 1;
        total_len = 1;
        if ((s = *d->str) == NULL) {
            SEND_TO_Q("Buffer is empty.\r\n", d);
            return;
        }
        if (line_low > 0) {
            while (s && (i < line_low))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    s++;
                }
            if ((i < line_low) || (s == NULL)) {
                SEND_TO_Q("Line(s) out of range; not deleting.\r\n", d);
                return;
            }
            t = s;
            while (s && (i < line_high))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    total_len++;
                    s++;
                }
            if ((s) && ((s = strchr(s, '\n')) != NULL)) {
                s++;
                while (*s != '\0')
                    *(t++) = *(s++);
            } else
                total_len--;
            *t = '\0';
            RECREATE(*d->str, char, strlen(*d->str) + 3);
            sprintf(buf, "%d line%sdeleted.\r\n", total_len,
                    ((total_len != 1) ? "s " : " "));
            SEND_TO_Q(buf, d);
        } else {
            SEND_TO_Q("Invalid line numbers to delete must be higher than 0.\r\n", d);
            return;
        }
        break;
    case PARSE_LIST_NORM:
        /* note: my buf,buf1,buf2 vars are defined at 32k sizes so they are
         * prolly ok fer what i want to do here. */
        *buf = '\0';
        if (*string != '\0')
            switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
            case 0:
                line_low = 1;
                line_high = 999999;
                break;
            case 1:
                line_high = line_low;
                break;
            }
        else {
            line_low = 1;
            line_high = 999999;
        }

        if (line_low < 1) {
            SEND_TO_Q("Line numbers must be greater than 0.\r\n", d);
            return;
        }
        if (line_high < line_low) {
            SEND_TO_Q("That range is invalid.\r\n", d);
            return;
        }
        *buf = '\0';
        if ((line_high < 999999) || (line_low > 1)) {
            sprintf(buf, "Current buffer range [%d - %d]:\r\n", line_low, line_high);
        }
        i = 1;
        total_len = 0;
        s = *d->str;
        while (s && (i < line_low))
            if ((s = strchr(s, '\n')) != NULL) {
                i++;
                s++;
            }
        if ((i < line_low) || (s == NULL)) {
            SEND_TO_Q("Line(s) out of range; no buffer listing.\r\n", d);
            return;
        }
        t = s;
        while (s && (i <= line_high))
            if ((s = strchr(s, '\n')) != NULL) {
                i++;
                total_len++;
                s++;
            }
        if (s) {
            temp = *s;
            *s = '\0';
            strcat(buf, t);
            *s = temp;
        } else
            strcat(buf, t);
        /* this is kind of annoying.. will have to take a poll and see..
         * sprintf(buf, "%s\r\n%d line%sshown.\r\n", buf, total_len,
         * ((total_len != 1)?"s ":" ")); */
        page_string(d, buf, TRUE);
        break;
    case PARSE_LIST_NUM:
        /* note: my buf,buf1,buf2 vars are defined at 32k sizes so they are
         * prolly ok fer what i want to do here. */
        *buf = '\0';
        if (*string != '\0')
            switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
            case 0:
                line_low = 1;
                line_high = 999999;
                break;
            case 1:
                line_high = line_low;
                break;
            }
        else {
            line_low = 1;
            line_high = 999999;
        }

        if (line_low < 1) {
            SEND_TO_Q("Line numbers must be greater than 0.\r\n", d);
            return;
        }
        if (line_high < line_low) {
            SEND_TO_Q("That range is invalid.\r\n", d);
            return;
        }
        *buf = '\0';
        i = 1;
        total_len = 0;
        s = *d->str;
        while (s && (i < line_low))
            if ((s = strchr(s, '\n')) != NULL) {
                i++;
                s++;
            }
        if ((i < line_low) || (s == NULL)) {
            SEND_TO_Q("Line(s) out of range; no buffer listing.\r\n", d);
            return;
        }
        t = s;
        while (s && (i <= line_high))
            if ((s = strchr(s, '\n')) != NULL) {
                i++;
                total_len++;
                s++;
                temp = *s;
                *s = '\0';
                sprintf(buf, "%s%4d:\r\n", buf, (i - 1));
                strcat(buf, t);
                *s = temp;
                t = s;
            }
        if (s && t) {
            temp = *s;
            *s = '\0';
            strcat(buf, t);
            *s = temp;
        } else if (t)
            strcat(buf, t);
        /* this is kind of annoying .. seeing as the lines are #ed
         * sprintf(buf, "%s\r\n%d numbered line%slisted.\r\n", buf,
         * total_len, ((total_len != 1)?"s ":" ")); */
        page_string(d, buf, TRUE);
        break;

    case PARSE_INSERT:
        half_chop(string, buf, buf2);
        if (*buf == '\0') {
            SEND_TO_Q("You must specify a line number before which to insert text.\r\n", d);
            return;
        }
        line_low = atoi(buf);
        strcat(buf2, "\r\n");

        i = 1;
        *buf = '\0';
        if ((s = *d->str) == NULL) {
            SEND_TO_Q("Buffer is empty, nowhere to insert.\r\n", d);
            return;
        }
        if (line_low > 0) {
            while (s && (i < line_low))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    s++;
                }
            if ((i < line_low) || (s == NULL)) {
                SEND_TO_Q("Line number out of range; insert aborted.\r\n", d);
                return;
            }
            temp = *s;
            *s = '\0';
            if ((strlen(*d->str) + strlen(buf2) + strlen(s + 1) + 3) > d->max_str) {
                *s = temp;
                SEND_TO_Q("Insert text pushes buffer over maximum size, insert aborted.\r\n", d);
                return;
            }
            if (*d->str && (**d->str != '\0'))
                strcat(buf, *d->str);
            *s = temp;
            strcat(buf, buf2);
            if (s && (*s != '\0'))
                strcat(buf, s);
            RECREATE(*d->str, char, strlen(buf) + 3);
            strcpy(*d->str, buf);
            SEND_TO_Q("Line inserted.\r\n", d);
        } else {
            SEND_TO_Q("Line number must be higher than 0.\r\n", d);
            return;
        }
        break;

    case PARSE_EDIT:
        half_chop(string, buf, buf2);
        if (*buf == '\0') {
            SEND_TO_Q("You must specify a line number at which to change text.\r\n", d);
            return;
        }
        line_low = atoi(buf);
        strcat(buf2, "\r\n");

        i = 1;
        *buf = '\0';
        if ((s = *d->str) == NULL) {
            SEND_TO_Q("Buffer is empty, nothing to change.\r\n", d);
            return;
        }
        if (line_low > 0) {
            /* loop through the text counting /n chars till we get to the
             * line */
            while (s && (i < line_low))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    s++;
                }
            /* make sure that there was a THAT line in the text */
            if ((i < line_low) || (s == NULL)) {
                SEND_TO_Q("Line number out of range; change aborted.\r\n", d);
                return;
            }
            /* if s is the same as *d->str that means im at the beginning of
             * the message text and i dont need to put that into the changed
             * buffer */
            if (s != *d->str) {
                /* first things first .. we get this part into buf. */
                temp = *s;
                *s = '\0';
                /* put the first 'good' half of the text into storage */
                strcat(buf, *d->str);
                *s = temp;
            }
            /* put the new 'good' line into place. */
            strcat(buf, buf2);
            if ((s = strchr(s, '\n')) != NULL) {
                /* this means that we are at the END of the line we want
                 * outta there. */
                /* BUT we want s to point to the beginning of the line AFTER
                 * the line we want edited */
                s++;
                /* now put the last 'good' half of buffer into storage */
                strcat(buf, s);
            }
            /* check for buffer overflow */
            if (strlen(buf) > d->max_str) {
                SEND_TO_Q("Change causes new length to exceed buffer maximum size, aborted.\r\n", d);
                return;
            }
            /* change the size of the REAL buffer to fit the new text */
            RECREATE(*d->str, char, strlen(buf) + 3);
            strcpy(*d->str, buf);
            SEND_TO_Q("Line changed.\r\n", d);
        } else {
            SEND_TO_Q("Line number must be higher than 0.\r\n", d);
            return;
        }
        break;
    default:
        SEND_TO_Q("Invalid option.\r\n", d);
        mudlog("SYSERR: invalid command passed to parse_action", BRF, LVL_IMPL, TRUE);
        return;
    }
}


/* Add user input to the 'current' string (as defined by d->str) */
void            string_add(struct descriptor_data * d, char *str)
{
    int             terminator = 0,
                                 action = 0, accept=1;
    register int    i = 2,
                        j = 0;
    char            actions[MAX_INPUT_LENGTH];
    extern char    *MENU;

    /* determine if this is the terminal string, and truncate if so */
    /* changed to accept '/<letter>' style editing commands - instead */
    /* of solitary '@' to end - (modification of improved_edit patch) */
    /* M. Scott 10/15/96 */
    delete_doubledollar(str);

    /* removed old handling of '@' char */
    /* if ((terminator = (*str == '@'))) *str = '\0'; */

    if ((action = (*str == '/'))) {
        while (str[i] != '\0') {
            actions[j] = str[i];
            i++;
            j++;
        }
        actions[j] = '\0';
        *str = '\0';
        switch (str[1]) {
        case 'a':
            terminator = 2;     /* working on an abort message */
            break;
        case 'c':
            if (*(d->str)) {
                DISPOSE(*d->str);
                *(d->str) = NULL;
                SEND_TO_Q("Current buffer cleared.\r\n", d);
            } else
                SEND_TO_Q("Current buffer empty.\r\n", d);
            break;
        case 'd':
            parse_action(PARSE_DELETE, actions, d);
            break;
        case 'e':
            parse_action(PARSE_EDIT, actions, d);
            break;
        case 'f':
            if (*(d->str))
                parse_action(PARSE_FORMAT, actions, d);
            else
                SEND_TO_Q("Current buffer empty.\r\n", d);
            break;
        case 'i':
            if (*(d->str))
                parse_action(PARSE_INSERT, actions, d);
            else
                SEND_TO_Q("Current buffer empty.\r\n", d);
            break;
        case 'h':
            parse_action(PARSE_HELP, actions, d);
            break;
        case 'l':
            if (*d->str)
                parse_action(PARSE_LIST_NORM, actions, d);
            else
                SEND_TO_Q("Current buffer empty.\r\n", d);
            break;
        case 'n':
            if (*d->str)
                parse_action(PARSE_LIST_NUM, actions, d);
            else
                SEND_TO_Q("Current buffer empty.\r\n", d);
            break;
        case 'r':
            parse_action(PARSE_REPLACE, actions, d);
            break;
        case 's':
            terminator = 1;
            *str = '\0';
            break;
        default:
            SEND_TO_Q("Invalid option.\r\n", d);
            break;
        }
    }
    if (!(*d->str)) {
        if (strlen(str) +3 > d->max_str) {
            send_to_char("String too long - Truncated.\r\n",
                         d->character);
            *(str + d->max_str-3) = '\0';
            

        }
        CREATE(*d->str, char, strlen(str) + 3);
        strcpy(*d->str, str);        
    } else {//if (*str) {
        if (strlen(str) + strlen(*d->str)+3 > d->max_str) {
            send_to_char("String too long.  Last line ignored.\r\n",
                         d->character);
             accept=0;            
            /* terminator = 1; */

        } else {
            if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) +
                                             strlen(str) + 3))) {
                perror("string_add");
                exit(1);
            }
            strcat(*d->str, str);
        }
    }

    if (terminator) {
        /* . OLC Edits . */
        extern void     oedit_disp_menu(struct descriptor_data * d);
        extern void     oedit_disp_extradesc_menu(struct descriptor_data * d);
        extern void     redit_disp_menu(struct descriptor_data * d);
        extern void     redit_disp_extradesc_menu(struct descriptor_data * d);
        extern void     redit_disp_exit_menu(struct descriptor_data * d);
        extern void     medit_disp_menu(struct descriptor_data * d);
#if defined(OASIS_MPROG)
        extern void     medit_change_mprog(struct descriptor_data * d);

        if (STATE(d) == CON_MEDIT) {
            switch (OLC_MODE(d)) {
            case MEDIT_D_DESC:
                medit_disp_menu(d);
                break;
            case MEDIT_MPROG_COMLIST:
                medit_change_mprog(d);
                break;
            }
        }
#endif

        /* here we check for the abort option and reset the pointers */
        if ((terminator == 2) &&
                ((STATE(d) == CON_REDIT) ||
                 (STATE(d) == CON_MEDIT) ||
                 (STATE(d) == CON_OEDIT) ||
                 (STATE(d) == CON_EXDESC))) {
            DISPOSE(*d->str);
            if (d->backstr) {
                *d->str = d->backstr;
            } else
                *d->str = NULL;
            d->backstr = NULL;
            d->str = NULL;
        }
        /* this fix causes the editor to NULL out empty messages -- M. Scott */
        else if ((d->str) && (*d->str) && (**d->str == '\0')) {
            DISPOSE(*d->str);
            *d->str = str_dup("Nothing.\r\n");
        }
        if (STATE(d) == CON_MEDIT)
            medit_disp_menu(d);


        if (STATE(d) == CON_OEDIT) {
            switch (OLC_MODE(d)) {
            case OEDIT_ACTDESC:
                oedit_disp_menu(d);
                break;
            case OEDIT_EXTRADESC_DESCRIPTION:
                oedit_disp_extradesc_menu(d);
            }
        } else if (STATE(d) == CON_REDIT) {
            switch (OLC_MODE(d)) {
            case REDIT_DESC:
                redit_disp_menu(d);
                break;
            case REDIT_EXIT_DESCRIPTION:
                redit_disp_exit_menu(d);
                break;
            case REDIT_EXTRADESC_DESCRIPTION:
                redit_disp_extradesc_menu(d);
                break;
            }
        } else if (!d->connected && (PLR_FLAGGED(d->character, PLR_MAILING))) {
            if ((terminator == 1) && *d->str) {
                store_mail(d->mail_to, GET_IDNUM(d->character), *d->str);
                SEND_TO_Q("Message sent!\r\n", d);
            } else
                SEND_TO_Q("Mail aborted.\r\n", d);
            d->mail_to = 0;
            DISPOSE(*d->str);
            DISPOSE(d->str);
        } else if (d->mail_to >= BOARD_MAGIC) {
            Board_save_board(d->mail_to - BOARD_MAGIC);
            SEND_TO_Q("Post not aborted, use REMOVE <post #>.\r\n", d);
            d->mail_to = 0;
        } else if (STATE(d) == CON_EXDESC) {
            if (terminator != 1)
                SEND_TO_Q("Description aborted.\r\n", d);
            SEND_TO_Q(MENU, d);
            d->connected = CON_MENU;
        } else if (!d->connected && d->character && !IS_NPC(d->character)) {
            if (terminator == 1) {
                if (strlen(*d->str) == 0) {
                    DISPOSE(*d->str);
                    *d->str = NULL;
                }
            } else {
                DISPOSE(*d->str);
                if (d->backstr) {
                    *d->str = d->backstr;
                } else
                    *d->str = NULL;
                d->backstr = NULL;
                SEND_TO_Q("Message aborted.\r\n", d);
            }
        }
        if (d->character && !IS_NPC(d->character))
            REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING | PLR_MAILING);
        if (d->backstr)
            DISPOSE(d->backstr);
        d->backstr = NULL;
        d->str = NULL;
    } else if (!action && accept)
        strcat(*d->str, "\r\n");

}





/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

ACMD(do_skillset)
{
    extern char    *spells[];
    struct char_data *vict;
    char            name[100],
    buf2[100],
    buf[100],
    help[MAX_STRING_LENGTH];
    int             skill,
    value,
    i,
    qend;

    argument = one_argument(argument, name);

    if (!*name) {               /* no arguments. print an informative text */
        send_to_char("Syntax: skillset <name> '<skill>' <value>\r\n", ch);
        strcpy(help, "Skill being one of the following:\r\n");
        for (i = 0; *spells[i] != '\n'; i++) {
            if (*spells[i] == '!')
                continue;
            sprintf(help + strlen(help), "%18s", spells[i]);
            if (i % 4 == 3) {
                strcat(help, "\r\n");
                send_to_char(help, ch);
                *help = '\0';
            }
        }
        if (*help)
            send_to_char(help, ch);
        send_to_char("\r\n", ch);
        return;
    }
    if (!(vict = get_char_vis(ch, name))) {
        send_to_char(NOPERSON, ch);
        return;
    }
    skip_spaces(&argument);

    /* If there is no chars in argument */
    if (!*argument) {
        send_to_char("Skill name expected.\r\n", ch);
        return;
    }
    if (*argument != '\'') {
        send_to_char("Skill must be enclosed in: ''\r\n", ch);
        return;
    }
    /* Locate the last quote && lowercase the magic words (if any) */

    for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
        *(argument + qend) = LOWER(*(argument + qend));

    if (*(argument + qend) != '\'') {
        send_to_char("Skill must be enclosed in: ''\r\n", ch);
        return;
    }
    strcpy(help, (argument + 1));
    help[qend - 1] = '\0';
    if ((skill = find_skill_num(help)) <= 0) {
        send_to_char("Unrecognized skill.\r\n", ch);
        return;
    }
    argument += qend + 1;       /* skip to next parameter */
    argument = one_argument(argument, buf);

    if (!*buf) {
        send_to_char("Learned value expected.\r\n", ch);
        return;
    }
    value = atoi(buf);
    if (value < 0) {
        send_to_char("Minimum value for learned is 0.\r\n", ch);
        return;
    }
    if (value > 110) {
        send_to_char("Max value for learned is 110.\r\n", ch);
        return;
    }
    if (IS_NPC(vict)) {
        send_to_char("You can't set NPC skills.\r\n", ch);
        return;
    }
    sprintf(buf2, "%s changed %s's %s to %d.", GET_NAME(ch), GET_NAME(vict),
            spells[skill], value);
    mudlog(buf2, BRF, -1, TRUE);

    SET_SKILL(vict, skill, value);

    sprintf(buf2, "You change %s's %s to %d.\r\n", GET_NAME(vict),
            spells[skill], value);
    send_to_char(buf2, ch);
}

ACMD(do_skillget)
{
    extern char    *spells[];
    struct char_data *vict;
    char            name[100],
    buf2[100],
    buf[100],
    help[MAX_STRING_LENGTH];
    int             skill,
    value,
    i,
    qend;

    argument = one_argument(argument, name);

    if (!*name) {               /* no arguments. print an informative text */
        send_to_char("Syntax: skillget <name> '<skill>'\r\n", ch);
        strcpy(help, "Skill being one of the following:\r\n");
        for (i = 0; *spells[i] != '\n'; i++) {
            if (*spells[i] == '!')
                continue;
            sprintf(help + strlen(help), "%18s", spells[i]);
            if (i % 4 == 3) {
                strcat(help, "\r\n");
                send_to_char(help, ch);
                *help = '\0';
            }
        }
        if (*help)
            send_to_char(help, ch);
        send_to_char("\r\n", ch);
        return;
    }
    if (!(vict = get_char_vis(ch, name))) {
        send_to_char(NOPERSON, ch);
        return;
    }
    skip_spaces(&argument);

    /* If there is no chars in argument */
    if (!*argument) {
        send_to_char("Skill name expected.\r\n", ch);
        return;
    }
    if (*argument != '\'') {
        send_to_char("Skill must be enclosed in: ''\r\n", ch);
        return;
    }
    /* Locate the last quote && lowercase the magic words (if any) */

    for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
        *(argument + qend) = LOWER(*(argument + qend));

    if (*(argument + qend) != '\'') {
        send_to_char("Skill must be enclosed in: ''\r\n", ch);
        return;
    }
    strcpy(help, (argument + 1));
    help[qend - 1] = '\0';
    if ((skill = find_skill_num(help)) <= 0) {
        send_to_char("Unrecognized skill.\r\n", ch);
        return;
    }

    sprintf(buf2, "%s's skill/spell %s is set to %d.\r\n", GET_NAME(vict),
            spells[skill], GET_SKILL(vict, skill));
    send_to_char(buf2, ch);
}


/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char           *one_word(char *argument, char *first_arg)
{
    int             found,
    begin,
    look_at;

    found = begin = 0;

    do {
        for (; isspace(*(argument + begin)); begin++);

        if (*(argument + begin) == '\"') {      /* is it a quote */

            begin++;

            for (look_at = 0; (*(argument + begin + look_at) >= ' ') &&
                    (*(argument + begin + look_at) != '\"'); look_at++)
                *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

            if (*(argument + begin + look_at) == '\"')
                begin++;

        } else {

            for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
                *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

        }

        *(first_arg + look_at) = '\0';
        begin += look_at;
    } while (fill_word(first_arg));

    return (argument + begin);
}


struct help_index_element *build_help_index(FILE * fl, int *num, char *helpshere[])
{
    int             nr = -1,
                         issorted,
                         i;
    struct help_index_element *list = 0,
                                                  mem;
    char            buf[128],
    tmp[128],
    temp[10000],
    *scan;
    long            pos=-1,pp;
    int             count_hash_records(FILE * fl);

    i = count_hash_records(fl) * 5;
    sprintf(buf, "   Counted %d help entries.", i/5);
    mudlog(buf, NRM, LVL_GOD, TRUE);
    rewind(fl);
    CREATE(list, struct help_index_element, i);

for (;;) {
        pos++;
        fgets(buf, 128, fl);
        strcpy(temp,buf);
        *(buf + strlen(buf) - 1) = '\0';
        scan = buf;
        for (;;) {
            /* extract the keywords */
            scan = one_word(scan, tmp);

            if (!*tmp)
                break;

            nr++;

            list[nr].pos = pos;
            CREATE(list[nr].keyword, char, strlen(tmp) + 1);
            strcpy(list[nr].keyword, tmp);
        }
        /* skip the text */
        do
        {
            fgets(buf, 128, fl);
            if (*buf!='#')	strcat(temp,buf);
        }
        while (*buf != '#');
        strcat(temp,"\0");
        CREATE(helpshere[pos],char,strlen(temp)+1);
        strcpy(helpshere[pos],temp);
        if (*(buf + 1) == '~')
            break;
    }
    /* we might as well sort the stuff */
    do {
        issorted = 1;
        for (i = 0; i < nr; i++)
            if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0) {
                mem = list[i];
                list[i] = list[i + 1];
                list[i + 1] = mem;
                issorted = 0;
            }
    } while (!issorted);

    *num = nr;
    return (list);
}



void            page_string(struct descriptor_data * d, char *str, int keep_internal)
{
    if (!d)
        return;

    if (keep_internal) {
        CREATE(d->showstr_head, char, strlen(str) + 1);
        strcpy(d->showstr_head, str);
        d->showstr_point = d->showstr_head;
    } else
        d->showstr_point = str;

    show_string(d, "");
}



void            show_string(struct descriptor_data * d, char *input)
{
    char            buffer[MAX_STRING_LENGTH],
    buf[MAX_INPUT_LENGTH];
    register char  *scan,
    *chk;
    int             lines = 0,
                            toggle = 1;

    one_argument(input, buf);

    if (*buf) {
        if (d->showstr_head) {
            DISPOSE(d->showstr_head);
            d->showstr_head = 0;
        }
        d->showstr_point = 0;
        return;
    }
    /* show a chunk */
    for (scan = buffer;; scan++, d->showstr_point++) {
        if ((((*scan = *d->showstr_point) == '\n') /*|| (*scan == '\r')*/)/* &&
                    ((toggle = -toggle) < 0)*/)
            lines++;
        else if (!*scan || (lines >= 21)) {
            if (lines >= 21) {
                /* We need to make sure that if we're breaking the input
                 * here, we must set showstr_point to the right place and
                 * null terminate the character after the last '\n' or '\r'
                 * so we don't lose it. -- Michael Buselli */
                //  d->showstr_point++;
                *(scan) = '\0';
            }
            SEND_TO_Q(buffer, d);

            /* see if this is the end (or near the end) of the string */
            for (chk = d->showstr_point; isspace(*chk); chk++);
            if (!*chk) {
                if (d->showstr_head) {
                    DISPOSE(d->showstr_head);
                    d->showstr_head = 0;
                }
                d->showstr_point = 0;
            }
            return;
        }
    }
}
