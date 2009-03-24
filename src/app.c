/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * Copyright (C) 2009 Kaity G. B. <uberChick@uberChicGeekChick.Com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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
 *
 * Authors: Kaity G. B. <uberChick@uberChicGeekChick.Com>
 */

#include <config.h>
#include <sys/stat.h>
#include <string.h>

#include <canberra-gtk.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libnotify/notify.h>

#include "debug.h"
#include "gconf.h"
#include "paths.h"
#include "glade.h"
#ifdef HAVE_GNOME_KEYRING
#include "keyring.h"
#endif

#include "main.h"
#include "about.h"
#include "account-dialog.h"
#include "add-dialog.h"
#include "app.h"
#include "geometry.h"
#include "hint.h"
#include "label.h"
#include "network.h"
#include "preferences.h"
#include "send-message-dialog.h"
#include "friends-manager.h"
#include "followers-dialog.h"
#include "ui-utils.h"
#include "tweet-list.h"

#ifdef HAVE_DBUS
#include "dbus.h"
#include <dbus/dbus-glib.h>
#endif

#define GET_PRIV(obj)(G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_APP, AppPriv ))

#define DEBUG_DOMAIN_SETUP       "AppSetup"
#define GLADE_FILE	"main_window.xml"
#define DEBUG_QUIT

#define TYPE_TWITTER "twitter"

struct _AppPriv {
	/* Main widgets */
	GtkWidget         *window;
	TweetList   *listview;
	GtkWidget         *statusbar;

	/*
	 * Widgets that are enabled when
	 * we are connected/disconnected
	 */
	GList             *widgets_connected;
	GList             *widgets_disconnected;

	/* Timeline menu items */
	GSList            *group;
	GtkRadioAction    *menu_combined;
	GtkRadioAction    *menu_public;
	GtkRadioAction    *menu_friends;
	GtkRadioAction    *menu_mine;
	GtkRadioAction    *menu_twitux;
	GtkRadioAction    *menu_direct_messages;
	GtkRadioAction    *menu_direct_replies;

	GtkAction         *view_friends;

	/* Status Icon */
	GtkStatusIcon     *status_icon;

	/* Status Icon Popup Menu */
	GtkWidget         *popup_menu;
	GtkToggleAction   *popup_menu_show_app;

	/* Account related data */
	DBusGProxy        *accounts_service;
	DBusGProxy        *account;
	char              *username;
	char              *password;

	/* Misc */
	guint              size_timeout_id;
	
	/* Expand messages widgets */
	GtkWidget         *expand_box;
	GtkWidget         *expand_image;
GtkWidget         *expand_title;
	GtkWidget         *expand_label;
};

static void	    app_class_init			(AppClass        *klass);
static void     app_init			(App             *app);
static void     app_finalize(GObject               *object);
static void     restore_main_window_geometry(GtkWidget             *main_window);
static void     on_accounts_destroy(DBusGProxy            *proxy, 
                                                  App             *app);
static void     disconnect(App             *app);
static void     reconnect(App             *app);
static gboolean update_account(DBusGProxy            *account,
                                                  App             *app,
                                                  GError **error);

static void     on_account_changed(DBusGProxy *account, App *app); 
static void     on_account_disabled(DBusGProxy *accounts,
					                              const char *opath,
					                              App *app);
static void     on_account_enabled(DBusGProxy *accounts,
					                              const char *opath,
					                              App *app);
static void     app_setup(void);
static void     main_window_destroy_cb(GtkWidget *window, App *app); 
static gboolean main_window_delete_event_cb(GtkWidget             *window,
												  GdkEvent              *event,
												  App             *app);
static void     app_set_radio_group(App *app, GtkBuilder *ui); 
static void     app_connect_cb(GtkWidget *window, App *app); 
static void     app_disconnect_cb(GtkWidget *window, App *app); 
static void     app_new_message_cb(GtkWidget *window, App *app); 
static void     app_send_direct_message_cb(GtkWidget *window, App *app); 
static void     app_quit_cb(GtkWidget *window, App *app); 
static void     app_refresh_cb(GtkWidget *window, App *app); 
static void     app_account_cb(GtkWidget *window, App *app); 
static void app_friends_manager_cb(GtkWidget *widget, App *app);
static void     app_preferences_cb(GtkWidget *window, App *app); 
static void app_combined_timeline_cb(GtkRadioAction *action, GtkRadioAction *current, App *app);
static void app_public_timeline_cb(GtkRadioAction *action, GtkRadioAction *current, App *app);
static void     app_friends_timeline_cb(GtkRadioAction        *action,
												  GtkRadioAction        *current,
												  App             *app);
static void     app_mine_timeline_cb(GtkRadioAction        *action,
												  GtkRadioAction        *current,
												  App             *app);
static void     app_view_direct_messages_cb(GtkRadioAction        *action,
												  GtkRadioAction        *current,
												  App             *app);
static void     app_view_direct_replies_cb(GtkRadioAction        *action,
												  GtkRadioAction        *current,
												  App             *app);
