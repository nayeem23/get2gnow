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
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <libsoup/soup.h>
#include <openssl/hmac.h>
#include <libxml/parser.h>

#include "config.h"
#include "main.h"

#include "online-services.h"
#include "network.h"

#include "tweet-list.h"
#include "tweets.h"
#include "users.h"

#include "parser.h"

#include "app.h"
#include "gconfig.h"
#include "cache.h"
#include "preferences.h"
#include "services-dialog.h"


/********************************************************
 *          Static method & function prototypes         *
 ********************************************************/
#define	ONLINE_SERVICES_ACCOUNTS	PREFS_PATH "/auth/services"

static void online_services_combo_box_add_new(GtkComboBox *combo_box, GtkListStore *list_store);

/********************************************************
 *          Variable definitions.                       *
 ********************************************************/
static OnlineServices *services=NULL;
OnlineServices *online_services=NULL;

/*OnlineService *selected_service=NULL;
OnlineService *in_reply_to_service=NULL;

unsigned long int in_reply_to_status_id=0;*/

#define	DEBUG_DOMAINS	"OnlineServices:UI:Network:Tweets:Requests:Users:Authentication"
#include "debug.h"

#define MAX_LOGINS 5


/********************************************************
 *   'Here be Dragons'...art, beauty, fun, & magic.     *
 ********************************************************/
OnlineServices *online_services_init(void){
	GSList		*k=NULL;
	OnlineService	*service=NULL;
	gchar	*account_key=NULL;
	
	services=g_new0(OnlineServices, 1);
	services->total=services->connected=0;
	
	gconfig_get_list_string(ONLINE_SERVICES_ACCOUNTS, &services->keys);
	
	for( k=services->keys; k; k=k->next ){
		account_key=(gchar *)k->data;
		debug("Loading '%s' account.", account_key);
		services->accounts=g_list_append(services->accounts, (online_service_open( (const gchar *)account_key )) );
		services->accounts=g_list_last(services->accounts);
		service=(OnlineService *)services->accounts->data;
		
		debug("Loaded account: '%s'.  Validating & connecting.", service->decoded_key);
		if(!service->enabled){
			debug("%s account not enabled.", service->decoded_key);
			continue;
		}
		
		if(G_STR_EMPTY(service->username) || G_STR_EMPTY(service->password)){
			debug("%s account is missing its username (=%s) and/or password (=%s).", service->decoded_key, service->username, service->password);
			continue;
		}
		
		if(!service->auto_connect){
			debug("%s account in not set to auto-connect.", service->decoded_key);
			continue;
		}
		
		if(online_service_connect(service)) services->total++;
	}
	
	services->accounts=g_list_first(services->accounts);
	
	online_services=services;
	
	if(!services->total)
		debug("No accounts are enabled or setup.  New accounts will be need to be setup." );
	else
		debug("%d services found and loaded.", services->total);
	
	return services;
}/*online_services_init*/

/* Login to services. */
gboolean online_services_login(OnlineServices *services){
	GList		*a=NULL;
	OnlineService	*service=NULL;
	gboolean	login_okay=FALSE;
	
	for(a=services->accounts; a; a=a->next){
		service=(OnlineService *)a->data;
		if(service->enabled && service->connected && service->auto_connect)
			if(online_service_login(service, FALSE))
				if(!login_okay) login_okay=TRUE;
	}
	if(!services->connected)
		debug("No accounts are enabled or setup.  New accounts will be need to be setup." );
	else
		debug("Connected to %d OnlineServices.", services->connected);
	
	app_state_on_connection(login_okay);
	return login_okay;
}/*online_services_login*/

/* Login to services. */
gboolean online_services_relogin(OnlineServices *services){
	GList		*a=NULL;
	OnlineService	*service=NULL;
	
	gboolean	relogin_okay=FALSE;
	for(a=services->accounts; a; a=a->next){
		service=(OnlineService *)a->data;
		if(!service->connected && service->enabled && service->auto_connect)
			online_service_reconnect(service);
		
		if(service->enabled && service->connected)
			if(online_service_login(service, FALSE))
				if(!relogin_okay) relogin_okay=TRUE;
	}
	app_state_on_connection(relogin_okay);
	return relogin_okay;
}/*online_services_relogin*/

void online_services_disconnect(OnlineServices *services){
	GList		*a=NULL;
	OnlineService	*service=NULL;
	
	selected_service=NULL;
	for(a=services->accounts; a; a=a->next){
		service=(OnlineService *)a->data;
		if(service->connected)
			online_service_disconnect(service, TRUE);
	}
	app_state_on_connection(FALSE);
}/*online_services_disconnect*/


