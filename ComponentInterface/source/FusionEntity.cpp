/*
*  Copyright (c) 2007-2012 Fusion Project Team
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

#include "FusionEntity.h"

#include "FusionArchetypalEntityManager.h"
#include "FusionExceptionFactory.h"
#include "FusionTransformComponent.h"
#include "FusionPlayerInput.h"
#include "FusionProfiling.h"
#include "FusionResourceManager.h"
#include "FusionStreamedResourceUser.h"
#include "FusionNetworkManager.h"


#include <boost/lexical_cast.hpp>

namespace FusionEngine
{

	Entity::Entity(EntityRepo* manager, const ComponentPtr& transform_component)
		: m_Name(""),
		m_HasDefaultName(false),
		m_Id(0),
		m_OwnerID(0),
		m_Terrain(false),
		m_Manager(manager),
		m_Flags(0),
		m_CellIndex(0xFFFFFFFF, 0xFFFFFFFF),
		m_Domain(GAME_DOMAIN),
		m_Layer(0),
		m_Depth(0),
		m_Spawned(false),
		m_StreamedIn(false),
		m_Paused(false),
		m_Hidden(false),
		m_WaitStepsRemaining(0),
		m_MarkedToRemove(false)
	{
		m_Authority = 0;

		m_MarkedToDeactivate = false;

		m_LockingReferences = 0;
		m_GCFlag = false;

		// Cache the transform component for quick access
		m_Transform = dynamic_cast<ITransform*>(transform_component.get());
		FSN_ASSERT(m_Transform);

		AddComponent(transform_component);
	}

	Entity::~Entity()
	{
		std::for_each(m_StreamedResources.begin(), m_StreamedResources.end(),
			[](StreamedResourceUser *user){ user->DestructionNotification = StreamedResourceUser::DestructionNotificationFn(); });
	}

	void Entity::SetName(const std::string &name)
	{
		m_Name = name;
		m_HasDefaultName = false;
	}

	const std::string &Entity::GetName() const
	{
		return m_Name;
	}

	void Entity::_notifyDefaultName(const std::string &name)
	{
		SetName(name);
		m_HasDefaultName = true;
	}

	bool Entity::HasDefaultName() const
	{
		return m_HasDefaultName;
	}

	void Entity::SetID(ObjectID id)
	{
		m_Id = id;
	}

	ObjectID Entity::GetID() const
	{
		return m_Id;
	}

	void Entity::SetOwnerID(PlayerID owner)
	{
		m_OwnerID = owner;
	}

	PlayerID Entity::GetOwnerID() const
	{
		return m_OwnerID;
	}

	void Entity::SetAuthority(PlayerID authority)
	{
		m_Authority = authority;
	}

	PlayerID Entity::GetAuthority() const
	{
		return m_Authority;
	}

	void Entity::SetTerrain(bool v)
	{
		m_Terrain = v;
	}

	bool Entity::IsTerrain() const
	{
		return m_Terrain;
	}

	bool Entity::IsSyncedEntity() const
	{
		return m_Id > 0;
	}

	bool Entity::IsPseudoEntity() const
	{
		return !IsSyncedEntity();
	}

	void Entity::SetArchetype(const std::string& type)
	{
		m_Type = type;
	}

	const std::string& Entity::GetArchetype() const
	{
		return m_Type;
	}

	ComponentPtr Entity::GetTransform() const
	{
		return m_Transform.p;
	}

	const Vector2 &Entity::GetPosition()
	{
		return m_Transform->Position.Get();
	}

	void Entity::SetPosition(const Vector2 &position)
	{
		m_Transform->Position.Set(position);
	}

	float Entity::GetAngle() const
	{
		return m_Transform->Angle.Get();
	}

	void Entity::SetAngle(float angle)
	{
		m_Transform->Angle.Set(angle);
	}

	void Entity::AddReference(EntityPtr entity)
	{
		IncrRefCount();
		tbb::mutex::scoped_lock lock(m_InRefsMutex);
		m_ReferencingEntities.insert(entity);
	}

	void Entity::RemoveReference(EntityPtr entity)
	{
		{
		tbb::mutex::scoped_lock lock(m_InRefsMutex);
		m_ReferencingEntities.erase(entity);
		}
		DecrRefCount();
	}

	void Entity::HoldReference(EntityPtr entity)
	{
		if (entity)
		{
			tbb::spin_rw_mutex::scoped_lock lock(m_OutRefsMutex);
			auto result = m_ReferencedEntities.insert(std::make_pair(entity, 0u));
			++(result.first->second);
			if (result.second && IsActive())
				entity->AddReference(this->shared_from_this());
		}
	}

	void Entity::DropReference(EntityPtr entity)
	{
		if (entity)
		{
			tbb::spin_rw_mutex::scoped_lock lock(m_OutRefsMutex);
			auto it = m_ReferencedEntities.find(entity);
			if (it != m_ReferencedEntities.end() && --it->second == 0)
			{
				m_ReferencedEntities.erase(entity);

				lock.release();

				if (IsActive())
					entity->RemoveReference(this->shared_from_this());
			}
		}
	}

	void Entity::AddComponent(const ComponentPtr& component, std::string identifier)
	{
		FSN_ASSERT(component);
		// Don't allow the addition of multiple transform components:
		FSN_ASSERT(dynamic_cast<ITransform*>(component.get()) == nullptr || dynamic_cast<ITransform*>(component.get()) == m_Transform.get());
		// TODO:
		//FSN_ASSERT(!DeltaTime::IsExecuting());

		tbb::spin_rw_mutex::scoped_lock lock(m_ComponentsMutex);

		component->InitInterfaces();

		// Add the new component to the component-by-interface map
		const auto& interfaceNames = component->GetInterfaces();
		for (auto it = interfaceNames.begin(), end = interfaceNames.end(); it != end; ++it)
		{
			auto& implementors = m_ComponentInterfaces[*it];
			std::string interfaceIdentifier = identifier;
			if (interfaceIdentifier.empty())
			{
				if (!implementors.empty())
				{
					std::stringstream str; str << *it << implementors.size();
					interfaceIdentifier = str.str();
				}
				else
					interfaceIdentifier = *it;
			}
			// TEMP:
			component->SetIdentifier(identifier);

			FSN_ASSERT(implementors.find(interfaceIdentifier) == implementors.end()); // no duplicates
			implementors[interfaceIdentifier] = component;
		}
		// Add the new component to the main list
		m_Components.push_back(component);
		component->SetParent(this);

		// Update the archetype definition
		if (m_ArchetypeDefinitionAgent)
		{
			m_ArchetypeDefinitionAgent->ComponentAdded(component);
		}
		if (m_ArchetypeAgent)
		{
			m_ArchetypeAgent->ComponentAddedToInstance(component);
		}
	}

	void Entity::RemoveComponent(const ComponentPtr& component)
	{
		FSN_ASSERT(std::find(m_Components.begin(), m_Components.end(), component) != m_Components.end());

		tbb::spin_rw_mutex::scoped_lock lock(m_ComponentsMutex);

		const std::string identifier = component->GetIdentifier();

		// Remove the given component from the main list, and notify all other components
		for (auto it = m_Components.begin(); it != m_Components.end(); )
		{
			if (*it == component)
			{
				//component->SetPropChangedQueue(nullptr);
				component->SetParent(nullptr);
				it = m_Components.erase(it);
			}
			else
			{
				(*it)->OnSiblingRemoved(component);
				component->OnSiblingRemoved(*it); // TODO: consider: should the component being removed be notified?
				++it;
			}
		}
		// Remove all the other references to this component from the component-by-interface map
		for (auto it = component->GetInterfaces().begin(), end = component->GetInterfaces().end(); it != end; ++it)
		{
			auto& implementors = m_ComponentInterfaces[*it];
			if (identifier.empty())
			{
				auto _where = std::find_if(implementors.begin(), implementors.end(), [&](const std::pair<std::string, ComponentPtr>& entry)->bool
				{ return entry.second == component; });
				implementors.erase(_where);
			}
			else
				implementors.erase(identifier);
		}

		// Update the archetype definition
		if (m_ArchetypeDefinitionAgent)
		{
			m_ArchetypeDefinitionAgent->ComponentRemoved(component);
		}
		if (m_ArchetypeAgent)
		{
			m_ArchetypeAgent->ComponentRemovedFromInstance(component);
		}
	}

	bool Entity::HasComponent(const ComponentPtr& component)
	{
		return std::find(m_Components.begin(), m_Components.end(), component) != m_Components.end();
	}

	void Entity::OnComponentActivated(const ComponentPtr& component)
	{
		FSN_PROFILE("OnComponentActivated");
		tbb::spin_rw_mutex::scoped_lock(m_ComponentsMutex, false);

		// Notify all other components of the new component, and notify the new component of the existing components
		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			(*it)->OnSiblingAdded(component);
			component->OnSiblingAdded(*it);
		}
	}

	ComponentPtr Entity::GetComponent(const std::string& type, std::string identifier) const
	{
		auto _where = m_ComponentInterfaces.find(type);
		if (_where != m_ComponentInterfaces.end())
		{
			FSN_ASSERT(!_where->second.empty());
			if (identifier.empty())
			{
				if (_where->second.begin()->second->IsActive())
					return _where->second.begin()->second;
			}
			else
			{
				auto implEntry = _where->second.find(identifier);
				if (implEntry != _where->second.end())
					if (implEntry->second->IsActive())
						return implEntry->second;
			}
		}
		return ComponentPtr();
	}

	const std::vector<ComponentPtr>& Entity::GetComponents() const
	{
		return m_Components;
	}

	const Entity::ComInterfaceMap& Entity::GetInterfaces() const
	{
		return m_ComponentInterfaces;
	}

	EntityPtr Entity::Clone(ComponentFactory* factory) const
	{
		if (!IsArchetypal())
		{
			auto entity = std::make_shared<Entity>(GetManager(), m_Transform.p->Clone(factory));

			entity->SetOwnerID(GetOwnerID());
			entity->SetTerrain(IsTerrain());

			// Note: ArchetypeDefinitionAgent is intentionally not copied, since
			//  the cloned entity doesn't define the archetype (duh)

			// Copy the rest of the components (transform is added by the ctor)
			for (auto it = m_Components.cbegin(); it != m_Components.cend(); ++it)
			{
				if (dynamic_cast<ITransform*>(it->get()) != m_Transform.get())
					entity->AddComponent((*it)->Clone(factory), (*it)->GetIdentifier());
			}
			return std::move(entity);
		}
		else
			FSN_EXCEPT(NotImplementedException, "Can't clone archetypal entities");
	}

	void Entity::SynchroniseParallelEdits()
	{
		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			(*it)->SynchronisePropertiesNow();
		}
	}

	void Entity::AddTag(const std::string &tag)
	{
		m_Tags.insert(tag);
	}

	void Entity::RemoveTag(const std::string &tag)
	{
		m_Tags.erase(tag);
	}

	void Entity::ClearTags()
	{
		m_Tags.clear();
	}

	bool Entity::CheckTag(const std::string &tag) const
	{
		return m_Tags.find(tag) != m_Tags.end();
	}

	StringVector Entity::GetTags() const
	{
		StringVector tagArray;
		for (TagSet::const_iterator it = m_Tags.begin(), end = m_Tags.end(); it != end; ++it)
			tagArray.push_back(*it);

		return tagArray;
	}

	void Entity::_notifyPausedTag(const std::string &tag)
	{
		if (CheckTag(tag))
			m_PausedTags.insert(tag);
	}

	void Entity::_notifyResumedTag(const std::string &tag)
	{
		if (CheckTag(tag))
			m_PausedTags.erase(tag);
	}

	void Entity::_notifyHiddenTag(const std::string &tag)
	{
		if (CheckTag(tag))
			m_HiddenTags.insert(tag);
	}

	void Entity::_notifyShownTag(const std::string &tag)
	{
		if (CheckTag(tag))
			m_HiddenTags.erase(tag);
	}

	const StringSet &Entity::GetPausedTags() const
	{
		return m_PausedTags;
	}

	const StringSet &Entity::GetHiddenTags() const
	{
		return m_HiddenTags;
	}

	bool Entity::IsPausedByTag() const
	{
		return !m_PausedTags.empty();
	}

	bool Entity::IsHiddenByTag() const
	{
		return !m_HiddenTags.empty();
	}

	void Entity::SetTagFlags(unsigned int flags)
	{
		m_Flags = flags;
	}

	void Entity::AddTagFlag(unsigned int flag)
	{
		m_Flags |= flag;
	}

	void Entity::RemoveTagFlag(unsigned int flag)
	{
		m_Flags &= ~flag;
	}

	unsigned int Entity::GetTagFlags() const
	{
		return m_Flags;
	}

	void Entity::SetDomain(EntityDomain domain_index)
	{
		if (domain_index < s_EntityDomainCount)
			m_Domain = domain_index;
		else
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to create an Entity in domain " + boost::lexical_cast<std::string>(domain_index) + ": valid domain values are 0-" + boost::lexical_cast<std::string>(s_EntityDomainCount-1));
			m_Domain = s_EntityDomainCount-1;
		}
	}

	EntityDomain Entity::GetDomain() const
	{
		return m_Domain;
	}

	void Entity::SetLayer(size_t layer)
	{
		m_Layer = layer;
	}

	size_t Entity::GetLayer() const
	{
		return m_Layer;
	}

	void Entity::SetStreamedIn(bool is_streamed_in)
	{
		m_StreamedIn = is_streamed_in;
	}

	bool Entity::IsStreamedIn() const
	{
		return m_StreamedIn;
	}

	bool Entity::IsActive() const
	{
		return IsStreamedIn();
	}

	void Entity::SetStreamingCellIndex(CellHandle index)
	{
		m_CellIndex = index;
	}

	CellHandle Entity::GetStreamingCellIndex() const
	{
		return m_CellIndex;
	}

	void Entity::SetPaused(bool is_paused)
	{
		m_Paused = is_paused;
	}

	bool Entity::IsPaused() const
	{
		return m_Paused;
	}

	void Entity::SetHidden(bool is_hidden)
	{
		m_Hidden = is_hidden;
	}

	bool Entity::IsHidden() const
	{
		return m_Hidden;
	}

	void Entity::SetDepth(int depth)
	{
		m_Depth = depth;
	}

	int Entity::GetDepth() const
	{
		return m_Depth;
	}

	void Entity::SetWait(unsigned int steps)
	{
		m_WaitStepsRemaining = steps;
	}

	bool Entity::Wait()
	{
		if (m_WaitStepsRemaining > 0)
		{
			--m_WaitStepsRemaining;
			return false;
		}
		else
			return true;
	}

	void Entity::MarkToRemove()
	{
		m_MarkedToRemove = true;
	}

	bool Entity::IsMarkedToRemove() const
	{
		return m_MarkedToRemove;
	}

	void Entity::MarkToDeactivate()
	{
		m_MarkedToDeactivate = true;
	}

	void Entity::RemoveDeactivateMark()
	{
		m_MarkedToDeactivate = false;
	}

	bool Entity::IsMarkedToDeactivate() const
	{
		return m_MarkedToDeactivate;
	}

	//RenderableArray &Entity::GetRenderables()
	//{
	//	return m_Renderables;
	//}

	//void Entity::AddRenderable(RenderablePtr renderable)
	//{
	//	m_Renderables.push_back(renderable);
	//}

	//void Entity::RemoveRenderable(RenderablePtr renderable)
	//{
	//	for (RenderableArray::iterator it = m_Renderables.begin(), end = m_Renderables.end(); it != end; ++it)
	//	{
	//		if (*it == renderable)
	//		{
	//			m_Renderables.erase(it);
	//			break;
	//		}
	//	}
	//}

	//void Entity::RemoveRenderablesWithTag(const std::string &tag)
	//{
	//	auto newEnd = std::remove_if(m_Renderables.begin(), m_Renderables.end(),
	//		[&](RenderablePtr &renderable)->bool
	//	{
	//		return renderable->HasTag(tag);
	//	});
	//	m_Renderables.erase(newEnd, m_Renderables.end());
	//}

	//CL_Rectf Entity::CalculateOnScreenAABB() const
	//{
	//	CL_Rectf bounding_box;
	//	if (!m_Renderables.empty())
	//	{
	//		auto it = m_Renderables.cbegin(), end = m_Renderables.cend();
	//		bounding_box = (*it)->GetAABB();
	//		for (++it; it != end; ++it)
	//			bounding_box.bounding_rect((*it)->GetAABB());
	//	}
	//	return bounding_box;
	//}

	//void Entity::AddStreamedResource(StreamedResourceUser * const user)
	//{
	//	m_StreamedResources.push_back(user);
	//	user->DestructionNotification = [this](StreamedResourceUser * const user){ this->RemoveStreamedResource(user); };
	//}

	//void Entity::RemoveStreamedResource(StreamedResourceUser * const user)
	//{
	//	user->DestructionNotification = StreamedResourceUser::DestructionNotificationFn();
	//	for (auto it = m_StreamedResources.begin(), end = m_StreamedResources.end(); it != end; ++it)
	//		if (*it == user)
	//		{
	//			m_StreamedResources.erase(it);
	//			break;
	//		}
	//}

	//void Entity::SetStreamedResources(const StreamedResourceArray &resources)
	//{
	//	m_StreamedResources = resources;
	//}

	//const Entity::StreamedResourceArray &Entity::GetStreamedResources() const
	//{
	//	return m_StreamedResources;
	//}

	//template <typename T>
	//void getPropValueOfType(boost::any &value, void *prop_addr, asUINT property_index)
	//{
	//	value = *(T*)prop_addr;
	//}
	//template <typename T>
	//void setPropValueOfType(const boost::any &value, void *prop_addr, asUINT property_index)
	//{
	//	*(T*)prop_addr = boost::any_cast<T>(value);
	//}

	//const boost::any& Entity::GetPropertyValue(unsigned int index) const
	//{
	//	boost::any value;

	//	int type_id = GetPropertyType(index);

	//	if (type_id == pt_bool)
	//		getPropValueOfType<bool>(value, GetAddressOfProperty(index), index);
	//	// Integer types
	//	else if (type_id == pt_int8)
	//		getPropValueOfType<int8_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_int16)
	//		getPropValueOfType<int16_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_int32)
	//		getPropValueOfType<int32_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_int64)
	//		getPropValueOfType<int64_t>(value, GetAddressOfProperty(index), index);
	//	// ... unsigned
	//	else if (type_id == pt_uint8)
	//		getPropValueOfType<uint8_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_uint16)
	//		getPropValueOfType<uint16_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_uint32)
	//		getPropValueOfType<uint32_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_uint64)
	//		getPropValueOfType<uint64_t>(value, GetAddressOfProperty(index), index);
	//	// Floating point types
	//	else if (type_id == pt_float)
	//		getPropValueOfType<float>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_double)
	//		getPropValueOfType<double>(value, GetAddressOfProperty(index), index);

	//	// Class types
	//	else if (type_id == pt_entity)
	//		getPropValueOfType<Entity*>(value, GetAddressOfProperty(index), index);
	//	else if (type_id & pt_string)
	//	{
	//		if (type_id & pt_pointer_flag)
	//			getPropValueOfType<std::string*>(value, GetAddressOfProperty(index), index);
	//		else
	//			getPropValueOfType<std::string>(value, GetAddressOfProperty(index), index);
	//	}
	//	else if (type_id & pt_vector)
	//	{
	//		if (type_id & pt_pointer_flag)
	//			getPropValueOfType<Vector2*>(value, GetAddressOfProperty(index), index);
	//		else
	//			getPropValueOfType<Vector2>(value, GetAddressOfProperty(index), index);
	//	}

	//	return value;
	//}

	//void Entity::SetPropertyValue(unsigned int index, const boost::any &value)
	//{
	//	int type_id = GetPropertyType(index);

	//	if (type_id == pt_bool)
	//		setPropValueOfType<bool>(value, GetAddressOfProperty(index), index);
	//	// Integer types
	//	else if (type_id == pt_int8)
	//		setPropValueOfType<int8_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_int16)
	//		setPropValueOfType<int16_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_int32)
	//		setPropValueOfType<int32_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_int64)
	//		setPropValueOfType<int64_t>(value, GetAddressOfProperty(index), index);
	//	// ... unsigned
	//	else if (type_id == pt_uint8)
	//		setPropValueOfType<uint8_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_uint16)
	//		setPropValueOfType<uint16_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_uint32)
	//		setPropValueOfType<uint32_t>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_uint64)
	//		setPropValueOfType<uint64_t>(value, GetAddressOfProperty(index), index);
	//	// Floating point types
	//	else if (type_id == pt_float)
	//		setPropValueOfType<float>(value, GetAddressOfProperty(index), index);
	//	else if (type_id == pt_double)
	//		setPropValueOfType<double>(value, GetAddressOfProperty(index), index);

	//	// Class types
	//	else if (type_id == pt_entity)
	//		setPropValueOfType<Entity*>(value, GetAddressOfProperty(index), index);
	//	else if (type_id & pt_string)
	//	{
	//		if (type_id & pt_pointer_flag)
	//			setPropValueOfType<std::string*>(value, GetAddressOfProperty(index), index);
	//		else
	//			setPropValueOfType<std::string>(value, GetAddressOfProperty(index), index);
	//	}
	//	else if (type_id & pt_vector)
	//	{
	//		if (type_id & pt_pointer_flag)
	//			setPropValueOfType<Vector2*>(value, GetAddressOfProperty(index), index);
	//		else
	//			setPropValueOfType<Vector2>(value, GetAddressOfProperty(index), index);
	//	}
	//}

	//EntityPtr Entity::GetPropertyEntity(unsigned int index, unsigned int array_index) const
	//{
	//	Entity **entity = static_cast<Entity**>( GetAddressOfProperty(index, array_index) );
	//	return EntityPtr(*entity);
	//}

	//void Entity::SetPropertyEntity(unsigned int index, unsigned int array_index, const EntityPtr &entity)
	//{
	//	Entity **value = static_cast<Entity**>( GetAddressOfProperty(index, array_index) );
	//	*value = entity.get();
	//}

	bool Entity::IsSpawned() const
	{
		return m_Spawned;
	}

	void Entity::SetSpawned(bool spawned)
	{
		m_Spawned = spawned;
	}

	void Entity::Spawn()
	{
		m_Spawned = true;

		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			(*it)->OnSpawn();
		}
	}

	void Entity::StreamIn()
	{
		SetStreamedIn(true);

		{
		tbb::spin_rw_mutex::scoped_lock lock(m_OutRefsMutex, false);
		for (auto it = m_ReferencedEntities.begin(), end = m_ReferencedEntities.end(); it != end; ++it)
		{
			(*it).first->AddReference(this->shared_from_this());
		}
		}

		std::for_each(m_StreamedResources.begin(), m_StreamedResources.end(), [](StreamedResourceUser *user) { user->StreamIn(); });

		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			(*it)->OnStreamIn();
		}
	}

	void Entity::StreamOut()
	{
		SetStreamedIn(false);
		{
		tbb::spin_rw_mutex::scoped_lock lock(m_OutRefsMutex, false);
		for (auto it = m_ReferencedEntities.begin(), end = m_ReferencedEntities.end(); it != end; ++it)
		{
			(*it).first->RemoveReference(this->shared_from_this());
		}
		}

		std::for_each(m_StreamedResources.begin(), m_StreamedResources.end(), [](StreamedResourceUser *user) { user->StreamOut(); });

		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			(*it)->OnStreamOut();
		}
	}

	void Entity::_setPlayerInput(const PlayerInputPtr &player_input)
	{
		m_PlayerInput = player_input;
	}

	bool Entity::InputIsActive(const std::string &input)
	{
		if (m_PlayerInput)
			return m_PlayerInput->IsActive(input);
		else
			return false;
	}

	float Entity::GetInputPosition(const std::string &input)
	{
		if (m_PlayerInput)
			return m_PlayerInput->GetPosition(input);
		else
			return false;
	}

}
