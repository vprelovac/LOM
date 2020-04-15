#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#define FALSE 0
#define TRUE (!FALSE)
#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))

#define NOCONV   0
#define MAXVALS  1
#define MIDVALS  2
#define MERCVALS 3

#define MAXSPLLVL 50
#define MAXCHARGE 15
#define MAX_OBJ_AFFS 6

#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)

int zonenum,nznr = -1,oznr,mobconv = NOCONV,maxznr;
int maxroom,warn = TRUE,maxrand = -1,randspells = FALSE,cifre=FALSE;
char zonename[256];
FILE *erf;

#define ROOM_RECALL (1 << 23)
#define ITEM_PILL   26 
#define AFF_FLY     (1 << 22)

#define SPELLMAX 83
int spells[]={
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
201,
-1,
-1,
54,
-1,
18,
91,
-1,
63,
-1,
-1,
-1,
113,
70,
67,
69,
59,
-1,
-1,
-1,
-1,
68,
-1,
-1,
50,
-1,
-1,
124,
114,
-1,
77
};



/* Spell Defines */
#define SPELL_PASSDOOR 	68
#define SPELL_MASSINVIS 59
#define SPELL_CHANGESEX 0
#define SPELL_FLY	54
#define SPELL_CONTLIGHT	0
#define SPELL_KNOWALIGN	18
#define SPELL_CURE_SER	63
#define SPELL_CAU_LIGHT	31
#define SPELL_CAU_CRIT	37
#define SPELL_CAU_SER	5
#define SPELL_FLAMESTR	113
#define SPELL_STONESKIN	70
#define SPELL_SHIELD	67
#define SPELL_WEAKEN	69
#define SPELL_ACIDBLAST	0
#define SPELL_FAERIEFRE	0
#define SPELL_FAERIEFOG	0
#define SPELL_INFRAVIS	50
#define SPELL_CREATESPR	124
#define SPELL_REFRESH	114
#define SPELL_GATE	78

const int mobexp[51] = {
  0,
  100,
  200,
  350,
  600,
  900,
  1500,
  2250,
  3750,
  6000,
  9000,
  11000,
  13000,
  16000,
  18000,
  21000,
  24000,
  28000,
  30000,
  35000,
  40000,
  50000,
  60000,
  80000,
  100000,
  130000,
  155000,
  200000,
  310000,
  450000,
  600000,
  700000,
  800000,
  900000,
  1000000,
  1100000,
  1200000,
  1300000,
  1400000,
  1500000,
  1600000,
  1700000,
  1800000,
  1900000,
  2000000,
  2100000,
  2200000,
  2300000,
  2400000,
  2500000,
  2600000
};

/* ***************************************************************************
 * The following functions is taken from Merc.                               *
 * Merc is created by Kahn, Hatchet and Furey                                *
 ************************************************************************** */

static	int	rgiState[2+55];

void init_mm( )
{
    int *piState;
    int iState;
    time_t current_time;
    struct timeval now_time;
    
    gettimeofday( &now_time, NULL);
    current_time = (time_t) now_time.tv_sec;

    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;

    piState[0]	= ((int) current_time) & ((1 << 30) - 1);
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = (piState[iState-1] + piState[iState-2])
			& ((1 << 30) - 1);
    }
    return;
}



int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}

int interpolate( int level, int value_00, int value_47 )
{
    return value_00 + level * (value_47 - value_00) / 47;
}

int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}

int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}

int number_range( int from, int to )
{
    int power;
    int number;

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm( ) & (power - 1) ) >= to )
	;

    return from + number;
}

