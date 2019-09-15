/***************************************************************************
 * OBJ and ROOM programs ported for CircleMUD by Vladimir Prelovac         *
 * tomcat@galeb.etf.bg.ac.yu            alfa.mas.bg.ac.yu 5000  	   *
 **************************************************************************/


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
 *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
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
#include "handler.h"
#include "db.h"
#include "events.h"
extern int      top_of_world;   /* In db.c */
extern const char     *pc_race_types[];
extern const char     *pc_class_types[];
extern struct time_info_data time_info;
int global_action=0;
int global_wait=0;


char            buf2[MAX_STRING_LENGTH];

extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;

extern void     death_cry(struct char_data * ch);
extern bool     str_prefix(const char *astr, const char *bstr);
extern int      number_percent(void);
extern int      number_range(int from, int to);

#define bug(x, y) { sprintf(buf2, (x), (y)); log(buf2); }

/* Defines by Narn for new mudprog parsing, used as
   return values from mprog_do_command. */
#define COMMANDOK    1
#define IFTRUE       2
#define IFFALSE      3
#define ORTRUE       4
#define ORFALSE      5
#define FOUNDELSE    6
#define FOUNDENDIF   7
#define IFIGNORED    8
#define ORIGNORED    9

/* Ifstate defines, used to create and access ifstate array
   in mprog_driver. */
#define MAX_IFS     20		/* should always be generous */
#define IN_IF        0
#define IN_ELSE      1
#define DO_IF        2
#define DO_ELSE      3

#define MAX_PROG_NEST   20
int global_newsupermob=0;

/*
 * Local function prototypes
 */
/*
char           *mprog_next_command(char *clist);
int             mprog_seval(char *lhs, char *opr, char *rhs, CHAR_DATA *mob);
int             mprog_veval(int lhs, char *opr, int rhs, CHAR_DATA *mob);
int             mprog_do_ifchck(char *ifchck, struct char_data * mob,
                          struct char_data * actor, struct obj_data * obj,
                                       void *vo, struct char_data * rndm);
char           *mprog_process_if(char *ifchck, char *com_list,
                         struct char_data * mob, struct char_data * actor,
                                          struct obj_data * obj, void *vo,
                                               struct char_data * rndm);
void            mprog_translate(char ch, char *t, struct char_data * mob,
                          struct char_data * actor, struct obj_data * obj,
                                       void *vo, struct char_data * rndm);
void            mprog_process_cmnd(char *cmnd, struct char_data * mob,
                          struct char_data * actor, struct obj_data * obj,
                                       void *vo, struct char_data * rndm);
void            mprog_driver(char *com_list, struct char_data * mob,
                          struct char_data * actor, struct obj_data * obj,
                                           void *vo);
*/
char *	mprog_next_command	( char* clist )  ;
int 	mprog_seval		( char* lhs, char* opr, char* rhs,
                    CHAR_DATA *mob ) ;
int 	mprog_veval		( int lhs, char* opr, int rhs,
                    CHAR_DATA *mob ) ;
int	mprog_do_ifcheck	( char* ifcheck, CHAR_DATA* mob,
                       CHAR_DATA* actor, OBJ_DATA* obj,
                       void* vo, CHAR_DATA* rndm ) ;
void	mprog_translate		 ( char ch, char* t, CHAR_DATA* mob,
                         CHAR_DATA* actor, OBJ_DATA* obj,
                         void* vo, CHAR_DATA* rndm ) ;
void	mprog_driver		 ( char* com_list, CHAR_DATA* mob,
                      CHAR_DATA* actor, OBJ_DATA* obj,
                      void* vo, bool single_step ) ;
int mprog_do_command( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor,
                      OBJ_DATA *obj, void *vo, CHAR_DATA *rndm,
                      bool ignore, bool ignore_ors );

bool mprog_keyword_check	 ( const char *argu, const char *argl );
void oprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
                           OBJ_DATA *obj, void *vo, int type, OBJ_DATA *iobj );

void rprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
                           OBJ_DATA *obj, void *vo, int type, ROOM_INDEX_DATA *room );


CHAR_DATA *supermob=NULL;
struct act_prog_data *room_act_list;
struct act_prog_data *obj_act_list;
struct act_prog_data *mob_act_list;
extern struct char_data *character_list;

static bool carryingvnum_visit( CHAR_DATA * ch, int vnum )
{
    /*
       pager_printf(ch, "***obj=%s vnum=%d\n\r", obj->name, obj->pIndexData->vnum );
    */  
    struct obj_data *obj;
    for (obj=ch->carrying;obj;obj=obj->next_content)
        if (GET_OBJ_VNUM(obj)==vnum)
            return TRUE;

    return FALSE;
}

void progbug( char *str, CHAR_DATA *mob )
{
    char buf[MAX_STRING_LENGTH];

    /* Check if we're dealing with supermob, which means the bug occurred
       in a room or obj prog. */
    if (GET_MOB_VNUM(mob)==SUPERMOB)
    {
        /* It's supermob.  In set_supermob and rset_supermob, the description
           was set to indicate the object or room, so we just need to show
           the description in the bug message. */
        sprintf( buf, "ERROR: %s, %s.", str,
                 mob->player.description == NULL ? "(unknown)" : mob->player.description );
    }
    else
    {
        sprintf( buf, "ERROR: %s, Mob #%d.", str, GET_MOB_VNUM(mob));
    }

    log( buf);
    return;
}
void release_supermob( )
{
    if (!supermob)
        return;
    //if (!global_newsupermob)
    {
        char_from_room( supermob );
        char_to_room( supermob, real_room(3) );

    }
    global_newsupermob=0;
    REMOVE_BIT(MOB_FLAGS(supermob), MOB_SPEC);

}



void init_supermob()
{

    release_supermob();
    supermob = read_mobile(real_mobile(SUPERMOB), REAL, world[real_room(3)].zone);
    char_to_room( supermob, real_room(3));

    if (supermob->player.name)
    {
        //DISPOSE(supermob->player.name);
        //supermob->player.name 		= str_dup("supermob");
    }
    if (supermob->player.short_descr)
    {
        //DISPOSE(supermob->player.short_descr);
        //supermob->player.short_descr 	= str_dup("supermob");
    }
    if (supermob->player.long_descr)
    {
        //DISPOSE(supermob->player.long_descr);
        //supermob->player.long_descr 	= str_dup("supermob is here\r\n");
    }

}

extern struct char_data *mob_proto;

void set_supermob( OBJ_DATA *obj)
{
    room_num room=0;
    OBJ_DATA *in_obj;
    CHAR_DATA *mob;
    int i;
    char buf[200];

    if ( !supermob )
    {
        log("No supermob!");
        init_supermob();
    }

    mob = supermob;   /* debugging */

    if(!obj)
        return;

    for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
        ;

    if ( in_obj->carried_by )
    {
        room = in_obj->carried_by->in_room;
    }
    else if ( in_obj->worn_by )
    {
        room = in_obj->worn_by->in_room;
    }
    else
    {
        room = obj->in_room;
    }

    if(!room)
        return;
    i = GET_MOB_RNUM(supermob);
    if (supermob->player.short_descr && (supermob->player.short_descr != mob_proto[i].player.short_descr))
        DISPOSE(supermob->player.short_descr);

    supermob->player.short_descr = str_dup(obj->short_description);
    supermob->mpscriptpos = obj->mpscriptpos;

    /* Added by Jenny to allow bug messages to show the vnum
       of the object, and not just supermob's vnum */
    sprintf( buf, "Object #%d", GET_OBJ_VNUM(obj));
    if (supermob->player.description && (supermob->player.description != mob_proto[i].player.description))
        DISPOSE( supermob->player.description );
    supermob->player.description = str_dup( buf );
    SET_BIT(MOB_FLAGS(supermob), MOB_SPEC);
    if(room)
    {
        char_from_room (supermob );
        char_to_room( supermob, room);
    }
    //GET_LEVEL(supermob)=GET_OBJ_LEVEL(obj); 
    supermob->protection=GET_OBJ_LEVEL(obj); 
}


void rset_supermob( ROOM_INDEX_DATA *room)
{
    char buf[200];
    int i;

    if (room)
    {        i = GET_MOB_RNUM(supermob);

        if (supermob->player.short_descr && (supermob->player.short_descr != mob_proto[i].player.short_descr))
            STRFREE(supermob->player.short_descr);
        supermob->player.short_descr = str_dup(room->name);
        if (supermob->player.name && (supermob->player.name != mob_proto[i].player.name))
            STRFREE(supermob->player.name);
        supermob->player.name        = str_dup(room->name);
        supermob->mpscriptpos = room->mpscriptpos;

        /* Added by Jenny to allow bug messages to show the vnum
           of the room, and not just supermob's vnum */
        sprintf( buf, "Room #%d", room->number );
        if (supermob->player.description && (supermob->player.description != mob_proto[i].player.description))
            STRFREE( supermob->player.description );
        supermob->player.description = str_dup( buf );

        char_from_room (supermob );
        char_to_room( supermob, real_room(room->number));
    }
}




/***************************************************************************
 * Local function code and brief comments.
 */

/* Used to get sequential lines of a multi line string (separated by "\r\n")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */

char           *mprog_next_command(char *clist)
{

    char           *pointer = clist;

    if (*pointer == '\r')
        pointer++;
    if (*pointer == '\n')
        pointer++;

    while (*pointer != '\n' && *pointer != '\0' && *pointer != '\r')
        pointer++;
    if (*pointer == '\n') {
        *pointer = '\0';
        pointer++;
    }
    if (*pointer == '\r') {
        *pointer = '\0';
        pointer++;
    }

    // SMAUG verzija
    /*  while ( *pointer != '\n' && *pointer != '\0' )
        pointer++;
      if ( *pointer == '\n' )
        *pointer++ = '\0';
      if ( *pointer == '\r' )
        *pointer++ = '\0';
      */

    return (pointer);

}




/* we need str_infix here because strstr is not case insensitive */

