/*
  Copyright (c) 2009 Fusion Project Team

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

#ifndef Header_FusionEngine_Serialised
#define Header_FusionEngine_Serialised

#if _MSC_VER > 1000
#pragma once
#endif

#include "Common.h"

namespace FusionEngine
{

	struct SerialisedData
	{
		SerialisedData()
			: mask(0xFFFFFFFF) // Set all bits to 1 - i.e. assume all components are present
		{}

		void Exclude(unsigned int component_index)
		{
			// Remove the component bit from the mask
			mask &= ~(1 << component_index);
		}
		void Include(unsigned int component_index)
		{
			// Add the component bit to the mask
			mask &= (1 << component_index);
		}


		bool IsIncluded(unsigned int component_index) const
		{
			// Check the mask for the component bit
			return (mask & (1 << component_index)) != 0;
		}

		//! Indicates the properties / components that the state contains
		unsigned int mask;
		std::string data;
	};

	//SerialisedData::SerialisedData()
	//	: mask(0xFFFFFFFF) // Set all bits to 1 - i.e. assume all components are present
	//{}

	//void SerialisedData::Excluded(unsigned int component_index)
	//{
	//	// Remove the component bit from the mask
	//	mask &= ~(1 << component_index);
	//}

	//void SerialisedData::Included(unsigned int component_index)
	//{
	//	// Add the component bit to the mask
	//	mask &= (1 << component_index);
	//}

	//bool SerialisedData::IsIncluded(unsigned int component_index) const
	//{
	//	// Check the mask for the component bit
	//	return (mask & (1 << component_index)) != 0;
	//}


}

#endif
