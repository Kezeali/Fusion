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

#ifndef Header_FusionMessageBoxManager
#define Header_FusionMessageBoxManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/signals2.hpp>
#include <functional>
#include <Rocket/Core/EventListener.h>
#include "FusionRefCounted.h"

namespace Rocket {
	namespace Core {
		class Context;
		class ElementDocument;
	}
}

namespace std {
	template <>
	struct hash<EMP::Core::String> : public unary_function<EMP::Core::String, size_t>
	{
		size_t operator()(const EMP::Core::String& key) const
		{
			std::hash<std::string> hash_fn;
			std::string keyStr(&key[0], &key[key.Length()-1]);
			return hash_fn(keyStr);
		}
	};
}

namespace FusionEngine
{

	//! Simplifies message-box (or "dialog-box") creation
	class MessageBox : public Rocket::Core::EventListener, public RefCounted
	{
	public:
		MessageBox(const std::string& document_filename);
		MessageBox(Rocket::Core::Context* context, const std::string& document_filename);
		virtual ~MessageBox();

		virtual void ProcessEvent(Rocket::Core::Event& ev);

		//! Should call asIScriptEngine::GCEnumCallback() on every GCed object held by this object
		virtual void EnumReferences(asIScriptEngine *engine) {};
		//! Should release every GCed object held by this object
		virtual void ReleaseAllReferences(asIScriptEngine *engine) {};

		Rocket::Core::ElementDocument* const GetDocument() const;

		typedef boost::signals2::signal<void (Rocket::Core::Event&)> EventSignal;
		EventSignal& GetEventSignal(const EMP::Core::String& type);

	protected:
		std::string m_Filename;
		Rocket::Core::ElementDocument* m_Document;
		
		std::unordered_map<EMP::Core::String, std::shared_ptr<EventSignal>> m_EventSignals;

		void loadDocument(Rocket::Core::Context* context, const std::string& document_filename);
	};

	//! Simplifies message-box (or "dialog box") creation
	class MessageBoxManager
	{
	public:
		//! Constructor
		MessageBoxManager();
		//! DTOR
		virtual ~MessageBoxManager();

		void Create(const std::string& document_filename);

		typedef std::function<void (Rocket::Core::Event&)> EventFunction;

		void ConnectToEvent(MessageBox& message_box, const EMP::Core::String& type, EventFunction function);

	protected:
		std::list<boost::signals2::connection> m_EventConnections;
	};

}

#endif
