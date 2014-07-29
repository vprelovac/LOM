/************************************************************************
 * OasisOLC - medit.c						v1.5	*
 * Copyright 1996 Harvey Gilpin.					*
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "structs.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "shop.h"
#include "olc.h"
#include "handler.h"
#include "class.h"
/*
 * Set this to 1 for debugging logs in medit_save_internally.
 */
#if 0
#define DEBUG
#endif

/*
 * Set this to 1 as a last resort to save mobiles.
 */
#if 0
#define I_CRASH
#endif

/*-------------------------------------------------------------------*/

/*
 * External variable declarations.
 */                               

extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern struct char_data *character_list;
extern int      top_of_mobt;
extern struct zone_data *zone_table;
extern int      top_of_zone_table;
extern struct player_special_data dummy_mob;
extern struct attack_hit_type attack_hit_text[];
extern char    *action_bits[];
extern char    *affected_bits[];
extern char    *position_types[];
extern char    *genders[];
extern int      top_shop;
extern struct shop_data *shop_index;
extern struct descriptor_data *descriptor_list;
#if defined(OASIS_MPROG)
extern char    *mobprog_types[];
#endif

/*-------------------------------------------------------------------*/

/*
 * Handy internal macros.
 */

#define GET_NDD(mob) ((mob)->mob_specials.damnodice)
#define GET_SDD(mob) ((mob)->mob_specials.damsizedice)
#define GET_ALIAS(mob) ((mob)->player.name)
#define GET_SDESC(mob) ((mob)->player.short_descr)
#define GET_LDESC(mob) ((mob)->player.long_descr)
#define GET_DDESC(mob) ((mob)->player.description)
#define GET_ATTACK(mob) ((mob)->mob_specials.attack_type)
#define S_KEEPER(shop) ((shop)->keeper)
#if defined(OASIS_MPROG)
#define GET_MPROG(mob)		(mob_index[(mob)->nr].mudprogs)
#define GET_MPROG_TYPE(mob)	(mob_index[(mob)->nr].progtypes)
#endif

/*-------------------------------------------------------------------*/

/*
 * Function prototypes.
 */
void            medit_parse(struct descriptor_data * d, char *arg);
void            medit_disp_menu(struct descriptor_data * d);
void            medit_setup_new(struct descriptor_data * d);
void            medit_setup_existing(struct descriptor_data * d, int rmob_num);
void            medit_save_internally(struct descriptor_data * d);
void            medit_save_to_disk(struct descriptor_data * d, int new_format);
void            init_mobile(struct char_data * mob);
void            copy_mobile(struct char_data * tmob, struct char_data * fmob);
void            medit_disp_positions(struct descriptor_data * d);
void            medit_disp_mob_flags(struct descriptor_data * d);
void            medit_disp_aff_flags(struct descriptor_data * d);
void            medit_disp_attack_types(struct descriptor_data * d);
#if defined(OASIS_MPROG)
void            medit_disp_mprog(struct descriptor_data * d);
void            medit_change_mprog(struct descriptor_data * d);
const char     *medit_get_mprog_type(struct mob_prog_data * mprog);
extern char *mprog_type_to_name( int type );
#endif

/*-------------------------------------------------------------------*\
  utility functions
\*-------------------------------------------------------------------*/

/*
 * Free a mobile structure that has been edited.
 * Take care of existing mobiles and their mob_proto!
 */

void            medit_free_mobile(struct char_data * mob)
{
    int             i;
    /* Non-prototyped mobile.  Also known as new mobiles. */
    if (!mob)
        return;
    else if (GET_MOB_RNUM(mob) == -1) {
        if (mob->player.name)
            DISPOSE(mob->player.name);
        if (mob->player.title)
            DISPOSE(mob->player.title);
        if (mob->player.short_descr)
            DISPOSE(mob->player.short_descr);
        if (mob->player.long_descr)
            DISPOSE(mob->player.long_descr);
        if (mob->player.description)
            DISPOSE(mob->player.description);
    } else if ((i = GET_MOB_RNUM(mob)) > -1) {  /* Prototyped mobile. */
        if (mob->player.name && mob->player.name != mob_proto[i].player.name)
            DISPOSE(mob->player.name);
        if (mob->player.title && mob->player.title != mob_proto[i].player.title)
            DISPOSE(mob->player.title);
        if (mob->player.short_descr && mob->player.short_descr != mob_proto[i].player.short_descr)
            DISPOSE(mob->player.short_descr);
        if (mob->player.long_descr && mob->player.long_descr != mob_proto[i].player.long_descr)
            DISPOSE(mob->player.long_descr);
        if (mob->player.description && mob->player.description != mob_proto[i].player.description)
            DISPOSE(mob->player.description);
    }
    while (mob->affected)
        affect_remove(mob, mob->affected);

    DISPOSE(mob);
}

