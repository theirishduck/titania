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
 * Copyright 1999, 2016 Holger Seelig <holger.seelig@yahoo.de>.
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

#include "X3DKeyDeviceSensorNode.h"

#include "../../Browser/X3DBrowser.h"

namespace titania {
namespace X3D {

X3DKeyDeviceSensorNode::X3DKeyDeviceSensorNode () :
	X3DSensorNode ()
{
	addType (X3DConstants::X3DKeyDeviceSensorNode);
}

void
X3DKeyDeviceSensorNode::initialize ()
{
	X3DSensorNode::initialize ();

	getExecutionContext () -> isLive () .addInterest (&X3DKeyDeviceSensorNode::set_live, this);
	isLive () .addInterest (&X3DKeyDeviceSensorNode::set_live, this);

	set_live ();
}

void
X3DKeyDeviceSensorNode::setExecutionContext (X3DExecutionContext* const executionContext)
throw (Error <INVALID_OPERATION_TIMING>,
       Error <DISPOSED>)
{
	if (isInitialized ())
	{
		getExecutionContext () -> isLive () .removeInterest (&X3DKeyDeviceSensorNode::set_live, this);
	
		if (executionContext -> getBrowser () not_eq getBrowser ())
		{
			if (isActive ())
				getBrowser () -> setKeyDeviceSensor (this);
		}
	}

	X3DSensorNode::setExecutionContext (executionContext);

	if (isInitialized ())
	{
		getExecutionContext () -> isLive () .addInterest (&X3DKeyDeviceSensorNode::set_live, this);

		set_live ();
	}
}

void
X3DKeyDeviceSensorNode::enable ()
{
	if (isActive ())
		return;

	X3DKeyDeviceSensorNode* const keyDeviceSensorNode = getBrowser () -> getKeyDeviceSensor ();

	if (keyDeviceSensorNode)
	{
		keyDeviceSensorNode -> enabled ()  = false;
		keyDeviceSensorNode -> isActive () = false;
	}

	getBrowser () -> setKeyDeviceSensor (this);

	isActive () = true;
}

void
X3DKeyDeviceSensorNode::disable ()
{
	if (not isActive ())
		return;

	getBrowser () -> setKeyDeviceSensor (nullptr);

	setKeyReleaseEvent ();
	isActive () = false;
}

void
X3DKeyDeviceSensorNode::set_live ()
{
	if (getExecutionContext () -> isLive () and isLive ())
	{
		enabled () .addInterest (&X3DKeyDeviceSensorNode::set_enabled, this);

		if (enabled ())
			enable ();
	}
	else
	{
		enabled () .removeInterest (&X3DKeyDeviceSensorNode::set_enabled, this);

		if (enabled ())
			disable ();
	}
}

void
X3DKeyDeviceSensorNode::set_enabled ()
{
	if (enabled ())
		enable ();

	else
		disable ();
}

} // X3D
} // titania
