/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern int global_no_timer;

/* public functions in utils.c */

extern int total_ram;

/*#if !defined(ADV_MEMORY) && 1
#define ADV_MEMORY	1
#endif
*/

//#include "spells.h"
extern struct index_data *mob_index;
//extern struct spell_info_type spell_info[];

//#define HAS_PROG(what, prog)	(IS_SET((what).progtypes, (prog)))
#define HAS_OBJ_PROG(obj, prog)	((obj) && !PURGED(obj) && IS_SET(obj_index[(obj)->item_number].progtypes, (prog)))
#define HAS_MOB_PROG(mob, prog)	((mob) && !DEAD(mob) && IS_NPC(mob) && IS_SET(mob_index[(mob)->nr].progtypes, (prog)))
#define HAS_ROOM_PROG(room, prog)	(IS_SET((room)->progtypes, (prog)))

#define DEAD(ch) ((ch)->in_room==-1 || (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), PLR_JUSTDIED)))
#define PURGED(obj) ((obj)->in_room==-2)
SPECIAL(shop_keeper); 

char *GetSoundexKey ( const char *szTxt );
int   SoundexMatch  ( char *szFirst, char *szSecond );



int find_deity_by_name(char *name);
int spot_trap(struct char_data *ch, int room);
void TrapDamage( struct char_data *ch, int trap, int level, int deity);
struct raff_node *find_raff_by_spell(int room, int spell);
struct raff_node *find_raff_by_aff(int room, int aff);
int get_trap_by_name(char *name);
void remove_trap(struct raff_node *raff);
void FireRoomTrap(struct char_data *ch);
int TriggerTrap( struct char_data *ch, int trap, int level, int deity);
int is_guarded(struct char_data *ch);
int num_fighting(struct char_data *ch);
void raw_awake(struct char_data *ch);
void apply_poison(struct char_data *ch, int duration);
int is_abbrev_multi(char *arg, char *help);
void perform_players(int mode);
void            quest_update(void);
void write_who_file();
void chatperform(struct char_data * ch, struct char_data * victim, char *msg);
void chatperformtoroom(struct char_data * ch, char *txt);
void sprintf_minutes( char* tmp, const time_t time );
int get_other_armor(struct char_data *victim);
int get_random_armor_pc(struct char_data *victim);
int get_random_armor_npc(struct char_data *victim);
char    *str_dup(const char *source);
int str_cmp(char *arg1, char *arg2);
int strn_cmp(char *arg1, char *arg2, int n);
void    log(char *str);
int touch(char *path);
void    mudlog(char *str, char type, sbyte level, byte file);
void    log_death_trap(struct char_data *ch);
int number(int from, int to);
int dice(int number, int size);
void    sprintbit(ulong vektor, char *names[], char *result);
void    sprinttype(int type, char *names[], char *result);
char   *sprintbitascii(ulong vektor, char *result);
/*int   get_line(FILE *fl, char *buf);
int     get_line2(FILE * fl, char *buf);*/
int get_filename(char *orig_name, char *filename, int mode);
struct time_info_data age(struct char_data *ch);
int GET_REINCARN(struct char_data *ch);
void    ADD_REINCARN(struct char_data *ch);
bool    CAN_DAMAGE(struct char_data *ch, struct char_data *vic);
bool    CAN_MURDER(struct char_data *ch, struct char_data *victim);
int get_alignment_type(int temp);
int     interpolate(int level, int thac_0,int thac_50);
int     replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size);
void    format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen);
bool is_same_group( struct char_data *ach, struct char_data *bch );
int     count_group(struct char_data *ch);
int exp_this_level(int lev);
int total_exp(int lev);
int power(int n);
int align_damage(struct char_data *ch,int dam);
char *get_mood(struct char_data *ch);
int get_zone_rnum(int vnum);
char* get_clan_name(struct char_data *ch);
int bti(int);
char *linewrap(char *str, int max);
char* getst(char *s,int num,char raz);
char *trim (char *str);


void extract_exit(room_num room, EXIT_DATA *pexit, int dir );
EXIT_DATA *make_exit( room_num pRoomIndex, room_num to_room, sh_int door );
/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif



int MAX(int a, int b);
int MIN(int a, int b);
unsigned int UMIN(unsigned int a, unsigned int b);
unsigned int UMAX(unsigned int a, unsigned int b);

#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))

/* in magic.c */
bool    circle_follow(struct char_data *ch, struct char_data * victim);
EXIT_DATA * find_door2(struct char_data * ch, char *dir, char *type);
/* in act.informative.c */
void    look_at_room(struct char_data *ch, int mode);

/* in act.movmement.c */
int do_simple_move(struct char_data *ch, int dir, int following);
int perform_move(struct char_data *ch, int dir, int following);

/* in limits.c */
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
float mana_gain(struct char_data *ch);
float hit_gain(struct char_data *ch);
float move_gain(struct char_data *ch);
void    advance_level(struct char_data *ch);
void    set_title(struct char_data *ch, char *title);
void    gain_exp(struct char_data *ch, int gain);
void    gain_exp_regardless(struct char_data *ch, int gain);
void    gain_condition(struct char_data *ch, int condition, int value);
void    check_idling(struct char_data *ch);
void    point_update(void);
void    update_pos(struct char_data *victim);


/* various constants *****************************************************/


/* defines for mudlog() */
#define OFF 0
#define BRF 1
#define NRM 2
#define CMP 3

/* get_filename() */
#define CRASH_FILE  0
#define ETEXT_FILE  1
#define ALIAS_FILE  2
#define QUEST_FILE  3

/* breadth-first searching */
#define BFS_ERROR       -1
#define BFS_ALREADY_THERE   -2
#define BFS_NO_PATH     -3

/* mud-life time */
#define SECS_PER_MUD_HOUR   60
#define SECS_PER_MUD_DAY    (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH  (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR   (17*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN   60
#define SECS_PER_REAL_HOUR  (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY   (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR  (365*SECS_PER_REAL_DAY)


