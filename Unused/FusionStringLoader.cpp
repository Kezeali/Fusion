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


	File Author(s):

		Elliot Hayward
*/

#include "FusionStringLoader.h"

#include "FusionResource.h"

namespace FusionEngine
{

	const std::string &StringLoader::GetType() const
	{
		static std::string strType("STRING");
		return strType;
	}

	Resource<std::string>* StringLoader::LoadResource(FusionEngine::ResourceTag tag, const std::string &text)
	{
		Resource<std::string> rsc(GetType().c_str(), tag, text, (std::string*)0);
		rsc.SetDataPtr(rsc._getTextPtr());
		return rsc;
	}

	void StringLoader::ReloadResource(Resource<std::string> *resource)
	{
		if (resource->IsValid())
			return;

		resource->_setValid(true);
	}

	void StringLoader::UnloadResource(Resource<std::string> *resource)
	{
		resource->_setValid(false);
	}

}
