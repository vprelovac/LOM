#define MAX_CLANS	 20
#define LVL_CLAN_GOD	 LVL_GOD
#define DEFAULT_APP_LVL	 8
#define CLAN_PLAN_LENGTH 800

#define GET_CLAN(ch)		((ch)->player_specials->saved.clan)
#define IS_EXPELLED(ch)		((ch)->player_specials->saved.clan_expelled)
#define GET_CLAN_RANK(ch)	((ch)->player_specials->saved.clan_rank)


#define CLAN_ENTRY_LEVEL 5

#define CLAN_NEWBIE	  0
#define CLAN_BLUE	  1
#define CLAN_RED	  2
#define CLAN_OUTLAW	  3

#define CP_SET_PLAN   0
#define CP_ENROLL     1
#define CP_EXPEL      2
#define CP_PROMOTE    3
#define CP_DEMOTE     4
#define CP_SET_FEES   5
#define CP_WITHDRAW   6
#define CP_SET_APPLEV 7
#define NUM_CP        8        /* Number of clan privileges */

#define CM_DUES   1
#define CM_APPFEE 2

#define CB_DEPOSIT  1
#define CB_WITHDRAW 2

void save_clans(void);
void init_clans(void);
void do_clan_enroll(struct char_data *ch,int clan);
sh_int find_clan_by_id(int clan_id);
sh_int find_clan(char *name);


extern int num_of_clans;



struct clan_rec {
    int	id;
    char	name[32];
    int	ranks;
    char	rank_name[20][20];
    long	treasure;
    long  healer_gold;
    int   soldiers;
    int	members;
    int	power;
    int	kills;
    int	score;
    int	spells[5];
    int	app_level;
    int	privilege[20];
    int	at_war[4];
    char  description[CLAN_PLAN_LENGTH];
};

struct clan_rec clan[MAX_CLANS];