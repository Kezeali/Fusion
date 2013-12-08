/*
  Copyright (c) 2013 Fusion Project Team

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

#ifndef H_Fusion_ClanLibRenderer_DebugDrawProvider
#define H_Fusion_ClanLibRenderer_DebugDrawProvider
#if _MSC_VER > 1000
#pragma once
#endif

#include "Visual/FusionDebugDrawImpl.h"

#include "FusionVectorTypes.h"
#include <ClanLib/Display/2D/canvas.h>

namespace FusionEngine { namespace ClanLibRenderer
{

	class DebugDrawImpl;

	class DebugDrawProvider : public FusionEngine::DebugDrawProvider
	{
	public:
		DebugDrawProvider();

		std::shared_ptr<FusionEngine::DebugDrawImpl> Create() override;

		std::shared_ptr<FusionEngine::DebugDrawImpl> GetPersistentDrawingContext() const override;

		void Draw(const clan::Canvas& canvas, const Vector2& camera_pos) const;
		//const std::concurrent_vector<std::weak_ptr<ClanLibRenderer::DebugDrawImpl>>& GetContexts() const;

	private:
		tbb::concurrent_vector<std::weak_ptr<ClanLibRenderer::DebugDrawImpl>> m_Contexts;

		std::shared_ptr<FusionEngine::DebugDrawImpl> m_PersistentDrawingContext;

	};

} }

#endif
