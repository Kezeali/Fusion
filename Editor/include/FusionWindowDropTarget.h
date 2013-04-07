/*
*  Copyright (c) 2013 Fusion Project Team
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

#ifndef H_FusionWindowDropTarget
#define H_FusionWindowDropTarget

#include "FusionPrerequisites.h"
#include "FusionRefCounted.h"

#include <boost/signals2.hpp>

namespace FusionEngine
{

	class WindowDropTarget
	{
	public:
		virtual ~WindowDropTarget()
		{
		}

		virtual boost::signals2::signal<bool (const Vector2i& drop_location)>& GetSigDragEnter() const = 0;
		virtual boost::signals2::signal<void (const std::string& filename, const Vector2i& drop_location)>& GetSigDrop() const = 0;

		struct DropEvent
		{
			std::vector<std::string> filesList;
			Vector2i dropPosition;
		};

		virtual bool TryPopDropEvent(DropEvent& out) = 0;
	};

}

#endif
