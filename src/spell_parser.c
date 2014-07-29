/* **********************************************************************
 *   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define EVENT_MAGIC
#define PULSE_SPELL_EVENT PULSE_SPELL
#define MAX_CAST_TIME	1.8
#define MIN_CAST_TIME	0.8

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "events.h"



extern CHAR_DATA *supermob;
extern void improve_skill(struct char_data * ch, int skill, int mod);
struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];
extern const char     *pc_class_types[];
#define SINFO spell_info[spellnum]

extern struct room_data *world;
char bufma[100];


/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

char *spells[MAX_SKILLS+1] =
    {
        "!RESERVED!",		/* 0 - reserved */

        /* SPELLS */

        "armor",			/* 1 */
        "teleport",
        "_bless",
        "blindness",
        "burning hands",
        "call lightning",
        "charm person",
        "chill touch",
        "clone",
        "color spray",		/* 10 */
        "control weather",
        "_sate",
        "_quench",
        "_cure blind",
        "_cure critic",
        "_cure light",
        "curse",
        "detect alignment",
        "detect invisibility",
        "detect magic",		/* 20 */
        "detect poison",
        "_dispel evil",
        "earthquake",
        "enchant weapon",
        "leech life",
        "fireball",
        "harm",
        "_heal",
        "invisibility",
        "lightning bolt",		/* 30 */
        "locate object",
        "magic missile",
        "poison",
        "spell charge",
        "remove curse",
        "_sanctuary",
        "shocking grasp",
        "sleep",
        "strength",
        "_summon",			/* 40 */
        "mass charm",
        "word of recall",
        "remove poison",
        "_sense life",
        "animate dead",
        "_dispel good",
        "surcease",
        "group heal",
        "group recall",
        "infravision",		/* 50 */
        "waterwalk",
        "portal",
        "domination",
        "fly",
        "slow",			/* 55 */
        "_protect from fire",
        "waterbreathing",
        "eagle eye",
        "group invisibility",
        "power heal",		/* 60 */
        "cold arrow",
        "flame arrow",
        "_cure serious",
        "meteor swarm",
        "chain lightning",		/* 65 */
        "strong mind",
        "mystic shield",
        "wraithform",
        "weaken",
        "stoneskin",		/* 70 */
        "cone of cold",
        "equinox",
        "barkskin",
        "MATERIALIZE",
        "faerie fog",		/* 75 */
        "endurance",
        "conjure elemental",
        "gate",
        "nap",
        "deflection shield",	/* 80 */
        "group deflection shield",
        "_summon ancient dragon",
        "primal fear",
        "faerie fire",
        "regenerate",		/* 85 */
        "fortress",
        "acid blast",
        "mirror image",
        "blink",
        "haste",			/* 90 */
        "dispel magic",
        "death dance",
        "_protect from cold",
        "disintigrate",
        "firestorm",		/* 95 */
        "guinness",
        "gust of wind",
        "hold monster",
        "power word",
        "locate creature",		/* 100 */
        "identify",
        "arrow rain",
        "fire shield",
        "group fire shield",
        "force field",
        "group force field",	/* 106 */
        "enhanced heal",
        "enhanced mana",
        "enhanced move",
        "dimension walk",		/* 110 */
        "adrenaline",
        "power of nature",
        "flamestrike",
        "refresh",
        "synost",			/* 115 */
        "improve stat",
        "holy aura",
        "_cure drunk",
        "transport",
        "retransport",		/* 120 */
        "arcane", "relocate", "enchant armor", "create spring", "_protect from lightning",	/* 125 */
        "wave of healing", "wrath of earth", "clairvoyance", "RAY OF DISRUPTION", "RESSURECTION",	/* 130 */
        "GIVE LIFE", "PRISMATIC SPHERE", "ULTRA DAMAGE", "PETRIFY", "CREATE RAIN",	/* 135 */
        "AREA EARTHQUAKE", "group power heal", "giant strength", "RAGNAROK", "tornado",	/* 140 */
        "astral projection", "omni eye", "beautify", "HOLY TOUCH", "local teleport",	/* 145 */
        "intelligize",     //146
        "recharge",
        "free",
        "shroud of obscurement",
        "mass sleep",
        "web",
        "magical protection",
        "runic shelter",
        "sunray",
        "mellon collie",
        "shillelagh",
        "feeblemind",
        "shamrock",
        "_spiritual hammer",
        "benediction",
        "divinity",
        "intesify",
        "shroud of darkness",
        "restoration",
        "vitality",
        "baptize",
        "retrieve corpse",
        "plague",
        "_cure plague",
        "atonement",
        "agility",
        "energy containment",
        "holy armor",
        "hand of god",
        "enlightment",
        "darkening", //176
        "choke",
        "cure",
        "minor protection",
        "protection",
        "heal",
        "major protection",
        "greater protection",
        "invigorate",
        "divine protection",
        "invigorate all",
        "miracle",
        "sanctuary",
        "punishment",
        "dispel evil",
        "spiritual hammer",
        "judgment",
        "smite evil",
        "hammer of justice",
        "rapture",
        "wrath of god",
        "divine intervention",
        "sense alignment",
        "quench thirst",
        "",
        /* OBJECT SPELLS AND NPC SPELLS/SKILLS */
        "identify",
        "burn",
        "freezee",
        "acid",
        "!critical hit",
        "fire breath",
        "gas breath",
        "frost breath",
        "acid breath",
        "lightning breath",
        "quad damage",
        "restore mana",
        "blade barrier",
        "214",
        "215",
        "216",
        "217",
        "218",
        "219",
        "220",
        "sate hunger",
        "illumination",
        "sense life",
        "recall",
        "peace",
        "summon",
        "voice of truth",
        "visions",
        "rejuvenate",
        "blinding light",
        "inspiration",
        "spiritual asylum",
        "revelation",
        "guiding light",
        "divine presence",
        "retrieve corpse",
        "true seeing",
        "summon avatar ",
        "ressurection",
        "wish",
        "blessing",
        "dispel good",
        "smite good",
        "shield of faith",        
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",

        /* SKILLS starting at 300*/
        "unarmed combat",
        "_sting_",
        "_whip_",
        "slashing weapons",
        "_bite_",
        "_bludgeon_",
        "_crush_",
        "blunt weapons",
        "_claw_",
        "_maul_",
        "_thrash_",
        "piercing weapons",
        "_blast_",
        "_punch_",
        "_stab_",
        "_missile_",

        "martial artist",
        "_sting_m",
        "_whip_m",
        "master in swords",
        "_bite_m",
        "_bludgeon_M",
        "_crush_m",
        "master in blunt weapons",
        "_claw_m",
        "_maul_m",
        "_thrash_m",
        "master in daggers and short swords",
        "_blast_m",
        "_punch_m",
        "_stab_m",
        "_missile_m",
        "dodging",
        "parrying",
        "shield blocking",
        "master dodger",
        "parry mastery",
        "shield block mastery",
        "sharp eye",
        "!fillet",
        "!cook",
        "evasive style",
        "aggressive style",
        "swimming",
        "riding",
        "",
        "backstab",			/* 346 */
        "bash",
        "hide",
        "kick",
        "pick lock",
        "trip",
        "rescue",
        "sneak",
        "steal",
        "track",			/* 355 */
        "disarm",
        "dual backstab",
        "second attack",
        "third attack",
        "enhanced damage",		/* 360 */
        "dual wield",
        "",
        "",
        "",
        "",			/* 365 */
        "",
        "move hidden",
        "berserk",
        "stun",
        "fist of doom",		/* 370 */
        "bashdoor", "fourth attack", "power blow", "second dual attack",	/* 174 */
        "tiger claw", "dragon strike", "quivering palm", "brew", "scribe",	/* 179 */
        "forge", "alfa nerve", "beta nerve", "gamma nerve", "circle backstab",	/* 184 */
        "!consider", "listen", "", "hammering", "fame",	/* 189 */
        "feign death", "behead", "map terrain", "kata", "meditation",	/* 194 */

        "steal mastery", "archirage", "battlecry", "stalk", "escape", "switch",/* 200 */

        "cover tracks",
        "hunt",
        "medic",
        "bandage",
        "archery",
        "forage",
        "peek",
        "choke",
        "spy",
        "conceal",
        "haggle",
        "whirlwind",
        "melee",
        "counter attack",
        "guard",
        "spin kick",
        "concentrate",
        "gouge",
        "buddha finger",
        "elbow swing",
        "knee thrust",
        "sweepkick",
        "critical hit",
        "combo",
        "retreat",
        "spring leap",  //246
        "spell recovery",
        "quick chant",
        "scare",
        "autopsy",             //250
        "sharpen",
        "greater enchant",
        "feint",
        "art of combat",
        "lunge",
        "counterstrike",
        "power kick",
        "gut",
        "leadership",
        "charge",
        "sixth sense",
        "knockout",
        "grip",
        "shield mastery",
        "dodge mastery",
        "spot traps",
        "eavesdrop",
        "duck",
        "dirtkick",
        "blind fighting",
        "ambush",
        "dance of daggers",
        "envenom",
        "disguise",
        "cutthroat",
        "tumble",
        "assassinate",
        "taunt",
        "dart trap",
        "poisonous dart trap",
        "horror trap",
        "fire trap",
        "local teleport trap",
        "blind trap",
        "stun trap",
        "acid trap",
        "web trap",
        "sleep trap",
        "teleport trap",
        "rope trap",
        "knife throwing",
        "distract",
        "dual circle backstab",
        "power push",
        "power drag",
        "ground control",
        "combat jump",
        "headbutt",
        "lodge",
        "caltrops trap",
        "doom trap",
        "gas trap",
        "",
        "",
        "tactics",
        "alertness",
        "first aid",
        "nerve",
        "resist magic",
        "downstrike",
        "monster lore",
        "fast healing",
        "warrior code",        
        "ambidexterity",
        "\n"			/* the end */
    };



