/* ************************************************************************
*  File: fight.h                                        Part of CircleMUD *
*  Usage: Combat system                                                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

void hit(struct char_data * ch, struct char_data * victim, int type);
void forget(struct char_data * ch, struct char_data * victim);
void remember(struct char_data * ch, struct char_data * victim);
int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
void mprog_hitprcnt_trigger(struct char_data * mob, struct char_data * ch);
void mprog_death_trigger(struct char_data * mob, struct char_data * killer);
void mprog_fight_trigger(struct char_data * mob, struct char_data * ch);
void appear(struct char_data * ch);
void load_messages(void);
void update_pos(struct char_data * victim);
void check_killer(struct char_data * ch, struct char_data * vict);
void set_fighting(struct char_data * ch, struct char_data * vict);
void stop_fighting(struct char_data * ch);
void make_corpse(struct char_data * ch, struct char_data * killer,int type);
void change_alignment(struct char_data * ch, struct char_data * victim);
int death_cry(struct char_data * ch);
void raw_kill(struct char_data * ch, struct char_data *killer);
void die(struct char_data * ch, struct char_data *killer);
void perform_group_gain(struct char_data * ch, int base, struct char_data * victim);
void group_gain(struct char_data * ch, struct char_data * victim);
char *replace_string(char *str, char *weapon_singular, char *weapon_plural);
void dam_message(int dam, struct char_data * ch, struct char_data * victim,
                 int w_type, struct obj_data *obj);
int skill_message(int dam, struct char_data * ch, struct char_data * vict,
                  int attacktype, struct obj_data *weap);
void damage(struct char_data * ch, struct char_data * victim, int dam,
            int attacktype, struct obj_data *obj);
void hit(struct char_data * ch, struct char_data * victim, int type);
void perform_violence(void);
bool range_hit(struct char_data *ch, struct char_data * victim, struct obj_data *obj);
bool fire_at_char(struct char_data *ch, struct char_data * list, struct obj_data * obj, int dir, char *name);
int check_kill(struct char_data *victim, char whatk[100]);
