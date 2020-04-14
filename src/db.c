/* ************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __DB_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "structs.h"
#include "rooms.h"
#include "objs.h"
#include "class.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "house.h"
#include "chatlink.h"
#include "arena.h"
#include "ego.h"

char *ends="$end~";
#define MAX_MOBS_PER_ROOM 10
#define MAX_OBJS_PER_ROOM 10

int total_ram=0;
int global_lwg=0;
void            auction_reset(void);
void            mprog_read_programs(FILE * fp, struct index_data * pMobIndex);
void            oprog_read_programs(FILE * fp, struct index_data * pObjIndex);
void            rprog_read_programs(FILE * fp, ROOM_INDEX_DATA * pRoomIndex);

char            err_buf[MAX_STRING_LENGTH];
extern unsigned int pulse;
/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */
struct timeval curr_time1;

struct room_data *world = NULL; /* array of rooms		 */
int             top_of_world = 0;       /* ref to top element of world	 */

struct char_data *character_list = NULL;        /* global linked list of
                                                 * chars	 */
struct index_data *mob_index;   /* index table for mobile file	 */
struct char_data *mob_proto;    /* prototypes for mobs		 */
int             top_of_mobt = 0;/* top of mobile index table	 */

struct obj_data *object_list = NULL;    /* global linked list of objs	 */
struct index_data *obj_index;   /* index table for object file	 */
struct obj_data *obj_proto;     /* prototypes for objs		 */
int             top_of_objt = 0;/* top of object index table	 */

extern char *spells[];
extern int      max_new_objects;
extern int      max_new_rooms;
extern int      max_new_zones;
extern int      max_new_zcmds;
extern int      max_new_mobs;
SPECIAL(shop_keeper);
struct zone_data *zone_table;   /* zone table			 */
int             top_of_zone_table = 0;  /* top element of zone tab	 */
struct message_list fight_messages[MAX_MESSAGES];       /* fighting messages	 */

struct player_index_element *player_table = NULL;       /* index to plr file	 */
FILE           *player_fl = NULL;       /* file desc of player file	 */
int             top_of_p_table = 0;     /* ref to top of table		 */
int             top_of_p_file = 0;      /* ref of size of p file	 */
long            top_idnum = 0;  /* highest idnum in use		 */

int             no_mail = 0;    /* mail disabled?		 */
int             mini_mud = 0;   /* mini-mud mode?		 */
int             no_rent_check = 0;      /* skip rent check on boot?	 */
time_t          boot_time = 0;  /* time of mud boot		 */
time_t          boot_time_real = 0;  /* time of mud boot		 */
int             restrict1 = 0;   /* level of game restriction	 */
sh_int          r_mortal_start_room;    /* rnum of mortal start room	 */
sh_int          r_immort_start_room;    /* rnum of immort start room	 */
sh_int          r_frozen_start_room;    /* rnum of frozen start room	 */
int current_zone_num=-1;

char           *buglist = NULL; /* bug list 			 */
char           *idealist = NULL;/* idea list 			 */
char           *typolist = NULL;/* typo list 			 */
char           *credits = NULL; /* game credits			 */
char           *news = NULL;    /* mud news			 */
char           *motd = NULL;    /* message of the day - mortals */
char           *imotd = NULL;   /* message of the day - immorts */
char           *help = NULL;    /* help screen			 */
char           *info = NULL;    /* info page			 */
char           *wizlist = NULL; /* list of higher gods		 */
char           *immlist = NULL; /* list of peon gods		 */
char           *background = NULL;      /* background story		 */
char           *handbook = NULL;/* handbook for new immortals	 */
char           *policies = NULL;/* policies page		 */

FILE           *help_fl = NULL; /* file for help text		 */
struct help_index_element *help_index = 0;      /* the help table	 */
int             top_of_helpt;   /* top of help index table	 */
struct help_index_element *help_index_skills = 0;      /* the skills help table	 */
int             top_of_helpt_skills;   /* top of help index table	 */

char * helps[1000];
char * helps_skills[1000];
int acmin=100;
struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;       /* the infomation about the weather */
struct player_special_data dummy_mob;   /* dummy spec area for mobs	 */
struct reset_q_type reset_q;    /* queue of zones to be reset	 */

/* local functions */
void            setup_dir(FILE * fl, int room, int dir);
void            index_boot(int mode);
void            discrete_load(FILE * fl, int mode, int area);
void            parse_room(FILE * fl, int virtual_nr);
void            parse_mobile(FILE * mob_f, int nr);
char           *parse_object(FILE * obj_f, int nr);
void            load_zones(FILE * fl, char *zonename, int area);
void            assign_mobiles(void);
void            assign_objects(void);
void            assign_rooms(void);
void            assign_the_shopkeepers(void);
void            build_player_index(void);
void            char_to_store(struct char_data * ch, struct char_file_u * st);
void            store_to_char(struct char_file_u * st, struct char_data * ch);
int             is_empty(int zone_nr);
void            reset_zone(int zone);
int             file_to_string(char *name, char *buf);
int             file_to_string_alloc(char *name, char **buf);
void            check_start_rooms(void);
void            renum_world(void);
void            renum_zone_table(void);
void            log_zone_error(int zone, int cmd_no, char *message);
void            reset_time(void);
void            clear_char(struct char_data * ch);
int             read_file_to_32strings_alloc(char *filename, char **buf);
int             read_file_to_32strings(char *filename, char **buf);

/* external functions */
extern struct descriptor_data *descriptor_list;
void            load_messages(void);
void            weather_and_time(int mode);
void            mag_assign_spells(void);
void            boot_social_messages(void);
void            update_obj_file(void);  /* In objsave.c */
void            sort_commands(void);
void            sort_spells(void);
void            load_banned(void);
void            Read_Invalid_List(void);
void            boot_the_shops(FILE * shop_f, char *filename, int rec_count, int area);
struct help_index_element *build_help_index(FILE * fl, int *num, char *helps[]);
/*int remove(char *filename);
int rename(char *filename, char *filename2);
LINUX !!!*/
void            load_hall_of_fame(void);
/* external vars */
extern int      no_specials;
struct raff_node *raff_list = NULL;


struct s_topdam topdam[10], tophurt[10];
struct s_topdam topdam_class[NUM_CLASSES][10], tophurt_class[NUM_CLASSES][10];

struct mob_kill_info mobkills[MAX_MOBKILL];

// just enable this for zone spawning errors
int boottime=0;

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

int find_zone_by_room(int vnum)
{
    int i;


    vnum/=100;
    vnum*=100;

    for (i = 0; i <= top_of_zone_table; i++)
        if ((zone_table[i].number * 100 <= vnum) && (zone_table[i].top >= vnum))
            return i;

    return -1;
}

void build_area_info()
{
    struct char_data *mob;
    int vnum, i, j, k;
    struct room_data *room;


    for (i = 0; i <= top_of_zone_table; i++)
    {
        area_info[i].avg=area_info[i].max=area_info[i].num=0;
        area_info[i].min=100;
        for (j=0;j<1000;j++)
        {
            area_info[i].connected_to[j]=0;
            area_info[i].connected_from[j]=0;
        }
    }
    for (mob=character_list; mob; mob=mob->next)
    {
        vnum=GET_MOB_VNUM(mob)/100;			// this is bonus for zones
        vnum*=100;

        for (i = 0; i <= top_of_zone_table; i++)
            if ((zone_table[i].number * 100 <= vnum) && (zone_table[i].top >= vnum))
            {
                area_info[i].num++;
                area_info[i].avg+=GET_LEVEL(mob);
                if (GET_LEVEL(mob)>area_info[i].max)
                    area_info[i].max=GET_LEVEL(mob);
                if (GET_LEVEL(mob)<area_info[i].min)
                    area_info[i].min=GET_LEVEL(mob);
                break;
            }
        if (i>top_of_zone_table)
            logs("ERROR: Mob %d outside any zone.", GET_MOB_VNUM(mob));
    }


    for (i=0;i<top_of_world;i++)
    {
        //k=find_zone_by_room(GET_ROOM_VNUM(i));
        k=world[i].zone;
        if (k==-1)
        {
            logs("ERROR: Room outside every zone (%d)", GET_ROOM_VNUM(i));
            continue;
        }
        for (j=0;j<NUM_OF_DIRS;j++)
            if (world[i].dir_option[j] && world[i].dir_option[j]->to_room!=NOWHERE)
            {
                if ((GET_ROOM_VNUM(i)/100) == (GET_ROOM_VNUM(world[i].dir_option[j]->to_room)/100))
                    continue;
                //vnum=find_zone_by_room(GET_ROOM_VNUM(world[i].dir_option[j]->to_room));
                vnum=world[world[i].dir_option[j]->to_room].zone;
                if (vnum==-1)
                {
                    logs("ERROR: Room outside every zone (%d)", GET_ROOM_VNUM(world[i].dir_option[j]->to_room));
                    continue;
                }
                if (k!=vnum)
                {
                    area_info[k].connected_to[vnum]++;
                    area_info[vnum].connected_from[k]++;
                }
                    
            }
    }


}









/* this is necessary for the autowiz system */
void            reboot_wizlists(void)
{
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
}

void log_ram()
{
    char trbuf[200];

    sprintf(trbuf, "Total ram so far:%d", total_ram);
    log(trbuf);
}



void read_mud_date_from_file(void)
{
    FILE *f;
    struct time_write read_date;

    f = fopen("etc/date_record", "rb");
    if(!f) {
        log("SYSERR: File etc/date_record not found, mud date will be reset to default!");
        return;
    }

    fread(&read_date, sizeof(struct time_write), 1, f);
    time_info.year = read_date.year;
    time_info.month = read_date.month;
    time_info.day   = read_date.day;
    fclose(f);
}


ACMD(do_reboot)
{
    int             i;

    one_argument(argument, arg);

    if (!str_cmp(arg, "all") || *arg == '*') {
        file_to_string_alloc(NEWS_FILE, &news);
        file_to_string_alloc(CREDITS_FILE, &credits);
        file_to_string_alloc(MOTD_FILE, &motd);
        file_to_string_alloc(IMOTD_FILE, &imotd);
        file_to_string_alloc(HELP_PAGE_FILE, &help);
        file_to_string_alloc(INFO_FILE, &info);
        file_to_string_alloc(WIZLIST_FILE, &wizlist);
        file_to_string_alloc(IMMLIST_FILE, &immlist);
        file_to_string_alloc(POLICIES_FILE, &policies);
        file_to_string_alloc(HANDBOOK_FILE, &handbook);
        file_to_string_alloc(BACKGROUND_FILE, &background);
        file_to_string_alloc(BUGLIST_FILE, &buglist);
        file_to_string_alloc(IDEALIST_FILE, &idealist);
        file_to_string_alloc(TYPOLIST_FILE, &typolist);
        /*    read_file_to_32strings_alloc(CHANNEL_FILE, channel_bits); */
    } else if (!str_cmp(arg, "wizlist"))
        file_to_string_alloc(WIZLIST_FILE, &wizlist);
    else if (!str_cmp(arg, "immlist"))
        file_to_string_alloc(IMMLIST_FILE, &immlist);
    else if (!str_cmp(arg, "news"))
        file_to_string_alloc(NEWS_FILE, &news);
    else if (!str_cmp(arg, "credits"))
        file_to_string_alloc(CREDITS_FILE, &credits);
    else if (!str_cmp(arg, "motd"))
        file_to_string_alloc(MOTD_FILE, &motd);
    else if (!str_cmp(arg, "imotd"))
        file_to_string_alloc(IMOTD_FILE, &imotd);
    else if (!str_cmp(arg, "help"))
        file_to_string_alloc(HELP_PAGE_FILE, &help);
    else if (!str_cmp(arg, "info"))
        file_to_string_alloc(INFO_FILE, &info);
    else if (!str_cmp(arg, "policy"))
        file_to_string_alloc(POLICIES_FILE, &policies);
    else if (!str_cmp(arg, "handbook"))
        file_to_string_alloc(HANDBOOK_FILE, &handbook);
    else if (!str_cmp(arg, "buglist"))
        file_to_string_alloc(BUGLIST_FILE, &buglist);
    else if (!str_cmp(arg, "idealist"))
        file_to_string_alloc(IDEALIST_FILE, &idealist);
    else if (!str_cmp(arg, "typolist"))
        file_to_string_alloc(TYPOLIST_FILE, &typolist);
    else if (!str_cmp(arg, "background"))
        file_to_string_alloc(BACKGROUND_FILE, &background);
    else if (!str_cmp(arg, "xhelp")) {
        if (help_fl)
            fclose(help_fl);
        if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
            return;
        else {
            for (i = 0; i < top_of_helpt; i++)
                DISPOSE(help_index[i].keyword);
            DISPOSE(help_index);
            help_index = build_help_index(help_fl, &top_of_helpt, helps);

            if (help_fl)
                fclose(help_fl);
            if ((help_fl = fopen(HELP_KWRD_FILE_SKILLS, "r")))
            {
                for (i = 0; i < top_of_helpt_skills; i++)
                    DISPOSE(help_index_skills[i].keyword);
                DISPOSE(help_index_skills);
                help_index_skills = build_help_index(help_fl, &top_of_helpt_skills, helps_skills);
            }
            if (help_fl)
                fclose(help_fl);
            help_fl = fopen(HELP_KWRD_FILE, "r");
        }
    } else {
        send_to_char("Unknown reboot option.\r\n", ch);
        return;
    }

    send_to_char(OK, ch);
}

void save_mobkills()
{
    FILE *f;
    f=fopen(MOBKILL_FILE, "wb");
    if (f)
    {
        fwrite(mobkills, sizeof(struct mob_kill_info), MAX_MOBKILL, f);
        fflush(f);
        fclose(f);
    }
    else log("SYSERR: Cannot open mobkill file for write\r\n");
}

void boot_mobkills()
{
    FILE *f;
    int i;
    f=fopen(MOBKILL_FILE, "rb");
    if (!f)
    {
        log("Database not existing. Creating new.");
        memset(mobkills, 0, MAX_MOBKILL*sizeof(struct mob_kill_info));
        save_mobkills();

    }
    else
    {
        fread(mobkills, sizeof(struct mob_kill_info), MAX_MOBKILL, f);
        fclose(f);

    }
    for (i=0;i<MAX_MOBKILL;i++)
        mobkills[i].rnum=65000;

}
void save_topdam()
{
    FILE *f;
    f=fopen(TOPDAM_FILE, "wb");
    if (f)
    {
        fwrite(topdam, sizeof(struct s_topdam), 10, f);
        fwrite(topdam_class, sizeof(struct s_topdam), NUM_CLASSES*10, f);
        fflush(f);
        fclose(f);
    }
    else log("SYSERR: Cannot open topdam file for write\r\n");
}
void save_tophurt()
{
    FILE *f;
    f=fopen(TOPHURT_FILE, "wb");
    if (f)
    {
        fwrite(tophurt, sizeof(struct s_topdam), 10, f);
        fwrite(tophurt_class, sizeof(struct s_topdam), NUM_CLASSES*10, f);
        fflush(f);
        fclose(f);
    }
    else log("SYSERR: Cannot open topdam file for write\r\n");
}

void boot_topdam()
{
    FILE *f;
    int i;
    f=fopen(TOPDAM_FILE, "rb");
    if (!f)
    {
        //log("Database not existing. Creating new.");
        memset(topdam, 0, 10*sizeof(struct s_topdam));
        memset(topdam_class, 0, NUM_CLASSES*10*sizeof(struct s_topdam));
        save_topdam();
    }
    else
    {
        fread(topdam, sizeof(struct s_topdam), 10, f);
        if (!feof(f))
            fread(topdam_class, sizeof(struct s_topdam), NUM_CLASSES*10, f);
        else
            memset(topdam_class, 0, NUM_CLASSES*10*sizeof(struct s_topdam));
        fclose(f);

    }

    f=fopen(TOPHURT_FILE, "rb");
    if (!f)
    {
        //log("Database not existing. Creating new.");
        memset(tophurt, 0, 10*sizeof(struct s_topdam));
        memset(tophurt_class, 0, NUM_CLASSES*10*sizeof(struct s_topdam));
        save_tophurt();
    }
    else
    {
        fread(tophurt, sizeof(struct s_topdam), 10, f);
        if (!feof(f))
            fread(tophurt_class, sizeof(struct s_topdam), NUM_CLASSES*10, f);
        else
            memset(tophurt_class, 0, NUM_CLASSES*10*sizeof(struct s_topdam));
        fclose(f);

    }
}

void check_add_topdam(char *ch, char *vict, int dam, int skill, int what, int kl)
{
    struct s_topdam *pom;
    int i, found=0, j;

    if (what==1)
        pom=topdam;
    else
        pom=tophurt;

    if (dam<pom[9].dam)
        goto dole;

    for (i=9;i>=0;i--)
    {
        if (*pom[i].ch && !strcmp(ch, pom[i].ch))
        {
            found=1;
            break;
        }
    }

    if (found)
    {
        if (dam<pom[i].dam)
            goto dole;
        for (j=i+1;j<10;j++)
            pom[j-1]=pom[j];

    }

    for (i=0;i<10;i++)
        if (dam>pom[i].dam)
        {
            for (j=9;j>i;j--)
                pom[j]=pom[j-1];
            strncpy(pom[i].ch, ch, 25);
            strncpy(pom[i].vict, vict, 25);
            pom[i].dam=dam;
            pom[i].skill=skill;
            break;
        }


    //adds to topdam class
dole: 
   found=0;
    if (what==1)
        pom=topdam_class[kl];
    else
        pom=tophurt_class[kl];

    if (dam<pom[9].dam)
        return;

    for (i=9;i>=0;i--)
    {
        if (*pom[i].ch && !strcmp(ch, pom[i].ch))
        {
            found=1;
            break;
        }
    }

    if (found)
    {
        if (dam<pom[i].dam)
            return;
        for (j=i+1;j<10;j++)
            pom[j-1]=pom[j];

    }

    for (i=0;i<10;i++)
        if (dam>pom[i].dam)
        {
            for (j=9;j>i;j--)
                pom[j]=pom[j-1];
            strncpy(pom[i].ch, ch, 25);
            strncpy(pom[i].vict, vict, 25);
            pom[i].dam=dam;
            pom[i].skill=skill;
            break;
        }
}






void            boot_world(void)
{
    /*  log("Loading arena hall of fame");
      load_hall_of_fame();*/

    //log("Init ego weapons.");
    //init_ego();

    log ("Loading Mob kill data.");
    boot_mobkills();

    log ("Loading topdam and tophurt data.");
    boot_topdam();

    //log_ram();
    log("Loading zone table.");
    index_boot(DB_BOOT_ZON);
    //log_ram();
    log("Loading rooms.");
    index_boot(DB_BOOT_WLD);

    log("Renumbering rooms.");
    renum_world();

    log("Checking start rooms.");
    check_start_rooms();
    //log_ram();

    log("Loading mobs and generating index.");
    index_boot(DB_BOOT_MOB);
    //log_ram();

    log("Loading objs and generating index.");
    index_boot(DB_BOOT_OBJ);
    //log_ram();

    init_supermob();
    log("Renumbering zone table.");
    renum_zone_table();

    if (!no_specials) {
        log("Loading shops.");
        index_boot(DB_BOOT_SHP);
    }
}


