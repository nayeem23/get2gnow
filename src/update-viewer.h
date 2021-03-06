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
#ifndef __UPDATE_VIEWER_H__
#define __UPDATE_VIEWER_H__

#define _GNU_SOURCE
#define _THREAD_SAFE


/**********************************************************************
 *        System & library headers, eg #include <gdk/gdkkeysyms.h>    *
 **********************************************************************/
#include <glib.h>
#include <gtk/gtk.h>


/*********************************************************************
 *        Objects, structures, and etc typedefs                      *
 *********************************************************************/
/* My, Kaity G. B., new uber update-viewer. */
G_BEGIN_DECLS

typedef struct _UpdateViewer UpdateViewer;

/********************************************************
 *          Global method  & function prototypes        *
 ********************************************************/
UpdateViewer *update_viewer_new(GtkWindow *parent);

GtkWindow *update_viewer_get_window(void);
GtkHBox *update_viewer_get_embed(void);
GtkPaned *update_viewer_get_vpaned(void);
GtkPaned *update_viewer_get_hpaned(void);
GtkButton *update_viewer_get_sexy_dm_button(void);
UpdateType update_viewer_get_update_update_type(void);
GtkVBox *update_viewer_get_status_view_vbox(void);

void update_viewer_postable_online_services_append(OnlineService *service);
gboolean update_viewer_postable_online_service_delete(OnlineService *service);

void update_viewer_view_update(OnlineService *service, const gdouble id, const gdouble user_id, const gchar *user_name, const gchar *nick_name, const gchar *date, const gchar *sexy_update, const gchar *text_update, GdkPixbuf *pixbuf, UpdateType update_type, gdouble retweet_update_id, const gchar *retweeted_user_name, const gchar *retweeted_user_nick);
gboolean update_viewer_set_in_reply_to_data(OnlineService *service, UpdateType update_type, const gchar *user_name, const gchar *update_text, gdouble user_id, gdouble update_id, gboolean reply, gboolean forwarding, gboolean force);

void update_viewer_best_friend_emulate_toggle_click(void);
void update_viewer_best_friends_start_dm(OnlineService *service, const gchar *user_name);

void update_viewer_emulate_embed_toggle_via_check_menu_item(GtkCheckMenuItem *view_update_viewer_floating_check_menu_item);
void update_viewer_emulate_compact_view_toggle(void);
void update_viewer_compact_view_toggled(GtkToggleButton *compact_toggle_button);
void update_viewer_set_embed_toggle_and_image(void);

/* BEGIN: UBERCHICK_HISTORY METHODS */
GtkWidget *update_viewer_sexy_entry_get_widget(void);

void update_viewer_hide_previous_updates(void);
void update_viewer_show_previous_updates(void);

void update_viewer_send(GtkWidget *activated_widget);
void update_viewer_sexy_select(GtkWidget *widget);

void update_viewer_beep(void);

void update_viewer_select_all(void);

gboolean update_viewer_sexy_entry_is_empty(void);
gboolean update_viewer_sexy_entry_clear(void);
void update_viewer_sexy_prefix_char(const char c);
gboolean update_viewer_sexy_prefix_string(const gchar *str, gboolean to_lower, gboolean uniq);
void update_viewer_sexy_insert_char(const char c);
gboolean update_viewer_sexy_insert_string(const gchar *str, gboolean to_lower, gboolean uniq);
void update_viewer_sexy_append_char(const char c);
gboolean update_viewer_sexy_append_string(const gchar *str, gboolean to_lower, gboolean uniq);
/* END: UBERCHICK_HISTORY METHODS */

void update_viewer_new_update(void);
void update_viewer_new_dm(void);
void update_viewer_reply(void);
void update_viewer_forward(void);
void update_viewer_retweet(void);

G_END_DECLS
#endif /* __UPDATE_VIEWER_H__ */
/********************************************************
 *                       eof                            *
 ********************************************************/
