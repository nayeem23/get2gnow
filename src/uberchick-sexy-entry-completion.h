/* -*- Mode: C; shift-width: 8; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * get2gnow is:
 * 	Copyright (c) 2006-2009 Kaity G. B. <uberchick@uberChicGeekChick.Com>
 * 	Released under the terms of the Reciprocal Public License (RPL).
 *
 * For more information or to find the latest release, visit our
 * website at: http://uberChicGeekChick.Com/?projects=get2gnow
 *
 * Writen by an uberchick, other uberchicks please meet me & others @:
 * 	http://uberchicks.Net/
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
#ifndef __UBERCHICK_SEXY_ENTRY_COMPLETION_H__
#define __UBERCHICK_SEXY_ENTRY_COMPLETION_H__

#define _GNU_SOURCE
#define _THREAD_SAFE


/********************************************************************************
 *      Project, system, & library headers.  eg #include <gdk/gdkkeysyms.h>     *
 ********************************************************************************/
#include <glib.h>
#include <gtk/gtk.h>


/********************************************************************************
 *        Methods, macros, constants, objects, structs, and enum typedefs       *
 ********************************************************************************/
G_BEGIN_DECLS

#define UBERCHICK_TYPE_SEXY_ENTRY_COMPLETION					\
			(uberchick_sexy_entry_completion_get_type())
#define UBERCHICK_SEXY_ENTRY_COMPLETION(o)					\
			(G_TYPE_CHECK_INSTANCE_CAST((o), UBERCHICK_TYPE_SEXY_ENTRY_COMPLETION, UberChickSexyEntryCompletion))
#define UBERCHICK_SEXY_ENTRY_COMPLETION_CLASS(k)				\
			(G_TYPE_CHECK_CLASS_CAST((k), UBERCHICK_TYPE_SEXY_ENTRY_COMPLETION, UberChickSexyEntryCompletionClass))
#define IS_UBERCHICK_SEXY_ENTRY_COMPLETION(o)					\
			(G_TYPE_CHECK_INSTANCE_TYPE((o), UBERCHICK_TYPE_SEXY_ENTRY_COMPLETION))
#define IS_UBERCHICK_SEXY_ENTRY_COMPLETION_CLASS(k)				\
			(G_TYPE_CHECK_CLASS_TYPE((k), UBERCHICK_TYPE_SEXY_ENTRY_COMPLETION))
#define UBERCHICK_SEXY_ENTRY_COMPLETION_GET_CLASS(o)				\
			(G_TYPE_INSTANCE_GET_CLASS((o), UBERCHICK_TYPE_SEXY_ENTRY_COMPLETION, UberChickSexyEntryCompletionClass))

typedef struct _UberChickSexyEntryCompletion      UberChickSexyEntryCompletion;
typedef struct _UberChickSexyEntryCompletionClass UberChickSexyEntryCompletionClass;
typedef struct _UberChickSexyEntryCompletionPrivate  UberChickSexyEntryCompletionPrivate;

struct _UberChickSexyEntryCompletion {
	GtkFrame		parent; /*What other Extends/Implements/Interfaces.*/
	/* Commonly Used:
	 * 	GObject			parent;
	 * 	GtkObject		parent;
	 * 	GtkWidget		parent;
	 * 	GtkContainer		parent;
	 * 	GBin			parent;
	 * 	GtkWindow		parent;
	 * 	GtkItem			parent;
	 * 	SexyLabel		parent;
	 */
};

struct _UberChickSexyEntryCompletionClass {
	GtkFrameClass		parent_class;
	/* Commonly Used:
	 * 	GObjectClass		parent_class;
	 * 	GtkObjectClass		parent_class;
	 * 	GtkWidgetClass		parent_class;
	 * 	GtkContainerClass	parent_class;
	 * 	GBinClass		parent_class;
	 *	GtkWindowClass		parent_class;
	 *	GtkItemClass		parent_class;
	 *	SexyLabelClass		parent_class;
	 */
};

GType uberchick_sexy_entry_completion_get_type(void) G_GNUC_CONST;
UberChickSexyEntryCompletion *uberchick_sexy_entry_completion_new(void);


/********************************************************************************
 *       prototypes for methods, handlers, callbacks, function, & etc           *
 ********************************************************************************/
SexySpellEntry *uberchick_sexy_entry_get_widget(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);

void uberchick_sexy_entry_completion_show_entries(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);
void uberchick_sexy_entry_completion_hide_entries(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);

/*void uberchick_sexy_entry_completion_send(GtkWidget *activated_widget);*/

void uberchick_sexy_entry_completion_beep(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);

void uberchick_sexy_entry_completion_select_all(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);

gboolean uberchick_sexy_entry_completion_entry_is_empty(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);
gboolean uberchick_sexy_entry_completion_entry_clear(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);
void uberchick_sexy_entry_completion_prefix_char(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion, const char c);
gboolean uberchick_sexy_entry_completion_prefix_string(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion, const gchar *str, gboolean to_lower, gboolean uniq);
void uberchick_sexy_entry_completion_insert_char(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion, const char c);
gboolean uberchick_sexy_entry_completion_insert_string(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion, const gchar *str, gboolean to_lower, gboolean uniq);
void uberchick_sexy_entry_completion_append_char(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion, const char c);
gboolean uberchick_sexy_entry_completion_append_string(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion, const gchar *str, gboolean to_lower, gboolean uniq);

void uberchick_sexy_entry_completion_select(UberChickSexyEntryCompletion *uberchick_sexy_entry_completion);

G_END_DECLS
#endif /* __UBERCHICK_SEXY_ENTRY_COMPLETION_H__ */
/********************************************************************************
 *                                    eof                                       *
 ********************************************************************************/


