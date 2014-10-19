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

#include "MagicImport.h"

#include "X3DBrowserWindow.h"
#include "BrowserSelection.h"

#include <Titania/X3D/Browser/ContextLock.h>

namespace titania {
namespace puck {

using namespace std::placeholders;

MagicImport::MagicImport (X3DBrowserWindow* const browserWindow) :
	X3DBaseInterface (browserWindow, browserWindow -> getBrowser ()),
	 importFunctions ({ std::make_pair ("Material", std::bind (&MagicImport::material, this, _1, _2, _3)),
	                    std::make_pair ("Texture",  std::bind (&MagicImport::texture,  this, _1, _2, _3)) })
{
	setup ();
}

bool
MagicImport::import (X3D::MFNode & selection, const X3D::X3DScenePtr & scene, const UndoStepPtr & undoStep)
{
	if (selection .empty ())
		return false;

	try
	{
		const std::string magic = scene -> getMetaData ("titania magic");

		return importFunctions .at (magic) (selection, scene, undoStep);
	}
	catch (const X3D::Error <X3D::INVALID_NAME> &)
	{
		return false;
	}
	catch (const std::out_of_range &)
	{
		return false;
	}
}

bool
MagicImport::material (X3D::MFNode & selection, const X3D::X3DScenePtr & scene, const UndoStepPtr & undoStep)
{
	X3D::ContextLock lock (getBrowser ());

	if (not lock)
		return false;

	// Find first material node in scene

	X3D::SFNode material;

	X3D::traverse (scene -> getRootNodes (), [this, &material, &undoStep] (X3D::SFNode & node)
	               {
	                  const X3D::AppearancePtr appearance (node);

	                  if (appearance and X3D::x3d_cast <X3D::X3DMaterialNode*> (appearance -> material ()))
	                  {
	                     importProtoDeclaration (appearance -> material (), undoStep);
	                     
	                     material = appearance -> material () -> copy (getExecutionContext (), X3D::FLAT_COPY);
	                     return false;
							}

	                  return true;
						});

	// Assign material to all appearances in selection

	X3D::traverse (selection, [this, &material, &undoStep] (X3D::SFNode & node)
	               {
	                  const auto appearance = dynamic_cast <X3D::Appearance*> (node .getValue ());

	                  if (appearance)
	                     getBrowserWindow () -> replaceNode (node, appearance -> material (), material, undoStep);

	                  return true;
						});

	getExecutionContext () -> realize ();
	return true;
}

bool
MagicImport::texture (X3D::MFNode & selection, const X3D::X3DScenePtr & scene, const UndoStepPtr & undoStep)
{
	X3D::ContextLock lock (getBrowser ());

	if (not lock)
		return false;

	// Find first texture node in scene

	X3D::SFNode texture;

	X3D::traverse (scene -> getRootNodes (), [this, &texture, &undoStep] (X3D::SFNode & node)
	               {
	                  const X3D::AppearancePtr appearance (node);

	                  if (appearance and X3D::x3d_cast <X3D::X3DTextureNode*> (appearance -> texture ()))
	                  {
	                     importProtoDeclaration (appearance -> texture (), undoStep);
	                     
	                     texture = appearance -> texture () -> copy (getExecutionContext (), X3D::FLAT_COPY);
	                     return false;
							}

	                  return true;
						});

	const X3D::X3DPtr <X3D::X3DTexture2DNode> texture2D (texture);
	const X3D::X3DPtr <X3D::X3DTexture3DNode> texture3D (texture);

	// Assign texture to all appearances in selection

	X3D::traverse (selection, [&] (X3D::SFNode & node)
	               {
	                  const auto appearance = dynamic_cast <X3D::Appearance*> (node .getValue ());

	                  if (appearance)
	                  {
								const X3D::X3DPtr <X3D::X3DTexture2DNode> oldTexture2D (appearance -> texture ());
								const X3D::X3DPtr <X3D::X3DTexture3DNode> oldTexture3D (appearance -> texture ());

	                     if (oldTexture2D and texture2D)
	                     {
	                        texture2D -> repeatS ()           = oldTexture2D -> repeatS ();
	                        texture2D -> repeatT ()           = oldTexture2D -> repeatT ();
	                        texture2D -> textureProperties () = oldTexture2D -> textureProperties ();
	                     }
                        else if (oldTexture3D and texture3D)
	                     {
	                        texture3D -> repeatS ()           = oldTexture3D -> repeatS ();
	                        texture3D -> repeatT ()           = oldTexture3D -> repeatT ();
	                        texture3D -> repeatR ()           = oldTexture3D -> repeatR ();
	                        texture3D -> textureProperties () = oldTexture3D -> textureProperties ();
	                     }

	                     getBrowserWindow () -> replaceNode (node, appearance -> texture (), texture, undoStep);
	                  }

	                  return true;
						});

	getExecutionContext () -> realize ();
	return true;
}

void
MagicImport::importProtoDeclaration (const X3D::SFNode & node, const UndoStepPtr & undoStep)
{
	const auto prototypeInstance = dynamic_cast <X3D::X3DPrototypeInstance*> (node .getValue ());

	if (prototypeInstance)
	{
		const auto name          = prototypeInstance -> getProtoNode () -> getName ();
		const bool isExternProto = prototypeInstance -> getProtoNode () -> isExternproto ();

		try
		{
			const auto protoDeclaration = getExecutionContext () -> getProtoDeclaration (name);
			
			undoStep -> addUndoFunction (&X3D::X3DExecutionContext::updateProtoDeclaration,
			                             getExecutionContext (),
			                             name,
			                             protoDeclaration);
		}
		catch (const X3D::X3DError &)
		{
			try
			{
				const auto externProtoDeclaration = getExecutionContext () -> getExternProtoDeclaration (name);
				
				undoStep -> addUndoFunction (&X3D::X3DExecutionContext::updateExternProtoDeclaration,
				                             getExecutionContext (),
				                             name,
				                             externProtoDeclaration);
			}
			catch (const X3D::X3DError &)
			{
				if (isExternProto)
				{
					undoStep -> addUndoFunction (&X3D::X3DExecutionContext::removeExternProtoDeclaration,
					                             getExecutionContext (),
					                             name);
				}
				else
				{
					undoStep -> addUndoFunction (&X3D::X3DExecutionContext::removeProtoDeclaration,
					                             getExecutionContext (),
					                             name);
				}
			}
		}

		const auto protoNode = prototypeInstance -> getProtoNode () -> copy (getExecutionContext (), X3D::COPY_OR_CLONE);
		
		if (isExternProto)
		{
			undoStep -> addRedoFunction (&X3D::X3DExecutionContext::updateExternProtoDeclaration,
			                             getExecutionContext (),
			                             name,
			                             X3D::ExternProtoDeclarationPtr (protoNode));
		}
		else
		{
			undoStep -> addRedoFunction (&X3D::X3DExecutionContext::updateProtoDeclaration,
			                             getExecutionContext (),
			                             name,
			                             X3D::ProtoDeclarationPtr (protoNode));
		}	
	}
}

MagicImport::~MagicImport ()
{
	dispose ();
}

} // puck
} // titania
