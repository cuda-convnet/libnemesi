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
 *  This file is largely and freely inspired by audio_out.h from MPlayer project.
 *  
 * */

#ifndef __AUDIO_DRIVERS_H
#define __AUDIO_DRIVERS_H

#include <config.h>
#include <nemesi/types.h>

#define ACTRL_GET_SYSBUF 0

typedef struct {
        /* driver name */
        const char *name;
        /* short name (for config strings) (e.g.:"sdl") */
        const char *short_name;
        /* author ("Author name & surname <mail>") */
        const char *author;
        /* any additional comments */
        const char *comment;
} NMSADrvInfo;

typedef struct {
	NMSADrvInfo *info;
	/*
	 * Preinitializes driver (real INITIALIZATION)
	 *   returns: zero on successful initialization, non-zero on error.
	 */
	uint32 (*preinit)(const char *arg);
        /*
         * Initialize (means CONFIGURE) the audio driver.
	 * params:
         * 	rate:
	 * 	channels: number of channels
	 * 	format:
         * returns : zero on successful initialization, non-zero on error.
         */
        uint32 (*config)(uint32 rate, uint8 channels, uint32 format, uint32 flags);
	/*
	 * Control interface
	 * params:
	 *	cmd: commnand to exec
	 *	arg: argument for command
	 */
	 uint32 (*control)(uint32 cmd, void *arg);
	/*
	 * allocs buffer for new decoded data.
	 * params:
	 *	len: length of requested buffer
	 * returns pointer to new buffer
	 */
	uint8 *(*get_buff)(uint32 len);
	/*
	 * This function must be called by decoder after the decoded data has
	 * been written on the allocated buffer
	 * params:
	 * 	data: buffer to play.
	 *	len: length of buffer
	 * retuns 1 on error, 0 otherwise
	 */
	uint32 (*play_buff)(uint8 *data, uint32 len);
        /*
         * Pauses the driver
         */
        void (*audio_pause)(void);
        /*
         * Resume playing
         */
        void (*audio_resume)(void);
	/*
	 * Reset driver.
	 */
	 void (*reset)(void);
        /*
         * Closes driver. Should restore the original state of the system.
         */
        void (*uninit)(void);
} NMSAFunctions;

extern NMSAFunctions nms_audio_oss;
#if HAVE_SDL
extern NMSAFunctions nms_audio_sdl;
#endif

/*
char *ao_format_name(int format);
int ao_init(void);

ao_functions_t* init_best_audio_out(char** ao_list);
void list_audio_out();

// NULL terminated array of all drivers
extern ao_functions_t* audio_out_drivers[];
 */

#endif // __AUDIO_DRIVERS_H
