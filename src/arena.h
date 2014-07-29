#include <sys/types.h>
#ifndef _ARENA_H_
#define _ARENA_H_

#define ARENA_OFF	0
#define ARENA_START	1
#define ARENA_RUNNING	2

#define PULSE_ARENA	(65 RL_SEC)

#define MAZEW 8
#define MAZEU (MAZEW*MAZEW*4)
#define FLAG_RED   24001 /* zone number */
#define FLAG_BLUE   24002 /* zone number */
#define ARENA_ZONE   240 /* zone number */
#define ARENA_PREP_START  30   /* vnum of first prep room	*/
#define ARENA_PREP_END    32   /* vnum of last prep room	*/
#define ARENA_START_ROOM  24000   /* vnum of first real arena room	*/
#define ARENA_END_ROOM    39   /* vnum of last real arena room	*/
#define ARENA_START_CHAL1 390   /* vnum of first challenge room	*/
#define ARENA_END_CHAL1   394   /* vnum of last challenge room	*/
#define ARENA_START_CHAL2 395   /* vnum of first challenge room	*/
#define ARENA_END_CHAL2   399   /* vnum of last challenge room	*/
#define HALL_FAME_FILE "etc/hallfame" /* the arena hall of fame */

#define MAX_BET	    500000 /* max betable */
#define MIN_ARENA_COST	100	/* minimum cost per level */

#define GET_BETTED_ON(ch) ((ch)->char_specials.betted_on)
#define GET_BET_AMT(ch)	  ((ch)->char_specials.bet_amt)

#define ARENA_CTF	1
#define ARENA_DM	2

struct hall_of_fame_element {
    char name[100];
    char lastname[81];
    time_t date;
    long award;
    int type;
    struct  hall_of_fame_element *next;
};

/* in arena.c */
void start_arena();
void show_jack_pot();
void do_game();
int num_in_arena();
void find_game_winner();
void do_end_game();
void start_game();
void silent_end();
void write_fame_list(void);
void write_one_fame_node(FILE * fp, struct hall_of_fame_element * node);
void load_hall_of_fame(void);
//void find_bet_winners(struct char_data *winner);
void sportschan(char *buf);

extern int in_arena;
extern int game_length;
extern int time_to_start;
extern int time_left_in_game;
extern int start_time;

#endif /* _ARENA_H_ */
