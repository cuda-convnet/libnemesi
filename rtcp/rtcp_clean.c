/* * 
 *  ./rtcp/rtcp_clean.c: $Revision: 1.2 $ -- $Date: 2002/11/07 12:12:13 $
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

#include <nemesi/rtcp.h>

void rtcp_clean(void *args)
{
	struct RTP_Session *rtp_sess_head=((struct RTP_Session *)args);
	struct RTP_Session *rtp_sess;
	struct Stream_Source *stm_src;

	for (rtp_sess=rtp_sess_head; rtp_sess; rtp_sess=rtp_sess->next)
		for(stm_src=rtp_sess->ssrc_queue; stm_src; stm_src=stm_src->next)
			if( stm_src->rtcptofd > 0)
				close(stm_src->rtcptofd);
	
	uiprintf("RTCP Thread R.I.P.\n");
}