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

#include "PrecompiledHeaders.h"

#include "FusionPrerequisites.h"

#include "FusionMessageBox.h"

#include <boost/lexical_cast.hpp>
#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include "FusionCommon.h"
#include "FusionGUI.h"

namespace FusionEngine
{

	namespace
	{
		Rocket::Core::String toEmp(const std::string& str)
		{
			return Rocket::Core::String(str.data(), str.data() + str.length());
		}
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

	void MessageBox::SetType(const std::string& type)
	{
		m_Type = type;
		m_Document->SetId(toEmp(type));
		m_Document->SetPseudoClass(toEmp(type), true);
	}

	void MessageBox::SetTitle(const std::string& title)
	{
		auto rocketStringTitle = toEmp(title);
		m_Document->SetTitle(rocketStringTitle);
	}

	void MessageBox::SetElement(const std::string& id, const std::string& text)
	{
		Rocket::Core::Element* message_label = m_Document->GetElementById(toEmp(id));
		message_label->SetInnerRML(toEmp(text));
	}

	void MessageBox::Show(bool modal)
	{
		m_Document->Show(modal ? (Rocket::Core::ElementDocument::MODAL | Rocket::Core::ElementDocument::FOCUS) : Rocket::Core::ElementDocument::FOCUS);
	}

	Rocket::Core::ElementDocument* const MessageBox::GetDocument() const
	{
		return m_Document;
	}

	MessageBox::EventSignal& MessageBox::GetEventSignal(const Rocket::Core::String& type)
	{
		m_Document->AddEventListener(type, this);
		return *(m_EventSignals[type] = std::shared_ptr<MessageBox::EventSignal>( new EventSignal ));
	}

	std::string MessageBoxMaker::GetParam(const ParamMap& params, const std::string& param_name)
	{
		auto _where = params.find(param_name);
		if (_where != params.end())
			return _where->second;
		else
			return std::string();
	}

	MessageBoxMaker::MessageBoxMaker(Rocket::Core::Context* context)
		: m_Context(context)
	{
		m_Context->AddReference();
	}

	MessageBoxMaker::~MessageBoxMaker()
	{
		m_Context->RemoveReference();
	}

	void MessageBoxMaker::addFactory(const std::string& name, const MessageBoxFactoryFn& function)
	{
		m_Factories[name] = function;
	}

	void MessageBoxMaker::removeFactory(const std::string& name)
	{
		m_Factories.erase(name);
	}

	MessageBox* MessageBoxMaker::create(const std::string& type, const std::string& params, Rocket::Core::Context* ctx)
	{
		auto _where = m_Factories.find(type);
		if (_where != m_Factories.end())
		{
			ParamMap pairizedParams;
			fe_pairize(params, pairizedParams);

			if (ctx == nullptr) ctx = m_Context;
			return _where->second(ctx, pairizedParams);
		}
		else
			return nullptr;
	}


}
