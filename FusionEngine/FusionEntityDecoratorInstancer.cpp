#include "FusionStableHeaders.h"

#include "FusionEntityDecoratorInstancer.h"

#include "FusionEntityDecorator.h"
#include "FusionEntityManager.h"
#include "FusionRenderer.h"

namespace FusionEngine
{

	EntityDecoratorInstancer::EntityDecoratorInstancer(EntityManager* source, Renderer* renderer)
		: m_EntityManager(source),
		m_Renderer(renderer)
	{
	}

	Rocket::Core::Decorator* EntityDecoratorInstancer::InstanceDecorator(const EMP::Core::String& name, const Rocket::Core::PropertyDictionary& properties)
	{
		EMP::Core::String entity_name = properties.GetProperty("file")->Get<EMP::Core::String>();
		EntityPtr entity = m_EntityManager->GetEntity(std::string(entity_name.CString()));
		return new EntityDecorator(entity, m_Renderer);
	}

	void EntityDecoratorInstancer::ReleaseDecorator(Rocket::Core::Decorator* decorator)
	{
		delete decorator;
	}

	void EntityDecoratorInstancer::Release()
	{
	}

}