int fread_number( FILE *fp )
{
    int number;
    int sign;
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	printf( "Fread_number: bad format.");
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/* ***************************************************************************
 * The follwing functions is taken from CircleMud                            *
 * CircleMud is created by Jeremy Elson                                      *
 ************************************************************************** */
 
char *str_dup(const char *source)
{
  char *new;

  CREATE(new, char, strlen(source) + 1);
  return (strcpy(new, source));
}

int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

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

char *fread_string(FILE * fl, char *error)
{
  char buf[32768], tmp[512], *rslt;
  register char *point;
  int done = 0, length = 0, templength = 0;

  *buf = '\0';

  do {
    if (!fgets(tmp, 512, fl)) {
      fprintf(erf, "SYSERR: fread_string: format error at or near %s\n",
	      error);
      exit(1);
    }
    /* If there is a '~', end the string; else put an "\n" over the '\n'. */
    if ((point = strchr(tmp, '~')) != NULL) {
      *point = '\0';
      done = 1;
    } else {
      point = tmp + strlen(tmp) - 1;
      *(point++) = '\r';
      *(point++) = '\n';
      *point = '\0';
    }

    templength = strlen(tmp);

    if (length + templength >= 32768) {
      fprintf(erf,"SYSERR: fread_string: string too large (db.c)");
      exit(1);
    } else {
      strcat(buf + length, tmp);
      length += templength;
    }
  } while (!done);

  /* allocate space for the new string and copy it */
  if (strlen(buf) > 0) {
    CREATE(rslt, char, length + 1);
    strcpy(rslt, buf);
  } else
    rslt = NULL;

  return rslt;
}

/* ***************************************************************************
 * The follwing is my own creation, feel free to make changes.               *
 ************************************************************************** */

int get_exp (int level)
{
  int diff,exp = 0;

  if (level > 50) {
    diff = level - 50;
    exp = (diff * 100000) + mobexp[50];
  } else
   exp = mobexp[level];

  return (exp); 
}


int change_number (int old)
{
  int i;

  if (nznr >= 0) {// && old >= oznr * 100 && old <= maxznr) {
    i = ((int) old / 100) - oznr;
    old = old % 100;
    old += (nznr+i) * 100;
  }

  return (old);
}
 
void calc_mob_vals(int num, int level, int *thac0, int *ac, int *h1, int *h2,int *h3,
                   int *d1, int *d2, int *d3, int *gold, int *exp)
{
  char buf[500];
  int hit = 0,dam = 0;

  init_mm();
  switch (mobconv) {
    case MERCVALS:
      *thac0 = interpolate (level,20,0);
      *ac = interpolate (level,10,-10);
      *exp = get_exp(level); 
      *gold = number_fuzzy(10) * number_fuzzy(level) * number_fuzzy(level);
      dam = number_range(level/2,(level*3)/2);
      hit = (level * 8) + number_range(level*level/4,level*level);
      break;
    case MAXVALS:
      *thac0 = interpolate (level,20,0);
      *ac = interpolate (level,100,-100);
      *exp = get_exp(level); 
      *gold = (10+1) * (level+1) * (level+1);
      dam = (level+3)/2;
      hit = (level * 8) + (level*level);
      break;
    case MIDVALS:
      *thac0 = interpolate (level,20,0);
      *ac = interpolate (level,100,-100);
      *gold = 10 * level * level;
      *exp = get_exp(level); 
      dam = ((level/2)+((level+3)/2)) / 2;
      hit = (level * 8) + (((level*level/4) + (level*level)) / 2);
      break;
   default:
    if (*thac0==0 && *ac==0)
    {
     *thac0 = interpolate (level,20,-5);   
     *ac = interpolate (level,10,-15);   
    }
 	break;     
  }
  if (dam > 0) {
    *d3 = dam / 3;
    *d1 = 1;
    *d2 = dam - *d3;
  }
  if (hit > 0) {
    *h3 = hit / 2;
    *h1 = 1;
    *h2 = hit - *h3;
  }
 if (level>50 || level<1) fprintf(erf,"W A R N I N G: Mob %d has level set to %d\n",num,level);
// if (((*h1)*(*h2)+(*h3))==0) fprintf(erf,"Warning: Mob %d has 0 hitpoints\n",num);
 if (((*h1)*((*h2+1)/2)+(*h3))>1000) fprintf(erf,"Warning: Mob %d has %d (%d) hitpoints\n",num,((*h1)*((*h2+1)/2)+(*h3)),((*h1)*(*h2)+(*h3)));
// if (((*d1)*(*d2)+(*d3))==0) fprintf(erf,"Warning: Mob %d does no damage\n",num); 
 if (((*d1)*((*d2+1)/2)+(*d3))>40) fprintf(erf,"Warning: Mob %d does %d (%d) damage\n",num,(*d1)*((*d2+1)/2)+(*d3),(*d1)*(*d2)+(*d3)); 
/* if (*gold==0) fprintf(erf,"Warning: Mob %d has no gold\n",num); 
 if (*exp==0) fprintf(erf,"Warning: Mob %d has no exp\n",num);  
*/ 
}

char *get_maffs (int i)
{
  switch (i) {
    case 6:
      return ("HOLD");
      break;
    case 8:
      return ("FAERIE FIRE");
      break;
    case 11:
      return ("FLAMING");
      break;
    case 13:
      return ("PARALYSIS");
      break;
    case 19:
      return ("FLYING");
      break;
    case 20:
      return ("PASS DOOR");
      break;
  }
  
  return ("ERROR BUG THIS SHOULD NOT BE HERE.");
}

char *get_mflag (int i)
{
  switch (i) {
    case 0:
      return ("IS NPC");
      break;
    case 8:
      return ("PET");
      break;
    case 9:
      return ("TRAIN PC's");
      break;
    case 10:
      return ("PRAC PC's");
      break;
  }
  
  return ("ERROR BUG THIS SHOULD NOT BE HERE.");
}

char *get_oflag (int i)
{
  switch (i) {
    case 2:
      return ("DARK");
      break;
    case 3:
      return ("LOCK");
      break;
    case 4:
      return ("EVIL");
      break;
    case 12:
      return ("!REMOVE");
      break;
    case 13:
      return ("INVENTORY");
      break;
  }
  
  return ("ERROR BUG THIS SHOULD NOT BE HERE.");
}

int check_spell (int splnr, int objnr)
{
  int nsplnr = 0, found = FALSE;

  if (splnr <= 44) {
    if (splnr==34 && splnr==41)
    {   
        if (warn)
            fprintf(erf,"SPELLS: Vent or Prot Evil (%d) in obj %d. Set to UNKNOWN\n",-1,objnr);
        nsplnr=-1;
    }
    else
        nsplnr = splnr;
    found = TRUE;
  } else
  if (splnr<=SPELLMAX)
  {
    nsplnr=spells[splnr-45]; 
    found=TRUE;
  }

  if (!found) {
    if (warn)
      fprintf(erf,"SPELLS: Unsupported SPELL (%d) found in obj %d. Set to -1.\n", splnr, objnr);
    nsplnr = -1;
  }

  return (nsplnr);
}

int check_mobaffs (int mflg, int mobnr)
{
  int i,nmflg = 0;

  for (i = 0; i <= 31;i++) {
    switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 7:
      case 13:
        if (IS_SET(mflg,(1 << i)))
          SET_BIT(nmflg,(1 << i));
        break;
      case 6:
      case 8:
      case 11:
      case 20:
        if (IS_SET(mflg,(1 << i))) {
          if (warn)
            fprintf(erf,"Unsupported Mob Affection - %s at mob %d. Removing\n",get_maffs(i),mobnr);
        }
        break;
      case 9:
        if (IS_SET(mflg, (1 << 9))) {
          SET_BIT(nmflg,(1 << 10));
        }
        break;
      case 10:
        if (IS_SET(mflg, (1 << 10))) {
          SET_BIT(nmflg,(1 << 9));
        }
        break;
      case 12:
        if (IS_SET(mflg, (1 << 12))) {
          SET_BIT(nmflg,(1 << 11));
        }
        break;
      case 15:
        if (IS_SET(mflg, (1 << 15))) {
          SET_BIT(nmflg,(1 << 18));
        }
        break;
      case 16:
        if (IS_SET(mflg, (1 << 16))) {
          SET_BIT(nmflg,(1 << 19));
        }
        break;
      case 17:
        if (IS_SET(mflg, (1 << 17))) {
          SET_BIT(nmflg,(1 << 14));
        }
        break;
      case 18:
        if (IS_SET(mflg, (1 << 18))) {
          SET_BIT(nmflg,(1 << 21));
        }
        break;
      case 19:
        if (IS_SET(mflg, (1 << 19))) 
            SET_BIT(nmflg,AFF_FLY);
        break;
      default:
        if (IS_SET(mflg,(1 << i))) {
          if (warn)
            fprintf(erf,"Unknown MOB AFF in mob %d. It has been removed.\n",mobnr);
        }
        break;
    }
  }

  return (nmflg);
}

