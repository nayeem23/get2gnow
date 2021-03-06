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

#include <string.h>
#include <gconf/gconf-client.h>


/********************************************************
 *        Project headers, eg #include "config.h"       *
 ********************************************************/
#include "config.h"
#include "program.h"
#include "preferences.defines.h"

#include "gconfig.h"


/********************************************************
 *          Variable definitions.                       *
 ********************************************************/
typedef struct {
	GConfClient *gconf_client;
	
	gchar		*cached_key_bool;
	gboolean	cached_value_bool;
	
	gchar		*cached_key_int;
	gint		cached_value_int;
	
	gchar		*cached_key_float;
	gfloat		cached_value_float;
	
	gchar		*cached_key_string;
	gchar		*cached_value_string;
	
	gchar		*cached_key_list;
	GSList		*cached_value_list;
	GConfValueType	cached_type_list;
} GConfigPriv;

typedef struct {
	GConfigNotifyFunc	func;
	gpointer		user_data;
} GConfigNotifyData;

typedef enum {
	GConfigBool,
	GConfigInt,
	GConfigFloat,
	GConfigString,
	GConfigList,
} GConfigSupportedValue;

#define DEBUG_DOMAINS "OnlineServices:Authentication:Preferences:Settings:Setup:Start-Up:gconfig.c"
#include "debug.h"

#define DESKTOP_INTERFACE_ROOT  "/desktop/gnome/interface"

#define	GET_PRIV(obj)	(G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_GCONFIG, GConfigPriv))
G_DEFINE_TYPE(GConfig, gconfig, G_TYPE_OBJECT);

static GConfig		*gconfig=NULL;
static GConfigPriv	*gconfig_priv=NULL;

/********************************************************
 *          Static method & function prototypes         *
 ********************************************************/
static void gconfig_class_init(GConfigClass *class);
static void gconfig_init(GConfig *gconfig);
static void gconfig_finalize(GObject *object);

static gboolean gconfig_check_cached_bool(const gchar *key, gboolean *value);
static gboolean gconfig_check_cached_int(const gchar *key, gint *value);
static gboolean gconfig_check_cached_float(const gchar *key, gfloat *value);
static gboolean gconfig_check_cached_string(const gchar *key, gchar **value);
static gboolean gconfig_check_cached_list(const gchar *key, GSList **value);
static gboolean gconfig_check_cached(GConfigSupportedValue which_cache, const gchar *key);

static void gconfig_suggest_sync(const gchar *gtype, const gchar *key);
static void gconfig_print_list_values(const gchar *key, GSList *value, GConfValueType list_type);

/********************************************************
 *   'Here be Dragons'...art, beauty, fun, & magic.     *
 ********************************************************/
static void gconfig_class_init(GConfigClass *class){
	GObjectClass *object_class;
	object_class=G_OBJECT_CLASS(class);
	object_class->finalize=gconfig_finalize;
	g_type_class_add_private(object_class, sizeof(GConfigPriv));
}

