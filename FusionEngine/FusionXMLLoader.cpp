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

#include "FusionXMLLoader.h"

#include "FusionXML.h"

namespace FusionEngine
{

	void LoadXml(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		if (resource->IsValid())
		{
			delete resource->GetDataPtr();
		}

		TiXmlDocument* doc = OpenXml(resource->GetPath(), vdir);

		resource->SetDataPtr(doc);
		resource->_setValid(true);
	}

	void UnloadXml(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		if (resource->IsValid())
			delete resource->GetDataPtr();
		resource->SetDataPtr(NULL);

		resource->_setValid(false);
	}

};
