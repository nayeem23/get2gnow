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



/********************************************************
 *        Project headers, eg #include "config.h"       *
 ********************************************************/
#include <glib.h>
#include "config.h"
#include "gconfig.h"
#include "main.h"
#include "debug.h"
#include "network.h"
#include "gtkbuilder.h"
#include "tweets.h"
#include "app.h"
#include "tweet-view.h"
#include "preferences.h"


/********************************************************
 *          Variable definitions.                       *
 ********************************************************/
#define DEBUG_DOMAIN "Tweets"
#define GtkBuilderUI "tweet-view.ui"
#define	TWEET_MAX_CHARS	140

static TweetView *tweet_view=NULL;


/********************************************************
 *          Static method & function prototypes         *
 ********************************************************/
static void tweet_view_sexy_init(void);
static void tweet_view_reorder(void);

static void tweet_view_tweet_selected_buttons_setup(GtkBuilder *ui);
static void tweet_view_tweet_selected_buttons_show(gboolean show);

static void tweet_view_count_tweet_char(GtkEntry *entry, GdkEventKey *event, GtkLabel *tweet_character_counter);
static void tweet_view_sexy_send(gpointer user_data);

static void tweet_view_dm_data_fill(GList *followers);
static void tweet_view_dm_data_set_sensitivity(GtkButton *button);
static void tweet_view_dm_data_show(void);
static void tweet_view_dm_data_hide(void);


/********************************************************
 *   'Here be Dragons'...art, beauty, fun, & magic.     *
 ********************************************************/
TweetView *tweet_view_new(GtkWindow *parent){
	GtkBuilder	*ui;
	
	debug(DEBUG_DOMAIN, "Building Tweet View");
	ui=gtkbuilder_get_file( GtkBuilderUI,
				"tweet_view", &tweet_view->tweet_view,
				"tweet_view_embed", &tweet_view->tweet_view_embed,
				
				"user_vbox", &tweet_view->user_vbox,
				"user_image", &tweet_view->user_image,
				
				"view_users_profile_button", &tweet_view->view_users_profile_button,
				"view_users_tweets_button", &tweet_view->view_users_tweets_button,
				
				"user_follow_button", &tweet_view->user_follow_button,
				"user_unfollow_button", &tweet_view->user_unfollow_button,
				"user_block_button", &tweet_view->user_block_button,
				
				"char_count_hbox", &tweet_view->char_count_hbox,
				"char_count", &tweet_view->char_count,
				
				"tweet_hbox", &tweet_view->tweet_hbox,
				"sexy_send", &tweet_view->sexy_send,
				"sexy_dm", &tweet_view->sexy_dm,
				
				"dm_form_hbox", &tweet_view->dm_form_hbox,
				"dm_refresh", &tweet_view->dm_refresh,
				"dm_frame_label", &tweet_view->dm_frame_label,
				"friends_combo_box", &tweet_view->friends_combo_box,
				"friends_send_dm", &tweet_view->friends_send_dm,
				"dm_data_hide", &tweet_view->dm_form_hide,
				"dm_data_show", &tweet_view->dm_form_show,
				
				"reply_button", &tweet_view->reply_button, 
				"retweet_retweet_button", &tweet_view->retweet_button,
				"make_fave_button", &tweet_view->make_fave_button,
			NULL
	);
	
	tweet_view_sexy_init();
	tweet_view_reorder();
	tweet_view_tweet_selected_buttons_setup(ui);
	g_signal_connect_after(tweet_view->tweet_view_embed, "key-press-event", G_CALLBACK(tweets_hotkey), NULL);
	
	/* Connect the signals */
	gtkbuilder_connect( ui, tweet_view,
				"view_profile_button", "clicked", tweets_user_view_profile,
				"view_tweets_button", "clicked", tweets_user_view_tweets,
				
				"user_follow_button", "clicked", tweets_user_follow,
				"user_unfollow_button", "clicked", tweets_user_unfollow,
				"user_block_button", "clicked", tweets_user_block,
				
				"friends_send_dm", "clicked", tweet_view_send,
				"sexy_send", "clicked", tweet_view_send,
				"sexy_dm", "clicked", tweet_view_send,
				
				"dm_form_hide", "clicked", tweet_view_dm_data_set_sensitivity,
				"dm_form_show", "clicked", tweet_view_new_dm,
				
				"reply_button", "clicked", tweets_reply,
				"retweet_button", "clicked", tweets_retweet,
				"make_fave_button", "clicked", tweets_save_fave,
			NULL
	);
	
	GtkCellRenderer *renderer=gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(tweet_view->friends_combo_box), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(tweet_view->friends_combo_box), renderer, "text", 0, NULL);
	tweet_view_tweet_selected_buttons_show(FALSE);
	return tweet_view;
}

