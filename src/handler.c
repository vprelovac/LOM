/* ************************************************************************
*   File: handler.c                                     Part of CircleMUD *
*  Usage: internal funcs: moving and finding chars/objs                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"
#include "auction.h"

/* external vars */
extern int      top_of_world;
extern int      pulse;
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern char    *MENU;
extern char    *DEATH_MESSG;
extern struct race_app_type race_app[];
/* external functions */
void            free_char(struct char_data * ch);
void            stop_fighting(struct char_data * ch);
void            remove_follower(struct char_data * ch);
void            clearMemory(struct char_data * ch);
extern char *spells[MAX_SKILLS+1];
extern char *spell_wear_off_msg[];
struct obj_ref_type *obj_ref_list;
struct zone_data *zone_table;   /* zone table			 */

CHAR_DATA	  *	cur_char;
room_num	cur_room;
bool			cur_char_died;
EXTRACT_OBJ_DATA  *	extracted_obj_queue=NULL;
EXTRACT_CHAR_DATA 	extracted_char_queue[10000];






int cur_qchars=0, cur_qobjs=0;

void event_null_char(CHAR_DATA *ch);
void event_null_obj(OBJ_DATA *ch);
extern struct char_data *get_fightning_with(struct char_data *ch);

/*
 * Check to see if ch died recently				-Thoric
 */
bool char_died( CHAR_DATA *ch )
{
      int i;
    for (i=0; i<cur_qchars; i++)
        if ( extracted_char_queue[i].ch == ch )
            return TRUE;
    return FALSE;
}

/*
 * Add ch to the queue of recently extracted characters		-Thoric
 */
void queue_extracted_char( CHAR_DATA *ch)
{
    EXTRACT_CHAR_DATA *ccd;

    if ( !ch )
    {
        log( "queue_extracted char: ch = NULL");
        return;
    }
    if (char_died(ch))
    {
        logs( "queue_extracted char: ch double died (%s)", GET_NAME(ch));
        return;
    }

  //  CREATE( ccd, EXTRACT_CHAR_DATA, 1 );      
    
    if (IS_NPC(ch))
        ch->in_room=-1;                                   
         
    else
        SET_BIT(PLR_FLAGS(ch ), PLR_JUSTDIED);
    extracted_char_queue[cur_qchars].ch			= ch;
    //ccd->next			= extracted_char_queue;
    //extracted_char_queue	= ccd;
    cur_qchars++;
    event_null_char(ch);
}


/*
 * clean out the extracted character queue
 */
void clean_char_queue()
{
            int i;
    for ( i=0; i<cur_qchars; i++)
           
    {    
        if (IS_NPC(extracted_char_queue[i].ch ))
            free_char( extracted_char_queue[i].ch );
        else
            REMOVE_BIT(PLR_FLAGS(extracted_char_queue[i].ch ), PLR_JUSTDIED);     
        
    }   
    cur_qchars=0;
}


bool obj_extracted(OBJ_DATA *obj )
{
    EXTRACT_OBJ_DATA *ccd;

    for (ccd = extracted_obj_queue; ccd; ccd = ccd->next )
        if ( ccd->obj == obj )
            return TRUE;
    return FALSE;
}

/*
 * Add ch to the queue of recently extracted characters		-Thoric
 */
void queue_extracted_obj( OBJ_DATA *obj)
{
    EXTRACT_OBJ_DATA *ccd;

    if ( !obj )
    {
        log( "queue_extracted obj: obj = NULL");
        return;
    }
    if (obj->in_room==-2)
    {
        logs( "queue_extracted obj: obj double extracted (%s)", obj->short_description);
        return;
    }

    CREATE( ccd, EXTRACT_OBJ_DATA, 1 );
    obj->in_room=-2;
    ccd->obj			= obj;
    ccd->next			= extracted_obj_queue;
    extracted_obj_queue	= ccd;
    cur_qobjs++;
    event_null_obj(obj);
}

void clean_obj_queue()
{
    EXTRACT_OBJ_DATA *ccd;

    for ( ccd = extracted_obj_queue; ccd; ccd = extracted_obj_queue )
    {
        extracted_obj_queue = ccd->next;
        free_obj( ccd->obj );
        DISPOSE( ccd );
        --cur_qobjs;
    }
}





void            obj_reference(struct obj_ref_type * ref)
{
    if (ref->inuse) {
        log("Reused obj_reference!");
        //        abort();
        return;
    }
    ref->inuse = TRUE;
    ref->next = obj_ref_list;
    obj_ref_list = ref;
}

void            obj_unreference(struct obj_data ** var)
{
    struct obj_ref_type *p,
                *last;

    for (p = obj_ref_list, last = NULL;
            p && p->var != var;
            last = p, p = p->next);

if (!p) {
        log("obj_unreference: var not found");
        return;
    }
    p->inuse = FALSE;

    if (!last)
        obj_ref_list = obj_ref_list->next;
    else
        last->next = p->next;
}

struct char_ref_type *char_ref_list;

void            char_reference(struct char_ref_type * ref)
{
    if (ref->inuse) {
        log("Reused char_reference!");
        //        abort();
        return;
    }
    ref->inuse = TRUE;
    ref->next = char_ref_list;
    char_ref_list = ref;
}

void            char_unreference(struct char_data ** var)
{
    struct char_ref_type *p,
                *last;

    for (p = char_ref_list, last = NULL;
            p && p->var != var;
            last = p, p = p->next);

if (!p) {
        log("char_unreference: var not found");
        return;
    }
    p->inuse = FALSE;

    if (!last)
        char_ref_list = char_ref_list->next;
    else
        last->next = p->next;
}

char           *fname(char *namelist)
{
    static char     holder[30];
    register char  *point;

    for (point = holder; isalpha(*namelist); namelist++, point++)
        *point = *namelist;

    *point = '\0';

    return (holder);
}

/*


int isname(const char *str, const char *namelist)
{
  const char *curname, *curstr;

  curname = namelist;
  for (;;) {
    for (curstr = str;; curstr++, curname++) {
      if (!*curstr && !isalpha(*curname))
	return (1);

      if (!*curname)
	return (0);

      if (!*curstr || *curname == ' ')
	break;

      if (LOWER(*curstr) != LOWER(*curname))
	break;
    }

    //skip to next name 

    for (; isalpha(*curname); curname++);
    if (!*curname)
      return (0);
    curname++;			// first char of new name
  }
}
*/

#define WHITESPACE " \t"

int             isname(char *str, char *namelist)
{
    char           *newlist;
    char           *curtok;

    if (!namelist || !str)
        return 0;

    newlist = strdup(namelist); /* make a copy since strtok 'modifies'
                                 * strings */

    for (curtok = strtok(newlist, WHITESPACE); curtok; curtok = strtok(NULL,
            WHITESPACE))
        if (curtok && is_abbrev(str, curtok)) {
            DISPOSE(newlist);
            return 1;
        }
    DISPOSE(newlist);
    return 0;
}

int             myisname(char *str, char *namelist)
{
    register char  *curname,
    *curstr;

    curname = namelist;
    for (;;) {
        for (curstr = str;; curstr++, curname++) {
            /*      if (!*curstr && !isalpha(*curname))
                    return (1);

                  if (!*curname)
                    return (0);

                  if (!*curstr || *curname == ' ')
                    break;
            */
            if (!*curstr)
                return (1);

            if (LOWER(*curstr) != LOWER(*curname))
                break;
        }

        /* skip to next name */

        for (; isalpha(*curname); curname++);
        if (!*curname)
            return (0);
        curname++;              /* first char of new name */
    }
}


