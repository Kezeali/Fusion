/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionComponentInspector
#define H_FusionComponentInspector

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"

#include <boost/intrusive_ptr.hpp>
#include <Gwen/Gwen.h>
#include <Gwen/Align.h>
#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Controls/Properties.h>
#include <Gwen/Controls/PropertyTree.h>

#include "FusionEditorCircleTool.h"
#include "FusionEditorPolygonTool.h"
#include "FusionEditorRectangleTool.h"
#include "FusionResourceEditorFactory.h"

namespace FusionEngine { namespace Inspectors
{

	//! Component inspector interface
	class ComponentInspector : public Gwen::Controls::Base
	{
	public:
		GWEN_CONTROL_INLINE(ComponentInspector, Gwen::Controls::Base)
		{
		}
		virtual ~ComponentInspector() {}

		virtual void SetComponents(const std::vector<ComponentPtr>& components) = 0;
		virtual void ReleaseComponents() = 0;

		virtual void ResetUIValues() = 0;

		virtual void SetCircleToolExecutor(const CircleToolExecutor_t& executor) {}
		virtual void SetRectangleToolExecutor(const RectangleToolExecutor_t& executor) {}
		virtual void SetPolygonToolExecutor(const PolygonToolExecutor_t& executor) {}

		virtual void SetResourceEditorFactory(ResourceEditorFactory* factory) {}
	};

} }

#endif