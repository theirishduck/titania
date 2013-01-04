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
 ******************************************************************************/

#ifndef __TITANIA_X3D_PARSER_PARSER_H__
#define __TITANIA_X3D_PARSER_PARSER_H__

#include "../Bits/Error.h"
#include "../Execution/Scene.h"
#include "../Parser/X3DParser.h"
#include "../Prototype/Proto.h"

#include <ios>
#include <pcrecpp.h>

namespace titania {
namespace X3D {

class Browser;

class Parser :
	public X3DParser
{
private:

	class AccessTypes :
		public std::map <const std::string, AccessType>
	{
	public:

		AccessTypes ();

	};

	class FieldTypes :
		public std::map <const std::string, const X3DFieldDefinition*>
	{
	public:

		FieldTypes ();

	};


public:

	Parser (X3DScene*);

	void
	parseIntoScene (std::istream & istream)
	throw (Error <INVALID_X3D>);

	static AccessTypes accessTypes;


private:

	typedef std::vector <X3DExecutionContext*> ExecutionContextStack;

	std::string
	getMessageFromError (const X3DError &);

	void
	pushExecutionContext (X3DExecutionContext* const);

	void
	popExecutionContext ();

	X3DExecutionContext*
	getExecutionContext () const
	throw (Error <INVALID_OPERATION_TIMING>,
	       Error <DISPOSED>);

	bool
	isInsideProtoDefinition () const;

	void
	addRootNode (const SFNode <X3DBaseNode> &);

	void
	comments ();

	bool
	comment (std::string &);

	void
	x3dScene ();

	bool
	headerStatement (std::string &, std::string &, std::string &, std::string &);

	const ProfileInfo*
	profileStatement ();

	ComponentInfoArray
	componentStatements ();

	const ComponentInfo*
	componentStatement ();

	bool
	componentSupportLevel (int32_t &);

	void
	unitStatements ();

	bool
	unitStatement ();

	bool
	unitConversionFactor (double &);

	bool
	exportStatement ();

	bool
	importStatement ();

	void
	metaStatements ();

	bool
	metaStatement ();

	bool
	metakey (std::string &);

	bool
	metavalue (std::string &);

	void
	statements ();

	bool
	statement ();

	bool
	nodeStatement (X3DFieldDefinition &);

	bool
	rootNodeStatement (SFNode <X3DBaseNode> &);

	bool
	protoStatement ();

	void
	protoStatements ();

	bool
	proto ();

	void
	protoBody ();

	FieldDefinitionArray
	interfaceDeclarations ();

	X3DFieldDefinition*
	restrictedInterfaceDeclaration ();

	X3DFieldDefinition*
	interfaceDeclaration ();

	bool
	externproto ();

	FieldDefinitionArray
	externInterfaceDeclarations ();

	X3DFieldDefinition*
	externInterfaceDeclaration ();

	bool
	routeStatement ();

	bool
	URLList (MFString*);

	bool
	node (X3DFieldDefinition &, const std::string & = "");

	void
	scriptBody (X3DBaseNode* const);

	bool
	scriptBodyElement (X3DBaseNode* const);

	void
	nodeBody (X3DBaseNode* const);

	bool
	nodeBodyElement (X3DBaseNode* const);

	bool
	Id (std::string &);

	bool
	componentNameId (std::string &);

	bool
	profileNameId (std::string & _Id) { return Id (_Id); }

	bool
	categoryNameId (std::string & _Id) { return Id (_Id); }

	bool
	unitNameId (std::string & _Id) { return Id (_Id); }

	bool
	nodeNameId (std::string & _Id) { return Id (_Id); }

	bool
	nodeTypeId (std::string & _Id) { return Id (_Id); }

	bool
	inlineNodeNameId (std::string & _Id) { return Id (_Id); }

	bool
	exportedNodeNameId (std::string & _Id) { return Id (_Id); }

	bool
	initializeOnlyId (std::string & _Id) { return Id (_Id); }

	bool
	inputOnlyId (std::string & _Id) { return Id (_Id); }

	bool
	inputOutputId (std::string & _Id) { return Id (_Id); }

	bool
	outputOnlyId (std::string & _Id) { return Id (_Id); }

	bool
	fieldType (std::string &);

	bool
	fieldValue (X3DFieldDefinition*);

	template <class Type>
	bool from_string (Type &, const std::string &, std::ios_base & (*) (std::ios_base &));

	bool
	_double (double &);

	bool
	_float (float &);

	bool
	_int32 (int32_t &);

	bool
	_hex (int32_t &);

	bool
	_string (std::string &);

	bool
	sfboolValue (SFBool*);

	bool
	mfboolValue (MFBool*);

	void
	sfboolValues (MFBool*);

	bool
	sfcolorValue (SFColor*);

	bool
	mfcolorValue (MFColor*);

	void
	sfcolorValues (MFColor*);

	bool
	sfcolorRGBAValue (SFColorRGBA*);

	bool
	mfcolorRGBAValue (MFColorRGBA*);

	void
	sfcolorRGBAValues (MFColorRGBA*);

	bool
	sfdoubleValue (SFDouble*);

	bool
	mfdoubleValue (MFDouble*);

	void
	sfdoubleValues (MFDouble*);

	bool
	sffloatValue (SFFloat*);

	bool
	mffloatValue (MFFloat*);

	void
	sffloatValues (MFFloat*);

	bool
	sfimageValue (SFImage*);

	bool
	mfimageValue (MFImage*);

	void
	sfimageValues (MFImage*);

	bool
	sfint32Value (SFInt32*);

	bool
	mfint32Value (MFInt32*);

	void
	sfint32Values (MFInt32*);

	bool
	sfmatrix3dValue (SFMatrix3d*);

	bool
	mfmatrix3dValue (MFMatrix3d*);

	void
	sfmatrix3dValues (MFMatrix3d*);

	bool
	sfmatrix3fValue (SFMatrix3f*);

	bool
	mfmatrix3fValue (MFMatrix3f*);

	void
	sfmatrix3fValues (MFMatrix3f*);

	bool
	sfmatrix4dValue (SFMatrix4d*);

	bool
	mfmatrix4dValue (MFMatrix4d*);

	void
	sfmatrix4dValues (MFMatrix4d*);

	bool
	sfmatrix4fValue (SFMatrix4f*);

	bool
	mfmatrix4fValue (MFMatrix4f*);

	void
	sfmatrix4fValues (MFMatrix4f*);

	bool
	sfnodeValue (X3DFieldDefinition* const);

	bool
	mfnodeValue (X3DFieldDefinition* const);

	void
	nodeStatements (MFNode <X3DBaseNode>*);

	bool
	sfrotationValue (SFRotation*);

	bool
	mfrotationValue (MFRotation*);

	void
	sfrotationValues (MFRotation*);

	bool
	sfstringValue (SFString*);

	bool
	mfstringValue (MFString*);

	void
	sfstringValues (MFString*);

	bool
	sftimeValue (SFTime*);

	bool
	mftimeValue (MFTime*);

	void
	sftimeValues (MFTime*);

	bool
	sfvec2dValue (SFVec2d*);

	bool
	mfvec2dValue (MFVec2d*);

	void
	sfvec2dValues (MFVec2d*);

	bool
	sfvec2fValue (SFVec2f*);

	bool
	mfvec2fValue (MFVec2f*);

	void
	sfvec2fValues (MFVec2f*);

	bool
	sfvec3dValue (SFVec3d*);

	bool
	mfvec3dValue (MFVec3d*);

	void
	sfvec3dValues (MFVec3d*);

	bool
	sfvec3fValue (SFVec3f*);

	bool
	mfvec3fValue (MFVec3f*);

	void
	sfvec3fValues (MFVec3f*);

	bool
	sfvec4dValue (SFVec4d*);

	bool
	mfvec4dValue (MFVec4d*);

	void
	sfvec4dValues (MFVec4d*);

	bool
	sfvec4fValue (SFVec4f*);

	bool
	mfvec4fValue (MFVec4f*);

	void
	sfvec4fValues (MFVec4f*);

	X3DFieldDefinition*
	createField (const std::type_info &);

	X3DScene*             scene;
	std::string           input;
	pcrecpp::StringPiece  string;
	ExecutionContextStack executionContextStack;

	std::deque <X3DBaseNode*> nodeList;

};

} // X3D
} // titania

#endif