int check_mobflags (int mflg, int mobnr)
{
  int i,nmflg = 0;

  for (i = 0; i <= 31;i++) {
    switch (i) {
      case 1:
      case 2:
      case 5:
      case 6:
      case 7:
        if (IS_SET(mflg,(1 << i)))
          SET_BIT(nmflg,(1 << i));
        break;
      case 0:
      case 8:
      case 9:
      case 10:
        if (IS_SET(mflg,(1 << i))) {
          if (warn)
            fprintf(erf,"Unsupported Mob Flag - %s at mob %d. Removing\n",get_mflag(i),mobnr);
        }
        break;
      default:
        if (IS_SET(mflg,(1 << i))) {
          if (warn)
            fprintf(erf,"Unknown MOB FLAG in mob %d. It has been removed.\n",mobnr);
        }
        break;
    }
  }

  return (nmflg);
}

int check_objflags (int oflg, int objnr)
{
  int i,noflg = 0;

  for (i = 0; i <= 31;i++) {
    switch (i) {
      case 0:
      case 1:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
        if (IS_SET(oflg,(1 << i)))
          SET_BIT(noflg,(1 << i));
        break;
      case 2:
      case 3:
      case 4:
      case 12:
        if (IS_SET(oflg,(1 << i))) {
          if (warn)
            fprintf(erf,"Unsupported Obj Flag - %s at obj %d. Removing\n",get_oflag(i),objnr);
        }
        break;
      case 13:
        if (IS_SET(oflg,(1 << i))) {
          if (warn)
            fprintf(erf,"Obj %d has NOREMOVE flag set\n",objnr);
             SET_BIT(noflg,(1 << i));
        }
        break;
      default:
    }
  }

  return (noflg);
}

int check_objtypes (int tpe,int objnr)
{
  if (tpe >= 23) {
    if (tpe == 25) {
      tpe = 23;
    } else if (tpe == 26) {
        tpe = 10;
        if (warn)
         fprintf(erf,"ITEM PILL found at obj %d. Setting to Potion.\n",objnr);
    } else if (tpe == 23 || tpe == 24) {
      if (warn)
        fprintf(erf,"ITEM CORPSE found at obj %d. Setting to container.\n",objnr);
      tpe = 15;
    } else {
      if (warn)
        fprintf(erf,"Unknown ITEM Type found at obj %d. Setting to Other.\n",objnr);
      tpe = 12;
    }
  }
  return (tpe);
}

int check_roomflags (int rflg)
{
  int i,nrflg = 0;

  for (i = 0; i <= 31; i++) {
    switch (i) {
      case 0:
      case 2:
      case 3:
      case 9:
        if (IS_SET(rflg,(1 << i)))
          SET_BIT(nrflg,(1 << i));
        break;
      case 10:
        if (IS_SET(rflg,(1 << 10))) {
          SET_BIT (nrflg,(1 << 4));
        }
        break;
      case 11:
        if (IS_SET(rflg,(1 << 11))) {
          SET_BIT (nrflg,(1 << 8));
        }
        break;
      case 12:
        if (IS_SET(rflg,(1 << 12))) {
          if (warn)
            fprintf(erf,"PET SHOP flag on room %d. Add to spec_assign.c.\n",maxroom);
        }
        break;
      case 14:
        if (IS_SET(rflg,(1 << 14))) {
          if (warn) 
            fprintf(erf,"Nonstandard MERC ROOM FLAG at room %d. silence/bank who knows?\n",maxroom);
        }
        break;
      default:
        if (IS_SET(rflg,(1 << i))) {
          if (warn)
            fprintf(erf,"Unknown ROOM FLAG in room %d. It has been removed.\n",maxroom);
        }
        break;
    }
  }

  return (nrflg);
}

int check_sectors (int stpe)
{

  if (stpe > 9) {
    if (stpe == 10) {
      if (warn)
        fprintf(erf,"Sectortype DESERT at room %d set to QUICKSAND.\n",maxroom);
      stpe = 10;
    } else if (stpe == 11) {
      if (warn)
        fprintf(erf,"Nonstandard MERC Sector at room %d. Setting to Field.\n",maxroom);
      stpe = 2;
    } else {
      if (warn)
        fprintf(erf,"Unknown sectortype at room %d. Setting to Field.\n",maxroom);
      stpe = 2;
    }
  }

  return (stpe);
}

