/* -*- Mode: C++; coding: utf-8; tab-width: 3; indent-tabs-mode: tab; c-basic-offset: 3 -*-
 *******************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright create3000, Scheffelstra�e 31a, Leipzig, Germany 2011.
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

#include "X3DEditorInterface.h"

#include "../Browser/X3DBrowserWindow.h"
#include "../Browser/BrowserSelection.h"

namespace titania {
namespace puck {

///  @name Construction

X3DEditorInterface::X3DEditorInterface () :
	X3DEditorInterface ("", "")
{ }

X3DEditorInterface::X3DEditorInterface (const std::string & widgetName, const std::string & configKey) :
	X3DDialogInterface (widgetName, configKey),
	   X3DEditorObject ()
{
}

void
X3DEditorInterface::setup ()
{
	X3DDialogInterface::setup ();
	X3DEditorObject::setup ();
}

void
X3DEditorInterface::initialize ()
{
	X3DDialogInterface::initialize ();
	X3DEditorObject::initialize ();

	getWidget () .signal_map ()   .connect (sigc::mem_fun (this, &X3DEditorInterface::on_map));
	getWidget () .signal_unmap () .connect (sigc::mem_fun (this, &X3DEditorInterface::on_unmap));

	on_map ();
}

///  @name Event handler

void
X3DEditorInterface::on_map ()
{
	getBrowserWindow () -> getSelection () -> getChildren () .addInterest (this, &X3DEditorInterface::set_selection);

	set_selection (getBrowserWindow () -> getSelection () -> getChildren ());
}

void
X3DEditorInterface::on_unmap ()
{
	getBrowserWindow () -> getSelection () -> getChildren () .removeInterest (this, &X3DEditorInterface::set_selection);

	set_selection ({ });
}

///  @name Destruction

void
X3DEditorInterface::dispose ()
{
	X3DEditorObject::dispose ();
	X3DDialogInterface::dispose ();
}

X3DEditorInterface::~X3DEditorInterface ()
{ }

} // puck
} // titania