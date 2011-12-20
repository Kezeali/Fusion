#ifndef Header_FusionEngine_ShipHealthDrawable
#define Header_FusionEngine_ShipHealthDrawable

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherits
#include "FusionDrawable.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Draws a ships current health.
	 *
	 * This includes a bar, which slides toward new health values (it doesn't
	 * change instantly, but slowly reduces/increases); particles being shot
	 * out when health is lost; and possibly (in the future) a numerical
	 * / fuzzy health counter.
	 */
	class ShipHealthDrawable : public FusionDrawable
	{
	public:
		//! Constructor.
		ShipHealthDrawable();

	public:
		//! Sets the image to use for particles
		void SetParticleImage(CL_Surface *image);
		//! Sets the width of the health bar
		void SetWidth(int width);
		//! Sets the maximum health
		void SetMax(int max);
		//! Sets the value of health to be drawn
		void SetHealth(int value);

		//! Updates particles and the sliding bar.
		void Update(unsigned int split);

		//! Draws the damn thing
		void Draw();

	protected:
		//! The width of the health bar
		int m_Width;
		//! The maximum health possible
		int m_Max;
		//! The most recently reported health
		int m_Value;
		//! If the health has changed since the last update, this will be the difference
		int m_Delta;

		//! The surface to use for particles
		CL_Surface *m_Image;
		//! Particle type to use in effects
		//L_Particle *m_ParticleType;
		//! Particle effect to use
		//L_ShootingEffect *m_Effect;
	};

}

#endif
