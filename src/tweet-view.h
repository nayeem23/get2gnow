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
#ifndef __TWEET_VIEW_H__
#define __TWEET_VIEW_H__


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
/* My, Kaity G. B., new uber tweet viewer. */
typedef struct {
	/* Selected Tweet Widgets */
	GtkDialog		*tweet_view;
	GtkHBox			*tweet_view_embed;
	
	/* Stuff for sending a dm to a selected user. */
	GtkVBox			*user_vbox;
	GtkImage		*user_image;
	
	/* Tweet View */
	GtkVBox			*status_view_vbox;
	Label			*sexy_title;
	Label			*sexy_tweet;
	
	/* My, Kaity G. B., new libsexy powered tweet entry box. */
	GtkHBox			*char_count_hbox;
	GtkLabel		*char_count;
	
	GtkHBox			*tweet_hbox;
	SexySpellEntry		*sexy_entry;
	GtkButton		*sexy_send;
	GtkButton		*sexy_dm;
	
	GtkHBox			*dm_form_hbox;
	GtkLabel		*dm_frame_label;
	GtkButton		*dm_refresh;
	GtkComboBox		*friends_combo_box;
	GtkButton		*friends_send_dm;
	GtkButton		*dm_form_hide;
	GtkButton		*dm_form_show;
	
	/* Widgets that are enabled when we a tweet is selected */
	GList			*tweet_selected_buttons;
			
	/* Buttons for viewing details about the user of the current selected/extended Tweet. */
	GtkButton		*view_users_profile_button;
	GtkButton		*view_users_tweets_button;
	
	/* Buttons for viewing details about the user of the current selected/extended Tweet. */
	GtkButton		*user_follow_button;
	GtkButton		*user_unfollow_button;
	GtkButton		*user_block_button;
	
	/* Buttons for stuff to do with the current selected & extended tweet. */
	GtkButton		*reply_button;
	GtkButton		*retweet_button;
	GtkButton		*make_fave_button;
} TweetView;
#define TWEETS_RETURN_MODIFIERS_STATUSBAR_MSG "HotKeys: press [Return] and '@' to reply, '>' to re-tweet, [Ctrl+N] to tweet, and/or [Ctrl+D] or <Shift>+[Return] to DM."

extern unsigned long int in_reply_to_status_id;


/********************************************************
 *          Global method  & function prototypes        *
 ********************************************************/
void set_selected_tweet(unsigned long int id, const gchar *user_name, const gchar *tweet);
void unset_selected_tweet(void);

TweetView *tweet_view_new(GtkWindow *parent);
void tweet_view_show_tweet(unsigned long int id, const gchar *user_name, const gchar *user_nick, const gchar *date, const gchar *tweet, GdkPixbuf *pixbuf );

GtkComboBox *tweet_view_get_friends_combo_box(void);

void tweet_view_sexy_select(void);

void tweet_view_sexy_prefix_char(const char c);
void tweet_view_sexy_prefix_string(const gchar *str);
void tweet_view_sexy_set(gchar *tweet);
void tweet_view_sexy_insert_char(const char c);
void tweet_view_sexy_insert_string(const gchar *str);
void tweet_view_sexy_append_char(const char c);
void tweet_view_sexy_append_string(const gchar *str);
gint tweet_view_sexy_puts(const gchar *str, gint position);

void tweet_view_new_dm(void);
void tweet_view_reply(void);
void tweet_view_send(GtkWidget *activated_widget);

#endif /* __TWEET_VIEW_H__ */

/********************************************************
 *                       eof                            *
 ********************************************************/