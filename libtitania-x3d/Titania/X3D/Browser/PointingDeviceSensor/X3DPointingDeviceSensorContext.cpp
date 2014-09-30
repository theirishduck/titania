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

#include "X3DPointingDeviceSensorContext.h"

#include "../../Components/Networking/Anchor.h"
#include "../../Components/PointingDeviceSensor/X3DDragSensorNode.h"
#include "../../Components/PointingDeviceSensor/X3DTouchSensorNode.h"
#include "../../Execution/World.h"
#include "../../Rendering/ViewVolume.h"
#include "../ContextLock.h"
#include "../Selection.h"
#include "../X3DBrowser.h"

namespace titania {
namespace X3D {

static constexpr time_type SELECTION_TIME = 0.01; // Use ExamineViewer SPIN_RELEASE_TIME here.

X3DPointingDeviceSensorContext::X3DPointingDeviceSensorContext () :
	   X3DBaseNode (),
	     sensitive (true),
	       pointer (),
	        hitRay (),
	          hits (),
	enabledSensors ({ NodeSet () }),
	   overSensors (),
	 activeSensors (),
	 selectedLayer (),
	     pressTime (0),
	      hasMoved (false)
{
	addChildren (sensitive,
	             overSensors,
	             activeSensors);
}

void
X3DPointingDeviceSensorContext::initialize ()
{
	getBrowser () -> shutdown () .addInterest (this, &X3DPointingDeviceSensorContext::set_shutdown);
}

void
X3DPointingDeviceSensorContext::set_shutdown ()
{
	hits          .clear ();
	overSensors   .clear ();
	activeSensors .clear ();
}

void
X3DPointingDeviceSensorContext::click (const double x, const double y)
{
	pointer = Vector2d (x, y);

	// Clear hits.

	hits .clear ();

	// Pick.

	ContextLock lock (getBrowser ());

	if (lock)
	{
		//update (); // We cannot make an update here because of gravity.

		getWorld () -> traverse (TraverseType::POINTER);
	}

	// Picking end.

	std::stable_sort (hits .begin (), hits .end (), HitComp { });

	enabledSensors = { NodeSet () };
}

bool
X3DPointingDeviceSensorContext::isPointerInRectangle (const Vector4i & rectangle) const
{
	return pointer .x () > rectangle .x () and 
	       pointer .x () < rectangle .x () + rectangle .z () and
	       pointer .y () > rectangle .y () and 
	       pointer .y () < rectangle .y () + rectangle .w ();
}

Line3d
X3DPointingDeviceSensorContext::getHitRay (const Matrix4d & modelViewMatrix, const Matrix4d & projectionMatrix, const Vector4i & viewport) const
{
	try
	{
		return ViewVolume::unProjectLine (pointer .x (), pointer .y (), modelViewMatrix, projectionMatrix, viewport);
	}
	catch (const std::domain_error &)
	{
		return Line3d (Vector3d (), Vector3d ());
	}
}

void
X3DPointingDeviceSensorContext::addHit (const Matrix4d & transformationMatrix, const IntersectionPtr & intersection, X3DShapeNode* const shape, X3DLayerNode* const layer)
{
	hits .emplace_front (new Hit (pointer, transformationMatrix, hitRay, intersection, enabledSensors .back (), shape, layer));
}

bool
X3DPointingDeviceSensorContext::motionNotifyEvent (const double x, const double y)
{
	hasMoved |= pointer .x () not_eq x or pointer .y () not_eq y;

	click (x, y);

	motion ();

	return not getHits () .empty () and not getNearestHit () -> sensors .empty ();
}

void
X3DPointingDeviceSensorContext::motion ()
{
	// Set isOver to FALSE for appropriate nodes

	std::vector <X3DBaseNode*> difference;

	if (getHits () .empty ())
		difference .assign (overSensors .begin (), overSensors .end ());

	else
	{
		// overSensors and sensors are always sorted.

		std::set_difference (overSensors .begin (), overSensors .end (),
		                     getNearestHit () -> sensors .begin (), getNearestHit () -> sensors .end (),
		                     std::back_inserter (difference));
	}

	for (const auto & node : difference)
	{
		const auto pointingDeviceSensorNode = dynamic_cast <X3DPointingDeviceSensorNode*> (node);

		if (pointingDeviceSensorNode)
			pointingDeviceSensorNode -> set_over (getNearestHit (), false);

		else
		{
			const auto anchor = dynamic_cast <Anchor*> (node);

			if (anchor)
				anchor -> set_over (false);
		}
	}

	// Set isOver to TRUE for appropriate nodes

	if (getHits () .empty ())
		overSensors .clear ();

	else
	{
		overSensors .assign (getNearestHit () -> sensors .begin (),
		                     getNearestHit () -> sensors .end ());

		for (const auto & node : overSensors)
		{
			const auto pointingDeviceSensorNode = dynamic_cast <X3DPointingDeviceSensorNode*> (node .getValue ());

			if (pointingDeviceSensorNode)
				pointingDeviceSensorNode -> set_over (getNearestHit (), true);

			else
			{
				const auto anchor = dynamic_cast <Anchor*> (node .getValue ());

				if (anchor)
					anchor -> set_over (true);
			}
		}
	}

	// Forward motion event to active drag sensor nodes

	for (const auto & node : activeSensors)
	{
		const auto dragSensorNode = dynamic_cast <X3DDragSensorNode*> (node .getValue ());

		if (dragSensorNode)
		{
			dragSensorNode -> set_motion (getHits () .empty ()
			                              ? std::make_shared <Hit> (pointer, Matrix4d (), hitRay, std::make_shared <Intersection> (), NodeSet (), nullptr, nullptr)
													: getNearestHit ());
		}
	}
}

bool
X3DPointingDeviceSensorContext::buttonPressEvent (const double x, const double y)
{
	pressTime = chrono::now ();
	hasMoved  = false;

	click (x, y);

	if (getHits () .empty ())
		return false;
		
	if (getNearestHit () -> sensors .empty ())
		return false;

	selectedLayer = getNearestHit () -> layer;

	activeSensors .assign (getNearestHit () -> sensors .begin (),
	                       getNearestHit () -> sensors .end ());

	for (const auto & node : activeSensors)
	{
		const auto pointingDeviceSensorNode = dynamic_cast <X3DPointingDeviceSensorNode*> (node .getValue ());

		if (pointingDeviceSensorNode)
			pointingDeviceSensorNode -> set_active (getNearestHit (), true);

		else
		{
			const auto anchor = dynamic_cast <Anchor*> (node .getValue ());

			if (anchor)
				anchor -> set_active (true);
		}
	}

	return true;
}

bool
X3DPointingDeviceSensorContext::buttonReleaseEvent ()
{
	if (not hasMoved or chrono::now () - pressTime < SELECTION_TIME)
	{
		if (getBrowser () -> getSelection () -> select ())
			return true;
	}

	selectedLayer = nullptr;

	for (const auto & node : activeSensors)
	{
		const auto pointingDeviceSensorNode = dynamic_cast <X3DPointingDeviceSensorNode*> (node .getValue ());

		if (pointingDeviceSensorNode)
			pointingDeviceSensorNode -> set_active (std::make_shared <Hit> (pointer, Matrix4d (), hitRay, std::make_shared <Intersection> (), NodeSet (), nullptr, nullptr),
			                                        false);

		else
		{
			const auto anchor = dynamic_cast <Anchor*> (node .getValue ());

			if (anchor)
				anchor -> set_active (false);
		}
	}

	activeSensors .clear ();

	// Selection

	return true;
}

void
X3DPointingDeviceSensorContext::leaveNotifyEvent ()
{
	// Clear hits.

	hits .clear ();

	motion ();
}

void
X3DPointingDeviceSensorContext::dispose ()
{ }

X3DPointingDeviceSensorContext::~X3DPointingDeviceSensorContext ()
{ }

} // X3D
} // titania
