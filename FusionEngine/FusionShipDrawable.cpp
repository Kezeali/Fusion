
#include "FusionEngineCommon.h"

/// Class
#include "FusionShipDrawable.h"

#include "FusionClientEnvironment.h"
#include "FusionNode.h"

using namespace FusionEngine;

FusionShipDrawable::FusionShipDrawable()
{
}

FusionShipDrawable::~FusionShipDrawable()
{
}

void FusionShipDrawable::SetResource(const std::string &resid)
{
	m_ResourceID = resid;
}

void FusionShipDrawable::Draw()
{
	CL_Vector2 pos = m_ParentNode->GetGlobalPosition();
	float rot = m_ParentNode->GetGlobalFacing();

	m_Env->GetShipResourceByID(m_ResourceID)->Images.Body->draw(pos.x, pos.y);
}
