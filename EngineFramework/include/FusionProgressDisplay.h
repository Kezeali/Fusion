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

#ifndef H_FusionProgressDisplay
#define H_FusionProgressDisplay

#include "FusionPrerequisites.h"

#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>

#include <functional>
#include <memory>
#include <string>
#include <tbb/concurrent_queue.h>

namespace FusionEngine
{

	//! UI element that displays progress of some task
	class ProgressDisplay : public Rocket::Core::Element
	{
	public:
		//! Default CTOR
		ProgressDisplay();
		//! Ctor
		ProgressDisplay(bool destroy_when_done);
		// DTOR
		virtual ~ProgressDisplay();

		// Creates a new progress display in it's own document
		static ProgressDisplay* Create(Rocket::Core::Context *context, bool destroy_when_done = true);

		enum ProgressType
		{
			Start,
			Progress,
			Error,
			// Done Events:
			Complete,
			Failure
		};

		std::function<void (ProgressType, const std::string&)> MakeProgressGenerator();

		virtual void OnUpdate();

	private:
		struct Event
		{
			ProgressType type;
			std::string message;
		};

		tbb::concurrent_queue<Event> m_IncommingEvents;

		bool m_DestroyWhenDone;
		Rocket::Core::ElementDocument* m_DocumentToDestory;

		Rocket::Core::Element* m_MessageLabel;
	};

}

#endif