bool            str_infix(const char *astr, const char *bstr)
{
    int             sstr1;
    int             sstr2;
    int             ichar;
    char            c0;

    if ((c0 = LOWER(astr[0])) == '\0')
        return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for (ichar = 0; ichar <= sstr2 - sstr1; ichar++) {
        if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
            return FALSE;
    }

    return TRUE;
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
/*int             mprog_seval(char *lhs, char *opr, char *rhs)
{

    if (!str_cmp(opr, "=="))
        return (!str_cmp(lhs, rhs));
    if (!str_cmp(opr, "!="))
        return (str_cmp(lhs, rhs));
    if (!str_cmp(opr, "/"))
        return (!str_infix(rhs, lhs));
    if (!str_cmp(opr, "!/"))
        return (str_infix(rhs, lhs));

    log("Improper MOBprog operator");
    return 0;

}

int             mprog_veval(int lhs, char *opr, int rhs)
{

    if (!str_cmp(opr, "=="))
        return (lhs == rhs);
    if (!str_cmp(opr, "!="))
        return (lhs != rhs);
    if (!str_cmp(opr, ">"))
        return (lhs > rhs);
    if (!str_cmp(opr, "<"))
        return (lhs < rhs);
    if (!str_cmp(opr, "<="))
        return (lhs <= rhs);
    if (!str_cmp(opr, ">="))
        return (lhs >= rhs);
    if (!str_cmp(opr, "&"))
        return (lhs & rhs);
    if (!str_cmp(opr, "|"))
        return (lhs | rhs);

    log("Improper MOBprog operator");
    return 0;

}
*/          

char log_buf[2000];
int mprog_seval( char *lhs, char *opr, char *rhs, CHAR_DATA *mob )
{

    if ( !str_cmp( opr, "==" ) )
        return ( bool )( !str_cmp( lhs, rhs ) );
    if ( !str_cmp( opr, "!=" ) )
        return ( bool )( str_cmp( lhs, rhs ) );
    if ( !str_cmp( opr, "/" ) )
        return ( bool )( !str_infix( rhs, lhs ) );
    if ( !str_cmp( opr, "!/" ) )
        return ( bool )( str_infix( rhs, lhs ) );

    sprintf( log_buf, "Improper MOBprog operator '%s'", opr );
    progbug( log_buf, mob );
    return 0;

}

int mprog_veval( int lhs, char *opr, int rhs, CHAR_DATA *mob )
{

    if ( !str_cmp( opr, "==" ) )
        return ( lhs == rhs );
    if ( !str_cmp( opr, "!=" ) )
        return ( lhs != rhs );
    if ( !str_cmp( opr, ">" ) )
        return ( lhs > rhs );
    if ( !str_cmp( opr, "<" ) )
        return ( lhs < rhs );
    if ( !str_cmp( opr, "<=" ) )
        return ( lhs <= rhs );
    if ( !str_cmp( opr, ">=" ) )
        return ( lhs >= rhs );
    if ( !str_cmp( opr, "&" ) )
        return ( lhs & rhs );
    if ( !str_cmp( opr, "|" ) )
        return ( lhs | rhs );

    sprintf( log_buf, "Improper MOBprog operator '%s'", opr );
    progbug( log_buf, mob );

    return 0;

}



/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifcheck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return BERR otherwise return boolean 1,0
 * Redone by Altrag.. kill all that big copy-code that performs the
 * same action on each variable..
 */
int mprog_do_ifcheck( char *ifcheck, CHAR_DATA *mob, CHAR_DATA *actor,
                      OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
    char cvar[MAX_INPUT_LENGTH];
    char chck[MAX_INPUT_LENGTH];
    char opr[MAX_INPUT_LENGTH];
    char rval[MAX_INPUT_LENGTH];
    char *point = ifcheck;
    char *pchck = chck;
    CHAR_DATA *chkchar = NULL;
    OBJ_DATA *chkobj = NULL;
    int lhsvl, rhsvl = 0;

    if ( !*point )
    {
        progbug( "Null ifcheck", mob );
        return BERR;
    }
    while ( *point == ' ' )
        point++;
    while ( *point != '(' )
        if ( *point == '\0' )
        {
            progbug( "Ifcheck syntax error", mob );
            return BERR;
        }
        else if ( *point == ' ' )
            point++;
        else
            *pchck++ = *point++;
    *pchck = '\0';
    point++;
    pchck = cvar;

    while ( *point != ')' )
        if ( *point == '\0' )
        {
            progbug( "Ifcheck syntax error", mob );
            return BERR;
        }
        else if ( *point == ' ' )
            point++;
        else
            *pchck++ = *point++;
    point++;
    *pchck = '\0';                          /* Gorog's bug fix */

    while ( *point == ' ' )
        point++;
    if ( !*point )
    {
        opr[0] = '\0';
        rval[0] = '\0';
    }
    else
    {
        pchck = opr;
        while ( *point != ' ' && !isalnum(*point) )
            if ( *point == '\0' )
            {
                progbug( "Ifcheck operator without value", mob );
                return BERR;
            }
            else
                *pchck++ = *point++;
        *pchck = '\0';

        while ( *point == ' ' )
            point++;
        pchck = rval;
        while ( *point != '\0' && *point != '\0' )
            *pchck++ = *point++;
        *pchck = '\0';
    }

    /* chck contains check, cvar is the variable in the (), opr is the
     * operator if there is one, and rval is the value if there was an
     * operator.
     */
    if ( cvar[0] == '$' )
    {
        switch(cvar[1])
        {
        case 'i':	chkchar = mob;			break;
        case 'n':	chkchar = actor;		break;
        case 't':	chkchar = (CHAR_DATA *)vo;	break;
        case 'r':	chkchar = rndm;			break;
        case 'q':	chkchar = rndm?rndm:mob;			break;
        case 'o':	chkobj = obj;			break;
        case 'p':	chkobj = (OBJ_DATA *)vo;	break;
        default:
            sprintf(rval, "Bad argument '%c' to '%s'", cvar[0], chck);
            progbug(rval, mob);
            return BERR;
        }
        if ( !chkchar && !chkobj )
            return BERR;
    }
    if ( !str_cmp(chck, "rand") )
    {
    	//int i=number(1,100),j=atoi(cvar);
    	//return (i<=j);
    	
        return (number(1, 100) <= atoi(cvar));
    }
    if ( !str_cmp(chck, "economy") )
    {
        progbug("Unsupported ifcheck 'economy'", mob);
        return BERR;
        /*
        int idx = atoi(cvar);
        ROOM_INDEX_DATA *room;

        if ( !idx )
    {
          if ( !mob->in_room )
          {
            progbug( "'economy' ifcheck: mob in NULL room with no room vnum "
                "argument", mob );
            return BERR;
          }
          room = mob->in_room;
    }
        else
          room = get_room_index(idx);
        if ( !room )
    {
          progbug( "Bad room vnum passed to 'economy'", mob );
          return BERR;
    }
        return mprog_veval( ((room->area->high_economy > 0) ? 1000000000 : 0)
        + room->area->low_economy, opr, atoi(rval), mob );*/
    }
    if(!str_cmp(chck, "mobinarea"))
    {
        //progbug("Unsupported ifcheck 'mobinarea'", mob);
        //return BERR;
        int vnum = atoi(cvar);
        int lhsvl;
        int world_count;
        int found_count;
        CHAR_DATA *tmob;


        if(vnum < 1 || vnum > 255767)
        {
            progbug("Bad vnum to 'mobinarea'", mob);
            return BERR;
        }



        lhsvl = 0;
        found_count = 0;

        for (tmob = character_list; tmob; tmob = tmob->next)
        {
            if(IS_NPC(tmob) && GET_MOB_VNUM(tmob) == vnum)
            {
                found_count++;

                if (world[tmob->in_room].zone == world[mob->in_room].zone)
                    lhsvl++;
            }
        }

        rhsvl = atoi(rval);
        // Changed below from 1 to 0
        if(rhsvl < 0)
            rhsvl = 0;
        if(!*opr)
            strcpy(opr, "==");

        return mprog_veval(lhsvl, opr, rhsvl, mob);
    }

    if ( !str_cmp(chck, "mobinroom") )
    {
        int vnum = atoi(cvar);
        int lhsvl;
        CHAR_DATA *oMob;


        if ( vnum < 1 || vnum > 900000 )
        {
            progbug( "Bad vnum to 'mobinroom'", mob );
            return BERR;
        }
        lhsvl = 0;
        for ( oMob = world[mob->in_room].people; oMob;
                oMob = oMob->next_in_room )
            if ( IS_NPC(oMob) && GET_MOB_VNUM(oMob) == vnum )
                lhsvl++;
        rhsvl = atoi(rval);

        if ( rhsvl < 0 ) rhsvl = 0;
        if ( !*opr )
            strcpy( opr, "==" );
        return mprog_veval(lhsvl, opr, rhsvl, mob);
    }

    if(!str_cmp(chck, "mobinworld"))
    {
        progbug("Unsupported ifcheck 'mobinworld'", mob);
        return BERR;
        /*
        int vnum = atoi(cvar);
        int lhsvl;
        MOB_INDEX_DATA *m_index;

        if(vnum < 1 || vnum > 32767)
    {
        	progbug("Bad vnum to 'mobinworld'", mob);
        	return BERR;
    }

        m_index = get_mob_index(vnum);

        if(!m_index)
        lhsvl = 0;
        else
        lhsvl = m_index->count;

        rhsvl = atoi(rval);
        // Changed below from 1 to 0 

        if(rhsvl < 0)
        rhsvl = 0;
        if(!*opr)
        strcpy(opr, "==");

        return mprog_veval(lhsvl, opr, rhsvl, mob);*/
    }
    if ( !str_cmp(chck, "timeskilled") )
    {
        progbug("Unsupported ifcheck 'timeskilled'", mob);
        return BERR;
        /*MOB_INDEX_DATA *pMob;

        if ( chkchar )
          pMob = chkchar->pIndexData;
        else if ( !(pMob = get_mob_index(atoi(cvar))) )
    {
          progbug("TimesKilled ifcheck: bad vnum", mob);
          return BERR;
    }
        return mprog_veval(pMob->killed, opr, atoi(rval), mob);*/
    }
    if ( !str_cmp(chck, "ovnumhere") )
    {
        progbug("Unsupported ifcheck 'ovnumhere'", mob);
        return BERR;
        /*OBJ_DATA *pObj;
        int vnum = atoi(cvar);

        if ( vnum < 1 || vnum > 32767 )
    {
          progbug("OvnumHere: bad vnum", mob);
          return BERR;
    }
        lhsvl = 0;
        for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
          if ( pObj->pIndexData->vnum == vnum )
        lhsvl+=pObj->count;
        for ( pObj = mob->in_room->first_content; pObj;
              pObj = pObj->next_content )
          if ( pObj->pIndexData->vnum == vnum )
        lhsvl+=pObj->count;
        rhsvl = is_number(rval) ? atoi(rval) : -1;
        // Changed from 1 to 0 
        if ( rhsvl < 0 )
          rhsvl = 0;
        if ( !*opr )
          strcpy(opr, "==");
        return mprog_veval(lhsvl, opr, rhsvl, mob);*/
    }
    if ( !str_cmp(chck, "otypehere") )
    {
        progbug("Unsupported ifcheck 'otypehere'", mob);
        return BERR;
        /*OBJ_DATA *pObj;
        int type;

        if ( is_number(cvar) )
          type = atoi(cvar);
        else
          type = get_otype(cvar);
        if ( type < 0 || type > MAX_ITEM_TYPE )
    {
          progbug("OtypeHere: bad type", mob);
          return BERR;
    }
        lhsvl = 0;
        for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
          if ( pObj->item_type == type )
        lhsvl+=pObj->count;
        for ( pObj = mob->in_room->first_content; pObj;
              pObj = pObj->next_content )
          if ( pObj->item_type == type )
        lhsvl+=pObj->count;
        rhsvl = is_number(rval) ? atoi(rval) : -1;
        // Change below from 1 to 0 
        if ( rhsvl < 0 )
          rhsvl = 0;
        if ( !*opr )
          strcpy(opr, "==");
        return mprog_veval(lhsvl, opr, rhsvl, mob);*/
    }
    if ( !str_cmp(chck, "ovnumroom") )
    {
        progbug("Unsupported ifcheck 'ovnumroom'", mob);
        return BERR;
        /*OBJ_DATA *pObj;
        int vnum = atoi(cvar);

        if ( vnum < 1 || vnum > 32767 )
    {
          progbug("OvnumRoom: bad vnum", mob);
          return BERR;
    }
        lhsvl = 0;
        for ( pObj = mob->in_room->first_content; pObj;
              pObj = pObj->next_content )
          if ( pObj->pIndexData->vnum == vnum )
        lhsvl+=pObj->count;
        rhsvl = is_number(rval) ? atoi(rval) : -1;
        // Changed below from 1 to 0 so can check for == no items Shaddai 
        if ( rhsvl < 0 )
          rhsvl = 0;
        if ( !*opr )
          strcpy(opr, "==");
        return mprog_veval(lhsvl, opr, rhsvl, mob);                   */
    }
    if ( !str_cmp(chck, "otyperoom") )
    {
        progbug("Unsupported ifcheck 'otyperoom'", mob);
        return BERR;
        /*    OBJ_DATA *pObj;
            int type;
            
            if ( is_number(cvar) )
              type = atoi(cvar);
            else
              type = get_otype(cvar);
            if ( type < 0 || type > MAX_ITEM_TYPE )
            {
              progbug("OtypeRoom: bad type", mob);
              return BERR;
            }
            lhsvl = 0;
            for ( pObj = mob->in_room->first_content; pObj;
                  pObj = pObj->next_content )
              if ( pObj->item_type == type )
        	lhsvl+=pObj->count;
            rhsvl = is_number(rval) ? atoi(rval) : -1;
         
            if ( rhsvl < 0 )
              rhsvl = 0;
            if ( !*opr )
              strcpy(opr, "==");
            return mprog_veval(lhsvl, opr, rhsvl, mob);*/
    }
    if ( !str_cmp(chck, "ovnumcarry") )
    {
        progbug("Unsupported ifcheck 'economy'", mob);
        return BERR;
        /*OBJ_DATA *pObj;
        int vnum = atoi(cvar);

        if ( vnum < 1 || vnum > 32767 )
    {
          progbug("OvnumCarry: bad vnum", mob);
          return BERR;
    }
        lhsvl = 0;

        for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
          if ( pObj->pIndexData->vnum == vnum )
        lhsvl+=pObj->count;
        rhsvl = is_number(rval) ? atoi(rval) : -1;

        if ( rhsvl < 0 )
          rhsvl = 0;
        if ( !*opr )
          strcpy(opr, "==");
        return mprog_veval(lhsvl, opr, rhsvl, mob);
        */
    }
    if ( !str_cmp(chck, "otypecarry") )
    {
        progbug("Unsupported ifcheck 'otypecarry'", mob);
        return BERR;
        /*OBJ_DATA *pObj;
        int type;

        if ( is_number(cvar) )
          type = atoi(cvar);
        else
          type = get_otype(cvar);
        if ( type < 0 || type > MAX_ITEM_TYPE )
    {
          progbug("OtypeCarry: bad type", mob);
          return BERR;
    }
        lhsvl = 0;
        for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
          if ( pObj->item_type == type )
        lhsvl+=pObj->count;
        rhsvl = is_number(rval) ? atoi(rval) : -1;
        if ( rhsvl < 0 )
          rhsvl = 0;
        if ( !*opr )
          strcpy(opr, "==");
        return mprog_veval(lhsvl, opr, rhsvl, mob);
        */
    }
    if ( !str_cmp(chck, "ovnumwear") )
    {
        progbug("Unsupported ifcheck 'ovnumwear'", mob);
        return BERR;
        /*
         OBJ_DATA *pObj;
         int vnum = atoi(cvar);
         
         if ( vnum < 1 || vnum > 32767 )
         {
           progbug("OvnumWear: bad vnum", mob);
           return BERR;
         }
         lhsvl = 0;
         for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
           if ( pObj->wear_loc != WEAR_NONE &&
                pObj->pIndexData->vnum == vnum )
        lhsvl+=pObj->count;
         rhsvl = is_number(rval) ? atoi(rval) : -1;

         if ( rhsvl < 0 )
           rhsvl = 0;
         if ( !*opr )
           strcpy(opr, "==");
         return mprog_veval(lhsvl, opr, rhsvl, mob);
         */
    }
    if ( !str_cmp(chck, "otypewear") )
    {
        progbug("Unsupported ifcheck 'otypewear'", mob);
        return BERR;
        /*
         OBJ_DATA *pObj;
         int type;
         
         if ( is_number(cvar) )
           type = atoi(cvar);
         else
           type = get_otype(cvar);
         if ( type < 0 || type > MAX_ITEM_TYPE )
         {
           progbug("OtypeWear: bad type", mob);
           return BERR;
         }
         lhsvl = 0;
         for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
           if ( pObj->wear_loc != WEAR_NONE && 
                pObj->item_type == type )
        lhsvl+=pObj->count;
         rhsvl = is_number(rval) ? atoi(rval) : -1;

         if ( rhsvl < 0 )
           rhsvl = 0;
         if ( !*opr )
           strcpy(opr, "==");
         return mprog_veval(lhsvl, opr, rhsvl, mob);
         */
    }
    if ( !str_cmp(chck, "ovnuminv") )
    {
        progbug("Unsupported ifcheck 'ovnuminv'", mob);
        return BERR;
        /*
         OBJ_DATA *pObj;
         int vnum = atoi(cvar);
         
         if ( vnum < 1 || vnum > 32767 )
         {
           progbug("OvnumInv: bad vnum", mob);
           return BERR;
         }
         lhsvl = 0;
         for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
           if ( pObj->wear_loc == WEAR_NONE &&
                pObj->pIndexData->vnum == vnum )
        lhsvl+=pObj->count;
         rhsvl = is_number(rval) ? atoi(rval) : -1;

         if ( rhsvl < 0 )
           rhsvl = 0;
         if ( !*opr )
           strcpy(opr, "==");
         return mprog_veval(lhsvl, opr, rhsvl, mob);
         */
    }
    if ( !str_cmp(chck, "otypeinv") )
    {
        progbug("Unsupported ifcheck 'otypeinv'", mob);
        return BERR;
        /*
         OBJ_DATA *pObj;
         int type;
         
         if ( is_number(cvar) )
           type = atoi(cvar);
         else
           type = get_otype(cvar);
         if ( type < 0 || type > MAX_ITEM_TYPE )
         {
           progbug("OtypeInv: bad type", mob);
           return BERR;
         }
         lhsvl = 0;
         for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
           if ( pObj->wear_loc == WEAR_NONE &&
                pObj->item_type == type )
        lhsvl+=pObj->count;
         rhsvl = is_number(rval) ? atoi(rval) : -1;

         if ( rhsvl < 0 )
           rhsvl = 0;
         if ( !*opr )
           strcpy(opr, "==");
         return mprog_veval(lhsvl, opr, rhsvl, mob);
         */
    }
    if ( chkchar )
    {
        if ( !str_cmp(chck, "ispacifist") )
        {
            progbug("Unsupported ifcheck 'ispacifist'", mob);
            return BERR;
            //return (IS_NPC(chkchar) && xIS_SET(chkchar->act, ACT_PACIFIST));
        }
        if ( !str_cmp(chck, "ismobinvis") )
        {
            progbug("Unsupported ifcheck 'ismobinvis'", mob);
            return (IS_AFFECTED(chkchar, AFF_INVISIBLE));
            //return (IS_NPC(chkchar) && xIS_SET(chkchar->act, ACT_MOBINVIS));
        }
        if ( !str_cmp(chck, "mobinvislevel") )
        {
            progbug("Unsupported ifcheck 'mobinvislevel'", mob);
            return (IS_AFFECTED(chkchar, AFF_INVISIBLE)? GET_LEVEL(chkchar):0);

            //return (IS_NPC(chkchar) ?  mprog_veval(chkchar->mobinvis, opr, atoi(rval), mob) : FALSE);
        }
        if ( !str_cmp(chck, "ispc") )
        {
            return IS_NPC(chkchar) ? FALSE : TRUE;
        }
        if ( !str_cmp(chck, "isnpc") )
        {
            return IS_NPC(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "cansee") )
        {
            return CAN_SEE( mob , chkchar );
        }
        if ( !str_cmp(chck, "ispassage") )
        {
            if ( find_door2( chkchar, rval , NULL ) == NULL )
                return FALSE;
            else
                return TRUE;
        }
        if ( !str_cmp(chck, "isopen" ) )
        {
            //progbug("Unsupported ifcheck 'isopen'", mob);
            //return BERR;
            EXIT_DATA * pexit;
            if ( (pexit = find_door2( chkchar, rval, NULL)) == NULL )
                return FALSE;
            if (!IS_SET(pexit->exit_info, EX_CLOSED))
                return TRUE;
            return FALSE;
        }
        if ( !str_cmp(chck, "islocked" ) )
        {

            EXIT_DATA *pexit;
            if ( (pexit = find_door2( chkchar, rval, NULL)) == NULL )
                return FALSE;
            if (!IS_SET(pexit->exit_info, EX_LOCKED))
                return TRUE;
            return FALSE;
        }
        if ( !str_cmp(chck, "ispkill") )
        {
            progbug("Unsupported ifcheck 'ispkill'", mob);
            return BERR;
            //return IS_PKILL(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isdevoted") )
        {
            progbug("Unsupported ifcheck 'isdevoted'", mob);
            return BERR;
            //return IS_DEVOTED(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "canpkill") )
        {
            progbug("Unsupported ifcheck 'canpkill'", mob);
            return BERR;
            //return CAN_PKILL(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "ismounted") )
        {   progbug("Unsupported ifcheck 'ismounted'", mob);
            return BERR;
            //return (chkchar->position == POS_MOUNTED);
        }
        if ( !str_cmp(chck, "ismorphed") )
        {   progbug("Unsupported ifcheck 'ismorphed'", mob);
            return BERR;
            //return (chkchar->morph != NULL) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isnuisance" ) )
        {
            progbug("Unsupported ifcheck 'isnuisance'", mob);
            return BERR;
            //return (!IS_NPC(chkchar)? chkchar->pcdata->nuisance? TRUE: FALSE: FALSE);
        }
        if ( !str_cmp(chck, "isgood") )
        {
            return IS_GOOD(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isneutral") )
        {
            return IS_NEUTRAL(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isevil") )
        {
            return IS_EVIL(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isfight") )
        {
            return FIGHTING(chkchar) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isimmort") )
        {
            return (IS_IMMORT(chkchar));
        }
        if ( !str_cmp(chck, "ischarmed") )
        {
            return IS_AFFECTED(chkchar, AFF_CHARM) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isflying") )
        {
            return IS_AFFECTED(chkchar, AFF_FLYING) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "isthief") )
        {
            progbug("Unsupported ifcheck 'isthief'", mob);
            return (IS_THIEF(chkchar));

            //return ( !IS_NPC(chkchar) && xIS_SET(chkchar->act, PLR_THIEF) );
        }
        if ( !str_cmp(chck, "isattacker") )
        {
            progbug("Unsupported ifcheck 'isattacker'", mob);
            return BERR;
            //return ( !IS_NPC(chkchar) && xIS_SET(chkchar->act, PLR_ATTACKER) );
        }
        if ( !str_cmp(chck, "iskiller") )
        {
            progbug("Unsupported ifcheck 'iskiller'", mob);
            return BERR;
            //return ( !IS_NPC(chkchar) && xIS_SET(chkchar->act, PLR_KILLER) );
        }
        if ( !str_cmp(chck, "isfollow") )
        {
            return (chkchar->master != NULL &&
                    chkchar->master->in_room == chkchar->in_room);
        }
        if ( !str_cmp(chck, "isaffected") )
        {
            int value = get_aflag(rval);

            if ( value < 0 || value > 31 )
            {
                progbug("Unknown affect being checked", mob);
                return BERR;
            }
            return IS_AFFECTED(chkchar, value) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "numfighting") )
        {
            progbug("Unsupported ifcheck 'numfighting'", mob);
            return BERR;
            //return mprog_veval(chkchar->num_fighting-1, opr, atoi(rval), mob );
        }
        if ( !str_cmp(chck, "hitprcnt") )
        {
            return mprog_veval(((chkchar->points.hit)*100/(chkchar->points.max_hit)), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "inroom") )
        {
            return mprog_veval(GET_ROOM_VNUM(chkchar->in_room), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "wasinroom") )
        {
            if ( !chkchar->was_in_room )
                return FALSE;
            return mprog_veval(GET_ROOM_VNUM(chkchar->was_in_room), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "norecall") )
        {
            progbug("Unsupported ifcheck 'norecall'", mob);
            return (ROOM_FLAGGED(chkchar->in_room, ROOM_NOMAGIC));
            //return IS_SET(chkchar->in_room->room_flags, ROOM_NO_RECALL) ? TRUE : FALSE;
        }
        if ( !str_cmp(chck, "sex") )
        {
            return mprog_veval(chkchar->player.sex, opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "position") )
        {
            return mprog_veval(chkchar->char_specials.position, opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "doingquest") )
        {
            progbug("Unsupported ifcheck 'doingquest'", mob);
            return BERR;
            //return IS_NPC(chkchar) ? FALSE :  mprog_veval(chkchar->pcdata->quest_number, opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "ishelled") )
        {
            progbug("Unsupported ifcheck 'ishelled'", mob);
            return BERR;
            //return IS_NPC(chkchar) ? FALSE :  mprog_veval(chkchar->pcdata->release_date, opr, atoi(rval), mob);
        }

        if ( !str_cmp(chck, "level") )
        {
            return mprog_veval(GET_LEVEL(chkchar), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "goldamt") )
        {
            return mprog_veval(chkchar->points.gold, opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "class") )
        {
            return mprog_seval((char *)pc_class_types[GET_CLASS_NUM(chkchar)], opr, rval, mob);

        }
        if ( !str_cmp(chck, "weight" ) )
        {
            return mprog_veval(IS_CARRYING_W(chkchar), opr, atoi(rval), mob );
        }

        if ( !str_cmp(chck, "hostdesc") )
        {
            progbug("Unsupported ifcheck 'hostdesc'", mob);
            return BERR;
            /*if ( IS_NPC(chkchar) || !chkchar->desc->host )
              return FALSE;
            return mprog_seval(chkchar->desc->host, opr, rval, mob);*/
        }
        if ( !str_cmp(chck, "multi") )
        {
            progbug("Unsupported ifcheck 'multi'", mob);
            return BERR;
            /*CHAR_DATA *ch;
            int lhsvl = 0;

            for ( ch = first_char; ch; ch = ch->next )
              if ( !IS_NPC( chkchar ) && !IS_NPC( ch )
            &&    ch->desc
            &&    chkchar->desc
            &&    ch->desc->host == chkchar->desc->host )
                lhsvl++;
            rhsvl = atoi(rval);
            if ( rhsvl < 0 ) rhsvl = 0;
            if ( !*opr ) strcpy( opr, "==" );
            return mprog_veval(lhsvl, opr, rhsvl, mob);*/
        }
        if ( !str_cmp(chck, "race") )
        {
            return mprog_seval((char *)pc_race_types[GET_RACE(chkchar)], opr,rval, mob);
        }
        if ( !str_cmp(chck, "morph" ) )
        {
            progbug("Unsupported ifcheck 'morph'", mob);
            return BERR;
            /*
                  if ( chkchar->morph == NULL )
                    return FALSE;
                  if ( chkchar->morph->morph == NULL )
                    return FALSE;
                  return mprog_veval(chkchar->morph->morph->vnum, opr, rhsvl, mob );*/
        }
        if ( !str_cmp(chck, "nuisance") )
        {
            progbug("Unsupported ifcheck 'nuisance'", mob);
            return BERR;
            /*
             if ( IS_NPC( chkchar ) || !chkchar->pcdata->nuisance )
            return FALSE;
             return mprog_veval(chkchar->pcdata->nuisance->flags, opr, rhsvl, mob );*/
        }
        if ( !str_cmp(chck, "clan") )
        {
            progbug("Unsupported ifcheck 'clan'", mob);
            return BERR;
            /*if ( IS_NPC(chkchar) || !chkchar->pcdata->clan )
              return FALSE;
            return mprog_seval(chkchar->pcdata->clan->name, opr, rval, mob);*/
        }
        /* Check added to see if the person isleader of == clan Shaddai */
        if (!str_cmp (chck, "isleader"))
        {
            progbug("Unsupported ifcheck 'isleader'", mob);
            return BERR;
            /*CLAN_DATA *temp;
            if ( IS_NPC ( chkchar ) )
                 return FALSE;
            if ( (temp = get_clan( rval )) == NULL )
                 return FALSE;
            if ( mprog_seval(chkchar->name, opr, temp->leader, mob ) ||
                 mprog_seval(chkchar->name, opr, temp->number1, mob ) ||
                 mprog_seval(chkchar->name, opr, temp->number2, mob ) )
                 return TRUE;
            else
                 return FALSE;*/
        }
        /* Check added to see if the person isleader of == clan Gorog */
        if (!str_cmp (chck, "isclanleader"))
        {
            progbug("Unsupported ifcheck 'isclanleader'", mob);
            return BERR;
            /*
                CLAN_DATA *temp;
                if ( IS_NPC ( chkchar ) )
                     return FALSE;
                if ( (temp = get_clan( rval )) == NULL )
                     return FALSE;
                if ( mprog_seval(chkchar->name, opr, temp->leader, mob ) )
                     return TRUE;
                else
                     return FALSE;*/
        }
        if (!str_cmp (chck, "isclan1"))
        {
            progbug("Unsupported ifcheck 'isclan1'", mob);
            return BERR;
            /*
                CLAN_DATA *temp;
                if ( IS_NPC ( chkchar ) )
                     return FALSE;
                if ( (temp = get_clan( rval )) == NULL )
                     return FALSE;
                if ( mprog_seval(chkchar->name, opr, temp->number1, mob ) )
                     return TRUE;
                else
                     return FALSE;*/
        }
        if (!str_cmp (chck, "isclan2"))
        {
            progbug("Unsupported ifcheck 'isclan2'", mob);
            return BERR;
            /*
                CLAN_DATA *temp;
                if ( IS_NPC ( chkchar ) )
                     return FALSE;
                if ( (temp = get_clan( rval )) == NULL )
                     return FALSE;
                if ( mprog_seval(chkchar->name, opr, temp->number2, mob ) )
                     return TRUE;
                else
                     return FALSE;*/
        }
        if ( !str_cmp(chck, "council") )
        {
            progbug("Unsupported ifcheck 'council'", mob);
            /*
             if ( IS_NPC(chkchar) || !chkchar->pcdata->council )
               return FALSE;
             return mprog_seval(chkchar->pcdata->council->name, opr, rval, mob);*/
        }
        if ( !str_cmp(chck, "deity") )
        {
            progbug("Unsupported ifcheck 'deity'", mob);
            return BERR;
            /*if (IS_NPC(chkchar) || !chkchar->pcdata->deity )
            return FALSE;
            return mprog_seval(chkchar->pcdata->deity->name, opr, rval, mob);*/
        }
        if ( !str_cmp(chck, "guild") )
        {
            progbug("Unsupported ifcheck 'guild'", mob);
            return BERR;
            /*if ( IS_NPC(chkchar) || !IS_GUILDED(chkchar) )
              return FALSE;
            return mprog_seval(chkchar->pcdata->clan->name, opr, rval, mob);*/
        }
        if ( !str_cmp(chck, "clantype") )
        {
            progbug("Unsupported ifcheck 'clantype'", mob);
            return BERR;
            /*
             if ( IS_NPC(chkchar) || !chkchar->pcdata->clan )
               return FALSE;
             return mprog_veval(chkchar->pcdata->clan->clan_type, opr, atoi(rval),
                 mob);*/
        }
        /* Is char wearing some specific vnum?  -- Gorog */
        /*if (!str_cmp (chck, "wearingvnum"))
    {
            OBJ_DATA *obj;

            if ( !is_number(rval) )
        	return FALSE;
            for (obj=chkchar->first_carrying; obj; obj=obj->next_content)
            {
        	if ( chkchar==obj->carried_by
        	&&   obj->wear_loc > -1 
        	&&   obj->pIndexData->vnum == atoi(rval) )
        	    return TRUE;
            }
            return FALSE;
    } */

        /* Is char carrying a specific piece of eq?  -- Gorog */
        if (!str_cmp (chck, "carryingvnum"))
        {
            int vnum;

            if ( !is_number(rval) )
                return FALSE;
            vnum = atoi(rval);
            if ( !chkchar->carrying )
                return FALSE;
            return (carryingvnum_visit(chkchar, vnum));
        }

        if ( !str_cmp(chck, "waitstate") )
        {
            return mprog_veval(IS_NPC(chkchar)?GET_MOB_WAIT(chkchar):chkchar->desc->wait, opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "asupressed") )
        {
            progbug("Unsupported ifcheck 'asuppressed'", mob);
            return BERR;
            //return mprog_veval( get_timer( chkchar, TIMER_ASUPRESSED ), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "favor") )
        {
            progbug("Unsupported ifcheck 'favor'", mob);
            return BERR;
            /*if ( IS_NPC(chkchar) || !chkchar->pcdata->favor )
              return FALSE;
            return mprog_veval(chkchar->pcdata->favor, opr, atoi(rval), mob);*/
        }
        if ( !str_cmp(chck, "hps") )
        {
            return mprog_veval(chkchar->points.hit, opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "mana") )
        {
            return mprog_veval(chkchar->points.mana, opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "str") )
        {
            return mprog_veval(GET_STR(chkchar), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "wis") )
        {
            return mprog_veval(GET_WIS(chkchar), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "int") )
        {
            return mprog_veval(GET_INT(chkchar), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "dex") )
        {
            return mprog_veval(GET_DEX(chkchar), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "con") )
        {
            return mprog_veval(GET_CON(chkchar), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "cha") )
        {
            return mprog_veval(GET_CHA(chkchar), opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "lck") )
        {
            progbug("Unsupported ifcheck 'lck = luck'", mob);
            return mprog_veval(GET_CHA(chkchar), opr, atoi(rval), mob);
        }
    }
    if ( chkobj )
    {
        if ( !str_cmp(chck, "objtype") )
        {
            progbug("Check this out: objtype ifcheck", mob);
            return mprog_veval(chkobj->obj_flags.type_flag, opr, get_otype(rval), mob);
        }
        if ( !str_cmp(chck, "leverpos") )
        {
            //progbug("Unsupported ifcheck 'leverpos'", mob);
            //return BERR;

            int isup = FALSE, wantsup=FALSE;
            if ( GET_OBJ_TYPE(chkobj)!=ITEM_LEVER ||  GET_OBJ_TYPE(chkobj)!=ITEM_BUTTON)
                return FALSE;

            if ( GET_OBJ_VAL(chkobj,0)== TRIG_UP )
                isup = TRUE;
            if ( !str_cmp( rval, "up" ) )
                wantsup = TRUE;
            return mprog_veval( wantsup, opr, isup, mob );
        }
        if ( !str_cmp(chck, "objval0") )
        {
            return mprog_veval(chkobj->obj_flags.value[0], opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "objval1") )
        {
            return mprog_veval(chkobj->obj_flags.value[1], opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "objval2") )
        {
            return mprog_veval(chkobj->obj_flags.value[2], opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "objval3") )
        {
            return mprog_veval(chkobj->obj_flags.value[3], opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "objval4") )
        {
            progbug("Unsupported ifcheck 'objval4'", mob);
            return BERR;
            //return mprog_veval(chkobj->value[4], opr, atoi(rval), mob);
        }
        if ( !str_cmp(chck, "objval5") )
        {
            progbug("Unsupported ifcheck 'objval5'", mob);
            return BERR;
            //return mprog_veval(chkobj->value[5], opr, atoi(rval), mob);
        }
    }
    /* The following checks depend on the fact that cval[1] can only contain
       one character, and that NULL checks were made previously. */
    if ( !str_cmp(chck, "number") )
    {
        if ( chkchar )
        {
            if ( !IS_NPC(chkchar) )
                return FALSE;
            lhsvl = (chkchar == mob) ? chkchar->points.gold : GET_MOB_VNUM(chkchar);
            return mprog_veval(lhsvl, opr, atoi(rval), mob);
        }
        return mprog_veval(GET_OBJ_VNUM(chkobj), opr, atoi(rval), mob);
    }
    if ( !str_cmp(chck, "time") )
    {
        return mprog_veval(time_info.hours, opr, atoi(rval), mob );
    }
    if ( !str_cmp(chck, "name") )
    {
        if ( chkchar )
            return mprog_seval(chkchar->player.name, opr, rval, mob);
        return mprog_seval(chkobj->name, opr, rval, mob);
    }

    if ( !str_cmp(chck, "mortinworld") )   /* -- Gorog */
    {
        progbug("Unsupported ifcheck 'mortinworld'", mob);
        return BERR;
        /*DESCRIPTOR_DATA *d;
        for ( d = first_descriptor; d; d = d->next ) 
            if   ( d->connected == CON_PLAYING
            &&     d->character
            &&     get_trust(d->character) < LEVEL_IMMORTAL
            &&     nifty_is_name(d->character->name, cvar) )
                return TRUE;
        return FALSE;*/
    }

    if ( !str_cmp(chck, "mortinroom") )   /* -- Gorog */
    {
        progbug("Unsupported ifcheck 'mortinroom'", mob);
        return BERR;
        /*
          CHAR_DATA *ch;
          for ( ch = mob->in_room->first_person; ch; ch = ch->next_in_room )
            if ( (!IS_NPC(ch))
            &&   get_trust(ch) < LEVEL_IMMORTAL
            &&   nifty_is_name(ch->name, cvar) )
                 return TRUE;
          return FALSE;*/
    }

    if ( !str_cmp(chck, "mortinarea") )   /* -- Gorog */
    {
        progbug("Unsupported ifcheck 'mortinarea'", mob);
        return BERR;
        /*
        CHAR_DATA *ch;
        for ( ch = first_char; ch; ch = ch->next )
            if ( (!IS_NPC(ch))
            &&   ch->in_room->area == mob->in_room->area
            &&   get_trust(ch) < LEVEL_IMMORTAL
            &&   nifty_is_name(ch->name, cvar) )
                 return TRUE;
        return FALSE;*/
    }


    if ( !str_cmp(chck, "mortcount") )   /* -- Gorog */
    {
        /*CHAR_DATA *tch;
        ROOM_INDEX_DATA *room;
        int count = 0;
        int rvnum = atoi( cvar );

        room = get_room_index ( rvnum ? rvnum : mob->in_room->vnum );    

        for ( tch = room?room->first_person:NULL; tch; tch = tch->next_in_room )
          if ( (!IS_NPC(tch))
          &&   get_trust(tch) < LEVEL_IMMORTAL )
               count++;*/
        return mprog_veval(players_online(), opr, atoi(rval), mob);
    }


    if ( !str_cmp(chck, "mobcount") )   /* -- Gorog */
    {
        progbug("Unsupported ifcheck 'mobcount'", mob);
        return BERR;
        /*
        CHAR_DATA *tch;
        ROOM_INDEX_DATA *room;
        int count = -1;
        int rvnum = atoi( cvar );

        room = get_room_index ( rvnum ? rvnum : mob->in_room->vnum );    

        for ( tch = room?room->first_person:NULL; tch; tch = tch->next_in_room )
          if ( (IS_NPC(tch)) )
               count++;
        return mprog_veval(count, opr, atoi(rval), mob);*/
    }


    if ( !str_cmp(chck, "charcount") )   /* -- Gorog */
    {
        /*
          CHAR_DATA *tch;
          ROOM_INDEX_DATA *room;
          int count = -1;
          int rvnum = atoi( cvar );

          room = get_room_index ( rvnum ? rvnum : mob->in_room->vnum );    

          for ( tch = room?room->first_person:NULL; tch; tch = tch->next_in_room )

            if ( ( (!IS_NPC(tch))
            &&      get_trust(tch) < LEVEL_IMMORTAL
                 )
            ||      IS_NPC(tch)  
               )
                 count++;*/
        return mprog_veval(players_online(), opr, atoi(rval), mob);
    }



    /* Ok... all the ifchecks are done, so if we didnt find ours then something
     * odd happened.  So report the bug and abort the MUDprogram (return error)
     */
    progbug( "Unknown ifcheck", mob );
    return BERR;
}




char            null[1];

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
void            mprog_translate(char ch, char *t, struct char_data * mob, struct char_data * actor,
                                struct obj_data * obj, void *vo, struct char_data * rndm)
{
    static char    *he_she[] = {"it", "he", "she"};
    static char    *him_her[] = {"it", "him", "her"};
    static char    *his_her[] = {"its", "his", "her"};
    struct char_data *vict = (struct char_data *) vo;
    struct obj_data *v_obj = (struct obj_data *) vo;

    if ( v_obj && !PURGED(v_obj))// && v_obj->serial )
        vict = NULL;
    else
        v_obj = NULL;

    *t = '\0';
    switch (ch) {
    case 'i':
        one_argument(mob->player.name, t);
        break;

    case 'I':
        strcpy(t, mob->player.short_descr);
        break;

    case 'Z':
        if (!DEAD(mob))
            strcpy(t,world[mob->in_room].name);
        else
            strcpy(t, "nowhere");
        break;

    case 'n':
        if (actor && !DEAD(actor)) {
            if (CAN_SEE(mob, actor)) {
                if (!IS_NPC(actor)) {
                    strcpy(t, actor->player.name);
                } else
                {
                    one_argument(actor->player.name, t);
                    *t=UPPER(*t);
                }
            } else
                strcpy(t, "Someone");
        }
        else
            strcpy(t, "Someone");
        break;

    case 'N':
        if (actor && !DEAD(actor))
        {
            if (CAN_SEE(mob, actor))
                if (IS_NPC(actor))
                    strcpy(t, actor->player.short_descr);
                else {
                    strcpy(t, actor->player.name);
                    strcat(t, " ");
                    strcat(t, actor->player.title);
                }
            else
                strcpy(t, "someone");
        }
        else strcpy(t, "someone");
        break;

    case 't':
        if (vict && !DEAD(vict)) {
            if (CAN_SEE(mob, vict)) {
                if (!IS_NPC(vict))
                    strcpy(t, vict->player.name);
                else
                {
                    one_argument(vict->player.name, t);
                    *t = UPPER( *t );
                }
            } else
                strcpy(t, "Someone");
        }
        else strcpy(t, "Someone");
        break;

    case 'T':
        if (vict && !DEAD(vict)){
            if (CAN_SEE(mob, vict))
                if (IS_NPC(vict))
                    strcpy(t, vict->player.short_descr);
                else {
                    strcpy(t, vict->player.name);
                    strcat(t, " ");
                    strcat(t, vict->player.title);
                }
            else
                strcpy(t, "someone");
        }
        else strcpy(t, "someone");
        break;

    case 'r':
        if (rndm && !DEAD(rndm)) {
            if (CAN_SEE(mob, rndm)) {
                if (!IS_NPC(rndm))
                    strcpy(t, rndm->player.name);
                else
                {
                    one_argument(rndm->player.name, t);
                    *t = UPPER( *t );
                }
            } else
                strcpy(t, "Someone");
        }
        else strcpy(t, "Someone");
        break;

    case 'R':
        if (rndm && !DEAD(rndm)) {
            if (CAN_SEE(mob, rndm))
                if (IS_NPC(rndm))
                    strcpy(t, rndm->player.short_descr);
                else {
                    strcpy(t, rndm->player.name);
                    strcat(t, " ");
                    strcat(t, rndm->player.title);
                }
            else
                strcpy(t, "someone");
        }
        else strcpy(t, "someone");
        break;

    case 'e':
        if (actor && !DEAD(actor))
            CAN_SEE(mob, actor) ? strcpy(t, he_she[(int) actor->player.sex])
            : strcpy(t, "someone");
        else strcpy(t, "it");
        break;

    case 'm':
        if (actor && !DEAD(actor))
            CAN_SEE(mob, actor) ? strcpy(t, him_her[(int) actor->player.sex])
            : strcpy(t, "someone");
        else strcpy(t, "it");
        break;

    case 's':
        if (actor && !DEAD(actor))
            CAN_SEE(mob, actor) ? strcpy(t, his_her[(int) actor->player.sex])
            : strcpy(t, "someone's");
        else strcpy(t, "its");
        break;

    case 'E':
        if (vict && !DEAD(vict))
            CAN_SEE(mob, vict) ? strcpy(t, he_she[(int) vict->player.sex])
            : strcpy(t, "someone");
        else strcpy(t, "it");
        break;

    case 'M':
        if (vict && !DEAD(vict))
            CAN_SEE(mob, vict) ? strcpy(t, him_her[(int) vict->player.sex])
            : strcpy(t, "someone");
        else strcpy(t, "it");
        break;

    case 'S':
        if (vict && !DEAD(vict))
            CAN_SEE(mob, vict) ? strcpy(t, his_her[(int) vict->player.sex])
            : strcpy(t, "someone's");
        else strcpy(t, "its");
        break;

    case 'j':
        strcpy(t, he_she[(int) mob->player.sex]);
        break;

    case 'k':
        strcpy(t, him_her[(int) mob->player.sex]);
        break;

    case 'l':
        strcpy(t, his_her[(int) mob->player.sex]);
        break;

    case 'J':
        if (rndm && !DEAD(rndm))
            CAN_SEE(mob, rndm) ? strcpy(t, he_she[(int) rndm->player.sex])
            : strcpy(t, "someone");
        else strcpy(t, "it");
        break;

    case 'K':
        if (rndm && !DEAD(rndm))
            CAN_SEE(mob, rndm) ? strcpy(t, him_her[(int) rndm->player.sex])
            : strcpy(t, "someone");
        else strcpy(t, "it");
        break;

    case 'L':
        if (rndm && !DEAD(rndm))
            CAN_SEE(mob, rndm) ? strcpy(t, his_her[(int) rndm->player.sex])
            : strcpy(t, "someone's");
        else strcpy(t, "its");
        break;

    case 'o':
        if (obj && !PURGED(obj))
            CAN_SEE_OBJ(mob, obj) ? one_argument(obj->name, t)
            : strcpy(t, "something");
        else
            strcpy( t, "something" );
        break;

    case 'O':
        if (obj && !PURGED(obj))
            CAN_SEE_OBJ(mob, obj) ? strcpy(t, obj->short_description)
            : strcpy(t, "something");
        else strcpy( t, "something" );

        break;

    case 'p':
        if (v_obj && !PURGED(v_obj))
            CAN_SEE_OBJ(mob, v_obj) ? one_argument(v_obj->name, t)
            : strcpy(t, "something");
        else
            strcpy( t, "something" );
        break;

    case 'P':
        if (v_obj && !PURGED(v_obj))
            CAN_SEE_OBJ(mob, v_obj) ? strcpy(t, v_obj->short_description)
            : strcpy(t, "something");
        else
            strcpy( t, "something" );
        break;

    case 'a':
        if (obj)
            switch (*(obj->name)) {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                strcpy(t, "an");
                break;
            default:
                strcpy(t, "a");
            }

        break;

    case 'A':
        if (v_obj)
            switch (*(v_obj->name)) {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                strcpy(t, "an");
                break;
            default:
                strcpy(t, "a");
            }
        break;

    case '$':
        strcpy(t, "$");
        break;

    default:
        bug("Mob: %d bad $var", mob_index[mob->nr].virtual);
        break;
    }

    return;

}



int prog_nest=0;

struct waitdriver_eo
{
    CHAR_DATA *mob, *actor, *rndm;
    OBJ_DATA *obj;
    void *vo;
    bool single_step;
    bool ifstate[MAX_IFS][DO_ELSE+1];
    int prog_nest;
    char tmpcmndlst[MAX_STRING_LENGTH];
    int command_list;
    room_num room;
    int iflevel, ignorelevel;
    int isobjprog;
};


struct wds {
    struct event * pevent;
    struct waitdriver_eo *eo;
    struct wds *next, *prev;
};

struct wds *wait_queue=NULL;

void add_waitq(struct event *pevent, struct waitdriver_eo *eo)
{
    struct wds *pom;

    if (!pevent)
        return;

    CREATE(pom, struct wds, 1);
    pom->pevent=pevent;
    pom->eo=eo;
    pom->next=wait_queue;
    pom->prev=NULL;

    if (wait_queue)
        wait_queue->prev=pom;

    wait_queue=pom;
}

void remove_waitq(struct waitdriver_eo *elem)
{
    struct wds *pom=wait_queue, *temp;

    while (pom)
    {
        if (pom->eo==elem)
        {
            if (pom->prev)
            {
                pom->prev->next=pom->next;
                if (pom->next)
                    pom->next->prev=pom->prev;

            }
            else
            {
                if (pom->next)
                    pom->next->prev=NULL;
                wait_queue=pom->next;
            }
            DISPOSE(pom);
            break;
        }
        pom=pom->next;
    }
}

void event_null_char(CHAR_DATA *ch)
{
    struct wds *pom=wait_queue, *temp;

    while (pom)
    {
        temp=pom->next;
        if (pom->eo->actor==ch)
            pom->eo->actor=NULL;
        if (pom->eo->rndm==ch)
            pom->eo->rndm=NULL;
        if (pom->eo->mob==ch)
        {
            event_cancel(pom->pevent);
            remove_waitq(pom->eo);
        }
        pom=temp;
    }
}

void event_null_obj(OBJ_DATA *ch)
{
    struct wds *pom=wait_queue, *temp;

    while (pom)
    {
        temp=pom->next;
        if (pom->eo->obj==ch)
        {
            if (pom->eo->isobjprog)
            {
                event_cancel(pom->pevent);
                remove_waitq(pom->eo);
            }
            else
                pom->eo->obj=NULL;
        }
        pom=temp;
    }
}




EVENTFUNC(waitdriver)
{


    struct waitdriver_eo *nevent, *cevent=(struct waitdriver_eo *) event_obj;
    struct char_data *ch;
    char *pom, *pom1, cmd[1000];
    struct char_data *temp;
    room_num room;

    char *tmpcmndlst;
    char *command_list;
    char *cmnd;
    CHAR_DATA *rndm  = NULL;
    CHAR_DATA *vch   = NULL;
    CHAR_DATA *mob;
    int count        = 0;
    int ignorelevel  = 0;
    int iflevel, result;
    bool ifstate[MAX_IFS][DO_ELSE+1];
    //int prog_nest;


    global_newsupermob=0;

    tmpcmndlst=cevent->tmpcmndlst;
    command_list=&cevent->tmpcmndlst[cevent->command_list];
    memcpy(ifstate, cevent->ifstate, sizeof(bool)*MAX_IFS*(DO_ELSE+1));
    mob=cevent->mob;
    prog_nest=cevent->prog_nest;
    iflevel=cevent->iflevel;
    if (IS_SUPERMOB(mob))
    {
        if (cevent->isobjprog && cevent->obj)
            set_supermob(cevent->obj);
        else
            rset_supermob(&world[cevent->room]);
    }


    while ( TRUE )
    {
        /* With these two lines, cmnd becomes the current line from the prog,
           and command_list becomes everything after that line. */
        cmnd         = command_list;
        command_list = mprog_next_command( command_list );

        /* Are we at the end? */
        if ( cmnd[0] == '\0' )
        {
            if ( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] )
            {
                progbug( "Missing endif", mob );
            }
            --prog_nest;
            if (global_newsupermob)
                init_supermob();
            goto kraj_event;
        }

        /* Evaluate/execute the command, check what happened. */
        global_wait=0;
        result = mprog_do_command( cmnd, mob, cevent->actor, cevent->obj, cevent->vo, cevent->rndm,
                                   ( ifstate[iflevel][IN_IF] && !ifstate[iflevel][DO_IF] )
                                   || ( ifstate[iflevel][IN_ELSE] && !ifstate[iflevel][DO_ELSE] ),
                                   ( ignorelevel > 0 ) );
        if (global_wait)
        {
            CREATE(nevent, struct waitdriver_eo, 1);
            nevent->mob=cevent->mob;
            nevent->actor=cevent->actor;
            nevent->rndm=cevent->rndm;
            nevent->obj=cevent->obj;
            nevent->vo=cevent->vo;
            nevent->room=mob->in_room;
            nevent->single_step=cevent->single_step;
            memcpy(nevent->ifstate, ifstate, sizeof(bool)*MAX_IFS*(DO_ELSE+1));
            nevent->prog_nest=cevent->prog_nest;
            //strcpy(nevent->tmpcmndlst, tmpcmndlst);
            memcpy(nevent->tmpcmndlst, tmpcmndlst, sizeof(char)*MAX_STRING_LENGTH);
            nevent->command_list=command_list-tmpcmndlst;
            nevent->iflevel=iflevel;
            nevent->ignorelevel=ignorelevel;
            nevent->isobjprog=cevent->isobjprog;
            add_waitq(event_create(waitdriver, nevent, global_wait RL_SEC), nevent);
            goto kraj_event;
        }

        global_wait=0;
        /* Script prog support  -Thoric */
        if ( cevent->single_step )
        {
            mob->mpscriptpos = command_list - tmpcmndlst;
            --prog_nest;
            if (global_newsupermob)
                init_supermob();
            goto kraj_event;
        }

        /* This is the complicated part.  Act on the goto kraj_evented value from
           mprog_do_command according to the current logic state. */
        switch ( result )
        {
        case COMMANDOK:


#ifdef DEBUG
            //log_string( "COMMANDOK" );
#endif
            /* Ok, this one's a no-brainer. */
            continue;
            break;

        case IFTRUE:
#ifdef DEBUG
            //log_string( "IFTRUE" );
#endif
            /* An if was evaluated and found true.  Note that we are in an
               if section and that we want to execute it. */
            iflevel++;
            if ( iflevel == MAX_IFS )
            {
                progbug( "Maximum nested ifs exceeded", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }

            ifstate[iflevel][IN_IF] = TRUE;
            ifstate[iflevel][DO_IF] = TRUE;
            break;

        case IFFALSE:
#ifdef DEBUG
            //log_string( "IFFALSE" );
#endif
            /* An if was evaluated and found false.  Note that we are in an
               if section and that we don't want to execute it unless we find
               an or that evaluates to true. */
            iflevel++;
            if ( iflevel == MAX_IFS )
            {
                progbug( "Maximum nested ifs exceeded", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }
            ifstate[iflevel][IN_IF] = TRUE;
            ifstate[iflevel][DO_IF] = FALSE;
            break;

        case ORTRUE:
#ifdef DEBUG
            //log_string( "ORTRUE" );
#endif
            /* An or was evaluated and found true.  We should already be in an
               if section, so note that we want to execute it. */
            if ( !ifstate[iflevel][IN_IF] )
            {
                progbug( "Unmatched or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }
            ifstate[iflevel][DO_IF] = TRUE;
            break;

        case ORFALSE:
#ifdef DEBUG
            //log_string( "ORFALSE" );
#endif
            /* An or was evaluated and found false.  We should already be in an
               if section, and we don't need to do much.  If the if was true or
               there were/will be other ors that evaluate(d) to true, they'll set
               do_if to true. */
            if ( !ifstate[iflevel][IN_IF] )
            {
                progbug( "Unmatched or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }
            continue;
            break;

        case FOUNDELSE:
#ifdef DEBUG
            //log_string( "FOUNDELSE" );
#endif
            /* Found an else.  Make sure we're in an if section, bug out if not.
               If this else is not one that we wish to ignore, note that we're now 
               in an else section, and look at whether or not we executed the if 
               section to decide whether to execute the else section.  Ca marche 
               bien. */
            if ( ignorelevel > 0 )
                continue;

            if ( ifstate[iflevel][IN_ELSE] )
            {
                progbug( "Found else in an else section", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }
            if ( !ifstate[iflevel][IN_IF] )
            {
                progbug( "Unmatched else", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }

            ifstate[iflevel][IN_ELSE] = TRUE;
            ifstate[iflevel][DO_ELSE] = !ifstate[iflevel][DO_IF];
            ifstate[iflevel][IN_IF]   = FALSE;
            ifstate[iflevel][DO_IF]   = FALSE;

            break;

        case FOUNDENDIF:
#ifdef DEBUG
            //log_string( "FOUNDENDIF" );
#endif
            /* Hmm, let's see... FOUNDENDIF must mean that we found an endif.
               So let's make sure we were expecting one, goto kraj_event if not.  If this
               endif matches the if or else that we're executing, note that we are 
               now no longer executing an if.  If not, keep track of what we're 
               ignoring. */
            if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
                progbug( "Unmatched endif", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }

            if ( ignorelevel > 0 )
            {
                ignorelevel--;
                continue;
            }

            ifstate[iflevel][IN_IF]   = FALSE;
            ifstate[iflevel][DO_IF]   = FALSE;
            ifstate[iflevel][IN_ELSE] = FALSE;
            ifstate[iflevel][DO_ELSE] = FALSE;

            iflevel--;
            break;

        case IFIGNORED:
#ifdef DEBUG
            //log_string( "IFIGNORED" );
#endif
            if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
                progbug( "Parse error, ignoring if while not in if or else", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }
            ignorelevel++;
            break;

        case ORIGNORED:
#ifdef DEBUG
            //log_string( "ORIGNORED" );
#endif
            if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
                progbug( "Unmatched or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }
            if ( ignorelevel == 0 )
            {
                progbug( "Parse error, mistakenly ignoring or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                goto kraj_event;
            }

            break;

        case BERR:
#ifdef DEBUG
            //log_string( "BERR" );
#endif
            --prog_nest;
            if (global_newsupermob)
                init_supermob();
            goto kraj_event;
            break;
        }
    }
    --prog_nest;

kraj_event:
    remove_waitq(cevent);
    
    DISPOSE(cevent);
    release_supermob();

    if (global_newsupermob)
        init_supermob();
    return 0;
}











/*  The main focus of the MOBprograms.  This routine is called
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 *
 *  This function rewritten by Narn for Realms of Despair, Dec/95.
 *
 */   

void mprog_driver ( char *com_list, CHAR_DATA *mob, CHAR_DATA *actor,
                    OBJ_DATA *obj, void *vo, bool single_step)
{
    char tmpcmndlst[ MAX_STRING_LENGTH ];
    char *command_list;
    char *cmnd;
    CHAR_DATA *rndm  = NULL;
    CHAR_DATA *vch   = NULL;
    int count        = 0;
    int ignorelevel  = 0;
    int iflevel, result;
    bool ifstate[MAX_IFS][ DO_ELSE + 1 ];
    //static int prog_nest;
    struct waitdriver_eo *nevent;

    if (IS_AFFECTED( mob, AFF_CHARM ))
        return;
    if (DEAD(mob))
        return;

    /* Next couple of checks stop program looping. -- Altrag */
    if ( mob == actor )
    {
        progbug( "triggering oneself.", mob );
        //logs("%s", com_list);
        return;
    }

    if ( ++prog_nest > MAX_PROG_NEST )
    {
        progbug( "max_prog_nest exceeded.", mob );
        --prog_nest;
        return;
    }

    /* Make sure all ifstate bools are set to FALSE */
    for ( iflevel = 0; iflevel < MAX_IFS; iflevel++ )
    {
        for ( count = 0; count < DO_ELSE; count++ )
        {
            ifstate[iflevel][count] = FALSE;
        }
    }

    iflevel = 0;

    /*
     * get a random visible player who is in the room with the mob.
     *
     *  If there isn't a random player in the room, rndm stays NULL.
     *  If you do a $r, $R, $j, or $k with rndm = NULL, you'll crash
     *  in mprog_translate.
     *
     *  Adding appropriate error checking in mprog_translate.
     *    -Haus
     *
     * This used to ignore players MAX_LEVEL - 3 and higher (standard
     * Merc has 4 immlevels).  Thought about changing it to ignore all
     * imms, but decided to just take it out.  If the mob can see you, 
     * you may be chosen as the random player. -Narn
     *
     */






    count = 0;
    for (vch = world[mob->in_room].people; vch; vch = vch->next_in_room)
        if (!IS_NPC(vch)
                // && vch->player.level < LVL_IMMORT
                && CAN_SEE(mob, vch)) {
            if (number(0, count) == 0)
                rndm = vch;
            count++;
        }

    strcpy( tmpcmndlst, com_list );
    command_list = tmpcmndlst;
    if ( single_step )
    {
        if ( mob->mpscriptpos > strlen( tmpcmndlst ) )
            mob->mpscriptpos = 0;
        else
            command_list += mob->mpscriptpos;
        if ( *command_list == '\0' )
        {
            command_list = tmpcmndlst;
            mob->mpscriptpos = 0;
        }
    }

    /* From here on down, the function is all mine.  The original code
       did not support nested ifs, so it had to be redone.  The max 
       logiclevel (MAX_IFS) is defined at the beginning of this file, 
       use it to increase/decrease max allowed nesting.  -Narn 
    */
    global_newsupermob=0;
    while ( TRUE )
    {
        /* With these two lines, cmnd becomes the current line from the prog,
           and command_list becomes everything after that line. */
        cmnd         = command_list;
        command_list = mprog_next_command( command_list );

        /* Are we at the end? */
        if ( cmnd[0] == '\0' )
        {
            if ( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] )
            {
                progbug( "Missing endif", mob );
            }
            --prog_nest;
            if (global_newsupermob)
                init_supermob();
            return;
        }
        global_wait=0;
        /* Evaluate/execute the command, check what happened. */
        result = mprog_do_command( cmnd, mob, actor, obj, vo, rndm,
                                   ( ifstate[iflevel][IN_IF] && !ifstate[iflevel][DO_IF] )
                                   || ( ifstate[iflevel][IN_ELSE] && !ifstate[iflevel][DO_ELSE] ),
                                   ( ignorelevel > 0 ) );

        if (global_wait)
        {
            CREATE(nevent, struct waitdriver_eo, 1);
            nevent->mob=mob;
            nevent->actor=actor;
            nevent->rndm=rndm;
            nevent->obj=obj;
            nevent->vo=vo;
            nevent->single_step=single_step;
            memcpy(nevent->ifstate, ifstate, sizeof(bool)*MAX_IFS*(DO_ELSE+1));
            nevent->prog_nest=prog_nest;
            //strcpy(nevent->tmpcmndlst, tmpcmndlst);
            memcpy(nevent->tmpcmndlst, tmpcmndlst, sizeof(char)*MAX_STRING_LENGTH);
            nevent->command_list=command_list-tmpcmndlst;
            nevent->room=mob->in_room;
            nevent->iflevel=iflevel;
            nevent->ignorelevel=ignorelevel;
            nevent->isobjprog=(IS_SUPERMOB(mob) && IS_SET(MOB_FLAGS(mob), MOB_SPEC));
            add_waitq(event_create(waitdriver, nevent, global_wait RL_SEC), nevent);
            return;
        }


        /* Script prog support  -Thoric */
        if ( single_step )
        {
            mob->mpscriptpos = command_list - tmpcmndlst;
            --prog_nest;
            if (global_newsupermob)
                init_supermob();
            return;
        }

        /* This is the complicated part.  Act on the returned value from
           mprog_do_command according to the current logic state. */
        switch ( result )
        {
        case COMMANDOK:


#ifdef DEBUG
            //log_string( "COMMANDOK" );
#endif
            /* Ok, this one's a no-brainer. */
            continue;
            break;

        case IFTRUE:
#ifdef DEBUG
            //log_string( "IFTRUE" );
#endif
            /* An if was evaluated and found true.  Note that we are in an
               if section and that we want to execute it. */
            iflevel++;
            if ( iflevel == MAX_IFS )
            {
                progbug( "Maximum nested ifs exceeded", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }

            ifstate[iflevel][IN_IF] = TRUE;
            ifstate[iflevel][DO_IF] = TRUE;
            break;

        case IFFALSE:
#ifdef DEBUG
            //log_string( "IFFALSE" );
#endif
            /* An if was evaluated and found false.  Note that we are in an
               if section and that we don't want to execute it unless we find
               an or that evaluates to true. */
            iflevel++;
            if ( iflevel == MAX_IFS )
            {
                progbug( "Maximum nested ifs exceeded", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }
            ifstate[iflevel][IN_IF] = TRUE;
            ifstate[iflevel][DO_IF] = FALSE;
            break;

        case ORTRUE:
#ifdef DEBUG
            //log_string( "ORTRUE" );
#endif
            /* An or was evaluated and found true.  We should already be in an
               if section, so note that we want to execute it. */
            if ( !ifstate[iflevel][IN_IF] )
            {
                progbug( "Unmatched or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }
            ifstate[iflevel][DO_IF] = TRUE;
            break;

        case ORFALSE:
#ifdef DEBUG
            //log_string( "ORFALSE" );
#endif
            /* An or was evaluated and found false.  We should already be in an
               if section, and we don't need to do much.  If the if was true or
               there were/will be other ors that evaluate(d) to true, they'll set
               do_if to true. */
            if ( !ifstate[iflevel][IN_IF] )
            {
                progbug( "Unmatched or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }
            continue;
            break;

        case FOUNDELSE:
#ifdef DEBUG
            //log_string( "FOUNDELSE" );
#endif
            /* Found an else.  Make sure we're in an if section, bug out if not.
               If this else is not one that we wish to ignore, note that we're now 
               in an else section, and look at whether or not we executed the if 
               section to decide whether to execute the else section.  Ca marche 
               bien. */
            if ( ignorelevel > 0 )
                continue;

            if ( ifstate[iflevel][IN_ELSE] )
            {
                progbug( "Found else in an else section", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }
            if ( !ifstate[iflevel][IN_IF] )
            {
                progbug( "Unmatched else", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }

            ifstate[iflevel][IN_ELSE] = TRUE;
            ifstate[iflevel][DO_ELSE] = !ifstate[iflevel][DO_IF];
            ifstate[iflevel][IN_IF]   = FALSE;
            ifstate[iflevel][DO_IF]   = FALSE;

            break;

        case FOUNDENDIF:
#ifdef DEBUG
            //log_string( "FOUNDENDIF" );
#endif
            /* Hmm, let's see... FOUNDENDIF must mean that we found an endif.
               So let's make sure we were expecting one, return if not.  If this
               endif matches the if or else that we're executing, note that we are 
               now no longer executing an if.  If not, keep track of what we're 
               ignoring. */
            if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
                progbug( "Unmatched endif", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }

            if ( ignorelevel > 0 )
            {
                ignorelevel--;
                continue;
            }

            ifstate[iflevel][IN_IF]   = FALSE;
            ifstate[iflevel][DO_IF]   = FALSE;
            ifstate[iflevel][IN_ELSE] = FALSE;
            ifstate[iflevel][DO_ELSE] = FALSE;

            iflevel--;
            break;

        case IFIGNORED:
#ifdef DEBUG
            //log_string( "IFIGNORED" );
#endif
            if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
                progbug( "Parse error, ignoring if while not in if or else", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }
            ignorelevel++;
            break;

        case ORIGNORED:
#ifdef DEBUG
            //log_string( "ORIGNORED" );
#endif
            if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
            {
                progbug( "Unmatched or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }
            if ( ignorelevel == 0 )
            {
                progbug( "Parse error, mistakenly ignoring or", mob );
                --prog_nest;
                if (global_newsupermob)
                    init_supermob();
                return;
            }

            break;

        case BERR:
#ifdef DEBUG
            //log_string( "BERR" );
#endif
            --prog_nest;
            if (global_newsupermob)
                init_supermob();
            return;
            break;
        }
    }
    --prog_nest;
    if (global_newsupermob)
        init_supermob();
    return;
}

/* This function replaces mprog_process_cmnd.  It is called from
 * mprog_driver, once for each line in a mud prog.  This function
 * checks what the line is, executes if/or checks and calls interpret
 * to perform the the commands.  Written by Narn, Dec 95.
 */
int mprog_do_command( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor,
                      OBJ_DATA *obj, void *vo, CHAR_DATA *rndm,
                      bool ignore, bool ignore_ors )
{
    char firstword[MAX_INPUT_LENGTH];
    char *ifcheck;
    char buf[ MAX_INPUT_LENGTH ];
    char tmp[ MAX_INPUT_LENGTH ];
    char *point, *str, *i;
    int validif, vnum;

    /* Isolate the first word of the line, it gives us a clue what
       we want to do. */
    ifcheck = one_argument( cmnd, firstword );

    if ( !str_cmp( firstword, "if" ) )
    {
        /* Ok, we found an if.  According to the boolean 'ignore', either
           ignore the ifcheck and report that back to mprog_driver or do
           the ifcheck and report whether it was successful. */
        if ( ignore )
            return IFIGNORED;
        else
            validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

        if ( validif == 1 )
            return IFTRUE;

        if ( validif == 0 )
            return IFFALSE;

        return BERR;
    }

    if ( !str_cmp( firstword, "or" ) )
    {
        /* Same behavior as with ifs, but use the boolean 'ignore_ors' to
           decide which way to go. */
        if ( ignore_ors )
            return ORIGNORED;
        else
            validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

        if ( validif == 1 )
            return ORTRUE;

        if ( validif == 0 )
            return ORFALSE;

        return BERR;
    }

    /* For else and endif, just report back what we found.  Mprog_driver
       keeps track of logiclevels. */
    if ( !str_cmp( firstword, "else" ) )
    {
        return FOUNDELSE;
    }

    if ( !str_cmp( firstword, "endif" ) )
    {
        return FOUNDENDIF;
    }

    /* Ok, didn't find an if, an or, an else or an endif.
       If the command is in an if or else section that is not to be 
       performed, the boolean 'ignore' is set to true and we just 
       return.  If not, we try to execute the command. */

    if ( ignore )
        return COMMANDOK;

    /* If the command is 'break', that's all folks. */
    if ( !str_cmp( firstword, "break" ) )
        return BERR;

    vnum = GET_MOB_VNUM(mob);
    point   = buf;
    str     = cmnd;

    /* This chunk of code taken from mprog_process_cmnd. */
    while ( *str != '\0' )
    {
        if ( *str != '$' )
        {
            *point++ = *str++;
            continue;
        }
        str++;
        mprog_translate( *str, tmp, mob, actor, obj, vo, rndm );
        i = tmp;
        ++str;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
    }
    *point = '\0';

    //  CREF(mob, CHAR_NULL);
    command_interpreter( mob, buf );
    //  CUREF(mob);
    /* If the mob is mentally unstable and does things like fireball
       itself, let's make sure it's still alive. */
    if (DEAD(mob))
    {
        logs( "SYSERR: Mob died while executing program, vnum %d trying %s.", vnum, buf );
        return BERR;
    }

    return COMMANDOK;
}

/***************************************************************************
 * Global function code and brief comments.
 */

bool mprog_keyword_check( const char *argu, const char *argl )
{
    char word[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int i;
    char *arg, *arglist;
    char *start, *end;

    strcpy( arg1, strlower( argu ) );
    arg = arg1;
    strcpy( arg2, strlower( argl ) );
    arglist = arg2;

    for ( i = 0; i < strlen( arglist ); i++ )
        arglist[i] = LOWER( arglist[i] );
    for ( i = 0; i < strlen( arg ); i++ )
        arg[i] = LOWER( arg[i] );
    if ( ( arglist[0] == 'p' ) && ( arglist[1] == ' ' ) )
    {
        arglist += 2;
        while ( ( start = strstr( arg, arglist ) ) )
            if ( (start == arg || *(start-1) == ' ' )
                    && ( *(end = start + strlen( arglist ) ) == ' '
                         ||   *end == '\n'
                         ||   *end == '\r'
                         ||   *end == '\0' ) )
                return TRUE;
            else
                arg = start+1;
    }
    else
    {
        arglist = one_argument( arglist, word );
        for ( ; word[0] != '\0'; arglist = one_argument( arglist, word ) )
            while ( ( start = strstr( arg, word ) ) )
                if ( ( start == arg || *(start-1) == ' ' )
                        && ( *(end = start + strlen( word ) ) == ' '
                             ||   *end == '\n'
                             ||   *end == '\r'
                             ||   *end == '\0' ) )
                    return TRUE;
                else
                    arg = start +1;
    }
    /*    bug( "don't match" ); */
    return FALSE;
}


/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
                           OBJ_DATA *obj, void *vo, int type )
{

    char	      temp1[ MAX_STRING_LENGTH ];
    char	      temp2[ MAX_INPUT_LENGTH ];
    char	      word[ MAX_INPUT_LENGTH ];
    MPROG_DATA *mprg;
    char       *list;
    char       *start;
    char       *dupl;
    char       *end;
    char tpom[1000];
    char *pom;
    int	      i;
    char *sep;

    //for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )

    for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next)
        if ( mprg->type == type )
        {
            strcpy( temp1, mprg->arglist );
            list = temp1;
            skip_spaces(&list);
            for ( i = 0; i < strlen( list ); i++ )
                list[i] = LOWER( list[i] );
            strcpy( temp2, arg );
            dupl = temp2;
            for ( i = 0; i < strlen( dupl ); i++ )
                dupl[i] = LOWER( dupl[i] );
            if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
            {
                list += 2;
                while ( ( start = strstr( dupl, list ) ) )
                    if ( (start == dupl || *(start-1) == ' ' )
                            && ( *(end = start + strlen( list ) ) == ' '
                                 || *end == '\n'
                                 || *end == '\r'
                                 || *end == '\0' ) )
                    {
                        mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                        break;
                    }
                    else
                        dupl = start+1;
            }
            else if ( ( list[0] == 'q' || list[0] == 'r' ) && ( list[1] == ' ' ) )
            {
                list += 2;

                sep=strchr(list, '|');
                pom=one_argument(dupl, tpom);
                if (!sep)
                {

                    dupl=one_argument(list, word);
                    if (is_abbrev(tpom, word))
                        if (!(*pom) || isname(pom, dupl))
                        {
                            global_action=(*(list-2)=='q'?1:2);
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            break;
                        }
                }
                else
                {
                    *sep=0;

                    if (!(*list) || isname(tpom, list))
                        if (!(*(sep+1)) || isname(pom, sep+1))
                        {
                            global_action=(*(list-2)=='q'?1:2);
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            break;
                        }

                }
            }
            else
            {
                list = one_argument( list, word );
                for( ; word[0] != '\0'; list = one_argument( list, word ) )
                    while ( ( start = strstr( dupl, word ) ) )
                        if ( ( start == dupl || *(start-1) == ' ' )
                                && ( *(end = start + strlen( word ) ) == ' '
                                     || *end == '\n'
                                     || *end == '\r'
                                     || *end == '\0' ) )
                        {
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            break;
                        }
                        else
                            dupl = start+1;
            }
        }

    return;

}



void            mprog_percent_check(struct char_data * mob, struct char_data * actor, struct obj_data * obj,
                                    void *vo, int type)
{
    MPROG_DATA     *mprg;

    for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next)
        if ((mprg->type & type)
                && (number(1, 100) < atoi(mprg->arglist))) {
            mprog_driver(mprg->comlist, mob, actor, obj, vo, FALSE);
            if (type != GREET_PROG && type != ALL_GREET_PROG)
                break;
        }
    return;

}


void mprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
                       void *vo, int type)
{
    MPROG_DATA * mprg;
    bool       trigger_time;

    for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next)
    {
        trigger_time = ( time_info.hours == atoi( mprg->arglist ) )  || (*(mprg->arglist+strlen(mprg->arglist)-1)=='*');

        if ( !trigger_time )
        {
            if ( mprg->triggered )
                mprg->triggered = FALSE;
            continue;
        }

        if ( ( mprg->type == type )
                && ( ( !mprg->triggered ) || ( mprg->type == HOUR_PROG ) ) )
        {
            mprg->triggered = TRUE;
            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
        }
    }
    return;
}


void mob_act_add( CHAR_DATA *mob )
{
    struct act_prog_data *runner;

    for ( runner = mob_act_list; runner; runner = runner->next )
        if ( runner->vo == mob )
            return;
    CREATE(runner, struct act_prog_data, 1);
    runner->vo = mob;
    runner->next = mob_act_list;
    mob_act_list = runner;
}



/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */                          

void            old_mprog_act_trigger(char *buf, struct char_data * mob, struct char_data * ch,
                                      struct obj_data * obj, void *vo)
{

    MPROG_ACT_LIST *tmp_act;

    if (!FIGHTING(mob) && (mob != ch)) {
        //        tmp_act = malloc(sizeof(MPROG_ACT_LIST));
        CREATE(tmp_act, MPROG_ACT_LIST, 1);
        if (mob->mpactnum > 0)
            tmp_act->next = mob->mpact;
        else
            tmp_act->next = NULL;

        mob->mpact = tmp_act;
        mob->mpact->buf = str_dup(buf);
        mob->mpact->ch = ch;
        mob->mpact->obj = obj;
        mob->mpact->vo = vo;
        mob->mpactnum++;

    }
    return;

}


void mprog_act_trigger( char *buf, CHAR_DATA *mob, CHAR_DATA *ch,
                        OBJ_DATA *obj, void *vo)
{
    MPROG_ACT_LIST * tmp_act;
    MPROG_DATA *mprg;
    bool found = FALSE;

    if ( IS_NPC( mob )
            &&   HAS_MOB_PROG( mob, ACT_PROG ) )
    {
        /* Don't let a mob trigger itself, nor one instance of a mob
          trigger another instance. */
        if ( ch && IS_NPC( ch ) && (GET_MOB_VNUM(ch) == GET_MOB_VNUM(mob)))
            return;

        /* make sure this is a matching trigger */
        for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next)
            if ( mprg->type == ACT_PROG
                    &&   mprog_keyword_check( buf, mprg->arglist ) )
            {
                found = TRUE;
                break;
            }
        if ( !found )
            return;

        CREATE( tmp_act, MPROG_ACT_LIST, 1 );
        if ( mob->mpactnum > 0 )
            tmp_act->next = mob->mpact;
        else
            tmp_act->next = NULL;

        mob->mpact      = tmp_act;
        mob->mpact->buf = str_dup( buf );
        mob->mpact->ch  = ch;
        mob->mpact->obj = obj;
        mob->mpact->vo  = vo;
        mob->mpactnum++;
        mob_act_add( mob );
    }
    return;
}



void            mprog_bribe_trigger(struct char_data * mob, struct char_data * ch, int amount)
{

    MPROG_DATA     *mprg;
    struct obj_data *obj;

    if (HAS_MOB_PROG(mob, BRIBE_PROG) && CAN_SEE(mob, ch) && !FIGHTING(mob)) {
        obj = create_money(amount);
        obj_to_char(obj, mob);
        mob->points.gold -= amount;

        for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next)
            if ((mprg->type & BRIBE_PROG)
                    && (amount >= atoi(mprg->arglist))) {
                mprog_driver(mprg->comlist, mob, ch, obj, NULL, FALSE);
                break;
            }
    }
    return;

}

void            mprog_death_trigger(struct char_data * mob, struct char_data * killer)
{

    if (HAS_MOB_PROG(mob, DEATH_PROG)) {
        GET_POS(mob)=POS_STANDING;
        mprog_percent_check(mob, killer, NULL, NULL, DEATH_PROG);
        GET_POS(mob)=POS_DEAD;
    }
    //if (!AFF2_FLAGGED(mob, AFF2_BEHEAD))
    //death_cry(mob);
    return;

}

void            mprog_entry_trigger(struct char_data * mob)
{

    if (HAS_MOB_PROG(mob, ENTRY_PROG) && !FIGHTING(mob))
        mprog_percent_check(mob, NULL, NULL, NULL, ENTRY_PROG);

    return;

}

void            mprog_fight_trigger(struct char_data * mob, struct char_data * ch)
{

    if (HAS_MOB_PROG(mob, FIGHT_PROG))
        mprog_percent_check(mob, ch, NULL, NULL, FIGHT_PROG);

    return;

}

void            mprog_give_trigger(struct char_data * mob, struct char_data * ch, struct obj_data * obj)
{

    char            buf[MAX_INPUT_LENGTH];
    MPROG_DATA     *mprg;

    if (HAS_MOB_PROG(mob, GIVE_PROG) && !FIGHTING(mob) && CAN_SEE(mob, ch))
        for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next) {
            one_argument(mprg->arglist, buf);
            if ((mprg->type & GIVE_PROG)
                    && ((!str_infix(obj->name, mprg->arglist))
                        || (!str_cmp("all", buf)))) {
                mprog_driver(mprg->comlist, mob, ch, obj, NULL, FALSE);
                break;
            }
        }

    return;

}

void            mprog_greet_trigger(struct char_data * ch)
{

    struct char_data *vmob;
    if (ch->in_room==NOWHERE)
    {
        char bufmgt[200];
        sprintf(bufmgt,"SYSERR: ch->in_room=NOWHERE, greet_trigger, ch: %s",GET_NAME(ch));
        log(bufmgt);
        return;
    }
    for (vmob = world[ch->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
    {
        if (!IS_NPC(vmob)
                || ch == vmob
                || (vmob->char_specials.fighting)
                || !AWAKE(vmob))
            continue;

        if ( IS_NPC( ch ) && (GET_MOB_VNUM(ch) == GET_MOB_VNUM(vmob)))
            continue;

        if ( HAS_MOB_PROG(vmob, GREET_PROG) && CAN_SEE(vmob, ch))
            mprog_percent_check( vmob, ch, NULL, NULL, GREET_PROG );
        else if ( HAS_MOB_PROG(vmob, ALL_GREET_PROG) )
            mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);
    }
    return;

}

void            mprog_hitprcnt_trigger(struct char_data * mob, struct char_data * ch)
{

    MPROG_DATA     *mprg;

    if (HAS_MOB_PROG(mob, HITPRCNT_PROG))
        for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next)
            if ((mprg->type & HITPRCNT_PROG)
                    && ((100 * mob->points.hit / mob->points.max_hit) < atoi(mprg->arglist))) {
                mprog_driver(mprg->comlist, mob, ch, NULL, NULL, FALSE);
                break;
            }
    return;

}

void            mprog_random_trigger(struct char_data * mob)
{
    if (HAS_MOB_PROG(mob, RAND_PROG) && !FIGHTING(mob))
        mprog_percent_check(mob, NULL, NULL, NULL, RAND_PROG);

    return;

}

void            mprog_speech_trigger(char *txt, struct char_data * mob)
{

    struct char_data *vmob;
    if (mob->in_room==NOWHERE)
    {
        char bufmgt[200];
        sprintf(bufmgt,"SYSERR: ch->in_room=NOWHERE, speech_trigger, ch: %s",GET_NAME(mob));
        log(bufmgt);
        return;
    }

    for (vmob = world[mob->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
        if ((mob != vmob) && IS_NPC(vmob)  && !FIGHTING(vmob) && (mob_index[vmob->nr].progtypes & SPEECH_PROG))
        {
            if ( IS_NPC( mob ) && (GET_MOB_VNUM(vmob) == GET_MOB_VNUM(mob)))
                continue;
            mprog_wordlist_check(txt, vmob, mob, NULL, NULL, SPEECH_PROG);
        }

    return;

}

void            mprog_action_trigger(char *txt, struct char_data * mob)
{

    struct char_data *vmob;
    if (mob->in_room==NOWHERE)
    {
        char bufmgt[200];
        sprintf(bufmgt,"SYSERR: ch->in_room=NOWHERE, action_trigger, ch: %s",GET_NAME(mob));
        log(bufmgt);
        return;
    }

    for (vmob = world[mob->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
        if ((mob != vmob) && IS_NPC(vmob)  && !FIGHTING(vmob) && (mob_index[vmob->nr].progtypes & ACTION_PROG))
        {
            if ( IS_NPC( mob ) && (GET_MOB_VNUM(vmob) == GET_MOB_VNUM(mob)))
                continue;
            mprog_wordlist_check(txt, vmob, mob, NULL, NULL, ACTION_PROG);
        }

    return;

}


void mprog_time_trigger( CHAR_DATA *mob )
{
    if ( HAS_MOB_PROG(mob, TIME_PROG) )
        mprog_time_check(mob,NULL,NULL,NULL,TIME_PROG);
}

void mprog_hour_trigger( CHAR_DATA *mob )
{
    if ( HAS_MOB_PROG(mob, HOUR_PROG) )
        mprog_time_check(mob,NULL,NULL,NULL,HOUR_PROG);
}



void mprog_script_trigger( CHAR_DATA *mob )
{
    MPROG_DATA * mprg;

    if ( HAS_MOB_PROG(mob, SCRIPT_PROG) )
        for (mprg = mob_index[mob->nr].mudprogs; mprg != NULL; mprg = mprg->next)
            if ( mprg->type == SCRIPT_PROG
                    &&  (!mprg->arglist || mprg->arglist[0] == '\0'
                         || mob->mpscriptpos != 0
                         || atoi(mprg->arglist) == time_info.hours) )
                mprog_driver( mprg->comlist, mob, NULL, NULL, NULL, TRUE );
}

void oprog_script_trigger( OBJ_DATA *obj )
{
    MPROG_DATA * mprg;

    if ( HAS_OBJ_PROG(obj, SCRIPT_PROG) )
        for ( mprg = obj_index[obj->item_number].mudprogs; mprg; mprg = mprg->next )
            if ( mprg->type == SCRIPT_PROG )
            {
                if (!mprg->arglist || mprg->arglist[0] == '\0'
                        ||   obj->mpscriptpos != 0
                        ||   atoi( mprg->arglist ) == time_info.hours )
                {
                    set_supermob( obj );
                    mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, TRUE );
                    obj->mpscriptpos = supermob->mpscriptpos;
                    release_supermob();
                }
            }
    return;
}

void rprog_script_trigger( ROOM_INDEX_DATA *room )
{
    MPROG_DATA * mprg;

    if ( HAS_ROOM_PROG(room, SCRIPT_PROG) )
        for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
            if ( mprg->type == SCRIPT_PROG )
            {
                if (!mprg->arglist || mprg->arglist[0] == '\0'
                        ||   room->mpscriptpos != 0
                        ||   atoi( mprg->arglist ) == time_info.hours )
                {
                    rset_supermob( room );
                    mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, TRUE );
                    room->mpscriptpos = supermob->mpscriptpos;
                    release_supermob();
                }
            }
    return;
}



bool oprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
                          void *vo, int type)
{
    MPROG_DATA * mprg;
    bool executed = FALSE;

    for ( mprg = obj_index[obj->item_number].mudprogs; mprg; mprg = mprg->next )
        if ( mprg->type == type
                && ( number(1, 100) <= atoi( mprg->arglist ) ) )
        {
            executed = TRUE;
            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE);
            if ( type != GREET_PROG )
                break;
        }

    return executed;
}



void oprog_greet_trigger( CHAR_DATA *ch )
{
    OBJ_DATA *vobj;
    if (ch->in_room<1)
    {
        char bufmgt[200];
        sprintf(bufmgt,"SYSERR: ch->in_room=NOWHERE, greet_trigger, ch: %s",GET_NAME(ch));
        log(bufmgt);
        return;
    }

    for ( vobj=world[ch->in_room].contents; vobj; vobj = vobj->next_content )
        if ( HAS_OBJ_PROG(vobj, GREET_PROG) )
        {
            set_supermob( vobj );  /* not very efficient to do here */
            oprog_percent_check( supermob, ch, vobj, NULL, GREET_PROG );
            release_supermob();
        }
        else if ( HAS_OBJ_PROG(vobj, ALL_GREET_PROG) )
        {
            set_supermob( vobj );  /* not very efficient to do here */
            oprog_percent_check( supermob, ch, vobj, NULL, ALL_GREET_PROG );
            release_supermob();
        }
}



void oprog_speech_trigger( char *txt, CHAR_DATA *ch )
{
    OBJ_DATA *vobj;
    if (ch->in_room<1)
    {
        char bufmgt[200];
        sprintf(bufmgt,"SYSERR: ch->in_room=NOWHERE, speech_trigger, ch: %s",GET_NAME(ch));
        log(bufmgt);
        return;
    }
    /* supermob is set and released in oprog_wordlist_check */
    for ( vobj=world[ch->in_room].contents; vobj; vobj = vobj->next_content )
        if ( HAS_OBJ_PROG(vobj, SPEECH_PROG) )
            oprog_wordlist_check(txt, supermob, ch, vobj, NULL, SPEECH_PROG, vobj);

    return;
}

void oprog_action_trigger( char *txt, CHAR_DATA *ch )
{
    OBJ_DATA *vobj;
    if (ch->in_room<1)
    {
        char bufmgt[200];
        sprintf(bufmgt,"SYSERR: ch->in_room=NOWHERE, action_trigger, ch: %s",GET_NAME(ch));
        log(bufmgt);
        return;
    }
    /* supermob is set and released in oprog_wordlist_check */
    for ( vobj=world[ch->in_room].contents; vobj; vobj = vobj->next_content )
        if ( HAS_OBJ_PROG(vobj, ACTION_PROG) )
            oprog_wordlist_check(txt, supermob, ch, vobj, NULL, ACTION_PROG, vobj);

    return;
}

void oprog_random_trigger( OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, RAND_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check(supermob,NULL,obj,NULL,RAND_PROG);
        release_supermob();
    }
}


void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, WEAR_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, WEAR_PROG );
        release_supermob();
    }
}



bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict,
                        OBJ_DATA *targ, void *vo )
{
    bool executed = FALSE;

    if ( HAS_OBJ_PROG(obj, USE_PROG) )
    {
        set_supermob( obj );
        if ( GET_OBJ_TYPE(obj) == ITEM_STAFF || GET_OBJ_TYPE(obj) == ITEM_WAND
                ||   GET_OBJ_TYPE(obj) == ITEM_SCROLL || GET_OBJ_TYPE(obj) == ITEM_POTION)
        {
            if ( vict )
                executed = oprog_percent_check( supermob, ch, obj, vict, USE_PROG );
            else
                executed = oprog_percent_check( supermob, ch, obj, targ, USE_PROG );
        }
        else
            executed = oprog_percent_check( supermob, ch, obj, NULL, USE_PROG );
        release_supermob();
    }
    return executed;
}







/*
 * call in remove_obj, right after unequip_char   
 * do a if(!ch) return right after, and return TRUE (?)
 * if !ch
 */
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, REMOVE_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, REMOVE_PROG );
        release_supermob();
    }
}


/*
 * call in do_sac, right before extract_obj
 */
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, SAC_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, SAC_PROG );
        release_supermob();
    }
}

/*
 * call in do_get, right before check_for_trap
 * do a if(!ch) return right after
 */
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, GET_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, GET_PROG );
        release_supermob();
    }
}

/*
 * called in damage_obj in act_obj.c
 */
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, DAMAGE_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, DAMAGE_PROG );
        release_supermob();
    }
}

/*
 * called in do_repair in shops.c
 */
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, REPAIR_PROG) )
    {

        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, REPAIR_PROG );
        release_supermob();
    }
}

/*
 * call twice in do_drop, right after the act( AT_ACTION,...)
 * do a if(!ch) return right after
 */
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, DROP_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, DROP_PROG );
        release_supermob();
    }
}