struct syllable syls[] = {
                             {" ", " "},
                             {"ar", "abra"},
                             {"ate", "i"},
                             {"cau", "kada"},
                             {"bl", "nost"},
                             {"bur", "mosa"},
                             {"cu", "judi"},
                             {"de", "oculo"},
                             {"dis", "mar"},
                             {"ect", "kamina"},
                             {"en", "uns"},
                             {"gro", "cra"},
                             {"he", "alarc"},
                             {"light", "dies"},
                             {"lo", "hi"},
                             {"magi", "kari"},
                             {"mon", "bar"},
                             {"mor", "zak"},
                             {"move", "sido"},
                             {"ness", "lacri"},
                             {"ning", "illa"},
                             {"peace", "pax"},
                             {"per", "duda"},
                             {"ra", "gru"},
                             {"re", "candus"},
                             {"son", "sabru"},
                             {"tect", "infra"},
                             {"tri", "cula"},
                             {"ven", "nofo"},
                             {"word of", "inset"},
                             {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
                             {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
                             {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
                             {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}, {"1", "b"},
                             {"2", "f"}, {"3", "t"}, {"4", "a"}, {"5", "j"}, {"6", "g"}, {"7", "i"}, {"8", "m"},
                             {"9", "c"}, {"0", "p"},
                         };

int hasenough(struct char_data * ch, int spellnum)
{
    struct char_data *tch, *k;
    struct follow_type *f, *f_next;
    int totmana = 0;
    int mana;

    mana = spell_info[spellnum].mana_min;

    if (ch->master != NULL)
        k = ch->master;
    else
        k = ch;
    for (f = k->followers; f; f = f_next) {
        f_next = f->next;
        tch = f->follower;
        if (tch->in_room != ch->in_room)
            continue;
        if (!IS_AFFECTED(tch, AFF_GROUP))
            continue;
        if (tch == ch)
            continue;
        if (IS_NPC(tch) || !IS_CASTER(tch))
            continue;

        if (GET_LEVEL(tch) < spell_info[spellnum].mana_max)
            continue;
        totmana += GET_MANA(tch);
    }
    if (k != ch)
        totmana += GET_MANA(ch);
    totmana += GET_MANA(k);
    if (totmana < SINFO.mana_min)
        return (mana);
    mana = ((SINFO.mana_min * GET_MANA(ch)) / totmana);
    return (mana);
}

bool can_cast(struct char_data *ch, int spellnum)
{
    int i=mag_manacost(ch, spellnum);
    if (GET_MANA(ch)-i>0)
        return TRUE;
    else return FALSE;

}


int cast_time(struct char_data *ch, int spellnum)
{
    int pom, i, min_level;
    int totmana = 0;
    float mana=0;

    if (!GET_SKILL(ch, spellnum) || !IS_SPELL(spellnum))
    {
    	return -1;
    }
    
    	
        if (IS_SET(SINFO.routines, MAG_DAMAGE))
            mana+=18;
        if (IS_SET(SINFO.routines, MAG_AREAS))
            mana+=28;
        if (IS_SET(SINFO.routines, MAG_ROOM))
            mana+=23;
        if (IS_SET(SINFO.routines, MAG_POINTS))
            mana+=18;
        if (IS_SET(SINFO.routines, MAG_SUMMONS))
            mana+=43;
        if (IS_SET(SINFO.routines, MAG_MASSES))
            mana+=33;
        if (IS_SET(SINFO.routines, MAG_MANUAL))
            mana+=23;
        if (IS_SET(SINFO.routines, MAG_GROUPS))
            mana+=23;
        if (IS_SET(SINFO.routines, MAG_AFFECTS))        
            mana+=11;        
        if (IS_SET(SINFO.routines, MAG_UNAFFECTS))        
            mana+=9;                 
                    
        mana*=(float) (MAX_CAST_TIME-(GET_SKILL(ch, spellnum)/110.0)*(MAX_CAST_TIME-MIN_CAST_TIME));
        
    
    return (MAX(1, (int) (mana+1)));	
}

int mag_manacost(struct char_data * ch, int spellnum)
{

    int pom, i, min_level;
    int totmana = 0;
    float mana;

    min_level = LVL_IMMORT;

    for (i = 0; i < NUM_CLASSES; i++) {
        if (IS_SET(GET_CLASS(ch), 1 << i) && (SINFO.min_level[i] < min_level))
            min_level = SINFO.min_level[i];
    }

    if (SINFO.mana_change!=111)
    {
        mana=min_level;
        mana=MAX(min_level, min_level*min_level/20);

        if (IS_SET(SINFO.routines, MAG_DAMAGE))
            mana*=1.5;
        if (IS_SET(SINFO.routines, MAG_AREAS))
            mana*=3.3;
        if (IS_SET(SINFO.routines, MAG_ROOM))
            mana*=2.5;
        if (IS_SET(SINFO.routines, MAG_POINTS))
            mana*=2.0;
        if (IS_SET(SINFO.routines, MAG_SUMMONS))
            mana*=8;
        if (IS_SET(SINFO.routines, MAG_MASSES))
            mana*=3.1;
        if (IS_SET(SINFO.routines, MAG_MANUAL))
            mana*=2.5;
        if (IS_SET(SINFO.routines, MAG_GROUPS))
            mana/=2;
        if (IS_SET(SINFO.routines, MAG_AFFECTS))
        {
            if (mana!=min_level)
                mana*=1.5;
            else
                mana*=3;
        }
        if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
        {
            if (mana!=min_level)
                mana*=1.2;
            else
                mana*=1.5;
        }
        // Special cases
        if (spellnum == SPELL_DEATHDANCE)
            mana=8*min_level;
        if (spellnum==SPELL_CHARM)
            mana=3*min_level;

        mana*=(float) (190-GET_SKILL(ch, spellnum))/100;
        mana*=1.4;
    }
    else
        mana = SINFO.mana_min;


    /*  mana = MAX(SINFO.mana_max - (SINFO.mana_change *
    		    (GET_LEVEL(ch) - min_level)),
    	     SINFO.mana_min);*/
    if (AFF_FLAGGED(ch, AFF_EQUINOX)) {
        if (mana >= 50)
            mana /= 2;
        else
            mana /= 3;
    }
    return (MAX(1, (int) mana));
}


/* say_spell erodes buf, buf1, buf2 */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch,
               struct obj_data * tobj)
{
    char lbuf[256];
    char lbuf1[256];
    char spec_buf1[1000];
    struct char_data *i;
    int j, ofs = 0;

    if (spellnum>=SPELL_FIRE_BREATH && spellnum<=SPELL_LIGHTNING_BREATH)
        return;

    if (ch==supermob)
        return;

    *spec_buf1=*lbuf=*lbuf1=*buf = *buf1= *buf2= '\0';
    strcpy(lbuf, spells[spellnum]);

    while (*(lbuf + ofs)) {
        for (j = 0; *(syls[j].org); j++) {
            if (!strn_cmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
                strcat(buf, syls[j].new);
                ofs += strlen(syls[j].org);
            }
        }
    }


    if (tch != NULL && tch->in_room == ch->in_room) {
        if (tch == ch)
        {
            sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
            sprintf(lbuf1, "You close your eyes and utter the words, '%%s'.");
        }
        else
        {
            sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
            sprintf(lbuf1, "You stare at $N and utter the words, '%%s'.");
        }
    } else if (tobj != NULL &&
               ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    {
        sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
        sprintf(lbuf1, "You stare at $p and utter the words, '%%s'.");
    }
    else
    {
        sprintf(lbuf, "$n utters the words, '%%s'.");
        sprintf(lbuf1, "You utter the words, '%%s'.");
    }

    sprintf(buf1, lbuf, spells[spellnum]);
    sprintf(spec_buf1, lbuf1, buf);
    act(spec_buf1, FALSE, ch, tobj, tch, TO_CHAR);
    sprintf(buf2, lbuf, buf);

    for (i = world[ch->in_room].people; i; i = i->next_in_room) {
        if (i== ch || i == tch || !i->desc || !AWAKE(i))
            continue;
        if (GET_CLASS(ch) == GET_CLASS(i) || IS_GOD(i))
            perform_act(buf1, ch, tobj, tch, i);
        else
            perform_act(buf2, ch, tobj, tch, i);
    }

    if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
        sprintf(buf1, "$n stares at you and utters the words, '%s'.",
                GET_CLASS(ch) == GET_CLASS(tch) ? spells[spellnum] : buf);
        act(buf1, FALSE, ch, NULL, tch, TO_VICT);
    }
}


int find_skill_num(char *name)
{
    int index = 0, ok;
    char *temp, *temp2;
    char first[256], first2[256];

    while (*spells[++index] != '\n') {
        if (is_abbrev(name, spells[index]))
            return index;

        ok = 1;
        temp = any_one_arg(spells[index], first);
        temp2 = any_one_arg(name, first2);
        while (*first && *first2 && ok) {
            if (!is_abbrev(first2, first))
                ok = 0;
            temp = any_one_arg(temp, first);
            temp2 = any_one_arg(temp2, first2);
        }

        if (ok && !*first2)
            return index;
    }

    return -1;
}

/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data * caster, struct char_data * cvict,
               struct obj_data * ovict, int spellnum, int level, int casttype, char *tar_str)
{
    bool CAN_MURDER(struct char_data * ch, struct char_data * vict);
    int savetype;

    if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
        return 0;

    if (caster && DEAD(caster))
        return;
    if (cvict && DEAD(cvict))
        return;
    if (ovict && PURGED(ovict))
        return;

