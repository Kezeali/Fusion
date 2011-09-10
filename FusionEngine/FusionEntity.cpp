/*
*  Copyright (c) 2007-2011 Fusion Project Team
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

#include "FusionEntity.h"

#include "FusionExceptionFactory.h"
#include "FusionResourceManager.h"
#include "FusionPhysicalComponent.h"

#include "FusionAngelScriptComponent.h"

#include <boost/lexical_cast.hpp>

namespace FusionEngine
{

	Entity::Entity(EntityManager* manager, PropChangedQueue *q, const ComponentPtr& transform_component)
		: m_Name("default"),
		m_HasDefaultName(true),
		m_Id(0),
		m_OwnerID(0),
		m_Authority(0),
		m_Flags(0),
		m_Domain(GAME_DOMAIN),
		m_Layer(0),
		m_MarkedToRemove(false),
		m_MarkedToDeactivate(false),
		m_StreamedIn(false),
		m_CellIndex(0xFFFFFFFF),
		m_Spawned(false),
		m_Paused(false),
		m_Hidden(false),
		m_Depth(0),
		m_WaitStepsRemaining(0),
		m_Manager(manager)
	{
		FSN_ASSERT(q);
		m_PropChangedQueue = q;

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

	bool Entity::IsSyncedEntity() const
	{
		return m_Id > 0;
	}

	bool Entity::IsPseudoEntity() const
	{
		return !IsSyncedEntity();
	}

	void Entity::SetType(const std::string& type)
	{
		m_Type = type;
	}

	std::string Entity::GetType() const
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
		tbb::spin_rw_mutex::scoped_lock lock(m_OutRefsMutex);
		auto result = m_ReferencedEntities.insert(std::make_pair(entity, 0u));
		++(result.first->second);
		if (result.second && IsActive())
			entity->AddReference(this->shared_from_this());
	}

	void Entity::DropReference(EntityPtr entity)
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

	void Entity::SerialiseReferencedEntitiesList(RakNet::BitStream& stream)
	{
#if 0
		stream.Write((size_t)std::count_if(m_ReferencedEntities.begin(), m_ReferencedEntities.end(), [](const std::pair<EntityPtr, size_t> &ref) { return ref.first->IsSyncedEntity(); }));
		for (auto it = m_ReferencedEntities.begin(), end = m_ReferencedEntities.end(); it != end; ++it)
		{
			auto& entity = it->first;
			auto count = it->second;
			if (entity->IsSyncedEntity())
			{
				stream.Write(entity->GetID());
				stream.Write(count);
			}
		}
#endif
	}

	void Entity::DeserialiseReferencedEntitiesList(RakNet::BitStream& stream, const EntityDeserialiser& directory)
	{
#if 0
		size_t numRefed;
		stream.Read(numRefed);
		for (size_t i = 0; i < numRefed; ++i)
		{
			ObjectID id;
			stream.Read(id);

			unsigned int numTimesReferenced;
			stream.Read(numTimesReferenced);

			auto entity = directory.GetEntity(id);
			if (entity)
				m_ReferencedEntities[entity] = numTimesReferenced;
			else
				m_UnloadedReferencedEntities.push_back(std::make_pair(id, numTimesReferenced));
		}
#endif
	}

	void Entity::AddComponent(const ComponentPtr& component, std::string identifier)
	{
		FSN_ASSERT(component);
		// Don't allow the addition of multiple transform components:
		FSN_ASSERT(dynamic_cast<ITransform*>(component.get()) == nullptr || dynamic_cast<ITransform*>(component.get()) == m_Transform.get());
		FSN_ASSERT(m_PropChangedQueue);
		// TODO:
		//FSN_ASSERT(!DeltaTime::IsExecuting());

		tbb::spin_rw_mutex::scoped_lock lock(m_ComponentsMutex);

		component->SetPropChangedQueue(m_PropChangedQueue);

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
			//component->SetIdentifier(identifier);
			FSN_ASSERT(implementors.find(interfaceIdentifier) == implementors.end()); // no duplicates
			implementors[interfaceIdentifier] = component;
		}
		// Add the new component to the main list
		m_Components.push_back(component);
		component->SetParent(this);
	}

	void Entity::RemoveComponent(const ComponentPtr& component, std::string identifier)
	{
		FSN_ASSERT(std::find(m_Components.begin(), m_Components.end(), component) != m_Components.end());

		tbb::spin_rw_mutex::scoped_lock lock(m_ComponentsMutex);

		// Remove the given component from the main list, and notify all other components
		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; )
		{
			if (*it == component)
			{
				component->SetPropChangedQueue(nullptr);
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
	}

	void Entity::OnComponentActivated(const ComponentPtr& component)
	{
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

	void Entity::SynchroniseParallelEdits()
	{
		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			(*it)->SynchronisePropertiesNow();
		}
	}

	void Entity::SetPropChangedQueue(PropChangedQueue *q)
	{
		FSN_ASSERT_MSG(m_Components.empty(), "Can't change the prop queue after components have been added");
		m_PropChangedQueue = q;
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

	inline void Entity::SetStreamedIn(bool is_streamed_in)
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

	void Entity::SetStreamingCellIndex(size_t index)
	{
		m_CellIndex = index;
	}

	size_t Entity::GetStreamingCellIndex() const
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

	//virtual void Entity::UpdateRenderables(float split)
	//{
	//	for (RenderableArray::iterator it = m_Renderables.begin(), end = m_Renderables.end(); it != end; ++it)
	//	{
	//		(*it)->Update(split);
	//	}
	//}

	void Entity::SerialiseIdentity(RakNet::BitStream& stream)
	{
		stream.Write(m_OwnerID);
		stream.Write(m_Authority);
	}

	std::string Entity::ToString() const
	{
		return GetType() + " - " + m_Name;
	}

	void Entity_GetPosition(Vector2 &out, Entity *entity)
	{
		out = entity->GetPosition();
	}

	void Entity_SetPosition(float x, float y, Entity *entity)
	{
		entity->SetPosition(Vector2(x, y));
	}

	class ASComponentFuture : public RefCounted
	{
	public:
		ASComponentFuture()
			: component(nullptr)
		{}

		ASComponentFuture(IComponent* com)
			: component(com)
		{}

		IComponent* component;

		static void Register(asIScriptEngine* engine);
		
	};

	static IComponent* ASComponentFuture_GetComponent(ASComponentFuture* obj)
	{
		return obj->component;
	}

	void ASComponentFuture::Register(asIScriptEngine* engine)
	{
		ASComponentFuture::RegisterType<ASComponentFuture>(engine, "ComponentFuture");

		int r;
		r = engine->RegisterObjectBehaviour("ComponentFuture", asBEHAVE_IMPLICIT_REF_CAST, "IComponent@ f()", asFUNCTION(ASComponentFuture_GetComponent), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("ComponentFuture", "IComponent@ get()", asFUNCTION(ASComponentFuture_GetComponent), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

	static ASComponentFuture* Entity_GetComponent(EntityPtr* obj, std::string type, std::string ident)
	{
		auto entity = *obj;
		auto com = entity->GetComponent(type, ident);

		auto future = new ASComponentFuture();

		if (com)
		{
			com->addRef();
			future->component = com.get();
		}
		else
		{
			ASScript::GetActiveScript()->YieldUntil([entity, type, ident, future]()->bool
			{
				auto com = entity->GetComponent(type, ident);
				if (com)
				{
					com->addRef();
					future->component = com.get();
					return true;
				}
				else
					return false;
			},
#ifdef PROFILE_BUILD
				20.0f);
#else
				5.f);
#endif
		}
		
		return future;
	}

	static ASComponentFuture* Entity_GetComponentB(EntityPtr* obj, std::string type)
	{
		return Entity_GetComponent(obj, type, std::string());
	}

	static bool Entity_InputIsActive(const std::string& input, EntityPtr* entity)
	{
		return (*entity)->InputIsActive(input);
	}

	static float Entity_InputGetPosition(const std::string& input, EntityPtr* entity)
	{
		return (*entity)->GetInputPosition(input);
	}

	//static PlayerInput* Entity_GetInput(EntityPtr* entity)
	//{
	//	return (*entity)->m_PlayerInput.get();
	//}

	void Entity::Register(asIScriptEngine *engine)
	{
		
		RegisterSharedPtrType<Entity>("Entity", engine);

		{
			typedef Scripting::Registation::ValueTypeHelper<std::weak_ptr<Entity>> helper_type;

			int r;
			r = engine->RegisterObjectType("EntityW", sizeof(std::weak_ptr<Entity>), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour("EntityW", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(helper_type::Construct), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectBehaviour("EntityW", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(helper_type::Destruct), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("EntityW", "EntityW& opAssign(const EntityW &in other)",
				asMETHODPR(std::weak_ptr<Entity>, operator=, (const std::weak_ptr<Entity> &), std::weak_ptr<Entity> &), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("EntityW", "EntityW& opAssign(Entity &in other)",
				asMETHODPR(std::weak_ptr<Entity>, operator=, (EntityPtr &), std::weak_ptr<Entity> &), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		}

		ASComponentFuture::Register(engine);

		int r;
		r = engine->RegisterObjectMethod("Entity",
			"ComponentFuture@ getComponent(string, string) const",
			asFUNCTION(Entity_GetComponent), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"ComponentFuture@ getComponent(string) const",
			asFUNCTION(Entity_GetComponentB), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );

		//r = engine->RegisterObjectMethod("Entity",
		//	"Input@ get_input() const",
		//	asFUNCTION(Entity_GetInput), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"bool inputIsActive(const string &in) const",
			asFUNCTION(Entity_InputIsActive), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"float inputGetPosition(const string &in) const",
			asFUNCTION(Entity_InputGetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"const string& getName() const",
			asMETHOD(Entity, GetName), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"uint16 getID() const",
			asMETHOD(Entity, GetID), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"void setDomain(uint8)",
			asMETHOD(Entity, SetDomain), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"uint8 getDomain() const",
			asMETHOD(Entity, GetDomain), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"void setOwnerID(uint8)",
			asMETHOD(Entity, SetOwnerID), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"uint8 getOwnerID() const",
			asMETHOD(Entity, GetOwnerID), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"bool isSynced() const",
			asMETHOD(Entity, IsSyncedEntity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		//r = engine->RegisterObjectMethod("Entity",
		//	"bool inputIsActive(const string &in) const",
		//	asMETHOD(Entity, InputIsActive), asCALL_THISCALL);
		//r = engine->RegisterObjectMethod("Entity",
		//	"float inputGetPosition(const string &in) const",
		//	asMETHOD(Entity, GetInputPosition), asCALL_THISCALL);

		// Physical state related methods
		r = engine->RegisterObjectMethod("Entity",
			"const Vector& getPosition()",
			asMETHOD(Entity, GetPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void getPosition(Vector &out)",
			asFUNCTION(Entity_GetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setPosition(const Vector &in)",
			asMETHOD(Entity, SetPosition), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setPosition(float, float)",
			asFUNCTION(Entity_SetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"float getAngle() const",
			asMETHOD(Entity, GetAngle), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setAngle(float)",
			asMETHOD(Entity, SetAngle), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		//r = engine->RegisterInterface("IEntity"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void OnSpawn()"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void Update()"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void Draw()"); FSN_ASSERT(r >= 0);
	}

}