/* body of the booting system */
void            boot_db(void)
{
    int             i;
    extern int      no_specials;

    log("Boot db -- BEGIN.");
    total_ram=0;

    log("Resetting the game time:");
    reset_time();

    log("Reading news, credits, help, bground, info & motds.");
    file_to_string_alloc(NEWS_FILE, &news);
    file_to_string_alloc(CREDITS_FILE, &credits);
    file_to_string_alloc(MOTD_FILE, &motd);
    file_to_string_alloc(IMOTD_FILE, &imotd);
    file_to_string_alloc(HELP_PAGE_FILE, &help);
    file_to_string_alloc(INFO_FILE, &info);
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
    file_to_string_alloc(POLICIES_FILE, &policies);
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
    file_to_string_alloc(BACKGROUND_FILE, &background);
    file_to_string_alloc(BUGLIST_FILE, &buglist);
    file_to_string_alloc(TYPOLIST_FILE, &typolist);
    file_to_string_alloc(IDEALIST_FILE, &idealist);
    /*  read_file_to_32strings_alloc(CHANNEL_FILE, channel_bits); */

    log("Opening main help file.");
    if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
        log("   Could not open main help file.");
    else
    {
        help_index = build_help_index(help_fl, &top_of_helpt, helps);
        fclose(help_fl);
    }
    log("Opening skills/spells help file.");
    if (!(help_fl = fopen(HELP_KWRD_FILE_SKILLS, "r")))
        log("   Could not open skills/spells help file.");
    else
    {
        help_index_skills = build_help_index(help_fl, &top_of_helpt_skills, helps_skills);
        fclose(help_fl);
    }



    boot_world();
    //fprintf(stderr, "**************** %d *************", acmin);
    //log_ram();
    log("Generating player index.");
    build_player_index();

    log("Loading fight messages.");
    load_messages();

    log("Loading social messages.");
    boot_social_messages();

    log("Loading Eliza data.");
    //startchat(CHAT_FILE);

    log("Booting clans.");
    init_clans();

    log("Assigning function pointers:");

    if (!no_specials) {
        log("   Mobiles.");
        assign_mobiles();
        log("   Shopkeepers.");
        assign_the_shopkeepers();
        log("   Objects.");
        assign_objects();
        log("   Rooms.");
        assign_rooms();
    }
    log("   Spells.");
    mag_assign_spells();

    log("Sorting command list and spells.");
    sort_commands();
    sort_spells();

    log("Booting mail system.");
    if (!scan_file()) {
        log("    Mail boot failed -- Mail system disabled");
        no_mail = 1;
    }
    log("Reading banned site and invalid-name list.");
    load_banned();
    Read_Invalid_List();

    log("Auction system reset.");
    auction_reset();
    if (!no_rent_check) {
        log("Deleting timed-out crash and rent files:");
        update_obj_file();
        log("Done.");
    }
    //log_ram();
    for (i = 0; i <= top_of_zone_table; i++) {
        sprintf(buf2, "Resetting %s (rooms %d-%d).",
                zone_table[i].name, (i ? (zone_table[i - 1].top + 1) : 0),
                zone_table[i].top);
        log(buf2);
        reset_zone(i);
    }

    reset_q.head = reset_q.tail = NULL;
    if (!mini_mud) {
        log("Booting houses.");
        House_boot();
    }
    boot_time = time(0);
    {

        gettimeofday( &curr_time1, NULL );
        boot_time_real = curr_time1.tv_sec;
    }
    log("Building area information.");
    build_area_info();
    generate_zonekill_ratio();

    //log_ram();

    MOBTrigger = TRUE;
    boottime=0;
    log("Boot db -- DONE.");
    //log_ram();
}


/* reset the time in the game from file */
void            reset_time(void)
{
    long            beginning_of_time = 650336715;
    struct time_info_data mud_time_passed(time_t t2, time_t t1);

    time_info = mud_time_passed(time(0), beginning_of_time);
    read_mud_date_from_file();

    if (time_info.hours <= 4)
        weather_info.sunlight = SUN_DARK;
    else if (time_info.hours == 5)
        weather_info.sunlight = SUN_RISE;
    else if (time_info.hours <= 20)
        weather_info.sunlight = SUN_LIGHT;
    else if (time_info.hours == 21)
        weather_info.sunlight = SUN_SET;
    else
        weather_info.sunlight = SUN_DARK;

    sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
            time_info.day, time_info.month, time_info.year);
    log(buf);

    weather_info.pressure = 960;
    if ((time_info.month >= 7) && (time_info.month <= 12))
        weather_info.pressure += dice(1, 50);
    else
        weather_info.pressure += dice(1, 80);

    weather_info.change = 0;

    if (weather_info.pressure <= 980)
        weather_info.sky = SKY_LIGHTNING;
    else if (weather_info.pressure <= 1000)
        weather_info.sky = SKY_RAINING;
    else if (weather_info.pressure <= 1020)
        weather_info.sky = SKY_CLOUDY;
    else
        weather_info.sky = SKY_CLOUDLESS;
}



