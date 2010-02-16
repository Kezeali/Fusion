/*
  Copyright (c) 2010 Fusion Project Team

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

#ifndef Header_FusionSpriteDefinition
#define Header_FusionSpriteDefinition

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <ClanLib/display.h>
#include "FusionXML.h"

namespace FusionEngine
{
	
	//! Used to load a sprite file
	class SpriteDefinition
	{
	public:
		typedef std::tr1::unordered_set<std::string> FilenameSet;

		struct Image
		{
			enum FrameType
			{
				FrameFile,
				FrameGridCell,
				FrameAlphaCell
			} type;

			CL_String filename;
			//CL_Rect cell;

			int xpos, ypos, width, height;

			int xarray, yarray;
			int array_skipframes;
			int xspacing, yspacing;

			float trans_limit;

			bool free;

			CL_PixelBuffer image_data;

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
		SpriteDefinition();
		~SpriteDefinition();

		void LoadXml(const std::string &working_directory, TiXmlDocument *document);

		CL_Sprite *CreateSprite(CL_GraphicContext &gc, CL_VirtualDirectory &dir);

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

		void loadImageElements(TiXmlElement *root);
		void loadMoreOptions(TiXmlElement *root);

		// Counts sprites based on this definition - when count reaches zero
		//  all image data is cleared
		unsigned int m_Users;

		std::string m_WorkingDirectory;

		FilenameSet m_ImageFiles;
		ImageArray m_Images;

		// Initial Sprite settings
		CL_Colorf m_Colour;

		struct AnimationOptions
		{
			int delay;
			bool loop;
			bool pingpong;
			bool backward;
			CL_Sprite::ShowOnFinish showOnFinish;
		} m_Animation;

		struct SpriteFrame
		{
			int number;
			int delay;
			CL_Point offset;
		};

		typedef std::vector<SpriteFrame> SpriteFrameArray;

		SpriteFrameArray m_Frames;

		float m_ScaleX, m_ScaleY;

		CL_Angle m_BaseAngle;

		CL_Origin m_OffsetOrigin;
		int m_OffsetX;
		int m_OffsetY;

		CL_Origin m_RotationOrigin;
		int m_RotationPointX;
		int m_RotationPointY;
	};

	//! Loads the given files into a SpriteDefinition
	void LoadSpriteDefinition(SpriteDefinition &def, const std::string &filepath, CL_VirtualDirectory vdir);

}

#endif