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

		virtual void EnumReferences(asIScriptEngine *engine) {};
		virtual void ReleaseAllReferences(asIScriptEngine *engine) {};

		void SetType(const std::string& type);

		void SetTitle(const std::string& title);
		void SetElement(const std::string& id, const std::string& message);

		void Show(bool modal = true);

		Rocket::Core::ElementDocument* const GetDocument() const;

		typedef boost::signals2::signal<void (Rocket::Core::Event&)> EventSignal;
		EventSignal& GetEventSignal(const EMP::Core::String& type);

	protected:
		std::string m_Filename;
		std::string m_Type;
		Rocket::Core::ElementDocument* m_Document;
		
		std::unordered_map<EMP::Core::String, std::shared_ptr<EventSignal>> m_EventSignals;

		void loadDocument(Rocket::Core::Context* context, const std::string& document_filename);
	};

	//! Simplifies message-box (or "dialog box") creation
	class MessageBoxMaker : public Singleton<MessageBoxMaker>
	{
	public:
		typedef std::map<std::string, std::string> ParamMap;
		typedef std::function<MessageBox* (Rocket::Core::Context*, const ParamMap& params)> MessageBoxFactoryFn;

		static void AddFactory(const std::string& name, const MessageBoxFactoryFn& function)
		{
			getSingleton().addFactory(name, function);
		}

		static void RemoveFactory(const std::string& name)
		{
			getSingleton().removeFactory(name);
		}

		static MessageBox* Create(const std::string& type, const std::string& params)
		{
			return getSingleton().create(type, params);
		}

		static std::string GetParam(const ParamMap& params, const std::string& param_name);

		MessageBoxMaker(Rocket::Core::Context* context);
		~MessageBoxMaker();

		void addFactory(const std::string& name, const MessageBoxFactoryFn& function);
		void removeFactory(const std::string& name);
		MessageBox* create(const std::string& type, const std::string& params);

	protected:
		std::unordered_map<std::string, MessageBoxFactoryFn> m_Factories;
		Rocket::Core::Context* m_Context;
	};

}

#endif