char *get_ascii(int val)
{
  char temp[32],*tmp;

  strcpy(temp,"\0");

  if (IS_SET(val, 1 << 0))
    strcat (temp, "a");
  if (IS_SET(val, 1 << 1))
    strcat (temp, "b");
  if (IS_SET(val, 1 << 2))
    strcat (temp, "c");
  if (IS_SET(val, 1 << 3))
    strcat (temp, "d");
  if (IS_SET(val, 1 << 4))
    strcat (temp, "e");
  if (IS_SET(val, 1 << 5))
    strcat (temp, "f");
  if (IS_SET(val, 1 << 6))
    strcat (temp, "g");
  if (IS_SET(val, 1 << 7))
    strcat (temp, "h");
  if (IS_SET(val, 1 << 8))
    strcat (temp, "i");
  if (IS_SET(val, 1 << 9))
    strcat (temp, "j");
  if (IS_SET(val, 1 << 10))
    strcat (temp, "k");
  if (IS_SET(val, 1 << 11))
    strcat (temp, "l");
  if (IS_SET(val, 1 << 12))
    strcat (temp, "m");
  if (IS_SET(val, 1 << 13))
    strcat (temp, "n");
  if (IS_SET(val, 1 << 14))
    strcat (temp, "o");
  if (IS_SET(val, 1 << 15))
    strcat (temp, "p");
  if (IS_SET(val, 1 << 16))
    strcat (temp, "q");
  if (IS_SET(val, 1 << 17))
    strcat (temp, "r");
  if (IS_SET(val, 1 << 18))
    strcat (temp, "s");
  if (IS_SET(val, 1 << 19))
    strcat (temp, "t");
  if (IS_SET(val, 1 << 20))
    strcat (temp, "u");
  if (IS_SET(val, 1 << 21))
    strcat (temp, "v");
  if (IS_SET(val, 1 << 22))
    strcat (temp, "w");
  if (IS_SET(val, 1 << 23))
    strcat (temp, "x");
  if (IS_SET(val, 1 << 24))
    strcat (temp, "y");
  if (IS_SET(val, 1 << 25))
    strcat (temp, "z");
  if (IS_SET(val, 1 << 26))
    strcat (temp, "A");
  if (IS_SET(val, 1 << 27))
    strcat (temp, "B");
  if (IS_SET(val, 1 << 28))
    strcat (temp, "C");
  if (IS_SET(val, 1 << 29))
    strcat (temp, "D");
  if (IS_SET(val, 1 << 30))
    strcat (temp, "E");
  if (IS_SET(val, 1 << 31))
    strcat (temp, "F");

  if (strlen(temp) < 1)
    strcat(temp ,"0");

  tmp = str_dup(temp);
  return (tmp);
}

void remove13(char *name)
{
  int i;
  FILE *fl;
  FILE *ofl;
  char buf[32768];
  char ch;

    i = 1;
    strcpy(buf,name);
    strcat(buf,".new");
    if ((ofl = fopen(buf,"w")) == NULL) {
      fprintf(erf,"An error occured at %s\n",buf);
      exit(1);
    }
    if ((fl = fopen(name,"r")) == NULL) {
      fprintf(erf,"An error occured at %s\n",name);
      exit(1);
    }
    ch = getc(fl);
    while (ch != EOF) {
      if (ch != '\r')
        putc(ch,ofl);
      ch = getc(fl);
    }
    fclose(fl);   
    fclose(ofl);
    remove (name);
    rename (buf,name);
}

void parse_shops (FILE *fl)
{
  char fname[10],fname2[10];
  char ch[32768],cs[32768];
  FILE *fp, *fs;
  int mob,t1,t2,t3,t4,t5,sell,buy,op,clo,objs[30],i = 0,j;
  int temp1,temp2,temp3,room;
  int tmp1,tmp2,tmp3,shpnr = 0;
  float fsell,fbuy;

  sprintf(fname,"%d.shp",zonenum);
  fp = fopen (fname,"w");
  fprintf(fp,"CircleMUD v3.0 Shop File~\n");

  get_line(fl,ch);
  while (strcmp(ch,"0")) {
    shpnr++;
    if (sscanf(ch," %d %d %d %d %d %d %d %d %d %d ",&mob,&t1,&t2,&t3,&t4,&t5,&sell,&buy,&op,&clo) != 10) {
      fprintf(erf,"Error in shop\n");
      exit (1);
    }
    mob=change_number(mob);
    sprintf(fname2,"%d.zon",zonenum);
    fs = fopen (fname2,"r");
    while (!feof(fs)) {
      get_line(fs,cs);
      if (sscanf(cs,"M %d %d %d %d ",&temp1,&temp2,&temp3,&room) == 4) {
        if (temp2 == mob)
          break; 
      } 
    }
    if (temp2 == mob) {
      i = 0;
      while (!feof(fs)) {
        get_line(fs,cs);
        if (*cs == 'M' || *cs == 'S')
          break;
        if (sscanf(cs,"G %d %d %d ",&tmp1,&tmp2,&tmp3) == 3) {
          objs[i] = tmp2;
          i++;
        }
      }
    }
    fclose(fs);
    fprintf(fp,"#%d0%d~\n",zonenum,shpnr);
    if (i > 0) {
      for (j = 0; j < i;j++)
        fprintf(fp,"%d\n",objs[j]);
    }
    fprintf(fp,"-1\n");
    fsell = sell / 100.00;
    fbuy = buy /100.00;
    fprintf(fp,"%1.2f\n",fsell);
    fprintf(fp,"%1.2f\n",fbuy);
    if (t1 > 0)
      fprintf(fp,"%d\n",t1);
    if (t2 > 0)
      fprintf(fp,"%d\n",t2);
    if (t3 > 0)
      fprintf(fp,"%d\n",t3);
    if (t4 > 0)
      fprintf(fp,"%d\n",t4);
    if (t5 > 0)
      fprintf(fp,"%d\n",t5);
    fprintf(fp,"-1\n");
    fprintf(fp,"%%s Haven't got that on storage - try list!~\n");
    fprintf(fp,"%%s You don't seem to have that.~\n");
    fprintf(fp,"%%s I don't buy THAT... Try another shop.~\n");
    fprintf(fp,"%%s I can't afford such a thing.~\n");
    fprintf(fp,"%%s You can't afford it!~\n");
    fprintf(fp,"%%s That'll be %%d coins, please.~\n");
    fprintf(fp,"%%s You get %%d coins for it!~\n");
    fprintf(fp,"0\n");
    fprintf(fp,"0\n");
    fprintf(fp,"%d\n",mob);
    fprintf(fp,"0\n");
    fprintf(fp,"%d\n",room);
    fprintf(fp,"-1\n");
    fprintf(fp,"%d\n",op);
    fprintf(fp,"%d\n",clo);
    fprintf(fp,"0\n");
    fprintf(fp,"0\n");
    get_line(fl,ch);
  }
  fprintf(fp,"$~\n");
  fclose (fp);
  remove13(fname);
}

