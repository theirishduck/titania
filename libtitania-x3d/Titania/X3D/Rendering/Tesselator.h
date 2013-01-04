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

#ifndef __TITANIA_X3D_RENDERING_TESSELATOR_H__
#define __TITANIA_X3D_RENDERING_TESSELATOR_H__

#include <Titania/Math/Numbers/Vector3.h>
#include <deque>
#include <iostream>
#include <tuple>

extern "C"
{
#include <GL/glew.h>

#include <GL/glu.h>

#include <GL/gl.h>
}

namespace titania {
namespace opengl {

typedef math::vector3 <double> vector3d;
typedef math::vector3 <float>  vector3f;

template <class ... Args>
class tesselator;

template <class ... Args>
class tesselator_vertex
{
public:

	typedef std::tuple <Args ...> Data;

	tesselator_vertex (const vector3f & point, const std::tuple <Args ...> data) :
		m_point (point),
		m_data  (data)
	{ }

	vector3d &
	point () { return m_point; }

	const vector3d &
	point () const { return m_point; }

	const Data &
	data () const { return m_data; }


private:

	vector3d m_point;
	Data     m_data;

};

template <class ... Args>
class polygon_element
{
public:

	typedef tesselator_vertex <Args ...>    Vertex;
	typedef std::deque <Vertex*>            VertexArray;
	typedef typename VertexArray::size_type size_type;

	polygon_element (GLenum type) :
		m_type (type),
		m_vertices ()
	{ }

	const GLenum &
	type () const { return m_type; }

	const Vertex &
	operator [ ] (size_t i) const { return *m_vertices [i]; }

	size_type
	size () const { return m_vertices .size (); }


private:

	GLenum      m_type;
	VertexArray m_vertices;

	friend class tesselator <Args ...>;

};

template <class ... Args>
class tesselator
{
public:

	typedef std::deque <polygon_element <Args ...>> Polygon;
	typedef tesselator_vertex <Args ...>             Vertex;

	tesselator ();

	void
	add_vertex (const vector3f &, const Args & ... args);

	void
	tesselate ();

	const Polygon &
	polygon () const { return tesselatedPolygon; }

	~tesselator ();


private:

	static void tessBeginData (GLenum, void*);

	static void
	tessVertexData (void*, void*);

	static void tessCombineData (GLdouble [3], void* [4], GLfloat [4], void**, void*);

	static void
	tessEndData (void*);

	static void tessError (GLenum);

	GLUtesselator*      tess;
	std::deque <Vertex> vertices;
	Polygon             tesselatedPolygon;

};

template <class ... Args>
tesselator <Args ...>::tesselator ()
{
	tess = gluNewTess ();

	if (tess)
	{
		gluTessProperty (tess, GLU_TESS_BOUNDARY_ONLY, GLU_FALSE);
		gluTessCallback (tess, GLU_TESS_BEGIN_DATA, _GLUfuncptr (&tesselator::tessBeginData));
		gluTessCallback (tess, GLU_TESS_VERTEX_DATA, _GLUfuncptr (&tesselator::tessVertexData));

		//gluTessCallback(tesselator, GLU_TESS_COMBINE_DATA, (_GLUfuncptr)&IndexedFaceSet::tessCombineData);
		gluTessCallback (tess, GLU_TESS_END_DATA, _GLUfuncptr (&tesselator::tessEndData));
		gluTessCallback (tess, GLU_TESS_ERROR, _GLUfuncptr (&tesselator::tessError));
	}
}

template <class ... Args>
void
tesselator <Args ...>::add_vertex (const vector3f & point, const Args & ... args)
{
	vertices .emplace_back (point, std::forward_as_tuple (args ...));
}

template <class ... Args>
void
tesselator <Args ...>::tesselate ()
{
	gluTessBeginPolygon (tess, &tesselatedPolygon);
	gluTessBeginContour (tess);

	for (auto & vertex : vertices)
		gluTessVertex (tess, vertex .point () .data (), &vertex);

	//gluTessEndContour(tess);
	gluEndPolygon (tess);
}

template <class ... Args>
void
tesselator <Args ...>::tessBeginData (GLenum type, void* polygon_data)
{
	Polygon* polygon = (Polygon*) polygon_data;

	polygon -> emplace_back (type);
}

template <class ... Args>
void
tesselator <Args ...>::tessVertexData (void* vertex_data, void* polygon_data)
{
	Polygon* polygon = (Polygon*) polygon_data;
	Vertex*  vertex  = (Vertex*) vertex_data;

	polygon -> back () .m_vertices .emplace_back (vertex);
}

template <class ... Args>
void
tesselator <Args ...>::tessCombineData (GLdouble coords [3], void* vertex_data [4],
                                        GLfloat weight [4], void** outData,
                                        void* polygon_data)
{
	// Not used yet
}

template <class ... Args>
void
tesselator <Args ...>::tessEndData (void* polygon_data)
{ }

template <class ... Args>
void
tesselator <Args ...>::tessError (GLenum err_no)
{
	std::clog << "Warning: tesselation error: '" << (char*) gluErrorString (err_no) << "'." << std::endl;
}

template <class ... Args>
tesselator <Args ...>::~tesselator ()
{
	if (tess)
		gluDeleteTess (tess);
}

} // opengl
} // titania

#endif
