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

#ifndef __TITANIA_WIDGETS_SCRIPT_EDITOR_SCRIPT_EDITOR_H__
#define __TITANIA_WIDGETS_SCRIPT_EDITOR_SCRIPT_EDITOR_H__

#include "../../ComposedWidgets.h"
#include "../../UserInterfaces/X3DScriptEditorInterface.h"
#include <Titania/X3D.h>

namespace titania {
namespace puck {

class BrowserWindow;
class NodeIndex;

class ScriptEditor :
	public X3DScriptEditorInterface
{
public:

	///  @name Construction

	ScriptEditor (BrowserWindow* const);

	///  @name Destruction

	virtual
	~ScriptEditor ();


private:

	///  @name Construction

	virtual
	void
	initialize () final override;

	virtual
	void
	on_map () final override;

	///  @name Member access

	void
	setTabs (const size_t);

	///  @name Event handlers

	void
	set_node (const X3D::SFNode &);

	virtual
	void
	on_save_clicked () final override;

	void
	set_cdata ();

	void
	connectCDATA (const X3D::MFString &);

	void
	on_loadState_clicked ();

	void
	set_loadState (const X3D::LoadState);

	///  @name Members

	std::unique_ptr <NodeIndex> nodeIndex;
	NameEntry                   nodeName;
	X3D::SFNode                 node;

};

} // puck
} // titania

#endif