void parse_helps (FILE *fl) {
  char fname[10];
  char ch[65536];
  char *help;
  FILE *fp;

  sprintf(fname,"%d.hlp",zonenum);
  fp = fopen (fname,"w");

  get_line (fl,ch);
  fprintf(fp,"%s\n",ch);

  help = fread_string (fl,ch);
  fprintf(fp,"%s\n",help);
  free (help);

  fclose (fp);

  remove13(fname);
}

  
void parse_specials (FILE *fl) {
  char fname[10];
  char ch[32768],tpe,spec[256];
  FILE *fp;
  int nr;

  sprintf(fname,"%d.spe",zonenum);
  fp = fopen (fname,"w");

  for (;;) {
    get_line (fl,ch);
    if (*ch != 'S') {
      if (sscanf(ch," %c %d %s ",&tpe,&nr,spec) != 3) {
        fprintf(erf,"Error in #SPECIALS section.\n");
        exit (1);
      } else {
        switch (tpe) {
          case 'M':
            fprintf(fp,"  ASSIGNMOB (%d, %s);\n",change_number(nr),spec);
            break;
          case '*':
            break;
        }
      }
    } else {
     fclose (fp);
     remove13(fname);
     return;
    }
  }
}
  
void parse_resets (FILE *fl) {
  char fname[10];
  char ch[32768],c,s[256],*i;
  int t1,t2,t3,t4;
  FILE *fp;

  maxroom += 99 - (maxroom % 100);

  sprintf(fname,"%d.zon",zonenum);
  fp = fopen (fname,"w");

  fprintf(fp,"#%d\n",zonenum);
  fprintf(fp,"%s\n",zonename);
  fprintf(fp,"%d 35 2\n",maxroom);

  for (;;) {
    get_line (fl,ch);
    if (*ch != 'S') {
      if (*ch != 'C' && *ch != 'R' && *ch != 'F') {
 {
          if (sscanf(ch,"%c %d %d %d %d ",&c,&t1,&t2,&t3,&t4) != 5) {
            if (sscanf(ch,"%c %d %d %d ",&c,&t1,&t2,&t3) == 4) {
              sprintf(s,"%d",t3);
              i = strstr(ch,s);
              while (*i ==  ' ' || isdigit(*i))
                i++;
              t2 = change_number(t2);
              if (c == 'M' || c == 'D' || c == 'O')
                t1 = 0;
              else
                t1 = 1;
              fprintf(fp,"%c %d %d %d %s\n",c,t1,t2,t3,i);
            } else {
              fprintf(fp,"%s\n",ch);
            }
          } else {
            sprintf(s,"%d",t4);
            i = strstr(ch,s);
            while (*i ==  ' ' || isdigit(*i))
              i++;
            t2 = change_number(t2);
            if (c!='E' && c!='D')
                t4 = change_number(t4);
            if (c == 'M' || c == 'D' || c == 'O')
              t1 = 0;
            else
              t1 = 1;
            fprintf(fp,"%c %d %d %d %d %s\n",c,t1,t2,t3,t4,i);
          }
        }
      } else {
        if (warn)
          fprintf(erf,"Unsupported Reset command : %s\n",ch);
      }
    } else {
     fprintf(fp,"S\n");
     fprintf(fp,"$~\n");
     fclose (fp);
     remove13(fname);
     return;
    }
  }
}

  
void parse_objects (FILE *fl) {
  char fname[10];
  char ch[32768], s10[200], s20[200], s30[200],  s40[200],s[256];
char *point,*s1,*s2,*s3,*s4;
  int t1,t2,t3,t4;
  int num, found = -1,numaffs = 0;
  int flgs,wears,tpe,totdam;
  FILE *fp;

  sprintf(fname,"%d.obj",zonenum);
  fp = fopen (fname,"w");
  get_line(fl,ch);
  for (;;) {
    if (!strcmp(ch,"#0"))
      break;
    if (sscanf(ch,"#%d ",&num) == 1) {
      numaffs = 0;
      if (nznr >= 0)
        num = change_number (num);
      fprintf(fp,"#%d\n",num);
      s1 = fread_string (fl,s);
      s2 = fread_string (fl,s);
      s3 = fread_string (fl,s);
      s4 = fread_string (fl,s);
      fprintf(fp,"%s~\n%s~\n%s~\n",s1,s2,s3);
      if (s4 == NULL)
        fprintf(fp,"~\n");
      else
        fprintf(fp,"%s~\n",s4);
      tpe = fread_number (fl);
      flgs = fread_number (fl);
      wears = fread_number (fl);
      tpe = check_objtypes(tpe,num);
      flgs = check_objflags (flgs,num);
      if (!cifre) fprintf(fp,"%d %s %s\n",tpe,get_ascii(flgs),get_ascii(wears));
       else  fprintf(fp,"%d %d %d\n",tpe,(flgs),(wears));
      get_line (fl,ch);
      if (sscanf (ch,"%s %s %s %s",s10,s20,s30,s40) != 4) {
        fprintf(erf,"Error at obj %d 2nd numeric line: %s\n",num,ch);
        exit (1);
      }
      if ((point = strchr(s10, '~')) != NULL)  *point = '\0';              
      t1=atoi(s10);
      if ((point = strchr(s20, '~')) != NULL)  *point = '\0';                    
      t2=atoi(s20);
      if ((point = strchr(s30, '~')) != NULL)  *point = '\0';              
      t3=atoi(s30);
      if ((point = strchr(s40, '~')) != NULL)  *point = '\0';
      t4=atoi(s40);
      if (tpe == 15) {
        if (nznr >= 0) {
          t3 = change_number(t3);
        }
      }
      if (tpe == 23) {
        if (t1 < 1 || t2 < 1) {
          t1 = 20000;
          t2 = t2;
        }
      }
      if (tpe == 9 && t1 == 0)
        t1 = number_range(1,11);
      if (tpe == 5) {                     /* Weapons */
        if (t4 < 1) {
          if (warn)
            fprintf(erf,"Weapon object %d is type hit. Setting to Bludgeon\n",num);
          t4 = 5;
        }
        if (t2*t3 > 50 && warn)
          fprintf(erf,"Weapon object %d might give excessive damage.\n",num);
        if (t2 < 1 || t3 < 1) {
          if (warn && maxrand < 1)
            fprintf(erf,"Weapon object %d will give no damage.\n",num);
          if (maxrand > 1) {
            totdam = number_range (2,maxrand);
            t2 = number_range(3,7);
            do {
              t2--;
              t3 = totdam / t2;
            } while (t2*t3 < totdam-1);
          }
        }
      }
      if (tpe == 2 || tpe == 10 || tpe == ITEM_PILL) {  /* Scroll and Potion */
        t2 = check_spell (t2,num);
        t3 = check_spell (t3,num);
        t4 = check_spell (t4,num);
//        if (t2 < 1 && t3 < 1 && t4 < 1 && warn)
//          fprintf(erf,"Potion/Scroll object %d has needs spell assigned.\n",num);
      } else if (tpe == 3 || tpe == 4) {  /* Wands and Staves */
        if (t1 < 1 && randspells)
          t1=number_range (1,MAXSPLLVL);
        if (t2 < 1 && randspells) {
          t2 = number_range(1,MAXCHARGE);
          t3 = t2;
        }
        if ((t1 < 1 || t2 < 1 || t3 < 1) && !randspells && warn)
          fprintf(erf,"Some values in staff/wand object %d is 0.\n",num);
        t4 = check_spell (t4,num);
  //      if (t4 < 1 && warn)
    //      fprintf(erf,"Staff/Wand object %d needs spell assigned.\n",num);
      }
      fprintf (fp,"%d %d %d %d\n",t1,t2,t3,t4);
      get_line (fl,ch);
      if (sscanf (ch," %d %d %d ",&t1,&t2,&t3) != 3) {
        fprintf(erf,"Error at obj %d 3rd numeric line.\n",num);
        exit (1);
      }
      fprintf (fp,"%d %d %d\n",t1,t2,t3);
      found = -1;
      while (found == -1) {
        get_line (fl,ch);
        switch (*ch) {
          case '#':
            found = 0;
            break;
          case 'A':
            numaffs++;
            if (sscanf(ch,"A %d %d ",&t1,&t2) != 2) {
              get_line (fl,ch);
              if (sscanf(ch," %d %d ",&t1,&t2) != 2) {
                fprintf(erf,"Error at obj %d in Affections.\n",num);
                exit (1);
              }
            }
            if (numaffs <= MAX_OBJ_AFFS) {
if (t1==18 && t2>10) fprintf(erf,"Object %d has hitroll of %d\n",num,t2);
if (t1==19 && t2>10) fprintf(erf,"Object %d has damroll of %d\n",num,t2);
if (t1==7 || t1==8 || t1==15 || t1==16) fprintf(erf,"Object %d has touched forbidden (%d)\n",num,t2);
else{              fprintf(fp,"A\n");
              fprintf(fp,"%d %d\n",t1,t2);}
            } else {
             if (warn)
               fprintf(erf,"Object %d has to many AFFECTIONS!!!.\n",num);
            }
            break;
          case 'E':
            fprintf(fp,"E\n");
            s1 = fread_string (fl,s);
            s2 = fread_string (fl,s);
            fprintf(fp,"%s~\n",s1);
            fprintf(fp,"%s~\n",s2);
            break;
        }
      }
    }
  }
  fprintf(fp,"$~\n");
  fclose(fp);
  remove13(fname);
}
    
