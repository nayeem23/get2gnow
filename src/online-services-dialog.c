/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * Copyright(C) 2007-2008 Brian Pepple <bpepple@fedoraproject.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include "gconfig.h"
#include "gtkbuilder.h"

#include "main.h"
#include "online-services-dialog.h"
#include "network.h"
#include "online-services-typedefs.h"
#include "online-services.h"
#include "online-service.h"
#include "online-service-request.h"

#include "preferences.h"


#define GtkBuilderUI "online-services-dialog.ui"

#define DEBUG_DOMAINS "OnlineServices:UI:GtkBuilder:GtkBuildable:Requests:Authentication:Preferences:Accounts:Settings:Setup:OnlineServicesDialog.c"
#include "debug.h"

#define PREFS_ACCOUNT_DELETE	GCONF_PATH "/auth/dont_confirm_delete"

typedef struct {	
	GtkDialog		*online_services_dialog;
	GtkCheckButton		*enabled;
	
	GtkComboBoxEntry	*urls;
	GtkListStore		*urls_liststore;
	GtkTreeModel		*urls_model;
	
	GtkButton		*online_service_new_button;
	GtkButton		*online_service_delete_button;
	
	GtkToggleButton		*https_toggle_button;
	
	/*currently unused
	GtkComboBox		*service_type_combobox;
	GtkListStore		*service_type_liststore;
	GtkTreeModel		*service_type_model;
	*/
	
	GtkEntry		*username;
	GtkEntry		*password;
	GtkCheckButton		*show_password;
	GtkCheckButton		*auto_connect;
	
	GtkButton		*online_service_connect_button;
	
	/*Buttons in services-dialog 'action-area.*/
	GtkButton		*online_services_okay_button;
	GtkButton		*online_services_save_button;
} OnlineServicesDialog;

static OnlineServicesDialog	*online_services_dialog=NULL;
static gboolean		okay_to_exit=TRUE;

static void online_services_dialog_response(GtkDialog *dialog, gint response, OnlineServicesDialog *services);
static void online_services_dialog_destroy(GtkDialog *dialog);
static void online_services_dialog_show_password(GtkCheckButton *show_password, OnlineServicesDialog *online_services_dialog);
static void online_services_dialog_load_service(GtkComboBoxEntry *urls, OnlineServicesDialog *online_services_dialog);

static OnlineService *online_services_dialog_get_active_service(OnlineServicesDialog *online_services_dialog);
static void online_services_dialog_new_service(GtkButton *online_service_new_button, OnlineServicesDialog *online_services_dialog);
static void online_services_dialog_save_service(GtkButton *online_services_save_button, OnlineServicesDialog *online_services_dialog);
static void online_services_dialog_connect(GtkButton *online_service_connect_button, OnlineServicesDialog *online_services_dialog);
static void online_services_dialog_delete_service(GtkButton *service_delete_button, OnlineServicesDialog *online_services_dialog);

static void online_services_dialog_check(GtkEntry *entry, GdkEventKey *event, OnlineServicesDialog *online_services_dialog);
static void online_services_dialog_check_service(OnlineServicesDialog *online_services_dialog);

static void online_services_dialog_setup(GtkWindow *parent);

static void online_services_dialog_response(GtkDialog *dialog, gint response, OnlineServicesDialog *online_services_dialog){
	debug("OnlineServicesDialog has received a response... changes will be now be saved.");
	switch(response){
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_REJECT:
		case GTK_RESPONSE_DELETE_EVENT:
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_NO:
			gtk_widget_destroy(GTK_WIDGET(dialog));
			online_services_dialog=NULL;
			return;
	}
	
	online_services_dialog_save_service(online_services_dialog->online_services_save_button, online_services_dialog);
	
	if(response==GTK_RESPONSE_OK && okay_to_exit ){
		gtk_widget_destroy(GTK_WIDGET(dialog));
		online_services_dialog=NULL;
	}
}/*online_services_dialog_response*/

