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
#ifdef HAVE_GNOME_KEYRING
#include "keyring.h"
#endif

#include "main.h"
#include "services-dialog.h"
#include "debug.h"
#include "network.h"
#include "online-services.h"

#define GtkBuilderUI "services-dialog.ui"
#define DEBUG_DOMAIN "OnlineServices"

typedef struct {	
	GtkDialog		*dialog;
	GtkCheckButton		*enabled;
	GtkComboBoxEntry	*urls;
	GtkListStore		*urls_liststore;
	GtkTreeModel		*urls_model;
	GtkComboBox		*service_type_combobox;
	GtkListStore		*service_type_liststore;
	GtkTreeModel		*service_type_model;
	GtkEntry		*username;
	GtkEntry		*password;
	GtkCheckButton		*show_password;
	GtkCheckButton		*auto_connect;
	GtkButton		*apply_button;
} ServicesDialog;
static ServicesDialog	*services_dialog=NULL;
static gboolean		okay_to_exit=TRUE;

static void services_dialog_response_cb(GtkDialog *dialog, gint response, ServicesDialog *services_dialog);
static void services_dialog_destroy_cb(GtkDialog *dialog, ServicesDialog *services_dialog);
static void services_dialog_show_password_cb(GtkCheckButton *show_passwor, ServicesDialog *services_dialog);
static void services_dialog_load_service(void);
static void services_dialog_apply(GtkButton *apply, ServicesDialog *services_dialog);
static void services_dialog_invalid_account(void);


static void services_dialog_response_cb(GtkDialog *dialog, gint response, ServicesDialog *services_dialog){
	if(response == GTK_RESPONSE_CLOSE)
		return gtk_widget_destroy(GTK_WIDGET(dialog));
	
	services_dialog_apply(services_dialog->apply_button, services_dialog);
	
	if(response == GTK_RESPONSE_OK && okay_to_exit )
		gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void services_dialog_invalid_account(void){
	GtkDialog *dialog=(GtkDialog *)gtk_dialog_new_with_buttons(
							"Missing Information",
							GTK_WINDOW(services_dialog->dialog),
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_OK,
							NULL
	);
	GtkVBox *dialog_content=(GtkVBox *)gtk_dialog_get_content_area(dialog);
	GtkLabel *dialog_notice=(GtkLabel *)gtk_label_new("You must enter a valid service url (ie identi.ca), username, and password.");
	gtk_widget_show(GTK_WIDGET(dialog_notice));
	gtk_box_pack_start(
				GTK_BOX(dialog_content),
				GTK_WIDGET(dialog_notice),
				TRUE, TRUE, 0
	);
	gtk_dialog_run(dialog);
}//services_dailog_invalid_account

static void services_dialog_apply(GtkButton *apply_button, ServicesDialog *services_dialog){
	debug(DEBUG_DOMAIN, "Saving services.");
	gchar *url=gtk_combo_box_get_active_text(GTK_COMBO_BOX(services_dialog->urls));
	const gchar *username=gtk_entry_get_text(GTK_ENTRY(services_dialog->username));
	const gchar *password=gtk_entry_get_text(GTK_ENTRY(services_dialog->password));
	if(!( url && username && password )){
		debug(DEBUG_DOMAIN, "Failed to save current account, server and/or password missing.");
		services_dialog_invalid_account();
		okay_to_exit=FALSE;
		g_free(url);
		return;
	}
	
	GtkTreeIter		*iter=g_new0(GtkTreeIter, 1);
	OnlineService		*service=NULL;
	
	debug(DEBUG_DOMAIN, "Retriving current service.");
	if(!( (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(services_dialog->urls), iter)) )){
		debug(DEBUG_DOMAIN, "Current service does not have a valid GtkTreeIter, service will be appended later.");
	}else{
		gtk_tree_model_get(
					services_dialog->urls_model,
					iter,
					OnlineServicePointer, &service,
					-1
		);
		debug(DEBUG_DOMAIN, "Saving existing service: '%s' as: '%s@%s'.", service->key, username, url);
	}

	
	online_services_save(
				online_services,
				service,
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(services_dialog->enabled)),
				(const gchar *)url,
				username,
				password,
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(services_dialog->auto_connect))
	);
	
	if(!iter){
		debug(DEBUG_DOMAIN, "Saving new service: '%s'.", service->key);
		gtk_list_store_append(services_dialog->urls_liststore, iter);
		gtk_list_store_set(services_dialog->urls_liststore, iter,
					UrlString, url,
					OnlineServicePointer, service,
					-1
		);
		debug(DEBUG_DOMAIN, "New service saved: '%s'.", service->key);
	}
	okay_to_exit=TRUE;
	g_free(url);
}

