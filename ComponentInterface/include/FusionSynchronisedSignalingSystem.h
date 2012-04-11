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

#ifndef H_FusionSynchronisedSignalingSystem
#define H_FusionSynchronisedSignalingSystem

#include "FusionPrerequisites.h"

#include "FusionExceptionFactory.h"

#include <boost/any.hpp>
#include <boost/signals2/signal.hpp>

#include <tbb/concurrent_queue.h>

namespace FusionEngine
{

	//! Transfers archetype changes to an entity
	template <class KeyT>
	class SynchronisedSignalingSystem
	{
	public:
		~SynchronisedSignalingSystem()
		{
			for (auto it = m_Generators.begin(); it != m_Generators.end(); ++it)
			{
				it->second->trigger_fn = std::function<void ()>();
			}
		}

		// Throws if the generator fn isn't convertable to Fn
		template <class T>
		boost::signals2::connection AddHandler(KeyT key, std::function<void (T)> handler_fn)
		{
			auto entry = m_Generators.find(key);
			if (entry != m_Generators.end())
			{
				if (auto generator = std::dynamic_pointer_cast<Generator<T>>(entry->second))
				{
					//auto& generator = boost::any_cast<Generator<T>>(entry->second);
					return generator->signal.connect(handler_fn);
				}
				else
				//catch (boost::bad_any_cast&)
				{
					FSN_EXCEPT(InvalidArgumentException, "The given handler's argument is not equivilent to the event type");
				}
			}
			else
				FSN_EXCEPT(InvalidArgumentException, "There is no generator for the given event type");
		}

		template <class T>
		struct SignalSerialiser
		{
			std::function<void (T)> serialise;
		};

		template <class T>
		std::function<void (T)> MakeGenerator(KeyT key)
		{
			FSN_ASSERT(m_Generators.find(key) == m_Generators.end());

			auto generator = std::make_shared<Generator<T>>();
			generator->trigger_fn = [this, key, generator]() { this->m_TriggeredGenerators.push(std::make_pair(key, generator)); };
			auto generator_callback = [generator](T value) { generator->Store(value); };
			m_Generators[key] = std::move(generator);
			return generator_callback;
		}

		void RemoveGenerator(KeyT key)
		{
			FSN_ASSERT(m_Generators.find(key) != m_Generators.end());
			auto entry = m_Generators.find(key);
			if (entry != m_Generators.end())
			{
				entry->second->trigger_fn = std::function<void ()>();
				m_Generators.erase(entry);
			}
		}

		void Run()
		{
			RakNet::BitStream packet;

			std::pair<KeyT, std::shared_ptr<GeneratorPlaceholder>> triggeredGenerator;
			while (m_TriggeredGenerators.try_pop(triggeredGenerator))
			{
				packet.Write(triggeredGenerator.first);
				RakNet::BitStream generatorData;
				triggeredGenerator.second->Fire(generatorData);
				packet.Write(generatorData);
			}
		}

	private:

		class GeneratorPlaceholder
		{
		public:
			virtual ~GeneratorPlaceholder() {}

			virtual void Fire(RakNet::BitStream&) = 0;

			std::function<void ()> trigger_fn;
		};

		template <typename T>
		class NullSerialiser
		{
		public:
			static void Write(RakNet::BitStream&, T) {}
			static void Read(RakNet::BitStream&, T&) {}
		};

		template <typename T, class SerialiserT = NullSerialiser<T>>
		class Generator : public GeneratorPlaceholder
		{
		public:
			boost::signals2::signal<void (T)> signal;

			typedef tbb::concurrent_queue<T> EventQueue_t;

			EventQueue_t queued_events;

			void Store(T value)
			{
				queued_events.push(value);
				trigger_fn();
			}

			void Fire(RakNet::BitStream& stream)
			{
				T v;
				while (queued_events.try_pop(v))
				{
					SerialiserT::Write(stream, v);
					signal(v);
				}
			}
		};

		std::map<KeyT, std::shared_ptr<GeneratorPlaceholder>> m_Generators;

		tbb::concurrent_queue<std::pair<KeyT, std::shared_ptr<GeneratorPlaceholder>>> m_TriggeredGenerators;
	};

}

#endif