OnlineService *online_services_save_service(OnlineServices *services, OnlineService *service, gboolean enabled, const gchar *url, gboolean https, const gchar *username, const gchar *password, gboolean auto_connect){
	if( G_STR_EMPTY(url) || G_STR_EMPTY(username) )
		return FALSE;
	
	if(service){
		debug("Saving existing service: '%s'.", service->decoded_key);
		if(!( g_str_equal(service->uri, url) && g_str_equal(service->username, username) ))
			online_services_delete_service(services, service);
		else{
			service->enabled=enabled;
			service->https=https;
			service->password=g_strdup(password);
			service->auto_connect=auto_connect;
			if( (online_service_save(service)) )
				if(online_service_reconnect(service))
					return service;
			debug("Unable to save existing OnlineService for: [%s].", service->decoded_key);
			return NULL;
		}
	}
	
	debug("Creating & saving new service: '%s@%s'.", username, url);
	service=online_service_new(enabled, url, https, username, password, auto_connect);
	debug("New OnlineService '%s' created.  Saving OnlineServices", service->decoded_key);
	
	services->total++;
	debug("New service: '%s' created.  OnlineServices total: %d.", service->decoded_key, services->total);
	
	debug("Adding '%s' to OnlineServices' keys.", service->decoded_key);
	if(!( (services->keys=g_slist_append(services->keys, service->key)) )){
		debug("**ERROR**: Failed to append new service's key: '%s', to OnlineServices' keys.", service->decoded_key);
		return NULL;
	}
	
	debug("Saving accounts & services list: '%s'.", ONLINE_SERVICES_ACCOUNTS);
	if(!( (gconfig_set_list_string(ONLINE_SERVICES_ACCOUNTS, services->keys)) )){
		debug("**ERROR**: Failed to save new service: '%s', couldn't save gconf's services list.", service->decoded_key);
		return NULL;
	}
	
	debug("Adding new service: '%s' to OnlineServices.", service->decoded_key);
	if(!( (services->accounts=g_list_append(services->accounts, service)) )){
		debug("**ERROR**: Failed to add: '%s', to OnlineServices' accounts.", service->decoded_key);
		return NULL;
	}
	
	debug("Retrieving new service: '%s' from OnlineServices accounts.", service->decoded_key);
	services->accounts=g_list_last(services->accounts);
	service=(OnlineService *)services->accounts->data;
	services->accounts=g_list_first(services->accounts);
	
	debug("Saving OnlineService: '%s' reloaded from OnlineServices accounts.", service->decoded_key);
	if(!( online_service_save(service) )){
		debug("**ERROR**: Failed saving new service: '%s@%s'.", username, url);
		return NULL;
	}

	online_service_connect(service);
	
	debug("Saving accounts & services successful.");
	if(service->connected){
		debug("\t\tConnecting to: '%s'\t[succeeded]", service->decoded_key);

		online_service_login(service, FALSE);
		network_refresh();
	}else
		debug("\t\tConnecting to: '%s'\t[failed]", service->decoded_key);
	
	debug("Saving '%s' service complete.  Total services: %d; Total connected: %d", service->decoded_key, services->total, services->connected);
	
	return service;
}/*online_services_save*/

void online_services_delete_service(OnlineServices *services, OnlineService *service){
	debug("Removing old OnlineService: '%s'.", service->decoded_key);
	service->enabled=FALSE;
	if(service->connected){
		online_service_disconnect(service, TRUE);
		if(!services->connected)
			app_state_on_connection(FALSE);
	}
	
	debug("Rebuilding OnlineServices' key while removing old key: %s.", service->key);
	g_slist_free(services->keys);
	services->keys=g_slist_alloc();
	GList *accounts=NULL;
	GSList *keys=NULL;
	debug("Rebuilding OnlineServices' keys, removing: '%s'.", service->key);
	OnlineService *key=NULL;
	for(accounts=services->accounts; accounts; accounts=accounts->next){
		key=(OnlineService *)accounts->data;
		if(!g_str_equal(key->key, service->key))
			keys=g_slist_append(keys, key->key);
	}
	services->keys=keys;
	
	debug("Saving re-built OnlineServices' keys.");
	if(!( (gconfig_set_list_string(ONLINE_SERVICES_ACCOUNTS, services->keys)) )){
		debug("**ERROR**: Failed to delete service: '%s', couldn't save gconf's services list.", service->decoded_key);
		return;
	}
	
	debug("Updating OnlineServices' accounts.  Removing OnlineService: [%s].", service->key);
	services->accounts=g_list_remove(services->accounts, service);
	
	debug("Determining what level of the OnlineService's cache directory to delete.");
	gboolean service_cache_rm_rf=TRUE;
	for(accounts=services->accounts; accounts; accounts=accounts->next){
		key=(OnlineService *)accounts->data;
		if(g_str_equal(key->uri, service->uri)){
			debug("OnlineService: [%s] share's avatars cache, only cookies will be deleted.", key->decoded_key);
			service_cache_rm_rf=FALSE;
		}
	}
	
	online_service_delete(service, service_cache_rm_rf);
	
	if(services->total) services->total--;
	debug("OnlineService deleted.  OnlineServices now has %d accounts.", services->total);
	if(!services->total){
		tweet_list_clear();
		network_deinit(TRUE, All);
		app_state_on_connection(FALSE);
		services_dialog_show(app_get_window());
	}
}/*online_services_delete(services, service);*/