void            medit_setup_new(struct descriptor_data * d)
{
    struct char_data *mob;

    /* Allocate a scratch mobile structure.  */
    CREATE(mob, struct char_data, 1);

    init_mobile(mob);

    GET_MOB_RNUM(mob) = -1;
    /* Set up some default strings. */
    GET_ALIAS(mob) = str_dup("mob unfinished");
    GET_SDESC(mob) = str_dup("the unfinished mob");
    GET_LDESC(mob) = str_dup("An unfinished mob stands here.\r\n");
    GET_DDESC(mob) = str_dup("It looks unfinished.\r\n");
#if defined(OASIS_MPROG)
    OLC_MPROGL(d) = NULL;
    OLC_MPROG(d) = NULL;
#endif

    OLC_MOB(d) = mob;
    OLC_VAL(d) = 0;             /* Has changed flag. (It hasn't so far, we
                                 * just made it.) */

    medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

void            medit_setup_existing(struct descriptor_data * d, int rmob_num)
{
    struct char_data *mob;
#if defined(OASIS_MPROG)
    MPROG_DATA     *temp;
    MPROG_DATA     *head;
#endif

    /* Allocate a scratch mobile structure. */
    CREATE(mob, struct char_data, 1);

    copy_mobile(mob, mob_proto + rmob_num);

#if defined(OASIS_MPROG)
    /* I think there needs to be a brace from the if statement to the #endif
     * according to the way the original patch was indented.  If this
     * crashes, try it with the braces and report to greerga@van.ml.org on if
     * that works. */
    if (GET_MPROG(mob))
        CREATE(OLC_MPROGL(d), MPROG_DATA, 1);
    head = OLC_MPROGL(d);
    for (temp = GET_MPROG(mob); temp; temp = temp->next) {
        OLC_MPROGL(d)->type = temp->type;
        OLC_MPROGL(d)->arglist =(char *) strdup(temp->arglist);
        OLC_MPROGL(d)->comlist =(char *) strdup(temp->comlist);
        if (temp->next) {
            CREATE(OLC_MPROGL(d)->next, MPROG_DATA, 1);
            OLC_MPROGL(d) = OLC_MPROGL(d)->next;
        }
    }
    OLC_MPROGL(d) = head;
    OLC_MPROG(d) = OLC_MPROGL(d);
#endif

    OLC_MOB(d) = mob;
    medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

/*
 * Copy one mob struct to another.
 */
void            copy_mobile(struct char_data * tmob, struct char_data * fmob)
{
    /* Free up any used strings. */
    if (GET_ALIAS(tmob))
        DISPOSE(GET_ALIAS(tmob));
    if (GET_SDESC(tmob))
        DISPOSE(GET_SDESC(tmob));
    if (GET_LDESC(tmob))
        DISPOSE(GET_LDESC(tmob));
    if (GET_DDESC(tmob))
        DISPOSE(GET_DDESC(tmob));

    /* Copy mob over. */
    *tmob = *fmob;

    /* Reallocate strings. */
    GET_ALIAS(tmob) = str_dup((GET_ALIAS(fmob) && *GET_ALIAS(fmob)) ? GET_ALIAS(fmob) : "undefined");
    GET_SDESC(tmob) = str_dup((GET_SDESC(fmob) && *GET_SDESC(fmob)) ? GET_SDESC(fmob) : "undefined");
    GET_LDESC(tmob) = str_dup((GET_LDESC(fmob) && *GET_LDESC(fmob)) ? GET_LDESC(fmob) : "undefined");
    GET_DDESC(tmob) = str_dup((GET_DDESC(fmob) && *GET_DDESC(fmob)) ? GET_DDESC(fmob) : "undefined");
}

/*-------------------------------------------------------------------*/

/*
 * Ideally, this function should be in db.c, but I'll put it here for
 * portability.
 */
void            init_mobile(struct char_data * mob)
{
    clear_char(mob);

    GET_HIT(mob) = GET_MANA(mob) = 1;
    GET_MAX_MANA(mob) = GET_MAX_MOVE(mob) = 100;
    GET_NDD(mob) = GET_SDD(mob) = 1;
    GET_WEIGHT(mob) = 200;
    GET_HEIGHT(mob) = 198;

    mob->real_abils.str = mob->real_abils.intel = mob->real_abils.wis = 13;
    mob->real_abils.dex = mob->real_abils.con = mob->real_abils.cha = 13;
    mob->aff_abils = mob->real_abils;

    SET_BIT(MOB_FLAGS(mob), MOB_ISNPC);
    mob->player_specials = &dummy_mob;
}

/*-------------------------------------------------------------------*/

#define ZCMD zone_table[zone].cmd[cmd_no]

/*
 * Save new/edited mob to memory.
 */
void            medit_save_internally(struct descriptor_data * d)
{
    int             rmob_num,
    found = 0,
            new_mob_num = 0,
                          zone,
                          cmd_no,
                          shop;
    struct char_data *new_proto;
    struct index_data *new_index;
    struct char_data *live_mob;
    struct descriptor_data *dsc;

    /* Mob exists? Just update it. */
    if ((rmob_num = real_mobile(OLC_NUM(d))) != -1) {
        copy_mobile((mob_proto + rmob_num), OLC_MOB(d));
        /* Update live mobiles. */
        for (live_mob = character_list; live_mob; live_mob = live_mob->next)
            if (IS_MOB(live_mob) && GET_MOB_RNUM(live_mob) == rmob_num) {
                /* Only really need to update the strings, since these can
                 * cause protection faults.  The rest can wait till a
                 * reset/reboot. */
                GET_ALIAS(live_mob) = GET_ALIAS(mob_proto + rmob_num);
                GET_SDESC(live_mob) = GET_SDESC(mob_proto + rmob_num);
                GET_LDESC(live_mob) = GET_LDESC(mob_proto + rmob_num);
                GET_DDESC(live_mob) = GET_DDESC(mob_proto + rmob_num);
            }
    }
    /* Mob does not exist, we have to add it. */
    else {
#if defined(DEBUG)
        fprintf(stderr, "top_of_mobt: %d, new top_of_mobt: %d", top_of_mobt, top_of_mobt + 1);
#endif

        CREATE(new_proto, struct char_data, top_of_mobt + 2);
        CREATE(new_index, struct index_data, top_of_mobt + 2);

        for (rmob_num = 0; rmob_num <= top_of_mobt; rmob_num++) {
            if (!found) {       /* Is this the place? */
                if ((rmob_num > top_of_mobt) || (mob_index[rmob_num].virtual > OLC_NUM(d))) {
                    /* Yep, stick it here. */
                    found = TRUE;
#if defined(DEBUG)
                    fprintf(stderr, "Inserted: rmob_num: %d", rmob_num);
#endif
                    new_index[rmob_num].virtual = OLC_NUM(d);
                    new_index[rmob_num].number = 0;
                    new_index[rmob_num].func = NULL;
                    new_mob_num = rmob_num;
                    GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
                    copy_mobile((new_proto + rmob_num), OLC_MOB(d));
                    /* Copy the mob that should be here on top. */
                    new_index[rmob_num + 1] = mob_index[rmob_num];
                    new_proto[rmob_num + 1] = mob_proto[rmob_num];
                    GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
                } else {        /* Nope, copy over as normal. */
                    new_index[rmob_num] = mob_index[rmob_num];
                    new_proto[rmob_num] = mob_proto[rmob_num];
                }
            } else {            /* We've already found it, copy the rest
                                         * over. */
                new_index[rmob_num + 1] = mob_index[rmob_num];
                new_proto[rmob_num + 1] = mob_proto[rmob_num];
                GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
            }
        }
#if defined(DEBUG)
        fprintf(stderr, "rmob_num: %d, top_of_mobt: %d, array size: 0-%d (%d)",
                rmob_num, top_of_mobt, top_of_mobt + 1, top_of_mobt + 2);
#endif
        if (!found) {           /* Still not found, must add it to the top of
                                         * the table. */
#if defined(DEBUG)
            fprintf(stderr, "Append.");
#endif
            new_index[rmob_num].virtual = OLC_NUM(d);
            new_index[rmob_num].number = 0;
            new_index[rmob_num].func = NULL;
            new_mob_num = rmob_num;
            GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
            copy_mobile((new_proto + rmob_num), OLC_MOB(d));
        }
        /* Replace tables. */
#if defined(DEBUG)
        fprintf(stderr, "Attempted free.");
#endif
        /* I have noticed that if you attempt to free a block of memory and
         * you give DISPOSE() a pointer other than it was originally given, the
         * program will crash.  It's a theory for the two DISPOSE()'s below. */
        /*
        #if !defined(I_CRASH)
            DISPOSE(mob_index);
            DISPOSE(mob_proto);
        #endif
        */
        mob_index = new_index;
        mob_proto = new_proto;
        top_of_mobt++;
#if defined(DEBUG)
        fprintf(stderr, "Free ok.");
#endif

        /* Update live mobile rnums. */
        for (live_mob = character_list; live_mob; live_mob = live_mob->next)
            if (GET_MOB_RNUM(live_mob) > new_mob_num)
                GET_MOB_RNUM(live_mob)++;

        /* Update zone table. */
        for (zone = 0; zone <= top_of_zone_table; zone++)
            for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
                if (ZCMD.command == 'M')
                    if (ZCMD.arg1 > new_mob_num)
                        ZCMD.arg1++;

        /* Update shop keepers. */
        for (shop = 0; shop <= top_shop; shop++)
            if (SHOP_KEEPER(shop) > new_mob_num)
                SHOP_KEEPER(shop)++;

        /* Update keepers in shops being edited and other mobs being edited. */
        for (dsc = descriptor_list; dsc; dsc = dsc->next)
            if (dsc->connected == CON_SEDIT) {
                if (S_KEEPER(OLC_SHOP(dsc)) >= new_mob_num)
                    S_KEEPER(OLC_SHOP(dsc))++;
            } else if (dsc->connected == CON_MEDIT) {
                if (GET_MOB_RNUM(OLC_MOB(dsc)) > new_mob_num)
                    GET_MOB_RNUM(OLC_MOB(dsc))++;
            }
    }

#if defined(OASIS_MPROG)
    GET_MPROG(OLC_MOB(d)) = OLC_MPROGL(d);
    GET_MPROG_TYPE(OLC_MOB(d)) = (OLC_MPROGL(d) ? OLC_MPROGL(d)->type : 0);
    OLC_MPROGL(d) = NULL;
#endif

    olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}

/*-------------------------------------------------------------------*/

/*
 * Save ALL mobiles for a zone to their .mob file, mobs are all
 * saved in Extended format, regardless of whether they have any
 * extended fields.  Thanks to Samedi for ideas on this bit of code
 */
void            medit_save_to_disk(struct descriptor_data * d, int new_format)
{
    int             i,
    rmob_num,
    zone,
    top;
    FILE           *mob_file;
    char            fname[64];
    struct char_data *mob;
#if defined(OASIS_MPROG)
    MPROG_DATA     *mob_prog = NULL;
#endif

    zone = zone_table[OLC_ZNUM(d)].number;
    top = zone_table[OLC_ZNUM(d)].top;

    if (!new_format)
    {
        sprintf(fname, "%s/%d.new", MOB_PREFIX, zone);
        if (!(mob_file = fopen(fname, "w"))) {
            mudlog("SYSERR: OLC: Cannot open mob file!", BRF, LVL_BUILDER, TRUE);
            send_to_char("Couldn't open the mob file for writing.\r\n", d->character);
            return;
        }
    }
    else
    {
        sprintf(fname, "%s.pom",  zone_table[OLC_ZNUM(d)].disk_name);
        if (!(mob_file = fopen(fname, "a+"))) {
            mudlog("SYSERR: OLC: Cannot open mob file!", BRF, LVL_BUILDER, TRUE);
            send_to_char("Couldn't open the mob file for writing.\r\n", d->character);
            return;
        }

    }
    /* Seach the database for mobs in this zone and save them. */
    for (i = zone * 100; i <= top; i++) {
        if ((rmob_num = real_mobile(i)) != -1) {
            if (fprintf(mob_file, "#%d\n", i) < 0) {
                mudlog("SYSERR: OLC: Cannot write mob file!\r\n", BRF, LVL_BUILDER, TRUE);
                fclose(mob_file);
                return;
            }
            mob = (mob_proto + rmob_num);

            /* Clean up strings. */
            strcpy(buf1, (GET_LDESC(mob) && *GET_LDESC(mob)) ? GET_LDESC(mob) : "undefined");
            strip_string(buf1);
            strcpy(buf2, (GET_DDESC(mob) && *GET_DDESC(mob)) ? GET_DDESC(mob) : "undefined");
            strip_string(buf2);

            fprintf(mob_file, "%s~\n"
                    "%s~\n"
                    "%s~\n"
                    "%s~\n"
                    "%ld %ld %d E\n"
                    "%d %d %d %dd%d+%d %dd%d+%d\n"
                    "%d %d\n"
                    "%d %d %d\n",
                    (GET_ALIAS(mob) && *GET_ALIAS(mob)) ? GET_ALIAS(mob) : "undefined",
                    (GET_SDESC(mob) && *GET_SDESC(mob)) ? GET_SDESC(mob) : "undefined",
                    buf1, buf2, MOB_FLAGS(mob), AFF_FLAGS(mob), GET_ALIGNMENT(mob),
                    GET_LEVEL(mob), 20 - GET_HITROLL(mob),      /* Hitroll -> THAC0 */
                    GET_AC(mob) / 10, GET_HIT(mob), GET_MANA(mob), GET_MOVE(mob),
                    GET_NDD(mob), GET_SDD(mob), GET_DAMROLL(mob), GET_GOLD(mob),
                    GET_EXP(mob), GET_POS(mob), GET_DEFAULT_POS(mob), GET_SEX(mob)
                   );

            /* Deal with Extra stats in case they are there. */
            if ((GET_ATTACK(mob) != 0) && (GET_ATTACK(mob) != 13))
                fprintf(mob_file, "BareHandAttack: %d\n", GET_ATTACK(mob));
            /*if (GET_STR(mob) != 13)
                fprintf(mob_file, "Str: %d\n", GET_STR(mob));
            if (GET_ADD(mob) != 0)
                fprintf(mob_file, "StrAdd: %d\n", GET_ADD(mob));
            if (GET_DEX(mob) != 13)
                fprintf(mob_file, "Dex: %d\n", GET_DEX(mob));
            if (GET_INT(mob) != 13)
                fprintf(mob_file, "Int: %d\n", GET_INT(mob));
            if (GET_WIS(mob) != 13)
                fprintf(mob_file, "Wis: %d\n", GET_WIS(mob));
            if (GET_CON(mob) != 13)
                fprintf(mob_file, "Con: %d\n", GET_CON(mob));
            if (GET_CHA(mob) != 13)
                fprintf(mob_file, "Cha: %d\n", GET_CHA(mob));
*/
            /* XXX: Add E-mob handlers here. */

            fprintf(mob_file, "E\n");

#if defined(OASIS_MPROG)
            /* Write out the MobProgs. */
            mob_prog = GET_MPROG(mob);
            while (mob_prog) {
                strcpy(buf1, mob_prog->arglist);
                strip_string(buf1);
                strcpy(buf2, mob_prog->comlist);
                strip_string(buf2);
                fprintf(mob_file, ">%s %s~\n%s", mprog_type_to_name(mob_prog->type),
                        buf1, buf2);
                mob_prog = mob_prog->next;
                fprintf(mob_file, "~\n%s", (!mob_prog ? "|\n" : ""));
            }
#endif
        }
    }
    if (!new_format)

        fprintf(mob_file, "$\n");

    else
        fprintf(mob_file, "$\n$end~\n\n\n#OBJECTS\n");
    fclose(mob_file);
    if (!new_format)
    {
        sprintf(buf2, "%s/%d.mob", MOB_PREFIX, zone);
        /* We're fubar'd if we crash between the two lines below. */
        remove(buf2);
        rename(fname, buf2);
    }
    olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}

/**************************************************************************
 Menu functions
 **************************************************************************/

/*
 * Display positions. (sitting, standing, etc)
 */
void            medit_disp_positions(struct descriptor_data * d)
{
    int             i;

    get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif
    for (i = 0; *position_types[i] != '\n'; i++) {
        sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, position_types[i]);
        send_to_char(buf, d->character);
    }
    send_to_char("Enter position number : ", d->character);
}

/*-------------------------------------------------------------------*/

#if defined(OASIS_MPROG)
/*
 * Get the type of MobProg.
 */
const char     *medit_get_mprog_type(struct mob_prog_data * mprog)
{
    switch (mprog->type) {
    case IN_FILE_PROG:
        return ">in_file_prog";
    case ACT_PROG:
        return ">act_prog";
    case SPEECH_PROG:
        return ">speech_prog";
    case RAND_PROG:
        return ">rand_prog";
    case FIGHT_PROG:
        return ">fight_prog";
    case HITPRCNT_PROG:
        return ">hitprcnt_prog";
    case DEATH_PROG:
        return ">death_prog";
    case ENTRY_PROG:
        return ">entry_prog";
    case GREET_PROG:
        return ">greet_prog";
    case ALL_GREET_PROG:
        return ">all_greet_prog";
    case GIVE_PROG:
        return ">give_prog";
    case BRIBE_PROG:
        return ">bribe_prog";
    }
    return ">ERROR_PROG";
}

/*-------------------------------------------------------------------*/

/*
 * Display the MobProgs.
 */
void            medit_disp_mprog(struct descriptor_data * d)
{
    struct mob_prog_data *mprog = OLC_MPROGL(d);

    OLC_MTOTAL(d) = 1;

#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif
    while (mprog) {
        sprintf(buf, "%d) %s %s\r\n", OLC_MTOTAL(d), medit_get_mprog_type(mprog),
                (mprog->arglist ? mprog->arglist : "NONE"));
        send_to_char(buf, d->character);
        OLC_MTOTAL(d)++;
        mprog = mprog->next;
    }
    sprintf(buf, "%d) Create New Mob Prog\r\n"
            "%d) Purge Mob Prog\r\n"
            "Enter number to edit [0 to exit]:  ",
            OLC_MTOTAL(d), OLC_MTOTAL(d) + 1);
    send_to_char(buf, d->character);
    OLC_MODE(d) = MEDIT_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProgs.
 */
void            medit_change_mprog(struct descriptor_data * d)
{
#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif
    sprintf(buf, "1) Type: %s\r\n"
            "2) Args: %s\r\n"
            "3) Commands:\r\n%s\r\r\n\n"
            "Enter number to edit [0 to exit]: ",
            medit_get_mprog_type(OLC_MPROG(d)),
            (OLC_MPROG(d)->arglist ? OLC_MPROG(d)->arglist : "NONE"),
            (OLC_MPROG(d)->comlist ? OLC_MPROG(d)->comlist : "NONE"));

    send_to_char(buf, d->character);
    OLC_MODE(d) = MEDIT_CHANGE_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProg type.
 */
void            medit_disp_mprog_types(struct descriptor_data * d)
{
    int             i;

    get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif

    for (i = 0; i < NUM_PROGS - 1; i++) {
        sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, mobprog_types[i]);
        send_to_char(buf, d->character);
    }
    send_to_char("Enter mob prog type : ", d->character);
    OLC_MODE(d) = MEDIT_MPROG_TYPE;
}

#endif

/*-------------------------------------------------------------------*/

/*
 * Display the gender of the mobile.
 */
void            medit_disp_sex(struct descriptor_data * d)
{
    int             i;

    get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif
    for (i = 0; i < NUM_GENDERS; i++) {
        sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, genders[i]);
        send_to_char(buf, d->character);
    }
    send_to_char("Enter gender number : ", d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display attack types menu.
 */
void            medit_disp_attack_types(struct descriptor_data * d)
{
    int             i;

    get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif
    for (i = 0; i < NUM_ATTACK_TYPES; i++) {
        sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, attack_hit_text[i].singular);
        send_to_char(buf, d->character);
    }
    send_to_char("Enter attack type : ", d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display mob-flags menu.
 */
void            medit_disp_mob_flags(struct descriptor_data * d)
{
    int             i,
    columns = 0;

    get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif
    for (i = 0; i < NUM_MOB_FLAGS; i++) {
        sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, action_bits[i],
                !(++columns % 2) ? "\r\n" : "");
        send_to_char(buf, d->character);
    }
    sprintbit(MOB_FLAGS(OLC_MOB(d)), action_bits, buf1);
    sprintf(buf, "\r\nCurrent flags : %s%s%s\r\nEnter mob flags (0 to quit) : ",
            cyn, buf1, nrm);
    send_to_char(buf, d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display affection flags menu.
 */
void            medit_disp_aff_flags(struct descriptor_data * d)
{
    int             i,
    columns = 0;

    get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
    send_to_char("[H[J", d->character);
#endif
    for (i = 0; i < NUM_AFF_FLAGS; i++) {
        sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, affected_bits[i],
                !(++columns % 2) ? "\r\n" : "");
        send_to_char(buf, d->character);
    }
    sprintbit(AFF_FLAGS(OLC_MOB(d)), affected_bits, buf1);
    sprintf(buf, "\r\nCurrent flags   : %s%s%s\r\nEnter aff flags (0 to quit) : ",
            cyn, buf1, nrm);
    send_to_char(buf, d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display main menu.
 */
void            medit_disp_menu(struct descriptor_data * d)
{
    struct char_data *mob;

    mob = OLC_MOB(d);
    get_char_cols(d->character);

    sprintf(buf,
#if defined(CLEAR_SCREEN)
            "[H[J"
#endif
            "-- Mob Number:  [%s%d%s]\r\n"
            "%s1%s) Sex: %s%-7.7s%s	         %s2%s) Alias: %s%s\r\n"
            "%s3%s) S-Desc: %s%s\r\n"
            "%s4%s) L-Desc:-\r\n%s%s"
            "%s5%s) D-Desc:-\r\n%s%s"
            "%s6%s) Level:       [%s%4d%s],  %s7%s) Alignment:    [%s%4d%s]\r\n"
            "%s8%s) Hitroll:     [%s%4d%s],  %s9%s) Damroll:      [%s%4d%s]\r\n"
            "%sA%s) NumDamDice:  [%s%4d%s],  %sB%s) SizeDamDice:  [%s%4d%s]\r\n"
            "%sC%s) Num HP Dice: [%s%4d%s],  %sD%s) Size HP Dice: [%s%4d%s],  %sE%s) HP Bonus: [%s%5d%s]\r\n"
            "%sF%s) Armor Class: [%s%4d%s],  %sG%s) Exp:     [%s%9d%s],  %sH%s) Gold:  [%s%8d%s]\r\n",

            cyn, OLC_NUM(d), nrm,
            grn, nrm, yel, genders[(int) GET_SEX(mob)], nrm,
            grn, nrm, yel, GET_ALIAS(mob),
            grn, nrm, yel, GET_SDESC(mob),
            grn, nrm, yel, GET_LDESC(mob),
            grn, nrm, yel, GET_DDESC(mob),
            grn, nrm, cyn, GET_LEVEL(mob), nrm,
            grn, nrm, cyn, GET_ALIGNMENT(mob), nrm,
            grn, nrm, cyn, GET_HITROLL(mob), nrm,
            grn, nrm, cyn, GET_DAMROLL(mob), nrm,
            grn, nrm, cyn, GET_NDD(mob), nrm,
            grn, nrm, cyn, GET_SDD(mob), nrm,
            grn, nrm, cyn, GET_HIT(mob), nrm,
            grn, nrm, cyn, GET_MANA(mob), nrm,
            grn, nrm, cyn, GET_MOVE(mob), nrm,
            grn, nrm, cyn, GET_AC(mob), nrm,
            grn, nrm, cyn, GET_EXP(mob), nrm,
            grn, nrm, cyn, GET_GOLD(mob), nrm
           );
    send_to_char(buf, d->character);

    sprintbit(MOB_FLAGS(mob), action_bits, buf1);
    sprintbit(AFF_FLAGS(mob), affected_bits, buf2);
    sprintf(buf,
            "%sI%s) Position  : %s%s\r\n"
            "%sJ%s) Default   : %s%s\r\n"
            "%sK%s) Attack    : %s%s\r\n"
            "%sL%s) NPC Flags : %s%s\r\n"
            "%sM%s) AFF Flags : %s%s\r\n"
#if defined(OASIS_MPROG)
            "%sP%s) Mob Progs : %s%s\r\n"
#endif
            "%sQ%s) Quit\r\n"
            "Enter choice : ",

            grn, nrm, yel, position_types[(int) GET_POS(mob)],
            grn, nrm, yel, position_types[(int) GET_DEFAULT_POS(mob)],
            grn, nrm, yel, attack_hit_text[GET_ATTACK(mob)].singular,
            grn, nrm, cyn, buf1,
            grn, nrm, cyn, buf2,
#if defined(OASIS_MPROG)
            grn, nrm, cyn, (OLC_MPROGL(d) ? "Set." : "Not Set."),
#endif
            grn, nrm
           );
    send_to_char(buf, d->character);

    OLC_MODE(d) = MEDIT_MAIN_MENU;
}

/************************************************************************
 *			The GARGANTAUN event handler			*
 ************************************************************************/

void            medit_parse(struct descriptor_data * d, char *arg)
{
    int             i;

    /*  if (OLC_MODE(d) > MEDIT_NUMERICAL_RESPONSE) {
        if (!*arg || (!isdigit(arg[0]) && ((*arg == '-') && (!isdigit(arg[1]))))) {
          send_to_char("Field must be numerical, try again : ", d->character);
          return;
        }
      }*/
    switch (OLC_MODE(d)) {
        /*-------------------------------------------------------------------*/
    case MEDIT_CONFIRM_SAVESTRING:
        /* Ensure mob has MOB_ISNPC set or things will go pair shaped. */
        SET_BIT(MOB_FLAGS(OLC_MOB(d)), MOB_ISNPC);
        switch (*arg) {
        case 'y':
        case 'Y':
            /* Save the mob in memory and to disk. */
            send_to_char("Saving mobile to memory.\r\n", d->character);
            medit_save_internally(d);
            sprintf(buf, "OLC: %s edits mob %d", GET_NAME(d->character), OLC_NUM(d));
            mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
            /* FALL THROUGH */
        case 'n':
        case 'N':
            cleanup_olc(d, CLEANUP_ALL);
            return;
        default:
            send_to_char("Invalid choice!\r\n", d->character);
            send_to_char("Do you wish to save the mobile? : ", d->character);
            return;
        }
        break;

        /*-------------------------------------------------------------------*/
    case MEDIT_MAIN_MENU:
        i = 0;
        switch (*arg) {
        case 'q':
        case 'Q':
            if (OLC_VAL(d)) {   /* Anything been changed? */
                send_to_char("Do you wish to save the changes to the mobile? (y/n) : ", d->character);
                OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
            } else
                cleanup_olc(d, CLEANUP_ALL);
            return;
        case '1':
            OLC_MODE(d) = MEDIT_SEX;
            medit_disp_sex(d);
            return;
        case '2':
            OLC_MODE(d) = MEDIT_ALIAS;
            i--;
            break;
        case '3':
            OLC_MODE(d) = MEDIT_S_DESC;
            i--;
            break;
        case '4':
            OLC_MODE(d) = MEDIT_L_DESC;
            i--;
            break;
        case '5':
            OLC_MODE(d) = MEDIT_D_DESC;
            SEND_TO_Q("Enter mob description: (/s saves /h for help)\r\r\n\n", d);
            d->backstr = NULL;
            if (OLC_MOB(d)->player.description) {
                SEND_TO_Q(OLC_MOB(d)->player.description, d);
                d->backstr = str_dup(OLC_MOB(d)->player.description);
            }
            d->str = &OLC_MOB(d)->player.description;
            d->max_str = MAX_MOB_DESC;
            d->mail_to = 0;
            return;
        case '6':
            OLC_MODE(d) = MEDIT_LEVEL;
            i++;
            break;
        case '7':
            OLC_MODE(d) = MEDIT_ALIGNMENT;
            i++;
            break;
        case '8':
            OLC_MODE(d) = MEDIT_HITROLL;
            i++;
            break;
        case '9':
            OLC_MODE(d) = MEDIT_DAMROLL;
            i++;
            break;
        case 'a':
        case 'A':
            OLC_MODE(d) = MEDIT_NDD;
            i++;
            break;
        case 'b':
        case 'B':
            OLC_MODE(d) = MEDIT_SDD;
            i++;
            break;
        case 'c':
        case 'C':
            OLC_MODE(d) = MEDIT_NUM_HP_DICE;
            i++;
            break;
        case 'd':
        case 'D':
            OLC_MODE(d) = MEDIT_SIZE_HP_DICE;
            i++;
            break;
        case 'e':
        case 'E':
            OLC_MODE(d) = MEDIT_ADD_HP;
            i++;
            break;
        case 'f':
        case 'F':
            OLC_MODE(d) = MEDIT_AC;
            i++;
            break;
        case 'g':
        case 'G':
            OLC_MODE(d) = MEDIT_EXP;
            i++;
            break;
        case 'h':
        case 'H':
            OLC_MODE(d) = MEDIT_GOLD;
            i++;
            break;
        case 'i':
        case 'I':
            OLC_MODE(d) = MEDIT_POS;
            medit_disp_positions(d);
            return;
        case 'j':
        case 'J':
            OLC_MODE(d) = MEDIT_DEFAULT_POS;
            medit_disp_positions(d);
            return;
        case 'k':
        case 'K':
            OLC_MODE(d) = MEDIT_ATTACK;
            medit_disp_attack_types(d);
            return;
        case 'l':
        case 'L':
            OLC_MODE(d) = MEDIT_NPC_FLAGS;
            medit_disp_mob_flags(d);
            return;
        case 'm':
        case 'M':
            OLC_MODE(d) = MEDIT_AFF_FLAGS;
            medit_disp_aff_flags(d);
            return;
#if defined(OASIS_MPROG)
        case 'p':
        case 'P':
            OLC_MODE(d) = MEDIT_MPROG;
            medit_disp_mprog(d);
            return;
#endif
        default:
            medit_disp_menu(d);
            return;
        }
        if (i != 0) {
            send_to_char(i == 1 ? "\r\nEnter new value : " :
                         i == -1 ? "\r\nEnter new text :\r\n] " :
                         "\r\nOops...:\r\n", d->character);
            return;
        }
        break;

        /*-------------------------------------------------------------------*/
    case MEDIT_ALIAS:
        if (GET_ALIAS(OLC_MOB(d)))
            DISPOSE(GET_ALIAS(OLC_MOB(d)));
        GET_ALIAS(OLC_MOB(d)) = str_dup((arg && *arg) ? arg : "undefined");
        break;
        /*-------------------------------------------------------------------*/
    case MEDIT_S_DESC:
        if (GET_SDESC(OLC_MOB(d)))
            DISPOSE(GET_SDESC(OLC_MOB(d)));
        GET_SDESC(OLC_MOB(d)) = str_dup((arg && *arg) ? arg : "undefined");
        break;
        /*-------------------------------------------------------------------*/
    case MEDIT_L_DESC:
        if (GET_LDESC(OLC_MOB(d)))
            DISPOSE(GET_LDESC(OLC_MOB(d)));
        if (arg && *arg) {
            strcpy(buf, arg);
            strcat(buf, "\r\n");
            GET_LDESC(OLC_MOB(d)) = str_dup(buf);
        } else
            GET_LDESC(OLC_MOB(d)) = str_dup("undefined");

        break;
        /*-------------------------------------------------------------------*/
    case MEDIT_D_DESC:
        /* We should never get here. */
        cleanup_olc(d, CLEANUP_ALL);
        mudlog("SYSERR: OLC: medit_parse(): Reached D_DESC case!",
               BRF, LVL_BUILDER, TRUE);
        send_to_char("Oops...\r\n", d->character);
        break;
        /*-------------------------------------------------------------------*/
#if defined(OASIS_MPROG)
    case MEDIT_MPROG_COMLIST:
        /* We should never get here, but if we do, bail out. */
        cleanup_olc(d, CLEANUP_ALL);
        mudlog("SYSERR: OLC: medit_parse(): Reached MPROG_COMLIST case!",
               BRF, LVL_BUILDER, TRUE);
        break;
#endif
        /*-------------------------------------------------------------------*/
    case MEDIT_NPC_FLAGS:
        if ((i = atoi(arg)) == 0)
            break;
        else if (!((i < 0) || (i > NUM_MOB_FLAGS)))
            TOGGLE_BIT(MOB_FLAGS(OLC_MOB(d)), 1 << (i - 1));
        medit_disp_mob_flags(d);
        return;
        /*-------------------------------------------------------------------*/
    case MEDIT_AFF_FLAGS:
        if ((i = atoi(arg)) == 0)
            break;
        else if (!((i < 0) || (i > NUM_AFF_FLAGS)))
            TOGGLE_BIT(AFF_FLAGS(OLC_MOB(d)), 1 << (i - 1));
        medit_disp_aff_flags(d);
        return;
        /*-------------------------------------------------------------------*/
#if defined(OASIS_MPROG)
    case MEDIT_MPROG:
        if ((i = atoi(arg)) == 0)
            medit_disp_menu(d);
        else if (i == OLC_MTOTAL(d)) {
            struct mob_prog_data *temp;
            CREATE(temp, struct mob_prog_data, 1);
            temp->next = OLC_MPROGL(d);
            temp->type = -1;
            temp->arglist = NULL;
            temp->comlist = NULL;
            OLC_MPROG(d) = temp;
            OLC_MPROGL(d) = temp;
            OLC_MODE(d) = MEDIT_CHANGE_MPROG;
            medit_change_mprog(d);
        } else if (i < OLC_MTOTAL(d)) {
            struct mob_prog_data *temp;
            int             x = 1;
            for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
                x++;
            OLC_MPROG(d) = temp;
            OLC_MODE(d) = MEDIT_CHANGE_MPROG;
            medit_change_mprog(d);
        } else if (i == OLC_MTOTAL(d) + 1) {
            send_to_char("Which mob prog do you want to purge? ", d->character);
            OLC_MODE(d) = MEDIT_PURGE_MPROG;
        } else
            medit_disp_menu(d);
        return;
        break;
    case MEDIT_PURGE_MPROG:
        if ((i = atoi(arg)) > 0 && i < OLC_MTOTAL(d)) {
            struct mob_prog_data *temp;
            int             x = 1;

            for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
                x++;
            OLC_MPROG(d) = temp;
            REMOVE_FROM_LIST(OLC_MPROG(d), OLC_MPROGL(d), next)
            DISPOSE(OLC_MPROG(d)->arglist);
            DISPOSE(OLC_MPROG(d)->comlist);
            DISPOSE(OLC_MPROG(d));
            OLC_MPROG(d) = NULL;
            OLC_VAL(d) = 1;
        }
        medit_disp_mprog(d);
        return;
        break;
    case MEDIT_CHANGE_MPROG:
        if ((i = atoi(arg)) == 1)
            medit_disp_mprog_types(d);
        else if (i == 2) {
            send_to_char("Enter new arg list: ", d->character);
            OLC_MODE(d) = MEDIT_MPROG_ARGS;
        } else if (i == 3) {
            send_to_char("Enter new mob prog commands:\r\n", d->character);
            /* Pass control to modify.c for typing. */
            OLC_MODE(d) = MEDIT_MPROG_COMLIST;
            d->backstr = NULL;
            if (OLC_MPROG(d)->comlist) {
                SEND_TO_Q(OLC_MPROG(d)->comlist, d);
                d->backstr = (char *) strdup(OLC_MPROG(d)->comlist);
            }
            d->str = &OLC_MPROG(d)->comlist;
            d->max_str = MAX_STRING_LENGTH;
            d->mail_to = 0;
            OLC_VAL(d) = 1;
        } else
            medit_disp_mprog(d);
        return;
#endif

        /*-------------------------------------------------------------------*/

        /*
         * Numerical responses.
         */

#if defined(OASIS_MPROG)
    case MEDIT_MPROG_TYPE:
        OLC_MPROG(d)->type = (1 << MAX(0, MIN(atoi(arg) - 1, NUM_PROGS - 1)));
        OLC_VAL(d) = 1;
        medit_change_mprog(d);
        return;

    case MEDIT_MPROG_ARGS:
        OLC_MPROG(d)->arglist = str_dup(arg);
        OLC_VAL(d) = 1;
        medit_change_mprog(d);
        return;
#endif

    case MEDIT_SEX:
        GET_SEX(OLC_MOB(d)) = MAX(0, MIN(NUM_GENDERS - 1, atoi(arg)));
        break;

    case MEDIT_HITROLL:
        GET_HITROLL(OLC_MOB(d)) = MAX(0, MIN(50, atoi(arg)));
        break;

    case MEDIT_DAMROLL:
        GET_DAMROLL(OLC_MOB(d)) = MAX(0, MIN(50, atoi(arg)));
        break;

    case MEDIT_NDD:
        GET_NDD(OLC_MOB(d)) = MAX(0, MIN(30, atoi(arg)));
        break;

    case MEDIT_SDD:
        GET_SDD(OLC_MOB(d)) = MAX(0, MIN(127, atoi(arg)));
        break;

    case MEDIT_NUM_HP_DICE:
        GET_HIT(OLC_MOB(d)) = MAX(0, MIN(30, atoi(arg)));
        break;

    case MEDIT_SIZE_HP_DICE:
        GET_MANA(OLC_MOB(d)) = MAX(0, MIN(1000, atoi(arg)));
        break;

    case MEDIT_ADD_HP:
        GET_MOVE(OLC_MOB(d)) = MAX(0, MIN(30000, atoi(arg)));
        break;

    case MEDIT_AC:
        GET_AC(OLC_MOB(d)) = MAX(-200, MIN(200, atoi(arg)));
        break;

    case MEDIT_EXP:
        GET_EXP(OLC_MOB(d)) = MAX(0, atoi(arg));
        break;

    case MEDIT_GOLD:
        GET_GOLD(OLC_MOB(d)) = MAX(0, atoi(arg));
        break;

    case MEDIT_POS:
        GET_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS - 1, atoi(arg)));
        break;

    case MEDIT_DEFAULT_POS:
        GET_DEFAULT_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS - 1, atoi(arg)));
        break;

    case MEDIT_ATTACK:
        GET_ATTACK(OLC_MOB(d)) = MAX(0, MIN(NUM_ATTACK_TYPES - 1, atoi(arg)));
        break;

    case MEDIT_LEVEL:
        GET_LEVEL(OLC_MOB(d)) = MAX(1, MIN(100, atoi(arg)));
        break;

    case MEDIT_ALIGNMENT:
        GET_ALIGNMENT(OLC_MOB(d)) = MAX(-1000, MIN(1000, atoi(arg)));
        break;

        /*-------------------------------------------------------------------*/
    default:
        /* We should never get here. */
        cleanup_olc(d, CLEANUP_ALL);
        mudlog("SYSERR: OLC: medit_parse(): Reached default case!", BRF, LVL_BUILDER, TRUE);
        send_to_char("Oops...\r\n", d->character);
        break;
    }
    /*-------------------------------------------------------------------*/

    /*
     * END OF CASE
     * If we get here, we have probably changed something, and now want to
     * return to main menu.  Use OLC_VAL as a 'has changed' flag
     */

    OLC_VAL(d) = 1;
    medit_disp_menu(d);
}
