/*
*  Copyright (c) 2012 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionComponentProperty.h"

#include "FusionPropertySignalingSystem.h"

#include <boost/mpl/for_each.hpp>
#include "FusionCommonAppTypes.h"

namespace FusionEngine
{

	static void ComponentProperty_follow(ComponentProperty* other, ComponentProperty* obj)
	{
		obj->Follow(EvesdroppingManager::getSingleton().GetSignalingSystem(), other->GetID());
	}

	static ComponentProperty* ComponentPropertyT_opAssign(ComponentProperty* other, ComponentProperty* obj)
	{
		if (other->GetImpl()->GetTypeId() == obj->GetImpl()->GetTypeId())
			obj->GetImpl()->Set(other->Get(), obj->GetImpl()->GetTypeId());
		else
			asGetActiveContext()->SetException("Incompatible Property types for opAssign ('prop = prop')");
		return obj;
	}
	
	static ComponentProperty* ComponentPropertyT_opAssign(void* ref, ComponentProperty* obj)
	{
		obj->GetImpl()->Set(ref, obj->GetImpl()->GetTypeId());
		return obj;
	}

	static ComponentProperty* ComponentPropertyT_opAssign_r(void* ref, ComponentProperty* obj)
	{
		*(void**)ref = obj->GetImpl()->GetRef();
		return obj;
	}

	static void ComponentPropertyT_ImplicitCast(asIScriptGeneric* gen)
	{
		auto obj = static_cast<ComponentProperty*>(gen->GetObject());

		auto engine = gen->GetEngine();

		auto refTypeId = gen->GetReturnTypeId();

		if (obj->GetImpl()->GetTypeId() & asTYPEID_MASK_OBJECT)
		{
			// Is the object type compatible with the stored value?

			// Copy the object into the given reference
			if (obj->GetImpl()->GetTypeId() == refTypeId)
			{
				engine->AssignScriptObject(gen->GetAddressOfReturnLocation(), obj->GetImpl()->GetRef(), obj->GetImpl()->GetTypeId());
				return;
			}
		}
		else
		{
			// Is the primitive type compatible with the stored value?

			if (obj->GetImpl()->GetTypeId() == refTypeId)
			{
				int size = engine->GetSizeOfPrimitiveType(refTypeId);
				memcpy(gen->GetAddressOfReturnLocation(), obj->GetImpl()->GetRef(), size);
				return;
			}
		}

		asGetActiveContext()->SetException("Invalid implicit value cast for the given Property type");
	}

	ComponentProperty *ComponentPropertyT_factory(asIObjectType *type)
	{
		return new ComponentProperty();
	}
	
	ComponentProperty *ComponentPropertyT_factory()
	{
		return new ComponentProperty();
	}

	void ComponentPropertyT_refcast(void* out, int type_id, ComponentProperty* obj)
	{
		if (obj)
		{
			FSN_ASSERT(type_id & asTYPEID_OBJHANDLE);
			
			const auto outType = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeById(type_id);
			if (outType && outType->GetSubTypeId() == obj->GetImpl()->GetTypeId())
			{
				obj->addRef();
				*(void**)out = obj;
			}
		}
	}

	ComponentProperty* ComponentPropertyTToPlaceholder(void* in, int type_id)
	{
		FSN_ASSERT(type_id & asTYPEID_OBJHANDLE);

		auto obj = static_cast<ComponentProperty*>(in);
		if (obj)
		{
			obj->addRef();
		}
		return obj;
	}

	void ComponentPropertyT_RegisterRefCast(asIScriptEngine *engine, const std::string &base, const std::string &derived)
	{
		int r;
		r = engine->RegisterObjectBehaviour(base.c_str(), asBEHAVE_REF_CAST,
			"void f(?&out)", asFUNCTION(ComponentPropertyT_refcast),
			asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectBehaviour(derived.c_str(), asBEHAVE_IMPLICIT_REF_CAST,
			(base + "@ f()").c_str(), asFUNCTION((convert_ref<ComponentProperty, ComponentProperty>)),
			asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	}

	void ComponentPropertyT_RegisterConversion(asIScriptEngine *engine, const std::string &base, const std::string &derived)
	{
		int r;
		r = engine->RegisterObjectMethod(base.c_str(),
			"void convert_into(?&out)", asFUNCTION(ComponentPropertyT_refcast),
			asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		// TODO: enable this when it doesn't cause the script engine to crash on destruction
		//r = engine->RegisterObjectMethod(derived.c_str(),
		//	"PropertyAny@ to_placeholder()", asFUNCTION((convert_ref<ComponentProperty, ComponentProperty>)),
		//	asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterGlobalFunction("PropertyAny@ to_placeholder(?&in)",
			asFUNCTION(ComponentPropertyTToPlaceholder),
			asCALL_CDECL); FSN_ASSERT( r >= 0 );
	}

	static void GeneratePropertySpecialisation(asIScriptEngine* engine, const std::string& subtype)
	{
		int r;
		const auto type = "Property<" + subtype + ">";
		const auto c_type = type.c_str();
		r = engine->RegisterObjectType(c_type, 0, asOBJ_REF); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(c_type, ("void follow(" + type + "@)").c_str(), asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(c_type, (type + "@ opAssign(" + type + "@)").c_str(), asFUNCTIONPR(ComponentPropertyT_opAssign, (ComponentProperty*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(c_type, (type + "@ opAssign(const " + subtype + " &in)").c_str(), asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectBehaviour(c_type, asBEHAVE_IMPLICIT_VALUE_CAST,
			(subtype + " f() const").c_str(), asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(c_type, ("const " + subtype + "& get_value() const").c_str(), asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(c_type, ("void set_value(const " + subtype + " &in)").c_str(), asMETHOD(ComponentProperty, Set), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	void ComponentProperty::Register(asIScriptEngine* engine)
	{
		FSN_ASSERT( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == NULL );

		int r;

		// Property wrapper - just allows properties to be passed to methods and followed
		RegisterType<ComponentProperty>(engine, "PropertyAny");

		r = engine->RegisterObjectMethod("PropertyAny", "void follow(PropertyAny@)", asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		// Typed property wrapper
		r = engine->RegisterObjectType("Property<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE); FSN_ASSERT( r >= 0 );
		
		r = engine->RegisterObjectBehaviour("Property<T>", asBEHAVE_FACTORY, "Property<T>@ f(int &in)", asFUNCTIONPR(ComponentPropertyT_factory, (asIObjectType*), ComponentProperty*), asCALL_CDECL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Property<T>", asBEHAVE_ADDREF, "void addref()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
		r = engine->RegisterObjectBehaviour("Property<T>", asBEHAVE_RELEASE, "void release()", asMETHOD(RefCounted, release), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Property<T>", "void follow(Property<T>@)", asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "Property<T> &opAssign(const Property<T> &in)", asFUNCTIONPR(ComponentPropertyT_opAssign, (ComponentProperty*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "void opAssign(const T &in)", asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "void opShl(const T &in)", asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "void opShl_r(T &out)", asFUNCTIONPR(ComponentPropertyT_opAssign_r, (void*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectBehaviour("Property<T>", asBEHAVE_IMPLICIT_VALUE_CAST,
			"T f() const", asFUNCTION(ComponentPropertyT_ImplicitCast), asCALL_GENERIC); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Property<T>", "const T &get_value() const", asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "void set_value(const T &in)", asMETHOD(ComponentProperty, Set), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		// Specialisation for bool (no implicit cast)
		r = engine->RegisterObjectType("Property<bool>", 0, asOBJ_REF); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectBehaviour("Property<bool>", asBEHAVE_FACTORY, "Property<bool> @f()", asFUNCTIONPR(ComponentPropertyT_factory, (void), ComponentProperty*), asCALL_CDECL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectBehaviour("Property<bool>", asBEHAVE_ADDREF, "void addref()", asMETHOD(RefCounted, addRef), asCALL_THISCALL);
		r = engine->RegisterObjectBehaviour("Property<bool>", asBEHAVE_RELEASE, "void release()", asMETHOD(RefCounted, release), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Property<bool>", "void follow(Property<bool>@)", asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "Property<bool> &opAssign(const Property<bool> &in)", asFUNCTIONPR(ComponentPropertyT_opAssign, (ComponentProperty*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "void opAssign(const bool &in)", asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "void opShl(bool)", asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "void opShl_r(bool &out)", asFUNCTIONPR(ComponentPropertyT_opAssign_r, (void*, ComponentProperty*), ComponentProperty*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Property<bool>", "const bool& get_value() const", asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "void set_value(const bool &in)", asMETHOD(ComponentProperty, Set), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		// Type conversion
		ComponentPropertyT_RegisterConversion(engine, "PropertyAny", "Property<T>");
	}

}
