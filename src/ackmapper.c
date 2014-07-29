/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Zepp 1998    *
 *    _/_/_/_/      _/          _/  _/             Version #: 4.3          *
 *   _/      _/      _/_/_/     _/    _/     _/                            *
 *                                                                         *
 *                        http://ackmud.nuc.net/                           *
 *                        zenithar@ackmud.nuc.net                          *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/* This code inspired by a snippet from :                                  */

/************************************************************************/
/* mlkesl@stthomas.edu  =====>  Ascii Automapper utility                */
/* Let me know if you use this. Give a newbie some _credit_,            */
/* at least I'm not asking how to add classes...                        */
/* Also, if you fix something could ya send me mail about, thanks       */
/* PLEASE mail me if you use this or like it, that way I will keep it up*/
/************************************************************************/

/*
 * Ported to DOTDII MUD (http://www.dotd.com) by Garil 6-15-99
 */

#include <ctype.h>  /* for isalpha */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "ackmapper.h"


#define safe_strcat(len, dest, src) strncat((dest), (src), (len)-strlen(dest))

int door_marks[10][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1},{0,0},{0,0},{-1, 1},{-1, -1},{1, 1},{1, -1} };
int offsets2[10][2] ={ {-2, 0},{ 0, 2},{ 2, 0},{ 0,-2},{0,0},{0,0},{-2, 2},{-2, -2},{2, 2},{2, -2} };

#define SECT_MAX       13
#define SECT_HERE       SECT_MAX
#define SECT_UNSEEN     ( SECT_MAX + 1 )
#define SECT_BLOCKED    ( SECT_UNSEEN + 1 )
#define SECT_TOP        ( SECT_BLOCKED + 1 )              

const struct map_info_type door_info[] =
{
    { DOOR_LOCKED, "&r" , "#&0", "","Locked door" },
    { DOOR_OPEN, "&W" , "#&0", "","Open door" },
    { DOOR_CLOSED, "&r" , "#&0", "","Closed Door" },
    { DOOR_NS, "&w" , "|&0", "","N/S Exit" },
    { DOOR_EW, "&w" , "-&0", "","E/W Exit" },
    { DOOR_NESW, "&w" , "/&0", "","NE/SW Exit" },
    { DOOR_NWSE, "&w" , "\\&0", "","NW/SE Exit" },
    { DOOR_NULL, "&w", " &0", "","Invalid" }
};

const struct map_info_type map_info[] =
{
    { SECT_BLOCKED,      "&R",   "!&0", "",   "blocked" },
    { SECT_UNSEEN,       "&w", " &0", "",     "unknown" },
    { SECT_HERE,         "&Y",   "@&0", "",   "you!" },
    
    { SECT_INSIDE,       "&W", "+&0", "",   "inside" },
    { SECT_CITY,         "&W", ".&0", "",   "city" },
    { SECT_FIELD,        "&C", ".&0", "",   "field" },
    { SECT_FOREST,       "&G", "F&0", "&W", "forest" },
    { SECT_HILLS,        "&r", "^&0", "&W", "hills" },
    { SECT_MOUNTAIN,     "&R", "^&0", "",   "mountain" },
    { SECT_WATER_SWIM,   "&b", "~&0", "",   "shallow water" },
    { SECT_WATER_NOSWIM, "&B", "~&0", "",   "deep running water" },
    { SECT_UNDERWATER,   "&B", "~&0", "",   "underwater" },
    { SECT_FLYING,       "&w", ":&0", "",   "air" },
    { SECT_QUICKSAND,    "&y", "=&0", "",   "desert" },
    { SECT_LAVA,         "&Y", "=&0", "",   "lava" },
    { SECT_ARCTIC,       "&W", "=&0", "",     "arctic" },
    
    { SECT_TOP,          "&Y",   "~&0", "",   "bad sector type" }
};

