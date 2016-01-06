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

#include "TextEditor.h"

#include "../../Configuration/config.h"

#include <Titania/X3D/Components/Shape/X3DShapeNode.h>
#include <Titania/String.h>

namespace titania {
namespace puck {

TextEditor::TextEditor (X3DBrowserWindow* const browserWindow) :
	      X3DBaseInterface (browserWindow, browserWindow -> getCurrentBrowser ()),
	X3DTextEditorInterface (get_ui ("Editors/TextEditor.glade"), gconf_dir ()),
	X3DFontStyleNodeEditor (),
	            shapeNodes (),
	    geometryNodeBuffer (),
	                  text (),
	               measure (),
	              undoStep (),
	              changing (false),
	             maxExtent (this,
	                        getTextMaxExtentAdjustment (),
	                        getTextMaxExtentSpinButton (),
	                        "maxExtent")
{
	addChildren (geometryNodeBuffer);
	geometryNodeBuffer .addInterest (this, &TextEditor::set_node);
	setup ();
}

void
TextEditor::initialize ()
{
	X3DTextEditorInterface::initialize ();
	X3DFontStyleNodeEditor::initialize ();

	getBrowserWindow () -> getSelection () -> getChildren () .addInterest (this, &TextEditor::set_selection);

	set_selection ();
}

void
TextEditor::set_selection ()
{
	for (const auto & shapeNode : shapeNodes)
		shapeNode -> geometry () .removeInterest (this, &TextEditor::set_geometry);

	shapeNodes = getSelection <X3D::X3DShapeNode> ({ X3D::X3DConstants::X3DShapeNode });

	for (const auto & shapeNode : shapeNodes)
		shapeNode -> geometry () .addInterest (this, &TextEditor::set_geometry);

	set_geometry ();
}

/***********************************************************************************************************************
 *
 *  Text
 *
 **********************************************************************************************************************/

void
TextEditor::on_text_unlink_clicked ()
{
	unlinkClone (shapeNodes, "geometry", undoStep);
}

void
TextEditor::on_text_toggled ()
{
	if (changing)
		return;

	getTextCheckButton () .set_inconsistent (false);
	getTextBox ()         .set_sensitive (getTextCheckButton () .get_active ());

	// Set field.

	addUndoFunction <X3D::SFNode> (shapeNodes, "geometry", undoStep);

	for (const auto & shapeNode : shapeNodes)
	{
		try
		{
			auto & field = shapeNode -> geometry ();

			field .removeInterest (this, &TextEditor::set_geometry);
			field .addInterest (this, &TextEditor::connectGeometry);

			if (getTextCheckButton () .get_active ())
				getBrowserWindow () -> replaceNode (getCurrentContext (), X3D::SFNode (shapeNode), field, X3D::SFNode (text), undoStep);
			else
				getBrowserWindow () -> replaceNode (getCurrentContext (), X3D::SFNode (shapeNode), field, nullptr, undoStep);
		}
		catch (const X3D::X3DError &)
		{ }
	}

	addRedoFunction <X3D::SFNode> (shapeNodes, "geometry", undoStep);

	X3DFontStyleNodeEditor::set_selection ();

	getTextUnlinkButton () .set_sensitive (getTextCheckButton () .get_active () and text -> getCloneCount () > 1);
}

void
TextEditor::set_geometry ()
{
	geometryNodeBuffer .addEvent ();
}

void
TextEditor::connectGeometry (const X3D::SFNode & field)
{
	field .removeInterest (this, &TextEditor::connectGeometry);
	field .addInterest (this, &TextEditor::set_geometry);
}

void
TextEditor::set_node ()
{
	undoStep .reset ();

	if (text)
		text -> string () .removeInterest (this, &TextEditor::set_string);

	auto  tuple             = getSelection <X3D::Text> (shapeNodes, "geometry");
	const int32_t active    = std::get <1> (tuple);
	const bool    hasParent = std::get <2> (tuple);
	const bool    hasField  = (active not_eq -2);

	text = std::move (std::get <0> (tuple));

	if (text)
	{
		measure = getCurrentContext () -> createNode <X3D::Text> ();
		measure -> lineBounds () .addInterest (this, &TextEditor::on_char_spacing_changed);

		text -> length ()    .addInterest (this, &TextEditor::set_length);
		text -> string ()    .addInterest (measure -> string ());
		text -> fontStyle () .addInterest (measure -> fontStyle ());

		measure -> string ()    = text -> string ();
		measure -> fontStyle () = text -> fontStyle ();
		getCurrentContext () -> realize ();

		set_length ();
	}
	else
	{
		text = new X3D::Text (getCurrentContext ());
		getCurrentContext () -> addUninitializedNode (text);
		getCurrentContext () -> realize ();
	}

	changing = true;

	getSelectTextBox ()   .set_sensitive (hasParent);
	getTextCheckButton () .set_sensitive (hasField);
	getTextCheckButton () .set_active (active > 0);
	getTextCheckButton () .set_inconsistent (active < 0);

	getTextUnlinkButton () .set_sensitive (active > 0 and text -> getCloneCount () > 1);
	getTextBox ()          .set_sensitive (active > 0);

	changing = false;

	text -> string () .addInterest (this, &TextEditor::set_string);

	set_string ();
	
	maxExtent .setNodes ({ text });

	X3DFontStyleNodeEditor::set_selection ();
}

/***********************************************************************************************************************
 *
 *  string
 *
 **********************************************************************************************************************/

void
TextEditor::on_string_changed ()
{
	if (changing)
		return;

	addUndoFunction (text, text -> string (), undoStep);

	text -> string () .removeInterest (this, &TextEditor::set_string);
	text -> string () .addInterest (this, &TextEditor::connectString);

	text -> string () .clear ();

	const auto string = basic::split (getTextStringTextBuffer () -> get_text (), "\n");

	for (auto & value : string)
		text -> string () .emplace_back (std::move (value));

	addRedoFunction (text -> string (), undoStep);
}

void
TextEditor::set_string ()
{
	changing = true;

	getTextStringTextBuffer () -> set_text ("");

	if (not text -> string () .empty ())
	{
		for (const auto & value : text -> string ())
		{
			getTextStringTextBuffer () -> insert (getTextStringTextBuffer () -> end (), value);
			getTextStringTextBuffer () -> insert (getTextStringTextBuffer () -> end (), "\n");
		}

		getTextStringTextBuffer () -> erase (-- getTextStringTextBuffer () -> end (), getTextStringTextBuffer () -> end ());
	}

	changing = false;
}

void
TextEditor::connectString (const X3D::MFString & field)
{
	field .removeInterest (this, &TextEditor::connectString);
	field .addInterest (this, &TextEditor::set_string);
}

void
TextEditor::on_char_spacing_changed ()
{
	if  (not text)
		return;

	if (changing)
		return;

	// Set text length

	text -> length () .removeInterest (this, &TextEditor::set_length);
	text -> length () .addInterest (this, &TextEditor::connectLength);
	text -> length () .addEvent ();

	const auto kerning = getTextCharSpacingAdjustment () -> get_value ();

	if (kerning)
	{
		for (size_t i = 0, size = text -> string () .size (); i < size; ++ i)
		{
			const auto numChars   = text -> string () .get1Value (i) .length ();
			const auto lineLength = text -> getFontStyle () -> horizontal ()
			                        ? measure -> lineBounds () .get1Value (i) .getX ()
			                        : measure -> lineBounds () .get1Value (i) .getY ();

			if (numChars)
				text -> length () .set1Value (i, (numChars - 1) * kerning + lineLength);
		}
	}
	else
		text -> length () .clear ();
}

void
TextEditor::set_length ()
{
	changing = true;

	double kerning = 0;

	for (size_t i = 0, size = std::min (text -> length () .size (), text -> string () .size ()); i < size; ++ i)
	{
		const auto numChars   = text -> string () .get1Value (i) .length ();
		const auto lineLength = text -> getFontStyle () -> horizontal ()
		                        ? measure -> lineBounds () .get1Value (i) .getX ()
		                        : measure -> lineBounds () .get1Value (i) .getY ();

		kerning += (text -> length () .get1Value (i) - lineLength) / (numChars - 1);
	}

	if (text -> lineBounds () .size ())
		kerning /= text -> lineBounds () .size ();

	getTextCharSpacingAdjustment () -> set_value (kerning);

	changing = false;
}

void
TextEditor::connectLength (const X3D::MFFloat & field)
{
	field .removeInterest (this, &TextEditor::connectLength);
	field .addInterest (this, &TextEditor::set_length);
}

TextEditor::~TextEditor ()
{
	dispose ();
}

} // puck
} // titania
