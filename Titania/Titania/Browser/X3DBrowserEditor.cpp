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

#include "BrowserWindow.h"

#include "../Browser/BrowserSelection.h"
#include "../Browser/MagicImport.h"
#include "../OutlineEditor/OutlineTreeViewEditor.h"

#include <Titania/OS.h>

namespace titania {
namespace puck {

X3DBrowserEditor::X3DBrowserEditor (int argc, char** argv) :
	X3DBrowserWidget (argc, argv),
	        modified (false),
	   saveConfirmed (false),
	    currentScene (),
	       selection (new BrowserSelection (getBrowserWindow ())),
	     magicImport (new MagicImport (getBrowserWindow ())),
	     undoHistory (),
	    undoMatrices (),
	    fileMonitors ()
{ }

void
X3DBrowserEditor::initialize ()
{
	X3DBrowserWidget::initialize ();

	getBrowser () -> initialized () .addInterest (this, &X3DBrowserEditor::set_initialized);
	getBrowser () -> shutdown ()    .addInterest (this, &X3DBrowserEditor::set_shutdown);

	getBrowser () -> getSelection () -> isActive () .addInterest (this, &X3DBrowserEditor::set_selection_active);

	undoHistory .changed () .addInterest (this, &X3DBrowserEditor::set_undoHistory);

	set_undoHistory ();
}

void
X3DBrowserEditor::restoreSession ()
{
	// ImportAsInline
	if (getConfig () .hasItem ("importAsInline"))
		getImportAsInlineMenuItem () .set_active (getConfig () .getBoolean ("importAsInline"));

	// Workspace
	if (getConfig () .getString ("workspace") == "BROWSER")
		getBrowserMenuItem () .set_active (true);

	else if (getConfig () .getString ("workspace") == "EDITOR")
		getEditorMenuItem () .set_active (true);

	// SelectLowest
	if (getConfig () .hasItem ("selectLowest"))
		getSelectLowestMenuItem () .set_active (getConfig () .getBoolean ("selectLowest"));

	// FollowPrimarySelection
	if (getConfig () .hasItem ("followPrimarySelection"))
		getFollowPrimarySelectionMenuItem () .set_active (getConfig () .getBoolean ("followPrimarySelection"));

	// This must be done after.
	X3DBrowserWidget::restoreSession ();
}

void
X3DBrowserEditor::set_initialized ()
{
	if (getBrowser () -> getExecutionContext () not_eq currentScene)
		isModified (false);

	currentScene = getBrowser () -> getExecutionContext ();
}

void
X3DBrowserEditor::set_shutdown ()
{
	try
	{
		if (isSaved ())
			shutdown ();

		else
			getBrowser () -> replaceWorld (getBrowser () -> getExecutionContext ());                                                                                     // Cancel shutdown

	}
	catch (const X3D::X3DError &)
	{ }
}

void
X3DBrowserEditor::set_selection_active (const bool value)
{
	if (value)
	{
		for (const auto & child : getBrowser () -> getSelection () -> getChildren ())
		{
			const X3D::X3DTransformNodePtr transform (child);

			if (transform)
				undoMatrices [transform] = std::make_pair (transform -> getMatrix (), transform -> center () .getValue ());
		}
	}
	else
	{
		bool changed = false;

		const auto undoStep = std::make_shared <UndoStep> (_ ("Edit Transform"));

		getSelection () -> redoRestoreSelection (undoStep);

		for (const auto & child : getBrowser () -> getSelection () -> getChildren ())
		{
			const X3D::X3DTransformNodePtr transform (child);

			if (transform)
			{
				try
				{
					const X3D::Matrix4f matrix = undoMatrices .at (transform) .first;
					const X3D::Vector3f center = undoMatrices .at (transform) .second;

					if (matrix not_eq transform -> getMatrix () or center not_eq transform -> center ())
					{
						changed = true;

						undoStep -> addUndoFunction (&X3D::X3DTransformNode::setMatrixWithCenter,
						                             transform,
						                             matrix,
						                             center);

						undoStep -> addRedoFunction (&X3D::X3DTransformNode::setMatrixWithCenter,
						                             transform,
						                             transform -> getMatrix (),
						                             transform -> center ());
					}
				}
				catch (const std::out_of_range &)
				{ }
			}
		}

		undoMatrices .clear ();

		getSelection () -> undoRestoreSelection (undoStep);

		if (changed)
			addUndoStep (undoStep);
	}
}

bool
X3DBrowserEditor::isSaved ()
{
	if (saveConfirmed)
		return true;

	if (isModified ())
	{
		getBrowserWindow () -> getKeys () .clear ();

		const auto response_id = getFileSaveWarningDialog () .run ();

		getFileSaveWarningDialog () .hide ();

		switch (response_id)
		{
			case Gtk::RESPONSE_OK:
			{
				on_save ();
				saveConfirmed = not isModified ();
				return saveConfirmed;
			}
			case Gtk::RESPONSE_CANCEL:
			case Gtk::RESPONSE_DELETE_EVENT:
			{
				return false;
			}
			default:
				break;
		}
	}

	saveConfirmed = true;
	return true;
}

void
X3DBrowserEditor::isModified (const bool value)
{
	modified      = value;
	saveConfirmed = false;

	updateTitle (value);

	if (not value)
		undoHistory .setSaved ();
}

// File operations

void
X3DBrowserEditor::import (const std::vector <basic::uri> & uris, const bool importAsInline)
{
	try
	{
		if (importAsInline)
		{
			// Imported As Inline

			const auto undoStep = std::make_shared <UndoStep> (_ ("Import As Inline"));

			getSelection () -> clear (undoStep);

			for (const auto & worldURL : uris)
			{
				const auto relativePath = getBrowser () -> getExecutionContext () -> getWorldURL () .relative_path (worldURL);

				std::string string;

				string += "DEF " + X3D::get_name_from_uri (worldURL) + " Transform {";
				string += "  children Inline {";
				string += "    url [";
				string += "      \"" + relativePath + "\"";
				string += "      \"" + worldURL + "\"";
				string += "    ]";
				string += "  }";
				string += "}";

				const auto scene = getBrowser () -> createX3DFromString (string);

				importScene (scene, undoStep);
			}

			addUndoStep (undoStep);
		}
		else
		{
			// Imported file

			const auto undoStep  = std::make_shared <UndoStep> (_ ("Import"));
			auto       selection = getBrowser () -> getSelection () -> getChildren ();

			getSelection () -> clear (undoStep);

			for (const auto & worldURL : uris)
			{
				const auto scene = getBrowser () -> createX3DFromURL ({ worldURL .str () });

				if (magicImport -> import (selection, scene, undoStep))
					continue;

				importScene (scene, undoStep);
			}

			addUndoStep (undoStep);
		}
	}
	catch (const X3D::X3DError & error)
	{
		std::clog << error .what () << std::endl;
	}
}

void
X3DBrowserEditor::importScene (const X3D::ScenePtr & scene, const UndoStepPtr & undoStep)
{
	try
	{
		const auto & rootNodes    = getBrowser () -> getExecutionContext () -> getRootNodes ();
		const size_t numRootNodes = rootNodes .size ();

		// Imported scene

		getBrowser () -> getExecutionContext () -> import (scene);

		X3D::MFNode importedNodes (rootNodes .begin () + numRootNodes, rootNodes .end ());

		const auto undoRemoveNodes = std::make_shared <UndoStep> ();

		removeNodes (importedNodes, undoRemoveNodes);

		undoStep -> addUndoFunction (&UndoStep::redoChanges, undoRemoveNodes);
		undoStep -> addRedoFunction (&UndoStep::undoChanges, undoRemoveNodes);

		undoRemoveNodes -> undoChanges ();

		// Select imported nodes

		getSelection () -> addChildren (importedNodes, undoStep);
	}
	catch (const X3D::X3DError & error)
	{
		std::clog << error .what () << std::endl;
	}
}

void
X3DBrowserEditor::save (const basic::uri & worldURL, const bool compressed)
{
	X3DBrowserWidget::save (worldURL, compressed);

	isModified (false);
}

bool
X3DBrowserEditor::close ()
{
	getWidget () .grab_focus ();

	if (isSaved ())
	{
		shutdown ();

		return X3DBrowserWidget::close ();
	}

	return true;
}

// Undo/Redo operations

void
X3DBrowserEditor::addUndoStep (const UndoStepPtr & undoStep)
{
	undoHistory .addUndoStep (undoStep);
}

void
X3DBrowserEditor::undo ()
{
	getBrowser () -> grab_focus ();

	undoHistory .undoChanges ();
}

void
X3DBrowserEditor::redo ()
{
	getBrowser () -> grab_focus ();

	undoHistory .redoChanges ();
}

void
X3DBrowserEditor::set_undoHistory ()
{
	const int index = undoHistory .getIndex ();

	if (index >= 0)
	{
		getUndoMenuItem () .set_label (undoHistory .getUndoDescription ());
		getUndoMenuItem () .set_sensitive (true);
		getUndoButton ()   .set_sensitive (true);
	}
	else
	{
		getUndoMenuItem () .set_label (_ ("Undo"));
		getUndoMenuItem () .set_sensitive (false);
		getUndoButton ()   .set_sensitive (false);
	}

	if (index + 1 < (int) undoHistory .getSize ())
	{
		getRedoMenuItem () .set_label (undoHistory .getRedoDescription ());
		getRedoMenuItem () .set_sensitive (true);
		getRedoButton ()   .set_sensitive (true);
	}
	else
	{
		getRedoMenuItem () .set_label (_ ("Redo"));
		getRedoMenuItem () .set_sensitive (false);
		getRedoButton ()   .set_sensitive (false);
	}

	isModified (undoHistory .isModified ());
}

// Clipboard operations

void
X3DBrowserEditor::cutNodes (X3D::MFNode nodes, const UndoStepPtr & undoStep) const
{
	// Detach from group

	detachFromGroup (nodes, true, undoStep);

	// Set clipboard text

	Gtk::Clipboard::get () -> set_text (toString (nodes));

	// Remove nodes

	removeNodes (nodes, undoStep);
}

void
X3DBrowserEditor::copyNodes (X3D::MFNode nodes) const
{
	// Detach from group

	const auto undoDetachFromGroup = std::make_shared <UndoStep> ();

	detachFromGroup (nodes, true, undoDetachFromGroup);

	// Set clipboard text

	Gtk::Clipboard::get () -> set_text (toString (nodes));

	// Undo detach from group

	undoDetachFromGroup -> undoChanges ();
}

std::string
X3DBrowserEditor::toString (X3D::MFNode & nodes) const
{
	// Find proto declarations

	std::set <X3D::X3DProtoObjectPtr> protoDeclarations;

	X3D::traverse (nodes, [&protoDeclarations] (X3D::SFNode & node)
	               {
	                  const auto protoInstance = dynamic_cast <X3D::X3DPrototypeInstance*> (node .getValue ());

	                  if (protoInstance)
								protoDeclarations .emplace (protoInstance -> getProtoDeclaration ());

	                  return true;
						});

	// Create node index

	std::set <X3D::SFNode> nodeIndex;

	X3D::traverse (nodes, [&nodeIndex] (X3D::SFNode & node)
	               {
	                  nodeIndex .emplace (node);
	                  return true;
						});

	// Find connected routes

	std::vector <X3D::Route*> routes;

	X3D::traverse (nodes, [this, &nodeIndex, &routes] (X3D::SFNode & node)
	               {
	                  for (const auto & field: node -> getFieldDefinitions ())
	                  {
	                     for (const auto & route: field -> getOutputRoutes ())
	                     {
	                        if (route -> getExecutionContext () == getBrowser () -> getExecutionContext ())
	                        {
	                           try
	                           {
	                              if (nodeIndex .find (route -> getDestinationNode ()) not_eq nodeIndex .end ())
												routes .emplace_back (route);
										}
	                           catch (const X3D::X3DError &)
	                           { }
									}
								}
							}

	                  return true;
						});

	// Generate text

	std::ostringstream text;

	text .imbue (std::locale::classic ());

	text
		<< "#X3D V3.3 utf8 Titania"
		<< std::endl
		<< std::endl
		<< '#' << getBrowser () -> getExecutionContext () -> getWorldURL ()
		<< std::endl
		<< std::endl;

	X3D::Generator::CompactStyle ();
	X3D::Generator::PushContext ();

	if (not protoDeclarations .empty ())
	{
		for (const auto & protoDeclaration : protoDeclarations)
			text << protoDeclaration << std::endl;

		text << std::endl;
	}

	for (const auto & node : nodes)
		text << node << std::endl;

	if (not routes .empty ())
	{
		text << std::endl;

		for (const auto & route : routes)
			text << *route << std::endl;
	}

	X3D::Generator::PopContext ();

	return text .str ();
}

void
X3DBrowserEditor::pasteNodes (const X3D::MFNode & nodes, const UndoStepPtr & undoStep)
{
	try
	{
		const Glib::RefPtr <Gtk::Clipboard> clipboard = Gtk::Clipboard::get ();

		if (clipboard -> wait_is_text_available ())
		{
			basic::ifilestream text (clipboard -> wait_for_text ());

			text .imbue (std::locale::classic ());

			std::string header;

			if (X3D::Grammar::comment (text, header))
			{
				std::string encoding, specificationVersion, characterEncoding, comment;

				if (X3D::Grammar::Header .FullMatch (header, &encoding, &specificationVersion, &characterEncoding, &comment))
				{
					std::string whitespaces;
					X3D::Grammar::whitespaces (text, whitespaces);

					std::string worldURL;

					if (X3D::Grammar::comment (text, worldURL))
					{
						const auto scene = getBrowser () -> createX3DFromStream (worldURL, text);

						getSelection () -> clear (undoStep);

						importScene (scene, undoStep);
					}
				}
			}
		}
	}
	catch (const X3D::X3DError & error)
	{
		std::clog << error .what () << std::endl;
	}
}

void
X3DBrowserEditor::updatePasteStatus ()
{
	getPasteMenuItem () .set_sensitive (getPasteStatus ());
}

bool
X3DBrowserEditor::getPasteStatus () const
{
	const Glib::RefPtr <Gtk::Clipboard> clipboard = Gtk::Clipboard::get ();

	if (clipboard -> wait_is_text_available ())
	{
		std::istringstream text (clipboard -> wait_for_text ());

		text .imbue (std::locale::classic ());

		std::string header;

		if (X3D::Grammar::comment (text, header))
		{
			std::string encoding, specificationVersion, characterEncoding, comment;

			if (X3D::Grammar::Header .FullMatch (header, &encoding, &specificationVersion, &characterEncoding, &comment))
				return true;
		}
	}

	return false;
}

// Edit operations

void
X3DBrowserEditor::replaceNode (const X3D::SFNode & parent, X3D::SFNode & node, const X3D::SFNode & newValue, const UndoStepPtr & undoStep) const
{
	if (node)
	{
		if (node -> getNumClones () == 1)
			removeNodes ({ node }, undoStep);
	}

	undoStep -> addVariables (parent);
	undoStep -> addUndoFunction (&X3D::SFNode::setValue, std::ref (node), node);
	undoStep -> addRedoFunction (&X3D::SFNode::setValue, std::ref (node), newValue);

	node = newValue;
}

void
X3DBrowserEditor::replaceNode (const X3D::SFNode & parent, X3D::MFNode & mfnode, const X3D::SFNode & node, const X3D::SFNode & newValue, const UndoStepPtr & undoStep) const
{
	const auto indices = mfnode .indices_of (node);

	if (indices .empty ())
		return;

	for (const auto & index : indices)
		replaceNode (parent, mfnode, index, newValue, undoStep);
}

void
X3DBrowserEditor::replaceNode (const X3D::SFNode & parent, X3D::MFNode & mfnode, const size_t index, const X3D::SFNode & newValue, const UndoStepPtr & undoStep) const
{
	undoStep -> addVariables (parent);

	const X3D::SFNode oldValue = mfnode [index];
	bool              remove   = false;

	if (oldValue)
		remove = (oldValue -> getNumClones () == 1);

	using set1Value = void (X3D::MFNode::*) (const X3D::MFNode::size_type, const X3D::SFNode &);

	undoStep -> addUndoFunction ((set1Value) & X3D::MFNode::set1Value, std::ref (mfnode), index, mfnode [index]);
	undoStep -> addRedoFunction ((set1Value) & X3D::MFNode::set1Value, std::ref (mfnode), index, newValue);

	mfnode [index] = newValue;

	if (remove)
		removeNodes ({ oldValue }, undoStep);
}

void
X3DBrowserEditor::removeNodes (const X3D::MFNode & nodes, const UndoStepPtr & undoStep) const
{
	for (const auto & node : nodes)
	{
		if (node)
			removeNodeFromScene (getBrowser () -> getExecutionContext (), node, undoStep);
	}
}

void
X3DBrowserEditor::removeNodeFromScene (const X3D::X3DExecutionContextPtr & executionContext, X3D::SFNode node, const UndoStepPtr & undoStep) const
{
	// Remove exported nodes

	X3D::ScenePtr scene (executionContext);

	if (scene)
		removeExportedNodes (scene, node, undoStep);

	removeNodeFromExecutionContext (executionContext, node, undoStep);

	// Delete children of node if not in scene

	std::set <X3D::SFNode> children;

	// Collect children

	X3D::traverse (node, [&children] (X3D::SFNode & child)
	               {
	                  children .emplace (child);
	                  return true;
						});

	children .erase (node);

	// Filter out scene nodes

	X3D::traverse (executionContext -> getRootNodes (), [&children] (X3D::SFNode & node)
	               {
	                  // If executionContext node is in children, remove from children.
	                  children .erase (node);
	                  return true;
						});

	// Remove rest, these are only nodes that are not in executionContext

	for (const auto & child : children)
	{
		removeNamedNode (executionContext, child, undoStep);

		if (scene)
			removeExportedNodes (scene, child, undoStep);

		removeImportedNodes (executionContext, X3D::InlinePtr (child), undoStep);
		deleteRoutes (executionContext, child, undoStep);

		// Hide node

		undoStep -> addUndoFunction (&X3D::X3DBaseNode::restoreState, child);
		undoStep -> addRedoFunction (&X3D::X3DBaseNode::saveState,    child);
		child -> saveState ();

		using isInternal = void (X3D::X3DBaseNode::*) (const bool);

		undoStep -> addUndoFunction ((isInternal) & X3D::X3DBaseNode::isInternal, child, false);
		undoStep -> addRedoFunction ((isInternal) & X3D::X3DBaseNode::isInternal, child, true);
		child -> isInternal (true);
	}
}

void
X3DBrowserEditor::removeExportedNodes (const X3D::ScenePtr & scene, const X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	// Remove exported nodes

	for (const auto & pair : X3D::ExportedNodeIndex (scene -> getExportedNodes ()))
	{
		try
		{
			const auto & exportedNode = pair .second;
			const auto   localNode    = exportedNode -> getLocalNode ();

			if (node == localNode)
			{
				undoStep -> addUndoFunction (&X3D::Scene::updateExportedNode, scene, exportedNode -> getExportedName (), localNode);
				undoStep -> addRedoFunction (&X3D::Scene::removeExportedNode, scene, exportedNode -> getExportedName ());

				scene -> removeExportedNode (exportedNode -> getExportedName ());
			}
		}
		catch (...)
		{ }
	}
}

void
X3DBrowserEditor::removeNodeFromExecutionContext (X3D::X3DExecutionContext* const executionContext, X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	// Remove node from scene graph

	removeNamedNode (executionContext, node, undoStep);
	removeImportedNodes (executionContext, X3D::InlinePtr (node), undoStep);
	deleteRoutes (executionContext, node, undoStep);
	removeNodeFromSceneGraph (executionContext, node, undoStep);

	// Remove unused Prototypes

	removePrototypes (executionContext, node, undoStep);

	// Hide node

	undoStep -> addUndoFunction (&X3D::X3DBaseNode::restoreState, node);
	undoStep -> addRedoFunction (&X3D::X3DBaseNode::saveState,    node);
	node -> saveState ();

	using isInternal = void (X3D::X3DBaseNode::*) (const bool);

	undoStep -> addUndoFunction ((isInternal) & X3D::X3DBaseNode::isInternal, node, false);
	undoStep -> addRedoFunction ((isInternal) & X3D::X3DBaseNode::isInternal, node, true);
	node -> isInternal (true);
}

void
X3DBrowserEditor::removeNodeFromSceneGraph (X3D::X3DExecutionContext* const executionContext, const X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	removeNodeFromMFNode (executionContext -> getRootNodes (), node, executionContext, undoStep);

	// Remove node from scene graph

	X3D::traverse (executionContext -> getRootNodes (), [this, &node, &undoStep] (X3D::SFNode & parent)
	               {
	                  for (auto & field: parent -> getFieldDefinitions ())
	                  {
	                     switch (field -> getType ())
	                     {
									case X3D::X3DConstants::SFNode:
										{
										   const auto sfnode = static_cast <X3D::SFNode*> (field);

										   if (*sfnode == node)
										   {
										      undoStep -> addVariables (parent);

										      undoStep -> addUndoFunction (&X3D::SFNode::setValue, sfnode, node);
										      undoStep -> addRedoFunction (&X3D::SFNode::setValue, sfnode, nullptr);

										      sfnode -> setValue (nullptr);
											}

										   break;
										}
									case X3D::X3DConstants::MFNode:
										{
										   const auto mfnode = static_cast <X3D::MFNode*> (field);

										   removeNodeFromMFNode (*mfnode, node, parent, undoStep);

										   break;
										}
									default:
										break;
								}
							}

	                  return true;
						});
}

void
X3DBrowserEditor::removeNodeFromMFNode (X3D::MFNode & mfnode, const X3D::SFNode & node, const X3D::SFNode & parent, const UndoStepPtr & undoStep) const
{
	const auto indices = mfnode .indices_of (node);

	if (indices .empty ())
		return;

	undoStep -> addVariables (parent);

	undoStep -> addUndoFunction (&X3DBrowserEditor::undoEraseNode, this,
	                             std::ref (mfnode),
	                             node,
	                             indices);

	undoStep -> addRedoFunction (&X3D::MFNode::remove, std::ref (mfnode), node);

	mfnode .remove (node);
}

void
X3DBrowserEditor::removeNamedNode (X3D::X3DExecutionContext* const executionContext, const X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	// Remove named node

	try
	{
		if (executionContext -> getNamedNode (node -> getName ()) == node)
		{
			undoStep -> addUndoFunction (&X3D::X3DExecutionContext::updateNamedNode, executionContext, node -> getName (), node);
			undoStep -> addRedoFunction (&X3D::X3DExecutionContext::removeNamedNode, executionContext, node -> getName ());

			executionContext -> removeNamedNode (node -> getName ());
		}
	}
	catch (const X3D::X3DError &)
	{ }
}

void
X3DBrowserEditor::removeImportedNodes (X3D::X3DExecutionContext* const executionContext, const X3D::InlinePtr & inlineNode, const UndoStepPtr & undoStep) const
{
	// Remove nodes imported from node

	if (inlineNode)
	{
		for (const auto & pair : X3D::ImportedNodeIndex (executionContext -> getImportedNodes ()))
		{
			try
			{
				const auto & importedNode = pair .second;

				if (inlineNode == importedNode -> getInlineNode ())
				{
					// Delete routes.

					try
					{
						deleteRoutes (executionContext, importedNode -> getExportedNode (), undoStep);
					}
					catch (...)
					{ }

					// Remove imported node.

					undoStep -> addUndoFunction (&X3D::X3DExecutionContext::addImportedNode, executionContext,
					                             importedNode -> getInlineNode (),
					                             importedNode -> getExportedName (),
					                             importedNode -> getImportedName ());

					undoStep -> addRedoFunction (&X3D::X3DExecutionContext::removeImportedNode, executionContext,
					                             importedNode -> getImportedName ());

					executionContext -> removeImportedNode (importedNode -> getImportedName ());
				}
			}
			catch (...)
			{ }
		}
	}
}

void
X3DBrowserEditor::deleteRoutes (X3D::X3DExecutionContext* const executionContext, const X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	// Delete routes from and to node

	for (const auto & field : node -> getFieldDefinitions ())
	{
		for (const auto & route : X3D::RouteSet (field -> getInputRoutes ()))
		{
			try
			{
				deleteRoute (executionContext,
				             route -> getSourceNode (),
				             route -> getSourceField (),
				             route -> getDestinationNode (),
				             route -> getDestinationField (),
				             undoStep);
			}
			catch (const X3D::X3DError &)
			{ }
		}

		for (const auto & route : X3D::RouteSet (field -> getOutputRoutes ()))
		{
			try
			{
				deleteRoute (executionContext,
				             route -> getSourceNode (),
				             route -> getSourceField (),
				             route -> getDestinationNode (),
				             route -> getDestinationField (),
				             undoStep);
			}
			catch (const X3D::X3DError &)
			{ }
		}
	}
}

void
X3DBrowserEditor::addRoute (X3D::X3DExecutionContext* const executionContext,
                            const X3D::SFNode & sourceNode,
                            const std::string & sourceField,
                            const X3D::SFNode & destinationNode,
                            const std::string & destinationField,
                            const UndoStepPtr & undoStep) const
throw (X3D::Error <X3D::INVALID_NODE>,
       X3D::Error <X3D::INVALID_FIELD>,
       X3D::Error <X3D::INVALID_OPERATION_TIMING>,
       X3D::Error <X3D::DISPOSED>)
{
	using deleteRoute = void (X3D::X3DExecutionContext::*) (const X3D::SFNode &, const std::string &, const X3D::SFNode &, const std::string &);

	bool sourceImported      = false;
	bool destinationImported = false;

	try
	{
		sourceImported = executionContext -> isImportedNode (sourceNode);
	}
	catch (...)
	{ }

	try
	{
		destinationImported = executionContext -> isImportedNode (destinationNode);
	}
	catch (...)
	{ }

	if (sourceImported and destinationImported)
	{
		undoStep -> addUndoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);

		undoStep -> addRedoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);
	}
	else if (sourceImported and not destinationImported)
	{
		undoStep -> addUndoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             destinationNode,
		                             destinationField);