void            affect_modify(struct char_data * ch, int loc, int mod, long bitv,
                              long bitv2, long bitv3, bool add)
{
    int             maxabil;

    if (add) {
        SET_BIT(AFF_FLAGS(ch), bitv);
        SET_BIT(AFF2_FLAGS(ch), bitv2);
        SET_BIT(AFF3_FLAGS(ch), bitv3);
    } else {
        REMOVE_BIT(AFF_FLAGS(ch), bitv);
        REMOVE_BIT(AFF2_FLAGS(ch), bitv2);
        REMOVE_BIT(AFF3_FLAGS(ch), bitv3);
        mod = -mod;
    }


    maxabil = 25;
    if (loc<0)
    {
        if (GET_SKILL(ch , -loc))
            SET_SKILL(ch, -loc, GET_SKILL(ch, -loc)+mod);
        return;
    }


    switch (loc) {
    case APPLY_NONE:
        break;

    case APPLY_STR:
        GET_STR(ch) += mod;
        break;
    case APPLY_DEX:
        //GET_MAX_MOVE(ch)+=mod*GET_LEVEL(ch)/4;
        GET_DEX(ch) += mod;
        break;
    case APPLY_INT:
        //GET_MAX_MANA(ch)+=mod*GET_LEVEL(ch);
        GET_INT(ch) += mod;
        break;
    case APPLY_WIS:
        GET_WIS(ch) += mod;
        break;
    case APPLY_CON:
        //GET_MAX_HIT(ch)+=mod*GET_LEVEL(ch);
        GET_CON(ch) += mod;
        break;
    case APPLY_CHA:
        GET_CHA(ch) += mod;
        break;

    case APPLY_CLASS:
        /* GET_CLASS(ch) == mod; */
        break;

    case APPLY_LEVEL:
        /* GET_LEVEL(ch) = MIN(GET_LEVEL(ch) + mod, 30); */
        break;

    //case APPLY_AGE:
      //  ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);
//        break;

    case APPLY_CHAR_WEIGHT:
        GET_WEIGHT(ch) += mod;
        break;

    case APPLY_CHAR_HEIGHT:
        GET_HEIGHT(ch) += mod;
        break;

    case APPLY_MANA:
        GET_MAX_MANA(ch) += mod;
        //       GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
        break;

    case APPLY_HIT:
        GET_MAX_HIT(ch) += mod;

        break;

    case APPLY_MOVE:
        GET_MAX_MOVE(ch) += mod;
        //     GET_MOVE(ch) = MIN(GET_MOVE(ch), GET_MAX_MOVE(ch));
        break;

    case APPLY_GOLD:
        break;

    case APPLY_EXP:
        break;

    case APPLY_AC:
        GET_MAGAC(ch) += mod;
        break;

    case APPLY_HITROLL:
        GET_HITROLL(ch) += mod;
        break;

    case APPLY_DAMROLL:
        GET_DAMROLL(ch) += mod;
        break;

    case APPLY_SAVING_PARA:
        GET_SAVE(ch, SAVING_PARA) += mod;
        break;

    case APPLY_SAVING_ROD:
        GET_SAVE(ch, SAVING_ROD) += mod;
        break;

    case APPLY_SAVING_PETRI:
        GET_SAVE(ch, SAVING_PETRI) += mod;
        break;

    case APPLY_SAVING_BREATH:
        GET_SAVE(ch, SAVING_BREATH) += mod;
        break;

    case APPLY_SAVING_SPELL:
        GET_SAVE(ch, SAVING_SPELL) += mod;
        break;

    case APPLY_REGENERATION:
        ch->regen+=mod;
        break;

    default:
        sprintf(buf, "%s %d %d", GET_NAME(ch), loc, mod);
        log(buf);
        log("SYSERR: Unknown apply adjust attempt (handler.c, affect_modify).");
        break;

    }                           /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */

int atot=0;
void            affect_total(struct char_data * ch)
{
    struct affected_type *af;
    int             i,
    j, p1, p2, p3;



    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i))
            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
                              GET_EQ(ch, i)->affected[j].modifier,
                              GET_EQ(ch, i)->obj_flags.bitvector, GET_EQ(ch, i)->obj_flags.bitvector2, GET_EQ(ch, i)->obj_flags.bitvector3, FALSE);
    }


    for (af = ch->affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector,
                      af->bitvector2, af->bitvector3, FALSE);

    ch->aff_abils = ch->real_abils;
    if (!IS_NPC(ch))
    {
        /*	//GET_MAX_MOVE(ch)=GET_MOVER(ch);
        	//GET_MAX_MANA(ch)=GET_MANAR(ch);
        	GET_MAX_HIT(ch)=GET_HITR(ch)-ch->mana_leech;*/
        GET_MAX_MOVE(ch)=GET_MOVER(ch);
        GET_MAX_MANA(ch)=GET_MANAR(ch)-ch->mana_leech;
        GET_MAX_HIT(ch)=GET_HITR(ch);
       
        	

    }
    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i))
            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
                              GET_EQ(ch, i)->affected[j].modifier,
                              GET_EQ(ch, i)->obj_flags.bitvector, GET_EQ(ch, i)->obj_flags.bitvector2, GET_EQ(ch, i)->obj_flags.bitvector3, TRUE);
    }


    for (af = ch->affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector,
                      af->bitvector2, af->bitvector3, TRUE);

    /* Make certain values are between 0..35, not < 0 and not > 35! */


    if (!IS_NPC(ch))
    {
        GET_DEX(ch) = MAX(race_app[GET_RACE(ch)].dex-10, MIN(GET_DEX(ch), MIN(25, race_app[GET_RACE(ch)].dex)));
        GET_INT(ch) = MAX(race_app[GET_RACE(ch)].intel-10, MIN(GET_INT(ch), MIN(25, race_app[GET_RACE(ch)].intel)));
        GET_WIS(ch) = MAX(race_app[GET_RACE(ch)].wis-10, MIN(GET_WIS(ch), MIN(25, race_app[GET_RACE(ch)].wis)));
        GET_CON(ch) = MAX(race_app[GET_RACE(ch)].con-10, MIN(GET_CON(ch), MIN(25, race_app[GET_RACE(ch)].con)));
        GET_STR(ch) = MAX(race_app[GET_RACE(ch)].str-10, MIN(GET_STR(ch), MIN(25, race_app[GET_RACE(ch)].str)));
        GET_CHA(ch) = MAX(race_app[GET_RACE(ch)].cha-10, MIN(GET_CHA(ch), MIN(25, race_app[GET_RACE(ch)].cha)));
        GET_MAX_MANA(ch)+=(GET_INT(ch)-GET_INTR(ch))*GET_LEVEL(ch);
        GET_MAX_MOVE(ch)+=(GET_DEX(ch)-GET_DEXR(ch))*8;
        GET_MAX_HIT(ch)+=(GET_CON(ch)-GET_CONR(ch))*GET_LEVEL(ch);
        if ((GET_MAX_MANA(ch)<0) && !atot)
        {
            atot=1;
            leech_from_char(ch, -123454321);
            atot=0;
        }
        ch->points.max_hit=GET_MAX_HIT(ch);
    }
    else
    {
        GET_DEX(ch) = MAX(1, MIN(GET_DEX(ch), 25));
        GET_INT(ch) = MAX(1, MIN(GET_INT(ch), 25));
        GET_WIS(ch) = MAX(1, MIN(GET_WIS(ch), 25));
        GET_CON(ch) = MAX(1, MIN(GET_CON(ch), 25));
        GET_STR(ch) = MAX(1, MIN(GET_STR(ch), 25));
        GET_CHA(ch) = MAX(1, MIN(GET_CHA(ch), 25));
    }

}


/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void            affect_to_char(struct char_data * ch, struct affected_type * af)
{
    struct affected_type *affected_alloc;

    CREATE(affected_alloc, struct affected_type, 1);

    *affected_alloc = *af;
    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier, af->bitvector,
                  af->bitvector2, af->bitvector3, TRUE);
    affect_total(ch);
}



void leechoff(struct char_data *ch, struct char_data *victim, int spellnum, int mana)
{
    if (ch && victim)
    {
        ch->mana_leech-=mana;
        GET_MAX_MANA(ch)+=mana;

        sprintf(buf, "You stop supporting '%s' on %s.\r\n", spells[spellnum], (victim==ch ? "yourself":GET_NAME(victim)));
        send_to_char(buf, ch);
    }
}

void add_leech(struct char_data *ch, struct char_data *victim, int spellnum, int mana)
{
    struct leech_type *affected_alloc;

    CREATE(affected_alloc, struct leech_type, 1);

    affected_alloc->from = ch;
    affected_alloc->to = victim;
    affected_alloc->type = spellnum;
    affected_alloc->mana = mana;
    ch->mana_leech+=mana;
    GET_MAX_MANA(ch)-=mana;

    affected_alloc->next = leech_list;

    leech_list = affected_alloc;

}



void            leech_from_char(struct char_data * ch, int spellnum)
{
    struct leech_type *hjp,
                *next, *temp;
    int k=0;

for (hjp = leech_list; hjp; hjp = next) {
        next = hjp->next;

        if (spellnum==SPELL_ALL)
        {
            if (hjp->to == ch)
            {
                if (hjp->to!=hjp->from)
                    leechoff(hjp->from, hjp->to, hjp->type, hjp->mana);
                REMOVE_FROM_LIST(hjp, leech_list, next);
                DISPOSE(hjp);
            }
            else if (hjp->from == ch)
            {
                //leechoff(hjp->from, hjp->to, SPELL_ALL, hjp->mana);
                if (IS_NPC(hjp->to) /*&& hjp->from==hjp->to->master*/ && MOB_FLAGGED(hjp->to, MOB_PET) && IS_CONJURING(hjp->type))
                {
                    die_follower(hjp->to);
                    act("$n perishes in a cloud of smoke!", FALSE, hjp->to, 0, 0, TO_ROOM);
                    REMOVE_FROM_LIST(hjp, leech_list, next);
                    extract_char(hjp->to);
                }
                else
                {
                    affect_from_char(hjp->to, hjp->type);
                    if (*spell_wear_off_msg[hjp->type]) {
                        send_to_char(spell_wear_off_msg[hjp->type], hjp->to);
                        send_to_char("\r\n", hjp->to);
                    }
                    REMOVE_FROM_LIST(hjp, leech_list, next);
                }

                DISPOSE(hjp);
            }
        }
        else {
            if (hjp->to == ch && spellnum>0 && hjp->type==spellnum)
            {
                //leechoff(hjp->from, hjp->to, hjp->type, hjp->mana);
                hjp->from->mana_leech-=hjp->mana;
                REMOVE_FROM_LIST(hjp, leech_list, next);
                DISPOSE(hjp);
            }
            else if (hjp->from == ch  && (--k==spellnum || spellnum==-123454321))
            {
                leechoff(hjp->from, hjp->to, hjp->type, hjp->mana);
                if (IS_NPC(hjp->to) /*&& hjp->from==hjp->to->master*/ && MOB_FLAGGED(hjp->to, MOB_PET) && IS_CONJURING(hjp->type))
                {
                    die_follower(hjp->to);
                    act("$n perishes in a cloud of smoke!", FALSE, hjp->to, 0, 0, TO_ROOM);
                    REMOVE_FROM_LIST(hjp, leech_list, next);
                    extract_char(hjp->to);
                }
                else
                {
                    affect_from_char(hjp->to, hjp->type);
                    if (*spell_wear_off_msg[hjp->type]) {
                        send_to_char(spell_wear_off_msg[hjp->type], hjp->to);
                        send_to_char("\r\n", hjp->to);
                    }
                    REMOVE_FROM_LIST(hjp, leech_list, next);
                }

                DISPOSE(hjp);
            }
        }
    }
}


