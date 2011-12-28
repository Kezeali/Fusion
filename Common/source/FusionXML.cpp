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

#include "PrecompiledHeaders.h"

#include "FusionXML.h"

#include "FusionExceptionFactory.h"
#include "FusionPhysFS.h"
#include "FusionVirtualFileSource_PhysFS.h"

#include <tinyxml/tinyxml.h>

namespace FusionEngine
{

	ClanLibTiXmlFile::ClanLibTiXmlFile(CL_IODevice file)
		: m_File(file),
		m_WriteFailed(false)
	{
	}

	void ClanLibTiXmlFile::Write(const char *data, size_t len)
	{
		m_WriteFailed =
			m_File.write(data, len) != len;
	}

	void ClanLibTiXmlFile::Print(const char *str)
	{
		size_t expected_length = strlen(str);
		m_WriteFailed =
			m_File.write(str, expected_length) != expected_length;
	}

	void ClanLibTiXmlFile::PutC(int c)
	{
		m_WriteFailed =
			m_File.write(&c, 1) != 1;
	}

	bool ClanLibTiXmlFile::Ok() const
	{
		return !m_WriteFailed && !m_File.is_null();
	}


	PhysFSTiXmlFile::PhysFSTiXmlFile(PHYSFS_File *file)
		: m_File(file),
		m_WriteFailed(false)
	{
	}

	void PhysFSTiXmlFile::Write(const char *data, size_t len)
	{
		m_WriteFailed =
			PHYSFS_write(m_File, data, 1, len) != len;
	}

	void PhysFSTiXmlFile::Print(const char *str)
	{
		size_t expected_length = strlen(str);
		m_WriteFailed =
			PHYSFS_write(m_File, str, 1, expected_length) != expected_length;
	}

	void PhysFSTiXmlFile::PutC(int c)
	{
		m_WriteFailed =
			PHYSFS_write(m_File, &c, 1, 1) != 1;
	}

	bool PhysFSTiXmlFile::Ok() const
	{
		return !m_WriteFailed && m_File != nullptr;
	}


	TiXmlDocument* OpenXml(const std::string &filename, CL_VirtualDirectory vdir)
	{
		TiXmlDocument* doc = nullptr;new TiXmlDocument();
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
			FSN_EXCEPT(ExCode::IO, "'" + filename + "' could not be opened");
		}
		catch (std::bad_alloc&)
		{
			delete doc;
			FSN_EXCEPT(ExCode::IO, "'" + filename + "' could not be opened");
		}

		return doc;
	}

	std::string &OpenString(std::string &content, const std::string &filename, CL_VirtualDirectory vdir)
	{
		try
		{
			CL_IODevice in = vdir.open_file(filename, CL_File::open_existing, CL_File::access_read);

			int len = in.get_size();
			content.resize(len);
			in.read(&content[0], len);
		}
		catch (CL_Exception&)
		{
			FSN_EXCEPT(ExCode::IO, "'" + filename + "' could not be opened");
		}

		return content;
	}

	std::string OpenString(const std::string &filename, CL_VirtualDirectory vdir)
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
			FSN_EXCEPT(ExCode::IO, "'" + filename + "' could not be opened");
		}

		return content;
	}

	void SaveXml(TiXmlDocument* doc, const std::string &filename, CL_VirtualDirectory vdir)
	{
		try
		{
			CL_IODevice out = vdir.open_file(filename, CL_File::create_always, CL_File::access_write);

			ClanLibTiXmlFile fileIntf(out);
			doc->SaveFile(&fileIntf);
		}
		catch (CL_Exception&)
		{
			FSN_EXCEPT(ExCode::IO, "'" + filename + "' could not be saved");
		}
	}

	void SaveString(const std::string &content, const std::string &filename, CL_VirtualDirectory vdir)
	{
		try
		{
			CL_IODevice out = vdir.open_file(filename, CL_File::create_always, CL_File::access_write);

			out.write(content.c_str(), content.length());
		}
		catch (CL_Exception&)
		{
			FSN_EXCEPT(ExCode::IO, "'" + filename + "' could not be saved");
		}
	}

	TiXmlDocument* OpenXml_PhysFS(const std::string &filename)
	{
		// Make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		return OpenXml(filename, vdir);
	}

	std::string OpenString_PhysFS(const std::string &filename)
	{
		// Make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		return OpenString(filename, vdir);
	}

	std::string &OpenString_PhysFS(std::string& content, const std::string &filename)
	{
		// Make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		return OpenString(content, filename, vdir);
	}

	void SaveXml_PhysFS(TiXmlDocument* doc, const std::string &filename)
	{
		// make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		SaveXml(doc, filename, vdir);
	}

	void SaveString_PhysFS(const std::string &content, const std::string &filename)
	{
		// make a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		SaveString(content, filename, vdir);
	}

}