		undoStep -> addRedoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             destinationNode,
		                             destinationField);
	}
	else if (destinationImported and not sourceImported)
	{
		undoStep -> addUndoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);

		undoStep -> addRedoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);
	}
	else
	{
		undoStep -> addUndoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             destinationNode,
		                             destinationField);

		undoStep -> addRedoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             destinationNode,
		                             destinationField);
	}

	executionContext -> addRoute (sourceNode, sourceField, destinationNode, destinationField);
}

void
X3DBrowserEditor::deleteRoute (X3D::X3DExecutionContext* const executionContext,
                               const X3D::SFNode & sourceNode,
                               const std::string & sourceField,
                               const X3D::SFNode & destinationNode,
                               const std::string & destinationField,
                               const UndoStepPtr & undoStep) const
{
	using deleteRoute = void (X3D::X3DExecutionContext::*) (const X3D::SFNode &, const std::string &, const X3D::SFNode &, const std::string &);

	bool sourceImported      = false;
	bool destinationImported = false;

	try
	{
		sourceImported = executionContext -> isImportedNode (sourceNode);
	}
	catch (...)
	{ }

	try
	{
		destinationImported = executionContext -> isImportedNode (destinationNode);
	}
	catch (...)
	{ }

	if (sourceImported and destinationImported)
	{
		undoStep -> addUndoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);

		undoStep -> addRedoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);
	}
	else if (sourceImported and not destinationImported)
	{
		undoStep -> addUndoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             destinationNode,
		                             destinationField);

		undoStep -> addRedoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (sourceNode)),
		                             sourceField,
		                             destinationNode,
		                             destinationField);
	}
	else if (destinationImported and not sourceImported)
	{
		undoStep -> addUndoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);

		undoStep -> addRedoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             std::bind (&X3D::X3DExecutionContext::getImportedNode, executionContext, executionContext -> getLocalName (destinationNode)),
		                             destinationField);
	}
	else
	{
		undoStep -> addUndoFunction (&X3D::X3DExecutionContext::addRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             destinationNode,
		                             destinationField);

		undoStep -> addRedoFunction ((deleteRoute) & X3D::X3DExecutionContext::deleteRoute, executionContext,
		                             sourceNode,
		                             sourceField,
		                             destinationNode,
		                             destinationField);
	}

	executionContext -> deleteRoute (sourceNode, sourceField, destinationNode, destinationField);
}

