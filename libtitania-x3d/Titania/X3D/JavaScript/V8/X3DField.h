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

#ifndef __TITANIA_X3D_JAVA_SCRIPT_V8_V8X3DFIELD_H__
#define __TITANIA_X3D_JAVA_SCRIPT_V8_V8X3DFIELD_H__

#include "X3DFieldDefinition.h"

namespace titania {
namespace X3D {
namespace GoogleV8 {

template <class T>
class X3DField :
	public X3DFieldDefinition <T>
{
public:

	using X3DFieldDefinition <T>::TypeName;
	using X3DFieldDefinition <T>::Type;

	///  @name Operations

	static
	v8::Handle <v8::Value>
	setValue (Context* const, T* const, const v8::Local <v8::Value> &)
	throw (std::invalid_argument);


protected:

	using X3DFieldDefinition <T>::getObject;

	///  @name Functions

	static
	v8::Handle <v8::Value>
	getType (const v8::Arguments &);

	static
	v8::Handle <v8::Value>
	isReadable (const v8::Arguments &);

	static
	v8::Handle <v8::Value>
	isWritable (const v8::Arguments &);

};

template <class T>
v8::Handle <v8::Value>
X3DField <T>::setValue (Context* const context, T* const field, const v8::Local <v8::Value> & value)
throw (std::invalid_argument)
{
	if (context -> getClass (Type ()) -> HasInstance (value))
	{
		field -> setValue (*getArgument <T> (value));
		return value;
	}

	throw std::invalid_argument ("RuntimeError: couldn't assign value to field '" + field -> getName () + "', value has wrong type, must be " + TypeName () + ".");
}

template <class T>
v8::Handle <v8::Value>
X3DField <T>::getType (const v8::Arguments & args)
{
	try
	{
		if (args .Length () not_eq 0)
			return v8::ThrowException (String (TypeName () + ".getType: wrong number of arguments."));

		const auto lhs = getObject (args);

		return v8::Number::New (lhs -> getType ());
	}
	catch (const std::exception & error)
	{
		return v8::ThrowException (String (error .what ()));
	}
}

template <class T>
v8::Handle <v8::Value>
X3DField <T>::isReadable (const v8::Arguments & args)
{
	try
	{
		if (args .Length () not_eq 0)
			return v8::ThrowException (String (TypeName () + ".isReadable: wrong number of arguments."));

		const auto lhs = getObject (args);

		return v8::Boolean::New (lhs -> getAccessType () not_eq inputOnly);
	}
	catch (const std::exception & error)
	{
		return v8::ThrowException (String (error .what ()));
	}
}

template <class T>
v8::Handle <v8::Value>
X3DField <T>::isWritable (const v8::Arguments & args)
{
	try
	{
		if (args .Length () not_eq 0)
			return v8::ThrowException (String (TypeName () + ".isWritable: wrong number of arguments."));

		const auto lhs = getObject (args);

		return v8::Boolean::New (lhs -> getAccessType () not_eq initializeOnly);
	}
	catch (const std::exception & error)
	{
		return v8::ThrowException (String (error .what ()));
	}
}

} // GoogleV8
} // X3D
} // titania

#endif