/*
 * call towards end of do_examine, right before check_for_trap
 */
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, EXA_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, EXA_PROG );
        release_supermob();
    }
}


/*
 * call in fight.c, group_gain, after (?) the obj_to_room
 */
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, ZAP_PROG) )
    {
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, ZAP_PROG );
        release_supermob();
    }
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, PULL_PROG) )
    {
        //logs("SYSERR: unsupported trigger pull_prog #%d - %s", GET_OBJ_VNUM(obj), obj->short_description);
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, PULL_PROG );
        release_supermob();
    }
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_OBJ_PROG(obj, PUSH_PROG) )
    {
        //logs("SYSERR: unsupported trigger push_prog #%d - %s", GET_OBJ_VNUM(obj), obj->short_description);
        set_supermob( obj );
        oprog_percent_check( supermob, ch, obj, NULL, PUSH_PROG );
        release_supermob();
    }
}

void obj_act_add( OBJ_DATA *obj );
void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
                        OBJ_DATA *obj, void *vo )
{
    if ( HAS_OBJ_PROG(mobj, ACT_PROG) )
    {
        MPROG_ACT_LIST *tmp_act;

        CREATE(tmp_act, MPROG_ACT_LIST, 1);
        if ( mobj->mpactnum > 0 )
            tmp_act->next = mobj->mpact;
        else
            tmp_act->next = NULL;

        mobj->mpact = tmp_act;
        mobj->mpact->buf = str_dup(buf);
        mobj->mpact->ch = ch;
        mobj->mpact->obj = obj;
        mobj->mpact->vo = vo;
        mobj->mpactnum++;
        obj_act_add(mobj);
    }
}

void oprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
                           OBJ_DATA *obj, void *vo, int type, OBJ_DATA *iobj )
{
    char        temp1[ MAX_STRING_LENGTH ];
    char        temp2[ MAX_INPUT_LENGTH ];
    char        word[ MAX_INPUT_LENGTH ];
    MPROG_DATA *mprg;
    char       *list;
    char       *start;
    char       *dupl;
    char       *end;
    int         i;
    char tpom[1000];
    char *pom, *sep;

    for ( mprg = obj_index[iobj->item_number].mudprogs; mprg; mprg = mprg->next )
        if ( mprg->type == type )
        {
            strcpy( temp1, mprg->arglist );
            list = temp1;
            for ( i = 0; i < strlen( list ); i++ )
                list[i] = LOWER( list[i] );
            strcpy( temp2, arg );
            dupl = temp2;
            for ( i = 0; i < strlen( dupl ); i++ )
                dupl[i] = LOWER( dupl[i] );
            if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
            {
                list += 2;
                while ( ( start = strstr( dupl, list ) ) )
                    if ( (start == dupl || *(start-1) == ' ' )
                            && ( *(end = start + strlen( list ) ) == ' '
                                 || *end == '\n'
                                 || *end == '\r'
                                 || *end == '\0' ) )
                    {
                        set_supermob( iobj );
                        mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                        release_supermob() ;
                        break;
                    }
                    else
                        dupl = start+1;
            }

            else if ( ( list[0] == 'q' || list[0] == 'r' ) && ( list[1] == ' ' ) )
            {
                list += 2;

                sep=strchr(list, '|');
                pom=one_argument(dupl, tpom);
                if (!sep)
                {

                    dupl=one_argument(list, word);
                    if (is_abbrev(tpom, word))
                        if (!(*pom) || isname(pom, dupl))
                        {
                            global_action=(*(list-2)=='q'?1:2);
                            set_supermob( iobj );
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            release_supermob() ;
                            break;
                        }
                }
                else
                {
                    *sep=0;

                    if (!(*list) || isname(tpom, list))
                        if (!(*(sep+1)) || isname(pom, sep+1))
                        {
                            global_action=(*(list-2)=='q'?1:2);
                            set_supermob( iobj );
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            release_supermob() ;
                            break;
                        }

                }
            }
            else
            {
                list = one_argument( list, word );
                for( ; word[0] != '\0'; list = one_argument( list, word ) )
                    while ( ( start = strstr( dupl, word ) ) )
                        if ( ( start == dupl || *(start-1) == ' ' )
                                && ( *(end = start + strlen( word ) ) == ' '
                                     || *end == '\n'
                                     || *end == '\r'
                                     || *end == '\0' ) )
                        {
                            set_supermob( iobj );
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            release_supermob();
                            break;
                        }
                        else
                            dupl = start+1;
            }
        }

    return;
}




void rprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
                          void *vo, int type)
{
    MPROG_DATA * mprg;

    if(!mob->in_room)
        return;

    for ( mprg = world[mob->in_room].mudprogs; mprg; mprg = mprg->next )
        if ( mprg->type == type
                &&   number(1, 100) <= atoi(mprg->arglist) )
        {
            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
            if( type!=ENTER_PROG )
                break;
        }
}

/*
 * Triggers follow
 */


/*
 *  Hold on this
 * Unhold. -- Alty
 */
void room_act_add( ROOM_INDEX_DATA *room );
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
                        OBJ_DATA *obj, void *vo )
{
    if ( HAS_ROOM_PROG(room, ACT_PROG) )
    {
        MPROG_ACT_LIST *tmp_act;

        CREATE(tmp_act, MPROG_ACT_LIST, 1);
        if ( room->mpactnum > 0 )
            tmp_act->next = room->mpact;
        else
            tmp_act->next = NULL;

        room->mpact = tmp_act;
        room->mpact->buf = str_dup(buf);
        room->mpact->ch = ch;
        room->mpact->obj = obj;
        room->mpact->vo = vo;
        room->mpactnum++;
        room_act_add(room);
    }
}
/*
 *
 */


void rprog_leave_trigger( CHAR_DATA *ch )
{
    if ( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], LEAVE_PROG) )
    {
        rset_supermob( &world[ch->in_room] );
        rprog_percent_check( supermob, ch, NULL, NULL, LEAVE_PROG );
        release_supermob();
    }
}

