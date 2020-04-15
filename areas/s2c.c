#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include "s2c.h"

#define FALSE 0
#define TRUE (!FALSE)
#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))

#define MAX_STRING_LENGTH	32768
#define NOCONV   0
#define MAXVALS  1
#define MIDVALS  2
#define MERCVALS 3

#define MAXSPLLVL 50
#define MAXCHARGE 15
#define MAX_OBJ_AFFS 9

#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)
int             level_add = 0;
int nochance = 0;
int             zonenum,
                nznr = -1,
                oznr,
                mobconv = NOCONV,
                maxznr,
                minznr = 10000000;
int             znum = -1;
int             maxroom,
                warn = TRUE,
                maxrand = -1,
                randspells = FALSE,
                cifre = FALSE;
char            zonename[256];
FILE           *erf1;
FILE           *prg_erf1;

#define ROOM_RECALL (1 << 23)
#define ITEM_PILL   26
#define AFF_FLY     (1 << 22)


#define SPELLMAX 83
int             spells[] = {
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

/* ***************************************************************************
 * The following functions is taken from Merc.                               *
 * Merc is created by Kahn, Hatchet and Furey                                *
 ************************************************************************** */

static int      rgiState[2 + 55];

char           *trim(char *str)
{
    char           *ibuf,
                   *obuf;

    if (str) {
        for (ibuf = obuf = str; *ibuf;) {
            while (*ibuf && (isspace(*ibuf)))
                ibuf++;
            if (*ibuf && (obuf != str))
                *(obuf++) = ' ';
            while (*ibuf && (!isspace(*ibuf)))
                *(obuf++) = *(ibuf++);
        }
        *obuf = 0;
    }
    return (str);
}


void            skip_spaces(char **string)
{
    for (; **string && isspace(**string); (*string)++);
}

void            init_mm()
{
    int            *piState;
    int             iState;
    time_t          current_time;
    struct timeval  now_time;

    gettimeofday(&now_time, NULL);
    current_time = (time_t) now_time.tv_sec;

    piState = &rgiState[2];

    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;

    piState[0] = ((int) current_time) & ((1 << 30) - 1);
    piState[1] = 1;
    for (iState = 2; iState < 55; iState++) {
        piState[iState] = (piState[iState - 1] + piState[iState - 2])
            & ((1 << 30) - 1);
    }
    return;
}



int             number_mm(void)
{
    int            *piState;
    int             iState1;
    int             iState2;
    int             iRand;

    piState = &rgiState[2];
    iState1 = piState[-2];
    iState2 = piState[-1];
    iRand = (piState[iState1] + piState[iState2])
        & ((1 << 30) - 1);
    piState[iState1] = iRand;
    if (++iState1 == 55)
        iState1 = 0;
    if (++iState2 == 55)
        iState2 = 0;
    piState[-2] = iState1;
    piState[-1] = iState2;
    return iRand >> 6;
}

int             interpolate(int level, int value_00, int value_47)
{
    return value_00 + level * (value_47 - value_00) / 47;
}

int             number_bits(int width)
{
    return number_mm() & ((1 << width) - 1);
}

int             number_fuzzy(int number)
{
    switch (number_bits(2)) {
 case 0:
        number -= 1;
        break;
    case 3:
        number += 1;
        break;
    }

    return UMAX(1, number);
}

int             number_range(int from, int to)
{
    int             power;
    int             number;

    if ((to = to - from + 1) <= 1)
        return from;

    for (power = 2; power < to; power <<= 1);

    while ((number = number_mm() & (power - 1)) >= to);

    return from + number;
}
char buf[1000];
int             fread_number(FILE * fp)
{
    int             number;
    int             sign;
    char            c;

    do {
        c = getc(fp);
    }
    while (isspace(c));

    number = 0;

    sign = FALSE;
    if (c == '+') {
        c = getc(fp);
    } else if (c == '-') {
        sign = TRUE;
        c = getc(fp);
    }
    if (!isdigit(c)) {
        
	if (c=='~')
	{
		c = getc(fp);
		c = getc(fp);
	}
	else                                                  
	{
		printf("Fread_number: bad format (%c).\r\n", c);
        	fgets( buf, 1000, fp);
        	fgets( buf, 1000, fp);
		printf("Fread_number: bad format: %s", buf);        
        	exit(1);
	}
    }
    while (isdigit(c)) {
        number = number * 10 + c - '0';
        c = getc(fp);
    }

    if (sign)
        number = 0 - number;

    if (c == '|')
        number += fread_number(fp);
    else if (c != ' ')
        ungetc(c, fp);

    return number;
}

/* ***************************************************************************
 * The follwing functions is taken from CircleMud                            *
 * CircleMud is created by Jeremy Elson                                      *
 ************************************************************************** */

char           *str_dup(const char *source)
{
    char           *new;

    CREATE(new, char, strlen(source) + 1);
    return (strcpy(new, source));
}

int             get_line(FILE * fl, char *buf)
{
    char            temp[256];
    int             lines = 0;

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

char           *fread_string2(FILE * fl, char *error)
{
    char            buf[32768],
                    tmp[512],
                   *rslt;
    register char  *point;
    int             done = 0,
                    length = 0,
                    templength = 0;

    *buf = '\0';

    do {
        if (!fgets(tmp, 512, fl)) {
            fprintf(erf1, "SYSERR: fread_string: format error at or near %s\n",
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
            fprintf(erf1, "SYSERR: fread_string: string too large (db.c)");
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
char           *fread_string(FILE * fp, char *error)
{
    char            buf[32768];
    char           *plast;
    char            c;
    int             ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /* Skip blanks. Read first char. */
    do {
        if (feof(fp)) {
            fprintf(stderr, "SYSERR: fread_string: format error at or near %s\n",
                    error);
            exit(1);

        }
        c = getc(fp);
    }
    while (isspace(c));

    if ((*plast++ = c) == '~')
        // return NULL;
        return str_dup("");

    for (;;) {
        if (ln >= (MAX_STRING_LENGTH - 1)) {
            fprintf(stderr, "SYSERR: fread_string: string too long near %s\n",
                    error);
            *plast = '\0';
            if (strlen(buf) > 0)
                return str_dup(buf);
            else
                // return NULL;
                return str_dup("");
        }
        switch (*plast = getc(fp)) {
        default:
            plast++;
            ln++;
            break;

        case EOF:
            fprintf(stderr, "SYSERR: fread_string: eof near %s\n",
                   error );
            *plast = '\0';
            if (strlen(buf) > 0)
                return str_dup(buf);
            else
                // return NULL;
                return str_dup("");
            break;

        case '\n':
            plast++;
            ln++;
            *plast++ = '\r';
            ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            if (strlen(buf) > 0)
                return str_dup(buf);
            else
                // return NULL;
                return str_dup("");
        }
    }
}


/* ***************************************************************************
 * The follwing is my own creation, feel free to make changes.               *
 ************************************************************************** */

int             get_exp(int level)
{
    int             diff,
                    exp = 0;

    if (level > 50) {
        diff = level - 50;
//    exp = (diff * 100000) + mobexp[50];
    } else
        // exp = mobexp[level];
        exp = 0;
    return (exp);
}

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

char           *one_argument(char *argument, char *arg_first)
{
    char            cEnd;
    int             count;

    count = 0;

    while (isspace(*argument))
        argument++;

    cEnd = ' ';
    // if ( *argument == '\'' || *argument == '"' )
//	cEnd = *argument++;

    while (*argument != '\0' || ++count >= 255) {
        if (*argument == cEnd) {
            argument++;
            break;
        }
        *arg_first = (*argument);
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while (isspace(*argument))
        argument++;

    return argument;
}


int             change_number(int old)
{
    int             i;

    if (nznr >= 0) {            // && old >= oznr * 100 && old <= maxznr) {
        i = ((int) old / 100) - oznr;
        old = old % 100;
        old += (nznr + i) * 100;
    }
    return (old);
}

void            check_prog(char *prog)
{
    char           *p,
                   *t;
    char            buf[1000];
    char            target[1000];
    char            nnr[1000];
    int             f = 0;
    int             a;
    int             cast = 0;
    int             first = 1;
    *target = 0;
    t = prog;

    // if (!nznr)
    // return;


    while (*t) {

        t = one_argument(t, buf);

        if ((a = atoi(buf)) && (a > znum * 100) && nznr != -1) {
            a = change_number(a);
            sprintf(buf, "%d%s", a, *(buf + strlen(buf) - 1) == '~' ? "~" : "");
            f = 1;
        }
        if (!first)
            strcat(target, " ");
        if (cast && !(*buf == '\'')) {
            strcat(target, "'");
            strcat(target, buf);
            strcat(target, "'");
            cast = 0;
            f = 1;
        } else {
            strcat(target, buf);
            cast = 0;
        }
        if ((*buf == 'c') && !(*(buf + 1)))
            cast = 1;
        first = 0;

    }
    if (f == 1 && warn)
        fprintf(prg_erf1, "PROG: Changed \"%s\" to \"%s\"\n", prog, target);
    else if ((a = strcmp(trim(prog), target)))
        fprintf(prg_erf1, "ERROR: Not identical at %d '%s' to '%s'\n", a, trim(prog), target);

    strcpy(prog, target);

}

void            calc_mob_vals(int num, int level, int *thac0, int *ac, int *h1, int *h2, int *h3,
                             int *d1, int *d2, int *d3, int *gold, int *exp)
{
    char            buf[500];
    int             hit = 0,
                    dam = 0;

    init_mm();
    switch (mobconv) {
    case MERCVALS:
        *thac0 = interpolate(level, 20, 0);
        *ac = interpolate(level, 10, -10);
        *exp = get_exp(level);
        *gold = number_fuzzy(10) * number_fuzzy(level) * number_fuzzy(level);
        dam = number_range(level / 2, (level * 3) / 2);
        hit = (level * 8) + number_range(level * level / 4, level * level);
        break;
    case MAXVALS:
        *thac0 = interpolate(level, 20, 0);
        *ac = interpolate(level, 100, -100);
        *exp = get_exp(level);
        *gold = (10 + 1) * (level + 1) * (level + 1);
        dam = (level + 3) / 2;
        hit = (level * 8) + (level * level);
        break;
    case MIDVALS:
        *thac0 = interpolate(level, 20, 0);
        *ac = interpolate(level, 100, -100);
        *gold = 10 * level * level;
        *exp = get_exp(level);
        dam = ((level / 2) + ((level + 3) / 2)) / 2;
        hit = (level * 8) + (((level * level / 4) + (level * level)) / 2);
        break;
    default:
        if (*thac0 == 0 && *ac == 0) {
            *thac0 = interpolate(level, 20, -5);
            *ac = interpolate(level, 10, -15);
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
    if (level > 50 || level < 1)
        fprintf(erf1, "W A R N I N G: Mob %d has level set to %d\n", num, level);
// if (((*h1)*(*h2)+(*h3))==0) fprintf(erf1,"Warning: Mob %d has 0 hitpoints\n",num);
    if (((*h1) * ((*h2 + 1) / 2) + (*h3)) > 1000)
        fprintf(erf1, "Warning: Mob %d has %d (%d) hitpoints\n", num, ((*h1) * ((*h2 + 1) / 2) + (*h3)), ((*h1) * (*h2) + (*h3)));
// if (((*d1)*(*d2)+(*d3))==0) fprintf(erf1,"Warning: Mob %d does no damage\n",num);
    if (((*d1) * ((*d2 + 1) / 2) + (*d3)) > 40)
        fprintf(erf1, "Warning: Mob %d does %d (%d) damage\n", num, (*d1) * ((*d2 + 1) / 2) + (*d3), (*d1) * (*d2) + (*d3));
/* if (*gold==0) fprintf(erf1,"Warning: Mob %d has no gold\n",num);
 if (*exp==0) fprintf(erf1,"Warning: Mob %d has no exp\n",num);
*/
}

char           *get_maffs(int i)
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

char           *get_mflag(int i)
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
    case 11:
        return ("IMMORTAL");
        break;
    case 14:
        return ("META AGGR");
        break;
    case 21:
        return ("SECRETIVE");
        break;
    case 23:
        return ("MOB INVIS");
        break;
    case 27:
        return ("NO ATTACKS");
        break;
    case 28:
        return ("ANNOYING");
        break;
    }

    return ("ERROR BUG THIS SHOULD NOT BE HERE.");
}


int             check_spell(int splnr, int objnr)
{
    int             nsplnr = -2,
                    found = FALSE;


    int             i;
    if (splnr == -1)
        return -1;

    for (i = 1; i < sizeof(smaug_spells) / sizeof(int); i += 2) {
        if (smaug_spells[i] == splnr) {
            nsplnr = smaug_spells[i - 1];
            break;
        }
    }
    if (nsplnr == -1)
        if (warn)
            fprintf(erf1, "SPELLS: Unsupported SPELL (%d) found in obj %d. Set to -1.\n", splnr, objnr);
    if (nsplnr == -2) {
        if (warn)
            fprintf(erf1, "SPELLS: Unsupported SMAUG SPELL (%d) found in obj %d. Set to -1.\n", splnr, objnr);
        nsplnr = -1;
    }
    return nsplnr;





    if (splnr <= 44) {
        if (splnr == 34 && splnr == 41) {
            if (warn)
                fprintf(erf1, "SPELLS: Vent or Prot Evil (%d) in obj %d. Set to UNKNOWN\n", -1, objnr);
            nsplnr = -1;
        } else
            nsplnr = splnr;
        found = TRUE;
    } else if (splnr <= SPELLMAX) {
        nsplnr = spells[splnr - 45];
        found = TRUE;
    }
    if (!found) {
        if (warn)
            fprintf(erf1, "SPELLS: Unsupported SPELL (%d) found in obj %d. Set to -1.\n", splnr, objnr);
        nsplnr = -1;
    }
    return (nsplnr);
}

int             check_mobaffs(int mflg, int mobnr)
{
    int             i,
                    nmflg = 0;

    for (i = 0; i <= 31; i++) {
        switch (i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 7:
            if (IS_SET(mflg, (1 << i)))
                SET_BIT(nmflg, (1 << i));
            break;

        case 9:
            if (IS_SET(mflg, (1 << 9))) {
                SET_BIT(nmflg, (1 << 10));
            }
            break;
        case 10:
            if (IS_SET(mflg, (1 << 10))) {
                SET_BIT(nmflg, (1 << 9));
            }
            break;
        case 12:
            if (IS_SET(mflg, (1 << 12))) {
                SET_BIT(nmflg, (1 << 11));
            }
            break;
        case 13:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 12));
            }
            break;
        case 14:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 16));
            }
            break;
        case 15:
            if (IS_SET(mflg, (1 << 15))) {
                SET_BIT(nmflg, (1 << 18));
            }
            break;
        case 16:
            if (IS_SET(mflg, (1 << 16))) {
                SET_BIT(nmflg, (1 << 19));
            }
            break;
        case 17:
            if (IS_SET(mflg, (1 << 17))) {
                SET_BIT(nmflg, (1 << 14));
            }
            break;
        case 18:
            if (IS_SET(mflg, (1 << 18))) {
                SET_BIT(nmflg, (1 << 21));
            }
            break;
        case 19:
            if (IS_SET(mflg, (1 << 19)))
                SET_BIT(nmflg, (1 << 22));
            break;
        case 20:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 26));
            }
            break;
        case 21:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 22));
            }
            break;
        case 22:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 5));
                SET_BIT(nmflg, (1 << 3));
            }
            break;
        case 25:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 29));
            }
        case 26:
        case 28:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 28));
            }
            break;
        case 31:
            if (IS_SET(mflg, (1 << i))) {
                SET_BIT(nmflg, (1 << 23));
            }
            break;
        default:
            if (IS_SET(mflg, (1 << i))) {
                if (warn) {
                    if (mobnr > 0)
                        fprintf(erf1, "Unknown MOB affect %s in mob %d. It has been removed.\n", mob_aff[i], mobnr);
                    else
                        fprintf(erf1, "Unknown APPLY_AFFECT %s in obj %d. It has been removed.\n", mob_aff[i], -mobnr);
                }
            }
            break;
        }
    }

    return (nmflg);
}

