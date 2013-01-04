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
 ******************************************************************************/

#include "Cylinder.h"

#include "../../Execution/X3DExecutionContext.h"

#define SEGMENTS 16.0

namespace titania {
namespace X3D {

Cylinder::Cylinder (X3DExecutionContext* const executionContext) :
	    X3DBaseNode (executionContext -> getBrowser (), executionContext), 
	X3DGeometryNode (),                                                    
	         bottom (true),                                                // SFBool  [ ] bottom  TRUE
	         height (2),                                                   // SFFloat [ ] height  2           (0,∞)
	         radius (1),                                                   // SFFloat [ ] radius  1           (0,∞)
	           side (true),                                                // SFBool  [ ] side    TRUE
	            top (true)                                                 // SFBool  [ ] top     TRUE
{
	setComponent ("Geometry3D");
	setTypeName ("Cylinder");

	addField (inputOutput,    "metadata", metadata);
	addField (initializeOnly, "top",      top);
	addField (initializeOnly, "bottom",   bottom);
	addField (initializeOnly, "side",     side);
	addField (initializeOnly, "height",   height);
	addField (initializeOnly, "radius",   radius);
	addField (initializeOnly, "solid",    solid);
}

X3DBaseNode*
Cylinder::create (X3DExecutionContext* const executionContext) const
{
	return new Cylinder (executionContext);
}

Box3f
Cylinder::createBBox ()
{
	float diameter = radius * 2;

	if (not top and not side and not bottom)
		return Box3f ();

	else if (not top and not side)
		return Box3f (Vector3f (diameter, 0, diameter), Vector3f (0, -height / 2, 0));

	else if (not bottom and not side)
		return Box3f (Vector3f (diameter, 0, diameter), Vector3f (0, height / 2, 0));

	else
		return Box3f (Vector3f (diameter, height, diameter), Vector3f ());
}

void
Cylinder::build ()
{
	float y1      = height / 2;
	float y2      = -y1;
	float _radius = radius;

	for (int i = 0; i < SEGMENTS; ++ i)
	{
		float u1     = i / SEGMENTS;
		float theta1 = 2 * M_PI * u1;
		float x1     = -std::sin (theta1);
		float z1     = -std::cos (theta1);

		float u2     = (i + 1) / SEGMENTS;
		float theta2 = 2 * M_PI * u2;
		float x2     = -std::sin (theta2);
		float z2     = -std::cos (theta2);

		if (top)
		{
			/*   p1
			 *  /  \
			 * p2 - p3
			 */

			// p1
			getTexCoord () .emplace_back (0.5, 0.5);
			getNormals  () .emplace_back (0, 1, 0);
			getVertices () .emplace_back (0, y1, 0);

			// p2
			getTexCoord () .emplace_back (+(x1 + 1) / 2, -(z1 - 1) / 2);
			getNormals  () .emplace_back (0, 1, 0);
			getVertices () .emplace_back (x1 * _radius, y1, z1 * _radius);

			// p3
			getTexCoord () .emplace_back (+(x2 + 1) / 2, -(z2 - 1) / 2);
			getNormals  () .emplace_back (0, 1, 0);
			getVertices () .emplace_back (x2 * _radius, y1, z2 * _radius);
		}

		if (side)
		{
			// p1 - p4
			//  | \ |
			// p2 - p3

			// p1
			getTexCoord () .emplace_back (u1, 1);
			getNormals  () .emplace_back (x1, 0, z1);
			getVertices () .emplace_back (x1 * _radius, y1, z1 * _radius);

			// p2
			getTexCoord () .emplace_back (u1, 0);
			getNormals  () .emplace_back (x1, 0, z1);
			getVertices () .emplace_back (x1 * _radius, y2, z1 * _radius);

			// p3
			getTexCoord () .emplace_back (u2, 0);
			getNormals  () .emplace_back (x2, 0, z2);
			getVertices () .emplace_back (x2 * _radius, y2, z2 * _radius);

			// p4
			getTexCoord () .emplace_back (u2, 1);
			getNormals  () .emplace_back (x2, 0, z2);
			getVertices () .emplace_back (x2 * _radius, y1, z2 * _radius);

			// p1
			getTexCoord () .emplace_back (u1, 1);
			getNormals  () .emplace_back (x1, 0, z1);
			getVertices () .emplace_back (x1 * _radius, y1, z1 * _radius);

			// p3
			getTexCoord () .emplace_back (u2, 0);
			getNormals  () .emplace_back (x2, 0, z2);
			getVertices () .emplace_back (x2 * _radius, y2, z2 * _radius);
		}

		if (bottom)
		{
			/*   p1
			 *  /  \
			 * p2 - p3
			 */

			// p1
			getTexCoord () .emplace_back (0.5, 0.5);
			getNormals  () .emplace_back (0, -1, 0);
			getVertices () .emplace_back (0, y2, 0);

			// p3
			getTexCoord () .emplace_back ((x2 + 1) / 2, (z2 + 1) / 2);
			getNormals  () .emplace_back (0, -1, 0);
			getVertices () .emplace_back (x2 * _radius, y2, z2 * _radius);

			// p2
			getTexCoord () .emplace_back ((x1 + 1) / 2, (z1 + 1) / 2);
			getNormals  () .emplace_back (0, -1, 0);
			getVertices () .emplace_back (x1 * _radius, y2, z1 * _radius);
		}
	}

	setVertexMode (GL_TRIANGLES);
}

} // X3D
} // titania
