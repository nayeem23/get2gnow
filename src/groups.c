/* -*- Mode: C; shift-width: 8; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * get2gnow is:
 * 	Copyright (c) 2006-2009 Kaity G. B. <uberChick@uberChicGeekChick.Com>
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
/********************************************************************************
 *                      My art, code, & programming.                            *
 ********************************************************************************/
#define _GNU_SOURCE
#define _THREAD_SAFE



/********************************************************************************
 *      Project, system & library headers, eg #include <gdk/gdkkeysyms.h>       *
 ********************************************************************************/
#include <glib/gi18n.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <libsoup/soup-message.h>

#include "config.h"
#include "program.h"

#include "online-services-typedefs.h"
#include "online-service.h"

#include "main-window.h"
#include "gconfig.h"
#include "preferences.h"

#include "parser.h"
#include "groups.h"


/********************************************************************************
 *        Methods, macros, constants, objects, structs, and enum typedefs       *
 ********************************************************************************/
#define	DEBUG_DOMAINS	"Search:Groups:Parser:Requests:OnlineServices:Tweets:UI:Refreshing:Dates:Times:Search.c"
#include "debug.h"


/* Parse a timeline XML file */
guint groups_parse_conversation(OnlineService *service, SoupMessage *xml, const gchar *uri, TweetList *tweet_list){
	xmlDoc		*doc=NULL;
	xmlNode		*root_element=NULL;
	xmlNode		*current_node=NULL;
	UserStatus 	*status=NULL;
	
	/* Count new tweets */
	guint		new_updates=0;
	gdouble		status_id=0;
	gdouble		id_newest_update=0, id_oldest_update=0;
	
	gchar		**uri_split=g_strsplit( g_strrstr(uri, "/"), "?", 2);
	gchar		*timeline=g_strdup(uri_split[0]);
			g_strfreev(uri_split);
	
	online_service_update_ids_get(service, timeline, &id_newest_update, &id_oldest_update);
	gdouble	last_notified_update=id_newest_update;
	id_newest_update=0.0;
	
	gboolean	has_loaded=tweet_list_has_loaded(tweet_list);
	gboolean	notify=((id_oldest_update&&has_loaded)?gconfig_if_bool(PREFS_NOTIFY_FOLLOWING, TRUE):FALSE);
	gboolean	save_oldest_id=(has_loaded?FALSE:TRUE);
	
	guint		tweet_list_notify_delay=tweet_list_get_notify_delay(tweet_list);
	const gint	tweet_display_interval=10;
	const gint	notify_priority=(tweet_list_get_page(tweet_list)-1)*100;
	
	if(!(doc=parse_xml_doc(xml, &root_element))){
		debug("Failed to parse xml document, timeline: %s; uri: %s.", timeline, uri);
		xmlCleanupParser();
		uber_free(timeline);
		return 0;
	}
	
	/* get tweets or direct messages */
	debug("Parsing %s timeline.", root_element->name);
	const gchar *service_user_name=online_service_get_user_name(service);
	gboolean free_status;
	for(current_node = root_element; current_node; current_node = current_node->next) {
		if(current_node->type != XML_ELEMENT_NODE ) continue;
		
		if( g_str_equal(current_node->name, "statuses") || g_str_equal(current_node->name, "direct-messages") ){
			if(!current_node->children) continue;
			current_node = current_node->children;
			continue;
		}
		
		if(!( g_str_equal(current_node->name, "status") || g_str_equal(current_node->name, "direct_message") ))
			continue;
		
		if(!current_node->children){
			debug("*WARNING:* Cannot parse %s. Its missing children nodes.", current_node->name);
			continue;
		}
		
		debug("Parsing %s.", (g_str_equal(current_node->name, "status") ?"status update" :"direct message" ) );
		
		status=NULL;
		debug("Creating tweet's Status *.");
		if(!( (( status=user_status_parse(service, current_node->children, Groups ))) && (status_id=user_status_get_id(status)) )){
			if(status) user_status_free(status);
			continue;
		}
		
		new_updates++;
		free_status=TRUE;
		debug("Adding UserStatus from: %s, ID: %f, on <%s> to TweetList.", user_status_get_user_name(status), status_id, online_service_get_key(service));
		user_status_store(status, tweet_list);
		
		if(!save_oldest_id && status_id > last_notified_update && strcasecmp(user_status_get_user_name(status), service_user_name) ){
			tweet_list_mark_as_unread(tweet_list);
			if(notify){
				free_status=FALSE;
				g_timeout_add_seconds_full(notify_priority, tweet_list_notify_delay, main_window_notify_on_timeout, status, (GDestroyNotify)user_status_free);
				tweet_list_notify_delay+=tweet_display_interval;
			}
		}
		
		if(!id_newest_update) id_newest_update=status_id;
		
		if(save_oldest_id)
			id_oldest_update=status_id;
		
		if(free_status) user_status_free(status);
	}
	
	if(new_updates && id_newest_update){
		/*TODO implement this only once it won't ending up causing bloating.
		 *cache_save_page(service, uri, xml->response_body);
		 */
		const gchar *online_service_guid=online_service_get_guid(service);
		debug("Processing <%s>'s requested URI's: [%s] new update IDs", online_service_guid, uri);
		debug("Saving <%s>'s; update IDs for [%s].  %f - newest ID.  %f - oldest ID.", online_service_guid, timeline, id_newest_update, id_oldest_update);
		online_service_update_ids_set(service, timeline, id_newest_update, id_oldest_update);
	}
	
	uber_free(timeline);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	return new_updates;
}/*search_parse_results(service, xml, uri, tweet_list);*/

/********************************************************************************
 *                                    eof                                       *
 ********************************************************************************/
