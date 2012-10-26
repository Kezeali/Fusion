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
#include <tbb/concurrent_hash_map.h>

#include <functional>

namespace FusionEngine
{

	//! Synchronised Signaling System namespace
	namespace SyncSig
	{

		typedef std::shared_ptr<boost::signals2::scoped_connection> HandlerConnection_t;

		//! Stores generated signal events in a queue
		class GeneratorQueue
		{
		public:
			template <class T>
			struct Impl
			{
				typedef std::function<void (T)> GeneratorFn_t;

				void OnDispose() {}

				void OnGenerate(T value) { queued_events.push(value); }

				bool HasMore()
				{
					return queued_events.try_pop(next);
				}

				T GetEvent()
				{
					return next;
				}

				GeneratorFn_t MakeGenerator(std::function<void (void)> trigger_callback)
				{
					return [this, trigger_callback](T value) { this->OnGenerate(value); trigger_callback(); };
				}

				typedef tbb::concurrent_queue<T> EventQueue_t;
				EventQueue_t queued_events;

				T next;
			};
		};

		//! Doesn't serialise
		template <class KeyT>
		class NullSerialiser
		{
		public:
			NullSerialiser()
			{}

			void BeginGenerator(KeyT)
			{}

			void EndGenerator()
			{}

			template <class T>
			void WriteEvent(T)
			{}
		};

		//! Thread-safe, serialisable, savable "signaling" (observer pattern) system
		template <class KeyT, class GeneratorDetail = GeneratorQueue, class SerialisationMechanism = NullSerialiser<KeyT>>
		class SynchronisedSignalingSystem
		{
		private:
			// Non-copyable
			SynchronisedSignalingSystem(const SynchronisedSignalingSystem& other)
			{
			}
		public:
			//! Generator function type provided by GeneratorDetail template param.
			typedef GeneratorDetail GeneratorDetail_t;
			typedef KeyT Key_t;
			typedef SerialisationMechanism SerialisationMechanism_t;
			typedef typename SynchronisedSignalingSystem<KeyT, GeneratorDetail, SerialisationMechanism> This_t;

			//! CTOR
			SynchronisedSignalingSystem()
			{
			}

			//! Dtor
			~SynchronisedSignalingSystem()
			{
				for (auto it = m_Generators.begin(); it != m_Generators.end(); ++it)
				{
					it->second->trigger_fn = std::function<void ()>();
				}
			}

			//! Adds a hander for the given generator key
			/*!
			* \Throws if the generator's return type isn't equivilent to the given handler's argument type
			*/
			template <class T>
			HandlerConnection_t AddHandler(KeyT key, std::function<void (T)> handler_fn)
			{
				auto range = m_Generators.equal_range(key);
				if (range.first != range.second)
				{
					const auto& entry = range.first;

					typedef typename std::remove_reference<T>::type value_type;
					typedef typename std::remove_const<value_type>::type naked_type;

					// Try exact match
					if (auto generator = std::dynamic_pointer_cast<Generator<T>>(entry->second))
					{
						return generator->ConnectHandler(handler_fn);
					}
					// Try non-reference type match (match handler(const T& value) to signal->T or const T)
					else if (auto generator = std::dynamic_pointer_cast<Generator<value_type>>(entry->second))
					{
						return generator->ConnectHandler(handler_fn);
					}
					// Try non-const type match (match handler(const T& value) to signal->T)
					else if (auto generator = std::dynamic_pointer_cast<Generator<naked_type>>(entry->second))
					{
						return generator->ConnectHandler(handler_fn);
					}
					// Type const-ref generator match
					//else if (auto generator = std::dynamic_pointer_cast<Generator<const_ref_type>>(entry->second))
					//{
					//	return std::make_shared<boost::signals2::scoped_connection>(generator->signal.connect(handler_fn));
					//}
					else
					{
						FSN_EXCEPT(InvalidArgumentException, "The given handler's argument is not compatible with the signal type");
					}
				}
				else
				{
					FSN_EXCEPT(InvalidArgumentException, "There is no generator for the given signal type");
				}
			}

			//! Adds the given handler without trying simmilar types
			/*!
			* Useful when adding handlers for non-copyable types (where the handler takes a ref)
			*/
			template <class T>
			HandlerConnection_t AddExactHandler(KeyT key, std::function<void (T)> handler_fn)
			{
				auto range = m_Generators.equal_range(key);
				if (range.first != range.second)
				{
					const auto& entry = range.first;

					// Try exact match
					if (auto generator = std::dynamic_pointer_cast<Generator<T>>(entry->second))
					{
						return generator->ConnectHandler(handler_fn);
					}
					else
					{
						FSN_EXCEPT(InvalidArgumentException, "The given handler's argument is not exactly equal to the signal type");
					}
				}
				else
				{
					FSN_EXCEPT(InvalidArgumentException, "There is no generator for the given signal type");
				}
			}

