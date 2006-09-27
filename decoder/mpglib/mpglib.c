/* * 
 *  $Id$
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

#include <unistd.h>
#include <string.h>

#include <nemesi/types.h>
#include <nemesi/output.h>

#include "common.h"
#include "interface.h"

/*#include <nemesi/comm.h>*/

/* char buf[16384]; */
/* struct mpstr mp; */

#include <nemesi/plugin.h>

plugin_init("MPA", 14)
#define BUFFER 8192
int get_plugin_pt(void)
{
	return 96;
}

static int decode(char *data, int len, nms_output * outc)
{
	nms_au_fnc *funcs = outc->audio->functions;

	static PMPSTR mp = NULL;

	int size = 0;
	char out[BUFFER];
	int ret = 0;
	uint8 *audio_data;

	if (!mp) {
		if ((mp = (PMPSTR) malloc(sizeof(MPSTR))) == NULL) {
			fprintf(stderr, "Cannot allocate memory!\n");
			return 1;
		}
		InitMP3(mp);
	}

	ret = decodeMP3(mp, data + 4, len - 4, out, BUFFER, &size);
#if 0
	while (ret == MP3_OK) {
/*		write(audio_fd, out, size); */
		audio_data = (*ab_get) ((uint32) size);
		memcpy(audio_data, out, size);
		ret = decodeMP3(mp, data + 4, 0, out, BUFFER, &size);
		fprintf(stderr, "\rret = %d\n", ret);
	}
#else
	if (ret == MP3_OK) {
		audio_data = funcs->get_buff((uint32) size);
		memcpy(audio_data, out, size);
		funcs->play_buff(audio_data, (uint32) size);
	} else {
		fprintf(stderr, "\ndecoded %d bytes with %d status\n", size,
			ret);
	}
#endif

	return 0;
}
