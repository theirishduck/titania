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

#include "TexturePreview.h"

#include "../Configuration/config.h"

#include <Titania/X3D/Components/Shape/Appearance.h>
#include <Titania/X3D/Components/Grouping/Transform.h>
#include <Titania/X3D/Components/Navigation/OrthoViewpoint.h>

namespace titania {
namespace puck {

TexturePreview::TexturePreview (X3DBaseInterface* const editor,
                                Gtk::Box & box,
                                Gtk::Label & label) :
	 X3DBaseInterface (editor -> getBrowserWindow (), editor -> getCurrentBrowser ()),
	X3DComposedWidget (editor),
	              box (box),
	            label (label),
	          preview (X3D::createBrowser (editor -> getMasterBrowser (), { get_ui ("Editors/TextureEditorPreview.x3dv") }))
{
	// Browser

	preview -> signal_configure_event () .connect (sigc::mem_fun (this, &TexturePreview::on_configure_event));
	preview -> initialized () .addInterest (this, &TexturePreview::set_initialized);
	preview -> setAntialiasing (4);
	preview -> show ();

	box .pack_start (*preview, true, true, 0);

	// Setup

	setup ();
}

void
TexturePreview::setTexture (const X3D::X3DPtr <X3D::X3DTextureNode> & value)
{
	if (textureNode)
		textureNode -> checkLoadState () .removeInterest (this, &TexturePreview::set_loadState);

	textureNode = value;

	if (textureNode)
		textureNode -> checkLoadState () .addInterest (this, &TexturePreview::set_loadState);

	set_texture ();
}

void
TexturePreview::set_initialized ()
{
	try
	{
		preview -> initialized () .removeInterest (this, &TexturePreview::set_initialized);
		preview -> set_opacity (1);
		preview -> getExecutionContext () -> getNamedNode ("Appearance") -> isPrivate (true);
	}
	catch (const X3D::X3DError &)
	{ }

	set_texture ();
}

void
TexturePreview::set_texture ()
{
	try
	{
		const X3D::AppearancePtr appearance (preview -> getExecutionContext () -> getNamedNode ("Appearance"));

		if (appearance)
		{
			if (appearance -> texture ())
				appearance -> texture () -> removeInterest (*preview, &X3D::Browser::addEvent);

			appearance -> texture () = textureNode;

			if (appearance -> texture ())
				appearance -> texture () -> addInterest (*preview, &X3D::Browser::addEvent);
		}
	}
	catch (const X3D::X3DError &)
	{ }

	set_camera ();
	set_loadState ();
}

void
TexturePreview::set_loadState ()
{
	if (not textureNode)
		return;

	std::string components;

	switch (textureNode -> getComponents ())
	{
		case 1: components = _ ("GRAY");       break;
		case 2: components = _ ("GRAY ALPHA"); break;
		case 3: components = _ ("RGB");        break;
		case 4: components = _ ("RGBA");       break;
		default:
			break;
	}

	label .set_text (std::to_string (textureNode -> getImageWidth ()) +
	                 " × " +
	                 std::to_string (textureNode -> getImageHeight ()) +
	                 " (" +
	                 components +
	                 ")");
}

bool
TexturePreview::on_configure_event (GdkEventConfigure* const)
{
	set_camera ();
	return false;
}

void
TexturePreview::set_camera ()
{
	if (not textureNode)
		return;

	try
	{
		const X3D::X3DPtr <X3D::OrthoViewpoint> viewpoint (preview -> getExecutionContext () -> getNamedNode ("OrthoViewpoint"));
		const X3D::X3DPtr <X3D::Transform>      transform (preview -> getExecutionContext () -> getNamedNode ("Texture2D"));

		if (viewpoint and transform)
		{
			double width  = textureNode -> getImageWidth ();
			double height = textureNode -> getImageHeight ();

			if (not width or not height)
			{
				width  = 1;
				height = 1;
			}

			viewpoint -> fieldOfView () = { -width, -height, width, height };
			transform -> scale ()       = X3D::Vector3f (width, height, 1);
		}
	}
	catch (const X3D::X3DError &)
	{ }
}

TexturePreview::~TexturePreview ()
{
	dispose ();
}

} // puck
} // titania