int             check_objapply(FILE * fp, int t1, int t2, int num)
{
    int             t = 0;
    if (t1 == 6 || t1 == 7 || t1 == 8 || t1 == 15 || t1 == 16)
        fprintf(erf1, "Object %d has touched forbidden (%s)\n", num, apply_types[t1]);
    else {
        if (t1 == 25)
            t1 = 6;

        else if (t1 == 32)
            t1 = -SKILL_BACKSTAB;
        else if (t1 == 33)
            t1 = -SKILL_PICK_LOCK;
        else if (t1 == 34)
            t1 = -SKILL_TRACK;
        else if (t1 == 35)
            t1 = -SKILL_STEAL;
        else if (t1 == 36)
            t1 = -SKILL_SNEAK;
        else if (t1 == 37)
            t1 = -SKILL_HIDE;
        else if (t1 == 40)
            t1 = -SKILL_DODGE;
        else if (t1 == 41)
            t1 = -SKILL_PEEK;
        else if (t1 == 43)
            t1 = -SKILL_GOUGE;
        else if (t1 == 46)
            t1 = -SKILL_DISARM;
        else if (t1 == 47)
            t1 = -SKILL_KICK;
        else if (t1 == 48)
            t1 = -SKILL_PARRY;
        else if (t1 == 49)
            t1 = -SKILL_BASH;
        else if (t1 == 53)
            t1 = -SKILL_GRIP;




        if (t1 == 26) {         // apply affect
            t2 = check_mobaffs(t2, -num);
            if (t2)
                fprintf(fp, "F\n%d 0 0\n", t2);
            // if (warn)
            // fprintf(erf1,"Object %d had APPLY_AFFECT. Converted.\n",num);


        } else if (t1 > 26) {
            if (t1 != 27 && t1 != 28 && t1 != 29 && t1 != 31)   // immune resis, susc
                                                                // and lck
                if (warn)
                    fprintf(erf1, "Object %d has unsupported apply - %s. Removed\n", num, apply_types[t1]);
        } else {
            fprintf(fp, "A\n");
            fprintf(fp, "%d %d\n", t1, t2);
            t = 1;
        }
    }
    return t;
}
int             check_mobflags(int mflg, int mobnr)
{
    int             i,
                    nmflg = 0;
    SET_BIT(nmflg, (1 << 12));
    for (i = 0; i <= 31; i++) {
        switch (i) {
        case 1:
        case 2:
        case 5:
        case 6:
        case 7:
            if (IS_SET(mflg, (1 << i)))
                SET_BIT(nmflg, (1 << i));
            break;
        case 24:
            if (IS_SET(mflg, (1 << i)))
                REMOVE_BIT(nmflg, (1 << 12));
            break;
        case 0:
        case 8:
        case 9:
        case 10:
        case 11:
        case 14:
        case 21:
        case 23:
        case 27:
        case 28:

            if (IS_SET(mflg, (1 << i))) {
                if (warn && i != 0)
                    fprintf(erf1, "Unsupported Mob Flag - %s at mob %d. Removing\n", get_mflag(i), mobnr);
            }
            break;

        case 18:
        case 19:

            if (IS_SET(mflg, (1 << i))) {
                if (warn)
                    fprintf(erf1, "MOUNTABLE or MOUNTED Mob Flag - at mob %d. Removing\n", mobnr);
            }
            break;
        default:
            if (IS_SET(mflg, (1 << i))) {
                // if (warn)
                // fprintf(erf1,"Unknown MOB FLAG in mob %d. It has been
                // removed.\n",mobnr);
            }
            break;
        }
    }

    return (nmflg);
}