/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "no")
#define ONOFF(a) ((a) ? "ON" : "off")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IF_STR(st) ((st) ? (st) : "\0")
//#define CAP(st)  (*(st) = UPPER(*(st)), st)
char *CAP(char *txt);

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")


/* memory utils **********************************************************/


#define OREF(v, type) do { \
static struct obj_ref_type s={FALSE,NULL,NULL,type}; s.var=&v; \
obj_reference(&s); } while(0)
#define OUREF(var) obj_unreference(&var);

#define CREF(v, type) do { \
static struct char_ref_type s={FALSE,NULL,NULL,type}; s.var=&v; \
char_reference(&s); } while(0)
#define CUREF(var) char_unreference(&var);

#if !defined(ADV_MEMORY)
#define CREATE(result, type, number)  do {\
    if (!((result) = (type *) calloc ((number), sizeof(type))))\
        { perror("malloc failure"); abort(); } total_ram+=number*sizeof(type);} while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
        { perror("realloc failure"); abort(); } } while(0)
#endif
/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)  \
   if ((item) == (head))        \
      head = (item)->next;        \
   else {               \
      temp = head;            \
      while (temp && (temp->next != (item))) \
     temp = temp->next;       \
      if (temp)             \
         temp->next = (item)->next;   \
   }                    \


/* basic bitvector utils *************************************************/

#define SANITY_CHECK(ch) 						\
if (!(ch))								\
{									\
		sprintf(buf, "SANITY: NULL baja (%s, %d).", GET_NAME(ch), __FUNCTION__ , __LINE__); \
		log(buf);						\
		return;                                                 \
}                                                                       \
else if (IS_NPC(ch))   					        	\
{                             						\
	if (ch->in_room==NOWHERE)                                       \
	{                                                               \
		sprintf(buf, "SANITY: %s in NOWHERE (%s, %d).", GET_NAME(ch), __FUNCTION__ , __LINE__); \
		log(buf);                                               \
		return;                                                 \
	}                                                               \
	if (!ch->in_room)                                               \
	{                                                               \
		sprintf(buf, "SANITY: %s in LIMBO (%s, %d).", GET_NAME(ch), __FUNCTION__ , __LINE__); \
		log(buf);                                               \
		return;                                                 \
	}                                                               \
	if (!world[(ch)->in_room].people)                                 \
	{		                                                \
		sprintf(buf, "SANITY: %s ima krizu licnosti u sobi %d (%s, %d).", GET_NAME(ch), (ch)->in_room, __FUNCTION__ , __LINE__); \
		log(buf);                                               \
		return;                                                 \
	}};                                                             


#define DEX_CHECK(ch) ((number(1, 26)<GET_DEX(ch)))
#define STR_CHECK(ch) ((number(1, 26)<GET_STR(ch)))
#define INT_CHECK(ch) ((number(1, 26)<GET_INT(ch)))
#define WIL_CHECK(ch) ((number(1, 26)<GET_WIS(ch)))
#define CON_CHECK(ch) ((number(1, 26)<GET_CON(ch)))
#define CHA_CHECK(ch) ((number(1, 26)<GET_CHA(ch)))

#define SKILL_CHECK(ch, skill) (number(1, 111)<GET_SKILL(ch, skill))




#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))
#define SET_OR_REMOVE(flagset, flags) { \
        if (on) SET_BIT(flagset, flags); \
        else if (off) REMOVE_BIT(flagset, flags); }

#define MOB_FLAGS(ch) ((ch)->char_specials.saved.act)
#define MOB2_FLAGS(ch) ((ch)->char_specials.saved.act2)
#define MOB3_FLAGS(ch) ((ch)->char_specials.saved.act3)
#define PLR_FLAGS(ch) ((ch)->char_specials.saved.act)
#define PLR2_FLAGS(ch) ((ch)->char_specials.saved.act2)
#define PLR3_FLAGS(ch) ((ch)->char_specials.saved.act3)
#define PRF_FLAGS(ch) ((ch)->player_specials->saved.pref)
#define PRF2_FLAGS(ch) ((ch)->player_specials->saved.pref2)
#define AFF_FLAGS(ch) ((ch)->char_specials.saved.affected_by)
#define AFF2_FLAGS(ch) ((ch)->char_specials.saved.affected_by2)
#define AFF3_FLAGS(ch) ((ch)->char_specials.saved.affected_by3)
#define ROOM_FLAGS(loc) (world[(loc)].room_flags)
#define RM_BLOOD(rm)   (world[rm].blood)

#define IS_NPC(ch)  (IS_SET(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)  (IS_NPC(ch) && ((ch)->nr >-1))

#define GET_SPELL_EVENT(ch) ((ch)->char_events[0])
#define GET_MOVE_EVENT(ch) ((ch)->char_events[1])
#define GET_WEAR_EVENT(ch) ((ch)->char_events[2])
#define GET_UTIL_EVENT(ch) ((ch)->char_events[3])
#define GET_FIGHT_EVENT(ch) ((ch)->char_events[4])
#define GET_WAIT_EVENT(ch) ((ch)->char_events[5])

#define IN_EVENT(ch) (GET_SPELL_EVENT(ch) || GET_MOVE_EVENT(ch) || GET_WEAR_EVENT(ch) || GET_UTIL_EVENT(ch))
#define IN_EVENT_FULL(ch) (GET_SPELL_EVENT(ch) || GET_MOVE_EVENT(ch) || GET_WEAR_EVENT(ch) || GET_UTIL_EVENT(ch) || GET_FIGHT_EVENT(ch) || GET_WAIT_EVENT(ch))
#define LEV_BONUS(vict) ((IS_NPC(vict)?MAX(0, ((GET_LEVEL(vict)-30)*(GET_LEVEL(vict)-30)/100)):0))
#define GET_COURAGE(ch) (GET_LEVEL(ch)*GET_LEVEL(ch)/100)

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB_FLAGS(ch), (flag)))
#define MOB2_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB2_FLAGS(ch), (flag)))
#define MOB3_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB3_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), (flag)))
#define PLR2_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR2_FLAGS(ch), (flag)))
#define PLR3_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR3_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET(AFF_FLAGS(ch), (flag)))
#define AFF2_FLAGGED(ch, flag) (IS_SET(AFF2_FLAGS(ch), (flag)))
#define AFF3_FLAGGED(ch, flag) (IS_SET(AFF3_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET(PRF_FLAGS(ch), (flag)))
#define PRF2_FLAGGED(ch, flag) (IS_SET(PRF2_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS(loc), (flag)) || ((flag==ROOM_PEACEFUL)?ROOM_AFFECTED(loc, RAFF_PEACEFUL):0)        )