void show_leech(struct char_data *ch)
{
    struct leech_type *hjp,
                *next;
    int k=1;
    int total=0;
    *buf=0;
    send_to_char(" ##  Spell                      Target                          Mana leech\r\n",ch);
    send_to_char("--------------------------------------------------------------------------\r\n\r\n",ch);
for (hjp = leech_list; hjp; hjp = next) {
        next = hjp->next;

        if (hjp->from==ch)
        {
            sprintf(buf1, spells[hjp->type]);
            sprintf(buf2, (hjp->to==hjp->from? "Self":GET_NAME(hjp->to)));
            CAP(buf1);
            CAP(buf2);
            sprintf(buf,"%s&c%3d&0) &C%-25s&0  &c%-30s&0     &G%3d&0\r\n", buf, k++, buf1, buf2, hjp->mana);
            total+=hjp->mana;
        }
    }
    if (k>1)
    {
        send_to_char(buf, ch);
        sprintf(buf, "\r\nTotal %d mana leeched.\r\n", total);
        send_to_char(buf, ch);
    }
    else
    {
        send_to_char("     &CNone.&0\r\n", ch);
        if (ch->mana_leech)
            send_to_char("     Something's goofed! Report this as a bug!.\r\n", ch);
    }

}



/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */
void            affect_remove(struct char_data * ch, struct affected_type * af)
{
    struct affected_type *temp;

    assert(ch->affected);
    /*  if (af->type==SPELL_WRATH_OF_GOD)
      {
      if (WRATHOBJ(ch)!=NULL){
      act("$p slowly dissolves.", FALSE, ch, WRATHOBJ(ch), 0, TO_ROOM);
      act("$p slowly dissolves.", FALSE, ch, WRATHOBJ(ch), 0, TO_CHAR);
      extract_obj(WRATHOBJ(ch));
      WRATHOBJ(ch)=NULL;}}   */
    affect_modify(ch, af->location, af->modifier, af->bitvector,
                  af->bitvector2, af->bitvector3, FALSE);
    //if (af->type==SPELL_DEATHDANCE)
    //  GET_HIT(ch)=MIN(GET_MAX_HIT(ch), GET_HIT(ch)+GET_MAX_HIT(ch)/2);
    REMOVE_FROM_LIST(af, ch->affected, next);
    DISPOSE(af);
    affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */

void            affect_from_char(struct char_data * ch, sh_int type)
{
    struct affected_type *hjp,
                *next;

for (hjp = ch->affected; hjp; hjp = next) {
        next = hjp->next;
        if (hjp->type == type)
            affect_remove(ch, hjp);
    }
}



/*
 * Return if a char is affected by a spell (SPELL_XXX), NULL indicates
 * not affected
 */
bool            affected_by_spell(struct char_data * ch, sh_int type)
{
    struct affected_type *hjp;

    for (hjp = ch->affected; hjp; hjp = hjp->next)
        if (hjp->type == type)
            return TRUE;

    return FALSE;
}

void            affect_join(struct char_data * ch, struct affected_type * af,
                            bool add_dur, bool avg_dur, bool add_mod, bool avg_mod)
{
    struct affected_type *hjp, *next;
    bool            found = FALSE;



    for (hjp = ch->affected; !found && hjp; hjp = next) {
    	next=hjp->next;

        if ((hjp->type == af->type) && (hjp->location == af->location)) {
            if (add_dur)
                af->duration += hjp->duration;
            if (avg_dur)
                af->duration >>= 1;

            if (add_mod)
                af->modifier += hjp->modifier;
            if (avg_mod)
                af->modifier >>= 1;

            affect_remove(ch, hjp);
            affect_to_char(ch, af);
            found = TRUE;
        }
    }
    if (!found)
        affect_to_char(ch, af);
}


/* move a player out of a room */
void            char_from_room(struct char_data * ch)
{
    struct char_data *temp, *tch, *next_tch;

    if (ch == NULL || ch->in_room == NOWHERE) {
        log("SYSERR: NULL or NOWHERE in handler.c, char_from_room");
        exit(1);
    }
    
    if (!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_NOHASSLE)) ch->last_zone=world[ch->in_room].zone;
    if (FIGHTING(ch)) {
        for (tch = world[ch->in_room].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if (FIGHTING(tch)==ch)
                stop_fighting(tch);
        }
        stop_fighting(ch);
    }

    /*//if (WRATHOBJ(ch)!=NULL)*/
    /*//{obj_from_room(WRATHOBJ(ch));act("A small, dark cloud follows $n.",FALSE,ch,0,0,TO_ROOM);}*/

    if (GET_EQ(ch, WEAR_LIGHT) != NULL)
        if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
            if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2)) /* Light is ON */
                world[ch->in_room].light--;

    REMOVE_FROM_LIST(ch, world[ch->in_room].people, next_in_room);
    ch->in_room = NOWHERE;
    ch->next_in_room = NULL;
    
}


/* place a character in a room */

