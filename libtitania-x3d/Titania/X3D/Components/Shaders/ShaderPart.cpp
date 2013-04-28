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

#include "ShaderPart.h"

#include "../../Execution/X3DExecutionContext.h"

namespace titania {
namespace X3D {

ShaderPart::Fields::Fields () :
	type (new SFString ("VERTEX"))
{ }

ShaderPart::ShaderPart (X3DExecutionContext* const executionContext) :
	 X3DBaseNode (executionContext -> getBrowser (), executionContext), 
	     X3DNode (),                                                    
	X3DUrlObject (),                                                    
	      fields (),
	    shaderId (0),                                                   
	       valid (false)                                                
{
	setComponent ("Shaders");
	setTypeName ("ShaderPart");

	addField (inputOutput,    "metadata", metadata ());
	addField (initializeOnly, "type",     type ());
	addField (inputOutput,    "url",      url ());
}

X3DBaseNode*
ShaderPart::create (X3DExecutionContext* const executionContext) const
{
	return new ShaderPart (executionContext);
}

void
ShaderPart::initialize ()
{
	X3DNode::initialize ();
	X3DUrlObject::initialize ();

	url () .addInterest (this, &ShaderPart::set_url);

	if (glXGetCurrentContext ())
	{
		shaderId = glCreateShader (getShaderType ());

		requestImmediateLoad ();
	}
}

GLenum
ShaderPart::getShaderType () const
{
	if (type () == "FRAGMENT")
		return GL_FRAGMENT_SHADER;

	if (type () == "VERTEX")
		return GL_VERTEX_SHADER;

	return GL_VERTEX_SHADER;
}

void
ShaderPart::requestImmediateLoad ()
{
	if (checkLoadState () == COMPLETE_STATE or checkLoadState () == IN_PROGRESS_STATE)
		return;

	setLoadState (IN_PROGRESS_STATE);

	for (const auto & URL : url ())
	{
		try
		{
			std::string shaderSource = loadDocument (URL);
			const char* string       = shaderSource .c_str ();

			glShaderSource  (shaderId, 1, &string, NULL);
			glCompileShader (shaderId);

			glGetShaderiv (shaderId, GL_COMPILE_STATUS, &valid);
			
			std::clog << getInfoLog ();

			if (not valid)
				continue;
		}
		catch (const X3DError & error)
		{
			std::clog << error .what () << std::endl;
		}
	}

	setLoadState (valid ? COMPLETE_STATE : FAILED_STATE);	
}

std::string
ShaderPart::getInfoLog () const
{
	GLint infoLogLength;

	glGetShaderiv (shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength)
	{
		char infoLog [infoLogLength];
		glGetShaderInfoLog (shaderId, infoLogLength, 0, infoLog);

		return infoLog;
	}

	return std::string ();
}

void
ShaderPart::set_url ()
{
	setLoadState (NOT_STARTED_STATE);

	requestImmediateLoad ();
}

void
ShaderPart::dispose ()
{
	if (shaderId)
		glDeleteShader (shaderId);

	X3DUrlObject::dispose ();
	X3DNode::dispose ();
}

} // X3D
} // titania

