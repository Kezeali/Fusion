/*
*  Copyright (c) 2012-2013 Fusion Project Team
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

#include "FusionScriptTypeRegistrationUtils.h"

#include "FusionThreadSafeProperty.h"

#include "FusionAngelScriptComponent.h"
#include "FusionBox2DComponent.h"
#include "FusionCLRenderComponent.h"
#include "FusionTransformComponent.h"

#define FSN_REGISTER_PROP_ACCESSORA(iface, type, scriptType, prop) \
	ThreadSafeProperty<iface, type>::RegisterProp(engine, scriptType);\
	engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "const " scriptType " &get_" #prop "() const", asMETHOD(iface, get_ ## prop ), asCALL_THISCALL)

namespace FusionEngine
{

	template <class IFaceT>
	IFaceT* GetIface(void* obj)
	{
		auto ifaceObj = dynamic_cast<IFaceT*>(static_cast<EntityComponent*>(obj));
		//FSN_ASSERT_MSG(ifaceObj, "The given component doesn't implement the expected interface");
		if (ifaceObj)
			return ifaceObj;
		else
			return static_cast<IFaceT*>(obj);
	}

}

//! Macro that defines the struct used by FSN_REGISTER_PROP_ACCESSOR
#define FSN_PROP_REGISTRATION_HELPER_STRUCT(iface, type, prop) \
struct iface##_##prop {\
	static ComponentProperty* get_ ## prop(void *obj) { return GetIface<iface>(obj)->prop.GetInterfaceObject(); }\
	static type get_ ## prop ## Value(void *obj) { return GetIface<iface>(obj)->prop.Get(); }\
	static void set_ ## prop ## Value(const type & value, void *obj) { return GetIface<iface>(obj)->prop.Set(value); }\
};

//! Macro to define the script interface for accessing properties on application-code components
/*!
* Uses a local class to define a method inline (within the method using this macro), and that method is
* then registered with the script engine to access the property in question.
*/
#define FSN_REGISTER_PROP_ACCESSOR(iface, type, scriptType, prop) \
FSN_PROP_REGISTRATION_HELPER_STRUCT(iface, type, prop)\
{\
	int r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "PropertyAny@ get_" #prop "Prop()", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
	r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), scriptType " get_" #prop "()", asFUNCTION(iface##_##prop :: get_ ## prop ## Value ), asCALL_CDECL_OBJLAST);\
	r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "void set_" #prop "(const " scriptType " &in)", asFUNCTION(iface##_##prop :: set_ ## prop ## Value ), asCALL_CDECL_OBJLAST);\
	FSN_ASSERT(r >= 0);\
}

//! Read-only version of FSN_REGISTER_PROP_ACCESSOR
#define FSN_REGISTER_PROP_ACCESSOR_R(iface, type, scriptType, prop) \
FSN_PROP_REGISTRATION_HELPER_STRUCT(iface, type, prop)\
{\
	int r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "const PropertyAny@ get_" #prop "Prop() const", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
	r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), scriptType " get_" #prop "()", asFUNCTION(iface##_##prop :: get_ ## prop ## Value ), asCALL_CDECL_OBJLAST);\
	FSN_ASSERT(r >= 0);\
}

namespace FusionEngine
{

	void ITransform_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<ITransform>(engine);

		FSN_REGISTER_PROP_ACCESSOR(ITransform, Vector2, "Vector", Position);
		FSN_REGISTER_PROP_ACCESSOR(ITransform, float, "float", Angle);		
		FSN_REGISTER_PROP_ACCESSOR(ITransform, int, "int", Depth);
	}

	void IRigidBody_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<IRigidBody>(engine);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", Interpolate);

		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, float, "float", Mass);
		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, float, "float", Inertia);
		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, Vector2, "Vector", CenterOfMass);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, Vector2, "Vector", Velocity);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", AngularVelocity);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", LinearDamping);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", AngularDamping);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", GravityScale);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", Active);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", SleepingAllowed);
		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, bool, "bool", Awake);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", Bullet);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", FixedRotation);
	}

	void IFixture_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<IFixture>(engine);

		FSN_REGISTER_PROP_ACCESSOR(IFixture, bool, "bool", Sensor);
		FSN_REGISTER_PROP_ACCESSOR(IFixture, float, "float", Density);
		FSN_REGISTER_PROP_ACCESSOR(IFixture, float, "float", Friction);
		FSN_REGISTER_PROP_ACCESSOR(IFixture, float, "float", Restitution);
	}

	void ICircleShape_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<ICircleShape>(engine);

		FSN_REGISTER_PROP_ACCESSOR(ICircleShape, Vector2, "Vector", Position);
		FSN_REGISTER_PROP_ACCESSOR(ICircleShape, float, "float", Radius);
	}

	void IRenderCom_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<IRenderCom>(engine);

		FSN_REGISTER_PROP_ACCESSOR(IRenderCom, Vector2, "Vector", Offset);
		FSN_REGISTER_PROP_ACCESSOR(IRenderCom, int, "int", LocalDepth);
	}

	void ISprite_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<ISprite>(engine);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, Vector2, "Vector", Offset);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, int, "int", LocalDepth);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, std::string, "string", ImagePath);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, std::string, "string", AnimationPath);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, clan::Origin, "PointOrigin", AlignmentOrigin);
		//FSN_REGISTER_PROP_ACCESSOR(ISprite, Vector2i, "VectorInt", AlignmentOffset);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, clan::Origin, "PointOrigin", RotationOrigin);
		//FSN_REGISTER_PROP_ACCESSOR(ISprite, Vector2i, "VectorInt", RotationOffset);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, clan::Colorf, "Colour", Colour);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, float, "float", Alpha);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, Vector2, "Vector", Scale);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, float, "float", BaseAngle);

		FSN_REGISTER_PROP_ACCESSOR_R(ISprite, bool, "bool", AnimationFinished);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, int, "int", AnimationFrame);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, bool, "bool", Playing);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, bool, "bool", Looping);
	}

	void IScript_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<IScript>(engine);

		//FSN_REGISTER_PROP_ACCESSOR(IScript, std::string, "string", ScriptPath);
	}

	void ICamera_RegisterScriptInterface(asIScriptEngine* engine)
	{
		RegisterComponentInterfaceType<ICamera>(engine);

		engine->RegisterEnum("SyncType");
		engine->RegisterEnumValue("SyncType", "NoSync", ICamera::SyncTypes::NoSync);
		engine->RegisterEnumValue("SyncType", "Owned", ICamera::SyncTypes::NoSync);
		engine->RegisterEnumValue("SyncType", "Shared", ICamera::SyncTypes::NoSync);

		FSN_REGISTER_PROP_ACCESSOR(ICamera, ICamera::SyncTypes, "SyncType", SyncType);
		FSN_REGISTER_PROP_ACCESSOR(ICamera, bool, "bool", ViewportEnabled);
		FSN_REGISTER_PROP_ACCESSOR(ICamera, clan::Rectf, "Rect", ViewportRect);
		FSN_REGISTER_PROP_ACCESSOR(ICamera, bool, "bool", AngleEnabled);
	}

}