static void     app_view_friends_cb(GtkAction *action, App *app); 
static void     app_about_cb(GtkWidget *window, App *app); 
static void     app_help_contents_cb(GtkWidget *widget, App *app); 
static void     app_status_icon_activate_cb(GtkStatusIcon *status_icon, App *app); 
static void     app_status_icon_popup_menu_cb(GtkStatusIcon         *status_icon,
												  guint                  button,
												  guint                  activate_time,
												  App             *app);
static gboolean request_accounts(App *a, GError **error);
static void     app_add_friend_cb(GtkAction *menuitem, App *app); 
static void     app_connection_items_setup(App *app, GtkBuilder *ui); 
static void     app_login(App             *app);
static void app_set_default_timeline(App *app, gchar *timeline);
static void     app_retrieve_default_timeline(void);
static void     app_status_icon_create_menu(void);
static void     app_status_icon_create(void);
static void     app_check_dir(void);
static void     app_toggle_visibility(void);
static gboolean configure_event_timeout_cb(GtkWidget             *widget);
static gboolean app_window_configure_event_cb(GtkWidget             *widget,
												  GdkEventConfigure     *event,
												  App             *app);

static App  *app = NULL;
static AppPriv *app_priv=NULL;

G_DEFINE_TYPE(App, app, G_TYPE_OBJECT);

static void
app_class_init(AppClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = app_finalize;

	g_type_class_add_private(object_class, sizeof(AppPriv));
}

static void
app_init(App *singleton_app)
{
	//AppPriv      *priv;	
	app = singleton_app;

	app_priv=GET_PRIV(app);
	app_priv->widgets_connected = NULL;
	app_priv->widgets_disconnected = NULL;
	app_priv->group = NULL;
}

static void
app_finalize(GObject *object)
{
	App	       *app;
	//AppPriv      *priv;	
	
	app = APP(object);
	app_priv = GET_PRIV(app);

	if(app_priv->size_timeout_id) {
		g_source_remove(app_priv->size_timeout_id);
	}

	g_list_free(app_priv->widgets_connected);
	g_list_free(app_priv->widgets_disconnected);
	g_slist_free(app_priv->group);

#ifdef HAVE_DBUS
	dbus_nm_finalize();
#endif

	conf_shutdown();
	
	G_OBJECT_CLASS(app_parent_class)->finalize(object);
}

static void
restore_main_window_geometry(GtkWidget *main_window)
{
	geometry_load_for_main_window(main_window);
}

static void
on_accounts_destroy(DBusGProxy *proxy, App *app)
{
		app_priv->account = NULL;
	app_priv->accounts_service = NULL;
}

static void
disconnect(App *app)
{
	GtkListStore *store = tweet_list_get_store();
	gtk_list_store_clear(store);
	network_logout();
	app_state_on_connection(FALSE);
}

static void
reconnect(App *app)
{
	Conf *conf;
	gboolean login;

	disconnect(app);
	conf = conf_get();

	/*Check to see if we should automatically login */
	conf_get_bool(conf,
						  PREFS_AUTH_AUTO_LOGIN,
						  &login);

	if(!login)
		return;

	app_login(app);
}