void parse_rooms (FILE *fl) {
  char fname[10];
  char ch[32768], *s1, *s2, s[256];
  int t1,t2,t3,znr,stpe;
  int num,found = -1;
  int flgs;
  FILE *fp;

  sprintf(fname,"%d.wld",zonenum);
  fp = fopen (fname,"w");
  get_line(fl,ch);
  for (;;) {
    if (!strcmp(ch,"#0"))
      break;
    if (sscanf(ch,"#%d ",&num) == 1) {
      if (nznr >= 0)
        num = change_number (num);
      fprintf(fp,"#%d\n",num);
      maxroom = num;
      s1 = fread_string (fl,s);
      s2 = fread_string (fl,s);
      fprintf(fp,"%s~\n%s~\n",s1,s2);
      znr = fread_number (fl);
      flgs = fread_number (fl);
      stpe = fread_number (fl);
      znr = zonenum;
      flgs = check_roomflags (flgs);
      stpe = check_sectors(stpe);
      if (!cifre) fprintf(fp,"%d %s %d\n",znr,get_ascii(flgs),stpe);
      else  fprintf(fp,"%d %d %d\n",znr,(flgs),stpe);
      found = -1;
      while (found == -1) {
        get_line(fl,ch);
        switch (*ch) {
          case 'S':
            found = 0;
            fprintf(fp,"S\n");
            get_line (fl, ch);
            break;
          case 'D':
            fprintf(fp,"%s\n",ch);
            s1 = fread_string(fl,s);
            s2 = fread_string(fl,s);
            if (s1 == NULL)
              fprintf(fp,"~\n");
            else
              fprintf(fp, "%s~\n", s1);
            if (s2 == NULL)
              fprintf(fp,"~\n");
            else
              fprintf(fp, "%s~\n", s2);
            get_line (fl,ch);
            if (sscanf(ch," %d %d %d ",&t1,&t2,&t3) != 3) {
              fprintf(erf,"At room %d error in Directional Data.\n",num);
              exit (1);
            }
            if (nznr >= 0 && t3 > 0)
              t3 = change_number(t3);
            if (t2>0)
              t2 - change_number(t2);
            fprintf(fp,"%d %d %d\n",t1,t2,t3);
            break;
          case 'E':
            s1 = fread_string(fl,s);
            s2 = fread_string(fl,s);
            fprintf(fp,"E\n");
            fprintf(fp,"%s~\n",s1);
            fprintf(fp,"%s~\n",s2);
            break;
        }
      }
    }
  }
  fprintf(fp,"$~\n");
  fclose(fp);
  remove13(fname);
}
    
