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

#ifndef __TITANIA_X3D_JAVA_SCRIPT_PEASE_BLOSSOM_FIELDS_X3DSCALAR_H__
#define __TITANIA_X3D_JAVA_SCRIPT_PEASE_BLOSSOM_FIELDS_X3DSCALAR_H__

#include "../../../Fields/SFString.h"
#include "../../../Fields/SFTime.h"
#include "../../../Fields/X3DScalar.h"
#include "../X3DField.h"

namespace titania {
namespace X3D {
namespace peaseblossom {

class SFBool
{
public:

	using internal_type = X3D::SFBool;

};

class SFDouble
{
public:

	using internal_type = X3D::SFDouble;

};

class SFFloat
{
public:

	using internal_type = X3D::SFFloat;

};

class SFInt32
{
public:

	using internal_type = X3D::SFInt32;

};

class SFString
{
public:

	using internal_type = X3D::SFString;

};

class SFTime
{
public:

	using internal_type = X3D::SFTime;

};

template <>
inline
pb::var
X3DField::get <SFBool> (Context* const, SFBool::internal_type* const field)
{
	return field -> getValue ();
}

template <>
inline
pb::var
X3DField::get <SFDouble> (Context* const, SFDouble::internal_type* const field)
{
	return field -> getValue ();
}

template <>
inline
pb::var
X3DField::get <SFFloat> (Context* const, SFFloat::internal_type* const field)
{
	return field -> getValue ();
}

template <>
inline
pb::var
X3DField::get <SFInt32> (Context* const, SFInt32::internal_type* const field)
{
	return field -> getValue ();
}

template <>
inline
pb::var
X3DField::get <SFString> (Context* const, SFString::internal_type* const field)
{
	return field -> getValue ();
}

template <>
inline
pb::var
X3DField::get <SFTime> (Context* const, SFTime::internal_type* const field)
{
	return field -> getValue ();
}

} // peaseblossom
} // X3D
} // titania

#endif