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

#ifndef __TITANIA_X3D_PEASE_BLOSSOM_PARSER_PARSER_H__
#define __TITANIA_X3D_PEASE_BLOSSOM_PARSER_PARSER_H__

#include "../Execution/Exception.h"
#include "../Execution/OperatorType.h"
#include "../Execution/jsScope.h"
#include "../Primitives.h"

namespace titania {
namespace pb {

class Parser
{
public:

	///  @name Construction

	Parser (std::istream & istream, jsScope* const);

	///  @name Operations

	void
	parseIntoContext ();


private:

	void
	comments ();

	// A.1 Lexical Grammar

	bool
	identifier ();

	bool
	identifierName (std::string &);

	bool
	identifierStart (std::string &);

	bool
	identifierPart (std::string &);

	bool
	reservedWord (const std::string &);

	bool
	literal (ValuePtr &);

	bool
	nullLiteral (ValuePtr &);

	bool
	booleanLiteral (ValuePtr &);

	bool
	numericLiteral (ValuePtr &);

	bool
	decimalLiteral (ValuePtr &);

	bool
	binaryIntegerLiteral (ValuePtr &);

	bool
	octalIntegerLiteral (ValuePtr &);

	bool
	hexIntegerLiteral (ValuePtr &);

	// A.2 Number Conversions

	// A.3 Expressions

	bool
	primaryExpression (ValuePtr &);

	bool
	memberExpression (ValuePtr &);

	bool
	newExpression (ValuePtr &);

	bool
	leftHandSideExpression (ValuePtr &);

	bool
	postfixExpression (ValuePtr &);

	bool
	unaryExpression (ValuePtr &);

	bool
	multiplicativeExpression (ValuePtr &);

	bool
	additiveExpression (ValuePtr &);

	bool
	shiftExpression (ValuePtr &);

	bool
	relationalExpression (ValuePtr &);

	bool
	equalityExpression (ValuePtr &);

	bool
	bitwiseANDExpression (ValuePtr &);

	bool
	bitwiseXORExpression (ValuePtr &);

	bool
	bitwiseORExpression (ValuePtr &);

	bool
	logicalANDExpression (ValuePtr &);

	bool
	logicalORExpression (ValuePtr &);

	bool
	conditionalExpression (ValuePtr &);

	bool
	assignmentExpression (ValuePtr &);

	bool
	assignmentOperator (OperatorType &);

	bool
	expression (ValuePtr &);

	// A.4 Statements

	bool
	statement ();

	bool
	expressionStatement ();

	bool
	emptyStatement ();

	// A.5 Functions and Programs

	bool
	functionDeclaration ();

	void
	program ();

	void
	sourceElements ();

	bool
	sourceElement ();

	///  @name Members

	std::istream & istream;

	std::string whiteSpaces;

};

} // pb
} // titania

#endif
