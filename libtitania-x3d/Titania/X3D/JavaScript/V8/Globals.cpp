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

#include "Globals.h"

#include "../../Browser/X3DBrowser.h"
#include "String.h"

namespace titania {
namespace X3D {
namespace GoogleV8 {

void
Globals::initialize (const v8::Local <v8::External> & context, const v8::Local <v8::Object> & global)
{
	global -> Set (String ("NULL"),  v8::Null (),              v8::PropertyAttribute (v8::ReadOnly | v8::DontDelete));
	global -> Set (String ("FALSE"), v8::Boolean::New (false), v8::PropertyAttribute (v8::ReadOnly | v8::DontDelete));
	global -> Set (String ("TRUE"),  v8::Boolean::New (true),  v8::PropertyAttribute (v8::ReadOnly | v8::DontDelete));

	global -> Set (String ("print"), v8::FunctionTemplate::New (print, context) -> GetFunction (), v8::PropertyAttribute (v8::ReadOnly | v8::DontDelete | v8::DontEnum));
	global -> Set (String ("trace"), v8::FunctionTemplate::New (print, context) -> GetFunction (), v8::PropertyAttribute (v8::ReadOnly | v8::DontDelete | v8::DontEnum));
}

v8::Handle <v8::Value>
Globals::print (const v8::Arguments & args)
{
	const auto browser = getContext (args) -> getBrowser ();

	for (size_t i = 0, size = args .Length (); i < size; ++ i)
		browser -> print (to_string (args [i]));

	browser -> print ("\n");

	return v8::Undefined ();
}

} // GoogleV8
} // X3D
} // titania