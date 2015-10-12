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

#include "LayerEditor.h"

#include "../../Browser/X3DBrowserWindow.h"
#include "../../Configuration/config.h"

#include <Titania/X3D/Execution/World.h>
#include <Titania/X3D/Components/Layering/LayerSet.h>

namespace titania {
namespace puck {

namespace Columns {

static constexpr int VISIBLE            = 0;
static constexpr int TYPE_NAME          = 1;
static constexpr int NAME               = 2;
static constexpr int WEIGHT             = 3;
static constexpr int STYLE              = 4;
static constexpr int INDEX              = 5;
static constexpr int PICKABLE           = 6;
static constexpr int EYE                = 7;
static constexpr int ACTIVE_LAYER       = 8;
static constexpr int ACTIVE_LAYER_IMAGE = 9;

};

namespace Weight {

static constexpr int NORMAL = 400;
static constexpr int BOLD   = 700;

};


LayerEditor::LayerEditor (X3DBrowserWindow* const browserWindow) :
	       X3DBaseInterface (browserWindow, browserWindow -> getBrowser ()),
	X3DLayerEditorInterface (get_ui ("Editors/LayerEditor.xml"), gconf_dir ()),
	                  world (),
	               layerSet (),
	                 layers ()
{
	setup ();
}

void
LayerEditor::initialize ()
{
	X3DLayerEditorInterface::initialize ();

	getExecutionContext () .addInterest (this, &LayerEditor::set_executionContext);

	set_executionContext ();
}

void
LayerEditor::on_index_clicked ()
{
}

void
LayerEditor::set_executionContext ()
{
	if (world)
	   world -> getLayerSet () .removeInterest (this, &LayerEditor::set_layersSet);

	world = getWorld ();

	if (world)
	{
		world -> getLayerSet () .addInterest (this, &LayerEditor::set_layersSet);

		set_layersSet ();
	}
}

void
LayerEditor::set_layersSet ()
{
	if (layerSet)
	{
		layerSet -> activeLayer () .removeInterest (this, &LayerEditor::set_layers);
		layerSet -> order ()       .removeInterest (this, &LayerEditor::set_layers);
		layerSet -> layers ()      .removeInterest (this, &LayerEditor::set_layers);
	}

	layerSet = world -> getLayerSet ();

	if (layerSet)
	{
		layerSet -> activeLayer () .addInterest (this, &LayerEditor::set_layers);
		layerSet -> order ()       .addInterest (this, &LayerEditor::set_layers);
		layerSet -> layers ()      .addInterest (this, &LayerEditor::set_layers);

		set_layers ();
	}
}

void
LayerEditor::set_layers ()
{
	for (const auto & layer : layers)
	{
	   if (layer)
	      layer -> isPickable () .removeInterest (this, &LayerEditor::set_treeView);
	}

	layers .assign (layerSet -> layers () .begin (), layerSet -> layers () .end ());

	for (const auto & layer : layers)
	{
	   if (layer)
	      layer -> isPickable () .addInterest (this, &LayerEditor::set_treeView);
	}

	set_treeView ();
}

void
LayerEditor::set_treeView ()
{
	getLayerTreeView () .unset_model ();
	getLayerListStore () -> clear ();

	const auto & order = layerSet -> order ();

	// Layer0

	const auto row         = getLayerListStore () -> append ();
	const bool visible     = std::find (order .begin (), order .end (), 0) not_eq order  .end ();
	const bool activeLayer = 0 == layerSet -> activeLayer ();

	row -> set_value (Columns::INDEX,              0);
	row -> set_value (Columns::VISIBLE,            visible);
	row -> set_value (Columns::EYE,                std::string (visible ? "EyeOpen" : "EyeClosed"));
	row -> set_value (Columns::PICKABLE,           std::string (layerSet -> getLayer0 () -> isPickable () ? "Arrow" : "gtk-stop"));
	row -> set_value (Columns::TYPE_NAME,          std::string ("Layer"));
	row -> set_value (Columns::NAME,               std::string (_ ("Default Layer")));
	row -> set_value (Columns::ACTIVE_LAYER,       activeLayer);
	row -> set_value (Columns::ACTIVE_LAYER_IMAGE, std::string (activeLayer ? "WalkViewer" : "gtk-stop"));
		
	if (0 == layerSet -> getActiveLayerIndex ())
	{
		row -> set_value (Columns::WEIGHT, Weight::BOLD);
		row -> set_value (Columns::STYLE,  Pango::STYLE_ITALIC);
	}
	else
	{
		row -> set_value (Columns::WEIGHT, Weight::NORMAL);
		row -> set_value (Columns::STYLE,  Pango::STYLE_ITALIC);
	}

	// Layers

	int32_t index = 1;

	for (const auto & node : layerSet -> layers ())
	{
	   const auto & layer       = layers [index - 1];
		const auto   row         = getLayerListStore () -> append ();
		const bool   visible     = std::find (order .begin (), order .end (), index) not_eq order .end ();
		const bool   activeLayer = index == layerSet -> activeLayer ();

		row -> set_value (Columns::INDEX,              index);
		row -> set_value (Columns::VISIBLE,            visible);
		row -> set_value (Columns::EYE,                std::string (visible ? "EyeOpen" : "EyeClosed"));
		row -> set_value (Columns::PICKABLE,           std::string (layer and layer -> isPickable () ? "Arrow" : "gtk-stop"));
		row -> set_value (Columns::TYPE_NAME,          node -> getTypeName ());
		row -> set_value (Columns::NAME,               node -> getName ());
		row -> set_value (Columns::ACTIVE_LAYER,       activeLayer);
		row -> set_value (Columns::ACTIVE_LAYER_IMAGE, std::string (activeLayer ? "WalkViewer" : "gtk-stop"));
			
		if (index == layerSet -> getActiveLayerIndex ())
		{
			row -> set_value (Columns::WEIGHT, Weight::BOLD);
			row -> set_value (Columns::STYLE,  Pango::STYLE_ITALIC);
		}
		else
		{
			row -> set_value (Columns::WEIGHT, Weight::NORMAL);
			row -> set_value (Columns::STYLE,  Pango::STYLE_NORMAL);
		}

		++ index;
	}

	getLayerTreeView () .set_model (getLayerListStore ());
}

void
LayerEditor::connectOrder ()
{
	layerSet -> order () .removeInterest (this, &LayerEditor::connectOrder);
	layerSet -> order () .addInterest (this, &LayerEditor::set_layers);
}

void
LayerEditor::connectActiveLayer ()
{
	layerSet -> activeLayer () .removeInterest (this, &LayerEditor::connectOrder);
	layerSet -> activeLayer () .addInterest (this, &LayerEditor::set_layers);
}

void
LayerEditor::connectLayers ()
{
	layerSet -> layers () .removeInterest (this, &LayerEditor::connectLayers);
	layerSet -> layers () .addInterest (this, &LayerEditor::set_layers);
}

void
LayerEditor::connectIsPickable (const X3D::X3DLayerNodePtr & layer)
{
	layer -> isPickable () .removeInterest (this, &LayerEditor::connectIsPickable);
	layer -> isPickable () .addInterest (this, &LayerEditor::set_treeView);
}

bool
LayerEditor::on_layers_button_release_event (GdkEventButton* event)
{
	Gtk::TreePath        path;
	Gtk::TreeViewColumn* column = nullptr;
	int                  cell_x = 0;
	int                  cell_y = 0;

	getLayerTreeView () .get_path_at_pos (event -> x, event -> y, path, column, cell_x, cell_y);

	const auto row = getLayerListStore () -> get_iter (path);

	if (getLayerListStore () -> iter_is_valid (row))
	{
		if (column == getPickableColumn () .operator -> ())
		{
			on_pickable_toggled (path);
			return true;
		}

		if (column == getVisibilityColumn () .operator -> ())
		{
			on_visibility_toggled (path);
			return true;
		}

		if (column == getActiveLayerColumn () .operator -> ())
		{
			on_active_layer_toggled (path);
			return true;
		}
	}

	return false;
}

void
LayerEditor::on_visibility_toggled (const Gtk::TreePath & path)
{
	// Toggle row

	const auto row     = getLayerListStore () -> get_iter (path);
	bool       visible = false;

	row -> get_value (Columns::VISIBLE, visible);

	visible = not visible;

	row -> set_value (Columns::VISIBLE, visible);
	row -> set_value (Columns::EYE,     std::string (visible ? "EyeOpen" : "EyeClosed"));

	// Set order

	const auto undoStep = std::make_shared <UndoStep> (_ ("Change Visibility Of Layer"));

	set_order (undoStep);

	getBrowserWindow () -> addUndoStep (undoStep);
}

void
LayerEditor::on_pickable_toggled (const Gtk::TreePath & path)
{
	size_t index = path .back () - 1;

	if (index >= layerSet -> layers () .size ())
	   return;

	const auto & layer = layers [index];

	if (not layer)
	   return;

	// Toggle isPickable

	layer -> isPickable () .removeInterest (this, &LayerEditor::set_treeView);
	layer -> isPickable () .addInterest (this, &LayerEditor::connectIsPickable, layer);

	const auto undoStep = std::make_shared <UndoStep> (_ ("Change Layer isPickable"));

	undoStep -> addObjects (layer);
	undoStep -> addUndoFunction (&X3D::SFBool::setValue, std::ref (layer -> isPickable ()), layer -> isPickable ());

	layer -> isPickable () = not layer -> isPickable ();

	undoStep -> addRedoFunction (&X3D::SFBool::setValue, std::ref (layer -> isPickable ()), layer -> isPickable ());
	getBrowserWindow () -> addUndoStep (undoStep);

	// Toggle row

	const auto row = getLayerListStore () -> get_iter (path);

	row -> set_value (Columns::PICKABLE, std::string (layer -> isPickable () ? "Arrow" : "gtk-stop"));
}

void
LayerEditor::on_active_layer_toggled (const Gtk::TreePath & path)
{
	size_t last  = 0;
	size_t index = path .back ();

	// Find last active layer

	for (const auto & row : getLayerListStore () -> children ())
	{
		bool active = false;

		row -> get_value (Columns::ACTIVE_LAYER, active);

		if (active)
		   break;
		
		++ last;
	}

	// Toggle last to false

	if (last >= 0)
	{
		const auto row = getLayerListStore () -> children () [last];
		row -> set_value (Columns::ACTIVE_LAYER,       false);
		row -> set_value (Columns::ACTIVE_LAYER_IMAGE, std::string ("gtk-stop"));
	}

	// Toggle current selection

	bool active = false;

	if (index not_eq last)
	{
		const auto row = getLayerListStore () -> get_iter (path);

		row -> get_value (Columns::ACTIVE_LAYER, active);

		active = not active;

		row -> set_value (Columns::ACTIVE_LAYER, active);
		row -> set_value (Columns::ACTIVE_LAYER_IMAGE, std::string (active ? "WalkViewer" : "gtk-stop"));
	}

	// Set value in LayerSet

	layerSet -> activeLayer () .removeInterest (this, &LayerEditor::set_layers);
	layerSet -> activeLayer () .addInterest (this, &LayerEditor::connectActiveLayer);

	const auto undoStep = std::make_shared <UndoStep> (_ ("Change Active Layer"));

	undoStep -> addObjects (layerSet);
	undoStep -> addUndoFunction (&X3D::SFInt32::setValue, std::ref (layerSet -> activeLayer ()), layerSet -> activeLayer ());

	layerSet -> activeLayer () = active ? index : -1;

	undoStep -> addRedoFunction (&X3D::SFInt32::setValue, std::ref (layerSet -> activeLayer ()), layerSet -> activeLayer ());
	getBrowserWindow () -> addUndoStep (undoStep);
}
void
LayerEditor::set_order (const UndoStepPtr & undoStep)
{
	// Set order

	X3D::MFInt32 order;

	for (const auto & row : getLayerListStore () -> children ())
	{
		int32_t index   = 0;
		bool    visible = false;

		row -> get_value (Columns::INDEX,   index);
		row -> get_value (Columns::VISIBLE, visible);
	   
	   if (visible)
			order .emplace_back (index);
	}

	undoStep -> addObjects (layerSet);
	undoStep -> addUndoFunction (&X3D::MFInt32::setValue, std::ref (layerSet -> order ()), layerSet -> order ());

	layerSet -> order () .removeInterest (this, &LayerEditor::set_layers);
	layerSet -> order () .addInterest (this, &LayerEditor::connectOrder);
	layerSet -> order () = std::move (order);

	undoStep -> addRedoFunction (&X3D::MFInt32::setValue, std::ref (layerSet -> order ()), layerSet -> order ());
}

void
LayerEditor::on_layer_activated (const Gtk::TreeModel::Path & path, Gtk::TreeViewColumn* column)
{
	// Deactivate layer

	const int32_t activeLayer = layerSet -> getActiveLayerIndex ();

	if (activeLayer not_eq -1)
	{
	   Gtk::TreePath path;

	   path .push_back (activeLayer);
		
		const auto row = getLayerListStore () -> get_iter (path);

		if (getLayerListStore () -> iter_is_valid (row))
		{
			row -> set_value (Columns::WEIGHT, Weight::NORMAL);
			row -> set_value (Columns::STYLE,  Pango::STYLE_NORMAL);
		}
	}

	// Activate layer

	const auto undoStep = std::make_shared <UndoStep> (_ ("Change Active Layer"));

	undoStep -> addObjects (layerSet);
	undoStep -> addUndoFunction (&X3D::LayerSet::setActiveLayerIndex, layerSet, layerSet -> getActiveLayerIndex ());

	const auto row = getLayerListStore () -> get_iter (path);

	int32_t index = 0;

	row -> get_value (Columns::INDEX, index);

	if (index not_eq layerSet -> getActiveLayerIndex ())
	{
		layerSet -> setActiveLayerIndex (index);

		row -> set_value (Columns::WEIGHT, Weight::BOLD);
		row -> set_value (Columns::STYLE,  Pango::STYLE_ITALIC);
	}
	else 
	{
		layerSet -> setActiveLayerIndex (-1);

		if (index == layerSet -> activeLayer ())
		{
			row -> set_value (Columns::WEIGHT, Weight::BOLD);
			row -> set_value (Columns::STYLE,  Pango::STYLE_ITALIC);
		}
		else
		{
			row -> set_value (Columns::WEIGHT, Weight::NORMAL);
			row -> set_value (Columns::STYLE,  Pango::STYLE_NORMAL);
		}
	}

	set_order (undoStep);

	undoStep -> addRedoFunction (&X3D::LayerSet::setActiveLayerIndex, layerSet, layerSet -> getActiveLayerIndex ());
	getBrowserWindow () -> addUndoStep (undoStep);
}

void
LayerEditor::on_layer_selection_changed ()
{
	const bool haveSelection = not getLayerSelection () -> get_selected_rows () .empty ();

	int32_t selectedIndex = -1;

	if (haveSelection)
		getLayerSelection () -> get_selected () -> get_value (Columns::INDEX, selectedIndex);
	
	const bool isFirstRow = selectedIndex == 1;
	const bool isLastRow  = selectedIndex == int32_t (getLayerListStore () -> children () .size ()) - 1;

	// Move box

	getMoveLayerBox () .set_sensitive (haveSelection and selectedIndex not_eq 0);

	getTopButton ()    .set_sensitive (not isFirstRow);
	getUpButton ()     .set_sensitive (not isFirstRow);
	getDownButton ()   .set_sensitive (not isLastRow);
	getBottomButton () .set_sensitive (not isLastRow);
}

void
LayerEditor::on_top_clicked ()
{
	const auto undoStep = std::make_shared <UndoStep> (_ ("Move Layer To Top"));

	undoStep -> addObjects (layerSet);
	undoStep -> addUndoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());

