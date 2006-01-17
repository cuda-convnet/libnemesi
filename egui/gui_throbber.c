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

#include <stdlib.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "support.h"
#include "gui_throbber.h"
#include <nemesi/comm.h> // se ne va, eh?

// static GtkProgress *progress;

static GNMSThrobber *throbber;
static gint progress_timeout(gpointer data);
static void destroy_throbber(void);

int create_throbber(GtkWidget *box)
{
	GNMSThrobber *new_throbber;
	unsigned int i;
	char filename[127];

	if ((new_throbber=malloc(sizeof(GNMSThrobber))) == NULL)
		return nmsprintf(NMSML_FATAL, "Could not alloc throbber structure\n");

	new_throbber->num_anim = 9; // TODO: automatizzare
	if ((new_throbber->anim=malloc(new_throbber->num_anim * sizeof(GtkWidget *))) == NULL)
		return nmsprintf(NMSML_FATAL, "Could not alloc throbber animation vector\n");

	new_throbber->rest = GTK_IMAGE(create_pixmap(NULL, "rest.png"));
	gtk_box_pack_end (GTK_BOX (box), GTK_WIDGET(new_throbber->rest), FALSE, FALSE, 0);
	for (i=0;i<new_throbber->num_anim;i++) {
		sprintf(filename, "%03d.png", i+1);
		new_throbber->anim[i] = GTK_IMAGE(create_pixmap(NULL, filename));
		gtk_box_pack_end (GTK_BOX (box), GTK_WIDGET(new_throbber->anim[i]), FALSE, FALSE, 0);
	}

	gtk_widget_show(GTK_WIDGET(new_throbber->rest));
	new_throbber->shown = -1;

	throbber = new_throbber;

	return 0;
}

void gui_throbber(void *arg)
{
	uint8 *busy = (uint8 *)arg;
	static gint timer=-1;

	update_toolbar();
	if (timer > 0)
		g_source_remove(timer);
	timer = g_timeout_add(55, progress_timeout, (gpointer *) busy);
}

static gint progress_timeout(gpointer data)
{
	uint8 *busy = data;

	if (*busy) {
		if (throbber->shown >= 0)
			gtk_widget_hide(GTK_WIDGET(throbber->anim[throbber->shown]));
		else
			gtk_widget_hide(GTK_WIDGET(throbber->rest));
		throbber->shown = (throbber->shown + 1 ) % throbber->num_anim;
		gtk_widget_show(GTK_WIDGET(throbber->anim[throbber->shown]));
	}else {
		if (throbber->shown >= 0) {
			gtk_widget_hide(GTK_WIDGET(throbber->anim[throbber->shown]));
			gtk_widget_show(GTK_WIDGET(throbber->rest));
			throbber->shown = -1;
		}
		update_toolbar();
		nmsprintf(NMSML_DBG3, "[gui] throbber done\n");
		return FALSE;
	}
	return TRUE;
}

static void destroy_throbber(void)
{
	unsigned int i;

	gtk_widget_destroy(GTK_WIDGET(throbber->rest));
	for(i=0;i<throbber->num_anim;i++)
		gtk_widget_destroy(GTK_WIDGET(throbber->anim[i]));

	free(throbber->anim);
	free(throbber);
}

