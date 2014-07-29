/***************************************************************************
 *  File:    showmail.c                                                    *
 *  Author:  Peter Ajamian                                                 *
 *  Usage:   List a Circle mailfile.  Will also show the first two blocks  *
 *           of deleted MudMails.                                          *
 ***************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "mail.h"
#include "utils.h"

char ** playerlist = 0;
long playertop = 0;

void freeallmem(void)
{
  long playernum;

  if(!playerlist)
    return;

  for(playernum = 0; playernum < playertop; playernum++)
    if (playerlist[playernum])
      free(playerlist[playernum]);
  free(playerlist);

  playertop = 0;
}

#define GET_CHAR_NAME(id) ((id) <= playertop && (id) > 0 && playerlist && \
                           playerlist[(id) - 1] ? playerlist[(id) - 1] : \
                           "Unknown" )

void init_players(char * filename)
{
  long playernum;
  struct char_file_u char_file_data;
  FILE * playerfile = 0;

  if (playertop)
    freeallmem();

  if (!playerfile)
    if (!(playerfile = fopen(filename, "r+"))) {
      perror("error opening player file");
      exit(EXIT_FAILURE);
    }

  for(;;) {
    fread(&char_file_data, sizeof(struct char_file_u), 1, playerfile);
    if (feof(playerfile))
      break;
    if (playertop < char_file_data.char_specials_saved.idnum)
      playertop = char_file_data.char_specials_saved.idnum;
  }

  if (!playertop) {
    fclose(playerfile);
    return;
  }

  rewind(playerfile);
  CREATE(playerlist, char*, playertop);
  for(playernum = 0; playernum < playertop; playernum++)
    playerlist[playernum] = 0;

  for(;;) {
    fread(&char_file_data, sizeof(struct char_file_u), 1, playerfile);
    if (feof(playerfile))
      break;
    if (char_file_data.char_specials_saved.idnum <= 0)
      continue;
    if (strlen(char_file_data.name) > MAX_NAME_LENGTH) {
      fprintf(stderr, "ERROR:  Invalid name in player file for player ID "
	      "number %ld.\n", char_file_data.char_specials_saved.idnum);
      continue;
    }
    CREATE(playerlist[char_file_data.char_specials_saved.idnum - 1], char,
	   strlen(char_file_data.name) + 1);
    strcpy(playerlist[char_file_data.char_specials_saved.idnum - 1],
	   char_file_data.name);
  }

  fclose(playerfile);
}

char * makelower(char * src, char * dest)
{
  char * destptr = 0;

  if (!dest)
    dest = src;

  for (destptr = dest; *src; src++, destptr++)
    *destptr = tolower(*src);
  *destptr = 0;

  return dest;
}

void getblock(FILE * fp, void * block, long index)
{
  if (fseek(fp, index * BLOCK_SIZE, SEEK_SET)) {
    perror("Error in file seek.");
    fclose(fp);
    exit(EXIT_FAILURE);
  }
  fread(block, BLOCK_SIZE, 1, fp);
}

void show(int argc, char * argv[])
{
  FILE *fl;
  long headnum = 0;
  long datanum = 0;
  long numblocks = 0;
  header_block_type headblock;
  data_block_type datablock;
  char buf[MAX_STRING_LENGTH];
  char * filename = 0;
  char * playername = 0;
  char p_name[MAX_NAME_LENGTH + 1];

  filename = argv[0];

  if (argc >= 4) {
    if (strlen(argv[1]) > MAX_NAME_LENGTH) {
      perror("Player name in argument three is too long.\n");
      exit(EXIT_FAILURE);
    }
    playername = makelower(argv[1], p_name);
  }
  
  if (!(fl = fopen(filename, "r+"))) {
    perror("error opening mail file");
    exit(EXIT_FAILURE);
  }

  for (;;) {
    fread(&headblock, BLOCK_SIZE, 1, fl);
    if (feof(fl))
      break;
    numblocks++;
  }

  for (headnum = 0;;headnum++) {
    getblock(fl, &headblock, headnum);
    if (feof(fl)) {
      fclose(fl);
      exit(EXIT_SUCCESS);
    }

    if (headblock.block_type == DELETED_BLOCK) {
      /* Try to determine if this is a header block by considering any invalid
	 field to be a data block. */
      if ((headblock.header_data.next_block       %        100 ) ||
	  (headblock.header_data.next_block       <         -2 ) ||
	  (headblock.header_data.next_block / 100 >= numblocks ) ||
	  (headblock.header_data.to               <=         0 ) ||
	  (headblock.header_data.from             <=         0 ) ||
	  (headblock.header_data.to               >  playertop ) ||
	  (headblock.header_data.from             >  playertop ) ||
	  (headblock.header_data.mail_time        >  time(NULL)) )
	continue;
    }

    if (headblock.block_type >= 0 || headblock.block_type == LAST_BLOCK)
      continue;

    if (headblock.block_type != HEADER_BLOCK &&
	headblock.block_type != DELETED_BLOCK) {
      fprintf(stderr, "\n\nERROR:  Invalid block_type for block number %ld",
	      headnum);
      continue;
    }

    if (playername &&
	strcmp(playername,
	       makelower(GET_CHAR_NAME(headblock.header_data.to), buf)) &&
	strcmp(playername,
	       makelower(GET_CHAR_NAME(headblock.header_data.from), buf)))
      continue;

    if (headblock.block_type == HEADER_BLOCK)
      printf("\n\n"
	     "***************************** Unread  Message "
	     "*****************************\n\n");
    else
      printf("\n\n"
	     "***************************** Deleted Message "
	     "*****************************\n\n");

    strftime(buf, MAX_STRING_LENGTH, "%A %B %d, %Y  %I:%M:%S %p",
	     localtime(&headblock.header_data.mail_time));

    printf  ("From:  %s[%ld]\n"
	     "To:    %s[%ld]\n"
	     "Date:  %s\n\n"
	     "**********************************************"
	     "*****************************\n%s",
	     GET_CHAR_NAME(headblock.header_data.from),
	     headblock.header_data.from,
	     GET_CHAR_NAME(headblock.header_data.to),
	     headblock.header_data.to, buf,
	     headblock.txt);

    for(datanum = headblock.header_data.next_block;
	datanum != LAST_BLOCK && datanum != DELETED_BLOCK;
	datanum = datablock.block_type) {
      if (datanum % 100 || datanum < 0 || datanum / 100 >= numblocks) {
	fprintf(stderr, "\nERROR:  Invalid block_type.\n\n");
	break;	       
      }

      getblock(fl, &datablock, datanum / 100);

      printf("%s", datablock.txt);
    }
    printf("\n"
	   "**********************************************"
	   "*****************************\n");
  }
}


int main(int argc, char **argv)
{
  if (argc < 3) {
    printf("Usage: %s <playerfile> <mailfile> [<playername>]\n"
	   "\n"
	   "       <playerfile> Player database file.\n"
           "       <mailfile>   Mudmail file.\n"
	   "       <playername> Limit displayed messages to those to or from\n"
	   "                    this player.\n"
	   "\n",
	   argv[0]);
    return 0;
  }
  init_players(argv[1]);
  show(argc, &argv[2]);

  freeallmem();
  return 0;
}
