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

#include "X3DGeometryNode.h"

namespace titania {
namespace X3D {

X3DGeometryNode::X3DGeometryNode () :
	         X3DNode (),      
	             ccw (true),  // SFBool  [ ]      ccw              TRUE
	           solid (true),  // SFBool  [ ]      solid            TRUE
	     creaseAngle (),      // SFFloat [ ]      creaseAngle      0           [0,∞)
	texCoordBufferId (0),     
	   colorBufferId (0),     
	  normalBufferId (0),     
	   pointBufferId (0),      
	     bufferUsage (GL_STATIC_DRAW) 
{
	addNodeType (X3DGeometryNodeType);
}

void
X3DGeometryNode::initialize ()
{
	X3DNode::initialize ();

	if (not GLEW_ARB_vertex_buffer_object)
		throw std::runtime_error ("X3DGeometryNode::initialize: The glew vertex buffer objects are not supported.");

	glGenBuffers (1, &texCoordBufferId);
	glGenBuffers (1, &colorBufferId);
	glGenBuffers (1, &normalBufferId);
	glGenBuffers (1, &pointBufferId);
}

void
X3DGeometryNode::eventsProcessed ()
{
	X3DNode::eventsProcessed ();

	update ();
}

const Box3f
X3DGeometryNode::getBBox ()
{
	if (not glPoints  .size ())
		update ();

	return bbox;
}

Box3f
X3DGeometryNode::createBBox ()
{
	if (glIndices >= 3)
	{
		Vector3f min = glPoints [0];
		Vector3f max = min;

		for (int i = 3; i < glIndices; ++ i)
		{
			min = math::min (min, glPoints [i]);
			max = math::max (max, glPoints [i]);
		}

		Vector3f size   = max - min;
		Vector3f center = min + size * 0.5f;

		//size = size .max(Vector3f(1e-5, 1e-5, 1e-5));

		return Box3f (size, center);
	}

	return Box3f ();
}

bool
X3DGeometryNode::intersect (const Line3f & hitRay, Hit*) const
{
	if (bbox .intersect (hitRay))
	{
		return true;
	}

	return false;
}

/*
 *  normalIndex: a map of vertices with an array of the normals associated to this vertex
 *
 *    [vertexIndex] -> [normalIndex1, normalIndex2, ... ]
 *
 *  normals: an array of a face normal for each vertex
 *
 *  Assume we have two polygons where two points (p2, p3) share more than one vertex. 
 * 
 *  p1                        p3
 *     v1 ------------- v3 v5
 *      | n1         n3  /|
 *      |              /  |
 *      |            / n5 |
 *      |          /      |
 *      |        /        |
 *      |      /          |
 *      | n2 /            |
 *      |  /  n4          |
 *      |/            n6  |
 *     v2 v4 ------------- v6
 *  p2                        p4                     
 *
 *  For these two polygons the normalIndex and the normal array would look like this:
 *
 *  normalIndex:
 *    [p1] -> [n1]
 *    [p2] -> [n2, n4]
 *    [p3] -> [n3, n5]
 *    [p4] -> [n6]
 *
 *  normals:	
 *    [n1, n2, n3, n4, n5, n6]
 */

void
X3DGeometryNode::refineNormals (const NormalIndex & normalIndex, std::vector <Vector3f> & normals)
{
	if (creaseAngle == 0.0f)
		return;

	if (not ccw)
	{
		for (auto & normal : normals)
			normal = -normal;
	}

	float cosCreaseAngle = std::cos (creaseAngle);

	std::vector <Vector3f> _normals (normals .size ());

	for (const auto & vertex : normalIndex)
	{
		for (const auto & index : vertex .second)
		{
			Vector3f         n;
			const Vector3f & m = normals [index];

			for (const auto & index : vertex .second)
			{
				if (dot (normals [index], m) > cosCreaseAngle)
					n += normals [index];
			}

			_normals [index] = normalize (n);
		}
	}

	normals .swap (_normals);
}

void
X3DGeometryNode::update ()
{
	clear ();
	build ();
	transfer ();
	bbox = createBBox ();
}

void
X3DGeometryNode::clear ()
{
	glTexCoord .clear ();
	textureCoordinateGenerator = nullptr;
	glColors     .clear ();
	glColorsRGBA .clear ();
	glNormals    .clear ();
	glPoints     .clear ();
	glIndices = 0;
	glVertexMode    = GL_TRIANGLES;
}

void
X3DGeometryNode::build ()
{ }

void
X3DGeometryNode::transfer ()
{
	glBindBuffer (GL_ARRAY_BUFFER, texCoordBufferId);
	glBufferData (GL_ARRAY_BUFFER, sizeof (Vector2f) * glTexCoord .size (), glTexCoord .data (), bufferUsage);

	if (glColors .size ())
	{
		glBindBuffer (GL_ARRAY_BUFFER, colorBufferId);
		glBufferData (GL_ARRAY_BUFFER, sizeof (Color3f) * glColors .size (), glColors .data (), bufferUsage);
	}
	else
	{
		glBindBuffer (GL_ARRAY_BUFFER, colorBufferId);
		glBufferData (GL_ARRAY_BUFFER, sizeof (Color4f) * glColorsRGBA .size (), glColorsRGBA .data (), bufferUsage);
	}
	
	glBindBuffer (GL_ARRAY_BUFFER, normalBufferId);
	glBufferData (GL_ARRAY_BUFFER, sizeof (Vector3f) * glNormals .size (), glNormals .data (), bufferUsage);

	glBindBuffer (GL_ARRAY_BUFFER, pointBufferId);
	glBufferData (GL_ARRAY_BUFFER, sizeof (Vector3f) * glPoints .size (), glPoints .data (), bufferUsage);

	glBindBuffer (GL_ARRAY_BUFFER, 0);

	bufferUsage = GL_DYNAMIC_DRAW;
}

void
X3DGeometryNode::display ()
{
	if (not glPoints  .size ())
		update ();

	draw ();
}

void
X3DGeometryNode::draw ()
{
	if (not glIndices or not glPoints .size ())
		return;

	if (solid)
	{
		glEnable (GL_CULL_FACE);
	}
	else
		glDisable (GL_CULL_FACE);

	glFrontFace (ccw ? GL_CCW : GL_CW);

	if (glIsEnabled (GL_TEXTURE_2D))
	{
		if (textureCoordinateGenerator)
			textureCoordinateGenerator -> enable ();
			
		else if (glTexCoord .size ())
		{
			glBindBuffer (GL_ARRAY_BUFFER, texCoordBufferId);
			glEnableClientState (GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer (2, GL_FLOAT, 0, 0);
		}
	}

	if (glColors .size () or glColorsRGBA .size ())
	{
		if (glIsEnabled (GL_LIGHTING))
			glEnable (GL_COLOR_MATERIAL);

		glBindBuffer (GL_ARRAY_BUFFER, colorBufferId);
		glEnableClientState (GL_COLOR_ARRAY);
		glColorPointer (glColors .size () ? 3 : 4, GL_FLOAT, 0, 0);
	}

	if (glIsEnabled (GL_LIGHTING))
	{
		if (glNormals .size ())
		{
			glBindBuffer (GL_ARRAY_BUFFER, normalBufferId);
			glEnableClientState (GL_NORMAL_ARRAY);
			glNormalPointer (GL_FLOAT, 0, 0);
		}
	}

	glBindBuffer (GL_ARRAY_BUFFER, pointBufferId);
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer (3, GL_FLOAT, 0, 0);

	glDrawArrays (glVertexMode, 0, glIndices);

	if (textureCoordinateGenerator)
		textureCoordinateGenerator -> disable ();

	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_COLOR_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);

	glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void
X3DGeometryNode::dispose ()
{
	if (texCoordBufferId)
		glDeleteBuffers (1, &texCoordBufferId);

	if (colorBufferId)
		glDeleteBuffers (1, &colorBufferId);

	if (normalBufferId)
		glDeleteBuffers (1, &normalBufferId);

	if (pointBufferId)
		glDeleteBuffers (1, &pointBufferId);

	X3DNode::dispose ();
}

} // X3D
} // titania
