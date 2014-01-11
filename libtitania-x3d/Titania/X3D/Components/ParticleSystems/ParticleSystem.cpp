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

#include "ParticleSystem.h"

#include "../../Bits/Cast.h"
#include "../../Bits/Random.h"
#include "../../Bits/config.h"
#include "../../Browser/X3DBrowser.h"
#include "../../Execution/X3DExecutionContext.h"
#include "../Rendering/X3DGeometryNode.h"
#include "../Shaders/ShaderPart.h"
#include "../Shape/X3DAppearanceNode.h"
#include "X3DParticlePhysicsModelNode.h"

#include <cstddef>

#include "../../Debug.h"

namespace titania {
namespace X3D {

const std::string ParticleSystem::componentName  = "ParticleSystems";
const std::string ParticleSystem::typeName       = "ParticleSystem";
const std::string ParticleSystem::containerField = "children";

static constexpr Vector3f yAxis (0, 1, 0);
static constexpr Vector3f zAxis (0, 0, 1);

// gcc switch -malign-double

struct ParticleSystem::Particle
{
	///  @name Construction

	Particle () :
		       seed (randomi ()),
		   lifetime (0),
		   position (),
		   velocity (),
		      color (),
		elapsedTime (std::numeric_limits <float>::max ())
	{ }

	///  @name Members

	int32_t seed;
	float lifetime;
	Vector3f position;
	Vector3f velocity;
	Color4f color;
	float elapsedTime;

};

struct ParticleSystem::Vertex
{
	Vertex () :
		position (),
		   color (),
		texCoord ()
	{ }

	Vector3f position;
	Color4f color;
	Vector4f texCoord;

};

ParticleSystem::Fields::Fields () :
	          enabled (new SFBool (true)),
	     geometryType (new SFString ("QUAD")),
	  createParticles (new SFBool (true)),
	     maxParticles (new SFInt32 (200)),
	 particleLifetime (new SFFloat (5)),
	lifetimeVariation (new SFFloat (0.25)),
	     particleSize (new SFVec2f (0.02, 0.02)),
	         colorKey (new MFFloat ()),
	      texCoordKey (new MFFloat ()),
	         isActive (new SFBool ()),
	          emitter (new SFNode ()),
	          physics (new MFNode ()),
	        colorRamp (new SFNode ()),
	     texCoordRamp (new SFNode ())
{ }

ParticleSystem::ParticleSystem (X3DExecutionContext* const executionContext) :
	        X3DBaseNode (executionContext -> getBrowser (), executionContext),
	       X3DShapeNode (),
	             fields (),
	     geometryTypeId (GeometryType::QUAD),
	     glGeometryType (GL_POINTS),
	        numVertices (0),
	       numParticles (0),
	       creationTime (0),
	         readBuffer (0),
	        writeBuffer (1),
	transformFeedbackId ({ 0, 0 }),
	   particleBufferId ({ 0, 0 }),
	      particleMapId (0),
	   vertexFeedbackId (0),
	     vertexBufferId (0),
	   geometryBufferId (0),
	    transformShader (),
	     geometryShader (),
	        emitterNode (nullptr),
	      colorRampNode (),
	          haveColor (false),
	          stepsLeft (0),
	               pass (0),
	              stage (0)
{
	addField (inputOutput,    "metadata",          metadata ());
	addField (inputOutput,    "enabled",           enabled ());

	addField (initializeOnly, "geometryType",      geometryType ());
	addField (inputOutput,    "createParticles",   createParticles ());
	addField (inputOutput,    "maxParticles",      maxParticles ());

	addField (inputOutput,    "particleLifetime",  particleLifetime ());
	addField (inputOutput,    "lifetimeVariation", lifetimeVariation ());
	addField (inputOutput,    "particleSize",      particleSize ());

	addField (initializeOnly, "colorKey",          colorKey ());
	addField (initializeOnly, "texCoordKey",       texCoordKey ());
	addField (outputOnly,     "isActive",          isActive ());

	addField (initializeOnly, "bboxSize",          bboxSize ());
	addField (initializeOnly, "bboxCenter",        bboxCenter ());
	addField (initializeOnly, "emitter",           emitter ());
	addField (initializeOnly, "physics",           physics ());
	addField (initializeOnly, "colorRamp",         colorRamp ());
	addField (initializeOnly, "texCoordRamp",      texCoordRamp ());
	addField (inputOutput,    "appearance",        appearance ());
	addField (inputOutput,    "geometry",          geometry ());

	addChildren (transformShader, geometryShader, colorRampNode);
}

X3DBaseNode*
ParticleSystem::create (X3DExecutionContext* const executionContext) const
{
	return new ParticleSystem (executionContext);
}

void
ParticleSystem::initialize ()
{
	X3DShapeNode::initialize ();

	if (glXGetCurrentContext ())
	{
		// Generate transform buffers

		glGenTransformFeedbacks (2, transformFeedbackId .data ());
		glGenBuffers (2, particleBufferId .data ());

		for (size_t i = 0; i < 2; ++ i)
		{
			glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, transformFeedbackId [i]);
			glBindBufferBase (GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBufferId [i]);
		}

		glGenTextures (1, &particleMapId);

		set_particle_map ();

		// Generate point buffer

		glGenTransformFeedbacks (1, &vertexFeedbackId);
		glGenBuffers (1, &vertexBufferId);

		glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, vertexFeedbackId);
		glBindBufferBase (GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBufferId);

		glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, 0);

		// Generate geometry buffer

		glGenBuffers (1, &geometryBufferId);

		// Setup

		enabled ()           .addInterest (this, &ParticleSystem::set_enabled);
		geometryType ()      .addInterest (this, &ParticleSystem::set_geometryType);
		maxParticles ()      .addInterest (this, &ParticleSystem::set_enabled);
		particleLifetime ()  .addInterest (this, &ParticleSystem::set_particle_buffers);
		lifetimeVariation () .addInterest (this, &ParticleSystem::set_particle_buffers);
		particleSize ()      .addInterest (this, &ParticleSystem::set_geometryType);
		colorKey ()          .addInterest (this, &ParticleSystem::set_colorKey);
		texCoordKey ()       .addInterest (this, &ParticleSystem::set_texCoordKey);
		emitter ()           .addInterest (this, &ParticleSystem::set_emitter);
		colorRamp ()         .addInterest (this, &ParticleSystem::set_colorRamp);
		texCoordRamp ()      .addInterest (this, &ParticleSystem::set_texCoordRamp);
		geometry ()          .addInterest (this, &ParticleSystem::set_geometry);

		set_geometry_shader ();
		set_enabled ();
		set_geometryType ();
		set_emitter ();
		set_colorKey ();
		set_colorRamp ();
		set_texCoordKey ();
		set_texCoordRamp ();
		set_geometry ();
	}
}

bool
ParticleSystem::isTransparent () const
{
	if (getAppearance () -> isTransparent ())
		return true;

	if (haveColor and colorRampNode -> isTransparent ())
		return true;

	return false;
}

bool
ParticleSystem::isLineGeometry () const
{
	switch (geometryTypeId)
	{
		case GeometryType::POINT:
		case GeometryType::LINE:
			return true;
		case GeometryType::TRIANGLE:
		case GeometryType::QUAD:
		case GeometryType::SPRITE:
			return false;
		case GeometryType::GEOMETRY:
		{
			if (getGeometry ())
				return getGeometry () -> isLineGeometry ();

			return false;
		}
	}

	return false;
}

Box3f
ParticleSystem::getBBox ()
{
	if (bboxSize () == Vector3f (-1, -1, -1))
	{
		return Box3f ();
	}

	return Box3f (bboxSize (), bboxCenter ());
}