char * get_sector_display( int sector )
{
    sh_int looper;
    for ( looper = 0; ; looper++ )
        if (  ( map_info[looper].sector_type == sector )
              || ( map_info[looper].sector_type == SECT_TOP )   )
            break;
    return ( map_info[looper].display_code );
}
char * get_sector_color( int sector )
{
    sh_int looper;
    for ( looper = 0; ; looper++ )
        if (  ( map_info[looper].sector_type == sector )
              || ( map_info[looper].sector_type == SECT_TOP )   )
            break;
    return ( map_info[looper].display_color );
}
char * get_invert_color( int sector )
{
    sh_int looper;
    for ( looper = 0; ; looper++ )
        if (  ( map_info[looper].sector_type == sector )
              || ( map_info[looper].sector_type == SECT_TOP )   )
            break;
    return ( map_info[looper].invert_color );
}
char * get_door_display( int door )
{
    sh_int looper;
    for ( looper = 0; ; looper++ )
        if (  ( door_info[looper].sector_type == door )
              || ( door_info[looper].sector_type == DOOR_NULL )   )
            break;
    return ( door_info[looper].display_code );
}
char * get_door_color( int door )
{
    sh_int looper;
    for ( looper = 0; ; looper++ )
        if (  ( door_info[looper].sector_type == door )
              || ( door_info[looper].sector_type == DOOR_NULL )   )
            break;
    return ( door_info[looper].display_color );
}
char * get_sector_name( int sector )
{
    sh_int looper;
    for ( looper = 0; ; looper++ )
        if (  ( map_info[looper].sector_type == sector )
              || ( map_info[looper].sector_type == SECT_TOP )   )
            break;
    return ( map_info[looper].desc );
}


/*
 * This code written by Altrag for Ack!Mud
 */

#define iseol(c)        ((c)=='\n' || (c)=='\r')

/* Like one_argument but saves case, and if len isnt NULL, fills it in
 * with the length.  Also accounts for color.  Does NOT account for
 * quoted text. */
char *break_arg(char *str, char *first_arg, int bufsize, int max, int *buflen,
int *len)
{
    int slen=0;
    char *arg;

    while (isspace(*str))
        ++str;
    if (*str=='\\' && str[1]=='b' && str[2]=='r')
    {
        strcpy(first_arg, "\n\r");
        if (buflen)
            *buflen=0;
        if (len)
            *len=0;
        str+=3;
        while (isspace(*str))
            ++str;
        return str;
    }
    arg=first_arg;
    while (*str && arg-first_arg<bufsize && slen<max)
    {
        if (isspace(*str))
        {
            ++str;
            break;
        }
        //else if (*str=='@' && str[1]=='@' && str[2]!='\0')
        else if (*str=='&' &&  str[1]!='\0')
        {
            if (arg-first_arg>=max-2)
                break;
            *arg++=*str++;
            *arg++=*str++;
            //*arg++=*str++;
        }
        else if (*str=='\\' && str[1]=='b' && str[2]=='r')
            break;
        else
        {
            *arg++=*str++;
            ++slen;
        }
    }
    *arg='\0';
    if (len)
        *len=slen;
    if (buflen)
        *buflen=arg-first_arg;
    while (isspace(*str))
        ++str;
    return str;
}

char *string_justify(char *str, int len, int width, int numwords, int *rlen)
{
    static char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int minspaces = numwords-1;
    int spaces_needed;
    float space_ratio, space_between;
    int i, j = 0, alen;
    char *bp = buf;

    spaces_needed = minspaces+(width-(len+1));
    if (spaces_needed<=minspaces || minspaces<=0)
    {
        sprintf(buf, "%s\n\r", str);
        return buf;
    }
    space_ratio = (float)spaces_needed/(float)minspaces;
    space_between = space_ratio;
    for (i = 0; i < minspaces; ++i)
    {
        str = break_arg(str, arg, sizeof(arg), width, &alen, NULL);
        strcpy(bp, arg);
        bp += alen;
        for (; j < (int)(space_between+0.5); ++j)
            *bp++ = ' ';
        space_between += space_ratio;
    }
    str = break_arg(str, arg, sizeof(arg), width, &alen, NULL);
    strcpy(bp, arg);
    bp += alen;
    /*  bp += sprintf(bp, "\n\r%d:%d:%d", len, width, numwords);
     bp += sprintf(bp, "\n\r%d:%d:%f", minspaces, spaces_needed, space_ratio);*/

    *bp++ = '\n';
    *bp++ = '\r';
    *bp = '\0';
    if (rlen)
        *rlen = bp-buf;
    return buf;
}

