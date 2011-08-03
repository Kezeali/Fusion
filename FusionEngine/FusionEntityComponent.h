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

#ifndef H_FusionEntityComponent
#define H_FusionEntityComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"

#include <vector>
#include <functional>

#include <BitStream.h>

#include <boost/mpl/vector.hpp>
#include <boost/preprocessor.hpp>

#include <tbb/enumerable_thread_specific.h>

#define FSN_SYNCH_PROPERTY(name) \
	m_##name.Synchronise();

#define FSN_BEGIN_COIFACE(interface_name) \
	class interface_name\
	{\
	public:\
	static std::string GetTypeName() { return #interface_name; }\
	virtual ~interface_name() {}

#define FSN_COIFACE_CTOR(iface_name, properties) \
	void InitProperties()\
	{\
	typedef iface_name iface;\
	BOOST_PP_SEQ_FOR_EACH(FSN_INIT_PROPS, _, properties) \
	}

#define FSN_COIFACE(interface_name, properties)\
	FSN_BEGIN_COIFACE(interface_name)\
	FSN_COIFACE_CTOR(interface_name, properties)

namespace FusionEngine
{

	class IComponentProperty;

	typedef tbb::concurrent_queue<IComponentProperty*> PropChangedQueue;
	
	class IComponent : public RefCounted
	{
	public:
		//! Cotr
		IComponent()
			: m_ChangedProperties(nullptr)
		{
		}
		//! Destructor
		virtual ~IComponent() {}

		void SetParent(Entity* parent) { m_Parent = parent; }
		Entity* GetParent() const { return m_Parent; }

		void SetIdentifier(const std::string& identifier) { m_Identifier = identifier; }
		const std::string& GetIdentifier() const { return m_Identifier; }

		virtual std::string GetType() const = 0;

		const std::set<std::string>& GetInterfaces()
		{
			if (m_Interfaces.empty())
				InitInterfaceList(); // The implementation of this method is generated by FSN_LIST_INTERFACES
			return m_Interfaces;
		}

		virtual void InitInterfaceList() = 0;

		void AddProperty(IComponentProperty* prop);
		void OnPropertyChanged(IComponentProperty* prop);

		void SynchronisePropertiesNow();

		void SetPropChangedQueue(PropChangedQueue* q) { m_ChangedProperties = q; }

		//! Should be called after all mission-critical resources are done loading
		void Ready() { m_Ready = true; }
		//! Returns true after this Ready is called on this component
		/*
		* ISystemWorld#OnActivation wont be called until all siblings are ready.
		* The default ISystemWorld#OnPrepare() calls IComponent#Ready() on the passed component
		* (i.e. it assumes that the component has no mission-critical resources.)
		*/
		bool IsReady() const { return m_Ready; }

		virtual void OnSpawn() {}
		virtual void OnStreamIn() {}
		virtual void OnStreamOut() {}

		virtual void OnSiblingAdded(const std::shared_ptr<IComponent>& com) {}
		virtual void OnSiblingRemoved(const std::shared_ptr<IComponent>& com) {}

		virtual void SynchroniseParallelEdits() {}
		virtual void FireSignals() {}

		virtual bool SerialiseContinuous(RakNet::BitStream& stream) { return false; }
		virtual void DeserialiseContinuous(RakNet::BitStream& stream) {}
		virtual bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all) { return false; }
		virtual void DeserialiseOccasional(RakNet::BitStream& stream, const bool all) {}

	protected:
		std::set<std::string> m_Interfaces;

	private:
		Entity* m_Parent;
		std::string m_Identifier; // How this component is identified within the entity

		std::vector<IComponentProperty*> m_Properties;
		tbb::concurrent_queue<IComponentProperty*> *m_ChangedProperties;

		tbb::atomic<bool> m_Ready;

	};

//! Adds the name of the given interface type to m_Interfaces, and calls InitProperties() on it
#define FSN_ADD_INTERFACE(r, data, elem) m_Interfaces.insert(elem::GetTypeName()); elem::InitProperties();
//! Generates an implementation of InitInterfaceList by calling FSN_ADD_INTERFACE against the given sequence
#define FSN_LIST_INTERFACES(interfaces) void InitInterfaceList() { BOOST_PP_SEQ_FOR_EACH(FSN_ADD_INTERFACE, _, interfaces) }

//	class InsertInterfaceName
//	{
//	public:
//		InsertInterfaceName(std::set<std::string>& container)
//			: m_Container(container)
//		{
//		}
//
//		template <typename I>
//		void operator() (I)
//		{
//			m_Container.insert(I::GetTypeName());
//		}
//
//		std::set<std::string>& m_Container;
//	};
//
//#define FSN_LIST_INTERFACES() boost::mpl::for_each<Interfaces>(InsertInterfaceName(m_Interfaces));

	static std::string IComponent_GetType(void* obj)
	{
		auto com = static_cast<IComponent*>(obj);
		return com->GetType();
	}

	//! Registers a script type for the given component interface
	/*!
	* Assumes that the interface defines the static function
	* <code>static std::string GetTypeName()</code>
	*/
	template <typename T>
	void RegisterComponentInterfaceType(asIScriptEngine *engine)
	{
		IComponent::RegisterType<IComponent>(engine, T::GetTypeName());
		//int v = engine->RegisterObjectType(T::GetTypeName().c_str(), 0, asOBJ_REF | asOBJ_NOHANDLE); FSN_ASSERT(v >= 0);
		//int r = engine->RegisterObjectMethod(T::GetTypeName().c_str(), "string@ getType()", asMETHOD(IComponent, GetType), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		int r = engine->RegisterObjectMethod(T::GetTypeName().c_str(), "string getType()", asFUNCTION(IComponent_GetType), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

	template <typename T>
	class ValueBuffer
	{
	public:
		//! this could also be called "LastValue"
		const T& Read() const
		{
			return m_ReadBuffer;
		}

		//! This could be called "SetNextValue"
		void Write(const T& value)
		{
			m_WriteBuffers.local() = value;
		}

		//! This could be called "NextValue"
		T& GetWriteBuf()
		{
			return m_WriteBuffers.local();
		}

		//! Copys a written value into the out param then clears the buffer
		bool ClearWrittenValue(T& out)
		{
			// Essentially this just picks a value at random (whatever happens to be the 'first' value)
			//  In a single threaded application, multiple calls to a 'Set' method in a single simulation
			//  step would result in the same beahviour (whatever value happend to be set last would be used)
			if (m_WriteBuffers.size() > 0)
			{
				m_ReadBuffer = out = *m_WriteBuffers.begin();
				m_WriteBuffers.clear();
				return true;
			}
			return false;
		}

		void SetReadValue(const T& value)
		{
			m_ReadBuffer = value;
		}

	protected:
		T m_ReadBuffer;
		tbb::enumerable_thread_specific<T> m_WriteBuffers;
	};

	//! A thread-save interface to a class property
	template <typename T>
	class ValueBufferProperty : public ValueBuffer<T>
	{
	public:
		void Synchronise()
		{
			if (set_internal)
			{
				if (m_WriteBuffers.size() > 0)
				set_internal(*m_WriteBuffers.begin());
				m_WriteBuffers.clear();
			}

			get_internal(m_ReadBuffer);
		}

		void Initialise(const std::function<void (T&)>& getter, const std::function<void (const T&)>& setter)
		{
			get_internal = getter;
			set_internal = setter;

			m_ReadBuffer = get_internal();
		}

		void Initialise(const std::function<void (T&)>& getter)
		{
			Initialise(getter, std::function<void (const T&)>());
		}

	private:
		std::function<void (T&)> get_internal;
		std::function<void (const T&)> set_internal;
	};

	typedef std::shared_ptr<Component> ComponentPtr;

	// This will be in a seperate header, included only in FusionEntityFactory.cpp
	class ComponentType
	{
		std::vector<std::string> m_PublishedProperties;
		std::vector<std::string> m_PublishedMethods;
	};

	struct Serialiser
	{
		Serialiser(bool _force_all, RakNet::BitStream& _stream)
			: force_all(_force_all),
			stream(_stream)
		{}
		bool force_all;
		RakNet::BitStream& stream;

		template <typename T>
		inline bool writeChange(T& old_value, const T& new_value)
		{
			if (force_all || new_value != old_value)
			{
				stream.Write1();
				stream.Write(new_value);
				old_value = new_value;
				return true;
			}
			else
			{
				stream.Write0();
				return false;
			}
		}

		template <>
		inline bool writeChange(bool& old_value, const bool& new_value)
		{
			stream.Write(new_value);
			if (force_all || new_value != old_value)
			{
				old_value = new_value;
				return true;
			}
			else
				return false;
		}

		template <typename T>
		inline bool readChange(T& new_value)
		{
			if (force_all || stream.ReadBit())
			{
				stream.Read(new_value);
				return true;
			}
			else
				return false;
		}

		template <>
		inline bool readChange(bool& new_value)
		{
			stream.Read(new_value);
			return true;
		}
	};

}

#endif