static void online_services_dialog_new_service(GtkButton *online_service_new_button, OnlineServicesDialog *online_services_dialog){
	debug("Preparing OnlineServicesDialog for a new account setup.");
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(online_services_dialog->enabled), TRUE);
	gtk_toggle_button_set_active(online_services_dialog->https_toggle_button, TRUE);
	gtk_entry_set_text(GTK_ENTRY(online_services_dialog->username), "");
	gtk_entry_set_text(GTK_ENTRY(online_services_dialog->password), "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(online_services_dialog->auto_connect), TRUE);
	
	if(!(online_services_dialog_get_active_service(online_services_dialog))) return;
	
	gtk_entry_set_text( GTK_ENTRY( GTK_BIN( online_services_dialog->urls)->child), "");
}/*online_services_dialog_new_service(online_services_dialog->online_service_new_button, online_services_dialog);*/

static void online_services_dialog_save_service(GtkButton *save_button, OnlineServicesDialog *online_services_dialog){
	debug("Saving services.");
	okay_to_exit=FALSE;
	
	gchar *uri=gtk_combo_box_get_active_text(GTK_COMBO_BOX(online_services_dialog->urls));
	const gchar *username=gtk_entry_get_text(GTK_ENTRY(online_services_dialog->username));
	const gchar *password=gtk_entry_get_text(GTK_ENTRY(online_services_dialog->password));
	if(!( G_STR_N_EMPTY(uri) && G_STR_N_EMPTY(username) && G_STR_N_EMPTY(password) )){
		debug("Failed to save current account.  Server, username, and/or password missing.");
		okay_to_exit=TRUE;
		g_free(uri);
		return;
	}
	
	OnlineService		*service=NULL;
	gboolean		new_service=FALSE;
	
	debug("Retriving current service.");
	if(!( (service=online_services_dialog_get_active_service(online_services_dialog)) )){
		debug("Current service does not have a valid GtkTreeIter, service will be appended later.");
		new_service=TRUE;
	}
	
	
	if(!( (service=online_services_save_service(
				online_services,
				service,
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(online_services_dialog->enabled)),
				(const gchar *)uri,
				gtk_toggle_button_get_active(online_services_dialog->https_toggle_button),
				username,
				password,
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(online_services_dialog->auto_connect))
	)) )){
		debug("**ERROR:** Failed to save online service for '%s@%s' please see above for further details.", username, uri);
		g_free(uri);
		return;
	}
	
	if(!new_service){
		okay_to_exit=TRUE;
		g_free(uri);
		return;
	}
	
	if(!( service && online_service_get_key(service) && online_service_get_uri(service) && online_service_get_username(service) )){
		debug("**ERROR:** OnlineServices saved resulting OnlineService is invalid.");
		g_free(uri);
		return;
	}
	
	GtkTreeIter *iter=g_new0(GtkTreeIter, 1);
	debug("Storing new service: '%s'.", online_service_get_key(service));
	gtk_list_store_append(online_services_dialog->urls_liststore, iter);
	gtk_list_store_set(
				online_services_dialog->urls_liststore, iter,
					UrlString, uri,
					OnlineServicePointer, service,
				-1
	);
	
	g_free(uri);
	
	debug("New service stored, selecting it new loction in the dialog.");
	okay_to_exit=TRUE;
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(online_services_dialog->urls), iter);
	uber_free(iter);
}/*online_services_dialog_save_service(online_services_dialog->online_services_save_button, online_services_dialog);*/

static void online_services_dialog_connect(GtkButton *connect_button, OnlineServicesDialog *online_services_dialog){
	OnlineService *service=NULL;
	
	gboolean enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(online_services_dialog->enabled));
	gboolean https=gtk_toggle_button_get_active(online_services_dialog->https_toggle_button);
	
	gchar *uri=gtk_combo_box_get_active_text(GTK_COMBO_BOX(online_services_dialog->urls));
	const gchar *username=gtk_entry_get_text(GTK_ENTRY(online_services_dialog->username));
	const gchar *password=gtk_entry_get_text(GTK_ENTRY(online_services_dialog->password));
	
	gboolean auto_connect=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(online_services_dialog->auto_connect));
	
	if(!(service=online_services_dialog_get_active_service(online_services_dialog))){
		debug("OnlineService is not saved.  Creating temporary OnlineService to connect with.");
		service=online_service_new(enabled, uri, https, username, password, auto_connect);
		online_service_connect(service);
		online_service_login(service, TRUE);
	}else if( service && G_STR_N_EMPTY(uri) && g_str_equal(online_service_get_uri(service), uri) && G_STR_N_EMPTY(username) && g_str_equal(online_service_get_username(service), username) && G_STR_N_EMPTY(password) && g_str_equal(online_service_get_password(service), password) ){
		if(online_service_is_connected(service)){
			debug("OnlineService: %s is already connected.", online_service_get_key(service));
			return;
		}
		online_service_connect(service);
		online_service_login(service, TRUE);
	}
}/*online_services_dialog_connect(online_services_dialog->online_service_connect_button, online_services_dialog);*/