int             check_objflags(int oflg, int objnr)
{
    int             i,
                    noflg = 0;

    for (i = 0; i <= 31; i++) {
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
            if (IS_SET(oflg, (1 << i))) {

                SET_BIT(noflg, (1 << i));
            }
            break;

        case 3:      // loyal->autoengrave
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 21));
            break;
        case 12:                // noremove
            if (IS_SET(oflg, (1 << i))) {
                SET_BIT(noflg, (1 << 18));
            }
            break;
        case 14:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 12));
            break;
        case 15:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 14));
            break;
        case 16:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 15));
            break;
        case 17:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 13));
            break;
        case 18:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 25));
            break;
        case 19:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 22));
            break;
        case 24:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, (1 << 23));
            break;
        case 29:                // buried
            if (IS_SET(oflg, (1 << i))) {
                if (warn)
                    fprintf(erf1, "Obj %d has BURIED flag set\n", objnr);
                SET_BIT(noflg, (1 << 28));
            }
            break;
        default:
            if (IS_SET(oflg, (1 << i))) {
                if (warn && i != 2 && i != 4)
                    fprintf(erf1, "Unsupported Obj Flag - %s at obj %d. Removing\n", item_flags[i], objnr);
            }
            break;
        }
    }

    return (noflg);
}

int             check_objvalues(int *t1, int *t2, int *t3, int *t4, int num, int nznr, int tpe)
{
    if (tpe == 15) {
        if (nznr >= 0) {
            *t3 = change_number(*t3);
        }
        if (IS_SET(*t2, (1 << 4))) {
            if (warn)
                fprintf(erf1, "EATKEY flag on container %d. Removed\n", num);
            REMOVE_BIT(*t3, 1 << 4);
        }
    }
    if (tpe == 23) {
        if (*t1 < 1 || *t2 < 1) {
            *t1 = 20000;
            *t2 = *t1;
        }
    }
    if (tpe == 5) {             /* Weapons */
        switch (*t4) {        
        case 3:
            *t4 = 3;
            break;    
        case 1:
        case 2:
        case 5:
        case 11:
            *t4 = 11;
            break;
        default:
            *t4 = 7;
            break;
        }

    }
    if (tpe == 2 || tpe == 10) {/* Scroll and Potion */
        *t2 = check_spell(*t2, num);
        *t3 = check_spell(*t3, num);
        *t4 = check_spell(*t4, num);
        if (*t2 < 1 && *t3 < 1 && *t4 < 1 && warn)
            fprintf(erf1, "Potion/Scroll object %d  needs spell assigned.\n", num);
    } else if (tpe == 3 || tpe == 4) {  /* Wands and Staves */
        if (*t1 < 1 && randspells)
            *t1 = number_range(1, MAXSPLLVL);
        if (*t2 < 1 && randspells) {
            *t2 = number_range(1, MAXCHARGE);
            *t3 = *t2;
        }
        if ((*t1 < 1 || *t2 < 1 || *t3 < 1) && !randspells && warn)
            fprintf(erf1, "Some values in staff/wand object %d are 0.\n", num);
        *t4 = check_spell(*t4, num);
        if (*t4 < 1 && warn)
            fprintf(erf1, "Staff/Wand object %d needs spell assigned.\n", num);
    }
return 0;
}

int             check_rstwears(int oflg)
{

    switch (oflg) {
 case 0:
 case 1:
 case 2:
 case 3:
 case 4:
 case 5:
 case 6:
 case 7:
 case 8:
 case 9:
 case 10:
 case 11:
 case 12:
 case 13:
 case 14:
 case 15:
 case 16:
 case 17:
 case 18:

        break;

    case 19:
        oflg = 21;
        break;
    case 20:
        oflg = 22;
        break;
    case 21:
        oflg = 17;
        break;
    case 22:
        oflg = 19;
        break;
    case 23:
        oflg = 20;
        break;
    case 24:
    case 25:
        oflg = oflg - 10;
        break;
    default:
        fprintf(erf1, "Unsupported wear Flag - %d \n", oflg);

        break;
    }


    return (oflg);
}

int             check_objwears(int oflg, int objnr)
{
    int             i,
                    noflg = 0;

    for (i = 0; i <= 31; i++) {
        switch (i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, 1 << i);
            break;

        case 15:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, 1 << 13);
            break;
        case 20:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, 1 << 16);
            break;
        case 19:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, 1 << 15);
            break;
        case 16:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, 1 << 17);
            break;
        case 17:
            if (IS_SET(oflg, (1 << i)))
                SET_BIT(noflg, 1 << 18);
            break;
        case 18:
            if (IS_SET(oflg, (1 << i))) {
                if (warn)
                    fprintf(erf1, "Unsupported Obj WEAR_MISSILEWIELD at obj %d. Setting to WEAR_HOLD\n", objnr);
                SET_BIT(noflg, 1 << 14);
            }
            break;
        case 21:
            if (IS_SET(oflg, (1 << i))) {
                if (warn)
                    fprintf(erf1, "Unsupported Obj WEAR_ANKLE at obj %d. Setting to WEAR_WRIST\n",  objnr);
                SET_BIT(noflg, 1 << 12);
            }
            break;

        default:
            if (IS_SET(oflg, (1 << i))) {
                if (warn)
                    fprintf(erf1, "Unsupported Obj WEAR - %d at obj %d. Removing\n", i,  objnr);
            }
            break;
        }
    }

    return (noflg);
}

