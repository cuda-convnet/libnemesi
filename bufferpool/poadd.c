/* * 
 *  ./bufferpool/poadd.c: $Revision: 1.2 $ -- $Date: 2002/11/07 12:12:06 $
 *  
 *  This file is part of NeMeSI
 *
 *  NeMeSI -- NEtwork MEdia Streamer I
 *
 *  Copyright (C) 2001 by
 *  	
 *  	Giampaolo "mancho" Mancini - manchoz@inwind.it
 *	Francesco "shawill" Varano - shawill@infinto.it
 *
 *  NeMeSI is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NeMeSI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NeMeSI; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * */

#include <nemesi/rtp.h>
#include <nemesi/bufferpool.h>

/**
* Inserisce un elemento nel Buffer di Playout.
* L'inserimento e' ordinato secondo il numero di sequenza del pacchetto RTP.
* @param po Il Buffer Pool corrente.
* @param index L'indice dello slot allocato dalla poget.
* @return 0
* @see poget
* @see bufferpool.h
* */
int poadd(playout_buff * po, int index, uint32 cycles)
{
	int i;
	uint32 cseq;

	i = po->pohead;
	cseq = (uint32)ntohs(((rtp_pkt *) (*(po->bufferpool) + index))->seq) + cycles;
	
	while ((i != -1) && ((uint32)ntohs(((rtp_pkt *) (*(po->bufferpool) + i))->seq) + po->cycles > cseq)) {
		i = po->pobuff[i].next;
	}
	if ( (i != -1) && (cseq == ((uint32)ntohs(((rtp_pkt *) (*(po->bufferpool) + i))->seq) + po->cycles)) )
		return 1;
	if (i == po->pohead) {	/* inserimento in testa */
		po->pobuff[index].next = i;
		po->pohead = index;
		if( i == -1 )
			po->potail=index;
		else
			po->pobuff[i].prev = index;
		po->pobuff[index].prev = -1;
		po->cycles=cycles;
	} else {		/* inserimento */
		po->pobuff[index].next = po->pobuff[i].next;
		po->pobuff[i].next = index;
		po->pobuff[index].prev = i;
		if (i == po->potail)	/* inserimento in coda */
			po->potail = index;
		else
			po->pobuff[po->pobuff[index].next].prev = index;
		return 2;
	}
	return 0;
}