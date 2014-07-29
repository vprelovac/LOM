#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "structs.h"
#include "class.h"
#include "objs.h"
#include "interpreter.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "boards.h"
#include "olc.h"

/* external variables */
extern struct obj_data *obj_proto;
extern int      top_of_objt;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;

/* for objects */
extern char    *item_types[];
extern char    *wear_bits[];
extern char    *extra_bits[];
extern char    *extra_bits2[];
extern char    *affected_bits[];
extern char    *affected_bits2[];
extern char    *affected_bits3[];
extern char    *drinks[];
extern char    *apply_types[];
extern char    *container_bits[];

/* for spell list */
extern char    *spells[];

#define NUM_ITEM_TYPES 27
#define NUM_WEAR_FLAGS 17
#define NUM_EXTRA_FLAGS 31
#define NUM_EXTRA_FLAGS2 6
#define NUM_APPLY_FLAGS 25
#define NUM_AFFECTED1_FLAGS 31
#define NUM_AFFECTED2_FLAGS 16
#define NUM_AFFECTED3_FLAGS 1
#define NUM_SPELLS 110

void            iedit_disp_container_flags_menu(struct descriptor_data * d);
void            iedit_disp_extradesc_menu(struct descriptor_data * d);
void            iedit_disp_weapon_menu(struct descriptor_data * d);
void            iedit_disp_val1_menu(struct descriptor_data * d);
void            iedit_disp_val2_menu(struct descriptor_data * d);
void            iedit_disp_val3_menu(struct descriptor_data * d);
void            iedit_disp_val4_menu(struct descriptor_data * d);
void            iedit_disp_type_menu(struct descriptor_data * d);
void            iedit_disp_extra_menu(struct descriptor_data * d);
void            iedit_disp_wear_menu(struct descriptor_data * d);
void            iedit_disp_menu(struct descriptor_data * d);

void            iedit_disp_affected1_menu(struct descriptor_data * d);
void            iedit_disp_affected2_menu(struct descriptor_data * d);
void            iedit_disp_affected3_menu(struct descriptor_data * d);

void            iedit_parse(struct descriptor_data * d, char *arg);
void            iedit_disp_spells_menu(struct descriptor_data * d);


/**************************************************************************
 Menu functions
 **************************************************************************/

/* For container flags */
void            iedit_disp_container_flags_menu(struct descriptor_data * d)
{
    send_to_char("1) CLOSEABLE\r\n", d->character);
    send_to_char("2) PICKPROOF\r\n", d->character);
    send_to_char("3) CLOSED\r\n", d->character);
    send_to_char("4) LOCKED\r\n", d->character);
    sprintbit(GET_OBJ_VAL(d->edit_obj, 1), container_bits, buf1);
    sprintf(buf, "Container flags: %s\r\n", buf1);
    send_to_char(buf, d->character);

    send_to_char("Enter flag, 0 to quit:", d->character);
}

/* For extra descriptions */
void            iedit_disp_extradesc_menu(struct descriptor_data * d)
{
    struct extra_descr_data *extra_desc =
                    (struct extra_descr_data *) * d->misc_data;

    send_to_char("Extra desc menu\r\n", d->character);
    send_to_char("0) Quit\r\n", d->character);
    sprintf(buf, "1) Keyword: %s\r\n", extra_desc->keyword
            ? extra_desc->keyword : "<NONE>");
    send_to_char(buf, d->character);
    sprintf(buf, "2) Description:\r\n%s\r\n", extra_desc->description ?
            extra_desc->description : "<NONE>");
    send_to_char(buf, d->character);
    if (!extra_desc->next)
        send_to_char("3) <NOT SET>\r\n", d->character);
    else
        send_to_char("3) Set. <NOT VIEWED>\r\n", d->character);
    d->edit_mode = IEDIT_EXTRADESC_MENU;
}

/* Ask for *which* apply to edit */
void            iedit_disp_prompt_apply_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
        if (d->edit_obj->affected[counter].modifier) {
            sprinttype(d->edit_obj->affected[counter].location, apply_types, buf2);
            sprintf(buf, " %d) %+d to %s\r\n", counter + 1,
                    d->edit_obj->affected[counter].modifier, buf2);
            send_to_char(buf, d->character);
        } else {
            sprintf(buf, " %d) None.\r\n", counter + 1);
            send_to_char(buf, d->character);
        }
    }
    send_to_char("Enter affection to modify (0 to quit):", d->character);
    d->edit_mode = IEDIT_PROMPT_APPLY;
}

/* The actual apply to set */
void            iedit_disp_apply_menu(struct descriptor_data * d)
{
    int             counter;

    for (counter = 0; counter < NUM_APPLY_FLAGS; counter += 2) {
        sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
                counter, apply_types[counter],
                counter + 1, counter + 1 < NUM_APPLY_FLAGS ?
                apply_types[counter + 1] : "");
        send_to_char(buf, d->character);
    }
    send_to_char("Enter apply type (0 is no apply):", d->character);
    d->edit_mode = IEDIT_APPLY;
}

/* weapon type */
void            iedit_disp_weapon_menu(struct descriptor_data * d)
{
    send_to_char("1) sting\r\n", d->character);
    send_to_char("2) whip\r\n", d->character);
    send_to_char("3) slash\r\n", d->character);
    send_to_char("4) bite\r\n", d->character);
    send_to_char("5) bludgeon\r\n", d->character);
    send_to_char("6) crush\r\n", d->character);
    send_to_char("7) pound\r\n", d->character);
    send_to_char("8) claw\r\n", d->character);
    send_to_char("9) maul\r\n", d->character);
    send_to_char("10) thrash\r\n", d->character);
    send_to_char("11) pierce\r\n", d->character);
    send_to_char("12) blast\r\n", d->character);
    send_to_char("13) punch\r\n", d->character);
    send_to_char("14) stab\r\n", d->character);
    send_to_char("Enter weapon type:\r\n", d->character);
}