static void tweet_view_sexy_init(void){
	/* Set-up expand tweet view.  Used to view tweets in detailed & send tweets and DMs. */
	tweet_view->sexy_title=label_new();
	gtk_widget_show(GTK_WIDGET(tweet_view->sexy_title));
	gtk_box_pack_start(
			GTK_BOX(tweet_view->status_view_vbox),
			GTK_WIDGET(tweet_view->sexy_title),
			TRUE, TRUE, 0
	);
	
	tweet_view->sexy_tweet=label_new();
	gtk_widget_show(GTK_WIDGET(tweet_view->sexy_tweet));
	gtk_box_pack_start(
			GTK_BOX(tweet_view->status_view_vbox),
			GTK_WIDGET(tweet_view->sexy_tweet),
			TRUE, TRUE, 0
	);
	
	tweet_view->sexy_entry=(SexySpellEntry *)sexy_spell_entry_new();
	gtk_widget_show(GTK_WIDGET(tweet_view->sexy_entry));
	gtk_box_pack_start(
			GTK_BOX(tweet_view->tweet_hbox),
			GTK_WIDGET(tweet_view->sexy_entry),
			TRUE, TRUE, 0
	);
	
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->tweet_hbox),
			GTK_WIDGET(tweet_view->sexy_entry),
			1
	);
	
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->tweet_hbox),
			GTK_WIDGET(tweet_view->sexy_send),
			2
	);
	
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->tweet_hbox),
			GTK_WIDGET(tweet_view->sexy_dm),
			3
	);
	
	g_signal_connect_after(tweet_view->sexy_entry, "key-press-event", G_CALLBACK(tweets_hotkey), NULL);
	g_signal_connect_after(tweet_view->sexy_entry, "key-release-event", G_CALLBACK(tweet_view_count_tweet_char), tweet_view->char_count);
	g_signal_connect(tweet_view->sexy_entry, "activate", G_CALLBACK(tweet_view_sexy_send), NULL);
	g_signal_connect_after(tweet_view->friends_combo_box, "changed", G_CALLBACK(tweet_view_sexy_select), tweet_view->friends_combo_box);
}//tweet_view_sexy_init

static void tweet_view_tweet_selected_buttons_setup(GtkBuilder *ui){
	const gchar *tweet_selected_buttons[]={
		"dm_button",
		
		"view_users_profile_button",
		"view_users_tweets_button",
		
		"user_follow_button",
		"user_unfollow_button",
		"user_block_button",
		
		"sexy_dm",
		
		"reply_button",
		"retweet_button",
		"make_fave_button",
	};
	
	GList *list=NULL;
	for(int i=0; i < G_N_ELEMENTS(tweet_selected_buttons); i++)
		list=g_list_append(list,(gtk_builder_get_object(ui, tweet_selected_buttons[i])) );
	tweet_view->tweet_selected_buttons=list;
}//tweet_view_selected_widgets_setup

static void tweet_view_tweet_selected_buttons_show(gboolean show){
	for(GList *l=tweet_view->tweet_selected_buttons; l; l=l->next)
		gtk_widget_set_sensitive( GTK_WIDGET(l->data), show );
}//tweet_view_selected_widgets_show

static void tweet_view_reorder(void){
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->status_view_vbox),
			GTK_WIDGET(tweet_view->sexy_title),
			0
	);
	
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->status_view_vbox),
			GTK_WIDGET(tweet_view->sexy_tweet),
			1
	);
	
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->status_view_vbox),
			GTK_WIDGET(tweet_view->char_count_hbox),
			2
	);
	
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->status_view_vbox),
			GTK_WIDGET(tweet_view->tweet_hbox),
			3
	);
	
	gtk_box_reorder_child(
			GTK_BOX(tweet_view->status_view_vbox),
			GTK_WIDGET(tweet_view->dm_form_hbox),
			4
	);
}//tweet_view_reorder