void            char_to_room(struct char_data * ch, int room)
{
    struct char_data *vmob;
    memory_rec     *names;
    int i,j;
    byte pom;
    int found=0;       
    

    if (ch == NULL || room < 0 || room > top_of_world)
        log("SYSERR: Illegal value(s) passed to char_to_room");
    else {
        ch->next_in_room = world[room].people;
        world[room].people = ch;
        ch->in_room = room;
        /*//if (WRATHOBJ(ch)!=NULL)*/
        /*//{obj_to_room(WRATHOBJ(ch),room);send_to_char("A small, dark cloud follows you.\r\n",ch);}*/

        if (ch->in_room!=NOWHERE)
        {
            /*if (IN_ARENA(ch)) {
            for (vmob = world[ch->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
                if (((RED(ch) && BLUE(vmob)) || (BLUE(ch) && RED(vmob))) && !PRF_FLAGGED(vmob, PRF_NOHASSLE)) {
                    sprintf(buf, "%s engages in mortal combat with %s!", GET_NAME(ch), GET_NAME(vmob));
                    sportschan(buf);
                    if (!FIGHTING(ch)) set_fighting(ch, vmob);
                    if (!FIGHTING(vmob)) set_fighting(vmob,ch);
                
                }
            return;
        }*/

 if (!PRF_FLAGGED(ch, PRF_NOHASSLE))
 {
            if (GET_EQ(ch, WEAR_LIGHT))
                if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
                    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2))     /* Light ON */
                    {
                        if (IS_DARK(room))
                            send_to_char("Your light illuminates the surroundings.\r\n",ch);
                        world[room].light++;
                    }
             if (!IS_NPC(ch) && (ch->last_zone!=world[ch->in_room].zone))
             {                                                                                                     
             	i=world[ch->in_room].zone;
             	ch_printf(ch, "&CYou now enter %s.&0\r\n", zone_table[world[ch->in_room].zone].name);
             	//if (((zone_table[i].avg+(area_info[i].avg-area_info[i].max-area_info[i].min)/(area_info[i].num-2))/2)>GET_LEVEL(ch)+4)
             	//	ch_printf("&PIt might be dangerous for you here.
             }
            if (!IS_NPC(ch))
            {
                i=room/8;
                j=room % 8;
                pom=GET_VISITED(ch, i);
                if (!(pom & (1<<j)) && !PRF_FLAGGED(ch, PRF_BRIEF))
                {
                    pom=pom | (1<<j);
                    GET_VISITED(ch, i)=pom;
                    send_to_char("You gain adventure points for exploring.\r\n", ch);
                    if (FOL_AZ(ch))
                    {
                    	send_to_char("Az rewards your progress.\r\n", ch);
                    	gain_exp(ch, 50);
                    }
                    
                    GET_QUESTPOINTS(ch)+=MAX(1, zone_table[world[room].zone].avg/10);
                }
            }
            if (!RM_BLOOD(ch->in_room) && GET_HIT(ch)<GET_MAX_HIT(ch)/5)
                RM_BLOOD(ch->in_room) = 1;  // add blood to the room

}


            if (!IS_NPC(ch) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && !(ch->in_room==NOWHERE))
            {
                for (vmob = world[ch->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
                    if (IS_NPC(vmob) && !FIGHTING(vmob) && CAN_SEE(vmob,ch) && GET_POS(vmob)>POS_SLEEPING && !PRF_FLAGGED(ch, PRF_NOHASSLE)) {
                        if (MOB_FLAGGED(vmob, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN) && (!MOB_FLAGGED(vmob, MOB_AGGR_TO_ALIGN) ||
                                (MOB_FLAGGED(vmob, MOB_AGGR_EVIL) && IS_EVIL(ch)) ||
                                (MOB_FLAGGED(vmob, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(ch)) ||
                                (MOB_FLAGGED(vmob, MOB_AGGR_GOOD) && IS_GOOD(ch)))) {
                                if (AFF_FLAGGED(ch, AFF_SNEAK) && GET_SKILL(ch, SKILL_SNEAK)<number(1, 111))
                                	improve_skill(ch, SKILL_SNEAK, 10);
                                else
                                {
                        	
                                	
                            if (number(7, (PRF2_FLAGGED(ch, PRF2_RUNNING)?60:35)) <= GET_CHA(ch)*GET_CHA(ch)/21) {
                                act("$n looks at $N with an &Rindifference&0.\r\n",
                                    FALSE, vmob, 0, ch, TO_NOTVICT);
                                act("$N looks at you with an &Rindifference&0.\r\n",
                                    FALSE, ch, 0, vmob, TO_CHAR);
                            } else {
                                //   hit(vmob, ch, TYPE_HIT);
                                set_fighting(vmob,ch);
                                if (!FIGHTING(ch)) set_fighting(ch, vmob);
                            }    
                            }
                        }
                        else if (MOB_FLAGGED(vmob, MOB_MEMORY) && MEMORY(vmob))
                            for (names = MEMORY(vmob); names && !found; names = names->next)
                                if (names->id == GET_IDNUM(ch)) {
                                    found = TRUE;
                                    //                      act("$n growls and lunges for your throat!!!",
                                    //                        FALSE, vmob, 0, ch, TO_VICT);
                                    //                        hit(vmob, ch, TYPE_HIT);
                                    if (AFF_FLAGGED(ch, AFF_SNEAK) && GET_SKILL(ch, SKILL_SNEAK)<number(1,111))
                                    	improve_skill(ch, SKILL_SNEAK, 10);
                                    else if (!IS_NPC(ch) && GET_SKILL(ch, SKILL_DISGUISE)>number(1, 101))
                                        improve_skill(ch, SKILL_DISGUISE, 1);
                                    else if (GET_LEVEL(vmob)>5)
                                    {
                                        act("&Y'Hey!  You're the fiend that attacked me!!!', exclaims $n.&0",FALSE, vmob, 0, 0, TO_ROOM);
                                        set_fighting(vmob,ch);
                                        if (!FIGHTING(ch))
                                            set_fighting(ch, vmob);
                                    }
                                }
                    }
            }
            else if (!ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)  && (MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN) || (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch))))
            {
                for (vmob = world[ch->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
                    if (!IS_NPC(vmob) && !FIGHTING(vmob) && CAN_SEE(ch,vmob) && GET_POS(ch)>POS_SLEEPING && !PRF_FLAGGED(vmob, PRF_NOHASSLE)) {
                        if (MOB_FLAGGED(ch, MOB_AGGRESSIVE | MOB_AGGR_TO_ALIGN) && (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) ||
                                (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vmob)) ||
                                (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vmob)) ||
                                (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vmob)))) {
                            if (number(7, 55) <= GET_CHA(vmob)*GET_CHA(vmob)/21) {
                                //                    if (!(AFF_FLAGGED(ch, AFF_SNEAK) && !CAN_SEE_HIDDEN(vmob))){
                                act("$n looks at $N with an &Rindifference&0.\r\n",
                                    FALSE, ch, 0, vmob, TO_NOTVICT);
                                act("$N looks at you with an &Rindifference&0.\r\n",
                                    FALSE, vmob, 0, ch, TO_CHAR);
                                //                          }
                            } else {
                                //                if (!(AFF_FLAGGED(ch, AFF_SNEAK) && !CAN_SEE_HIDDEN(vmob))) {
                                if (!FIGHTING(ch))  set_fighting(ch, vmob);
                                if (!FIGHTING(vmob)) set_fighting(vmob,ch);
                                //               }
                            }
                        }
                        else if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch))
                            for (names = MEMORY(ch); names && !found; names = names->next)
                                if (names->id == GET_IDNUM(vmob)) {
                                    found = TRUE;
                                    //                        act("$n growls and lunges for your throat!!!",
                                    //                           FALSE, ch, 0, vmob, TO_VICT);
                                    //                     hit(ch, vmob, TYPE_HIT);
                                    if (!IS_NPC(ch) && GET_SKILL(vmob, SKILL_DISGUISE)>number(1, 101))
                                        improve_skill(vmob, SKILL_DISGUISE, 1);
                                    else
                                    {
                                        if (GET_LEVEL(ch)>7)
                                            act("'Hey!  You're the fiend that attacked me!!!', exclaims $n.",FALSE, ch, 0, 0, TO_ROOM);
                                        if (!FIGHTING(ch))
                                            set_fighting(ch, vmob);
                                        if (!FIGHTING(vmob))
                                            set_fighting(vmob,ch);
                                    }


                                }
                    }

            }
        }




        /* --------------------- NEW CODE added below ------------------ */
        /* the rest of this procedure hasn't been changed */
        /*
                if (!IS_NPC(ch) && world[ch->in_room].tele != NULL) {
                    if (world[ch->in_room].tele->cnt > 0 && world[ch->in_room].tele->time == 0) {
                        // this is a teleport countdown room 
                        world[ch->in_room].tele->time = pulse + world[ch->in_room].tele->cnt;   // now round up 
                        if (world[ch->in_room].tele->time % 10)
                            world[ch->in_room].tele->time += 10 - (world[ch->in_room].tele->time % 10);
                        if (world[ch->in_room].tele->time > 2400) {
                            world[ch->in_room].tele->time = world[ch->in_room].tele->cnt;
                            // start of next day 
                        }
                    }
                }*/
    }
}


/* give an object to a char   */
void            obj_to_char(struct obj_data * object, struct char_data * ch)
{
    if (object && ch) {
        object->next_content = ch->carrying;
        ch->carrying = object;
        object->carried_by = ch;
        object->in_room = NOWHERE;
        IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
        IS_CARRYING_N(ch)++;

        if (IS_OBJ_STAT(object, ITEM_AUTOENGRAVE) && !IS_OBJ_STAT(object, ITEM_ENGRAVED) && !IS_NPC(ch)) {
            strcpy(object->owner_name, GET_NAME(ch));
            SET_BIT(GET_OBJ_EXTRA(object), ITEM_ENGRAVED);
        }
        /* set flag for crash-save system */
        if (!IS_NPC(ch))
            SET_BIT(PLR_FLAGS(ch), PLR_CRASH);
    } else
        log("SYSERR: NULL obj or char passed to obj_to_char");
}


/* take an object from a char */
void            obj_from_char(struct obj_data * object)
{
    struct obj_data *temp;

    if (object == NULL) {
        log("SYSERR: NULL object passed to obj_from_char");
        return;
    }
    REMOVE_FROM_LIST(object, object->carried_by->carrying, next_content);

    /* set flag for crash-save system, but not on mobs! */
    if (!IS_NPC(object->carried_by))
        SET_BIT(PLR_FLAGS(object->carried_by), PLR_CRASH);

    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(object->carried_by)--;
    object->carried_by = NULL;
    object->next_content = NULL;
}



/* Return the effect of a piece of armor in position eq_pos */
int             apply_ac(struct char_data * ch, int eq_pos)
{
    int             factor;

    assert(GET_EQ(ch, eq_pos));

    if ((GET_OBJ_TYPE(GET_EQ(ch, eq_pos)) != ITEM_ARMOR))
        return 0;
    return (GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0));

    switch (eq_pos) {

    case WEAR_BODY:
        factor = 3;
        break;                  /* 30% */
    case WEAR_HEAD:
        factor = 2;
        break;                  /* 20% */
    case WEAR_LEGS:
        factor = 2;
        break;                  /* 20% */
    default:
        factor = 1;
        break;                  /* all others 10% */
    }

    return (factor * GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0));
}



