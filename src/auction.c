/************************************************************************
* auction.c	An automated auctioning system based on CircleMUD 3.0	*
*		bpl12 with mobile auctioneer and queue of items.	*
*									*
* Copyright (C) 1996,1997 by George Greer				*
************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "auction.h"

/*
 * Local global variables.
 */


/*
 * Function prototypes.
 */
void            free_auction(struct auction_data * auc);
void            auction_forfeit(struct char_data * mob);
void            auction_reset(void);
int             is_on_block(struct obj_data * obj);
struct obj_data *check_obj(struct char_data * ch, struct obj_data * obj);
struct char_data *check_ch(struct char_data * ch);
void            show_auction_status(struct char_data * ch);
ACMD(do_gen_comm);
struct char_data *get_char(char *name);

/*
 * External global variables.
 */
extern struct char_data *character_list;
extern struct room_data *world;

/*
 * User tweakables.
 */
#define AUC_MOB		"Puff"
#define AUC_LIMIT	10

/* -------------------------------------------------------------------------- */

int             valid_auction(struct auction_data * ac)
{
    if (ac->ticks == AUC_NONE)
        return FALSE;
    if (check_ch(ac->seller) == NULL)
        return FALSE;
    if (check_obj(ac->seller, ac->obj) == NULL)
        return FALSE;
    return TRUE;                /* Auction is ok to go. */
}

void            auction_update(void)
{
    struct char_data *mob = NULL;

    if (auction && auction->ticks == AUC_NONE)
        while (auction && !valid_auction(auction)) {
            struct auction_data *free_auc = auction;
            auction = auction->next;
            free_auction(free_auc);
        }

    if (auction == NULL)
        return;

    /* Can't be static, the mob may have died. :)                                                                                                                              */
    mob = get_char(AUC_MOB);

    /* Seller with us? */
    if (!check_ch(auction->seller)) {
        if (mob)
            do_gen_comm(mob, "The seller is no longer with us.", 0, SCMD_AUCTION);
        auction_reset();
        return;
        /* Seller's object? */
    } else if (!check_obj(auction->seller, auction->obj)) {
        auction_forfeit(mob);
        return;
        /* Make sure bidder exists */
    } else if (auction->bidder && !check_ch(auction->bidder)) {
        if (mob)
            do_gen_comm(mob, "The bidder has left, restarting.", 0, SCMD_AUCTION);
        auction->ticks = AUC_BID;
        auction->bidder = NULL;
        return;
    }
    /* Seller and bidder exist and seller has object */
    switch (auction->ticks) {
    case AUC_NEW:
    case AUC_BID:
    case AUC_ONCE:
    case AUC_TWICE:
        if (auction->bidder)
            sprintf(buf2, "%s for %ld coin%sto %s.%s%s",
                    auction->obj->short_description, auction->bid,
                    auction->bid != 1 ? "s " : " ",
                    GET_NAME(auction->bidder),
                    auction->ticks == AUC_ONCE ? " ONCE!" : "",
                    auction->ticks == AUC_TWICE ? " TWICE!!" : "");
        else if (auction->ticks == AUC_TWICE) {
            sprintf(buf2, "%s withdrawn, no interest.",
                    auction->obj->short_description);
            auction_reset();
            auction->ticks--;
        } else
            sprintf(buf2, "%s%s, %ld coin%sminimum.",
                    auction->ticks == AUC_NEW ? "New item: " : "",
                    auction->obj->short_description, auction->bid, auction->bid != 1 ?
                    "s " : " ");
        if (mob)
            do_gen_comm(mob, buf2, 0, SCMD_AUCTION);
        auction->ticks++;
        return;

    case AUC_SOLD:
        sprintf(buf2, "%s SOLD to %s for %ld coin%s",
                auction->obj->short_description,
                GET_NAME(auction->bidder),
                auction->bid, auction->bid != 1 ? "s." : ".");
        if (mob)
            do_gen_comm(mob, buf2, 0, SCMD_AUCTION);

        /* Make sure object exists */
        if ((auction->obj = check_obj(auction->seller, auction->obj))) {
            /* Swap gold */
            GET_GOLD(auction->bidder) -= auction->bid;
            GET_GOLD(auction->seller) += auction->bid;
            obj_from_char(auction->obj);
            obj_to_char(auction->obj, auction->bidder);

            act("A small daemon pops in, takes $p, hands you some gold, and leaves.",
                FALSE, auction->seller, auction->obj, 0, TO_CHAR);
            act("A small daemon pops in, gives you $p, takes some gold, and leaves.",
                FALSE, auction->bidder, auction->obj, 0, TO_CHAR);
            if (auction->bidder->in_room == auction->seller->in_room)
                act("A small daemon pops in, takes $p from $n, takes some gold "
                    "from $N, gives $p to $N, and gives some gold to $n.",
                    FALSE, auction->seller, auction->obj, auction->bidder,
                    TO_NOTVICT);
            else {
                act("A small daemon pops in, takes $p and gives some gold from $n, and leaves.",
                    FALSE, auction->seller, auction->obj, auction->bidder,
                    TO_ROOM);
                act("A small daemon pops in, takes some gold and gives $p to $n, and leaves.",
                    FALSE, auction->bidder, auction->obj, auction->bidder,
                    TO_ROOM);
            }
        } else
            auction_forfeit(mob);
        auction_reset();
        return;
    }
}