void tweet_view_show_tweet(unsigned long int id, const gchar *user_name, const gchar *user_nick, const gchar *date, const gchar *tweet, GdkPixbuf *pixbuf){
	set_selected_tweet(id, user_name, tweet);
	tweet_view_tweet_selected_buttons_show( (id ?TRUE :FALSE ) );
	
	gchar *title_text=NULL;
	if(!( G_STR_EMPTY(user_nick) && G_STR_EMPTY(user_name) && G_STR_EMPTY(date) ))
		title_text=g_strdup_printf("<b>%s( @%s )</b> - %s", user_nick, user_name, date);
	else
		title_text=g_strdup("");
	
	label_set_text(tweet_view->sexy_title, title_text);
	label_set_text(tweet_view->sexy_tweet, tweet);
	g_free(title_text);
	
	if(!pixbuf)
		gtk_image_set_from_icon_name(tweet_view->user_image, PACKAGE_NAME, ImagesExpanded);
	else{
		GdkPixbuf *resized=NULL;
		resized=images_expand_pixbuf( pixbuf );
		gtk_image_set_from_pixbuf(tweet_view->user_image, resized);
		if(resized)
			g_object_unref(resized);
	}
	
	tweet_view_sexy_select();
}//tweet_view_show_tweet

static gshort tweetlen(gchar *tweet){
	gushort character_count=0;
	while(*tweet){
		unsigned char l=*tweet++;
		if(l=='<' || l=='>')
			character_count+=3;
		character_count++;
	}
	
	return TWEET_MAX_CHARS-character_count;
}//tweetlen

static void tweet_view_count_tweet_char(GtkEntry *entry, GdkEventKey *event, GtkLabel *tweet_character_counter){
	gshort character_count=tweetlen(entry->text);
	gchar *remaining_characters=NULL;
	if(character_count < 0){
		if(!gconfig_if_bool(PREFS_UI_NO_ALERT))
			gtk_widget_error_bell(GTK_WIDGET(entry));
		remaining_characters=g_markup_printf_escaped("<span size=\"small\" foreground=\"red\">%i</span>", character_count);
	}else
		remaining_characters=g_markup_printf_escaped("<span size=\"small\" foreground=\"green\">%i</span>", character_count);
	
	gtk_label_set_markup( tweet_character_counter, remaining_characters );
	g_free(remaining_characters);
}//tweet_view_count_tweet_char

void tweet_view_sexy_select(void){
	gtk_widget_grab_focus(GTK_WIDGET(tweet_view->sexy_entry));
	gtk_entry_set_position(GTK_ENTRY(tweet_view->sexy_entry), -1 );
}//tweet_view_sexy_select

void tweet_view_sexy_prefix_char(const char c){
	gchar *str=g_strdup_printf("%c", c);
	tweet_view_sexy_prefix_string((const gchar *)str);
	g_free(str);
}//tweet_view_sexy_prefix_char

void tweet_view_sexy_prefix_string(const gchar *str){
	tweet_view_sexy_puts(str, 0);
}//tweet_view_sexy_prefix_string

void tweet_view_sexy_set(gchar *tweet){
	gtk_entry_set_text(GTK_ENTRY(tweet_view->sexy_entry), tweet);
	tweet_view_sexy_select();
}//tweet_view_sexy_set

void tweet_view_sexy_insert_char(const char c){
	gchar *str=g_strdup_printf("%c", c);
	tweet_view_sexy_insert_string((const gchar *)str);
	g_free(str);
}//tweet_view_sexy_insert_char

void tweet_view_sexy_insert_string(const gchar *str){
	tweet_view_sexy_puts(str, gtk_editable_get_position(GTK_EDITABLE(tweet_view->sexy_entry)) );
}//tweet_view_sexy_insert_string

void tweet_view_sexy_append_char(const char c){
	gchar *str=g_strdup_printf("%c", c);
	tweet_view_sexy_append_string((const gchar *)str);
	g_free(str);
}//tweet_view_sexy_append_char

void tweet_view_sexy_append_string(const gchar *str){
	tweet_view_sexy_puts(str,(gint)gtk_entry_get_text_length(GTK_ENTRY(tweet_view->sexy_entry)) );
}//tweet_view_sexy_append_string

gint tweet_view_sexy_puts(const gchar *str, gint position){
	gtk_editable_insert_text(GTK_EDITABLE(tweet_view->sexy_entry), str, -1, &position );
	gtk_entry_set_position(GTK_ENTRY(tweet_view->sexy_entry), position );
	tweet_view_sexy_select();
	return position;
}//tweet_view_sexy_puts

void tweet_view_send(GtkWidget *activated_widget){
	gchar *user_name=NULL;
	
	if( G_STR_EMPTY(GTK_ENTRY(tweet_view->sexy_entry)->text) )
		return tweets_reply();
		
	if( activated_widget==GTK_WIDGET(tweet_view->sexy_dm) )
		tweet_view_sexy_send( selected_tweet_get_user_name() );
	
	else if( (activated_widget==GTK_WIDGET(tweet_view->friends_send_dm)) && (GTK_WIDGET_IS_SENSITIVE(tweet_view->friends_combo_box)) && (user_name=gtk_combo_box_get_active_text(tweet_view->friends_combo_box)) ){
		tweet_view_sexy_send(user_name);
		g_free(user_name);
		gtk_combo_box_set_active(tweet_view->friends_combo_box, 0);
	}else
		tweet_view_sexy_send(NULL);
}//tweet_view_send
	
