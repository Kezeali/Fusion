#include "FusionStableHeaders.h"

#include "FusionEntityDecoratorInstancer.h"

#include "FusionEntityDecorator.h"
#include "FusionEntityManager.h"
#include "FusionRenderer.h"

namespace FusionEngine
{

	EntityDecoratorInstancer::EntityDecoratorInstancer(EntityManager* source, Renderer* renderer)
		: Rocket::Core::DecoratorInstancer(),
		m_EntityManager(source),
		m_Renderer(renderer)
	{
		RegisterProperty("name", "none").AddParser("string");
	}

	Rocket::Core::Decorator* EntityDecoratorInstancer::InstanceDecorator(const Rocket::Core::String& name, const Rocket::Core::PropertyDictionary& properties)
	{
		Rocket::Core::String entity_name = properties.GetProperty("name")->Get<Rocket::Core::String>();
		EntityPtr entity = m_EntityManager->GetEntity(std::string(entity_name.CString()));
		if (entity)
			return new EntityDecorator(entity, m_Renderer);
		else
			return nullptr;
	}

	void EntityDecoratorInstancer::ReleaseDecorator(Rocket::Core::Decorator* decorator)
	{
		delete decorator;
	}

	void EntityDecoratorInstancer::Release()
	{
	}

}
