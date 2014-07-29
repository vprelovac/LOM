#define MAX_EGO_WEAPON_LOW 17
#define MAX_EGO_WEAPON_MED 8
#define MAX_EGO_WEAPON_HIG 1
#define MAX_EGO_ARMOR_LOW 6
#define MAX_EGO_ARMOR_MED 5
#define MAX_EGO_ARMOR_HIG 1

#define MAX_EGO_ARMOR 20
#define MAX_EGO_WEAPON 33

#define EGO_WEAPONS_FILE "egoweapons.dat"
#define EGO_ARMORS_FILE "egoarmors.dat"


struct Sego_weapons
{
    char name[100];
    int min_lev;
    int item2;
    int loc1, mod1, loc2, mod2, loc3, mod3;
    int aff1, aff2, aff3; 
    int spell, timer;
    int enh;
};


struct Sego_weapons make_ego(struct obj_data *obj, int lev);
int check_existing_ego(char *name, int type);
void write_ego_to_file(struct Sego_weapons ego, int type);
void init_ego();