			//! Adds a listener (like handler that doesn't receive the new value)
			HandlerConnection_t AddListener(KeyT key, std::function<void ()> handler_fn)
			{
				auto range = m_Generators.equal_range(key);
				if (range.first != range.second)
				{
					const auto& entry = range.first;
					auto generatorPlaceholder = entry->second;
					FSN_ASSERT(generatorPlaceholder);

					return generatorPlaceholder->ConnectListener(handler_fn);
				}
				else
				{
					FSN_EXCEPT(InvalidArgumentException, "There is no generator for the given signal type");
				}
			}

			//! Returns a new generator functor
			template <class T>
			typename GeneratorDetail::Impl<T>::GeneratorFn_t MakeGenerator(KeyT key);
			//! Returns a new generator functor; for generators that require 1 argument
			template <class T, class Arg0T>
			typename GeneratorDetail::Impl<T>::GeneratorFn_t MakeGenerator(KeyT key, Arg0T arg0);

			//! Don't call this manually
			/*!
			* Don't call this manually... unless you know what you are doing: this
			* should be called automatically when the functor returned by
			* MakeGenerator goes out of scope.
			*/
			void RemoveGenerator(KeyT key)
			{
				FSN_ASSERT(m_Generators.count(key) > 0);
				auto range = m_Generators.equal_range(key);
				if (range.first != range.second)
				{
					range.first->second->trigger_fn = std::function<void ()>();
					range.first->second->Dispose();
					m_Generators.erase(key);
				}
			}

			//! Fires queued events
			SerialisationMechanism Fire()
			{
				SerialisationMechanism serialiser;

				std::pair<KeyT, std::weak_ptr<GeneratorPlaceholder>> triggeredGenerator;
				while (m_TriggeredGenerators.try_pop(triggeredGenerator))
				{
					if (auto lockedGenerator = triggeredGenerator.second.lock())
					{
						serialiser.BeginGenerator(triggeredGenerator.first);
						lockedGenerator->Fire(serialiser);
						serialiser.EndGenerator();
					}
				}

				return serialiser;
			}

			//! Returns true if the generator is defined
			bool HasGenerator(KeyT key) const
			{
				return m_Generators.count(key) > 0;
			}

			//! Subscribe to receive notification when new generators are added
			HandlerConnection_t SubscribeNewGenerators(KeyT key, const std::function<void (KeyT, This_t&)>& callback)
			{
				FSN_ASSERT(!HasGenerator(key));

				NewGeneratorSignalMap_t::accessor accessor;
				const bool inserted = m_NewGeneratorSignals.insert(accessor, key);
				if (inserted)
					accessor->second = std::make_shared<SigNewGenerator_t>();
				auto connection = std::make_shared<boost::signals2::scoped_connection>(accessor->second->connect(callback));
				// It's possible that this callback was inserted concurrently with the signal being retrieved
				//  due to the generator coming available: check for that
				if (inserted && HasGenerator(key))
				{
					callback(key, *this);
					m_NewGeneratorSignals.erase(accessor);
					return HandlerConnection_t();
				}
				else
					return std::move(connection);
			}

			//! Remove subscription to the given generator key
			void OnUnsubscribeNewGenerators(KeyT key)
			{
				NewGeneratorSignalMap_t::accessor accessor;
				if (m_NewGeneratorSignals.find(accessor, key))
				{
					if (accessor->second->num_slots() == 0)
						m_NewGeneratorSignals.erase(accessor);
				}
			}

			boost::signals2::signal<void (KeyT, This_t&)> SigGeneratorDefined;

		private:
			//! Interface for any generator (type-erasure ish, blah blah)
			class GeneratorPlaceholder
			{
			public:
				virtual ~GeneratorPlaceholder() {}
				
				virtual void Dispose() = 0;

				virtual void Fire(SerialisationMechanism&) = 0;

				virtual HandlerConnection_t ConnectListener(const std::function<void ()>& listener) = 0;

				std::function<void ()> trigger_fn;
			};

			template <typename T>
			class Generator : public GeneratorPlaceholder, public GeneratorDetail::Impl<T>
			{
			private:
				Generator(const Generator& other) {}
			
				typedef boost::signals2::signal<void (T)> Signal_t;
				std::unique_ptr<Signal_t> signal;

