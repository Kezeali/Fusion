/*
  Copyright (c) 2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef Header_FusionEngine_Command
#define Header_FusionEngine_Command

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Input thing-a-ma-bob
	 *
	 * Commands are much like quake engine usercmds or 'The Zen of Networked Physics' (by Glenn Fiedler)s 'Move' class
	 */
	class Command
	{
	public:
		Command()
			: m_Thrust(false),
			m_Left(false),
			m_Right(false),
			m_PrimaryFire(false),
			m_SecondaryFire(false),
			m_SpecialFire(false)
		{}

	public:
		bool m_Thrust;
		bool m_Left;
		bool m_Right;
		bool m_PrimaryFire;
		bool m_SecondaryFire;
		bool m_SpecialFire;
		
	};

}

#endif