    if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC) && !IS_GOD(caster) && caster!=supermob) {
    	if (IS_CLERIC(caster))
    	{       
    		send_to_char("Your prayer fizzles out and dies.\r\n", caster);
        	act("$n's prayer fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
    	}
    	else
    	{    		
        	send_to_char("Your magic fizzles out and dies.\r\n", caster);
        	act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
        }
        return 0;
    }

    if (!(ROOM_FLAGGED(caster->in_room, ROOM_ARENA)))
        if (cvict != NULL && (ROOM_FLAGGED(caster->in_room, ROOM_PEACEFUL) || (caster!=cvict && !CAN_MURDER(caster, cvict)))
                &&
                (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE)) && casttype!=CAST_POTION) {
            ch_printf(caster, "A flash of white light fills the room, dispelling your "
                         "violent %s!\r\n", SORP(caster));
            act("White light from no particular source suddenly fills the room, "
                "then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
            return 0;
        }

    /*    if (mag_manacost(caster, spellnum)+CAST_LIMIT(caster)>GET_MANA(caster))
        {
        	send_to_char("You weren't able to control the magic.\r\n", caster);
        	return;
        }*/
    /* determine the type of saving throw */
    switch (casttype) {
    case CAST_STAFF:
    case CAST_SCROLL:
    case CAST_POTION:
    case CAST_WAND:
        savetype = SAVING_ROD;
        break;
    case CAST_SPELL:
    case CAST_BOUND_SPELL:
        savetype = SAVING_SPELL;
        break;
    default:
        savetype = SAVING_BREATH;
        break;
    }


    if (casttype==CAST_SPELL)
    {
        int mana=10;
        if (IS_SET(SINFO.routines, MAG_AREAS))
            mana=5;
        if (IS_SET(SINFO.routines, MAG_POINTS))
            mana=10;
        if (IS_SET(SINFO.routines, MAG_SUMMONS))
            mana=3;
        if (IS_SET(SINFO.routines, MAG_MASSES))
            mana=3;
        if (IS_SET(SINFO.routines, MAG_GROUPS))
            mana=3;
        if (IS_SET(SINFO.routines, MAG_AFFECTS))
            mana=3;
        if (IS_SET(SINFO.routines, MAG_ROOM))
            mana=2;
        improve_skill(caster, spellnum, mana);
    }

    //    CREF(cvict, CHAR_NULL);

    if (IS_SET(SINFO.routines, MAG_AREAS))
        mag_areas(level, caster, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_DAMAGE))
        mag_damage(level, caster, cvict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_AFFECTS))
        mag_affects(level, caster, cvict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
        mag_unaffects(level, caster, cvict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_POINTS))
        mag_points(level, caster, cvict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
        mag_alter_objs(level, caster, ovict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_GROUPS))
        mag_groups(level, caster, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_CASTERS))
        mag_casters(level, caster, cvict,ovict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_SUMMONS))
        mag_summons(level, caster, ovict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_CREATIONS))
        mag_creations(level, caster, spellnum);

    if (IS_SET(SINFO.routines, MAG_MASSES))
        mag_masses(level, caster, spellnum, savetype);
    
    if (IS_SET(SINFO.routines, MAG_ROOM))
        mag_room(level, caster, spellnum, savetype);
        
        

    if (DEAD(caster))
        return;

    if (IS_SET(SINFO.routines, MAG_MANUAL))
        switch (spellnum) {
        case SPELL_FEAR:
            MANUAL_SPELL(spell_fear);
            break;
        case SPELL_IMPROVE_STAT:
            MANUAL_SPELL(spell_improve_stat);
            break;
        case SPELL_DISINTIGRATE:
            MANUAL_SPELL(spell_disintigrate);
            break;
        case SPELL_TELEPORT:
            MANUAL_SPELL(spell_teleport);
            break;
        case SPELL_ENCHANT_WEAPON:
            MANUAL_SPELL(spell_enchant_weapon);
            break;
        case SPELL_CHARM:
            if (casttype!=CAST_SPELL)
                spell_charm(level, caster, cvict, ovict,"nospell");
            else
                MANUAL_SPELL(spell_charm);
            break;
        case SPELL_WORD_OF_RECALL:
        case PRAYER_RECALL:
            MANUAL_SPELL(spell_recall);
            break;
        case PRAYER_VOICE_OF_TRUTH:
        case SPELL_IDENTIFY:
            MANUAL_SPELL(spell_identify);
            break;
        case SPELL_SUMMON:
         case PRAYER_SUMMON:
            MANUAL_SPELL(spell_summon);
            break;
        case SPELL_LOCATE_OBJECT:
            MANUAL_SPELL(spell_locate_object);
            break;
        case SPELL_DETECT_POISON:
            MANUAL_SPELL(spell_detect_poison);
            break;
        case SPELL_PIDENTIFY:
            MANUAL_SPELL(spell_identify);
            break;
        case SPELL_RELOCATE:
            MANUAL_SPELL(spell_portal2);
            break;
        case SPELL_PEACE:
        case PRAYER_PEACE:
            MANUAL_SPELL(spell_peace);
            break;   
	case PRAYER_VISIONS:
            MANUAL_SPELL(spell_visions);
            break;            
        case SPELL_ASTRAL_WALK:
            MANUAL_SPELL(spell_relocate);
            break;
        case SPELL_LOCATE_CREATURE:
            MANUAL_SPELL(spell_locate_creature);
            break;
        case SPELL_ARCANE:
            MANUAL_SPELL(spell_arcane);
            break;
        case SPELL_PORTAL:
            MANUAL_SPELL(spell_portal);
            break;
        case SPELL_DISPEL_MAGIC:
            MANUAL_SPELL(spell_dispel_magic);
            break;
        case SPELL_ENCHANT_ARMOR:
            MANUAL_SPELL(spell_enchant_armor);
            break;
        case SPELL_CREATE_SPRING:
            MANUAL_SPELL(spell_create_spring);
            break;
        case SPELL_CONTROL_WEATHER:
            MANUAL_SPELL(spell_control_weather);
            break;
            /*	case SPELL_VISITATION:
            	    MANUAL_SPELL(spell_visitation);
            	    break;*/
        case SPELL_CLAIRVOYANCE:
            MANUAL_SPELL(spell_clairvoyance);
            break;
        case SPELL_TRANSPORT:
            MANUAL_SPELL(spell_transport);
            break;
        case SPELL_RETRANSPORT:
            MANUAL_SPELL(spell_retransport);
            break;
        case SPELL_GUST_OF_WIND:
            MANUAL_SPELL(spell_gust_of_wind);
            break;
        case SPELL_TORNADO:
            MANUAL_SPELL(spell_gust_of_wind);
            break;
        case SPELL_ASTRAL_PROJECTION:
            MANUAL_SPELL(spell_astral_projection);
            break;
        case SPELL_OMNI_EYE:
            MANUAL_SPELL(spell_omni_eye);
            break;
        case SPELL_LOCAL_TELEPORT:
            MANUAL_SPELL(spell_local_teleport);
            break;
        case SPELL_CHARGE:
            MANUAL_SPELL(spell_charge);
            break;
        case SPELL_EAGLE_EYE:
            MANUAL_SPELL(spell_eagle_eye);
            break;
        case SPELL_RECHARGE:
            MANUAL_SPELL(spell_recharge);
            break;
        case SPELL_FAERIE_FOG:
            MANUAL_SPELL(spell_fearie_fog);
            break;
        case PRAYER_RETRIEVE_CORPSE:
        case SPELL_RETRIEVE_CORPSE:
            MANUAL_SPELL(spell_fearie_fog);
            break;    

        }

    //  CUREF(cvict);
    return 1;
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.
 *
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified.
 */

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
                     char *argument)
{
    int i, k;
    struct char_data *tch = NULL, *next_tch;
    struct obj_data *tobj = NULL;

    one_argument(argument, arg);

    k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                     FIND_OBJ_EQUIP, ch, &tch, &tobj);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_STAFF:
        act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
        if (obj->action_description != NULL &&
                !isname("undefined", obj->action_description) && !isname("Undefined", obj->action_description))
            act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
        else
            act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

        if (GET_OBJ_VAL(obj, 2) <= 0) {
            act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
            act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
        } else {
            GET_OBJ_VAL(obj, 2)--;
            WAIT_STATE(ch, PULSE_SPELL);
            if (IS_SET(spell_info[GET_OBJ_VAL(obj,3)].routines, MAG_AREAS))
            {
                for (tch = world[ch->in_room].people; tch; tch = next_tch) {
                    next_tch = tch->next_in_room;
                    if (ch == tch)
                        continue;
                    if (GET_OBJ_VAL(obj, 0))
                        call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
                                   GET_OBJ_VAL(obj, 0), CAST_STAFF, 0);
                    else
                        call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
                                   DEFAULT_STAFF_LVL, CAST_STAFF, 0);
                    break;
                }
            }
            else
                for (tch = world[ch->in_room].people; tch; tch = next_tch) {
                    next_tch = tch->next_in_room;
                    if (ch == tch)
                        continue;
                    if (GET_OBJ_VAL(obj, 0))
                        call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
                                   GET_OBJ_VAL(obj, 0), CAST_STAFF, 0);
                    else
                        call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
                                   DEFAULT_STAFF_LVL, CAST_STAFF, 0);
                }
        }
        break;
    case ITEM_WAND:
        if (k == FIND_CHAR_ROOM) {
            if (tch == ch) {
                act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
                act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
            } else {
                act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
                if (obj->action_description != NULL &&
                        !isname("undefined", obj->action_description) && !isname("Undefined", obj->action_description))
                    act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
                else
                {
                    act("$n points $p at $N.", TRUE, ch, obj, tch, TO_NOTVICT);
                    act("$n points $p at you.", TRUE, ch, obj, tch, TO_VICT);
                }
            }

        } else if (tobj != NULL) {
            act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
            if (obj->action_description != NULL)
                act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
            else
                act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
        } else {
            act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
            return;
        }

        if (GET_OBJ_VAL(obj, 2) <= 0) {
            act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
            act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
            return;
        }
        GET_OBJ_VAL(obj, 2)--;
        WAIT_STATE(ch, PULSE_SPELL);
        if (GET_OBJ_VAL(obj, 0))
            call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
                       GET_OBJ_VAL(obj, 0), CAST_WAND, 0);
        else
            call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
                       DEFAULT_WAND_LVL, CAST_WAND, 0);
        break;
    case ITEM_SCROLL:
        if (*arg) {
            if (!k) {
                act("There is nothing here to affect with $p.", FALSE,
                    ch, obj, NULL, TO_CHAR);
                return;
            }
        } else
            tch = ch;

        act("As you finish reciting, $p dissolves.", TRUE, ch, obj, 0, TO_CHAR);
        if (obj->action_description != NULL &&
                !isname("undefined", obj->action_description) && !isname("Undefined", obj->action_description))
            act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
        else
            act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

        //WAIT_STATE(ch, PULSE_SPELL);
        //	CREF(tch, CHAR_NULL);
        for (i = 1; i < 4; i++)
            if (!(tch || tobj) || !(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
                                               GET_OBJ_VAL(obj, 0), CAST_SCROLL, 0)))
                break;
        //	CUREF(tch);
        if (obj != NULL)
            extract_obj(obj);
        break;
    case ITEM_POTION:
        tch = ch;
        act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
        if (obj->action_description != NULL &&
                !isname("undefined", obj->action_description) && !isname("Undefined", obj->action_description))
            act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
        else
            act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

        //WAIT_STATE(ch, PULSE_SPELL);
        for (i = 1; i < 4; i++)
            if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
                             GET_OBJ_VAL(obj, 0), CAST_POTION, 0)))
                break;

        if (obj != NULL)
            extract_obj(obj);
        break;
    default:
        log("SYSERR: Unknown object_type in mag_objectmagic");
        break;
    }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(struct char_data * ch, struct char_data * tch,
               struct obj_data * tobj, int spellnum, char *tar_str)
{
    if (GET_POS(ch) < SINFO.min_position && GET_POS(ch)!=POS_FIGHTING) {
        switch (GET_POS(ch)) {
        case POS_SLEEPING:
            send_to_char("You dream about great magical powers.\r\n", ch);
            break;
        case POS_RESTING:
            send_to_char("You cannot concentrate while resting.\r\n", ch);
            break;
        case POS_SITTING:
            send_to_char("You can't do this sitting!\r\n", ch);
            break;
            //case POS_FIGHTING:
            //  send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
            //break;
        default:
            send_to_char("You can't do much of anything like this!\r\n", ch);
            break;
        }
        return 0;
    }

    //    if (IS_NPC(ch) && !GET_SKILL(ch, spellnum) && ch!=supermob)
    //  	return;

    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tch) && SINFO.violent) {
        send_to_char("You are afraid you might hurt your master!\r\n", ch);
        return 0;
    }
    if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
        ch_printf(ch, "You can only cast this %s upon yourself!\r\n", SORP(ch));
        return 0;
    }
    if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {  
    	ch_printf(ch, "You cannot cast this %s upon yourself!\r\n", SORP(ch));        
        return 0;
    }
    if (IS_SET(SINFO.routines, MAG_GROUPS) && (!IS_AFFECTED(ch, AFF_GROUP) || count_group(ch) == 1) && !spellnum==SPELL_HEALING_TOUCH) {
        ch_printf(ch, "You can't cast this %s if you're not in a group!\r\n", SORP(ch));
        return 0;
    }
    if (IS_SET(SINFO.routines, MAG_CASTERS) && !IS_AFFECTED(ch, AFF_GROUP)) {
        ch_printf(ch, "You can't cast this %s if you're not in a group!\r\n",SORP(ch));
        return 0;
    }
    if (IS_SET(SINFO.routines, MAG_CASTERS) && !OUTSIDE(ch)) {
        ch_printf(ch, "You must be somewhere outside before you can cast this powerful %s!\r\n", SORP(ch));
        return 0;
    }
    if (IS_SET(SINFO.routines, MAG_CASTERS) && (!(SECT(ch->in_room) < 6) || AFF_FLAGGED(ch, AFF_FLYING) || AFF_FLAGGED(ch, AFF_WATERWALK))) {
        ch_printf(ch, "You must be standing on the firm ground before casting this powerful %s!\r\n", SORP(ch));
        return 0;
    }
    if ((spellnum == SPELL_EARTHQUAKE) && (!(SECT(ch->in_room) < 6) || AFF_FLAGGED(ch, AFF_FLYING) || AFF_FLAGGED(ch, AFF_WATERWALK))) {
        ch_printf(ch, "You must be standing on the firm ground before casting this powerful %s!\r\n", SORP(ch));
        return 0;
    }
    
    
    if ((spellnum == SPELL_QUAD_DAMAGE) && (!IS_IMPL(ch))) {
        ch_printf(ch, "This spell is too powerful for you to handle.\r\n");
        return 0;
    }
    