void count_rooms (FILE *fl) {
  char ch[32768], *s1, *s2, s[256];
  int znr,stpe;
  int num,found = -1;
  int flgs;

  while (!feof(fl)) {
    get_line (fl,ch);
    if (!feof(fl)) {
      if (strstr(ch,"#ROOMS"))
        break;
    }
  }

  if (feof(fl)) {
    fprintf(erf,"Could not find the #ROOMS section.\n");
    exit(1);
  }

  get_line(fl,ch);
  for (;;) {
    if (!strcmp(ch,"#0"))
      break;
    if (sscanf(ch,"#%d ",&num) == 1) {
      maxznr = num;
      s1 = fread_string (fl,s);
      s2 = fread_string (fl,s);
      znr = fread_number (fl);
      flgs = fread_number (fl);
      stpe = fread_number (fl);
      found = -1;
      while (found == -1) {
        get_line(fl,ch);
        switch (*ch) {
          case 'S':
            found = 0;
            get_line (fl, ch);
            break;
          case 'D':
            s1 = fread_string(fl,s);
            s2 = fread_string(fl,s);
            get_line (fl,ch);
            break;
          case 'E':
            s1 = fread_string(fl,s);
            s2 = fread_string(fl,s);
            break;
        }
      }
    }
  }
  maxznr += 99 - (maxznr % 100);
}
    
void parse_mobiles (FILE *fl) {
  char fname[10];
  char ch[32768], *s1, *s2, *s3, *s4,s[256],mtpe;
  int t1,t2,t3,t4,t5,t6,t7,t8,t9;
  int num;
  int flgs,affs,algn,mgold,mexp;
  FILE *fp;
  int mp=0,mpn=0;
  sprintf(fname,"%d.mob",zonenum);
  fp = fopen (fname,"w");
  for (;;) {
    get_line(fl,ch);
    if (!strcmp(ch,"#0"))
      break;
    if (sscanf(ch,"#%d ",&num) == 1) {
      if (nznr >= 0)
        num = change_number (num);
      fprintf(fp,"#%d\n",num);
      s1 = fread_string (fl,s);
      s2 = fread_string (fl,s);
      s3 = fread_string (fl,s);
      s4 = fread_string (fl,s);
      fprintf(fp,"%s~\n%s~\n%s~\n%s~\n",s1,s2,s3,s4);
      flgs = fread_number (fl);
      affs = fread_number (fl);
      flgs = check_mobflags (flgs, num);
      affs = check_mobaffs (affs,num);
      get_line (fl,ch);
      if (sscanf (ch," %d %c ",&algn,&mtpe) != 2) {
        fprintf(erf,"Error at mob %d 1st numeric line.\n",num);
        exit (1);
      }
if (!cifre) fprintf(fp,"%s %s %d %c\n",get_ascii(flgs),get_ascii(affs),algn,mtpe);
else fprintf(fp,"%d %d %d %c\n",(flgs),(affs),algn,mtpe);
      get_line (fl,ch);
      if (sscanf (ch," %d %d %d %dd%d+%d %dd%d+%d",&t1,&t2,&t3,&t4,&t5,&t6,&t7,&t8,&t9) != 9) {
        fprintf(erf,"Error at mob %d 2nd numeric line.\n",num);
        exit (1);
      }
      get_line (fl,ch);
      if (sscanf (ch," %d %d ",&mgold,&mexp) != 2) {
        fprintf(erf,"Error at mob %d 3rd numeric line.\n",num);
        exit (1);
      }
      calc_mob_vals(num,t1,&t2,&t3,&t4,&t5,&t6,&t7,&t8,&t9,&mgold,&mexp);
      fprintf (fp,"%d %d %d %dd%d+%d %dd%d+%d\n",t1,t2,t3,t4,t5,t6,t7,t8,t9);
      fprintf (fp,"%d %d\n",mgold,mexp);
      get_line (fl,ch);
      if (sscanf (ch," %d %s %d ",&t1,s1,&t3) != 3) {
        fprintf(erf,"Error at mob %d 4th numeric line.\n",num);
        exit (1);
      }
      if (t1==0 || t1==7) t1=8;
      fprintf (fp,"%d %d %d\n",t1,t1,t3);
    }
    else if ((*ch=='>') || mp){
    if (*ch=='>') {mp=1;mpn++;};
    if (*ch=='|') {mp=0;
    		printf("MOB %d has %d mobprogs assigned.\n",num,mpn);
    		mpn=0;
    }
    fprintf(fp,"%s\n",ch);
   }    
  }
  fprintf(fp,"$~\n");
  fclose(fp);
  remove13(fname);
}
    