void            equip_char(struct char_data * ch, struct obj_data * obj, int pos)
{
    int             j;
    int             invalid_class(struct char_data * ch, struct obj_data * obj);
    int             invalid_race(struct char_data * ch, struct obj_data * obj);

    assert(pos >= 0 && pos < NUM_WEARS);

    if (GET_EQ(ch, pos)) {
        sprintf(buf, "SYSERR: Char is already equipped: %s, %s", GET_NAME(ch),
                obj->short_description);
        log(buf);
        return;
    }
    if (obj->carried_by) {
        log("SYSERR: EQUIP: Obj is carried_by when equip.");
        return;
    }
    if (obj->in_room != NOWHERE) {
        log("SYSERR: EQUIP: Obj is in_room when equip.");
        return;
    }
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
            (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
            (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
        act("You are ZAPPED by $p and instantly let go of it.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n is zapped by $p and instantly lets go of it.", FALSE, ch, obj, 0, TO_ROOM);
        obj_to_char(obj, ch);   /* changed to drop in inventory instead of
                                 * ground */
        oprog_zap_trigger(ch, obj);  /* mudprogs */
        return;
    }


    if (!IS_NPC(ch) && IS_CLERIC(ch) && !IS_GOD(ch) && GET_OBJ_TYPE(obj)==ITEM_WEAPON && ((GET_OBJ_VAL(obj, 3) == TYPE_SLASH - TYPE_HIT) ||
            (GET_OBJ_VAL(obj, 3) == TYPE_SLASH - TYPE_PIERCE))) {
        act("Your deity prevents you from using sharp weapons such as $p.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n removes $p.",FALSE, ch, obj, NULL, TO_ROOM);
        obj_to_char(obj, ch);
        return;
    }

    if (invalid_class(ch, obj)) {
        act("Your suddenly feel very strange and decide not to use $p.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n removes $p.",FALSE, ch, obj, NULL, TO_ROOM);
        obj_to_char(obj, ch);
        return;
    }
    if (invalid_race(ch, obj)) {
        act("Your suddenly feel very strange and decide not to use $p.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n removes $p.",FALSE, ch, obj, NULL, TO_ROOM);
        obj_to_char(obj, ch);
        return;
    }

    GET_EQ(ch, pos) = obj;
    obj->worn_by = ch;
    obj->worn_on = pos;
    IS_EQUIP_W(ch)+=GET_OBJ_WEIGHT(obj);

    if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)// && (IS_NPC(ch) || (pos!=WEAR_BODY && pos!=WEAR_HEAD && pos!=WEAR_ARMS && pos!=WEAR_LEGS)))
        GET_AC(ch) += apply_ac(ch, pos);

    if (ch->in_room != NOWHERE) {
        if (pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
            if (GET_OBJ_VAL(obj, 2))    /* if light is ON */
                world[ch->in_room].light++;
    } else
        log("SYSERR: ch->in_room = NOWHERE when equipping char.");

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
        affect_modify(ch, obj->affected[j].location,
                      obj->affected[j].modifier,
                      obj->obj_flags.bitvector, obj->obj_flags.bitvector2, obj->obj_flags.bitvector3, TRUE);
    affect_total(ch);
    if (!IS_NPC(ch) && GET_MAX_HIT(ch)<1)
    {
        char bufm[100];
        sprintf(bufm, "%s [%d] killed by %s at %s (%d)", GET_NAME(ch), GET_LEVEL(ch),
                "unproper use of item",
                world[ch->in_room].name, world[ch->in_room].number);
        log(bufm);
        sprintf(bufm, "\r\nINFO || %s killed by unproper use of magical item.\r\n", GET_NAME(ch));
        INFO_OUT(bufm);
        obj_to_char(unequip_char(ch, pos), ch);
        die(ch, NULL);
        return;
    }
}



struct obj_data *unequip_char(struct char_data * ch, int pos)
{
    int             j;
    struct obj_data *obj;

    assert(pos >= 0 && pos < NUM_WEARS);
    assert(GET_EQ(ch, pos));

    obj = GET_EQ(ch, pos);
    obj->worn_by = NULL;
    obj->worn_on = -1;
    IS_EQUIP_W(ch)-=GET_OBJ_WEIGHT(obj);
    if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)// && (IS_NPC(ch) || (pos!=WEAR_BODY && pos!=WEAR_HEAD && pos!=WEAR_ARMS && pos!=WEAR_LEGS)))
        GET_AC(ch) -= apply_ac(ch, pos);

    if (ch->in_room != NOWHERE) {
        if (pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
            if (GET_OBJ_VAL(obj, 2))    /* if light is ON */
                world[ch->in_room].light--;
    } else
        log("SYSERR: ch->in_room = NOWHERE when equipping char.");

    GET_EQ(ch, pos) = NULL;

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
        affect_modify(ch, obj->affected[j].location,
                      obj->affected[j].modifier,
                      obj->obj_flags.bitvector, obj->obj_flags.bitvector2,
                      obj->obj_flags.bitvector3, FALSE);
    affect_total(ch);

    return (obj);
}


int             get_number(char **name)
{
    int             i;
    char           *ppos;
    char            number[MAX_INPUT_LENGTH];
    char 	tmp[100];

    *number = '\0';
    
    if ((ppos = strchr(*name, '.'))) {
        *ppos++ = '\0';
        strcpy(number, *name);
        strcpy(tmp, ppos);
        
        strcpy(*name, tmp);

        for (i = 0; *(number + i); i++)
            if (!isdigit(*(number + i)))
                return 0;

        return (atoi(number));
    }
    return 1;
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data * list)
{
    struct obj_data *i;

    for (i = list; i; i = i->next_content)
        if (GET_OBJ_RNUM(i) == num)
            return i;

    return NULL;
}



/* search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
    struct obj_data *i;

    for (i = object_list; i; i = i->next)
        if (GET_OBJ_RNUM(i) == nr)
            return i;

    return NULL;
}



/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
    struct char_data *i;
    int             j = 0,
                        number;
    char            tmpname[MAX_INPUT_LENGTH];
    char           *tmp = tmpname;

    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return NULL;

    for (i = world[room].people; i && (j <= number); i = i->next_in_room)
        if (isname(tmp, i->player.name))
            if (++j == number)
                return i;

    return NULL;
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next)
        if (GET_MOB_RNUM(i) == nr)
            return i;

    return NULL;
}


extern int max_obj_time;
/* put an object in a room */
void            obj_to_room(struct obj_data * object, int room)
{
    if (!object || room < 0 || room > top_of_world) {
        log("SYSERR: Illegal value(s) passed to obj_to_room");
        return;
    }
    
    object->next_content = world[room].contents;
    world[room].contents = object;
    object->in_room = room;
    object->carried_by = NULL;
    if (ROOM_FLAGGED(room, ROOM_HOUSE))
        SET_BIT(ROOM_FLAGS(room), ROOM_HOUSE_CRASH);
    else if (!global_no_timer)
        {
        	GET_OBJ_TIMER(object)=max_obj_time;
        	
        	if ((SECT(room) == SECT_WATER_SWIM || SECT(room) == SECT_WATER_NOSWIM) && GET_OBJ_WEIGHT(object) > 0)
        	{
    			if (GET_OBJ_TYPE(object) == ITEM_BOAT || (GET_OBJ_TYPE(object)==ITEM_CONTAINER && GET_OBJ_VAL(object, 3) == 1))
    		            		act("$p floats on the surface.", FALSE, 0, object, 0, TO_ROOM);
    				else {
                			act("$p sinks into the murky depths.", FALSE, 0, object, 0, TO_ROOM);
                			GET_OBJ_TIMER(object)=1;
    	            			//extract_obj(object);
    	            		      }        	        	    	
   	       }
   	}       
}


/* Take an object from a room */
void            obj_from_room(struct obj_data * object)
{
    struct obj_data *temp;

    if (!object || object->in_room == NOWHERE) {
        log("SYSERR: NULL object or obj not in a room passed to obj_from_room");
        return;
    }
    REMOVE_FROM_LIST(object, world[object->in_room].contents, next_content);

    if (ROOM_FLAGGED(object->in_room, ROOM_HOUSE))
        SET_BIT(ROOM_FLAGS(object->in_room), ROOM_HOUSE_CRASH);
    object->in_room = NOWHERE;
    object->next_content = NULL;
    if (!((GET_OBJ_TYPE(object) == ITEM_CONTAINER) && GET_OBJ_VAL(object, 3)))
        GET_OBJ_TIMER(object)=0;
}


/* put an object in an object (quaint)  */
void            obj_to_obj(struct obj_data * obj, struct obj_data * obj_to)
{
    struct obj_data *tmp_obj;

    if (!obj || !obj_to || obj == obj_to) {
        log("SYSERR: NULL object or same source and target obj passed to obj_to_obj");
        return;
    }
    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;

    for (tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj)
        GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

    /* top level object.  Subtract weight from inventory if necessary. */
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
    if (tmp_obj->carried_by)
        IS_CARRYING_W(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);
}


/* remove an object from an object */
void            obj_from_obj(struct obj_data * obj)
{
    struct obj_data *temp,
                *obj_from;

if (obj->in_obj == NULL) {
        log("error (handler.c): trying to illegally extract obj from obj");
        return;
    }
    obj_from = obj->in_obj;
    REMOVE_FROM_LIST(obj, obj_from->contains, next_content);

    /* Subtract weight from containers container */
    for (temp = obj->in_obj; temp->in_obj; temp = temp->in_obj)
        GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);

    /* Subtract weight from char that carries the object */
    GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);
    if (temp->carried_by)
        IS_CARRYING_W(temp->carried_by) -= GET_OBJ_WEIGHT(obj);

    obj->in_obj = NULL;
    obj->next_content = NULL;
}


/* Set all carried_by to point to new owner */
void            object_list_new_owner(struct obj_data * list, struct char_data * ch)
{
    if (list) {
        object_list_new_owner(list->contains, ch);
        object_list_new_owner(list->next_content, ch);
        list->carried_by = ch;
    }
}


/* Extract an object from the world */
void            extract_obj(struct obj_data * obj)
{
    struct obj_data *temp;
    struct obj_ref_type *ref;


    if (obj->in_room==-2)
    {
        logs( "extract_obj: obj double extracted (%s)", obj->short_description);
        return;
    }

    /*
    for (ref = obj_ref_list; ref; ref = ref->next)
    if (*ref->var == obj)
    switch (ref->type) {
    case OBJ_NEXT:
    *ref->var = obj->next;
    break;
    case OBJ_NEXTCONTENT:
    *ref->var = obj->next_content;
    break;
    case OBJ_NULL:
    *ref->var = NULL;
    break;
    default:
    log("Bad obj_ref_list type");
    break;
}       */
    if (obj->worn_by != NULL)
        if (unequip_char(obj->worn_by, obj->worn_on) != obj)
            log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
    if (obj->in_room != NOWHERE)
        obj_from_room(obj);
    else if (obj->carried_by)
        obj_from_char(obj);
    else if (obj->in_obj)
        obj_from_obj(obj);

    /* Get rid of the contents of the object, as well. */
    while (obj->contains)
        extract_obj(obj->contains);

    REMOVE_FROM_LIST(obj, object_list, next);

    /*
        if (GET_OBJ_RNUM(obj) >= 0)
            if (obj->worn_by)
                (obj_index[GET_OBJ_RNUM(obj)].number[world[obj->worn_by->in_room].zone]) -= 1;
            else if (obj->carried_by)
                (obj_index[GET_OBJ_RNUM(obj)].number[world[obj->carried_by->in_room].zone]) -= 1;
            else
                (obj_index[GET_OBJ_RNUM(obj)].number[0]) -= 1;
    */
    if (GET_OBJ_RNUM(obj) >= 0) {
        (obj_index[GET_OBJ_RNUM(obj)].number[obj->orig_zone])--;
        if (obj_index[GET_OBJ_RNUM(obj)].number[obj->orig_zone] < 0)
            obj_index[GET_OBJ_RNUM(obj)].number[obj->orig_zone] = 0;
    }


    queue_extracted_obj(obj);
    //free_obj(obj);
}

