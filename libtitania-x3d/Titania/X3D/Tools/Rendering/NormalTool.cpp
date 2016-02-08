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

#include "NormalTool.h"

#include "../../Browser/Networking/config.h"
#include "../../Browser/X3DBrowser.h"
#include "../../Execution/X3DExecutionContext.h"

namespace titania {
namespace X3D {

const ComponentType NormalTool::component      = ComponentType::TITANIA;
const std::string   NormalTool::typeName       = "NormalTool";
const std::string   NormalTool::containerField = "grid";

NormalTool::Fields::Fields () :
	        enabled (new SFBool (true)),
	modelViewMatrix (new SFMatrix4f ()),
	         length (new SFFloat (1)),
	          color (new SFColorRGBA (0.7, 0.85, 1, 1)),
	    vertexCount (new MFInt32 ()),
	          point (new MFVec3f ())
{ }

NormalTool::NormalTool (X3DExecutionContext* const executionContext) :
	      X3DBaseNode (executionContext -> getBrowser (), executionContext),
	X3DNormalNodeTool (),
	           fields (),
	             show (false)
{
	//addType (X3DConstants::NormalTool);

	addField (inputOutput, "metadata",        metadata ());
	addField (inputOutput, "enabled",         enabled ());
	addField (inputOutput, "modelViewMatrix", modelViewMatrix ());
	addField (inputOutput, "length",          length ());
	addField (inputOutput, "color",           color ());
	addField (inputOutput, "vertexCount",     vertexCount ());
	addField (inputOutput, "point",           point ());
}

X3DBaseNode*
NormalTool::create (X3DExecutionContext* const executionContext) const
{
	return new NormalTool (executionContext);
}

void
NormalTool::initialize ()
{
	X3DNormalNodeTool::initialize ();

	enabled () .addInterest (getInlineNode () -> load ());

	getInlineNode () -> load () = enabled ();
	getInlineNode () -> url ()  = { get_tool ("NormalTool.x3dv") .str () };
}

void
NormalTool::realize ()
{
	X3DNormalNodeTool::realize ();

	try
	{
		getBrowser () -> prepareEvents () .addInterest (this, &NormalTool::prepareEvent);

		auto & set_modelViewMatrix = getToolNode () -> getField <SFMatrix4f> ("set_modelViewMatrix");
		modelViewMatrix () .addInterest (set_modelViewMatrix);
		set_modelViewMatrix = modelViewMatrix ();

		auto & set_vertexCount = getToolNode () -> getField <MFInt32> ("set_vertexCount");
		vertexCount () .addInterest (set_vertexCount);
		set_vertexCount = vertexCount ();

		length () .addInterest (this, &NormalTool::set_point);
		color ()  .addInterest (this, &NormalTool::set_color);
		point ()  .addInterest (this, &NormalTool::set_point);

		set_point ();
	}
	catch (const X3DError & error)
	{
		__LOG__ << error .what () << std::endl;
	}
}

void
NormalTool::prepareEvent ()
{
	try
	{
		getToolNode () -> setField <SFBool> ("set_enabled", show, true);
		show = false;
	}
	catch (const X3DError & error)
	{
		__LOG__ << error .what () << std::endl;
	}
}

void
NormalTool::set_color ()
{
	try
	{
		getToolNode () -> setField <SFColor> ("set_color", Color3f (color () .getRed (), color () .getGreen (), color () .getBlue ()));
		getToolNode () -> setField <SFFloat> ("set_transparency", color () .getAlpha ());
	}
	catch (const X3DError & error)
	{
		__LOG__ << error .what () << std::endl;
	}
}

void
NormalTool::set_point ()
{
	try
	{
		if (length () == 1.0f)
			getToolNode () -> setField <MFVec3f> ("set_point", point ());

		else
		{
			auto & set_point = getToolNode () -> getField <MFVec3f> ("set_point");
			set_point = point ();

			for (size_t i = 0, size = point () .size (); i < size; i += 2)
			{
				const auto normal = set_point [i + 1] .getValue () - set_point [i + 0] .getValue ();

				set_point [i + 1] = set_point [i + 0] .getValue () + normal * length () .getValue ();
			}
		}
	}
	catch (const X3DError & error)
	{
		__LOG__ << error .what () << std::endl;
	}
}

void
NormalTool::draw (const ShapeContainer* const container)
{
	if (getCurrentLayer () != getActiveLayer ())
		return;

	show = true;

	const auto mvm = container -> getModelViewMatrix () * getCurrentViewpoint () -> getCameraSpaceMatrix ();

	if (mvm != modelViewMatrix ())
		modelViewMatrix () = mvm;
}

} // X3D
} // titania