#define IN_ARENA(ch) (ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
#define RED(ch) (AFF3_FLAGGED(ch, AFF3_RED))
#define HAS_RED(ch) (AFF3_FLAGGED(ch, AFF3_HAS_RED))
#define HAS_BLUE(ch) (AFF3_FLAGGED(ch, AFF3_HAS_BLUE))
#define BLUE(ch) (AFF3_FLAGGED(ch, AFF3_BLUE))
#define RED_FLAG(obj) (GET_OBJ_TYPE(obj)==ITEM_RED_FLAG)
#define BLUE_FLAG(obj) (GET_OBJ_TYPE(obj)==ITEM_BLUE_FLAG)

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch), (flag))) & (flag))
#define PLR2_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR2_FLAGS(ch), (flag))) & (flag))
#define PLR3_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR3_FLAGS(ch), (flag))) & (flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))
#define PRF2_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF2_FLAGS(ch), (flag))) & (flag))

/* room utils ************************************************************/


#define SECT(room)  (world[(room)].sector_type)

#define IS_DARK(room)  ( !world[room].light && \
                         (ROOM_FLAGGED(room, ROOM_DARK) || \
                          ( ( SECT(room) != SECT_INSIDE && \
                              SECT(room) != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
                 weather_info.sunlight == SUN_DARK)) ) && !ROOM_AFFECTED(room, RAFF_ILLUMINATION))

#define IS_MURKY(room) (ROOM_FLAGGED(room, ROOM_MURKY))
#define IS_LIGHT(room)  (!IS_DARK(room))
#define IS_WET(ch) ((SECT(ch->in_room)>=6 && SECT(ch->in_room)<=8) || (OUTSIDE(ch) && (weather_info.sky==SKY_RAINING || weather_info.sky==SKY_LIGHTNING)))

#define GET_ROOM_SPEC(room) ((room) >= 0 ? world[(room)].func : NULL)

/* char utils ************************************************************/

/* color utils */
#define STATUS_COLOR(i, colorbuf, ch, C_TRAP)   \
   if (i > 80)                  \
      sprintf(colorbuf, "&G"); \
   else if (i > 50)             \
      sprintf(colorbuf, "&Y"); \
   else if (i > 25)             \
      sprintf(colorbuf, "&P"); \
   else                     \
      sprintf(colorbuf, "&R")  \


#define IN_ROOM(ch) ((ch)->in_room)
#define GET_WAS_IN(ch)  ((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch).year)

#define GET_NAME(ch)    (IS_NPC(ch) ? (ch)->player.short_descr : (ch)->player.name)
#define SET_NAME(ch,num)    if (IS_NPC(ch)) (ch)->player.short_descr=num; else (ch)->player.name=num;

#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_NUM_OF_CLASS(ch)    (IS_DUALCLASS(ch) ? 2 : (IS_TRIPLECLASS(ch) ? 3 : 1))


#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_PASSWD(ch)  ((ch)->player.passwd)
#define GET_PFILEPOS(ch)((ch)->pfilepos)

/*
 * I wonder if this definition of GET_REAL_LEVEL should be the definition
 * of GET_LEVEL?  JE
 */
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define CHANCE(i)  (number(1,100)<=i)


/*#define MOVE_LIMIT(ch) (GET_MAX_HIT(ch)/20)
#define CAST_LIMIT(ch) (GET_MAX_HIT(ch)/10)
#define PERCENT(ch) (GET_MAX_HIT(ch)/100)*/

#define MOVE_LIMIT(ch) (1) // (GET_MAX_MOVE(ch)/20)
#define CAST_LIMIT(ch) (1) // (GET_MAX_MANA(ch)/10)
#define PERCENT(ch) (3)

#define GET_CLASS(ch)   ((ch)->player.class)
#define GET_RACE(ch)    ((ch)->player.race)
#define GET_HOME(ch)    ((ch)->player.hometown)
#define GET_HEIGHT(ch)  ((ch)->player.height)
#define GET_WEIGHT(ch)  ((ch)->player.weight)
#define GET_SEX(ch) ((ch)->player.sex)

#define GET_TOTAL_WEIGHT(ch) ((GET_WEIGHT(ch)+IS_EQUIP_W(ch)+IS_CARRYING_W(ch)))

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_ADD(ch)     ((ch)->aff_abils.str_add)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)

#define GET_STRR(ch)     ((ch)->real_abils.str)
#define GET_ADDR(ch)     ((ch)->real_abils.str_add)
#define GET_DEXR(ch)     ((ch)->real_abils.dex)
#define GET_INTR(ch)     ((ch)->real_abils.intel)
#define GET_WISR(ch)     ((ch)->real_abils.wis)
#define GET_CONR(ch)     ((ch)->real_abils.con)
#define GET_CHAR(ch)     ((ch)->real_abils.cha)

#define GET_EXP(ch)       ((ch)->points.exp)
#define GET_RATIO(ch)     ((ch)->exp_ratio)
#define GET_AC(ch)        ((ch)->points.armor)
#define GET_MAGAC(ch)     ((ch)->points.magic_armor)
#define GET_HIT(ch)      ((ch)->points.hit)
#define GET_MAX_HIT(ch)   ((ch)->points.max_hit)
#define GET_MOVE(ch)      ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)      ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_MANAR(ch)      ((ch)->points.manar)
#define GET_MOVER(ch)      ((ch)->points.mover)
#define GET_HITR(ch)      ((ch)->points.hitr)
#define GET_GOLD(ch)      ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)   ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)