void
ParticleSystem::set_enabled ()
{
	if (enabled () and maxParticles ())
	{
		getBrowser () -> prepareEvents () .addInterest (this, &ParticleSystem::prepareEvents);
		getBrowser () -> sensors ()       .addInterest (this, &ParticleSystem::update);

		isActive () = true;

		set_particle_buffers ();
	}
	else
	{
		getBrowser () -> prepareEvents () .removeInterest (this, &ParticleSystem::prepareEvents);
		getBrowser () -> sensors ()       .removeInterest (this, &ParticleSystem::update);

		isActive () = false;
	}
}

void
ParticleSystem::set_geometryType ()
{
	static const std::map <std::string, GeometryType> geometryTypes = {
		std::make_pair ("POINT",    GeometryType::POINT),
		std::make_pair ("LINE",     GeometryType::LINE),
		std::make_pair ("TRIANGLE", GeometryType::TRIANGLE),
		std::make_pair ("QUAD",     GeometryType::QUAD),
		std::make_pair ("GEOMETRY", GeometryType::GEOMETRY),
		std::make_pair ("SPRITE",   GeometryType::SPRITE)
	};

	auto iter = geometryTypes .find (geometryType ());

	if (iter not_eq geometryTypes .end ())
		geometryTypeId = iter -> second;

	else
		geometryTypeId = GeometryType::QUAD;

	//

	MFVec4f                texCoord;
	std::vector <Vector3f> vertices;

	Vector2f particleSize1_2 = particleSize () / 2.0f;

	switch (geometryTypeId)
	{
		case GeometryType::POINT:
		{
			glGeometryType = GL_POINTS;
			numVertices    = 0;
			break;
		}
		case GeometryType::LINE:
		{
			glGeometryType = GL_LINES;
			numVertices    = 2;

			texCoord .emplace_back (0, 0, 0, 1);
			texCoord .emplace_back (0, 1, 0, 1);

			vertices .reserve (numVertices);
			vertices .emplace_back (0, 0, -particleSize1_2 .y ());
			vertices .emplace_back (0, 0,  particleSize1_2 .y ());
			break;
		}
		case GeometryType::TRIANGLE:
		{
			glGeometryType = GL_TRIANGLES;
			numVertices    = 6;

			texCoord .emplace_back (0, 0, 0, 1);
			texCoord .emplace_back (1, 0, 0, 1);
			texCoord .emplace_back (1, 1, 0, 1);

			texCoord .emplace_back (0, 0, 0, 1);
			texCoord .emplace_back (1, 1, 0, 1);
			texCoord .emplace_back (0, 1, 0, 1);

			vertices .reserve (numVertices);
			vertices .emplace_back (-particleSize1_2 .x (), -particleSize1_2 .y (), 0);
			vertices .emplace_back (+particleSize1_2 .x (), -particleSize1_2 .y (), 0);
			vertices .emplace_back (+particleSize1_2 .x (),  particleSize1_2 .y (), 0);

			vertices .emplace_back (-particleSize1_2 .x (), -particleSize1_2 .y (), 0);
			vertices .emplace_back (+particleSize1_2 .x (),  particleSize1_2 .y (), 0);
			vertices .emplace_back (-particleSize1_2 .x (),  particleSize1_2 .y (), 0);
			break;
		}
		case GeometryType::QUAD:
		case GeometryType::SPRITE:
		{
			glGeometryType = GL_QUADS;
			numVertices    = 4;

			texCoord .emplace_back (0, 0, 0, 1);
			texCoord .emplace_back (1, 0, 0, 1);
			texCoord .emplace_back (1, 1, 0, 1);
			texCoord .emplace_back (0, 1, 0, 1);

			vertices .reserve (numVertices);
			vertices .emplace_back (-particleSize1_2 .x (), -particleSize1_2 .y (), 0);
			vertices .emplace_back (+particleSize1_2 .x (), -particleSize1_2 .y (), 0);
			vertices .emplace_back (+particleSize1_2 .x (),  particleSize1_2 .y (), 0);
			vertices .emplace_back (-particleSize1_2 .x (),  particleSize1_2 .y (), 0);
			break;
		}
		case GeometryType::GEOMETRY:
		{
			glGeometryType = GL_TRIANGLES;
			numVertices    = 1;

			vertices .resize (numVertices);
			break;
		}
	}

	if (numVertices)
	{
		glBindBuffer (GL_ARRAY_BUFFER, geometryBufferId);
		glBufferData (GL_ARRAY_BUFFER, sizeof (Vector3f) * numVertices, vertices .data (), GL_STATIC_DRAW);

		glBindBuffer (GL_ARRAY_BUFFER, 0);

		// Vertex buffer

		set_vertex_buffer ();
	}

	geometryShader -> setField <SFInt32> ("geometryType", int32_t (geometryTypeId));
	geometryShader -> setField <MFVec4f> ("texCoord", texCoord);
}