void
X3DBrowserEditor::removePrototypes (X3D::X3DExecutionContext* const executionContext, X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	std::set <X3D::X3DProtoObjectPtr> protoDeclarations;

	// Find proto declaration used in node and children of node

	X3D::traverse (node, [&protoDeclarations] (X3D::SFNode & child)
	               {
	                  const X3D::X3DPrototypeInstancePtr instance (child);

	                  if (instance)
								protoDeclarations .emplace (instance -> getProtoDeclaration ());

	                  return true;
						});

	// Filter out proto declaration used in scene

	X3D::traverse (executionContext -> getRootNodes (), [&protoDeclarations] (X3D::SFNode & child)
	               {
	                  const X3D::X3DPrototypeInstancePtr instance (child);

	                  if (instance)
								protoDeclarations .erase (instance -> getProtoDeclaration ());

	                  return true;
						});

	// Remove unused Protos from X3DExecutionContext

	for (const auto & protoDeclaration : protoDeclarations)
	{
		if (protoDeclaration -> getName () .empty ())
			continue;

		try
		{
			const auto proto = executionContext -> getProtoDeclaration (protoDeclaration -> getName ());

			undoStep -> addUndoFunction (&X3D::X3DExecutionContext::updateProtoDeclaration, executionContext, protoDeclaration -> getName (), proto);
			undoStep -> addRedoFunction (&X3D::X3DExecutionContext::removeProtoDeclaration, executionContext, protoDeclaration -> getName ());

			executionContext -> removeProtoDeclaration (protoDeclaration -> getName ());
		}
		catch (const X3D::X3DError &)
		{ }

		try
		{
			const auto externProto = executionContext -> getExternProtoDeclaration (protoDeclaration -> getName ());

			undoStep -> addUndoFunction (&X3D::X3DExecutionContext::updateExternProtoDeclaration, executionContext, protoDeclaration -> getName (), externProto);
			undoStep -> addRedoFunction (&X3D::X3DExecutionContext::removeExternProtoDeclaration, executionContext, protoDeclaration -> getName ());

			executionContext -> removeExternProtoDeclaration (protoDeclaration -> getName ());
		}
		catch (const X3D::X3DError &)
		{ }
	}
}