ACMD(do_bid)
{
    long            bid;
    struct char_data *mob = get_char((char *) AUC_MOB);
    int             num = 0,
                          i = 0;
    struct auction_data *auc = auction;

    two_arguments(argument, buf, buf1);
    bid = atoi(buf);
    if (*buf1) {
        if (*buf1 == '#') {
            if ((num = atoi(buf1 + 1)) != 0)
                for (auc = auction, i = 0; auc && i != num; auc = auc->next, i++)
                    if (!auc->next)
                        break;
        } else
            for (auc = auction, i = 0; auc && i < AUC_LIMIT; auc = auc->next, i++) {
                if (!auc->obj) {
                    log("do_bid: ack, no object");
                    break;
                } else if (isname(buf1, auc->obj->name)) {
                    num = i;
                    break;
                }
            }
    }
    if (i != num) {
        send_to_char("That number does not exist.\r\n", ch);
        return;
    } else if (i == 0)
        *buf1 = '\0';

    if (auc->ticks == AUC_NONE) {
        send_to_char("Nothing is up for sale.\r\n", ch);
        return;
    }
    if (!*buf) {
        sprintf(buf2, "Current bid: %ld coin%s\r\n", auc->bid,
                auc->bid != 1 ? "s." : ".");
        send_to_char(buf2, ch);
    } else if (ch == auc->bidder)
        send_to_char("You're trying to outbid yourself.\r\n", ch);
    else if (ch == auc->seller)
        send_to_char("You can't bid on your own item.\r\n", ch);
    else if (!isname(buf1, auc->obj->name) && *buf1 && num == 0)
        send_to_char("That object is not for sale currently.\r\n", ch);
    else if (bid < auc->bid && !auc->bidder) {
        sprintf(buf2, "You have to bid over the minimum of %ld coin%s\r\n",
                auc->bid, auc->bid != 1 ? "s." : ".");
        send_to_char(buf2, ch);
    } else if ((bid < (auc->bid * 1.05) && auc->bidder) || bid == 0) {
        sprintf(buf2, "Try bidding at least 5%% over the current bid of %ld. (%.0f coins).\r\n",
                auc->bid, auc->bid * 1.05 + 1);
        send_to_char(buf2, ch);
    } else if (GET_GOLD(ch) < bid) {
        sprintf(buf2, "You have only %d coins on hand.\r\n", GET_GOLD(ch));
        send_to_char(buf2, ch);
    } else if (PLR_FLAGGED(ch, PLR_NOSHOUT))
        send_to_char("You can't auction.\r\n", ch);
    else if (mob == NULL)
        send_to_char("The auctioneer seems to be dead.\r\n", ch);
    else {
        if (auc->bidder) {
            sprintf(buf, "%s has placed a %ld bid over your %ld bid for %s.\r\n",
                    GET_NAME(ch), bid, auc->bid, auc->obj->short_description);
            send_to_char(buf, auc->bidder);
        }
        auc->bid = bid;
        auc->bidder = ch;
        auc->ticks = AUC_BID;
        if (i == 0) {
            sprintf(buf2, "%ld coin", bid);
            if (auc->bid != 1)
                strcat(buf2, "s.");
            else
                strcat(buf2, ".");
            do_gen_comm(ch, buf2, 0, SCMD_AUCTION);
        } else {
            sprintf(buf, "A %ld coin bid placed on %s.\r\n", bid,
                    auc->obj->short_description);
            send_to_char(buf, ch);
        }
    }
}