int main (int argc, char *argv[])
{
  FILE *fl;
  int arg;
  char ch[32768];
  char ch1[32768];

  if (argc < 2) {
    fprintf(erf,"Illegal number of arguments.\n");
    exit (1);
  }

  for (arg = 1; arg < argc;arg++) {
    if (*(argv[arg]) == '-') {
      switch (*(argv[arg]+1)) {
        case 'c':
           cifre=TRUE;
           break;
        case 'n' :
          strcpy (ch,(argv[arg]+2));
          nznr = atoi (ch);
          if (nznr < 1 || nznr > 320) {
            fprintf(erf,"Invalid val for new zonenr. Range is 1 - 320.\n");
            exit(1);
          } 
//          fprintf(erf,"%d\n",nznr);
          break;
        case 't' :
          strcpy (ch,(argv[arg]+2));
          if (!strcmp(ch,"max")) {
            mobconv = MAXVALS;
          } else if (!strcmp(ch,"mid")) {
            mobconv = MIDVALS;
          } else if (!strcmp(ch,"merc")) {
            mobconv = MERCVALS;
          } else {
            fprintf(erf,"Invalid type for mobconversion. Valid types are mid,max and merc.\n");
            exit(1);
          }
          break;
        case 's':
          randspells = TRUE;
          break;
        case 'w':
          warn = FALSE;
          break;
        case 'r' :
          strcpy (ch,(argv[arg]+2));
          maxrand = atoi (ch);
          if (maxrand <= 1) {
            fprintf(erf,"Value for randomization must be > 1.\n");
            exit(1);
          } 
          break;
        case 'h':
          fprintf(erf,"Usage: merc2circle [options] mercfile.are\n");
          fprintf(erf,"The following options are valid.\n");
          fprintf(erf,"-n<val> Renumbers zone to new val.\n");
          fprintf(erf,"-c      Don't write ascii flags.\n");
          fprintf(erf,"-w      Display no warnings.\n");
          fprintf(erf,"-s      Randomize level and charges if 0.\n");
          fprintf(erf,"-r<val> Randomize Weapons to give a max dam of val.\n");
          fprintf(erf,"\n");
          fprintf(erf,"-t<max|mid|merc> How to calculate the Stats for mobs as in\n");
          fprintf(erf,"                 Merc areas these are often set to 0.\n");
          fprintf(erf,"                 The calculations are taken from Merc's source.\n");
          fprintf(erf,"        Max - Sets mobs to max values.\n");
          fprintf(erf,"        Mid - Sets mobs to average values.\n");
          fprintf(erf,"        Merc - Sets mobs to values generated in the same way\n");
          fprintf(erf,"               as they are on Merc. In circle these are permanent.\n");
          exit(0);
        default:
          fprintf(erf,"Illegal option %s.\n",argv[arg]);
          break;
      }
    } else
      break;
  }

  fl = fopen (argv[arg],"r");
  if (fl == NULL) {
    fprintf(erf,"Could not open file.\n");
    exit (1);
  }
  zonenum=0;
  while (zonenum==0) {
    get_line(fl,ch);
    if (sscanf(ch,"#%d",&zonenum))
    if (zonenum>0)
      break;
  }
  fclose (fl);
    fl = fopen (argv[arg],"r");
  count_rooms(fl);
  fclose (fl);

  zonenum = zonenum / 100;

  if (nznr >= 0) {
    oznr = zonenum;
    zonenum = nznr;
  }

  fl = fopen (argv[arg],"r");
  if (fl == NULL) {
    fprintf(erf,"Could not open file.\n\n");
    exit (1);
  }
{
char aa[10];
sprintf(aa,"%d.err",zonenum);
        erf=fopen(aa,"w");
}
  init_mm();
  while (!feof(fl)) {
    get_line (fl,ch);
    if (!feof(fl))
      if (sscanf(ch,"#AREA %s~",ch1) == 1) {
        strcpy(zonename,ch+6);
        fprintf(erf,"Zone %d - %s\n",zonenum, zonename);
      }
      if (!strcmp("#ROOMS",ch)) {
        parse_rooms(fl);
        fprintf(erf,"Finished Converting Rooms.\n\n");
      }
      if (!strcmp("#OBJECTS",ch)) {
        parse_objects(fl);
        fprintf(erf,"Finished Converting Objects.\n\n");
      }
      if (!strcmp("#MOBILES",ch)) {
        parse_mobiles(fl);
        fprintf(erf,"Finished Converting Mobs.\n\n");
      }
      if (!strcmp("#MOBPROGS",ch)) {
        fprintf(erf,"MOBPROGS section exists!\n\n");
      }
      
      if (!strcmp("#RESETS",ch)) {
        parse_resets(fl);
        fprintf(erf,"Finished Converting Resets.\n\n");
      }
      if (!strcmp("#SHOPS",ch)) {
        parse_shops(fl);
        fprintf(erf,"Finished Converting Shops.\n\n");
      }
      if (!strcmp("#SPECIALS",ch)) {
        parse_specials(fl);
        fprintf(erf,"Finished Converting Specials.\n\n");
      }
      if (!strcmp("#HELPS",ch)) {
        parse_helps(fl);
        fprintf(erf,"Finished Copying Help.\n\n");
      }
  }

  fclose (fl);
  fclose(erf);
  return 0;
}

