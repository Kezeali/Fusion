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

#ifndef H_FusionPropertySyncSigDetail
#define H_FusionPropertySyncSigDetail

#include "FusionPrerequisites.h"

#include <functional>

namespace FusionEngine
{
	
	namespace SyncSig
	{

		class PropertyCallback
		{
		public:
			template <class T>
			class Impl
			{
			public:
				typedef std::function<T (void)> CallbackFn_t;
				typedef std::function<void (void)> GeneratorFn_t;

				CallbackFn_t callback;
				bool hasEvent;

				Impl() : hasEvent(false) {}

				bool HasMore()
				{
					return hasEvent;
				}

				T GetEvent()
				{
					FSN_ASSERT(callback);
					hasEvent = false;
					return callback();
				}

				GeneratorFn_t MakeGenerator(std::function<void (void)> trigger_callback, CallbackFn_t event_callback)
				{
					FSN_ASSERT(event_callback);
					callback = event_callback;
					return [this, trigger_callback]() { this->hasEvent = true; trigger_callback(); };
				}
			};
		};

	}

}

#endif
