#ifndef AUTHORITYMANAGER_H
#define AUTHORITYMANAGER_H
#if _MSC_VER > 1000
#pragma once
#endif

#include "../FusionEngine/FusionCommon.h"

#include "Ship.h"

using namespace FusionEngine;

const float s_RestTime = 1.0f;
const float s_LinearRestThreshold = 0.1f;
const float s_AngularRestThreshold = 0.12f;

const float s_AuthorityTimeOut = 1.0f;

class Authority
{
public:
	Authority()
		: m_AuthorityId(0),
		m_TimeAtRest(0.f),
		m_AuthorityTimeOut(0.f)
	{
	}

	ObjectID m_AuthorityId;
	float m_TimeAtRest;
	float m_AuthorityTimeOut;
};

typedef std::tr1::unordered_map<ObjectID, Authority> AuthorityMap;

typedef std::tr1::unordered_map<ObjectID, Command> InputMap;
typedef std::tr1::unordered_map<ObjectID, Action> ActionMap;

typedef std::tr1::unordered_map<ObjectID, ObjectID> ObjectIDMap;
typedef std::tr1::unordered_set<ObjectID> ObjectIDSet;
typedef std::list<ObjectID> ObjectIDList;

class AuthorityState
{
public:
	typedef unsigned int size_type;

	AuthorityState()
		: m_LocalAuthorityId(0)
	{
	}

	AuthorityState(ObjectID localAuthId)
		: m_LocalAuthorityId(localAuthId)
	{
	}

	bool IsLocal(ObjectID id) const;
	bool IsRemote(ObjectID id) const;
	bool IsFree(ObjectID id) const;

	Authority &GetEntityAuthority(ObjectID id);
	//const Authority &GetEntityAuthority(ObjectID id) const;

	ObjectID GetEntityAuthorityId(ObjectID id);
	//ObjectID GetEntityAuthorityId(ObjectID id) const;

	size_t EntityCount() const;

public:
	AuthorityMap m_EntityAuthority;

	ObjectID m_LocalAuthorityId;
};

class EntitySimulationState
{
public:
	bool IsInteracting(ObjectID otherEntity) const;

	Command& GetInput();
	Action& GetAction();

	Command m_Input;
	Action m_Action;

	bool m_Enabled;

	ObjectIDSet m_Interactions;
};

typedef std::tr1::unordered_map<ObjectID, EntitySimulationState> EntityStateMap;

class SimulationState
{
public:
	SimulationState()
	{
	}

	bool IsInteracting(ObjectID entity, ObjectID otherEntity) const;
	const EntitySimulationState& GetState(ObjectID id) const;

	Command& GetInput(ObjectID id);
	Action& GetAction(ObjectID id);
	bool IsEnabled(ObjectID id);

	void SetEntityInput(ObjectID id, const Command& input);
	void SetEntityAction(ObjectID id, const Action& state);
	void SetEnabled(ObjectID id, bool enabled);

public:
	EntityStateMap m_Entities;
};

// AuthId 0  -> Free
// AuthId 1  -> Server
// AuthId 2+ -> Client
class AuthorityManager
{
public:
	AuthorityManager()
		: m_LocalAuthorityId(0),
		m_AuthorityByDefault(false)
	{
	}

	AuthorityManager(ObjectID localId, bool authByDefault)
		: m_LocalAuthorityId(localId),
		m_AuthorityByDefault(authByDefault),
		m_AuthorityState(localId)
	{
	}

	void SetLocalAuthorityId(ObjectID localId);

	void SetEntityOwnership(ObjectID entityId, ObjectID ownerAuthorityId = 0);
	void SetLocalEntityOwnership(ObjectID entityId);

	ObjectID GetLocalAuthorityId() const;

	ObjectID GetEntityOwnership(ObjectID entityId) const;

	void InjectInput(ObjectID entityId, const Command& input);
	void InjectState(ObjectID authorityId, ObjectID entityId, const Action& state);

