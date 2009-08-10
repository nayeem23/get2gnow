/* -*- Mode: C; shift-width: 8; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * get2gnow is:
 * 	Copyright (c) 2009 Kaity G. B. <uberChick@uberChicGeekChick.Com>
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
 * License ("RPL") Version 1.5, or subsequent versions as allowed by the RPL,
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
 * user-visible display as defined in Section 6.4 (d):
 * 
 * Initial art work including: design, logic, programming, and graphics are
 * Copyright (C) 2009 Kaity G. B. and released under the RPL where sapplicable.
 * All materials not covered under the terms of the RPL are all still
 * Copyright (C) 2009 Kaity G. B. and released under the terms of the
 * Creative Commons Non-Comercial, Attribution, Share-A-Like version 3.0 US license.
 * 
 * Any & all data stored by this Software created, generated and/or uploaded by any User
 * and any data gathered by the Software that connects back to the User.  All data stored
 * by this Software is Copyright (C) of the User the data is connected to.
 * Users may lisences their data under the terms of an OSI approved or Creative Commons
 * license.  Users must be allowed to select their choice of license for each piece of data
 * on an individual bases and cannot be blanketly applied to all of the Users.  The User may
 * select a default license for their data.  All of the Software's data pertaining to each
 * User must be fully accessible, exportable, and deletable to that User.
 */

/********************************************************
 *          My art, code, & programming.                *
 ********************************************************/
#define _GNU_SOURCE
#define _THREAD_SAFE



/********************************************************
 *        Project headers, eg #include "config.h"       *
 ********************************************************/
#include "config.h"
#include "program.h"

#include "cache.h"
#include "users-glists.h"

#include "main-window.h"
#include "users.h"

#include "online-service-wrapper.h"
#include "online-service-request.h"
#include "online-service.h"

#include "network.h"
#include "parser.h"

#include "friends-manager.h"
#include "following-viewer.h"
#include "tweet-view.h"


/********************************************************
 *          Variable definitions.                       *
 ********************************************************/
#define	DEBUG_DOMAINS	"Networking:OnlineServices:Friends:Followers:XML:Parser:UsersGlists."
#include "debug.h"


#define API_FOLLOWING		"/statuses/friends.xml"
#define API_FOLLOWERS		"/statuses/followers.xml"

static OnlineService *service=NULL;
static UsersGListLoadFunc on_load_func=NULL;
static guint page=0;

static gboolean fetching_users=FALSE;
gboolean getting_followers=FALSE;

static guint which_pass=0;

/*static UsersGListGetWhich users_glist_get_which_previous;*/


/*******************************************************
 *          Static method & function prototypes         *
 ********************************************************/
static GList *users_glist_parse(OnlineService *service, SoupMessage *xml);

static void users_glists_free(OnlineService *service, UsersGListGetWhich users_glist_get_which);


/********************************************************
 *   'Here be Dragons'...art, beauty, fun, & magic.     *
 ********************************************************/
GList *users_glist_get(UsersGListGetWhich users_glist_get_which, gboolean refresh, UsersGListLoadFunc func){
	if(!selected_service) return NULL;
	
	if(!fetching_users){
		if(!which_pass){
			on_load_func=func;
			if(refresh && service && service==selected_service)
				users_glists_free_lists(service);
			service=selected_service;
		}
		
		if( !refresh && service && service==selected_service ){
			GList *users=online_service_users_glist_get(service, users_glist_get_which);
			switch(users_glist_get_which){
				case GetFriends:
					if(!users) break;
					debug("Displaying & loading, %d pages, friends list for: [%s].", page, online_service_get_key(service));
					users=g_list_sort(users, (GCompareFunc)usrglistscasecmp);
					break;
				case GetFollowers:
					if(!users) break;
					debug("Displaying & loading, %d pages, followers list for: [%s].", page, online_service_get_key(service));
					users=g_list_sort(users, (GCompareFunc)usrglistscasecmp);
					break;
				case GetBoth:
					if(!(online_service_users_glist_get(service, GetFriends) && online_service_users_glist_get(service, GetFollowers))) break;
					debug("Displaying & loading, %d pages, friends and followers list for: [%s].", page, online_service_get_key(service));
					if(which_pass<2) break;
					GList *friends=online_service_users_glist_get(service, GetFriends), *followers=online_service_users_glist_get(service, GetFollowers);
					users=g_list_alloc();
					users=g_list_concat(friends, followers);
					users=g_list_sort(users, (GCompareFunc)usrglistscasecmp);
					online_service_users_glist_set(service, GetBoth, users);
					break;
			}
			if(users){
				page=0;
				which_pass=0;
				if(on_load_func!=NULL)
					on_load_func(users);
				return users;
			}
		}
		page=0;
	}
	
	fetching_users=TRUE;
	page++;
	debug("Please wait while %s downloads %s page #%d.", GETTEXT_PACKAGE, ( (users_glist_get_which==GetFollowers||which_pass) ?"followers" :"friends"), page );
	main_window_statusbar_printf("Please wait while %s downloads page #%d of users %s.", GETTEXT_PACKAGE, page, ( (users_glist_get_which==GetFollowers||which_pass) ?"who are following you" :"you're following") );
	
	gchar *uri=NULL;
	if( (users_glist_get_which==GetFriends || (users_glist_get_which==GetBoth && !which_pass && !online_service_users_glist_get(service, GetFriends) ) ) ){
		getting_followers=FALSE;
		uri=g_strdup_printf("%s?page=%d", API_FOLLOWING, page);
	}else{
		if(users_glist_get_which==GetBoth && online_service_users_glist_get(service, GetFriends) && online_service_users_glist_get(service, GetFollowers) ){
			fetching_users=FALSE;
			which_pass=2;
			return users_glist_get(users_glist_get_which, FALSE, NULL);
		}
		which_pass=1;
		getting_followers=TRUE;
		uri=g_strdup_printf("%s?page=%d", API_FOLLOWERS, page);
	}
	
	if(!service){
		debug("**ERROR:** OnlineService is not set.  I'm unable to retreive page %d of users %s.", page, (which_pass?_("who are following you"):_("you're following")));
		return NULL;
	}else{
		debug("Getting users page, uri:  %s.", uri);
		online_service_request(service, QUEUE, uri, users_glist_save, users_glist_process, (gpointer)users_glist_get_which, NULL);
	}
	g_free(uri);
	return NULL;
}/*users_glist_get*/

