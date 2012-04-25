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

	static ComponentProperty& ComponentPropertyT_opAssign(ComponentProperty* other, ComponentProperty* obj)
	{
		if (other->GetImpl()->GetTypeId() == obj->GetImpl()->GetTypeId())
			obj->GetImpl()->Set(other->Get(), obj->GetImpl()->GetTypeId());
		else
			asGetActiveContext()->SetException("Incompatible Property types for opAssign ('prop = prop')");
		return *obj;
	}
	
	static ComponentProperty& ComponentPropertyT_opAssign(void* ref, ComponentProperty* obj)
	{
		obj->GetImpl()->Set(ref, obj->GetImpl()->GetTypeId());
		return *obj;
	}

	static void* ComponentPropertyT_ImplicitCast(ComponentProperty* obj)
	{
		return obj->Get();
	}

	static void GeneratePropertySpecialisation(asIScriptEngine* engine, const std::string& subtype)
	{
		int r;
		const auto type = "Property<" + subtype + ">";
		const auto c_type = type.c_str();
		r = engine->RegisterObjectType(c_type, 0, asOBJ_REF); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod(c_type, ("void follow(" + type + "@)").c_str(), asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(c_type, (type + " &opAssign(" + type + "@)").c_str(), asFUNCTIONPR(ComponentPropertyT_opAssign, (ComponentProperty*, ComponentProperty*), ComponentProperty&), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(c_type, (type + " &opAssign(const " + subtype + " &in)").c_str(), asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty&), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectBehaviour(c_type, asBEHAVE_IMPLICIT_VALUE_CAST,
			(subtype + " f() const").c_str(), asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(c_type, ("const " + subtype + "& get_value() const").c_str(), asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(c_type, ("void set_value(const " + subtype + " &in)").c_str(), asMETHOD(ComponentProperty, Set), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	void ComponentProperty::Register(asIScriptEngine* engine)
	{
		int r;

		RegisterType<ComponentProperty>(engine, "IProperty");

		r = engine->RegisterObjectMethod("IProperty", "void follow(IProperty@)", asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		//r = engine->RegisterObjectMethod("IProperty", "IProperty& opAssign(IProperty@)", asFUNCTIONPR(ComponentPropertyT_opAssign, (ComponentProperty*, ComponentProperty*), ComponentProperty&), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectType("Property<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Property<T>", "void follow(Property<T>@)", asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "Property<T> &opAssign(Property<T>@)", asFUNCTIONPR(ComponentPropertyT_opAssign, (ComponentProperty*, ComponentProperty*), ComponentProperty&), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "Property<T> &opAssign(const T &in)", asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty&), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectBehaviour("Property<T>", asBEHAVE_IMPLICIT_VALUE_CAST,
			"T f() const", asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Property<T>", "const T &get_value() const", asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<T>", "void set_value(const T &in)", asMETHOD(ComponentProperty, Set), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		// Specialisation for bool (no implicit cast)
		r = engine->RegisterObjectType("Property<bool>", 0, asOBJ_REF); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Property<bool>", "void follow(Property<bool>@)", asFUNCTION(ComponentProperty_follow), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "Property<bool> &opAssign(Property<bool>@)", asFUNCTIONPR(ComponentPropertyT_opAssign, (ComponentProperty*, ComponentProperty*), ComponentProperty&), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "Property<bool> &opAssign(bool)", asFUNCTIONPR(ComponentPropertyT_opAssign, (void*, ComponentProperty*), ComponentProperty&), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Property<bool>", "const bool& get_value() const", asMETHOD(ComponentProperty, Get), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Property<bool>", "void set_value(const bool &in)", asMETHOD(ComponentProperty, Set), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		//struct Gen
		//{
		//	asIScriptEngine* engine;
		//	template <typename T>
		//	void operator() (T) { GeneratePropertySpecialisation(engine, AppType<T>::name); }
		//};
		//Gen generator; generator.engine = engine;
		//boost::mpl::for_each<Scripting::FundimentalTypes>(generator);

		// Type conversion
		RegisterBaseOf<ComponentProperty, ComponentProperty>(engine, "IProperty", "Property<T>");
	}

}
