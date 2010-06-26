/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionMessageBox.h"

#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include "FusionGUI.h"

namespace FusionEngine
{

	static inline EMP::Core::String toEmp(const std::string& str)
	{
		return EMP::Core::String(str.data(), str.data() + str.length());
	}

	MessageBox::MessageBox(const std::string& document_filename)
		: m_Filename(document_filename),
		m_Document(nullptr)
	{
		loadDocument(GUI::getSingleton().GetContext(), document_filename);
	}

	MessageBox::MessageBox(Rocket::Core::Context* context, const std::string& document_filename)
		: m_Filename(document_filename),
		m_Document(nullptr)
	{
		loadDocument(context, document_filename);
	}

	void MessageBox::loadDocument(Rocket::Core::Context* context, const std::string& document_filename)
	{
		m_Document = context->LoadDocument( toEmp(document_filename) );
	}

	MessageBox::~MessageBox()
	{
		if (m_Document != nullptr)
		{
			for (auto it = m_EventSignals.begin(), end = m_EventSignals.end(); it != end; ++it)
				m_Document->RemoveEventListener(it->first, this);
			m_Document->Close();
			m_Document->RemoveReference();
		}
	}

	void MessageBox::ProcessEvent(Rocket::Core::Event& ev)
	{
		auto _where = m_EventSignals.find(ev.GetType());
		if (_where != m_EventSignals.end())
		{
			auto& eventFn = *_where->second;
			eventFn(ev);
		}
	}

	Rocket::Core::ElementDocument* const MessageBox::GetDocument() const
	{
		return m_Document;
	}

	MessageBox::EventSignal& MessageBox::GetEventSignal(const EMP::Core::String& type)
	{
		m_Document->AddEventListener(type, this);
		return *(m_EventSignals[type] = std::shared_ptr<MessageBox::EventSignal>( new EventSignal ));
	}

	MessageBoxManager::MessageBoxManager()
	{
	}

	MessageBoxManager::~MessageBoxManager()
	{
		std::for_each(m_EventConnections.begin(), m_EventConnections.end(), [](boost::signals2::connection& connection) { connection.disconnect(); });
	}

	void MessageBoxManager::ConnectToEvent(MessageBox& message_box, const EMP::Core::String& type, EventFunction function)
	{
		m_EventConnections.emplace_back( message_box.GetEventSignal(type).connect(function) );
	}

}
