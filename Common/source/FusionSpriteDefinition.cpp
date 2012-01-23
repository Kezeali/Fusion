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

#include "PrecompiledHeaders.h"

#include "FusionSpriteDefinition.h"

#include "FusionExceptionFactory.h"
#include "FusionPhysFS.h"
#include "FusionXML.h"

//#include "FusionPhysFSIOStream.h"
#include <boost/iostreams/stream.hpp>

#include <yaml-cpp/yaml.h>

namespace FusionEngine
{

	namespace IO
	{

		class CLStreamDevice
		{
		public:
			struct category
				: boost::iostreams::seekable_device_tag/*, boost::iostreams::flushable_tag*/, boost::iostreams::closable_tag
			{};
			typedef char char_type;

			CLStreamDevice(CL_IODevice dev);

			void close();

			std::streamsize read(char* s, std::streamsize n);
			std::streamsize write(const char* s, std::streamsize n);
			std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

		private:
			CL_IODevice m_Device;
		};

		typedef boost::iostreams::stream<CLStreamDevice> CLStream;

		CLStreamDevice::CLStreamDevice(CL_IODevice dev)
			: m_Device(dev)
		{
			if (m_Device.is_null())
				FSN_EXCEPT(FileSystemException, "Tried to load data from an invalid source");
		}

		void CLStreamDevice::close()
		{
			m_Device = CL_IODevice();
		}

		std::streamsize CLStreamDevice::read(char* s, std::streamsize n)
		{
			int ret = m_Device.read(static_cast<void*>(s), (int)n);
			return std::streamsize(ret);
		}

		std::streamsize CLStreamDevice::write(const char* s, std::streamsize n)
		{
			int ret = m_Device.write(static_cast<const void*>(s), (int)n);
			return std::streamsize(ret);
		}

		std::streampos CLStreamDevice::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
		{
			CL_IODevice::SeekMode clSeekMode;
			switch (way)
			{
			case std::ios_base::cur:
				clSeekMode = CL_IODevice::seek_cur;
				break;
			case std::ios_base::end:
				clSeekMode = CL_IODevice::seek_end;
				break;
			default:
				clSeekMode = CL_IODevice::seek_set;
				break;
			};

			if (m_Device.seek((int)off, clSeekMode))
				FSN_EXCEPT(FileSystemException, "Tried to go to an invalid position within a data-source");

			return std::streampos(m_Device.get_position());
		}

	}

	SpriteAnimation::SpriteAnimation()
		: m_DefaultDelay(0.0f)
	{
	}

	SpriteAnimation::SpriteAnimation(CL_IODevice dev)
		: m_DefaultDelay(0.0f)
	{
		Load(dev);
	}

	void operator >> (const YAML::Node& node, CL_Rect& rect)
	{
		node[0] >> rect.left;
		node[1] >> rect.top;
		node[2] >> rect.right;
		node[3] >> rect.bottom;
	}

	void operator >> (const YAML::Node& node, Vector2& vec)
	{
		node[0] >> vec.x;
		node[1] >> vec.y;
	}

	struct FrameInfo
	{
		CL_Rect frameRect;
		double delay;
		Vector2 offset;
	};

	void operator >> (const YAML::Node& node, FrameInfo& frame_info)
	{
		node["cell"] >> frame_info.frameRect;
		node["delay"] >> frame_info.delay;
		node["offset"] >> frame_info.offset;
	}

	inline const YAML::Node* FindValueOf(const YAML::Node& parent, const std::string& k0, const std::string& k1, const std::string& k2 = std::string(), const std::string& k3 = std::string(), const std::string& k4 = std::string())
	{
		if (auto node = parent.FindValue(k0))
			return node;
		else if (auto node = parent.FindValue(k1))
			return node;
		else if (auto node = parent.FindValue(k2))
			return node;
		else if (auto node = parent.FindValue(k3))
			return node;
		else if (auto node = parent.FindValue(k4))
			return node;
		else
			return nullptr;
	}

