/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define SAVETYPE_GROUP -345312

#define MAGIC_SPRING	     10001

#define DEFAULT_STAFF_LVL	10
#define DEFAULT_WAND_LVL	10


#define CAST_UNDEFINED	-1
#define CAST_SPELL	0
#define CAST_POTION	1
#define CAST_WAND	2
#define CAST_STAFF	3
#define CAST_SCROLL	4
#define CAST_BOUND_SPELL	5

#define MAG_DAMAGE	(1 << 0)
#define MAG_AFFECTS	(1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS	(1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS	(1 << 5)
#define MAG_MASSES	(1 << 6)
#define MAG_AREAS	(1 << 7)
#define MAG_SUMMONS	(1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL	(1 << 10)
#define MAG_CASTERS     (1 << 11)
#define MAG_ROOM        (1 << 12)

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */
#define MAX_SPELLS		    299
#define TOP_SPELL_DEFINE	    500
/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */

#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                   7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOR_SPRAY            10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARGE                 34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MASS_CHARM             41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD	     45 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD	     46 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SURCEASE	     47 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL	     48 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL	     49 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION	     50 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERWALK		     51 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_RELOCATE		     52
#define SPELL_PEACE		     53
#define SPELL_FLY		     54
#define SPELL_SLOW                   55
#define SPELL_PROT_FIRE              56
#define SPELL_WATERBREATH	     57
#define SPELL_EAGLE_EYE		     58
#define SPELL_GROUP_INVIS            59
#define SPELL_POWER_HEAL             60
#define SPELL_COLD_ARROW	     61
#define SPELL_FLAME_ARROW	     62
#define SPELL_CURE_SERIOUS	     63
#define SPELL_MINUTE_METEOR	     64
#define SPELL_AREA_LIGHTNING	     65
#define SPELL_STRONG_MIND            66
#define SPELL_SHIELD		     67
#define SPELL_WRAITHFORM	     68
#define SPELL_WEAKEN		     69
#define SPELL_STONESKIN		     70
#define SPELL_CONE_OF_COLD	     71
#define SPELL_EQUINOX		     72
#define SPELL_BARKSKIN               73
#define SPELL_MATERIALIZE            74
#define SPELL_FAERIE_FOG             75
#define SPELL_ENDURANCE              76
#define SPELL_CONJ_ELEMENTAL	     77
#define SPELL_MONSUM_I		     78
#define SPELL_NAP		     79
#define SPELL_DEFLECTION	     80
#define SPELL_GROUP_DEFLECTION       81
#define SPELL_MONSUM_V               82
#define SPELL_FEAR                   83
#define SPELL_FAERIE_FIRE            84
#define SPELL_REGENERATE             85
#define SPELL_FORTRESS               86
#define SPELL_ACID_BLAST       	     87
#define SPELL_MIRROR_IMAGE           88
#define SPELL_BLINK                  89
#define SPELL_HASTE	             90
#define SPELL_DISPEL_MAGIC           91
#define SPELL_DEATHDANCE	     92
#define SPELL_PROT_COLD		     93
#define SPELL_DISINTIGRATE	     94
#define SPELL_FIRESTORM		     95
#define SPELL_GUINESS		     96
#define SPELL_GUST_OF_WIND	     97
#define SPELL_HOLD_MONSTER	     98
#define SPELL_HOLY_WORD		     99
#define SPELL_LOCATE_CREATURE        100
#define SPELL_PIDENTIFY              101
#define SPELL_ARROW_RAIN	     102
#define SPELL_FIRE_SHIELD            103
#define SPELL_GROUP_FIRE_SHIELD	     104
#define SPELL_FORCE_FIELD            105
#define SPELL_GROUP_FORCE_FIELD      106
#define SPELL_ENH_HEAL		     107
#define SPELL_ENH_MANA		     108
#define SPELL_ENH_MOVE		     109
#define SPELL_ASTRAL_WALK	     110
#define SPELL_ADRENALIN		     111
#define SPELL_POWER_OF_NATURE	     112
#define SPELL_FLAMESTRIKE	     113
#define SPELL_RESTORE   	     114
#define SPELL_SYNOST    	     115
#define SPELL_IMPROVE_STAT	     116
#define SPELL_AURA		     117
#define SPELL_CURE_DRUNK	     118
#define SPELL_TRANSPORT 	     119
#define SPELL_RETRANSPORT	     120
#define SPELL_ARCANE		     121
#define SPELL_PORTAL		     122
#define SPELL_ENCHANT_ARMOR	     123
#define SPELL_CREATE_SPRING	     124
#define SPELL_PROT_LIGHTNING	     125
#define SPELL_HEALING_TOUCH	     126
#define SPELL_WRATH_OF_EARTH	     127
#define SPELL_CLAIRVOYANCE	     128
#define SPELL_CONJURING_BEAM	     129
#define SPELL_RESSURECTION	     130
#define SPELL_GIVE_LIFE			131
#define SPELL_PRISMATIC_SPHERE		132
#define SPELL_ULTRA_DAMAGE	     	133
#define SPELL_PETRIFY		     	134
#define SPELL_CREATE_RAIN	     	135
#define SPELL_AREA_EARTHQUAKE		136
#define SPELL_GROUP_POWER_HEAL		137
#define SPELL_GIANT_STRENGTH		138
#define SPELL_RAGNAROK			139
#define SPELL_TORNADO           140
#define SPELL_ASTRAL_PROJECTION 141
#define SPELL_OMNI_EYE          142
#define SPELL_BEAUTIFY          143
#define SPELL_HOLY_TOUCH        144
#define SPELL_LOCAL_TELEPORT    145
#define SPELL_INTELLIGIZE       146
#define SPELL_RECHARGE          147
#define SPELL_free              148
#define SPELL_SHROUD_OF_OBSCUREMENT       149
#define SPELL_MASS_SLEEP        150
#define SPELL_WEB               151
#define SPELL_MAGICAL_PROTECTION 152                               
#define SPELL_RUNIC_SHELTER     153
#define SPELL_SUNRAY            154
#define SPELL_MELLON_COLLIE     155
#define SPELL_SHILLELAGH        156
#define SPELL_FEEBLEMIND        157
#define SPELL_SHAMROCK          158                                
#define SPELL_SPIRITUAL_HAMMER  159
#define SPELL_BENEDICTION       160
#define SPELL_DIVINITY          161
#define SPELL_INTESIFY          162
#define SPELL_SHROUD_OF_DARKNESS 163
#define SPELL_RESTORATION       164
#define SPELL_VITALITY          165
#define SPELL_BAPTIZE           166
#define SPELL_RETRIEVE_CORPSE   167
#define SPELL_PLAGUE            168
#define SPELL_CURE_PLAGUE       169           
#define SPELL_ATONEMENT         170
#define SPELL_AGILITY           171
#define SPELL_ENERGY_CONTAINMENT 172
#define SPELL_HOLY_ARMOR        173
#define SPELL_HOG               174
#define SPELL_ENLIGHTMENT        175
#define SPELL_DARKENING          176
#define SPELL_CHOKE             177