char last_color(char *str)
{
    char *end;

    for (end=str+strlen(str)-2; end > str; --end)
        //if (*end == '@' && end[-1]=='@')
        if (*end == '&')
            return end[1];
    return '\0';
}

char *string_format(char *str, int *numlines, int width, int height, bool unjust)
{
    static char ret[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int alen;
    int blen=0;
    char *pbuf=buf, *pret=ret;
    int len;
    int currline = 0;
    int last;
    char *sp;
    char c;
    int numwords = 0;
    int jlen;

    --height;
    --width;
    *pret='\0';
    for (sp = break_arg(str, arg, sizeof(arg), width, &len, &alen); *arg;
         sp = break_arg(sp, arg, sizeof(arg), width, &len, &alen))
    {
        blen += alen;
        if (blen+1>=width || iseol(*arg))
        {
            *pbuf++='\n';
            *pbuf++='\r';
            *pbuf='\0';
            c = last_color(buf);
            if (unjust || iseol(*arg))
            {
                strcpy(pret, buf);
                pret += pbuf-buf;
            }
            else
            {
                strcpy(pret, string_justify(buf, blen-alen, width, numwords, &jlen));
                pret += jlen;
            }
            pbuf=buf;
            if (++currline>height)
                break;
            if (c)
            {
                //*pbuf++='@';
                //*pbuf++='@';
                *pbuf++='&';
                *pbuf++=c;
            }
            blen=alen;
            if (iseol(*arg))
                *arg='\0';
            numwords = 0;
        }
        else if (pbuf>buf)
        {
            //if (pbuf-buf>2 && pbuf[-2]=='@' && pbuf[-3]=='@')
            if (pbuf-buf>2 && pbuf[-2]=='@' && pbuf[-3]=='@')
            {
                if (pbuf-buf==3)
                    last=0;
                else
                    last=-4;
            }
            else
                last=-1;
            if (last)
            {
                if (unjust && pbuf[last]=='.')
                    *pbuf++=' ', ++blen;
                    //        if (!iseol(pbuf[last]))
                    *pbuf++=' ', ++blen;
            }
        }
        strcpy(pbuf, arg);
        pbuf+=len;
        ++numwords;
    }
    if (pbuf>buf)
    {
        if (pbuf-buf>2 && pbuf[-2]=='@' && pbuf[-3]=='@')
        {
            if (pbuf-buf==3)
                last=0;
            else
                last=-4;
        }
        else
            last=-1;
        if (last && pbuf[last]!='\n' && pbuf[last]!='\r')
        {
            *pbuf++='\n';
            *pbuf++='\r';
            ++currline;
        }
    }
    *pbuf='\0';
    strcpy(pret, buf);
    if (numlines)
        *numlines=currline;
    return ret;
}

char *map_format(char *str, int start, char map[MAP_Y][MAX_STRING_LENGTH], int
*numlines,
                 int term_width, int height, bool unjust)
{
    static char ret[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int width = (start<MAP_Y ? term_width-15 : term_width-1);
    int alen;
    int blen=0;
    char *pbuf=buf, *pret=ret;
    int len;
    int currline = start;
    int last;
    char *sp;
    char c;
    int numwords = 0;
    int jlen;

    --height;
    *pret='\0';
    for (sp = break_arg(str, arg, sizeof(arg), width, &len, &alen); *arg;
         sp = break_arg(sp, arg, sizeof(arg), width, &len, &alen))
    {
        blen += alen;
        if (blen+1>=width || iseol(*arg))
        {
            *pbuf++='\n';
            *pbuf++='\r';
            *pbuf='\0';
            c = last_color(buf);
            if (currline<MAP_Y)
                pret += sprintf(pret, "%s   ", map[currline]);
            else if (currline==MAP_Y)
                strcpy(pret, "              "), pret+=14;
                if (unjust || iseol(*arg))
                {
                    strcpy(pret, buf);
                    pret += pbuf-buf;
                }
                else
                {
                    strcpy(pret, string_justify(buf, blen-alen, width, numwords, &jlen));
                    pret += jlen;
                }
                if (currline==MAP_Y)
                    width=term_width-1;
                pbuf=buf;
                if (++currline>height)
                    break;
                if (c)
                {
                    //*pbuf++='@';
                    //*pbuf++='@';
                    *pbuf++='&';
                    *pbuf++=c;
                }
                blen=alen;
                if (iseol(*arg))
                    *arg='\0';
                numwords = 0;
        }
        else if (pbuf>buf)
        {
            if (pbuf-buf>2 && pbuf[-2]=='@' && pbuf[-3]=='@')
            {
                if (pbuf-buf==3)
                    last=0;
                else
                    last=-4;
            }
            else
                last=-1;
            if (last)
            {
                if (unjust && pbuf[last]=='.')
                    *pbuf++=' ', ++blen;
                    //        if (!iseol(pbuf[last]))
                    *pbuf++=' ', ++blen;
            }
        }
        strcpy(pbuf, arg);
        pbuf+=len;
        ++numwords;
    }
    /*  sprintf(bug_buf, "%d:%d", width, blen);
     monitor_chan(bug_buf, MONITOR_DEBUG);*/
    if (pbuf>buf)
    {
        if (pbuf-buf>2 && pbuf[-2]=='@' && pbuf[-3]=='@')
        {
            if (pbuf-buf==3)
                last=0;
            else
                last=-4;
        }
        else
            last=-1;
        if (last && pbuf[last]!='\n' && pbuf[last]!='\r')
        {
            *pbuf++='\n';
            *pbuf++='\r';
            if (currline<MAP_Y)
                pret += sprintf(pret, "%s   ", map[currline]);
            else if (currline==MAP_Y || (currline==MAP_Y+1 && blen<=term_width-15))
                strcpy(pret, "              "), pret+=14;
                ++currline;
        }
    }
    *pbuf='\0';
    strcpy(pret, buf);
    if (numlines)
        *numlines=currline;
    return ret;
}

char *exit_string( CHAR_DATA * ch, ROOM_INDEX_DATA *r)
{
    EXIT_DATA *e;
    int door;
    static char buf[128];

    strcpy(buf, "[");
     for (door = 0; door < NUM_OF_DIRS; door++) {
        
        if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE) 
            {                                            
            	if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
                    continue;
                if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
                	sprintf(buf+strlen(buf), " (%s)", dirs[door]);
                else
                	sprintf(buf+strlen(buf), " %s", dirs[door]);
            }
     }
    strcat(buf, " ]");
    return buf;
}

void disp_map(char *border, char *map, CHAR_DATA *ch)
{
    int cols=80;
    int rows=9999;
    char bufs[MAP_Y][MAX_STRING_LENGTH];
    char disp[MAX_STRING_LENGTH];
    int y, ty=MAP_Y-1;
    char *x, *ox;

    strcpy(bufs[0], border);
    strcpy(bufs[ty], border);
    x = map;
    for (y = 1; y < ty && *x; ++y)
    {
        while (*x=='\n' || *x=='\r')
            ++x;
        ox = x;
        while (*x!='\n' && *x!='\r' && *x!='\0')
            ++x;
        sprintf(bufs[y], "%.*s", (x-ox), ox);
    }
    strcpy(disp, map_format(world[ch->in_room].name, 0, bufs, &y, cols, rows, TRUE));
    strcat(disp, map_format(exit_string(ch, &world[ch->in_room]), y, bufs, &y, cols, rows, TRUE));
    strcat(disp, map_format(world[ch->in_room].description, y, bufs, &y, cols, rows,TRUE));
    
    if (y<MAP_Y)
    {
        x = disp+strlen(disp);
        while (y<MAP_Y)
            x += sprintf(x, "%s\n\r", bufs[y]), ++y;
    }
    send_to_char(disp, ch);
    return;
}


void MapArea2(struct room_data *room, CHAR_DATA *ch,
             int x, int y, int min, int max, int line_of_sight, int test_los )
{
    CHAR_DATA * victim;
    int looper;
    int door;                  
    struct room_direction_data *pexit;
    struct room_data prospect_room;
    sh_int door_type = 0;

    if ( map[x][y] < 0 )
        return; /* it's a door, not a room in the map */

    /* marks the room as visited */
    map[x][y]=room->sector_type;

    /* displays seen mobs nearby as numbers */
    for ( looper = 0, victim = room->people; victim; victim = victim->next_in_room )
        if ( CAN_SEE( ch, victim ))// && !is_same_group( ch, victim ) )
            looper++;
    if ( looper > 0 )
        sprintf( contents[x][y].string, "%i", MIN( looper, 9 ) );
    else
        contents[x][y].string[0] = '\0';

    for ( door = 0; door < MAX_MAP_DIR; door++ )
    {
        if (door==UP || door==DOWN)
            continue;
        pexit=room->dir_option[door];
        if (!(pexit  && (pexit->to_room>0) && !IS_SET(pexit->exit_info, EX_HIDDEN)))
        continue;
    
        if (x<min || y<min || x>max || y>max)
            return;
        
          prospect_room = world[pexit->to_room];

        if ( prospect_room.dir_option[rev_dir[door]] &&
             prospect_room.dir_option[rev_dir[door]]->to_room_vnum!=room->number)               
        {
            map[x][y]=SECT_BLOCKED; // one way into area OR maze 
            return;
        }

        /* selects door symbol */
        door_type = 0;
        if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
        {
            if ( door == DIR_NORTH || door== DIR_SOUTH )
                door_type = DOOR_NS;
            else if ( door == DIR_EAST || door== DIR_WEST )
                door_type = DOOR_EW;  
            else if ( door == DIR_NORTHEAST || door== DIR_SOUTHWEST )
                door_type = DOOR_NESW;      
            else if ( door == DIR_NORTHWEST || door== DIR_SOUTHEAST )
                door_type = DOOR_NWSE;      
            
        }
        else if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
        {
            door_type = DOOR_LOCKED;
        }
        else if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            door_type = DOOR_CLOSED;
        }
        else
            door_type = DOOR_OPEN;

        if ( !IS_NPC( ch ) &&
             //!str_cmp( pexit->keyword, "" ) &&
             ( door_type <= DOOR_OPEN ||
               !IS_SET(pexit->exit_info, EX_ISDOOR) ||
               ( IS_SET( pexit->exit_info, EX_CLOSED ) &&
                 !IS_SET( pexit->exit_info, EX_HIDDEN )
                  )  )  )
        {
            map[x+door_marks[door][0]][y+door_marks[door][1]]=door_type;
            if (  door_type < DOOR_CLOSED &&
                  (!test_los || ( line_of_sight == LOS_INITIAL ||
                    door == line_of_sight) ) &&
                    map[x+offsets2[door][0]][y+offsets2[door][1]]==SECT_UNSEEN  )
            {
                MapArea2(&world[pexit->to_room], ch,
                        x+offsets2[door][0],
                        y+offsets2[door][1],
                        min, max,
                        line_of_sight==LOS_INITIAL?door:line_of_sight, test_los );
            }
        }
    }
    return;
}