static void online_services_combo_box_add_new(GtkComboBox *combo_box, GtkListStore *list_store){
	GtkTreeIter *iter=g_new0(GtkTreeIter, 1);
	gtk_list_store_append(list_store, iter);
	gtk_list_store_set(
			list_store, iter,
				UrlString, "[new account]",
				OnlineServicePointer, NULL,
			-1
	);
	uber_free(iter);
	gtk_combo_box_set_active(combo_box, 0);
}/*online_services_combo_box_add_new(combo_box, list_store);*/

gboolean online_services_combo_box_fill(OnlineServices *services, GtkComboBox *combo_box, GtkListStore *list_store, gboolean connected_only){
	if(!list_store) return FALSE;
	gtk_list_store_clear(list_store);
	if(!online_services->total){
		debug("No services found to load, new accounts need to be setup.");
		if(!connected_only) online_services_combo_box_add_new(combo_box, list_store);
		return FALSE;
	}
	
	GtkTreeIter		*iter=NULL;
	GList			*a=NULL;
	guint			services_loaded=0;
	debug("Loading services into tree view. total services: '%d'.", online_services->total);
	for(a=services->accounts; a; a=a->next){
		OnlineService *service=(OnlineService *)a->data;
		if( connected_only && !service->connected ) continue;
		
		debug("Appending account: '%s'; server: %s.", service->decoded_key, service->uri);
		iter=g_new0(GtkTreeIter, 1);
		gtk_list_store_append(list_store, iter);
		gtk_list_store_set(
					list_store, iter,
						UrlString, service->uri,
						OnlineServicePointer, service,
					-1
		);
		services_loaded++;
		uber_free(iter);
	}
	if(!connected_only)
		online_services_combo_box_add_new(combo_box, list_store);
	else if(services_loaded){
		debug("Accounts & services loaded.  Selecting combo_box's default account.");
		gtk_combo_box_set_active(combo_box, 0);
	}
	
	return (services_loaded ?TRUE :FALSE );
}/*online_services_combo_box_fill*/

OnlineService *online_services_connected_get_first(OnlineServices *services){
	GList		*a=NULL;
	OnlineService	*service=NULL;
	
	for(a=services->accounts; a; a=a->next){
		service=(OnlineService *)a->data;
		if(service->connected)
			break;
	}
	return service;
}/*online_services_connected_get_first*/

void online_services_request(OnlineServices *services, RequestMethod request, const gchar *uri, SoupSessionCallback callback, gpointer user_data, gpointer formdata){
	GList		*a=NULL;
	OnlineService	*service=NULL;
	
	for(a=services->accounts; a; a=a->next){
		service=(OnlineService *)a->data;
		if(!(service->enabled && service->connected)){
			debug("Unable to load: %s.  You're not connected to %s.", uri, service->key);
			app_statusbar_printf("Unable to load: %s.  You're not connected to: %s.", uri, service->key);
			continue;
		}
		online_service_request(service, request, uri, callback, user_data, formdata);
	}
}/*online_services_request*/

void online_services_increment_connected(OnlineServices *services){
	services->connected++;
	debug("OnlineServices has a new connections.  Total connected: #%d.", services->connected);
}/*online_services_increment_connected(online_services);*/

void online_services_decrement_connected(OnlineServices *services, gboolean no_state_change){
	if(services->connected) services->connected--;
	
	debug("OnlineServices still connected: #%d.", services->connected);
	if(services->connected) return;
	
	tweet_list_clear();
	network_deinit(TRUE, All);
	app_state_on_connection(FALSE);
	
	if(no_state_change) return;
	services_dialog_show(app_get_window());
}/*online_services_decrement_connected(online_services);*/

void online_services_deinit(OnlineServices *services){
	debug("**SHUTDOWN:** Closing & releasing %d accounts.", services->total);
	g_list_foreach(services->accounts, (GFunc)online_service_free, NULL);

	debug("**SHUTDOWN:** freeing OnlineServices keys.");
	g_slist_free(services->keys);
	services->keys=NULL;
	
	g_free(services);
	services=NULL;
	
	selected_service=NULL;
	
	services=NULL;
	online_services=NULL;
}/*online_services_deinit*/

/********************************************************
 *                       eof                            *
 ********************************************************/

