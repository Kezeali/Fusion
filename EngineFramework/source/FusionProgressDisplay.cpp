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

#include "PrecompiledHeaders.h"

#include "FusionProgressDisplay.h"

#include "FusionGUI.h"

using namespace std::placeholders;

namespace FusionEngine
{

	ProgressDisplay::ProgressDisplay()
		: Rocket::Core::Element("progress_display"),
		m_DestroyWhenDone(true),
		m_DocumentToDestory(nullptr)
	{
	}

	ProgressDisplay::ProgressDisplay(bool destroy_when_done)
		: Rocket::Core::Element("progress_display"),
		m_DestroyWhenDone(destroy_when_done),
		m_DocumentToDestory(nullptr)
	{
	}

	ProgressDisplay::~ProgressDisplay()
	{
	}

	ProgressDisplay* ProgressDisplay::Create(Rocket::Core::Context *context, bool destroy_when_done /* = true */)
	{
		auto document = context->LoadDocument("core/gui/progress_display.rml");
		Rocket::Core::ElementList elements;
		document->GetElementsByTagName(elements, "progress_display");
		if (!elements.empty())
		{
			if (auto progressDisplay = dynamic_cast<ProgressDisplay*>(elements[0]))
			{
				using namespace std::placeholders;

				progressDisplay->m_DestroyWhenDone = destroy_when_done;
				if (destroy_when_done)
					progressDisplay->m_DocumentToDestory = document;

				document->Show();
				return progressDisplay;
			}
		}

		document->RemoveReference();
		FSN_EXCEPT(FileTypeException, "Missing progress_display element in progress_display.rml");
	}

	std::function<void (ProgressDisplay::ProgressType, const std::string&)> ProgressDisplay::MakeProgressGenerator()
	{
		boost::intrusive_ptr<ProgressDisplay> smartyThis(this);
		return [smartyThis](ProgressType type, const std::string& message)
		{
			Event newEvent;
			newEvent.type = type;
			newEvent.message = message;
			smartyThis->m_IncommingEvents.push(newEvent);
		};
	}

	void ProgressDisplay::OnUpdate()
	{
		Element::OnUpdate();

		Event ev;
		if (m_IncommingEvents.try_pop(ev))
		{
			m_MessageLabel->SetInnerRML(Rocket::Core::String(ev.message.data(), ev.message.data() + ev.message.length()));

			if (m_DestroyWhenDone)
			{
				if (ev.type == Complete || ev.type == Failure)
				{
					if (m_DocumentToDestory)
					{
						m_DocumentToDestory->Close();
						m_DocumentToDestory->RemoveReference();
					}
					else if (auto parent = this->GetParentNode())
					{
						parent->RemoveChild(this);
					}
				}
			}
		}
	}

}