static gboolean
update_account(DBusGProxy  *account,
				App   *app,
				GError     **error)
{
		gchar         *username;
	gchar         *password;


	
	/* the account is changing */
	if(!app_priv->account) {
		app_priv->account = account;
		dbus_g_proxy_add_signal(app_priv->account, "Changed", G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(app_priv->account, "Changed", G_CALLBACK(on_account_changed), app, NULL);
	}else if((strcmp((g_strdup((dbus_g_proxy_get_path(account)) )),(g_strdup((dbus_g_proxy_get_path(app_priv->account)) )) )) != 0 ) {
		g_object_unref(app_priv->account);
		app_priv->account=NULL;
	}
  

	/* The username should normally never change on an existing account, but let's update
	 * it in any case. Since we are only interested in the enabled account,
	 * and are connected to an account disabled signal, we don't need to check the
	 * enabled flag here. */
	if(!dbus_g_proxy_call(app_priv->account,
						"GetUsername",
						error,
						G_TYPE_INVALID,
						G_TYPE_STRING,
						&username,
						G_TYPE_INVALID)){
return FALSE;
	}

	if(!dbus_g_proxy_call(app_priv->account,
							"GetPassword",
							error,
							G_TYPE_INVALID,
							G_TYPE_STRING,
							&password,
							G_TYPE_INVALID)){
return FALSE;
	}

	app_priv->username=username;
	app_priv->password=password;
return TRUE;
}               

static void
on_account_changed(DBusGProxy *account,
					App  *app)
{
		GError        *error = NULL;

	
	/* we shouldn't be getting a signal about an account that is no longer current, 
	 * but let's do these checks just in case */
	if(!app_priv->account || strcmp(g_strdup(dbus_g_proxy_get_path(account)),
								  g_strdup(dbus_g_proxy_get_path(app_priv->account))) != 0){
return;
	}

	if(!update_account(account, app, &error)) {
		g_printerr("failed to update an account that changed: %s", error->message);
return; 
	}

	reconnect(app);
}

static void
on_account_disabled(DBusGProxy *accounts,
					 const char *opath,
					 App  *app)
{
		GError        *error = NULL;

	
	if(!app_priv->account || strcmp(opath, g_strdup(dbus_g_proxy_get_path(app_priv->account))) != 0){
return;
	}

	
	if(!request_accounts(app, &error)) {
		g_warning("Failed to get accounts: %s", error->message);
		g_clear_error(&error);
return;
	}

	reconnect(app);
}

static void
on_account_enabled(DBusGProxy *accounts,
					const char *opath,
					App  *app)
{
	GError *error = NULL;
	
	DBusGProxy *account;

	account = dbus_g_proxy_new_for_name_owner(dbus_g_bus_get(DBUS_BUS_SESSION, NULL), "org.gnome.OnlineAccounts", opath, "org.gnome.OnlineAccounts", &error);

	if(!account) {
	        g_warning("Could not get an account object for opath: %s", opath);
		g_error_free(error);
		return;
	}

	if(!update_account(account, app, &error)) {
		g_printerr("failed to update an account that got enabled: %s", error->message);
		g_error_free(error);
		return;
	}
	if(error)
		g_error_free(error);
	
	reconnect(app);
}
	
static void
app_setup(void)
{
		Conf		 *conf;
	GtkBuilder       *ui;
	GtkWidget        *scrolled_window;
	GtkWidget        *expand_vbox;
	gchar            *timeline;
	gboolean          login;
	gboolean		  hidden;

	GError           *error = NULL;
	guint32           result;
	DBusGProxy       *session_bus;

	debug(DEBUG_DOMAIN_SETUP, "Beginning....");

	
	/* Set up interface */
	debug(DEBUG_DOMAIN_SETUP, "Initialising interface");
	ui = glade_get_file(GLADE_FILE,
							  "main_window", &app_priv->window,
							  "main_scrolledwindow", &scrolled_window,
							  "main_statusbar", &app_priv->statusbar,
							  "view_combined_timeline", &app_priv->menu_combined,
							  "view_public_timeline", &app_priv->menu_public,
							  "view_friends_timeline", &app_priv->menu_friends,
							  "view_my_timeline", &app_priv->menu_mine,
							  "view_direct_messages", &app_priv->menu_direct_messages,
							  "view_direct_replies", &app_priv->menu_direct_replies,
							  "view_friends", &app_priv->view_friends,
							  "expand_box", &app_priv->expand_box,
							  "expand_vbox", &expand_vbox,
							  "expand_image", &app_priv->expand_image,
							  "expand_title", &app_priv->expand_title,
							  NULL);

	/* Set group for menu radio actions */
	app_set_radio_group(app, ui);

	/* Grab the conf object */
	conf = conf_get();

	/*
	 * Set the default timeline.  This needs
	 * to be done before connecting signals.
	 */
	conf_get_string(conf,
							PREFS_TWEETS_HOME_TIMELINE,
							&timeline);
	app_set_default_timeline(app, timeline);
	g_free(timeline);

	/* Connect the signals */
	glade_connect(ui, app,
						"main_window", "destroy", main_window_destroy_cb,
						"main_window", "delete_event", main_window_delete_event_cb,
						"main_window", "configure_event", app_window_configure_event_cb,
						"twitter_connect", "activate", app_connect_cb,
						"twitter_disconnect", "activate", app_disconnect_cb,
						"twitter_new_message", "activate", app_new_message_cb,
						"twitter_send_direct_message", "activate", app_send_direct_message_cb,
						"twitter_refresh", "activate", app_refresh_cb,
						"twitter_quit", "activate", app_quit_cb,
						"twitter_add_friend", "activate", app_add_friend_cb,
						"settings_account", "activate", app_account_cb,
						"settings_friends_manager", "activate", app_friends_manager_cb,
						"settings_preferences", "activate", app_preferences_cb,
						"view_combined_timeline", "changed", app_combined_timeline_cb,
						"view_public_timeline", "changed", app_public_timeline_cb,
						"view_friends_timeline", "changed", app_friends_timeline_cb,
						"view_my_timeline", "changed", app_mine_timeline_cb,
						"view_direct_messages", "changed", app_view_direct_messages_cb,
						"view_direct_replies", "changed", app_view_direct_replies_cb,
						"help_contents", "activate", app_help_contents_cb,
						"view_friends", "activate", app_view_friends_cb,
						"help_about", "activate", app_about_cb,
						NULL);

	/* Set up connected related widgets */
	app_connection_items_setup(app, ui);
	g_object_unref(ui);

	/* Let's hide the main window, while we are setting up the ui */
	gtk_widget_hide(GTK_WIDGET(app_priv->window));

#ifdef HAVE_DBUS
	/* Initialize NM */
	dbus_nm_init();
#endif

	app_priv->accounts_service = NULL;

	session_bus = dbus_g_proxy_new_for_name(dbus_g_bus_get(DBUS_BUS_SESSION, NULL),
                                             "org.freedesktop.DBus",
                                             "/org/freedesktop/DBus",
                                             "org.freedesktop.DBus");

	if(dbus_g_proxy_call(session_bus, "StartServiceByName", &error,
				G_TYPE_STRING, "org.gnome.OnlineAccounts",
				G_TYPE_UINT, 0, G_TYPE_INVALID,
				G_TYPE_UINT, &result, G_TYPE_INVALID))
		app_priv->accounts_service =
			dbus_g_proxy_new_for_name(dbus_g_bus_get(DBUS_BUS_SESSION, NULL),
									   "org.gnome.OnlineAccounts",
									   "/onlineaccounts",
									   "org.gnome.OnlineAccounts");

	if(app_priv->accounts_service) {
		g_signal_connect(app_priv->accounts_service, "destroy",
						G_CALLBACK(on_accounts_destroy), app); 
		dbus_g_proxy_call(app_priv->accounts_service,
						"EnsureAccountType",
						&error,
						G_TYPE_STRING, TYPE_TWITTER,
						G_TYPE_STRING, "Twitter",
						G_TYPE_STRING, "Twitter username",
						G_TYPE_STRING, "http://twitter.com",
						G_TYPE_INVALID);
		dbus_g_proxy_add_signal(app_priv->accounts_service,
						"AccountDisabled",
						DBUS_TYPE_G_OBJECT_PATH,
						G_TYPE_INVALID);
		dbus_g_proxy_add_signal(app_priv->accounts_service,
						"AccountEnabled",
						DBUS_TYPE_G_OBJECT_PATH,
						G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(app_priv->accounts_service,
						"AccountDisabled",
						G_CALLBACK(on_account_disabled),
						app, NULL);
		dbus_g_proxy_connect_signal(app_priv->accounts_service,
							"AccountEnabled",
							G_CALLBACK(on_account_enabled),
							app, NULL);
	}

	/* Set-up the notification area */
	debug(DEBUG_DOMAIN_SETUP,
			"Configuring notification area widget...");
	app_status_icon_create_menu();
	app_status_icon_create();
	
	/* Set the main window geometry */ 	 
	restore_main_window_geometry(app_priv->window);

	/* Set-up list view */
	app_priv->listview = tweet_list_new();
	gtk_widget_show(GTK_WIDGET(app_priv->listview));
	gtk_container_add(GTK_CONTAINER(scrolled_window),
					   GTK_WIDGET(app_priv->listview));

	/* Set-up expand messages panel */
	app_priv->expand_label = label_new();
	gtk_widget_show(GTK_WIDGET(app_priv->expand_label));
	gtk_box_pack_end(GTK_BOX(expand_vbox),
					   GTK_WIDGET(app_priv->expand_label),
					   TRUE, TRUE, 0);
	gtk_widget_hide(GTK_WIDGET(app_priv->expand_box));

	/* Initial status of widgets */
	app_state_on_connection(FALSE);

	/* Check Greet-Tweet-Know directory and images cache */
	app_check_dir();
	
	/* Get the gconf value for whether the window should be hidden on start-up */
	conf_get_bool(conf,
						  PREFS_UI_MAIN_WINDOW_HIDDEN,
						  &hidden);
	
	/* Ok, set the window state based on the gconf value */				  
	if(!hidden)
		gtk_widget_show(app_priv->window);
	else
		gtk_widget_hide(app_priv->window);

	/*Check to see if we should automatically login */
	conf_get_bool(conf,
						  PREFS_AUTH_AUTO_LOGIN,
						  &login);

	if(login) 
		app_login(app);
}

static void
main_window_destroy_cb(GtkWidget *window, App *app)
{
	/* Add any clean-up code here */
#ifdef DEBUG_QUIT
	gtk_main_quit();
#else
	exit(0);
#endif
}

static gboolean
main_window_delete_event_cb(GtkWidget *window,
							 GdkEvent  *event,
							 App *app)
{
		
	if(gtk_status_icon_is_embedded(app_priv->status_icon)) {
		hint_show(PREFS_HINTS_CLOSE_MAIN_WINDOW,
						  _("Greet-Tweet-Know is still running, it is just hidden."),
						  _("Click on the notification area icon to show Greet-Tweet-Know."),
						   GTK_WINDOW(app_get_window()),
						   NULL, NULL);
		
		app_set_visibility(FALSE);

return TRUE;
	}
	
	if(hint_dialog_show(PREFS_HINTS_CLOSE_MAIN_WINDOW,
								_("You were about to quit!"),
								_("Since no system or notification tray has been "
								"found, this action would normally quit Greet-Tweet-Know.\n\n"
								"This is just a reminder, from now on, Greet-Tweet-Know will "
								"quit when performing this action unless you uncheck "
								"the option below."),
								GTK_WINDOW(app_get_window()),
								NULL, NULL)) {
		/* Shown, we don't quit because the callback will
		 * decide that based on the YES|NO response from the
		 * question we are about to ask, since this behaviour
		 * is new.
		 */
return TRUE;
	}

	/* At this point, we have checked we have:
	 *   - No tray
	 *   - Have NOT shown the hint
	 * So we just quit.
	 */

return FALSE;
}

static void
app_set_radio_group(App  *app,
					 GtkBuilder *ui)
{
		GtkRadioAction *w;
	gint            i;

	const gchar     *radio_actions[] = {
		"view_public_timeline",
		"view_friends_timeline",
		"view_my_timeline",
		"view_direct_messages",
		"view_direct_replies"
	};

	
	for(i = 0; i < G_N_ELEMENTS(radio_actions); i++) {
		w = GTK_RADIO_ACTION(gtk_builder_get_object(ui, radio_actions[i]));
		gtk_radio_action_set_group(w, app_priv->group);
		app_priv->group = gtk_radio_action_get_group(w);
	}
}

static void
app_toggle_visibility(void)
{
		gboolean       visible;

	
	visible = window_get_is_visible(GTK_WINDOW(app_priv->window));

	if(visible && gtk_status_icon_is_embedded(app_priv->status_icon)) {
		gint x, y, w, h;

		gtk_window_get_size(GTK_WINDOW(app_priv->window), &w, &h);
		gtk_window_get_position(GTK_WINDOW(app_priv->window), &x, &y);
		gtk_widget_hide(app_priv->window);

		geometry_save_for_main_window(x, y, w, h);

		if(app_priv->size_timeout_id) {
			g_source_remove(app_priv->size_timeout_id);
			app_priv->size_timeout_id = 0;
		}
	} else {
		geometry_load_for_main_window(app_priv->window);
		window_present(GTK_WINDOW(app_priv->window), TRUE);
	}
	/* Save the window visibility state */
	conf_set_bool(conf_get(),
						  PREFS_UI_MAIN_WINDOW_HIDDEN,
						  visible);
}

void
app_set_visibility(gboolean visible)
{
	GtkWidget *window;

	window = app_get_window();

	conf_set_bool(conf_get(),
						  PREFS_UI_MAIN_WINDOW_HIDDEN,
						  !visible);

	if(!visible)
		gtk_widget_hide(window);
	else
		window_present(GTK_WINDOW(window), TRUE);
}

static void
app_connect_cb(GtkWidget *widget,
				App *app)
{
	app_login(app);
}

static void
app_disconnect_cb(GtkWidget *widget,
				   App *app)
{
	disconnect(app);
}

static void
app_new_message_cb(GtkWidget *widget,
					App *app)
{
	
	
	send_message_dialog_show(GTK_WINDOW(app_priv->window));
	message_show_friends(FALSE);
}

static void
app_send_direct_message_cb(GtkWidget *widget,
							App *app)
{
	
	
	send_message_dialog_show(GTK_WINDOW(app_priv->window));
	message_show_friends(TRUE);
}

static void
app_quit_cb(GtkWidget  *widget,
			 App  *app)
{
	
	
	gtk_widget_destroy(app_priv->window);
}

static void
app_refresh_cb(GtkWidget *window,
				App *app)
{
	network_refresh();
}

static void app_combined_timeline_cb(GtkRadioAction *action, GtkRadioAction *current, App *app){
	if(current!=app_priv->menu_combined) return;
	
	network_get_combined_timeline();
}

static void app_public_timeline_cb(GtkRadioAction *action, GtkRadioAction *current, App *app){
	if(app_priv->menu_public == current)
		network_get_timeline(API_TWITTER_TIMELINE_PUBLIC);
}
	
static void
app_friends_timeline_cb(GtkRadioAction *action,
						 GtkRadioAction *current,
						 App      *app)
{
	
	
	if(app_priv->menu_friends == current) 
		network_get_timeline(API_TWITTER_TIMELINE_FRIENDS);
}

static void
app_mine_timeline_cb(GtkRadioAction *action,
					  GtkRadioAction *current,
					  App      *app)
{
	
	
	if(app_priv->menu_mine == current)
		network_get_user(NULL);
}

static void
app_view_direct_messages_cb(GtkRadioAction *action,
							 GtkRadioAction *current,
							 App      *app)
{
	
	
	if(app_priv->menu_direct_messages == current)
		network_get_timeline(API_TWITTER_DIRECT_MESSAGES);
}

static void
app_view_direct_replies_cb(GtkRadioAction *action,
							GtkRadioAction *current,
							App      *app)
{
	
	
	if(app_priv->menu_direct_replies == current) 
		network_get_timeline(API_TWITTER_REPLIES);
}

static void
app_view_friends_cb(GtkAction *action,
							App *app)
{
			followers_dialog_show(GTK_WINDOW(app_priv->window));
}

static char**
get_account_set_request(App *app)
{
	static const char* twitter[2] = { TYPE_TWITTER, NULL };

	return(char **)twitter;
}

static void
app_account_cb(GtkWidget *widget,
				App *app)
{
	
	
	if(app_priv->accounts_service)
		dbus_g_proxy_call_no_reply(app_priv->accounts_service,
						"OpenAccountsDialogWithTypes",
						G_TYPE_STRV,
						get_account_set_request(app),
	    					G_TYPE_UINT,
	    					gtk_get_current_event_time(),
						G_TYPE_INVALID);
	else
		account_dialog_show(GTK_WINDOW(app_priv->window));
	
}

static void app_friends_manager_cb(GtkWidget *widget, App *app){
	friends_manager_show(GTK_WINDOW(app_priv->window));
}

static void app_preferences_cb(GtkWidget *widget, App *app){
	preferences_dialog_show(GTK_WINDOW(app_priv->window));
}

static void app_about_cb(GtkWidget *widget, App *app){
	about_dialog_new(GTK_WINDOW(app_priv->window));
}

static void
app_help_contents_cb(GtkWidget *widget,
					  App *app)
{
	
	
	help_show(GTK_WINDOW(app_priv->window));
}

static void
app_add_friend_cb(GtkAction *item,
				   App *app)
{
	
	
	add_dialog_show(GTK_WINDOW(app_priv->window));
}

static void
app_show_hide_cb(GtkWidget *widget,
				  App *app)
{
	app_toggle_visibility();
}

static void
app_status_icon_activate_cb(GtkStatusIcon *status_icon,
							 App     *app)
{
	app_toggle_visibility();
}

static void
app_status_icon_popup_menu_cb(GtkStatusIcon *status_icon,
							   guint          button,
							   guint          activate_time,
							   App     *app)
{
		gboolean       show;

	
	show = window_get_is_visible(GTK_WINDOW(app_priv->window));

	g_signal_handlers_block_by_func(app_priv->popup_menu_show_app,
									 app_show_hide_cb, app);

	gtk_toggle_action_set_active(app_priv->popup_menu_show_app, show);

	g_signal_handlers_unblock_by_func(app_priv->popup_menu_show_app,
									   app_show_hide_cb, app);

	gtk_menu_popup(GTK_MENU(app_priv->popup_menu),
					NULL, NULL,
					gtk_status_icon_position_menu,
					app_priv->status_icon,
					button,
					activate_time);
}

static void
app_status_icon_create_menu(void)
{
		GtkAction       *new_msg;
	GtkAction       *quit;
	GtkWidget       *w;

	
	app_priv->popup_menu_show_app = gtk_toggle_action_new("tray_show_app",
													   _("_Show "),
													   NULL,
													   NULL);
	g_signal_connect(G_OBJECT(app_priv->popup_menu_show_app),
					  "toggled", G_CALLBACK(app_show_hide_cb),
					  app);

	new_msg = gtk_action_new("tray_new_message",
							  _("_New Message"),
							  NULL,
							  "gtk-new");
	g_signal_connect(G_OBJECT(new_msg),
					  "activate", G_CALLBACK(app_new_message_cb),
					  app);

	quit = gtk_action_new("tray_quit",
						   _("_Quit"),
						   NULL,
						   "gtk-quit");
	g_signal_connect(G_OBJECT(quit),
					  "activate", G_CALLBACK(app_quit_cb),
					  app);

	app_priv->popup_menu = gtk_menu_new();
	w = gtk_action_create_menu_item(GTK_ACTION(app_priv->popup_menu_show_app));
	gtk_menu_shell_append(GTK_MENU_SHELL(app_priv->popup_menu), w);
	w = gtk_separator_menu_item_new();
	gtk_widget_show(w);
	gtk_menu_shell_append(GTK_MENU_SHELL(app_priv->popup_menu), w);
	w = gtk_action_create_menu_item(new_msg);
	gtk_menu_shell_append(GTK_MENU_SHELL(app_priv->popup_menu), w);
	w = gtk_action_create_menu_item(quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(app_priv->popup_menu), w);
}

static void
app_status_icon_create(void)
{
	
	
	app_priv->status_icon = gtk_status_icon_new_from_icon_name("greet-tweet-know");
	g_signal_connect(app_priv->status_icon,
					  "activate",
					  G_CALLBACK(app_status_icon_activate_cb),
					  app);

	g_signal_connect(app_priv->status_icon,
					  "popup_menu",
					  G_CALLBACK(app_status_icon_popup_menu_cb),
					  app);

	gtk_status_icon_set_visible(app_priv->status_icon, TRUE);
}

void
app_create(void)
{
	g_object_new(TYPE_APP, NULL);
	app_setup();
}

App *
app_get(void)
{
	g_assert(app != NULL);
	return app;
}
 
static gboolean
configure_event_timeout_cb(GtkWidget *widget)
{
		gint           x, y, w, h;

	
	gtk_window_get_size(GTK_WINDOW(widget), &w, &h);
	gtk_window_get_position(GTK_WINDOW(widget), &x, &y);

	geometry_save_for_main_window(x, y, w, h);

	app_priv->size_timeout_id = 0;

	
return FALSE;
}

static gboolean
app_window_configure_event_cb(GtkWidget         *widget,
							   GdkEventConfigure *event,
							   App         *app)
{
	
	
	if(app_priv->size_timeout_id)
		g_source_remove(app_priv->size_timeout_id);

	app_priv->size_timeout_id = g_timeout_add(500,
					(GSourceFunc) configure_event_timeout_cb,
					   widget);

return FALSE;
}

static gboolean
request_accounts(App  *app,
				  GError    **error)
{
		guint           i;
	GPtrArray      *accounts = NULL;
	char          **accountset;
	gboolean        succeeded = TRUE;

	
	accountset = get_account_set_request(app);

	if(!G_STR_EMPTY(app_priv->username))
		g_free(app_priv->username);
	
	app_priv->username = NULL;

	if(!G_STR_EMPTY(app_priv->password))
		g_free(app_priv->password);
	app_priv->password = NULL;

	if(app_priv->account)
		g_object_unref(app_priv->account);
	app_priv->account = NULL;
 
	if(!dbus_g_proxy_call(app_priv->accounts_service,
							"GetEnabledAccountsWithTypes",
							error,
							G_TYPE_STRV,
							accountset,
							G_TYPE_INVALID,
							dbus_g_type_get_collection("GPtrArray", DBUS_TYPE_G_PROXY),
							&accounts,
							G_TYPE_INVALID)){
return FALSE;
	}

	/* We only use the first account */
	for(i = 0; i < accounts->len && i == 0; i++)
	        succeeded = update_account((DBusGProxy*)g_ptr_array_index(accounts, i), app, error);
	
return succeeded;
}

static void
request_username_password(App *a)
{
		Conf    *conf;

	
	conf = conf_get();

	if(app_priv->accounts_service){ 
		GError *error = NULL;
		if(!request_accounts(a, &error))
			g_printerr("failed to get accounts: %s", error->message);
		
return;
	}

	g_free(app_priv->username);
	app_priv->username = NULL;
	conf_get_string(conf,
				PREFS_AUTH_USER_ID,
				&app_priv->username);
	g_free(app_priv->password);
#ifdef HAVE_GNOME_KEYRING
	app_priv->password = NULL;
	if(G_STR_EMPTY(app_priv->username))
		app_priv->password = NULL;
	else if(!(keyring_get_password(app_priv->username, &app_priv->password)))
		app_priv->password = NULL;
#else
	conf_get_string(conf,
				PREFS_AUTH_PASSWORD,
				&app_priv->password);
#endif
}

static void
app_login(App *a)
{
	
#ifdef HAVE_DBUS
	gboolean connected = TRUE;

	/*
	 * Don't try to connect if we have
	 * Network Manager state and we are NOT connected.
	 */
	if(dbus_nm_get_state(&connected) && !connected)
		return;
#endif
	
	
	request_username_password(a);

	if(G_STR_EMPTY(app_priv->username) || G_STR_EMPTY(app_priv->password))
		app_account_cb(NULL, a);
	else {
		network_login(app_priv->username, app_priv->password);
		app_retrieve_default_timeline();
	}
}

/*
 * Function to set the default
 * timeline in the menu.
 */
static void
app_set_default_timeline(App *app, gchar *timeline)
{
	/* This shouldn't happen, but just in case */
	if(G_STR_EMPTY(timeline)) {
		g_warning("Default timeline in not set");
		return;
	}

	/*if( !(strcmp( timeline, "combined" )) )
		return gtk_radio_action_set_current_value( app_priv->menu_combined, 1);*/
	if( !(strcmp( timeline, API_TWITTER_TIMELINE_PUBLIC )) )
		return gtk_radio_action_set_current_value( app_priv->menu_public, 1);
	
	if( !(strcmp( timeline, API_TWITTER_TIMELINE_MY )) )
		return gtk_radio_action_set_current_value(app_priv->menu_mine, 1);
	
	/* Let's fallback to friends timeline */
	gtk_radio_action_set_current_value(app_priv->menu_friends,	1);
}

/* Function to retrieve the users default timeline */
static void
app_retrieve_default_timeline(void)
{
	gchar         *timeline=NULL;

	conf_get_string(conf_get(),
							PREFS_TWEETS_HOME_TIMELINE,
							&timeline);

	if(G_STR_EMPTY(timeline)){
		timeline = g_strdup(API_TWITTER_TIMELINE_FRIENDS);
		app_set_default_timeline(app, API_TWITTER_TIMELINE_FRIENDS);
	}

	network_get_timeline(timeline);
	g_free(timeline);
}

static void
app_check_dir(void)
{
	gchar    *file;

	file = g_build_filename(g_get_home_dir(), ".gnome2", CACHE_IMAGES, NULL);

	if(!g_file_test(file, G_FILE_TEST_EXISTS|G_FILE_TEST_IS_DIR)) {
		debug(DEBUG_DOMAIN_SETUP, "Making directory: %s", file);
		g_mkdir_with_parents(file, S_IRUSR|S_IWUSR|S_IXUSR);
	}

	g_free(file);
}

static void
app_connection_items_setup(App  *app,
							GtkBuilder *ui)
{
	GList         *list;
	GObject       *w;
	gint           i;

	const gchar   *widgets_connected[] = {
		"twitter_disconnect",
		"twitter_new_message",
		"twitter_send_direct_message",
		"twitter_refresh",
		"twitter_add_friend",
		"view1"
	};

	const gchar   *widgets_disconnected[] = {
		"twitter_connect"
	};
	
	for(i = 0, list = NULL; i < G_N_ELEMENTS(widgets_connected); i++) {
		w=gtk_builder_get_object(ui, widgets_connected[i]);
		list=g_list_prepend(list, w);
	}
	
	app_priv->widgets_connected=list;
	for(i = 0, list = NULL; i < G_N_ELEMENTS(widgets_disconnected); i++) {
		w=gtk_builder_get_object(ui, widgets_disconnected[i]);
		list=g_list_prepend(list, w);
	}
	
	app_priv->widgets_disconnected=list;
}

void
app_state_on_connection(gboolean connected)
{
	GList         *l;
	
		
	for(l = app_priv->widgets_connected; l; l = l->next)
		g_object_set(l->data, "sensitive", connected, NULL);
	
	for(l = app_priv->widgets_disconnected; l; l = l->next)
		g_object_set(l->data, "sensitive", !connected, NULL);
	
	g_list_free(l);
}

GtkWidget *
app_get_window(void)
{
	return app_priv->window;
}

void
app_set_statusbar_msg(gchar *message)
{
	
	
	/* Avoid some warnings */
	if(!app_priv->statusbar || !GTK_IS_STATUSBAR(app_priv->statusbar))
		return;

	/* conext ID will be always 1 */
	gtk_statusbar_pop(GTK_STATUSBAR(app_priv->statusbar), 1);
	gtk_statusbar_push(GTK_STATUSBAR(app_priv->statusbar), 1, message);
}

void
app_notify_sound(void)
{
	gboolean sound;

	conf_get_bool(conf_get(),
						  PREFS_UI_SOUND,
						  &sound);

	if(sound) {
		ca_context_play(ca_gtk_context_get(),
						 0,
						 CA_PROP_APPLICATION_NAME, g_get_application_name(),
						 CA_PROP_EVENT_ID, "message-new-instant",
						 CA_PROP_EVENT_DESCRIPTION, _("New tweet received"),
						 NULL);
	}
}

void
app_notify(gchar *msg)
{
	gboolean notify;

	conf_get_bool(conf_get(),
						  PREFS_UI_NOTIFICATION,
						  &notify);

	if(!notify)
		return;
	NotifyNotification *notification;
	GError             *error = NULL;

	notification = notify_notification_new(PACKAGE_NAME,
										msg,
										"twitux",
										NULL);

	notify_notification_set_timeout(notification, 8 * 1000);
	notify_notification_show(notification, &error);
	g_object_unref(G_OBJECT(notification));

	if(!error)
		return;
	
	debug(DEBUG_DOMAIN_SETUP,
				  "Error displaying notification: %s",
				  error->message);
	g_error_free(error);
}

void
app_set_image(const gchar *file,
                      GtkTreeIter  iter)
{
	GdkPixbuf	 *pixbuf, *resized;
	GError		 *error = NULL;

	if( !(pixbuf=gdk_pixbuf_new_from_file(file, &error)) ){
		debug(DEBUG_DOMAIN_SETUP, "Image error: %s: %s", file, error->message);
		if(error) g_error_free(error);
		return;
	}

	resized=gdk_pixbuf_scale_simple( pixbuf, 48, 48, GDK_INTERP_BILINEAR );

	gtk_list_store_set((tweet_list_get_store()) , &iter, PIXBUF_AVATAR, resized, -1);
	g_object_unref(pixbuf);
	g_object_unref(resized);
	if(error) g_error_free(error);
}

void
app_expand_message(const gchar *name,
                           const gchar *date,
                           const gchar *tweet,
                           GdkPixbuf   *pixbuf)
{
	gchar *title_text=g_strdup_printf("<b>%s</b> - %s", name, date);
	
	label_set_text(LABEL(app_priv->expand_label), tweet);
	gtk_label_set_markup(GTK_LABEL(app_priv->expand_title), title_text);
	g_free(title_text);
	
	if(pixbuf) {
		GdkPixbuf *resized=NULL;
		resized=gdk_pixbuf_scale_simple( pixbuf, 73, 73, GDK_INTERP_BILINEAR );
		gtk_image_set_from_pixbuf(GTK_IMAGE(app_priv->expand_image), resized);
		if(resized)
			g_object_unref(resized);
	}
	
	gtk_widget_show(app_priv->expand_box);
}
