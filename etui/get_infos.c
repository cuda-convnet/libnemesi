/* * 
 *  ./etui/get_infos.c: $Revision: 1.2 $ -- $Date: 2002/11/07 12:12:09 $
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

#include <nemesi/etui.h>

int get_infos(struct RTSP_args *rtsp_args)
{
	struct RTSP_Session *sess;
	struct RTSP_Medium *med;
	char **str;
	struct attr *attr;

	char *sdes[13]={ "Protocol Version",
		"Session Identifier & Creator",
		"Session Name",
		"Session Info",
		"URI Description",
		"e-mail Address",
		"Phone Number",
		"Connection Info",
		"Bandwidth Info",
		"Active Time",
		"I' so' llu re",
		"Time Zone",
		"Crypto Key"
	};
	char *mdes[5]={ "Multimedia Type & Transport Address",
		"Medium Title",
		"Connection Info",
		"Bandwidth Info",
		"Crypto Key"
	};
	
	sess=rtsp_args->rtsp_th->rtsp_queue;
	
	uiprintf(BLANK_LINE);

	if (!sess){
		uiprintf("No Connection!\n\n");
		return 0;
	}

	while(sess){
		med=sess->media_queue;
		uiprintf("---- RTSP Session Infos: %s ----\n", sess->pathname);
		for(str=(char **)&(sess->info); str < (char **)&(sess->info.attr_list); str++)
			if (*str)
				uiprintf("* %s: %s\n", sdes[str-(char **)&(sess->info)], *str);
		for(attr=sess->info.attr_list; attr; attr=attr->next)
			uiprintf("%s\n", attr->a);
		while (med) {
			uiprintf("\n\t---- RTSP Medium Infos: %s ----\n", med->filename);
			for(str=(char **)&(med->medium_info); str < (char **)&(med->medium_info.attr_list); str++)
				if(*str)
					uiprintf("\t* %s: %s\n", mdes[str-(char **)&(med->medium_info)], *str);
			for(attr=med->medium_info.attr_list; attr; attr=attr->next)
				uiprintf("\t* %s\n", attr->a);
			med=med->next;
		}
		sess=sess->next;
	}
	uiprintf("\n");

	return 0;
}