#define QUEST_REPEAT 			24 * 60   // ~ 24 hours
#define MIN_PLAYERS_BEFORE_QUEST 	3
#define MIN_PLAYERS_FOR_QUEST   	2
#define QUEST_JOIN_TIME			3      // mud hours

#define QUEST_NONE	0
#define QUEST_MASK_B	1
#define QUEST_MASK_A	2
#define QUEST_MASK_O	3
#define QUEST_GIFT	4
#define QUEST_BERSERK	5
#define QUEST_RUSH	6
#define QUEST_BLOOD	7
#define QUEST_TREASURE  8
#define QUEST_PATH	9
#define QUEST_CHASE	10
#define QUEST_NIRVANA	11

#define QUEST_JOIN	0
#define QUEST_PREPARE	1
#define QUEST_ACTION	2


extern int current_quest, quest_step, quest_time_left, last_quest_timer, quest_join;
int newquest_update();
int quest_mask(int code);
int quest_gift(int code);
int quest_berserk(int code);
int quest_rush(int code);
int quest_blood	(int code);
int quest_treasure(int code);
int quest_path	(int code);
int quest_chase	(int code);
int quest_nirvana(int code);

