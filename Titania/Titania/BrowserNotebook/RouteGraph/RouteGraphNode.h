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
 * Copyright 1999, 2016 Holger Seelig <holger.seelig@yahoo.de>.
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

#ifndef __TITANIA_BROWSER_NOTEBOOK_ROUTE_GRAPH_ROUTE_GRAPH_NODE_H__
#define __TITANIA_BROWSER_NOTEBOOK_ROUTE_GRAPH_ROUTE_GRAPH_NODE_H__

#include <Titania/X3D/Fields/SFNode.h>

#include <gtkmm.h>

namespace titania {
namespace puck {

class RouteGraph;

class RouteGraphNode :
	public Gtk::Box
{
public:

	///  @name Construction

	RouteGraphNode (RouteGraph* const routeGraph, const X3D::SFNode & node, const X3D::Vector2i & position);

	///  @name Member access

	const X3D::Vector2i &
	getPosition () const
	{ return position; }

	void
	setPosition (const X3D::Vector2i & value);

	///  @name Operations

	void
	bringToFront ();

	///  @name Destruction

	virtual
	~RouteGraphNode () final override;


protected:


private:

	///  @name Event handlers

	void
	set_name (Gtk::Label* const name);

	bool
	on_route_graph_button_press_event (GdkEventButton* event);
	
	bool
	on_route_graph_button_release_event (GdkEventButton* event);

	bool
	on_route_graph_motion_notify_event (GdkEventMotion* event);

	void
	on_footer_clicked (Gtk::Revealer* const fieldRevealer);

	void
	on_reveal_fields (Gtk::Revealer* const fieldRevealer);
	
	void
	on_fields_revealed (Gtk::Revealer* const fieldRevealer);

	///  @name Operations

	void
	build ();

	///  @name Members

	RouteGraph* const                                  routeGraph;
	const X3D::SFNode                                 node;
	std::map <X3D::X3DFieldDefinition*, Gtk::Button*> inputs;
	std::map <X3D::X3DFieldDefinition*, Gtk::Button*> outputs;
	int32_t                                           button;
	X3D::Vector2i                                     position;
	X3D::Vector2i                                     startPosition;
	bool                                              expanded;

};

} // puck
} // titania

#endif