/* generate index table for the player file */
void            build_player_index(void)
{
    int             nr = -1,
                         i;
    long            size,
    recs;
    struct char_file_u dummy;

    if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
        if (errno != ENOENT) {
            perror("fatal error opening playerfile");
            exit(1);
        } else {
            log("No playerfile.  Creating a new one.");
            touch(PLAYER_FILE);
            if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
                perror("fatal error opening playerfile");
                exit(1);
            }
        }
    }
    fseek(player_fl, 0L, SEEK_END);
    size = ftell(player_fl);
    rewind(player_fl);
    if (size % sizeof(struct char_file_u))
    {
        fprintf(stderr, "\aWARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
        exit(1);
    }
    recs = size / sizeof(struct char_file_u);
    if (recs) {
        sprintf(buf, "   %ld players in database.", recs);
        log(buf);
        CREATE(player_table, struct player_index_element, recs);
    } else {
        player_table = NULL;
        top_of_p_file = top_of_p_table = -1;
        return;
    }

    for (; !feof(player_fl);) {
        fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
        if (!feof(player_fl)) { /* new record */
            nr++;
            CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
            for (i = 0;
                    (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
            player_table[nr].id = dummy.char_specials_saved.idnum;
            top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);
        }
    }

    top_of_p_file = top_of_p_table = nr;
}



/* function to count how many hash-mark delimited records exist in a file */
int             count_hash_records(FILE * fl)
{
    char            buf[128];
    int             count = 0;

    rewind(fl);
    while (fgets(buf, 128, fl))
        if (*buf == '#')
            count++;

    return count;
}


/* function to count how many hash-mark delimited records exist in a file */
int             count_hash_records_area(FILE * fl, int mode)
{
    char            buf[128];
    int             count = 0;
    char pom[100];

    int in=0;

    switch (mode) {
    case DB_BOOT_WLD:
        strcpy(pom, "#ROOMS");
        break;
    case DB_BOOT_MOB:
        strcpy(pom, "#MOBILES");
        break;
    case DB_BOOT_OBJ:
        strcpy(pom, "#OBJECTS");
        break;
    case DB_BOOT_SHP:
        strcpy(pom, "#SHOPS");
        break;
    default:
        logs("SYSERR: Unknown subcommand %d to count_hash_area!", mode);
        exit(1);
        break;
    }

    rewind(fl);
    while (fgets(buf, 128, fl))
    {
        if (!strncmp(buf, pom, strlen(pom)))
            in=1;
        else if (!strncmp(buf, ends, strlen(ends)) && in)
            return count;
        else if (*buf == '#' && in)
            count++;
    }

    logs("SYSERR: Shouldnt get here count_hash_area() %d", mode);
    exit(1);
    return count;
}



void            index_boot(int mode)
{
    char           *index_filename,
    *prefix, *prefix2;
    FILE           *index=NULL,*index2=NULL,
                                       *db_file=NULL,*db_file2=NULL;
    int            *temp_pt;
    int             rec_count = 0;
    int area=0;

    switch (mode) {
    case DB_BOOT_WLD:
        prefix = WLD_PREFIX;
        break;
    case DB_BOOT_MOB:
        prefix = MOB_PREFIX;
        break;
    case DB_BOOT_OBJ:
        prefix = OBJ_PREFIX;
        break;
    case DB_BOOT_ZON:
        prefix = ZON_PREFIX;
        break;
    case DB_BOOT_SHP:
        prefix = SHP_PREFIX;
        break;
    default:
        log("SYSERR: Unknown subcommand to index_boot!");
        exit(1);
        break;
    }

    if (mini_mud)
        index_filename = MINDEX_FILE;
    else
        index_filename = INDEX_FILE;

    sprintf(buf2, "%s/%s", prefix, index_filename);

    if (!(index = fopen(buf2, "r"))) {
        sprintf(buf1, "Error opening index file '%s'", buf2);
        perror(buf1);
        exit(1);
    }
    /* first, count the number of records in the file so we can malloc */

    fscanf(index, "%s\n", buf1);
    while (*buf1 != '$') {
        sprintf(buf2, "%s/%s", prefix, buf1);
        if (!(db_file = fopen(buf2, "r"))) {
            logs("SYSERR: File '%s' listed in '%s/%s': %s", buf2, prefix,
                 index_filename, strerror(errno));
            fscanf(index, "%s\n", buf1);
            continue;
        } else {
            if (mode == DB_BOOT_ZON)
                rec_count++;
            else
                rec_count += count_hash_records(db_file);
        }
        fclose(db_file);
        fscanf(index, "%s\n", buf1);
    }




    prefix2=AREA_PREFIX;
    sprintf(buf2, "%s/%s", prefix2, index_filename);

    if (!(index2 = fopen(buf2, "r"))) {
        sprintf(buf1, "Error opening index file '%s'", buf2);
        perror(buf1);
        //exit(1);
    }
    else
        area=1;
    /* first, count the number of records in the file so we can malloc */

    if (area)
    {
        fscanf(index2, "%s\n", buf1);
        while (*buf1 != '$') {
            sprintf(buf2, "%s/%s", prefix2, buf1);
            if (!(db_file = fopen(buf2, "r"))) {
                logs("SYSERR: File '%s' listed in '%s/%s': %s", buf2, prefix2,
                     index_filename, strerror(errno));
                fscanf(index, "%s\n", buf1);
                continue;
            } else {
                if (mode == DB_BOOT_ZON)
                    rec_count++;
                else
                    rec_count += count_hash_records_area(db_file, mode);
            }
            fclose(db_file);
            fscanf(index2, "%s\n", buf1);
        }
    }








    /* Exit if 0 records, unless this is shops */
    if (!rec_count) {
        if (mode == DB_BOOT_SHP)
            return;
        logs("SYSERR: boot error - 0 records counted in %s/%s.", prefix,
             index_filename);
        if (index)
            fclose(index);
        if (index2)
            fclose(index2);

        exit(1);
    }
    rec_count++;

    switch (mode) {
    case DB_BOOT_WLD:
        logs("   %d rooms, total %d kilobytes.", rec_count, sizeof(struct room_data)*rec_count/1024);
        CREATE(world, struct room_data, rec_count + max_new_rooms);
        break;

    case DB_BOOT_MOB:
        logs("   %d mobs, total %d kilobytes.", rec_count, sizeof(struct char_data)*rec_count/1024);
        rec_count += max_new_mobs;
        CREATE(mob_proto, struct char_data, rec_count);
        CREATE(mob_index, struct index_data, rec_count);
        //CREATE(temp_pt, int, rec_count*(top_of_zone_table + 1));
        for (; rec_count > 0; rec_count--) {
            CREATE(temp_pt, int, top_of_zone_table + 1);
            mob_index[rec_count-1].number = temp_pt;
            temp_pt = NULL;
        }
        break;

    case DB_BOOT_OBJ:
        logs("   %d objects, total %d kilobytes.", rec_count, sizeof(struct obj_data)*rec_count/1024);
        rec_count += max_new_objects;
        CREATE(obj_proto, struct obj_data, rec_count);
        CREATE(obj_index, struct index_data, rec_count);
        for (; rec_count > 0; rec_count--) {
            CREATE(temp_pt, int, top_of_zone_table + 1);
            obj_index[rec_count-1].number = temp_pt;
            temp_pt = NULL;
        }
        break;

    case DB_BOOT_ZON:
        logs("   %d zones.", rec_count);
        CREATE(zone_table, struct zone_data, rec_count);
        break;
    }

    rewind(index);
    fscanf(index, "%s\n", buf1);
    while (*buf1 != '$') {
        sprintf(buf2, "%s/%s", prefix, buf1);
        if (!(db_file = fopen(buf2, "r"))) {
            logs("SYSERR: %s: %s", buf2, strerror(errno));
            exit(1);
        }
        switch (mode) {
        case DB_BOOT_WLD:
        case DB_BOOT_OBJ:
        case DB_BOOT_MOB:
            discrete_load(db_file, mode, 0);
            break;
        case DB_BOOT_ZON:
            load_zones(db_file, buf2, 0);
            break;
        case DB_BOOT_SHP:
            boot_the_shops(db_file, buf2, rec_count, 0);
            break;
        }

        fclose(db_file);
        fscanf(index, "%s\n", buf1);
    }
    fclose(index);

    if (area)
    {
        rewind(index2);
        fscanf(index2, "%s\n", buf1);
        while (*buf1 != '$') {
            sprintf(buf2, "%s/%s", prefix2, buf1);
            if (!(db_file = fopen(buf2, "r"))) {
                logs("SYSERR: %s: %s", buf2, strerror(errno));
                exit(1);
            }
            switch (mode) {
            case DB_BOOT_WLD:
            case DB_BOOT_OBJ:
            case DB_BOOT_MOB:
                discrete_load(db_file, mode, 1);
                break;
            case DB_BOOT_ZON:
                load_zones(db_file, buf2, 1);
                break;
            case DB_BOOT_SHP:
                boot_the_shops(db_file, buf2, rec_count, 1);
                break;
            }

            fclose(db_file);
            fscanf(index2, "%s\n", buf1);
        }
        fclose(index2);
    }


}

void            index_boot_area(int mode)
{
    char           *index_filename,
    *prefix;
    FILE           *index,
    *db_file;
    int            *temp_pt;
    int             rec_count = 0;

    switch (mode) {
    case DB_BOOT_WLD:

    case DB_BOOT_MOB:

    case DB_BOOT_OBJ:

    case DB_BOOT_ZON:

    case DB_BOOT_SHP:
        prefix=AREA_PREFIX;
        break;
    default:
        log("SYSERR: Unknown subcommand to index_boot!");
        exit(1);
        break;
    }

    if (mini_mud)
        index_filename = MINDEX_FILE;
    else
        index_filename = INDEX_FILE;


    sprintf(buf2, "%s/%s", prefix, index_filename);

    if (!(index = fopen(buf2, "r"))) {
        sprintf(buf1, "Error opening index file '%s'", buf2);
        perror(buf1);
        //exit(1);
        return;
    }
    /* first, count the number of records in the file so we can malloc */

    fscanf(index, "%s\n", buf1);
    while (*buf1 != '$') {
        sprintf(buf2, "%s/%s", prefix, buf1);
        if (!(db_file = fopen(buf2, "r"))) {
            logs("SYSERR: File '%s' listed in '%s/%s': %s", buf2, prefix,
                 index_filename, strerror(errno));
            fscanf(index, "%s\n", buf1);
            continue;
        } else {
            if (mode == DB_BOOT_ZON)
                rec_count++;
            else
                rec_count += count_hash_records_area(db_file, mode);
        }
        fclose(db_file);
        fscanf(index, "%s\n", buf1);
    }





    /* Exit if 0 records, unless this is shops */
    if (!rec_count) {
        if (mode == DB_BOOT_SHP)
            return;
        logs("SYSERR: boot error - 0 records counted in %s/%s.", prefix,
             index_filename);
        fclose(index);
        exit(1);
    }
    rec_count++;

    switch (mode) {
    case DB_BOOT_WLD:
        logs("   %d rooms, total %d kilobytes.", rec_count, sizeof(struct room_data)*rec_count/1024);
        CREATE(world, struct room_data, rec_count + max_new_rooms);
        break;

    case DB_BOOT_MOB:
        logs("   %d mobs, total %d kilobytes.", rec_count, sizeof(struct char_data)*rec_count/1024);
        rec_count += max_new_mobs;
        CREATE(mob_proto, struct char_data, rec_count);
        CREATE(mob_index, struct index_data, rec_count);
        //CREATE(temp_pt, int, rec_count*(top_of_zone_table + 1));
        for (; rec_count > 0; rec_count--) {
            CREATE(temp_pt, int, top_of_zone_table + 1);
            mob_index[rec_count-1].number = temp_pt;
            temp_pt = NULL;
        }
        break;

    case DB_BOOT_OBJ:
        logs("   %d objects, total %d kilobytes.", rec_count, sizeof(struct obj_data)*rec_count/1024);
        rec_count += max_new_objects;
        CREATE(obj_proto, struct obj_data, rec_count);
        CREATE(obj_index, struct index_data, rec_count);
        for (; rec_count > 0; rec_count--) {
            CREATE(temp_pt, int, top_of_zone_table + 1);
            obj_index[rec_count-1].number = temp_pt;
            temp_pt = NULL;
        }
        break;

    case DB_BOOT_ZON:
        logs("   %d zones.", rec_count);
        CREATE(zone_table, struct zone_data, rec_count);
        break;
    }

    rewind(index);
    fscanf(index, "%s\n", buf1);
    while (*buf1 != '$') {
        sprintf(buf2, "%s/%s", prefix, buf1);
        if (!(db_file = fopen(buf2, "r"))) {
            logs("SYSERR: %s: %s", buf2, strerror(errno));
            exit(1);
        }
        switch (mode) {
        case DB_BOOT_WLD:
        case DB_BOOT_OBJ:
        case DB_BOOT_MOB:
            discrete_load(db_file, mode, 1);
            break;
        case DB_BOOT_ZON:
            load_zones(db_file, buf2, 1);
            break;
        case DB_BOOT_SHP:
            boot_the_shops(db_file, buf2, rec_count, 1);
            break;
        }

        fclose(db_file);
        fscanf(index, "%s\n", buf1);
    }
    fclose(index);

}



void            discrete_load(FILE * fl, int mode, int area)
{
    int             nr = -1,
                         last = 0,
                                mobzone_flag=0;
    char            line[256];

    char           *modes[] = {"world", "mob", "obj"};
    int in=0;
    char pom[100];

    if (area)	// loading whole area
    {
        switch (mode) {
        case DB_BOOT_WLD:
            strcpy(pom, "#ROOMS");
            break;
        case DB_BOOT_MOB:
            strcpy(pom, "#MOBILES");
            break;
        case DB_BOOT_OBJ:
            strcpy(pom, "#OBJECTS");
            break;
        default:
            logs("SYSERR: Unknown subcommand %d to count_hash_area!", mode);
            exit(1);
            break;
        }

        rewind(fl);
        while (fgets(buf, 128, fl))
            if (!strncmp(buf, pom, strlen(pom)))
            {in=1;break;}

        if (!in)
        {logs("SYSERR: %s section not found! discrete_load()", pom);exit(1);}
    }

    for (;;) {
        /* we have to do special processing with the obj files because they
         * have no end-of-record marker :( */
        if (mode != DB_BOOT_OBJ || nr < 0)
            if (!get_line(fl, line)) {
                fprintf(stderr, "Format error after %s #%d\n", modes[mode], nr);
                exit(1);
            }
        if (*line == '$')
        {
            if (current_zone_num!=-1 && mode==DB_BOOT_MOB)
            {
                if (zone_table[current_zone_num].num<=5)
                    zone_table[current_zone_num].avg=zone_table[current_zone_num].sum/zone_table[current_zone_num].num;
                else
                    zone_table[current_zone_num].avg=(zone_table[current_zone_num].sum-zone_table[current_zone_num].max-zone_table[current_zone_num].min)/(zone_table[current_zone_num].num-2);
            }
            return;
        }

        if (*line == '#') {
            last = nr;
            if (sscanf(line, "#%d", &nr) != 1) {
                fprintf(stderr, "Format error after %s #%d\n", modes[mode], last);
                exit(1);
            }
            if (mobzone_flag==0 && mode==DB_BOOT_MOB)
            {
                current_zone_num=find_zone_by_num(nr/100);
                if (current_zone_num==-1)
                    logs("ERROR: Can not find zone #%d !!!\r\n", nr/100);
                else
                {
                    zone_table[current_zone_num].num=0;
                    zone_table[current_zone_num].sum=0;
                    zone_table[current_zone_num].max=-1000;
                    zone_table[current_zone_num].min=1000;
                    zone_table[current_zone_num].avg=0;
                    zone_table[current_zone_num].calc_ratio=100;
                    zone_table[current_zone_num].def_ratio=100;

                }
                mobzone_flag=1;
            }
            //    if (nr >= 99999)
            //      return;
            //else
            switch (mode) {
            case DB_BOOT_WLD:
                parse_room(fl, nr);
                break;
            case DB_BOOT_MOB:
                parse_mobile(fl, nr);
                break;
            case DB_BOOT_OBJ:
                strcpy(line, parse_object(fl, nr));
                break;
            }
        } else {
            fprintf(stderr, "Format error in %s file near %s #%d\n",
                    modes[mode], modes[mode], nr);
            fprintf(stderr, "Offending line: '%s'\n", line);
            exit(1);
        }
    }


}


ulong            asciiflag_conv(char *flag)
{
    ulong            flags = 0;
    int             is_number = 1;
    register char  *p;

    for (p = flag; *p; p++) {
        if (islower(*p))
            flags |= 1 << (*p - 'a');
        else if (isupper(*p))
            flags |= 1 << (26 + (*p - 'A'));

        if (!isdigit(*p))
            is_number = 0;
    }

    if (is_number)
        flags = atol(flag);

    return flags;
}

char            fread_letter(FILE * fp)
{
    char            c;
    do {
        c = getc(fp);
    } while (isspace(c));
    return c;
}

/* load the rooms */
void            parse_room(FILE * fl, int virtual_nr)
{
    static int      room_nr = 0,
                              zone = 0;
    int             t[10],
    i;
    char            line[256],
    flags[128];
    char letter;
    struct extra_descr_data *new_descr;
    struct teleport_data *new_tele;
    struct broadcast_data *new_broad;

    sprintf(buf2, "room #%d", virtual_nr);

    if (virtual_nr <= (zone ? zone_table[zone - 1].top : -1)) {
        fprintf(stderr, "Room #%d is below zone %d %d.\n", virtual_nr, zone, zone_table[zone - 1].top);
        exit(1);
    }
    while (virtual_nr > zone_table[zone].top)
        if (++zone > top_of_zone_table) {
            fprintf(stderr, "Room %d is outside of any zone.\n", virtual_nr);
            exit(1);
        }
    world[room_nr].zone = zone;
    world[room_nr].number = virtual_nr;
    world[room_nr].name = fread_string(fl, buf2);
    world[room_nr].description =fread_string(fl, buf2);


    if (!get_line(fl, line)) {
        fprintf(stderr, "Format error in room #%d\n", virtual_nr);
        exit(1);
    }

    if (sscanf(line, " %d %s %d %d %d %d", t, flags, t + 2, t+1, t+1, t+1) != 3)
        if (sscanf(line, " %d %s %d %d %d %d", t, flags, t+1, t+1, t+1, t + 2) != 6)
        {
            fprintf(stderr, "Format error in room #%d\n", virtual_nr);
            exit(1);
        }
    /* t[0] is the zone number; ignored with the zone-file system */
    world[room_nr].room_flags = asciiflag_conv(flags);
    world[room_nr].sector_type = t[2];

    world[room_nr].func = NULL;
    world[room_nr].contents = NULL;
    world[room_nr].people = NULL;
    //    world[room_nr].tele = NULL;
    //    world[room_nr].broad = NULL;
    world[room_nr].light = 0;   /* Zero light sources */

    for (i = 0; i < NUM_OF_DIRS; i++)
        world[room_nr].dir_option[i] = NULL;

    world[room_nr].ex_description = NULL;

    sprintf(buf, "Format error in room #%d (expecting D/E/S)", virtual_nr);

    for (;;) {
        if (!get_line(fl, line)) {
            fprintf(stderr, "%s\n", buf);
            exit(1);
        }
        switch (*line) {
        case 'D':
            setup_dir(fl, room_nr, atoi(line + 1));
            break;
        case 'E':
            CREATE(new_descr, struct extra_descr_data, 1);
            new_descr->keyword = fread_string(fl, buf2);
            new_descr->description = fread_string(fl, buf2);
            if (!new_descr->description)
            	fprintf(stderr, "Format error in room #%d's section E, missing description\n", virtual_nr);	
            new_descr->next = world[room_nr].ex_description;
            world[room_nr].ex_description = new_descr;
            break;
        case '>':
            (void) rprog_read_programs(fl, &world[room_nr]);
            break;
        case 'S':               /* end of room */
            top_of_world = room_nr++;
            return;
            break;
        case 'T':
            if (!get_line(fl, line) || sscanf(line, " %d %d %d %d", t, t + 1, t + 2, t + 3) != 4) {
                fprintf(stderr, "Format error in room #%d's section T\n", virtual_nr);
                exit(1);
            }
            /*            CREATE(new_tele, struct teleport_data, 1);
                        world[room_nr].tele = new_tele;
                        world[room_nr].tele->targ = t[0];
                        world[room_nr].tele->mask = t[1];
                        world[room_nr].tele->time = t[2];
                  if (IS_SET(world[room_nr].tele->mask, TELE_COUNT))
                     world[room_nr].tele->cnt = t[3]; 
                        world[room_nr].tele->obj = t[3];*/
            break;
        case 'B':
            if (!get_line(fl, line) || sscanf(line, " %d %d %d", t, t + 1, t + 2) != 3) {
                fprintf(stderr, "Format error in room #%d's section B\n", virtual_nr);
                exit(1);
            }
            /*            CREATE(new_broad, struct broadcast_data, 1);
                        world[room_nr].broad = new_broad;
                        new_broad = NULL;
                        world[room_nr].broad->channel = t[0];
                        world[room_nr].broad->targ1 = t[1];
                        world[room_nr].broad->targ2 = t[2];*/
            break;
        default:
            fprintf(stderr, "%s\n", buf);
            exit(1);
            break;
        }
    }


}



/* read direction data */
void            setup_dir(FILE * fl, int room, int dir)
{
    int             t[5], pom;
    char            line[256];

    sprintf(buf2, "room #%d, direction D%d", world[room].number, dir);

    CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
    world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
    world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

    if (!get_line(fl, line)) {
        fprintf(stderr, "Format error, %s\n", buf2);
        exit(1);
    }
    if ((pom=sscanf(line, " %d %d %d %d", t, t + 1, t + 2, t+3)) != 3) {
        if (pom==4 && t[3])
            fprintf(stderr, "%s, is special hidden exit\n", buf2);
        else if (pom!=4)
        {
            fprintf(stderr, "Format error, %s\n", buf2);
            exit(1);
        }
    }
    if (t[0]<0)	// smaug area conversion
        world[room].dir_option[dir]->exit_info=-t[0];
    else
    {
        if (t[0] == 1)
            world[room].dir_option[dir]->exit_info = EX_ISDOOR;
        else if (t[0] == 2)
            world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
        else if (t[0] == 3)
            world[room].dir_option[dir]->exit_info = EX_HIDDEN;
        else if (t[0] == 4)
            world[room].dir_option[dir]->exit_info = EX_HIDDEN | EX_ISDOOR;
        else if (t[0] == 5)
            world[room].dir_option[dir]->exit_info = EX_HIDDEN | EX_ISDOOR | EX_PICKPROOF;
        else
            world[room].dir_option[dir]->exit_info = 0;
    }
    if (dir==SOMEWHERE)
        world[room].dir_option[dir]->exit_info |= EX_HIDDEN;

    world[room].dir_option[dir]->key = t[1];
    world[room].dir_option[dir]->to_room = t[2];
}


/* make sure the start rooms exist & resolve their vnums to rnums */
void            check_start_rooms(void)
{
    extern sh_int   mortal_start_room;
    extern sh_int   immort_start_room;
    extern sh_int   frozen_start_room;

    if ((r_mortal_start_room = real_room(mortal_start_room)) < 0) {
        log("SYSERR:  Mortal start room does not exist.  Change in config.c.");
        exit(1);
    }
    if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
        if (!mini_mud)
            log("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
        r_immort_start_room = r_mortal_start_room;
    }
    if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
        if (!mini_mud)
            log("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
        r_frozen_start_room = r_mortal_start_room;
    }
}


/* resolve all vnums into rnums in the world */
void            renum_world(void)
{
    register int    room,
    door;

    /* before renumbering the exits, copy them to to_room_vnum */
    for (room = 0; room <= top_of_world; room++)
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (world[room].dir_option[door]) {
                /* copy */
                world[room].dir_option[door]->to_room_vnum =
                    world[room].dir_option[door]->to_room;
                if (world[room].dir_option[door]->to_room != NOWHERE)
                    world[room].dir_option[door]->to_room =
                        real_room(world[room].dir_option[door]->to_room);
            }
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void            renum_zone_table(void)
{
    int             zone,
    cmd_no,
    a,
    b,
    c,
    olda,
    oldb,
    oldc;
    char            buf[128];


    for (zone = 0; zone <= top_of_zone_table; zone++)
        for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
            a = b = c = 0;
            olda = ZCMD.arg1;
            oldb = ZCMD.arg2;
            oldc = ZCMD.arg3;

            switch (ZCMD.command) {
            case 'M':
                a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
                c = ZCMD.arg3 = real_room(ZCMD.arg3);
                if (ZCMD.arg2<=0)
                {
                    sprintf(buf, "ERROR (zone): Zero mobs in room [%5d] - %d", oldc, ZCMD.arg2);
                    log(buf);
                    ZCMD.arg2=1;
                }
                if (ZCMD.arg2>MAX_MOBS_PER_ROOM)
                {
                    //sprintf(buf, "ERROR (zone): Too many mobs in room [%5d] - %d", oldc, ZCMD.arg2);
                    //log(buf);
                    ZCMD.arg2=MAX_MOBS_PER_ROOM;
                }
                mobkills[olda].rnum=a;
                break;
            case 'O':
                
                a = ZCMD.arg1 = real_object(ZCMD.arg1);
                if (ZCMD.arg3 != NOWHERE)
                    c = ZCMD.arg3 = real_room(ZCMD.arg3);
                
                if (a!=-1 && GET_OBJ_TYPE(&obj_proto[a])==ITEM_KEY)
                {                	
                	ZCMD.arg2=10;
                }
                if (ZCMD.arg2<=0)
                {
                    //sprintf(buf, "ERROR (zone): Zero objs in room [%5d] - %d", oldc, ZCMD.arg2);
                    //log(buf);
                    ZCMD.arg2=1;
                }
                if (ZCMD.arg2>MAX_OBJS_PER_ROOM)
                {
                    //sprintf(buf, "ERROR (zone): Too many objs in room [%5d] - %d", oldc, ZCMD.arg2);
                    //log(buf);
                    ZCMD.arg2=MAX_OBJS_PER_ROOM;
                }
                break;
            case 'G':
                a = ZCMD.arg1 = real_object(ZCMD.arg1);
                if (a!=-1 && GET_OBJ_TYPE(&obj_proto[a])==ITEM_KEY)
                {                	
                	ZCMD.arg2=5;
                }
                break;
            case 'E':
                a = ZCMD.arg1 = real_object(ZCMD.arg1);
                if (a!=-1 && GET_OBJ_TYPE(&obj_proto[a])==ITEM_KEY)
                {                	
                	ZCMD.arg2=5;
                }
                break;
            case 'P':
            
                a = ZCMD.arg1 = real_object(ZCMD.arg1);
                c = ZCMD.arg3 = real_object(ZCMD.arg3);
                if (a!=-1 && GET_OBJ_TYPE(&obj_proto[a])==ITEM_KEY)
                {                	
                	ZCMD.arg2=5;
                }
                break;
            case 'D':
                a = ZCMD.arg1 = real_room(ZCMD.arg1);
                break;
            case 'R':           /* rem obj from room */
                a = ZCMD.arg1 = real_room(ZCMD.arg1);
                b = ZCMD.arg2 = real_object(ZCMD.arg2);
                break;
            }
            if (a < 0 || b < 0 || c < 0) {
                if (!mini_mud) {
                    sprintf(buf, "Invalid vnum %d, cmd disabled",
                            (a < 0) ? olda : ((b < 0) ? oldb : oldc));
                    log_zone_error(zone, cmd_no, buf);
                } ZCMD.command = '*';
            }
        }
}



void            parse_simple_mob(FILE * mob_f, int i, int nr)
{
    int             j,
    t[10];
    char            line[256];

    mob_proto[i].real_abils.str = 1;
    mob_proto[i].real_abils.intel = 1;
    mob_proto[i].real_abils.wis = 1;
    mob_proto[i].real_abils.dex = 1;
    mob_proto[i].real_abils.con = 1;
    mob_proto[i].real_abils.cha = 1;

    if (!get_line(mob_f, line) || (sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
                                          t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9)) {
        fprintf(stderr, "SYSERR: Format error in mob #%d, first line after S flag\n"
                "...expecting line of form '# # # #d#+# #d#+#'\n", nr);
        exit(1);
    }
    GET_LEVEL(mob_proto + i) = (t[0] !=0 ? t[0] :1);
    mob_proto[i].points.hitroll = 25 - t[1];
    //mob_proto[i].points.armor = 10 * t[2];
    //ovde
    mob_proto[i].points.armor = 0;//GET_LEVEL(mob_proto + i); see fight.c
    mob_proto[i].points.magic_armor = 0;

    /* max hit = 0 is a flag that H, M, V is xdy+z */
    mob_proto[i].points.max_hit = 0;
    mob_proto[i].points.hit = t[3];
    mob_proto[i].points.mana = t[4];
    mob_proto[i].points.move = t[5];

    mob_proto[i].points.max_mana = 100;
    mob_proto[i].points.max_move = 50;

    mob_proto[i].mob_specials.damnodice = t[6];
    mob_proto[i].mob_specials.damsizedice = t[7];

    mob_proto[i].points.damroll = t[8];


    if (!get_line(mob_f, line) || (sscanf(line, " %d %d ", t + 1, t + 2) != 2)) {
        fprintf(stderr, "SYSERR: Format error in mob #%d, second line after S flag\n"
                "...expecting line of form '# #'\n", nr);
        exit(1);
    }
    if (t[1] + t[2] == 0) {
        GET_GOLD(mob_proto + i) = t[0] * (MIN(1, t[0] - 3 + number(0, 6))) * t[0] / 2;
    } else {
        GET_GOLD(mob_proto + i) = t[1];
    }

    if (t[2] < 0)
        GET_RATIO(mob_proto + i) = -t[2];
    else
        GET_RATIO(mob_proto + i) = 100;
    GET_EXP(mob_proto+i)=0;

    if (!get_line(mob_f, line) || (sscanf(line, " %d %d %d %d ", t, t + 1, t + 2, t + 3) != 3)) {
        fprintf(stderr, "SYSERR: Format error in last line of mob #%d\n"
                "...expecting line of form '# # #'\n", nr);
        exit(1);
    }
    mob_proto[i].char_specials.position = t[0];
    mob_proto[i].mob_specials.default_pos = t[1];
    mob_proto[i].player.sex = t[2];

    for (j = 0; j < 3; j++)
        GET_COND(mob_proto + i, j) = -1;

    /* these are now save applies; base save numbers for MOBs are now from
     * the warrior save table. */
    for (j = 0; j < 5; j++)
        GET_SAVE(mob_proto + i, j) = 0;
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 */

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#define RANGE(low, high) (num_arg = MAX((low), MIN((high), (num_arg))))

void            interpret_espec(char *keyword, char *value, int i, int nr)
{
    int             num_arg,
    matched = 0;

    num_arg = atoi(value);

    CASE("BareHandAttack") {
        RANGE(0, 99);
        mob_proto[i].mob_specials.attack_type = num_arg;
    }
    CASE("Class") {
        RANGE(0, ANYCLASS);
        mob_proto[i].player.class = num_arg;
    }
    CASE("Race") {
        RANGE(0, MAX_NPC_RACE);
        mob_proto[i].player.race = num_arg;
    }
    CASE("Attacks") {
        RANGE(1, 5);
        mob_proto[i].mob_specials.attack_num = num_arg;
    }

    CASE("Str") {
        RANGE(3, 25);
        mob_proto[i].real_abils.str = num_arg;
    }

    CASE("StrAdd") {
        RANGE(0, 100);
        mob_proto[i].real_abils.str_add = num_arg;
    }

    CASE("Int") {
        RANGE(3, 25);
        mob_proto[i].real_abils.intel = num_arg;
    }

    CASE("Wis") {
        RANGE(3, 25);
        mob_proto[i].real_abils.wis = num_arg;
    }

    CASE("Dex") {
        RANGE(3, 25);
        mob_proto[i].real_abils.dex = num_arg;
    }

    CASE("Con") {
        RANGE(3, 25);
        mob_proto[i].real_abils.con = num_arg;
    }

    CASE("Cha") {
        RANGE(3, 25);
        mob_proto[i].real_abils.cha = num_arg;
    }

    if (!matched) {
        fprintf(stderr, "Warning: unrecognized espec keyword %s in mob #%d\n",
                keyword, nr);
    }
}

#undef CASE
#undef RANGE

void            parse_espec(char *buf, int i, int nr)
{
    char           *ptr;

    if ((ptr = strchr(buf, ':')) != NULL) {
        *(ptr++) = '\0';
        while (isspace(*ptr))
            ptr++;
    } else
        ptr = "";

    interpret_espec(buf, ptr, i, nr);
}


void            parse_enhanced_mob(FILE * mob_f, int i, int nr)
{
    char            line[256];

    parse_simple_mob(mob_f, i, nr);

    while (get_line(mob_f, line)) {
        if (!strcmp(line, "E")) /* end of the ehanced section */
            return;
        else if (*line == '#') {/* we've hit the next mob, maybe? */
            fprintf(stderr, "Unterminated E section in mob #%d\n", nr);
            exit(1);
        } else
            parse_espec(line, i, nr);
    }

    fprintf(stderr, "Unexpected end of file reached after mob #%d\n", nr);
    exit(1);
}

void            parse_complex_mob(FILE * mob_f, int i, int nr)
{
    char            line[256];
    int cnt=0;
    int temp, temp1;
    parse_simple_mob(mob_f, i, nr);

    while (get_line(mob_f, line)) {
        if (!strcmp(line, "C")) /* end of the ehanced section */
            return;
        else if (*line == '#') {/* we've hit the next mob, maybe? */
            fprintf(stderr, "Unterminated C section in mob #%d\n", nr);
            exit(1);
        } else{
            switch(cnt)
            {
            case 0:
                if (sscanf(line, "%d %d %d %d %d %d %d",
                           &mob_proto[i].real_abils.str, &mob_proto[i].real_abils.intel, &mob_proto[i].real_abils.wis, &mob_proto[i].real_abils.dex, &mob_proto[i].real_abils.con, &mob_proto[i].real_abils.cha, &temp)!=7)
                {
                    fprintf(stderr, "Error in C section in mob #%d (1 line)\n", nr);
                    exit(1);
                }
                cnt++;
                break;
            case 1:
                if (sscanf(line, "%d %d %d", &mob_proto[i].player.race, &temp, &temp1)!=3)
                {
                    fprintf(stderr, "Error in C section in mob #%d (2 line)\n", nr);
                    exit(1);
                }
                mob_proto[i].player.class=1<<temp;
                if (mob_proto[i].player.race==RACE_ANIMAL || mob_proto[i].player.race==RACE_SMALL_ANIMAL)
                    mob_proto[i].mob_specials.attack_type=TYPE_BITE-TYPE_HIT;
                cnt++;
                break;
            default:
                fprintf(stderr, "Error in C section in mob #%d (line unknown)\n", nr);
                exit(1);
            }
        }

    }

    fprintf(stderr, "Unexpected end of file reached after mob #%d\n", nr);
    exit(1);
}


void            parse_mobile(FILE * mob_f, int nr)
{
    static int      i = 0;
    int             j,
    t[10], pom;
    char            line[256],
    *tmpptr,
    letter;
    char            f1[128],
    f2[128],
    f3[128],
    f4[128],
    f5[128];
    CHAR_DATA *mob;
    int plev;


    mob_index[i].virtual = nr;


    mob_index[i].func = NULL;

    clear_char(mob_proto + i);

    mob_proto[i].player_specials = &dummy_mob;
    mob_proto[i].mob_specials.attack_type = TYPE_HIT-TYPE_HIT;
    mob_proto[i].mob_specials.attack_num = 1;
    //    mob_proto[i].mob_specials.brag=NULL;
    //sprintf(buf2, "mob vnum %d", nr);

    /***** String data *** */
    
    mob_proto[i].player.name = fread_string(mob_f, buf2);
    tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
    if (tmpptr && *tmpptr)
        if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
                !str_cmp(fname(tmpptr), "the"))
            *tmpptr = LOWER(*tmpptr);
    mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
    mob_proto[i].player.description = fread_string(mob_f, buf2);
    mob_proto[i].player.title = NULL;
    mob_proto[i].player.class = CLASS_WARRIOR;
    mob_proto[i].player.race = RACE_NORMAL;

    /* *** Numeric data *** */
    //strcpy(line, fread_string(mob_f, buf2));
    get_line(mob_f, line);
    if (sscanf(line, " %s %s %s %c ", f1, f2, f3, &letter) == 3) {
        /* new style */
        MOB_FLAGS(mob_proto + i) = asciiflag_conv(f1);
        MOB2_FLAGS(mob_proto + i) = asciiflag_conv(f2);
        MOB3_FLAGS(mob_proto + i) = asciiflag_conv(f3);
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
        get_line(mob_f, line);
        sscanf(line, " %s %s %s ", f1, f2, f3);
        AFF_FLAGS(mob_proto + i) = asciiflag_conv(f1);
        AFF2_FLAGS(mob_proto + i) = asciiflag_conv(f2);
        AFF3_FLAGS(mob_proto + i) = asciiflag_conv(f3);
        get_line(mob_f, line);
        sscanf(line, " %d %c ", t + 2, &letter);
        AFF3_FLAGS(mob_proto + i) = 0;
    } else if ( sscanf(line, " %s %s %s %s %s %s %s %s %d %c ", f1, f3,f3,f3, f2, f3, f3, f3, t + 2, &letter)==10)
    {
        /* Dibrova style */
        //sscanf(line, " %s %s %d %c ", f1, f2, t + 2, &letter);
        MOB_FLAGS(mob_proto + i) = asciiflag_conv(f1);
        MOB2_FLAGS(mob_proto + i) = 0;
        MOB3_FLAGS(mob_proto + i) = 0;
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
        AFF_FLAGS(mob_proto + i) = asciiflag_conv(f2);
        AFF2_FLAGS(mob_proto + i) = 0;
        AFF3_FLAGS(mob_proto + i) = 0;
    } else if ( sscanf(line, " %s %s %d %c ", f1, f2, t + 2, &letter)==4)
    {
        /* old style */
        //sscanf(line, " %s %s %d %c ", f1, f2, t + 2, &letter);
        MOB_FLAGS(mob_proto + i) = asciiflag_conv(f1);
        MOB2_FLAGS(mob_proto + i) = 0;
        MOB3_FLAGS(mob_proto + i) = 0;
        SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
        AFF_FLAGS(mob_proto + i) = asciiflag_conv(f2);
        AFF2_FLAGS(mob_proto + i) = 0;
    }

    if (t[2]>=0) t[2]=1000;
    else if (t[2]<0) t[2]=-1000;
    else t[2]=0;

    GET_ALIGNMENT(mob_proto + i) = t[2];
    switch (UPPER(letter)) {
    case 'S':                   /* Simple monsters */
        parse_simple_mob(mob_f, i, nr);
        break;
    case 'E':                   /* Circle3 Enhanced monsters */
        parse_enhanced_mob(mob_f, i, nr);
        break;
    case 'C':                   /* Smaug Enhanced monsters */
        parse_complex_mob(mob_f, i, nr);
        break;
        /* add new mob types here.. */
    default:
        fprintf(stderr, "Unsupported mob type '%c' in mob #%d - %s\n", letter, nr, mob_proto[i].player.name);
        exit(1);
        break;
    }

    if (current_zone_num!=-1)
    {
        zone_table[current_zone_num].num++;
        zone_table[current_zone_num].sum+=GET_LEVEL(mob_proto+i);
        if (zone_table[current_zone_num].min>GET_LEVEL(mob_proto+i))
            zone_table[current_zone_num].min=GET_LEVEL(mob_proto+i);
        if (zone_table[current_zone_num].max<GET_LEVEL(mob_proto+i))
            zone_table[current_zone_num].max=GET_LEVEL(mob_proto+i);
        //    	zone_table[current_zone_num].avg=zone_table[current_zone_num].sum/zone_table[current_zone_num].num;
    }

    mob_proto[i].player.weight = (50+GET_LEVEL(mob_proto+i)*GET_LEVEL(mob_proto+i)/7)*2;
    mob_proto[i].player.height = 100+GET_LEVEL(mob_proto+i)*4;


    /*mob_proto[i].real_abils.str += GET_LEVEL(mob_proto + i)/10;
    mob_proto[i].real_abils.intel  += GET_LEVEL(mob_proto + i)/15;
    mob_proto[i].real_abils.wis  += GET_LEVEL(mob_proto + i)/20;
    mob_proto[i].real_abils.dex  += GET_LEVEL(mob_proto + i)/20;
    mob_proto[i].real_abils.con += GET_LEVEL(mob_proto + i)/12;
    mob_proto[i].real_abils.cha  += GET_LEVEL(mob_proto + i)/8;

    mob_proto[i].real_abils.cha = MIN(25, mob_proto[i].real_abils.cha);
    mob_proto[i].real_abils.wis = MIN(25, mob_proto[i].real_abils.wis);
    mob_proto[i].real_abils.con = MIN(25, mob_proto[i].real_abils.con);
    mob_proto[i].real_abils.dex = MIN(25, mob_proto[i].real_abils.dex);
    mob_proto[i].real_abils.intel = MIN(25, mob_proto[i].real_abils.intel);
    mob_proto[i].real_abils.str = MIN(25, mob_proto[i].real_abils.str);
      */

    mob_proto[i].aff_abils = mob_proto[i].real_abils;

    for (j = 0; j < NUM_WEARS; j++)
        mob_proto[i].equipment[j] = NULL;

    mob_proto[i].nr = i;
    mob_proto[i].desc = NULL;

    letter = fread_letter(mob_f);
    if (letter == '>') {
        ungetc(letter, mob_f);
        (void) mprog_read_programs(mob_f, &mob_index[i]);
    } else
        ungetc(letter, mob_f);
        
                    
    mob = &mob_proto[i];    
    my_srand(GET_MOB_VNUM(mob));

    //plev=(GET_LEVEL(mob)<=5 ? GET_LEVEL(mob): MAX(1,fuzzy(GET_LEVEL(mob))));
    plev=GET_LEVEL(mob);
    mob->points.damroll=MAX(0,fuzper(3*(plev*plev/106+plev/2)/4, 5));//fuzzy(plev)*fuzzy(plev)/100+plev/4;
    mob->points.hitroll=fuzper(3*(plev*plev/106+plev/2)/4, 5);
    mob->points.max_hit= 8*(plev*9 + (plev * plev / 4 + fuzzy(plev) * plev) / 2 +plev*plev*plev/64-2)/7;
    mob->points.max_move= 150+plev*3;
    mob->points.max_mana= 8*(plev*9 + (plev * plev / 4 + fuzzy(plev) * plev) / 2 +plev*plev*plev/64-2)/7;
    mob->points.armor=MAX(0, fuzper(3*(4*plev*plev/23+3*plev)/4, 5));  // formula se koristi u fight.c-u za max armor moba
    mob->points.magic_armor=fuzper(3*(plev*plev/55+3*plev/10)/4, 5);
    
    	
    /*if (!number(0, 9))// || number(1, 6000)>plev*plev)
        mob->points.gold=MAX(0, MAX(plev-3,fuzzy(fuzzy(plev)*(plev)/fuzzy(16))));
    else*/
        mob->points.gold=0;
      
    mob->real_abils.str=  MIN(25, 11+plev/4+number(0, 3));
    mob->real_abils.dex=  MIN(25, 11+plev/4+number(0, 3));
    mob->real_abils.con=  MIN(25, 11+plev/4+number(0, 3));
    mob->real_abils.cha=  MIN(25, 9+plev/4+number(0, 5));
    mob->real_abils.wis=  MIN(25, 10+plev/4+number(0, 4));
    mob->real_abils.intel=MIN(25, 10+plev/4+number(0, 4));

    mob->aff_abils = mob->real_abils;

    //mob->max_vitality=mob->points.max_hit;
    
    SET_STYLE(mob,0);

    mob->points.hitr=mob->points.hit = mob->points.max_hit;
    mob->points.manar=mob->points.mana = mob->points.max_mana;
    mob->points.mover=mob->points.move = mob->points.max_move;
    GET_HIT(mob)=GET_MAX_HIT(mob);

    
    
    // get mobs deity
    if (CHANCE(75) || GET_LEVEL(mob)<10)
    {	
        SET_DEITY(mob, number(1, 3));
    }
    else
    {
    	SET_DEITY(mob, number(4, MAX_DEITY));   
    }
  //  if (GET_DEITY(mob)!=3)
    //	logs("%d - %s", GET_MOB_VNUM(mob), deity_list[GET_DEITY(mob)].name);
    
    my_srand(rand());
    

    top_of_mobt = i++;
}


void parse_druid_monk(struct obj_data *obj, int vnum)
{

    //DRUID

    if (IS_OBJ_STAT(obj,ITEM_ANTI_CLERIC) && vnum%2==0)
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_DRUID);

    if (IS_OBJ_STAT(obj,ITEM_ANTI_MAGIC_USER) && vnum%3==0)
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_DRUID);

    if (IS_OBJ_STAT(obj,ITEM_ANTI_MAGIC_USER) && IS_OBJ_STAT(obj,ITEM_ANTI_CLERIC))
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_DRUID);

    //MONK

    if (IS_OBJ_STAT(obj,ITEM_ANTI_WARRIOR) && vnum%2==0)
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_MONK);

    if (IS_OBJ_STAT(obj,ITEM_ANTI_THIEF) && vnum%3==0)
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_MONK);

    if (IS_OBJ_STAT(obj,ITEM_ANTI_WARRIOR) && IS_OBJ_STAT(obj,ITEM_ANTI_THIEF))
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_MONK);

    //RANGER

    if (IS_OBJ_STAT(obj,ITEM_ANTI_WARRIOR) && vnum%2==0)
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_RANGER);

    if (IS_OBJ_STAT(obj,ITEM_ANTI_CLERIC) && vnum%3==0)
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_RANGER);

    if (IS_OBJ_STAT(obj,ITEM_ANTI_WARRIOR) && IS_OBJ_STAT(obj,ITEM_ANTI_CLERIC))
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_RANGER);

}

