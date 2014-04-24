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

#include "X3DNetworkingContext.h"

#include "../X3DBrowser.h"
#include "../../Execution/Scene.h"

#include <omp.h>

namespace titania {
namespace X3D {

static constexpr int32_t DOWNLOAD_THREADS_MAX = 8;

const std::string X3DNetworkingContext::providerUrl = "http://titania.create3000.de";

X3DNetworkingContext::X3DNetworkingContext () :
	       X3DBaseNode (),
	         userAgent (),
	        emptyScene (),
	downloadMutexIndex (0),
	   downloadMutexes (1),
	     downloadMutex ()
{
	addChildren (emptyScene);
}

void
X3DNetworkingContext::initialize ()
{
	userAgent = getBrowser () -> getName () + "/" + getBrowser () -> getVersion () + " (X3D Browser; +" + providerUrl + ")";

	emptyScene = getBrowser () -> createScene ();
	emptyScene -> setup ();

	downloadMutexes .resize (std::min <int32_t> (omp_get_max_threads () * 2, DOWNLOAD_THREADS_MAX));
}

std::mutex &
X3DNetworkingContext::getDownloadMutex ()
{
	std::lock_guard <std::mutex> lock (downloadMutex);

	downloadMutexIndex = (downloadMutexIndex + 1) % downloadMutexes .size ();

	return downloadMutexes [downloadMutexIndex];
}

void
X3DNetworkingContext::lock ()
{
	for (auto & downloadMutex : downloadMutexes)
		downloadMutex .lock ();
}

void
X3DNetworkingContext::unlock ()
{
	for (auto & downloadMutex : downloadMutexes)
		downloadMutex .unlock ();
}

X3DNetworkingContext::~X3DNetworkingContext ()
{ }

} // X3D
} // titania
