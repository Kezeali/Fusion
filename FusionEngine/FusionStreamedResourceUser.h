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

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"

#include <boost/signals2/connection.hpp>

#include "FusionResource.h"

namespace FusionEngine
{

	//! Base class that helps automate resource loading when an Entity is streamed in
	class StreamedResourceUser
	{
	public:
		//! CTOR
		StreamedResourceUser();
		//! CTOR, sets the resource to load initially
		StreamedResourceUser(ResourceManager *res_man, const std::string &type, const std::string &path, int base_priority = 0);
		//! DTOR
		virtual ~StreamedResourceUser();

		typedef std::function<void (StreamedResourceUser* const)> DestructionNotificationFn;
		DestructionNotificationFn DestructionNotification;

		//! Returns true if the renderable is bound to an entity
		bool IsBound() const;

		//! Sets the resource to load when streamed in
		void SetResource(ResourceManager *res_man, const std::string &path);

		//! Loads the resource
		void StreamIn(int priority = 0);
		//! Informs the resource user that it has been streamed out
		void StreamOut();

		//! Called when the ResourceManager finishes loading the resource
		virtual void OnResourceLoad(ResourceDataPtr resource) =0;
		//! Called when StreamIn is called
		/*!
		* This is called after the resource has been requested, but most likely
		* before it has finished loading.
		*/
		virtual void OnStreamIn() {};
		//! Called when the SRU is streamed out
		/*!
		* You should release any references to the resource pointer you received from OnResourceLoad
		* in the implementation of this method. Otherwise the resource will never be unloaded
		*/
		virtual void OnStreamOut() {};

	private:
		ResourceManager* m_ResourceManager;
		std::string m_ResourceType;
		std::string m_ResourcePath;
		bsig2::connection m_LoadConnection;

		//ResourceDataPtr m_Resource;

		int m_Priority;
	};

	//class StreamedResource
	//{
	//public:
	//	//! Loads the resource
	//	void StreamIn(int priority = 0);
	//	//! Informs the resource user that it has been streamed out
	//	void StreamOut();

	//private:
	//	StreamedResourceUser *m_User;
	//};

	//typedef std::shared_ptr<StreamedResource> StreamedResourceUserPtr;

}

#endif