#define GET_PP(ch)	(9*(GET_WISR(ch)+GET_INTR(ch))/4)


#define GET_PREF(ch) 		((ch)->pref)
#define GET_HOST(ch)		((ch)->hostname)
#define GET_POS(ch)   ((ch)->char_specials.position)
#define GET_IDNUM(ch)     ((ch)->char_specials.saved.idnum)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_EQUIP_W(ch) ((ch)->char_specials.equip_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)      ((ch)->char_specials.fighting)
#define HUNTING(ch)   ((ch)->char_specials.hunting)
#define WRATHOBJ(ch)      ((ch)->char_specials.wrathobj)
#define GET_SAVE(ch, i)   ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)
#define GET_REAL_ALIGNMENT(ch) ((ch)->char_specials.saved.real_alignment)
#define GET_HONOR(ch)     ((ch)->player_specials->saved.honor)
#define GET_CHANNEL(ch)   ((ch)->player_specials->saved.channel)
#define GET_TERM(ch)      ((ch)->player_specials->saved.term)
#define GET_TERM_SIZE(ch)     ((ch)->player_specials->saved.size)
#define GET_LINEWRAP(ch)     ((ch)->player_specials->saved.linewrap)
#define GET_QUESTMOB(ch)      ((ch)->player_specials->saved.questmob)
#define GET_QUESTOBJ(ch)      ((ch)->player_specials->saved.questitem)
#define GET_QUESTG(ch)        ((ch)->player_specials->saved.questgiver)
#define GET_NEXTQUEST(ch)     ((ch)->player_specials->saved.questnext)
#define GET_STYLE(ch)         (IS_NPC(ch)? ((ch)->mob_specials.style) : ((ch)->player_specials->saved.spare19))
#define SET_STYLE(ch, num)    if (IS_NPC(ch)) (ch)->mob_specials.style=num; else (ch)->player_specials->saved.spare19=num;
#define GET_QUESTPOINTS(ch)   ((ch)->player_specials->saved.questpoints)
#define GET_COUNTDOWN(ch)     ((ch)->player_specials->saved.questcdown)
#define IS_QUESTOR(ch)     (PLR_FLAGGED(ch, PLR_QUESTOR))
#define QUESTING(ch)	(PRF_FLAGGED(ch, PRF_QUEST))
#define GET_TELLAS(ch)     ((ch)->player_specials->saved.sold_in_tellas)	// items sold at salesman
#define GET_COND(ch, i)     ((ch)->player_specials->saved.conditions[(i)])
#define GET_LOADROOM(ch)    ((ch)->player_specials->saved.load_room)
#define GET_PRACTICES(ch)   ((ch)->player_specials->saved.spells_to_learn)
#define GET_INVIS_LEV(ch)   ((ch)->player_specials->saved.invis_level)
#define GET_WIMP_LEV(ch)    ((ch)->player_specials->saved.wimp_level)
#define GET_VISITED(ch, i)     ((ch)->player_specials->saved.rooms_visited[(i)])
#define GET_KILLED_MOB(ch, i)     ((ch)->player_specials->saved.mobs_killed[(i)])
#define WRATH(ch)           ((ch)->player_specials->saved.wrath)
#define GET_FREEZE_LEV(ch)  ((ch)->player_specials->saved.freeze_level)
#define GET_BAD_PWS(ch)     ((ch)->player_specials->saved.bad_pws)
#define GET_TALK(ch, i)     ((ch)->player_specials->saved.talks[i])
#define POOFIN(ch)      ((ch)->player_specials->poofin)
#define POOFOUT(ch)     ((ch)->player_specials->poofout)
#define GET_LAST_OLC_TARG(ch)   ((ch)->player_specials->last_olc_targ)
#define GET_LAST_OLC_MODE(ch)   ((ch)->player_specials->last_olc_mode)
#define GET_ALIASES(ch)     ((ch)->player_specials->aliases)
#define GET_QUESTS(ch)      ((ch)->player_specials->quests)
#define GET_LAST_TELL(ch)   ((ch)->player_specials->last_tell)


#define GET_DEITY(ch)  (IS_NPC(ch)?(ch)->mob_specials.deity:(ch)->player_specials->saved.deity)
#define SET_DEITY(ch, num)  if (IS_NPC(ch)) (ch)->mob_specials.deity=num; else (ch)->player_specials->saved.deity=num;
#define DEITY_NAME(ch) ((IS_GOD(ch)? "self" : deity_list[GET_DEITY(ch)].name))
#define GET_FAITH(ch)  (IS_NPC(ch)?201:(ch)->player_specials->saved.faith)
#define SET_FAITH(ch, num)  ((ch)->player_specials->saved.faith=MAX(-400, MIN(400,num)))

#define FOL_AMURON(ch) (GET_DEITY(ch)==DEITY_AMURON)
#define FOL_VALERIA(ch) (GET_DEITY(ch)==DEITY_VALERIA)
#define FOL_AZ(ch) (GET_DEITY(ch)==DEITY_AZ)
#define FOL_SKIG(ch) (GET_DEITY(ch)==DEITY_SKIG)
#define FOL_URG(ch) (GET_DEITY(ch)==DEITY_URG)
#define FOL_BOUGAR(ch) (GET_DEITY(ch)==DEITY_BOUGAR)
#define FOL_MUGRAK(ch) (GET_DEITY(ch)==DEITY_MUGRAK)
#define FOL_VETIIN(ch) (GET_DEITY(ch)==DEITY_VETIIN)


#define HAS_SKILL(ch, skill) ((GET_LEVEL(ch)>=spell_info[skill].min_level[GET_CLASS_NUM(ch)]))
#define GET_SKILL(ch, i)    (IS_NPC(ch) ? (IS_COMBAT_SKILL(i)? GET_LEVEL(ch)*2+5:(HAS_SKILL(ch, i)? 2*(GET_INT(ch)+GET_WIS(ch))+5:0)): (ch)->player_specials->saved.skills[i])

