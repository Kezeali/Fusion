/*
*  Copyright (c) 2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionDeltaTime
#define H_FusionDeltaTime

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

namespace FusionEngine
{

	// TODO: put this in FusionTypes or whatever
	typedef uint32_t Tick_t;
	
	//! Global access to pacing data
	class DeltaTime
	{
		friend class TaskScheduler;
	public:
		//! Returns the fixed (simulation) frame-time
		static float GetDeltaTime() { return m_DT; }

		//! Returns the actual amount of time that passed since the last frame
		static float GetActualDeltaTime() { return m_ActualDTMS * 0.001f; }
		static unsigned int GetActualDeltaTimeMS() { return m_ActualDTMS; }

		//! Returns the percentage of a full fixed-time frame that has passed
		static float GetInterpolationAlpha() { return m_Alpha; }

		//! Returns the number of frames simulation frames that have passed since the last rendered frame
		/*!
		* If frame-skip is enabled, some frames may not be rendered if the overall framerate is lower
		* than the fixed deltatime (this may improve performance if the game is render-bound, but
		* this is unlikely for a 2D game so frameskip should generally not be enabled)
		*/
		static unsigned int GetFramesSkipped() { return m_FramesSkipped; }

		static Tick_t GetTick() { return m_Tick; }

	private:
		static float m_DT;
		static unsigned int m_ActualDTMS;
		static float m_Alpha;

		static unsigned int m_FramesSkipped;

		static Tick_t m_Tick;
	};

}

#endif
