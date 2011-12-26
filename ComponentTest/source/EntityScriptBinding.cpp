/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionAngelScriptComponent.h"

#include "FusionEntity.h"

namespace FusionEngine
{

	void Entity_GetPosition(Vector2 &out, Entity *entity)
	{
		out = entity->GetPosition();
	}

	void Entity_SetPosition(float x, float y, Entity *entity)
	{
		entity->SetPosition(Vector2(x, y));
	}

	class ASComponentFuture : public RefCounted
	{
	public:
		ASComponentFuture()
			: component(nullptr)
		{}

		ASComponentFuture(IComponent* com)
			: component(com)
		{}

		IComponent* component;

		static void Register(asIScriptEngine* engine);
		
	};

	static IComponent* ASComponentFuture_GetComponent(ASComponentFuture* obj)
	{
		return obj->component;
	}

	void ASComponentFuture::Register(asIScriptEngine* engine)
	{
		ASComponentFuture::RegisterType<ASComponentFuture>(engine, "ComponentFuture");

		int r;
		r = engine->RegisterObjectBehaviour("ComponentFuture", asBEHAVE_IMPLICIT_REF_CAST, "IComponent@ f()", asFUNCTION(ASComponentFuture_GetComponent), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("ComponentFuture", "IComponent@ get()", asFUNCTION(ASComponentFuture_GetComponent), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

	static ASComponentFuture* Entity_GetComponent(EntityPtr* obj, std::string type, std::string ident)
	{
		auto entity = *obj;
		auto com = entity->GetComponent(type, ident);

		auto future = new ASComponentFuture();

		if (com)
		{
			com->addRef();
			future->component = com.get();
		}
		else
		{
			ASScript::GetActiveScript()->YieldUntil([entity, type, ident, future]()->bool
			{
				auto com = entity->GetComponent(type, ident);
				if (com)
				{
					com->addRef();
					future->component = com.get();
					return true;
				}
				else
					return false;
			},
#ifdef PROFILE_BUILD
				20.0f);
#else
				5.f);
#endif
		}
		
		return future;
	}

	static ASComponentFuture* Entity_GetComponentB(EntityPtr* obj, std::string type)
	{
		return Entity_GetComponent(obj, type, std::string());
	}

	static bool Entity_IsNull(EntityPtr* obj)
	{
		return !(*obj);
	}

	static bool Entity_OpEquals(EntityPtr* obj, const EntityPtr& other)
	{
		return *obj == other;
	}

	static PlayerID Entity_GetOwnerID(EntityPtr* obj)
	{
		return (*obj)->GetOwnerID();
	}

	static bool Entity_InputIsActive(const std::string& input, EntityPtr* entity)
	{
		return (*entity)->InputIsActive(input);
	}

	static float Entity_InputGetPosition(const std::string& input, EntityPtr* entity)
	{
		return (*entity)->GetInputPosition(input);
	}

	//static PlayerInput* Entity_GetInput(EntityPtr* entity)
	//{
	//	return (*entity)->m_PlayerInput.get();
	//}

	void Entity::Register(asIScriptEngine *engine)
	{
		RegisterSharedPtrType<Entity>("Entity", engine);

		{
			typedef Scripting::Registation::ValueTypeHelper<std::weak_ptr<Entity>> helper_type;

			int r;
			r = engine->RegisterObjectType("EntityW", sizeof(std::weak_ptr<Entity>), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour("EntityW", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(helper_type::Construct), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour("EntityW", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(helper_type::Destruct), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("EntityW", "EntityW& opAssign(const EntityW &in other)",
				asMETHODPR(std::weak_ptr<Entity>, operator=, (const std::weak_ptr<Entity> &), std::weak_ptr<Entity> &), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("EntityW", "EntityW& opAssign(Entity &in other)",
				asMETHODPR(std::weak_ptr<Entity>, operator=, (EntityPtr &), std::weak_ptr<Entity> &), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("EntityW", "Entity lock() const",
				asMETHODPR(std::weak_ptr<Entity>, lock, () const, EntityPtr), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		}

		ASComponentFuture::Register(engine);

		int r;
		r = engine->RegisterObjectMethod("Entity",
			"ComponentFuture@ getComponent(string, string) const",
			asFUNCTION(Entity_GetComponent), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"ComponentFuture@ getComponent(string) const",
			asFUNCTION(Entity_GetComponentB), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"bool isNull() const",
			asFUNCTION(Entity_IsNull), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"PlayerID getOwnerID() const",
			asFUNCTION(Entity_GetOwnerID), asCALL_CDECL_OBJFIRST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Entity", "bool opEquals(const Entity &in other)",
			asFUNCTION(Entity_OpEquals), asCALL_CDECL_OBJFIRST); FSN_ASSERT(r >= 0);

		//r = engine->RegisterObjectMethod("Entity",
		//	"Input@ get_input() const",
		//	asFUNCTION(Entity_GetInput), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"bool inputIsActive(const string &in) const",
			asFUNCTION(Entity_InputIsActive), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"float inputGetPosition(const string &in) const",
			asFUNCTION(Entity_InputGetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"const string& getName() const",
			asMETHOD(Entity, GetName), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"uint16 getID() const",
			asMETHOD(Entity, GetID), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"void setDomain(uint8)",
			asMETHOD(Entity, SetDomain), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"uint8 getDomain() const",
			asMETHOD(Entity, GetDomain), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"void setOwnerID(uint8)",
			asMETHOD(Entity, SetOwnerID), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		//r = engine->RegisterObjectMethod("Entity",
		//	"uint8 getOwnerID() const",
		//	asMETHOD(Entity, GetOwnerID), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"bool isSynced() const",
			asMETHOD(Entity, IsSyncedEntity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		//r = engine->RegisterObjectMethod("Entity",
		//	"bool inputIsActive(const string &in) const",
		//	asMETHOD(Entity, InputIsActive), asCALL_THISCALL);
		//r = engine->RegisterObjectMethod("Entity",
		//	"float inputGetPosition(const string &in) const",
		//	asMETHOD(Entity, GetInputPosition), asCALL_THISCALL);

		// Physical state related methods
		r = engine->RegisterObjectMethod("Entity",
			"const Vector& getPosition()",
			asMETHOD(Entity, GetPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void getPosition(Vector &out)",
			asFUNCTION(Entity_GetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setPosition(const Vector &in)",
			asMETHOD(Entity, SetPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setPosition(float, float)",
			asFUNCTION(Entity_SetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"float getAngle() const",
			asMETHOD(Entity, GetAngle), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setAngle(float)",
			asMETHOD(Entity, SetAngle), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		//r = engine->RegisterInterface("IEntity"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void OnSpawn()"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void Update()"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void Draw()"); FSN_ASSERT(r >= 0);
	}

}