#define SET_SKILL(ch, i, pct)   {(ch)->player_specials->saved.skills[i] = pct;}

#define IS_SPELL(i) ((i>0 && i<=MAX_SPELLS))
#define IS_PRAYER(i) ((i>178 && i<=199) || (i>221 && i<=MAX_SPELLS))
#define IS_SKILL(i) ((i>MAX_SPELLS) && (i!=TYPE_SUFFERING))
#define IS_HIT_E(i) (i>=TYPE_HIT && i<=TYPE_MISSILE)
#define IS_CONJURING(num) (num==SPELL_ANIMATE_DEAD || num==SPELL_MONSUM_I || num==SPELL_MONSUM_V || num==SPELL_CLONE || num==SPELL_CONJ_ELEMENTAL)

#define GET_EQ(ch, i)       ((ch)->equipment[i])

#define GET_MOB_SPEC(ch) (IS_MOB(ch) ? (mob_index[(ch->nr)].func) : NULL)
#define GET_MOB_RNUM(mob)   ((mob)->nr)
#define GET_MOB_VNUM(mob)   (IS_MOB(mob) ? \
                 mob_index[GET_MOB_RNUM(mob)].virtual : -1)
#define VALID_RNUM(rnum)	((rnum) >= 0 && (rnum) <= top_of_world)
#define GET_ROOM_VNUM(rnum) 	((int)(VALID_RNUM(rnum) ? world[(rnum)].number : NOWHERE))                 

#define GET_MOB_WAIT(ch)    ((ch)->mob_specials.wait_state)
#define GET_DEFAULT_POS(ch) ((ch)->mob_specials.default_pos)
#define MEMORY(ch)      ((ch)->mob_specials.memory)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 25) ? 36 :( \
          (GET_ADD(ch) <= 50) ? 37 :( \
          (GET_ADD(ch) <= 70) ? 38 :( \
          (GET_ADD(ch) <= 90) ? 39 :  40 ) ) )                   \
        )


#define IS_SHOPKEEPER(mob) (IS_NPC(mob)? (mob_index[GET_MOB_RNUM(mob)].func == shop_keeper) : 0)


#define CAN_CARRY_W(ch) (3*(str_app[GET_STR(ch)].carry_w)/4)
#define CAN_WIELD_W(ch) (str_app[GET_STR(ch)].wield_w)

#define CAN_CARRY_N(ch) (9 + (GET_CON(ch) +GET_DEX(ch))/2)
#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)

#define CAN_SEE_IN_DARK(ch) \
   (AFF_FLAGGED(ch, AFF_INFRAVISION) || PRF_FLAGGED(ch, PRF_HOLYLIGHT) \
    || IS_ELF(ch) || IS_DROW(ch) || IN_ARENA(ch))

#define CAN_SEE_HIDDEN(ch) \
   (AFF_FLAGGED(ch, AFF_SENSE_LIFE) || IS_IMMORT(ch) || IN_ARENA(ch))

#define CAN_SEE_MAGIC(ch) \
   (IS_IMMORT(ch) || AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
/* || IS_ELF(ch) || IS_HEMNOV(ch) || IS_DROW(ch))*/

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 333)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -333)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

/* descriptor-based utils ************************************************/


#define WAIT_STATE(ch, cycle) { \
    if (ch){ \
    if ((ch)->desc) (ch)->desc->wait = (cycle); \
    else if (IS_NPC(ch)) GET_MOB_WAIT(ch) = (cycle); }}

#define CHECK_WAIT(ch)  (((ch)->desc) ? ((ch)->desc->wait > 1) : 0)
#define STATE(d)    ((d)->connected)


/* object utils **********************************************************/

#define ENEMIES(ch, vict) ((GET_ALIGNMENT(ch)!=GET_ALIGNMENT(victim)))

#define IS_BURIED(obj)          ((obj) &&  IS_SET(GET_OBJ_EXTRA(obj), ITEM_BURIED))
#define IS_BLOODY(obj)		((obj) &&  IS_SET(GET_OBJ_EXTRA2(obj), ITEM2_BLOODY))   
#define IS_METAL(obj)		((obj) && IS_SET(GET_OBJ_EXTRA(obj), ITEM_METAL))
#define IS_ORGANIC(obj)		((obj) &&  IS_SET(GET_OBJ_EXTRA(obj), ITEM_ORGANIC))
#define GET_OBJ_SPELL(obj)  ((obj)->bound_spell)
#define GET_OBJ_SPELL_LEVEL(obj)  ((obj)->bound_spell_level)
#define GET_OBJ_SPELL_TIMER(obj)  ((obj)->bound_spell_timer)
#define GET_OBJ_TYPE(obj)   ((obj)->obj_flags.type_flag)
#define GET_OBJ_DATA(obj)   ((obj)->obj_flags.data)
#define GET_OBJ_LEVEL(obj)   ((obj)->obj_flags.data)
#define GET_OBJ_COST(obj)   ((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)   ((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)  ((obj)->obj_flags.extra_flags)
#define GET_OBJ_EXTRA2(obj) ((obj)->obj_flags.extra_flags2)
#define GET_OBJ_EXTRA3(obj) ((obj)->obj_flags.extra_flags3)
#define GET_OBJ_WEAR(obj)   ((obj)->obj_flags.wear_flags)
#define GET_OBJ_BIT(obj)    ((obj)->obj_flags.bitvector)
#define GET_OBJ_BIT2(obj)   ((obj)->obj_flags.bitvector2)
#define GET_OBJ_BIT3(obj)   ((obj)->obj_flags.bitvector3)
#define GET_OBJ_VAL(obj, val)   ((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)
#define GET_OBJ_TIMER(obj)  ((obj)->obj_flags.timer)
#define GET_OBJ_RNUM(obj)   ((obj)->item_number)
#define GET_OBJ_VNUM(obj)   (GET_OBJ_RNUM(obj) >= 0 ? \
                 obj_index[GET_OBJ_RNUM(obj)].virtual : -1)     