/* read all objects from obj file; generate index and prototypes */
char           *parse_object(FILE * obj_f, int nr)
{
    static int      i = 0,
                        retval;
    static char     line[256];
    int             t[10],
    j;
    char           *tmpptr;
    char            f1[256],
    f2[256],
    f4[256],
    f5[256],
    f3[256];

    struct extra_descr_data *new_descr;
    char letter;
    obj_index[i].virtual = nr;
    obj_index[i].func = NULL;
    obj_proto[i].obj_flags.bitvector = 0;
    obj_proto[i].obj_flags.bitvector2 = 0;
    obj_proto[i].obj_flags.bitvector3 = 0;

    clear_object(obj_proto + i);
    obj_proto[i].in_room = NOWHERE;
    obj_proto[i].item_number = i;

    sprintf(buf2, "object #%d", nr);

    /* *** string data *** */
    if ((obj_proto[i].name = fread_string(obj_f, buf2)) == NULL) {
        fprintf(stderr, "Null obj name or format error at or near %s\n", buf2);
        exit(1);
    }
    tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
    if (tmpptr && *tmpptr)
        if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
                !str_cmp(fname(tmpptr), "the"))
            *tmpptr = LOWER(*tmpptr);

    tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
    if (tmpptr && *tmpptr)
        *tmpptr = UPPER(*tmpptr);
    obj_proto[i].action_description = fread_string(obj_f, buf2);

    /* *** numeric data *** */
    retval = 0;
    if (get_line(obj_f, line)) {
        if ((retval = sscanf(line, " %d %s %s %s %s %s %s %s %s", t, f1, f2, f3, f5, f4, f5, f5, f5)) == 4) {
            /* new style */
            obj_proto[i].obj_flags.type_flag = t[0];
            obj_proto[i].obj_flags.extra_flags = asciiflag_conv(f1);
            obj_proto[i].obj_flags.extra_flags2 = asciiflag_conv(f2);
            obj_proto[i].obj_flags.wear_flags = asciiflag_conv(f3);
            parse_druid_monk(&obj_proto[i],obj_index[i].virtual);
        } else if (retval == 3) {
            /* old style */
            obj_proto[i].obj_flags.type_flag = t[0];
            obj_proto[i].obj_flags.extra_flags = asciiflag_conv(f1);
            obj_proto[i].obj_flags.extra_flags2 = 0;
            obj_proto[i].obj_flags.wear_flags = asciiflag_conv(f2);
            parse_druid_monk(&obj_proto[i],obj_index[i].virtual);
        } else if (retval == 9) {
            /* Dibrova style */
            obj_proto[i].obj_flags.type_flag = t[0];
            obj_proto[i].obj_flags.extra_flags = asciiflag_conv(f1);
            obj_proto[i].obj_flags.extra_flags2 = 0;
            obj_proto[i].obj_flags.wear_flags = asciiflag_conv(f4);
            parse_druid_monk(&obj_proto[i],obj_index[i].virtual);
        } else {
            retval = 0;
        }
    }
    if (retval == 0) {
        fprintf(stderr, "Format error in first numeric line (expecting 4 args, got %d), %s\n", retval, buf2);
        exit(1);
    }
    if (!get_line(obj_f, line) ||
            (retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t + 3)) != 4) {
        fprintf(stderr, "Format error in second numeric line (expecting 4 args, got %d), %s\n", retval, buf2);
        exit(1);
    }
    //if (obj_proto[i].obj_flags.type_flag ==ITEM_ARMOR)
    //t[0]=-t[0];
    obj_proto[i].obj_flags.value[0] = t[0];
    obj_proto[i].obj_flags.value[1] = t[1];
    obj_proto[i].obj_flags.value[2] = t[2];
    obj_proto[i].obj_flags.value[3] = t[3];
    obj_proto[i].obj_flags.data = 0;
    
    
    REMOVE_BIT(obj_proto[i].obj_flags.extra_flags, ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL);
    REMOVE_BIT(obj_proto[i].obj_flags.extra_flags, ITEM_HIDDEN_EGO);
    REMOVE_BIT(obj_proto[i].obj_flags.extra_flags, ITEM_EGO);       
  
    if ((obj_proto[i].obj_flags.type_flag==ITEM_ARMOR) && !CANBE_ARMOR(&obj_proto[i]))
       obj_proto[i].obj_flags.type_flag=ITEM_WORN;
               

    /*if (obj_proto[i].obj_flags.type_flag==ITEM_FOOD)
{
    	char buffood[200];
    	if (!strstr(obj_proto[i].name, "food")) 
    	{
    		sprintf(buffood,"%s food", obj_proto[i].name);
    		obj_proto[i].name=str_dup(buffood);    
    	}
} */

    if (!get_line(obj_f, line) ||
            (retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t+3)) != 3) {

        if (retval !=4)
        {
            fprintf(stderr, "Format error in third numeric line (expecting 3 args, got %d), %s\n", retval, buf2);
            exit(1);
        }
    }
    obj_proto[i].obj_flags.weight = MAX(1, 3*t[0]/4);
    obj_proto[i].obj_flags.cost = t[1];
    obj_proto[i].obj_flags.cost_per_day = t[2];
    if (obj_proto[i].obj_flags.type_flag ==ITEM_WEAPON)
    {
        switch (obj_proto[i].obj_flags.value[3]+TYPE_HIT) {
        case TYPE_POUND:
        case TYPE_BLUDGEON:
        case TYPE_CRUSH:
        case TYPE_THRASH:
        case TYPE_WHIP:
        case TYPE_CLAW:
            obj_proto[i].obj_flags.value[3] = TYPE_POUND-TYPE_HIT; break;
        case TYPE_SLASH:
        case TYPE_MAUL:
            obj_proto[i].obj_flags.value[3] = TYPE_SLASH-TYPE_HIT; break;
        case TYPE_PIERCE:
        case TYPE_STAB:
        case TYPE_STING:
            obj_proto[i].obj_flags.value[3] = TYPE_PIERCE-TYPE_HIT; break;
        default:
            fprintf(stderr, "Illegal type: %d - %s (type %d)\n", obj_index[i].virtual, obj_proto[i].short_description, obj_proto[i].obj_flags.value[3]);
            obj_proto[i].obj_flags.value[3] = TYPE_POUND-TYPE_HIT; break;
        }
        if (obj_proto[i].obj_flags.weight>60)
        {
            fprintf(stderr, "Illegal weight: %d - %s (weight %d)\n", obj_index[i].virtual, obj_proto[i].short_description, obj_proto[i].obj_flags.weight);
            obj_proto[i].obj_flags.weight=60;
        }
    }

    /* check to make sure that weight of containers exceeds curr. quantity */
    if (obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON ||
            obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN) {
        if (obj_proto[i].obj_flags.weight < obj_proto[i].obj_flags.value[1])
            obj_proto[i].obj_flags.weight = obj_proto[i].obj_flags.value[1] + 5;
    }
    /* *** extra descriptions and affect fields *** */

    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        obj_proto[i].affected[j].location = APPLY_NONE;
        obj_proto[i].affected[j].modifier = 0;
    }

    strcat(buf2, ", after numeric constants (expecting E/A/F/C/#xxx)");
    j = 0;
    obj_proto[i].bound_spell=obj_proto[i].bound_spell_level=obj_proto[i].bound_spell_timer=0;
    for (;;) {
        if (!get_line(obj_f, line)) {
            fprintf(stderr, "Format error in %s\n", buf2);
            exit(1);
        }
        switch (*line) {
        case 'E':
            CREATE(new_descr, struct extra_descr_data, 1);
            new_descr->keyword = fread_string(obj_f, buf2);
            new_descr->description = fread_string(obj_f, buf2);
            new_descr->next = obj_proto[i].ex_description;
            obj_proto[i].ex_description = new_descr;
            break;
        case 'A':
            if (j >= MAX_OBJ_AFFECT) {
                fprintf(stderr, "Too many A fields (%d max), %s\n", MAX_OBJ_AFFECT, buf2);
                exit(1);
            }
            if (!get_line(obj_f, line) || (sscanf(line, " %d %d ", t, t + 1) != 2)) {
                fprintf(stderr, "SYSERR: Format error in 'A' field, %s\n", buf2);
                fprintf(stderr, "...offending line: '%s'\n", line);
            }


            //            if (t[0]==APPLY_HIT && t[1]<0)
            //          	fprintf(stderr,"#%d - %s  %d to hit points\n", nr, obj_proto[i].short_description, t[1]);

            if (t[0] == APPLY_AC)
                t[1]=-t[1];

            if (t[0] == APPLY_LEVEL)
            {
                obj_proto[i].obj_flags.data=t[1];
                fprintf(stderr,"#%d - %s, level set to %d (by APPLY_LEV)\r\n", nr, obj_proto[i].short_description, t[1]);
            }
            
	     if (t[0] == APPLY_AGE)
	     	break;
            //else
            //if (t[0]!= APPLY_AGE && t[0]!= APPLY_CHAR_HEIGHT && t[0]!= APPLY_CHAR_WEIGHT)
            {
                obj_proto[i].affected[j].location = t[0];
                obj_proto[i].affected[j].modifier = t[1];
                j++;
            }

            /* if (obj_proto[i].affected[j].location == APPLY_AC && t[1] < acmin && t[1]>-30)
             acmin= t[1];
             obj_proto[i].affected[j].modifier = t[1];
             if (t[1]==-30)
             	fprintf(stderr,"#%d - %s\n", nr, obj_proto[i].short_description);*/
            break;
        case 'F':
            get_line(obj_f, line);
            sscanf(line, " %s %s %s ", f1, f2, f3);
            obj_proto[i].obj_flags.bitvector = asciiflag_conv(f1);
            obj_proto[i].obj_flags.bitvector2 = asciiflag_conv(f2);
            obj_proto[i].obj_flags.bitvector3 = asciiflag_conv(f3);
            break;
        case 'S':
            get_line(obj_f, line);
            sscanf(line, " %s %s %s ", f1, f2, f3);
            obj_proto[i].bound_spell = asciiflag_conv(f1);
            obj_proto[i].bound_spell_level = asciiflag_conv(f2);
            obj_proto[i].bound_spell_timer = asciiflag_conv(f3);
            break;
        case 'C':

            //tmpptr = obj_proto[i].attack_verb = fread_string(obj_f, buf2);
            if (!get_line(obj_f, line) || (sscanf(line, " %d %d ", t, t + 1) != 2)) {
                fprintf(stderr, "SYSERR: Format error in 'A' field, %s\n", buf2);
                fprintf(stderr, "...offending line: '%s'\n", line);
            }
            fprintf(stderr,"#%d - %s has a C modifier (%s by %d)\n", nr, obj_proto[i].short_description, spells[t[0]], t[1]);
            //tmpptr =  fread_string(obj_f, buf2);
            //*tmpptr = LOWER(*tmpptr);
            break;
        case '>':
            (void) oprog_read_programs(obj_f, &obj_index[i]);
            break;
      
        case '$':
        case '#':
            top_of_objt = i++;
            return line;
            break;
        default:
            fprintf(stderr, "Format error in %s\n", buf2);
           // exit(1);
            break;
        }
    }
    
    
    

}


#define Z	zone_table[zone]

/* load the zone table and command tables */
void            load_zones(FILE * fl, char *zonename, int area)
{
    static int      zone = 0;
    int             cmd_no = 0,
                             num_of_cmds = 0,
                                           line_num = 0,
                                                      tmp,
                                                      error;
    char           *ptr,
    buf[256],
    zname[256];
    char            s1[256];
    int             d[2];
    char *p;







    strcpy(zname, zonename);

    /* Skip first 3 lines lest we mistake the zone name for a command. */
    for (tmp = 0; tmp < 3; tmp++)
        get_line(fl, buf);

    /* More accurate count. Previous was always 4 or 5 too high. -gg 2001/1/17 */
    while (get_line(fl, buf) && (!area || strncmp(buf, ends, strlen(ends))))
        if ((strchr("MOGEPDRZ", buf[0]) && buf[1] == ' ') || (buf[0] == 'S' && buf[1] == '\0'))
            num_of_cmds++;

    //  while (get_line(fl, buf))
    //      num_of_cmds++;          /* this should be correct within 3 or so */
    rewind(fl);

    if (num_of_cmds == 0) {
        fprintf(stderr, "%s is empty!\n", zname);
        exit(0);
    } else
        CREATE(Z.cmd, struct reset_com, num_of_cmds);

    line_num += get_line(fl, buf);

    if (sscanf(buf, "#%d %s %d %d", &Z.number, s1, d, d + 1) == 1) {
        /* old style */
        Z.creator = strdup("Unknown");
        Z.lvl_low = 1;
        Z.lvl_high = LVL_IMMORT;
    } else {
        Z.creator = strdup(s1);
        Z.lvl_low = d[0];
        Z.lvl_high = d[1];
    }
    strcpy(Z.reset_msg, "You hear some squeaking sounds...");
    if (area)
    	strcpy(Z.disk_name, zonename);
    else 
    	*(Z.disk_name)=0;

    sprintf(buf2, "beginning of zone #%d", Z.number);


    line_num += get_line(fl, buf);
    if ((ptr = strchr(buf, '~')) != NULL)       /* take off the '~' if it's
                                                     * there */
        *ptr = '\0';
    Z.name = str_dup(buf);

    line_num += get_line(fl, buf);
    if (sscanf(buf, " %d %d %d", &Z.top, &Z.lifespan, &Z.reset_mode) != 3) {
        fprintf(stderr, "SYSERR: Format error in 3-constant line of %s", zname);
        exit(1);
    }
    cmd_no = 0;

    for (;;) {
        if ((tmp = get_line(fl, buf)) == 0) {
            fprintf(stderr, "Format error in %s - premature end of file\n", zname);
            exit(1);
        }
        line_num += tmp;
        ptr = buf;
        skip_spaces(&ptr);

        if ((ZCMD.command = *ptr) == '*')
            continue;



        ptr++;

        if (ZCMD.command =='#')
        {

            p=getst(ptr, 1, ' ');
            if (!strcasecmp(p, "author"))
                Z.creator = strdup(getst(ptr+7, 1, '~'));
            else if (!strcasecmp(p, "resetmsg"))
                strcpy(Z.reset_msg, getst(ptr+9, 1, '~'));
            else
            {
                fprintf(stderr, "Format error in %s, line %d: '%s'\n", zname, line_num, buf);
                exit(1);
            }
            continue;
        }

        if (ZCMD.command == 'S' || ZCMD.command == '$') {
            ZCMD.command = 'S';
            break;
        }
        error = 0;
        if (strchr("MOEPD", ZCMD.command) == NULL) {    /* a 3-arg command */
            if (sscanf(ptr, " %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2) != 3)
                error = 1;
        else if (ZCMD.command=='T')
      		continue;
        } else {

            if (sscanf(ptr, " %d %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2,
                       &ZCMD.arg3) != 4)
                error = 1;
        }

        ZCMD.if_flag = tmp;

        if (error) {
            fprintf(stderr, "Format error in %s, line %d: '%s'\n", zname, line_num, buf);
            exit(1);
        }
        ZCMD.line = line_num;
        cmd_no++;
    }

    top_of_zone_table = zone++;
}

#undef Z


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */



int             vnum_mobile(char *searchname, struct char_data * ch)
{
    int             nr,
    found = 0;

    for (nr = 0; nr <= top_of_mobt; nr++) {
        if (isname(searchname, mob_proto[nr].player.name)) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                    mob_index[nr].virtual,
                    mob_proto[nr].player.short_descr);
            send_to_char(buf, ch);
        }
    }

    return (found);
}