void
ParticleSystem::set_colorKey ()
{
	set_color ();
}

void
ParticleSystem::set_texCoordKey ()
{ }

void
ParticleSystem::set_emitter ()
{
	// Emitter node

	emitterNode = x3d_cast <X3DParticleEmitterNode*> (emitter ());

	if (not emitterNode)
		emitterNode = getBrowser () -> getBrowserOptions () -> emitter ();

	// Shader

	set_transform_shader ();
}

void
ParticleSystem::set_colorRamp ()
{
	if (colorRampNode)
		colorRampNode -> removeInterest (this, &ParticleSystem::set_color);

	colorRampNode = colorRamp ();

	if (colorRampNode)
		colorRampNode -> addInterest (this, &ParticleSystem::set_color);

	set_color ();
}

void
ParticleSystem::set_color ()
{
	if (colorRampNode and not colorKey () .empty ())
	{
		MFVec4f color;

		colorRampNode -> getHSVA (color);

		color .resize (colorKey () .size (), SFVec4f (0, 1, 1, 1));

		transformShader -> setField <MFFloat> ("colorKey",  colorKey ());
		transformShader -> setField <MFVec4f> ("colorRamp", color);
		transformShader -> setField <SFInt32> ("numColors", int32_t (colorKey () .size ()));

		haveColor = true;
	}
	else
	{
		transformShader -> setField <SFInt32> ("numColors", 0);
		haveColor = false;
	}
}

void
ParticleSystem::set_texCoordRamp ()
{ }

void
ParticleSystem::set_geometry ()
{ }

void
ParticleSystem::set_particle_buffers ()
{
	// Initialize array buffers

	size_t maxParticles = std::max (0, this -> maxParticles () .getValue ());

	// Particle buffers

	Particle particleArray [maxParticles];

	for (size_t i = 0; i < 2; ++ i)
	{
		glBindBuffer (GL_ARRAY_BUFFER, particleBufferId [i]);
		glBufferData (GL_ARRAY_BUFFER, sizeof (particleArray), particleArray, GL_DYNAMIC_DRAW);
	}

	glBindBuffer (GL_ARRAY_BUFFER, 0);

	// Vertex buffer

	set_vertex_buffer ();

	// Reset state

	numParticles = 0;
	creationTime = chrono::now ();
}