void
X3DBrowserEditor::createClone (const X3D::SFNode & clone, const X3D::MFNode & nodes, const UndoStepPtr & undoStep) const
{
	for (const auto & node : nodes)
	{
		if (not node)
			continue;

		const auto executionContext = getBrowser () -> getExecutionContext ();

		X3D::traverse (executionContext -> getRootNodes (), [&] (X3D::SFNode & parent)
		               {
		                  for (auto & field: parent -> getFieldDefinitions ())
		                  {
		                     switch (field -> getType ())
		                     {
										case X3D::X3DConstants::SFNode:
											{
											   const auto sfnode = static_cast <X3D::SFNode*> (field);

											   if (*sfnode == node)
													replaceNode (parent, *sfnode, clone, undoStep);

											   break;
											}
										case X3D::X3DConstants::MFNode:
											{
											   const auto mfnode = static_cast <X3D::MFNode*> (field);

											   replaceNode (parent, *mfnode, node, clone, undoStep);

											   break;
											}
										default:
											break;
									}
								}

		                  return true;
							});

		replaceNode (executionContext .getValue (), executionContext -> getRootNodes (), node, clone, undoStep);
	}
}

X3D::MFNode
X3DBrowserEditor::unlinkClone (const X3D::MFNode & clones, const UndoStepPtr & undoStep) const
{
	X3D::MFNode nodes;

	const auto executionContext = getBrowser () -> getExecutionContext ();

	for (const auto & clone : clones)
	{
		nodes .emplace_back (clone);

		bool first = true;

		X3D::traverse (executionContext -> getRootNodes (), [&] (X3D::SFNode & parent)
		               {
		                  for (auto & field: parent -> getFieldDefinitions ())
		                  {
		                     switch (field -> getType ())
		                     {
										case X3D::X3DConstants::SFNode:
											{
											   try
											   {
											      const auto sfnode = static_cast <X3D::SFNode*> (field);

											      if (*sfnode == clone)
											      {
											         if (not first)
											         {
											            const X3D::SFNode node = (*sfnode) -> copy (executionContext, X3D::FlattCopyType ());

											            replaceNode (parent, *sfnode, node, undoStep);

											            nodes .emplace_back (node);
														}

											         first = false;
													}
												}
											   catch (const X3D::Error <X3D::NOT_SUPPORTED> &)
											   { }

											   break;
											}
										case X3D::X3DConstants::MFNode:
											{
											   const auto mfnode = static_cast <X3D::MFNode*> (field);

											   unlinkClone (executionContext, parent, *mfnode, clone, nodes, first, undoStep);

											   break;
											}
										default:
											break;
									}
								}

		                  return true;
							});

		// Unlink in rootNodes array.

		unlinkClone (executionContext, executionContext .getValue (), executionContext -> getRootNodes (), clone, nodes, first, undoStep);
	}

	// Setup new nodes.
	executionContext -> realize ();

	return nodes;
}

