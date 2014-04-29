/* -*- Mode: C++; coding: utf-8; tab-width: 3; indent-tabs-mode: tab; c-basic-offset: 3 -*-
 *******************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright create3000, Scheffelstraße 31a, Leipzig, Germany 2011.
 *
 * All rights reserved. Holger Seelig <holger.seelig@yahoo.de>.
 *
 * THIS IS UNPUBLISHED SOURCE CODE OF create3000.
 *
 * The copyright notice above does not evidence any actual of intended
 * publication of such source code, and is an unpublished work by create3000.
 * This material contains CONFIDENTIAL INFORMATION that is the property of
 * create3000.
 *
 * No permission is granted to copy, distribute, or create derivative works from
 * the contents of this software, in whole or in part, without the prior written
 * permission of create3000.
 *
 * NON-MILITARY USE ONLY
 *
 * All create3000 software are effectively free software with a non-military use
 * restriction. It is free. Well commented source is provided. You may reuse the
 * source in any way you please with the exception anything that uses it must be
 * marked to indicate is contains 'non-military use only' components.
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 1999, 2012 Holger Seelig <holger.seelig@yahoo.de>.
 *
 * This file is part of the Titania Project.
 *
 * Titania is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3 only, as published by the
 * Free Software Foundation.
 *
 * Titania is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License version 3 for more
 * details (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version 3
 * along with Titania.  If not, see <http://www.gnu.org/licenses/gpl.html> for a
 * copy of the GPLv3 License.
 *
 * For Silvio, Joy and Adi.
 *
 ******************************************************************************/
#include "X3DNodePropertiesEditorInterface.h"

namespace titania {
namespace puck {

const std::string X3DNodePropertiesEditorInterface::m_widgetName = "NodePropertiesEditor";

void
X3DNodePropertiesEditorInterface::create (const std::string & filename)
{
	// Create Builder.
	m_builder = Gtk::Builder::create_from_file (filename);

	// Get objects.
	m_accessTypeIconFactory      = Glib::RefPtr <Gtk::IconFactory>::cast_dynamic (m_builder -> get_object ("AccessTypeIconFactory"));
	m_fieldTypeIconFactory       = Glib::RefPtr <Gtk::IconFactory>::cast_dynamic (m_builder -> get_object ("FieldTypeIconFactory"));
	m_userDefinedFieldsListStore = Glib::RefPtr <Gtk::ListStore>::cast_dynamic (m_builder -> get_object ("UserDefinedFieldsListStore"));
	m_cellRendererType           = Glib::RefPtr <Gtk::CellRendererPixbuf>::cast_dynamic (m_builder -> get_object ("CellRendererType"));
	m_cellRendererName           = Glib::RefPtr <Gtk::CellRendererText>::cast_dynamic (m_builder -> get_object ("CellRendererName"));
	m_cellRendererAccessType     = Glib::RefPtr <Gtk::CellRendererPixbuf>::cast_dynamic (m_builder -> get_object ("CellRendererAccessType"));

	// Get widgets.
	m_builder -> get_widget ("Window", m_window);
	m_window -> set_name ("Window");
	m_builder -> get_widget ("CancelButton", m_cancelButton);
	m_cancelButton -> set_name ("CancelButton");
	m_builder -> get_widget ("OkButton", m_okButton);
	m_okButton -> set_name ("OkButton");
	m_builder -> get_widget ("Widget", m_widget);
	m_widget -> set_name ("Widget");
	m_builder -> get_widget ("HeaderLabel", m_headerLabel);
	m_headerLabel -> set_name ("HeaderLabel");
	m_builder -> get_widget ("NodePropertiesExpander", m_nodePropertiesExpander);
	m_nodePropertiesExpander -> set_name ("NodePropertiesExpander");
	m_builder -> get_widget ("TypeNameLabel", m_typeNameLabel);
	m_typeNameLabel -> set_name ("TypeNameLabel");
	m_builder -> get_widget ("NameLabel", m_nameLabel);
	m_nameLabel -> set_name ("NameLabel");
	m_builder -> get_widget ("TypeNameEntry", m_typeNameEntry);
	m_typeNameEntry -> set_name ("TypeNameEntry");
	m_builder -> get_widget ("NameEntry", m_nameEntry);
	m_nameEntry -> set_name ("NameEntry");
	m_builder -> get_widget ("UserDefinedFieldsExpander", m_userDefinedFieldsExpander);
	m_userDefinedFieldsExpander -> set_name ("UserDefinedFieldsExpander");
	m_builder -> get_widget ("UserDefinedFieldsTreeView", m_userDefinedFieldsTreeView);
	m_userDefinedFieldsTreeView -> set_name ("UserDefinedFieldsTreeView");

	// Connect object Gtk::Button with id 'CancelButton'.
	m_cancelButton -> signal_clicked () .connect (sigc::mem_fun (*this, &X3DNodePropertiesEditorInterface::on_cancel));
	m_okButton -> signal_clicked () .connect (sigc::mem_fun (*this, &X3DNodePropertiesEditorInterface::on_ok));

	// Connect object Gtk::Entry with id 'TypeNameEntry'.
	m_typeNameEntry -> signal_delete_text () .connect (sigc::mem_fun (*this, &X3DNodePropertiesEditorInterface::on_type_name_delete_text), false);
	m_typeNameEntry -> signal_insert_text () .connect (sigc::mem_fun (*this, &X3DNodePropertiesEditorInterface::on_type_name_insert_text), false);
	m_nameEntry -> signal_delete_text () .connect (sigc::mem_fun (*this, &X3DNodePropertiesEditorInterface::on_name_delete_text), false);
	m_nameEntry -> signal_insert_text () .connect (sigc::mem_fun (*this, &X3DNodePropertiesEditorInterface::on_name_insert_text), false);

	// Call construct handler of base class.
	construct ();
}

X3DNodePropertiesEditorInterface::~X3DNodePropertiesEditorInterface ()
{
	delete m_window;
}

} // puck
} // titania
