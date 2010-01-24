/*
  Copyright (c) 2007-2009 Fusion Project Team

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

#include "FusionImageLoader.h"

#include "FusionXml.h"
#include "FusionPhysFS.h"
//#include "FusionResourceManager.h"


namespace FusionEngine
{

	void LoadImageResource(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			delete resource->GetDataPtr();
		}

		CL_String ext = CL_PathHelp::get_extension(resource->GetPath());
		CL_PixelBuffer sp;
		try
		{
			sp = CL_ImageProviderFactory::load(resource->GetPath(), vdir, ext);
		}
		catch (CL_Exception&)
		{
			FSN_WEXCEPT(ExCode::IO, L"LoadImageResource", L"'" + resource->GetPath() + L"' could not be loaded");
		}

		CL_PixelBuffer *data = new CL_PixelBuffer(sp);
		resource->SetDataPtr(data);

		resource->_setValid(true);
	}

	void UnloadImageResouce(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			resource->_setValid(false);
			delete resource->GetDataPtr();
		}
		resource->SetDataPtr(NULL);
	}

	void LoadSpriteResource(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			delete resource->GetDataPtr();
		}

		if (!resource->HasQuickLoadData())
		{
			SpriteDefinition *def = new SpriteDefinition();
			try
			{
				LoadSpriteDefinition(*def, resource->GetPath(), vdir);
				resource->SetQuickLoadDataPtr(def);
				resource->_setHasQuickLoadData(true);
			}
			catch (FileSystemException& ex)
			{
				delete def;
				FSN_WEXCEPT(ExCode::IO, L"LoadSpriteResource", L"Definition data for '" + resource->GetPath() + L"' could not be loaded: " + fe_widen(ex.GetDescription()));
			}
		}

		CL_Sprite *sprite = NULL;
		try
		{
			SpriteDefinition *def = static_cast<SpriteDefinition*>( resource->GetQuickLoadDataPtr() );
			sprite = def->CreateSprite(gc, vdir);
		}
		catch (CL_Exception&)
		{
			FSN_WEXCEPT(ExCode::IO, L"LoadSpriteResource", L"'" + resource->GetPath() + L"' could not be loaded");
		}

		resource->SetDataPtr(sprite);
		resource->_setValid(sprite != NULL);
	}

	void UnloadSpriteResource(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->IsLoaded())
		{
			resource->_setValid(false);

			delete resource->GetDataPtr();

			if (resource->HasQuickLoadData())
			{
				SpriteDefinition *def = static_cast<SpriteDefinition*>( resource->GetQuickLoadDataPtr() );
				def->SpriteReleased();
			}
		}

		resource->SetDataPtr(NULL);
	}

	void UnloadSpriteQuickLoadData(ResourceContainer* resource, CL_VirtualDirectory vdir, CL_GraphicContext &gc, void* userData)
	{
		if (resource->HasQuickLoadData())
			delete resource->GetQuickLoadDataPtr();

		resource->SetQuickLoadDataPtr(NULL);
		resource->_setHasQuickLoadData(false);
	}

	void LoadSpriteDefinition(SpriteDefinition &def, const std::wstring &filepath, CL_VirtualDirectory vdir)
	{
		TiXmlDocument *document = OpenXml(filepath, vdir);
		CL_String workingDirectory = vdir.get_path() + CL_PathHelp::get_basepath(filepath.c_str(), CL_PathHelp::path_type_virtual);

		if (workingDirectory[workingDirectory.length()-1] == '/')
			workingDirectory.erase(workingDirectory.length()-1);
		
		def.LoadXml(CL_StringHelp::text_to_local8( workingDirectory ).c_str(), document);
	}

	SpriteDefinition::SpriteDefinition()
		: m_Users(0),
		m_ScaleX(1.f), m_ScaleY(1.f),
		m_BaseAngle(0.f, cl_radians),
		m_OffsetOrigin(origin_top_left),
		m_OffsetX(0), m_OffsetY(0),
		m_RotationOrigin(origin_center),
		m_RotationPointX(0), m_RotationPointY(0)
	{
	}

	SpriteDefinition::~SpriteDefinition()
	{
		ClearImageData();
	}

	void SpriteDefinition::LoadXml(const std::string &working_directory, TiXmlDocument *document)
	{
		m_WorkingDirectory = working_directory;

		TiXmlElement *root = document->FirstChildElement();
		if (root == NULL)
			return;
		//FSN_EXCEPT(ExCode::FileType, "SpriteDefinition::LoadXml", "Document has no elements.");

		if (root->ValueStr() != "sprite")
			FSN_EXCEPT(ExCode::FileType, "SpriteDefinition::LoadXml", "Tried to load non-sprite document");

		// Load id
		//m_ID = CL_StringHelp::text_to_int(attribute_text);

		loadImageElements(root);
		loadMoreOptions(root);
	}

	CL_Sprite *SpriteDefinition::CreateSprite(CL_GraphicContext &gc, CL_VirtualDirectory &dir)
	{
		//ResourceManager *man = ResourceManager::getSingletonPtr();
		//if (man == NULL)
		//	return NULL;

		CL_SpriteDescription description;
		for (ImageArray::iterator it = m_Images.begin(), end = m_Images.end(); it != end; ++it)
		{
			// Load image data if necessary
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
			if (it->image_data.is_null())
					it->image_data = CL_ImageProviderFactory::load(it->filename, dir);
#endif

			CL_Texture texture = CL_SharedGCData::load_texture(gc, it->filename, dir);

			if (it->type == Image::FrameFile)
			{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
				description.add_frame(it->image_data);
#else
				description.add_frame(texture);
				//description.add_frame(it->filename, dir);
#endif
			}
			else if (it->type == Image::FrameGridCell)
			{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
				description.add_gridclipped_frames(
					it->image_data,
					it->xpos, it->ypos, it->width, it->height,
					it->xarray, it->yarray,
					it->array_skipframes,
					it->xspacing, it->yspacing);
#else
				//CL_PixelBuffer buf = CL_ImageProviderFactory::load(it->filename, dir);
				description.add_gridclipped_frames(
					texture,
					it->xpos, it->ypos, it->width, it->height,
					it->xarray, it->yarray,
					it->array_skipframes,
					it->xspacing, it->yspacing);
#endif
			}
			else if (it->type == Image::FrameAlphaCell)
			{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
				if (!it->free)
					description.add_alphaclipped_frames(it->image_data, it->xpos, it->ypos, it->trans_limit);
				else
					description.add_alphaclipped_frames(it->image_data, it->xpos, it->ypos, it->trans_limit);
#else
				//CL_PixelBuffer buf = CL_ImageProviderFactory::load(it->filename, dir);
				if (!it->free)
					description.add_alphaclipped_frames(texture, it->xpos, it->ypos, it->trans_limit);
				else
					description.add_alphaclipped_frames(texture, it->xpos, it->ypos, it->trans_limit);
#endif
			}
		}

		CL_Sprite *sprite = new CL_Sprite(gc, description);

		sprite->set_alignment(m_OffsetOrigin, m_OffsetX, m_OffsetY);
		sprite->set_rotation_hotspot(m_RotationOrigin, m_RotationPointX, m_RotationPointY);
		sprite->set_base_angle(m_BaseAngle);
		sprite->set_color(m_Colour);
		sprite->set_scale(m_ScaleX, m_ScaleY);
		// Animation
		sprite->set_delay(m_Animation.delay);
		sprite->set_play_loop(m_Animation.loop);
		sprite->set_play_pingpong(m_Animation.pingpong);
		sprite->set_play_backward(m_Animation.backward);
		sprite->set_show_on_finish(m_Animation.showOnFinish);
		// Frame delay / offset
		for (SpriteFrameArray::const_iterator it = m_Frames.begin(), end = m_Frames.end(); it != end; ++it)
		{
			sprite->set_frame_delay(it->number, it->delay);
			sprite->set_frame_offset(it->number, it->offset);
		}

		// Increment usage counter
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
		++m_Users;
#endif

		return sprite;
	}

	void SpriteDefinition::SpriteReleased()
	{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
		if (--m_Users == 0)
			ClearImageData();
#endif
	}

	void SpriteDefinition::ClearImageData()
	{
		// Drop all refs to frame pixelbuffers
		for (ImageArray::iterator it = m_Images.begin(), end = m_Images.end(); it != end; ++it)
		{
			it->image_data = CL_PixelBuffer();
		}
	}

	void SpriteDefinition::addImage(const SpriteDefinition::Image &image)
	{
		m_ImageFiles.insert(fe_narrow(image.filename.c_str()));
		m_Images.push_back(image);
	}

	void SpriteDefinition::addImage(const std::string &filename)
	{
		std::string expandedPath = SetupPhysFS::parse_path(m_WorkingDirectory, filename);

		Image image;
		image.type = Image::FrameFile;
		image.filename = expandedPath.c_str();

		m_Images.push_back(image);
		m_ImageFiles.insert(expandedPath);
	}

	void SpriteDefinition::addImage(const std::string &filename, int xpos, int ypos, int width, int height, int xarray, int yarray, int array_skipframes, int xspacing, int yspacing)
	{
		std::string expandedPath = SetupPhysFS::parse_path(m_WorkingDirectory, filename);

		Image image;
		image.type = Image::FrameGridCell;
		image.filename = expandedPath.c_str();

		image.xpos = xpos;
		image.ypos = ypos;

		image.width = width;
		image.height = height;

		image.xarray = xarray;
		image.yarray = yarray;

		image.array_skipframes = array_skipframes;

		image.xspacing = xspacing;
		image.yspacing = yspacing;

		m_Images.push_back(image);
		m_ImageFiles.insert(expandedPath);
	}

	void SpriteDefinition::addImage(const std::string &filename, int xpos, int ypos, float trans_limit, bool free)
	{
		std::string expandedPath = SetupPhysFS::parse_path(m_WorkingDirectory, filename);

		Image image;
		image.type = Image::FrameFile;
		image.filename = expandedPath.c_str();

		image.xpos = xpos;
		image.ypos = ypos;

		image.trans_limit = trans_limit;

		image.free = free;

		m_Images.push_back(image);
		m_ImageFiles.insert(expandedPath);
	}

	bool SpriteDefinition::exists(const std::string &filename)
	{
		std::string ex = SetupPhysFS::parse_path(m_WorkingDirectory, filename);
		return PHYSFS_exists(ex.c_str()) != 0;
	}

	void SpriteDefinition::loadImageElements(TiXmlElement *root)
	{
		TiXmlElement *element = root->FirstChildElement();
		while (element != NULL)
		{
			std::string tag_name = element->ValueStr();
			if (tag_name == "image" || tag_name == "image-file")
			{
				if (element->Attribute("filesequence") != NULL)
				{
					int start_index = 0;
					const char *attribute_text = element->Attribute("start_index");
					if (attribute_text != NULL)
						start_index = CL_StringHelp::local8_to_int(attribute_text);

					int skip_index = 1;
					attribute_text = element->Attribute("skip_index");
					if (attribute_text != NULL)
						skip_index = CL_StringHelp::text_to_int(attribute_text);

					int leading_zeroes = 0;
					attribute_text = element->Attribute("leading_zeroes");
					if (attribute_text != NULL)
						leading_zeroes =  CL_StringHelp::text_to_int(attribute_text);

					std::string prefix = element->Attribute("filesequence");
					std::string suffix = std::string(".") +
						CL_StringHelp::text_to_local8( CL_PathHelp::get_extension(CL_String(prefix.c_str()), CL_PathHelp::path_type_virtual) ).c_str();
					prefix.erase(prefix.length() - suffix.length(), prefix.length()); //remove the extension

					for (int i = start_index;; i += skip_index)
					{
						std::string file_name = prefix;

						std::string  frame_text = CL_StringHelp::int_to_local8(i);
						for (int zeroes_to_add = (leading_zeroes+1) - frame_text.length(); zeroes_to_add > 0; zeroes_to_add--)
							file_name += "0";

						file_name += frame_text + suffix;

						if (!exists(file_name))
							break;

						addImage(file_name);
					}
				}
				else
				{
					const char *attribute_text = element->Attribute("file");
					if (attribute_text == NULL)
						continue;

					std::string image_name(attribute_text);
					
					if (!exists(image_name))
					{
						element = element->NextSiblingElement();
						continue;
					}

					TiXmlElement *cur_child = element->FirstChildElement();
					if(cur_child == NULL) 
					{
						addImage(image_name);
					}
					else 
					{
						do {
							if(cur_child->ValueStr() == "grid")
							{
								int xpos = 0;
								int ypos = 0;
								int xarray = 1;
								int yarray = 1;
								int array_skipframes = 0;
								int xspacing = 0;
								int yspacing = 0;
								int width = 0;
								int height = 0;

								const char *attribute_text = cur_child->Attribute("size");
								std::vector<CL_TempString> image_size = CL_StringHelp::split_text(CL_String(attribute_text), cl_text(","));
								if (image_size.size() > 0)
									width = CL_StringHelp::text_to_int(image_size[0]);
								if (image_size.size() > 1)
									height = CL_StringHelp::text_to_int(image_size[1]);

								attribute_text = cur_child->Attribute("pos");
								if (attribute_text != NULL)
								{
									std::vector<CL_TempString> image_pos = CL_StringHelp::split_text(CL_String(attribute_text), cl_text(","));
									if (image_pos.size() > 0)
										xpos = CL_StringHelp::text_to_int(image_pos[0]);
									if (image_pos.size() > 1)
										ypos = CL_StringHelp::text_to_int(image_pos[1]);
								}

								attribute_text = cur_child->Attribute("array");
								if (attribute_text != NULL)
								{
									std::vector<CL_TempString> image_array = CL_StringHelp::split_text(CL_String(attribute_text), cl_text(","));
									if (image_array.size() == 2)
									{
										xarray = CL_StringHelp::text_to_int(image_array[0]);
										yarray = CL_StringHelp::text_to_int(image_array[1]);
									}
									else
									{
										FSN_EXCEPT(ExCode::FileType,
											"SpriteDescription::LoadXml",
											"Sprite using image '" + image_name + "' has incorrect array attribute, must be \"X,Y\"!");
									}
								}

								attribute_text = cur_child->Attribute("array_skipframes");
								if (attribute_text != NULL)
								{
									array_skipframes = CL_StringHelp::text_to_int( CL_String(attribute_text) );
								}

								attribute_text = cur_child->Attribute("spacing");
								if (attribute_text != NULL)
								{
									std::vector<CL_TempString> image_spacing = CL_StringHelp::split_text(CL_String(attribute_text), cl_text(","));
									xspacing = CL_StringHelp::text_to_int(image_spacing[0]);
									yspacing = CL_StringHelp::text_to_int(image_spacing[1]);
								}

								addImage(image_name, xpos, ypos, width, height, xarray, yarray, array_skipframes, xspacing, yspacing);
							}
							else if(cur_child->ValueStr() == "alpha")
							{
								int xpos = 0;
								int ypos = 0;
								float trans_limit = 0.05f;

								const char *attribute_text = cur_child->Attribute("pos");
								if (attribute_text != NULL)
								{
									std::vector<CL_TempString> image_pos = CL_StringHelp::split_text(CL_String(attribute_text), cl_text(","));
									xpos = CL_StringHelp::text_to_int(image_pos[0]);
									ypos = CL_StringHelp::text_to_int(image_pos[1]);
								}

								attribute_text = cur_child->Attribute("trans_limit");
								if (attribute_text != NULL)
								{
									trans_limit = CL_StringHelp::text_to_float(CL_String(attribute_text));
								}

								if (cur_child->Attribute("free") != NULL)
								{
									addImage(
										image_name,
										xpos, ypos,
										trans_limit,
										true);
								}
								else
								{
									addImage(
										image_name,
										xpos, ypos,
										trans_limit,
										false);
								}
							}

							cur_child = cur_child->NextSiblingElement();
						} while(cur_child != NULL);
					}
				}
			}

			element = element->NextSiblingElement();
		}

		if (m_Images.empty()) 
			FSN_EXCEPT(ExCode::FileType, "SpriteDefinition::LoadXml", "Sprite resource contained no frames!");
	}

	CL_Origin readOrigin(const char *attribute_text, CL_Origin default_)
	{
		if (attribute_text != NULL)
		{
			std::string originAttribute = std::string(attribute_text);
			if(originAttribute == "center")
				return origin_center;
			else if(originAttribute == "top_left")
				return origin_top_left;
			else if(originAttribute == "top_center")
				return origin_top_center;
			else if(originAttribute == "top_right")
				return origin_top_right;
			else if(originAttribute == "center_left")
				return origin_center_left;
			else if(originAttribute == "center_right")
				return origin_center_right;
			else if(originAttribute == "bottom_left")
				return origin_bottom_left;
			else if(originAttribute == "bottom_center")
				return origin_bottom_center;
			else if(originAttribute == "bottom_right")
				return origin_bottom_right;
		}

		return default_;
	}

	void SpriteDefinition::loadMoreOptions(TiXmlElement *root)
	{
		// Load play options	
		TiXmlElement *element = root->FirstChildElement();
		while (element != NULL)
		{
			std::string tag_name = element->ValueStr();

			const char *attribute_text;

			// <color red="float" green="float" blue="float" alpha="float" />
			if (tag_name == "color")
			{
				attribute_text = element->Attribute("red");
				if (attribute_text != NULL)
					m_Colour.r = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));
				else
					m_Colour.r = 1.f;

				attribute_text = element->Attribute("green");
				if (attribute_text != NULL)
					m_Colour.g = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));
				else
					m_Colour.g = 1.f;

				attribute_text = element->Attribute("blue");
				if (attribute_text != NULL)
					m_Colour.b = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));
				else
					m_Colour.b = 0.f;
				
				attribute_text = element->Attribute("alpha");
				if (attribute_text != NULL)
					m_Colour.a = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));
				else
					m_Colour.a = 1.f;
			}
			// <animation speed="integer" loop="[yes,no]" pingpong="[yes,no]" direction="[backward,forward]" on_finish="[blank,last_frame,first_frame]"/>
			else if (tag_name == "animation")
			{
				attribute_text = element->Attribute("speed");
				if (attribute_text != NULL)
					m_Animation.delay = CL_StringHelp::text_to_int(CL_String(attribute_text));
				else
					m_Animation.delay = 60;

				attribute_text = element->Attribute("loop");
				if (attribute_text != NULL)
					m_Animation.loop = CL_StringHelp::local8_to_bool(attribute_text);
				else
					m_Animation.loop = true;

				attribute_text = element->Attribute("pingpong");
				if (attribute_text != NULL)
					m_Animation.pingpong = CL_StringHelp::local8_to_bool(attribute_text);
				else
					m_Animation.pingpong = false;

				attribute_text = element->Attribute("direction");
				if (attribute_text != NULL)
					m_Animation.backward = std::string(attribute_text) == "backward";
				else
					m_Animation.backward = false; // forward

				attribute_text = element->Attribute("on_finish");
				bool on_finish_Valid = false;
				if (attribute_text != NULL)
				{
					on_finish_Valid = true;
					std::string on_finish(attribute_text);
					if (on_finish == "first_frame")
						m_Animation.showOnFinish = CL_Sprite::show_first_frame;
					else if(on_finish == "last_frame")
						m_Animation.showOnFinish = CL_Sprite::show_last_frame;
					else
						on_finish_Valid = false;
				}
				if (!on_finish_Valid)
					m_Animation.showOnFinish = CL_Sprite::show_blank;
			}
			// <scale x="float" y="float />
			else if (tag_name == "scale")
			{
				attribute_text = element->Attribute("x");
				if (attribute_text != NULL) m_ScaleX = CL_StringHelp::local8_to_float(attribute_text);
				else m_ScaleX = 1.f;

				attribute_text = element->Attribute("y");
				if (attribute_text != NULL) m_ScaleY = CL_StringHelp::local8_to_float(attribute_text);
				else m_ScaleY = 1.f;
			}
			// <translation origin="string" x="integer" y="integer" />
			else if (tag_name == "translation")
			{
				const char *attribute_text = element->Attribute("origin");
				m_OffsetOrigin = readOrigin(attribute_text, origin_top_left);

				attribute_text = element->Attribute("x");
				if (attribute_text != NULL)
					m_OffsetX = CL_StringHelp::local8_to_int(attribute_text);
				else m_OffsetX = 0;

				attribute_text = element->Attribute("y");
				if (attribute_text != NULL)
					m_OffsetY = CL_StringHelp::local8_to_int(attribute_text);
				else m_OffsetY = 0;

			}
			// <rotation origin="string" x="integer" y="integer" />
			else if (tag_name == "rotation")
			{
				const char *attribute_text = element->Attribute("base_angle");
				if (attribute_text != NULL)
					m_BaseAngle = CL_Angle(CL_StringHelp::local8_to_float(attribute_text), cl_degrees);
				
				attribute_text = element->Attribute("origin");
				m_RotationOrigin = readOrigin(attribute_text, origin_center);

				attribute_text = element->Attribute("x");
				if (attribute_text != NULL)
					m_OffsetX = CL_StringHelp::local8_to_int(attribute_text);
				else m_OffsetX = 0;

				attribute_text = element->Attribute("y");
				if (attribute_text != NULL)
					m_RotationPointX = CL_StringHelp::local8_to_int(attribute_text);
				else m_RotationPointY = 0;
			}
			// <frame nr="integer" speed="integer" x="integer" y="integer" />
			else if (tag_name == "frame")
			{
				SpriteFrame frame;
				frame.number = 0;

				const char *attribute_text = element->Attribute("number");
				if (attribute_text != NULL)
					frame.number = CL_StringHelp::local8_to_int(attribute_text);

				attribute_text = element->Attribute("speed");
				frame.delay = 60;
				if (attribute_text != NULL)
					frame.delay = CL_StringHelp::local8_to_int(attribute_text);

				attribute_text = element->Attribute("x");
				frame.offset.x = 0;
				if (attribute_text != NULL)
					frame.offset.x = CL_StringHelp::local8_to_int(attribute_text);

				attribute_text = element->Attribute("y");
				frame.offset.y = 0;
				if (attribute_text != NULL)
					frame.offset.y = CL_StringHelp::local8_to_int(attribute_text);

				m_Frames.push_back(frame);
			}

			element = element->NextSiblingElement();
		}
	}

	/*void SpriteDefinition::addGridclippedFrames(
		const CL_Texture &texture, 
		int xpos, int ypos, 
		int width, int height, 
		int xarray, int yarray, 
		int array_skipframes, 
		int xspace, int yspace)
	{
		int ystart = ypos;
		for(int y = 0; y < yarray; y++)
		{
			int xstart = xpos;
			for(int x = 0; x < xarray; x++)
			{
				if (y == yarray -1 && x >= xarray - array_skipframes)
					break;

				if(xstart + width > texture.get_width() || ystart + height > texture.get_height())
					throw CL_Exception(cl_text("add_gridclipped_frames: Outside texture bounds"));

				impl->frames.push_back(CL_SpriteDescriptionFrame(texture, CL_Rect(xstart, ystart, xstart + width, ystart + height)));
				xstart += width + xspace;
			}
			ystart += height + yspace;
		}
	}*/

};