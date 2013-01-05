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

#include <RakNet/BitStream.h>
#include <tbb/atomic.h>

namespace FusionEngine
{

	typedef int PropertyID;

	typedef SyncSig::SynchronisedSignalingSystem<PropertyID, SyncSig::PropertyCallback> PropertySignalingSystem_t;

	class PersistentFollower
	{
	public:
		PersistentFollower()
			: m_PropertyID(-1)
		{}

		virtual ~PersistentFollower();

		void Subscribe(PropertySignalingSystem_t& system, int property_id)
		{
			m_PropertyID = property_id;
			ActivateSubscription(system);
		}

		bool IsSubscribed() const { return m_PropertyID != -1; }

		void SetFollowedPropertyID(PropertyID value) { m_PropertyID = value; }
		PropertyID GetFollowedPropertyID() const { return m_PropertyID; }

		void SaveSubscription(std::ostream& out)
		{
			out.write((char*)&m_PropertyID, sizeof(m_PropertyID));
		}
		void SaveSubscription(RakNet::BitStream& out)
		{
			if (IsSubscribed())
			{
				out.Write1();
				out.Write(GetFollowedPropertyID());
			}
			else
				out.Write0();
		}
		void LoadSubscription(std::istream& in, PropertySignalingSystem_t& system)
		{
			in.read((char*)&m_PropertyID, sizeof(m_PropertyID));
			ActivateSubscription(system);
		}
		void LoadSubscription(RakNet::BitStream& in)
		{
			if (in.ReadBit())
			{
				auto prop_id = GetFollowedPropertyID();
				in.Read(prop_id);
				SetFollowedPropertyID(prop_id);
			}
		}
		
		void ActivateSubscription(PropertySignalingSystem_t& system);

	private:
		SyncSig::HandlerConnection_t m_WaitingConnection;
		SyncSig::HandlerConnection_t m_ActiveConnection;
		PropertyID m_PropertyID;

		virtual SyncSig::HandlerConnection_t AddHandler(PropertySignalingSystem_t& system) = 0;
	};

	typedef std::shared_ptr<PersistentFollower> PersistentFollowerPtr;

	template <class T>
	class PersistentConnectionAgent : public PersistentFollower
	{
	public:
		PersistentConnectionAgent()
			: PersistentFollower()
		{}
		PersistentConnectionAgent(const std::function<void (T)>& handler)
			: PersistentFollower(),
			m_HandlerFn(handler)
		{}

		void SetHandlerFn(const std::function<void (T)>& handler) { m_HandlerFn = handler; }

	private:
		std::function<void (T)> m_HandlerFn;

		SyncSig::HandlerConnection_t AddHandler(PropertySignalingSystem_t& system);
	};

	class EvesdroppingManager : public Singleton<EvesdroppingManager>
	{
	public:
		EvesdroppingManager();

		PropertySignalingSystem_t& GetSignalingSystem()
		{
			return m_SignalingSystem;
		}

		PropertyID GenerateNextId()
		{
			return m_NextID++;
		}

		static void RegisterScriptInterface(asIScriptEngine* engine);

	protected:
		PropertySignalingSystem_t m_SignalingSystem;

		tbb::atomic<PropertyID> m_NextID;
	};

	inline PersistentFollower::~PersistentFollower()
	{
		if (m_WaitingConnection)
			EvesdroppingManager::getSingleton().GetSignalingSystem().OnUnsubscribeNewGenerators(m_PropertyID);
	}

	inline void PersistentFollower::ActivateSubscription(PropertySignalingSystem_t& system)
	{
		if (IsSubscribed())
		{
			if (system.HasGenerator(m_PropertyID))
				m_ActiveConnection = AddHandler(system);
			else
			{
				m_ActiveConnection.reset();
				m_WaitingConnection = system.SubscribeNewGenerators(m_PropertyID, [this](PropertyID key, PropertySignalingSystem_t& system)
				{
					m_WaitingConnection.reset();
					m_ActiveConnection = AddHandler(system);
				});
			}
		}
	}

	template <class T>
	inline SyncSig::HandlerConnection_t PersistentConnectionAgent<T>::AddHandler(PropertySignalingSystem_t& system)
	{
		return system.AddHandler<T>(GetFollowedPropertyID(), m_HandlerFn);
	}

}

#endif
