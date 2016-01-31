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

#include "BackgroundEditor.h"

#include "../../Configuration/config.h"
#include "../../Dialogs/NodeIndex/NodeIndex.h"
#include "../../ComposedWidgets/TexturePreview.h"

#include <Titania/X3D/Components/EnvironmentalEffects/X3DBackgroundNode.h>

namespace titania {
namespace puck {

BackgroundEditor::BackgroundEditor (X3DBrowserWindow* const browserWindow) :
	            X3DBaseInterface (browserWindow, browserWindow -> getCurrentBrowser ()),
	X3DBackgroundEditorInterface (get_ui ("Editors/BackgroundEditor.glade"), gconf_dir ()),
	         X3DBackgroundEditor (),
	                    nodeName (this, getNameEntry (), getRenameButton ()),
	                         sky (this, "Sky Gradient", getSkyGradientBox (), "skyAngle", "skyColor"),
	                    skyColor (this,
	                              getSkyColorButton (),
	                              getSkyColorAdjustment (),
	                              getSkyColorGrid (),
	                              getAddSkyColorButton (),
	                              getRemoveSkyColorButton (),
	                              getSkyColorsScrolledWindow (),
	                              "skyColor"),
	                    skyAngle (this, getSkyAngleAdjustment (), getSkyAngleSpinButton (), "skyAngle"),
	                      ground (this, "Ground Gradient", getGroundGradientBox (), "groundAngle", "groundColor"),
	                 groundColor (this,
	                              getGroundColorButton (),
	                              getGroundColorAdjustment (),
	                              getGroundColorGrid (),
	                              getAddGroundColorButton (),
	                              getRemoveGroundColorButton (),
	                              getGroundColorsScrolledWindow (),
	                              "groundColor"),
	                 groundAngle (this, getGroundAngleAdjustment (), getGroundAngleSpinButton (), "groundAngle"),
	                transparency (this, getTransparencyAdjustment (), getTransparencyScale (), "transparency"),
	                frontPreview (new TexturePreview (this, getFrontTexturePreviewBox (), getFrontTextureFormatLabel ())),
	                 backPreview (new TexturePreview (this, getBackTexturePreviewBox (), getBackTextureFormatLabel ())),
	                 leftPreview (new TexturePreview (this, getLeftTexturePreviewBox (), getLeftTextureFormatLabel ())),
	                rightPreview (new TexturePreview (this, getRightTexturePreviewBox (), getRightTextureFormatLabel ())),
	                  topPreview (new TexturePreview (this, getTopTexturePreviewBox (), getTopTextureFormatLabel ())),
	               bottomPreview (new TexturePreview (this, getBottomTexturePreviewBox (), getBottomTextureFormatLabel ())),
	              backgroundNode (),
	                    changing (false)
{
	getNotebook ()         .set_current_page (getConfig () .getInteger ("currentPage"));
	getTexturesNotebook () .set_current_page (getConfig () .getInteger ("texturePage"));

	sky      .signal_whichChoice_changed () .connect (sigc::mem_fun (this, &BackgroundEditor::on_sky_whichChoice_changed)); 
	skyColor .signal_index_changed ()       .connect (sigc::mem_fun (this, &BackgroundEditor::on_sky_color_index_changed)); 

	ground      .signal_whichChoice_changed () .connect (sigc::mem_fun (this, &BackgroundEditor::on_ground_whichChoice_changed)); 
	groundColor .signal_index_changed ()       .connect (sigc::mem_fun (this, &BackgroundEditor::on_ground_color_index_changed)); 
	
	sky    .addClass ("notebook");
	ground .addClass ("notebook");
	
	skyColor    .setColorsSize (16);
	groundColor .setColorsSize (16);

	setup ();
}

void
BackgroundEditor::initialize ()
{
	X3DBackgroundEditorInterface::initialize ();
	X3DBackgroundEditor::initialize ();

	getBrowserWindow () -> getSelection () -> getChildren () .addInterest (this, &BackgroundEditor::set_selection);

	set_selection (getBrowserWindow () -> getSelection () -> getChildren ());
}

void
BackgroundEditor::set_selection (const X3D::MFNode & selection)
{
	if (backgroundNode)
	{
		backgroundNode -> getFrontTexture ()  .removeInterest (this, &BackgroundEditor::set_texture);
		backgroundNode -> getBackTexture ()   .removeInterest (this, &BackgroundEditor::set_texture);
		backgroundNode -> getLeftTexture ()   .removeInterest (this, &BackgroundEditor::set_texture);
		backgroundNode -> getRightTexture ()  .removeInterest (this, &BackgroundEditor::set_texture);
		backgroundNode -> getTopTexture ()    .removeInterest (this, &BackgroundEditor::set_texture);
		backgroundNode -> getBottomTexture () .removeInterest (this, &BackgroundEditor::set_texture);

		backgroundNode -> isBound () .removeInterest (this, &BackgroundEditor::set_bind);
	}

	backgroundNode   = selection .empty () ? nullptr : selection .back ();
	const auto nodes = backgroundNode ? X3D::MFNode ({ backgroundNode }) : X3D::MFNode ();

	setBackground (backgroundNode);

	nodeName .setNode (X3D::SFNode (backgroundNode));
	sky          .setNodes (nodes);
	skyColor     .setNodes (nodes);
	skyAngle     .setNodes (nodes);
	ground       .setNodes (nodes);
	groundColor  .setNodes (nodes);
	groundAngle  .setNodes (nodes);
	transparency .setNodes (nodes);

	getRemoveBackgroundButton () .set_sensitive (backgroundNode);
	getBindToggleButton ()        .set_sensitive (backgroundNode);

	if (backgroundNode)
	{
		backgroundNode -> getFrontTexture ()  .addInterest (this, &BackgroundEditor::set_texture, frontPreview,  std::cref (backgroundNode -> getFrontTexture ()));
		backgroundNode -> getBackTexture ()   .addInterest (this, &BackgroundEditor::set_texture, backPreview,   std::cref (backgroundNode -> getBackTexture ()));
		backgroundNode -> getLeftTexture ()   .addInterest (this, &BackgroundEditor::set_texture, leftPreview,   std::cref (backgroundNode -> getLeftTexture ()));
		backgroundNode -> getRightTexture ()  .addInterest (this, &BackgroundEditor::set_texture, rightPreview,  std::cref (backgroundNode -> getRightTexture ()));
		backgroundNode -> getTopTexture ()    .addInterest (this, &BackgroundEditor::set_texture, topPreview,    std::cref (backgroundNode -> getTopTexture ()));
		backgroundNode -> getBottomTexture () .addInterest (this, &BackgroundEditor::set_texture, bottomPreview, std::cref (backgroundNode -> getBottomTexture ()));

		backgroundNode -> isBound () .addInterest (this, &BackgroundEditor::set_bind);

		set_bind ();

		set_texture (frontPreview,  backgroundNode -> getFrontTexture ());
		set_texture (backPreview,   backgroundNode -> getBackTexture ());
		set_texture (leftPreview,   backgroundNode -> getLeftTexture ());
		set_texture (rightPreview , backgroundNode -> getRightTexture ());
		set_texture (topPreview,    backgroundNode -> getTopTexture ());
		set_texture (bottomPreview, backgroundNode -> getBottomTexture ());
	}
	else
	{
		set_bind ();

		set_texture (frontPreview,  nullptr);
		set_texture (backPreview,   nullptr);
		set_texture (leftPreview,   nullptr);
		set_texture (rightPreview , nullptr);
		set_texture (topPreview,    nullptr);
		set_texture (bottomPreview, nullptr);
	}
}

void
BackgroundEditor::set_texture (const std::shared_ptr <TexturePreview> & preview, const X3D::X3DPtr <X3D::X3DTextureNode> & texture)
{
__LOG__ << X3D::SFTime (chrono::now ()) << std::endl;

	preview -> setTexture (texture);
}

void
BackgroundEditor::on_sky_whichChoice_changed ()
{
	if (changing)
		return;

	changing = true;

	skyColor .setIndex (sky .getWhichChoice ());
	skyAngle .setIndex (sky .getWhichChoice () - 1);

	changing = false;
}

void
BackgroundEditor::on_sky_color_index_changed ()
{
	//__LOG__ << skyColor .getIndex () << std::endl;

	if (changing)
		return;

	changing = true;

	sky      .setWhichChoice (skyColor .getIndex ());
	skyAngle .setIndex       (skyColor .getIndex () - 1);

	changing = false;
}

void
BackgroundEditor::on_ground_whichChoice_changed ()
{
	//__LOG__ << ground .getWhichChoice () << std::endl;

	if (changing)
		return;

	changing = true;

	groundColor .setIndex (ground .getWhichChoice ());
	groundAngle .setIndex (ground .getWhichChoice () - 1);

	changing = false;
}

void
BackgroundEditor::on_ground_color_index_changed ()
{
	if (changing)
		return;

	changing = true;

	ground      .setWhichChoice (groundColor .getIndex ());
	groundAngle .setIndex       (groundColor .getIndex () - 1);

	changing = false;
}

BackgroundEditor::~BackgroundEditor ()
{
	getConfig () .setItem ("currentPage", getNotebook ()         .get_current_page ());
	getConfig () .setItem ("texturePage", getTexturesNotebook () .get_current_page ());

	dispose ();
}

/*
 * Action buttons
 */

void
BackgroundEditor::on_new_background_activated ()
{
	const auto undoStep = std::make_shared <X3D::UndoStep> (_ ("Create New Background"));
	const X3D::X3DPtr <X3D::X3DBackgroundNode> node (getBrowserWindow () -> createNode ("Background", undoStep));
	node -> set_bind () = true;
	getBrowserWindow () -> addUndoStep (undoStep);
}

void
BackgroundEditor::on_new_texture_background_activated ()
{
	const auto undoStep = std::make_shared <X3D::UndoStep> (_ ("Create New TextureBackground"));
	const X3D::X3DPtr <X3D::X3DBackgroundNode> node (getBrowserWindow () -> createNode ("TextureBackground", undoStep));
	node -> set_bind () = true;
	getBrowserWindow () -> addUndoStep (undoStep);
}

void
BackgroundEditor::on_remove_background_clicked ()
{
	const auto undoStep = std::make_shared <X3D::UndoStep> (_ ("Remove NavigationInfo"));

	getBrowserWindow () -> removeNodesFromScene (getCurrentContext (), { nodeName .getNode () }, true, undoStep);
	getBrowserWindow () -> addUndoStep (undoStep);
}

void
BackgroundEditor::on_bind_toggled ()
{
	if (changing)
		return;

	if (backgroundNode)
		backgroundNode -> set_bind () = not backgroundNode -> isBound ();
}

void
BackgroundEditor::set_bind ()
{
	changing = true;

	if (backgroundNode)
	{
		getBindToggleButton () .set_active (backgroundNode -> isBound ());
		getBindImage () .set (Gtk::StockID (backgroundNode -> isBound () ? "Bound" : "Bind"), Gtk::IconSize (Gtk::ICON_SIZE_BUTTON));
	}
	else
	{
		getBindToggleButton () .set_active (false);
		getBindImage () .set (Gtk::StockID ("Bind"), Gtk::IconSize (Gtk::ICON_SIZE_BUTTON));
	}

	changing = false;
}

void
BackgroundEditor::on_index_clicked ()
{
	const auto nodeIndex = std::dynamic_pointer_cast <NodeIndex> (getBrowserWindow () -> addDialog ("NodeIndex"));

	nodeIndex -> setTypes ({ X3D::X3DConstants::Background, X3D::X3DConstants::TextureBackground });
}

} // puck
} // titania