void
ParticleSystem::set_vertex_buffer ()
{
	// Initialize array buffers

	size_t maxParticles = std::max (0, this -> maxParticles () .getValue ());

	// Vertex buffer

	Vertex vertexArray [maxParticles * numVertices];

	glBindBuffer (GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData (GL_ARRAY_BUFFER, sizeof (vertexArray), vertexArray, GL_DYNAMIC_DRAW);

	glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void
ParticleSystem::set_transform_shader ()
{
	// Shader

	X3DSFNode <ShaderPart> vertexPart = new ShaderPart (getExecutionContext ());
	vertexPart -> url () = emitterNode -> getShaderUrl ();
	vertexPart -> setup ();

	transformShader = new ComposedShader (getExecutionContext ());
	transformShader -> addUserDefinedField (inputOutput, "deltaTime",         new SFFloat ());
	transformShader -> addUserDefinedField (inputOutput, "particleLifetime",  new SFFloat ());
	transformShader -> addUserDefinedField (inputOutput, "lifetimeVariation", new SFFloat ());

	transformShader -> addUserDefinedField (inputOutput, "position",          new SFVec3f ());
	transformShader -> addUserDefinedField (inputOutput, "direction",         new SFVec3f ());
	transformShader -> addUserDefinedField (inputOutput, "speed",             new SFFloat ());
	transformShader -> addUserDefinedField (inputOutput, "variation",         new SFFloat ());
	transformShader -> addUserDefinedField (inputOutput, "velocity",          new MFVec3f ());
	transformShader -> addUserDefinedField (inputOutput, "turbulence",        new MFFloat ());
	transformShader -> addUserDefinedField (inputOutput, "numForces",         new SFInt32 ());

	transformShader -> addUserDefinedField (inputOutput, "colorKey",          new MFFloat ());
	transformShader -> addUserDefinedField (inputOutput, "colorRamp",         new MFVec4f ());
	transformShader -> addUserDefinedField (inputOutput, "numColors",         new SFInt32 ());

	transformShader -> addUserDefinedField (inputOutput, "modelViewMatrix",   new SFMatrix4f ());
	transformShader -> addUserDefinedField (inputOutput, "stride",            new SFInt32 (sizeof (Particle) / sizeof (float)));
	transformShader -> addUserDefinedField (inputOutput, "seedOffset",        new SFInt32 (offsetof (Particle, seed) / sizeof (float)));
	transformShader -> addUserDefinedField (inputOutput, "lifetimeOffset",    new SFInt32 (offsetof (Particle, lifetime) / sizeof (float)));
	transformShader -> addUserDefinedField (inputOutput, "positionOffset",    new SFInt32 (offsetof (Particle, position) / sizeof (float)));
	transformShader -> addUserDefinedField (inputOutput, "velocityOffset",    new SFInt32 (offsetof (Particle, velocity) / sizeof (float)));
	transformShader -> addUserDefinedField (inputOutput, "colorOffset",       new SFInt32 (offsetof (Particle, color) / sizeof (float)));
	transformShader -> addUserDefinedField (inputOutput, "elapsedTimeOffset", new SFInt32 (offsetof (Particle, elapsedTime) / sizeof (float)));

	transformShader -> addUserDefinedField (inputOutput, "Param",             new SFVec4f ());
	transformShader -> addUserDefinedField (inputOutput, "size",              new SFInt32 ());

	transformShader -> language () = "GLSL";
	transformShader -> parts () .emplace_back (vertexPart);
	transformShader -> setTransformFeedbackVaryings ({
	                                                    "To.seed", "To.lifetime", "To.position", "To.velocity", "To.color", "To.elapsedTime"
																	 });
	transformShader -> setup ();

	transformShader -> setTextureBuffer ("particleMap", particleMapId);
}

void
ParticleSystem::set_particle_map ()
{
	glBindTexture (GL_TEXTURE_BUFFER, particleMapId);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R32F, particleBufferId [readBuffer]);
	glBindTexture (GL_TEXTURE_BUFFER, 0);
}

void
ParticleSystem::set_geometry_shader ()
{
	X3DSFNode <ShaderPart> vertexPart = new ShaderPart (getExecutionContext ());
	vertexPart -> url () = { get_shader ("ParticleSystems/Primitive.vs") .str () };
	vertexPart -> setup ();

	geometryShader = new ComposedShader (getExecutionContext ());
	geometryShader -> addUserDefinedField (inputOutput, "geometryType", new SFInt32 ());
	geometryShader -> addUserDefinedField (inputOutput, "rotation",     new SFMatrix3f ());
	geometryShader -> addUserDefinedField (inputOutput, "texCoord",     new MFVec4f ());

	geometryShader -> language () = "GLSL";
	geometryShader -> parts () .emplace_back (vertexPart);
	geometryShader -> setTransformFeedbackVaryings ({
	                                                   "To.position", "To.color", "To.texCoord"
																	});
	geometryShader -> setup ();
}

bool
ParticleSystem::intersect (const Sphere3f & sphere, const Matrix4f & matrix, const CollectableObjectArray & localObjects)
{
	return false;
}

void
ParticleSystem::traverse (const TraverseType type)
{
	switch (type)
	{
		case TraverseType::PICKING:
		{
			break;
		}
		case TraverseType::NAVIGATION:
		case TraverseType::COLLISION:
		{
			break;
		}
		case TraverseType::COLLECT:
		{
			getBrowser () -> getRenderers () .top () -> addShape (this);
			break;
		}
		default:
			break;
	}
}

void
ParticleSystem::prepareEvents ()
{
	// Create new particles if possible.

	time_type now             = chrono::now ();
	int32_t   createParticles = (now - creationTime) * maxParticles () / particleLifetime ();

	if (createParticles)
		creationTime = now;

	numParticles = std::min (std::max (0, maxParticles () .getValue ()), createParticles + numParticles);

	// Update shader

	float deltaTime = 1 / getBrowser () -> getCurrentFrameRate ();

	transformShader -> setField <SFFloat> ("deltaTime",         deltaTime);
	transformShader -> setField <SFFloat> ("particleLifetime",  particleLifetime (),  true);
	transformShader -> setField <SFFloat> ("lifetimeVariation", lifetimeVariation (), true);

	emitterNode -> setShaderFields (transformShader);

	MFVec3f velocity;
	MFFloat turbulence;

	for (const auto & node : physics ())
	{
		auto physics = x3d_cast <X3DParticlePhysicsModelNode*> (node);

		if (physics)
			physics -> getForce (emitterNode, velocity, turbulence);
	}

	if (emitterNode -> mass ())
	{
		for (auto & v : velocity)
			v *= deltaTime / emitterNode -> mass ();

		transformShader -> setField <MFVec3f> ("velocity",   velocity,                    true);
		transformShader -> setField <MFFloat> ("turbulence", turbulence,                  true);
		transformShader -> setField <SFInt32> ("numForces",  int32_t (velocity .size ()), true);
	}
	else
		transformShader -> setField <SFInt32> ("numForces", 0, true);

	// Sort step

	oddEvenMergeSort (numParticles);
}

void
ParticleSystem::oddEvenMergeSort (int32_t size)
{
	if (size < maxParticles ())
		return;

	if (stepsLeft <= 0)
	{
		transformShader -> setField <SFInt32> ("size", size, true);

		// Reset

		int fieldSize = next_power_of_two (int (std::ceil (std::sqrt (size))));
		
		if (fieldSize)
		{
			int logFieldSize = std::log2 (fieldSize);

			stepsLeft = logFieldSize * (2 * logFieldSize + 1);
		}

		stage = pass = -1;
	}

	// Sort pass

	-- pass;

	if (pass < 0)
	{
		// next stage
		++ stage;
		pass = stage;
	}

	// Sort step

	int pstage = 1 << stage;
	int ppass  = 1 << pass;

	transformShader -> setField <SFVec4f> ("Param", Vector4f (pstage + pstage, ppass % pstage, (pstage + pstage) - (ppass % pstage) - 1, ppass));

	//

	-- stepsLeft;
}

void
ParticleSystem::update ()
{
	// Transform particles.

	transformShader -> enable ();

	glEnable (GL_RASTERIZER_DISCARD);
	glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, transformFeedbackId [writeBuffer]);
	glBeginTransformFeedback (GL_POINTS);

	glDrawArrays (GL_POINTS, 0, numParticles);

	glEndTransformFeedback ();
	glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, 0);
	glDisable (GL_RASTERIZER_DISCARD);

	transformShader -> disable ();

	std::swap (readBuffer, writeBuffer);
	set_particle_map ();

	// Geometry shader

	switch (geometryTypeId)
	{
		case GeometryType::POINT:
			break;
		case GeometryType::LINE:
		case GeometryType::TRIANGLE:
		case GeometryType::QUAD:
		case GeometryType::SPRITE:
		{
			geometryShader -> enable ();

			glEnableVertexAttribArray (0);
			glEnableVertexAttribArray (1);
			glEnableVertexAttribArray (2);
			glEnableVertexAttribArray (3);

			glBindBuffer (GL_ARRAY_BUFFER, geometryBufferId);
			glVertexAttribPointer (0, 3, GL_FLOAT, false, 0, (void*) 0);

			glBindBuffer (GL_ARRAY_BUFFER, particleBufferId [readBuffer]);
			glVertexAttribPointer (1, 3, GL_FLOAT, false, sizeof (Particle), (void*) offsetof (Particle, position));
			glVertexAttribPointer (2, 3, GL_FLOAT, false, sizeof (Particle), (void*) offsetof (Particle, velocity));
			glVertexAttribPointer (3, 4, GL_FLOAT, false, sizeof (Particle), (void*) offsetof (Particle, color));

			glVertexAttribDivisor (1, 1);
			glVertexAttribDivisor (2, 1);
			glVertexAttribDivisor (3, 1);

			glEnable (GL_RASTERIZER_DISCARD);
			glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, vertexFeedbackId);
			glBeginTransformFeedback (GL_POINTS);

			glDrawArraysInstanced (GL_POINTS, 0, numVertices, numParticles);

			glEndTransformFeedback ();
			glBindTransformFeedback (GL_TRANSFORM_FEEDBACK, 0);
			glDisable (GL_RASTERIZER_DISCARD);

			glVertexAttribDivisor (1, 0);
			glVertexAttribDivisor (2, 0);
			glVertexAttribDivisor (3, 0);

			glDisableVertexAttribArray (0);
			glDisableVertexAttribArray (1);
			glDisableVertexAttribArray (2);
			glDisableVertexAttribArray (3);
			glBindBuffer (GL_ARRAY_BUFFER, 0);

			geometryShader -> disable ();
			break;
		}
		case GeometryType::GEOMETRY:
		{
			break;
		}
	}

	GL_ERROR;
}

