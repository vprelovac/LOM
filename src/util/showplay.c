/* ************************************************************************
*  file:  showplay.c                                  Part of CircleMud   *
*  Usage: list a diku playerfile                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
************************************************************************* */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
struct char_data mob;
struct obj_data obj;
struct room_data room;

void show(char *filename)
{
  char sexname;
  char classname[10];
  FILE *fl;
  struct char_file_u player;
  int num = 0;
  long size;

  if (!(fl = fopen(filename, "r+"))) {
    perror("error opening playerfile");
    exit(1);
  }
  fseek(fl, 0L, SEEK_END);
  size = ftell(fl);
  rewind(fl);
  if (size % sizeof(struct char_file_u)) {
    fprintf(stderr, "\aWARNING:  File size does not match structure, recompile showplay.  %d %d\n", size % sizeof(struct char_file_u),  sizeof(struct char_file_u ));
    fclose(fl);
    exit(0);
  }

  for (;;) {
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (feof(fl)) {
      fclose(fl);
      exit(1);
    }
    switch (player.class) {
    case CLASS_THIEF:
      strcpy(classname, "Th");
      break;
    case CLASS_WARRIOR:
      strcpy(classname, "Wa");
      break;
    case CLASS_MAGIC_USER:
      strcpy(classname, "Mu");
      break;
    case CLASS_CLERIC:
      strcpy(classname, "Cl");
      break;
       case CLASS_MONK:
      strcpy(classname, "Mo");
      break;
          case CLASS_DRUID:
      strcpy(classname, "Dr");
            break;
          case CLASS_RANGER:
      strcpy(classname, "Ra");
      break;
    default:
      strcpy(classname, "--");
      break;
    }

    switch (player.sex) {
    case SEX_FEMALE:
      sexname = 'F';
      break;
    case SEX_MALE:
      sexname = 'M';
      break;
    case SEX_NEUTRAL:
      sexname = 'N';
      break;
    default:
      sexname = '-';
      break;
    }

    printf("%5d. ID: %5ld (%c) [%2d %s] %-10s %30s %9dg %9db\n", ++num,
	   player.char_specials_saved.idnum, sexname, player.level,
	   classname, player.name, player.player_specials_saved.email, player.points.gold,
	   player.points.bank_gold);
  }
}


int main(int argc, char **argv)
{

printf("int:%d long:%d room:%d mob:%d obj:%d\n", sizeof(int), sizeof(long), 
sizeof(room), sizeof(mob), sizeof(obj));
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    show(argv[1]);

  return 0;
}
