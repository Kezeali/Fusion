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
		{}

		// Throws if the generator fn isn't convertable to Fn
		template <class T>
		boost::signals2::connection AddHandler(KeyT key, std::function<void (T)> handler_fn)
		{
			auto entry = generators.find(key);
			if (entry != generators.end())
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
		std::function<void (T)> MakeGenerator(KeyT key)
		{
			FSN_ASSERT(generators.find(key) == generators.end());

			auto generator = std::make_shared<Generator<T>>();

			auto generator_callback = [generator](T value) { generator->queued_events.push(value); };

			generators[key] = std::move(generator);

			return generator_callback;
		}

		void Run()
		{
			for (auto it = generators.begin(); it != generators.end(); ++it)
			{
				it->second->Fire();
			}
		}

	private:

		class GeneratorPlaceholder
		{
		public:
			virtual ~GeneratorPlaceholder() {}

			virtual void Fire() = 0;
		};

		template <typename T>
		class Generator : public GeneratorPlaceholder
		{
		public:
			std::function<void (T)> generator_fn;
			boost::signals2::signal<void (T)> signal;

			typedef tbb::concurrent_queue<T> EventQueue_t;

			EventQueue_t queued_events;

			void Fire()
			{
				T v;
				while (queued_events.try_pop(v))
				{
					signal(v);
				}
			}
		};

		std::map<KeyT, std::shared_ptr<GeneratorPlaceholder>> generators;
	};

}

#endif
