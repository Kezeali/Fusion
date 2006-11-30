
#include "FusionEngineCommon.h"

/// Class
#include "FusionShipDrawable.h"

//#include "FusionEnvironmentClient.h"
#include "FusionNode.h"

using namespace FusionEngine;

FusionShipDrawable::FusionShipDrawable()
{
}

FusionShipDrawable::~FusionShipDrawable()
{
}

//void FusionShipDrawable::SetResource(const std::string &resid)
//{
//	m_ResourceID = resid;
//}

void FusionShipDrawable::Draw()
{
	float rot = m_ParentNode->GetGlobalFacing();
	m_Image->set_angle(rot);
	CL_Vector2 pos = m_ParentNode->GetGlobalPosition();
	// Maybe CL_Surface_Drawparams should be used here
	m_Image->draw(pos.x, pos.y);

	// This is really messy - really, the env should draw this ships, because
	//  it has direct access to the resources, and this should just queue up
	//  draw events (or something.)

	//m_Env->GetShipResourceByID(m_ResourceID)->Images.Body->draw(pos.x, pos.y);
}
