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

#define _GNU_SOURCE
#define _THREAD_SAFE


#include "config.h"
#include "program.h"

#include "gtkbuilder.h"

#define	DEBUG_DOMAINS	"Debug:UI:GtkBuilder:GtkBuildable:Setup:gtkbuilder.c"
#include "debug.h"



static gchar *gtkbuilder_get_path(const gchar *filename);
static GtkBuilder *gtkbuilder_load_file(const gchar *filename, const gchar *first_widget, va_list widgets);
static gchar *gtkbuilder_ui_test_filename(gchar *gtkbuilder_ui_filename);
static gboolean gtkbuilder_get_ui_filename(const gchar *base_filename, gchar **gtkbuilder_ui_filename, gboolean get_local, gboolean use_template);

static void gtkbuilder_get_objects_from_ui(GtkBuilder *ui, const gchar *filename, const gchar *first_widget, va_list widgets);



static gchar *gtkbuilder_get_path(const gchar *base_filename){
	gchar *gtkbuilder_ui_filename=NULL;
	
#ifndef	GNOME_ENABLE_DEBUG
	
	if(gtkbuilder_get_ui_filename(base_filename, &gtkbuilder_ui_filename, FALSE, FALSE))
		return gtkbuilder_ui_filename;
	
	else if(gtkbuilder_get_ui_filename(base_filename, &gtkbuilder_ui_filename, TRUE, FALSE))
		return gtkbuilder_ui_filename;
	
#else
	
	if(gtkbuilder_get_ui_filename(base_filename, &gtkbuilder_ui_filename, TRUE, FALSE))
		return gtkbuilder_ui_filename;
	
	else if(gtkbuilder_get_ui_filename(base_filename, &gtkbuilder_ui_filename, FALSE, FALSE))
		return gtkbuilder_ui_filename;
#endif
	
	else if(gtkbuilder_get_ui_filename(base_filename, &gtkbuilder_ui_filename, TRUE, TRUE))
		return gtkbuilder_ui_filename;
	
	debug("**ERROR:** Unable to load gtkbuilder ui: [%s.ui]", base_filename);
	
	return NULL;
}/*gtkbuilder_get_path("update-viewer");*/

static gboolean gtkbuilder_get_ui_filename(const gchar *base_filename, gchar **gtkbuilder_ui_filename, gboolean get_local, gboolean use_template){
	gchar *gtkbuilder_ui_file=NULL;
	gtkbuilder_ui_file=g_strdup_printf("%s%s.ui", (use_template ?".in" :""), base_filename);
	if(!get_local)
		*gtkbuilder_ui_filename=g_build_filename(DATADIR, PACKAGE_TARNAME, gtkbuilder_ui_file, NULL);
	else
		*gtkbuilder_ui_filename=g_build_filename(BUILDDIR, "data", gtkbuilder_ui_file, NULL);
	uber_free(gtkbuilder_ui_file);
	if(gtkbuilder_ui_test_filename(*gtkbuilder_ui_filename)){
		debug("Loading GtkBuildable UI from: [%s]", *gtkbuilder_ui_filename);
		
		if(use_template){
			debug("**ERROR:** Loading GtkBuildable UI from: [%s]", *gtkbuilder_ui_filename);
			debug("**ERROR:** GTK_BUILDER_UI_FILENAME: [%s] needs converted", *gtkbuilder_ui_filename);
			debug("**ERROR:** GTK_BUILDER_UI_FILENAME: template file: [data/%s.in.ui] exists; needs converted to [data/%s.ui]", base_filename, base_filename);
			debug("**ERROR:** GTK_BUILDER_UI_FILENAME: can be converted by running: (cd data/ ; touch %s.in.ui; make %s.ui).  Or re-run `make`", base_filename, base_filename);
		}
		return TRUE;
	}
	
	uber_free(*gtkbuilder_ui_filename);
	return FALSE;
}/*gtkbuilder_get_ui_filename(base_filename, &gtkbuilder_ui_filename, TRUE|FALSE);*/

