/* * 
 *  $Id$
 *  
 *  This file is part of NeMeSI
 *
 *  NeMeSI -- NEtwork MEdia Streamer I
 *
 *  Copyright (C) 2001 by
 *  	
 *  	Giampaolo "mancho" Mancini - giampaolo.mancini@polito.it
 *	Francesco "shawill" Varano - francesco.varano@polito.it
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

#include <stdio.h>
#include <stdarg.h>

#include <config.h>
#include <nemesi/comm.h>

#ifndef ENABLE_DEBUG
#define DEFAULT_VERBOSITY NMSML_NORM
#else // ENABLE_DEBUG => debug enabled
#define DEFAULT_VERBOSITY NMSML_DBG1
#endif // ENABLE_DEBUG

int (*nmsprintf)(int level, const char *fmt, ...) = nmsprintf_default;
int (*nmsstatusprintf)(int cmd, const char *fmt, ...) = nmsstatusprintf_default;

static int verbosity=DEFAULT_VERBOSITY;

int nmsverbosity_set(int level){
	if (level >= 0) {
		if ( (level + NMSML_NORM) > NMSML_MAX)
			verbosity = NMSML_MAX;
		else
			verbosity = NMSML_NORM + level;
	} else {
		fprintf(stderr, NMSCLR_YELLOW"warning: verbosity level must be a non negative integer. Setting to 0\n"NMSCLR_DEFAULT);
		verbosity = 0;
	}
	return verbosity-NMSML_NORM;
}

int nmsverbosity_get(void){
	return verbosity-NMSML_NORM;
}

/*!  \brief Default print function.
 *
 * This function manages the message printing on the <em>standard error</em> or <em>standard output</em>
 * stream according with a <tt>message level</tt> parameter.
 * For each messege to print there is a level associated (the type of message) and if the
 * global verbosity is greater than this level then the message will be
 * printed.
 *
 * \param level level associated to message. In case of
 *        <tt>fmt==NULL</tt> it is used to set the global verbosity level.
 * \param fmt string containing message format (<tt>printf</tt> like).
 * \param ... variable list of arguments according with <tt>fmt</tt> description.
 * \return 1 if message type is an error, 0 otherwise.
 */
int nmsprintf_default(int level, const char *fmt, ...)
{
	// int ret=0;
	va_list args;
	FILE *out_stm = ( level <= NMSML_WARN ) ? stderr : stdout;
#ifdef NMS_COLOURED
	char *colours[NMSML_MAX+1] = {NMSML_COLOURS};
#endif // NMS_COLOURED

	/*
	if (level < 0) {
		fprintf(stderr, NMSCLR_YELLOW"warning: verbosity level must be a non negative integer. Setting to 0\n"NMSCLR_DEFAULT);
		level = 0;
	}
	*/

	if ( verbosity >= level) {
		// fprintf(stderr, "\r");
		nmscolour(out_stm, colours[level]);
		switch (level) {
			case NMSML_ERR:
				fprintf(out_stm, "Error: ");
				nmscolour(out_stm, NMSCLR_DEFAULT);
				break;
			case NMSML_WARN:
				fprintf(out_stm, "Warning: ");
				nmscolour(out_stm, NMSCLR_DEFAULT);
				break;
		}
		va_start(args, fmt);
		/*ret=*/vfprintf(out_stm, fmt, args);
		va_end(args);
		nmscolour(out_stm, NMSCLR_DEFAULT);
		fflush(out_stm);
	}

	// return ret;
	return ( level < NMSML_WARN ) ? 1 : 0;
}

/*! \brief Default buffer status print function
 *
 * This function manages the printing on standard error of the stutus of both
 * buffer fill level and audio and video elapsed time.  There are two
 * interested threads that give messages in concurrency and so this is the
 * function that manages the correct printing on the screeen.
 *
 * \param cmd kind of status that printed: currently: BUFFERS_STATUS or
 * ELAPSED_STATUS
 * \param fmt string containing message format (<tt>printf</tt> like).
 * \param ... variable list of arguments according with <tt>fmt</tt> description.
 * \return the number of characters written or negative value in case of error.
 */

int nmsstatusprintf_default(int cmd, const char *fmt, ...)
{
	static char buffers[256]="\0";
	static char elapsed[256]="\0";
	static char no_status = 0;
	int ret=0;
	va_list args;

	if ( cmd == PRINT_STATUS )
		no_status = 0;
	if ( no_status )
		return 0;
	switch( cmd ) {
		case NO_STATUS:
			no_status = 1;
			return 0;
			break;
		case BUFFERS_STATUS:
			if (verbosity>=BUFFERS_STATUS_VERBOSITY) {
				va_start(args, fmt);
				ret=vsprintf(buffers, fmt, args);
				va_end(args);
			}
			break;
		case ELAPSED_STATUS:
			if (verbosity>=ELAPSED_STATUS_VERBOSITY) {
				va_start(args, fmt);
				ret=vsprintf(elapsed, fmt, args);
				va_end(args);
			}
			break;
		default:
			return 0;
			break;
	}

	if ( (verbosity>=BUFFERS_STATUS_VERBOSITY) && (verbosity>=ELAPSED_STATUS_VERBOSITY) )
		fprintf(stderr, "\r%s - %s   ", elapsed, buffers);
	else if ( verbosity>=BUFFERS_STATUS_VERBOSITY )
		fprintf(stderr, "\r%s   ", buffers);
	else if ( verbosity>=ELAPSED_STATUS_VERBOSITY )
		fprintf(stderr, "\r%s   ", elapsed);

	return ret;
}

