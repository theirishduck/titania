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

#include "X3DPointingDevice.h"

#include "../Browser.h"

namespace titania {
namespace X3D {

X3DPointingDevice::X3DPointingDevice (Browser* const browser) :
	          X3DBrowserObject (browser),
	     key_press_conncection (),
	   key_release_conncection (),
	  button_press_conncection (),
	button_release_conncection (),
	 motion_notify_conncection (),
	  leave_notify_conncection (),
	                      keys (),
	                    button (0),
	                    isOver (false)
{ }

void
X3DPointingDevice::connect ()
{
	disconnect ();

	key_press_conncection   = getBrowser () -> signal_key_press_event      () .connect (sigc::mem_fun (*this, &PointingDevice::on_key_press_event));
	key_release_conncection = getBrowser () -> signal_key_release_event    () .connect (sigc::mem_fun (*this, &PointingDevice::on_key_release_event));

	button_press_conncection   = getBrowser () -> signal_button_press_event   () .connect (sigc::mem_fun (*this, &PointingDevice::on_button_press_event),   false);
	button_release_conncection = getBrowser () -> signal_button_release_event () .connect (sigc::mem_fun (*this, &PointingDevice::on_button_release_event), false);
	motion_notify_conncection  = getBrowser () -> signal_motion_notify_event  () .connect (sigc::mem_fun (*this, &PointingDevice::on_motion_notify_event));
	leave_notify_conncection   = getBrowser () -> signal_leave_notify_event   () .connect (sigc::mem_fun (*this, &PointingDevice::on_leave_notify_event));

	isOver = false;
}

void
X3DPointingDevice::disconnect ()
{
	key_press_conncection   .disconnect ();
	key_release_conncection .disconnect ();

	button_press_conncection   .disconnect ();
	button_release_conncection .disconnect ();
	motion_notify_conncection  .disconnect ();
	leave_notify_conncection   .disconnect ();
}

bool
X3DPointingDevice::on_key_press_event (GdkEventKey* event)
{
	keys .press (event);
	return false;
}

bool
X3DPointingDevice::on_key_release_event (GdkEventKey* event)
{
	keys .release (event);
	return false;
}

bool
X3DPointingDevice::on_motion_notify_event (GdkEventMotion* event)
{
	if (button == 0 or button == 1)
		set_motion (event -> x, event -> y);

	return false;
}

void
X3DPointingDevice::set_motion (const double x, const double y)
{
	if (not keys .control ())
	{
		const bool picked = pick (x, y);

		if (picked)
		{
			if (haveSensor ())
			{
				if (not isOver)
				{
					getBrowser () -> setCursor (Gdk::HAND2);
					isOver = true;
				}

				getBrowser () -> motionNotifyEvent ();

				return;
			}
		}

		if (isOver)
		{
			getBrowser () -> setCursor (Gdk::ARROW);
			isOver = false;
		}

		getBrowser () -> motionNotifyEvent ();
		motionNotifyEvent (picked);
	}
}

void
X3DPointingDevice::set_verify_motion (const double x, const double y)
{
	// Veryfy isOver state. This is neccessay if an Switch changes on buttonReleaseEvent
	// and the new child has a sensor node inside. This sensor node must be update to
	// reflect the correct isOver state.

	getBrowser () -> finished () .removeInterest (this, &X3DPointingDevice::set_verify_motion);
	set_motion (x, y);
}

bool
X3DPointingDevice::on_button_press_event (GdkEventButton* event)
{
	button = event -> button;

	getBrowser () -> grab_focus ();

	switch (button)
	{
		case 1:
		{
			if (not keys .control ())
			{
				const bool picked = pick (event -> x, event -> y);

				if (picked)
				{
					if (haveSensor ())
					{
						getBrowser () -> buttonPressEvent ();

						getBrowser () -> setCursor (Gdk::HAND1);

						getBrowser () -> finished () .addInterest (this, &X3DPointingDevice::set_verify_motion, event -> x, event -> y);

						return true;
					}
				}

				const bool eventHandled = buttonPressEvent (picked, event -> button);

				if (eventHandled)
					return not trackSensors ();
			}

			getBrowser () -> setCursor (Gdk::FLEUR);

			break;
		}
		case 2:
		{
			if (not keys .control ())
			{
				const bool picked       = pick (event -> x, event -> y);
				const bool eventHandled = buttonPressEvent (picked, event -> button);

				if (eventHandled)
					return not trackSensors ();
			}

			getBrowser () -> setCursor (Gdk::FLEUR);
			break;
		}
		default:
			break;
	}

	return false;
}

bool
X3DPointingDevice::on_button_release_event (GdkEventButton* event)
{
	button = 0;

	switch (event -> button)
	{
		case 1:
		{
			getBrowser () -> buttonReleaseEvent ();

			getBrowser () -> finished () .addInterest (this, &X3DPointingDevice::set_verify_motion, event -> x, event -> y);

			if (not getBrowser () -> getHits () .empty ())
			{
				if (not haveSensor ())
					buttonReleaseEvent (true, event -> button);
			}
			else
				buttonReleaseEvent (false, event -> button);

			// Set cursor

			if (isOver)
				getBrowser () -> setCursor (Gdk::HAND2);

			else
				getBrowser () -> setCursor (Gdk::ARROW);

			break;
		}
		case 2:
		{
			if (isOver)
				getBrowser () -> setCursor (Gdk::HAND2);

			else
				getBrowser () -> setCursor (Gdk::ARROW);

			break;
		}
		default:
			break;
	}

	return false;
}

bool
X3DPointingDevice::on_leave_notify_event (GdkEventCrossing*)
{
	getBrowser () -> leaveNotifyEvent ();
	leaveNotifyEvent ();

	getBrowser () -> setCursor (Gdk::ARROW);
	isOver = false;

	return false;
}

bool
X3DPointingDevice::pick (const double x, const double y)
{
	getBrowser () -> pick (x, getBrowser () -> get_height () - y);

	return not getBrowser () -> getHits () .empty ();
}

bool
X3DPointingDevice::haveSensor ()
{
	return not getBrowser () -> getHits () .front () -> sensors .empty ()
	       and trackSensors ();
}

} // X3D
} // titania
