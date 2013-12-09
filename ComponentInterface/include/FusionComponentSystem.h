/*
*  Copyright (c) 2011-2013 Fusion Project Team
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

#ifndef H_FusionComponentSystem
#define H_FusionComponentSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSystemType.h"
#include "FusionResourceLoader.h"

#include <angelscript.h>
#include <memory>
#include <string>

namespace clan
{
	class DisplayWindow;
	class Canvas;
}

namespace FusionEngine
{

	class ClientOptions;
	class ArchetypeFactory;
	class RegionCellArchivist;
	class SaveDataArchive;
	class WorldSaver;

}

namespace FusionEngine { namespace System
{

	class TaskBase;
	class WorldBase;

	typedef std::shared_ptr<WorldBase> SystemWorldPtr;

	//! Component System
	class ISystem
	{
	public:
		virtual ~ISystem() {}

		virtual SystemType GetType() const = 0;

		virtual std::string GetName() const = 0;

		virtual void RegisterScriptInterface(asIScriptEngine* engine) = 0;

		virtual std::vector<ResourceLoader> GetResourceLoaders() = 0;

		virtual std::shared_ptr<WorldBase> CreateWorld() = 0;

		virtual void SetOptions(const ClientOptions& options) = 0;
		virtual void SetDisplay(const clan::DisplayWindow& display) {}
		virtual void SetCanvas(const clan::Canvas& canvas) {}
		virtual void SetComponentFactory(const std::shared_ptr<ComponentFactory>& factory) {}
		virtual void SetEntityInstantiator(const std::shared_ptr<EntityInstantiator>& instantiator) {}
		virtual void SetEntityManager(const std::shared_ptr<EntityManager>& manager) {}
		virtual void SetArchetypeFactory(const std::shared_ptr<ArchetypeFactory>& factory) {}
		virtual void SetMapLoader(const std::shared_ptr<RegionCellArchivist>& map_loader) {}
		virtual void SetStreamingManager(const std::shared_ptr<StreamingManager>& manager) {}
		virtual void SetDataArchiver(const std::shared_ptr<SaveDataArchive>& archiver) {}
		virtual void SetWorldSaver(WorldSaver* saver) {}
	};

} }

#endif