void *users_glist_process(SoupSession *session, SoupMessage *xml, OnlineServiceWrapper *service_wrapper){
	OnlineService *service=online_service_wrapper_get_online_service(service_wrapper);
	
	const gchar *uri=online_service_wrapper_get_requested_uri(service_wrapper);
	UsersGListGetWhich users_glist_get_which=(UsersGListGetWhich)online_service_wrapper_get_user_data(service_wrapper);
	const gchar *which_glist_str=( (users_glist_get_which==GetFriends) ? _("friends") : ( (users_glist_get_which==GetFollowers) ? _("followers") :_("friends and followers" ) ) );
	const gchar *service_key=online_service_get_key(service);
	const gchar *page_num_str=g_strrstr(g_strrstr(uri, "?"), "=");
	debug("Processing <%s>'s %s, page #%s.  Server response: %s [%i].", service_key, which_glist_str, page_num_str, xml->reason_phrase, xml->status_code);
	
	if(!network_check_http(service, xml)){
		debug("**ERROR:** No more %s could be downloaded the request was not successful./", which_glist_str);
		debug("**ERROR:** <%s>'s %s should be refreshed.", service_key, which_glist_str);
		return NULL;
	}
	
	GList *new_users=NULL;
	debug("Parsing user list");
	if(!(new_users=users_glist_parse(service, xml)) ){
		debug("No more %s where found, yippies we've got'em all.", which_glist_str);
		//which_pass++;
		return NULL;
	}
	
	/*cache_save_page(service, uri, xml->response_body);*/
	
	return new_users;
}/*users_glist_process(session, xml, service_wrapper);*/

void users_glist_save(OnlineServiceWrapper *service_wrapper, gpointer soup_session_callback_return_gpointer){
	GList *new_users=(GList *)soup_session_callback_return_gpointer;
	/*UsersGListGetWhich users_glist_get_which=(UsersGListGetWhich)online_service_wrapper_get_user_data(service_wrapper);*/
	UsersGListGetWhich users_glist_get_which=(which_pass ?GetFollowers :GetFriends);
	
	if(!(new_users)){
		fetching_users=FALSE;
		which_pass++;
		users_glist_get(users_glist_get_which, FALSE, NULL);
		return;
	}
	
	if(!which_pass)
		debug("Appending users to friends list.");
	else
		debug("Addending users to followers list.");
	
	online_service_users_glist_set(service, users_glist_get_which, new_users);
	/*now we get the next page - or send the results where they belong.*/
	debug("Retrieving the next page of %s.", (which_pass ?"followers" :"friends") );
	users_glist_get(users_glist_get_which, FALSE, NULL);
}/*users_glist_save*/

/**
 *returns: 1 if a is different from b, -1 if b is different from a, if they're equal
 *		than it returns 1 if a is a follower & b is not
 */
int users_glists_sort_by_user_name(User *user1, User *user2){
	return strcasecmp( user_get_user_name(user1), user_get_user_name(user2) );
}/*user_sort_by_user_name(l1->data, { l1=l1->next; l1->data; } );*/


