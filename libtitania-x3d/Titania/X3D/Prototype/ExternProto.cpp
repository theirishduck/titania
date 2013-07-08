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

#include "ExternProto.h"

#include "../Bits/Error.h"
#include "../Components/Core/X3DPrototypeInstance.h"
#include "../Parser/RegEx.h"

#include <iomanip>

namespace titania {
namespace X3D {

ExternProto::ExternProto (X3DExecutionContext* const executionContext) :
	 X3DBaseNode (executionContext -> getBrowser (), executionContext), 
	    X3DProto (),                                                    
	X3DUrlObject (),                                                    
	       scene (),                                                    
	       proto ()                                                     
{
	setTypeName ("ExternProto");

	addField (inputOutput, "metadata", metadata ());

	addChildren (url (), scene, proto);
}

X3DBaseNode*
ExternProto::create (X3DExecutionContext* const executionContext) const
{
	return new ExternProto (executionContext);
}

ExternProto*
ExternProto::clone (X3DExecutionContext* const executionContext) const
{
	ExternProto* copy = dynamic_cast <ExternProto*> (X3DProto::clone (executionContext));

	copy -> url () = url ();

	return copy;
}

void
ExternProto::initialize ()
{
	X3DProto::initialize ();
	X3DUrlObject::initialize ();
}

X3DPrototypeInstance*
ExternProto::createInstance (X3DExecutionContext* const executionContext)
{
	requestImmediateLoad ();

	return new X3DPrototypeInstance (executionContext, proto);
}

void
ExternProto::requestImmediateLoad ()
{
	if (checkLoadState () == COMPLETE_STATE or checkLoadState () == IN_PROGRESS_STATE)
		return;

	setLoadState (IN_PROGRESS_STATE);

	try
	{
		scene = createX3DFromURL (url ());
	}
	catch (const Error <INVALID_URL> & error)
	{
		scene .setValue (NULL);

		setLoadState (FAILED_STATE);

		throw Error <URL_UNAVAILABLE> ("Couldn't load any URL specified for EXTERNPROTO '" + getName () + "'\n" + error .what ());
	}

	std::string protoName = getWorldURL () .fragment () .length ()
	                        ? getWorldURL () .fragment ()
									: getName ();

	proto = scene -> getProtoDeclaration (protoName);

	if (proto)
	{
		for (const auto & fieldDefinition : getUserDefinedFields ())
		{
			try
			{
				X3DFieldDefinition* protoField = proto -> getField (fieldDefinition -> getName ());

				if (protoField -> getAccessType () == fieldDefinition -> getAccessType ())
				{
					if (protoField -> getType () == fieldDefinition -> getType ())
					{
						fieldDefinition -> write (*protoField);
					}
					else
					{
						setLoadState (FAILED_STATE);
						throw Error <INVALID_FIELD> ("EXTERNPROTO '" + getName () + "' field '" + fieldDefinition -> getName () + "' and PROTO field have different types");
					}
				}
				else
				{
					setLoadState (FAILED_STATE);
					throw Error <INVALID_ACCESS_TYPE> ("EXTERNPROTO '" + getName () + "' field '" + fieldDefinition -> getName () + "' and PROTO field have different access types");
				}
			}
			catch (const Error <INVALID_NAME> &)
			{
				setLoadState (FAILED_STATE);
				throw Error <INVALID_NAME> ("EXTERNPROTO field '" + fieldDefinition -> getName () + "' not found in PROTO '" + proto -> getName () + "'");
			}
		}

		setLoadState (COMPLETE_STATE);

		return;
	}
	else
	{
		setLoadState (FAILED_STATE);
		throw Error <INVALID_NAME> ("No PROTO '" + protoName + "' found for EXTERNPROTO '" + getName () + "' in url '" + getWorldURL () + "'");
	}

}

void
ExternProto::toStream (std::ostream & ostream) const
{
	if (getComments () .size ())
	{
		for (const auto & comment : getComments ())
		{
			ostream
				<< Generator::Indent
				<< Generator::Comment
				<< comment
				<< Generator::Break;
		}
	
		ostream << Generator::TidyBreak;
	}

	ostream
		<< Generator::Indent
		<< "EXTERNPROTO"
		<< Generator::Space
		<< getName ()
		<< Generator::TidySpace
		<< '[';

	size_t typeLength       = 0;
	size_t accessTypeLength = 0;

	FieldDefinitionArray fields = getUserDefinedFields ();

	if (fields .size ())
	{
		if (Generator::Style () not_eq "clean")
		{
			for (const auto & field : fields)
			{
				typeLength = std::max (typeLength, field -> getTypeName () .length ());

				accessTypeLength = std::max (accessTypeLength, Generator::AccessTypes [field] .length ());
			}
		}

		ostream
			<< Generator::TidyBreak
			<< Generator::IncIndent;

		for (const auto & field : fields)
		{
			for (const auto & comment : field -> getComments ())
			{
				ostream
					<< Generator::Indent
					<< Generator::Comment
					<< comment
					<< Generator::Break;
			}
		
			ostream
				<< Generator::Indent
				<< std::setiosflags (std::ios::left)
				<< std::setw (accessTypeLength);

			ostream << Generator::AccessTypes [field];

			ostream
				<< Generator::Space
				<< std::setiosflags (std::ios::left) << std::setw (typeLength) << field -> getTypeName ()
				<< Generator::Space
				<< field -> getName ()
				<< Generator::Break;
		}
		
		for (const auto & comment : getInterfaceComments ())
		{
			ostream
				<< Generator::Indent
				<< Generator::Comment
				<< comment
				<< Generator::Break;
		}

		ostream
			<< Generator::DecIndent
			<< Generator::Indent;
	}
	else
	{
		if (getInterfaceComments () .size ())
		{
			ostream
				<< Generator::TidyBreak
				<< Generator::IncIndent;
				
			for (const auto & comment : getInterfaceComments ())
			{
				ostream
					<< Generator::Indent
					<< Generator::Comment
					<< comment
					<< Generator::Break;
			}
	
			ostream
				<< Generator::DecIndent
				<< Generator::Indent;
		}
		
		else
			ostream << Generator::TidySpace;
	}

	ostream
		<< ']'
		<< Generator::TidyBreak;

	for (const auto & comment : getInnerComments ())
	{
		ostream
			<< Generator::Indent
			<< Generator::Comment
			<< comment
			<< Generator::Break;
	}

	ostream
		<< Generator::Indent
		<< url ();
}

void
ExternProto::dispose ()
{
	removeChildren (url ());

	scene .dispose ();
	proto .dispose ();

	X3DUrlObject::dispose ();
	X3DProto::dispose ();
}

ExternProto::~ExternProto ()
{ }

} // X3D
} // titania
