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

#ifndef Header_FusionEngine_Renderer
#define Header_FusionEngine_Renderer

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionEntity.h"
#include "FusionCamera.h"
#include "FusionViewport.h"


namespace FusionEngine
{

	/*!
	 * \brief
	 * Renders Entities
	 *
	 * \see
	 * Viewport | Renderable | EntityManager
	 */
	class Renderer
	{
	protected:
		//typedef std::set<std::string> BlockedTagSet;

		typedef std::set<EntityPtr> EntitySet;

	public:
		//! Constructor
		Renderer(const CL_GraphicContext &gc);
		//! Destructor
		virtual ~Renderer();

		//enum ViewportArea
		//{
		//	ViewFull,
		//	ViewVerticalHalf,
		//	ViewHorizontalHalf,
		//	ViewQuarter
		//};
		void CalculateScreenArea(CL_Rect &area, const ViewportPtr &viewport);


		const CL_GraphicContext &GetGraphicContext() const;

		int GetContextWidth() const;
		int GetContextHeight() const;

		void Add(const EntityPtr &entity);
		void Remove(const EntityPtr &entity);

		void Clear();

		void ShowTag(const std::string &tag);
		void HideTag(const std::string &tag);

		//void AddViewport(ViewportPtr viewport);

		void Update(float split);
		void Draw(ViewportPtr viewport);

	protected:
		bool updateTags(const EntityPtr &entity) const;

		void drawRenderables(EntityPtr &entity, const CL_Rectf &cull_outside);

		void drawNormally(const CL_Rectf &cull_outside);
		void updateDrawArray();

		struct ChangingTagCollection
		{
			void show(const std::string &tag)
			{
				m_HiddenTags.erase(tag);
				m_ShownTags.insert(tag);
			}

			void hide(const std::string &tag)
			{
				m_ShownTags.erase(tag);
				m_HiddenTags.insert(tag);
			}

			void clear()
			{
				m_ShownTags.clear();
				m_HiddenTags.clear();
			}

			bool somethingWasShown() const
			{
				return !m_ShownTags.empty();
			}

			bool checkShown(const std::string &tag) const
			{
				return m_ShownTags.find(tag) == m_ShownTags.end();
			}

			bool wasShown(const EntityPtr &entity) const
			{
				for (StringSet::const_iterator it = m_ShownTags.begin(), end = m_ShownTags.end(); it != end; ++it)
				{
					if (entity->CheckTag(*it))
						return true;
				}

				return false;
			}

			bool checkHidden(const std::string &tag) const
			{
				return m_HiddenTags.find(tag) == m_HiddenTags.end();
			}

			bool wasHidden(const EntityPtr &entity) const
			{
				for (StringSet::const_iterator it = m_HiddenTags.begin(), end = m_HiddenTags.end(); it != end; ++it)
				{
					if (entity->CheckTag(*it))
						return true;
				}

				return false;
			}

			const StringSet &shownTags() const
			{
				return m_ShownTags;
			}

			const StringSet &hiddenTags() const
			{
				return m_HiddenTags;
			}

			std::set<std::string> m_ShownTags;
			std::set<std::string> m_HiddenTags;
		} m_ChangedTags;

		//StringSet m_ShownTags;
		StringSet m_HiddenTags;

		EntitySet m_Entities;

		bool m_EntityAdded;

		EntityArray m_EntitiesToDraw;

		RenderableArray m_Renderables;

		//ViewportArray m_Viewports;

		CL_GraphicContext m_GC;

	};

}

#endif