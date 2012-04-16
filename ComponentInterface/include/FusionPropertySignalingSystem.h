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

#ifndef H_FusionPropertySignalingSystem
#define H_FusionPropertySignalingSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSingleton.h"

#include "FusionSynchronisedSignalingSystem.h"
#include "FusionPropertySyncSigDetail.h"

#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	typedef SyncSig::SynchronisedSignalingSystem<int, SyncSig::PropertyCallback> PropertySignalingSystem_t;

	template <class T>
	class PersistentConnectionAgent
	{
	public:
		PersistentConnectionAgent()
			: m_PropertyID(-1)
		{}
		PersistentConnectionAgent(const std::function<void (T)>& handler)
			: m_PropertyID(-1),
			m_HandlerFn(handler)
		{}

		void SetHandlerFn(const std::function<void (T)>& handler)
		{
			m_HandlerFn = handler;
		}

		void Subscribe(PropertySignalingSystem_t& system, int property_id)
		{
			m_PropertyID = property_id;
			ActivateSubscription(system);
		}

		void SaveSubscription(std::ostream& out) { out.write((char*)&m_PropertyID, sizeof(m_PropertyID)); }
		void LoadSubscription(std::istream& in, PropertySignalingSystem_t& system);

	private:
		SyncSig::HandlerConnection_t m_ActiveConnection;
		int m_PropertyID;
		std::function<void (T)> m_HandlerFn;

		void ActivateSubscription(PropertySignalingSystem_t& system);

		void AddHandler(PropertySignalingSystem_t& system);
	};

	template <class T>
	void PersistentConnectionAgent<T>::ActivateSubscription(PropertySignalingSystem_t& system)
	{
		if (system.HasGenerator(m_PropertyID))
			AddHandler(system);
		else
		{
			// Using the same connection holder here means that the NewGenerators
			//  subscription will be broken as soon as a handler is successfully added
			m_ActiveConnection = system.SubscribeNewGenerators([this](int key, PropertySignalingSystem_t& system)
			{
				if (key == this->m_PropertyID)
					AddHandler(system);
			});
		}
	}

	template <class T>
	void PersistentConnectionAgent<T>::AddHandler(PropertySignalingSystem_t& system)
	{
		m_ActiveConnection = system.AddHandler<T>(m_PropertyID, m_HandlerFn);
	}

	template <class T>
	void PersistentConnectionAgent<T>::LoadSubscription(std::istream& in, PropertySignalingSystem_t& system)
	{
		in.read((char*)&m_PropertyID, sizeof(m_PropertyID));
		ActivateSubscription(system);
	}

	class EvesdroppingManager : public Singleton<EvesdroppingManager>
	{
	public:
		EvesdroppingManager();

		PropertySignalingSystem_t& GetSignalingSystem()
		{
			return m_SignalingSystem;
		}

		static void RegisterScriptInterface(asIScriptEngine* engine);

	protected:
		PropertySignalingSystem_t m_SignalingSystem;
	};

}

#endif
