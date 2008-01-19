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

#ifndef Header_FusionEngine_ResourceManager
#define Header_FusionEngine_ResourceManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionResourcePointer.h"
#include "FusionResourceManager.h"

namespace FusionEngine
{
	//! Returns a CL_Surface resource pointer
	static ResourcePointer<CL_Surface> ResourceManager_GetImage(std::string& path, ResourceManager* lhs)
	{
		return lhs->GetResource<CL_Surface>(path, "IMAGE");
	}

	//! Returns a CL_SoundBuffer resource pointer
	static ResourcePointer<CL_SoundBuffer> ResourceManager_GetSound(std::string& path, ResourceManager* lhs)
	{
		return lhs->GetResource<CL_SoundBuffer>(path, "AUDIO");
	}

	//! Returns a XmlDocument resource pointer
	static ResourcePointer<TiXmlDocument> ResourceManager_GetXml(std::string& path, ResourceManager* lhs)
	{
		return lhs->GetResource<TiXmlDocument>(path, "XML");
	}

	//! Returns a std::string resource pointer
	static std::string ResourceManager_GetText(std::string& path, ResourceManager* lhs)
	{
		ResourcePointer<std::string> resource = ResourceManager::getSingleton().GetResource<std::string>(path, "TEXT");
		if (resource.IsValid())
			return *(resource.GetDataPtr());

		else
			return "";
	}

	//! Draws the image pointed to by the given resource pointer
	static void Image_Draw(ResourcePointer<CL_Surface> *lhs, float x, float y)
	{
		if (!lhs->IsValid())
			return;

		CL_Surface* data = lhs->GetDataPtr();
		if (data != NULL)
			data->draw(x, y);
	}

	//! Draws the image pointed to by the given resource pointer
	static void Image_Draw(ResourcePointer<CL_Surface> *lhs, float x, float y, float angle)
	{
		if (!lhs->IsValid())
			return;

		CL_Surface* data = lhs->GetDataPtr();
		if (data != NULL)
		{
			CL_Surface_DrawParams2 dp;
			dp.rotate_angle = angle;
			dp.destX = x;
			dp.destY = y;
			data->draw(dp);
		}
	}

	//! Returns a CL_SoundBuffer_Session for the given sound
	static CL_SoundBuffer_Session Sound_Prepare(ResourcePointer<CL_SoundBuffer> *lhs, bool looping)
	{
		if (!lhs->IsValid())
			FSN_EXCEPT(ExCode::ResourceNotLoaded, "PrepareSession", "The resource is invalid");

		CL_SoundBuffer* data = lhs->GetDataPtr();
		if (data != NULL)
			return data->prepare(looping);

		else
			FSN_EXCEPT(ExCode::ResourceNotLoaded, "PrepareSession", "The resource is invalid");
	}

	//! Plays the given sound
	static void Sound_Play(ResourcePointer<CL_SoundBuffer> *lhs, bool looping)
	{
		if (!lhs->IsValid())
			return;

		CL_SoundBuffer* data = lhs->GetDataPtr();
		if (data != NULL)
			data->play(looping);
	}

	//! Stops the given sound
	static void Sound_Stop(ResourcePointer<CL_SoundBuffer> *lhs)
	{
		if (!lhs->IsValid())
			return;

		CL_SoundBuffer* data = lhs->GetDataPtr();
		if (data != NULL)
			data->stop();
	}

	//! Returns true if the given sound is playing
	static bool Sound_IsPlaying(ResourcePointer<CL_SoundBuffer> *lhs)
	{
		if (!lhs->IsValid())
			return false;

		CL_SoundBuffer* data = lhs->GetDataPtr();
		if (data != NULL)
			return data->is_playing();

		return false;
	}

