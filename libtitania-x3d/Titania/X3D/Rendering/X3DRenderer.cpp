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

#include "X3DRenderer.h"

#include "../Browser/X3DBrowser.h"
#include "../Components/Navigation/X3DViewpointNode.h"

#include <Titania/Physics/Constants.h>
#include <Titania/Utility/Adapter.h>
#include <algorithm>
#include <stdexcept>

#define DEPTH_BUFFER_WIDTH   16
#define DEPTH_BUFFER_HEIGHT  16

namespace titania {
namespace X3D {

class X3DRenderer::DepthBuffer
{
public:

	DepthBuffer (size_t width, size_t height) :
		width (width),
		height (height),
		id (0),
		depthBuffer (0),
		depth (width * height)
	{
		if (glXGetCurrentContext ()) // GL_EXT_framebuffer_object
		{
			glGenFramebuffers (1, &id);
			glGenRenderbuffers (1, &depthBuffer);

			// Bind frame buffer.
			glBindFramebuffer (GL_FRAMEBUFFER, id);

			// The depth buffer
			glBindRenderbuffer (GL_RENDERBUFFER, depthBuffer);
			glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
			glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

			// Always check that our framebuffer is ok
			if (glCheckFramebufferStatus (GL_FRAMEBUFFER) not_eq GL_FRAMEBUFFER_COMPLETE)
				throw std::runtime_error ("Couldn't create frame buffer.");

			glBindFramebuffer (GL_FRAMEBUFFER, 0);
		}
	}

	double
	getDistance (float zNear, float zFar)
	{
		glReadPixels (0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth .data ());

		float distance = zNear + (zFar - zNear) * depth [0];

		for (const auto & d : depth)
		{
			distance = std::min (distance, zNear + (zFar - zNear) * d);
		}

		return distance;
	}

	void
	bind ()
	{
		// Bind frame buffer.
		glBindFramebuffer (GL_FRAMEBUFFER, id);

		glGetIntegerv (GL_VIEWPORT, viewport);
		glViewport (0, 0, width, height);
		glScissor  (0, 0, width, height);
		glEnable (GL_SCISSOR_TEST);
		glClear (GL_DEPTH_BUFFER_BIT);
	}

	void
	unbind ()
	{
		glDisable (GL_SCISSOR_TEST);
		glViewport (viewport [0], viewport [1], viewport [2], viewport [3]);

		// Bind frame buffer.
		glBindFramebuffer (GL_FRAMEBUFFER, 0);
	}

	~DepthBuffer ()
	{
		if (depthBuffer)
			glDeleteRenderbuffers (1, &depthBuffer);

		if (id)
			glDeleteFramebuffers (1, &id);
	}

	size_t width;
	size_t height;