	void SpriteAnimation::Load(CL_IODevice dev, const std::string& animation_name)
	{
		try
		{
			IO::CLStream stream(dev);
			YAML::Parser p(stream);
			YAML::Node doc;
			if (p.GetNextDocument(doc))
			{
				if (doc.begin() == doc.end())
					return;

				auto currentAnimationNode = &(*doc.begin());
				if (!animation_name.empty())
				{
					bool got = false;
					auto it = doc.begin();
					for (; it != doc.end(); ++it)
					{
						if (auto node = it->FindValue("name"))
						{
							std::string name;
							*node >> name;
							if (name == animation_name)
							{
								currentAnimationNode = &(*it);
								got = true;
								break;
							}
						}
					}
				}
					
				if (!currentAnimationNode)
					return;


				if (auto node = FindValueOf(*currentAnimationNode, "default_delay", "default_frame_time", "default_duration"))
					*node >> m_DefaultDelay;
				else if (auto node = currentAnimationNode->FindValue("framerate"))
				{
					double framerate;
					*node >> framerate;
					m_DefaultDelay = 1.0 / framerate;
				}

				auto& framesNode = (*currentAnimationNode)["frames"];
				m_Frames.resize(framesNode.size());
				for (unsigned i = 0; i < framesNode.size(); ++i)
				{
					auto& frameNode = framesNode[i];

					if (frameNode.GetType() == YAML::CONTENT_TYPE::CT_SEQUENCE)
					{
						auto& frameRect = m_Frames.at(i);
						frameNode >> frameRect;
					}
					else
					{
						if (auto node = FindValueOf(frameNode, "rect", "frame"))
						{
							auto& frameRect = m_Frames.at(i);
							*node >> frameRect;
						}

						if (auto node = FindValueOf(frameNode, "delay", "duration", "frame_time"))
						{
							double delay;
							*node >> delay;
							m_FrameDelays.push_back(std::make_pair(i, delay));
						}

						if (auto node = frameNode.FindValue("offset"))
						{
							// TODO: ?make this Vector2i
							Vector2 offset;
							*node >> offset;
							m_FrameOffsets.push_back(std::make_pair(i, offset));
						}
					}
				}
			}
			else
			{
				if (animation_name.empty())
					FSN_EXCEPT(FileTypeException, "The given animation file contains no entries");
				else
					FSN_EXCEPT(FileTypeException, "The given animation file does not contain the document '" + animation_name + "'");
			}
		}
		catch (YAML::Exception& ex)
		{
			FSN_EXCEPT(FileTypeException, std::string("Invalid animation description: ") + ex.what());
		}
	}

	void LoadAnimationResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		FSN_ASSERT(!resource->IsLoaded());

		resource->setLoaded(false);