			public:
				Generator()
				{}

				void Dispose()
				{
					OnDispose();
				}

				void Trigger()
				{
					if (trigger_fn)
						trigger_fn();
				}

				void Fire(SerialisationMechanism& serialiser)
				{
					if (signal)
					{
						while (HasMore())
						{
							T v = GetEvent();
							serialiser.WriteEvent<T>(v);
							(*signal)(v);
						}
					}
				}

				HandlerConnection_t ConnectListener(const std::function<void ()>& listener)
				{
					if (!signal)
						signal.reset(new Signal_t);
					return std::make_shared<boost::signals2::scoped_connection>( signal->connect([=](T){ listener(); }) );
				}

				HandlerConnection_t ConnectHandler(const std::function<void (T)>& handler)
				{
					if (!signal)
						signal.reset(new Signal_t);
					return std::make_shared<boost::signals2::scoped_connection>( signal->connect(handler) );
				}
			};

			class GeneratorAutoRemover
			{
			public:
				GeneratorAutoRemover(This_t* parent_, KeyT key_)
					: parent(parent_), key(key_)
				{}
				~GeneratorAutoRemover()
				{
					parent->RemoveGenerator(key);
				}

				This_t* parent;
				KeyT key;
			};

			typedef std::shared_ptr<GeneratorAutoRemover> AutoRemoverPtr;

			typedef tbb::concurrent_hash_map<KeyT, std::shared_ptr<GeneratorPlaceholder>> GeneratorMap_t;
			GeneratorMap_t m_Generators;
			tbb::concurrent_queue<std::pair<KeyT, std::weak_ptr<GeneratorPlaceholder>>> m_TriggeredGenerators;

			typedef boost::signals2::signal<void (KeyT, This_t&)> SigNewGenerator_t;
			typedef tbb::concurrent_hash_map<KeyT, std::shared_ptr<SigNewGenerator_t>> NewGeneratorSignalMap_t;
			NewGeneratorSignalMap_t m_NewGeneratorSignals;

			template <class T>
			std::shared_ptr<Generator<T>> MakeGeneratorObj(KeyT key)
			{
				FSN_ASSERT(m_Generators.count(key) == 0);

				auto generator = std::make_shared<Generator<T>>();
				generator->trigger_fn = [this, key, generator]() { this->m_TriggeredGenerators.push(std::make_pair(key, generator)); };
				m_Generators.insert(std::make_pair(key, generator));

				//SigGeneratorDefined(key, *this);
				NewGeneratorSignalMap_t::const_accessor accessor;
				if (m_NewGeneratorSignals.find(accessor, key))
				{
					auto& callback = *accessor->second;
					callback(key, *this);
					m_NewGeneratorSignals.erase(accessor);
				}

				return std::move(generator);
			}
		};

		template <class KeyT, class GeneratorDetail, class SerialisationMechanism>
		template <class T>
		typename GeneratorDetail::Impl<T>::GeneratorFn_t SynchronisedSignalingSystem<KeyT, GeneratorDetail, SerialisationMechanism>::MakeGenerator(KeyT key)
		{
			// This will remove the generator whey the trigger callback goes out of scope
			AutoRemoverPtr autoRemover(new GeneratorAutoRemover(this, key));

			auto generator = MakeGeneratorObj<T>(key);

			auto generator_trigger = generator->MakeGenerator([generator, autoRemover]() { generator->Trigger(); });
			FSN_ASSERT_MSG(autoRemover.use_count() >= 2, "The GeneratorDetail provided seems to have failed to make a copy of the trigger callback passed");

			return std::move(generator_trigger);
		}

		template <class KeyT, class GeneratorDetail, class SerialisationMechanism>
		template <class T, class Arg0T>
		typename GeneratorDetail::Impl<T>::GeneratorFn_t SynchronisedSignalingSystem<KeyT, GeneratorDetail, SerialisationMechanism>::MakeGenerator(KeyT key, Arg0T arg0)
		{
			// This will remove the generator whey the trigger callback goes out of scope
			AutoRemoverPtr autoRemover(new GeneratorAutoRemover(this, key));

			auto generator = MakeGeneratorObj<T>(key);

			auto generator_trigger = generator->MakeGenerator([generator, autoRemover]() { generator->Trigger(); }, arg0);
			FSN_ASSERT_MSG(autoRemover.use_count() >= 2, "The GeneratorDetail provided seems to have failed to make a copy of the trigger callback passed");

			return std::move(generator_trigger);
		}

	}

}

#endif