#define PRAYER_CURE		178                              
#define PRAYER_MINOR_P		179                              
#define PRAYER_PROTECTION	180                              
#define PRAYER_HEAL		181                              
#define PRAYER_MAJOR_P		182                              
#define PRAYER_GREATER_P	183                              
#define PRAYER_INVIGORATE	184                              
#define PRAYER_DIVINE_P		185                                         
#define PRAYER_INVIGORATE_ALL	186                                         
#define PRAYER_MIRACLE		187                                         
#define PRAYER_SANCTUARY	188                                         

#define PRAYER_PUNISHMENT       189                                         
#define PRAYER_DISPEL_EVIL	190                                         
#define PRAYER_SPIRITUAL_HAMMER 191                                         
#define PRAYER_JUDGMENT		192                                         
#define PRAYER_SMITE_EVIL	193                                         
#define PRAYER_HOJ		194                                         
#define PRAYER_RAPTURE		195                                         
#define PRAYER_WOG		196                                         
#define PRAYER_DIVINE_I		197                                         
#define PRAYER_SENSE_ALIGN      198                                         
#define PRAYER_QUENCH           199


                                         
#define PRAYER_SATE	        221                                         
#define PRAYER_ILLUMINATION	222
#define PRAYER_SENSE_LIFE	223
#define PRAYER_RECALL		224
#define PRAYER_PEACE		225
#define PRAYER_SUMMON		226
#define PRAYER_VOICE_OF_TRUTH	227
#define PRAYER_VISIONS		228
#define PRAYER_REJUVENATE	229
#define PRAYER_BLINDING_LIGHT	230
#define PRAYER_INSPIRATION	231
#define PRAYER_SPIRITUAL_ASYLUM 232
#define PRAYER_REVELATION	233
#define PRAYER_GUIDING_LIGHT	234
#define PRAYER_DIVINE_PRESENCE	235
#define PRAYER_RETRIEVE_CORPSE	236
#define PRAYER_TRUE_SEEING	237
#define PRAYER_SUMMON_AVATAR  	238
#define PRAYER_RESSURECTION   	239
#define PRAYER_WISH             240
#define PRAYER_BLESSING         241
#define PRAYER_DISPEL_GOOD      242
#define PRAYER_SMITE_GOOD       243
#define PRAYER_SHIELD_OF_FAITH  244






