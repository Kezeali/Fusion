#ifndef Header_FusionEngine_FusionShipEngine
#define Header_FusionEngine_FusionShipEngine

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	/*!
	 * \brief
	 * Represents an engine visually.
	 *
	 */
	class FusionShipEngine : public FusionDrawable
	{
	public:
		virtual void Draw();
	};

}

#endif