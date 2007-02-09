
#include "FusionShipEngine.h"

//#include "FusionClientEnvironment.h"
#include "FusionNode.h"

using namespace FusionEngine;

void FusionShipEngine::SetImage(CL_Surface *image)
{
	m_Image = image;
}

/*
void FusionShipEngine::SetResource(const std::string &resid)
{
	m_ResourceID = resid;
}
*/

void FusionShipEngine::Draw()
{
  Vector2 pos = m_ParentNode->GetPosition();

	m_Image->draw(pos.x, pos.y);
	//m_Env->GetShipResourceByID(m_ResourceID)->Images.LeftEngine->draw(pos.x, pos.y);
}