/* spell type */
void            iedit_disp_spells_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 1; counter < NUM_SPELLS; counter += 3) {
        sprintf(buf, "%2d) %20.20s %2d) %20.20s %2d) %20.20s\r\n",
                counter, spells[counter],
                counter + 1, counter + 1 < NUM_SPELLS ? spells[counter + 1] : "",
                counter + 2, counter + 2 < NUM_SPELLS ? spells[counter + 2] : "");
        send_to_char(buf, d->character);
    }
    send_to_char("Enter spell choice (0 for none):\r\n", d->character);
}

/* object value 1 */
void            iedit_disp_val1_menu(struct descriptor_data * d)
{
    d->edit_mode = IEDIT_VALUE_1;
    switch (GET_OBJ_TYPE(d->edit_obj)) {
    case ITEM_LIGHT:
        /* values 0 and 1 are unused.. jump to 2 */
        iedit_disp_val3_menu(d);
        break;
    case ITEM_SCROLL:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_POTION:
        send_to_char("Spell level:", d->character);
        break;
    case ITEM_WEAPON:
    case ITEM_FIREWEAPON:
        /* this seems to be a circleism.. not part of normal diku? */
        send_to_char("+Dam:", d->character);
        break;
    case ITEM_MISSILE:
        send_to_char("Range:", d->character);
        break;
    case ITEM_ARMOR:
        send_to_char("Apply to AC:", d->character);
        break;
    case ITEM_CONTAINER:
        send_to_char("Max weight to contain:", d->character);
        break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
        send_to_char("Max drink units:", d->character);
        break;
    case ITEM_FOOD:
        send_to_char("Hours to fill stomach:", d->character);
        break;
    case ITEM_MONEY:
        send_to_char("Number of gold coins:", d->character);
        break;
    case ITEM_NOTE:
        /* this is supposed to be language, but it's unused */
        break;
    default:
        iedit_disp_menu(d);
    }
}

/* object value 2 */
void            iedit_disp_val2_menu(struct descriptor_data * d)
{
    d->edit_mode = IEDIT_VALUE_2;
    switch (GET_OBJ_TYPE(d->edit_obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
        iedit_disp_spells_menu(d);
        break;
    case ITEM_WAND:
    case ITEM_STAFF:
        send_to_char("Max number of charges:", d->character);
        break;
    case ITEM_FIREWEAPON:
    case ITEM_WEAPON:
    case ITEM_MISSILE:
        send_to_char("Number of damage dice:", d->character);
        break;
    case ITEM_FOOD:
        /* values 2 and 3 are unused, jump to 4. how odd */
        iedit_disp_val4_menu(d);
        break;
    case ITEM_CONTAINER:
        /* these are flags, needs a bit of special handling */
        iedit_disp_container_flags_menu(d);
        break;
    default:
        iedit_disp_menu(d);
    }
}

/* object value 3 */
void            iedit_disp_val3_menu(struct descriptor_data * d)
{
    d->edit_mode = IEDIT_VALUE_3;
    switch (GET_OBJ_TYPE(d->edit_obj)) {
    case ITEM_LIGHT:
        send_to_char("Number of hours (0 = burnt, -1 is infinite):", d->character);
        break;
    case ITEM_SCROLL:
    case ITEM_POTION:
        iedit_disp_spells_menu(d);
        break;
    case ITEM_WAND:
    case ITEM_STAFF:
        send_to_char("Number of charges remaining:", d->character);
        break;
    case ITEM_WEAPON:
    case ITEM_FIREWEAPON:
    case ITEM_MISSILE:
        send_to_char("Size of damage dice:", d->character);
        break;
    case ITEM_CONTAINER:
        send_to_char("Vnum of key to open container (-1 for no key):", d->character);
        break;
    default:
        iedit_disp_menu(d);
    }
}

/* object value 4 */
void            iedit_disp_val4_menu(struct descriptor_data * d)
{
    d->edit_mode = IEDIT_VALUE_4;
    switch (GET_OBJ_TYPE(d->edit_obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_WAND:
    case ITEM_STAFF:
        iedit_disp_spells_menu(d);
        break;
    case ITEM_WEAPON:
        iedit_disp_weapon_menu(d);
        break;
    case ITEM_FIREWEAPON:
    case ITEM_MISSILE:
        send_to_char("MissileID: ", d->character);
        break;
    case ITEM_FOOD:
        send_to_char("Poisoned (0 = not poison)", d->character);
        break;
    default:
        iedit_disp_menu(d);
    }
}

/* object type */
void            iedit_disp_type_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 0; counter < NUM_ITEM_TYPES; counter += 2) {
        sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
                counter, item_types[counter],
                counter + 1, counter + 1 < NUM_ITEM_TYPES ?
                item_types[counter + 1] : "");
        send_to_char(buf, d->character);
    }
    send_to_char("Enter object type:", d->character);
}

/* object extra flags */
void            iedit_disp_extra_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 0; counter <= NUM_EXTRA_FLAGS + NUM_EXTRA_FLAGS2; counter += 2) {
        if (counter <= NUM_EXTRA_FLAGS)
            sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
                    counter + 1, extra_bits[counter],
                    counter + 2, counter + 1 < NUM_EXTRA_FLAGS ?
                    extra_bits[counter + 1] : "");
        else
            sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
                    counter + 1, extra_bits2[counter - 32],
                    counter + 2, counter + 1 < NUM_EXTRA_FLAGS + NUM_EXTRA_FLAGS2 ?
                    extra_bits2[counter - 31] : "");
        send_to_char(buf, d->character);
    }
    sprintbit(GET_OBJ_EXTRA(d->edit_obj), extra_bits, buf1);
    sprintf(buf, "Object flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    sprintbit(GET_OBJ_EXTRA2(d->edit_obj), extra_bits2, buf1);
    sprintf(buf, "Object flags2: %s\r\n", buf1);
    send_to_char(buf, d->character);
    send_to_char("Enter object extra flag, 0 to quit:", d->character);
}

