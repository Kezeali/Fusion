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

#include <vector>
#include <functional>

#include <BitStream.h>

#include <boost/mpl/vector.hpp>
#include <boost/preprocessor.hpp>

#include <tbb/enumerable_thread_specific.h>

#define FSN_TS_PROPERTY_GETSET(type, name, get, set) \
	ThreadSafeProperty<type> m_##name;\
	type Get##name() const { return m_##name.Get(); }\
	void Set##name(const type & value) { m_##name.Set(value); }\
	void Init##name() { m_##name.Initialise([this](type & value) { get(value); }, [this](const type & value) { set(value); }); }

#define FSN_TS_PROPERTY_GET(type, name, get) \
	ThreadSafeProperty<type> m_##name;\
	type Get##name() const { return m_##name.Get(); }\
	void Init##name() { m_##name.Initialise([this](T& value) { get(value); }); }

#define FSN_SYNCH_PROPERTY(name) \
	m_##name.Synchronise();

#define FSN_BEGIN_COIFACE(interface_name) \
	class interface_name\
	{\
	public:\
	static std::string GetTypeName() { return #interface_name; }\
	virtual ~interface_name() {}

namespace FusionEngine
{

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

	class IComponent
	{
	public:
		//! Destructor
		virtual ~IComponent() {}

		void SetParent(Entity* parent) { m_Parent = parent; }
		Entity* GetParent() const { return m_Parent; }

		virtual std::string GetType() const = 0;

		virtual void OnSiblingAdded(const std::set<std::string>& interfaces, const std::shared_ptr<IComponent>& com) {}

		virtual void SynchroniseParallelEdits() = 0;

		virtual bool SerialiseContinuous(RakNet::BitStream& stream) { return false; }
		virtual void DeserialiseContinuous(RakNet::BitStream& stream) {}
		virtual bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all) { return false; }
		virtual void DeserialiseOccasional(RakNet::BitStream& stream, const bool all) {}

	private:
		Entity* m_Parent;
	};

}

#endif
