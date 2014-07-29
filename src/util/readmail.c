/* ************************************************************************
*  file:  readmail.c                                  Part of CircleMud   *
*  Usage: read mail in a player mail file without removing it from file   *
*  Written by Jeremy Elson                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Copyright (C) 1993 The Trustees of The Johns Hopkins University        *
**************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"
#include "../db.h"
#include "../mail.h"

#define log(x) puts(x)


/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

#ifdef ENOENT
#undef ENOENT
#endif

#define ENOENT 1

struct player_index_element *player_table = NULL;	/* index to plr file	 */
FILE *player_fl = NULL;		/* file desc of player file	 */
int top_of_p_table = 0;		/* ref to top of table		 */
int top_of_p_file = 0;		/* ref of size of p file	 */
long top_idnum = 0;		/* highest idnum in use		 */
char buf[MAX_STRING_LENGTH];

mail_index_type		*mail_index = 0; /* list of recs in the mail file  */
position_list_type 	*free_list = 0;  /* list of free positions in file */
long	file_end_pos = 0; /* length of file */
//int errno = 0;

int MAX(int a, int b)
{
  return a > b ? a : b;
}

/* generate index table for the player file */
void build_player_index(void)
{
  int nr = -1, i;
  long size, recs;
  struct char_file_u dummy;

  if (!(player_fl = fopen("../lib/etc/players", "r+b"))) {
    if (errno != ENOENT) {
      perror("fatal error opening playerfile");
      exit(1);
    } else {
      if (!(player_fl = fopen("../lib/etc/players", "r+b"))) {
	perror("fatal error opening playerfile");
	exit(1);
      }
    }
  }

  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "\aWARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs) {
    sprintf(buf, "   %ld players in database.", recs);
    log(buf);
    CREATE(player_table, struct player_index_element, recs);
  } else {
    player_table = NULL;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  for (; !feof(player_fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
    if (!feof(player_fl)) {	/* new record */
		nr++;
      CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
      for (i = 0;
	   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
		player_table[nr].id = dummy.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);
    }
  }

  top_of_p_file = top_of_p_table = nr;
}

void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}

long get_id_by_name(char *name)
{
  int i;

skip_spaces(&name);
  for (i = 0; i <= top_of_p_table; i++)
    if (!strcmp((player_table + i)->name, name))
      return ((player_table + i)->id);

  return -1;
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if ((player_table + i)->id == id)
      return ((player_table + i)->name);

  return NULL;
}


void	push_free_list(long pos)
{
   position_list_type * new_pos;

   CREATE(new_pos, position_list_type, 1);
   new_pos->position = pos;
   new_pos->next = free_list;
   free_list = new_pos;
}



long	pop_free_list(void)
{
   position_list_type * old_pos;
   long	return_value;

   if ((old_pos = free_list) != 0) {
      return_value = free_list->position;
      free_list = old_pos->next;
      free(old_pos);
      return return_value;
   } else
      return file_end_pos;
}




mail_index_type *find_char_in_index(long searchee)
{
   mail_index_type * tmp;

   if (searchee < 0) {
      log("Mail system -- non fatal error #1");
      return 0;
   }
  for (tmp = mail_index; (tmp && tmp->recipient != searchee); tmp = tmp->next);

   return tmp;
}





void	read_from_file(void *buf, int size, long filepos)
{
   FILE * mail_file;

   if (!(mail_file = fopen("../lib/etc/plrmail", "r"))) {
      perror("Error opening mail file for read");
      exit(1);
   }


   if (filepos % BLOCK_SIZE) {
      log("Mail system -- fatal error #2!!!");
      return;
   }

   fseek(mail_file, filepos, SEEK_SET);
   fread(buf, size, 1, mail_file);
   fclose(mail_file);
   return;
}




void	index_mail(long id_to_index, long pos)
{
   mail_index_type     * new_index;
   position_list_type * new_position;

   if (id_to_index < 0) {
      log("Mail system -- non-fatal error 4");
      return;
   }

   if (!(new_index = find_char_in_index(id_to_index))) {
      /* name not already in index.. add it */
      CREATE(new_index, mail_index_type, 1);
      new_index->recipient = id_to_index;
      new_index->list_start = NULL;

      /* add to front of list */
      new_index->next = mail_index;
      mail_index = new_index;
   }

   /* now, add this position to front of position list */
   CREATE(new_position, position_list_type, 1);
   new_position->position = pos;
   new_position->next = new_index->list_start;
   new_index->list_start = new_position;
}


/* SCAN_FILE */
/* scan_file is called once during boot-up.  It scans through the mail file
   and indexes all entries currently in the mail file. */