		SpriteAnimation *data = new SpriteAnimation();
		try
		{
			const auto pathEnd = resource->GetPath().find(":");
			const auto path = resource->GetPath().substr(0, pathEnd);
			const auto animationName = pathEnd != std::string::npos ? resource->GetPath().substr(pathEnd + 1) : "";
			auto dev = vdir.open_file(path, CL_File::open_existing, CL_File::access_read);
			data->Load(dev, animationName);
		}
		catch (CL_Exception& ex)
		{
			delete data;
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + std::string(ex.what()));
		}
		catch (Exception&)
		{
			delete data;
			throw;
		}

		resource->SetDataPtr(data);

		resource->setLoaded(true);
	}

	void UnloadAnimationResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<SpriteAnimation*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	SpriteDefinition2::SpriteDefinition2(const ResourcePointer<CL_Texture>& texture, const ResourcePointer<SpriteAnimation>& animation)
		: m_Texture(texture),
		m_Animation(animation)
	{
		GenerateDescription();
	}

	void SpriteDefinition2::GenerateDescription()
	{
		if (m_Texture.IsLoaded() && !m_Texture.Get()->is_null())
		{
			if (!m_Animation.IsLoaded())
			{
				m_Description.add_frame(*m_Texture.Get());
			}
			else
			{
				auto& frames = m_Animation->GetFrames();
				m_Description.add_frames(*m_Texture.Get(), frames.data(), frames.size());

				auto& frameDelays = m_Animation->GetFrameDelays();
				for (auto it = frameDelays.begin(), end = frameDelays.end(); it != end; ++it)
				{
					m_Description.set_frame_delay(it->first, it->second);
				}
			}
		}
		else
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to generate a sprite using an unloaded texture");
		}
	}

	CL_Sprite SpriteDefinition2::CreateSprite(CL_GraphicContext &gc)
	{
		CL_Sprite sprite(gc, m_Description);
		if (m_Animation.IsLoaded())
		{
			sprite.set_delay((int)(m_Animation->GetDefaultDelay() * 1000 + 0.5));
			auto& frameOffsets = m_Animation->GetFrameOffsets();
			for (auto it = frameOffsets.begin(), end = frameOffsets.end(); it != end; ++it)
			{
				sprite.set_frame_offset(it->first, CL_Point(it->second.x, it->second.y));
			}
		}
		return sprite;
	}

	void LoadSpriteDefinition(SpriteDefinition &def, const std::string &filepath, CL_VirtualDirectory vdir)
	{
		TiXmlDocument *document = OpenXml(filepath, vdir);
		CL_String workingDirectory = vdir.get_path() + CL_PathHelp::get_basepath(filepath.c_str(), CL_PathHelp::path_type_virtual);

		if (workingDirectory[workingDirectory.length()-1] == '/')
			workingDirectory.erase(workingDirectory.length()-1);
		
		def.LoadXml(CL_StringHelp::text_to_local8( workingDirectory ), document, vdir);
	}

	SpriteDefinition::SpriteDefinition()
		: m_Users(0),
		m_ScaleX(1.f), m_ScaleY(1.f),
		m_BaseAngle(0.f, cl_radians),
		m_OffsetOrigin(origin_top_left),
		m_OffsetX(0), m_OffsetY(0),
		m_RotationOrigin(origin_center),
		m_RotationPointX(0), m_RotationPointY(0),
		m_Colour(CL_Color::white),
		m_Alpha(1.0f)
	{
	}

	SpriteDefinition::~SpriteDefinition()
	{
		ClearImageData();
	}

	void SpriteDefinition::LoadXml(const std::string &working_directory, TiXmlDocument *document, CL_VirtualDirectory &dir)
	{
		m_WorkingDirectory = working_directory;

		TiXmlElement *root = document->FirstChildElement();
		if (root == nullptr)
			FSN_EXCEPT(FileTypeException, "Not a valid XML sprite definition.");

		if (root->ValueStr() != "sprite")
			FSN_EXCEPT(FileTypeException, "Not a sprite definition.");

		// Load id
		//m_ID = CL_StringHelp::text_to_int(attribute_text);

		loadImageElements(root);
		loadMoreOptions(root);

		for (ImageArray::iterator it = m_Images.begin(), end = m_Images.end(); it != end; ++it)
		{
			// Load image data if necessary
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
			if (it->image_data.is_null())
				it->image_data = CL_ImageProviderFactory::load(it->filename, dir);
#else
			CL_PixelBuffer image_data = CL_ImageProviderFactory::load(it->filename, dir);
#endif

			if (it->type == Image::FrameFile)
			{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
				m_Description.add_frame(it->image_data);
#else
				m_Description.add_frame(image_data);
#endif
			}
			else if (it->type == Image::FrameGridCell)
			{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
				m_Description.add_gridclipped_frames(
					it->image_data,
					it->xpos, it->ypos, it->width, it->height,
					it->xarray, it->yarray,
					it->array_skipframes,
					it->xspacing, it->yspacing);
#else
				m_Description.add_gridclipped_frames(
					image_data,
					it->xpos, it->ypos, it->width, it->height,
					it->xarray, it->yarray,
					it->array_skipframes,
					it->xspacing, it->yspacing);
#endif
			}
			else if (it->type == Image::FrameAlphaCell)
			{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
				if (it->free)
					m_Description.add_alphaclipped_frames_free(it->image_data, it->xpos, it->ypos, it->trans_limit);
				else
					m_Description.add_alphaclipped_frames(it->image_data, it->xpos, it->ypos, it->trans_limit);
#else
				if (it->free)
					m_Description.add_alphaclipped_frames_free(image_data, it->xpos, it->ypos, it->trans_limit);
				else
					m_Description.add_alphaclipped_frames(image_data, it->xpos, it->ypos, it->trans_limit);
#endif
			}
		}
	}

	CL_Sprite SpriteDefinition::CreateSprite(CL_GraphicContext &gc)
	{
		CL_Sprite sprite(gc, m_Description);

		sprite.set_alignment(m_OffsetOrigin, m_OffsetX, m_OffsetY);
		sprite.set_rotation_hotspot(m_RotationOrigin, m_RotationPointX, m_RotationPointY);
		sprite.set_base_angle(m_BaseAngle);
		sprite.set_color(m_Colour);
		sprite.set_scale(m_ScaleX, m_ScaleY);
		// Animation
		sprite.set_delay(m_Animation.delay);
		sprite.set_play_loop(m_Animation.loop);
		sprite.set_play_pingpong(m_Animation.pingpong);
		sprite.set_play_backward(m_Animation.backward);
		sprite.set_show_on_finish(m_Animation.showOnFinish);
		// Frame delay / offset
		for (SpriteFrameArray::const_iterator it = m_Frames.begin(), end = m_Frames.end(); it != end; ++it)
		{
			sprite.set_frame_delay(it->number, it->delay);
			sprite.set_frame_offset(it->number, it->offset);
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
		m_ImageFiles.insert(image.filename);
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
		while (element != nullptr)
		{
			std::string tag_name = element->ValueStr();
			if (tag_name == "image" || tag_name == "image-file")
			{
				if (element->Attribute("filesequence") != nullptr)
				{
					int start_index = 0;
					const char *attribute_text = element->Attribute("start_index");
					if (attribute_text != nullptr)
						start_index = CL_StringHelp::local8_to_int(attribute_text);

					int skip_index = 1;
					attribute_text = element->Attribute("skip_index");
					if (attribute_text != nullptr)
						skip_index = CL_StringHelp::text_to_int(attribute_text);

					int leading_zeroes = 0;
					attribute_text = element->Attribute("leading_zeroes");
					if (attribute_text != nullptr)
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
					if (attribute_text == nullptr)
						continue;

					std::string image_name(attribute_text);
					
					if (!exists(image_name))
					{
						element = element->NextSiblingElement();
						continue;
					}

					TiXmlElement *cur_child = element->FirstChildElement();
					if(cur_child == nullptr) 
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
								std::vector<CL_String> image_size = CL_StringHelp::split_text(CL_String(attribute_text), ",");
								if (image_size.size() > 0)
									width = CL_StringHelp::text_to_int(image_size[0]);
								if (image_size.size() > 1)
									height = CL_StringHelp::text_to_int(image_size[1]);

								attribute_text = cur_child->Attribute("pos");
								if (attribute_text != nullptr)
								{
									std::vector<CL_String> image_pos = CL_StringHelp::split_text(CL_String(attribute_text), ",");
									if (image_pos.size() > 0)
										xpos = CL_StringHelp::text_to_int(image_pos[0]);
									if (image_pos.size() > 1)
										ypos = CL_StringHelp::text_to_int(image_pos[1]);
								}

								attribute_text = cur_child->Attribute("array");
								if (attribute_text != nullptr)
								{
									std::vector<CL_String> image_array = CL_StringHelp::split_text(CL_String(attribute_text), ",");
									if (image_array.size() == 2)
									{
										xarray = CL_StringHelp::text_to_int(image_array[0]);
										yarray = CL_StringHelp::text_to_int(image_array[1]);
									}
									else
									{
										FSN_EXCEPT(FileTypeException,
											"Sprite using image '" + image_name + "' has incorrect array attribute, must be \"X,Y\"!");
									}
								}

								attribute_text = cur_child->Attribute("array_skipframes");
								if (attribute_text != nullptr)
								{
									array_skipframes = CL_StringHelp::text_to_int( CL_String(attribute_text) );
								}

								attribute_text = cur_child->Attribute("spacing");
								if (attribute_text != nullptr)
								{
									std::vector<CL_String> image_spacing = CL_StringHelp::split_text(CL_String(attribute_text), ",");
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
								if (attribute_text != nullptr)
								{
									std::vector<CL_String> image_pos = CL_StringHelp::split_text(CL_String(attribute_text), ",");
									xpos = CL_StringHelp::text_to_int(image_pos[0]);
									ypos = CL_StringHelp::text_to_int(image_pos[1]);
								}

								attribute_text = cur_child->Attribute("trans_limit");
								if (attribute_text != nullptr)
								{
									trans_limit = CL_StringHelp::text_to_float(CL_String(attribute_text));
								}

								if (cur_child->Attribute("free") != nullptr)
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
						} while(cur_child != nullptr);
					}
				}
			}

			element = element->NextSiblingElement();
		}

		if (m_Images.empty()) 
			FSN_EXCEPT(FileTypeException, "Sprite resource contained no frames!");
	}

	CL_Origin readOrigin(const char *attribute_text, CL_Origin default_)
	{
		if (attribute_text != nullptr)
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
		while (element != nullptr)
		{
			std::string tag_name = element->ValueStr();

			const char *attribute_text;

			// <color name="string" red="float" green="float" blue="float" alpha="float" />
			if (tag_name == "color")
			{
				attribute_text = element->Attribute("name");
				if (attribute_text != nullptr)
					m_Colour = CL_Colorf( CL_Color::find_color(attribute_text) );

				attribute_text = element->Attribute("red");
				if (attribute_text != nullptr)
					m_Colour.r = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));

				attribute_text = element->Attribute("green");
				if (attribute_text != nullptr)
					m_Colour.g = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));

				attribute_text = element->Attribute("blue");
				if (attribute_text != nullptr)
					m_Colour.b = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));
				
				attribute_text = element->Attribute("alpha");
				if (attribute_text != nullptr)
					m_Colour.a = (float)CL_StringHelp::text_to_float(CL_String(attribute_text));
			}
			// <animation speed="integer" loop="[yes,no]" pingpong="[yes,no]" direction="[backward,forward]" on_finish="[blank,last_frame,first_frame]"/>
			else if (tag_name == "animation")
			{
				attribute_text = element->Attribute("speed");
				if (attribute_text != nullptr)
					m_Animation.delay = CL_StringHelp::text_to_int(CL_String(attribute_text));
				else
					m_Animation.delay = 60;

				attribute_text = element->Attribute("loop");
				if (attribute_text != nullptr)
					m_Animation.loop = CL_StringHelp::local8_to_bool(attribute_text);
				else
					m_Animation.loop = true;

				attribute_text = element->Attribute("pingpong");
				if (attribute_text != nullptr)
					m_Animation.pingpong = CL_StringHelp::local8_to_bool(attribute_text);
				else
					m_Animation.pingpong = false;

				attribute_text = element->Attribute("direction");
				if (attribute_text != nullptr)
					m_Animation.backward = std::string(attribute_text) == "backward";
				else
					m_Animation.backward = false; // forward

				attribute_text = element->Attribute("on_finish");
				bool on_finish_Valid = false;
				if (attribute_text != nullptr)
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
				if (attribute_text != nullptr) m_ScaleX = CL_StringHelp::local8_to_float(attribute_text);
				else m_ScaleX = 1.f;

				attribute_text = element->Attribute("y");
				if (attribute_text != nullptr) m_ScaleY = CL_StringHelp::local8_to_float(attribute_text);
				else m_ScaleY = 1.f;
			}
			// <translation origin="string" x="integer" y="integer" />
			else if (tag_name == "translation")
			{
				const char *attribute_text = element->Attribute("origin");
				m_OffsetOrigin = readOrigin(attribute_text, origin_top_left);

				attribute_text = element->Attribute("x");
				if (attribute_text != nullptr)
					m_OffsetX = CL_StringHelp::local8_to_int(attribute_text);
				else m_OffsetX = 0;

				attribute_text = element->Attribute("y");
				if (attribute_text != nullptr)
					m_OffsetY = CL_StringHelp::local8_to_int(attribute_text);
				else m_OffsetY = 0;

			}
			// <rotation origin="string" x="integer" y="integer" />
			else if (tag_name == "rotation")
			{
				const char *attribute_text = element->Attribute("base_angle");
				if (attribute_text != nullptr)
					m_BaseAngle = CL_Angle(CL_StringHelp::local8_to_float(attribute_text), cl_degrees);
				
				attribute_text = element->Attribute("origin");
				m_RotationOrigin = readOrigin(attribute_text, origin_center);

				attribute_text = element->Attribute("x");
				if (attribute_text != nullptr)
					m_OffsetX = CL_StringHelp::local8_to_int(attribute_text);
				else m_OffsetX = 0;

				attribute_text = element->Attribute("y");
				if (attribute_text != nullptr)
					m_RotationPointX = CL_StringHelp::local8_to_int(attribute_text);
				else m_RotationPointY = 0;
			}
			// <frame nr="integer" speed="integer" x="integer" y="integer" />
			else if (tag_name == "frame")
			{
				SpriteFrame frame;
				frame.number = 0;

				const char *attribute_text = element->Attribute("number");
				if (attribute_text != nullptr)
					frame.number = CL_StringHelp::local8_to_int(attribute_text);

				attribute_text = element->Attribute("speed");
				frame.delay = 60;
				if (attribute_text != nullptr)
					frame.delay = CL_StringHelp::local8_to_int(attribute_text);

				attribute_text = element->Attribute("x");
				frame.offset.x = 0;
				if (attribute_text != nullptr)
					frame.offset.x = CL_StringHelp::local8_to_int(attribute_text);

				attribute_text = element->Attribute("y");
				frame.offset.y = 0;
				if (attribute_text != nullptr)
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
					throw CL_Exception("add_gridclipped_frames: Outside texture bounds");

				impl->frames.push_back(CL_SpriteDescriptionFrame(texture, CL_Rect(xstart, ystart, xstart + width, ystart + height)));
				xstart += width + xspace;
			}
			ystart += height + yspace;
		}
	}*/

}
