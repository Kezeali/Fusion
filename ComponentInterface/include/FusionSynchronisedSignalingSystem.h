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

#include <boost/signals2/signal.hpp>

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>

#include <functional>

namespace FusionEngine
{

	namespace SyncSig
	{

		template <class KeyT>
		class NullMechanism
		{
		public:
			NullMechanism()
			{}

			void BeginGenerator(KeyT)
			{}

			void EndGenerator()
			{}

			template <class T>
			void WriteEvent(T)
			{}
		};

		class GeneratorQueue
		{
		public:
			template <class T>
			struct Impl
			{
				typedef std::function<void (T)> GeneratorFn_t;

				void OnGenerate(T value) { queued_events.push(value); }

				bool HasMore()
				{
					return queued_events.try_pop(next);
				}

				T GetEvent()
				{
					return next;
				}

				GeneratorFn_t MakeGenerator(std::function<void (void)> trigger)
				{
					return [this, trigger](T value) { this->OnGenerate(value); trigger(); };
				}

				typedef tbb::concurrent_queue<T> EventQueue_t;
				EventQueue_t queued_events;

				T next;
			};
		};

		//! Transfers archetype changes to an entity
		template <class KeyT, class GeneratorDetail = GeneratorQueue, class SerialisationMechanism = NullMechanism<KeyT>>
		class SynchronisedSignalingSystem
		{
		public:
			//! Generator function type provided by GeneratorDetail template param.
			typedef GeneratorDetail GeneratorDetail_t;

			//! Dtor
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
			typename GeneratorDetail::Impl<T>::GeneratorFn_t MakeGenerator(KeyT key);

			template <class T, class Arg0T>
			typename GeneratorDetail::Impl<T>::GeneratorFn_t MakeGenerator(KeyT key, Arg0T arg0);

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

			SerialisationMechanism Run()
			{
				SerialisationMechanism serialiser;

				std::pair<KeyT, std::shared_ptr<GeneratorPlaceholder>> triggeredGenerator;
				while (m_TriggeredGenerators.try_pop(triggeredGenerator))
				{
					serialiser.BeginGenerator(triggeredGenerator.first);
					triggeredGenerator.second->Fire(serialiser);
					serialiser.EndGenerator();
				}

				return serialiser;
			}

		private:
			//! Interface for any generator
			class GeneratorPlaceholder
			{
			public:
				virtual ~GeneratorPlaceholder() {}

				virtual void Fire(SerialisationMechanism&) = 0;

				std::function<void ()> trigger_fn;
			};

			template <typename T>
			class Generator : public GeneratorPlaceholder, public GeneratorDetail::Impl<T>
			{
			public:
				boost::signals2::signal<void (T)> signal;

				void Trigger()
				{
					trigger_fn();
				}

				void Fire(SerialisationMechanism& serialiser)
				{
					while (HasMore())
					{
						T v = GetEvent();
						serialiser.WriteEvent<T>(v);
						signal(v);
					}
				}
			};

			std::map<KeyT, std::shared_ptr<GeneratorPlaceholder>> m_Generators;
			tbb::concurrent_queue<std::pair<KeyT, std::shared_ptr<GeneratorPlaceholder>>> m_TriggeredGenerators;

			template <class T>
			std::shared_ptr<Generator<T>> MakeGeneratorObj(KeyT key)
			{
				FSN_ASSERT(m_Generators.find(key) == m_Generators.end());

				auto generator = std::make_shared<Generator<T>>();
				generator->trigger_fn = [this, key, generator]() { this->m_TriggeredGenerators.push(std::make_pair(key, generator)); };
				return std::move(generator);
			}
		};

		template <class KeyT, class GeneratorDetail, class SerialisationMechanism>
		template <class T>
		typename GeneratorDetail::Impl<T>::GeneratorFn_t SynchronisedSignalingSystem<KeyT, GeneratorDetail, SerialisationMechanism>::MakeGenerator(KeyT key)
		{
			auto generator = MakeGeneratorObj<T>(key);
			auto generator_trigger = generator->MakeGenerator([generator]() { generator->Trigger(); });
			m_Generators[key] = std::move(generator);
			return generator_trigger;
		}

		template <class KeyT, class GeneratorDetail, class SerialisationMechanism>
		template <class T, class Arg0T>
		typename GeneratorDetail::Impl<T>::GeneratorFn_t SynchronisedSignalingSystem<KeyT, GeneratorDetail, SerialisationMechanism>::MakeGenerator(KeyT key, Arg0T arg0)
		{
			auto generator = MakeGeneratorObj<T>(key);
			auto generator_trigger = generator->MakeGenerator([generator]() { generator->Trigger(); }, arg0);
			m_Generators[key] = std::move(generator);
			return generator_trigger;
		}

	}

}

#endif