void
X3DBrowserEditor::unlinkClone (X3D::X3DExecutionContext* const executionContext,
                               const X3D::SFNode & parent,
                               X3D::MFNode & mfnode,
                               const X3D::SFNode & clone,
                               X3D::MFNode & nodes,
                               bool & first,
                               const UndoStepPtr & undoStep) const
{
	const auto indices = mfnode .indices_of (clone);

	if (indices .empty ())
		return;

	for (const auto & index : indices)
	{
		try
		{
			if (not first)
			{
				const X3D::SFNode node = mfnode [index] -> copy (executionContext, X3D::FlattCopyType ());

				replaceNode (parent, mfnode, index, node, undoStep);

				nodes .emplace_back (node);
			}

			first = false;
		}
		catch (const X3D::Error <X3D::NOT_SUPPORTED> &)
		{ }
	}
}

// Grouping operations

X3D::SFNode
X3DBrowserEditor::groupNodes (const X3D::MFNode & nodes, const UndoStepPtr & undoStep) const
{
	const auto executionContext = getBrowser () -> getExecutionContext ();

	const X3D::X3DGroupingNodePtr group (executionContext -> createNode ("Transform"));

	undoStep -> addVariables (group);

	for (const auto & child : nodes)
	{
		if (not child)
			continue;

		// Adjust transformation
		X3D::Matrix4d                  childModelViewMatrix = findModelViewMatrix (child);
		const X3D::X3DTransformNodePtr transform (child);

		if (transform)
		{
			childModelViewMatrix .mult_left (transform -> getMatrix ());

			setMatrix (transform, childModelViewMatrix, undoStep);
		}

		// Remove child from scene graph

		removeNodeFromSceneGraph (getBrowser () -> getExecutionContext (), child, undoStep);

		// Add to group

		emplaceBack (group -> children (), child, undoStep);
	}

	group -> setup ();

	emplaceBack (executionContext -> getRootNodes (), X3D::SFNode (group), undoStep);

	return X3D::SFNode (group);
}