GList *users_glist_parse(OnlineService *service, SoupMessage *xml){
	xmlDoc		*doc=NULL;
	xmlNode		*root_element=NULL;
	
	debug("Parsing users xml.");
	if(!( (doc=parse_xml_doc(xml, &root_element)) && root_element )){
		xmlCleanupParser();
		return NULL;
	}
	
	gboolean users_found=FALSE;
	xmlNode *current_node=NULL;
	debug("Parsed new users. Starting with: '%s' node.", root_element->name);
	while(!users_found){
		if(!current_node) current_node=root_element;
		else if(!current_node->next) break;
		else current_node=current_node->next;
		
		if(!( (current_node->type==XML_ELEMENT_NODE) && g_str_equal(current_node->name, "users") ))
			continue;
		
		if(!current_node->children) break;
		
		users_found=TRUE;
		current_node=current_node->children;
	}
	
	if(!users_found){
		xmlFreeDoc(doc);
		xmlCleanupParser();
		return NULL;
	}
	
	User *new_user=NULL;
	GList *new_users=NULL;
	for(; current_node; current_node=current_node->next){
		if(!( (current_node->type==XML_ELEMENT_NODE) && g_str_equal(current_node->name, "user") ))
			continue;
		
		if(!(new_user=user_parse_node(service, current_node->children)))
			continue;
		
		debug("Added user: [%s] to user list.", user_get_user_name(new_user));
		new_users=g_list_append(new_users, new_user);
		new_user=NULL;
	}
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	return new_users;
}/*users_glist_parse(service, xml)*/


void users_glists_free_lists(OnlineService *service){
	if(online_service_users_glist_get(service, GetBoth)) return users_glists_free(service, GetBoth);
	
	if(online_service_users_glist_get(service, GetFriends)) users_glists_free(service, GetFriends);
	if(online_service_users_glist_get(service, GetFollowers)) users_glists_free(service, GetFollowers);
}/*users_glists_free_lists(service);*/

static void users_glists_free(OnlineService *service, UsersGListGetWhich users_glist_get_which){
	GList *glist_free=NULL;
	if(!( glist_free=online_service_users_glist_get(service, users_glist_get_which) )) return;
	
	switch(users_glist_get_which){
		case GetBoth:
			debug("%s %s %s %s %s, %s %s %s %s %s %s %s %s.", _("friends"), _("of"), online_service_get_key(service), _("i.e."), _("who"), _("is"), _("following"), _("you"), _("and"), _("who"), _("is"), _("following"), online_service_get_key(service));
			break;
			
		case GetFriends:
			debug("%s %s %s %s, %s %s %s %s.", _("friends"), _("of"), online_service_get_key(service), _("i.e."), _("who"), _("is"), _("following"), online_service_get_key(service));
			break;
			
		case GetFollowers:
			debug("%s %s %s %s.", _("who"), _("is"), _("following"), online_service_get_key(service));
			break;
	}
	
	g_list_foreach(glist_free, (GFunc)user_free, NULL);
	
	uber_list_free(glist_free);
	if(users_glist_get_which==GetBoth) return;
	
	if( ((users_glist_get_which!=GetFriends)) && (glist_free=online_service_users_glist_get(service, GetFriends)) ){
		g_list_foreach(glist_free, (GFunc)user_free, NULL);
		uber_list_free(glist_free);
	}
	
	if( ((users_glist_get_which!=GetFollowers)) && (glist_free=online_service_users_glist_get(service, GetFollowers)) ){
		g_list_foreach(glist_free, (GFunc)user_free, NULL);
		uber_list_free(glist_free);
	}
}/*users_glists_free(service, GetBoth);*/


void users_glists_append_friend(OnlineService *service, User *user){
	GList *users=NULL;
	if(!(users=online_service_users_glist_get(service, GetFriends))) return;
	
	users=g_list_append(users, user);
	main_window_statusbar_printf("%s's %s %s %s %s.", online_service_get_key(service), _("is"), _("now"), _("following"), user_get_user_name(user));
}/*users_glists_append_friend*/

void users_glists_remove_friend(OnlineService *service, User *user){
	GList *users=NULL;
	if(!(users=online_service_users_glist_get(service, GetFriends))) return;
	
	users=g_list_remove(users, user);
	main_window_statusbar_printf("%s's %s %s %s %s.", online_service_get_key(service), _("is"), _("now"), _("following"), user_get_user_name(user));
}/*users_glists_remove_friend*/

void users_glists_remove_follower(OnlineService *service, User *user){
	GList *users=NULL;
	if(!(users=online_service_users_glist_get(service, GetFollowers))) return;
	
	users=g_list_append(users, user);
	main_window_statusbar_printf("%s's %s %s %s %s %s.", online_service_get_key(service), _("is"), _("no"), _("longer"), _("following"), user_get_user_name(user));
}/*users_glists_remove_friend*/


/********************************************************
 *                       eof                            *
 ********************************************************/
