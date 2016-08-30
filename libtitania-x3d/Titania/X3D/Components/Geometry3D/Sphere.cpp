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

#include "Sphere.h"

#include "../../Browser/Geometry3D/X3DSphereOptionNode.h"
#include "../../Browser/X3DBrowser.h"
#include "../../Execution/X3DExecutionContext.h"

namespace titania {
namespace X3D {

const ComponentType Sphere::component      = ComponentType::GEOMETRY_3D;
const std::string   Sphere::typeName       = "Sphere";
const std::string   Sphere::containerField = "geometry";

Sphere::Fields::Fields () :
	radius (new SFFloat (1)),
	 solid (new SFBool (true))
{ }

Sphere::Sphere (X3DExecutionContext* const executionContext) :
	    X3DBaseNode (executionContext -> getBrowser (), executionContext),
	X3DGeometryNode (),
	         fields ()
{
	addType (X3DConstants::Sphere);

	addField (inputOutput,    "metadata", metadata ());
	addField (initializeOnly, "radius",   radius ());
	addField (initializeOnly, "solid",    solid ());

	radius () .setUnit (UnitCategory::LENGTH);
}

X3DBaseNode*
Sphere::create (X3DExecutionContext* const executionContext) const
{
	return new Sphere (executionContext);
}

void
Sphere::initialize ()
{
	X3DGeometryNode::initialize ();

	getBrowser () -> getSphereOptions () .addInterest (this, &Sphere::update);
}

void
Sphere::setExecutionContext (X3DExecutionContext* const executionContext)
throw (Error <INVALID_OPERATION_TIMING>,
       Error <DISPOSED>)
{
	if (isInitialized ())
		getBrowser () -> getSphereOptions () .removeInterest (this, &Sphere::update);

	X3DGeometryNode::setExecutionContext (executionContext);

	if (isInitialized ())
		getBrowser () -> getSphereOptions () .addInterest (this, &Sphere::update);
}

Box3d
Sphere::createBBox () const
{
	const double diameter = 2 * radius ();

	return Box3d (Vector3d (diameter, diameter, diameter), Vector3d ());
}

void
Sphere::build ()
{
	const auto & options = getBrowser () -> getSphereOptions ();

	getTexCoords () .emplace_back ();
	getTexCoords () [0] .reserve (options -> getTexCoords () .size ());
	getTexCoords () [0] = options -> getTexCoords ();

	getNormals () .reserve (options -> getNormals () .size ());
	getNormals () = options -> getNormals  ();

	if (radius () == 1.0f)
		getVertices () = options -> getVertices ();

	else
	{
		getVertices () .reserve (options -> getVertices () .size ());

		for (const auto & vertex : options -> getVertices ())
			getVertices () .emplace_back (vertex * double (radius () .getValue ()));
	}

	addElements (options -> getVertexMode (), getVertices () .size ());
	setSolid (solid ());
}

SFNode
Sphere::toPrimitive () const
throw (Error <NOT_SUPPORTED>,
       Error <DISPOSED>)
{
	const auto & options  = getBrowser () -> getSphereOptions ();
	const auto   geometry = options -> toPrimitive (getExecutionContext ());

	geometry -> getField <SFNode> ("metadata") = metadata ();
	geometry -> getField <SFBool> ("solid")    = solid ();

	if (radius () == 1.0f)
		return geometry;

	for (auto & point : geometry -> getField <SFNode> ("coord") -> getField <MFVec3f> ("point"))
		point *= radius () .getValue ();

	return geometry;
}

} // X3D
} // titania
