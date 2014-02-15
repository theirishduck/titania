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

#ifndef __TITANIA_X3D_COMPONENTS_LAYERING_X3DLAYER_NODE_H__
#define __TITANIA_X3D_COMPONENTS_LAYERING_X3DLAYER_NODE_H__

#include "../../Rendering/CollisionArray.h"
#include "../../Rendering/X3DRenderer.h"
#include "../EnvironmentalEffects/LocalFog.h"
#include "../Grouping/X3DGroupingNode.h"
#include "../Layering/X3DViewportNode.h"

#include "../EnvironmentalEffects/Fog.h"
#include "../EnvironmentalEffects/X3DBackgroundNode.h"
#include "../Navigation/NavigationInfo.h"
#include "../Navigation/X3DViewpointNode.h"

#include <stack>

namespace titania {
namespace X3D {

template <class Type>
class X3DBindableNodeStack;

template <class Type>
class X3DBindableNodeList;

typedef X3DBindableNodeStack <NavigationInfo>    NavigationInfoStack;
typedef X3DBindableNodeStack <X3DBackgroundNode> BackgroundStack;
typedef X3DBindableNodeStack <Fog>               FogStack;
typedef X3DBindableNodeStack <X3DViewpointNode>  ViewpointStack;

typedef X3DBindableNodeList <NavigationInfo>    NavigationInfoList;
typedef X3DBindableNodeList <X3DBackgroundNode> BackgroundList;
typedef X3DBindableNodeList <Fog>               FogList;
typedef X3DBindableNodeList <X3DViewpointNode>  ViewpointList;

typedef std::vector <X3D::X3DViewpointNode*> UserViewpointList;
typedef std::stack <LocalFog*>               LocalFogStack;

class X3DLayerNode :
	virtual public X3DNode, public X3DRenderer
{
public:

	///  @name Fields

	SFBool &
	isPickable ()
	{ return *fields .isPickable; }

	const SFBool &
	isPickable () const
	{ return *fields .isPickable; }

	SFNode &
	viewport ()
	{ return *fields .viewport; }

	const SFNode &
	viewport () const
	{ return *fields .viewport; }

	MFNode &
	addChildren ()
	{ return *fields .addChildren; }

	const MFNode &
	addChildren () const
	{ return *fields .addChildren; }

	MFNode &
	removeChildren ()
	{ return *fields .removeChildren; }

	const MFNode &
	removeChildren () const
	{ return *fields .removeChildren; }

	MFNode &
	children ()
	{ return *fields .children; }

	const MFNode &
	children () const
	{ return *fields .children; }

	///  @name Member access

	void
	isLayer0 (bool value)
	{ layer0 = value; }

	bool
	isLayer0 () const
	{ return layer0; }

	Box3f
	getBBox ();

	///  @name Bindable node stack handling

	X3DViewportNode*
	getViewport () const
	{ return currentViewport; }

	NavigationInfo*
	getNavigationInfo () const;

	X3DBackgroundNode*
	getBackground () const;

	X3DFogObject*
	getFog () const;

	X3DViewpointNode*
	getViewpoint () const;

	///  @name X3DBindableNode stack handling

	const X3DSFNode <NavigationInfoStack> &
	getNavigationInfoStack () const
	{ return navigationInfoStack; }

	const X3DSFNode <BackgroundStack> &
	getBackgroundStack () const
	{ return backgroundStack; }

	const X3DSFNode <FogStack> &
	getFogStack () const
	{ return fogStack; }

	const X3DSFNode <ViewpointStack> &
	getViewpointStack () const
	{ return viewpointStack; }

	///  @name X3DBindableNode list handling

	const X3DSFNode <NavigationInfoList> &
	getNavigationInfos () const
	{ return navigationInfos; }

	const X3DSFNode <BackgroundList> &
	getBackgrounds () const
	{ return backgrounds; }

	const X3DSFNode <FogList> &
	getFogs () const
	{ return fogs; }

	const X3DSFNode <ViewpointList> &
	getViewpoints () const
	{ return viewpoints; }

	UserViewpointList
	getUserViewpoints () const;

	///  @name Fog handling

	LocalFogStack &
	getLocalFogs ()
	{ return localFogs; }

	///  @name Collision handling

	CollisionArray &
	getCollisions ()
	{ return collisions; }

	///  @name Operations

	Vector3f
	getTranslation (const Vector3f &, const float, const float, const Vector3f &);

	void
	lookAt ();

	virtual
	void
	traverse (const TraverseType) override;

	///  @name Destruction

	virtual
	void
	dispose () override;


protected:

	using X3DNode::addChildren;
	using X3DNode::removeChildren;

	///  @name Construction

	X3DLayerNode (X3DViewpointNode*, X3DGroupingNode*);

	virtual
	void
	initialize () override;

	///  @name Member access

	const X3DSFNode <X3DGroupingNode>
	getGroup () const
	{ return group; }


private:

	///  @name Member access

	float
	getDistance (const Vector3f &, const float, const float, const Vector3f &);

	///  @name Operations

	void
	set_viewport ();

	void
	pick ();

	void
	camera ();

	void
	navigation ();

	void
	collision ();

	void
	collect ();

	virtual
	void
	collect (const TraverseType) override;

	///  @name Members

	struct Fields
	{
		Fields ();

		SFBool* const isPickable;
		SFNode* const viewport;
		MFNode* const addChildren;
		MFNode* const removeChildren;
		MFNode* const children;
	};

	Fields fields;
	
	bool layer0;

	X3DSFNode <NavigationInfo>    defaultNavigationInfo;
	X3DSFNode <X3DBackgroundNode> defaultBackground;
	X3DSFNode <Fog>               defaultFog;
	X3DSFNode <X3DViewpointNode>  defaultViewpoint;

	X3DViewportNode* currentViewport;

	X3DSFNode <NavigationInfoStack> navigationInfoStack;
	X3DSFNode <BackgroundStack>     backgroundStack;
	X3DSFNode <FogStack>            fogStack;
	X3DSFNode <ViewpointStack>      viewpointStack;

	X3DSFNode <NavigationInfoList> navigationInfos;
	X3DSFNode <BackgroundList>     backgrounds;
	X3DSFNode <FogList>            fogs;
	X3DSFNode <ViewpointList>      viewpoints;

	LocalFogStack  localFogs;
	CollisionArray collisions;

	X3DSFNode <X3DGroupingNode> group;

};

} // X3D
} // titania

#endif
