
#include "FusionCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"
#include "FusionPhysicsBody.h"

/// Class
#include "FusionEntity.h"

namespace FusionEngine
{

	Entity::Entity()
		: m_Name("default"),
		m_Flags(0)
	{
	}

	Entity::Entity(const std::string &name)
		: m_Name(name),
		m_Flags(0)
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

	const std:string &Entity::GetName() const
	{
		return m_Name;
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

	std::string Entity::ToString() const
	{
		return GetType() + " - Name: " + m_Name;
	}

	void Entity::ContactBegin(const Contact &contact)
	{
	}

}