int             check_objtypes(int tpe, int objnr)
{

    if (tpe == 25) {
        tpe = 23;
    } else if (tpe == 26) {
        tpe = 10;
        if (warn)
            fprintf(erf1, "ITEM PILL found at obj %d. Setting to Potion.\n", objnr);
    } else if (tpe == 23 || tpe == 24) {
        if (warn)
            fprintf(erf1, "ITEM CORPSE found at obj %d. Setting to container.\n", objnr);
        tpe = 15;
    } else if (tpe == 45 || tpe == 47) {
        if (warn)
            fprintf(erf1, "ITEM MAP OR PAPER found at obj %d. Setting to NOTE.\n", objnr);
        tpe = 16;
    } else if (tpe == 54 || tpe == 55 || tpe == 56) {
        if (warn)
            fprintf(erf1, "ITEM BOW found at obj %d. Setting to FIREWEAPON.\n", objnr);
        tpe = 6;
    } else if (tpe == 57) {
        if (warn)
            fprintf(erf1, "ITEM PROJECTILE found at obj %d. Setting to MISSILE.\n", objnr);
        tpe = 7;
    } else if (tpe == 14) {
        if (warn)
            fprintf(erf1, "ITEM TRAP found at obj %d. Setting to OTHER\n", objnr);
        tpe = 12;

    } else if (tpe == 36 || tpe == 37 || tpe == 38) {
        // if (warn)
        // fprintf(erf1,"NOTICE: ITEM LEVER found at obj %d.\n",objnr);
        tpe = 31;
    } else if (tpe == 39) {
        if (warn)
            fprintf(erf1, "NOTICE: ITEM LBUTTON found at obj %d.\n", objnr);
        tpe = 32;
    } else if (tpe > 22) {
        if (warn)
            fprintf(erf1, "Unsupported ITEM Type (%s) found at obj %d. Setting to Other.\n", item_types[tpe], objnr);
        tpe = 12;
    }
    return (tpe);
}

int             check_exitflags(int rflg)
{
    int             i,
                    nrflg = 0;

    for (i = 0; i <= 31; i++) {
        switch (i) {
        case 0:
        case 1:
        case 2:
            if (IS_SET(rflg, (1 << i)))
                SET_BIT(nrflg, (1 << i));
            break;
        case 5:

            if (IS_SET(rflg, (1 << i)))
                SET_BIT(nrflg, (1 << 3));
            break;
        case 3:
        case 11:
            if (IS_SET(rflg, (1 << i))) {
                SET_BIT(nrflg, (1 << 4));
                // if (warn)
                // fprintf(erf1,"Hidden or secret exit in room
                // %d.\n",maxroom);
            }
            break;
        case 12:

            if (IS_SET(rflg, (1 << i)))
                SET_BIT(nrflg, (1 << 5));
            break;

        case 24:

            if (IS_SET(rflg, (1 << i)))
                SET_BIT(nrflg, (1 << 6));
            break;
        case 23:

            if (IS_SET(rflg, (1 << i)))
                SET_BIT(nrflg, (1 << 8));
            break;
        case 10:

            if (IS_SET(rflg, (1 << i)))
                SET_BIT(nrflg, (1 << 7));
            break;

        default:
            if (IS_SET(rflg, (1 << i))) {
                if (warn)
                    if ((i >= 16 && i <= 19) || i == 6 || i == 7)   // xXXX fly climb
                    {
                        break;
                    }    
                    else {
                        fprintf(erf1, "Unsupported EXIT FLAG (%s) in room %d. It has been removed.\n", exit_flags[i], maxroom);
                    }
            }
            break;
        }
    }

    return (-nrflg);
}



  int nor=0, nos=0;
int             check_roomflags(int rflg)
{
    int             i,
                    nrflg = 0;

    for (i = 0; i <= 31; i++) {
        switch (i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 7:
        case 8:
        case 9:
            if (IS_SET(rflg, (1 << i)))
                SET_BIT(nrflg, (1 << i));
            break;
        case 10:
            if (IS_SET(rflg, (1 << 10))) {
                SET_BIT(nrflg, (1 << 4));
            }
            break;
        case 11:
            if (IS_SET(rflg, (1 << 11))) {
                SET_BIT(nrflg, (1 << 8));
            }
            break;
        case 16:
            if (IS_SET(rflg, (1 << 16))) {
                SET_BIT(nrflg, (1 << 5));
            }
            break;
        case 12:
            if (IS_SET(rflg, (1 << 12))) {
                if (warn)
                    fprintf(erf1, "PET SHOP flag on room %d. Add to spec_assign.c.\n", maxroom);
            }
            break;
        case 13:
        case 21:
            if (IS_SET(rflg, (1 << i))) {
                SET_BIT(nrflg, (1 << 23));
                nor++;
            }
            break;
        case 20:
            if (IS_SET(rflg, (1 << 20))) {
                SET_BIT(nrflg, (1 << 24));
                nos++;
            }
            break;
        case 30:
            if (IS_SET(rflg, (1 << 30))) {
                if (warn)
                    fprintf(erf1, "ROOM PROTOTYPE flag on room %d.\n", maxroom);
            }
            break;
            /* case 24: if (IS_SET(rflg,(1 << 14))) { if (warn)
             * fprintf(erf1,"Nonstandard MERC ROOM FLAG at room %d.
             * silence/bank who knows?\n",maxroom); } break; */
        default:
            if (IS_SET(rflg, (1 << i))) {
                if (warn && i != 4 && i != 5 && i != 6)
                    fprintf(erf1, "Unsupported ROOM FLAG (%s) in room %d. It has been removed.\n", room_flags[i], maxroom);
            }
            break;
        }
    }

    return (nrflg);
}

int             check_sectors(int stpe)
{

    if (stpe > 9) {
        if (stpe == 10) {
            if (warn)
                fprintf(erf1, "Sectortype DESERT at room %d set to FIELD.\n", maxroom);
            stpe = 2;
        } else if (stpe == 11) {
            if (warn)
                fprintf(erf1, "Nonstandard MERC Sector at room %d. Setting to Field.\n", maxroom);
            stpe = 2;
        } else if (stpe == 12) {
            if (warn)
                fprintf(erf1, "Sectortype OCEANFLOOR at room %d set to UNDERWATER.\n", maxroom);
            stpe = 8;
        } else if (stpe == 13) {
            if (warn)
                fprintf(erf1, "Sectortype UNDERGROUND at room %d set to INSIDE.\n", maxroom);
            stpe = 0;
        } else {
            if (warn)
                fprintf(erf1, "Unknown sectortype at room %d. Setting to Field.\n", maxroom);
            stpe = 2;
        }
    }
    return (stpe);
}

char           *get_ascii(int val)
{
    char            temp[32],
                   *tmp;

    strcpy(temp, "\0");

    if (IS_SET(val, 1 << 0))
        strcat(temp, "a");
    if (IS_SET(val, 1 << 1))
        strcat(temp, "b");
    if (IS_SET(val, 1 << 2))
        strcat(temp, "c");
    if (IS_SET(val, 1 << 3))
        strcat(temp, "d");
    if (IS_SET(val, 1 << 4))
        strcat(temp, "e");
    if (IS_SET(val, 1 << 5))
        strcat(temp, "f");
    if (IS_SET(val, 1 << 6))
        strcat(temp, "g");
    if (IS_SET(val, 1 << 7))
        strcat(temp, "h");
    if (IS_SET(val, 1 << 8))
        strcat(temp, "i");
    if (IS_SET(val, 1 << 9))
        strcat(temp, "j");
    if (IS_SET(val, 1 << 10))
        strcat(temp, "k");
    if (IS_SET(val, 1 << 11))
        strcat(temp, "l");
    if (IS_SET(val, 1 << 12))
        strcat(temp, "m");
    if (IS_SET(val, 1 << 13))
        strcat(temp, "n");
    if (IS_SET(val, 1 << 14))
        strcat(temp, "o");
    if (IS_SET(val, 1 << 15))
        strcat(temp, "p");
    if (IS_SET(val, 1 << 16))
        strcat(temp, "q");
    if (IS_SET(val, 1 << 17))
        strcat(temp, "r");
    if (IS_SET(val, 1 << 18))
        strcat(temp, "s");
    if (IS_SET(val, 1 << 19))
        strcat(temp, "t");
    if (IS_SET(val, 1 << 20))
        strcat(temp, "u");
    if (IS_SET(val, 1 << 21))
        strcat(temp, "v");
    if (IS_SET(val, 1 << 22))
        strcat(temp, "w");
    if (IS_SET(val, 1 << 23))
        strcat(temp, "x");
    if (IS_SET(val, 1 << 24))
        strcat(temp, "y");
    if (IS_SET(val, 1 << 25))
        strcat(temp, "z");
    if (IS_SET(val, 1 << 26))
        strcat(temp, "A");
    if (IS_SET(val, 1 << 27))
        strcat(temp, "B");
    if (IS_SET(val, 1 << 28))
        strcat(temp, "C");
    if (IS_SET(val, 1 << 29))
        strcat(temp, "D");
    if (IS_SET(val, 1 << 30))
        strcat(temp, "E");
    if (IS_SET(val, 1 << 31))
        strcat(temp, "F");

    if (strlen(temp) < 1)
        strcat(temp, "0");

    tmp = str_dup(temp);
    return (tmp);
}

