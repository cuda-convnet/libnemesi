/* * 
 *  ./rtsp/close_cmd.c: $Revision: 1.2 $ -- $Date: 2002/11/07 12:12:13 $
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
#include <stdarg.h>

int close_cmd(struct RTSP_Thread *rtsp_th, ...)
{

	if(rtsp_th->status == INIT){
		rtsp_th->busy=0;
		uiprintf(BLANK_LINE);
		uiprintf("No Connection to close\n");
		return 1;
	}
	get_curr_sess(NULL, NULL, NULL);
	get_curr_sess(rtsp_th, NULL, NULL);
	if (send_teardown_request(rtsp_th))
		return 1;

	return 0;
}