int             vnum_object(char *searchname, struct char_data * ch)
{
    int             nr,
    found = 0;

    for (nr = 0; nr <= top_of_objt; nr++) {
        if (isname(searchname, obj_proto[nr].name)) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                    obj_index[nr].virtual,
                    obj_proto[nr].short_description);
            send_to_char(buf, ch);
        }
    }
    return (found);
}


/* create a character, and add it to the char list */
struct char_data *create_char(void)
{
    struct char_data *ch;

    CREATE(ch, struct char_data, 1);
    clear_char(ch);
    ch->next = character_list;
    character_list = ch;

    return ch;
}

int fuzzy(int i)
{
    switch (number(1,4))
    {
    case 1:i--;break;
    case 4:i++;break;
    default:break;
    }
    return i;
}





/* create a new mobile from a prototype */
struct char_data *read_mobile(int nr, int type, int rznum)
{
    int             i, plev;
    struct char_data *mob;

    if (nr==-1)
        return NULL;
    if (type == VIRTUAL) {
        if ((i = real_mobile(nr)) < 0) {
            sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
            log(buf);
            return (NULL);
        }
    } else
        i = nr;

    CREATE(mob, struct char_data, 1);
    clear_char(mob);
    *mob = mob_proto[i];
    mob->next = character_list;
    character_list = mob;
          
    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);
  
    mob_index[i].number[rznum] += 1;
    mob->mob_specials.orig_zone = rznum;
    
    if (FOL_URG(mob))
    	{SET_STYLE(mob,2);}
    	
    my_srand(GET_MOB_VNUM(mob));    	
    plev=GET_LEVEL(mob);
    if (IS_SHOPKEEPER(mob))
    {   
    	
        mob->points.gold=0;
        SET_BIT(MOB_FLAGS(mob), MOB_SHOP);
        SET_BIT(MOB_FLAGS(mob), MOB_TALK);
    }
    
    	
    if (mob_proto[i].player.class!=CLASS_CLERIC)
        REMOVE_BIT(AFF_FLAGS(mob), AFF_SANCTUARY);
    if (MOB_FLAGGED(mob,MOB_MONK))
        GET_CLASS(mob)=CLASS_MONK;
        
    if (!IS_WARRIOR(mob) && !IS_SHOPKEEPER(mob))
        mob->points.gold=MAX(0, MAX(plev-3,fuzzy(fuzzy(plev)*(plev))/fuzzy(18)));
    
    if (mob->player.race==RACE_ANIMAL || mob->player.race==RACE_SMALL_ANIMAL)
    	mob->points.gold=0;
    
        
        
    if (IS_CASTER(mob))
    {
        mob->real_abils.wis=  MIN(25, mob->real_abils.wis + number(1, 3));
        mob->real_abils.intel=MIN(25, mob->real_abils.intel+number(1, 3));
    }
	
	
    my_srand(rand());    	
    return mob;
}


/* create an object, and add it to the object list */
struct obj_data *create_obj(void)
{
    struct obj_data *obj;

    CREATE(obj, struct obj_data, 1);
    clear_object(obj);
    obj->next = object_list;
    object_list = obj;

    return obj;
}



void adjust_object(struct obj_data *obj, int level)
{

    int i;
    float base;
    int pom;
    if (level==OBJ_PERM)
        return;

    GET_OBJ_LEVEL(obj)=level;
    GET_OBJ_DAMAGE(obj)=100;

    my_srand(GET_OBJ_VNUM(obj));

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0)) {
            
            //if (!number(0, 3))
              //      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
            
            if (obj->affected[i].location == APPLY_AC)
            {
                if (obj->affected[i].modifier>0)
                    obj->affected[i].modifier=MAX(1, MIN(fuzzy(level/4), obj->affected[i].modifier));
                else                                                                                 
                	obj->affected[i].modifier=MAX(1, MAX(fuzzy(-level/4), obj->affected[i].modifier));
                    
                //    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
            }
            else if (obj->affected[i].location == APPLY_HITROLL)
            {   
            	if (!CANBE_DAMAGE(obj))
            	{
            		obj->affected[i].location=APPLY_NONE;
            			obj->affected[i].modifier=0;
            	}
            	
                else if (obj->affected[i].modifier>0)
                    obj->affected[i].modifier=MAX(1, MIN(fuzzy(level/8), obj->affected[i].modifier));
                    
                  //  SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
            }
            else if (obj->affected[i].location == APPLY_DAMROLL)
            {
            	if (!CANBE_DAMAGE(obj)) 
            	{
            		obj->affected[i].location=APPLY_NONE;
            		obj->affected[i].modifier=0;
            	}
                else if (obj->affected[i].modifier>0)
                    obj->affected[i].modifier=MAX(1, MIN(fuzzy(level/8), obj->affected[i].modifier));
                    
//                    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
            } 
    		else if (obj->affected[i].location == APPLY_STR || obj->affected[i].location == APPLY_CON || obj->affected[i].location == APPLY_DEX || obj->affected[i].location == APPLY_WIS || obj->affected[i].location == APPLY_INT)
               {    int val=obj->affected[i].modifier; 
               	
               	    if (val>1)
               	    {                
			if (val%2)               	    	
			{
		    		val=(val-1)/2;        	    	
		    		if (CHANCE(2*level))
		    			val++;
		    		
		    	}
		    	else	
		    		val/=2;	   
		    		
		    	if (CHANCE(level))
		    			val++;	    		
		    	obj->affected[i].modifier=MAX(1, MIN(obj->affected[i].modifier,val));
		    			
               	    	
               	    }
               	
               	}

        }
    }




    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_WEAPON:
    case ITEM_FIREWEAPON:
        {
            GET_OBJ_VAL(obj, 0)=MAX(0, (fuzzy((level-3)/6)));
            GET_OBJ_VAL(obj, 1)=MAX(1, MIN(8,(fuzzy(level/5+1))));
            GET_OBJ_VAL(obj, 2)=MAX(1, (fuzzy(((18*level)/19-GET_OBJ_VAL(obj,0))*2/GET_OBJ_VAL(obj,1))));

            if (GET_OBJ_VAL(obj, 3) == TYPE_PIERCE - TYPE_HIT)
                GET_OBJ_VAL(obj, 2)=GET_OBJ_VAL(obj, 2)-1;   // daggers do less damage      
                
            if (IS_OBJ_STAT(obj, ITEM_2HANDED))
            {
                GET_OBJ_VAL(obj, 2)=GET_OBJ_VAL(obj, 2)+number(1,2);
                GET_OBJ_VAL(obj, 1)=GET_OBJ_VAL(obj, 1)+number(1,2);
                GET_OBJ_VAL(obj, 0)=GET_OBJ_VAL(obj, 0)+number(2,4);
                GET_OBJ_WEIGHT(obj)=MIN(64, 128*GET_OBJ_WEIGHT(obj)/100);
            }

            GET_OBJ_COST(obj)=WEAPON_COST(obj);

            break;
        }
    case ITEM_MISSILE:
        {
            GET_OBJ_VAL(obj, 0)=MAX(1, level/9);	//distance
            GET_OBJ_VAL(obj, 1)=MAX(1, MIN(8,fuzzy(fuzzy(level/8+1))));
            GET_OBJ_VAL(obj, 2)=MAX(1, fuzzy(((fuzzy(8)*level)/10)*2/GET_OBJ_VAL(obj,1)));
            GET_OBJ_COST(obj)=MAX(fuzzy(level)*level/24, (fuzzy(level)*(level)*(level)/(280)))/3+level;
            break;
        }
    case ITEM_ARMOR:
        {

            //GET_OBJ_VAL(obj, 0)=MAX(1, fuzzy(level/4));
            //GET_OBJ_COST(obj)=MAX(fuzzy(level)*level/32, (fuzzy(level)*(level)*(level)/(340)))+level+1;
            base=level/3.0+(level*level)/500.0;

            if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_BODY))
                base*=5;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_SHIELD))
                base*=4.2;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_LEGS) || IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_ARMS))
                base*=3.3;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_HEAD))
                base*=2.9;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_ABOUT))
                base*=1.4;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_FEET)  || IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_HANDS))
                base*=1.5;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_FACE))
                base*=1.6;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_BACK))
                base*=1.8;
            else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_WAIST))
                base*=1;
           // else if (IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_FINGER) || IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_EYES) || IS_SET(GET_OBJ_WEAR(obj), ITEM_WEAR_EARS))
              //  base*=0.8;
            pom=(int) base;
            pom=MAX(1, fuzper(pom, 5));

            GET_OBJ_VAL(obj, 0)=pom;
            GET_OBJ_VAL(obj, 1)=pom;	// for repairs

            GET_OBJ_COST(obj)=ARMOR_COST(obj);

            break;
        }
    case ITEM_SCROLL:
        GET_OBJ_VAL(obj, 0)=level;
        GET_OBJ_COST(obj)=MAX(fuzzy(level)*level/8, (fuzzy(level)*(level)*(level)/(300)))+level;
        break;
    case ITEM_WAND:
    case ITEM_POTION:
        GET_OBJ_VAL(obj, 0)=level;
        GET_OBJ_COST(obj)=MAX(fuzzy(level)*level/7, (fuzzy(level)*(level)*(level)/(300)))+level;
        break;
    case ITEM_STAFF:
        GET_OBJ_VAL(obj, 0)=level;
        GET_OBJ_COST(obj)=MAX(fuzzy(level)*level/6, (fuzzy(level)*(level)*(level)/(300)))+level;
        break;
    case ITEM_DRINKCON:
    case ITEM_FOOD:
        GET_OBJ_COST(obj)=MIN(10, GET_OBJ_VAL(obj, 0)/12);
        break;
    case ITEM_TREASURE:
        if (global_lwg)
            GET_OBJ_COST(obj)=MAX(0, MAX(level-3,fuzzy(fuzzy(level)*(level)/fuzzy(16))));
        else
            GET_OBJ_COST(obj)=MAX(fuzzy(level)*level/4, (fuzzy(level)*(level)*(level)/(180)))+2*level+1;
        GET_OBJ_VAL(obj, 0)=GET_OBJ_COST(obj);
        break;
    case ITEM_MONEY:
        if (global_lwg)
            GET_OBJ_COST(obj)=MAX(0, MAX(level-3,fuzzy(fuzzy(level)*(level)/fuzzy(16))));
        else
            GET_OBJ_COST(obj)=MAX(fuzzy(level)*level/4, (fuzzy(level)*(level)*(level)/(180)))+2*level+1;
        GET_OBJ_VAL(obj, 0)=GET_OBJ_COST(obj);
        break;
    case ITEM_CONTAINER:
        GET_OBJ_COST(obj)=MIN(25, GET_OBJ_VAL(obj, 0)/10);
        break;
    case ITEM_LIGHT:
        GET_OBJ_COST(obj)=MIN(25, GET_OBJ_VAL(obj, 2)/20);
        break;
    default:
        {
            GET_OBJ_COST(obj)=MAX(1, fuzzy(fuzzy(level)*fuzzy(level)*fuzzy(level)/700))+level/4;
            break;
        }
    }
    //	my_srand(pulse);
    //my_srand(time(0));
    
      my_srand(rand());
}


/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type, int rzone, int level)
{
    struct obj_data *obj;
    int             i;

    if (nr < 0) {
        log("SYSERR: trying to create obj with negative num!");
        return NULL;
    }
    if (type == VIRTUAL) {
        if ((i = real_object(nr)) < 0) {
            sprintf(buf, "SYSERR: Object (V) %d does not exist in database.", nr);
            log(buf);
            return NULL;
        }
    } else
        i = nr;



    CREATE(obj, struct obj_data, 1);

    clear_object(obj);
    *obj = obj_proto[i];
    obj->next = object_list;

    object_list = obj;

    //    sprintf(buf, "obj - %d", GET_OBJ_VNUM(obj));
    //log(buf);


    obj_index[i].number[rzone] += 1;
    obj->orig_zone = rzone;

    adjust_object(obj, level);
    if (GET_OBJ_TYPE(obj)==ITEM_WEAPON || GET_OBJ_TYPE(obj)==ITEM_FIREWEAPON || GET_OBJ_TYPE(obj)==ITEM_ARMOR)        
        if (!number(0, 26))        
        {
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_EGO);
            SET_BIT(GET_OBJ_EXTRA(obj), ITEM_HIDDEN_EGO);
        }
    return (obj);
}



#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void            zone_update(void)
{
    int             i;
    struct reset_q_element *update_u,
                *temp;
    static int      timer = 0;
    char            buf[128];

    /* jelson 10/22/92 */
if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60) {
        /* one minute has passed */
        /* NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or
         * a factor of 60 */

        timer = 0;

        /* since one minute has passed, increment zone ages */
        for (i = 0; i <= top_of_zone_table; i++) {
            if (zone_table[i].age < zone_table[i].lifespan &&
                    zone_table[i].reset_mode)
                (zone_table[i].age)++;

            if (zone_table[i].age >= zone_table[i].lifespan &&
                    zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
                /* enqueue zone */

                CREATE(update_u, struct reset_q_element, 1);

                update_u->zone_to_reset = i;
                update_u->next = 0;

                if (!reset_q.head)
                    reset_q.head = reset_q.tail = update_u;
                else {
                    reset_q.tail->next = update_u;
                    reset_q.tail = update_u;
                }

                zone_table[i].age = ZO_DEAD;
            }
        }
    }                           /* end - one minute has passed */
    /* dequeue zones (if possible) and reset */
    /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
    for (update_u = reset_q.head; update_u; update_u = update_u->next)
        if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
                is_empty(update_u->zone_to_reset)) {
            reset_zone(update_u->zone_to_reset);
            sprintf(buf, "Auto zone reset: %s",
                    zone_table[update_u->zone_to_reset].name);
            mudlog(buf, CMP, LVL_GOD, FALSE);
            /* dequeue */
            if (update_u == reset_q.head)
                reset_q.head = reset_q.head->next;
            else {
                for (temp = reset_q.head; temp->next != update_u;
                        temp = temp->next);

                if (!update_u->next)
                    reset_q.tail = temp;

                temp->next = update_u->next;
            }

            DISPOSE(update_u);
            break;
        }
}

void            log_zone_error(int zone, int cmd_no, char *message)
{
    char            buf[256];

    sprintf(buf, "ZONE: file %s", message);
    mudlog(buf, NRM, LVL_GOD, TRUE);

    sprintf(buf, "ZONE: ...offending cmd: '%c' cmd in zone #%d, line %d",
            ZCMD.command, zone_table[zone].number, ZCMD.line);
    mudlog(buf, NRM, LVL_GOD, TRUE);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = 0; }


extern int rev_dir[];
void make_maze(int zone)
{
    int card[MAZEU], temp, x, y, dir, room;
    int num, next_room = 0, test, r_back;
    int vnum = zone_table[zone].number;

    for (test = 0; test < MAZEU; test++) {
        card[test] = test;
        temp = test;
        dir = temp / (MAZEW*MAZEW);
        temp = temp - (dir * MAZEW*MAZEW);
        x = temp / MAZEW;
        temp = temp - (x * MAZEW);
        y = temp;
        room = (vnum * 100) + (x * MAZEW) + y;
        room = real_room(room);
        if ((x == 0) && (dir == 0))
            continue;
        if ((y == MAZEW-1) && (dir == 1))
            continue;
        if ((x == MAZEW-1) && (dir == 2))
            continue;
        if ((y == 0) && (dir == 3))
            continue;
        world[room].dir_option[dir]->to_room = -1;
        REMOVE_BIT(ROOM_FLAGS(room), ROOM_NOTRACK);
    }
    for (x = 0; x < MAZEU-1; x++) {
        y = number(0, MAZEU-1);
        temp = card[y];
        card[y] = card[x];
        card[x] = temp;
    }

    for (num = 0; num < MAZEU; num++) {
        temp = card[num];
        dir = temp / (MAZEW*MAZEW);
        temp = temp - (dir * MAZEW*MAZEW);
        x = temp / MAZEW;
        temp = temp - (x * MAZEW);
        y = temp;
        room = (vnum * 100) + (x * MAZEW) + y;
        r_back = room;
        room = real_room(room);
        if ((x == 0) && (dir == 0))
            continue;
        if ((y == MAZEW-1) && (dir == 1))
            continue;
        if ((x == MAZEW-1) && (dir == 2))
            continue;
        if ((y == 0) && (dir == 3))
            continue;
        if (world[room].dir_option[dir]->to_room != -1)
            continue;
        switch(dir) {
        case 0:
            next_room = r_back - MAZEW;
            break;
        case 1:
            next_room = r_back + 1;
            break;
        case 2:
            next_room = r_back + MAZEW;
            break;
        case 3:
            next_room = r_back - 1;
            break;
        }
        next_room = real_room(next_room);
        test = find_first_step(room, next_room);
        switch (test) {
        case BFS_ERROR:
            log("Maze making error.");
            break;
        case BFS_ALREADY_THERE:
            log("Maze making error.");
            break;
        case BFS_NO_PATH:
            world[room].dir_option[dir]->to_room = next_room;
            world[next_room].dir_option[(int) rev_dir[dir]]->to_room = room;
            break;
        }
    }
    for (num = 0;num < MAZEW*MAZEW;num++) {
        room = (vnum * 100) + num;
        room = real_room(room);
        /* Remove the next line if you want to be able to track your way through
        the maze */
        SET_BIT(ROOM_FLAGS(room), ROOM_NOTRACK);
        SET_BIT(ROOM_FLAGS(room), ROOM_ARENA);
        REMOVE_BIT(ROOM_FLAGS(room), ROOM_BFS_MARK);
        DISPOSE(world[room].name);
        DISPOSE(world[room].description);
        world[room].name=str_dup("Arena Maze");
        world[room].description=str_dup("You are surrounded by cold arena walls. It's just you and your enemies here.\r\n");
    }
    room=real_room(vnum*100);
    SET_BIT(ROOM_FLAGS(room), ROOM_RED_BASE);
    DISPOSE(world[room].name);
    DISPOSE(world[room].description);
    world[room].name=str_dup("&RRED BASE&0");
    world[room].description=str_dup("This is the &RRED BASE&0. You can drop the blue flag here.\r\n");
    room=real_room(vnum*100+MAZEW*MAZEW-1);
    SET_BIT(ROOM_FLAGS(room), ROOM_BLUE_BASE);
    DISPOSE(world[room].name);
    DISPOSE(world[room].description);
    world[room].name=str_dup("&BBLUE BASE&0");
    world[room].description=str_dup("This is the &BBLUE BASE&0. You can drop the red flag here.\r\n");
}
extern int no_more;