/* object wear flags */
void            iedit_disp_wear_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 0; counter < NUM_WEAR_FLAGS; counter += 2) {
        sprintf(buf, "%2d) %20.20s %2d %20.20s\r\n",
                counter + 1, wear_bits[counter],
                counter + 2, counter + 1 < NUM_WEAR_FLAGS ?
                wear_bits[counter + 1] : "");
        send_to_char(buf, d->character);
    }
    sprintbit(GET_OBJ_WEAR(d->edit_obj), wear_bits, buf1);
    sprintf(buf, "Wear flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    send_to_char("Enter wear flag, 0 to quit:", d->character);
}

/* object affected flags */
void            iedit_disp_affected1_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 0; counter < NUM_AFFECTED1_FLAGS; counter += 2) {
        sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
                counter + 1, affected_bits[counter],
                counter + 2, counter + 1 < NUM_AFFECTED1_FLAGS ?
                affected_bits[counter + 1] : "");
        send_to_char(buf, d->character);
    }
    sprintbit(GET_OBJ_BIT(d->edit_obj), affected_bits, buf1);
    sprintf(buf, "Object affected 1 flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    send_to_char("Enter object affect flag, 0 to quit:", d->character);
}

/* object affected flags */
void            iedit_disp_affected2_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 0; counter < NUM_AFFECTED2_FLAGS; counter += 2) {
        sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
                counter + 1, affected_bits2[counter],
                counter + 2, counter + 1 < NUM_AFFECTED2_FLAGS ?
                affected_bits2[counter + 1] : "");
        send_to_char(buf, d->character);
    }
    sprintbit(GET_OBJ_BIT2(d->edit_obj), affected_bits2, buf1);
    sprintf(buf, "Object affected 2 flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    send_to_char("Enter object affect flag, 0 to quit:", d->character);
}