void            remove13(char *name)
{
    int             i;
    FILE           *fl;
    FILE           *ofl;
    char            buf[32768];
    char            ch;

    i = 1;
    strcpy(buf, name);
    strcat(buf, ".new");
    if ((ofl = fopen(buf, "w")) == NULL) {
        fprintf(erf1, "An error occured at %s\n", buf);
        exit(1);
    }
    if ((fl = fopen(name, "r")) == NULL) {
        fprintf(erf1, "An error occured at %s\n", name);
        exit(1);
    }
    ch = getc(fl);
    while (ch != EOF) {
        if (ch != '\r')
            putc(ch, ofl);
        ch = getc(fl);
    }
    fclose(fl);
    fclose(ofl);
    remove(name);
    rename(buf, name);
}

void            parse_shops(FILE * fl)
{
    char            fname[10],
                    fname2[10];
    char            ch[32768],
                    cs[32768];
    FILE           *fp,
                   *fs;
    int             mob,
                    t1,
                    t2,
                    t3,
                    t4,
                    t5,
                    sell,
                    buy,
                    op,
                    clo,
                    objs[30],
                    i = 0,
                    j;
    int             temp1,
                    temp2,
                    temp3,
                    room;
    int             tmp1,
                    tmp2,
                    tmp3,
                    shpnr = 0;
    float           fsell,
                    fbuy;
    int             open = 0;



    get_line(fl, ch);
    while (strcmp(ch, "0")) {
        if (!open) {
            sprintf(fname, "%d.shp", zonenum);
            fp = fopen(fname, "w");
            fprintf(fp, "CircleMUD v3.0 Shop File~\n");
            open = 1;
        }
        shpnr++;
        if (sscanf(ch, " %d %d %d %d %d %d %d %d %d %d ", &mob, &t1, &t2, &t3, &t4, &t5, &sell, &buy, &op, &clo) != 10) {
            fprintf(erf1, "Error in shop\n");
            exit(1);
        }
        mob = change_number(mob);
        sprintf(fname2, "%d.zon", zonenum);
        fs = fopen(fname2, "r");
        while (!feof(fs)) {
            get_line(fs, cs);
            if (sscanf(cs, "M %d %d %d %d ", &temp1, &temp2, &temp3, &room) == 4) {
                if (temp2 == mob)
                    break;
            }
        }
        if (temp2 == mob) {
            i = 0;
            while (!feof(fs)) {
                get_line(fs, cs);
                if (*cs == 'M' || *cs == 'S')
                    break;
                if (sscanf(cs, "G %d %d %d ", &tmp1, &tmp2, &tmp3) == 3) {
                    objs[i] = tmp2;
                    i++;
                }
            }
        }
        fclose(fs);
        fprintf(fp, "#%d0%d~\n", zonenum, shpnr);
        if (i > 0) {
            for (j = 0; j < i; j++)
                fprintf(fp, "%d\n", objs[j]);
        }
        fprintf(fp, "-1\n");
        fsell = sell / 100.00;
        fbuy = buy / 100.00;
        fprintf(fp, "%1.2f\n", fsell);
        fprintf(fp, "%1.2f\n", fbuy);
        t1 = check_objtypes(t1, shpnr);
        t2 = check_objtypes(t2, shpnr);
        t3 = check_objtypes(t3, shpnr);
        t4 = check_objtypes(t4, shpnr);
        t5 = check_objtypes(t5, shpnr);

        if (t1 > 0)
            fprintf(fp, "%d\n", t1);
        if (t2 > 0)
            fprintf(fp, "%d\n", t2);
        if (t3 > 0)
            fprintf(fp, "%d\n", t3);
        if (t4 > 0)
            fprintf(fp, "%d\n", t4);
        if (t5 > 0)
            fprintf(fp, "%d\n", t5);
        fprintf(fp, "-1\n");
        fprintf(fp, "%%s Haven't got that on storage - try list!~\n");
        fprintf(fp, "%%s You don't seem to have that.~\n");
        fprintf(fp, "%%s I don't buy THAT... Try another shop.~\n");
        fprintf(fp, "%%s I can't afford such a thing.~\n");
        fprintf(fp, "%%s You can't afford it!~\n");
        fprintf(fp, "%%s That'll be %%d coins, please.~\n");
        fprintf(fp, "%%s You get %%d coins for it!~\n");
        fprintf(fp, "0\n");
        fprintf(fp, "0\n");
        fprintf(fp, "%d\n", mob);
        fprintf(fp, "0\n");
        fprintf(fp, "%d\n", room);
        fprintf(fp, "-1\n");
        fprintf(fp, "%d\n", op);
        fprintf(fp, "%d\n", clo);
        fprintf(fp, "0\n");
        fprintf(fp, "0\n");
        get_line(fl, ch);
    }
    if (open) {
        fprintf(fp, "$~\n");
        fclose(fp);
        remove13(fname);
    } else
        fprintf(erf1, "No shops here.\n");
}

void            parse_helps1(FILE * fl)
{
    char            fname[10];
    char            ch[65536];
    char           *help;
    FILE           *fp;

    sprintf(fname, "%d.hlp", zonenum);
    fp = fopen(fname, "w");

    get_line(fl, ch);
    fprintf(fp, "%s\n", ch);

    help = fread_string(fl, ch);
    fprintf(fp, "%s\n", help);
    free(help);

    fclose(fp);

    remove13(fname);
}

void parse_helps( FILE *fl )
{
	 char            fname[10];
    int i;
    char            ch[65536];
    char           *help;
    FILE           *fp;

    
    sprintf(fname, "%d.hlp", zonenum);
    fp = fopen(fname, "w");
    
    for ( ; ; )
    {
	
	i	= fread_number( fl );
	help	= fread_string( fl, ch );
	if ( help[0] == '$' )
	{	    
		free(help);
	    break;
	}                            
	fprintf(fp, "#\n%s\n\n", help);
	free(help);
	
	help	= fread_string( fl, ch );
	
	if (help[0] == '\0' )
	{
	    free (help);
	    continue;
	}	
	fprintf(fp, "%s\n", help);
	free (help);
    }    
    
    fclose(fp);

    remove13(fname);
}




char            spec_buf[100];
char           *check_spec(char *spec)
{
    int             i;

    for (i = 0; i < NUM_SPEC; i += 2)
        if (!strcmp(spec_names[i], spec))
            return spec_names[i + 1];
    return spec;
}
void            parse_specials(FILE * fl)
{
    char            fname[10];
    char            ch[32768],
                    tpe,
                    spec[256];
    FILE           *fp;
    int             nr;

    sprintf(fname, "%d.spe", zonenum);
    fp = fopen(fname, "w");

    for (;;) {
        get_line(fl, ch);
        if (*ch != 'S') {
            if (sscanf(ch, " %c %d %s ", &tpe, &nr, spec) != 3) {
                fprintf(erf1, "Error in #SPECIALS section.\n");
                exit(1);
            } else {
                switch (tpe) {
                case 'M':
                    fprintf(fp, "  ASSIGNMOB (%d, %s);\n", change_number(nr), check_spec(spec));
                    break;
                case '*':
                    break;
                }
            }
        } else {
            fclose(fp);
            remove13(fname);
            return;
        }
    }
}

char            author[100];
char            resetmsg[100];


void            parse_author(char *buf)
{
    strncpy(author, buf, strlen(buf) - 1);
}

void            parse_resetmsg(char *buf)
{
    strncpy(resetmsg, buf, strlen(buf) - 1);
}

