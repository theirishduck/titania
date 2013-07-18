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

#include "X3DScalar.h"

#include "../Parser/Grammar.h"

namespace titania {
namespace X3D {

template <>
const std::string X3DField <bool>::typeName ("SFBool");

template <>
const std::string X3DField <Double>::typeName ("SFDouble");

template <>
const std::string X3DField <Float>::typeName ("SFFloat");

template <>
const std::string X3DField <Int32>::typeName ("SFInt32");

template <>
const X3DConstants::FieldType X3DField <bool>::type = X3DConstants::SFBool;

template <>
const X3DConstants::FieldType X3DField <Double>::type = X3DConstants::SFDouble;

template <>
const X3DConstants::FieldType X3DField <Float>::type = X3DConstants::SFFloat;

template <>
const X3DConstants::FieldType X3DField <Int32>::type = X3DConstants::SFInt32;

template <>
void
X3DScalar <bool>::fromStream (std::istream & istream)
throw (Error <INVALID_X3D>,
       Error <NOT_SUPPORTED>,
       Error <INVALID_OPERATION_TIMING>,
       Error <DISPOSED>)
{
	Grammar::whitespaces (istream);

	if (Grammar::_true (istream))
	{
		setValue (true);
		return;
	}

	if (Grammar::_false (istream))
	{
		setValue (false);
		return;
	}
}

template <>
void
X3DScalar <Double>::fromStream (std::istream & istream)
throw (Error <INVALID_X3D>,
       Error <NOT_SUPPORTED>,
       Error <INVALID_OPERATION_TIMING>,
       Error <DISPOSED>)
{
	Grammar::whitespaces (istream);

	istream >> get ();

	if (istream)
		addEvent ();
}

template <>
void
X3DScalar <Float>::fromStream (std::istream & istream)
throw (Error <INVALID_X3D>,
       Error <NOT_SUPPORTED>,
       Error <INVALID_OPERATION_TIMING>,
       Error <DISPOSED>)
{
	Grammar::whitespaces (istream);

	istream >> get ();

	if (istream)
		addEvent ();
}

template <>
void
X3DScalar <Int32>::fromStream (std::istream & istream)
throw (Error <INVALID_X3D>,
       Error <NOT_SUPPORTED>,
       Error <INVALID_OPERATION_TIMING>,
       Error <DISPOSED>)
{
	Grammar::whitespaces (istream);

	Grammar::Int32 (istream, get ());

	if (istream)
		addEvent ();
}

template <>
void
X3DScalar <bool>::toStream (std::ostream & ostream) const
{
	ostream << (getValue () ? "TRUE" : "FALSE");
}

template <>
void
X3DScalar <Double>::toStream (std::ostream & ostream) const
{
	ostream << Generator::Precision <Double> << getValue ();
}

template <>
void
X3DScalar <Float>::toStream (std::ostream & ostream) const
{
	ostream << Generator::Precision <Float> << getValue ();
}

template <>
void
X3DScalar <Int32>::toStream (std::ostream & ostream) const
{
	ostream << getValue ();
}

template class X3DField <bool>;
template class X3DField <Double>;
template class X3DField <Float>;
template class X3DField <Int32>;

// SFBool, SFDouble, SFFloat and SFInt32
template class X3DScalar <bool>;
template class X3DScalar <Double>;
template class X3DScalar <Float>;
template class X3DScalar <Int32>;

} // X3D
} // titania