	GLuint                id;
	GLuint                depthBuffer;
	std::vector <GLfloat> depth;
	GLint                 viewport [4];

};

X3DRenderer::X3DRenderer () :
	            X3DNode (),  
	             shapes (),  
	  transparentShapes (),  
	        depthBuffer (),  
	              speed (),  
	     numOpaqueNodes (0), 
	numTransparentNodes (0)  
{ }

void
X3DRenderer::initialize ()
{
	depthBuffer .reset (new DepthBuffer (DEPTH_BUFFER_WIDTH, DEPTH_BUFFER_HEIGHT));
}

void
X3DRenderer::addShape (X3DShapeNode* shape)
{
	X3DFogObject*               fog         = getCurrentLayer () -> getFog ();
	const LightContainerArray & localLights = getCurrentLayer () -> getLocalLights ();

	if (shape -> isTransparent ())
	{
		if (numTransparentNodes < transparentShapes .size ())
			transparentShapes [numTransparentNodes] -> assign (shape, fog, localLights);
		else
			transparentShapes .emplace_back (new ShapeContainer (shape, fog, localLights));

		++ numTransparentNodes;
	}
	else
	{
		if (numOpaqueNodes < shapes .size ())
			shapes [numOpaqueNodes] -> assign (shape, fog, localLights);
		else
			shapes .emplace_back (new ShapeContainer (shape, fog, localLights));

		++ numOpaqueNodes;
	}
}

void
X3DRenderer::render (TraverseType type)
{
	numOpaqueNodes      = 0;
	numTransparentNodes = 0;

	getBrowser () -> getRenderers () .emplace (this);

	collect (type);

	switch (type)
	{
		case TraverseType::COLLISION:
		{
			collide ();
			gravite ();
			break;
		}
		case TraverseType::COLLECT:
		{
			draw ();
			break;
		}
		default:
			break;
	}

	getBrowser () -> getRenderers () .pop ();
}

void
X3DRenderer::draw ()
{
	glPushMatrix ();

	// Enable global lights

	const LightContainerArray & globalLights = getCurrentLayer () -> getGlobalLights ();

	for (const auto & light : globalLights)
		light -> enable ();

	if (1)
	{
		// Sorted blend

		size_t numNodesDrawn = 0;

		// Render opaque objects first

		glEnable (GL_DEPTH_TEST);
		glDepthMask (GL_TRUE);
		glDisable (GL_BLEND);

		for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
			numNodesDrawn += shape -> draw ();

		// Render transparent objects

		glDepthMask (GL_FALSE);
		glEnable (GL_BLEND);

		std::stable_sort (transparentShapes .begin (), transparentShapes .begin () + numTransparentNodes, ShapeContainerComp ());

		for (const auto & shape : basic::adapter (transparentShapes .cbegin (), transparentShapes .cbegin () + numTransparentNodes))
			numNodesDrawn += shape -> draw ();

		glDepthMask (GL_TRUE);
		glDisable (GL_BLEND);

		//__LOG__ << numOpaqueNodes + numTransparentNodes << " : " << numNodesDrawn << std::endl;
	}
	else
	{
		//	http://wiki.delphigl.com/index.php/Blenden

		glEnable (GL_DEPTH_TEST);

		// Render opaque objects first

		glDepthMask (GL_TRUE);
		glDisable (GL_BLEND);

		for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
			shape -> draw ();

		// Render transparent objects

		std::stable_sort (transparentShapes .begin (), transparentShapes .begin () + numTransparentNodes, ShapeContainerComp ());

		glEnable (GL_BLEND);

		for (const auto & shape : basic::adapter (transparentShapes .cbegin (), transparentShapes .cbegin () + numTransparentNodes))
		{
			glDepthFunc (GL_GREATER);
			glDepthMask (GL_FALSE);
			glBlendFunc (GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

			shape -> draw ();

			glDepthFunc (GL_LEQUAL);
			glDepthMask (GL_TRUE);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			shape -> draw ();
		}

		//		// render opaque objects
		//
		//		glDepthFunc (GL_GREATER);
		//		glDepthMask (GL_FALSE);
		//		glBlendFunc (GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
		//		glBlendFuncSeparate (GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//
		//		for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
		//		{
		//			numNodesDrawn += shape -> redraw ();
		//		}
		//
		//		glDisable (GL_BLEND);
		//		glDepthFunc (GL_LEQUAL);
		//		glDepthMask (GL_TRUE);
		//
		//		for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
		//		{
		//			shape -> redraw ();
		//		}
	}

	// Disable global lights

	for (const auto & light : basic::adapter (globalLights .crbegin (), globalLights .crend ()))
		light -> disable ();

	glPopMatrix ();
}

void
X3DRenderer::collide ()
{
	// Collision
	// Bind buffer

	depthBuffer -> bind ();

	// Reshape viewpoint

	auto navigationInfo = getCurrentNavigationInfo ();

	float zNear           = navigationInfo -> getZNear ();
	float zFar            = navigationInfo -> getZFar ();
	float collisionRadius = navigationInfo -> getCollisionRadius ();

	glPushMatrix ();

	Matrix4f cameraSpaceMatrix = getCurrentViewpoint () -> getModelViewMatrix ();

	cameraSpaceMatrix .translate (getCurrentViewpoint () -> getUserPosition ());
	cameraSpaceMatrix .rotate (Rotation4f (Vector3f (0, 0, 1), -getBrowser () -> velocity));
	multMatrix (inverse (cameraSpaceMatrix));

	{
		// Render opaque objects first

		glEnable (GL_DEPTH_TEST);
		glDepthMask (GL_TRUE);
		glDisable (GL_BLEND);

		for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
			shape -> draw ();

		// Render transparent objects

		for (const auto & shape : basic::adapter (transparentShapes .cbegin (), transparentShapes .cbegin () + numTransparentNodes))
			shape -> draw ();
	}

	glPopMatrix ();

	// Gravite or step up

	Vector3f translation = getBrowser () -> velocity / (float) getBrowser () -> getCurrentFrameRate ();
	float    length      = math::abs (translation);

	if (length)
	{
		float distance = depthBuffer -> getDistance (zNear, zFar);

		//__LOG__ << distance << std::endl;

		if (zFar - distance > 0) // Are there polygons under the viewer
		{
			distance -= collisionRadius;

			if (distance > 0)
			{
				// Move

				if (length > distance)
				{
					// The ground is reached.
					length = distance;
				}

				getCurrentViewpoint () -> positionOffset += normalize (translation) * length;
			}
			else
			{
				// Collision
			}
		}
		else
			getCurrentViewpoint () -> positionOffset += translation;

		getBrowser () -> velocity = Vector3f ();
	}

	// Unbind buffer

	depthBuffer -> unbind ();

	{
		Matrix4f cameraSpaceMatrix = getCurrentViewpoint () -> getModelViewMatrix ();

		cameraSpaceMatrix .translate (getCurrentViewpoint () -> getUserPosition ());
		cameraSpaceMatrix .rotate (Rotation4f (Vector3f (0, 0, 1), -getBrowser () -> velocity));
		multMatrix (inverse (cameraSpaceMatrix));

		bool     intersected = false;
		Sphere3f collisionSphere (collisionRadius, Vector3f ());

		std::deque <Vector3f> collisionNormal;

		for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
		{
			if ((intersected = shape -> intersect (collisionSphere, collisionNormal)))
				break;
		}

		if (not intersected)
		{
			for (const auto & shape : basic::adapter (transparentShapes .cbegin (), transparentShapes .cbegin () + numTransparentNodes))
			{
				if ((intersected = shape -> intersect (collisionSphere, collisionNormal)))
					break;
			}
		}

		if (intersected)
		{
			if (getBrowser () -> getViewerType () == ViewerType::WALK or getBrowser () -> getViewerType () == ViewerType::FLY)
			{
				if (collisionNormal .size () == 1)
				{
					collisionNormal [0] = cameraSpaceMatrix .multDirMatrix (collisionNormal [0]);

					auto translation = inverse (getCurrentViewpoint () -> getModelViewMatrix ()) .multDirMatrix (collisionNormal [0]);

					getCurrentViewpoint () -> positionOffset += translation;
				}
			}

			__LOG__ << SFTime (chrono::now ()) << " : " << intersected << " : " << collisionNormal .size () << std::endl;
		}
	}
}

void
X3DRenderer::gravite ()
{
	// Terrain following and gravitation

	if (getBrowser () -> getViewerType () not_eq ViewerType::WALK)
		return;

	// Bind buffer

	depthBuffer -> bind ();

	// Reshape viewpoint

	auto navigationInfo = getCurrentNavigationInfo ();

	float zNear      = navigationInfo -> getZNear ();
	float zFar       = navigationInfo -> getZFar ();
	float height     = navigationInfo -> getAvatarHeight ();
	float stepHeight = navigationInfo -> getStepHeight ();

	glPushMatrix ();

	multMatrix (inverse (getCurrentViewpoint () -> getDownViewMatrix ()));

	{
		// Render opaque objects first

		glEnable (GL_DEPTH_TEST);
		glDepthMask (GL_TRUE);
		glDisable (GL_BLEND);

		for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
			shape -> draw ();

		// Render transparent objects

		for (const auto & shape : basic::adapter (transparentShapes .cbegin (), transparentShapes .cbegin () + numTransparentNodes))
			shape -> draw ();
	}

	glPopMatrix ();

	// Gravite or step up

	float distance = depthBuffer -> getDistance (zNear, zFar);

	if (zFar - distance > 0) // Are there polygons under the viewer
	{
		distance -= height;

		if (distance > 0)
		{
			// Gravite

			float currentFrameRate = getBrowser () -> getCurrentFrameRate ();

			speed += P_GN / currentFrameRate;

			float translation = speed / currentFrameRate;

			if (translation < -distance)
			{
				// The ground is reached.
				translation = -distance;
				speed       = 0;
			}

			getCurrentViewpoint () -> positionOffset += Vector3f (0, translation, 0);
		}
		else
		{
			speed = 0;

			if (-distance > 0.01 and - distance < stepHeight)
			{
				// Step up
				getCurrentViewpoint () -> positionOffset += Vector3f (0, -distance, 0);
			}
		}
	}

	// Unbind buffer

	depthBuffer -> unbind ();
}

void
X3DRenderer::multMatrix (const Matrix4f & matrix)
{
	//glMultMatrixf (matrix .data ());

	for (const auto & shape : basic::adapter (shapes .cbegin (), shapes .cbegin () + numOpaqueNodes))
		shape -> multMatrix (matrix);

	for (const auto & shape : basic::adapter (transparentShapes .cbegin (), transparentShapes .cbegin () + numTransparentNodes))
		shape -> multMatrix (matrix);
}

void
X3DRenderer::clear ()
{
	for (const auto & shape : shapes)
		delete shape;

	shapes .clear ();

	for (const auto & shape : transparentShapes)
		delete shape;

	transparentShapes .clear ();
}

void
X3DRenderer::dispose ()
{
	depthBuffer .reset ();
}

X3DRenderer::~X3DRenderer ()
{
	clear ();
}

} // X3D
} // titania
