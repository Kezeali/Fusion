#ifndef Header_FusionEngine_FusionShipEngine
#define Header_FusionEngine_FusionShipEngine

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

// inherits
#include "FusionDrawable.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Represents an engine (thruster) visually.
	 */
	class FusionShipEngine : public FusionDrawable
	{
	public:

		void SetResource(const std::string &resid);

		void Draw();

	protected:
		//! Allows the engine drawable to access the relavant ShipResource
		ClientEnvironment *m_Env;

		//! Resource (sent by FusionClientShip)
		std::string m_ResourceID;
	};

}

#endif
