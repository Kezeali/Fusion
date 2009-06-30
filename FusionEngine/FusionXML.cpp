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

#include "FusionCommon.h"

#include "FusionXML.h"

#include "FusionPhysFS.h"

namespace FusionEngine
{

	TiXmlDocument* OpenXml(const std::wstring &filename, CL_VirtualDirectory vdir)
	{
		TiXmlDocument* doc = new TiXmlDocument();
		try
		{
			CL_IODevice in = vdir.open_file(filename, CL_File::open_existing, CL_File::access_read);

			char *filedata = new char[in.get_size()];
			in.read(filedata, in.get_size());

			doc->Parse((const char*)filedata, 0, TIXML_ENCODING_UTF8);
		}
		catch (CL_Exception&)
		{
			delete doc;
			FSN_WEXCEPT(ExCode::IO, L"OpenXml", L"'" + filename + L"' could not be opened");
		}

		return doc;
	}

	std::string &OpenString(std::string &content, const std::wstring &filename, CL_VirtualDirectory vdir)
	{
		;
		try
		{
			CL_IODevice in = vdir.open_file(filename, CL_File::open_existing, CL_File::access_read);

			int len = in.get_size();
			content.resize(len);
			in.read(&content[0], len);
		}
		catch (CL_Exception&)
		{
			FSN_WEXCEPT(ExCode::IO, L"OpenString", L"'" + filename + L"' could not be opened");
		}

		return content;
	}

	std::string OpenString(const std::wstring &filename, CL_VirtualDirectory vdir)
	{
		std::string content;
		try
		{
			CL_IODevice in = vdir.open_file(filename, CL_File::open_existing, CL_File::access_read);

			int len = in.get_size();
			content.resize(len);
			in.read(&content[0], len);
		}
		catch (CL_Exception&)
		{
			FSN_WEXCEPT(ExCode::IO, L"OpenString", L"'" + filename + L"' could not be opened");
		}

		return content;
	}

	void SaveXml(TiXmlDocument* doc, const std::wstring &filename, CL_VirtualDirectory vdir)
	{
		try
		{
			CL_IODevice out = vdir.open_file(filename, CL_File::create_always, CL_File::access_write);

			out.write(doc->ValueStr().c_str(), doc->ValueStr().length());
		}
		catch (CL_Exception&)
		{
			FSN_WEXCEPT(ExCode::IO, L"SaveXml", L"'" + filename + L"' could not be saved");
		}
	}

	void SaveString(const std::string &content, const std::wstring &filename, CL_VirtualDirectory vdir)
	{
		try
		{
			CL_IODevice out = vdir.open_file(filename, CL_File::create_always, CL_File::access_write);

			out.write(content.c_str(), content.length());
		}
		catch (CL_Exception&)
		{
			FSN_WEXCEPT(ExCode::IO, L"SaveString", L"'" + filename + L"' could not be saved");
		}
	}

	TiXmlDocument* OpenXml_PhysFS(const std::wstring &filename)
	{
		// Make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		return OpenXml(filename, vdir);
	}

	std::string OpenString_PhysFS(const std::wstring &filename)
	{
		// Make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		return OpenString(filename, vdir);
	}

	std::string &OpenString_PhysFS(std::string& content, const std::wstring &filename)
	{
		// Make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		return OpenString(content, filename, vdir);
	}

	void SaveXml_PhysFS(TiXmlDocument* doc, const std::wstring &filename)
	{
		// make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		SaveXml(doc, filename, vdir);
	}

	void SaveString_PhysFS(const std::string &content, const std::wstring &filename)
	{
		// make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		SaveString(content, filename, vdir);
	}

}