	//! Executes the given XPath expression on the given XmlDocument
	static std::string XML_ExecuteXPathExpr_String(ResourcePointer<TiXmlDocument> *lhs, std::string& expr)
	{
		if (!lhs->IsValid())
			return "";

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return TinyXPath::S_xpath_string(data->RootElement(), expr.c_str());

		else
			return "";
	}
	//! Executes the given XPath expression on the given XmlDocument
	static double XML_ExecuteXPathExpr_Double(ResourcePointer<TiXmlDocument> *lhs, std::string& expr)
	{
		if (!lhs->IsValid())
			return 0.0;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return TinyXPath::d_xpath_double(data->RootElement(), expr.c_str());

		else
			return 0.0;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static float XML_ExecuteXPathExpr_Float(ResourcePointer<TiXmlDocument> *lhs, std::string& expr)
	{
		if (!lhs->IsValid())
			return 0.f;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return (float)TinyXPath::d_xpath_double(data->RootElement(), expr.c_str());

		else
			return 0.f;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static int XML_ExecuteXPathExpr_Int(ResourcePointer<TiXmlDocument> *lhs, std::string& expr)
	{
		if (!lhs->IsValid())
			return 0;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return TinyXPath::i_xpath_int(data->RootElement(), expr.c_str());

		else
			return 0;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static unsigned int XML_ExecuteXPathExpr_UInt(ResourcePointer<TiXmlDocument> *lhs, std::string& expr)
	{
		if (!lhs->IsValid())
			return 0;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return (unsigned int)TinyXPath::d_xpath_double(data->RootElement(), expr.c_str());

		else
			return 0;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static bool XML_ExecuteXPathExpr_Bool(ResourcePointer<TiXmlDocument> *lhs, std::string& expr)
	{
		if (!lhs->IsValid())
			return false;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
		{
			//std::string uppercaseVal = fe_newupper( TinyXPath::S_xpath_string(data->RootElement(), expr.c_str()) );
			//return (uppercaseVal == "TRUE" ? true : false);
			return TinyXPath::o_xpath_bool(data->RootElement(), expr.c_str());
		}

		else
			return false;
	}

	//! Executes the given XPath expression on the given XmlDocument
	static bool XML_ExecuteXPathExpr_CheckedString(ResourcePointer<TiXmlDocument> *lhs, std::string& expr, std::string& out)
	{
		if (!lhs->IsValid())
			return false;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return TinyXPath::o_xpath_string(data->RootElement(), expr.c_str(), out);

		else
			return false;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static bool XML_ExecuteXPathExpr_CheckedDouble(ResourcePointer<TiXmlDocument> *lhs, std::string& expr, double& out)
	{
		if (!lhs->IsValid())
			return false;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return TinyXPath::o_xpath_double(data->RootElement(), expr.c_str(), out);

		else
			return false;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static bool XML_ExecuteXPathExpr_CheckedFloat(ResourcePointer<TiXmlDocument> *lhs, std::string& expr, float& out)
	{
		if (!lhs->IsValid())
			return false;

		double dOut;
		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
		{
			bool success = TinyXPath::o_xpath_double(data->RootElement(), expr.c_str(), dOut);
			if (success)
				out = (float)dOut;
			return success;
		}

		else
			return false;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static bool XML_ExecuteXPathExpr_CheckedInt(ResourcePointer<TiXmlDocument> *lhs, std::string& expr, int& out)
	{
		if (!lhs->IsValid())
			return false;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return TinyXPath::o_xpath_int(data->RootElement(), expr.c_str(), out);

		else
			return false;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static bool XML_ExecuteXPathExpr_CheckedUInt(ResourcePointer<TiXmlDocument> *lhs, std::string& expr, unsigned int& out)
	{
		if (!lhs->IsValid())
			return false;

		double dOut;
		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
		{
			bool success = TinyXPath::o_xpath_double(data->RootElement(), expr.c_str(), dOut);
			if (success)
				out = (unsigned int)dOut;
			return success;
		}

		else
			return false;
	}
	//! Executes the given XPath expression on the given XmlDocument
	static bool XML_ExecuteXPathExpr_CheckedBool(ResourcePointer<TiXmlDocument> *lhs, std::string& expr, bool& out)
	{
		if (!lhs->IsValid())
			return false;

		TiXmlDocument* data = lhs->GetDataPtr();
		if (data != NULL)
			return TinyXPath::o_xpath_bool(data->RootElement(), expr.c_str(), out);

		else
			return false;
	}

}