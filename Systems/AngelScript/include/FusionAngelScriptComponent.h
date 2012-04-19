/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#ifndef H_FusionAngelScriptComponent
#define H_FusionAngelScriptComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionScriptComponent.h"

#include "FusionSerialisationHelper.h"

#include "FusionResourcePointer.h"
#include "FusionScriptModule.h"
#include "FusionScriptReference.h"

#include "FusionTypes.h"

#include <tbb/concurrent_queue.h>
#include <physfs.h>

#ifdef Yield
#undef Yield
#endif

class CScriptAny;

namespace FusionEngine
{

	class ScriptCollisionEvent;
	class Box2DContactListener;
	class ASScriptB2ContactListener;

	class CLBinaryStream : public asIBinaryStream
	{
	public:
		enum OpenMode { open_read, open_write };

		CLBinaryStream(const std::string& filename, OpenMode open_mode);
		~CLBinaryStream();

		void Open(const std::string& filename, OpenMode open_mode);

		void Read(void *data, asUINT size);
		void Write(const void *data, asUINT size);

	private:
		PHYSFS_File *m_File;
	};

	//! Sprite resource loader callback
	void LoadScriptResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData);
	//! Sprite resource unloader callback
	void UnloadScriptResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData);

	class CoroutineSemaphore
	{
		virtual ~CoroutineSemaphore();
		virtual bool IsReady() = 0;
	};

	struct ConditionalCoroutine
	{
		ConditionalCoroutine() : timeout_time(std::numeric_limits<unsigned int>::max()) {}
		ConditionalCoroutine(ConditionalCoroutine&& other)
			: new_ctx(std::move(other.new_ctx)),
			condition(std::move(other.condition)),
			timeout_time(other.timeout_time)
		{}
		ConditionalCoroutine& operator= (ConditionalCoroutine&& other)
		{
			new_ctx = std::move(other.new_ctx);
			condition = std::move(other.condition);
			timeout_time = other.timeout_time;
			return *this;
		}
		//! Generally, this shouldn't be used
		ConditionalCoroutine(const ConditionalCoroutine& other) :
			new_ctx(other.new_ctx), condition(other.condition), timeout_time(other.timeout_time)
		{
#ifndef _WIN32
#warning copying ConditionalCoroutine;
#endif
		}
		//! Generally, this shouldn't be used
		ConditionalCoroutine& operator= (const ConditionalCoroutine& other)
		{
#ifndef _WIN32
#warning copying ConditionalCoroutine;
#endif
			new_ctx = other.new_ctx;
			condition = other.condition;
			timeout_time = other.timeout_time;
			return *this;
		}
		//! new_ctx is an optional new context (used when this is created as a new co-routine, rather than for a yield)
		boost::intrusive_ptr<asIScriptContext> new_ctx;
		std::function<bool (void)> condition;
		unsigned int timeout_time;

		void SetTimeout(float seconds)
		{
			timeout_time = CL_System::get_time() + (unsigned int)(seconds * 1000);
		}

		bool IsReady() const
		{
			if (!condition)
			{
				return timeout_time == std::numeric_limits<unsigned int>::max() || CL_System::get_time() >= timeout_time;
			}
			else
			{
				return condition()
					|| (timeout_time != std::numeric_limits<unsigned int>::max() && CL_System::get_time() >= timeout_time);
			}
		}
	};

	//! Angelscript component
	class ASScript : public IComponent, public IScript
	{
		friend class AngelScriptWorld;
		friend class AngelScriptTask;
		friend class AngelScriptTaskB;
	public:
		FSN_LIST_INTERFACES((IScript))

		struct PropsIdx { enum Names : size_t {
			ScriptPath,
			Unused,
			NumProps
		}; };
		typedef SerialisationHelper<
			std::string, std::string> // ScriptPath
			DeltaSerialiser_t;
		static_assert(PropsIdx::NumProps == DeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		//! CTOR
		ASScript();
		//! DTOR
		virtual ~ASScript();

		std::shared_ptr<Box2DContactListener> GetContactListener();

		bool HasContactListener() { return (bool)m_ContactListener; }

		bool PopCollisionEnterEvent(boost::intrusive_ptr<ScriptCollisionEvent>& ev);
		bool PopCollisionExitEvent(boost::intrusive_ptr<ScriptCollisionEvent>& ev);

		boost::intrusive_ptr<asIScriptContext> PrepareMethod(ScriptManager* script_manager, const std::string& decl);
		boost::intrusive_ptr<asIScriptContext> PrepareMethod(ScriptManager* script_manager, int id);

		int GetMethodId(const std::string& decl);

		void MakeFollower(PropertySignalingSystem_t& system, PropertyID id, const std::string& decl);

		//! Enable/disable auto-yield
		void SetAutoYield(const bool value) { m_AutoYield = value; }
		//! Returns true if the script runner is allowed to force this script to yield at will
		bool AllowsAutoYield() const { return m_AutoYield; }

		void Yield();
		void YieldUntil(std::function<bool (void)> condition, float timeout = 0.f);
		void CreateCoroutine(const std::string& functionName, float delay = 0.f);
		void CreateCoroutine(asIScriptFunction* function);
		CScriptAny* GetProperty(unsigned int index);
		bool SetProperty(unsigned int index, void* ref, int typeId);

		std::uint32_t CreateEntityWrapperId(const EntityPtr& entityReferenced);

		asIScriptObject* CreateEntityWrapper(const EntityPtr& entityReferenced);

		class ScriptInterface : public GarbageCollected<ScriptInterface>
		{
		public:
			ScriptInterface(ASScript* owner, asIScriptObject* script_object);

			void Yield();
			void YieldUntil(std::function<bool (void)> condition, float timeout = 0.f);
			void CreateCoroutine(const std::string& functionName, float delay = 0.f);
			void CreateCoroutine(asIScriptFunction* function);
			CScriptAny* GetProperty(unsigned int index);
			bool SetProperty(unsigned int index, void* ref, int typeId);

			void EnumReferences(asIScriptEngine *engine);
			void ReleaseAllReferences(asIScriptEngine *engine);

			static void Register(asIScriptEngine* engine);

			boost::intrusive_ptr<asIScriptObject> object; // An instance of the class that the script defines

			ASScript* component;
		};

		void SetScriptObject(asIScriptObject* obj, const std::vector<std::pair<std::string, std::string>>& interface_properties);

		const boost::intrusive_ptr<ScriptInterface>& GetScriptInterface() const { return m_ScriptObject; }

		boost::intrusive_ptr<asIScriptObject> GetScriptObject() const
		{
			const auto& scriptInterface = GetScriptInterface();
			if (scriptInterface)
				return scriptInterface->object;
			else
				return boost::intrusive_ptr<asIScriptObject>();
		}

		struct PropInfo
		{
			std::string name;
			int type_id;
		};
		const std::vector<PropInfo>& GetScriptProperties() const { return m_ScriptPropertyInfo; }

		void CheckChangedPropertiesIn();
		
		void OnModuleLoaded(ResourceDataPtr resource);

		asIScriptModule* GetScriptModule() const { return m_Module.Get(); }

		// Called after the script is loaded
		bool InitialiseEntityWrappers();

		static ASScript* GetActiveScript();
		//static void YeildActiveScript();
		static void YeildActiveScriptUntil(std::function<bool (void)> condition, float timeout = 0.f);

	private:
		// IComponent
		std::string GetType() const { return "ASScript"; }

		void OnSiblingAdded(const ComponentPtr& com);
		void OnSiblingRemoved(const ComponentPtr& com);

		void SerialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		void SerialiseOccasional(RakNet::BitStream& stream);
		void DeserialiseOccasional(RakNet::BitStream& stream);

		bool SerialiseProp(RakNet::BitStream& stream, CScriptAny* any);
		bool DeserialiseProp(RakNet::BitStream& stream, CScriptAny* any, size_t prop_index, const std::string& prop_name = std::string());
		void DeserNonStandardProp(RakNet::BitStream& stream, CScriptAny* any, size_t prop_index, const std::string& prop_name);
		//void CacheProp(CScriptAny* any, const std::string& prop_name);

		DeltaSerialiser_t m_DeltaSerialisationHelper;

		// IScript interface
		const std::string& GetScriptPath() const;
		void SetScriptPath(const std::string& path);

		struct ScriptMethodInfo
		{
			PersistentFollowerPtr persistentFollower; // Used to bind this method as a callback to a component property
			asIScriptFunction* function;
			int id;
		};

		ScriptMethodInfo* GetMethod(const std::string& decl);

		std::string m_Path;
		bool m_ReloadScript;

		bool m_ModuleReloaded;

		boost::signals2::connection m_ModuleLoadedConnection;
		ResourcePointer<asIScriptModule> m_Module;
		int m_EntityWrapperTypeId;
		std::map<std::string, ScriptMethodInfo> m_ScriptMethods;
		boost::intrusive_ptr<ScriptInterface> m_ScriptObject;
		std::vector<std::shared_ptr<IComponentProperty>> m_ScriptProperties;
		std::vector<PropInfo> m_ScriptPropertyInfo;

		std::shared_ptr<ASScriptB2ContactListener> m_ContactListener;

		IComponent::SerialiseMode m_LastDeserMode;
		std::vector<boost::intrusive_ptr<CScriptAny>> m_CachedProperties;
		std::map<std::string, boost::intrusive_ptr<CScriptAny>> m_EditableCachedProperties;
		std::vector<std::pair<size_t, uint32_t>> m_UninitialisedEntityWrappers;
		std::map<std::string, uint32_t> m_EditableUninitialisedEntityWrappers;
		// TODO: do this properly (use a callback from GetEntity or something, rather than just calling InitialiseWrappers over and over)
		bool m_FirstInit;
		
		std::vector<std::pair<boost::intrusive_ptr<asIScriptContext>, ConditionalCoroutine>> m_ActiveCoroutines;
		std::map<asIScriptContext*, ConditionalCoroutine> m_ActiveCoroutinesWithConditions;

		bool m_AutoYield;

		static int s_ASScriptTypeId;
	};

}

#endif
