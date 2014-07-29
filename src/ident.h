/* ************************************************************************
*  File: ident.h                              				  *
*                                                                         *
*  Usage: Header file containing stuff required for rfc 931/1413 ident    *
*         lookups                                                         *
*                                                                         *
*  Written by Eric Green (egreen@cypronet.com)				  *
************************************************************************ */

void ident_start(struct descriptor_data *d, long addr);
void ident_check(struct descriptor_data *d, int pulse);
int waiting_for_ident(struct descriptor_data *d);

extern int ident;