X3D::MFNode
X3DBrowserEditor::ungroupNodes (const X3D::MFNode & groups, const UndoStepPtr & undoStep) const
{
	X3D::MFNode children;

	const auto executionContext = getBrowser () -> getExecutionContext ();

	for (const auto & node : groups)
	{
		try
		{
			const X3D::X3DGroupingNodePtr group (node);

			if (not group)
				continue;

			undoStep -> addVariables (node);

			const auto layers = group -> getLayers ();

			// Ungroup children

			const auto groupingField = static_cast <X3D::MFNode*> (node -> getField ("children"));

			for (const auto & child : *groupingField)
			{
				if (not child)
					continue;

				// Adjust transformation

				X3D::Matrix4d                  childModelViewMatrix = findModelViewMatrix (child);
				const X3D::X3DTransformNodePtr transform (child);

				if (transform)
				{
					childModelViewMatrix .mult_left (transform -> getMatrix ());

					setMatrix (transform, childModelViewMatrix, undoStep);
				}

				// Add to layer

				for (const auto & layer : layers)
				{
					if (layer -> isLayer0 ())
						emplaceBack (executionContext -> getRootNodes (), child, undoStep);

					else
					{
						undoStep -> addVariables (X3D::SFNode (layer));

						emplaceBack (layer -> children (), child, undoStep);
					}
				}

				undoStep -> addUndoFunction (&X3D::MFNode::push_front, groupingField, child);

				children .emplace_back (child);
			}

			undoStep -> addRedoFunction (&X3D::MFNode::clear, groupingField);

			groupingField -> clear ();

			// Remove group from scene

			removeNodeFromScene (executionContext, node, undoStep);
		}
		catch (const X3D::Error <X3D::INVALID_NODE> &)
		{
			// No grouping field found
		}
	}

	return children;
}

bool
X3DBrowserEditor::addToGroup (const X3D::SFNode & group, const X3D::MFNode & children, const UndoStepPtr & undoStep) const
{
	if (not group)
		return false;

	bool added = false;

	undoStep -> addVariables (group);

	for (const auto & child : children)
	{
		try
		{
			if (child == group)
				continue;

			if (not child)
				continue;

			const auto containerField = getContainerField (group, child);

			// Get group modelview matrix

			X3D::Matrix4d                          groupModelViewMatrix = findModelViewMatrix (group);
			const X3D::X3DTransformMatrix4DNodePtr transform (group);

			if (transform)
				groupModelViewMatrix .mult_left (transform -> getMatrix ());

			// Adjust child transformation

			{
				X3D::Matrix4d                  childModelViewMatrix = findModelViewMatrix (child);
				const X3D::X3DTransformNodePtr transform (child);

				if (transform)
				{
					try
					{
						childModelViewMatrix .mult_left (transform -> getMatrix ());
						childModelViewMatrix .mult_right (~groupModelViewMatrix);

						setMatrix (transform, childModelViewMatrix, undoStep);
					}
					catch (const std::domain_error & error)
					{ }
				}
			}

			// Remove child from scene graph

			removeNodeFromSceneGraph (getBrowser () -> getExecutionContext (), child, undoStep);

			// Add child to group

			const auto sfnode = dynamic_cast <X3D::SFNode*> (containerField);

			if (sfnode)
			{
				undoStep -> addUndoFunction (&X3D::SFNode::setValue, sfnode, *sfnode);
				undoStep -> addRedoFunction (&X3D::SFNode::setValue, sfnode, child);

				sfnode -> setValue (child);

				added = true;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                // XXX Remove previous child completely from scene if not in scene anymore
			}
			else
			{
				const auto mfnode = dynamic_cast <X3D::MFNode*> (containerField);

				if (mfnode)
				{
					emplaceBack (*mfnode, child, undoStep);

					added = true;
				}
			}
		}
		catch (const X3D::Error <X3D::INVALID_NODE> &)
		{
			// No container field found
		}
	}

	return added;
}

void
X3DBrowserEditor::detachFromGroup (X3D::MFNode children, const bool detachToLayer0, const UndoStepPtr & undoStep) const
{
	const auto executionContext = getBrowser () -> getExecutionContext ();

	for (const auto & child : children)
	{
		X3D::X3DNodePtr node (child);

		if (not node)
			continue;

		const auto layers = node -> getLayers ();

		// Adjust transformation

		const X3D::X3DTransformNodePtr transform (node);

		if (transform)
		{
			X3D::Matrix4d childModelViewMatrix = findModelViewMatrix (node);

			childModelViewMatrix .mult_left (transform -> getMatrix ());

			setMatrix (transform, childModelViewMatrix, undoStep);
		}

		// Remove child from scene graph

		removeNodeFromSceneGraph (executionContext, child, undoStep);

		// Add to layers

		if (detachToLayer0)
			emplaceBack (executionContext -> getRootNodes (), child, undoStep);

		else
		{
			for (const auto & layer : layers)
			{
				if (layer -> isLayer0 ())
					emplaceBack (executionContext -> getRootNodes (), child, undoStep);

				else
				{
					undoStep -> addVariables (X3D::SFNode (layer));

					emplaceBack (layer -> children (), child, undoStep);
				}
			}
		}
	}
}

X3D::MFNode
X3DBrowserEditor::createParentGroup (const X3D::MFNode & children, const UndoStepPtr & undoStep) const
{
	X3D::MFNode groups;

	const auto executionContext = getBrowser () -> getExecutionContext ();

	for (const auto & child : children)
	{
		if (not child)
			continue;

		X3D::traverse (executionContext -> getRootNodes (), [&] (X3D::SFNode & parent)
		               {
		                  for (auto & field: parent -> getFieldDefinitions ())
		                  {
		                     switch (field -> getType ())
		                     {
										case X3D::X3DConstants::SFNode:
											{
											   const auto sfnode = static_cast <X3D::SFNode*> (field);

											   if (*sfnode == child)
											   {
											      // Add node to group

											      const X3D::X3DGroupingNodePtr group (executionContext -> createNode ("Transform"));

											      undoStep -> addVariables (group);

											      emplaceBack (group -> children (), child, undoStep);

											      group -> setup ();

											      // Replace node with Transform

											      undoStep -> addVariables (parent);

											      undoStep -> addUndoFunction (&X3D::SFNode::setValue, sfnode, child);
											      undoStep -> addRedoFunction (&X3D::SFNode::setValue, sfnode, group);

											      sfnode -> setValue (group);

											      groups .emplace_front (group);
												}

											   break;
											}
										case X3D::X3DConstants::MFNode:
											{
											   const auto mfnode = static_cast <X3D::MFNode*> (field);

											   createParentGroup (*mfnode, child, parent, groups, undoStep);

											   break;
											}
										default:
											break;
									}
								}

		                  return true;
							});

		createParentGroup (executionContext -> getRootNodes (), child, executionContext .getValue (), groups, undoStep);
	}

	return groups;
}

