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

#include "X3DGroupingNode.h"

#include "../../Bits/Cast.h"
#include "../../Browser/X3DBrowser.h"

#include <Titania/Utility/Adapter.h>

namespace titania {
namespace X3D {

X3DGroupingNode::Fields::Fields () :
	addChildren (new MFNode ()),
	removeChildren (new MFNode ()),
	children (new MFNode ())
{ }

X3DGroupingNode::X3DGroupingNode () :
	    X3DChildNode (), 
	X3DBoundedObject (), 
	          fields (),
	          handle () 
{
	addNodeType (X3DConstants::X3DGroupingNode);
	
	addChildren (handle);
}

void
X3DGroupingNode::initialize ()
{
	X3DChildNode::initialize ();
	X3DBoundedObject::initialize ();

	addChildren ()    .addInterest (this, &X3DGroupingNode::set_addChildren);
	removeChildren () .addInterest (this, &X3DGroupingNode::set_removeChildren);
	children ()       .addInterest (this, &X3DGroupingNode::set_children);

	set_children ();
}

Box3f
X3DGroupingNode::getBBox ()
{
	if (bboxSize () == Vector3f (-1, -1, -1))
		return X3DBoundedObject::getBBox (children ());

	return Box3f (bboxSize (), bboxCenter ());
}

void
X3DGroupingNode::addHandle ()
{
	handle = new TransformHandle (this, getExecutionContext ());
	handle -> setup ();
}

void
X3DGroupingNode::removeHandle ()
{
	handle = NULL;
}

void
X3DGroupingNode::set_addChildren ()
{
	if (addChildren () .size ())
	{
		add (addChildren ());

		children () .insert (children () .end (), addChildren () .begin (), addChildren () .end ());

		//children .removeInterest (this, &X3DGroupingNode::set_children);
		//children .addInterest    (this, &X3DGroupingNode::set_endChildren);
	}
}

void
X3DGroupingNode::set_removeChildren ()
{
	if (removeChildren () .size ())
	{
		auto new_end = basic::remove (children () .begin (), children () .end (), removeChildren () .begin (), removeChildren () .end ());
		children () .erase (new_end, children () .end ());

		//children .removeInterest (this, &X3DGroupingNode::set_children);
		//children .addInterest    (this, &X3DGroupingNode::set_endChildren);

		set_children ();
	}
}

//void
//X3DGroupingNode::set_endChildren ()
//{
//	children () .removeInterest (this, &X3DGroupingNode::set_endChildren);
//	children () .addInterest    (this, &X3DGroupingNode::set_children);
//}

void
X3DGroupingNode::set_children ()
{
	pointingDeviceSensors .clear ();
	lights     .clear ();
	localFogs  .clear ();
	childNodes .clear ();

	add (children ());
}

void
X3DGroupingNode::add (const MFNode & children)
{
	for (const auto & child : children)
	{
		auto pointingDeviceSensorNode = x3d_cast <X3DPointingDeviceSensorNode*> (child);

		if (pointingDeviceSensorNode)
			pointingDeviceSensors .emplace_back (pointingDeviceSensorNode);

		else
		{
			auto lightNode = x3d_cast <X3DLightNode*> (child);

			if (lightNode)
				lights .emplace_back (lightNode);

			else
			{
				auto localFog = x3d_cast <LocalFog*> (child);

				if (localFog)
					localFogs .emplace_back (localFog);

				else
				{
					auto childNode = x3d_cast <X3DChildNode*> (child);

					if (childNode)
						childNodes .emplace_back (childNode);
				}
			}
		}
	}
}

void
X3DGroupingNode::traverse (TraverseType type)
{
	switch (type)
	{
		case TraverseType::PICKING:
		{
			pick ();
			break;
		}
		case TraverseType::CAMERA:
		case TraverseType::NAVIGATION:
		case TraverseType::COLLISION:
		{
			for (const auto & child : childNodes)
				child -> traverse (type);

			break;
		}
		case TraverseType::COLLECT:
		{
			collect ();
			break;
		}
	}
	
	if (handle)
		handle -> traverse (type);
}

void
X3DGroupingNode::pick ()
{
	if (pointingDeviceSensors .size ())
	{
		getBrowser () -> getSensors () .emplace_back ();

		for (const auto & child : pointingDeviceSensors)
			child -> push ();
	}

	for (const auto & child : childNodes)
		child -> traverse (TraverseType::PICKING);

	if (pointingDeviceSensors .size ())
		getBrowser () -> getSensors () .pop_back ();
}

void
X3DGroupingNode::collect ()
{
	for (const auto & child : lights)
		child -> push ();

	if (localFogs .size ())
		localFogs .front () -> push ();

	for (const auto & child : childNodes)
		child -> traverse (TraverseType::COLLECT);

	if (localFogs .size ())
		localFogs .front () -> pop ();

	for (const auto & child : basic::adapter (lights .crbegin (), lights .crend ()))
		child -> pop ();
}

void
X3DGroupingNode::dispose ()
{
	handle .dispose ();

	X3DBoundedObject::dispose ();
	X3DChildNode::dispose ();
}

} // X3D
} // titania
