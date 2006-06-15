
#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"

/// Class
#include "FusionShip.h"

using namespace FusionEngine;

FusionShip::FusionShip()
{
}

FusionShip::~FusionShip()
{
}

void FusionShip::UpdateNode()
{
	GetParentSceneNode()->SetPosition(CurrentState.position);
	GetParentSceneNode()->SetFacing(CurrentState.facing);
}

void FusionShip::Draw()
{
	//GetParentSceneNode()->GetGlobalPosition()
}