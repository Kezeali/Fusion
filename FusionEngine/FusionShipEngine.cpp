
#include "FusionShipEngine.h"

#include "FusionClientEnvironment.h"
#include "FusionNode.h"

using namespace FusionEngine;

void FusionShipEngine::SetResource(const std::string &resid)
{
	m_ResourceID = resid;
}

void FusionShipEngine::Draw()
{
    CL_Vector2 pos = m_ParentNode->GetPosition();

	m_Env->GetShipResourceByID(m_ResourceID)->Images.Engine->draw(pos.x, pos.y);
}