#ifndef EVENT_MAGIC    
    say_spell(ch, spellnum, tch, tobj);
#endif
    /*  sprintf(bufma,"You  %s.\r\n",spells[spellnum]);
      send_to_char(bufma,ch);*/
    return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL, tar_str));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */
char bufc[100];
struct spell_event_obj
{
    struct char_data *ch;
    int spell;
    char argument[500];
    int step;
    int casttime;
};
        
        int global_spell_ok=0;
ACMD(do_cast);
EVENTFUNC(cast_spell_event)
{
    struct char_data *ch, *tch = NULL;
    struct obj_data *tobj = NULL;
    char *s, *t;
    int mana, spellnum, i, min_lev, target = 0, num, mod;
    struct spell_event_obj *spellobj=(struct spell_event_obj *) event_obj;
    char argument[500];
    
    
    ch=spellobj->ch;
    if (!ch || DEAD(ch))
    	{
    		 if (ch)
    		 	GET_SPELL_EVENT(ch)=NULL;
   		 DISPOSE(spellobj);
   		 return 0;
    	}
   strcpy(argument,spellobj->argument);
   
   if (spellobj->step==1)
   {
    global_spell_ok=0;
    do_cast(ch, argument, 1, SUB_EVENT); 
    
    if (global_spell_ok)
    {    
	    spellobj->step=2; 
	    WAIT_STATE(ch, spellobj->casttime+1);
	    return (spellobj->casttime);    
    }                     
    else
    {
             if (ch)
    		GET_SPELL_EVENT(ch)=NULL;
             DISPOSE(spellobj);
   	     return 0;
    }
    }  	
    		
    
    if (IS_CLERIC(ch))
    	ch_printf(ch, "&p+++ Your call for %s. +++&0\r\n", spells[spellobj->spell]);
    else
    	ch_printf(ch, "&p+++ You cast %s. +++&0\r\n", spells[spellobj->spell]);
    
    //send_to_char("&WYou have completed your casting.&0\r\n", ch);

    GET_SPELL_EVENT(spellobj->ch)=NULL;
    DISPOSE(spellobj);

    do_cast(ch, argument, 0, SUB_EVENT);
    
    return 0;
    
}




ACMD(do_cast)
{
    struct char_data *tch = NULL;
    struct obj_data *tobj = NULL;
    char *s, *t;
    int mana, spellnum, i, min_lev, target = 0, num, mod;
    struct spell_event_obj *spellobj;
    char pom[500];
    strcpy(pom, argument);

    
    if (subcmd!=SUB_EVENT)
    {
    	if (GET_SPELL_EVENT(ch))
    	{
    		send_to_char("You are already busy.\r\n", ch);
    		return;
    	}
    if (IS_CLERIC(ch) && subcmd!=SCMD_PRAY)
    {
    	send_to_char("You should use 'prayfor' command for calling prayers.\r\n", ch);
    	return;
    }
    else                                   
    if (!IS_CLERIC(ch) && subcmd==SCMD_PRAY)
    {
    	if (!IS_CASTER(ch))
    		send_to_char("Type prac to see skills..\r\n", ch);
    	else
    		send_to_char("You should use 'cast' command to cast spells.\r\n", ch);
    	return;
    }
    }
    
    /* get: blank, spell name, target name */
    s = strtok(argument, "'");

    if (s == NULL) {
        send_to_char("What where?\r\n", ch);
        return;
    }
    s = strtok(NULL, "'");
    if (s == NULL) {
        ch_printf(ch, "Name of the %s must be enclosed in ancient symbols: '\r\n", SORP(ch));
        return;
    }
    t = strtok(NULL, "\0");

    /* spellnum = search_block(s, spells, 0); */

    spellnum = find_skill_num(s);

    if (!IS_SPELL(spellnum)) {                  
    	if (IS_CLERIC(ch))
        	send_to_char("Pray for what?!?\r\n", ch);
        else
        	send_to_char("Cast what?!?\r\n", ch);
        return;
    }
    min_lev = LVL_IMMORT;
    /*for (i = 0; i < NUM_CLASSES; i++)
        if (IS_SET(GET_CLASS(ch), 1 << i) && (SINFO.min_level[i] < min_lev))
            min_lev = SINFO.min_level[i];
    if (GET_LEVEL(ch) < min_lev && ch!=supermob) 
    if (!GET_SKILL(ch, spellnum))
    {
        ch_printf(ch, "You do not know that %s!\r\n", SORP(ch));
        return;
    }                               */
    if (GET_SKILL(ch, spellnum) == 0 && ch!=supermob) {
        ch_printf(ch, "You are unfamiliar with that %s.\r\n", SORP(ch));
        return;
    }

    /* Find the target */
    if (t != NULL) {
        one_argument(strcpy(arg, t), t);
        skip_spaces(&t);
    }
    if (IS_SET(SINFO.targets, TAR_IGNORE)) {
        target = TRUE;
    } else if (t != NULL && *t) {
        if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
            if ((tch = get_char_room_vis(ch, t)) != NULL)
                target = TRUE;
        }
        if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
            if ((tch = get_char_vis(ch, t)))
                target = TRUE;

        if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
            if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
                target = TRUE;

        if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
            for (i = 0; !target && i < NUM_WEARS; i++)
                if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) && isname(t, GET_EQ(ch, i)->name)) {
                    tobj = GET_EQ(ch, i);
                    target = TRUE;
                }
        }
        if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
            if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
                target = TRUE;

        if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
            if ((tobj = get_obj_vis(ch, t)))
                target = TRUE;

    } else {			/* if target string is empty */
        if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
            if (FIGHTING(ch) != NULL) {
                tch = ch;
                target = TRUE;
            }
        if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
            if (FIGHTING(ch) != NULL) {
                tch = FIGHTING(ch);
                target = TRUE;
            }
        /* if no target specified, and the spell isn't violent, default to
           self */
        if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
                !SINFO.violent) {
            tch = ch;
            target = TRUE;
        }
        if (!target) { 
        	if (subcmd==SUB_EVENT)
        	        sprintf(buf, "\r\n>> Your %s is disrupted. <<\r\n\r\n", SORP(ch));        		
            	else
            		sprintf(buf, "Upon %s should the %s be cast?\r\n",
                    IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
                    "what" : "who", SORP(ch));
            send_to_char(buf, ch);
            return;
        }
    }

    if (target && (tch == ch) && SINFO.violent) {
        send_to_char("Your anima speaks up nervously and argues persuasively that doing it to yourself would be fatuous.\r\n", ch);
        return;
    }
    if (!target) {
    	if (subcmd==SUB_EVENT)
             ch_printf(ch, "\r\n>> Your %s is disrupted. <<\r\n\r\n", SORP(ch));        		
        else
             ch_printf(ch, "Cannot find the target of your %s!\r\n", SORP(ch));
        return;
    }

    mana = mag_manacost(ch, spellnum);

    if (SINFO.mana_change == 99 && IS_NPC(ch))
        return;
    if (SINFO.mana_change == 111) {
        if (GET_LEVEL(ch) < SINFO.mana_max) {
            sprintf(bufc, "You need to be at least level %d to cast this powerful %s.\r\n", SINFO.mana_max, SORP(ch));
            send_to_char(bufc, ch);
            return;
        }
        if (GET_SKILL(ch, spellnum) < 33) {
            ch_printf(ch, "You should first learn better this powerful %s.\r\n", SORP(ch));
            return;
        }
        if ((mana = hasenough(ch, spellnum)) == SINFO.mana_min) {
            ch_printf(ch, "Your group does not have enough power to cast this powerful %s.\r\n", SORP(ch));
            return;
        }
        if (!AFF_FLAGGED(ch, AFF_GROUP)) {
            send_to_char("You must be grouped first!\r\n", ch);
            return;
        }
    }
    if ((mana > 0) && (GET_MANA(ch) < mana) /*&& (GET_LEVEL(ch) < LVL_IMMORT)*/ && ch!=supermob) {
        ch_printf(ch, "You haven't the energy for that %s!\r\n", SORP(ch));
        return;
    }

    if (GET_MOVE(ch)<2 && ch!=supermob) {
        ch_printf(ch, "You are too exhausted to cast that %s!\r\n", SORP(ch));
        return;
    }



    if (AFF_FLAGGED(ch, AFF_EQUINOX)) {
        GET_HIT(ch) -= number(1, GET_MAX_HIT(ch)/8);
        if (check_kill(ch, "equinox"))
            return;
    }
#ifdef EVENT_MAGIC 
    if (subcmd!=SUB_EVENT)
    {
	    CREATE(spellobj, struct spell_event_obj, 1);
	    spellobj->ch=ch;
	    spellobj->spell=spellnum;
	    spellobj->step=1;
	    spellobj->casttime=cast_time(ch, spellnum)-cast_time(ch, spellnum)/2;
	    strcpy(spellobj->argument, pom);
	    if (IS_CLERIC(ch))
	    	ch_printf(ch, "You begin praying for %s.\r\n", spells[spellnum]);
	    else
	    	ch_printf(ch, "You begin casting %s.\r\n", spells[spellnum]);    
	    GET_SPELL_EVENT(ch)=event_create(cast_spell_event, spellobj, cast_time(ch, spellnum)/2);
	    WAIT_STATE(ch, cast_time(ch, spellnum)/2+1);
	    return;
    }          
	if (!cmd && subcmd==SUB_EVENT)
		goto castit;
#endif


    /* You throws the dice and you takes your chances.. 101% is total
       failure */
    num=number(1, 126);
    mod=GET_SKILL(ch, spellnum)+GET_WIS(ch);
    if (AFF3_FLAGGED(ch, AFF3_SHAMROCK))
        mod+=10;
    if (IS_GNOME(ch))
        mod+=10;
    if (IS_DWARF(ch) && !IS_CLERIC(ch))
        mod-=10;

    if (ch!=supermob && (num > mod) && (!WIL_CHECK(ch) || !INT_CHECK(ch))) {
#ifndef EVENT_MAGIC 	
        if (number(1, 121)>GET_SKILL(ch, SKILL_SPELL_RECOVERY))
	        WAIT_STATE(ch, PULSE_SPELL)
            else
                improve_skill(ch, SKILL_SPELL_RECOVERY, 2);
#endif  
        if (!tch || !skill_message(0, ch, tch, spellnum, 0))
            send_to_char("You lost your concentration!\r\n", ch);
        if (mana > 0 && !IS_NPC(ch) && !IS_GOD(ch))
        {
            GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - MAX(1, (mana >> 1))));
        }
        if (SINFO.violent && tch && IS_NPC(tch) && GET_POS(tch)>POS_SLEEPING)
            hit(tch, ch, TYPE_UNDEFINED);
    } else {			/* cast spell returns 1 on success;
           subtract mana & set waitstate */
#ifdef EVENT_MAGIC
           if (cmd==1)
           {
           	if (IS_CLERIC(ch)) 
		    {
		    	ch_printf(ch, "You whisper the words of the prayer.\r\n");
		    	act("$n whispers a prayer.", FALSE, ch, 0, 0, TO_ROOM);
		    }
		    else
		    {                                                                      		    	
		    	ch_printf(ch, "You utter the mystical phrases.\r\n");
		    	act("$n utters the mystical words.", FALSE, ch, 0, 0, TO_ROOM);
		    }
		    global_spell_ok=1;
		    return;
            }
#endif             
castit:		    	
        if (cast_spell(ch, tch, tobj, spellnum, t)) { 
        	if (DEAD(ch))
        		return;
#ifndef EVENT_MAGIC 	        	
            if (!IS_NPC(ch) && number(1, 301)>GET_SKILL(ch, SKILL_QUICK_CHANT))
                WAIT_STATE(ch, PULSE_SPELL)
                else
                {
                    WAIT_STATE(ch, PULSE_SPELL/2);
                    improve_skill(ch, SKILL_QUICK_CHANT, 2);
                }
#endif                
            if (mana > 0 && !IS_NPC(ch) && !IS_GOD(ch))
            {
                GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
                GET_MOVE(ch) = MAX(0, MIN(GET_MAX_MOVE(ch), GET_MOVE(ch) - 2));
                //if (IS_SET(SINFO.routines, MAG_DAMAGE))
                  //  GET_HIT(ch) = MAX(0, MIN(GET_MAX_HIT(ch), GET_HIT(ch) - (MAX(1,mana/3))));
            }
        }
    }
}








