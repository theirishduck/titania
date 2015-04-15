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

#include "GarbageCollector.h"

#include "../Base/pbChildObject.h"
#include "../Cache/Cache.h"
#include "../Objects/Object.h"
#include "../Objects/pbFunction.h"

#include <malloc.h>
#include <thread>

#include <Titania/Backtrace.h>

namespace titania {
namespace pb {

// Important: std::deque is used here for objects because it is much more faster than std::vector!

GarbageCollector::ObjectArray GarbageCollector::objects;
std::mutex                    GarbageCollector::mutex;

void
GarbageCollector::trimFreeMemory ()
{
	malloc_trim (0);
}

void
GarbageCollector::addDisposedObject (const pbChildObject* const object)
{
	std::lock_guard <std::mutex> lock (mutex);

	objects .emplace_back (object);
}

void
GarbageCollector::deleteObjectsAsync ()
{
	if (objects .empty () and pbCache::empty ())
		return;

	std::thread (&GarbageCollector::deleteObjects, std::move (objects)) .detach ();
}

void
GarbageCollector::deleteObjects (const ObjectArray & objects)
{
	// Residual waste
	
	//__LOG__ << objects .size () << " : " << pbCache::size () << std::endl;

	for (const auto & object : objects)
		delete object;

	// Recycled waste

	ObjectArray cache;

	Cache <ptr <pbObject>>::clear (cache);
	Cache <Object>::clear (cache);

	for (const auto & object : cache)
		delete object;
}

} // pb
} // titania