#define ZEDIT_CONFIRM_EDIT	0
#define ZEDIT_MAIN_MENU		1
#define ZEDIT_CONFIRM_EXIT	2
#define ZEDIT_CONFIRM_SAVE	3
#define ZEDIT_CONFIRM_SAVEDB	4
#define ZEDIT_MOB_MENU		5	/* main menu option 1 */
#define ZEDIT_OBJ_MENU		6	/* main menu option 2 */
#define ZEDIT_ROOM_MENU		7	/* main menu option 3 */
#define ZEDIT_CONT_MENU		8	/* main menu option 4 */
#define ZEDIT_REMOVE_MENU	9	/* main menu option 5 */
#define ZEDIT_MOB_EDIT	  	10
#define ZEDIT_OBJECT_EDIT	11
#define ZEDIT_ROOM_EDIT		12
#define ZEDIT_CONTAINER_EDIT	13
#define ZEDIT_INVENTORY_EDIT	14
#define ZEDIT_EQUIPMENT_EDIT	15

#define ZEDIT_MAIN_RESET	16
#define ZEDIT_MAIN_LIFESPAN	17
#define ZEDIT_MAIN_TOP		18
#define ZEDIT_MAIN_DESC		19

#define ZEDIT_MOB_NAME		20
#define ZEDIT_MOB_ROOM		21
#define ZEDIT_MOB_VNUM		22
#define ZEDIT_MOB_MAXNUM	23

#define ZEDIT_INVENTORY_NAME	24
#define ZEDIT_INVENTORY_MAXNUM	25

#define ZEDIT_EQUIPMENT_NAME	26
#define ZEDIT_EQUIPMENT_MAXNUM	27
#define ZEDIT_EQUIPMENT_LOC	28

#define ZEDIT_OBJECT_NAME	29
#define ZEDIT_OBJECT_MAXNUM	30
#define ZEDIT_OBJECT_LOC	31

#define ZEDIT_MAIN_CREATOR	32
#define ZEDIT_MAIN_MINLEV	33
#define ZEDIT_MAIN_MAXLEV	34

#define ZEDIT_ROOM_NAME		35
#define ZEDIT_ROOM_DIR		36
#define ZEDIT_ROOM_STATE        37

struct zedit_obj_list {
    int if_flag;
    int obj_vnum;
    int max_exist;
    int room_vnum;	/* doubles for worn position or container's vnum */
    struct zedit_obj_list *previous;
    struct zedit_obj_list *next;
};

struct zedit_mob_list {
    int if_flag;
    int mob_vnum;
    int max_exist;
    int room_vnum;
    struct zedit_obj_list *inventory;
    struct zedit_obj_list *equipment;
    struct zedit_mob_list *previous;
    struct zedit_mob_list *next;
};

struct zedit_room_list {
    int if_flag;
    int room_vnum;
    int door_direction;
    int door_state;
    struct zedit_room_list *previous;
    struct zedit_room_list *next;
};

struct zedit_remove_list {
    int if_flag;
    int room_vnum;
    int obj_vnum;
    struct zedit_remove_list *previous;
    struct zedit_remove_list *next;
};

struct zedit_struct {
    int	cmds;

    int	reg1;
    int	reg2;

    int	vnum;
    char 	*name;
    int	top;
    int	lifespan;
    int	reset_mode;
    char	*creator;
    int	lvl_low;
    int	lvl_high;

    struct zedit_room_list *rooms;	/* linked list of rooms */
    struct zedit_mob_list *mobs;	/* linked list of mobs */
    struct zedit_obj_list *objs;	/* linked list of objs */
    struct zedit_obj_list *contains;
    struct zedit_remove_list *removes;

    struct zedit_room_list *current_room;
    struct zedit_mob_list *current_mob;
    struct zedit_obj_list *current_obj;
    struct zedit_obj_list *current_contain;
    struct zedit_remove_list *current_remove;
};