static void tweet_view_sexy_send(gpointer user_data){
	if(!( (GTK_ENTRY(tweet_view->sexy_entry)->text) && (tweetlen(GTK_ENTRY(tweet_view->sexy_entry)->text) <= TWEET_MAX_CHARS) )){
		if(!gconfig_if_bool(PREFS_UI_NO_ALERT))
			gtk_widget_error_bell(GTK_WIDGET(tweet_view->sexy_entry));
		return;
	}
	
	gchar *tweet=url_encode(GTK_ENTRY(tweet_view->sexy_entry)->text);
	const gchar *user_name=(const gchar *)user_data;
	if(G_STR_EMPTY(user_name))
		network_post_status(tweet);
	else
		network_send_message(user_name, (const gchar *)tweet);
	
	g_free(tweet);
	tweet_view_sexy_set((gchar *)"");
}//tweet_view_sexy_send

void tweet_view_new_dm(void){
	tweet_view_dm_data_set_sensitivity(tweet_view->dm_form_show);
	tweet_view_dm_data_fill( user_get_followers(FALSE) );
}//tweets_new_dm

void tweet_view_dm_data_fill(GList *followers){
	GList		*list;
	GtkTreeIter	iter;
	User		*user;
	GtkListStore	*model_followers;
	gchar		*null_friend=g_strdup("");
	
	model_followers=GTK_LIST_STORE( gtk_combo_box_get_model( tweet_view->friends_combo_box ) );
	gtk_list_store_append(model_followers, &iter);
	gtk_list_store_set( model_followers, &iter, 0, null_friend, -1 );
	for(list=followers; list; list=list->next) {
		user =(User *)list->data;
		gtk_list_store_append(model_followers, &iter);
		gtk_list_store_set(
				model_followers,
				&iter,
				0, user->user_name,
				-1
		);
	}
}//tweet_view_dm_data_fill

static void tweet_view_dm_data_set_sensitivity(GtkButton *button){
	gboolean dm_activate=( button==tweet_view->dm_form_show );
	gtk_widget_set_sensitive( GTK_WIDGET(tweet_view->dm_form_hbox), dm_activate );
	gtk_widget_set_sensitive( GTK_WIDGET(tweet_view->friends_combo_box), dm_activate );
	gtk_widget_set_sensitive( GTK_WIDGET(tweet_view->dm_frame_label), dm_activate );
	gtk_widget_set_sensitive( GTK_WIDGET(tweet_view->friends_send_dm), dm_activate );
	gtk_widget_set_sensitive( GTK_WIDGET(tweet_view->dm_form_hide), dm_activate );
	gtk_widget_set_sensitive( GTK_WIDGET(tweet_view->dm_form_show), !dm_activate );
	return (dm_activate ?tweet_view_dm_data_show() :tweet_view_dm_data_hide() );
}//tweet_view_dm_data_set_sensitivity

static void tweet_view_dm_data_show(void){
	gtk_widget_show( GTK_WIDGET(tweet_view->dm_form_hbox) );
	gtk_widget_show( GTK_WIDGET(tweet_view->friends_combo_box) );
	gtk_widget_show( GTK_WIDGET(tweet_view->dm_frame_label) );
	gtk_widget_show( GTK_WIDGET(tweet_view->friends_send_dm) );
	gtk_widget_show( GTK_WIDGET(tweet_view->dm_form_hide) );
	gtk_widget_hide( GTK_WIDGET(tweet_view->dm_form_show) );
	gtk_widget_grab_focus(GTK_WIDGET(tweet_view->friends_combo_box));
}//tweet_view_dm_data_show

static void tweet_view_dm_data_hide(void){
	gtk_widget_hide( GTK_WIDGET(tweet_view->dm_form_hbox) );
	gtk_widget_hide( GTK_WIDGET(tweet_view->friends_combo_box) );
	gtk_widget_hide( GTK_WIDGET(tweet_view->dm_frame_label) );
	gtk_widget_hide( GTK_WIDGET(tweet_view->friends_send_dm) );
	gtk_widget_hide( GTK_WIDGET(tweet_view->dm_form_hide) );
	gtk_widget_show( GTK_WIDGET(tweet_view->dm_form_show) );
	gtk_widget_grab_focus(GTK_WIDGET(tweet_view->sexy_entry));
}//tweet_view_dm_data_hide

/********************************************************
 *                       eof                            *
 ********************************************************/