void            update_object(struct obj_data * obj, int use)
{
    if (GET_OBJ_TIMER(obj) > 0)
        GET_OBJ_TIMER(obj) -= use;
    if (obj->contains)
        update_object(obj->contains, use);
    if (obj->next_content)
        update_object(obj->next_content, use);
}


void            update_char_objects(struct char_data * ch)
{
    int             i;

    if (GET_EQ(ch, WEAR_LIGHT) != NULL)
        if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
            if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2) > 0) {
                i = --GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2);
                if (i == 1) {
                    act("Your light begins to flicker and fade.", FALSE, ch, 0, 0, TO_CHAR);
                    act("$n's light begins to flicker and fade.", FALSE, ch, 0, 0, TO_ROOM);
                } else if (i == 0) {
                    act("Your light sputters out and dies.", FALSE, ch, 0, 0, TO_CHAR);
                    act("$n's light sputters out and dies.", FALSE, ch, 0, 0, TO_ROOM);
                    world[ch->in_room].light--;
                }
            }
    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i))
            update_object(GET_EQ(ch, i), 2);

    if (ch->carrying)
        update_object(ch->carrying, 1);
}


extern void release_supermob( );
extern struct char_data *supermob;
/* Extract a ch completely from the world, and leave his stuff behind */
void            extract_char(struct char_data * ch)
{
    struct char_data *k,
                *temp, *mob, *next_ch, *fw;
    struct descriptor_data *t_desc;
    struct obj_data *obj;
    int             i,
    freed = 0;
    char            mbuf[300];
    extern struct char_data *combat_list;
    struct char_ref_type *ref;
    ACMD(do_return);

    if (!ch)
        return;

    //if (IS_SUPERMOB(ch))
    if (supermob==ch)
{
        log("ERROR: Trying to extract supermob");
        release_supermob();
        return;
    }

    if (ch->in_room<1)
    {
        logs("ERROR: extract_char: %s trying to double die - in room %d", GET_NAME(ch), ch->in_room);
        if (!ch->in_room)
        {
            char_from_room(ch);
            char_to_room(ch, 3);
        }
        else
            return;
    }

    /*
    for (ref = char_ref_list; ref; ref = ref->next)
    if (*ref->var == ch)
    switch (ref->type) {
    case CHAR_NEXT:
    *ref->var = ch->next;
    break;
    case CHAR_NEXTROOM:
    *ref->var = ch->next_in_room;
    break;
    case CHAR_NULL:
    *ref->var = NULL;
    break;
    default:
    log("Bad char_ref_list type");
    break;
}
         */

    /*//if (WRATHOBJ(ch)!=NULL) {extract_obj(WRATHOBJ(ch));WRATHOBJ(ch)=NULL;}*/

    for (i=0;i<MAX_CHAR_EVENTS;i++)
        if (ch->char_events[i])
        {
            event_cancel(ch->char_events[i]);
            ch->char_events[i]=NULL;
        }

    /* if (GET_UTIL_EVENT(ch)) {
       event_cancel(GET_UTIL_EVENT(ch));
       GET_UTIL_EVENT(ch) = NULL;
     }
     if (GET_WEAR_EVENT(ch)) {
       event_cancel(GET_WEAR_EVENT(ch));
       GET_WEAR_EVENT(ch) = NULL;
     }
     if (GET_MOVE_EVENT(ch)) {
       event_cancel(GET_MOVE_EVENT(ch));
       GET_MOVE_EVENT(ch) = NULL;
     }        
      if (GET_SPELL_EVENT(ch)) {
       event_cancel(GET_SPELL_EVENT(ch));
       GET_SPELL_EVENT(ch) = NULL;
     }                    
      if (GET_FIGHT_EVENT(ch)) {
       event_cancel(GET_FIGHT_EVENT(ch));
       GET_FIGHT_EVENT(ch) = NULL;
     }                    
      */


    if (!IS_NPC(ch) && ch->player.long_descr)	// disguise
        DISPOSE(ch->player.long_descr);

    if (ch->ambush_name)	// ambush
        DISPOSE(ch->ambush_name);
    if (ch->listening_to) {

        REMOVE_FROM_LIST(ch, world[ch->listening_to].listeners, next_listener);
        ch->listening_to = 0;
        //send_to_char("You stop listening.\r\n", ch);
    }



    leech_from_char(ch, SPELL_ALL);
    ch->mana_leech=0;
    //GET_COND(ch, FULL)=24;
    //GET_COND(ch, THIRST)=24;	// moved to reset_char
    //GET_COND(ch, DRUNK)=0;
    // skloni ono sto bi ga moglo ubiti jos jedanput
    killfire(ch);
    killcold(ch);
    killacid(ch);


    if (ch->affected)
        while (ch->affected)
            affect_remove(ch, ch->affected);

    if (!IS_NPC(ch) && !ch->desc) {
        for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
            if (t_desc->original == ch)
                do_return(t_desc->character, "", 0, 0);
    }
    if (ch->in_room == NOWHERE) {
        logs("SYSERR: NOWHERE extracting char. (handler.c, extract_char) %s", GET_NAME(ch));
        exit(1);
    }

    if (!IS_NPC(ch) && !IN_ARENA(ch))
        for (mob = character_list; mob; mob = next_ch) {
            next_ch = mob->next;

            if (MOB_FLAGGED(mob, MOB_MEMORY))
                forget(mob, ch);
            if (HUNTING(mob)==ch)
            {
                HUNTING(mob)=NULL;
                if (GET_UTIL_EVENT(mob))
                    event_cancel(GET_UTIL_EVENT(mob));
                GET_UTIL_EVENT(mob)=0;
                if (MOB_FLAGGED(mob, MOB_ASSASSIN)) {
                    if (HIRED_BY(mob))
                    {
                        char bb[100];
                        sprintf(bb,"%s It seems that my victim vanished.", GET_NAME(HIRED_BY(mob)));
                        do_tell(ch,bb,0,0);
                    }
                    act("$n dissapears in a hurry.",FALSE, mob, NULL, NULL, TO_ROOM);
                    char_from_room(mob);
                    char_to_room(mob, real_room(8245));
                    HIRED_BY(mob)=NULL;
                }
            }
        }

    if (AFF_FLAGGED(ch, AFF_TARGET))
    {
        for (mob = character_list; mob; mob = mob->next)
            if (mob->current_target==ch)
                mob->current_target=NULL;
        REMOVE_BIT(AFF_FLAGS(ch), AFF_TARGET);
    }

    // cleanup guard stuff

    if (AFF2_FLAGGED(ch, AFF2_GUARDED))
    {
        for (mob = character_list; mob; mob = mob->next) {
            if (mob->guarding==ch)
            {
                act("You stop guarding $N,", FALSE, mob, 0, ch, TO_CHAR);
                REMOVE_BIT(AFF3_FLAGS(mob), AFF3_GUARD);
                mob->guarding=NULL;
            }
        }
        REMOVE_BIT(AFF2_FLAGS(ch), AFF2_GUARDED);
    }

    if (AFF3_FLAGGED(ch, AFF3_GUARD))
    {
        REMOVE_BIT(AFF3_FLAGS(ch), AFF3_GUARD);
        mob=ch->guarding;
        ch->guarding=NULL;
        if (!is_guarded(mob))
            REMOVE_BIT(AFF2_FLAGS(mob), AFF2_GUARDED);
    }





    if (ch->followers || ch->master)
        die_follower(ch);

    /* Forget snooping, if applicable */
    if (ch->desc) {
        if (ch->desc->snooping) {
            ch->desc->snooping->snoop_by = NULL;
            ch->desc->snooping = NULL;
        }
        if (ch->desc->snoop_by) {
            SEND_TO_Q("Your victim is no longer among us.\r\n",
                      ch->desc->snoop_by);
            ch->desc->snoop_by->snooping = NULL;
            ch->desc->snoop_by = NULL;
        }
    }
    /* transfer objects to room, if any - PC's loose eq in make_corpse()*/
    if (!IN_ARENA(ch) && (IS_NPC(ch) || !IS_SET(PLR_FLAGS(ch), PLR_JUSTDIED)))
    {
        if (!IS_NPC(ch) && !IS_SET(PLR_FLAGS(ch), PLR_JUSTDIED) && IS_SET(PRF2_FLAGS(ch), PRF2_NOQUIT) && GET_ROOM_VNUM(ch->in_room)!=1)
            logs("Error: Player came to extract char without making a corpse first, %s in room %d", GET_NAME(ch), GET_ROOM_VNUM(ch->in_room));

        while (ch->carrying) {
            obj = ch->carrying;
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
        }

        /* transfer equipment to room, if any */
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i))
            {
                //    logs("Mob came with equipment to extract char, vnum %d", GET_MOB_VNUM(ch))	;
                obj_to_room(unequip_char(ch, i), ch->in_room);
            }

    }


    if (FIGHTING(ch))
        stop_fighting(ch);

    for (k = combat_list; k; k = temp) {
        temp = k->next_fighting;
        if (FIGHTING(k) == ch)
        {
            stop_fighting(k);
            if (!DEAD(k) && (GET_POS(k)==POS_STANDING) 
            && (fw=get_fightning_with(k)))
    		set_fighting(k, fw);
 	   }
        }


                REMOVE_BIT(AFF2_FLAGS(ch), AFF2_BEHEAD);
                REMOVE_BIT(AFF3_FLAGS(ch), AFF3_FLEEING);

    
    if (!IN_ARENA(ch))
    {
        char_from_room(ch);

        /* pull the char from the list */
        REMOVE_FROM_LIST(ch, character_list, next);

        if (ch->desc && ch->desc->original)
            do_return(ch, "", 0, 0);

        if (!IS_NPC(ch)) {
            Crash_delete_crashfile(ch);
        } else {
            clearMemory(ch);        /* Only NPC's can have memory */
            //  free_char(ch);
            queue_extracted_char(ch);
            freed = 1;
        }

    
    if (GET_MOB_RNUM(ch) > -1)  /* if mobile */
        mob_index[GET_MOB_RNUM(ch)].number[ch->mob_specials.orig_zone] -= 1;


        if (!freed && ch->char_specials.timer < 60){ //ch->desc != NULL)    not idle extracted
            if (PRF2_FLAGGED(ch, PRF2_NOQUIT)) {
                //        sprintf(buf, DEATH_MESSG, GET_NAME(ch), GET_TITLE(ch));
                sprintf(buf, "\r\n\r\nYour mind floats through the empty space...\r\n.\r\n.\r\n.\r\nYou see a dim light...\r\n.\r\n.\r\n");
                send_to_char(buf, ch);
                reset_char(ch);
                char_to_room(ch, real_room(DEATHR));
                save_char(ch, real_room(DEATHR));
                Crash_load(ch);
                ch->next = character_list;
                character_list = ch;
                act("The cold air breezes you as $n suddenly appears.", TRUE, ch, 0, 0, TO_ROOM);
                look_at_room(ch, 0);
                SET_BIT(PLR_FLAGS(ch), PLR_JUSTDIED);
                if (FOL_AMURON(ch) || FOL_MUGRAK(ch))
                {       
                	ch_printf(ch, "\r\nYou are fully restored by %s.\r\n", DEITY_NAME(ch));
                	GET_HIT(ch) = GET_MAX_HIT(ch);
    			GET_MANA(ch) = GET_MAX_MANA(ch);;
    			GET_MOVE(ch) = GET_MAX_MOVE(ch);
    		}

                queue_extracted_char(ch);                
            } else {
                STATE(ch->desc) = CON_MENU;
                SEND_TO_Q(MENU, ch->desc);
            }
        } else {                    /* if a player gets purged from within the
                                         * game */
            if (!freed && IS_NPC(ch)) {
                queue_extracted_char(ch);
                //free_char(ch);
            }
        }
    }
    else
        queue_extracted_char(ch);

}