	Authority& GetEntityAuthority(ObjectID id);

	const AuthorityState& GetAuthorityState() const;

	void SetSimulationState(const SimulationState& state);
	void ReadSimulationState(SimulationState &state);

	void Update(float deltaTime);

protected:
	ObjectID m_LocalAuthorityId;
	bool m_AuthorityByDefault;

	ObjectIDSet m_OwnedEntities; // Entity pointers to entities owned locally
	// ObjectID's mapped to the authority ID of the client that owns them
	ObjectIDMap m_EntityOwnership;

	AuthorityState m_AuthorityState;
	SimulationState m_SimulationState;

	typedef std::tr1::unordered_map<ObjectID, Command> EntityInputs;
	typedef std::tr1::unordered_map<ObjectID, Action> EntityStates;
	// Client input buffer
	EntityInputs m_CurrentInput;
	EntityStates m_CurrentState;
};


bool EntitySimulationState::IsInteracting(ObjectID otherEntity) const
{
	return m_Interactions.find(otherEntity) != m_Interactions.end();
}

Command& EntitySimulationState::GetInput()
{
	return m_Input;
}

Action& EntitySimulationState::GetAction()
{
	return m_Action;
}


bool SimulationState::IsInteracting(ObjectID entity, ObjectID otherEntity) const
{
	EntityStateMap::const_iterator _where = m_Entities.find(entity);
	if (_where == m_Entities.end())
		return false;
	return _where->second.IsInteracting(otherEntity);
}

const EntitySimulationState& SimulationState::GetState(ObjectID id) const
{
	assert (m_Entities.find(id) != m_Entities.end());
	return m_Entities.find(id)->second;
}

Command& SimulationState::GetInput(ObjectID id)
{
	//assert(m_Entities.find(id) != m_Entities.end());
	return m_Entities[id].m_Input;
}

Action& SimulationState::GetAction(ObjectID id)
{
	//assert(m_Entities.find(id) != m_Entities.end());
	return m_Entities[id].m_Action;
}

bool SimulationState::IsEnabled(ObjectID id)
{
	//assert(m_Entities.find(id) != m_Entities.end());
	return m_Entities[id].m_Enabled;
}

void SimulationState::SetEntityInput(ObjectID id, const Command& input)
{
	m_Entities[id].m_Input = input;
}

void SimulationState::SetEntityAction(ObjectID id, const Action& state)
{
	m_Entities[id].m_Action = state;
}

void SimulationState::SetEnabled(ObjectID id, bool enabled)
{
	m_Entities[id].m_Enabled = enabled;
}


bool AuthorityState::IsLocal(ObjectID id) const
{
	AuthorityMap::const_iterator _where = m_EntityAuthority.find(id);
	if (_where == m_EntityAuthority.end())
		return false;
	return _where->second.m_AuthorityId == m_LocalAuthorityId;
}

bool AuthorityState::IsRemote(ObjectID id) const
{
	return !IsLocal(id);
}

bool AuthorityState::IsFree(ObjectID id) const
{
	AuthorityMap::const_iterator _where = m_EntityAuthority.find(id);
	if (_where == m_EntityAuthority.end())
		return false;
	return _where->second.m_AuthorityId == 0;
}

Authority& AuthorityState::GetEntityAuthority(ObjectID id)
{
	//AuthorityMap::iterator _where = m_EntityAuthority.find(id);
	//assert(_where != m_EntityAuthority.end());
	//return _where->second;
	return m_EntityAuthority[id];
}

//const Authority& AuthorityState::GetEntityAuthority(ObjectID id) const
//{
//	AuthorityMap::const_iterator _where = m_EntityAuthority.find(id);
//	assert(_where != m_EntityAuthority.end());
//	return _where->second;
//}

ObjectID AuthorityState::GetEntityAuthorityId(ObjectID id)
{
	return GetEntityAuthority(id).m_AuthorityId;
}