void rprog_enter_trigger( CHAR_DATA *ch )
{
    if ( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], ENTER_PROG) )
    {
        rset_supermob( &world[ch->in_room] );
        rprog_percent_check( supermob, ch, NULL, NULL, ENTER_PROG );
        release_supermob();
    }
}

void rprog_sleep_trigger( CHAR_DATA *ch )
{
    if ( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], SLEEP_PROG) )
    {
        rset_supermob( &world[ch->in_room] );
        rprog_percent_check( supermob, ch, NULL, NULL, SLEEP_PROG );
        release_supermob();
    }
}

void rprog_rest_trigger( CHAR_DATA *ch )
{
    if( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], REST_PROG) )
    {
        rset_supermob( &world[ch->in_room] );
        rprog_percent_check( supermob, ch, NULL, NULL, REST_PROG );
        release_supermob();
    }
}

void rprog_rfight_trigger( CHAR_DATA *ch )
{
    if( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], RFIGHT_PROG) )
    {
        rset_supermob( &world[ch->in_room] );
        rprog_percent_check( supermob, ch, NULL, NULL, RFIGHT_PROG );
        release_supermob();
    }
}

void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch )
{
    if( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], RDEATH_PROG) )
    {
        rset_supermob(&world[ch->in_room]);
        rprog_percent_check( supermob, ch, NULL, NULL, RDEATH_PROG );
        release_supermob();
    }
}