void ShowRoom( CHAR_DATA *ch, int min, int max, int size, int center)
{
    void disp_map(char *border, char *map, CHAR_DATA *ch);
    int x,y, looper;
    char outbuf[MAX_STRING_LENGTH];
    char catbuf[MAX_STRING_LENGTH];
    char borderbuf[MAX_STRING_LENGTH];
    char colorbuf[MAX_STRING_LENGTH];
    char displaybuf[MAX_STRING_LENGTH];
    outbuf[0] = '\0';
    sprintf( outbuf, "%s", "\n\r" );
    
    sprintf( borderbuf, "%s", "+" );
    for ( looper = 0; looper <= size+1; looper++ )
    {
        sprintf( catbuf, "%s", "-" );
        safe_strcat( MAX_STRING_LENGTH, borderbuf, catbuf );
    }
    safe_strcat( MAX_STRING_LENGTH, borderbuf, "+" );
    for (x = min; x <= max; ++x)
    { /* every row */

        safe_strcat( MAX_STRING_LENGTH, outbuf, "| " );
        for (y = min; y <= max; ++y)
        { /* every column */
            if ( (y==min) || (map[x][y-1]!=map[x][y]) )
            {
                sprintf( colorbuf, "%s%s",
                         ( map[x][y]< 0 ?
                           get_door_color( map[x][y] ) :
                           get_sector_color( map[x][y] ) ),
                         ( ( contents[x][y].string[0] == '\0' ) ?
                           "" :
                           get_invert_color( map[x][y] ) )  );
                sprintf( displaybuf, "%s",
                         ( map[x][y]< 0 ?
                           get_door_display( map[x][y] ) :
                           ( contents[x][y].string[0] == '\0' ?
                             get_sector_display(map[x][y] ) :
                             contents[x][y].string )  )  );
                sprintf( catbuf, "%s%s",
                         colorbuf, displaybuf );
                safe_strcat( MAX_STRING_LENGTH, outbuf, catbuf  );

            }
            else
            {
                sprintf( catbuf, "%s",
                         (map[x][y]<0) ?
                         get_door_display( map[x][y] ) :
                         get_sector_display(map[x][y]) );
                safe_strcat( MAX_STRING_LENGTH, outbuf, catbuf  );
            }

        }
        safe_strcat( MAX_STRING_LENGTH, outbuf, " |\n\r" );
    }

    disp_map(borderbuf, outbuf, ch);
    //disp_map("", outbuf, ch);

    /* NOTE: with your imp, probably put the three together into one string? */

    return;
}


    char outbuf[MAX_STRING_LENGTH];
    char catbuf[MAX_STRING_LENGTH];
    char borderbuf[MAX_STRING_LENGTH];
    char colorbuf[MAX_STRING_LENGTH];
    char displaybuf[MAX_STRING_LENGTH];
    char mybuf[MAX_STRING_LENGTH];
    