#define GET_OBJ_DAMAGE10(obj)    (MIN(10,     ((obj)->damage/10+1) ))
#define GET_OBJ_DAMAGE(obj)    ((obj)->damage)
#define GET_OBJ_ORIG(obj) ((obj)->orig_value)

#define IS_OBJ_STAT(obj,stat)   (IS_SET((obj)->obj_flags.extra_flags,stat))
#define IS_OBJ_STAT2(obj,stat)  (IS_SET((obj)->obj_flags.extra_flags2,stat))


#define ARMOR_COST(obj) (GET_OBJ_LEVEL(obj)*GET_OBJ_LEVEL(obj)*GET_OBJ_LEVEL(obj)/200+(GET_OBJ_VAL(obj, 0))*GET_OBJ_VAL(obj, 0)/32+MIN(30, GET_OBJ_VAL(obj, 0)*2))
#define WPN_COST(level) (MAX((level)*level/14, ((level)*(level)*(level)/(128))))
#define WEAPON_COST(obj) ((WPN_COST((int) AVE_DAM(obj))+MIN(25, (int) ((float) 2.0*AVE_DAM(obj))))*GET_OBJ_DAMAGE(obj)/100)
#define AVE_DAM(obj)	((((float) (GET_OBJ_VAL(obj, 2) + 1) / 2.0) * (float) GET_OBJ_VAL(obj, 1) + GET_OBJ_VAL(obj, 0)))
#define FORGE_LEVEL(obj)  (GET_OBJ_RENT(obj) <0 ? -GET_OBJ_RENT(obj): 0)


#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
    (obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, (part)))

#define CAN_PRACTICE(ch, spell) (spell_info[(spell)].type && (GET_LEVEL((ch)) >= spell_info[(spell)].min_level[GET_CLASS_NUM((ch))]))

/* CLASS/RACE macros */
#define IS_IMPL(ch)     (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_IMPL))
#define IS_GRGOD(ch)    (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_GRGOD))
#define IS_GOD(ch)      (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_GOD))
#define IS_IMMORT(ch)    (!IS_NPC(ch) &&  (GET_LEVEL(ch) >= LVL_IMMORT))

#define CLASS_ABBR(ch)  (IS_NPC(ch) ? "---" : \
            (IS_IMPL(ch) ? "IMP" : \
            (IS_GRGOD(ch) ? "GRG" : \
            (IS_GOD(ch) ? "Dei" : \
            (IS_IMMORT(ch) ? "Imm" : \
            (IS_TRIPLECLASS(ch) ? "Tri" : \
            (IS_DUALCLASS(ch) ? "Dua" : class_abbrevs[(int)GET_CLASS_NUM(ch)])))))))

#define RACE_ABBR(ch)   (IS_NPC(ch) ? "NPC" : race_abbrevs[(int)GET_RACE(ch)])

#define IS_MAGIC_USER(ch)   (IS_SET(GET_CLASS(ch), CLASS_MAGIC_USER))
#define IS_CLERIC(ch)       (IS_SET(GET_CLASS(ch), CLASS_CLERIC) || IS_SET(GET_CLASS(ch), CLASS_NEWCLERIC))
#define IS_THIEF(ch)        (IS_SET(GET_CLASS(ch), CLASS_THIEF))
#define IS_WARRIOR(ch)      (IS_SET(GET_CLASS(ch), CLASS_WARRIOR))
#define IS_RANGER(ch)       (IS_SET(GET_CLASS(ch), CLASS_RANGER))
#define IS_BARD(ch)     (IS_SET(GET_CLASS(ch), CLASS_BARD))
#define IS_DRUID(ch)        (IS_SET(GET_CLASS(ch), CLASS_DRUID))
#define IS_NECRON(ch)       (IS_SET(GET_CLASS(ch), CLASS_NECRON))
#define IS_MONK(ch)     (IS_SET(GET_CLASS(ch), CLASS_MONK) || MOB_FLAGGED(ch,MOB_MONK))
#define IS_DEATHK(ch)       (IS_SET(GET_CLASS(ch), CLASS_DEATHK))
#define IS_ADVENTURER(ch)        (IS_SET(GET_CLASS(ch), CLASS_NEWCLERIC))

#define IS_CLASS_MAGE_LIKE(ch) (IS_MAGIC_USER(ch) || IS_NECRON(ch))
#define IS_CLASS_CLERIC_LIKE(ch) (IS_CLERIC(ch) || IS_DRUID(ch) )
#define IS_CLASS_WARRIOR_LIKE(ch) (IS_WARRIOR(ch) || IS_RANGER(ch) || IS_MONK(ch) || IS_DEATHK(ch))
#define IS_CLASS_THIEF_LIKE(ch) (IS_THIEF(ch) || IS_BARD(ch))

#define IS_CASTER(ch)           (!IS_NPC(ch) && (IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_RANGER(ch) || IS_DRUID(ch) || IS_NECRON(ch) || IS_DEATHK(ch) ||  IS_SET(GET_CLASS(ch), CLASS_NEWCLERIC)))
#define IS_CASTERMOB(ch) ( (IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_RANGER(ch) || IS_DRUID(ch)))
#define SORP(ch) (IS_CLERIC(ch)?"prayer":"spell")

#define GET_CLASS_NUM(ch)   ( \
                IS_MAGIC_USER(ch) ?   0  : ( \
                IS_WARRIOR(ch) ?      3  : ( \
                IS_ADVENTURER(ch) ?   10  : ( \
                IS_DEATHK(ch) ?       9  : ( \
                IS_MONK(ch) ?         8  : ( \
                IS_CLERIC(ch) ?       1  : ( \
                IS_RANGER(ch) ?    4  : ( \
                IS_BARD(ch) ?      5  : ( \
                IS_DRUID(ch) ?        6  : ( \
                IS_NECRON(ch) ?       7  : ( \
                IS_THIEF(ch) ?        2  : -1 ) ) ) ) ) ) ) ) ) ) )

