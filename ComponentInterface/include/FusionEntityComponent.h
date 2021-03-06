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

#ifndef H_FusionEntityComponent
#define H_FusionEntityComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"
#include "FusionTypes.h"
// TEMP (for m_Properties to compile)
#include "FusionComponentProperty.h"

#include <bitset>
#include <vector>
#include <functional>

#include <RakNet/BitStream.h>

#include <boost/mpl/vector.hpp>
#include <boost/preprocessor.hpp>

#include <tbb/enumerable_thread_specific.h>
#include <tbb/concurrent_queue.h>

#define FSN_BEGIN_COIFACE(interface_name) \
	class interface_name\
	{\
	public:\
	static std::string GetTypeName() { return #interface_name; }\
	virtual ~interface_name() {}

#define FSN_END_COIFACE()\
	};

namespace FusionEngine
{

	class ComponentProperty;
	class IComponentProperty;

	template <class T>
	class ComponentIPtr
	{
	private:
		typedef ComponentIPtr this_type;

	public:
		ComponentIPtr()
		{}

		ComponentIPtr(T* i)
			: p(dynamic_cast<EntityComponent*>(i))
		{}

		ComponentIPtr(ComponentPtr com)
			: p(com)
		{}

		//ComponentIPtr(EntityComponent* com)
		//	: p(com)
		//{}

		template<class U> ComponentIPtr & operator=(ComponentIPtr<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    ComponentIPtr(ComponentIPtr && rhs)
			: p( std::move(rhs.p) )
    {
			//rhs.px = 0;
    }

    ComponentIPtr & operator=(ComponentIPtr && rhs)
    {
        this_type( static_cast< ComponentIPtr && >( rhs ) ).swap(*this);
        return *this;
    }

    ComponentIPtr & operator=(ComponentIPtr const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    ComponentIPtr & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset()
    {
        this_type().swap( *this );
    }

    void reset( T * rhs )
    {
        this_type( rhs ).swap( *this );
    }

		T * get() const
		{
			return dynamic_cast<T*>(p.get());
		}

		T & operator*() const
		{
			FSN_ASSERT(p);
			return *dynamic_cast<T*>(p.get());
		}

		T * operator->() const
		{
			FSN_ASSERT(p);
			return dynamic_cast<T*>(p.get());
		}

		operator ComponentPtr::unspecified_bool_type() const
		{
			return p;
		}

		void swap(ComponentIPtr & rhs)
		{
			//T * tmp = px;
			//px = rhs.px;
			//rhs.px = tmp;
			p.swap(rhs.p);
		}

	//private:
		ComponentPtr p;
	};

	//typedef tbb::concurrent_queue<std::pair<std::weak_ptr<PropLock>, IComponentProperty*>> PropChangedQueue;
	
	class EntityComponent : public RefCounted
	{
	public:
		//! Ctor
		EntityComponent()
			: RefCounted(0),
			m_ArchetypeId(std::numeric_limits<Archetypes::ComponentID_t>::max()),
			m_InterfacesInitialised(false),
			m_LastContinuousProperty(nullptr),
			m_LastOccasionalProperty(nullptr)
		{
			m_Ready = NotReady;
		}
		//! Destructor
		virtual ~EntityComponent()
		{
			if (m_LastContinuousProperty)
				delete m_LastContinuousProperty;
			if (m_LastOccasionalProperty)
				delete m_LastOccasionalProperty;
		}

		//void OnNoReferences()
		//{
		//	delete this;
		//}

		//! Clones this component
		/*!
		* Default implementation uses serialisation to make a copy of the component
		*/
		virtual ComponentPtr Clone(ComponentFactory* factory);

		void SetParent(Entity* parent) { m_Parent = parent; }
		Entity* GetParent() const { return m_Parent; }

		void SetIdentifier(const std::string& identifier) { m_Identifier = identifier; }
		const std::string& GetIdentifier() const { return m_Identifier; }

		//! Sets the archetype-id that refers to this component (if it is part of an archetype)
		void SetArchetypeID(Archetypes::ComponentID_t id) { m_ArchetypeId = id; }
		//! Gets the archetype-id that refers to this component (if it is part of an archetype)
		Archetypes::ComponentID_t GetArchetypeID() const { return m_ArchetypeId; }

		virtual std::string GetType() const = 0;

		//! Gets the specific type of this component. Used by ComponentTypeInfoCache.
		/*!
		 * The specific type should be defined such that the properties
		 * can be assumed to be consistent at runtime
		 */
		virtual std::string GetProfileType() const { return GetType(); }

		const std::set<std::string>& GetInterfaces()
		{
			if (m_Interfaces.empty())
			{
				InitInterfaceList(); // The implementation of this method is generated by FSN_LIST_INTERFACES
			}
			return m_Interfaces;
		}

		virtual void InitInterfaces() = 0;
		virtual void InitInterfaceList() = 0;

		boost::intrusive_ptr<ComponentProperty> AddProperty(const std::string& name, IComponentProperty* prop);

		const std::vector<std::pair<std::string, boost::intrusive_ptr<ComponentProperty>>>& GetProperties() const { return m_Properties; }

		void SynchronisePropertiesNow();

		//! Possible ready states
		enum ReadyState /*: uint32_t*/ { NotReady, Preparing, Ready, Active };

		//! Set the ready state
		void SetReadyState(ReadyState state) { m_Ready = state; }
		ReadyState GetReadyState() const { return m_Ready; }

		bool IsPreparing() const { return m_Ready == Preparing; }

		//! Should be called after all mission-critical resources are done loading
		void MarkReady() { m_Ready = Ready; }

		//! Returns true after this Ready is called on this component
		/*
		* ISystemWorld#OnActivation wont be called until all siblings are ready.
		* The default ISystemWorld#Prepare() calls EntityComponent#Ready() on the passed component
		* (i.e. it assumes that the component has no mission-critical resources.)
		*/
		bool IsReady() const { return m_Ready == Ready; }

		bool IsActive() const { return m_Ready == Active; }

		virtual void OnSpawn() {}
		virtual void OnStreamIn() {}
		virtual void OnStreamOut() {}

		virtual void OnSiblingAdded(const ComponentPtr& com) {}
		virtual void OnSiblingRemoved(const ComponentPtr& com) {}

		virtual void SerialiseContinuous(RakNet::BitStream& stream);
		virtual void DeserialiseContinuous(RakNet::BitStream& stream);
		virtual void SerialiseOccasional(RakNet::BitStream& stream);
		virtual void DeserialiseOccasional(RakNet::BitStream& stream);

		virtual void SerialiseEditable(RakNet::BitStream& stream);
		virtual void DeserialiseEditable(RakNet::BitStream& stream);

		virtual void OnPostDeserialisation() {}

	protected:
		std::set<std::string> m_Interfaces;
		bool m_InterfacesInitialised;
		
		typedef boost::intrusive_ptr<ComponentProperty> PropertyPtr;
		std::vector<std::pair<std::string, PropertyPtr>> m_Properties;

	private:
		Entity* m_Parent;
		std::string m_Identifier; // How this component is identified within the entity

		Archetypes::ComponentID_t m_ArchetypeId; // Identifies this component if it is part of an arch definition

		tbb::atomic<ReadyState> m_Ready;

		//! Used to create singly linked lists of continuous / occasional properties (for purposes of serialisation)
		class PropertyListNode
		{
		public:
			PropertyListNode(ComponentProperty* prop_) : prop(prop_), previous(nullptr) {}
			~PropertyListNode() { if (previous) delete previous; }
			ComponentProperty* prop;
			PropertyListNode* previous;
		};
		PropertyListNode* m_LastContinuousProperty;
		PropertyListNode* m_LastOccasionalProperty;

		//std::shared_ptr<ComponentImpl> m_Impl;

		std::shared_ptr<std::map<std::string, size_t>> m_PropertiesMap;

		virtual void OnPropertyAdded(const PropertyPtr& prop) {}
	};


//! Calls InitProperties() on the given interface type
#define FSN_INIT_INTERFACE(r, data, elem) elem::InitProperties();

//! Adds the name of the given interface type to m_Interfaces
#define FSN_ADD_INTERFACE(r, data, elem) m_Interfaces.insert(elem::GetTypeName());
//! Generates implementations of InitInterfaceList and InitInterfaces by calling FSN_ADD_INTERFACE against the given sequence
#define FSN_LIST_INTERFACES(interfaces) \
	void InitInterfaceList() { BOOST_PP_SEQ_FOR_EACH(FSN_ADD_INTERFACE, _, interfaces) }\
	void InitInterfaces() { if (!m_InterfacesInitialised) { BOOST_PP_SEQ_FOR_EACH(FSN_INIT_INTERFACE, _, interfaces) m_InterfacesInitialised = true; } }


	template <class T>
	static void IComponent_addRef(T* obj)
	{
		auto com = dynamic_cast<EntityComponent*>(obj);
		com->addRef();
	}

	template <class T>
	static void IComponent_release(T* obj)
	{
		auto com = dynamic_cast<EntityComponent*>(obj);
		com->release();
	}

	template <class T>
	static std::string IComponent_GetType(T* obj)
	{
		auto com = dynamic_cast<EntityComponent*>(obj);
		return com->GetType();
	}

	template <class Derived>
	Derived* ComponentCast(EntityComponent *obj)
	{
		if (obj != nullptr)
		{
			Derived* casted = dynamic_cast<Derived*>(obj);
			if (casted != nullptr)
			{
				obj->addRef();
			}
			return casted;
		}
		else
			return nullptr;
	}

	//! Registers a script type for the given component interface
	/*!
	* Assumes that the interface defines the static function
	* <code>static std::string GetTypeName()</code>
	*/
	template <typename T>
	void RegisterComponentInterfaceType(asIScriptEngine *engine)
	{
		//EntityComponent::RegisterType<EntityComponent>(engine, T::GetTypeName());
		//int v = engine->RegisterObjectType(T::GetTypeName().c_str(), 0, asOBJ_REF | asOBJ_NOHANDLE); FSN_ASSERT(v >= 0);
		int r;
		r = engine->RegisterObjectType(T::GetTypeName().c_str(), 0, asOBJ_REF); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectBehaviour(T::GetTypeName().c_str(), asBEHAVE_ADDREF, "void addref()", asFUNCTION(IComponent_addRef<T>), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour(T::GetTypeName().c_str(), asBEHAVE_RELEASE, "void release()", asFUNCTION(IComponent_release<T>), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod(T::GetTypeName().c_str(), "string getType()", asFUNCTION(IComponent_GetType<T>), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectBehaviour("EntityComponent", asBEHAVE_REF_CAST, (T::GetTypeName() + "@ f()").c_str(), asFUNCTION(ComponentCast<T>), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

}

#endif