void            parse_resets(FILE * fl)
{
    char            fname[10];
    char            ch[32768],
                    c,
                    s[256],
                   *i;
    int             t1,
                    t2,
                    t3,
                    t4;
    FILE           *fp;

    maxroom += 99 - (maxroom % 100);

    sprintf(fname, "%d.zon", zonenum);
    fp = fopen(fname, "w");

    fprintf(fp, "#%d\n", zonenum);
    fprintf(fp, "%s\n", zonename);
    fprintf(fp, "%d 35 2\n\n", maxroom);
    if (*author)
        fprintf(fp, "#AUTHOR %s~\n\n", author);
    if (*resetmsg)
        fprintf(fp, "#RESETMSG %s~\n\n", resetmsg);


    for (;;) {
        get_line(fl, ch);
        if (*ch != 'S') {
            if (*ch != 'C' && *ch != 'R' && *ch != 'F') {
                {
                    if (sscanf(ch, "%c %d %d %d %d ", &c, &t1, &t2, &t3, &t4) != 5) {
                        if (sscanf(ch, "%c %d %d %d ", &c, &t1, &t2, &t3) == 4) {
                            sprintf(s, "%d", t3);
                            i = strstr(ch, s);
                            while (*i == ' ' || isdigit(*i))
                                i++;
                            t2 = change_number(t2);
                            if (t1 <= 1 || nochance) {
                                if (c == 'M' || c == 'D' || c == 'O')
                                    t1 = 0;
                                else
                                    t1 = 1;
                            } else if (warn)
                                fprintf(erf1, "NOTICE: Chance? %s\n", ch);

                            fprintf(fp, "%c %d %d %d %s\n", c, t1, t2, t3, i);
                        } else {
                            fprintf(fp, "%s\n", ch);
                        }
                    } else {
                        sprintf(s, "%d", t4);
                        i = strstr(ch, s);
                        while (*i == ' ' || isdigit(*i))
                            i++;
                        t2 = change_number(t2);
                        if (c != 'E' && c != 'D')
                            t4 = change_number(t4);
                        else if (c == 'E')
                            t4 = check_rstwears(t4);
                        if (t1 <= 1 || nochance) {
                            if (c == 'M' || c == 'D' || c == 'O')
                                t1 = 0;
                            else
                                t1 = 1;
                        } else if (warn)
                            fprintf(erf1, "NOTICE: Chance? %s\n", ch);
                        fprintf(fp, "%c %d %d %d %d %s\n", c, t1, t2, t3, t4, i);
                    }
                }
            } else {
                if (warn)
                    fprintf(erf1, "Unsupported Reset command : %s\n", ch);
            }
        } else {
            fprintf(fp, "S\n");
            fprintf(fp, "$~\n");
            fclose(fp);
            remove13(fname);
            return;
        }
    }
}


void            parse_objects(FILE * fl)
{
    char            fname[10];
    char            ch[32768],
                    s10[200],
                    s20[200],
                    s30[200],
                    s40[200],
                    s[256];
    char           *point,
                   *s1,
                   *s2,
                   *s3,
                   *s4;
    int             t1,
                    t2,
                    t3,
                    t4;
    int             tt;
    int             num,
                    found = -1,
                    numaffs = 0;
    int             flgs,
                    wears,
                    tpe,
                    totdam;
    FILE           *fp;
    int             mpn = 0,
                    mp = 0;

    sprintf(fname, "%d.obj", zonenum);
    fp = fopen(fname, "w");
    get_line(fl, ch);
    for (;;) {
        if (!strcmp(ch, "#0"))
            break;
        if (sscanf(ch, "#%d ", &num) == 1) {
            numaffs = 0;
            if (nznr >= 0)
                num = change_number(num);
            fprintf(fp, "#%d\n", num);
            s1 = fread_string(fl, s);
            s2 = fread_string(fl, s);
            s3 = fread_string(fl, s);
            s4 = fread_string(fl, s);
            fprintf(fp, "%s~\n%s~\n%s~\n", s1, s2, s3);
            if (s4 == NULL)
                fprintf(fp, "~\n");
            else
                fprintf(fp, "%s~\n", s4);

            get_line(fl, ch);
            if (sscanf(ch, "%d %d %d %d", &tpe, &flgs, &wears, &tt) != 4) {
                if (sscanf(ch, "%d %d %d", &tpe, &flgs, &wears) != 3) {
                    fprintf(erf1, "Error at obj %d 2nd numeric line: %s\n", num, ch);
                    exit(1);
                }
            }
            /* tpe = fread_number (fl); flgs = fread_number (fl); wears =
             * fread_number (fl); */
            wears = check_objwears(wears, num);
            tpe = check_objtypes(tpe, num);
            flgs = check_objflags(flgs, num);
            if (!cifre)
                fprintf(fp, "%d %s %s\n", tpe, get_ascii(flgs), get_ascii(wears));
            else
                fprintf(fp, "%d %d %d\n", tpe, (flgs), (wears));
            get_line(fl, ch);
            if (sscanf(ch, "%s %s %s %s", s10, s20, s30, s40) != 4) {
                fprintf(erf1, "Error at obj %d 2nd numeric line: %s\n", num, ch);
                exit(1);
            }
            if ((point = strchr(s10, '~')) != NULL)
                *point = '\0';
            t1 = atoi(s10);
            if ((point = strchr(s20, '~')) != NULL)
                *point = '\0';
            t2 = atoi(s20);
            if ((point = strchr(s30, '~')) != NULL)
                *point = '\0';
            t3 = atoi(s30);
            if ((point = strchr(s40, '~')) != NULL)
                *point = '\0';
            t4 = atoi(s40);

            check_objvalues(&t1, &t2, &t3, &t4, num, nznr, tpe);
            fprintf(fp, "%d %d %d %d\n", t1, t2, t3, t4);
            get_line(fl, ch);
            if (sscanf(ch, " %d %d %d ", &t1, &t2, &t3) != 3) {
                fprintf(erf1, "Error at obj %d 3rd numeric line.\n", num);
                exit(1);
            }
            fprintf(fp, "%d %d %d\n", t1, t2, t3);
            found = -1;
            while (found == -1) {
                get_line(fl, ch);
                switch (*ch) {
                case '#':
                    found = 0;
                    break;
                case 'A':
                    numaffs++;
                    if (sscanf(ch, "A %d %d ", &t1, &t2) != 2) {
                        get_line(fl, ch);
                        if (sscanf(ch, " %d %d ", &t1, &t2) != 2) {
                            fprintf(erf1, "Error at obj %d in Affections.\n", num);
                            exit(1);
                        }
                    }
                    if (numaffs <= MAX_OBJ_AFFS) {

                        if (!check_objapply(fp, t1, t2, num))
                            numaffs--;


                    } else {
                        if (warn)
                            fprintf(erf1, "Object %d has to many AFFECTIONS!!!.\n", num);
                    }
                    break;
                case 'E':
                    fprintf(fp, "E\n");
                    s1 = fread_string(fl, s);
                    s2 = fread_string(fl, s);
                    fprintf(fp, "%s~\n", s1);
                    fprintf(fp, "%s~\n", s2);
                    break;
                case '>':
                    fprintf(fp, ">\n%s\n", trim(ch + 1));
                    mpn++;
                    while (*ch != '|') {
                        get_line(fl, ch);
                        if (*ch == '>') {
                            fprintf(fp, ">\n%s\n", trim(ch + 1));
                            mpn++;
                        } else {
                            check_prog(ch);
                            fprintf(fp, "%s\n", ch);
                        }
                    }
                    mp = 0;
                    if (warn)
                        fprintf(prg_erf1, "OBJ %d has %d objprogs assigned.\n", num, mpn);
                    mpn = 0;
                    break;
                }
            }
        }
    }
    fprintf(fp, "$~\n");
    fclose(fp);
    remove13(fname);
}