/* execute the reset command table of a given zone */
void            reset_zone(int zone)
{
    int             cmd_no,
    last_cmd = 0, pom, i;
    float cst=1;
    struct char_data *mob = NULL;
    struct obj_data *obj,
                *obj_to, *lastobj;
    struct descriptor_data *d;
    int lwg=0;
    int level=0, err=0;

for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {

        if (ZCMD.if_flag<=1)
        {if (ZCMD.if_flag==1 && !last_cmd)
            continue;
        }
        else if (!CHANCE(ZCMD.if_flag))
            continue;

        switch (ZCMD.command) {
        case '*':               /* ignore command */
            last_cmd = 0;
            lwg=0;;
            break;
        case 'Z':
            make_maze(zone);
            lwg=0;;

            break;

        case 'M':               /* read a mobile */
            if (mob_index[ZCMD.arg1].number[zone] < (ZCMD.arg2>0 ? ZCMD.arg2:MAX_MOBS_PER_ROOM)) {
                mob = read_mobile(ZCMD.arg1, REAL, zone);
                char_to_room(mob, ZCMD.arg3);
                last_cmd = 1;
                level=GET_LEVEL(mob);
            } else
            {
                if (boottime)
                    ZONE_ERROR("Mob couldnt be spawned");
                last_cmd = 0;
            }

            lwg=0;;
            break;

        case 'O':               /* read an object */
            //if (!no_more)
            if (obj_index[ZCMD.arg1].number[zone] < (ZCMD.arg2>0 ? ZCMD.arg2:MAX_OBJS_PER_ROOM)) {
                if (ZCMD.arg3 >= 0) {
                    //if (!level)
                    //ZONE_ERROR("LEVEL==0");
                    err=0;


                    if (obj_proto[ZCMD.arg1].obj_flags.data>0)
                        level=obj_proto[ZCMD.arg1].obj_flags.data;
                    else
                    {
                        level=zone_table[zone].avg;err=1;

                    }

                    obj = read_object(ZCMD.arg1, REAL, zone, level);
                    if (err)// && (GET_OBJ_TYPE(obj)==ITEM_ARMOR || GET_OBJ_TYPE(obj)==ITEM_WEAPON || GET_OBJ_TYPE(obj)==ITEM_FIREWEAPON || GET_OBJ_TYPE(obj)==ITEM_MISSILE))
                    {
                        //fprintf(stderr, "Obj on ground: #%d %s in room %d assigned level %d\r\n",GET_OBJ_VNUM(obj), obj->short_description, world[ZCMD.arg3].number, level);
                    }
                    if (GET_OBJ_TYPE(obj)==ITEM_MONEY)
                        ;//fprintf(stderr, "Error: Money on ground: #%d %s in room %d assigned level %d\n", GET_OBJ_VNUM(obj), obj->short_description, world[ZCMD.arg3].number,  zone_table[zone].avg);
                    else
                    {
                    	global_no_timer=1;
                        obj_to_room(obj, ZCMD.arg3);
                        global_no_timer=0;
                     }
                    lastobj=obj;
                    last_cmd = 1;
                } else {
                    /*
                    obj = read_object(ZCMD.arg1, REAL, zone, level);
                    obj->in_room = NOWHERE;*/
                    last_cmd = 1;
                }
            } else{
                last_cmd = 0;
                if (boottime)
                    ZONE_ERROR("Obj couldnt be spawned");
            }
            lwg=0;;
            break;

        case 'P':               /* object to object */
            //if (!no_more)
            if (obj_index[ZCMD.arg1].number[zone] < (ZCMD.arg2>0 ? ZCMD.arg2:MAX_OBJS_PER_ROOM)) {
                err=0;
                if (obj_proto[ZCMD.arg1].obj_flags.data>0)
                    level=obj_proto[ZCMD.arg1].obj_flags.data;
                else
                {
                    if (!lwg)
                    {level=zone_table[zone].avg;err=1;}

                }
                if (lwg)
                    global_lwg=1;
                obj = read_object(ZCMD.arg1, REAL, zone, level);
                global_lwg=0;
                if (err && (GET_OBJ_TYPE(obj)==ITEM_ARMOR || GET_OBJ_TYPE(obj)==ITEM_WEAPON || GET_OBJ_TYPE(obj)==ITEM_FIREWEAPON || GET_OBJ_TYPE(obj)==ITEM_MISSILE))
                {
                    //		fprintf(stderr,"Put into: #%d %s put into %s, assigned level %d\r\n", GET_OBJ_VNUM(obj), obj->short_description, obj_to->short_description, level);
                }

                if (!ZCMD.arg3)	// smaug case, defaults to last obj loaded
                {
                    int iNest;

                    if ( !lastobj )
                    {
                        fprintf(stderr,"!last obj %d\r\n",ZCMD.arg1);
                        exit (0);
                    }

                    obj_to=lastobj;
                }
                else     if (!(obj_to = get_obj_num(ZCMD.arg3))) {
                    ZONE_ERROR("target obj not found, command disabled");
                    ZCMD.command = '*';
                    break;
                }
                //else
                obj_to_obj(obj, obj_to);
                last_cmd = 1;
            } else
            {
                last_cmd = 0;
                if (boottime)
                    ZONE_ERROR("obj couldnt be put");
            }
            lwg=0;;
            break;

        case 'G':               /* obj_to_char */
            if (!mob) {
                ZONE_ERROR("attempt to give obj to non-existant mob");
                ZCMD.command = '*';
                lwg=0;
                break;
            }

            
            // if (!no_more)
            if (obj_index[ZCMD.arg1].number[zone] < (ZCMD.arg2>0 ? ZCMD.arg2:MAX_OBJS_PER_ROOM) || mob_index[GET_MOB_RNUM(mob)].func == shop_keeper) {
//            	if (1) {
                //if (!level)
                //ZONE_ERROR("LEVEL==0");


                obj = read_object(ZCMD.arg1, REAL, zone, level);

                if (mob_index[GET_MOB_RNUM(mob)].func == shop_keeper)
                {
                    int olevel;
                    cst=1;
                    my_srand(GET_OBJ_VNUM(obj));

                    switch (GET_OBJ_TYPE(obj))
                    {

                    case ITEM_WEAPON:
                    case ITEM_FIREWEAPON:
                    case ITEM_MISSILE:
                    case ITEM_ARMOR:
              //          olevel=fuzzy(number(6, 8));
              		olevel=level;
              		cst*=0.85;
                        break;
                    case ITEM_SCROLL:
                        olevel=fuzzy(number(10, 15));
                        cst=1.4;
                        break;

                    case ITEM_POTION:
                    case ITEM_WAND:
                        olevel=fuzzy(number(10, 15));
                        cst=1.5;
                        break;
                    case ITEM_STAFF:
                        olevel=fuzzy(number(10, 15));
                        cst=1.6;
                        break;
                    case ITEM_FOOD:
                    case ITEM_DRINKCON:
                    case ITEM_CONTAINER:
                    case ITEM_LIGHT:
                        olevel=number(1,5);
                        break;
                    case ITEM_TREASURE:
                        olevel=number(4, 6);
                        break;
                    default:
                        {
                            olevel=number(7, 12);
                            //				fprintf(stderr, "Illegal TO SHOP: %s to %s\n", obj->short_description,  GET_NAME(mob));
                            break;
                        }
                    }


                    extract_obj(obj);

                    obj = read_object(ZCMD.arg1, REAL, zone, olevel);
                    obj_index[ZCMD.arg1].number[zone]-=1;

                    if (IS_SET(GET_OBJ_EXTRA(obj), ITEM_HIDDEN_EGO))
                    {
                        REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_HIDDEN_EGO);
                        REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_EGO);
                    }
                    GET_OBJ_COST(obj)*=cst;
                    SET_BIT(GET_OBJ_EXTRA2(obj), ITEM2_SHOPINV);
                }
                 if (IS_SET(GET_OBJ_EXTRA(obj), ITEM_HIDDEN_EGO) && !IS_WARRIOR(mob))      //ego
        	    make_ego(obj, GET_OBJ_LEVEL(obj)); 
                
                obj_to_char(obj, mob);
                last_cmd = 1;
            } else
            {
            	if (boottime)
                ZONE_ERROR("Obj couldnt be given");
                last_cmd = 0;
            }
            lwg=1;
            break;

        case 'E':               /* object to equipment list */
            if (!mob) {
                ZONE_ERROR("trying to equip non-existant mob");
                break;
            }
//            if (obj_index[ZCMD.arg1].number[zone] < (ZCMD.arg2>0 ? ZCMD.arg2:100)) {
            	if (1) {
                if (ZCMD.arg3 < 0 || ZCMD.arg3 >= NUM_WEARS) {
                    ZONE_ERROR("invalid equipment pos number");
                } else {
                    //if (!level)
                    //ZONE_ERROR("LEVEL==0");
                    if (GET_EQ(mob, ZCMD.arg3))
                    {
                    	last_cmd=0;
                    	break;
                    }
                    obj = read_object(ZCMD.arg1, REAL, zone, level);
                    if (IS_SET(GET_OBJ_EXTRA(obj), ITEM_HIDDEN_EGO)  && !IS_WARRIOR(mob))      //ego
        		    make_ego(obj, GET_OBJ_LEVEL(obj)); 
                
                    equip_char(mob, obj, ZCMD.arg3);
                    if (ZCMD.arg3==WEAR_WIELD && (GET_OBJ_VAL(obj, 3) == TYPE_PIERCE - TYPE_HIT) && GET_CLASS(mob)==CLASS_WARRIOR && GET_CLASS(mob)!=CLASS_THIEF)
                    {
                        GET_CLASS(mob)=CLASS_THIEF;
                        GET_DEX(mob)+=number(1, 3);
                    }  
                    
                    if (ZCMD.arg3==WEAR_WIELD && mob->points.gold==0)
                        mob->points.gold=MAX(0, MAX(level-3,fuzzy(fuzzy(level)*(level))/fuzzy(18)));
                        
                    last_cmd = 1;
                }
            } else
            {
                if (boottime)
                    ZONE_ERROR("Obj couldnt be equiped");
                last_cmd = 0;
            }
            lwg=1;
            break;

        case 'R':               /* rem obj from room */
            if ((obj = get_obj_in_list_num(ZCMD.arg2, world[ZCMD.arg1].contents)) != NULL) {
                obj_from_room(obj);
                extract_obj(obj);
            }
            //            else
            //          	ZONE_ERROR("Remove obj!");
            last_cmd = 1;
            lwg=0;;
            break;


        case 'D':               /* set state of door */
            if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
                    (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
                ZONE_ERROR("door does not exist");
            } else
                switch (ZCMD.arg3) {
                case 0:
                    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                               EX_LOCKED);
                    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                               EX_CLOSED);
                    break;
                case 1:
                    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                            EX_CLOSED);
                    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                               EX_LOCKED);
                    break;
                case 2:
                    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                            EX_LOCKED);
                    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                            EX_CLOSED);
                    break;
                }
            last_cmd = 1;
            lwg=0;;
            break;

        default:
            ZONE_ERROR("unknown cmd in reset table; cmd disabled");
            ZCMD.command = '*';
            break;
        }
    }




    sprintf(buf, "%s\r\n", zone_table[zone].reset_msg);
    for (d = descriptor_list; d; d = d->next)
        if (!d->connected && d->character && AWAKE(d->character) &&
                world[(d->character)->in_room].zone==zone )
            SEND_TO_Q(buf, d);

    zone_table[zone].age = 0;
}



/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int             is_empty(int zone_nr)
{
    struct descriptor_data *i;

    for (i = descriptor_list; i; i = i->next)
        if (!i->connected)
            if (world[i->character->in_room].zone == zone_nr)
                return 0;

    return 1;
}





/*************************************************************************
*  stuff related to the save/load player system				 *
*********************************************************************** */


long            get_id_by_name(char *name)
{
    int             i;

    one_argument(name, arg);
    for (i = 0; i <= top_of_p_table; i++)
        if (!strcmp((player_table + i)->name, arg))
            return ((player_table + i)->id);

    return -1;
}


char           *get_name_by_id(long id)
{
    int             i;

    for (i = 0; i <= top_of_p_table; i++)
        if ((player_table + i)->id == id)
            return ((player_table + i)->name);

    return NULL;
}


/* Load a char, TRUE if loaded, FALSE if not */
int             load_char(char *name, struct char_file_u * char_element)
{
    int             player_i,rr;
    char buf[100];
    int             find_name(char *name);

    if ((player_i = find_name(name)) >= 0) {
        fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
        fread(char_element, sizeof(struct char_file_u), 1, player_fl);
        rr=real_room(char_element->player_specials_saved.load_room);
        if (rr<0 || char_element->player_specials_saved.load_room==0 || char_element->player_specials_saved.load_room==1)
        {
            //          sprintf(buf,"SYSERR: load_char(), %s in real: %d, virtual: %d",char_element->name, rr, char_element->player_specials_saved.load_room);
            //            log(buf);
            rr=real_room(r_mortal_start_room);
        }
        char_element->player_specials_saved.load_room=rr;
        return (player_i);
    } else
        return (-1);
}




/* write the vital data of a player to the player file */
void            save_char(struct char_data * ch, sh_int load_room)
{
    struct char_file_u st;

    if (IS_NPC(ch) || !ch->desc || GET_PFILEPOS(ch) < 0)// || IN_ARENA(ch))
        return;

    write_aliases(ch);



    if (load_room!=NOWHERE && !IN_ARENA(ch) && world[load_room].number!=0 && world[load_room].number!=1)
    {
        st.player_specials_saved.load_room = world[load_room].number;
        ch->player_specials->saved.load_room = world[load_room].number;
    }
    char_to_store(ch, &st);

    strncpy(st.host, ch->desc->host, HOST_LENGTH);
    st.host[HOST_LENGTH] = '\0';

    /*//  if (!PLR_FLAGGED(ch, PLR_LOADROOM))*/
    /*  //  if (load_room > 1 && (!AFF_FLAGGED(ch, AFF_VISITING))) {*/

    /*  //  }*/


    fseek(player_fl, GET_PFILEPOS(ch) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
    fflush(player_fl);
    if (load_room!=NOWHERE && world[load_room].number!=0 && world[load_room].number!=1)
    {
        ch->player_specials->saved.load_room=load_room;
    }

}



/* copy data from the file structure to a char struct */
void            store_to_char(struct char_file_u * st, struct char_data * ch)
{
    int             i;

    /* to save memory, only PC's -- not MOB's -- have player_specials */
    if (ch->player_specials == NULL)
        CREATE(ch->player_specials, struct player_special_data, 1);

    GET_SEX(ch) = st->sex;
    GET_CLASS(ch) = st->class;
    GET_LEVEL(ch) = st->level;
    GET_RACE(ch) = st->race;

    ch->player.short_descr = NULL;
    ch->player.long_descr = NULL;
    ch->player.title = str_dup(st->title);
    ch->player.description = str_dup(st->description);

    ch->player.hometown = st->hometown;
    ch->player.time.birth = st->birth;
    ch->player.time.played = st->played;
    ch->player.time.logon = time(0);

    ch->player.weight = st->weight;
    ch->player.height = st->height;

    ch->real_abils = st->abilities;
    ch->aff_abils = st->abilities;
    ch->points = st->points;
    ch->char_specials.saved = st->char_specials_saved;
    ch->player_specials->saved = st->player_specials_saved;
    POOFIN(ch) = NULL;
    POOFOUT(ch) = NULL;

    //    if (ch->points.max_mana < 100)
    //      ch->points.max_mana = 100;

    ch->player.obj_buffer = NULL;
    ch->player.mob_buf = NULL;
    ch->player.zone_edit = NOWHERE;

    ch->char_specials.carry_weight = 0;
    ch->char_specials.equip_weight = 0;
    ch->char_specials.carry_items = 0;
    ch->points.armor = 0;
    ch->points.magic_armor = 0;
    ch->points.hitroll = 0;
    ch->points.damroll = 0;

    CREATE(ch->player.name, char, strlen(st->name) + 1);
    strcpy(ch->player.name, st->name);
    strcpy(ch->player.passwd, st->pwd);

    /* Add all spell effects */
    /*    for (i = 0; i < MAX_AFFECT; i++) {
            if (st->affected[i].type)
                affect_to_char(ch, &st->affected[i]);
        }
    */
    ch->in_room = GET_LOADROOM(ch);
    GET_HIT(ch)=ch->points.hit;
    //affect_total(ch);

    /* If you're not poisioned and you've been away for more than an hour,
     * we'll set your HMV back to full */

    if (!IS_AFFECTED(ch, AFF_POISON) &&
            (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR)) {
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
    }
}                               /* store_to_char */




/* copy vital data from a players char-structure to the file structure */
extern int atot;
void            char_to_store(struct char_data * ch, struct char_file_u * st)
{
    int             i;
    struct affected_type *af;
    struct obj_data *char_eq[NUM_WEARS];

    /* Unaffect everything a character can be affected by */
        atot=1;
        for (i = 0; i < NUM_WEARS; i++) {
            if (GET_EQ(ch, i))
                char_eq[i] = unequip_char(ch, i);
            else
                char_eq[i] = NULL;
        }

        for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
            if (af) {
                st->affected[i] = *af;
                st->affected[i].next = 0;
                af = af->next;
            } else {
                st->affected[i].type = 0;   // Zero signifies not used 
                st->affected[i].duration = 0;
                st->affected[i].modifier = 0;
                st->affected[i].location = 0;
                st->affected[i].bitvector = 0;
                st->affected[i].bitvector2 = 0;
                st->affected[i].bitvector3 = 0;
                st->affected[i].next = 0;
            }
        }
        
    
    /* remove the affections so that the raw values are stored; otherwise the
     * effects are doubled when the char logs back in. */

    while (ch->affected)
    affect_remove(ch, ch->affected);

       if ((i >= MAX_AFFECT) && af && af->next)
          log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

      ch->aff_abils = ch->real_abils;

    st->birth = ch->player.time.birth;
    st->played = ch->player.time.played;
    st->played += (long) (time(0) - ch->player.time.logon);
    st->last_logon = time(0);

    ch->player.time.played = st->played;
    ch->player.time.logon = time(0);

    st->hometown = ch->player.hometown;
    st->weight = GET_WEIGHT(ch);
    st->height = GET_HEIGHT(ch);
    st->sex = GET_SEX(ch);
    st->class = GET_CLASS(ch);
    st->level = GET_LEVEL(ch);
    st->race = GET_RACE(ch);
    st->abilities = ch->real_abils;
    st->points = ch->points;
    //    st->points.hit=(int) ch->vitality;
    st->char_specials_saved = ch->char_specials.saved;
    st->char_specials_saved.affected_by=0;
    st->char_specials_saved.affected_by2=0;
    st->char_specials_saved.affected_by3=0;
    st->player_specials_saved = ch->player_specials->saved;


    st->points.armor = 0;
    st->points.magic_armor = 0;
    st->points.hitroll = 0;
    st->points.damroll = 0;

    if (GET_TITLE(ch))
        strcpy(st->title, GET_TITLE(ch));
    else
        *st->title = '\0';

    if (ch->player.description)
        strcpy(st->description, ch->player.description);
    else
        *st->description = '\0';

    strcpy(st->name, GET_NAME(ch));
    strcpy(st->pwd, GET_PASSWD(ch));

    /* add spell and eq affections back in now */
    for (i = 0; i < MAX_AFFECT; i++) {
        if (st->affected[i].type)
            affect_to_char(ch, &st->affected[i]);
}

    for (i = 0; i < NUM_WEARS; i++) {
        if (char_eq[i])
            equip_char(ch, char_eq[i], i);
}

    affect_total(ch);// unnecessary, I think !?!
    atot=0;
}                               /* Char to store */



void            save_etext(struct char_data * ch)
{
    /* this will be really cool soon */

}


/* create a new entry in the in-memory index table for the player file */
int             create_entry(char *name)
{
    int             i;

    if (top_of_p_table == -1) {
        CREATE(player_table, struct player_index_element, 1);
        top_of_p_table = 0;
    } else if (!(player_table = (struct player_index_element *)
                                realloc(player_table, sizeof(struct player_index_element) *
                                        (++top_of_p_table + 1)))) {
        perror("create entry");
        exit(1);
    }
    CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

    /* copy lowercase equivalent of name to table field */
    for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
            i++);

    return (top_of_p_table);
}



/************************************************************************
*  funcs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char           *fread_string2(FILE * fl, char *error)
{
    char            buf[MAX_STRING_LENGTH],
    tmp[512],
    *rslt;
    register char  *point;
    int             done = 0,
                           length = 0,
                                    templength = 0;

    *buf = '\0';

    do {
        if (!fgets(tmp, 512, fl)) {
            fprintf(stderr, "SYSERR: fread_string: format error at or near %s\n",
                    error);
            exit(1);
        }
        /* If there is a '~', end the string; else put an "\r\n" over the
         * '\n'. */
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

        if (length + templength >= MAX_STRING_LENGTH) {
            log("SYSERR: fread_string: string too large (db.c)");
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
/*
 * Read a string from file fp
 */
char *fread_string( FILE *fp, char *error)
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( feof(fp) )
        {
            fprintf(stderr, "SYSERR: fread_string: format error at or near %s\n",
                    error);
            exit(1);

        }
        c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
        return NULL;//str_dup( "" );

    for ( ;; )
    {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
            fprintf(stderr, "SYSERR: fread_string: string too long near %s\n",
                    error);
            *plast = '\0';
            if (strlen(buf)>0)
                return str_dup(buf);
            else
                return NULL;
        }
        switch ( *plast = getc( fp ) )
        {
        default:
            plast++; ln++;
            break;

        case EOF:
            fprintf(stderr, "SYSERR: fread_string: eof near %s\n",
                    error);
            *plast = '\0';
            if (strlen(buf)>0)
                return str_dup(buf);
            else
                return NULL;
            break;

        case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            if (strlen(buf)>0)
                return str_dup(buf);
            else
                return NULL;
        }
    }
}


