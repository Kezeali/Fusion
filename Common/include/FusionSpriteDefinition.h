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

	//! Used when loading sprite files
	class LegacySpriteDefinition
	{
	public:
		typedef std::unordered_set<std::string> FilenameSet;

		struct Image
		{
			enum FrameType
			{
				FrameFile,
				FrameGridCell,
				FrameAlphaCell
			} type;

			std::string filename;
			//clan::Rect cell;

			int xpos, ypos, width, height;

			int xarray, yarray;
			int array_skipframes;
			int xspacing, yspacing;

			float trans_limit;

			bool free;

			clan::PixelBuffer image_data;

			Image()
				: xpos(0), ypos(0),
				width(0), height(0),
				xarray(1), yarray(1), array_skipframes(0),
				xspacing(0), yspacing(0),
				trans_limit(0.f),
				free(false)
			{}
		};

		typedef std::vector<Image> ImageArray;
	public:
		LegacySpriteDefinition();
		~LegacySpriteDefinition();

		//! Load the given XML document
		void LoadXml(const std::string &working_directory, const ticpp::Document &document, clan::VirtualDirectory &dir);

		clan::Sprite CreateSprite(clan::GraphicContext &gc);

		void SpriteReleased();

		void ClearImageData();

	protected:
		//! Add image definition
		void addImage(const Image &image_definition);

		//! Adds an image file
		void addImage(const std::string &filename);

		//! Adds a grid-clipped image
		void addImage(const std::string &filename, int xpos, int ypos, int width, int height, int xarray, int yarray, int array_skipframes, int xspacing, int yspacing);
		//! Adds an alpha-clipped image
		void addImage(const std::string &filename, int xpos, int ypos, float trans_limit, bool free = false);

		//! Returns true if the file exists
		bool exists(const std::string &filename);

		void loadImageElements(ticpp::Element *root);
		void loadMoreOptions(ticpp::Element *root);

		// Counts sprites based on this definition - when count reaches zero
		//  all image data is cleared
		unsigned int m_Users;

		std::string m_WorkingDirectory;

		FilenameSet m_ImageFiles;
		ImageArray m_Images;

		clan::SpriteDescription m_Description;

		// Initial Sprite settings
		clan::Colorf m_Colour; // colour overlay

		float m_Alpha; // image opacity

		struct AnimationOptions
		{
			int delay;
			bool loop;
			bool pingpong;
			bool backward;
			clan::Sprite::ShowOnFinish showOnFinish;
		} m_Animation;

		struct SpriteFrame
		{
			int number;
			int delay;
			clan::Point offset;
		};

		typedef std::vector<SpriteFrame> SpriteFrameArray;

		SpriteFrameArray m_Frames;

		float m_ScaleX, m_ScaleY;

		clan::Angle m_BaseAngle;

		clan::Origin m_OffsetOrigin;
		int m_OffsetX;
		int m_OffsetY;

		clan::Origin m_RotationOrigin;
		int m_RotationPointX;
		int m_RotationPointY;
	};

	//! Loads the given files into a SpriteDefinition
	void LoadSpriteDefinition(LegacySpriteDefinition &def, const std::string &filepath, clan::VirtualDirectory vdir);

}

#endif