static void online_services_dialog_delete_service(GtkButton *online_service_delete_button, OnlineServicesDialog *online_services_dialog){
	OnlineService		*service=NULL;
	
	debug("Retriving current service.");
	if(!( (service=online_services_dialog_get_active_service(online_services_dialog)) )){
		debug("There is no valid account selected.");
		return;
	}
	
	gchar *confirm_message=g_strdup_printf("%s: %s?", _("Are you sure you want to delete"), online_service_get_key(service));
	if(!(online_service_request_popup_confirmation_dialog(
					PREFS_ACCOUNT_DELETE,
					confirm_message,
					_("You will no longer be able to send or recieve messages using this account.\n\nIf you want to use this service again you will have to set it up again."),
					NULL, NULL
	))){
		uber_free(confirm_message);
		debug("Account deletion was canceled by the user.");
		return;
	}

	uber_free(confirm_message);
	
	debug("Deleting service: '%s'.", online_service_get_key(service));
	online_services_delete_service(online_services, service);
	
	debug("Reloading OnlineServices into OnlineServicesDialog 'urls_list_store.");
	online_services_combo_box_fill(online_services, GTK_COMBO_BOX(online_services_dialog->urls), online_services_dialog->urls_liststore, FALSE);
	online_services_dialog_new_service(online_services_dialog->online_service_new_button, online_services_dialog);
}/*online_services_dialog_delete_service(online_services_dialog->online_service_delete_button, online_services_dialog);*/

static void online_services_dialog_destroy(GtkDialog *dialog){
	uber_free(online_services_dialog);
}/*online_services_dialog_destroy*/

static void online_services_dialog_show_password(GtkCheckButton *show_password, OnlineServicesDialog *online_services_dialog){
	gboolean visible;

	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(online_services_dialog->show_password));
	gtk_entry_set_visibility(GTK_ENTRY(online_services_dialog->password), visible);
}/*online_services_dialog_show_password*/

static OnlineService *online_services_dialog_get_active_service(OnlineServicesDialog *online_services_dialog){
	GtkTreeIter		*iter=g_new0(GtkTreeIter, 1);
	OnlineService		*service=NULL;
	
	if(!( (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(online_services_dialog->urls), iter)) )){
		return NULL;
	}
	
	gtk_tree_model_get(
				online_services_dialog->urls_model, iter,
					OnlineServicePointer, &service,
				-1
	);
	if(!(service && online_service_get_key(service) && online_service_get_uri(service) && online_service_get_username(service)))
		return NULL;
	
	return service;
}/*online_services_dialog_get_active_service*/

static void online_services_dialog_check(GtkEntry *entry, GdkEventKey *event, OnlineServicesDialog *online_services_dialog){
	online_services_dialog_check_service(online_services_dialog);
}/*online_services_dialog_check*/