/* release memory allocated for a char struct */
void            free_char(struct char_data * ch)
{
    int             i;
    struct alias   *a;
    struct quest   *q;

    void            free_alias(struct alias * a);
    void            free_quest(struct quest * q);

    /*//if (WRATHOBJ(ch)) extract_obj(WRATHOBJ(ch));*/
    if (ch->player_specials != NULL && ch->player_specials != &dummy_mob) {
        if (ch->player_specials->poofin)
            DISPOSE(ch->player_specials->poofin);
        if (ch->player_specials->poofout)
            DISPOSE(ch->player_specials->poofout);
        while ((a = GET_ALIASES(ch)) != NULL) {
            GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
            free_alias(a);
        }
        while ((q = GET_QUESTS(ch)) != NULL) {
            GET_QUESTS(ch) = (GET_QUESTS(ch))->next;
            free_quest(q);
        }

        DISPOSE(ch->player_specials);
        if (IS_NPC(ch))
            log("SYSERR: Mob had player_specials allocated!");
    }
    if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
        /* if this is a player, or a non-prototyped non-player, free all */
        if (GET_NAME(ch))
        {
            if (IS_NPC(ch))
             DISPOSE((ch)->player.short_descr); else DISPOSE((ch)->player.name);
        }    
            
        if (ch->player.title)
            DISPOSE(ch->player.title);
        if (ch->player.short_descr)
            DISPOSE(ch->player.short_descr);
        if (ch->player.long_descr)
            DISPOSE(ch->player.long_descr);
        if (ch->player.description)
            DISPOSE(ch->player.description);
    } else if ((i = GET_MOB_RNUM(ch)) > -1) {
        /* otherwise, free strings only if the string is not pointing at
         * proto */
        if (ch->player.name && (ch->player.name != mob_proto[i].player.name))
            DISPOSE(ch->player.name);
        if (ch->player.title && (ch->player.title != mob_proto[i].player.title))
            DISPOSE(ch->player.title);
        if (ch->player.short_descr && (ch->player.short_descr != mob_proto[i].player.short_descr))
            DISPOSE(ch->player.short_descr);
        if (ch->player.long_descr && (ch->player.long_descr != mob_proto[i].player.long_descr))
            DISPOSE(ch->player.long_descr);
        if (ch->player.description && (ch->player.description != mob_proto[i].player.description))
            DISPOSE(ch->player.description);
    }
    while (ch->affected)
        affect_remove(ch, ch->affected);

    if (ch->hostname)
        DISPOSE(ch->hostname);
    if (ch->ambush_name)
        DISPOSE(ch->ambush_name);
    if (ch->wait_buffer)
        DISPOSE(ch->wait_buffer);

    for (i=0; i<MAX_TELLS;i ++)
        if (ch->last_tells[i])
            DISPOSE(ch->last_tells[i]);

    ch->in_room = NOWHERE;
    DISPOSE(ch);

    ch = NULL;
}


void            free_room(struct room_data * room)
{
    struct extra_descr_data *this,
                *next_one;
    int             counter;
    if (room->name)
        DISPOSE(room->name);
    if (room->description)
        DISPOSE(room->description);
for (counter = 0; counter < NUM_OF_DIRS; counter++) {
        if (room->dir_option[counter]) {
            if (room->dir_option[counter]->general_description)
                DISPOSE(room->dir_option[counter]->general_description);
            if (room->dir_option[counter]->keyword)
                DISPOSE(room->dir_option[counter]->keyword);
            DISPOSE(room->dir_option[counter]);
        }

    }
    if (room->ex_description)
        for (this = room->ex_description; this; this = next_one) {
            next_one = this->next;
            if (this->keyword)
                DISPOSE(this->keyword);
            if (this->description)
                DISPOSE(this->description);
            DISPOSE(this);
        }
    /*    if (room->broad)
            DISPOSE(room->broad);
        if (room->tele)
            DISPOSE(room->tele);*/
}

/* release memory allocated for an obj struct */
void            free_obj(struct obj_data * obj)
{
    int             nr;
    struct extra_descr_data *this,
                *next_one;

if ((nr = GET_OBJ_RNUM(obj)) == -1) {
        if (obj->name)
            DISPOSE(obj->name);
        if (obj->description)
            DISPOSE(obj->description);
        if (obj->short_description)
            DISPOSE(obj->short_description);
        if (obj->action_description)
            DISPOSE(obj->action_description);
        if (obj->attack_verb)
            DISPOSE(obj->attack_verb);
        if (obj->ex_description)
            for (this = obj->ex_description; this; this = next_one) {
                next_one = this->next;
                if (this->keyword)
                    DISPOSE(this->keyword);
                if (this->description)
                    DISPOSE(this->description);
                DISPOSE(this);
            }
    } else {
        if (obj->name && obj->name != obj_proto[nr].name)
            DISPOSE(obj->name);
        if (obj->description && obj->description != obj_proto[nr].description)
            DISPOSE(obj->description);
        if (obj->short_description && obj->short_description != obj_proto[nr].short_description)
            DISPOSE(obj->short_description);
        if (obj->action_description && obj->action_description != obj_proto[nr].action_description)
            DISPOSE(obj->action_description);
        if (obj->attack_verb && obj->attack_verb != obj_proto[nr].attack_verb)
            DISPOSE(obj->attack_verb);
        if (obj->ex_description && obj->ex_description != obj_proto[nr].ex_description)
            for (this = obj->ex_description; this; this = next_one) {
                next_one = this->next;
                if (this->keyword)
                    DISPOSE(this->keyword);
                if (this->description)
                    DISPOSE(this->description);
                DISPOSE(this);
            }
    }

    DISPOSE(obj);
}



/* read contets of a text file, alloc space, point buf to it */
int             file_to_string_alloc(char *name, char **buf)
{
    char            temp[MAX_STRING_LENGTH];

    if (file_to_string(name, temp) < 0)
        return -1;

    if (*buf)
        DISPOSE(*buf);

    *buf = str_dup(temp);

    return 0;
}



/* read contents of a text file, and place in buf */
int             file_to_string(char *name, char *buf)
{
    FILE           *fl;
    char            tmp[MAX_STRING_LENGTH];
    char            tname[80];   
     int len;

    *buf = '\0';
    *tname = '\0';

     if (!(fl = fopen(name, "r"))) {
    logs("SYSERR: reading %s: %s", name, strerror(errno));
    return (-1);
}   

    if (feof(fl)) {             /* reditmod* if empty file, load the backup */
        fclose(fl);
        fprintf(stderr, "olc: empty file %s, using backup", name);
        sprintf(tname, "%s.back", name);
        remove(name);
        rename(tname, name);
        if (!(fl = fopen(name, "r"))) {
            sprintf(tmp, "Error reading %s", name);
            perror(tmp);
            return (-1);
        }
    }
   /* do {
        fgets(tmp, MAX_STRING_LENGTH, fl);
        tmp[strlen(tmp) - 1] = '\0';    
        strcat(tmp, "\r\n");

        if (!feof(fl)) {
            if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH) {
                log("SYSERR: fl->strng: string too big (db.c, file_to_string)");
                *buf = '\0';
                return (-1);
            }
            strcat(buf, tmp);
        }
    } while (!feof(fl));*/
    
    for (;;) {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))	/* EOF check */
      break;
    if ((len = strlen(tmp)) > 0)
      tmp[len - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");

    if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH) {
      logs("SYSERR: %s: string too big (%d max)", name, MAX_STRING_LENGTH);
      *buf = '\0';
      fclose(fl);
      return (-1);
    }
    strcat(buf, tmp);
  }

    fclose(fl);

    return (0);
}

int             read_file_to_32strings_alloc(char *filename, char **buf)
{
    int             counter = 0;

    while (counter < 32 && *buf && *buf[counter])
        DISPOSE(buf[counter]);

    if (read_file_to_32strings(filename, buf) < 0)
        return -1;

    return 0;
}


int             read_file_to_32strings(char *filename, char **buf)
{
    FILE           *file;
    char            temp[256];
    int             counter = 0;

    if (!(file = fopen(filename, "r"))) {
        sprintf(temp, "Error reading %s", filename);
        perror(temp);
        return (-1);
    }
    while (counter == 0 && !feof(file)) {
        get_line(file, temp);
        buf[counter] = strdup(temp);
        counter++;
    }
    while (counter < 32) {
        sprintf(temp, "NONE");
        buf[counter] = strdup(temp);
        counter++;
    }
    return 0;
}

/* clear some of the the working variables of a char */
void            reset_char(struct char_data * ch)
{
    int             i;

    if (IS_NPC(ch) || !IS_SET(PLR_FLAGS(ch), PLR_JUSTDIED))
    {
        for (i = 0; i < NUM_WEARS; i++)
            GET_EQ(ch, i) = NULL;

        ch->char_specials.equip_weight = 0;
    }
    ch->carrying = NULL;
    ch->char_specials.carry_weight = 0;
    ch->char_specials.carry_items = 0;

    ch->followers = NULL;
    ch->master = NULL;
    /* ch->in_room = NOWHERE; Used for start in room */

    ch->next = NULL;
    ch->next_fighting = NULL;
    ch->next_in_room = NULL;
    /*//  if (WRATHOBJ(ch)!=NULL) DISPOSE(WRATHOBJ(ch));*/
    FIGHTING(ch) = NULL;
    for (i=0;i<MAX_CHAR_EVENTS;i++)
        ch->char_events[i]=NULL;
    ch->char_specials.position = POS_STANDING;
    ch->mob_specials.default_pos = POS_STANDING;


    if (GET_HIT(ch) <= 0)
        GET_HIT(ch) = 1;
    if (GET_MOVE(ch) <= 0)
        GET_MOVE(ch) = 1;
    if (GET_MANA(ch) <= 0)
        GET_MANA(ch) = 1;
    GET_COND(ch, FULL)=24;
    GET_COND(ch, THIRST)=24;
    GET_COND(ch, DRUNK)=0;
    REMOVE_BIT(AFF3_FLAGS(ch), AFF3_RED | AFF3_BLUE | AFF3_HAS_RED | AFF3_HAS_BLUE);
    GET_LAST_TELL(ch) = NOBODY;
    SET_BIT(PRF2_FLAGS(ch), PRF2_NOQUIT);
    REMOVE_BIT(AFF2_FLAGS(ch), AFF2_BEHEAD);
    REMOVE_BIT(AFF3_FLAGS(ch), AFF3_FLEEING);
    
    if (ch->desc)
    {
    ch->desc->p1[0]='\0';
    ch->desc->p2[0]='\0';
    ch->desc->p3[0]='\0';
    }
    HUNTING(ch)=0;
}



/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void            clear_char(struct char_data * ch)
{
    memset((char *) ch, 0, sizeof(struct char_data));

    ch->regen=0;
    ch->in_room = NOWHERE;
    GET_PFILEPOS(ch) = -1;
    GET_WAS_IN(ch) = NOWHERE;
    GET_POS(ch) = POS_STANDING;

    ch->mob_specials.default_pos = POS_STANDING;

    GET_AC(ch) = 0;           /* Basic Armor */
    GET_MAGAC(ch) = 0;           /* Magic Armor */
    ch->ambush_name=NULL;
    ch->next_listener=NULL;
    ch->listening_to=0;
    ch->guarding=NULL;
    ch->wait_buffer=NULL;    
}


void            clear_object(struct obj_data * obj)
{
    memset((char *) obj, 0, sizeof(struct obj_data));

    obj->item_number = NOTHING;
    obj->in_room = NOWHERE;
}


void default_skills(struct char_data *ch)
{
    if (IS_THIEF(ch) || IS_RANGER(ch))
        SET_SKILL(ch, SKILL_ENH_SIGHT, 40);
    if (IS_ELF(ch) || IS_DROW(ch))
        SET_SKILL(ch, SKILL_ENH_SIGHT, GET_SKILL(ch, SKILL_ENH_SIGHT)+40);
        
    if (FOL_SKIG(ch))
    {
    	SET_SKILL(ch, SKILL_BACKSTAB, GET_PP(ch));
    	SET_SKILL(ch, SKILL_HIDE, GET_PP(ch));
    	SET_SKILL(ch, SKILL_PEEK, GET_PP(ch));    
    	SET_SKILL(ch, SKILL_STEAL, GET_PP(ch));
    }
    if (FOL_URG(ch))
    {
    	SET_SKILL(ch, SKILL_AGGRESIVE, 100);
    	SET_SKILL(ch, SKILL_ENH_DAMAGE, 100);
    	SET_SKILL(ch, SKILL_BERSERK, 100);
    }
    	
    	    
        
    //SET_SKILL(ch, SKILL_AGGRESIVE, MIN(99, 6*(GET_INTR(ch)+GET_WISR(ch))/4));
    //SET_SKILL(ch, SKILL_EVASIVE, MIN(99, 6*(GET_INTR(ch)+GET_WISR(ch))/4));
}


/* initialize a new character only if class is set */
void            init_char(struct char_data * ch)
{
    int             i;

    /* create a player_special structure */
    if (ch->player_specials == NULL)
        CREATE(ch->player_specials, struct player_special_data, 1);

    /* *** if this is our first player --- he be God *** */

    ch->points.max_mana = 1;
    ch->points.mana = 1;
    ch->points.max_hit = 1;
    ch->points.hit = 1;
    GET_MAX_HIT(ch)=1;
    GET_HIT(ch)=1;
    ch->points.max_move = 1;
    ch->points.move = 1;
    ch->points.armor = 0;
    ch->points.magic_armor = 0;
    ch->regen=0;

    for (i=0;i<MAX_ROOMS_IN_MUD;i++)
        GET_VISITED(ch, i)=0;

    if (top_of_p_table == 0) {
        GET_EXP(ch) = 1;
        GET_LEVEL(ch) = LVL_IMPL;

        ch->points.max_hit = 5000;
        ch->points.max_mana = 5000;
        ch->points.max_move = 5000;
        GET_MAX_HIT(ch)=5000;
        GET_HIT(ch)=5000;
        GET_MANA(ch)=5000;
        GET_MOVE(ch)=5000;
    }
    ch->points.hitr=ch->points.max_hit;
    ch->points.manar=ch->points.max_mana;
    ch->points.mover=ch->points.max_move;

    set_title(ch, NULL);

    ch->player.short_descr = NULL;
    ch->player.long_descr = NULL;

    ch->player.description = NULL;
    ch->wait_buffer=NULL;

    //ch->player.hometown = 3001;

    ch->ambush_name=NULL;
    ch->player.time.birth = time(0);
    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

    for (i = 0; i < MAX_TONGUE; i++)
        GET_TALK(ch, i) = 0;

    /* make favors for sex */

    ch->player.weight = number(120, 160);
    ch->player.height = number(100, 150);

    if (IS_OGRE(ch) || IS_TROLL(ch)) {
        ch->player.weight=number(230,255);
        ch->player.height+=160;
    }
    if (IS_ORC(ch)) {
        SET_SKILL(ch, SKILL_FILLET, 80);
        ch->player.weight+=70;
        ch->player.height+=90;
    }

    if (IS_ENT(ch)) {
        //SET_SKILL(ch, SKILL_FILLET, 80);
        ch->player.weight+=110;
        ch->player.height+=120;
    }
    if (IS_HUMAN(ch)) {
        ch->player.weight+=30;
        ch->player.height+=55;
    }
    if (IS_GOBLIN(ch) || IS_HALFLING(ch)) {
        ch->player.weight-=30;
        ch->player.height-=35;
    }
    if (IS_ELF(ch) || IS_DROW(ch))
    {
        ch->player.weight+=30;
        ch->player.height+=65;
    }

    ch->player.height=MIN(ch->player.height, 254);
    ch->player.weight=MIN(ch->player.weight, 254);
    if (ch->player.sex == SEX_FEMALE) {
        ch->player.weight-=20;
        ch->player.height-=15;
    }
    ch->player.height=MIN(ch->player.height, 254);
    ch->player.weight=MIN(ch->player.weight, 254);

    //ch->points.mana = GET_MAX_MANA(ch);
    ch->points.hit = GET_MAX_HIT(ch);
    //ch->points.move = GET_MAX_MOVE(ch);

    ch->points.armor = 0;
    ch->points.magic_armor = 0;



    player_table[top_of_p_table].id = GET_IDNUM(ch) = ++top_idnum;


    for (i = 1; i <= MAX_SKILLS; i++) {
        if (GET_LEVEL(ch) < LVL_IMPL)
            SET_SKILL(ch, i, 0)
            else
                SET_SKILL(ch, i, 100);
    }
    default_skills(ch);

    ch->char_specials.saved.affected_by = 0;


    for (i = 0; i < 5; i++)
        GET_SAVE(ch, i) = 0;

    for (i = 0; i < 3; i++)
        GET_COND(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? -1 : 24);

    GET_MOBRATIO(ch)=DEFAULT_MOBRATIO;
    strcpy(ch->player_specials->saved.enrage, "off");
    //if (!IS_CASTER(ch))
    //strcpy(ch->player_specials->saved.prompt, "%f %h/%HH %v/%VV %xexp %b %a> ");
    //else
    strcpy(ch->player_specials->saved.prompt, DEFAULT_PROMPT);
    strcpy(ch->player_specials->saved.prompt_combat, DEFAULT_COMBAT_PROMPT);
    strcpy(ch->player_specials->saved.grats, "Grats!!");
    strcpy(ch->player_specials->saved.walkin, "off");
    strcpy(ch->player_specials->saved.walkout, "off");
    GET_WIMP_LEV(ch) = 5;
    SET_BIT(PRF_FLAGS(ch), PRF_NOWIZ | PRF_NOCLAN | PRF_AUTODIR | PRF_AUTOSAVE);
    SET_BIT(PRF2_FLAGS(ch), PRF2_NOQUIT | PRF2_AUTOSPLIT | PRF2_AUTOLOOT | PRF2_AUTOSCAN | PRF2_AUTOASSIST | PRF2_AUTOMAP | PRF2_DISPDAM | PRF2_RUNNING);
    SET_BIT(PLR_FLAGS(ch), PLR_CRASH);
    
    GET_LINEWRAP(ch)=0;
    if (FOL_URG(ch))
    {
    	SET_STYLE(ch,2);
    }
}



/* returns the real number of the room with given virtual number */
int             real_room(int virtual)
{

    int bot=0, top=top_of_world, mid;

    for (;;) {
        mid = (bot + top) >> 1;

        if ((world + mid)->number == virtual)
            return mid;
        if (bot >= top)
            return -1;
        if ((world + mid)->number > virtual)
            top = mid - 1;
        else
            bot = mid + 1;
    }
    return -1;
}
/*Note: reditmod - changed binary search to incremental */
/*
    int             nr,
                    found = -1;

    for (nr = 0; nr <= top_of_world; nr++) {
        if (world[nr].number == virtual) {
            found = nr;
            break;
        }
    }
    return (found);
}
*/

/* meditmod */

/* returns the real number of the monster with given virtual number */
int             real_mobile(int virtual)
{
    int bot=0, top=top_of_mobt, mid;


    for (;;) {
        mid = (bot + top) / 2;

        if ((mob_index + mid)->virtual == virtual)
            return (mid);
        if (bot >= top)
            return (-1);
        if ((mob_index + mid)->virtual > virtual)
            top = mid - 1;
        else
            bot = mid + 1;
    }
    return -1;
}
/*
    int             nr,
                    found = -1;

    for (nr = 0; nr <= top_of_mobt; nr++) {
        if (mob_index[nr].virtual == virtual) {
            found = nr;
            break;
        }
    }
    return (found);
}
*/

/* returns the real number of the object with given virtual number */
int             real_object(int virtual)
{
    int bot=0, top=top_of_objt, mid;


    for (;;) {
        mid = (bot + top) / 2;

        if ((obj_index + mid)->virtual == virtual)
            return (mid);
        if (bot >= top)
            return (-1);
        if ((obj_index + mid)->virtual > virtual)
            top = mid - 1;
        else
            bot = mid + 1;
    }
    return -1;
}
/*
    int             nr,
                    found = -1;

    for (nr = 0; nr <= top_of_objt; nr++) {
        if (obj_index[nr].virtual == virtual) {
            found = nr;
            break;
        }
    }
    return (found);
}
*/
/* the functions */

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */
/*
int             mprog_name_to_type(char *name)
{
    if (!str_cmp(name, "in_file_prog"))
        return IN_FILE_PROG;
    if (!str_cmp(name, "act_prog"))
        return ACT_PROG;
    if (!str_cmp(name, "speech_prog"))
        return SPEECH_PROG;
    if (!str_cmp(name, "rand_prog"))
        return RAND_PROG;
    if (!str_cmp(name, "fight_prog"))
        return FIGHT_PROG;
    if (!str_cmp(name, "hitprcnt_prog"))
        return HITPRCNT_PROG;
    if (!str_cmp(name, "death_prog"))
        return DEATH_PROG;
    if (!str_cmp(name, "entry_prog"))
        return ENTRY_PROG;
    if (!str_cmp(name, "greet_prog"))
        return GREET_PROG;
    if (!str_cmp(name, "all_greet_prog"))
        return ALL_GREET_PROG;
    if (!str_cmp(name, "give_prog"))
        return GIVE_PROG;
    if (!str_cmp(name, "bribe_prog"))
        return BRIBE_PROG;

    return (ERROR_PROG);
}
  */