/* object affected flags */
void            iedit_disp_affected3_menu(struct descriptor_data * d)
{
    int             counter;

    send_to_char("[H[J", d->character);
    for (counter = 0; counter < NUM_AFFECTED3_FLAGS; counter += 2) {
        sprintf(buf, "%2d) %20.20s %2d) %20.20s\r\n",
                counter + 1, affected_bits3[counter],
                counter + 2, counter + 1 < NUM_AFFECTED3_FLAGS ?
                affected_bits3[counter + 1] : "");
        send_to_char(buf, d->character);
    }
    sprintbit(GET_OBJ_BIT3(d->edit_obj), affected_bits3, buf1);
    sprintf(buf, "Object affected 3 flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    send_to_char("Enter object affect flag, 0 to quit:", d->character);
}


/* display main menu */
void            iedit_disp_menu(struct descriptor_data * d)
{
    send_to_char("[H[J", d->character);
    sprintf(buf, "Item number: %d\r\n", d->edit_number);
    send_to_char(buf, d->character);
    sprintf(buf, "1) Item namelist: %s\r\n", d->edit_obj->name);
    send_to_char(buf, d->character);
    sprintf(buf, "2) Item shortdesc: %s\r\n", d->edit_obj->short_description);
    send_to_char(buf, d->character);
    sprintf(buf, "3) Item longdesc: %s\r\n", d->edit_obj->description);
    send_to_char(buf, d->character);
    sprintf(buf, "4) Item actdesc: %s\r\n",
            d->edit_obj->action_description ?
            d->edit_obj->action_description :
            "NOT SET");
    send_to_char(buf, d->character);
    sprinttype(GET_OBJ_TYPE(d->edit_obj), item_types, buf1);
    sprintf(buf, "5) Item type: %s\r\n", buf1);
    send_to_char(buf, d->character);
    sprintbit(GET_OBJ_EXTRA(d->edit_obj), extra_bits, buf1);
    sprintf(buf, "6) Item extra flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    sprintbit(GET_OBJ_EXTRA2(d->edit_obj), extra_bits2, buf1);
    sprintf(buf, "   Item extra2 flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    sprintbit(GET_OBJ_WEAR(d->edit_obj), wear_bits, buf1);
    sprintf(buf, "7) Item wear flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    sprintf(buf, "8) Item weight: %d\r\n", GET_OBJ_WEIGHT(d->edit_obj));
    send_to_char(buf, d->character);
    sprintf(buf, "9) Item cost: %d\r\n", GET_OBJ_COST(d->edit_obj));
    send_to_char(buf, d->character);
    sprintf(buf, "a) Item cost per day: %d\r\n", GET_OBJ_RENT(d->edit_obj));
    send_to_char(buf, d->character);
    sprintf(buf, "b) Item timer: %d\r\n", GET_OBJ_TIMER(d->edit_obj));
    send_to_char(buf, d->character);
    sprintf(buf, "c) Item values: %d %d %d %d\r\n",
            GET_OBJ_VAL(d->edit_obj, 0), GET_OBJ_VAL(d->edit_obj, 1),
            GET_OBJ_VAL(d->edit_obj, 2), GET_OBJ_VAL(d->edit_obj, 3));
    send_to_char(buf, d->character);
    send_to_char("d) Item applies:\r\n", d->character);
    send_to_char("e) Item extra descriptions:\r\n", d->character);
    sprintbit(GET_OBJ_BIT(d->edit_obj), affected_bits, buf1);
    sprintf(buf, "f) Item affect flags: %s\r\n", buf1);
    send_to_char(buf, d->character);
    sprintbit(GET_OBJ_BIT2(d->edit_obj), affected_bits2, buf1);
    sprintf(buf, "g) Item affect flags2: %s\r\n", buf1);
    send_to_char(buf, d->character);
    sprintbit(GET_OBJ_BIT3(d->edit_obj), affected_bits3, buf1);
    sprintf(buf, "h) Item affect flags3: %s\r\n", buf1);
    send_to_char("q) Quit\r\n", d->character);
    send_to_char("Enter your choice:\r\n", d->character);
    d->edit_mode = IEDIT_MAIN_MENU;
}

/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/


void            iedit_parse(struct descriptor_data * d, char *arg)
{
    int             number;
    int             obj_number; /* the RNUM */
    switch (d->edit_mode) {

    case IEDIT_CONFIRM_EDIT:
        /* if player hits 'Y' then edit obj */
        switch (*arg) {
        case 'y':
        case 'Y':
            SET_BIT(PLR_FLAGS(d->character), PLR_EDITING);
            iedit_disp_menu(d);
            break;
        case 'n':
        case 'N':
            STATE(d) = CON_PLAYING;
            /* free up the editing object */
            if (d->edit_obj)
                free_obj(d->edit_obj);
            d->edit_obj = NULL;
            d->edit_number = 0;
            REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
            break;
        default:
            send_to_char("That's not a valid choice!\r\n", d->character);
            send_to_char("Do you wish to edit it?\r\n", d->character);
            break;
        }
        break;                  /* end of IEDIT_CONFIRM_EDIT */

    case IEDIT_CONFIRM_SAVESTRING:
        switch (*arg) {
        case 'y':
        case 'Y':
            {
                /* write to internal tables */
                obj_number = real_object(d->edit_number);
                if (obj_number > 0) {
                    /* we need to run through each and every object currently
                     * in the game to see which ones are pointing to this
                     * prototype */
                    struct obj_data *i;
                    struct extra_descr_data *this,
                                *next_one;
                    extern struct obj_data *object_list;
                    struct obj_data *temp;

                    /* if object is pointing to this prototype, then we need
                     * to replace with the new one */
            for (i = object_list; i; i = i->next) {
                        if (i->item_number == obj_number) {
                            /* alloc temp object */
                            CREATE(temp, struct obj_data, 1);

                            *temp = *i;
                            *i = *d->edit_obj;
                            /* copy game-time dependent vars over */
                            i->in_room = temp->in_room;
                            i->item_number = obj_number;
                            i->carried_by = temp->carried_by;
                            i->worn_by = temp->worn_by;
                            i->worn_on = temp->worn_on;
                            i->in_obj = temp->in_obj;
                            i->contains = temp->contains;
                            i->next_content = temp->next_content;
                            i->next = temp->next;
                            free_obj(temp);
                        }
                    }
                    /* now safe to free old proto and write over */
                    if (obj_proto[obj_number].name)
                        DISPOSE(obj_proto[obj_number].name);
                    if (obj_proto[obj_number].description)
                        DISPOSE(obj_proto[obj_number].description);
                    if (obj_proto[obj_number].short_description)
                        DISPOSE(obj_proto[obj_number].short_description);
                    if (obj_proto[obj_number].action_description)
                        DISPOSE(obj_proto[obj_number].action_description);
                    if (obj_proto[obj_number].ex_description)
                        for (this = obj_proto[obj_number].ex_description;
                                this; this = next_one) {
                            next_one = this->next;
                            if (this->keyword)
                                DISPOSE(this->keyword);
                            if (this->description)
                                DISPOSE(this->description);
                            DISPOSE(this);
                        }
                    obj_proto[obj_number] = *d->edit_obj;
                    obj_proto[obj_number].item_number = obj_number;
                } else {
                    /* uhoh.. need to make a new place in the object
                     * prototype table */
                    int             counter;
                    int             found = FALSE;

                    struct obj_data *new_obj_proto;
                    struct index_data *new_obj_index;

                    /* + 2.. strange but true */
                    CREATE(new_obj_index, struct index_data, top_of_objt + 2);
                    CREATE(new_obj_proto, struct obj_data, top_of_objt + 2);
                    /* start counting through both tables */
                    for (counter = 0; counter < top_of_objt + 1; counter++) {
                        /* if we haven't found it */
                        if (!found) {
                            /* check if current virtual is bigger than our
                             * virtual */
                            if (obj_index[counter].virtual > d->edit_number) {
                                /* eureka. insert now */
                                /*---------*/
                                new_obj_index[counter].virtual = d->edit_number;
                                new_obj_index[counter].func = NULL;
                                /*---------*/
                                new_obj_proto[counter] = *(d->edit_obj);
                                new_obj_proto[counter].in_room = NOWHERE;
                                /* it is now safe (and necessary!) to assign
                                 * real number to the edit_obj, which has
                                 * been -1 all this time */
                                d->edit_obj->item_number = counter;
                                /* and assign to prototype as well */
                                new_obj_proto[counter].item_number = counter;
                                found = TRUE;
                                /* insert the other proto at this point */
                                new_obj_index[counter + 1] = obj_index[counter];
                                new_obj_proto[counter + 1] = obj_proto[counter];
                                new_obj_proto[counter + 1].item_number = counter + 1;
                            } else {
                                /* just copy from old to new, no num change */
                                new_obj_proto[counter] = obj_proto[counter];
                                new_obj_index[counter] = obj_index[counter];
                            }
                        } else {
                            /* we HAVE already found it.. therefore copy to
                             * object + 1 */
                            new_obj_index[counter + 1] = obj_index[counter];
                            new_obj_proto[counter + 1] = obj_proto[counter];
                            new_obj_proto[counter + 1].item_number = counter + 1;
                        }
                    }
                    /* if we STILL haven't found it, means the object was >
                     * than all the other objects.. so insert at end */
                    if (!found) {
                        new_obj_index[top_of_objt + 1].virtual = d->edit_number;
                        new_obj_index[top_of_objt + 1].func = NULL;

                        clear_object(new_obj_proto + top_of_objt + 1);
                        new_obj_proto[top_of_objt + 1] = *(d->edit_obj);
                        new_obj_proto[top_of_objt + 1].in_room = NOWHERE;
                        new_obj_proto[top_of_objt + 1].item_number = top_of_objt + 1;
                    }
                    top_of_objt++;

                    /* free and replace old tables */
                    DISPOSE(obj_proto);
                    DISPOSE(obj_index);
                    obj_proto = new_obj_proto;
                    obj_index = new_obj_index;
                }
                send_to_char("Do you wish to write this object to disk?\r\n", d->character);
                d->edit_mode = IEDIT_CONFIRM_SAVEDB;
            }
            break;
        case 'n':
        case 'N':
            send_to_char("Object not saved, aborting.\r\n", d->character);
            STATE(d) = CON_PLAYING;
            /* free up the editing object. free_obj *is* safe since it checks
             * against prototype table */
            if (d->edit_obj)
                free_obj(d->edit_obj);
            d->edit_obj = NULL;
            d->edit_number = 0;
            REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
            break;
        default:
            send_to_char("Invalid choice!\r\n", d->character);
            send_to_char("Do you wish to save this object internally?\r\n", d->character);
            break;
        }
        break;                  /* end of IEDIT_CONFIRM_SAVESTRING */

    case IEDIT_CONFIRM_SAVEDB:
        switch (*arg) {
        case 'y':
        case 'Y':
            send_to_char("Writing object to disk..", d->character);
            {
                int             counter,
                counter2,
                realcounter;
                FILE           *fp;
                char            newname[50],
                oldname[50];
                char            buf[33],
                buf2[33],
                buf3[33];
                struct obj_data *obj;
                struct extra_descr_data *ex_desc;

                /* i want to use "obj" instead of just obj_proto[] because
                 * some of the macros assume it's a pointer instead of just a
                 * struct as obj_proto is, plus it's short to type in :P */
                CREATE(obj, struct obj_data, 1);

                sprintf(newname, "%s/%d.obj.back", OBJ_PREFIX,
                        zone_table[d->edit_zone].number);
                sprintf(oldname, "%s/%d.obj", OBJ_PREFIX,
                        zone_table[d->edit_zone].number);
                fp = fopen(newname, "w+");

                /* start running through all objects in this zone */
                for (counter = zone_table[d->edit_zone].number * 100;
                        counter <= zone_table[d->edit_zone].top;
                        counter++) {
                    /* write object to disk */
                    realcounter = real_object(counter);
                    if (realcounter >= 0) {
                        *obj = obj_proto[realcounter];
                        fprintf(fp, "#%d\n", GET_OBJ_VNUM(obj));
                        fprintf(fp, "%s~\n", obj->name);
                        fprintf(fp, "%s~\n", obj->short_description);
                        fprintf(fp, "%s~\n", obj->description);
                        if (obj->action_description)
                            fprintf(fp, "%s~\n", obj->action_description);
                        else
                            fprintf(fp, "~\n");
                        fprintf(fp, "%d %s %s %s\n", GET_OBJ_TYPE(obj),
                                sprintbitascii(GET_OBJ_EXTRA(obj), buf),
                                sprintbitascii(GET_OBJ_EXTRA2(obj), buf2),
                                sprintbitascii(GET_OBJ_WEAR(obj), buf3));
                        fprintf(fp, "%d %d %d %d\n", GET_OBJ_VAL(obj, 0),
                                GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2),
                                GET_OBJ_VAL(obj, 3));
                        fprintf(fp, "%d %d %d\n", GET_OBJ_WEIGHT(obj),
                                GET_OBJ_COST(obj), GET_OBJ_RENT(obj));
                        /* do we have extra descriptions? */
                        if (obj->ex_description) {
                            for (ex_desc = obj->ex_description; ex_desc; ex_desc = ex_desc->next) {
                                fprintf(fp, "E\n");
                                fprintf(fp, "%s~\n", ex_desc->keyword);
                                fprintf(fp, "%s~\n", ex_desc->description);
                            }
                        }
                        if (obj->obj_flags.bitvector || obj->obj_flags.bitvector2 || obj->obj_flags.bitvector3) {
                            fprintf(fp, "F\n");
                            fprintf(fp, "%s %s %s\n",
                                    sprintbitascii(obj->obj_flags.bitvector, buf),
                                    sprintbitascii(obj->obj_flags.bitvector2, buf2),
                                    sprintbitascii(obj->obj_flags.bitvector3, buf3));
                        }
                        /* do we have affects? */
                        for (counter2 = 0; counter2 < MAX_OBJ_AFFECT; counter2++) {
                            if (obj->affected[counter2].modifier) {
                                fprintf(fp, "A\n");
                                fprintf(fp, "%d %d\n", obj->affected[counter2].location,
                                        obj->affected[counter2].modifier);
                            }
                        }
                    }
                }
                /* write final line, close */
                fprintf(fp, "$~\n");
                fclose(fp);
                remove(oldname);
                rename(newname, oldname);
                /* nuke temp object */
                free_obj(obj);
            }
            if (d->edit_obj)
                free_obj(d->edit_obj);
            d->edit_obj = NULL;
            STATE(d) = CON_PLAYING;
            REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
            send_to_char("Done.\r\n", d->character);
            break;
        case 'n':
        case 'N':
            send_to_char("Not saved to DB.\r\n", d->character);
            send_to_char("This object is available until the next reboot.\r\n", d->character);
            if (d->edit_obj)
                free_obj(d->edit_obj);
            d->edit_obj = NULL;
            STATE(d) = CON_PLAYING;
            REMOVE_BIT(PLR_FLAGS(d->character), PLR_EDITING);
            send_to_char("Done.\r\n", d->character);
            break;
        default:
            send_to_char("Invalid choice!\r\n", d->character);
            send_to_char("Do you wish to write this object to disk?\r\n", d->character);
            break;

        }
        break;                  /* end of IEDIT_CONFIRM_SAVEDB */

    case IEDIT_MAIN_MENU:
        /* throw us out to whichever edit mode based on user input */
        switch (*arg) {
        case 'q':
        case 'Q':
            send_to_char("Do you wish to save this object internally?\r\n", d->character);
            d->edit_mode = IEDIT_CONFIRM_SAVESTRING;
            break;
        case '1':
            send_to_char("Enter namelist:", d->character);
            d->edit_mode = IEDIT_EDIT_NAMELIST;
            break;
        case '2':
            send_to_char("Enter short desc:", d->character);
            d->edit_mode = IEDIT_SHORTDESC;
            break;
        case '3':
            send_to_char("Enter long desc:\r\n", d->character);
            d->edit_mode = IEDIT_LONGDESC;
            break;
        case '4':
            /* let's go out to modify.c */
            send_to_char("Enter action desc:\r\n", d->character);
            d->edit_mode = IEDIT_ACTDESC;
            d->str = (char **) malloc(sizeof(char *));

            *(d->str) = NULL;
            d->max_str = MAX_MESSAGE_LENGTH;
            d->mail_to = 0;
            break;
        case '5':
            iedit_disp_type_menu(d);
            d->edit_mode = IEDIT_TYPE;
            break;
        case '6':
            iedit_disp_extra_menu(d);
            d->edit_mode = IEDIT_EXTRAS;
            break;
        case '7':
            iedit_disp_wear_menu(d);
            d->edit_mode = IEDIT_WEAR;
            break;
        case '8':
            send_to_char("Enter weight:", d->character);
            d->edit_mode = IEDIT_WEIGHT;
            break;
        case '9':
            send_to_char("Enter cost:", d->character);
            d->edit_mode = IEDIT_COST;
            break;
        case 'a':
        case 'A':
            send_to_char("Enter cost per day:", d->character);
            d->edit_mode = IEDIT_COSTPERDAY;
            break;
        case 'b':
        case 'B':
            send_to_char("Enter timer:", d->character);
            d->edit_mode = IEDIT_TIMER;
            break;
        case 'c':
        case 'C':
            iedit_disp_val1_menu(d);
            break;
        case 'd':
        case 'D':
            iedit_disp_prompt_apply_menu(d);
            break;
        case 'e':
        case 'E':
            /* if extra desc doesn't exist . */
            if (!d->edit_obj->ex_description) {
                CREATE(d->edit_obj->ex_description, struct extra_descr_data, 1);

                d->edit_obj->ex_description->next = NULL;
            }
            /* There is a reason I need the double pointer. If at the extra
             * desc menu user presses '0' then I need to free the extra
             * description. Since it's at the end of list it's pointer must
             * be pointing to NULL.. thus the double pointer */
            d->misc_data = (void **) &(d->edit_obj->ex_description);
            iedit_disp_extradesc_menu(d);
            break;
        case 'f':
        case 'F':
            iedit_disp_affected1_menu(d);
            d->edit_mode = IEDIT_AFFECTED1;
            break;
        case 'g':
        case 'G':
            iedit_disp_affected2_menu(d);
            d->edit_mode = IEDIT_AFFECTED2;
            break;
        case 'h':
        case 'H':
            iedit_disp_affected3_menu(d);
            d->edit_mode = IEDIT_AFFECTED3;
            break;
        default:
            /* hm, i just realized you probably can't see this.. maybe prompt
             * for an extra RETURN. well, maybe later */
            send_to_char("That's not a valid choice!\r\n", d->character);
            iedit_disp_menu(d);
            break;
        }
        break;                  /* end of IEDIT_MAIN_MENU */

    case IEDIT_EDIT_NAMELIST:
        if (d->edit_obj->name)
            DISPOSE(d->edit_obj->name);
        d->edit_obj->name = str_dup(arg);
        iedit_disp_menu(d);
        break;
    case IEDIT_SHORTDESC:
        if (d->edit_obj->short_description)
            DISPOSE(d->edit_obj->short_description);
        d->edit_obj->short_description = str_dup(arg);
        iedit_disp_menu(d);
        break;
    case IEDIT_LONGDESC:
        if (d->edit_obj->description)
            DISPOSE(d->edit_obj->description);
        d->edit_obj->description = str_dup(arg);
        iedit_disp_menu(d);
        break;
    case IEDIT_ACTDESC:
        /* WE SHOULD NEVER GET HERE!! */
        break;
    case IEDIT_TYPE:
        number = atoi(arg);
        if ((number < 1) || (number >= NUM_ITEM_TYPES))
            send_to_char("That's not a valid choice!\r\n", d->character);
        else {
            GET_OBJ_TYPE(d->edit_obj) = number;
            iedit_disp_menu(d);
        }
        break;
    case IEDIT_EXTRAS:
        number = atoi(arg);
        if ((number < 0) || (number > NUM_EXTRA_FLAGS + NUM_EXTRA_FLAGS2 + 1)) {
            send_to_char("That's not a valid choice!\r\n", d->character);
            iedit_disp_extra_menu(d);
        } else {
            /* if 0, quit */
            if (number == 0)
                iedit_disp_menu(d);
            /* if already set.. remove */
            else if (number > NUM_EXTRA_FLAGS) {
                number -= (NUM_EXTRA_FLAGS + 1);
                if (IS_SET(GET_OBJ_EXTRA2(d->edit_obj), 1 << (number - 1)))
                    REMOVE_BIT(GET_OBJ_EXTRA2(d->edit_obj), 1 << (number - 1));
                else
                    /* set */
                    SET_BIT(GET_OBJ_EXTRA2(d->edit_obj), 1 << (number - 1));
                iedit_disp_extra_menu(d);
            } else {
                if (IS_SET(GET_OBJ_EXTRA(d->edit_obj), 1 << (number - 1)))
                    REMOVE_BIT(GET_OBJ_EXTRA(d->edit_obj), 1 << (number - 1));
                else
                    /* set */
                    SET_BIT(GET_OBJ_EXTRA(d->edit_obj), 1 << (number - 1));
                iedit_disp_extra_menu(d);
            }
        }
        break;
    case IEDIT_WEAR:
        number = atoi(arg);
        if ((number < 0) || (number > NUM_WEAR_FLAGS)) {
            send_to_char("That's not a valid choice!\r\n", d->character);
            iedit_disp_wear_menu(d);
        } else {
            /* if 0, quit */
            if (number == 0)
                iedit_disp_menu(d);
            /* if already set.. remove */
            else {
                if (IS_SET(GET_OBJ_WEAR(d->edit_obj), 1 << (number - 1)))
                    REMOVE_BIT(GET_OBJ_WEAR(d->edit_obj), 1 << (number - 1));
                else
                    SET_BIT(GET_OBJ_WEAR(d->edit_obj), 1 << (number - 1));
                iedit_disp_wear_menu(d);
            }
        }
        break;
    case IEDIT_WEIGHT:
        number = atoi(arg);
        GET_OBJ_WEIGHT(d->edit_obj) = number;
        iedit_disp_menu(d);
        break;
    case IEDIT_COST:
        number = atoi(arg);
        GET_OBJ_COST(d->edit_obj) = number;
        iedit_disp_menu(d);
        break;
    case IEDIT_COSTPERDAY:
        number = atoi(arg);
        GET_OBJ_RENT(d->edit_obj) = number;
        iedit_disp_menu(d);
        break;
    case IEDIT_TIMER:
        number = atoi(arg);
        GET_OBJ_TIMER(d->edit_obj) = number;
        iedit_disp_menu(d);
        break;
    case IEDIT_VALUE_1:
        /* lucky, I don't need to check any of these for outofrange values */
        number = atoi(arg);
        switch (GET_OBJ_TYPE(d->edit_obj)) {
        case ITEM_WEAPON:
        case ITEM_FIREWEAPON:
            GET_OBJ_VAL(d->edit_obj, 0) = number;
            iedit_disp_val2_menu(d);
            break;
        case ITEM_MISSILE:
            if (number < 0 || number > 10) {
                send_to_char("Invalid choice! Range is from 1 to 10", d->character);
                iedit_disp_val1_menu(d);
            } else {
                GET_OBJ_VAL(d->edit_obj, 0) = number;
                iedit_disp_val2_menu(d);
            }
            break;
        default:
            GET_OBJ_VAL(d->edit_obj, 0) = number;
            iedit_disp_val2_menu(d);
            /* proceed to menu 2 */
        }
        break;
    case IEDIT_VALUE_2:
        /* here, I do need to check for outofrange values */
        number = atoi(arg);
        switch (GET_OBJ_TYPE(d->edit_obj)) {
        case ITEM_SCROLL:
        case ITEM_POTION:
            if (number < 1 || number >= NUM_SPELLS) {
                send_to_char("Invalid choice!", d->character);
                iedit_disp_val2_menu(d);
            } else {
                GET_OBJ_VAL(d->edit_obj, 1) = number;
                iedit_disp_val3_menu(d);
            }
            break;
        case ITEM_CONTAINER:
            /* needs some special handling since we are dealing with flag
             * values here */
            /* if 0, quit */
            number = atoi(arg);
            if (number < 0 || number > 4) {
                send_to_char("Invalid choice!\r\n", d->character);
                iedit_disp_container_flags_menu(d);
            } else {
                /* if 0, quit */
                if (number != 0) {
                    if (IS_SET(GET_OBJ_VAL(d->edit_obj, 1), 1 << (number - 1)))
                        REMOVE_BIT(GET_OBJ_VAL(d->edit_obj, 1), 1 << (number - 1));
                    else
                        SET_BIT(GET_OBJ_VAL(d->edit_obj, 1), 1 << (number - 1));
                    iedit_disp_val2_menu(d);
                } else
                    iedit_disp_val3_menu(d);
            }
            break;
        default:
            GET_OBJ_VAL(d->edit_obj, 1) = number;
            iedit_disp_val3_menu(d);
        }
        /* i think that's all that needs checking */
        break;
    case IEDIT_VALUE_3:
        number = atoi(arg);
        switch (GET_OBJ_TYPE(d->edit_obj)) {
        case ITEM_SCROLL:
        case ITEM_POTION:
            if (number < 1 || number >= NUM_SPELLS) {
                send_to_char("Invalid choice!", d->character);
                iedit_disp_val3_menu(d);
            }
            break;
        }
        GET_OBJ_VAL(d->edit_obj, 2) = number;
        iedit_disp_val4_menu(d);
        break;
    case IEDIT_VALUE_4:
        number = atoi(arg);
        switch (GET_OBJ_TYPE(d->edit_obj)) {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_WAND:
        case ITEM_STAFF:
            if (number < 1 || number >= NUM_SPELLS) {
                send_to_char("Invalid choice!", d->character);
                iedit_disp_val4_menu(d);
            }
            break;
        case ITEM_WEAPON:
            if (number < 1 || number > 11) {
                send_to_char("Invalid choice!", d->character);
                iedit_disp_weapon_menu(d);
            }
            break;
        default:
            break;
        }
        GET_OBJ_VAL(d->edit_obj, 3) = number;
        iedit_disp_menu(d);
        break;
    case IEDIT_AFFECTED1:
        number = atoi(arg);
        if ((number < 0) || (number > NUM_AFFECTED1_FLAGS)) {
            send_to_char("That's not a valid choice!\r\n", d->character);
            iedit_disp_affected1_menu(d);
        } else {
            /* if 0, quit */
            if (number == 0)
                iedit_disp_menu(d);
            /* if already set.. remove */
            else {
                if (IS_SET(GET_OBJ_BIT(d->edit_obj), 1 << (number - 1)))
                    REMOVE_BIT(GET_OBJ_BIT(d->edit_obj), 1 << (number - 1));
                else
                    /* set */
                    SET_BIT(GET_OBJ_BIT(d->edit_obj), 1 << (number - 1));
                iedit_disp_affected1_menu(d);
            }
        }
        break;
    case IEDIT_AFFECTED2:
        number = atoi(arg);
        if ((number < 0) || (number > NUM_AFFECTED2_FLAGS)) {
            send_to_char("That's not a valid choice!\r\n", d->character);
            iedit_disp_affected2_menu(d);
        } else {
            /* if 0, quit */
            if (number == 0)
                iedit_disp_menu(d);
            /* if already set.. remove */
            else {
                if (IS_SET(GET_OBJ_BIT2(d->edit_obj), 1 << (number - 1)))
                    REMOVE_BIT(GET_OBJ_BIT2(d->edit_obj), 1 << (number - 1));
                else
                    /* set */
                    SET_BIT(GET_OBJ_BIT2(d->edit_obj), 1 << (number - 1));
                iedit_disp_affected2_menu(d);
            }
        }
        break;
    case IEDIT_AFFECTED3:
        number = atoi(arg);
        if ((number < 0) || (number > NUM_AFFECTED3_FLAGS)) {
            send_to_char("That's not a valid choice!\r\n", d->character);
            iedit_disp_affected3_menu(d);
        } else {
            /* if 0, quit */
            if (number == 0)
                iedit_disp_menu(d);
            /* if already set.. remove */
            else {
                if (IS_SET(GET_OBJ_BIT3(d->edit_obj), 1 << (number - 1)))
                    REMOVE_BIT(GET_OBJ_BIT3(d->edit_obj), 1 << (number - 1));
                else
                    /* set */
                    SET_BIT(GET_OBJ_BIT3(d->edit_obj), 1 << (number - 1));
                iedit_disp_affected3_menu(d);
            }
        }
        break;
    case IEDIT_PROMPT_APPLY:
        number = atoi(arg);
        if (number == 0) {
            iedit_disp_menu(d);
            return;
        } else if (number < 0 || number > MAX_OBJ_AFFECT) {
            send_to_char("Invalid choice!\r\n", d->character);
            iedit_disp_prompt_apply_menu(d);
        }
        d->edit_number2 = number - 1;
        d->edit_mode = IEDIT_APPLY;
        iedit_disp_apply_menu(d);
        break;
    case IEDIT_APPLY:
        number = atoi(arg);
        if (number == 0) {
            d->edit_obj->affected[d->edit_number2].location = 0;
            d->edit_obj->affected[d->edit_number2].modifier = 0;
            iedit_disp_prompt_apply_menu(d);
        } else if (number < 0 || number >= NUM_APPLY_FLAGS) {
            send_to_char("Invalid choice!\r\n", d->character);
            iedit_disp_apply_menu(d);
        } else {
            d->edit_obj->affected[d->edit_number2].location = number;
            send_to_char("Modifier:", d->character);
            d->edit_mode = IEDIT_APPLYMOD;
        }
        break;
    case IEDIT_APPLYMOD:
        number = atoi(arg);
        d->edit_obj->affected[d->edit_number2].modifier = number;
        iedit_disp_prompt_apply_menu(d);
        break;
    case IEDIT_EXTRADESC_KEY:
        if (((struct extra_descr_data *) * d->misc_data)->keyword)
            DISPOSE(((struct extra_descr_data *) * d->misc_data)->keyword);
        ((struct extra_descr_data *) * d->misc_data)->keyword = str_dup(arg);
        iedit_disp_extradesc_menu(d);
        break;
    case IEDIT_EXTRADESC_MENU:
        number = atoi(arg);
        switch (number) {
        case 0:
            {
                /* if something got left out */
                if (!((struct extra_descr_data *) * d->misc_data)->keyword ||
                        !((struct extra_descr_data *) * d->misc_data)->description) {
                    if (((struct extra_descr_data *) * d->misc_data)->keyword)
                        DISPOSE(((struct extra_descr_data *) * d->misc_data)->keyword);
                    if (((struct extra_descr_data *) * d->misc_data)->description)
                        DISPOSE(((struct extra_descr_data *) * d->misc_data)->description);

                    DISPOSE(*d->misc_data);
                    *d->misc_data = NULL;
                }
                /* else, we don't need to do anything.. jump to menu */
                iedit_disp_menu(d);
            }
            break;
        case 1:
            d->edit_mode = IEDIT_EXTRADESC_KEY;
            send_to_char("Enter keywords, separated by spaces:", d->character);
            break;
        case 2:
            d->edit_mode = IEDIT_EXTRADESC_DESCRIPTION;
            send_to_char("Enter description:\r\n", d->character);
            /* send out to modify.c */
            d->str = (char **) malloc(sizeof(char *));

            *(d->str) = NULL;
            d->max_str = MAX_MESSAGE_LENGTH;
            d->mail_to = 0;
            break;
        case 3:
            /* if keyword or description has not been changed don't allow
             * person to edit next */
            if (!((struct extra_descr_data *) d->misc_data)->keyword ||
                    !((struct extra_descr_data *) d->misc_data)->description) {
                send_to_char("You can't edit the next extra desc  without completing this one.\r\n", d->character);
                iedit_disp_extradesc_menu(d);
            } else {
                struct extra_descr_data *new_extra;

                if (((struct extra_descr_data *) * d->misc_data)->next)
                    d->misc_data = (void **) &((struct extra_descr_data *) * d->misc_data)->next;
                else {
                    /* make new extra, attach at end */
                    CREATE(new_extra, struct extra_descr_data, 1);

                    ((struct extra_descr_data *) * d->misc_data)->next = new_extra;
                    /* edit new extra, we NEED double pointer because i will
                     * set *d->misc_data to NULL later */
                    d->misc_data =
                        (void **) &((struct extra_descr_data *) * d->misc_data)->next;

                }
                iedit_disp_extradesc_menu(d);

            }
            break;
        default:
            send_to_char("Invalid choice!\r\n", d->character);
            iedit_disp_extradesc_menu(d);
            break;
        }
        break;
    default:
        break;
    }
}
