#ifndef Header_FusionEngine_FusionProjectile
#define Header_FusionEngine_FusionProjectile

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Holds all state information for each projectile.
	 *
	 * An instance of this object holds the state data for each projectile in the game.
	 * This object should remain general enough to be used by client and server.
	 *
	 * \remarks
	 * When a weapon fires, and instance of this class is produced.
	 */
	class FusionProjectile
	{
	public:

		//@{
		/*!
		 * I don't think there's much point protecting these, that'll just make
		 * everything more complicated. External classes (such as ServerEnvironment)
		 * can just access these directly. :P If you wan't it to be more ObjectO, (i.e.
		 * uses member funcions to access these) tell me and I _might_ get around to it.
		 */
		ShipState CurrentState;
		ShipState InitialState;
		//@}

		FusionProjectile();
		~FusionProjectile();
	};

}

#endif
