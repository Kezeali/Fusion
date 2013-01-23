/*
*  Copyright (c) 2010-2011 Fusion Project Team
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

#ifndef H_FusionSpriteDefinition
#define H_FusionSpriteDefinition

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <ClanLib/display.h>
#include "FusionResourceLoader.h"
#include "FusionResourcePointer.h"
#include "FusionXML.h"

#include <string>
#include <vector>

namespace FusionEngine
{

	//! Sprite animation definition
	class SpriteAnimation
	{
	public:
		SpriteAnimation();
		SpriteAnimation(clan::IODevice dev);

		void Load(clan::IODevice dev, const std::string& animation_name = std::string());

		std::vector<clan::Rect>& GetFrames() { return m_Frames; }
		std::vector<std::pair<int, double>>& GetFrameDelays() { return m_FrameDelays; }
		std::vector<std::pair<int, Vector2i>>& GetFrameOffsets() { return m_FrameOffsets; }

		double GetDefaultDelay() const { return m_DefaultDelay; }

		clan::Sprite::ShowOnFinish GetShowOnFinish() const { return m_ShowOnFinish; }
		bool GetPlayLoop() const { return m_PlayLoop; }
		bool GetPlayPingPong() const { return m_PlayPingPong; }

	private:
		std::vector<clan::Rect> m_Frames;
		std::vector<std::pair<int, double>> m_FrameDelays;
		std::vector<std::pair<int, Vector2i>> m_FrameOffsets;
		double m_DefaultDelay;
		clan::Sprite::ShowOnFinish m_ShowOnFinish;
		bool m_PlayLoop;
		bool m_PlayPingPong;
	};

	//! SpriteAnimation resource loader callback
	void LoadAnimationResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data);
	//! SpriteAnimation resource unloader callback
	void UnloadAnimationResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data);
	
	//! Sprite factory
	/*!
	* Used to create clan::Sprite objects from given texture and animation resources
	*/
	class SpriteDefinition
	{
	public:
		SpriteDefinition(const ResourcePointer<clan::Texture2D>& texture, const ResourcePointer<SpriteAnimation>& animation);
		~SpriteDefinition();

		void GenerateDescription();

		clan::Sprite CreateSprite(clan::GraphicContext &gc);

		ResourcePointer<clan::Texture2D> m_Texture;
		ResourcePointer<SpriteAnimation> m_Animation;

		clan::SpriteDescription m_Description;

		std::function<void (void)> m_UnusedCallback;
	};

	//! SpriteDefinition resource prereqs loader callback
	void LoadSpriteDefinitionResourcePrereqs(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data);
	//! SpriteDefinition resource loader callback
	void LoadSpriteDefinitionResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data);
	//! SpriteDefinition resource unloader callback
	void UnloadSpriteDefinitionResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data);

}

#endif