/* mlk :: shows a map, specified by size */
void ShowMap2( CHAR_DATA *ch, int min, int max, int size, int center )
{
    int x, y, looper;
   

    strcpy(outbuf, "\r\n");
    strcpy(mybuf, "\r\n");
    
    //sprintf( borderbuf, "%s", "+" );

    /*for ( looper = 0; looper <= size+1; looper++ )
    {
        sprintf( catbuf, "%s", "-" );
        safe_strcat( MAX_STRING_LENGTH, borderbuf, catbuf );
    } */
    //safe_strcat( MAX_STRING_LENGTH, borderbuf, "+" );
    for (x = min; x <= max; ++x)
    {
        //safe_strcat( MAX_STRING_LENGTH, outbuf, "    " );
        strcpy(mybuf, "    ");
        for (y = min; y <= max; ++y)
        {
            if ( (y==min) || (map[x][y-1]!=map[x][y]) )
            {
                sprintf( colorbuf, "%s%s",
                         ( map[x][y] < 0 ?
                           get_door_color( map[x][y] ) :
                           get_sector_color( map[x][y] ) ),
                         ( contents[x][y].string[0] == '\0' ?
                           "" :
                           get_invert_color( map[x][y] ) )  );
                sprintf( displaybuf, "%s",
                         ( map[x][y]< 0 ?
                           get_door_display( map[x][y] ) :
                           ( contents[x][y].string[0] == '\0' ?
                             get_sector_display(map[x][y] ) :
                             contents[x][y].string )  )  );
                sprintf( catbuf, "%s%s&0", colorbuf, displaybuf );
                //safe_strcat( MAX_STRING_LENGTH, outbuf, catbuf );
                safe_strcat( MAX_STRING_LENGTH, mybuf, catbuf );
            }
            else
            {
                sprintf( catbuf, "%s&0",
                         (map[x][y]<0) ?
                         get_door_display( map[x][y] ) :
                         get_sector_display(map[x][y]) );
                //safe_strcat( MAX_STRING_LENGTH, outbuf, catbuf  );
                safe_strcat( MAX_STRING_LENGTH, mybuf, catbuf  );
            }

        }
        if (strlen(mybuf)>51)        
        {          
        	safe_strcat( MAX_STRING_LENGTH, outbuf, mybuf );
        	safe_strcat( MAX_STRING_LENGTH, outbuf, " \r\n" );
        }
    }
    //send_to_char( "\r\n", ch );
    /* this is the top line of the map itself, currently not part of the mapstring */
    //send_to_char( borderbuf, ch );
    /* this is the contents of the map */
    
    send_to_char( outbuf, ch );
    /* this is the bottom line of the map */
    //send_to_char( borderbuf, ch );
    //send_to_char( "\n\r", ch );
    //send_to_char( "Also try 'newmap legend' and 'newmap 7'.\n\r", ch );
    return;
}