#define UU (LVL_IMPL+1)
#define UNUSED UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,UU,0,0,0,0,0,0,0,0

#define X LVL_IMMORT

FILE *fajl;
void log1(char *str)
{
    if (*str == '@') {
        fajl = fopen("spells", "w");
    } else if (*str == '#')
        fclose(fajl);
    else if (*str == '^') {
        fclose(fajl);
        fajl = fopen("skills", "w");
    } else {
        fputs(str, fajl);
        fputs("\n", fajl);
    }
}

/* Assign the spells on boot up */
char buff1[300];

int classnumskills[20]={0};

void spello(int spl, int mlev, int clev, int tlev, int flev, int blev, int slev, int dlev, int wlev,
            int olev, int alev, int nlev, int ylev, int xlev,
            int spelltype,
            int max_mana, int min_mana, int mana_change, int minpos,
            int targets, int violent, int routines)
{
    int i;
    *buff1 = '\0';



    if (spelltype > 0) {
        if (spl == (MAX_SPELLS + 1)) {
            *buff1 = '^';
            log1(buff1);
            sprintf(buff1, "!%-25s          Mag     Cle     Dru     War     Thi     Mon\n", "SKILLS");
            sprintf(buff1, "%s!------------------------------------------------------------------------------", buff1);
            log1(buff1);
        }
        if (spl < 200) {
            sprintf(buff1, "%25s     ", spells[spl]);
            sprintf(buff1, "%s   %5d", buff1, (mlev != X ? mlev : 0));
            sprintf(buff1, "%s   %5d", buff1, (clev != X ? clev : 0));
            sprintf(buff1, "%s   %5d", buff1, (dlev != X ? dlev : 0));
            sprintf(buff1, "%s   %5d", buff1, (flev != X ? flev : 0));
            sprintf(buff1, "%s   %5d", buff1, (tlev != X ? tlev : 0));
            sprintf(buff1, "%s   %5d", buff1, (olev != X ? olev : 0));
            log1(buff1);
        }
    }
    spell_info[spl].min_level[0] = mlev;	/* class_magic_user */
    spell_info[spl].min_level[1] = clev;	/* class_cleric */
    spell_info[spl].min_level[2] = tlev;	/* class_thief */
    spell_info[spl].min_level[3] = flev;	/* class_fighter */
    spell_info[spl].min_level[4] = blev;	/* class_ranger */
    spell_info[spl].min_level[5] = slev;	/* class_bard */
    spell_info[spl].min_level[6] = dlev;	/* class_druid */
    spell_info[spl].min_level[7] = wlev;	/* class_necron */
    spell_info[spl].min_level[8] = olev;	/* class_monk */
    spell_info[spl].min_level[9] = alev;	/* class_avatar */
    spell_info[spl].min_level[10] = nlev;	/* class_ninja */
    spell_info[spl].min_level[11] = ylev;	/* class_dual */
    spell_info[spl].min_level[12] = xlev;	/* class_triple */

    for (i= 0;i<13;i++)
        if ((spell_info[spl].min_level[i]) <LVL_IMMORT)
        {

            classnumskills[i]++;
        }


    spell_info[spl].type = spelltype;
    spell_info[spl].mana_max = max_mana;
    spell_info[spl].mana_min = min_mana;
    spell_info[spl].mana_change = mana_change;
    spell_info[spl].min_position = minpos;
    spell_info[spl].targets = targets;
    spell_info[spl].violent = violent;
    spell_info[spl].routines = routines;
}