static void gconfig_init(GConfig *gconfig){
	gconfig_priv=GET_PRIV(gconfig);
	gconfig_priv->gconf_client=gconf_client_get_default();
	
	gconf_client_add_dir(gconfig_priv->gconf_client, GCONF_PATH, GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_add_dir(gconfig_priv->gconf_client, DESKTOP_INTERFACE_ROOT, GCONF_CLIENT_PRELOAD_NONE, NULL);
	
	gconfig_priv->cached_key_bool=NULL;
	gconfig_priv->cached_value_bool=FALSE;
	
	gconfig_priv->cached_key_int=NULL;
	gconfig_priv->cached_value_int=0;
	
	gconfig_priv->cached_key_float=NULL;
	gconfig_priv->cached_value_float=0.0;
	
	gconfig_priv->cached_key_string=NULL;
	gconfig_priv->cached_value_string=NULL;
	
	gconfig_priv->cached_key_list=NULL;
	gconfig_priv->cached_value_list=NULL;
}/*gconfig_init(gconfig);*/

static void gconfig_finalize(GObject *object){
	if((!(gconfig && gconfig_priv))){
		G_OBJECT_CLASS(gconfig_parent_class)->finalize(object);
		return;
	}
	
	if(gconfig_priv->cached_key_bool)
		uber_free(gconfig_priv->cached_key_bool);
	
	if(gconfig_priv->cached_key_int)
		uber_free(gconfig_priv->cached_key_int);
	
	if(gconfig_priv->cached_key_float)
		uber_free(gconfig_priv->cached_key_float);
	
	if(gconfig_priv->cached_key_list)
		uber_free(gconfig_priv->cached_key_list);
	
	if(gconfig_priv->cached_value_list)
		uber_slist_free(gconfig_priv->cached_value_list);
	
	if(gconfig_priv->cached_key_string)
		uber_free(gconfig_priv->cached_key_string);
	
	if(gconfig_priv->cached_value_string)
		uber_free(gconfig_priv->cached_value_string);
	
	if(gconfig_priv->gconf_client){
		gconf_client_remove_dir(gconfig_priv->gconf_client, GCONF_PATH, NULL);
		gconf_client_remove_dir(gconfig_priv->gconf_client, DESKTOP_INTERFACE_ROOT, NULL);
		uber_object_unref(gconfig_priv->gconf_client);
	}
	
	G_OBJECT_CLASS(gconfig_parent_class)->finalize(object);
}/*gconfig_finalize*/

void gconfig_start(void){
	if(!gconfig) gconfig=g_object_new(TYPE_GCONFIG, NULL);
}/*gconfig_start();*/

void gconfig_shutdown(void){
	if(!gconfig)
		return;
	
	uber_object_unref(gconfig);
}/*gconfig_shutdown();*/

static void gconfig_suggest_sync(const gchar *gtype, const gchar *key){
	GError *error=NULL;
	gconf_client_suggest_sync(gconfig_priv->gconf_client, &error);
	if(error){
		debug("**ERROR:** Failed to suggest gconf client syncing w/gconf deamon");
		debug("**ERROR:** \t\tGConf deamon return: [%s] after saving: %s (%s)", key, error->message, gtype);
		g_error_free(error);
	}
}/*gconfig_suggest_sync("bool", key);*/

gboolean gconfig_set_int(const gchar *key, gint value){
	if(G_STR_N_EMPTY(gconfig_priv->cached_key_int) && g_str_equal(gconfig_priv->cached_key_int, key))
		gconfig_priv->cached_value_int=value;
	debug("Setting int:'%s' to %d", key, value);
	GError *error=NULL;
	gboolean success=gconf_client_set_int(gconfig_priv->gconf_client, key, value, &error);
	if(!success){
		debug("**ERROR:** GConf Client failed: save %s %s to %i. gconf error: %s", "int", key, value, error->message);
		g_error_free(error);
	}else
		gconfig_suggest_sync("int", key);
	return success;
}/*gconfig_set_int(key, int);*/

gboolean gconfig_get_int_or_default(const gchar *key, gint *value, gint default_int){
	if(gconfig_check_cached_int(key, value))
		return TRUE;
	
	GConfValue *gconf_value=NULL;
	GError *error=NULL;
	if((!(gconf_value=gconf_client_get(gconfig_priv->gconf_client, key, &error))) || error){
		*value=default_int;
		if(error){
			debug("**ERROR:** Failed to retrieve gconf value for: %s.  Error: %s", key, error->message);
			g_error_free(error);
			return FALSE;
		}
		
		debug("**NOTICE:** Setting default value for: %s to [%i]", key, default_int);
		if(!(gconfig_set_int(key, default_int))){
			debug("**ERROR:** failed to set '%s' to default value: '%d'", key, default_int);
			return FALSE;
		}
		
		gconfig_suggest_sync("int", key);
		gconfig_priv->cached_value_int=default_int;
		
		return FALSE;
	}
	
	if(gconf_value->type!=GCONF_VALUE_INT){
		debug("**ERROR:** Requested gconf key: %s does not contain a int value", key);
		*value=default_int;
	}else{
		*value=gconf_value_get_int(gconf_value);
		debug("Retrieved int value for %s [%d]", key, *value);
	}
	gconf_value_free(gconf_value);
	
	gconfig_priv->cached_value_int=*value;
	
	return TRUE;
}/*gconfig_get_int_or_default(key, default_boolean);*/


gboolean gconfig_get_int(const gchar *key, gint *value){
	if(gconfig_check_cached_int(key, value))
		return TRUE;
	
	*value=0;
	GError         *error=NULL;
	*value=gconf_client_get_int(gconfig_priv->gconf_client, key, &error);
	
	if(error){
		debug("**ERROR:** %s", error->message);
		g_error_free(error);
		return FALSE;
	}
	
	debug("Retrieved int:'%s'(=%d)", key, *value);
		
	gconfig_priv->cached_value_int=(*value);
	return TRUE;
}/*gconfig_get_int(key, &int);*/

gboolean gconfig_set_float(const gchar *key, gfloat value){
	if(G_STR_N_EMPTY(gconfig_priv->cached_key_float) && g_str_equal(gconfig_priv->cached_key_float, key))
		gconfig_priv->cached_value_float=value;
	
	debug("Setting: '%s' to %f [type: float]", key, value);
	GError *error=NULL;
	gboolean success=gconf_client_set_float(gconfig_priv->gconf_client, key, value, &error);
	if(!success){
		debug("**ERROR:** GConf Client failed: save %s %s to %f. gconf error: %s", "float", key, value, error->message);
		g_error_free(error);
	}else
		gconfig_suggest_sync("float", key);
	return success;
}/*gconfig_set_float(key, float);*/

gboolean gconfig_get_float(const gchar *key, gfloat *value){
	if(gconfig_check_cached_float(key, value))
		return TRUE;
	
	GError         *error=NULL;
	*value=gconf_client_get_float(gconfig_priv->gconf_client, key, &error);
	
	if(error){
		debug("**ERROR:** failed to load %s.  GConf error: %s", key, error->message);
		g_error_free(error);
		return FALSE;
	}
	
	debug("Retrieved: '%s'(=%f) [type: float]", key, *value);
		
	gconfig_priv->cached_value_float=(*value);
	return TRUE;
}/*gconfig_get_float(key, &float);*/

gboolean gconfig_if_bool(const gchar *key, gboolean default_boolean){
	gboolean value=default_boolean;
	if(gconfig_check_cached_bool(key, &value))
		return value;
	
	GConfValue *gconf_value=NULL;
	GError *error=NULL;
	if((!(gconf_value=gconf_client_get(gconfig_priv->gconf_client, key, &error))) || error){
		if(error){
			debug("**ERROR:** Failed to retrieve gconf value for: %s.  Error: %s", key, error->message);
			g_error_free(error);
			return default_boolean;
		}
		
		debug("*NOTICE:* Setting default value for: %s to [%s]", key, (default_boolean ?"TRUE" :"FALSE"));
		if(!(gconfig_set_bool(key, default_boolean))){
			debug("**ERROR:** failed to set '%s' to default value: '%s'", key, (default_boolean ?"TRUE" :"FALSE"));
			return default_boolean;
		}
		
		gconfig_suggest_sync("bool", key);
		gconfig_priv->cached_value_bool=default_boolean;
		
		return default_boolean;
	}
	
	if(gconf_value->type!=GCONF_VALUE_BOOL){
		debug("**ERROR:** Requested gconf key: %s does not contain a boolean value", key);
		value=default_boolean;
	}else{
		value=gconf_value_get_bool(gconf_value);
		debug("Retrieved boolean value for %s [%s]", key, (value ?"TRUE" :"FALSE"));
	}
	gconf_value_free(gconf_value);
	
	gconfig_priv->cached_value_bool=value;
	
	return value;
}/*gconfig_if_bool(key, default_boolean);*/

gboolean gconfig_set_bool(const gchar *key, gboolean value){
	if(G_STR_N_EMPTY(gconfig_priv->cached_key_bool) && g_str_equal(gconfig_priv->cached_key_bool, key))
		gconfig_priv->cached_value_bool=value;
	
	debug("Setting bool:'%s' to %s", key, (value ? "TRUE" : "FALSE"));
	GError *error=NULL;
	gboolean success=gconf_client_set_bool(gconfig_priv->gconf_client, key, value, &error);
	if(!success){
		debug("**ERROR:** Failed to save %s to %s. gconf error: %s", key, (value?"TRUE":"FALSE"), error->message);
		g_error_free(error);
	}else
		gconfig_suggest_sync("string", key);
	
	return success;
}/*gconfig_set_bool(key, boolean);*/

gboolean gconfig_get_bool(const gchar *key, gboolean *value){
	if(gconfig_check_cached_bool(key, value))
		return TRUE;
	
	GError *error=NULL;
	*value=FALSE;
	
	g_return_val_if_fail(value != NULL, FALSE);
	*value=gconf_client_get_bool(gconfig_priv->gconf_client, key, &error);
	
	if(error) {
		debug("**ERROR:** %s", error->message);
		g_error_free(error);
		return FALSE;
	}
	
	debug("Retrieved bool:'%s' to %s(=%d)", key, (*value ? "TRUE" : "FALSE"), *value);
		
	return TRUE;
}/*gconfig_get_bool(key, &boolean);*/

gboolean gconfig_set_string(const gchar *key, const gchar *value){
	if(G_STR_N_EMPTY(gconfig_priv->cached_key_string) && g_str_equal(gconfig_priv->cached_key_string, key)){
		uber_free(gconfig_priv->cached_value_string);
		gconfig_priv->cached_value_string=g_strdup(value);
	}
	
	debug("Setting string:'%s' to '%s'", key, value);
	GError *error=NULL;
	gboolean success=gconf_client_set_string(gconfig_priv->gconf_client, key, value, &error);
	if(!success){
		debug("**ERROR:** GConf Client failed: save %s %s to %s. gconf error: %s", "string", key, value, error->message);
		g_error_free(error);
	}else
		gconfig_suggest_sync("string", key);
	return success;
}/*gconfig_set_string(key, string);*/

gboolean gconfig_get_string(const gchar *key, gchar **value){
	if(gconfig_check_cached_string(key, value))
		return TRUE;
	
	GError         *error=NULL;
	*value=NULL;
	*value=gconf_client_get_string(gconfig_priv->gconf_client, key, &error);
	
	if(error){
		debug("**ERROR:** %s", error->message);
		g_error_free(error);
		return FALSE;
	}
	
	debug("Retrieved string: '%s'(='%s')", key, *value);
	
	uber_free(gconfig_priv->cached_key_string);
	gconfig_priv->cached_value_string=g_strdup(*value);
	
	return TRUE;
}/*gconfig_get_string(key, &string);*/

gboolean gconfig_set_list_string(const gchar *key, GSList *value){
	return gconfig_set_list(key, value, GCONF_VALUE_STRING);
}//gconfig_set_list_string

gboolean gconfig_get_list_string(const gchar *key, GSList **value){
	return gconfig_get_list(key, value, GCONF_VALUE_STRING);
}//gconfig_set_list_string

/* Possible values for 'list_type' are one of the follwing:
 * 	GCONF_VALUE_STRING, GCONF_VALUE_INT, GCONF_VALUE_BOOL, GCONF_VALUE_FLOAT, GCONF_VALUE_INVALID, GCONF_VALUE_SCHEMA, or 
 */
static void gconfig_print_list_values(const gchar *key, GSList *value, GConfValueType list_type){
	GSList *l=NULL;
	if(list_type==GCONF_VALUE_INVALID || list_type==GCONF_VALUE_SCHEMA || list_type==GCONF_VALUE_LIST || list_type==GCONF_VALUE_PAIR){
		debug("[undisplayable/mixed values]");
		return;
	}
	
	debug("GConf list: '%s', values:(=", key);
	for(l=value; l; l=l->next){
		switch(list_type){
			case GCONF_VALUE_STRING:
			case GCONF_VALUE_INT:
			case GCONF_VALUE_FLOAT:
				debug("'%s'", (gchar *)l->data);
				break;
			case GCONF_VALUE_BOOL:
				debug("'%s'", (g_str_equal((gchar *)l->data, "true") ?"TRUE" :"FALSE"));
				break;
			case GCONF_VALUE_INVALID:
			case GCONF_VALUE_SCHEMA:
			case GCONF_VALUE_LIST:
			case GCONF_VALUE_PAIR:
			default:
				/* yes we know this is never executed, it catches gcc errors & warning. */
				break;
		}
	}
	debug(")");
}/*gconfig_print_list_values(list, GCONF_VALUE_STRING);*/

gboolean gconfig_set_list(const gchar *key, GSList *value, GConfValueType list_type){
	if(G_STR_N_EMPTY(gconfig_priv->cached_key_list) && g_str_equal(gconfig_priv->cached_key_list, key)){
		uber_slist_free(gconfig_priv->cached_value_list);
		gconfig_priv->cached_value_list=g_slist_copy(value);
		gconfig_priv->cached_type_list=list_type;
	}
	
	if(IF_DEBUG){
		debug("Saving list:");
		gconfig_print_list_values(key, value, list_type);
	}
	GError *error=NULL;
	gboolean success=gconf_client_set_list(gconfig_priv->gconf_client, key, list_type, value, &error);
	if(!success){
		debug("**ERROR:** GConf client save failure.  GConf save %s %s. gconf error: %s", "list", key, error->message);
		g_error_free(error);
	}else
		gconfig_suggest_sync("list", key);
	return success;
}/*gconfig_set_list(key, value, GCONF_VALUE_STRING);*/

gboolean gconfig_get_list(const gchar  *key, GSList **value, GConfValueType list_type){
	if(gconfig_check_cached_list(key, value))
		return TRUE;
	
	GError *error=NULL;
	*value=NULL;
	
	*value=gconf_client_get_list(gconfig_priv->gconf_client, key, list_type, &error);
	if(error){
		debug("**ERROR:** failed to retrieve list: %s, error: %s", key, error->message);
		g_error_free(error);
		return FALSE;
	}else if(IF_DEBUG){
		debug("Retrieved list:");
		gconfig_print_list_values(key, *value, list_type);
	}
	return TRUE;
}/*gconfig_get_list(key, &list, GCONF_VALUE_STRING);*/

static gboolean gconfig_check_cached_bool(const gchar *key, gboolean *value){
	if(!gconfig_check_cached(GConfigBool, key))
		return FALSE;
	*value=gconfig_priv->cached_value_bool;
	return TRUE;
}/*gconfig_check_cached_bool(const gchar *key, gboolean *value);*/

static gboolean gconfig_check_cached_int(const gchar *key, gint *value){
	if(!gconfig_check_cached(GConfigInt, key))
		return FALSE;
	*value=gconfig_priv->cached_value_int;
	return TRUE;
}/*gconfig_check_cached_int(const gchar *key, gint *value);*/

static gboolean gconfig_check_cached_float(const gchar *key, gfloat *value){
	if(!gconfig_check_cached(GConfigFloat, key))
		return FALSE;
	*value=gconfig_priv->cached_value_float;
	return TRUE;
}/*gconfig_check_cached_float(const gchar *key, gfloat *value);*/

static gboolean gconfig_check_cached_string(const gchar *key, gchar **value){
	if(!gconfig_check_cached(GConfigString, key))
		return FALSE;
	*value=g_strdup(gconfig_priv->cached_value_string);
	return TRUE;
}/*gconfig_check_cached_string(const gchar *key, gstring *value);*/

static gboolean gconfig_check_cached_list(const gchar *key, GSList **value){
	if(!gconfig_check_cached(GConfigList, key))
		return FALSE;
	*value=gconfig_priv->cached_value_list;
	return TRUE;
}/*gconfig_check_cached_list(const gchar *key, GSList **value);*/

static gboolean gconfig_check_cached(GConfigSupportedValue which_cache, const gchar *key){
	gchar *cached_key=NULL;
	switch(which_cache){
		case	GConfigBool:
			cached_key=gconfig_priv->cached_key_bool;
			break;
		case	GConfigInt:
			cached_key=gconfig_priv->cached_key_int;
			break;
		case	GConfigFloat:
			cached_key=gconfig_priv->cached_key_float;
			break;
		case	GConfigString:
			cached_key=gconfig_priv->cached_key_string;
			break;
		case	GConfigList:
			cached_key=gconfig_priv->cached_key_list;
			break;
	}
	
	if(G_STR_N_EMPTY(cached_key) && g_str_equal(cached_key, key))
		return TRUE;
	
	switch(which_cache){
		case	GConfigBool:
			uber_free(gconfig_priv->cached_key_bool);
			gconfig_priv->cached_key_bool=g_strdup(key);
			break;
		case	GConfigInt:
			uber_free(gconfig_priv->cached_key_int);
			gconfig_priv->cached_key_int=g_strdup(key);
			break;
		case	GConfigFloat:
			uber_free(gconfig_priv->cached_key_float);
			gconfig_priv->cached_key_float=g_strdup(key);
			break;
		case	GConfigString:
			uber_free(gconfig_priv->cached_key_string);
			gconfig_priv->cached_key_string=g_strdup(key);
			break;
		case	GConfigList:
			uber_free(gconfig_priv->cached_key_list);
			gconfig_priv->cached_key_list=g_strdup(key);
			break;
	}
	return FALSE;
}/*gconfig_check_cached(GConfigBool|GConfigInt|GConfigFloat|GConfigString, "/apps/"GETTEXT_PACKAGE"/settings/value", &value);*/

gboolean gconfig_rm_rf(const gchar *key){
	GError *error=NULL;
	debug("Removing %s and all keys below it", key);
	gboolean success=gconf_client_recursive_unset(gconfig_priv->gconf_client, key, GCONF_UNSET_INCLUDING_SCHEMA_NAMES, &error);
	if(error){
		debug("**ERROR**: Failed to recursively unset: %s; gconf encountered an error: %s", key, error->message);
		g_error_free(error);
		return FALSE;
	}
	
	if(!success){
		debug("**ERROR**: Failed to recursively unset: %s; an unknow error occured", key);
		return FALSE;
	}
	
	gconfig_suggest_sync("unsetting", key);
	return TRUE;
}/*gconfig_rm_rf*/

static void gconfig_notify_data_free(GConfigNotifyData *data){
	g_slice_free(GConfigNotifyData, data);
}/*gconfig_notify_data_free(key, value);*/

static void gconfig_notify_func(GConfClient *client, guint id, GConfEntry  *entry, gpointer user_data){
	GConfigNotifyData *data;
	data=user_data;
	data->func(gconf_entry_get_key(entry), data->user_data);
}/*gconfig_notify_func(key, value);*/

guint gconfig_notify_add(const gchar *key, GConfigNotifyFunc func, gpointer user_data){
	GConfigNotifyData *data;
	
	data=g_slice_new0(GConfigNotifyData);
	data->func=func;
	data->user_data=user_data;
	
	return gconf_client_notify_add(gconfig_priv->gconf_client, key, gconfig_notify_func, data, (GFreeFunc)gconfig_notify_data_free, NULL);
}/*gconfig_notify_add(key, (*GconfigNotifyFunc), user_data);*/

gboolean gconfig_notify_remove(guint id){
	gconf_client_notify_remove(gconfig_priv->gconf_client, id);
	return TRUE;
}/*gconfig_notify_remove(id);*/


/********************************************************
 *                       eof                            *
 ********************************************************/