ACMD(do_newmap)
{
    int size,center,x,y,min,max,looper;     
    int test_los=1;
    char arg1[MAX_STRING_LENGTH];
    char catbuf[MAX_STRING_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    one_argument( argument, arg1 );
    if ( isname( arg1, "legend key help" ) )
    {
        sprintf( outbuf, "Map Legend:\n\r" );
        for ( looper = 0; looper < SECT_TOP; looper++ )
        {
            sprintf( catbuf, "%s%s&0 : %s\n\r" ,
                     map_info[looper].display_color,
                     map_info[looper].display_code,
                     map_info[looper].desc );
            safe_strcat( MAX_STRING_LENGTH, outbuf, catbuf );
        }
        for ( looper = 0; looper < 7; looper++ )
        {
            sprintf( catbuf, "%s%s&0 : %s\n\r" ,
                     door_info[looper].display_color,
                     door_info[looper].display_code,
                     door_info[looper].desc );
            safe_strcat( MAX_STRING_LENGTH, outbuf, catbuf );
        }
        send_to_char( outbuf, ch );
        return;
    }
    size = ( is_number(  arg1 ) ) ?
        atoi (arg1) :
        31;

    if ( size != 7 )
    {
        size = IS_IMMORT( ch ) ? size :
            9;
        if ( size % 2  == 0 )
            size +=1;
        size = URANGE( 9, size, MAX_MAP );
    }
    size=URANGE(3,size,MAX_MAP);

    center=MAX_MAP/2;

    min = MAX_MAP/2-size/2;
    max = MAX_MAP/2+size/2;

    for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
        {
            map[x][y]=SECT_UNSEEN;
            contents[x][y].string[0] = '\0';
        }

    /* starts the mapping with the center room */
    MapArea2(&world[ch->in_room], ch, center, center, min-1, max, LOS_INITIAL, test_los);

    /* marks the center, where ch is */
    sprintf( contents[center][center].string, "%s", "&Y@" );
    if ( size == 7 )
        ShowRoom( ch, min, max, size, center );
    else
        ShowMap2( ch, min, max, size, center );
    return;

}