//ObjectID AuthorityState::GetEntityAuthorityId(ObjectID id) const
//{
//	return GetEntityAuthority(id).m_AuthorityId;
//}

AuthorityState::size_type AuthorityState::EntityCount() const
{
	return m_EntityAuthority.size();
}


void AuthorityManager::SetLocalAuthorityId(ObjectID localId)
{
	m_LocalAuthorityId = localId;
	m_AuthorityState.m_LocalAuthorityId = localId;
}

void AuthorityManager::SetEntityOwnership(ObjectID entityId, ObjectID ownerAuthorityId)
{
	if (ownerAuthorityId != 0)
		m_EntityOwnership.insert(ObjectIDMap::value_type(entityId, ownerAuthorityId));
	else
		m_EntityOwnership.erase(entityId);

	if (ownerAuthorityId == m_LocalAuthorityId)
		m_OwnedEntities.insert(entityId);
	else
		m_OwnedEntities.erase(entityId);
}

void AuthorityManager::SetLocalEntityOwnership(ObjectID entityId)
{
	if (m_LocalAuthorityId != 0)
	{
		m_EntityOwnership.insert(ObjectIDMap::value_type(entityId, m_LocalAuthorityId));
		m_OwnedEntities.insert(entityId);
	}
}

ObjectID AuthorityManager::GetLocalAuthorityId() const
{
	return m_LocalAuthorityId;
}

ObjectID AuthorityManager::GetEntityOwnership(ObjectID entityId) const
{
	ObjectIDMap::const_iterator _where = m_EntityOwnership.find(entityId);
	if (_where == m_EntityOwnership.end())
		return 0;
	return _where->second;
}

void AuthorityManager::InjectInput(ObjectID entityId, const Command& input)
{
	//assert( playerId >= 0 );
	//assert( playerId < MaxPlayers );

	m_SimulationState.GetInput(entityId) = input;
	// Client buffers the most recent input to apply at the next Update
	if (m_LocalAuthorityId > 1)
		m_CurrentInput[entityId] = input;
}

void AuthorityManager::InjectState(ObjectID authorityId, ObjectID entityId, const Action& state)
{
	//assert(authorityId >= -1);
	//assert(authorityId < MaxPlayers);

	//assert(cubeId >= 0);
	//assert(cubeId < MaxCubes);
	//assert(cubeId < m_AuthorityState.EntityCount());

	// Default authority overrides non-authoritative ('free') injection
	if (m_AuthorityByDefault && authorityId == 0)
		return;
	// Ignore injections for owned entities
	if (m_OwnedEntities.find(entityId) != m_OwnedEntities.end())
		return;
	// If we have default authority, don't let other authorities override ours
	if (m_AuthorityByDefault && m_AuthorityState.IsLocal(entityId) && authorityId != m_LocalAuthorityId)
		return;
	// If there is an existing authority, don't override it with no authority
	if (authorityId == 0 && !m_AuthorityState.IsFree(entityId))
		return;

	// change authority if applicable
	if (authorityId != 0 && authorityId != m_AuthorityState.GetEntityAuthority(entityId).m_AuthorityId)
		m_AuthorityState.GetEntityAuthority(entityId).m_AuthorityId = authorityId;

	// set cube state
	m_SimulationState.GetAction(entityId) = state;

	if (m_LocalAuthorityId > 1)
		m_CurrentState[entityId] = state;
}

Authority& AuthorityManager::GetEntityAuthority(FusionEngine::ObjectID id)
{
	return m_AuthorityState.GetEntityAuthority(id);
}

const AuthorityState& AuthorityManager::GetAuthorityState() const
{
	return m_AuthorityState;
}

void AuthorityManager::SetSimulationState(const SimulationState &state)
{
	m_SimulationState = state;
}

void AuthorityManager::ReadSimulationState(SimulationState &state)
{
	state = m_SimulationState;
}

