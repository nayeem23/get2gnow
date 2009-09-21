/* -*- Mode: C; shift-width: 8; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * get2gnow is:
 * 	Copyright(c) 2009 Kaity G. B. <uberChick@uberChicGeekChick.Com>
 * 	Released under the terms of the RPL
 *
 * For more information or to find the latest release, visit our
 * website at: http://uberChicGeekChick.Com/?projects=get2gnow
 *
 * Writen by an uberChick, other uberChicks please meet me & others @:
 * 	http://uberChicks.Net/
 *
 * I'm also disabled. I live with a progressive neuro-muscular disease.
 * DYT1+ Early-Onset Generalized Dystonia, a type of Generalized Dystonia.
 * 	http://Dystonia-DREAMS.Org/
 *
 *
 *
 * Unless explicitly acquired and licensed from Licensor under another
 * license, the contents of this file are subject to the Reciprocal Public
 * License("RPL") Version 1.5, or subsequent versions as allowed by the RPL,
 * and You may not copy or use this file in either source code or executable
 * form, except in compliance with the terms and conditions of the RPL.
 *
 * All software distributed under the RPL is provided strictly on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND
 * LICENSOR HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, QUIET ENJOYMENT, OR NON-INFRINGEMENT. See the RPL for specific
 * language governing rights and limitations under the RPL.
 *
 * The User-Visible Attribution Notice below, when provided, must appear in each
 * user-visible display as defined in Section 6.4(d):
 * 
 * Initial art work including: design, logic, programming, and graphics are
 * Copyright(C) 2009 Kaity G. B. and released under the RPL where sapplicable.
 * All materials not covered under the terms of the RPL are all still
 * Copyright(C) 2009 Kaity G. B. and released under the terms of the
 * Creative Commons Non-Comercial, Attribution, Share-A-Like version 3.0 US license.
 * 
 * Any & all data stored by this Software created, generated and/or uploaded by any User
 * and any data gathered by the Software that connects back to the User.  All data stored
 * by this Software is Copyright(C) of the User the data is connected to.
 * Users may lisences their data under the terms of an OSI approved or Creative Commons
 * license.  Users must be allowed to select their choice of license for each piece of data
 * on an individual bases and cannot be blanketly applied to all of the Users.  The User may
 * select a default license for their data.  All of the Software's data pertaining to each
 * User must be fully accessible, exportable, and deletable to that User.
 */

/********************************************************
 *          My art, code, & programming.                *
 ********************************************************/
#ifndef __CONTROL_VIEWER_H__
#define __CONTROL_VIEWER_H__

#define _GNU_SOURCE
#define _THREAD_SAFE


/**********************************************************************
 *        System & library headers, eg #include <gdk/gdkkeysyms.h>    *
 **********************************************************************/
#include <glib.h>
#include <gtk/gtk.h>
#include <libsexy/sexy.h>
#include "label.h"


/*********************************************************************
 *        Objects, structures, and etc typedefs                      *
 *********************************************************************/
/* My, Kaity G. B., new uber control-panel. */
G_BEGIN_DECLS

typedef struct _ControlPanel ControlPanel;

/********************************************************
 *          Global method  & function prototypes        *
 ********************************************************/
ControlPanel *control_panel_new(GtkWindow *parent);
GtkWindow *control_panel_get_window(void);
GtkHBox *control_panel_get_embed(void);

#define	tweets_beep	control_panel_beep
void control_panel_beep(void);

void control_panel_view_selected_update(OnlineService *service, const gdouble id, const gdouble user_id, const gchar *user_name, const gchar *user_nick, const gchar *date, const gchar *sexy_tweet, const gchar *text_tweet, GdkPixbuf *pixbuf );

void control_panel_best_friends_start_dm( OnlineService *service, const gchar *user_name );

void control_panel_dm_data_fill(GList *followers);

void contol_panel_emulate_embed_toggle(void);
void contol_panel_emulate_compact_view_toggle(void);
void control_panel_compact_view_toggled(GtkToggleButton *compact_toggle_button);
void control_panel_set_embed_toggle_and_image(void);

void control_panel_sexy_select(void);

void control_panel_sexy_prefix_char(const char c);
void control_panel_sexy_prefix_string(const gchar *str);
void control_panel_sexy_set(gchar *tweet);
void control_panel_sexy_insert_char(const char c);
void control_panel_sexy_insert_string(const gchar *str);
void control_panel_sexy_append_char(const char c);
void control_panel_sexy_append_string(const gchar *str);
gint control_panel_sexy_puts(const gchar *str, gint position);

void control_panel_new_update(void);
void control_panel_new_dm(void);
void control_panel_reply(void);

void control_panel_hide_previous_tweets(void);
void control_panel_show_previous_tweets(void);

void control_panel_send(GtkWidget *activated_widget);
void control_panel_sexy_send_dm(void);

G_END_DECLS
#endif /* __CONTROL_VIEWER_H__ */
/********************************************************
 *                       eof                            *
 ********************************************************/