void
ParticleSystem::drawCollision ()
{ }

void
ParticleSystem::drawGeometry ()
{
	bool   solid     = false;
	GLenum frontFace = GL_CCW;

	if (solid)
		glEnable (GL_CULL_FACE);

	else
		glDisable (GL_CULL_FACE);

	glFrontFace (ModelViewMatrix4f () .determinant3 () > 0 ? frontFace : (frontFace == GL_CCW ? GL_CW : GL_CCW));

	glNormal3f (0, 0, 1);

	switch (geometryTypeId)
	{
		case GeometryType::POINT:
		{
			glBindBuffer (GL_ARRAY_BUFFER, particleBufferId [readBuffer]);

			if (haveColor)
			{
				if (glIsEnabled (GL_LIGHTING))
					glEnable (GL_COLOR_MATERIAL);

				glEnableClientState (GL_COLOR_ARRAY);
				glColorPointer (4, GL_FLOAT, sizeof (Particle), (void*) offsetof (Particle, color));
			}

			glEnableClientState (GL_VERTEX_ARRAY);
			glVertexPointer (3, GL_FLOAT, sizeof (Particle), (void*) offsetof (Particle, position));

			glDrawArrays (GL_POINTS, 0, numParticles);

			glDisableClientState (GL_COLOR_ARRAY);
			glDisableClientState (GL_VERTEX_ARRAY);
			glBindBuffer (GL_ARRAY_BUFFER, 0);
			break;
		}
		case GeometryType::SPRITE:
		{
			try
			{
				Matrix3f rotation = getScreenAlignedRotation ();

				glNormal3fv (rotation [2] .data ());

				geometryShader -> setField <SFMatrix3f> ("rotation", rotation);
			}
			catch (const std::domain_error &)
			{ }

			// Proceed with next case.
		}
		case GeometryType::LINE:
		case GeometryType::TRIANGLE:
		case GeometryType::QUAD:
		{
			glBindBuffer (GL_ARRAY_BUFFER, vertexBufferId);

			if (haveColor)
			{
				if (glIsEnabled (GL_LIGHTING))
					glEnable (GL_COLOR_MATERIAL);

				glEnableClientState (GL_COLOR_ARRAY);
				glColorPointer (4, GL_FLOAT, sizeof (Vertex), (void*) offsetof (Vertex, color));
			}

			if (getBrowser () -> isEnabledTexture ())
				enableTexCoord ();

			glEnableClientState (GL_VERTEX_ARRAY);
			glVertexPointer (3, GL_FLOAT, sizeof (Vertex), (void*) offsetof (Vertex, position));

			glDrawArrays (glGeometryType, 0, numParticles * numVertices);

			if (getBrowser () -> isEnabledTexture ())
				disableTexCoord ();

			glDisableClientState (GL_COLOR_ARRAY);
			glDisableClientState (GL_VERTEX_ARRAY);
			glBindBuffer (GL_ARRAY_BUFFER, 0);
			break;
		}
		case GeometryType::GEOMETRY:
		{
			break;
		}
	}

	transformShader -> setField <SFMatrix4f> ("modelViewMatrix", ModelViewMatrix4f ());
}