void
X3DBrowserEditor::createParentGroup (X3D::MFNode & mfnode, const X3D::SFNode & child, const X3D::SFNode & parent, X3D::MFNode & groups, const UndoStepPtr & undoStep) const
{
	const auto indices = mfnode .indices_of (child);

	if (indices .empty ())
		return;

	const auto executionContext = getBrowser () -> getExecutionContext ();

	undoStep -> addVariables (parent);

	for (const auto & index : indices)
	{
		// Add node to group

		const X3D::X3DGroupingNodePtr group (executionContext -> createNode ("Transform"));

		undoStep -> addVariables (group);

		emplaceBack (group -> children (), child, undoStep);

		group -> setup ();

		// Replace node with Transform

		using set1Value = void (X3D::MFNode::*) (const X3D::MFNode::size_type, const X3D::SFNode &);

		undoStep -> addUndoFunction ((set1Value) & X3D::MFNode::set1Value, std::ref (mfnode), index, child);
		undoStep -> addRedoFunction ((set1Value) & X3D::MFNode::set1Value, std::ref (mfnode), index, X3D::SFNode (group));

		mfnode [index] = group;

		groups .emplace_front (group);
	}
}

void
X3DBrowserEditor::createProtoInstance (const std::string & name, const UndoStepPtr & undoStep) const
{
	try
	{
		const auto        executionContext = getBrowser () -> getExecutionContext ();
		const X3D::SFNode instance (executionContext -> createProtoInstance (name));

		executionContext -> addUninitializedNode (instance);
		executionContext -> realize ();

		emplaceBack (executionContext -> getRootNodes (), instance, undoStep);
	}
	catch (const X3D::X3DError &)
	{ }
}

// Undo functions

void
X3DBrowserEditor::translateSelection (const X3D::Vector3f & translation, const bool alongFrontPlane)
{
	for (const auto & node : basic::reverse_adapter (getBrowser () -> getSelection () -> getChildren ()))
	{
		X3D::X3DTransformNodePtr transform (node);

		if (transform)
		{
			using setValue = void (X3D::SFVec3f::*) (const X3D::Vector3f &);

			const auto undoStep = std::make_shared <UndoStep> (_ ("Nudge"));

			getSelection () -> redoRestoreSelection (undoStep);

			//undoStep -> addVariables (node);
			undoStep -> addUndoFunction ((setValue) & X3D::SFVec3f::setValue, std::ref (transform -> translation ()), transform -> translation ());
			undoStep -> addRedoFunction ((setValue) & X3D::SFVec3f::setValue, std::ref (transform -> translation ()), transform -> translation () + translation);

			transform -> translation () += translation;

			getSelection () -> undoRestoreSelection (undoStep);

			addUndoStep (undoStep);
			break;
		}
	}
}

void
X3DBrowserEditor::saveMatrix (const X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	X3D::X3DTransformNodePtr transform (node);

	if (transform)
	{
		undoStep -> addUndoFunction (&X3D::X3DTransformNode::setMatrix,
		                             transform,
		                             transform -> getMatrix ());
	}
}

void
X3DBrowserEditor::setMatrix (const X3D::X3DTransformNodePtr & transform, const X3D::Matrix4d & matrix, const UndoStepPtr & undoStep) const
{
	undoStep -> addUndoFunction (&X3D::X3DTransformNode::setMatrix,
	                             transform,
	                             transform -> getMatrix ());

	undoStep -> addRedoFunction (&X3D::X3DTransformNode::setMatrix,
	                             transform,
	                             matrix);

	transform -> setMatrix (matrix);
}

void
X3DBrowserEditor::emplaceBack (X3D::MFNode & array, const X3D::SFNode & node, const UndoStepPtr & undoStep) const
{
	// Add to group

	undoStep -> addUndoFunction (&X3DBrowserEditor::undoInsertNode, this,
	                             std::ref (array),
	                             array .size (),
	                             node);

	undoStep -> addRedoFunction (&X3D::MFNode::push_back, std::ref (array), node);

	array .emplace_back (node);
}

void
X3DBrowserEditor::undoInsertNode (X3D::MFNode & field, size_t index, const X3D::SFNode & node) const
{
	if (index < field .size () and field [index] == node)
	{
		field .erase (field .begin () + index);
	}
	else
	{
		// There has something changed, clear history.
		__LOG__ << std::endl;
	}
}

void
X3DBrowserEditor::undoEraseNode (X3D::MFNode & field, const X3D::SFNode & value, const std::vector <size_t> & indices) const
{
	for (const auto & index : indices)
	{
		if (index <= field .size ())
		{
			field .insert (field .begin () + index, value);
		}
		else
		{
			// There is has something changed, clear history
			__LOG__ << std::endl;
		}
	}
}

// Misc

X3D::Matrix4d
X3DBrowserEditor::findModelViewMatrix (X3D::X3DBaseNode* const node) const
{
	X3D::Matrix4d modelViewMatix;

	std::set <X3D::X3DBaseNode*> seen;

	for (const auto & parentNode : getParentNodes (node))
	{
		if (findModelViewMatrix (parentNode, modelViewMatix, seen))
			break;
	}

	return modelViewMatix;
}

bool
X3DBrowserEditor::findModelViewMatrix (X3D::X3DBaseNode* const node, X3D::Matrix4d & modelViewMatix, std::set <X3D::X3DBaseNode*> & seen) const
{
	if (not seen .emplace (node) .second)
		return false;

	// Do we found the layer?

	if (dynamic_cast <X3D::X3DLayerNode*> (node))
		return true;

	// Bad nodes

	if (dynamic_cast <X3D::X3DBrowser*> (node))
		return false;

	if (dynamic_cast <X3D::Scene*> (node))
		return false;

	if (dynamic_cast <X3D::Proto*> (node))
		return false;

	if (dynamic_cast <X3D::X3DScriptNode*> (node))
		return false;

	if (dynamic_cast <X3D::X3DProgrammableShaderObject*> (node))
		return false;

	// Iterate over parents

	for (const auto & parentNode : getParentNodes (node))
	{
		if (findModelViewMatrix (parentNode, modelViewMatix, seen))
		{
			const auto transform = dynamic_cast <X3D::X3DTransformMatrix4DNode*> (node);

			if (transform)
				modelViewMatix .mult_left (transform -> getMatrix ());

			return true;
		}
	}

	return false;
}