/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS                              
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with objects (such as SPELL_IDENTIFY used with scrolls of
 *  identify) or non-players (such as NPC-only spells).
*/                                 
#define SPELL_200		     200
#define SPELL_IDENTIFY               201
#define SPELL_BURN		             202
#define SPELL_FREEZE                 203
#define SPELL_ACID                   204
#define SPELL_CRIT_HIT               205
#define SPELL_FIRE_BREATH            206
#define SPELL_GAS_BREATH             207
#define SPELL_FROST_BREATH           208
#define SPELL_ACID_BREATH            209
#define SPELL_LIGHTNING_BREATH       210
#define SPELL_QUAD_DAMAGE            211
#define SPELL_RESTORE_MANA           212
#define SPELL_BLADEBARRIER           213
#define SPELL_214	             214
#define SPELL_215	             215
#define SPELL_216	             216
#define SPELL_217	             217
#define SPELL_218	             218
#define SPELL_219	             219
#define SPELL_220	             220


#define TOP_SPELL_USED  	     241


#define IS_COMBAT_SKILL(i) (!(i!=SKILL_DODGE && i!=SKILL_PARRY && i!=SKILL_SHIELD && i!=SKILL_HIT && i!=SKILL_PIERCE && i!=SKILL_SLASH && i!=SKILL_POUND && i!=SKILL_AGGRESIVE && i!=SKILL_EVASIVE))
/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_HIT                     300
#define SKILL_STING                   301
#define SKILL_WHIP                    302
#define SKILL_SLASH                   303
#define SKILL_BITE                    304
#define SKILL_BLUDGEON                305
#define SKILL_CRUSH                   306
#define SKILL_POUND                   307
#define SKILL_CLAW                    308
#define SKILL_MAUL                    309
#define SKILL_THRASH                  310
#define SKILL_PIERCE                  311
#define SKILL_BLAST	              312
#define SKILL_PUNCH	 	      313
#define SKILL_STAB		      314
#define SKILL_MISSILE		      315

#define SKILL_HIT_MASTER                     316
#define SKILL_STING_MASTER                   317
#define SKILL_WHIP_MASTER                    318
#define SKILL_SLASH_MASTER                   319
#define SKILL_BITE_MASTER                    320
#define SKILL_BLUDGEON_MASTER                321
#define SKILL_CRUSH_MASTER                   322
#define SKILL_POUND_MASTER                   323
#define SKILL_CLAW_MASTER                    324
#define SKILL_MAUL_MASTER                    325
#define SKILL_THRASH_MASTER                  326
#define SKILL_PIERCE_MASTER                  327
#define SKILL_BLAST_MASTER	             328
#define SKILL_PUNCH_MASTER	 	     329
#define SKILL_STAB_MASTER		     330
#define SKILL_MISSILE_MASTER		     331

#define SKILL_DODGE		    332
#define SKILL_PARRY		    333
#define SKILL_SHIELD		    334
#define SKILL_DODGE_MASTERY         335
#define SKILL_PARRY_MASTERY         336
#define SKILL_SHIELD_MASTERY        337
#define SKILL_ENH_SIGHT		    338
#define SKILL_FILLET                339
#define SKILL_COOK                  340
#define SKILL_EVASIVE               341
#define SKILL_AGGRESIVE             342
#define SKILL_SWIMMING		    343
#define SKILL_RIDING		    344



#define SKILL_BACKSTAB              346 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                  347 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                  348 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                  349 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK             350 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRIP                  351 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                352 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                 353 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                 354 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK		    355 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_DISARM                356
#define SKILL_DUAL_BACKSTAB         357
#define SKILL_SECOND_ATTACK         358
#define SKILL_THIRD_ATTACK          359
#define SKILL_ENH_DAMAGE            360
#define SKILL_DUAL_WIELD            361


#define frees1 362 
#define frees2 363 
#define frees3 364 
#define frees4 365 
#define frees5 366

