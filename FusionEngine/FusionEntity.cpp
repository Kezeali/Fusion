
#include "FusionStableHeaders.h"

#include "FusionEntity.h"

#include "FusionExceptionFactory.h"
#include "FusionResourceManager.h"

#include <boost/lexical_cast.hpp>

namespace FusionEngine
{

	bool TagFlagDictionary::AddTag(const std::string &tag, EntityPtr entity, bool add_flag)
	{
		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where == m_Entries.end())
		{
			_where->second.Tag = tag;

			if (add_flag)
			{
				_where->second.Flag = m_MinFreeFlag; // If there are no flags left this will be zero, so Flag will be set correctly even in that case

				if (m_MinFreeFlag != 0)
					takeMinFreeFlag(); // Update m_MinFreeFlag and the FreeFlags mask
			}
		}

		_where->second.References.insert(entity->GetName());
		// If there is a flag for the given tag add it to the given entity
		if (add_flag && _where->second.Flag != 0)
			entity->AddTagFlag(_where->second.Flag);

		return add_flag && m_MinFreeFlag != 0; // Report whether a flag was given
	}

	void TagFlagDictionary::RemoveTag(const std::string &tag, EntityPtr entity)
	{
		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where != m_Entries.end())
		{
			Entry &entry = _where->second;
			entry.References.erase(entity->GetName());
			entity->RemoveTagFlag(_where->second.Flag);

			// If there are no more entities using this flag
			if (entry.References.empty())
			{
				flagFreed(entry.Flag);
				m_Entries.erase(_where);
			}
		}
	}

	bool TagFlagDictionary::RequestFlagFor(const std::string &tag)
	{
		if (m_MinFreeFlag == 0)
			return false;

		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where != m_Entries.end())
		{
			_where->second.Flag = m_MinFreeFlag;
		}

		return true;
	}

	void TagFlagDictionary::ForceFlagFor(const std::string &tag)
	{
		FSN_EXCEPT(ExCode::NotImplemented, "TagFlagDictionary::ForceFlagFor", "ForceFlagFor() is not implemented.");
	}

	unsigned int TagFlagDictionary::GetFlagFor(const std::string &tag)
	{
		TagFlagMap::iterator _where = m_Entries.find(tag);
		if (_where != m_Entries.end())
			return _where->second.Flag;
		else
			return 0; // No such tag
	}

	void TagFlagDictionary::takeMinFreeFlag()
	{
		m_FreeFlags &= ~m_MinFreeFlag; // remove the chosen flag from the FreeFlags mask

		// Set m_MinFreeFlag to the next free flag
		unsigned int checkFlag = m_MinFreeFlag;
		while (checkFlag != 0)
		{
			checkFlag = checkFlag << 1;
			if (m_FreeFlags & checkFlag)
			{
				m_MinFreeFlag = checkFlag;
				return;
			}
		}

		// No more free flags
		m_MinFreeFlag = 0;
	}

	void TagFlagDictionary::flagFreed(unsigned int flag)
	{
		if (flag < m_MinFreeFlag) m_MinFreeFlag = flag;
		m_FreeFlags |= flag;
	}

	Entity::Entity()
		: m_Name("default"),
		m_HasDefaultName(true),
		m_Id(0),
		m_OwnerID(0),
		m_Authority(0),
		m_Flags(0),
		m_Domain(0),
		m_Layer(0),
		m_MarkedToRemove(false),
		m_MarkedToDeactivate(false),
		m_StreamedIn(true),
		m_CellIndex(0xFFFFFFFF),
		m_Paused(false),
		m_Hidden(false),
		m_Depth(0),
		m_WaitStepsRemaining(0)
	{
	}

	Entity::Entity(const std::string &name)
		: m_Name(name),
		m_HasDefaultName(false),
		m_Id(0),
		m_OwnerID(0),
		m_Authority(0),
		m_Flags(0),
		m_Domain(0),
		m_Layer(0),
		m_MarkedToRemove(false),
		m_MarkedToDeactivate(false),
		m_StreamedIn(true),
		m_CellIndex(0xFFFFFFFF),
		m_Paused(false),
		m_Hidden(false),
		m_Depth(0),
		m_WaitStepsRemaining(0)
	{
	}


	Entity::~Entity()
	{
		std::for_each(m_StreamedResources.begin(), m_StreamedResources.end(),
			[](StreamedResourceUser *user){ user->DestructionNotification = StreamedResourceUser::DestructionNotificationFn(); });
	}

	void Entity::_setName(const std::string &name)
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
		_setName(name);
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

	void Entity::_setDomain(EntityDomain domain_index)
	{
		if (domain_index < s_EntityDomainCount)
			m_Domain = domain_index;
		else
		{
#ifdef _DEBUG
			FSN_EXCEPT(ExCode::InvalidArgument, "Entity::SetDomain", "Valid domain values are 0-" + boost::lexical_cast<std::string>(domain_index));
#endif
			//Logger::getSingleton().Add("Tried to set entity to invalid domain, valid domains are 0-7.");
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

	void Entity::SetStreamingCellIndex(unsigned int index)
	{
		m_CellIndex = index;
	}

	unsigned int Entity::GetStreamingCellIndex() const
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

	RenderableArray &Entity::GetRenderables()
	{
		return m_Renderables;
	}

	void Entity::AddRenderable(RenderablePtr renderable)
	{
		m_Renderables.push_back(renderable);
	}

	void Entity::RemoveRenderable(RenderablePtr renderable)
	{
		for (RenderableArray::iterator it = m_Renderables.begin(), end = m_Renderables.end(); it != end; ++it)
		{
			if (*it == renderable)
			{
				m_Renderables.erase(it);
				break;
			}
		}
	}

	void Entity::RemoveRenderablesWithTag(const std::string &tag)
	{
		auto newEnd = std::remove_if(m_Renderables.begin(), m_Renderables.end(),
			[&](RenderablePtr &renderable)->bool
		{
			return renderable->HasTag(tag);
		});
		m_Renderables.erase(newEnd, m_Renderables.end());
	}

	CL_Rectf Entity::CalculateOnScreenAABB() const
	{
		CL_Rectf bounding_box;
		if (!m_Renderables.empty())
		{
			auto it = m_Renderables.cbegin(), end = m_Renderables.cend();
			bounding_box = (*it)->GetAABB();
			for (++it; it != end; ++it)
				bounding_box.bounding_rect((*it)->GetAABB());
		}
		return bounding_box;
	}

	void Entity::AddStreamedResource(StreamedResourceUser * const user)
	{
		m_StreamedResources.push_back(user);
		user->DestructionNotification = [this](StreamedResourceUser * const user){ this->RemoveStreamedResource(user); };
	}

	void Entity::RemoveStreamedResource(StreamedResourceUser * const user)
	{
		user->DestructionNotification = StreamedResourceUser::DestructionNotificationFn();
		for (auto it = m_StreamedResources.begin(), end = m_StreamedResources.end(); it != end; ++it)
			if (*it == user)
			{
				m_StreamedResources.erase(it);
				break;
			}
	}

	//void Entity::SetStreamedResources(const StreamedResourceArray &resources)
	//{
	//	m_StreamedResources = resources;
	//}

	//const Entity::StreamedResourceArray &Entity::GetStreamedResources() const
	//{
	//	return m_StreamedResources;
	//}

	template <typename T>
	void getPropValueOfType(boost::any &value, void *prop_addr, asUINT property_index)
	{
		value = *(T*)prop_addr;
	}
	template <typename T>
	void setPropValueOfType(const boost::any &value, void *prop_addr, asUINT property_index)
	{
		*(T*)prop_addr = boost::any_cast<T>(value);
	}

	boost::any Entity::GetPropertyValue(unsigned int index) const
	{
		boost::any value;

		int type_id = GetPropertyType(index);

		if (type_id == pt_bool)
			getPropValueOfType<bool>(value, GetAddressOfProperty(index), index);
		// Integer types
		else if (type_id == pt_int8)
			getPropValueOfType<int8_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_int16)
			getPropValueOfType<int16_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_int32)
			getPropValueOfType<int32_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_int64)
			getPropValueOfType<int64_t>(value, GetAddressOfProperty(index), index);
		// ... unsigned
		else if (type_id == pt_uint8)
			getPropValueOfType<uint8_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_uint16)
			getPropValueOfType<uint16_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_uint32)
			getPropValueOfType<uint32_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_uint64)
			getPropValueOfType<uint64_t>(value, GetAddressOfProperty(index), index);
		// Floating point types
		else if (type_id == pt_float)
			getPropValueOfType<float>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_double)
			getPropValueOfType<double>(value, GetAddressOfProperty(index), index);

		// Class types
		else if (type_id == pt_entity)
			getPropValueOfType<Entity*>(value, GetAddressOfProperty(index), index);
		else if (type_id & pt_string)
		{
			if (type_id & pt_pointer_flag)
				getPropValueOfType<std::string*>(value, GetAddressOfProperty(index), index);
			else
				getPropValueOfType<std::string>(value, GetAddressOfProperty(index), index);
		}
		else if (type_id & pt_vector)
		{
			if (type_id & pt_pointer_flag)
				getPropValueOfType<Vector2*>(value, GetAddressOfProperty(index), index);
			else
				getPropValueOfType<Vector2>(value, GetAddressOfProperty(index), index);
		}

		return value;
	}

	void Entity::SetPropertyValue(unsigned int index, const boost::any &value)
	{
		int type_id = GetPropertyType(index);

		if (type_id == pt_bool)
			setPropValueOfType<bool>(value, GetAddressOfProperty(index), index);
		// Integer types
		else if (type_id == pt_int8)
			setPropValueOfType<int8_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_int16)
			setPropValueOfType<int16_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_int32)
			setPropValueOfType<int32_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_int64)
			setPropValueOfType<int64_t>(value, GetAddressOfProperty(index), index);
		// ... unsigned
		else if (type_id == pt_uint8)
			setPropValueOfType<uint8_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_uint16)
			setPropValueOfType<uint16_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_uint32)
			setPropValueOfType<uint32_t>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_uint64)
			setPropValueOfType<uint64_t>(value, GetAddressOfProperty(index), index);
		// Floating point types
		else if (type_id == pt_float)
			setPropValueOfType<float>(value, GetAddressOfProperty(index), index);
		else if (type_id == pt_double)
			setPropValueOfType<double>(value, GetAddressOfProperty(index), index);

		// Class types
		else if (type_id == pt_entity)
			setPropValueOfType<Entity*>(value, GetAddressOfProperty(index), index);
		else if (type_id & pt_string)
		{
			if (type_id & pt_pointer_flag)
				setPropValueOfType<std::string*>(value, GetAddressOfProperty(index), index);
			else
				setPropValueOfType<std::string>(value, GetAddressOfProperty(index), index);
		}
		else if (type_id & pt_vector)
		{
			if (type_id & pt_pointer_flag)
				setPropValueOfType<Vector2*>(value, GetAddressOfProperty(index), index);
			else
				setPropValueOfType<Vector2>(value, GetAddressOfProperty(index), index);
		}
	}

	EntityPtr Entity::GetPropertyEntity(unsigned int index, unsigned int array_index) const
	{
		Entity **entity = static_cast<Entity**>( GetAddressOfProperty(index, array_index) );
		return EntityPtr(*entity);
	}

	void Entity::SetPropertyEntity(unsigned int index, unsigned int array_index, const EntityPtr &entity)
	{
		Entity **value = static_cast<Entity**>( GetAddressOfProperty(index, array_index) );
		*value = entity.get();
	}

	void Entity::StreamIn()
	{
		SetStreamedIn(true);

		std::for_each(m_StreamedResources.begin(), m_StreamedResources.end(), [](StreamedResourceUser *user) { user->StreamIn(); });
	}

	void Entity::StreamOut()
	{
		SetStreamedIn(false);

		std::for_each(m_StreamedResources.begin(), m_StreamedResources.end(), [](StreamedResourceUser *user) { user->StreamOut(); });
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

	void Entity::DefineInstanceToPrepare(const std::string &type, unsigned int count, bool copy_owner)
	{
		InstancePrepDefinition definition;
		definition.Type = type;
		definition.Count = count;
		definition.CopyOwner = copy_owner;

		m_InstancesToPrepare.push_back(definition);
	}

	const InstancesToPrepareArray &Entity::GetInstancesToPrepare() const
	{
		return m_InstancesToPrepare;
	}

	//virtual void Entity::UpdateRenderables(float split)
	//{
	//	for (RenderableArray::iterator it = m_Renderables.begin(), end = m_Renderables.end(); it != end; ++it)
	//	{
	//		(*it)->Update(split);
	//	}
	//}

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

	void Entity::Register(asIScriptEngine *engine)
	{
		int r;
		Entity::RegisterGCType(engine, "Entity");

		r = engine->RegisterObjectMethod("Entity",
			"const string& getName() const",
			asMETHOD(Entity, GetName), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Entity",
			"uint16 getOwnerID() const",
			asMETHOD(Entity, GetOwnerID), asCALL_THISCALL);

		r = engine->RegisterObjectMethod("Entity",
			"bool inputIsActive(const string &in) const",
			asMETHOD(Entity, InputIsActive), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("Entity",
			"float getInputPosition(const string &in) const",
			asMETHOD(Entity, GetInputPosition), asCALL_THISCALL);

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
			"const Vector& getVelocity()",
			asMETHOD(Entity, GetVelocity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setVelocity(const Vector &in)",
			asMETHOD(Entity, SetVelocity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"float getAngle() const",
			asMETHOD(Entity, GetAngle), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setAngle(float)",
			asMETHOD(Entity, SetAngle), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterObjectMethod("Entity",
			"float getAngularVelocity() const",
			asMETHOD(Entity, GetAngularVelocity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
		r = engine->RegisterObjectMethod("Entity",
			"void setAngularVelocity(float)",
			asMETHOD(Entity, SetAngularVelocity), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

		r = engine->RegisterInterface("IEntity"); FSN_ASSERT(r >= 0);
		r = engine->RegisterInterfaceMethod("IEntity", "void Spawn()"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void Update()"); FSN_ASSERT(r >= 0);
		//r = engine->RegisterInterfaceMethod("IEntity", "void Draw()"); FSN_ASSERT(r >= 0);
	}

}