int mprog_name_to_type ( char *name )
{
    if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
    if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
    if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
    if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
    if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
    if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
    if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
    if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
    if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
    if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
    if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
    if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;
    if ( !str_cmp( name, "time_prog"     ) )	return TIME_PROG;
    if ( !str_cmp( name, "hour_prog"     ) )	return HOUR_PROG;
    if ( !str_cmp( name, "wear_prog"     ) )	return WEAR_PROG;
    if ( !str_cmp( name, "remove_prog"   ) )	return REMOVE_PROG;
    if ( !str_cmp( name, "sac_prog"      ) )	return SAC_PROG;
    if ( !str_cmp( name, "look_prog"     ) )	return LOOK_PROG;
    if ( !str_cmp( name, "exa_prog"      ) )	return EXA_PROG;
    if ( !str_cmp( name, "zap_prog"      ) )	return ZAP_PROG;
    if ( !str_cmp( name, "get_prog"      ) ) 	return GET_PROG;
    if ( !str_cmp( name, "drop_prog"     ) )	return DROP_PROG;
    if ( !str_cmp( name, "damage_prog"   ) )	return DAMAGE_PROG;
    if ( !str_cmp( name, "repair_prog"   ) )	return REPAIR_PROG;
    if ( !str_cmp( name, "greet_prog"    ) )	return GREET_PROG;
    if ( !str_cmp( name, "grand_prog"   ) )	return RANDIW_PROG;
    if ( !str_cmp( name, "action_prog" ) )	return ACTION_PROG;
    if ( !str_cmp( name, "pull_prog"	) )     return PULL_PROG;
    if ( !str_cmp( name, "push_prog"	) )     return PUSH_PROG;
    if ( !str_cmp( name, "sleep_prog"    ) )	return SLEEP_PROG;
    if ( !str_cmp( name, "rest_prog"	) )	return REST_PROG;
    if ( !str_cmp( name, "rfight_prog"   ) )	return FIGHT_PROG;
    if ( !str_cmp( name, "enter_prog"    ) )	return ENTRY_PROG;
    if ( !str_cmp( name, "leave_prog"    ) )	return LEAVE_PROG;
    if ( !str_cmp( name, "rdeath_prog"	) )	return DEATH_PROG;
    if ( !str_cmp( name, "script_prog"	) )	return SCRIPT_PROG;
    if ( !str_cmp( name, "use_prog"	) )	return USE_PROG;
    return( ERROR_PROG );
}
// SMAUG mobprogs


/* Read a number from a file. */
int             fread_number(FILE * fp)
{
    int             number;
    bool            sign;
    char            c;

    do {
        c = getc(fp);
    } while (isspace(c));

    number = 0;

    sign = FALSE;
    if (c == '+') {
        c = getc(fp);
    } else if (c == '-') {
        sign = TRUE;
        c = getc(fp);
    }
    if (!isdigit(c)) {
        log("Fread_number: bad format.");
        exit(1);
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

/* Read to end of line (for comments). */
void            fread_to_eol(FILE * fp)
{
    char            c;

    do {
        c = getc(fp);
    } while (c != '\n' && c != '\r');

    do {
        c = getc(fp);
    } while (c == '\n' || c == '\r');

    ungetc(c, fp);
    return;
}

/* Read one word (into static buffer). */
char           *fread_word2(FILE * fp)
{
    static char     word[MAX_INPUT_LENGTH];
    char           *pword;
    char            cEnd;

    do {
        cEnd = getc(fp);
    }
    while (isspace(cEnd));

    if (cEnd == '\'' || cEnd == '"') {
        pword = word;
    } else {
        word[0] = cEnd;
        pword = word + 1;
        cEnd = ' ';
    }

    for (; pword < word + MAX_INPUT_LENGTH; pword++) {
        *pword = getc(fp);
        if (cEnd == ' ' ? isspace(*pword) || *pword == '~' : *pword == cEnd) {
            if (cEnd == ' ' || cEnd == '~')
                ungetc(*pword, fp);
            *pword = '\0';
            return word;
        }
    }

    log("SYSERR: Fread_word: word too long.");
    exit(1);
    return NULL;
}
/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
        if ( feof(fp) )
        {
            log("SYSERR:fread_word: EOF encountered on read.\n\r");

            exit(1);
            word[0] = '\0';
            return word;
        }
        cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word+1;
        cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        if ( feof(fp) )
        {
            log("SYSERR:fread_word: EOF encountered on read.\n\r");

            exit(1);
            *pword = '\0';
            return word;
        }
        *pword = getc( fp );
        if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
        {
            if ( cEnd == ' ' )
                ungetc( *pword, fp );
            *pword = '\0';
            return word;
        }
    }

    log("SYSERR:Fread_word: word too long" );
    exit( 1 );
    return NULL;
}



struct index_data *get_obj_index(int vnum)
{
    int             nr;
    for (nr = 0; nr <= top_of_objt; nr++) {
        if (obj_index[nr].virtual == vnum)
            return &obj_index[nr];
    }
    return NULL;
}

struct index_data *get_mob_index(int vnum)
{
    int             nr;
    for (nr = 0; nr <= top_of_mobt; nr++) {
        if (mob_index[nr].virtual == vnum)
            return &mob_index[nr];
    }
    return NULL;
}


/* This routine reads in scripts of MOBprograms from a file */
char *mprog_type_to_name( int type ) ;
MPROG_DATA     *mprog_file_read(char *f, MPROG_DATA * mprg,
                                struct index_data * pMobIndex)
{

    char            MOBProgfile[MAX_INPUT_LENGTH];
    MPROG_DATA     *mprg2;
    FILE           *progfile;
    char            letter;
    bool            done = FALSE;

    sprintf(MOBProgfile, "%s/%s", MOB_DIR, f);

    progfile = fopen(MOBProgfile, "r");
    if (!progfile) {
        sprintf(err_buf, "Mob: %d couldnt open mobprog file %s", pMobIndex->virtual, MOBProgfile);
        log(err_buf);
        exit(1);
    }
    mprg2 = mprg;
    switch (letter = fread_letter(progfile)) {
    case '>':
        break;
    case '|':
        log("empty mobprog file.");
        exit(1);
        break;
    default:
        log("in mobprog file syntax error.");
        exit(1);
        break;
    }

    while (!done) {
        mprg2->type = mprog_name_to_type(fread_word(progfile));
        switch (mprg2->type) {
        case ERROR_PROG:
            log("mobprog file type error");
            exit(1);
            break;
        case IN_FILE_PROG:
            log("mprog file contains a call to file.");
            exit(1);
            break;
        default:
            sprintf(buf2, "Error in file %s", f);
            pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
            mprg2->arglist = fread_string(progfile, buf2);
            if (!(mprg2->arglist))
            {
            	sprintf(err_buf, "SYSERR: %s (no args to %s, 100 used).", buf2, mprog_type_to_name(mprg2->type));
                log(err_buf);
                mprg2->arglist=strdup("100");
                }
            mprg2->comlist = fread_string(progfile, buf2);
            switch (letter = fread_letter(progfile)) {
            case '>':
                CREATE(mprg2->next, MPROG_DATA, 1);
                //mprg2->next = (MPROG_DATA *) malloc(sizeof(MPROG_DATA));
                mprg2 = mprg2->next;
                mprg2->next = NULL;
                break;
            case '|':
                done = TRUE;
                break;
            default:
                sprintf(err_buf, "in mobprog file %s syntax error.", f);
                log(err_buf);
                exit(1);
                break;
            }
            break;
        }
    }
    fclose(progfile);
    return mprg2;
}

/* This procedure is responsible for reading any in_file MOBprograms. */

void            mprog_read_programs(FILE * fp, struct index_data * pMobIndex)
{
    MPROG_DATA     *mprg;
    char            letter;
    bool            done = FALSE;

    if ((letter = fread_letter(fp)) != '>') {
        sprintf(err_buf, "Load_mobiles: vnum %d MOBPROG char", pMobIndex->virtual);
        log(err_buf);
        exit(1);
    }
    CREATE( mprg, MPROG_DATA, 1 );
    pMobIndex->mudprogs = mprg;
    //pMobIndex->mudprogs = (MPROG_DATA *) malloc(sizeof(MPROG_DATA));
    //mprg = pMobIndex->mobprogs;

    while (!done) {
        mprg->type = mprog_name_to_type(fread_word(fp));
        switch (mprg->type) {
        case ERROR_PROG:
            sprintf(err_buf, "Load_mobiles: vnum %d MOBPROG type.", pMobIndex->virtual);
            log(err_buf);
            exit(1);
            break;
        case IN_FILE_PROG:
            sprintf(buf2, "Mobprog for mob #%d", pMobIndex->virtual);
            mprg = mprog_file_read(fread_string(fp, ""), mprg, pMobIndex);
            fread_to_eol(fp);   /* need to strip off that silly ~ */
            switch (letter = fread_letter(fp)) {
            case '>':
                // mprg->next = (MPROG_DATA *) malloc(sizeof(MPROG_DATA));
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                mprg->next = NULL;
                break;
            case '|':
                mprg->next = NULL;
                fread_to_eol(fp);
                done = TRUE;
                break;
            default:
                sprintf(err_buf, "Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->virtual);
                log(err_buf);
                exit(1);
                break;
            }
            break;
        default:
            sprintf(buf2, "Mobprog for mob #%d", pMobIndex->virtual);
            pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
            mprg->arglist = fread_string(fp, buf2);
            if (!(mprg->arglist))
            {   
            	sprintf(err_buf, "SYSERR: Load_mobiles: vnum %d bad MOBPROG (no args to %s, 100 used).", pMobIndex->virtual, mprog_type_to_name(mprg->type));
                log(err_buf);
                mprg->arglist=strdup("100");
            }
            //           fread_to_eol( fp );
            mprg->comlist = fread_string(fp, buf2);
            //         fread_to_eol( fp );
            switch (letter = fread_letter(fp)) {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                mprg->next = NULL;
                break;
            case '|':
                mprg->next = NULL;
                fread_to_eol(fp);
                done = TRUE;
                break;
            default:
                sprintf(err_buf, "Load_mobiles: vnum %d bad MOBPROG (%c).", pMobIndex->virtual, letter);
                log(err_buf);
                log(mprg->arglist);
                log(mprg->comlist);
                exit(1);
                break;
            }
            break;
        }
    }

    return;
}



/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA *oprog_file_read( char *f, MPROG_DATA *mprg,
                             struct index_data *pObjIndex )
{

    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE       *progfile;
    char        letter;
    MPROG_DATA *mprg_next, *mprg2;
    bool        done = FALSE;

    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

    progfile = fopen( MUDProgfile, "r" );

    if (!progfile) {
        sprintf(err_buf, "Obj: %d couldnt open objprog file", pObjIndex->virtual);
        log(err_buf);
        exit(1);
    }

    mprg2 = mprg;
    switch ( letter = fread_letter( progfile ) )
    {
    case '>':
        break;
    case '|':
        log( "empty objprog file." );
        exit( 1 );
        break;
    default:
        log( "in objprog file syntax error." );
        exit( 1 );
        break;
    }

    while ( !done )
    {
        mprg2->type = mprog_name_to_type( fread_word( progfile ) );
        switch ( mprg2->type )
        {
        case ERROR_PROG:
            log( "objprog file type error" );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            log( "objprog file contains a call to file." );
            exit( 1 );
            break;
        default:
            sprintf(buf2, "Error in file %s", f);
            pObjIndex->progtypes = pObjIndex->progtypes | mprg2->type;
            mprg2->arglist = fread_string(progfile, buf2);
             if (!(mprg2->arglist))
            {
            	sprintf(err_buf, "SYSERR: %s (no args to %s, 100 used).", buf2, mprog_type_to_name(mprg2->type));
                log(err_buf);
                mprg2->arglist=strdup("100");
                }
            mprg2->comlist = fread_string(progfile, buf2);
            switch ( letter = fread_letter( progfile ) )
            {
            case '>':
                CREATE( mprg_next, MPROG_DATA, 1 );
                mprg_next->next = mprg2;
                mprg2 = mprg_next;
                break;
            case '|':
                done = TRUE;
                break;
            default:
                log( "in objprog file syntax error." );
                exit( 1 );
                break;
            }
            break;
        }
    }
    fclose( progfile );
    return mprg2;
}


/* This procedure is responsible for reading any in_file OBJprograms.
 */

void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
    MPROG_DATA *mprg;
    char        letter;
    bool        done = FALSE;

    /*if ((letter = fread_letter(fp)) != '>') {
          sprintf(err_buf, "Load_mobiles: vnum %d OBJPROG char", pObjIndex->virtual);
          log(err_buf);
          exit(1);
      }      */
    CREATE( mprg, MPROG_DATA, 1 );
    pObjIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->type = mprog_name_to_type( fread_word( fp ) );
        switch ( mprg->type )
        {
        case ERROR_PROG:
            sprintf(err_buf, "Load_objects: vnum %d OBJPROG type.", pObjIndex->virtual);
            log(err_buf);
            exit( 1 );
            break;
        case IN_FILE_PROG:
            sprintf(buf2, "Objprog for obj #%d", pObjIndex->virtual);
            mprg = oprog_file_read( fread_string( fp,"" ), mprg, pObjIndex );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                mprg->next = NULL;
                break;
            case '|':
                mprg->next = NULL;
                fread_to_eol( fp );
                done = TRUE;
                break;
            default:
                sprintf(err_buf, "Load_objects: vnum %d bad OBJPROG.", pObjIndex->virtual);
                log(err_buf);
                exit(1);
                break;
            }
            break;
        default:
            sprintf(buf2, "Objprog for obj #%d", pObjIndex->virtual);
            pObjIndex->progtypes = pObjIndex->progtypes | mprg->type;
            mprg->arglist = fread_string(fp, buf2);
           if (!(mprg->arglist))
            {   
            	sprintf(err_buf, "SYSERR: Load_objects: vnum %d bad OBJPROG (no args to %s, 100 used).", pObjIndex->virtual, mprog_type_to_name(mprg->type));
                log(err_buf);
                mprg->arglist=strdup("100");
            }
            //   fread_to_eol( fp );
            mprg->comlist = fread_string(fp, buf2);
            //fread_to_eol( fp );


            switch ( letter = fread_letter( fp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                mprg->next = NULL;
                break;
            case '|':
                mprg->next = NULL;
                fread_to_eol( fp );
                done = TRUE;
                break;
            default:
                sprintf(err_buf, "Load_objects: vnum %d bad OBJPROG (%c).", pObjIndex->virtual, letter);
                log(err_buf);
                exit( 1 );
                break;
            }
            break;
        }
    }

    return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */

MPROG_DATA *rprog_file_read( char *f, MPROG_DATA *mprg,
                             ROOM_INDEX_DATA *RoomIndex )
{

    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE       *progfile;
    char        letter;
    MPROG_DATA *mprg_next, *mprg2;
    bool        done = FALSE;

    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

    progfile = fopen( MUDProgfile, "r" );
    if ( !progfile )
    {
        logs( "Room: %d couldnt open roomprog file", RoomIndex->number );
        exit( 1 );
    }

    mprg2 = mprg;
    switch ( letter = fread_letter( progfile ) )
    {
    case '>':
        break;
    case '|':
        logs( "#%d: empty roomprog file.",RoomIndex->number );
        exit( 1 );
        break;
    default:
        logs( "#%d: in roomprog file syntax error.",RoomIndex->number );
        exit( 1 );
        break;
    }

    while ( !done )
    {
        mprg2->type = mprog_name_to_type( fread_word( progfile ) );
        switch ( mprg2->type )
        {
        case ERROR_PROG:
            logs( "#%d: roomprog file type error" ,RoomIndex->number);
            exit( 1 );
            break;
        case IN_FILE_PROG:
            logs( "#%d: roomprog file contains a call to file.",RoomIndex->number );
            exit( 1 );
            break;
        default:
            sprintf(buf2, "Error in file %s", f);
            RoomIndex->progtypes|= mprg2->type;
            mprg2->arglist       = fread_string( progfile , buf2);
             if (!(mprg2->arglist))
            {
            	sprintf(err_buf, "SYSERR: %s (no args to %s, 100 used).", buf2, mprog_type_to_name(mprg2->type));
                log(err_buf);
                mprg2->arglist=strdup("100");
                }
            mprg2->comlist       = fread_string( progfile , buf2);
            switch ( letter = fread_letter( progfile ) )
            {
            case '>':
                CREATE( mprg_next, MPROG_DATA, 1 );
                mprg_next->next = mprg2;
                mprg2 = mprg_next;
                break;
            case '|':
                done = TRUE;
                break;
            default:
                logs( "#%d: in roomprog file syntax error.",RoomIndex->number );
                exit( 1 );
                break;
            }
            break;
        }
    }
    fclose( progfile );
    return mprg2;
}

void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
    MPROG_DATA *mprg;
    char        letter;
    bool        done = FALSE;

    /*  if ( ( letter = fread_letter( fp ) ) != '>' )
      {
          logs( "Load_rooms: vnum %d ROOMPROG char", pRoomIndex->number );
          exit( 1 );
      }*/
    CREATE( mprg, MPROG_DATA, 1 );
    pRoomIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->type = mprog_name_to_type( fread_word( fp ) );
        switch ( mprg->type )
        {
        case ERROR_PROG:
            logs( "Load_rooms: vnum %d ROOMPROG type.", pRoomIndex->number );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            sprintf(buf2, "Roomprog for room #d", pRoomIndex->number);
            mprg = rprog_file_read( fread_string( fp, buf2 ), mprg,pRoomIndex );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                fread_to_eol( fp );
                done = TRUE;
                break;
            default:
                logs( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->number );
                exit( 1 );
                break;
            }
            break;
        default:
            sprintf(buf2, "Roomprog for room #%d", pRoomIndex->number);
            pRoomIndex->progtypes |= mprg->type;
            mprg->arglist        = fread_string( fp, buf2 );
     	if (!(mprg->arglist))
            {   
            	sprintf(err_buf, "SYSERR: Load_rooms: vnum %d bad ROOMPROG. (no args to %s, 100 used).", pRoomIndex->number, mprog_type_to_name(mprg->type));
                log(err_buf);
                mprg->arglist=strdup("100");
            }         
            //fread_to_eol( fp );
            mprg->comlist        = fread_string( fp, buf2 );
            //fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                fread_to_eol( fp );
                done = TRUE;
                break;
            default:
                logs( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->number );
                exit( 1 );
                break;
            }
            break;
        }
    }

    return;

}

void            save_char_file_u(struct char_file_u st)
{
    int             player_i;
    int             find_name(char *name);

    if ((player_i = find_name(st.name)) >= 0) {
        fseek(player_fl, player_i * sizeof(struct char_file_u), SEEK_SET);
        fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
        fflush(player_fl);
    }
}


int find_zone_by_num(int num)
{
    int i;
    for (i = 0; i <= top_of_zone_table; i++)
        if (zone_table[i].number == num)
            return (i);
    return (-1);
}





int get_new_pref() {
    FILE *fp;
    char filename[1024];
    int i;

    sprintf(filename,PREF_FILE);
    if(!(fp=fopen(filename,"r"))) {
        touch(filename);
        if(!(fp=fopen(filename,"w"))) {
            return (number(1,128000));
        }
        fprintf(fp,"1\r\n");
        fclose(fp);
        return 1;
    }
    fscanf(fp,"%d",&i);
    fclose(fp);
    unlink(filename);
    if(!(fp=fopen(filename,"w"))) {
        log("Unable to open p-ref file for writing");
        return i;
    }
    fprintf(fp,"%d\n",i+1);
    fclose(fp);
    return i;

}


void vwear_obj(int type, struct char_data * ch)
{
    int nr, found =0;
    for (nr = 0; nr <= top_of_objt; nr++) {
        if (GET_OBJ_TYPE(&obj_proto[nr]) == type) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                    obj_index[nr].virtual, obj_proto[nr].short_description);
            send_to_char(buf, ch);
        }
    }
}

void vwear_object(int wearpos, struct char_data * ch)
{
    int nr, found = 0;
    for (nr = 0; nr <= top_of_objt; nr++) {
        if (CAN_WEAR(&obj_proto[nr], wearpos)) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                    obj_index[nr].virtual, obj_proto[nr].short_description);
            send_to_char(buf, ch);
        }
    }
}
