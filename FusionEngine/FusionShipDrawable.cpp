
#include "FusionEngineCommon.h"

/// STL

/// Class
#include "FusionShipDrawable.h"

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
	CL_Vector pos = m_ParentNode->GetGlobalPosition();
	float rot = m_ParentNode->GetGlobalFacing();

	m_Env->GetShipResourceByID(m_ResourceID);
}