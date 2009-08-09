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
*/

#ifndef Header_FusionEngine_StreamedResourceUser
#define Header_FusionEngine_StreamedResourceUser

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionRefCounted.h"
#include "FusionBoostSignals2.h"


namespace FusionEngine
{

	class StreamedResourceUser : public RefCounted, no_factory_noncopyable
	{
	public:
		StreamedResourceUser();
		StreamedResourceUser(ResourceManager *res_man, const std::string &type, const std::wstring &path, int base_priority = 0);

		void SetResource(ResourceManager *res_man, const std::wstring &path);
		const ResourceDataPtr &GetResource() const;

		void StreamIn(int priority = 0);
		void StreamOut();

		virtual void OnResourceLoad(ResourceDataPtr resource) =0;
		virtual void OnStreamIn() {};
		virtual void OnStreamOut() {};

	private:
		ResourceManager* m_ResourceManager;
		std::string m_ResourceType;
		std::wstring m_ResourcePath;
		bsig2::connection m_LoadConnection;

		ResourceDataPtr m_Resource;

		int m_Priority;
	};

	typedef boost::intrusive_ptr<StreamedResourceUser> StreamedResourceUserPtr;

}

#endif
