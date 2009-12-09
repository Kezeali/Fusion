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

#ifndef Header_FusionEngine_Xml
#define Header_FusionEngine_Xml

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"


namespace FusionEngine
{

	class ClanLibTiXmlFile : public TiXmlFileInterface
	{
	public:
		ClanLibTiXmlFile(CL_IODevice file);

		void Write(const char *data, size_t len);
		void Print(const char *ste);
		void PutC(int c);

		bool Ok() const;

	protected:
		CL_IODevice m_File;
		bool m_WriteFailed;
	};

	/*!
	 * \brief
	 * Opens an xml file using the given VirtualDirectory object
	 */
	TiXmlDocument* OpenXml(const std::wstring &filename, CL_VirtualDirectory vdir);

	std::string &OpenString(std::string& content, const std::wstring &filename, CL_VirtualDirectory vdir);

	std::string OpenString(const std::wstring &filename, CL_VirtualDirectory vdir);
	
	void SaveXml(TiXmlDocument* doc, const std::wstring &filename, CL_VirtualDirectory vdir);

	void SaveString(const std::string &content, const std::wstring &filename, CL_VirtualDirectory vdir);

	/*!
	 * \brief
	 * Helper for opening an xml file using PhysFS (which will be the usual way.)
	 */
	TiXmlDocument* OpenXml_PhysFS(const std::wstring &filename);

	std::string &OpenString_PhysFS(std::string &content, const std::wstring &filename);

	std::string OpenString_PhysFS(const std::wstring &filename);

	void SaveXml_PhysFS(TiXmlDocument* doc, const std::wstring &filename);

	void SaveString_PhysFS(const std::string &content, const std::wstring &filename);

}

#endif