#define GET_CLASS_NUM_FULL(ch)  ( \
                IS_TRIPLECLASS(ch) ? 12  : ( \
                IS_DUALCLASS(ch) ?   11  : ( \
                IS_ADVENTURER(ch) ?       10  : ( \
                IS_DEATHK(ch) ?       9  : ( \
                IS_MONK(ch) ?         8  : ( \
                IS_NECRON(ch) ?       7  : ( \
                IS_DRUID(ch) ?        6  : ( \
                IS_BARD(ch) ?      5  : ( \
                IS_RANGER(ch) ?    4  : ( \
                IS_WARRIOR(ch) ?      3  : ( \
                IS_THIEF(ch) ?        2  : ( \
                IS_CLERIC(ch) ?       1  : ( \
                IS_MAGIC_USER(ch) ?   0  : -1  ) ) ) ) ) ) ) ) ) ) ) ) ) 

#define IS_DUALCLASS(ch)    (IS_SET(GET_CLASS(ch), CLASS_DUAL))
#define IS_TRIPLECLASS(ch)  (IS_SET(GET_CLASS(ch), CLASS_TRIPLE))

#define IS_OGRE(ch)         (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_OGRE))
#define IS_TROLL(ch)            (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_TROLL))
#define IS_ORC(ch)          (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_ORC))

#define IS_ENT(ch)          (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_ENT))


#define IS_DROW(ch)     (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_DROW))
#define IS_HUMAN(ch)        (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_HUMAN))
#define IS_ELF(ch)      (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_ELF))
#define IS_GOBLIN(ch)       (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_GOBLIN))
#define IS_DWARF(ch)        (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_DWARF))
#define IS_HALFLING(ch)     (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_HALFLING))
#define IS_GNOME(ch)        (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_GNOME))
#define IS_HEMNOV(ch)       (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_HEMNOV))
#define IS_LLYRA(ch)        (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_LLYRA))
#define IS_MINOTAUR(ch)     (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_MINOTAUR))
#define IS_PIXIE(ch)        (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_PIXIE))

#define IS_WEREFORM(ch)     (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_WEREFORM))

#define IS_DRAGON(ch)       (!IS_NPC(ch) && \
                (GET_RACE(ch) == RACE_DRAGON))

#define IS_DARKGNOME(ch)    (IS_GNOME(ch) && PRF2_FLAGGED(ch, PRF2_WAR_DRUHARI))

#define IS_GOODGNOME(ch)    (IS_GNOME(ch) && !PRF2_FLAGGED(ch, PRF2_WAR_DRUHARI))

/* compound utilities and other macros **********************************/


#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


/* Various macros building up to CAN_SEE */
#define IS_SUPERMOB(mob) (mob && (IS_NPC(mob) && (GET_MOB_VNUM(mob)==SUPERMOB)))

#define LIGHT_OK(sub)   (!IS_AFFECTED(sub, AFF_BLIND) && !AFF3_FLAGGED(sub, AFF3_TEMP_BLIND) &&\
   (IN_ARENA(sub) || IS_LIGHT((sub)->in_room) || CAN_SEE_IN_DARK(sub)))

#define INVIS_OK(sub, obj) \
 ((IN_ARENA(sub) || ((!IS_AFFECTED((obj),AFF_INVISIBLE) || (IS_AFFECTED(sub,AFF_DETECT_INVIS) && GET_LEVEL(sub)>=GET_LEVEL(obj)) ) && \
 (!IS_AFFECTED((obj), AFF_HIDE) || ((CAN_SEE_HIDDEN(sub) || (IS_AFFECTED(sub, AFF_HIDE) && (sub)->in_room==(obj)->in_room)) && GET_LEVEL(sub)>=GET_LEVEL(obj))))) && !MOB_FLAGGED(obj, MOB_ETHEREAL))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || PRF_FLAGGED(sub, PRF_HOLYLIGHT) || IS_SUPERMOB(sub))

#define SELF(sub, obj)  ((sub) == (obj))

#define HIRED_BY(ch) ((ch)->char_specials.hired_by)

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || IN_ARENA(sub) || \
   ((GET_REAL_LEVEL(sub) >= GET_INVIS_LEV(obj)) && IMM_CAN_SEE(sub, obj)))

/* End of CAN_SEE */

//#define ENEMIES(ch, vict) (!IS_NPC(ch) && !IS_NPC(vict) && (IS_GOOD((ch)) && IS_EVIL((vict))) || ((IS_GOOD((vict)) && IS_EVIL((ch)))))

#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || IS_AFFECTED((sub), AFF_DETECT_INVIS) || IS_IMMORT(sub) || IS_SUPERMOB(sub))

#define MORT_CAN_SEE_OBJ(sub, obj) (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   ((MORT_CAN_SEE_OBJ(sub, obj)  && !IS_BURIED(obj)) || PRF_FLAGGED((sub), PRF_HOLYLIGHT))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))

//#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? (ENEMIES(ch, vict) ? "you worst enemy" : GET_NAME(ch)) : "someone")
#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : "someone")
#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
    (obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
    fname((obj)->name) : "something")


#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])
#define EXITN(room, door)               (world[room].dir_option[door])
#define _2ND_EXIT(ch, door) (world[EXIT(ch, door)->to_room].dir_option[door])
#define _3RD_EXIT(ch, door) (world[_2ND_EXIT(ch, door)->to_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
             (EXIT(ch,door)->to_room != NOWHERE) && \
             !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
#define CAN_LISTEN_BEHIND_DOOR(ch,dir)  (EXIT(ch, dir) && \
                (EXIT(ch, dir)->to_room != NOWHERE) && \
                 IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED))


#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS) && !(SECT(ch->in_room)==SECT_INSIDE))

#define lvD2(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 2)))
#define lvD3(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 3)))
#define lvD4(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 4)))
#define lvD5(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 5)))
#define lvD6(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 6)))
#define lvD8(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 8)))
#define lvD10(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 10)))
#define lvD12(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 12)))
#define lvD15(ch) ( MAX(1, (int) (GET_LEVEL(ch) / 15)))

#define MOB_EXP_BASE	  900.0
#define PLAYERS_PER_LEVEL 20
#define DEFAULT_MOBRATIO  30.0    
#define TOTAL_MOBNUM      6000