std::vector <X3D::X3DBaseNode*>
X3DBrowserEditor::getParentNodes (X3D::X3DBaseNode* const node) const
{
	std::vector <X3D::X3DBaseNode*> parentNodes;

	for (const auto & parent : node -> getParents ())
	{
		const auto sfnode = dynamic_cast <X3D::SFNode*> (parent);

		if (sfnode)
		{
			for (const auto & secondParent : sfnode -> getParents ())
			{
				const auto mfnode = dynamic_cast <X3D::MFNode*> (secondParent);

				if (mfnode)
				{
					for (const auto & thirdParent : mfnode -> getParents ())
					{
						const auto baseNode = dynamic_cast <X3D::X3DBaseNode*> (thirdParent);

						if (baseNode)
							parentNodes .emplace_back (baseNode);
					}

					continue;
				}

				const auto baseNode = dynamic_cast <X3D::X3DBaseNode*> (secondParent);

				if (baseNode)
				{
					parentNodes .emplace_back (baseNode);
					continue;
				}
			}
		}
	}

	return parentNodes;
}

X3D::X3DFieldDefinition*
X3DBrowserEditor::getContainerField (const X3D::SFNode & parent, const X3D::SFNode & child) const
throw (X3D::Error <X3D::INVALID_NODE>)
{
	try
	{
		const auto field = parent -> getField (child -> getInnerNode () -> getContainerField ());

		if (field -> getAccessType () & X3D::initializeOnly)
		{
			if (field -> getType () == X3D::X3DConstants::SFNode or field -> getType () == X3D::X3DConstants::MFNode)
				return field;
		}
	}
	catch (const X3D::X3DError &)
	{ }

	// Find appropriate field

	try
	{
		const auto field = parent -> getField ("children");

		if (field -> getAccessType () == X3D::initializeOnly or field -> getAccessType () == X3D::inputOutput)
		{
			if (field -> getType () == X3D::X3DConstants::MFNode)
				return field;
		}
	}
	catch (const X3D::X3DError &)
	{ }

	// Find last MFNode

	for (auto & field : basic::reverse_adapter (parent -> getFieldDefinitions ()))
	{
		if (field -> getAccessType () == X3D::initializeOnly or field -> getAccessType () == X3D::inputOutput)
		{
			if (field -> getType () == X3D::X3DConstants::MFNode)
				return field;
		}
	}

	// Find last SFNode not metadata

	for (auto & field : basic::reverse_adapter (parent -> getFieldDefinitions ()))
	{
		if (field -> getAccessType () == X3D::initializeOnly or field -> getAccessType () == X3D::inputOutput)
		{
			if (field -> getType () == X3D::X3DConstants::SFNode and field -> getName () not_eq "metadata")
				return field;
		}
	}

	throw X3D::Error <X3D::INVALID_NODE> ("No appropriate container field found.");
}

void
X3DBrowserEditor::editCData (const X3D::SFNode & node)
{
	X3D::MFString* const cdata          = node -> getCData ();
	std::string          filename       = "/tmp/titania-XXXXXX.js";
	const int            fileDescriptor = mkstemps (&filename [0], 3);

	if (not cdata or fileDescriptor == -1)
		return;

	// Output file

	std::ofstream ostream (filename);

	std::string name = node -> getName ();
	pcrecpp::RE (R"/((\*/))/") .GlobalReplace ("", &name);

	ostream
		<< "/**" << std::endl
		<< " * " << node -> getTypeName () << " " << name << std::endl
		<< " * " << _ ("This file is automaticaly generated to edit CDATA fields.") << std::endl
		<< " * " << _ ("Each SFString value is enclosed inside a CDATA section.") << std::endl
		<< " * " << _ ("A CDATA section starts with \"<![CDATA[\" and ends with \"]]>\".") << std::endl
		<< " * " << _ ("If this is a Script node a inline script must start with \"javascript:\".") << std::endl
		<< " * " << _ ("If this is a shader a it must start with \"data:text/plain,\".") << std::endl
		<< " * " << _ ("Just save this file to apply changes.") << std::endl
		<< " **/" << std::endl
		<< std::endl;

	for (const auto & string : *cdata)
	{
		ostream
			<< std::endl
			<< "<![CDATA["
			<< X3D::escape_cdata (string)
			<< "]]>"
			<< std::endl;
	}

	// Launch TextEditor

	Glib::RefPtr <Gio::File>        file        = Gio::File::create_for_path (filename);
	Glib::RefPtr <Gio::FileMonitor> fileMonitor = file -> monitor_file ();

	fileMonitor -> signal_changed () .connect (sigc::bind (sigc::mem_fun (*this, &X3DBrowserEditor::on_cdata_changed), node));
	fileMonitors .emplace (file, fileMonitor);

	try
	{
		Gio::AppInfo::create_from_commandline (os::realpath ("/usr/bin/gnome-text-editor"), "", Gio::APP_INFO_CREATE_NONE) -> launch (file);
	}
	catch (...)
	{ }

	::close (fileDescriptor);
}

void
X3DBrowserEditor::on_cdata_changed (const Glib::RefPtr <Gio::File> & file, const Glib::RefPtr <Gio::File> &, Gio::FileMonitorEvent event, const X3D::SFNode & node)
{
	io::string         comment_start ("/*");
	io::inverse_string comment ("*/");
	io::sequence       whitespaces ("\r\n \t");
	io::string         cdata_start ("<![CDATA[");
	io::inverse_string contents ("]]>");

	if (event not_eq Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
		return;

	// Parse file

	X3D::MFString string;

	std::ifstream istream (file -> get_path ());

	std::string ws;

	whitespaces (istream, ws);
	comment_start (istream);
	comment (istream, ws);

	while (istream)
	{
		whitespaces (istream, ws);

		if (cdata_start (istream))
		{
			std::string value;

			contents (istream, value);

			string .emplace_back (std::move (value));
		}
		else
			break;
	}

	// Set value.

	X3D::MFString* const cdata = node -> getCData ();

	if (string not_eq *cdata)
	{
		const auto undoStep = std::make_shared <UndoStep> (_ ("Edit CDATA Field"));

		undoStep -> addVariables (node);
		undoStep -> addUndoFunction (&OutlineTreeViewEditor::queue_draw, getBrowserWindow () -> getOutlineTreeView ());

		undoStep -> addUndoFunction (&X3D::MFString::setValue, cdata, *cdata);
		undoStep -> addRedoFunction (&X3D::MFString::setValue, cdata, string);
		cdata -> setValue (string);

		undoStep -> addRedoFunction (&OutlineTreeViewEditor::queue_draw, getBrowserWindow () -> getOutlineTreeView ());
		getBrowserWindow () -> getOutlineTreeView () -> queue_draw ();

		addUndoStep (undoStep);
	}
	else
		__LOG__ << std::endl;
}

void
X3DBrowserEditor::shutdown ()
{
	undoHistory  .clear ();

	for (const auto & fileMonitor : fileMonitors)
	{
		fileMonitor .second -> cancel ();
		fileMonitor .first -> remove ();
	}

	fileMonitors .clear ();
}

X3DBrowserEditor::~X3DBrowserEditor ()
{ }

} // puck
} // titania
