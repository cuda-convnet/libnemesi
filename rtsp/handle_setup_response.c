/* * 
 *  ./rtsp/handle_setup_response.c: $Revision: 1.2 $ -- $Date: 2002/11/07 12:12:13 $
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

#include <nemesi/rtsp.h>
#include <nemesi/methods.h>

int handle_setup_response(struct RTSP_Thread *rtsp_th)
{
	char *tkn;		/* contiene una riga di descrizione */
	char *prev_tkn;		/* punta prima al token precedente per il controllo sulla fine dell'header
				   e poi ai vari componenti della riga di comando */
	
	struct RTSP_Session *rtsp_sess;
	struct RTSP_Medium *rtsp_med;

	if (get_curr_sess(NULL, &rtsp_sess, &rtsp_med))
		return 1;

	if ((prev_tkn = strtok((rtsp_th->in_buffer).data, "\n")) == NULL) {
		uiprintf("Invalid RTSP-DESCRIBE response\n");
		rtsp_th->busy=0;
		return 1;
	}
	if ( check_status(prev_tkn, rtsp_th) ){
		remove_pkt(rtsp_th);
		rtsp_th->busy=0;
		return 1;
	}
	while (((tkn = strtok(NULL, "\n")) != NULL) && ((tkn - prev_tkn) > 1)) {
		if (((tkn - prev_tkn) == 2) && (*prev_tkn == '\r'))
			break;
		prev_tkn = tkn;

		if (!strncmpcase(prev_tkn, "Transport", 9)) {
			prev_tkn += 9;
			get_transport_str(rtsp_med->rtp_sess, prev_tkn);
		}
		if (!strncmpcase(prev_tkn, "Session", 7)) {
			prev_tkn += 7;
			sscanf(prev_tkn, " : %llu ; ", &(rtsp_sess->Session_ID));
		}
	}
	while ( (tkn!=NULL) && ((*tkn=='\r') || (*tkn=='\n') || (*tkn=='\0')) )
		tkn=strtok(NULL, "\n"); /* cerco l'inizio del body o, eventualmente, del prossimo pkt */
	if ( tkn!=NULL )
		tkn[strlen(tkn)]='\n'; /* rimetto a posto il \n modificato dalla strtok */

	remove_pkt(rtsp_th);
	memset(rtsp_th->waiting_for, 0, strlen(rtsp_th->waiting_for));
	return 0;
}