void rprog_speech_trigger( char *txt, CHAR_DATA *ch )
{
    if( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], SPEECH_PROG) )
    {
        /* supermob is set and released in rprog_wordlist_check */
        rprog_wordlist_check( txt, supermob, ch, NULL, NULL, SPEECH_PROG, &world[ch->in_room] );
    }
}

void rprog_action_trigger( char *txt, CHAR_DATA *ch )
{
    if( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], ACTION_PROG) )
    {
        /* supermob is set and released in rprog_wordlist_check */
        rprog_wordlist_check( txt, supermob, ch, NULL, NULL, ACTION_PROG, &world[ch->in_room] );
    }
}

void rprog_random_trigger( CHAR_DATA *ch )
{
    if (!DEAD(ch) &&  HAS_ROOM_PROG(&world[ch->in_room], RAND_PROG) )
    {
        rset_supermob(&world[ch->in_room] );
        rprog_percent_check(supermob,ch,NULL,NULL,RAND_PROG);
        release_supermob();
    }
}


void rprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
                           OBJ_DATA *obj, void *vo, int type, ROOM_INDEX_DATA *room )
{

    char        temp1[ MAX_STRING_LENGTH ];
    char        temp2[ MAX_INPUT_LENGTH ];
    char        word[ MAX_INPUT_LENGTH ];
    MPROG_DATA *mprg;
    char       *list;
    char       *start;
    char       *dupl;
    char       *end;
    int         i;
    char tpom[1000];
    char *pom;
    char *sep;

    if ( actor && actor->in_room && !DEAD(actor))
        room = &world[actor->in_room];

    for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
        if ( mprg->type == type )
        {
            strcpy( temp1, mprg->arglist );
            list = temp1;
            for ( i = 0; i < strlen( list ); i++ )
                list[i] = LOWER( list[i] );
            strcpy( temp2, arg );
            dupl = temp2;
            for ( i = 0; i < strlen( dupl ); i++ )
                dupl[i] = LOWER( dupl[i] );
            if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
            {
                list += 2;
                while ( ( start = strstr( dupl, list ) ) )
                    if ( (start == dupl || *(start-1) == ' ' )
                            && ( *(end = start + strlen( list ) ) == ' '
                                 || *end == '\n'
                                 || *end == '\r'
                                 || *end == '\0' ) )
                    {
                        rset_supermob( room );
                        mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                        release_supermob() ;
                        break;
                    }
                    else
                        dupl = start+1;
            }
            else if ( ( list[0] == 'q' || list[0] == 'r' ) && ( list[1] == ' ' ) )
            {
                list += 2;

                sep=strchr(list, '|');
                pom=one_argument(dupl, tpom);
                if (!sep)
                {

                    dupl=one_argument(list, word);
                    if (is_abbrev(tpom, word))
                        if (!(*pom) || isname(pom, dupl))
                        {
                            global_action=(*(list-2)=='q'?1:2);
                            rset_supermob( room );
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            release_supermob() ;
                            break;
                        }
                }
                else
                {
                    *sep=0;

                    if (!(*list) || isname(tpom, list))
                        if (!(*(sep+1)) || isname(pom, sep+1))
                        {
                            global_action=(*(list-2)=='q'?1:2);
                            rset_supermob( room );
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            release_supermob() ;
                            break;
                        }

                }
            }
            else
            {
                list = one_argument( list, word );
                for( ; word[0] != '\0'; list = one_argument( list, word ) )
                    while ( ( start = strstr( dupl, word ) ) )
                        if ( ( start == dupl || *(start-1) == ' ' )
                                && ( *(end = start + strlen( word ) ) == ' '
                                     || *end == '\n'
                                     || *end == '\r'
                                     || *end == '\0' ) )
                        {
                            rset_supermob( room );
                            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
                            release_supermob();
                            break;
                        }
                        else
                            dupl = start+1;
            }
        }
    return;
}

void rprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
                       void *vo, int type )
{
    ROOM_INDEX_DATA * room = (ROOM_INDEX_DATA *) vo;
    MPROG_DATA * mprg;
    bool 	       trigger_time;

    for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
    {
        trigger_time = ( time_info.hours == atoi( mprg->arglist ) )  || (*(mprg->arglist+strlen(mprg->arglist)-1)=='*');

        if ( !trigger_time )
        {
            if ( mprg->triggered )
                mprg->triggered = FALSE;
            continue;
        }

        if ( mprg->type == type
                && ( ( !mprg->triggered ) || ( mprg->type == HOUR_PROG ) ) )
        {
            mprg->triggered = TRUE;
            mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
        }
    }
    return;
}

void rprog_room_random_trigger( room_num nr )
{
    //if (HAS_ROOM_PROG(&world[nr], RAND_PROG) )
    {
        rset_supermob(&world[nr] );
        rprog_percent_check(supermob,NULL,NULL,NULL,RANDIW_PROG);
        release_supermob();
    }
}
void rprog_room_time_trigger( room_num nr)
{
    // if (HAS_ROOM_PROG(&world[nr], TIME_PROG))
    {
        rset_supermob(&world[nr] );
        rprog_time_check( supermob, NULL, NULL,&world[nr], TIME_PROG );
        release_supermob();
    }
}

void rprog_room_hour_trigger( room_num nr)
{
    // if (HAS_ROOM_PROG(&world[nr], HOUR_PROG))
    {
        rset_supermob(&world[nr] );
        rprog_time_check( supermob, NULL, NULL, &world[nr], HOUR_PROG );
        release_supermob();
    }
}



void rprog_time_trigger( CHAR_DATA *ch )
{
    if (!DEAD(ch) &&  HAS_ROOM_PROG(&world[ch->in_room], TIME_PROG))
    {
        rset_supermob( &world[ch->in_room] );
        rprog_time_check( supermob, NULL, NULL,&world[ch->in_room], TIME_PROG );
        release_supermob();
    }
}

void rprog_hour_trigger( CHAR_DATA *ch )
{
    if ( !DEAD(ch) && HAS_ROOM_PROG(&world[ch->in_room], HOUR_PROG))
    {
        rset_supermob( &world[ch->in_room]);
        rprog_time_check( supermob, NULL, NULL, &world[ch->in_room], HOUR_PROG );
        release_supermob();
    }
}



/* Room act prog updates.  Use a separate list cuz we dont really wanna go
   thru 5-10000 rooms every pulse.. can we say lag? -- Alty */

void room_act_add( ROOM_INDEX_DATA *room )
{
    struct act_prog_data *runner;

    for ( runner = room_act_list; runner; runner = runner->next )
        if ( runner->vo == room )
            return;
    CREATE(runner, struct act_prog_data, 1);
    runner->vo = room;
    runner->next = room_act_list;
    room_act_list = runner;
}


void room_act_update( void )
{
    struct act_prog_data *runner;
    MPROG_ACT_LIST *mpact;

    while ( (runner = room_act_list) != NULL )
    {
        ROOM_INDEX_DATA *room = runner->vo;

        while ( (mpact = room->mpact) != NULL )
        {
            if ( &world[mpact->ch->in_room] == room )
                rprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
                                     mpact->vo, ACT_PROG, room);
            room->mpact = mpact->next;
            DISPOSE(mpact->buf);
            DISPOSE(mpact);
        }
        room->mpact = NULL;
        room->mpactnum = 0;
        room_act_list = runner->next;
        DISPOSE(runner);
    }
    return;
}

void obj_act_add( OBJ_DATA *obj )
{
    struct act_prog_data *runner;

    for ( runner = obj_act_list; runner; runner = runner->next )
        if ( runner->vo == obj )
            return;
    CREATE(runner, struct act_prog_data, 1);
    runner->vo = obj;
    runner->next = obj_act_list;
    obj_act_list = runner;
}

void obj_act_update( void )
{
    struct act_prog_data *runner;
    MPROG_ACT_LIST *mpact;

    while ( (runner = obj_act_list) != NULL )
    {
        OBJ_DATA *obj = runner->vo;

        while ( (mpact = obj->mpact) != NULL )
        {
            oprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
                                 mpact->vo, ACT_PROG, obj);
            obj->mpact = mpact->next;
            DISPOSE(mpact->buf);
            DISPOSE(mpact);
        }
        obj->mpact = NULL;
        obj->mpactnum = 0;
        obj_act_list = runner->next;
        DISPOSE(runner);
    }
    return;
}





void mob_act_update()
{
    CHAR_DATA *wch;
    struct act_prog_data *apdtmp;

    while ( (apdtmp = mob_act_list) != NULL )
    {
        wch = mob_act_list->vo;
        if ( wch->mpactnum > 0 )
        {
            MPROG_ACT_LIST * tmp_act;

            while ( (tmp_act = wch->mpact) != NULL )
            {
                if ( tmp_act->obj && PURGED(tmp_act->obj) )
                    tmp_act->obj = NULL;
                if ( tmp_act->ch && !DEAD(tmp_act->ch) )
                    mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
                                          tmp_act->obj, tmp_act->vo, ACT_PROG );
                wch->mpact = tmp_act->next;
                DISPOSE(tmp_act->buf);
                DISPOSE(tmp_act);
            }
            wch->mpactnum = 0;
            wch->mpact    = NULL;
        }
        mob_act_list = apdtmp->next;
        DISPOSE( apdtmp );
    }
}