	layerSet -> layers () .removeInterest (this, &LayerEditor::set_layers);
	layerSet -> layers () .addInterest (this, &LayerEditor::connectLayers);

	// Move to top
	{
		const auto selected = getLayerSelection () -> get_selected ();

		int32_t selectedIndex = 0;

		selected -> get_value (Columns::INDEX, selectedIndex);

		if (selectedIndex == 0)
			return;

		// Move row
		{
			Gtk::TreePath destination;

			destination .push_back (1);

			getLayerListStore () -> move (selected, getLayerListStore () -> get_iter (destination));
		}

		// Update index
		{
			int32_t index = 0;

			for (const auto & row : getLayerListStore () -> children ())
			   row -> set_value (Columns::INDEX, index ++);
		}

		// Move layer
		{
			-- selectedIndex;

			auto layer = std::move (layerSet -> layers () [selectedIndex]);

			layerSet -> layers () .erase (layerSet -> layers () .begin () + selectedIndex);
			layerSet -> layers () .emplace_front (std::move (layer));
		}
	}

	set_order (undoStep);

	undoStep -> addRedoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());
	getBrowserWindow () -> addUndoStep (undoStep);

	on_layer_selection_changed ();
}

void
LayerEditor::on_up_clicked ()
{
	const auto undoStep = std::make_shared <UndoStep> (_ ("Move Layer Up"));

	undoStep -> addObjects (layerSet);
	undoStep -> addUndoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());

	layerSet -> layers () .removeInterest (this, &LayerEditor::set_layers);
	layerSet -> layers () .addInterest (this, &LayerEditor::connectLayers);

	// Move to top
	{
		const auto selected = getLayerSelection () -> get_selected ();

		int32_t selectedIndex = 0;

		selected -> get_value (Columns::INDEX, selectedIndex);

		if (selectedIndex == 0)
			return;

		if (selectedIndex == 1)
			return;

		// Move row
		{
			Gtk::TreePath destination;

			destination .push_back (selectedIndex - 1);

			getLayerListStore () -> move (selected, getLayerListStore () -> get_iter (destination));
		}

		// Update index
		{
			int32_t index = 0;

			for (const auto & row : getLayerListStore () -> children ())
			   row -> set_value (Columns::INDEX, index ++);
		}

		// Move layer
		{
			-- selectedIndex;

			auto layer = std::move (layerSet -> layers () [selectedIndex]);

			layerSet -> layers () .erase (layerSet -> layers () .begin () + selectedIndex);
			layerSet -> layers () .insert (layerSet -> layers () .begin () + (selectedIndex - 1), std::move (layer));
		}
	}

	set_order (undoStep);

	undoStep -> addRedoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());
	getBrowserWindow () -> addUndoStep (undoStep);

	on_layer_selection_changed ();
}