void            parse_rooms(FILE * fl)
{
    char            fname[10];
    char            ch[32768],
                   *s1,
                   *s2,
                    s[256];
    int             t1,
                    t2,
                    t3,
                    znr,
                    stpe;
    int             num,
                    found = -1;
    int             flgs;
    FILE           *fp;
    int             mp = 0,
                    mpn = 0;
    sprintf(fname, "%d.wld", zonenum);
    fp = fopen(fname, "w");
    get_line(fl, ch);
    for (;;) {
        if (!strcmp(ch, "#0"))
            break;
        if (sscanf(ch, "#%d ", &num) == 1) {
            if (nznr >= 0)
                num = change_number(num);
            fprintf(fp, "#%d\n", num);
            maxroom = num;
            s1 = fread_string(fl, s);
            s2 = fread_string(fl, s);
            fprintf(fp, "%s~\n%s~\n", s1, s2 ? s2 : "");
            znr = fread_number(fl);
            flgs = fread_number(fl);
            stpe = fread_number(fl);
            znr = zonenum;
            flgs = check_roomflags(flgs);
            stpe = check_sectors(stpe);
            if (!cifre)
                fprintf(fp, "%d %s %d\n", znr, get_ascii(flgs), stpe);
            else
                fprintf(fp, "%d %d %d\n", znr, (flgs), stpe);
            found = -1;
            while (found == -1) {
                get_line(fl, ch);
                switch (*ch) {
                case 'S':
                    found = 0;
                    fprintf(fp, "S\n");
                    get_line(fl, ch);
                    break;
                case 'D':
                    fprintf(fp, "%s\n", ch);
                    s1 = fread_string(fl, s);
                    s2 = fread_string(fl, s);
                    if (s1 == NULL)
                        fprintf(fp, "~\n");
                    else
                        fprintf(fp, "%s~\n", s1);
                    if (s2 == NULL)
                        fprintf(fp, "~\n");
                    else
                        fprintf(fp, "%s~\n", s2);
                    get_line(fl, ch);
                    if (sscanf(ch, " %d %d %d ", &t1, &t2, &t3) != 3) {
                        fprintf(erf1, "At room %d error in Directional Data.\n", num);
                        exit(1);
                    }
                    if (t3 < minznr || t3 > maxznr)
                        if (warn)
                            fprintf(erf1, "NOTICE: Room %d, exit leads out of zone (to room %d)\n", num, t3);
                    if (nznr >= 0 && t3 > 0)
                        t3 = change_number(t3);
                    if (t2 > 0)
                        t2 = change_number(t2);
                    t1 = check_exitflags(t1);
                    fprintf(fp, "%d %d %d\n", t1, t2, t3);
                    break;
                case 'E':
                    s1 = fread_string(fl, s);
                    s2 = fread_string(fl, s);
                    fprintf(fp, "E\n");
                    fprintf(fp, "%s~\n", s1);
                    fprintf(fp, "%s~\n", s2);
                    break;
                case '>':
                    fprintf(fp, ">\n%s\n", trim(ch + 1));
                    mpn++;
                    while (*ch != '|') {
                        get_line(fl, ch);
                        if (*ch == '>') {
                            fprintf(fp, ">\n%s\n", trim(ch + 1));
                            mpn++;
                        } else {
                            check_prog(ch);
                            fprintf(fp, "%s\n", ch);
                        }

                    }
                    mp = 0;
                    if (warn)
                        fprintf(prg_erf1, "ROOM %d has %d roomprogs assigned.\n", num, mpn);
                    mpn = 0;
                    break;
                }
            }
        }
    }
    fprintf(fp, "$~\n");
    fclose(fp);
    remove13(fname);
}

void            count_rooms(FILE * fl)
{
    char            ch[32768],
                   *s1,
                   *s2,
                    s[256];
    int             znr,
                    stpe;
    int             num,
                    found = -1;
    int             flgs;

    while (!feof(fl)) {
        get_line(fl, ch);
        if (!feof(fl)) {
            if (strstr(ch, "#ROOMS"))
                break;
        }
    }

    if (feof(fl)) {
        fprintf(erf1, "Could not find the #ROOMS section.\n");
        exit(1);
    }
    get_line(fl, ch);
    for (;;) {
        if (!strcmp(ch, "#0"))
            break;
        if (sscanf(ch, "#%d ", &num) == 1) {
            if (num < minznr)
                minznr = num;
            maxznr = num;
            s1 = fread_string(fl, s);  
            strcpy(s, s1);
            s2 = fread_string(fl, s);
            znr = fread_number(fl);
            flgs = fread_number(fl);
            stpe = fread_number(fl);
            found = -1;
            while (found == -1) {
                get_line(fl, ch);
                switch (*ch) {
                case 'S':
                    found = 0;
                    get_line(fl, ch);
                    break;
                case 'D':
                    s1 = fread_string(fl, s);
                    s2 = fread_string(fl, s);
                    get_line(fl, ch);
                    break;
                case 'E':
                    s1 = fread_string(fl, s);
                    s2 = fread_string(fl, s);
                    break;
                }
            }
        }
    }
    maxznr += 99 - (maxznr % 100);
}

int             check_race(int race, int num)
{
    switch (race) {
 case 0:
 case 1:
 case 8:
        break;
    case 2:
        race = 3;
        break;
    case 3:
        race = 4;
        break;
    case 6:
        race = 7;
        break;
    case 7:
        race = 9;
        break;
    case 11:
        race = 6;
        break;
    case 12:
    case 9:
        race = 1;
        break;
    case 14:
        race = 5;
        break;
    case 39:
    case 43:
        race = 2;
        break;
    case 52:
    case 61:
        race = 20;
        break;
    case 53:
        race = 9;
        break;
    case 69:
    case 76:
    case 79:
    case 24:
    case 86:
    case 27:
    case 87:
    case 21:
        race = 15;
        break;
    case 81:
    case 30:
        race = 22;
        break;
    case 42:
    case 23:
        race = 23;
        break;    
    case 90:
        race = 21;
        break;
    case 67:
    case 71:
    case 36:
    case 60:
        race = 13;
        break;
    case 63:
    case 83:                    // insect
        race = 17;
        break;
    case 31:
        race = 11;
        break;
    case 89:
        race = 12;
        break;
    case 85:
        race = 19;
        break;
    default:

        if (warn)
            fprintf(erf1, "Mob %d has race set to %d (%s)\n", num, race, npc_race[race]);
        race = 12;
        break;

    }
    return race;
}
char           *klase[] =
{
    "m",
    "c",
    "t",
    "w",
    "VAMPIRE",
    "DRUID",
    "RANGER",
    "AUGURER",
    "PALADIN",
    "NEPHANDI",
    "SAVAGE"
};


int             check_class(int race, int num)
{

    switch (race) {
 case 0:
 case 1:
 case 2:
 case 3:
        break;
    case 5:
        if (warn)
            fprintf(erf1, "Mob %d has class set to %s\n", num, klase[race]);
        race = 1;
        break;
    case 6:
        if (warn)
            fprintf(erf1, "Mob %d has class set to %s\n", num, klase[race]);
        race = 3;
        break;

    default:

        if (warn && race <= 10)
            fprintf(erf1, "Mob %d has class set to %s\n", num, klase[race]);
        else
            fprintf(erf1, "Mob %d has class set to %d\n", num, race);
        race = 3;
        break;

    }
    return race;
}


void            parse_mobiles(FILE * fl)
{
    char            fname[10];
    char            ch[32768],
                   *s1,
                   *s2,
                   *s3,
                   *s4,
                    s[256],
                    mtpe;
    int             t1,
                    t2,
                    t3,
                    t4,
                    t5,
                    t6,
                    t7,
                    t8,
                    t9;
    int             num;
    int             flgs,
                    affs,
                    algn,
                    mgold,
                    mexp;
    FILE           *fp;
    int             mp = 0,
                    mpn = 0;
    char            buff[1000];
    sprintf(fname, "%d.mob", zonenum);
    fp = fopen(fname, "w");
    for (;;) {
        get_line(fl, ch);
        if (!strcmp(ch, "#0"))
            break;
        if (sscanf(ch, "#%d ", &num) == 1) {
            if (nznr >= 0)
                num = change_number(num);
            fprintf(fp, "#%d\n", num);
            s1 = fread_string(fl, s);
            s2 = fread_string(fl, s);
            s3 = fread_string(fl, s);
            s4 = fread_string(fl, s);
            fprintf(fp, "%s~\n%s~\n%s~\n%s~\n", s1, s2, s3, s4 ? s4 : "");
            flgs = fread_number(fl);
            affs = fread_number(fl);
            flgs = check_mobflags(flgs, num);
            affs = check_mobaffs(affs, num);
            get_line(fl, ch);
            if (sscanf(ch, " %d %c ", &algn, &mtpe) != 2) {
                fprintf(erf1, "Error at mob %d 1st numeric line.\n", num);
                exit(1);
            }
            if (!cifre)
                fprintf(fp, "%s %s %d %c\n", get_ascii(flgs), get_ascii(affs), algn, mtpe);
            else
                fprintf(fp, "%d %d %d %c\n", (flgs), (affs), algn, mtpe);
            get_line(fl, ch);
            if (sscanf(ch, " %d %d %d %dd%d+%d %dd%d+%d", &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9) != 9) {
                fprintf(erf1, "Error at mob %d 2nd numeric line.\n", num);
                exit(1);
            }
            get_line(fl, ch);
            if (sscanf(ch, " %d %d ", &mgold, &mexp) != 2) {
                fprintf(erf1, "Error at mob %d 3rd numeric line.\n", num);
                exit(1);
            }
            // calc_mob_vals(num,t1,&t2,&t3,&t4,&t5,&t6,&t7,&t8,&t9,&mgold,&me
            // xp);
            fprintf(fp, "%d %d %d %dd%d+%d %dd%d+%d\n", UMAX(1, t1 + level_add), t2, t3, t4, t5, t6, t7, t8, t9);
            fprintf(fp, "%d %d\n", mgold, mexp);
            get_line(fl, ch);
            if (sscanf(ch, " %d %d %d ", &t1, &t2, &t3) != 3) {
                fprintf(erf1, "Error at mob %d 4th numeric line.\n", num);
                exit(1);
            }
            if (t1 == 0 || t1 == 7)
                t1 = 8;
            if (t1 == 9) {
                t1 = 8;
                if (warn)
                    fprintf(erf1, "NOTICE: Mob %d is mounted. Setting to standing.\n", num);
            }
            if (t1 > 9) {
                t1 = 8;
                if (warn)
                    fprintf(erf1, "Mob %d has unusual default position. Setting to standing.\n", num);
            }
            if (t2 == 0 || t2 == 7)
                t2 = 8;
            if (t2 == 9) {
                t2 = 8;
                if (warn)
                    fprintf(erf1, "NOTICE: Mob %d is mounted. Setting to standing.\n", num);
            }
            if (t2 > 9) {
                t2 = 8;
                if (warn)
                    fprintf(erf1, "Mob %d has unusual default position. Setting to standing.\n", num);
            }
            fprintf(fp, "%d %d %d\n", t1, t1, t3);
            if (mtpe == 'C') {
                get_line(fl, ch);
                fprintf(fp, "%s\n", ch);
                get_line(fl, ch);   // sav0...
                // fprintf(fp, "%s\n", ch);
                get_line(fl, ch);
                if (sscanf(ch, " %d %d %d %d %d %d %d", &t1, &t2, &t3, &t4, &t5, &t6, &t7) != 7) {
                    fprintf(erf1, "Error at mob %d race-class line.\n", num);
                    exit(1);
                }
                t1 = check_race(t1, num);
                t2 = check_class(t2, num);
                fprintf(fp, "%d %d %d\nC\n", t1, t2, t7);
                get_line(fl, ch);

            }
        } else if ((*ch == '>') || mp) {
            if (*ch == '>') {
                mp = 1;
                mpn++;
            };
            if (*ch == '|') {
                mp = 0;
                if (warn)
                    fprintf(prg_erf1, "MOB %d has %d mobprogs assigned.\n", num, mpn);
                mpn = 0;
            }
            check_prog(ch);
            fprintf(fp, "%s\n", ch);
        }
    }
    fprintf(fp, "$~\n");
    fclose(fp);
    remove13(fname);
}