ACMD(do_auction)
{
    struct obj_data *obj;
    struct char_data *mob = get_char(AUC_MOB);
    struct auction_data *auc_add;
    int             i = 1;

    two_arguments(argument, buf1, buf2);

    if (!*buf1) {
        send_to_char("Format: auction item [min]\r\n"
                     "        auction list/status\r\n", ch);
        if (GET_LEVEL(ch) >= LVL_GOD)
            send_to_char("        auction stop\r\n", ch);
        send_to_char("Auction what for what minimum?\r\n", ch);
    } else if (isname(buf1, "list status info"))
        show_auction_status(ch);
    else if (GET_LEVEL(ch) >= LVL_GOD && is_abbrev(buf1, "stop"))
        auction_reset();
    else if ((obj = get_obj_in_list_vis(ch, buf1, ch->carrying)) == NULL) {
        send_to_char("You don't seem to have that to sell.\r\n", ch);
    } else if (is_on_block(obj))
        send_to_char("You are already auctioning that!\r\n", ch);
    else if (mob == NULL)
        send_to_char("The auctioneer seems to be dead.\r\n", ch);
    else {
        struct auction_data *n_auc;
        for (n_auc = auction; n_auc && n_auc->next; n_auc = n_auc->next, i++);
        if (i >= AUC_LIMIT) {
            sprintf(buf, "Sorry, only %d items allowed on the block at a time.\r\n", AUC_LIMIT);
            send_to_char(buf, ch);
            return;
        }
        CREATE(auc_add, struct auction_data, 1);
        auc_add->ticks = AUC_NEW;
        auc_add->seller = ch;
        auc_add->bid = (atoi(buf2) != 0 ? (atoi(buf2)<GET_OBJ_COST(obj)?GET_OBJ_COST(obj):atoi(buf2)) : GET_OBJ_COST(obj));
        auc_add->obj = obj;
        auc_add->bidder = NULL;
        auc_add->next = NULL;
        if (n_auc)
            n_auc->next = auc_add;
        else
            auction = auc_add;  /* Making the list. */
        sprintf(buf2, "Auction noted, currently %d item%sahead of yours.\r\n",
                i, i == 1 ? " " : "s ");
        send_to_char(buf2, ch);
    }
}

void            auction_reset(void)
{
    auction->bidder = NULL;
    auction->seller = NULL;
    auction->obj = NULL;
    auction->ticks = AUC_NONE;
    auction->bid = 0;
    auction->spec = 0;
}

struct obj_data *check_obj(struct char_data * ch, struct obj_data * obj)
{
    struct obj_data *ch_obj;

    for (ch_obj = ch->carrying; ch_obj; ch_obj = ch_obj->next_content)
        if (ch_obj->item_number == obj->item_number)
            return ch_obj;

    return NULL;
}

struct char_data *check_ch(struct char_data * ch)
{
    register struct char_data *tch;

    if (!ch)
        return NULL;

    for (tch = character_list; tch; tch = tch->next)
        if (ch == tch)
            return tch;

    return NULL;
}

void            auction_forfeit(struct char_data * mob)
{
    /* Item not found */
    if (mob) {
        sprintf(buf2, "%s no longer holds object, auction is forfeit.", GET_NAME(auction->seller));
        do_gen_comm(mob, buf2, 0, SCMD_AUCTION);
    }
    send_to_char("A small daemon pops in, takes some gold, and taunts you.\r\n", auction->seller);
    act("A small daemon pops in, takes some gold from $n, and sticks its tongue out at $m.",
        FALSE, auction->seller, auction->obj, 0, TO_ROOM);
    GET_GOLD(auction->seller) -= auction->bid;
    auction_reset();
}

void            show_auction_status(struct char_data * ch)
{
    struct auction_data *auc;
    int             i = 1;

    *buf = '\0';

    send_to_char("Items up for auction:\r\n", ch);
    for (auc = auction, i = 0; auc; auc = auc->next, i++)
        if (auc->seller)
            sprintf(buf + strlen(buf), "%d. %s auctioning %s for %ld coin%sto %s.\r\n",
                    i, PERS(auc->seller, ch), auc->obj->short_description,
                    auc->bid, auc->bid == 1 ? " " : "s ",
                    auc->bidder ? GET_NAME(auc->bidder) : "no one");

    send_to_char(*buf ? buf : "  Nothing.\r\n", ch);
}

int             is_on_block(struct obj_data * obj)
{
    struct auction_data *auc;

    for (auc = auction; auc; auc = auc->next)
        if (auc->obj == obj)
            return 1;

    return 0;
}

void            free_auction(struct auction_data * auc)
{
    DISPOSE(auc);
}

struct char_data *get_char(char *name)
{
    struct char_data *i;
    int             j = 0,
                        gcnumber;
    char            tmpname[MAX_INPUT_LENGTH];
    char           *tmp = tmpname;

    strcpy(tmp, name);
    if (!(gcnumber = get_number(&tmp)))
        return NULL;

    for (i = character_list; i && (j <= gcnumber); i = i->next)
        if (isname(tmp, i->player.name))
            if (++j == gcnumber)
                return i;

    return NULL;
}