void
LayerEditor::on_down_clicked ()
{
	const auto undoStep = std::make_shared <UndoStep> (_ ("Move Layer Down"));

	undoStep -> addObjects (layerSet);
	undoStep -> addUndoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());

	layerSet -> layers () .removeInterest (this, &LayerEditor::set_layers);
	layerSet -> layers () .addInterest (this, &LayerEditor::connectLayers);

	// Move to top
	{
		const auto selected = getLayerSelection () -> get_selected ();
		const auto children = getLayerListStore () -> children ();

		int32_t selectedIndex = 0;

		selected -> get_value (Columns::INDEX, selectedIndex);

		if (selectedIndex == 0)
			return;

		if (selectedIndex > int32_t (children .size () - 1))
			return;

		// Move row
		{
			Gtk::TreePath destination;

			destination .push_back (selectedIndex + 2);

			if (destination .back () < int32_t (children .size ()))
				getLayerListStore () -> move (selected, getLayerListStore () -> get_iter (destination));
			else
				getLayerListStore () -> move (selected, children .end ());
		}

		// Update index
		{
			int32_t index = 0;

			for (const auto & row : getLayerListStore () -> children ())
			   row -> set_value (Columns::INDEX, index ++);
		}

		// Move layer
		{
			-- selectedIndex;

			auto layer = std::move (layerSet -> layers () [selectedIndex]);

			layerSet -> layers () .erase (layerSet -> layers () .begin () + selectedIndex);

			if (selectedIndex + 1 < int32_t (layerSet -> layers () .size ()))
				layerSet -> layers () .insert (layerSet -> layers () .begin () + (selectedIndex + 1), std::move (layer));
			else
				layerSet -> layers () .emplace_back (std::move (layer));
		}
	}

	set_order (undoStep);

	undoStep -> addRedoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());
	getBrowserWindow () -> addUndoStep (undoStep);

	on_layer_selection_changed ();
}