#define SKILL_MOVE_HIDDEN           367
#define SKILL_BERSERK 		    368
#define SKILL_STUN		    369
#define SKILL_FIST_OF_DOOM	    370
#define SKILL_DOORBASH		    371
#define SKILL_FOURTH_ATTACK	    372
#define SKILL_POWER_BLOW	    373
#define SKILL_SECOND_DUAL	    374
#define SKILL_ELBOW_SWING           375
#define SKILL_KNEE_THRUST	    376
#define SKILL_QUIVER_PALM           377
#define SKILL_BREW		    378
#define SKILL_SCRIBE		    379
#define SKILL_FORGE		    380
#define SKILL_ALFA_NERVE	    381
#define SKILL_BETA_NERVE	    382
#define SKILL_GAMMA_NERVE	    383
#define SKILL_CIRCLE		    384
#define SKILL_CONSIDER              385
#define SKILL_LISTEN		    386

#define frees6 387

#define SKILL_HAMMER		    388
#define SKILL_FAME		    389
#define SKILL_FEIGN_DEATH	    390
#define SKILL_BEHEAD		    391
#define SKILL_MAP                   392
#define SKILL_KATA		    393
#define SKILL_MEDITATE              394
#define SKILL_IMPROVED_STEAL        395
#define SKILL_ARCHIRAGE             396
#define SKILL_BATTLECRY             397
#define SKILL_STALK                 398
#define SKILL_ESCAPE                399
#define SKILL_SWITCH                400
#define SKILL_COVER_TRACKS          401
#define SKILL_HUNT                  402
#define SKILL_MEDIC                 403
#define SKILL_BANDAGE               404
#define SKILL_ARCHERY               405
#define SKILL_FORAGE                406
#define SKILL_PEEK                  407
#define SKILL_CHOKE                 408
#define SKILL_SPY                   409
#define SKILL_PLANT                 410
#define SKILL_HAGGLE                411
#define SKILL_WHIRLWIND             412
#define SKILL_MELEE                 413
#define SKILL_RIPOSTE               414
#define SKILL_GUARD                 415
#define SKILL_SPIN_KICK             416
#define SKILL_TRANSFER              417
#define SKILL_GOUGE                 418
#define SKILL_BUDDHA_FINGER         419
#define SKILL_ELBOW2                420
#define SKILL_KNEE2                 421
#define SKILL_SWEEPKICK             422
#define SKILL_CRITICAL_HIT          423
#define SKILL_COMBO                 424
#define SKILL_RETREAT               425
#define SKILL_SPRING_LEAP           426
#define SKILL_SPELL_RECOVERY        427
#define SKILL_QUICK_CHANT           428
#define SKILL_SCARE                 429
#define SKILL_AUTOPSY               430
#define SKILL_SHARPEN               431
#define SKILL_GREATER_ENCH          432

// new warrior
#define SKILL_FEINT		    433
#define SKILL_ARTOFWAR              434
#define SKILL_LUNGE		    435
#define SKILL_COUNTERSTRIKE         436
#define SKILL_KICKFLIP              437
#define SKILL_GUT                   438
#define SKILL_LEADERSHIP            439
#define SKILL_CHARGE                440
#define SKILL_SIXTHSENSE            441
#define SKILL_KNOCKOUT              442
#define SKILL_GRIP                  443
#define SKILL_SHIELDMASTERY         444

//new thief
#define SKILL_DODGEMASTERY	    445
#define SKILL_SPOT_TRAPS            446
#define SKILL_EAVESDROP		    447
#define SKILL_DUCK		    448
#define SKILL_DIRTKICK		    449
#define SKILL_BLINDFIGHTING	    450
#define SKILL_AMBUSH		    451
#define SKILL_DANCEOFDAGGERS        452
#define SKILL_ENVENOM		    453
#define SKILL_DISGUISE 		    454
#define SKILL_CUTTHROAT             455
#define SKILL_TUMBLE                456
#define SKILL_ASSASSINATE           457
#define SKILL_TAUNT                 458


#define SKILL_TRAP_DART		    459
#define SKILL_TRAP_POISON           460
#define SKILL_TRAP_HORROR           461
#define SKILL_TRAP_FIRE		    462
#define SKILL_TRAP_LOCAL_TELEPORT   463
#define SKILL_TRAP_BLIND            464
#define SKILL_TRAP_STUN             465
#define SKILL_TRAP_ACID             466
#define SKILL_TRAP_WEB              467
#define SKILL_TRAP_SLEEP            468
#define SKILL_TRAP_TELEPORT         469
#define SKILL_TRAP_ROPE             470


