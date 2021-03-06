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
#include <libnotify/notify.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "config.h"
#include "program.h"
#include "datetime.h"

#include "ipc.h"
#include "gconfig.h"

#include "main-window.h"

#include "online-service-request.h"
#include "online-services.h"
#include "network.h"
#include "proxy.h"

#include "cache.h"
#include "images.h"
#include "www.h"
#include "xml.h"
#include "groups.h"


/********************************************************
 *          Variable definitions.                       *
 ********************************************************/
#define DEBUG_DOMAINS "OnlineServices:Authentication:Preferences:Settings:Setup:Start-Up:program.c"
#include "debug.h"

static gboolean notifing=FALSE;
static gchar **remaining_argv=NULL;
static gint remaining_argc=0;
static GnomeProgram *program=NULL;


/********************************************************
 *          Static method & function prototypes         *
 ********************************************************/


/********************************************************
 *   'Here be Dragons'...art, beauty, fun, & magic.     *
 ********************************************************/
gboolean program_init(int argc, char **argv){
	if((ipc_init_check(argc, argv))){
		g_fprintf(stdout, "%s is already running.  Be sure to check your desktop's notification tray for %s's icon.\n", GETTEXT_PACKAGE, GETTEXT_PACKAGE);
		ipc_deinit();
		return FALSE;
	}
	
	datetime_locale_init();
	
	GOptionContext *option_context=g_option_context_new(GETTEXT_PACKAGE);
	GOptionEntry option_entries[]={
					{
						G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY,
						&remaining_argv,
						"Special option that collects any remaining arguments for us"
					},
					{ NULL }
	};
	
	g_option_context_add_main_entries(option_context, option_entries, NULL);
	
	program=gnome_program_init(
					GETTEXT_PACKAGE, PACKAGE_RELEASE_VERSION,
					LIBGNOME_MODULE,
					argc, argv,
					GNOME_PARAM_GOPTION_CONTEXT, option_context,
					GNOME_PARAM_NONE
	);
	
	if(remaining_argv)
		remaining_argc=g_strv_length(remaining_argv)-1;
	
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	
	g_set_application_name(GETTEXT_PACKAGE);
	gtk_window_set_default_icon_name(GETTEXT_PACKAGE);
	
	if(!g_thread_supported()) g_thread_init(NULL);
	
	gtk_init(&argc, &argv);
	
	cache_init();
	
	debug_init();
	
	gconfig_start();
	
	notifing=notify_init(GETTEXT_PACKAGE);
	
	proxy_init();
	online_services_init();
	
	www_init();
	images_init();
	groups_init();
	
	main_window_create();
	
	return TRUE;
}/*program_init();*/

void program_deinit(void){
	/* Close libnotify */
	if(notifing) notify_uninit();
	
	gconfig_shutdown();
	
	online_service_request_unset_selected_update();
	online_services_deinit();
	proxy_deinit();
	
	ipc_deinit();
	
	www_deinit();
	groups_deinit();
	
	images_deinit();
	
	cache_deinit();
	
	xml_parser_deinit();	
	
	debug("**NOTICE:** %s exited", GETTEXT_PACKAGE);
	debug_deinit();
	
	datetime_locale_deinit();
}/*program_deinit();*/

const gchar *program_gtk_response_to_string(gint response){
	switch(response){
		case GTK_RESPONSE_NONE: return _("GTK_RESPONSE_NONE");
		case GTK_RESPONSE_REJECT: return _("GTK_RESPONSE_REJECT");
		case GTK_RESPONSE_ACCEPT: return _("GTK_RESPONSE_ACCEPT");
		case GTK_RESPONSE_DELETE_EVENT: return _("GTK_RESPONSE_DELETE_EVENT");
		case GTK_RESPONSE_OK: return _("GTK_RESPONSE_OK");
		case GTK_RESPONSE_CANCEL: return _("GTK_RESPONSE_CANCEL");
		case GTK_RESPONSE_CLOSE: return _("GTK_RESPONSE_CLOSE");
		case GTK_RESPONSE_YES: return _("GTK_RESPONSE_YES");
		case GTK_RESPONSE_NO: return _("GTK_RESPONSE_NO");
		case GTK_RESPONSE_APPLY: return _("GTK_RESPONSE_APPLY");
		case GTK_RESPONSE_HELP: return _("GTK_RESPONSE_HELP");
		default: return _("UNKNOWN");
	}
}/*program_gtk_response_to_string(response);*/

void program_uber_object_free(gpointer pointer1, ...){
	gpointer *pointer;
	va_list pointers;
	va_start(pointers, pointer1);
	for(pointer=pointer1; pointer; pointer=va_arg(pointers, gpointer)){
		if(! *pointer) continue;
		uber_free(*pointer);
	}
	va_end(pointers);
}/*void program_uber_object_free(pointer1, pointer2, NULL);*/

void program_timeout_remove(guint *id, const gchar *usage){
	if(!(*id > 0)) return;
	
	debug("Stopping %s; timeout id: %d", (G_STR_N_EMPTY(usage) ?usage :"[unidentified pthread timeout]"), (*id));
	g_source_remove((*id));
	*id=0;
}/*program_timeout_remove(&id, _("message"));*/

gchar *program_gdouble_drop_precision(const gdouble gdouble_value){
	gchar *gdouble_str=g_strdup_printf("%f", gdouble_value);
	g_ascii_dtostr(gdouble_str, strlen(gdouble_str), gdouble_value);
	return gdouble_str;
}/*program_gdouble_drop_precision();*/

gboolean program_gtk_widget_get_gboolean_property_value(GtkWidget *widget, const gchar *property){
	if(!(widget && GTK_IS_WIDGET(widget))) return FALSE;
	gboolean value=FALSE;
	g_object_get(widget, property, &value, NULL);
	return value;
}/*program_gtk_widget_get_property(GtkWidget *widget, "has-focus"|"visibility"|"sensitive");
MACROS:
	gtk_widget_is_visible(widget) == program_gtk_widget_get_gboolean_property_value(widget, "visibile")
	gtk_widget_is_sensitive(widget) == program_gtk_widget_get_gboolean_property_value(widget, "sensitive")
	gtk_widget_has_focus(widget) == program_gtk_widget_get_gboolean_property_value(widget, "has-focus")
*/

/********************************************************
 *                       eof                            *
 ********************************************************/