/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functions
   which incorporate the actual player-data.
   *********************************************************************** */


struct char_data *get_player_vis(struct char_data * ch, char *name, int inroom)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next)
        if (!IS_NPC(i) && (!inroom || i->in_room == ch->in_room) &&
                /*	!str_cmp(i->player.name, name) && CAN_SEE(ch, i)) */
                isname(name, i->player.name) && CAN_SEE(ch, i))
            return i;

    return NULL;
}


struct char_data *get_player_vis_in_room(struct char_data * ch, char *name, int inroom, int room)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next)
        if (!IS_NPC(i) && (!inroom || i->in_room == room) &&
                /*	!str_cmp(i->player.name, name) && CAN_SEE(ch, i)) */
                isname(name, i->player.name) && CAN_SEE(ch, i))
            return i;

    return NULL;
}


struct char_data *get_char_room_vis(struct char_data * ch, char *name)
{
    struct char_data *i;
    int             j = 0,
                        number;
    char            tmpname[MAX_INPUT_LENGTH];
    char           *tmp = tmpname;

    /* JE 7/18/94 :-) :-) */
    if (!str_cmp(name, "self") || !str_cmp(name, "me"))
        return ch;

    /* 0.<name> means PC with name */
    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return get_player_vis(ch, tmp, 1);

    for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room)
        if (isname(tmp, i->player.name))
            if (CAN_SEE(ch, i))
                if (!MOB_FLAGGED(i, MOB_ETHEREAL) || GET_LEVEL(ch) > LVL_IMMORT)
                    if (++j == number)
                        return i;
    return NULL;
}



struct char_data *get_char_room_vis_in_room(struct char_data * ch, char *name, int room)
{
    struct char_data *i;
    int             j = 0,
                        number;
    char            tmpname[MAX_INPUT_LENGTH];
    char           *tmp = tmpname;

    if (!room)
        return NULL;

    /* JE 7/18/94 :-) :-) */
    if (!str_cmp(name, "self") || !str_cmp(name, "me"))
        if (ch->in_room==room)
            return ch;

    /* 0.<name> means PC with name */
    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return get_player_vis_in_room(ch, tmp, 1, room);

    for (i = world[room].people; i && j <= number; i = i->next_in_room)
        if (isname(tmp, i->player.name))
            if (CAN_SEE(ch, i))
                if (!MOB_FLAGGED(i, MOB_ETHEREAL) || GET_LEVEL(ch) > LVL_IMMORT)
                    if (++j == number)
                        return i;
    return NULL;
}


struct char_data *get_char_vis(struct char_data * ch, char *name)
{
    struct char_data *i;
    int             j = 0,
                        number;
    char            tmpname[MAX_INPUT_LENGTH];
    char           *tmp = tmpname;

    /* check the room first */
    if ((i = get_char_room_vis(ch, name)) != NULL)
        return i;

    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return get_player_vis(ch, tmp, 0);

    for (i = character_list; i && (j <= number); i = i->next)
        if (isname(tmp, i->player.name) && CAN_SEE(ch, i) &&
                (!MOB_FLAGGED(i, MOB_ETHEREAL) || GET_LEVEL(ch) > LVL_IMMORT))
            if (++j == number)
                return i;

    return NULL;
}



struct obj_data *get_obj_in_list_vis(struct char_data * ch, char *name,
                                                 struct obj_data * list)
{
    struct obj_data *i;
    int             j = 0,
                        number;
    char            tmpname[MAX_INPUT_LENGTH];
    char           *tmp = tmpname;

    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return NULL;

    for (i = list; i && (j <= number); i = i->next_content)
        if (isname(tmp, i->name))
            if (CAN_SEE_OBJ(ch, i) || GET_OBJ_TYPE(i) == ITEM_LIGHT)
                if (++j == number)
                    return i;

    return NULL;
}

struct obj_data *get_obj_in_list_vis_food(struct char_data * ch, struct obj_data * list)
{
    struct obj_data *i;
    int             j = 0,
                        number;

    for (i = list; i ; i = i->next_content)
        if (GET_OBJ_TYPE(i)==ITEM_FOOD)
            if (CAN_SEE_OBJ(ch, i))
                return i;

    return NULL;
}




/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data * ch, char *name)
{
    struct obj_data *i;
    int             j = 0,
                        number;
    char            tmpname[MAX_INPUT_LENGTH];
    char           *tmp = tmpname;

    /* scan items carried */
    if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
        return i;

    /* scan room */
    if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
        return i;

    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return NULL;

    /* ok.. no luck yet. scan the entire obj list   */
    for (i = object_list; i && (j <= number); i = i->next)
        if (isname(tmp, i->name))
            if (CAN_SEE_OBJ(ch, i) && (!i->carried_by || CAN_SEE(ch, i->carried_by)))
                if (++j == number)
                    return i;

    return NULL;
}



struct obj_data *get_object_in_equip_vis(struct char_data * ch,
                    char *arg, struct obj_data * equipment[], int *j)
{
    for ((*j) = 0; (*j) < NUM_WEARS; (*j)++)
        if (equipment[(*j)])
            if (CAN_SEE_OBJ(ch, equipment[(*j)]))
                if (isname(arg, equipment[(*j)]->name))
                    return (equipment[(*j)]);

    return NULL;
}


char           *money_desc(int amount)
{
    static char     buf[128];

    if (amount <= 0) {
        log("SYSERR: Try to create negative or 0 money.");
        return NULL;
    }
    if (amount == 1)
        strcpy(buf, "a coin");
    else if (amount <= 5)
        strcpy(buf, "a tiny pile of coins");
    else if (amount <= 12)
        strcpy(buf, "a handful of coins");
    else if (amount <= 30)
        strcpy(buf, "a little pile of coins");
    else if (amount <= 50)
        strcpy(buf, "a small pile of coins");
    else if (amount <= 90)
        strcpy(buf, "a pile of coins");
    else if (amount <= 140)
        strcpy(buf, "a big pile of coins");
    else if (amount <= 200)
        strcpy(buf, "a large heap of coins");
    else if (amount <= 400)
        strcpy(buf, "a huge mound of coins");
    else if (amount <= 800)
        strcpy(buf, "an enormous mound of coins");
    else if (amount <= 1200)
        strcpy(buf, "a small mountain of coins");
    else if (amount <= 3000)
        strcpy(buf, "a mountain of coins");
    else if (amount <= 10000)
        strcpy(buf, "a huge mountain of coins");
    else if (amount <= 50000)
        strcpy(buf, "an enormous mountain of coins");
    else
        strcpy(buf, "an absolutely colossal mountain of coins");

    return buf;
}


struct obj_data *create_money(int amount)
{
    struct obj_data *obj;
    struct extra_descr_data *new_descr;
    char            buf[200];

    if (amount <= 0) {
        log("SYSERR: Try to create negative or 0 money.");
        return NULL;
    }
    obj = create_obj();
    CREATE(new_descr, struct extra_descr_data, 1);