Matrix3f
ParticleSystem::getScreenAlignedRotation () const
throw (std::domain_error)
{
	Matrix4f inverseModelViewMatrix = ~ModelViewMatrix4f ();

	Vector3f billboardToScreen = inverseModelViewMatrix .multDirMatrix (zAxis);
	Vector3f viewerYAxis       = inverseModelViewMatrix .multDirMatrix (yAxis);

	Vector3f x = cross (viewerYAxis, billboardToScreen);
	Vector3f y = cross (billboardToScreen, x);
	Vector3f z = billboardToScreen;

	// Compose rotation

	x .normalize ();
	y .normalize ();
	z .normalize ();

	return Matrix3f (x [0], x [1], x [2],
	                 y [0], y [1], y [2],
	                 z [0], z [1], z [2]);
}

void
ParticleSystem::enableTexCoord () const
{
	if (getBrowser () -> getTextureStages () .empty ())
	{
		glClientActiveTexture (GL_TEXTURE0);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer (4, GL_FLOAT, sizeof (Vertex), (void*) offsetof (Vertex, texCoord));
	}
	else
	{
		for (const auto & unit : getBrowser () -> getTextureStages ())
		{
			if (unit >= 0)
			{
				glClientActiveTexture (GL_TEXTURE0 + unit);
				glEnableClientState (GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer (4, GL_FLOAT, sizeof (Vertex), (void*) offsetof (Vertex, texCoord));
			}
		}
	}
}

void
ParticleSystem::disableTexCoord () const
{
	if (getBrowser () -> getTextureStages () .empty ())
	{
		glClientActiveTexture (GL_TEXTURE0);
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		for (const auto & unit : getBrowser () -> getTextureStages ())
		{
			if (unit >= 0)
			{
				glClientActiveTexture (GL_TEXTURE0 + unit);
				glDisableClientState (GL_TEXTURE_COORD_ARRAY);
			}
		}
	}
}

void
ParticleSystem::dispose ()
{
	transformShader .dispose ();
	geometryShader  .dispose ();
	colorRampNode   .dispose ();

	if (transformFeedbackId [0])
		glDeleteTransformFeedbacks (2, transformFeedbackId .data ());

	if (particleBufferId [0])
		glDeleteBuffers (2, particleBufferId .data ());

	if (particleMapId)
		glDeleteTextures (1, &particleMapId);

	if (vertexFeedbackId)
		glDeleteTransformFeedbacks (1, &vertexFeedbackId);

	if (vertexBufferId)
		glDeleteBuffers (1, &vertexBufferId);

	if (geometryBufferId)
		glDeleteBuffers (1, &geometryBufferId);

	X3DShapeNode::dispose ();
}

} // X3D
} // titania