#define SKILL_THROW_DAGGER          471
#define SKILL_DISTRACT              472
#define SKILL_DUAL_CIRCLE           473
#define SKILL_PUSH		    474
#define SKILL_DRAG		    475
#define SKILL_GROUNDCONTROL	    476
#define SKILL_JUMP		    477
#define SKILL_HEADBUTT		    478
#define SKILL_LODGE		    479


#define SKILL_TRAP_CALTROPS	    480
#define SKILL_TRAP_DOOM		    481
#define SKILL_TRAP_GAS		    482

#define IS_TRAP_SKILL(i) ( (((i)>=459) && ((i)<=470)) || (((i)>=480) && ((i)<=482)))

#define SKILL_TACTICS		    485
#define SKILL_ALERTNESS		    486
#define SKILL_FIRSTAID		    487
#define SKILL_NERVE		    488
#define SKILL_RESISTANCE	    489
#define SKILL_DOWNSTRIKE	    490
#define SKILL_MONSTERLORE	    491

#define SKILL_FASTHEALING	    492
#define SKILL_WARRIORCODE	    493              
#define SKILL_AMBIDEX		    494





#define TOP_SKILL_USED  	     493











/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     300
#define TYPE_STING                   301
#define TYPE_WHIP                    302
#define TYPE_SLASH                   303
#define TYPE_BITE                    304
#define TYPE_BLUDGEON                305
#define TYPE_CRUSH                   306
#define TYPE_POUND                   307
#define TYPE_CLAW                    308
#define TYPE_MAUL                    309
#define TYPE_THRASH                  310
#define TYPE_PIERCE                  311
#define TYPE_BLAST		     312
#define TYPE_PUNCH		     313
#define TYPE_STAB		     314
#define TYPE_MISSILE		     315

/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SUFFERING		     399




#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct syllable {
    char *org;
    char *new;
};

struct spell_info_type {
    int min_position;	/* Position for caster	 */
    int mana_min;	/* Min amount of mana used by a spell (highest lev) */
    int mana_max;	/* Max amount of mana used by a spell (lowest lev) */
    int mana_change;	/* Change in mana used by spell from lev to lev */

    int min_level[NUM_CLASSES];
    int type;
    int routines;
    int violent;
    sh_int targets;         /* See below for use with TAR_XXX  */
};
extern struct spell_info_type spell_info[];


/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
    char	*singular;
    char	*plural;
};


#define ASPELL(spellname) \
void	spellname(int level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj, char *tar_str)

#define MANUAL_SPELL(spellname)	spellname(level, caster, cvict, ovict,tar_str);

ASPELL(spell_disintigrate);
ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_teleport);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_relocate);
ASPELL(spell_peace);
ASPELL(spell_locate_creature);
ASPELL(spell_astral_walk);
ASPELL(spell_improve_stat);
ASPELL(spell_fear);
ASPELL(spell_portal);
ASPELL(spell_arcane);
ASPELL(spell_enchant_armor);
ASPELL(spell_dispel_magic);
ASPELL(spell_create_spring);
ASPELL(spell_control_weather);
ASPELL(spell_trasnport);
ASPELL(spell_retransport);
ASPELL(spell_gust_of_wind);
ASPELL(spell_tornado);
ASPELL(spell_astral_projection);
ASPELL(spell_omni_eye);
ASPELL(spell_visions);
/* basic magic calling functions */

int find_skill_num(char *name);

void mag_damage(int level, struct char_data *ch, struct char_data *victim,
                int spellnum, int savetype);

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
                 int spellnum, int savetype);

void mag_group_switch(int level, struct char_data *ch, struct char_data *tch,
                      int spellnum, int savetype);

void mag_groups(int level, struct char_data *ch, int spellnum, int savetype);

void mag_masses(int level, struct char_data *ch, int spellnum, int savetype);

void mag_areas(int level, struct char_data *ch, int spellnum, int savetype);

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
                 int spellnum, int savetype);

void mag_points(int level, struct char_data *ch, struct char_data *victim,
                int spellnum, int savetype);

void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
                   int spellnum, int type);

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
                    int spellnum, int type);

void mag_creations(int level, struct char_data *ch, int spellnum);

int	call_magic(struct char_data *caster, struct char_data *cvict,
               struct obj_data *ovict, int spellnum, int level, int casttype,char *tar_str);

void	mag_objectmagic(struct char_data *ch, struct obj_data *obj,
                     char *argument);

int	cast_spell(struct char_data *ch, struct char_data *tch,
               struct obj_data *tobj, int spellnum,char *tar_str);
bool can_cast(struct char_data *ch, int spellnum);