void
LayerEditor::on_bottom_clicked ()
{
	const auto undoStep = std::make_shared <UndoStep> (_ ("Move Layer To Bottom"));

	undoStep -> addObjects (layerSet);
	undoStep -> addUndoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());

	layerSet -> layers () .removeInterest (this, &LayerEditor::set_layers);
	layerSet -> layers () .addInterest (this, &LayerEditor::connectLayers);

	// Move to bottom
	{
		const auto selected = getLayerSelection () -> get_selected ();

		int32_t selectedIndex = 0;

		selected -> get_value (Columns::INDEX, selectedIndex);

		if (selectedIndex == 0)
			return;
		
		// Move row
		getLayerListStore () -> move (selected, getLayerListStore () -> children () .end ());
	
		// Update index
		{
			int32_t index = 0;

			for (const auto & row : getLayerListStore () -> children ())
			   row -> set_value (Columns::INDEX, index ++);
		}

		// Move layer
		{
			-- selectedIndex;

			auto layer = std::move (layerSet -> layers () [selectedIndex]);

			layerSet -> layers () .erase (layerSet -> layers () .begin () + selectedIndex);
			layerSet -> layers () .emplace_back (std::move (layer));
		}
	}

	undoStep -> addRedoFunction (&X3D::MFNode::setValue, std::ref (layerSet -> layers ()), layerSet -> layers ());
	getBrowserWindow () -> addUndoStep (undoStep);

	on_layer_selection_changed ();
}

LayerEditor::~LayerEditor ()
{
	dispose ();
}

} // puck
} // titania