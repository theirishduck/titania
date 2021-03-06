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

#ifndef __TITANIA_PEASE_BLOSSOM_EXPRESSIONS_BLOCK_STATEMENT_H__
#define __TITANIA_PEASE_BLOSSOM_EXPRESSIONS_BLOCK_STATEMENT_H__

#include "../Expressions/pbStatement.h"

#include <cmath>

namespace titania {
namespace pb {

/**
 *  Class to represent a ECMAScript block statement.
 */
class BlockStatement :
	public pbStatement
{
public:

	///  @name Construction

	///  Constructs new BlockStatement expression.
	BlockStatement (array <ptr <pbStatement>> && statements) :
		pbStatement (StatementType::BLOCK_STATEMENT),
		 statements (std::move (statements))
	{ construct (); }

	///  Creates a copy of this object.
	virtual
	ptr <pbStatement>
	copy (pbExecutionContext* const executionContext) const
	noexcept (true) final override
	{
		array <ptr <pbStatement>>  statements;

		for (const auto & statement : this -> statements)
			statements .emplace_back (statement -> copy (executionContext));

		return new BlockStatement (std::move (statements));
	}

	///  @name Operations

	///  Converts its argument to a value of type Boolean.
	virtual
	CompletionType
	getValue () const
	throw (pbError) final override
	{
		CompletionType result;

		for (const auto & statement : statements)
		{
			auto value = statement -> getValue ();

			if (value .getStatement ())
			{
				switch (value .getStatement () -> getType ())
				{
					case StatementType::CONTINUE_STATEMENT:
					case StatementType::RETURN_STATEMENT:
						return value;
					default:
						continue;
				}
			}

			result = std::move (value);
		}

		return result;
	}

	///  @name Input/Output

	///  Inserts this object into the output stream @a ostream.
	virtual
	void
	toStream (std::ostream & ostream) const final override
	{
		ostream
			<< '{'
			<< Generator::TidyBreak
			<< Generator::IncIndent
			<< Generator::Indent
			<< pb::toStream (statements)
			<< Generator::TidyBreak
			<< Generator::DecIndent
			<< Generator::Indent
			<< '}';
	}

private:

	///  @name Construction

	///  Performs neccessary operations after construction.
	void
	construct ()
	{ addChildren (statements); }

	///  @name Members

	const array <ptr <pbStatement>> statements;

};

} // pb
} // titania

#endif
