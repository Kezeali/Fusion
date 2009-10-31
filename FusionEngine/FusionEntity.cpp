
#include "FusionCommon.h"

// Class
#include "FusionEntity.h"

// Fusion
#include "FusionResourceManager.h"


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

	Renderable::Renderable()
		: m_Angle(0.f),
		m_DerivedAngle(0.f),
		m_Colour(255, 255, 255, 255),
		m_Alpha(1.f),
		m_Depth(0),
		m_PreviousWidth(0),
		m_PreviousHeight(0),
		m_PositionChanged(false)
	{
	}

	Renderable::Renderable(ResourceManager *res_man, const std::wstring &sprite_path, int priority)
		: StreamedResourceUser(res_man, "SPRITE", sprite_path, priority),
		m_Angle(0.f),
		m_DerivedAngle(0.f),
		m_Colour(255, 255, 255, 255),
		m_Alpha(1.f),
		m_Depth(0),
		m_PreviousWidth(0),
		m_PreviousHeight(0),
		m_PositionChanged(false)
	{
	}

	Renderable::~Renderable()
	{
	}

	//void Renderable::_notifyAttached(const EntityPtr &entity)
	//{
	//	m_Entity = entity;
	//}

	//EntityPtr Renderable::GetEntity() const
	//{
	//	return m_Entity;
	//}

	void Renderable::SetAlpha(float _alpha)
	{
		if (m_Sprite.IsLoaded())
			m_Sprite->set_alpha(_alpha);
		m_Alpha = _alpha;
	}

	float Renderable::GetAlpha() const
	{
		return m_Alpha;
	}

	void Renderable::SetColour(unsigned int r, unsigned int g, unsigned int b)
	{
		m_Colour.set_color(r, g, b);

		if (m_Sprite.IsLoaded())
			m_Sprite->set_color(m_Colour);
	}

	const CL_Color &Renderable::GetColour() const
	{
		return m_Colour;
	}

	void Renderable::SetPosition(float x, float y)
	{
		m_Position.x = x;
		m_Position.y = y;

		m_PositionChanged = true;
	}

	void Renderable::SetPosition(const Vector2 &position)
	{
		m_Position = position;

		m_PositionChanged = true;
	}

	void Renderable::SetPosition(const CL_Vec2f &_position)
	{
		m_Position.x = _position.x;
		m_Position.y = _position.y;

		m_PositionChanged = true;
	}

	const Vector2 &Renderable::GetPosition() const
	{
		return m_Position;
	}

	void Renderable::SetAngle(float angle)
	{
		if (m_Sprite.IsLoaded())
			m_Sprite->set_angle(CL_Angle(m_Angle, cl_radians));

		if (!fe_fequal(angle, m_Angle))
		{
			CL_Origin origin;
			int x, y;
			m_Sprite->get_alignment(origin, x, y);

			m_AABB = m_AABB.get_rot_bounds(origin, (float)x, (float)y, CL_Angle(m_Angle-angle, cl_radians));
		}

		m_Angle = angle;
	}

	float Renderable::GetAngle() const
	{
		return m_Angle;
	}

	void Renderable::SetDepth(int depth)
	{
		m_Depth = depth;
	}

	int Renderable::GetDepth() const
	{
		return m_Depth;
	}

	void Renderable::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
	}

	bool Renderable::IsEnabled() const
	{
		return m_Enabled;
	}

	const CL_Rectf &Renderable::GetAABB() const
	{
		return m_AABB;
	}

	void Renderable::Update(float split/*, const Vector2 &entity_position, float entity_angle*/)
	{
		if (m_Sprite.IsLoaded())
		{
			bool bbChanged = false;

			//// Set the derived position from the entity position
			//CL_Vec2f position = CL_Vec2f::rotate(CL_Vec2f(entity_position.x, entity_position.y), 
			//m_DerivedPosition = entity_position + m_Position;
			//// Set the derived angle from the entity base-angle
			//if (!fe_fequal(entity_angle + m_Angle, m_DerivedAngle)) // Check that the entity angle has changed
			//{
			//	bbChanged = true;
			//	m_Sprite->set_angle(CL_Angle(entity_angle + m_Angle, cl_radians));
			//}
			//m_DerivedAngle = entity_angle + m_Angle;

			// Check whether AABB needs to be upadated (frame width / height has changed)
			if (m_Sprite->get_height() != m_PreviousHeight)
			{
				bbChanged = true;
				m_PreviousHeight = m_Sprite->get_height();
			}
			if (m_Sprite->get_width() != m_PreviousWidth)
			{
				bbChanged = true;
				m_PreviousWidth = m_Sprite->get_width();
			}

			if (bbChanged || m_PositionChanged)
			{
				CL_Rectf bb;
				bb.left = m_Position.x;
				bb.top = m_Position.y;
				bb.right = m_Position.x + m_Sprite->get_width();
				bb.bottom = m_Position.y + m_Sprite->get_height();

				CL_Origin origin;
				int x, y;
				m_Sprite->get_alignment(origin, x, y);

				bb.translate(-CL_Vec2f::calc_origin(origin, bb.get_size()));

				m_AABB = bb.get_rot_bounds(origin, (float)x, (float)y, m_Sprite->get_angle());

				m_PositionChanged = false;
			}
		}
	}

	//void Renderable::SetSpriteResource(ResourceManager *res_man, const std::string &path)
	//{
	//	SetResource(res_man, path);
	//}

	ResourcePointer<CL_Sprite> &Renderable::GetSpriteResource()
	{
		return m_Sprite;
	}

	void Renderable::OnResourceLoad(ResourceDataPtr resource)
	{
		m_Sprite.SetTarget(resource);
		m_Sprite->set_angle(CL_Angle(m_Angle/*m_DerivedAngle*/, cl_radians));
		m_Sprite->set_alpha(m_Alpha);
		m_Sprite->set_color(m_Colour);
	}

	//void Renderable::OnStreamIn()
	//{
	//	m_ResourceManager->GetResource("SPRITE", m_SpritePath, std::tr1::bind(&Renderable::OnSpriteLoad, this, _1), 1);
	//}

	void Renderable::OnStreamOut()
	{
		m_Sprite.Release();
	}

	void Renderable::Draw(CL_GraphicContext &gc)
	{
		if (m_Enabled && m_Sprite.IsLoaded())
		{
			m_Sprite->draw(gc, m_DerivedPosition.x, m_DerivedPosition.y);
		}
	}

	void Renderable::Draw(CL_GraphicContext &gc, const Vector2 &origin)
	{
		if (m_Enabled && m_Sprite.IsLoaded())
		{
			m_Sprite->draw(gc, m_Position.x + origin.x, m_Position.y + origin.y);
		}
	}

	Entity::Entity()
		: m_Name("default"),
		m_Id(0),
		m_OwnerID(0),
		m_Authority(0),
		m_Flags(0),
		m_Domain(0),
		m_Layer(0),
		m_MarkedToRemove(false),
		m_StreamedOut(true),
		m_Paused(false),
		m_Hidden(false),
		m_Depth(0),
		m_WaitStepsRemaining(0)
	{
	}

	Entity::Entity(const std::string &name)
		: m_Name(name),
		m_Id(0),
		m_OwnerID(0),
		m_Authority(0),
		m_Flags(0),
		m_Domain(0),
		m_Layer(0),
		m_MarkedToRemove(false),
		m_StreamedOut(true),
		m_Paused(false),
		m_Hidden(false),
		m_Depth(0),
		m_WaitStepsRemaining(0)
	{
	}


	Entity::~Entity()
	{
		// Nothing to do here
	}

	void Entity::_setName(const std::string &name)
	{
		m_Name = name;
	}

	const std::string &Entity::GetName() const
	{
		return m_Name;
	}

	void Entity::SetID(ObjectID id)
	{
		m_Id = id;
	}

	ObjectID Entity::GetID() const
	{
		return m_Id;
	}

	void Entity::SetOwnerID(ObjectID owner)
	{
		m_OwnerID = owner;
	}

	ObjectID Entity::GetOwnerID() const
	{
		return m_OwnerID;
	}

	void Entity::SetAuthority(ObjectID authority)
	{
		m_Authority = authority;
	}

	ObjectID Entity::GetAuthority() const
	{
		return m_Authority;
	}

	//void Entity::SetPseudoEntity(bool pseudo_entity)
	//{
	//	if (pseudo_entity)
	//		m_Id = 0;
	//	m_PseudoEntity = pseudo_entity;
	//}

	bool Entity::IsPseudoEntity() const
	{
		return m_Id == 0;
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
			FSN_EXCEPT(ExCode::InvalidArgument, "Entity::SetDomain", "Valid domain values are 0-7");
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

	void Entity::SetStreamedIn(bool is_streamed_in)
	{
		m_StreamedOut = !is_streamed_in;
	}

	bool Entity::IsStreamedOut() const
	{
		return m_StreamedOut;
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

	RenderableArray &Entity::GetRenderables()
	{
		return m_Renderables;
	}

	void Entity::AddRenderable(RenderablePtr renderable)
	{
		m_Renderables.push_back(renderable);
		//renderable->_notifyAttached(this);
	}

	void Entity::RemoveRenderable(RenderablePtr renderable)
	{
		for (RenderableArray::iterator it = m_Renderables.begin(), end = m_Renderables.end(); it != end; ++it)
		{
			if (*it == renderable)
			{
				m_Renderables.erase(it);
				//renderable->_notifyAttached(EntityPtr());
				break;
			}
		}
	}

	void Entity::SetStreamedResources(const StreamedResourceArray &resources)
	{
		m_StreamedResources = resources;
	}

	void Entity::AddStreamedResource(const StreamedResourceUserPtr &resource)
	{
		m_StreamedResources.push_back(resource);
	}

	const Entity::StreamedResourceArray &Entity::GetStreamedResources() const
	{
		return m_StreamedResources;
	}

	void Entity::StreamIn()
	{
		SetStreamedIn(true);

		for (StreamedResourceArray::iterator it = m_StreamedResources.begin(), end = m_StreamedResources.end(); it != end; ++it)
		{
			StreamedResourceUserPtr &user = *it;
			user->StreamIn();
		}
	}

	void Entity::StreamOut()
	{
		SetStreamedIn(false);

		for (StreamedResourceArray::iterator it = m_StreamedResources.begin(), end = m_StreamedResources.end(); it != end; ++it)
		{
			StreamedResourceUserPtr &user = *it;
			user->StreamOut();
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
		//RefCounted::RegisterType<EntityInputs>(engine, "EntityInputs");
		//r = engine->RegisterObjectMethod("EntityInputs"
		//	"bool isActive(const string &in) const",
		//	asMETHOD(EntityInputs, IsActive), asCALL_THISCALL);

		//RefCounted::RegisterType<Entity>(engine, "Entity");
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

		r = engine->RegisterObjectMethod("Entity",
			"void defineInstanceToPrepare(const string &in, uint, bool)",
			asMETHOD(Entity, DefineInstanceToPrepare), asCALL_THISCALL);

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
		r = engine->RegisterInterfaceMethod("IEntity", "void Update()"); FSN_ASSERT(r >= 0);
		r = engine->RegisterInterfaceMethod("IEntity", "void Draw()"); FSN_ASSERT(r >= 0);
	}

}