static void services_dialog_destroy_cb(GtkDialog *dialog, ServicesDialog *services_dialog){
	g_free(services_dialog);
	services_dialog=NULL;
}

static void services_dialog_show_password_cb(GtkCheckButton *show_password, ServicesDialog *services_dialog){
	gboolean visible;

	visible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(services_dialog->show_password));
	gtk_entry_set_visibility(GTK_ENTRY(services_dialog->password), visible);
}

void services_dialog_show(GtkWindow *parent){
	GtkBuilder		*ui;
	
	if(services_dialog){
		debug(DEBUG_DOMAIN, "Service Dialog exists, presenting.");
		gtk_window_present(GTK_WINDOW(services_dialog->dialog));
		return;
	}
	
	debug(DEBUG_DOMAIN, "Creating Services Dialog from: '%s'.", GtkBuilderUI);
	services_dialog=g_new0(ServicesDialog, 1);
	
	/* Get widgets */
	ui=gtkbuilder_get_file(
				GtkBuilderUI,
					"services_dialog", &services_dialog->dialog,
					"service_enabled", &services_dialog->enabled,
					"urls", &services_dialog->urls,
					"urls_liststore", &services_dialog->urls_liststore,
					"username_entry", &services_dialog->username,
					"password_entry", &services_dialog->password,
					"show_password_checkbutton", &services_dialog->show_password,
					"autoconnect_checkbutton", &services_dialog->auto_connect,
					"apply_button", &services_dialog->apply_button,
				NULL
	);
	debug(DEBUG_DOMAIN, "UI loaded... setting services tree view model.");
	services_dialog->urls_model=gtk_combo_box_get_model(GTK_COMBO_BOX(services_dialog->urls));
	
	/* Connect the signals */
	debug(DEBUG_DOMAIN, "Services tree view model retrieved... setting signal handlers.");
	gtkbuilder_connect(ui, services_dialog,
				"services_dialog", "destroy", services_dialog_destroy_cb,
				"services_dialog", "response", services_dialog_response_cb,
				"show_password_checkbutton", "toggled", services_dialog_show_password_cb,
				"urls", "changed", services_dialog_load_service,
			NULL
	);
	
	g_object_unref(ui);
	
	debug(DEBUG_DOMAIN, "Signal handlers set... loading accounts.");
	if(!( online_services_fill_liststore(online_services, services_dialog->urls_liststore) ))
		debug(DEBUG_DOMAIN, "No services found to load, new accounts need to be setup.");
	else{
		debug(DEBUG_DOMAIN, "OnlineServices found & loaded.  Selecting active service.");
		gtk_combo_box_entry_set_text_column(services_dialog->urls, 0);
		gtk_combo_box_set_active(GTK_COMBO_BOX(services_dialog->urls), 0);
		services_dialog_load_service();
	}
	
	if(parent){
		g_object_add_weak_pointer(G_OBJECT(services_dialog->dialog),(gpointer) &services_dialog);
		gtk_window_set_transient_for(GTK_WINDOW(services_dialog->dialog), parent);
	}
	gtk_window_present(GTK_WINDOW(services_dialog->dialog));
	
	debug(DEBUG_DOMAIN, "Loading default service.");
}

static void services_dialog_load_service(void){
	if(!services_dialog) return;
	/*gchar *url=gtk_combo_box_get_active_text(GTK_COMBO_BOX(services_dialog->urls));
	const gchar *username=gtk_entry_get_text(GTK_ENTRY(services_dialog->username));
	if(G_STR_EMPTY(url) || G_STR_EMPTY(username) ){
		debug(DEBUG_DOMAIN, "Server or Username missing, unable to load service.");
		g_free(url);
		return;
	}
	*/
	GtkTreeIter		*iter=g_new0(GtkTreeIter, 1);
	OnlineService		*service=NULL;
	
	if(!( (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(services_dialog->urls), iter)) ))
		return;
	
	gtk_tree_model_get(
			services_dialog->urls_model,
			iter,
			OnlineServicePointer, &service,
			-1
	);
	
	debug(DEBUG_DOMAIN, "Loading account information for '%s'.", service->key);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(services_dialog->enabled), service->enabled);
	gtk_entry_set_text(GTK_ENTRY(services_dialog->username), service->username ? service->username : "");
	gtk_entry_set_text(GTK_ENTRY(services_dialog->password), service->password ? service->password : "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(services_dialog->auto_connect), service->auto_connect);
	
	g_free(iter);
}//services_dialog_load_service
