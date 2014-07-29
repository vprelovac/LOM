struct auction_data {
    struct char_data *seller;
    struct char_data *bidder;
    struct obj_data *obj;
    long bid;
    long spec;
    int ticks;
    struct auction_data *next;
};

struct char_data *get_ch_by_id(long idnum);
struct auction_data auction;

extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
struct descriptor_data *d;

#define AUC_NONE	-1          
#define AUC_NEW		0   
#define AUC_BID		1   
#define AUC_ONCE	2           
#define AUC_TWICE	3           
#define AUC_SOLD	4           

#define PULSE_AUCTION	(12 RL_SEC)                                   


/* Change the line below to whatever you want to use to
 * output the auction channel to the mud, or simply
 * replace the AUC_OUT()'s in the code with the code
 */

/*#define AUC_OUT(txt)	sprintf(buf, "[Auction]: %s\r\n", txt);send_to_all(buf)*/

/* Example of descriptor dependant output for AUC_OUT()
a  Just add the #, uncomment these lines, and remove the other AUC_OUT to use.*/

#define AUC_OUT(txt) 						\
  sprintf(buf, "\r\n&M[Auction]: &c%s&0\r\n", txt);				\
  for (d = descriptor_list; d; d = d->next)			\
   if (!d->connected && d->character &&				\
       !PLR_FLAGGED(d->character, PLR_WRITING) &&		\
       !PRF_FLAGGED(d->character, PRF_NOAUCT) &&		\
       !ROOM_FLAGGED(d->character->in_room, ROOM_SOUNDPROOF))	\
     send_to_char(buf, d->character)

#define INFO_OUT(buf) 						\
  for (d = descriptor_list; d; d = d->next)			\
   if (!d->connected && d->character &&		\
       !PLR_FLAGGED(d->character, PLR_WRITING) &&		\
       !PRF2_FLAGGED(d->character, PRF2_NOINFO) &&		\
       !ROOM_FLAGGED(d->character->in_room, ROOM_SOUNDPROOF))	\
     send_to_char(buf, d->character)

#define GRATS_OUT(buf,ch) 						\
  for (d = descriptor_list; d; d = d->next)			\
   if (!d->connected && d->character &&				\
       PRF2_FLAGGED(d->character, PRF2_AUTOGRAT) &&		\
       !ROOM_FLAGGED(d->character->in_room, ROOM_SOUNDPROOF) && \
       d->character->player_specials->saved.grats)	\
     { \
     sprintf(buf,"[Grats from %s]: &c%s&0\r\n",GET_NAME(d->character),d->character->player_specials->saved.grats); \
     if (ch!=(d->character)) send_to_char(buf, ch); \
     sprintf(buf,"You congratulate %s.\r\n",GET_NAME(ch)); \
     if (ch!=(d->character)) send_to_char(buf, d->character); }


