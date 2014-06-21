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

#ifndef __TITANIA_X3D_PEASE_BLOSSOM_EXPRESSIONS_ASSIGNMENT_EXPRESSION_H__
#define __TITANIA_X3D_PEASE_BLOSSOM_EXPRESSIONS_ASSIGNMENT_EXPRESSION_H__

#include "../Execution/vsExecutionContext.h"
#include "../Expressions/AssignmentOperatorType.h"
#include "../Expressions/vsExpression.h"
#include "../Primitives/Int32.h"
#include "../Primitives/String.h"
#include "../Primitives/UInt32.h"
#include "../Primitives/vsValue.h"

namespace titania {
namespace pb {

/**
 *  Class to represent a ECMAScript identifier expression.
 */
class AssignmentExpression :
	public vsExpression
{
public:

	///  @name Construction

	///  Constructs new AssignmentExpression expression.
	AssignmentExpression (vsExecutionContext* const executionContext, var && lhs, var && rhs, AssignmentOperatorType type) :
		    vsExpression (),
		executionContext (executionContext),
		             lhs (std::move (lhs)),
		             rhs (std::move (rhs)),
		            type (type)
	{ construct (); }

	///  Creates a copy of this object.
	virtual
	var
	copy (vsExecutionContext* const executionContext) const final override
	{ return make_var <AssignmentExpression> (executionContext, lhs -> copy (executionContext), rhs -> copy (executionContext), type); }

	///  @name Common members

	///  Returns the type of the value. For this expression this is »VARIABLE«.
	virtual
	ValueType
	getType () const final override
	{ return ASSIGNMENT_EXPRESSION; }

	///  @name Operations

	///  Converts its input argument to either Primitive or Object type.
	virtual
	var
	toValue () const final override
	{ return setProperty (); }


private:

	///  @name Construction

	///  Performs neccessary operations after construction.
	void
	construct ()
	{
		if (not lhs)
			throw ReferenceError ("Invalid assignment left-hand side.");

		addChildren (executionContext, lhs, rhs);
	}

	///  @name Operations

	var
	setProperty () const
	{
		var value = rhs -> toValue ();

		switch (type)
		{
			case AssignmentOperatorType::ASSIGNMENT:
				break;

			case AssignmentOperatorType::MULTIPLICATION_ASSIGNMENT:
			{
				value = make_var <Number> (lhs -> toNumber () * value -> toNumber ());
				break;
			}
			case AssignmentOperatorType::DIVISION_ASSIGNMENT:
			{
				value = make_var <Number> (lhs -> toNumber () / value -> toNumber ());
				break;
			}
			case AssignmentOperatorType::REMAINDER_ASSIGNMENT:
			{
				value = make_var <Number> (std::fmod (lhs -> toNumber (), value -> toNumber ()));
				break;
			}
			case AssignmentOperatorType::ADDITION_ASSIGNMENT:
			{
				if (lhs -> getType () == STRING or value -> getType () == STRING)
					value = make_var <String> (lhs -> toString () + value -> toString ());

				else
					value = make_var <Number> (lhs -> toNumber () + value -> toNumber ());

				break;
			}
			case AssignmentOperatorType::SUBTRACTION_ASSIGNMENT:
			{
				value = make_var <Number> (lhs -> toNumber () - value -> toNumber ());
				break;
			}
			case AssignmentOperatorType::LEFT_SHIFT_ASSIGNMENT:
			{
				value = make_var <Int32> (lhs -> toInt32 () << (value -> toUInt32 () & 0x1f));
				break;
			}
			case AssignmentOperatorType::RIGHT_SHIFT_ASSIGNMENT:
			{
				value = make_var <Int32> (lhs -> toInt32 () >> (value -> toUInt32 () & 0x1f));
				break;
			}
			case AssignmentOperatorType::UNSIGNED_RIGHT_SHIFT_ASSIGNMENT:
			{
				value = make_var <UInt32> (lhs -> toUInt32 () >> (value -> toUInt32 () & 0x1f));
				break;
			}
			case AssignmentOperatorType::BITWISE_AND_ASSIGNMENT:
			{
				value = make_var <Int32> (lhs -> toInt32 () & value -> toInt32 ());
				break;
			}
			case AssignmentOperatorType::BITWISE_XOR_ASSIGNMENT:
			{
				value = make_var <Int32> (lhs -> toInt32 () ^ value -> toInt32 ());
				break;
			}
			case AssignmentOperatorType::BITWISE_OR_ASSIGNMENT:
			{
				value = make_var <Int32> (lhs -> toInt32 () | value -> toInt32 ());
				break;
			}
		}

		return lhs -> setValue (std::move (value));
	}

	///  @name Members

	const basic_ptr <vsExecutionContext> executionContext;
	const basic_ptr <vsExpression>       lhs;
	const var                            rhs;
	const AssignmentOperatorType         type;

};

} // pb
} // titania

#endif