int             main(int argc, char *argv[])
{
    FILE           *fl;
    int             arg;
    char            ch[32768];
    char            ch1[32768];

    if (argc < 2) {
        printf("Illegal number of arguments.\n");
        exit(1);
    }
    for (arg = 1; arg < argc; arg++) {
        if (*(argv[arg]) == '-') {
            switch (*(argv[arg] + 1)) {
            case 'c':
                cifre = TRUE;
                break;
            case 'n':
                strcpy(ch, (argv[arg] + 2));
                nznr = atoi(ch);
                if (nznr < 1 || nznr > 500) {
                    printf("Invalid val for new zonenr. Range is 1 - 320.\n");
                    exit(1);
                }
                printf("Renumbering to zone vnum %d.\n", nznr);
//          fprintf(erf1,"%d\n",nznr);
                break;
            case 't':
                strcpy(ch, (argv[arg] + 2));
                if (!strcmp(ch, "max")) {
                    mobconv = MAXVALS;
                } else if (!strcmp(ch, "mid")) {
                    mobconv = MIDVALS;
                } else if (!strcmp(ch, "merc")) {
                    mobconv = MERCVALS;
                } else {
                    printf("Invalid type for mobconversion. Valid types are mid,max and merc.\n");
                    exit(1);
                }
                break;
            case 's':
                randspells = TRUE;
                break;
            case 'p':
                nochance = 1;
                break;
            case 'w':
                warn = FALSE;
                break;
            case 'r':
                strcpy(ch, (argv[arg] + 2));
                maxrand = atoi(ch);
                if (maxrand <= 1) {
                    printf("Value for randomization must be > 1.\n");
                    exit(1);
                }
                break;
            case 'l':
                strcpy(ch, (argv[arg] + 2));
                level_add = atoi(ch);
                printf("Adding %d to mob levels.\n", level_add);
                break;
            case 'h':
                printf("Usage: s2c [options] smaugfile.are\n\n");
                printf("The following options are valid.\n");
                printf("-n<val> Renumbers zone to new val.\n");
                printf("-c      Don't write ascii flags.\n");
                printf("-w      Display no warnings.\n");
                printf("-s      Randomize level and charges if 0.\n");
                printf("-p      No chance percent to zone resets.\n");
                printf("-l<val> Add <val> to mob levels.\n");
                printf("-r<val> Randomize Weapons to give a max dam of val.\n");
                printf("\n");
                printf("-t<max|mid|merc> How to calculate the Stats for mobs as in\n");
                printf("                 Merc areas these are often set to 0.\n");
                printf("                 The calculations are taken from Merc's source.\n");
                printf("        Max - Sets mobs to max values.\n");
                printf("        Mid - Sets mobs to average values.\n");
                printf("        Merc - Sets mobs to values generated in the same way\n");
                printf("               as they are on Merc. In circle these are permanent.\n");
                exit(0);
            default:
                printf("Illegal option %s.\n", argv[arg]);
                break;
            }
        } else
            break;
    }
    *author = *resetmsg = 0;
    fl = fopen(argv[arg], "r");
    if (fl == NULL) {
        printf("Could not open file.\n");
        exit(1);
    }
    zonenum = 0;
    while (zonenum == 0) {
        get_line(fl, ch);
        if (sscanf(ch, "#%d", &zonenum))
            if (zonenum > 0)
                break;
    }
    fclose(fl);
    fl = fopen(argv[arg], "r");
    count_rooms(fl);
    fclose(fl);


    zonenum = zonenum / 100;
    znum = zonenum;

    if (nznr >= 0) {
        oznr = zonenum;
        zonenum = nznr;
    }
    fl = fopen(argv[arg], "r");
    if (fl == NULL) {
        printf("Could not open file.\n\n");
        exit(1);
    }
    printf("Zone VNUM is %d.\n", zonenum);

    {
        char            aa[10];
        sprintf(aa, "%d.err", zonenum);
        erf1 = fopen(aa, "w");
        sprintf(aa, "%d_prg.err", zonenum);
        prg_erf1 = fopen(aa, "w");
    }
    init_mm();
    while (!feof(fl)) {
        get_line(fl, ch);
        if (!feof(fl))
            if (sscanf(ch, "#AREA %s~", ch1) == 1) {
                strcpy(zonename, ch + 6);
                // skip_spaces(zonename);
                trim(zonename);
                fprintf(erf1, "Zone %d - %s \n", zonenum, zonename);
            }
        if (!strncmp("#AUTHOR", ch, 7)) {
            parse_author(ch + 8);
        }
        else if (!strncmp("#RESETMSG", ch, 9)) {
            parse_resetmsg(ch + 10);
        }
        else if (!strncmp("#ROOMS", ch, 6)) {
            parse_rooms(fl);
            if (nor+nos>40)                                
            	fprintf(erf1, "WARNING: Possible NORECALL+NOSUMMON area. \n");
            fprintf(erf1, "Finished Converting Rooms.\n\n");
            
            
        }
        else if (!strncmp("#OBJECTS", ch, 8)) {
            parse_objects(fl);
            fprintf(erf1, "Finished Converting Objects.\n\n");
        }
        else if (!strncmp("#MOBILES", ch, 8)) {
            parse_mobiles(fl);
            fprintf(erf1, "Finished Converting Mobs.\n\n");
        }
        else if (!strcmp("#MOBPROGS", ch)) {
            fprintf(erf1, "MOBPROGS section exists!\n\n");
        }
        else if (!strncmp("#RESETS", ch,7)) {
            parse_resets(fl);
            fprintf(erf1, "Finished Converting Resets.\n\n");
        }
        else if (!strncmp("#SHOPS", ch,6)) {
            parse_shops(fl);
            fprintf(erf1, "Finished Converting Shops.\n\n");
        }
        else if (!strncmp("#SPECIALS", ch, 9)) {
            parse_specials(fl);
            fprintf(erf1, "Finished Converting Specials.\n\n");
        }
        else if (!strcmp("#HELPS", ch)) {
            parse_helps(fl);
            fprintf(erf1, "Finished Copying Help.\n\n");
        }
    }


    fclose(fl);
    fclose(erf1);

    return zonenum;
}
