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

#include "X3DPointLightEditor.h"

#include <Titania/X3D/Components/Lighting/PointLight.h>
#include <Titania/X3D/Components/Layering/X3DLayerNode.h>
#include <Titania/X3D/Execution/World.h>

namespace titania {
namespace puck {

X3DPointLightEditor::X3DPointLightEditor () :
	X3DLightEditorInterface (),
	            attenuation (this,
	                         getPointLightAttenuationXAdjustment (),
	                         getPointLightAttenuationYAdjustment (),
	                         getPointLightAttenuationZAdjustment (),
	                         getPointLightAttenuationBox (),
	                         "attenuation"),
	               location (this,
	                         getPointLightLocationXAdjustment (),
	                         getPointLightLocationYAdjustment (),
	                         getPointLightLocationZAdjustment (),
	                         getPointLightLocationBox (),
	                         "location"),
	                 radius (this, getPointLightRadiusAdjustment (), getPointLightRadiusSpinButton (), "radius")
{ }

void
X3DPointLightEditor::setPointLight (const X3D::X3DPtr <X3D::X3DLightNode> & lightNode)
{
	const X3D::X3DPtr <X3D::PointLight> pointLight (lightNode);

	getPointLightExpander () .set_visible (pointLight);

	const auto pointLights = pointLight ? X3D::MFNode ({ pointLight }) : X3D::MFNode ();

	attenuation .setNodes (pointLights);
	location    .setNodes (pointLights);
	radius      .setNodes (pointLights);
}

void
X3DPointLightEditor::on_new_point_light_activated ()
{
	const auto undoStep = std::make_shared <UndoStep> (_ ("Create New PointLight"));

	const auto & activeLayer = getWorld () -> getActiveLayer ();
	auto &       children    = activeLayer and activeLayer not_eq getWorld () -> getLayer0 ()
	                           ? activeLayer -> children ()
	                           : getExecutionContext () -> getRootNodes ();

	undoStep -> addObjects (getExecutionContext (), activeLayer);
	undoStep -> addUndoFunction (&X3D::MFNode::setValue, std::ref (children), children);

	const auto node = getExecutionContext () -> createNode <X3D::PointLight> ();
	children .emplace_back (node);
	getExecutionContext () -> realize ();
	getBrowserWindow () -> getSelection () -> setChildren ({ node }, undoStep);

	undoStep -> addRedoFunction (&X3D::MFNode::setValue, std::ref (children), children);
	getBrowserWindow () -> addUndoStep (undoStep);
}

} // puck
} // titania