static gchar *gtkbuilder_ui_test_filename(gchar *gtkbuilder_ui_filename){
	debug("Checking existance of for GtkBuilder UI filename: %s", gtkbuilder_ui_filename);
	if(!g_file_test(gtkbuilder_ui_filename, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)){
		debug("Unable to load GtkBuilder UI filename: %s", gtkbuilder_ui_filename);
		return NULL;
	}
	
	debug("GtkBuilder UI found filename: %s", gtkbuilder_ui_filename);
	return gtkbuilder_ui_filename;
}/*gtkbuilder_ui_test_file(gtkbuilder_ui_filename);*/



static GtkBuilder *gtkbuilder_load_file(const gchar *filename, const gchar *first_widget, va_list widgets){
	GtkBuilder  *ui=NULL;
	gchar       *path=NULL;
	GError      *error=NULL;

	/* Create gtkbuilder & load the xml file */
	ui=gtk_builder_new();
	gtk_builder_set_translation_domain(ui, GETTEXT_PACKAGE);
	if(!(path=gtkbuilder_get_path(filename))){
		debug("Unable to load GtkBuilder UI filename: %s", filename);
		return NULL;
	}
	if(!gtk_builder_add_from_file(ui, path, &error)){
		debug("**ERROR:** XML file error: %s", error->message);
		g_error_free(error);
		g_free(path);
		return NULL;
	}
	
	gtkbuilder_get_objects_from_ui(ui, path, first_widget, widgets);
	
	if(path) uber_free(path);
	
	return ui;
}/*gtkbuilder_load_file(GTK_BUILDER_UI_FILENAME, "widget", private_objects_ui->widget, NULL);*/

void gtkbuilder_get_objects(GtkBuilder *ui, const gchar *filename, const gchar *first_widget, ...){
	gchar *path=NULL;
	if(!(path=gtkbuilder_get_path(filename))){
		debug("Unable to load GtkBuilder UI filename: %s", filename);
		return;
	}
	va_list widgets;
	va_start(widgets, first_widget);
	gtkbuilder_get_objects_from_ui(ui, path, first_widget, widgets);
	va_end(widgets);
	uber_free(path);
}/*gtkbuilder_get_objects(ui, "widget", private_objects_ui->widget, NULL);*/

static void gtkbuilder_get_objects_from_ui(GtkBuilder *ui, const gchar *path, const gchar *first_widget, va_list widgets){
	GObject    **pointer;
	const char  *name=NULL;
	for(name=first_widget; name; name=va_arg(widgets, char *)){
		pointer=va_arg(widgets, void *);
		if(!(*pointer=gtk_builder_get_object(ui, name)))
			debug("**ERROR:** Widget '%s' at '%s' is missing", name, path);
	}
}/*gtkbuilder_get_objects(ui, path, first_widget, widgets);*/

GtkBuilder *gtkbuilder_get_file (const gchar *filename, const gchar *first_widget, ...){
	va_list widgets;
	va_start(widgets, first_widget);
	GtkBuilder *ui=gtkbuilder_load_file(filename, first_widget, widgets);
	va_end(widgets);

	return (ui ? ui : NULL);
}

void gtkbuilder_signals_connect(gboolean connect_after, GtkBuilder *ui, gpointer user_data, gchar *first_widget, ...){
	va_list args;
	va_start(args, first_widget);
	
	for(const gchar *name=first_widget; name; name=va_arg (args, gchar *)) {
		GObject *instance=NULL;
		const gchar *signal=va_arg(args, gchar *);
		GCallback callback=va_arg(args, GCallback);
		
		if(!(instance=gtk_builder_get_object(ui, name))){
			debug("**ERROR:** Missing widget '%s'", name);
			continue;
		}
		
		if(!connect_after)
			g_signal_connect(instance, signal, (GCallback)callback, user_data);
		else
			g_signal_connect_after(instance, signal, (GCallback)callback, user_data);
	}
	va_end(args);
}/*gtkbuilder_signals_connect((TRUE: use g_signal_connect; FALSE uses g_signal_connec_after), ui, user_data, "widget_name", va_args);*/