static void online_services_dialog_check_service(OnlineServicesDialog *online_services_dialog){
	gboolean		service_okay_to_save=FALSE, service_exists=FALSE, service_is_saved=FALSE, service_is_connectable=FALSE;
	OnlineService		*service=online_services_dialog_get_active_service(online_services_dialog);
	
	gchar *uri=gtk_combo_box_get_active_text(GTK_COMBO_BOX(online_services_dialog->urls));
	const gchar *username=gtk_entry_get_text(GTK_ENTRY(online_services_dialog->username));
	const gchar *password=gtk_entry_get_text(GTK_ENTRY(online_services_dialog->password));
	
	if(service){
		debug("Service dialog is editing an existing service: <%s>.", online_service_get_uri(service));
		service_exists=TRUE;
	}
	
	if( service && G_STR_N_EMPTY(uri) && g_str_equal(online_service_get_uri(service), uri) && G_STR_N_EMPTY(username) && g_str_equal(online_service_get_username(service), username) && G_STR_N_EMPTY(password) && g_str_equal(online_service_get_password(service), password) ){
		debug("Services is up to date, no changes need saved.");
		service_is_saved=TRUE;
		service_okay_to_save=FALSE;
	}else if( service && G_STR_N_EMPTY(uri) && !g_str_equal(online_service_get_uri(service), uri) && G_STR_N_EMPTY(username) && !g_str_equal(online_service_get_username(service), username) && !G_STR_N_EMPTY(password) && !g_str_equal(online_service_get_password(service), password) ){
		debug("Existing service has changes that need to be saved.");
		service_okay_to_save=TRUE;
	}else if(!service && (G_STR_N_EMPTY(uri) && G_STR_N_EMPTY(username) && G_STR_N_EMPTY(password)) ){
		debug("Service Dialog is creating a OnlineService & all needed fields have data so the service may be saved.");
		service_okay_to_save=TRUE;
	}
	
	debug("%s 'service_delete_button'.", (service_exists ?_("Enabling") :_("Disabling")) );
	gtk_widget_set_sensitive(GTK_WIDGET(online_services_dialog->online_service_delete_button), service_exists);
	
	debug("%s 'service_new_button'.", ((service_okay_to_save) ?_("Disabling") :_("Enabling")) );
	gtk_widget_set_sensitive(GTK_WIDGET(online_services_dialog->online_service_new_button), !service_okay_to_save);
	
	service_is_connectable=((service_is_saved && !online_service_is_connected(service)) || service_okay_to_save);
	debug("%s 'online_service_connect_button'.", (service_is_connectable ?_("Enabling") :_("Disabling")) );
	gtk_widget_set_sensitive(GTK_WIDGET(online_services_dialog->online_service_connect_button), service_is_connectable);
	
	debug("%s 'save_button' & 'okay_button'.", (service_okay_to_save ?_("Enabling") :_("Disabling")) );
	gtk_widget_set_sensitive(GTK_WIDGET(online_services_dialog->online_services_save_button), service_okay_to_save);
	gtk_widget_set_sensitive(GTK_WIDGET(online_services_dialog->online_services_okay_button), service_okay_to_save);
}/*online_services_dialog_check*/

void online_services_dialog_show(GtkWindow *parent){
	if(!(online_services_dialog && online_services_dialog->online_services_dialog ))
		return online_services_dialog_setup(parent);
	
	debug("Service Dialog exists, presenting.");
	gtk_window_present(GTK_WINDOW(online_services_dialog->online_services_dialog));
}/*online_services_dialog_show*/