/*
 * Arguments for spello calls:
 *
 * spellnum, levels (MCTFBSDW23), maxmana, minmana, manachng, minpos, targets,
 * violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL). levels  :  Minimum level (mage, cleric,
 * thief, warrior) a player must be to cast this spell.  Use 'X' for immortal
 * only. maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell). minmana :  The minimum
 * mana this spell will take, no matter how high level the caster is.
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|'). violent :  TRUE or FALSE,
 * depending on if this is considered a violent
 * in PEACEFUL rooms or on yourself. routines:  A list of magic routines
 * which are associated with this spell. Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

char mojne[2];
void mag_assign_spells(void)
{
    int i;
    *mojne = '@';
    log1(mojne);
    sprintf(buff1, "!%-25s          Mag     Cle     Dru     War     Thi     Mon\n", "SPELLS");
    sprintf(buff1, "%s!------------------------------------------------------------------------------", buff1);
    log1(buff1);
    for (i = 1; i <= TOP_SPELL_DEFINE; i++)
        spello(i, UNUSED);
    /* C L A S S E S                M A N A   */
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_ARMOR, 5, 1, X, X, X, X, 3, X, X, 1, X, X, X,
           SKILL_TYPE_DRUID, 30, 5, 3,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_HOLY_ARMOR, X, 27, X, X, X, X, X, X, X, 1, X, X, X,
           SKILL_TYPE_DRUID, 30, 20, 3,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_BLESS, X, 5, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 35, 5, 3,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

    spello(SPELL_BLINDNESS, 8, 6, X, X, 24, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 25, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

    spello(SPELL_BURNING_HANDS, 5, X, X, X, X, X, 2, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 15, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);
    /* Ma  Cl  Th  Fi  Ba  Sa  Dr  Wi  Mo  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_CALL_LIGHTNING, X, 11, X, X, 33, X, 8, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 40, 15, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_CHARM, 18, X, X, X, X, X, X, X, X, 1, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 75, 25, 99,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

    spello(SPELL_MASS_CHARM, 25, X, X, X, X, X, X, X, X, 1, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 75, 50, 99,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_MASSES);

    spello(SPELL_CHILL_TOUCH, 3, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 30, 15, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

    spello(SPELL_COLOR_SPRAY, 11, X, X, X, X, X, X, 5, X, 1, X, X, X,
           SKILL_TYPE_MAGIC, 30, 15, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);
    /* Ma  Cl  Th  Fi  Ba  Sa  Dr  Wi  Mo  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_CONTROL_WEATHER, 10, 13, X, X, X, X, 7, X, X, 1, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 75, 25, 5,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_CREATE_FOOD, X, 3, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 30, 5, 4,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(SPELL_CREATE_WATER, X, 2, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 30, 5, 4,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(SPELL_CURE_BLIND, X, 4, X, X, 11, X, 3, X, X, 1, X, X, X,
           SKILL_TYPE_DRUID, 30, 5, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

    spello(SPELL_CURE_CRITIC, X, 8, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 20, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(SPELL_HEALING_TOUCH, X, X, X, X, X, X, 13, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 30, 2,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_CURE_LIGHT, X, 1, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_CURSE, 12, X, X, X, X, X, X, 13, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 80, 20, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

    spello(SPELL_DETECT_ALIGN, X, 15, X, X, X, X, 15, X, X, 1, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 20, 10, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_DETECT_INVIS, 6, 7, X, X, X, X, 5, X, X, 1, X, X, X,
           SKILL_TYPE_DRUID, 20, 5, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_DETECT_MAGIC, 7, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 20, 5, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_DETECT_POISON, X, 7, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 15, 5, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

    spello(SPELL_DISPEL_EVIL, X, 10, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 50, 25, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_DISPEL_GOOD, X, 10, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 50, 25, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_EARTHQUAKE, X, 12, X, X, X, X, 9, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 50, 45, 3,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_ENCHANT_WEAPON, 26, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 150, 100, 5,
           POS_STANDING, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

    spello(SPELL_ENCHANT_ARMOR, 26, X, X, X, X, X, X, X, X, 20, X, X, X,
           SKILL_TYPE_MAGIC, 150, 100, 5,
           POS_STANDING, TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

    spello(SPELL_ENERGY_DRAIN, X, X, X, X, X, X, 29, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 40, 80, 1,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_SURCEASE, X, X, X, X, 22, X, X, X, X, 5, X, X, X,
           SKILL_TYPE_DRUID, 50, 40, 2,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_ARROW_RAIN, X, X, X, X, 20, X, X, X, X, 5, X, X, X,
           SKILL_TYPE_DRUID | SKILL_TYPE_MUNDANE, 50,50, 2,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_AREAS);

    spello(SPELL_ENDURANCE, X, X, X, X, 15, X, X, X, X, 5, X, X, X,
           SKILL_TYPE_DRUID | SKILL_TYPE_MUNDANE, 50, 50, 2,
           POS_FIGHTING, TAR_CHAR_ROOM  | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);


    spello(SPELL_FIREBALL, 15, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 40, 20, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

    spello(SPELL_ACID_BLAST, 25, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 50, 50, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

    spello(SPELL_GROUP_HEAL, X, 26, X, X, X, X, X, X, X, 5, X, X, X,
           SKILL_TYPE_DRUID, 80, 110, 5,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_GROUP_RECALL, X, 26, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 55, 55, 5,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_HARM, X, 15, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 80, 45, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_SPIRITUAL_HAMMER, X, 21, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 80, 50, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_HEAL, X, 14, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 75, 50, 3,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(SPELL_BENEDICTION, X, 33, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 75, 40, 3,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_DIVINITY, X, 39, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 75, 100, 3,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_HOG, X, 37, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 75, 100, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_INFRAVISION, 2, X, X, X, 6, X, 6, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 25, 5, 1,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_WRATH_OF_EARTH, X, X, X, X, X, X, 21, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 70, 1,
           POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

    spello(SPELL_INVISIBLE, 4, X, X, X, X, X, X, 3, X, 1, X, X, X,
           SKILL_TYPE_MAGIC, 35, 5, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

    spello(SPELL_LIGHTNING_BOLT, 9, X, X, X, X, X, X, 9, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 30, 15, 1,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */

    spello(SPELL_LOCATE_OBJECT, 10, 17, X, X, X, X, 17, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 85, 20, 1,
           POS_STANDING, TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

    spello(SPELL_MAGIC_MISSILE, 1, X, X, X, X, X, X, 1, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 25, 15, 3,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_POISON, X, 8, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 50, 10, 3,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

    spello(SPELL_REMOVE_CURSE, X, 20, X, X, X, X, X, X, X, 1, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 45, 35, 5,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

    spello(SPELL_SANCTUARY, X, 13, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 100, 75, 5,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_SHOCKING_GRASP, 7, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 30, 15, 1,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(SPELL_SLEEP, 14, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 60, 15, 2,
           POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_STRENGTH, 6, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 35, 20, 1,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_GIANT_STRENGTH, 19, X, X, X, X, X, 27, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 35, 40, 1,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_SUMMON, X, 17, X, X, X, X, 25, X, X, 8, X, X, X,
           SKILL_TYPE_CLERIC, 75, 50, 3,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

    spello(SPELL_TRANSPORT, X, X, X, X, X, X, 18, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 75, 50, 3,
           POS_STANDING, TAR_OBJ_INV, FALSE, MAG_MANUAL);

    spello(SPELL_RETRANSPORT, X, X, X, X, X, X, 18, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 75, 50, 3,
           POS_STANDING, TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

    spello(SPELL_CLAIRVOYANCE, 34, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 75, 30, 3,
           POS_STANDING, TAR_CHAR_WORLD, FALSE, MAG_MANUAL);

    spello(SPELL_WORD_OF_RECALL, X, 16, X, X, X, X, X, X, X, 7, X, X, X,
           SKILL_TYPE_CLERIC, 20, 25, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

    spello(SPELL_REMOVE_POISON, X, 9, X, X, 4, X, 12, X, X, 3, X, X, X,
           SKILL_TYPE_CLERIC, 40, 5, 4,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

    spello(SPELL_SENSE_LIFE, 11, X, X, X, 19, X, 9, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 20, 10, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_PIDENTIFY, 15, 20, X, X, X, X, 16, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 50, 40, 5,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

    spello(SPELL_FIRE_BREATH, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 100, 50, 4,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_AFFECTS);

    spello(SPELL_GAS_BREATH, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 100, 50, 3,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

    spello(SPELL_FROST_BREATH, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 100, 50, 4,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_AFFECTS);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_ACID_BREATH, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 100, 100, 0,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_AFFECTS);

    spello(SPELL_LIGHTNING_BREATH, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 100, 50, 5,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

    spello(SPELL_QUAD_DAMAGE, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 0, 0, 0,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_RESTORE_MANA, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 0, 0, 0,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_POINTS);

    spello(SPELL_RELOCATE, 35, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 100, 70, 3,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

    spello(SPELL_PEACE, X, 29, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 50, 90, 4,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_MANUAL);

    spello(SPELL_FLY, 9, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 50, 25, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    /*   spello(SPELL_LEVITATE, X, 9, X, X, X, X, X, X, X, X, X, X, X,
       SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 30, 15, 3,
       POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);*/

    spello(SPELL_WATERWALK, X, 11, X, X, X, X, 11, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 30, 15, 3,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_PROT_FIRE, X, 14, X, X, 21, X, 9, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 10, 2,
           POS_STANDING, TAR_CHAR_ROOM , FALSE, MAG_AFFECTS);

    spello(SPELL_WATERBREATH, 18, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 20, 15, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_EAGLE_EYE, X, X, X, X, 26, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 50, 2,
           POS_STANDING, TAR_CHAR_WORLD, FALSE, MAG_MANUAL);

    spello(SPELL_GROUP_INVIS, 16, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 80, 50, 2,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_POWER_HEAL, X, 38, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 80, 140, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(SPELL_PLAGUE, X, 30, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 80, 70, 2,
           POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

    spello(SPELL_DARKENING, X, 33, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 80, 130, 2,
           POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

    spello(SPELL_ENLIGHTMENT, X, 36, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 80, 75, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_CURE_PLAGUE, X, 25, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 80, 70, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

    spello(SPELL_GROUP_POWER_HEAL, X, 40, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 80, 180, 2,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_COLD_ARROW, X, X, X, X, X, X, 11, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 15, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

    spello(SPELL_CHARGE, X, X, X, X, X, X, 25, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 15, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL);

    spello(SPELL_RUNIC_SHELTER, X, X, X, X, X, X, 32, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 60, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_ENERGY_CONTAINMENT, X, X, X, X, X, X, 39, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 100, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_SUNRAY, X, X, X, X, X, X, 26, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 70, 2,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_AFFECTS);

    spello(SPELL_MELLON_COLLIE, X, X, X, X, X, X, 24, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 40, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

    spello(SPELL_SHILLELAGH, X, X, X, X, X, X, 1, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 20, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_FEEBLEMIND, X, X, X, X, X, X, 30, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 75, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

    spello(SPELL_ATONEMENT, X, X, X, X, X, X, 28, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 150, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_POINTS);

    spello(SPELL_SHAMROCK, X, X, X, X, X, X, 13, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 30, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_INTELLIGIZE, X, X, X, X, X, X, 38, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 50, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_AGILITY,  X, X, X, X, X, X, 22, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 40, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);


    spello(SPELL_FLAME_ARROW, X, X, X, X, X, X, 15, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 30, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_CURE_SERIOUS, X, 5, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 40, 15, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);
    spello(SPELL_MINUTE_METEOR, 31, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 45, 60, 2,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

    spello(SPELL_AREA_LIGHTNING, X, X, X, X, X, X, 20, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 60, 40, 3,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

    spello(SPELL_STRONG_MIND, X, X, X, X, X, X, 19, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 20, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_SHIELD, 13, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 35, 12, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_NAP, X, X, X, X, X, X, 16, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 80, 2,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_REGENERATE, X, 31, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 60, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_FORCE_FIELD, X, 23, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 30, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_FIRE_SHIELD, 24, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 35, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_DEFLECTION, X, X, X, X, X, X, 22, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 40, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_GROUP_DEFLECTION, X, X, X, X, X, X, 36, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 280, 2,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_GROUP_FIRE_SHIELD, 37, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 280, 2,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_GROUP_FORCE_FIELD, X, 36, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 280, 2,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(SPELL_WRAITHFORM, 20, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 35, 15, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_WEAKEN, 16, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 35, 20, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

    spello(SPELL_STONESKIN, 17, X, X, X, 31, X, 21, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 80, 12, 3,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_CONE_OF_COLD, X, X, X, X, X, X, 27, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 100, 55, 4,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_AFFECTS);

    spello(SPELL_EQUINOX, 47, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 350, 3,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_BARKSKIN, X, 9, X, X, 10, X, 8, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 45, 10, 3,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    /*spello(SPELL_FORTRESS, 12, 12, X, X, X, X, 12, X, X, X, X, X, X,
    SKILL_TYPE_DRUID, 60, 100, 3,
    POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);
      */
    spello(SPELL_FAERIE_FOG, 13, X, X, X, X, X, 14, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 35, 35, 4,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_BLADEBARRIER, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 50, 35, 4,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

    spello(SPELL_CONJ_ELEMENTAL, X, X, X, X, X, X, 19, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 120, 110, 3,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

    spello(SPELL_MONSUM_I, 22, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 100, 120, 99,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);
    /*
      spello(SPELL_MONSUM_II, 15, 18, X, X, X, X, 18, 14, X, X, X, X, X,
    	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
             POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

      spello(SPELL_MONSUM_III, 20, 22, X, X, X, X, 22, 18, X, X, X, X, X,
    	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
             POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

      spello(SPELL_MONSUM_IV, 24, 26, X, X, X, X, 26, 22, X, X,  X, X, X,
    	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 60, 2,
             POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);
    */
    spello(SPELL_MONSUM_V, 36, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 120, 160, 99,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_SUMMONS);

    spello(SPELL_FEAR, X, X, X, X, X, X, 29, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 30, 60, 2,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_MANUAL);


    spello(SPELL_FAERIE_FIRE, 4, X, X, X, 17, X, 4, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 30, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);
    /*
      spello(SPELL_KNOCK, 7, X, X, X, X, X, X, X, X, X, X, X, X,
    	 SKILL_TYPE_MAGIC, 40, 20, 2,
    	 POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

      spello(SPELL_HOLD_PERSON, 14, 10, X, X, X, X, 10, X, X, X, X, X, X,
    	 SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 70, 40, 2,
    	 POS_STANDING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL);

      spello(SPELL_PLANESHIFT, X, 18, X, X, X, X, 18, X, X, 18, X, X, X,
    	 SKILL_TYPE_CLERIC, 80, 40, 2,
             POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL);*/

    spello(SPELL_MIRROR_IMAGE, 27, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 60, 30, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_BLINK, 21, X, X, X, X, X, X, 16, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 50, 25, 1,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_HASTE, 23, X, X, X, X, X, X, X, X, 1, X, X, X,
           SKILL_TYPE_MAGIC, 50, 30, 1,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_DISPEL_MAGIC, 30, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 80, 85, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV, FALSE, MAG_MANUAL | MAG_ALTER_OBJS);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */

    spello(SPELL_ANIMATE_DEAD, X, 22, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 100, 120, 1,
           POS_STANDING, TAR_OBJ_ROOM, FALSE, MAG_SUMMONS);

    spello(SPELL_CLONE, 39, X, X, X, X, X, X, X, X, 27, X, X, X,
           SKILL_TYPE_MAGIC, 100, 400, 99,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_SUMMONS);

    spello(SPELL_DEATHDANCE, 21, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 100, 70, 0,
           POS_STANDING, TAR_CHAR_ROOM , FALSE, MAG_AFFECTS);

    spello(SPELL_PROT_COLD, X, 11, X, X, 13, X, 9, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 10, 2,
           POS_STANDING, TAR_CHAR_ROOM , FALSE, MAG_AFFECTS);

    spello(SPELL_PROT_LIGHTNING, X, 7, X, X, 9, X, 7, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 10, 2,
           POS_STANDING, TAR_CHAR_ROOM , FALSE, MAG_AFFECTS);

    spello(SPELL_ENH_HEAL, X, 27, X, X, X, X, 31, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 60, 30, 3,
           0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    /*spello(SPELL_ENH_MANA, X, 24, X, X, X, X, 26, X, X, 15, X, X, X,
    SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 60, 30, 3,
    0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_ENH_MOVE, X, 22, X, X, X, X, 23, X, X, X, X, X, X,
    SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 60, 30, 3,
    0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);*/
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SPELL_TELEPORT, 17, X, X, X, X, X, X, X, X, 3, X, X, X,
           SKILL_TYPE_MAGIC, 60, 50, 3,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

    spello(SPELL_LOCAL_TELEPORT, 14, X, X, X, X, X, X, X, X, 3, X, X, X,
           SKILL_TYPE_MAGIC, 50, 40, 3,
           POS_FIGHTING, TAR_CHAR_ROOM , FALSE, MAG_MANUAL);


    spello(SPELL_DISINTIGRATE, 41, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC, 100, 120, 0,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT| TAR_OBJ_ROOM, TRUE, MAG_MANUAL);

    spello(SPELL_FIRESTORM, X, X, X, X, X, X, 34, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 100, 60, 10,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_AFFECTS);

    spello(SPELL_GUINESS, X, X, X, X, X, X, 14, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 80, 50, 5,
           POS_STANDING, TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

    spello(SPELL_GUST_OF_WIND, 28, X, X, X, X, X, X, X, X,X, X, X, X,
           SKILL_TYPE_DRUID, 70, 70, 5,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

    spello(SPELL_TORNADO, X, X, X, X, X, X, 24, X, X,X, X, X, X,
           SKILL_TYPE_DRUID, 90, 90, 5,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_MANUAL);

    spello(SPELL_HOLD_MONSTER, X, 37, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 70, 80, 5,
           POS_STANDING, TAR_CHAR_WORLD, TRUE, MAG_AFFECTS);

    spello(SPELL_ASTRAL_WALK, X, 32, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 100, 80, 5,
           POS_STANDING, TAR_CHAR_WORLD, FALSE, MAG_MANUAL);

    spello(SPELL_HOLY_WORD, X, 35, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 100, 120, 0,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_AREAS);

    spello(SPELL_LOCATE_CREATURE, 19, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 80, 50, 5,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

    spello(SPELL_ADRENALIN, X, 24, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 60, 35, 0,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_POWER_OF_NATURE, X, X, X, X, X, X, 41, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 100, 200, 0,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);

    spello(SPELL_FLAMESTRIKE, X, 19, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC, 60, 55, 0,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_AFFECTS | MAG_AREAS);

    spello(SPELL_RESTORE, X, 23, X, X, 28, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 60, 60, 0,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(SPELL_SYNOST, X, X, X, X, X, X, 28, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 60, 25, 0,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_POINTS);

    spello(SPELL_IMPROVE_STAT, X, X, X, X, X, X, 45, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 200, 500, 0,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_AURA, X, 34, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 60, 0,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_RESTORATION, X, 45, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 200, 0,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_POINTS);

    spello(SPELL_VITALITY, X, 18, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 50, 0,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_BAPTIZE, X, 25, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 90, 0,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF , FALSE, MAG_POINTS);

    spello(SPELL_RETRIEVE_CORPSE, X, 41, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 80, 0,
           POS_STANDING, TAR_IGNORE , FALSE, MAG_MANUAL);


    spello(SPELL_SHROUD_OF_DARKNESS, X, 28, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 40, 0,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);


    spello(SPELL_INTESIFY, 33, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 50, 0,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);


    spello(SPELL_CURE_DRUNK, X, 8, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC | SKILL_TYPE_MAGIC, 30, 10, 2,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    /*    spello(SPELL_AIR_SERVANT, X, 8, X, X, X, X, 3, 8, X,1, X, X, X,
             SKILL_TYPE_CLERIC|SKILL_TYPE_MAGE, 30 , 10, 2,
              POS_STANDING, TAR_CHAR_ROOM , FALSE, MAG_POINTS);

        spello(SPELL_EARTH_SERVANT, X, 8, X, X, X, X, 3, 8, X,1, X, X, X,
             SKILL_TYPE_CLERIC|SKILL_TYPE_MAGE , 30 , 10, 2,
              POS_STANDING, TAR_CHAR_ROOM , FALSE,  MAG_POINTS);*/

    spello(SPELL_ARCANE, X, X, X, X, X, X, 33, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 100, 50, 3,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_PORTAL, X, X, X, X, X, X, 35, X, X, 12, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 120, 80, 3,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_CREATE_SPRING, X, X, X, X, 8, X, 6, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC, 120, 10, 3,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_ASTRAL_PROJECTION, X, X, X, X, X, X, 23, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 30, 3,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_OMNI_EYE, X, X, X, X, X, X, 37, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 120, 10, 3,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(SPELL_BEAUTIFY, X, X, X, X, X, X, 15, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 60, 30, 3,
           POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);


    spello(SPELL_SLOW, 32, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 50, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

    spello(SPELL_SHROUD_OF_OBSCUREMENT, 38, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 70, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

    spello(SPELL_MASS_SLEEP, 23, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 60, 1,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_MASSES);

    spello(SPELL_WEB, 20, X, X, X, 29, 17, 17, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 40, 1,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

    spello(SPELL_RECHARGE, 22, X, X, X, X, X, 20, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 40, 1,
           POS_STANDING, TAR_OBJ_INV, FALSE, MAG_MANUAL);

    spello(SPELL_MAGICAL_PROTECTION, 29, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 50, 40, 1,
           POS_STANDING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);


    /* Ma  Cl  Th  Fi  Ba  Sa  Dr  Wi  Mo  Av  Ni  2  3  Type Max Min Chn */
    /* SKILLS

    The only parameters needed for skills are only the minimum levels for
       each class.  The remaining 8 fields of the structure should be
       filled with 0's. */

    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    /*spello(SKILL_ENH_SIGHT, X, X, 1, X, 1, X, X, X, X, X, X, X, X,
    SKILL_TYPE_MUNDANE,
    0, 0, 0, 0, 0, 0, 0);
      */
    spello(SKILL_PARRY, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_DODGE, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SHIELD, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_HIT,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_SLASH,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_POUND,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_PIERCE,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);

    /*spello(SKILL_RIDING, X, X, 3, X, X, X, X, X, 3, 2, 10, X, X,
    0,
    0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SWIMMING, X, X, 3, X, X, X, X, X, 3, 2, 10, X, X,
    0,
    0, 0, 0, 0, 0, 0, 0);
      */  
    spello(SKILL_EVASIVE, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, X, X,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_AGGRESIVE, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, X, X,
           SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
           0, 0, 0, 0, 0, 0, 0);


    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */

    /*spello(SKILL_FILLET, 1, 1, 1, 1, 1, X, 1, X, 1, X, X, X, X,
    SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
    0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_COOK, 1, 1, 1, 1, 1, X, 1, X, 1, X, X, X, X,
    SKILL_TYPE_MUNDANE | SKILL_TYPE_DRUID,
    0, 0, 0, 0, 0, 0, 0);
      */




    spello(SKILL_BACKSTAB, X, X, 1, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_PEEK, X, X, 11, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SPY, X, X, 9, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_DIRTKICK, X, X, 22, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_HAGGLE, X, X, 18, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_CIRCLE, X, X, 16, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_BASH, X, X, X, 5, 18, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_HIDE, X, X, 3, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_DODGEMASTERY, X, X, 21, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_DISTRACT, X, X, 28, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_THROW_DAGGER , X, X, 14, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_LISTEN, X, X, 24, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_KICK, X, X, X, 2, 5, X, X, X, X, 1, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_PICK_LOCK, X, X, 10, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_RESCUE, X, X, X, 8, 11, 6, X, X, X, 1, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SKILL_SNEAK, X, X, 12, X, 10, X, 16, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_STEAL, X, X, 4, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_IMPROVED_STEAL, X, X, 25, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_CRITICAL_HIT, X, X, X, X, X, X, X, X, 20, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_MOVE_HIDDEN, X, X, 19, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_TRIP, X, X, 10, X, 9, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_STALK, X, X, 29, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_PLANT, X, X, 17, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_EAVESDROP, X, X, 23, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);



    spello(SKILL_DUCK, X, X, 11, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_STUN, X, X, 40, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_DUAL_CIRCLE, X, X, 37, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_ESCAPE, X, X, 26, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_AMBUSH, X, X, 25, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_BLINDFIGHTING, X, X, 22, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_GOUGE, X, X, 34, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SCARE, X, X, 20, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);



    spello(SKILL_DUAL_BACKSTAB, X, X, 32, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_DANCEOFDAGGERS, X, X, 29, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_ENVENOM, X, X, 15, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_TAUNT, X, X, 33, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_DISGUISE, X, X, 28, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_CUTTHROAT, X, X, 42, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_TUMBLE, X, X, 38, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_ASSASSINATE, X, X, 46, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_TACTICS, X, X, 31, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_ALERTNESS, X, X, 19, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_FIRSTAID, X, X, 5, X, X, X, X, X, X, X, 3, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_NERVE, X, X, 35, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_RESISTANCE, X, X, 34, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    /*spello(SKILL_DOWNSTRIKE, X, X, 13, X, X, X, X, X, X, X, X, X, X,
    	   SKILL_TYPE_THIEF,
    	   0, 0, 0, 0, 0, 0, 0);*/
    spello(SKILL_MONSTERLORE, X, X, 23, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_LODGE, X, X, 13, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);



    spello(SKILL_TRAP_ROPE  , X, X, 6, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_CALTROPS  , X, X, 12, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_GAS  , X, X, 41, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_DOOM  , X, X, 45, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_DART  , X, X, 9, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_POISON, X, X, 15, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_HORROR     , X, X, 18, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_FIRE       , X, X, 21, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_LOCAL_TELEPORT, X, X, 24, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_BLIND      , X, X, 27, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_STUN       , X, X, 30, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_ACID       , X, X, 33, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_WEB, X, X, 36, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_SLEEP,  X, X, 39, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_TRAP_TELEPORT, X, X, 43, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);






    spello(SKILL_SPOT_TRAPS, X, X, 14, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_THIEF,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_TRACK, X, X, 13, X, 4, X, 14, X, 10, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_DISARM, X, X, X, 27, 23, X, X, X, 18, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);


    /*  spello(SKILL_WEAPON_BREAK, X, X, X, 16, X, 14, X, X, X, X, X, X, X,
    	 SKILL_TYPE_FIGHTER,
    	 0, 0, 0, 0, 0, 0, 0);*/
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SKILL_SECOND_ATTACK, X, X, 7, 10, 12, X, 19, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_THIRD_ATTACK, X, X, 27, 20, 25, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_ENH_DAMAGE, X, X, X, 12, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    /*ello(SKILL_SWITCH, X, X, X, 17, X, X, X, X, X, X, X, X, X,
    SKILL_TYPE_FIGHTER,
    0, 0, 0, 0, 0, 0, 0);
      */
    spello(SKILL_DUAL_WIELD, X, X, 20, 15, 19, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);



    /*  spello(SKILL_CAMP, X, X, X, X, X, X, X, X, X, X, X, X, X,
    	 SKILL_TYPE_MUNDANE | SKILL_TYPE_MAGIC | SKILL_TYPE_CLERIC,
    	 0, 0, 0, 0, 0, 0, 0);*/
    spello(SKILL_BERSERK, X, X, X, 32, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    /*spello(SKILL_FIRST_AID, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1,
    	 SKILL_TYPE_MUNDANE,
    	 0, 0, 0, 0, 0, 0, 0);*/
    spello(SKILL_FIST_OF_DOOM, X, X, X, X, X, X, X, X, 29, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_BANDAGE, X, X, X, X, 2, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_ARCHERY, X, X, X, X, 1, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_FORAGE, X, X, X, X, 8, X, 9, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_COVER_TRACKS, X, X, X, X, 13, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_SHARPEN, X, X, X, X, 13, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_ELBOW2, X, X, X, X, 16, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_HUNT, X, X, 26, X, 17, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_KNEE2, X, X, X, X, 22, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_MEDIC, X, X, X, X, 27, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_SWEEPKICK, X, X, X, X, 30, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_GUARD, X, X, X, 38, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_WHIRLWIND, X, X, X, X, 35, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SKILL_COMBO, X, X, X, X, 39, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_RETREAT, X, X, X, X, 21, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_SPRING_LEAP, X, X, X, X, 24, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_AUTOPSY, X, X, X, X, 15, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SPELL_RECOVERY, 22, 23, X, X, X, X, 23, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_QUICK_CHANT, 32, 31, X, X, X, X, 33, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_GREATER_ENCH, 45, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_MELEE, X, X, X, 36, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_FOURTH_ATTACK, X, X, X, 39, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_POWER_BLOW, X, X, X, X, X, X, X, X, 39, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SECOND_DUAL, X, X, X, 30, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_ELBOW_SWING, X, X, X, X, X, X, X, X, 12, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_KNEE_THRUST, X, X, X, X, X, X, X, X, 27, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_BUDDHA_FINGER, X, X, X, X, X, X, X, X, 31, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_QUIVER_PALM, X, X, X, X, X, X, X, X, 1, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);
    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    spello(SKILL_BREW, X, 1, X, X, X, X, 1, X, X, X, X, X, X,
           SKILL_TYPE_CLERIC,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SCRIBE, 1, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MAGIC,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_FORGE, X, X, 30, 34, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_HAMMER, X, X, X, X, X, X, X, X, 33, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_SPIN_KICK, X, X, X, X, X, X, X, X, 36, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_ALFA_NERVE, X, X, X, X, X, X, X, X, 24, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    /*spello(SKILL_BETA_NERVE, X, X, X, X, X, X, X, X, 24, X, X, X, X,
    SKILL_TYPE_FIGHTER,
    0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_GAMMA_NERVE, X, X, X, X, X, X, X, X, 23, X, X, X, X,
    SKILL_TYPE_FIGHTER,
    0, 0, 0, 0, 0, 0, 0);
    */
    /*spello(SKILL_CONSIDER, X, X, 7, 6, 6, X, X, X, 6, X, X, X, X,
    SKILL_TYPE_MUNDANE,
    0, 0, 0, 0, 0, 0, 0);
      */
    /*  spello(SKILL_TRANSFER, X, X, X, X, X, X, X, X, 22, X, X, X, X,
    	   SKILL_TYPE_MUNDANE,
    	   0, 0, 0, 0, 0, 0, 0);
    */
    spello(SKILL_FAME, X, X, X, 45, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_FEIGN_DEATH, X, X, 17, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_KATA, X, X, X, X, X, X, X, X, 8, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_MEDITATE, X, X, X, X, X, X, X, X, 17, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_BEHEAD, X, X, X, 42, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_ARCHIRAGE, X, X, X, 28, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_BATTLECRY, X, X, X, 24, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_FIGHTER,
           0, 0, 0, 0, 0, 0, 0);


    spello(SKILL_MAP, X, X, X, X, 14, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */

    spello(SKILL_FEINT, X, X, X, 25, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_JUMP, X, X, X, 23, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_FASTHEALING, X, X, X, 18, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_PUSH, X, X, X, 26, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_DRAG, X, X, X, 26, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_GROUNDCONTROL, X, X, X, 29, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_ARTOFWAR, X, X, X, 31, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_LUNGE, X, X, X, 16, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_COUNTERSTRIKE, X, X, X, 17, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_HEADBUTT, X, X, X, 33, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_AMBIDEX, X, X, 36, 41, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);

    spello(SKILL_KICKFLIP, X, X, X, 19, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_GUT, X, X, X, 18, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_LEADERSHIP, X, X, X, 37, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_CHARGE, X, X, X, 22, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_DOORBASH, X, X, X, 23, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_KNOCKOUT, X, X, X, 35, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_SIXTHSENSE, X, X, X, 43, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_WARRIORCODE, X, X, X, 25, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_GRIP, X, X, X, 13, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);
    spello(SKILL_SHIELDMASTERY, X, X, X, 21, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_MUNDANE,
           0, 0, 0, 0, 0, 0, 0);









    spello(SPELL_IDENTIFY, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_E_MAGIC, 0, 0, 0,
           0, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

    spello(SPELL_BURN, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_E_MAGIC, 0, 0, 0,
           0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_FREEZE, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_E_MAGIC, 0, 0, 0,
           0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_ACID, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_E_MAGIC, 0, 0, 0,
           0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(SPELL_CRIT_HIT, X, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_E_MAGIC, 0, 0, 0,
           0, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);


    spello(SPELL_CONJURING_BEAM, 1, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 15, 800, 111,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, TRUE, MAG_CASTERS);

    spello(SPELL_RESSURECTION, X, 1, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 25, 800, 111,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_CASTERS);

    spello(SPELL_GIVE_LIFE, X, X, X, X, X, X, 1, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 25, 1100, 111,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_CASTERS);

    spello(SPELL_PRISMATIC_SPHERE, 1, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 20, 900, 111,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_CASTERS);

    spello(SPELL_PETRIFY, X, 1, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 20, 900, 111,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_CASTERS);

    spello(SPELL_ULTRA_DAMAGE, 1, X, X, X, X, X, X, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 25, 1000, 111,
           POS_STANDING, TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_CASTERS);

    spello(SPELL_CREATE_RAIN, X, 1, X, X, X, X, 1, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 5, 500, 111,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_CASTERS);

    spello(SPELL_AREA_EARTHQUAKE, X, 1, X, X, X, X, 1, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 10, 500, 111,
           POS_STANDING, TAR_IGNORE, TRUE, MAG_CASTERS);

    spello(SPELL_RAGNAROK, X, X, X, X, X, X, 1, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 30, 1200, 111,
           POS_STANDING, TAR_IGNORE, TRUE, MAG_CASTERS);
    /*
        spello(SPELL_HOLY_TOUCH, X, 1, X, X, X, X, X, X, X, X, X, X, X,
    	   SKILL_TYPE_DRUID, 10, 1000, 111,
    	POS_STANDING, TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_CASTERS);
    */	
    spello(SPELL_MATERIALIZE, 1, X, X, X, X, X, 1, X, X, X, X, X, X,
           SKILL_TYPE_DRUID, 20, 700, 111,
           POS_STANDING, TAR_OBJ_WORLD, FALSE, MAG_CASTERS);



    spello(PRAYER_CURE, X, X, X, X, X, X, X, X, X, X, 1, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS|MAG_UNAFFECTS);
    
    spello(PRAYER_MINOR_P, X, X, X, X, X, X, X, X, X, X, 2, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(PRAYER_PROTECTION, X, X, X, X, X, X, X, X, X, X, 10, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(PRAYER_HEAL, X, X, X, X, X, X, X, X, X, X, 15, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(PRAYER_MAJOR_P, X, X, X, X, X, X, X, X, X, X, 20, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(PRAYER_GREATER_P, X, X, X, X, X, X, X, X, X, X, 30, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE,MAG_AFFECTS);

    spello(PRAYER_INVIGORATE, X, X, X, X, X, X, X, X, X, X, 33, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(PRAYER_DIVINE_P, X, X, X, X, X, X, X, X, X, X, 40, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

    spello(PRAYER_INVIGORATE_ALL, X, X, X, X, X, X, X, X, X, X, 41, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);
    
    spello(PRAYER_SHIELD_OF_FAITH, X, X, X, X, X, X, X, X, X, X, 25, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM  | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);
           

    spello(PRAYER_MIRACLE, X, X, X, X, X, X, X, X, X, X, 45, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS|MAG_UNAFFECTS);

    spello(PRAYER_SANCTUARY, X, X, X, X, X, X, X, X, X, X, 50, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       


    spello(PRAYER_PUNISHMENT, X, X, X, X, X, X, X, X, X, X, 9, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(PRAYER_DISPEL_EVIL, X, X, X, X, X, X, X, X, X, X, 13, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);
    spello(PRAYER_DISPEL_GOOD, X, X, X, X, X, X, X, X, X, X, 13, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);           

    spello(PRAYER_SPIRITUAL_HAMMER, X, X, X, X, X, X, X, X, X, X, 17, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(PRAYER_JUDGMENT, X, X, X, X, X, X, X, X, X, X, 24, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);
           
    spello(PRAYER_SMITE_EVIL, X, X, X, X, X, X, X, X, X, X, 28, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);
    spello(PRAYER_SMITE_GOOD, X, X, X, X, X, X, X, X, X, X, 28, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(PRAYER_HOJ, X, X, X, X, X, X, X, X, X, X, 31, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(PRAYER_RAPTURE, X, X, X, X, X, X, X, X, X, X, 38, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS);       

    spello(PRAYER_WOG, X, X, X, X, X, X, X, X, X, X, 42, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

    spello(PRAYER_DIVINE_I, X, X, X, X, X, X, X, X, X, X, 46, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL);       


//Utility

    spello(PRAYER_BLESSING, X, X, X, X, X, X, X, X, X, X, 5, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING,TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

    spello(PRAYER_SENSE_ALIGN, X, X, X, X, X, X, X, X, X, X, 6, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);       

    spello(PRAYER_QUENCH, X, X, X, X, X, X, X, X, X, X, 8, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);       

    spello(PRAYER_SATE, X, X, X, X, X, X, X, X, X, X, 7, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);       

    spello(PRAYER_ILLUMINATION, X, X, X, X, X, X, X, X, X, X, 11, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_ROOM);       

    spello(PRAYER_SENSE_LIFE, X, X, X, X, X, X, X, X, X, X, 14, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);       

    spello(PRAYER_RECALL, X, X, X, X, X, X, X, X, X, X, 16, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL);       

    spello(PRAYER_PEACE, X, X, X, X, X, X, X, X, X, X, 18, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_IGNORE, FALSE, MAG_MANUAL);

    spello(PRAYER_SUMMON, X, X, X, X, X, X, X, X, X, X, 19, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_WORLD, FALSE, MAG_MANUAL);       
    
    spello(PRAYER_VOICE_OF_TRUTH, X, X, X, X, X, X, X, X, X, X, 21, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_STANDING, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

    spello(PRAYER_VISIONS, X, X, X, X, X, X, X, X, X, X, 22, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_STANDING, TAR_CHAR_WORLD, FALSE, MAG_MANUAL);       

    spello(PRAYER_REJUVENATE, X, X, X, X, X, X, X, X, X, X, 23, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_POINTS);

    spello(PRAYER_BLINDING_LIGHT, X, X, X, X, X, X, X, X, X, X, 26, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_IGNORE, TRUE, MAG_AREAS | MAG_AFFECTS);

    spello(PRAYER_INSPIRATION, X, X, X, X, X, X, X, X, X, X, 27, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_GROUPS);

    spello(PRAYER_SPIRITUAL_ASYLUM, X, X, X, X, X, X, X, X, X, X, 29, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_STANDING, TAR_IGNORE, FALSE, MAG_ROOM);       

    spello(PRAYER_REVELATION, X, X, X, X, X, X, X, X, X, X, 32, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       

    spello(PRAYER_GUIDING_LIGHT, X, X, X, X, X, X, X, X, X, X, 34, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       

    spello(PRAYER_DIVINE_PRESENCE, X, X, X, X, X, X, X, X, X, X, 35, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       

    spello(PRAYER_RETRIEVE_CORPSE, X, X, X, X, X, X, X, X, X, X, 36, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_MANUAL);       

    spello(PRAYER_TRUE_SEEING, X, X, X, X, X, X, X, X, X, X, 37, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       

    spello(PRAYER_SUMMON_AVATAR, X, X, X, X, X, X, X, X, X, X, 39, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       

    spello(PRAYER_RESSURECTION, X, X, X, X, X, X, X, X, X, X, 43, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       

    spello(PRAYER_WISH, X, X, X, X, X, X, X, X, X, X, 48, X, X,
           SKILL_TYPE_CLERIC, 30, 10, 2,
           POS_FIGHTING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);       



    /* Mag  Cle  Thi  War  Ran  Bar  Dru  Nec  Mon  Av  Ni  2  3  Type Max Min Chn */
    *mojne = '#';
    log1(mojne);
    logs("Number of spells/skills by class:");
    for (i=0;i<11;i++)
        logs("   %-12s %2d", pc_class_types[i], classnumskills[i]);
    /*gs("   Cleric  %d.", classnumskills[1]);
    logs("   Thief   %d.", classnumskills[2]);
    logs("   Warrior %d.", classnumskills[3]);
    logs("   Ranger  %d.", classnumskills[4]);
    logs("   Druid   %d.", classnumskills[6]);
    logs("   Monk    %d.", classnumskills[8]);*/


}
