/*
  Copyright (c) 2006 Fusion Project Team

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

#include "FusionScript.h"

namespace FusionEngine
{

	const char* Script::GetModule() const
	{
		return m_Module.c_str();
	}

	bool Script::LoadFile(const std::string &filename, 
												CL_InputSourceProvider *provider,
												bool delete_provider)
	{
		CL_InputSource *input;

		// Assume a file provider is none is supplied
		if (provider == 0)
		{
			provider = new CL_InputSourceProvider_File(filename);

			delete_provider = true;
		}

		// Open a file from the source
		input = provider->open_source(filename);


		// Read the file
		int len = input->size();
		m_Script.resize(len);

		input->read(&m_Script[0], len);


		input->close();
		delete input;


		// Delete the provider
		if (delete_provider) delete provider;

		return true;
	}

	void Script::_setModule(const char *module)
	{
		m_Module = *module;
	}

	void Script::_notifyRegistration()
	{
		m_Registered = true;
	}

	const std::string& Script::GetScript() const
	{
		return m_Script;
	}

}