static void online_services_dialog_setup(GtkWindow *parent){
	GtkBuilder		*ui;
	
	debug("Creating Services Dialog from: '%s'.", GtkBuilderUI);
	online_services_dialog=g_new0(OnlineServicesDialog, 1);
	
	/* Get widgets */
	ui=gtkbuilder_get_file(
				GtkBuilderUI,
					"online_services_dialog", &online_services_dialog->online_services_dialog,
					"online_service_enabled", &online_services_dialog->enabled,
					
					"urls", &online_services_dialog->urls,
					"urls_liststore", &online_services_dialog->urls_liststore,
					
					"online_service_new_button", &online_services_dialog->online_service_new_button,
					"online_service_delete_button", &online_services_dialog->online_service_delete_button,
					
					"https_toggle_button", &online_services_dialog->https_toggle_button,
					
					"username_entry", &online_services_dialog->username,
					"password_entry", &online_services_dialog->password,
					
					"show_password_checkbutton", &online_services_dialog->show_password,
					"autoconnect_checkbutton", &online_services_dialog->auto_connect,
					
					"online_service_connect_button", &online_services_dialog->online_service_connect_button,
					
					"online_service_save_button", &online_services_dialog->online_services_save_button,
					"online_services_okay_button", &online_services_dialog->online_services_okay_button,
				NULL
	);
	
	debug("UI loaded... setting services tree view model.");
	online_services_dialog->urls_model=gtk_combo_box_get_model(GTK_COMBO_BOX(online_services_dialog->urls));
	
	/* Connect the signals */
	debug("Services tree view model retrieved... setting signal handlers.");
	gtkbuilder_connect(ui, online_services_dialog,
				"online_services_dialog", "destroy", online_services_dialog_destroy,
				"online_services_dialog", "response", online_services_dialog_response,
				
				"urls", "changed", online_services_dialog_load_service,
				
				"online_service_new_button", "clicked", online_services_dialog_new_service,
				"online_service_delete_button", "clicked", online_services_dialog_delete_service,
				
				"username_entry", "key-release-event", online_services_dialog_check,
				"password_entry", "key-release-event", online_services_dialog_check,
				
				"online_service_connect_button", "clicked", online_services_dialog_connect,
				
				"show_password_checkbutton", "toggled", online_services_dialog_show_password,
			NULL
	);
	
	g_object_unref(ui);
	
	debug("Signal handlers set... loading accounts.");
	gtk_combo_box_entry_set_text_column(online_services_dialog->urls, UrlString);
	if(!( online_services_combo_box_fill(online_services, GTK_COMBO_BOX(online_services_dialog->urls), online_services_dialog->urls_liststore, FALSE) )){
		debug("No services found to load, new accounts need to be setup.");
		online_services_dialog_new_service(online_services_dialog->online_service_new_button, online_services_dialog);
	}else
		debug("OnlineServices found & loaded.  Selecting active service.");
	
	if(parent){
		g_object_add_weak_pointer(G_OBJECT(online_services_dialog->online_services_dialog), (gpointer)&online_services_dialog);
		gtk_window_set_transient_for(GTK_WINDOW(online_services_dialog->online_services_dialog), parent);
	}
	gtk_window_present(GTK_WINDOW(online_services_dialog->online_services_dialog));
	
	debug("Loading default service.");
}

static void online_services_dialog_load_service(GtkComboBoxEntry *urls, OnlineServicesDialog *online_services_dialog){
	if(!online_services_dialog) return;
	OnlineService		*service=NULL;
	
	if(!( (service=online_services_dialog_get_active_service(online_services_dialog)) )){
		gchar *uri=gtk_combo_box_get_active_text(GTK_COMBO_BOX(online_services_dialog->urls));
		if(G_STR_EMPTY(uri) || g_str_equal(uri, "[new account]") )
			online_services_dialog_new_service(online_services_dialog->online_service_new_button, online_services_dialog);
		g_free(uri);
		online_services_dialog_check_service(online_services_dialog);
		return;
	}
	
	if(!( online_service_get_key(service) && online_service_get_username(service) )){
		debug("Unable to load valid account information from 'urls_liststore'.");
		online_services_dialog_check_service(online_services_dialog);
		return;
	}
	
	debug("Accounts dialog loaded OnlineService.\n\t\taccount '%s(=%s)'\t\t\t[%sabled]\n\t\tservice url: %s; username: %s; password: %s; auto_connect: [%s]", online_service_get_key(service), online_service_get_key(service), (online_service_is_enabled(service)?"en":"dis"), online_service_get_uri(service), online_service_get_username(service), online_service_get_password(service), (online_service_is_auto_connected(service)?"TRUE":"FALSE"));
	
	online_services_dialog_check_service(online_services_dialog);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(online_services_dialog->enabled), online_service_is_enabled(service));
	gtk_toggle_button_set_active(online_services_dialog->https_toggle_button, online_service_is_secure(service));
	gtk_entry_set_text(GTK_ENTRY(online_services_dialog->username), online_service_get_username(service) );
	gtk_entry_set_text(GTK_ENTRY(online_services_dialog->password), online_service_get_password(service) );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(online_services_dialog->auto_connect), online_service_is_auto_connected(service));
	online_services_dialog_check_service(online_services_dialog);
}/*online_services_dialog_load_service*/