    if (amount == 1) {
        obj->name = str_dup("coin gold");
        obj->short_description = str_dup("a coin");
        obj->description = str_dup("One miserable coin is lying here.");
        new_descr->keyword = str_dup("coin gold");
        new_descr->description = str_dup("It's just one miserable little coin.");
    } else {
        obj->name = str_dup("coins gold");
        obj->short_description = str_dup(money_desc(amount));
        sprintf(buf, "%s is lying here.", money_desc(amount));
        obj->description = str_dup(CAP(buf));

        new_descr->keyword = str_dup("coins gold");
        if (amount < 10) {
            sprintf(buf, "There are %d coins.", amount);
            new_descr->description = str_dup(buf);
        } else if (amount < 100) {
            sprintf(buf, "There are about %d coins.", 10 * (amount / 10));
            new_descr->description = str_dup(buf);
        } else if (amount < 1000) {
            sprintf(buf, "It looks to be about %d coins.", 100 * (amount / 100));
            new_descr->description = str_dup(buf);
        } else if (amount < 100000) {
            sprintf(buf, "You guess there are, maybe, %d coins.",
                    1000 * ((amount / 1000) + number(0, (amount / 1000))));
            new_descr->description = str_dup(buf);
        } else
            new_descr->description = str_dup("There are a LOT of coins.");
    }

    new_descr->next = NULL;
    obj->ex_description = new_descr;

    GET_OBJ_TYPE(obj) = ITEM_MONEY;
    GET_OBJ_WEAR(obj) = ITEM_WEAR_TAKE;
    GET_OBJ_VAL(obj, 0) = amount;
    GET_OBJ_COST(obj) = amount;
    obj->item_number = NOTHING;
    GET_OBJ_RNUM(obj) = 0;

    return obj;
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */


int             generic_find(char *arg, int bitvector, struct char_data * ch,
                             struct char_data ** tar_ch, struct obj_data ** tar_obj)
{
    int             i,
    found;
    char            name[256];

    one_argument(arg, name);

    if (!*name)
        return (0);

    *tar_ch = NULL;
    *tar_obj = NULL;

    if (IS_SET(bitvector, FIND_CHAR_ROOM)) {    /* Find person in room */
        if ((*tar_ch = get_char_room_vis(ch, name))) {
            return (FIND_CHAR_ROOM);
        }
    }
    if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
        if ((*tar_ch = get_char_vis(ch, name))) {
            return (FIND_CHAR_WORLD);
        }
    }
    
    if (IS_SET(bitvector, FIND_OBJ_INV)) {
        if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
            return (FIND_OBJ_INV);
        }
    }
    if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
        for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
            if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) && isname(name, GET_EQ(ch, i)->name)) {
                *tar_obj = GET_EQ(ch, i);
                found = TRUE;
            }
        if (found) {
            return (FIND_OBJ_EQUIP);
        }
    }
    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
        if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) {
            return (FIND_OBJ_ROOM);
        }
    }
    if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
        if ((*tar_obj = get_obj_vis(ch, name))) {
            return (FIND_OBJ_WORLD);
        }
    }
    return (0);
}



/* a function to scan for "all" or "all.x" */
int             find_all_dots(char *arg)
{
    if (!strcmp(arg, "all"))
        return FIND_ALL;
    else if (!strncmp(arg, "all.", 4)) {
        strcpy(arg, arg + 4);
        return FIND_ALLDOT;
    } else
        return FIND_INDIV;
}




const char *last_array[11] = {
                                 "Connect",
                                 "Enter Game",
                                 "Reconnect",
                                 "Takeover",
                                 "Quit",
                                 "Idleout",
                                 "Disconnect",
                                 "Shutdown",
                                 "Reboot",
                                 "Crash",
                                 "Playing"
                             };

struct last_entry *find_llog_entry(int punique, int idnum,int close) {
    FILE *fp;
    struct last_entry mlast;
    struct last_entry *llast;
    int size,recs,tmp;

    if(!(fp=fopen(LAST_FILE,"rb"))) {
        log("error opening last_file for reading");
        return NULL;
    }
    fseek(fp,0L,SEEK_END);
    size=ftell(fp);

    /* recs = number of records in the last file */

    recs = size/sizeof(struct last_entry);
    /* we'll search last to first, since it's faster than any thing else
          we can do (like searching for the last shutdown/etc..) */
    for(tmp=recs-1; tmp > 0; tmp--) {
        fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);
        fread(&mlast,sizeof(struct last_entry),1,fp);
        /*another one to keep that stepback */
        fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);

        if(mlast.punique == punique &&
                mlast.idnum == idnum) {
            /* then we've found a match */
            CREATE(llast,struct last_entry,1);
            memcpy(llast,&mlast,sizeof(struct last_entry));
            fclose(fp);
            return llast;
        }
        /* check for crash/reboot/etc code */
        if(mlast.punique < 0 && close != 0) {
            fclose(fp);
            return NULL;
        }
        /*not the one we seek. next */
    }
    /*not found, no problem, quit */
    fclose(fp);
    return NULL;
}

/* mod_llog_entry assumes that llast is accurate */
void mod_llog_entry(struct last_entry *llast,int type) {
    FILE *fp;
    struct last_entry mlast;
    int size,recs,tmp;

    if(!(fp=fopen(LAST_FILE,"r+b"))) {
        log("error opening last_file for reading and writing");
        return;
    }
    fseek(fp,0L,SEEK_END);
    size=ftell(fp);

    /* recs = number of records in the last file */

    recs = size/sizeof(struct last_entry);

    /* we'll search last to first, since it's faster than any thing else
          we can do (like searching for the last shutdown/etc..) */
    for(tmp=recs; tmp > 0; tmp--) {
        fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);
        fread(&mlast,sizeof(struct last_entry),1,fp);
        /*another one to keep that stepback */
        fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);

        if(mlast.punique == llast->punique &&
                mlast.idnum == llast->idnum) {
            /* then we've found a match */
            /* lets assume quit is inviolate, mainly because
                    disconnect is called after each of these */
            if(mlast.close_type != LAST_QUIT &&
                    mlast.close_type != LAST_IDLEOUT &&
                    mlast.close_type != LAST_REBOOT &&
                    mlast.close_type != LAST_SHUTDOWN) {
                mlast.close_type=type;
            }
            mlast.close_time=time(0);

            /*write it, and we're done!*/
            fwrite(&mlast,sizeof(struct last_entry),1,fp);
            fflush(fp);
            fclose(fp);
            return;
        }
        /*not the one we seek. next */
    }
    fclose(fp);

    /*not found, no problem, quit */
    return;
}

void add_llog_entry(struct char_data *ch, int type) {
    FILE *fp;
    struct last_entry *llast;

    /* so if a char enteres a name, but bad password, otherwise
          loses link before he gets a pref assinged, we
          won't record it */
    if(GET_PREF(ch) <= 0) {
        return;
    }

    /* we use the close at 0 because if we're modifying a current one,
       we'll take the most recent one, but, if the mud has rebooted/etc
       then they won't be _after_ that (after coming from the direction
       that we're counting, which is last to first) */
    llast = find_llog_entry(GET_PREF(ch), GET_IDNUM(ch),0);

    if(llast == NULL) {  /* no entry found, add ..error if close! */
        CREATE(llast,struct last_entry,1);
        strncpy(llast->username,GET_NAME(ch),16);
        strncpy(llast->hostname,GET_HOST(ch),128);
        llast->idnum=GET_IDNUM(ch);
        llast->punique=GET_PREF(ch);
        llast->time=time(0);
        llast->close_time=0;
        llast->close_type=LAST_CRASH;

        if(!(fp=fopen(LAST_FILE,"ab"))) {
            log("error opening last_file for appending");
            DISPOSE(llast);
            return;
        }
        fwrite(llast,sizeof(struct last_entry),1,fp);
        fflush(fp);
        fclose(fp);
        return;
    } else {
        /* we're modifying a found entry */
        mod_llog_entry(llast,type);
        DISPOSE(llast);
    }
}

/* debugging stuff, if you wanna see the whole file */
char *list_llog_entry() {
    FILE *fp;
    struct last_entry llast;
    char buffer[12800];
    extern const char *last_array[];

    if(!(fp=fopen(LAST_FILE,"rb"))) {
        log("bad things.");
        return strdup("Error.");
    }

    sprintf(buffer,"Last log\r\n");

    fread(&llast,sizeof(struct last_entry),1,fp);

    while(!feof(fp)) {
        sprintf(buffer,"%s%10s\t%s\t%d\t%s",
                buffer,llast.username,last_array[llast.close_type],
                llast.punique,ctime(&llast.time));
        fread(&llast,sizeof(struct last_entry),1,fp);
    }
    return strdup(buffer);
}



/*

bool obj_extracted( OBJ_DATA *obj )
{
    OBJ_DATA *cod;

    if ( obj->serial == cur_obj
    &&   cur_obj_extracted )
	return TRUE;

    for (cod = extracted_obj_queue; cod; cod = cod->next )
	if ( obj == cod )
	     return TRUE;
    return FALSE;
}

void queue_extracted_obj( OBJ_DATA *obj )
{
    
    ++cur_qobjs;
    obj->next = extracted_obj_queue;
    extracted_obj_queue = obj;
}

void clean_obj_queue()
{
    OBJ_DATA *obj;

    while ( extracted_obj_queue )
    {
	obj = extracted_obj_queue;
	extracted_obj_queue = extracted_obj_queue->next;
	STRFREE( obj->name        );
	STRFREE( obj->description );
	STRFREE( obj->short_descr );
	DISPOSE( obj );
	--cur_qobjs;
    }
}
  */
