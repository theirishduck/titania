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

#include "JavaScriptContext.h"

#include "String.h"
#include "jsBrowser.h"
#include "jsFields.h"
#include "jsGlobals.h"
#include "jsX3DConstants.h"
#include "jsfield.h"

namespace titania {
namespace X3D {

JSClass JavaScriptContext::global_class = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS

};

JavaScriptContext::JavaScriptContext (X3DScriptNode* script, const std::string & ecmascript, const basic::uri & uri, size_t index) :
	      X3DBaseNode (script -> getExecutionContext () -> getBrowser (), script -> getExecutionContext ()), 
	          X3DNode (),                                                                                    
	     X3DUrlObject (),                                                                                                                                                                 
	          context (NULL),                                                                                
	           global (NULL),                                                                                
	          browser (script -> getBrowser ()),                                                             
	           script (script),                                                                              
	         worldURL ({ uri }),                                                                             
	            index (index),                                                                               
	     initializeFn (),                                                                                    
	  prepareEventsFn (),                                                                                    
	eventsProcessedFn (),                                                                                    
	       shutdownFn (),                                                                                    
	           fields (),                                                                                    
	        functions (),                                                                                    
	       references (),                                                                                    
	            files ()                                                                                     
{
	setComponent ("Browser");
	setTypeName ("JavaScriptContext");

	addField (inputOutput, "metadata", metadata ());

	// Get a JS runtime.
	JSRuntime* runtime = getBrowser () -> getJavaScriptEngine () -> getRuntime ();

	if (runtime == NULL)
		return;

	// Create a context.
	context = JS_NewContext (runtime, 8192);

	if (context == NULL)
		return;

	JS_SetOptions (context, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT);
	JS_SetVersion (context, JSVERSION_LATEST);
	JS_SetErrorReporter (context, error);

	// Create the global object.
	global = JS_NewCompartmentAndGlobalObject (context, &global_class, NULL);

	if (global == NULL)
		return;

	initContext ();

	initNode ();

	if (not evaluate (ecmascript, uri .str ()))
	{
		dispose ();
		throw std::invalid_argument ("Couldn't evaluate script.");
	}

	initEventHandler ();
}

X3DBaseNode*
JavaScriptContext::create (X3DExecutionContext* const) const
{
	std::string ecmascript;

	script -> loadDocument (script -> url () [index], ecmascript);

	return new JavaScriptContext (script, ecmascript, worldURL .front (), index);
}

void
JavaScriptContext::initialize ()
{
	X3DNode::initialize ();
	X3DUrlObject::initialize ();
}

void
JavaScriptContext::initContext ()
{
	// Populate the global object with the standard globals, like Object and Array.
	if (not JS_InitStandardClasses (context, global))
		return;

	JS_SetContextPrivate (context, this);

	jsBrowser::defineObject (context, global);
	jsX3DConstants::defineObject (context, global);

	jsSFColor::init     (context, global);
	jsSFColorRGBA::init (context, global);
	jsSFImage::init     (context, global);
	jsSFMatrix3d::init  (context, global);
	jsSFMatrix3f::init  (context, global);
	jsSFMatrix4d::init  (context, global);
	jsSFMatrix4f::init  (context, global);
	jsSFNode::init      (context, global);
	jsSFRotation::init  (context, global);
	jsSFVec2d::init     (context, global);
	jsSFVec2f::init     (context, global);
	jsSFVec3d::init     (context, global);
	jsSFVec3f::init     (context, global);
	jsSFVec4d::init     (context, global);
	jsSFVec4f::init     (context, global);
	jsVrmlMatrix::init  (context, global);

	jsMFBool::init      (context, global);
	jsMFColor::init     (context, global);
	jsMFColorRGBA::init (context, global);
	jsMFDouble::init    (context, global);
	jsMFFloat::init     (context, global);
	jsMFImage::init     (context, global);
	jsMFInt32::init     (context, global);
	jsMFMatrix3d::init  (context, global);
	jsMFMatrix3f::init  (context, global);
	jsMFMatrix4d::init  (context, global);
	jsMFMatrix4f::init  (context, global);
	jsMFNode::init      (context, global);
	jsMFRotation::init  (context, global);
	jsMFString::init    (context, global);
	jsMFTime::init      (context, global);
	jsMFVec2d::init     (context, global);
	jsMFVec2f::init     (context, global);
	jsMFVec3d::init     (context, global);
	jsMFVec3f::init     (context, global);
	jsMFVec4d::init     (context, global);
	jsMFVec4f::init     (context, global);

	jsGlobals::init (context, global);
}

void
JavaScriptContext::initNode ()
{
	for (auto & field : script -> getUserDefinedFields ())
	{
		switch (field -> getAccessType ())
		{
			case initializeOnly :
			case outputOnly     :
				{
					addUserDefinedField (field);
					defineProperty (context, global, field, field -> getName (), JSPROP_ENUMERATE);
					break;
				}
			case inputOnly:
				break;
			case inputOutput:
			{
				addUserDefinedField (field);
				defineProperty (context, global, field, field -> getName (),              JSPROP_ENUMERATE);
				defineProperty (context, global, field, "set_" + field -> getName (),     0);
				defineProperty (context, global, field, field -> getName () + "_changed", 0);
				break;
			}
		}
	}
}

void
JavaScriptContext::addUserDefinedField (X3DFieldDefinition* const field)
{
	switch (field -> getType ())
	{
		case X3DConstants::SFBool:
		case X3DConstants::SFDouble:
		case X3DConstants::SFFloat:
		case X3DConstants::SFInt32:
		case X3DConstants::SFString:
		case X3DConstants::SFTime:
			break;
		default:
		{
			jsval vp;

			if (JS_NewFieldValue (context, field, &vp))
			{
				fields [field -> getName ()] = vp;
				JS_AddValueRoot (context, &fields [field -> getName ()]);
			}

			break;
		}
	}
}

void
JavaScriptContext::defineProperty (JSContext* context,
                                   JSObject* obj,
                                   X3DFieldDefinition* const field,
                                   const std::string & name,
                                   uintN attrs)
{
	switch (field -> getType ())
	{
		case X3DConstants::SFBool:
		case X3DConstants::SFDouble:
		case X3DConstants::SFFloat:
		case X3DConstants::SFInt32:
		case X3DConstants::SFString:
		case X3DConstants::SFTime:
			JS_DefineProperty (context,
			                   obj, name .c_str (),
			                   JSVAL_VOID,
			                   getBuildInProperty, setProperty,
			                   JSPROP_PERMANENT | JSPROP_SHARED | attrs);
			break;
		default:
			JS_DefineProperty (context,
			                   obj, name .c_str (),
			                   JSVAL_VOID,
			                   getProperty, setProperty,
			                   JSPROP_PERMANENT | JSPROP_SHARED | attrs);
			break;
	}
}

void
JavaScriptContext::initEventHandler ()
{
	initializeFn      = getFunction ("initialize");
	prepareEventsFn   = getFunction ("prepareEvents");
	eventsProcessedFn = getFunction ("eventsProcessed");
	shutdownFn        = getFunction ("shutdown");

	for (auto & field : script -> getUserDefinedFields ())
	{
		switch (field -> getAccessType ())
		{
			case inputOnly :
				{
					jsval function = getFunction (field -> getName ());

					if (not JSVAL_IS_VOID (function))
					{
						functions [field] = function;
						field -> addInterest (this, &JavaScriptContext::set_field, field);
					}

					break;
				}
			default:
				break;
		}
	}
}

JSBool
JavaScriptContext::getBuildInProperty (JSContext* context, JSObject* obj, jsid id, jsval* vp)
{
	jsval name;

	if (JS_IdToValue (context, id, &name))
	{
		JavaScriptContext*  javaScript = static_cast <JavaScriptContext*> (JS_GetContextPrivate (context));
		X3DScriptNode*      script     = javaScript -> getNode ();
		X3DFieldDefinition* field      = script -> getField (JS_GetString (context, name));

		return JS_NewFieldValue (context, field, vp);
	}

	return JS_TRUE;
}

JSBool
JavaScriptContext::getProperty (JSContext* context, JSObject* obj, jsid id, jsval* vp)
{
	jsval name;

	if (JS_IdToValue (context, id, &name))
	{
		JavaScriptContext* javaScript = static_cast <JavaScriptContext*> (JS_GetContextPrivate (context));

		*vp = javaScript -> fields [JS_GetString (context, name)];
		return JS_TRUE;
	}

	return JS_TRUE;
}

JSBool
JavaScriptContext::setProperty (JSContext* context, JSObject* obj, jsid id, JSBool strict, jsval* vp)
{
	jsval name;

	if (JS_IdToValue (context, id, &name))
	{
		X3DScriptNode* script = static_cast <JavaScriptContext*> (JS_GetContextPrivate (context)) -> getNode ();

		X3DFieldDefinition* field = script -> getField (JS_GetString (context, name));

		return JS_ValueToField (context, field, vp);
	}

	return JS_TRUE;
}

JSBool
JavaScriptContext::require (const basic::uri & uri, jsval & rval)
{
	try
	{
		// Resolve uri

		basic::uri base = worldURL .back () .size ()
		                  ? worldURL .back ()
								: getExecutionContext () -> getWorldURL ();

		basic::uri resolvedURL = transformURI (base, uri);

		// Get cached result

		auto file = files .find (resolvedURL);

		if (file not_eq files .end ())
		{
			rval = file -> second;
			return JS_TRUE;
		}

		// Load document

		std::string document = loadDocument (resolvedURL);

		// Evaluate script

		worldURL .emplace_back (resolvedURL);

		auto success = evaluate (document, resolvedURL, rval);

		worldURL .pop_back ();

		// Cache result

		if (success)
		{
			JS_AddValueRoot (context, &rval);

			files .insert (std::make_pair (resolvedURL, rval));

			return JS_TRUE;
		}
	}
	catch (const X3DError &)
	{ }

	return JS_FALSE;
}

JSBool
JavaScriptContext::evaluate (const std::string & string, const std::string & filename)
{
	jsval rval;

	return evaluate (string, filename, rval);
}

JSBool
JavaScriptContext::evaluate (const std::string & string, const std::string & filename, jsval & rval)
{
	return JS_EvaluateScript (context, global,
	                          string .c_str (), string .length (),
	                          filename .size () ? filename .c_str () : NULL,
	                          1,
	                          &rval);
}

void
JavaScriptContext::set_field (X3DFieldDefinition* field)
{
	jsval argv [2];

	JS_NewFieldValue (context, field, &argv [0], true);
	JS_NewNumberValue (context, browser -> getCurrentTime (), &argv [1]);

	jsval rval;
	JS_CallFunctionValue (context, global, functions [field], 2, argv, &rval);
	
	JS_MaybeGC (context);
	//JS_GC (context);
}

void
JavaScriptContext::set_initialized ()
{
	if (not JSVAL_IS_VOID (initializeFn))
		callFunction (initializeFn);
}

void
JavaScriptContext::prepareEvents ()
{
	if (not JSVAL_IS_VOID (prepareEventsFn))
		callFunction (prepareEventsFn);
}

void
JavaScriptContext::eventsProcessed ()
{
	if (not JSVAL_IS_VOID (eventsProcessedFn))
		callFunction (eventsProcessedFn);
}

void
JavaScriptContext::shutdown ()
{
	if (not JSVAL_IS_VOID (shutdownFn))
		callFunction (shutdownFn);
}

jsval
JavaScriptContext::getFunction (const std::string & name)
{
	jsval     function = JSVAL_VOID;
	JSObject* objp     = NULL;

	JSBool result = JS_GetMethod (context, global, name .c_str (), &objp, &function);

	if (result)
		return function;

	return JSVAL_VOID;
}

void
JavaScriptContext::callFunction (const std::string & name)
{
	jsval     function = JSVAL_VOID;
	JSObject* objp     = NULL;

	JSBool result = JS_GetMethod (context, global, name .c_str (), &objp, &function);

	if (not result or JSVAL_IS_VOID (function))
		return;

	callFunction (function);
}

void
JavaScriptContext::callFunction (jsval function)
{
	jsval rval;

	JS_CallFunctionValue (context, global, function, 0, NULL, &rval);

	JS_MaybeGC (context);
	//JS_GC (context);
}

void
JavaScriptContext::addField (X3DFieldDefinition* field)
{
	auto reference = references .find (field);

	if (reference == references .end ())
	{
		references .insert (std::make_pair (field, 1));

		field -> addParent (this);
	}
	else
		++ reference -> second;
}

void
JavaScriptContext::removeField (X3DFieldDefinition* field)
{
	auto reference = references .find (field);

	if (-- reference -> second == 0)
	{
		references .erase (reference);

		field -> removeParent (this);
	}
}

void
JavaScriptContext::error (JSContext* context, const char* message, JSErrorReport* report)
{
	JavaScriptContext* javaScript = static_cast <JavaScriptContext*> (JS_GetContextPrivate (context));
	X3DScriptNode*     script     = javaScript -> getNode ();
	X3DBrowser*        browser    = script -> getBrowser ();

	// Get script

	std::string ecmascript;

	if (report -> filename)
	{
		try
		{
			ecmascript = script -> loadDocument (report -> filename);
		}
		catch (const X3DError &)
		{ }
	}

	else
		script -> loadDocument (script -> url () [javaScript -> index], ecmascript);

	// Find error line

	std::string line = "Couldn't load file!";

	if (ecmascript .size ())
	{
		char nl = ecmascript .find ('\n', 0) == String::npos ? '\r' : '\n';

		std::string::size_type start = 0;
		std::string::size_type end   = 0;
		size_t                 i     = 0;

		for ( ; i < report -> lineno - 1; ++ i)
		{
			if ((start = ecmascript .find (nl, start)) == String::npos)
				break;

			else
				++ start;
		}

		if (start not_eq String::npos)
		{
			if ((end = ecmascript .find (nl, start)) == String::npos)
				end = ecmascript .length ();
				
			line = ecmascript .substr (start, end - start);
		}
	}

	// Pretty print error

	browser -> print ("# Javascript: runtime error at line ", report -> lineno, ":", '\n',
	                  "# in url '", (report -> filename ? report -> filename : "<inline>"), "' from Script '", script -> getName (), '\n',
	                  "# World URL is '", script -> getExecutionContext () -> getWorldURL (), "'", '\n',
	                  "# ", '\n',
	                  "# ", message, '\n',
	                  "#  ", line, '\n');
}

void
JavaScriptContext::dispose ()
{
	shutdown ();

	for (auto & field : fields)
		JS_RemoveValueRoot (context, &field .second);

	for (auto & file : files)
		JS_RemoveValueRoot (context, &file .second);

	// Cleanup.
	JS_DestroyContext (context);

	//assert (references .size () == 0);

	X3DUrlObject::dispose ();
	X3DNode::dispose ();
}

JavaScriptContext::~JavaScriptContext ()
{ }

} // X3D

} // titania
