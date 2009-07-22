
#include "FusionCommon.h"

// Fusion

// Class
#include "FusionEntity.h"

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
		: m_Angle(0),
		m_Colour(255, 255, 255, 255),
		m_Alpha(1.f),
		m_Depth(0),
		m_PreviousWidth(0),
		m_PreviousHeight(0)
	{
	}

	Renderable::Renderable(const FusionEngine::ResourcePointer<CL_Sprite> &resource)
		: m_Sprite(resource),
		m_Angle(0),
		m_Colour(255, 255, 255, 255),
		m_Alpha(1.f),
		m_Depth(0),
		m_PreviousWidth(0),
		m_PreviousHeight(0)
	{
		m_LoadConnection = m_Sprite.SigLoad().connect( boost::bind(&Renderable::OnSpriteLoad, this) );
	}

	Renderable::~Renderable()
	{
		m_LoadConnection.disconnect();
	}

	void Renderable::_notifyAttached(const EntityPtr &entity)
	{
		m_Entity = entity;
	}

	EntityPtr Renderable::GetEntity() const
	{
		return m_Entity;
	}

	void Renderable::SetAlpha(float _alpha)
	{
		if (m_Sprite.IsValid())
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

		if (m_Sprite.IsValid())
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
	}

	void Renderable::SetPosition(const Vector2 &position)
	{
		m_Position = position;
	}

	void Renderable::SetPosition(const CL_Vec2f &_position)
	{
		m_Position.x = _position.x;
		m_Position.y = _position.y;
	}

	const Vector2 &Renderable::GetPosition() const
	{
		return m_Position;
	}

	void Renderable::SetAngle(float angle)
	{
		if (m_Sprite.IsValid())
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

	void Renderable::Update(float split)
	{
		if (m_Sprite.Lock())
		{
			m_Sprite->update(split);

			// Check whether AABB needs to be upadated (frame width / height has changed)
			bool bbChanged = false;
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

			if (bbChanged)
			{
				CL_Rectf bb;
				bb.left = m_Position.x;
				bb.top = m_Position.y;
				bb.right = m_Position.x + m_Sprite->get_width();
				bb.bottom = m_Position.y + m_Sprite->get_height();

				CL_Origin origin;
				int x, y;
				m_Sprite->get_alignment(origin, x, y);

				m_AABB = bb.get_rot_bounds(origin, (float)x, (float)y, m_Sprite->get_angle());
			}

			m_Sprite.Unlock();
		}
	}

	void Renderable::SetSpriteResource(const ResourcePointer<CL_Sprite> &resource)
	{
		m_LoadConnection.disconnect();
		m_Sprite = resource;
		m_LoadConnection = m_Sprite.SigLoad().connect( boost::bind(&Renderable::OnSpriteLoad, this) );
	}

	ResourcePointer<CL_Sprite> &Renderable::GetSpriteResource()
	{
		return m_Sprite;
	}

	void Renderable::OnSpriteLoad()
	{
		if (m_Sprite.Lock())
		{
			m_Sprite->set_angle(CL_Angle(m_Angle, cl_radians));
			m_Sprite->set_alpha(m_Alpha);
			m_Sprite->set_color(m_Colour);

			m_Sprite.Unlock();
		}
	}

	void Renderable::Draw(CL_GraphicContext &gc)
	{
		if (m_Enabled && m_Sprite.Lock())
		{
			m_Sprite->draw(gc, m_Position.x, m_Position.y);
			m_Sprite.Unlock();
		}
	}

	Entity::Entity()
		: m_Name("default"),
		m_Id(0),
		m_Flags(0),
		m_MarkedToRemove(false),
		m_Paused(false),
		m_Hidden(false),
		m_Depth(0)
	{
	}

	Entity::Entity(const std::string &name)
		: m_Name(name),
		m_Id(0),
		m_Flags(0),
		m_MarkedToRemove(false),
		m_Paused(false),
		m_Hidden(false),
		m_Depth(0)
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

	void Entity::SetStreamedOut(bool is_streamed_out)
	{
		m_StreamedOut = is_streamed_out;
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
			return true;
		}
		else
			return false;
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
		renderable->_notifyAttached(this);
	}

	void Entity::RemoveRenderable(RenderablePtr renderable)
	{
		for (RenderableArray::iterator it = m_Renderables.begin(), end = m_Renderables.end(); it != end; ++it)
		{
			if (*it == renderable)
			{
				m_Renderables.erase(it);
				renderable->_notifyAttached(EntityPtr());
				break;
			}
		}
	}

	const StringSet &Entity::GetStreamedResources() const
	{
		return m_StreamedResources;
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

}
