
#include "FusionCommon.h"

/// Class
#include "FusionShipDrawable.h"

#include "FusionResourceLoader.h"
#include "FusionShipResource.h"
#include "FusionNode.h"

using namespace FusionEngine;

FusionShipDrawable::FusionShipDrawable(const std::string& resource_id)
{
	m_ResourceID = resource_id;
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
	float rot = m_ParentNode->GetGlobalFacing();
	Vector2 pos = m_ParentNode->GetGlobalPosition();
	// Get resource form the rscmgr singleton
	ShipResource* rsc = ResourceManager::getSingletonPtr()->GetShipResource(m_ResourceID);

	CL_Surface_DrawParams2 dp;
	// Set the params
	dp.rotate_angle = rot;
	dp.destX = pos.x;
	dp.destY = pos.y;

	// We don't really care about these...
	dp.alpha = 255;

	// Finally, draw the image
	rsc->Images.Body->draw(dp);
}