void AuthorityManager::Update(float deltaTime)
{
	//assert( localAuthorityId >= 0 );
	//assert( localAuthorityId < MaxPlayers );

	if (m_LocalAuthorityId > 1)
	{
		for (EntityInputs::iterator it = m_CurrentInput.begin(), end = m_CurrentInput.end(); it != end; ++it)
		{
			m_SimulationState.GetInput(it->first) = it->second;
		}
		for (EntityStates::iterator it = m_CurrentState.begin(), end = m_CurrentState.end(); it != end; ++it)
		{
			m_SimulationState.GetAction(it->first) = it->second;
		}

		m_CurrentInput.clear();
		m_CurrentState.clear();
	}

	// detect local authority changes
	//m_AuthorityState.m_Size = simulationState.numCubes;
	EntityStateMap& entityStates = m_SimulationState.m_Entities;
	AuthorityMap& entityAuthority = m_AuthorityState.m_EntityAuthority;
	for (EntityStateMap::iterator simIt = entityStates.begin(), simEnd = entityStates.end();
		simIt != simEnd; ++simIt)
	{
		// Update the authority state with new entities
		if (entityAuthority.find(simIt->first) == entityAuthority.end())
			entityAuthority.insert(AuthorityMap::value_type(simIt->first, Authority()));

		for (ObjectIDMap::iterator entityIt = m_EntityOwnership.begin(), entityEnd = m_EntityOwnership.end();
			entityIt != entityEnd; ++entityIt)
		{
			if (simIt->second.IsInteracting(entityIt->first) && m_AuthorityState.IsFree(entityIt->first))
			{
				Authority& authority = m_AuthorityState.GetEntityAuthority(entityIt->first);

				authority.m_AuthorityId = entityIt->second;
				authority.m_AuthorityTimeOut = 0.0f;
				break;
			}
		}
	}

	/// Box2D does this
	// detect simulation bodies at rest
	//for (AuthorityMap::iterator authIt = entityAuthority.begin(), authEnd = entityAuthority.end();
	//		authIt != authEnd; ++authIt)
	//{
	//	const float linearSpeed = m_SimulationState.GetAction(authIt->first).linearVelocity.length();
	//	const float angularSpeed = m_SimulationState.GetAction(authIt->first).angularSpeed;

	//	const bool resting = linearSpeed < s_LinearRestThreshold && angularSpeed < s_AngularRestThreshold;

	//	if ( resting )
	//		authIt->second.m_TimeAtRest += deltaTime;
	//	else
	//		authIt->second.m_TimeAtRest = 0.0f;

	//	m_SimulationState.m_Entities[authIt->first].m_Enabled = authIt->second.m_TimeAtRest < s_RestTime;
	//}

	// update authority timeout
	for (AuthorityMap::iterator authIt = entityAuthority.begin(), authEnd = entityAuthority.end();
			authIt != authEnd; ++authIt)
	{
		// Check for entities that have been removed from the state
		if (entityStates.find(authIt->first) == entityStates.end())
		{
			AuthorityMap::iterator toRemove = authIt++;
			entityAuthority.erase(toRemove);
			if (authIt == authEnd)
				break;
		}

		if (!m_SimulationState.IsEnabled(authIt->first))
			authIt->second.m_AuthorityTimeOut += deltaTime;
		else
			authIt->second.m_AuthorityTimeOut = 0.0f;

		if (authIt->second.m_AuthorityTimeOut > s_AuthorityTimeOut)
			authIt->second.m_AuthorityId = 0;
	}

	// force authority for player owned entities
	for (ObjectIDMap::iterator ownerIt = m_EntityOwnership.begin(), ownerEnd = m_EntityOwnership.end();
		ownerIt != ownerEnd; ++ownerIt)
	{
		//assert( playerCubeId >= 0 );
		//assert( playerCubeId < MaxCubes );

		m_AuthorityState.GetEntityAuthority(ownerIt->first).m_AuthorityId = ownerIt->second;
		m_AuthorityState.GetEntityAuthority(ownerIt->first).m_TimeAtRest = 0.0f;
	}
}

#endif