int	scan_file(void)
{
   FILE 		   * mail_file;
   header_block_type  next_block;
   int	total_messages = 0, block_num = 0;
   char	buf[100];

   if (!(mail_file = fopen("../lib/etc/plrmail", "r"))) {
      perror("Error opening mail file for read");
      exit(0);
   }

   while (fread(&next_block, sizeof(header_block_type), 1, mail_file)) {
      if (next_block.block_type == HEADER_BLOCK) {
	 index_mail(next_block.header_data.to, block_num * BLOCK_SIZE);
	 total_messages++;
      } else if (next_block.block_type == DELETED_BLOCK)
	 push_free_list(block_num * BLOCK_SIZE);
      block_num++;
   }

   file_end_pos = ftell(mail_file);
   fclose(mail_file);
   sprintf(buf, "   %ld bytes read.", file_end_pos);
   log(buf);
   if (file_end_pos % BLOCK_SIZE) {
      log("Error booting mail system -- Mail file corrupt!");
      log("Mail disabled!");
      return 0;
   }
   sprintf(buf, "   Mail file read -- %d messages.", total_messages);
   log(buf);
   return 1;
} /* end of scan_file */


/* HAS_MAIL */
/* a simple little function which tells you if the guy has mail or not */
int	has_mail(long recipient)
{
   if (find_char_in_index(recipient))
      return 1;
   return 0;
}





/* READ_DELETE */
/* read_delete takes 1 char pointer to the name of the person whose mail
you're retrieving.  It returns to you a char pointer to the message text.
The mail is then discarded from the file and the mail index. */

char	*read_delete(long recipient)
/* recipient is the name as it appears in the index.
   recipient_formatted is the name as it should appear on the mail
   header (i.e. the text handed to the player) */
{
   header_block_type header;
   data_block_type data;
   mail_index_type *mail_pointer, *prev_mail;
   position_list_type *position_pointer;
   long	mail_address, following_block;
   char	*message, *tmstr, buf[200];
   size_t string_size;

   if (recipient < 0) {
      log("Mail system -- non fatal error 6");
      return 0;
   }
   if (!(mail_pointer = find_char_in_index(recipient))) {
      log("Stupid post-office-spec_proc-error 7");
      return 0;
   }
   if (!(position_pointer = mail_pointer->list_start)) {
      log("Mail system -- non fatal error 8");
      return 0;
   }

   if (!(position_pointer->next)) /* just 1 entry in list. */ {
      mail_address = position_pointer->position;
      free(position_pointer);

      /* now free up the actual name entry */
      if (mail_index == mail_pointer) /* name is 1st in list */ {
	 mail_index = mail_pointer->next;
	 free(mail_pointer);
      } else {
	 /* find entry before the one we're going to del */
	 for (prev_mail = mail_index; 
	     prev_mail->next != mail_pointer; 
	     prev_mail = prev_mail->next);
	 prev_mail->next = mail_pointer->next;
	 free(mail_pointer);
      }
   } else {
      /* move to next-to-last record */
      while (position_pointer->next->next)
	 position_pointer = position_pointer->next;
      mail_address = position_pointer->next->position;
      free(position_pointer->next);
      position_pointer->next = 0;
   }

   /* ok, now lets do some readin'! */
   read_from_file(&header, BLOCK_SIZE, mail_address);

   if (header.block_type != HEADER_BLOCK) {
      log("Oh dear.");
      log("Mail system disabled! -- error 9");
      return 0;
   }

   tmstr = asctime(localtime(&header.header_data.mail_time));
   *(tmstr + strlen(tmstr) - 1) = '\0';

   sprintf(buf, " * * * * Axxenfall Mail System * * * *\n\r"
       "Date: %s\n\r"
       "  To: %s\n\r"
       "From: %s\n\r\n\r", tmstr, get_name_by_id(recipient), 
	   get_name_by_id(header.header_data.from));

   string_size = (sizeof(char) * (strlen(buf) + strlen(header.txt) + 1));
   CREATE(message, char, string_size);
   strcpy(message, buf);
   message[strlen(buf)] = '\0';
   strcat(message, header.txt);
   message[string_size - 1] = '\0';
   following_block = header.header_data.next_block;


   while (following_block != LAST_BLOCK) {
      read_from_file(&data, BLOCK_SIZE, following_block);

      string_size = (sizeof(char) * (strlen(message) + strlen(data.txt) + 1));
      RECREATE(message, char, string_size);
      strcat(message, data.txt);
      message[string_size - 1] = '\0';
      mail_address = following_block;
      following_block = data.block_type;
   }

   return message;
}



int	main(int argc, char * argv[])
{
   long searchee;
   char *ptr;

   if (argc != 2) {
      fprintf(stderr, "Usage: %s <plrname>\n", argv[0]);
      exit(1);
   }

   build_player_index();
   scan_file();
   searchee = get_id_by_name(argv[1]);
   if (!(searchee > 0))
     exit(0);
   if (!has_mail(searchee)) {
      printf("The person you were looking for does not have any mail in the mail file.\n"/*, searchee*/);
      exit(1);
   }

   while (has_mail(searchee)) {
      ptr = read_delete(searchee);
      printf("%s\n\n-----------------\n\n", ptr);
      free(ptr);
   }
   exit(0);
}