int             level_power(int base, int n);

#define GET_MOBRATIO(ch)     ((ch)->player_specials->saved.mobratio)
#define LEVELEXP(ch) ((int) (GET_MOBRATIO(ch)*MOB_EXP_BASE))
//#define LEVELDIFF(base, lev1, lev2) (base+8*base*(lev2-lev1)/100)
#define LEVELDIFF(base, lev1, lev2) (level_power(base, lev2-lev1))


//#define MOB_EXP(lev) (power(lev-1))

//#define MOBS_PER_LEVEL(lev) ((int) (MIN(200, lev*(15+lev))))

//#define MOBS_PER_LEVEL(lev) ((int) (20+lev*8)




/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
#endif

/*
 * Some systems such as Sun's don't have prototyping in their header files.
 * Thus, we try to compensate for them.
 *
 * Much of this is from Merc 2.2, used with permission.
 */

#if defined(_AIX)
char    *crypt(const char *key, const char *salt);
#endif

#if defined(apollo)
int atoi (const char *string);
void    *calloc( unsigned nelem, size_t size);
char    *crypt( const char *key, const char *salt);
#endif

#if defined(hpux)
char    *crypt(char *key, const char *salt);
#endif

#if defined(linux)
char    *crypt( const char *key, const char *salt);
#endif

#if defined(MIPS_OS)
char    *crypt(const char *key, const char *salt);
#endif

#if defined(NeXT)
char    *crypt(const char *key, const char *salt);
int unlink(const char *path);
int getpid(void);
#endif

/*
 * The proto for [NeXT's] getpid() is defined in the man pages are returning
 * pid_t but the compiler pukes on it (cc). Since pid_t is just
 * normally a typedef for int, I just use int instead.
 * So far I have had no other problems but if I find more I will pass
 * them along...
 * -reni
 */

#if defined(sequent)
char    *crypt(const char *key, const char *salt);
int fclose(FILE *stream);
int fprintf(FILE *stream, const char *format, ... );
int fread(void *ptr, int size, int n, FILE *stream);
int fseek(FILE *stream, long offset, int ptrname);
void    perror(const char *s);
int ungetc(int c, FILE *stream);
#endif

#if defined(sun)
#include <memory.h>
void    bzero(char *b, int length);
char    *crypt(const char *key, const char *salt);
int fclose(FILE *stream);
int fflush(FILE *stream);
void    rewind(FILE *stream);
int sscanf(const char *s, const char *format, ... );
int fprintf(FILE *stream, const char *format, ... );
int fscanf(FILE *stream, const char *format, ... );
int fseek(FILE *stream, long offset, int ptrname);
size_t  fread(void *ptr, size_t size, size_t n, FILE *stream);
size_t  fwrite(const void *ptr, size_t size, size_t n, FILE *stream);
void    perror(const char *s);
int ungetc(int c, FILE *stream);
time_t  time(time_t *tloc);
int system(const char *string);
#endif

#if defined(ultrix)
char    *crypt(const char *key, const char *salt);
#endif

#if defined(DGUX_TARGET)
#ifndef NOCRYPT
#include <crypt.h>
#endif
#define bzero(a, b) memset((a), 0, (b))
#endif

#if defined(sgi)
#include <bstring.h>
#ifndef NOCRYPT
#include <crypt.h>
#endif
#endif


/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#define CRYPT(a,b) (a)

#if defined(ADV_MEMORY)
	#include "memory.h"
#endif

#define GET_PRIVATE(ch)         ((ch)->player_specials->priv)
#define ROOM_AFFECTIONS(loc)    (world[(loc)].room_affections)
#define ROOM_AFFECTED(loc, aff) (IS_SET(ROOM_AFFECTIONS(loc), (aff)))
char *strupper( const char *str );
char *strlower( const char *str );

#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	logs( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free((point));						\
  point = NULL;							\
} while(0)


#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
	logs( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)

void ch_printf(CHAR_DATA *ch, char *fmt, ...);

#define SEEMISS_F(ch) ((!IS_NPC(ch) && !PRF2_FLAGGED(ch, PRF2_NOMISSF)))
#define SEEMISS_E(ch) ((!IS_NPC(ch) && !PRF2_FLAGGED(ch, PRF2_NOMISSE)))

#define NUM_PETS_ALLOWED 1
#define MSG_TOO_MANY_PETS "You already have a mob follower.\r\n"


#define FAITH_WORTHLESS(ch) (GET_FAITH(ch)<=-250)
#define FAITH_QUESTIONABLE(ch) ((GET_FAITH(ch)<=-100))
#define FAITH_NORMAL(ch)  ((GET_FAITH(ch)>=-100) && (GET_FAITH(ch)<100))
#define FAITH_STRONG(ch)  ((GET_FAITH(ch)>=100))
#define FAITH_EXCEPTIONAL(ch) (GET_FAITH(ch)>=250)


#define CANBE_ARMOR(obj) (GET_OBJ_WEAR(obj) & (ITEM_WEAR_BODY | ITEM_WEAR_HEAD | ITEM_WEAR_LEGS  | ITEM_WEAR_FEET  | ITEM_WEAR_HANDS  | ITEM_WEAR_ARMS  | ITEM_WEAR_SHIELD  | ITEM_WEAR_ABOUT  | ITEM_WEAR_FACE))
#define CANBE_DAMAGE(obj)    (GET_OBJ_WEAR(obj) & (ITEM_WEAR_WIELD | ITEM_WEAR_HOLD  | ITEM_WEAR_HANDS  | ITEM_WEAR_FEET | ITEM_WEAR_FINGER | ITEM_WEAR_NECK | ITEM_WEAR_WRIST | ITEM_WEAR_HEAD | ITEM_WEAR_SHIELD)) 

#define G_SENDOK(ch) (AWAKE(ch) && \
		    !PLR_FLAGGED((ch), PLR_WRITING) && \
		    !PLR_FLAGGED((ch), PLR_EDITING))



extern struct Sdeity deity_list[];