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

#ifndef __TITANIA_X3D_FIELDS_X3DPTR_H__
#define __TITANIA_X3D_FIELDS_X3DPTR_H__

#include "../Basic/X3DField.h"

namespace titania {
namespace X3D {

class X3DPtrBase :
	virtual public X3DBase
{
public:

	///  @name Operations

	virtual
	X3DChildObject*
	getObject () const
	throw (Error <DISPOSED>) = 0;


protected:

	X3DPtrBase () :
		X3DBase ()
	{ }

};

template <class ValueType>
class X3DPtr :
	public X3DField <ValueType*>, public X3DPtrBase
{
public:

	typedef ValueType* internal_type;
	typedef ValueType* value_type;

	using X3DField <ValueType*>::addEvent;
	using X3DField <ValueType*>::operator =;
	using X3DField <ValueType*>::addInterest;
	using X3DField <ValueType*>::setValue;
	using X3DField <ValueType*>::getValue;

	///  @name Constructors

	X3DPtr () :
		X3DField <ValueType*> (nullptr)
	{ }

	X3DPtr (const X3DPtr & field) :
		X3DPtr (field .getValue ())
	{ }

	explicit
	X3DPtr (const X3DPtrBase & field) :
		X3DPtr (dynamic_cast <ValueType*> (field .getObject ()))
	{ }

	X3DPtr (X3DPtr && field) :
		X3DField <ValueType*> ()
	{ moveObject (field); }

	template <class Up>
	explicit
	X3DPtr (X3DPtr <Up> && field) :
		X3DField <ValueType*> ()
	{ moveObject (field); }

	//explicit
	X3DPtr (ValueType* const value) :
		X3DField <ValueType*> (value)
	{ addObject (value); }

	template <class Up>
	explicit
	X3DPtr (Up* const value) :
		X3DPtr (dynamic_cast <ValueType*> (value))
	{ }

	///  @name Construction

	virtual
	X3DPtr*
	create () const final override
	{ return new X3DPtr (); }

	virtual
	X3DPtr*
	clone () const
	throw (Error <INVALID_NAME>,
	       Error <NOT_SUPPORTED>) final override
	{ return new X3DPtr (*this); }

	virtual
	X3DPtr*
	clone (X3DExecutionContext* const executionContext) const
	throw (Error <INVALID_NAME>,
	       Error <NOT_SUPPORTED>) final override
	{
		X3DPtr* const field = new X3DPtr ();

		clone (executionContext, field);

		return field;
	}

	virtual
	void
	clone (X3DExecutionContext* const executionContext, X3DFieldDefinition* fieldDefinition) const
	throw (Error <INVALID_NAME>,
	       Error <NOT_SUPPORTED>) final override
	{
		X3DPtr* const field = static_cast <X3DPtr*> (fieldDefinition);

		if (getValue ())
			field -> set (dynamic_cast <ValueType*> (getValue () -> clone (executionContext)));

		else
			field -> set (nullptr);
	}

	/// @name Assignment operators

	///  Default assignment operator.  Behaves the same as the 6.7.6 setValue service.
	X3DPtr &
	operator = (const X3DPtr & field)
	{
		setValue (field);
		return *this;
	}

	X3DPtr &
	operator = (const X3DPtrBase & field)
	{
		setValue (dynamic_cast <ValueType*> (field .getObject ()));
		return *this;
	}

	///  Move assignment operator.  Behaves the same as the 6.7.6 setValue service.
	X3DPtr &
	operator = (X3DPtr && field)
	{
		if (&field == this)
			return *this;

		removeObject (getValue ());
		moveObject (field);
		addEvent ();

		return *this;
	}

	template <class Up>
	X3DPtr &
	operator = (X3DPtr <Up> && field)
	{
		if (&field == this)
			return *this;

		removeObject (getValue ());
		moveObject (field);
		addEvent ();

		return *this;
	}

	template <class Up>
	X3DPtr &
	operator = (Up* const value)
	{
		setValue (dynamic_cast <ValueType*> (value));
		return *this;
	}

	///  @name Field services

	virtual
	X3DConstants::FieldType
	getType () const final override
	{ return X3DConstants::SFNode; }

	virtual
	const std::string &
	getTypeName () const
	throw (Error <DISPOSED>) final override
	{ return typeName; }

	///  @name Set value services

	virtual
	void
	set (const internal_type & value) final override
	{
		if (value not_eq getValue ())
		{
			removeObject (getValue ());
			addObject (value);
		}

		setObject (value);
	}

	virtual
	void
	write (const X3DChildObject & field) final override
	{
		X3DChildObject* const object = dynamic_cast <const X3DPtrBase &> (field) .getObject ();

		set (dynamic_cast <internal_type> (object));
	}

	virtual
	X3DChildObject*
	getObject () const
	throw (Error <DISPOSED>) final override
	{ return getValue (); }

	///  @name Boolean operator

	operator bool () const
	{ return getValue (); }

	///  @name Access operators

	ValueType*
	operator -> () const
	{ return getValue (); }

	ValueType &
	operator * () const
	{ return *getValue (); }

	///  @name 6.7.7 Add field interest.

	template <class Class>
	void
	addInterest (Class* const object, void (Class::* memberFunction) (const X3DPtr &)) const
	{ addInterest (object, memberFunction, std::cref (*this)); }

	template <class Class>
	void
	addInterest (Class & object, void (Class::* memberFunction) (const X3DPtr &)) const
	{ addInterest (object, memberFunction, std::cref (*this)); }

	///  @name Input/Output
	virtual
	void
	fromStream (std::istream &)
	throw (Error <INVALID_X3D>,
	       Error <NOT_SUPPORTED>,
	       Error <INVALID_OPERATION_TIMING>,
	       Error <DISPOSED>) final override
	{ }

	virtual
	void
	toStream (std::ostream & ostream) const final override
	{
		if (getValue ())
			getValue () -> toStream (ostream);

		else
			ostream << "NULL";
	}

	virtual
	void
	toXMLStream (std::ostream & ostream) const final override
	{
		if (getValue ())
			getValue () -> toXMLStream (ostream);

		else
			ostream << "NULL";
	}

	///  @name Dispose

	virtual
	void
	dispose () final override
	{
		removeObject (getValue ());
		setObject (nullptr);

		X3DField <ValueType*>::dispose ();
	}

	virtual
	~X3DPtr ()
	{ removeObject (getValue ()); }


private:

	template <class Up>
	friend class X3DPtr;

	using X3DField <ValueType*>::reset;

	void
	addObject (ValueType* const value)
	{
		if (value)
			value -> addParent (this);
	}

	void
	moveObject (X3DPtr & field)
	{
		setObject (field .getValue ());

		if (getValue ())
		{
			field .get () -> replaceParent (&field, this);
			field .setObject (nullptr);
			field .addEvent ();
		}
	}

	template <class Up>
	void
	moveObject (X3DPtr <Up> & field)
	{
		setObject (dynamic_cast <ValueType*> (field .getValue ()));

		if (getValue ())
		{
			field .get () -> replaceParent (&field, this);
			field .setObject (nullptr);
			field .addEvent ();
		}
		else
			field = nullptr;
	}

	void
	removeObject (ValueType* const value)
	{
		if (value)
			value -> removeParent (this);
	}

	void
	setObject (ValueType* const value)
	{ X3DField <ValueType*>::set (value); }

	///  TypeName identifer for X3DFields.
	static const std::string typeName;

};

template <class ValueType>
const std::string X3DPtr <ValueType>::typeName ("SFNode");

///  @relates X3DPtr
///  @name Comparision operations

///  Compares two X3DPtr.
///  Returns true if @a lhs is equal to @a rhs.
template <class ValueType>
inline
bool
operator == (const X3DPtr <ValueType> & lhs, const X3DPtr <ValueType> & rhs)
{
	X3DBase* const a = lhs ? lhs -> getId () : nullptr;
	X3DBase* const b = rhs ? rhs -> getId () : nullptr;

	return a == b;
}

///  Compares two X3DPtr.
///  Returns true if @a lhs is not equal to @a rhs.
template <class ValueType>
inline
bool
operator not_eq (const X3DPtr <ValueType> & lhs, const X3DPtr <ValueType> & rhs)
{
	X3DBase* const a = lhs ? lhs -> getId () : nullptr;
	X3DBase* const b = rhs ? rhs -> getId () : nullptr;

	return a not_eq b;
}

///  Compares two X3DPtr.
///  Returns true if @a lhs less than @a rhs.
template <class ValueType>
inline
bool
operator < (const X3DPtr <ValueType> & lhs, const X3DPtr <ValueType> & rhs)
{
	X3DBase* const a = lhs ? lhs -> getId () : nullptr;
	X3DBase* const b = rhs ? rhs -> getId () : nullptr;

	return a < b;
}

///  Compares two X3DPtr.
///  Returns true if @a lhs less than equal to @a rhs.
template <class ValueType>
inline
bool
operator <= (const X3DPtr <ValueType> & lhs, const X3DPtr <ValueType> & rhs)
{
	X3DBase* const a = lhs ? lhs -> getId () : nullptr;
	X3DBase* const b = rhs ? rhs -> getId () : nullptr;

	return a <= b;
}

///  Compares two X3DPtr.
///  Returns true if @a lhs greater than @a rhs.
template <class ValueType>
inline
bool
operator > (const X3DPtr <ValueType> & lhs, const X3DPtr <ValueType> & rhs)
{
	X3DBase* const a = lhs ? lhs -> getId () : nullptr;
	X3DBase* const b = rhs ? rhs -> getId () : nullptr;

	return a > b;
}

///  Compares two X3DPtr.
///  Returns true if @a lhs greater than equal to @a rhs.
template <class ValueType>
inline
bool
operator >= (const X3DPtr <ValueType> & lhs, const X3DPtr <ValueType> & rhs)
{
	X3DBase* const a = lhs ? lhs -> getId () : nullptr;
	X3DBase* const b = rhs ? rhs -> getId () : nullptr;

	return a >= b;
}

} // X3D
} // titania

#endif
