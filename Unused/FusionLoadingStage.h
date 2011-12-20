/*
  Copyright (c) 2006-2007 Fusion Project Team

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
		
		
	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_LoadingStage
#define Header_FusionEngine_LoadingStage

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"


namespace FusionEngine
{

	//! A specific stage within the loading process
	class LoadingStage
	{
	public:
		//! Constructor
		LoadingStage();

		//! Destructor
		~LoadingStage();

	public:
		//! Initialise
		virtual bool Initialise()=0;
		//! Update
		/*!
		 * \returns
		 * Returns the current progress.
		 */
		virtual float Update(unsigned int split)=0;
		//! CleanUp
		virtual void CleanUp()=0;

		const std::string& GetName() const
		{
			return m_Name;
		}

		const std::string& GetDescription() const
		{
			return m_Description;
		}

		//! Returns the current progress
		float GetProgress() const
		{
			return m_Progress;
		}

		//! Returns true if this stage is finished
		bool IsDone() const
		{
			return m_Done;
		}

		bool Failed() const
		{
			return m_Failed;
		}

	protected:
		//! Current loading progress (percent)
		float m_Progress;

		//! Set to true if this stage is finsihed
		bool m_Done;
		//! Set to true if this stage failed
		bool m_Failed;

		std::string m_Name;
		std::string m_Description;

	};

}

#endif