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

#include "FusionCLStream.h"
#include "FusionExceptionFactory.h"
#include "FusionPhysFS.h"
#include "FusionResourceLoaderUtils.h"
#include "FusionXML.h"

#include <yaml-cpp/yaml.h>

namespace FusionEngine
{

	struct FrameInfo
	{
		clan::Rect frameRect;
		double delay;
		Vector2 offset;
	};

}

namespace YAML
{
   template<>
   struct convert<FusionEngine::Vector2>
	 {
      static Node encode(const FusionEngine::Vector2& value)
			{
         Node node;
         node.push_back(value.x);
         node.push_back(value.y);
         return node;
      }

      static bool decode(const Node& node, FusionEngine::Vector2& value)
			{
         if(!node.IsSequence())
            return false;
         if(node.size() != 2)
            return false;

         value.x = node[0].as<float>();
         value.y = node[1].as<float>();
         return true;
      }
   };

	 template<>
   struct convert<clan::Rect>
	 {
      static Node encode(const clan::Rect& value)
			{
         Node node;
         node.push_back(value.left);
         node.push_back(value.top);
				 node.push_back(value.right);
         node.push_back(value.bottom);
         return node;
      }

      static bool decode(const Node& node, clan::Rect& value)
			{
         if(!node.IsSequence())
            return false;
         if(node.size() != 4)
            return false;

         value.left = node[0].as<int>();
         value.top = node[1].as<int>();
				 value.right = node[1].as<int>();
				 value.bottom = node[1].as<int>();
         return true;
      }
   };

	 template<>
   struct convert<FusionEngine::FrameInfo>
	 {
      static Node encode(const FusionEngine::FrameInfo& value)
			{
         Node node;
         node.push_back(value.frameRect);
         node.push_back(value.delay);
				 node.push_back(value.offset);
         return node;
      }

      static bool decode(const Node& node, FusionEngine::FrameInfo& value)
			{
         if(!node.IsMap())
            return false;

         value.frameRect = node[std::string("cell")].as<clan::Rect>();
         value.delay = node[std::string("delay")].as<double>();
				 value.offset = node[std::string("offset")].as<FusionEngine::Vector2>();
         return true;
      }
   };

	 template<>
   struct convert<clan::Sprite::ShowOnFinish>
	 {
      static Node encode(const clan::Sprite::ShowOnFinish& value)
			{
				Node node;
				switch (value)
				{
				case clan::Sprite::show_blank:
					node = "blank";
					break;
				case clan::Sprite::show_last_frame:
					node = "last";
					break;
				case clan::Sprite::show_first_frame:
					node = "first";
					break;
				default:
					node = "last";
					break;
				}
         node.push_back(value);
         return node;
      }

      static bool decode(const Node& node, clan::Sprite::ShowOnFinish& value)
			{
         if(!node.IsScalar())
            return false;

				 auto string = node.as<std::string>();
				 if (string == "none" || string == "blank")
					 value = clan::Sprite::show_blank;
				 else if (string == "last")
					 value = clan::Sprite::show_last_frame;
				 else if (string == "first")
					 value = clan::Sprite::show_first_frame;

         return true;
      }
   };

}

namespace FusionEngine
{

	SpriteAnimation::SpriteAnimation()
		: m_DefaultDelay(0.0f)
	{
	}

	SpriteAnimation::SpriteAnimation(clan::IODevice dev)
		: m_DefaultDelay(0.0f)
	{
		Load(dev);
	}

	//void operator >> (const YAML::Node& node, clan::Rect& rect)
	//{
	//	node[0] >> rect.left;
	//	node[1] >> rect.top;
	//	node[2] >> rect.right;
	//	node[3] >> rect.bottom;
	//}

	//void operator >> (const YAML::Node& node, Vector2& vec)
	//{
	//	node[0] >> vec.x;
	//	node[1] >> vec.y;
	//}

	//void operator >> (const YAML::Node& node, FrameInfo& frame_info)
	//{
	//	node["cell"] >> frame_info.frameRect;
	//	node["delay"] >> frame_info.delay;
	//	node["offset"] >> frame_info.offset;
	//}

	//void operator >> (const YAML::Node& node, clan::Sprite::ShowOnFinish& on_finish)
	//{
	//	if (node == "none" || node == "blank")
	//		on_finish = clan::Sprite::show_blank;
	//	else if (node == "last")
	//		on_finish = clan::Sprite::show_last_frame;
	//	else if (node == "first")
	//		on_finish = clan::Sprite::show_first_frame;
	//}

	// Just pretend this is implemented w/ varadic template :P
	YAML::Node FindFirstOf(const YAML::Node& parent, const std::string& k0, const std::string& k1, const std::string& k2 = std::string(), const std::string& k3 = std::string(), const std::string& k4 = std::string())
	{
		if (parent.IsMap())
		{
			if (auto node = parent[k0])
				return node;
			else if (auto node = parent[k1])
				return node;
			else if (auto node = parent[k2])
				return node;
			else if (auto node = parent[k3])
				return node;
			else if (auto node = parent[k4])
				return node;
		}

		return YAML::Node();
	}

	void SpriteAnimation::Load(clan::IODevice dev, const std::string& animation_name)
	{
		try
		{
			IO::CLStream stream(dev);
			YAML::Node doc = YAML::Load(stream);
			if (doc)
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
						if (auto node = (*it)[std::string("name")])
						{
							std::string name;
							name = node.as<std::string>();
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


				if (auto node = FindFirstOf(*currentAnimationNode, "default_delay", "default_frame_time", "default_duration"))
					m_DefaultDelay = node.as<double>();
				else if (auto node = (*currentAnimationNode)[std::string("framerate")])
				{
					const double framerate = node.as<double>();
					m_DefaultDelay = 1.0 / framerate;
				}

				if (auto node = (*currentAnimationNode)[std::string("showOnFinish")])
				{
					m_ShowOnFinish = node.as<clan::Sprite::ShowOnFinish>();
				}

				if (auto node = (*currentAnimationNode)[std::string("loop")])
				{
					m_PlayLoop = node.as<bool>();
				}

				if (auto node = (*currentAnimationNode)[std::string("pingpong")])
				{
					m_PlayPingPong = node.as<bool>();
				}

				auto& framesNode = (*currentAnimationNode)[std::string("frames")];
				FSN_ASSERT(framesNode.IsSequence());
				m_Frames.resize(framesNode.size());
				for (unsigned i = 0; i < m_Frames.size(); ++i)
				{
					auto& frameNode = framesNode[i];

					if (frameNode.IsSequence())
					{
						m_Frames[i] = frameNode.as<clan::Rect>();
					}
					else
					{
						if (auto node = FindFirstOf(frameNode, "rect", "frame"))
						{
							m_Frames[i] = node.as<clan::Rect>();
						}

						if (auto node = FindFirstOf(frameNode, "delay", "duration", "frame_time"))
						{
							const double delay = node.as<double>();
							m_FrameDelays.push_back(std::make_pair(i, delay));
						}

						if (auto node = frameNode[std::string("offset")])
						{
							// TODO: ? make this Vector2i
							const Vector2 offset = node.as<Vector2>();
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

	void LoadAnimationResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		FSN_ASSERT(!resource->IsLoaded());

		resource->setLoaded(false);

		SpriteAnimation *data = new SpriteAnimation();
		try
		{
			const auto pathEnd = resource->GetPath().find(":");
			const auto path = resource->GetPath().substr(0, pathEnd);
			const auto animationName = pathEnd != std::string::npos ? resource->GetPath().substr(pathEnd + 1) : "";
			auto dev = vdir.open_file(path, clan::File::open_existing, clan::File::access_read);
			data->Load(dev, animationName);

			resource->SetMetadata(CreateFileMetadata(path, IO::CLStream(dev)));
		}
		catch (clan::Exception& ex)
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

	void UnloadAnimationResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<SpriteAnimation*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadSpriteDefinitionResourcePrereqs(ResourceContainer* res, DepsList& dependencies, boost::any user_data)
	{
		const auto& path = res->GetPath();
		const auto animationPathOffset = path.find('|');
		if (animationPathOffset != std::string::npos)
		{
			dependencies.push_back(std::make_pair("TEXTURE", path.substr(0, animationPathOffset)));
			dependencies.push_back(std::make_pair("ANIMATION", path.substr(animationPathOffset + 1)));
		}
		else
			dependencies.push_back(std::make_pair("TEXTURE", path));
	}

	void LoadSpriteDefinitionResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			delete static_cast<SpriteDefinition*>(resource->GetDataPtr());
		}

		ResourcePointer<clan::Texture2D> texture;
		ResourcePointer<SpriteAnimation> animation;

		const auto& deps = resource->GetDependencies();
		for (auto it = deps.begin(); it != deps.end(); ++it)
		{
			const auto& dep = *it;
			if (dep->GetType() == "TEXTURE")
				texture.SetTarget(dep);
			else if (dep->GetType() == "ANIMATION")
				animation.SetTarget(dep);
		}

		SpriteDefinition* def = new SpriteDefinition(texture, animation);
		resource->SetDataPtr(def);
		resource->setLoaded(true);
	}

	void UnloadSpriteDefinitionResource(ResourceContainer* resource, clan::VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<SpriteDefinition*>(resource->GetDataPtr());
		}

		resource->SetDataPtr(nullptr);
	}

	SpriteDefinition::SpriteDefinition(const ResourcePointer<clan::Texture2D>& texture, const ResourcePointer<SpriteAnimation>& animation)
		: m_Texture(texture),
		m_Animation(animation)
	{
		GenerateDescription();
	}

	SpriteDefinition::~SpriteDefinition()
	{
		if (m_UnusedCallback)
			m_UnusedCallback();
	}

	void SpriteDefinition::GenerateDescription()
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

	clan::Sprite SpriteDefinition::CreateSprite(clan::GraphicContext &gc)
	{
		clan::Sprite sprite(gc, m_Description);
		if (m_Animation.IsLoaded())
		{
			sprite.set_delay((int)(m_Animation->GetDefaultDelay() * 1000 + 0.5));
			auto& frameOffsets = m_Animation->GetFrameOffsets();
			for (auto it = frameOffsets.begin(), end = frameOffsets.end(); it != end; ++it)
			{
				sprite.set_frame_offset(it->first, clan::Point(it->second.x, it->second.y));
			}
			sprite.set_play_loop(m_Animation->GetPlayLoop());
			sprite.set_play_pingpong(m_Animation->GetPlayPingPong());
			sprite.set_show_on_finish(m_Animation->GetShowOnFinish());
		}
		return sprite;
	}

	void LoadSpriteDefinition(LegacySpriteDefinition &def, const std::string &filepath, clan::VirtualDirectory vdir)
	{
		ticpp::Document document = OpenXml(filepath, vdir);
		std::string workingDirectory = vdir.get_path() + clan::PathHelp::get_basepath(filepath.c_str(), clan::PathHelp::path_type_virtual);

		if (workingDirectory[workingDirectory.length()-1] == '/')
			workingDirectory.erase(workingDirectory.length()-1);
		
		def.LoadXml(clan::StringHelp::text_to_local8( workingDirectory ), document, vdir);
	}

	LegacySpriteDefinition::LegacySpriteDefinition()
		: m_Users(0),
		m_ScaleX(1.f), m_ScaleY(1.f),
		m_BaseAngle(0.f, clan::angle_radians),
		m_OffsetOrigin(clan::origin_top_left),
		m_OffsetX(0), m_OffsetY(0),
		m_RotationOrigin(clan::origin_center),
		m_RotationPointX(0), m_RotationPointY(0),
		m_Colour(clan::Color::white),
		m_Alpha(1.0f)
	{
	}

	LegacySpriteDefinition::~LegacySpriteDefinition()
	{
		ClearImageData();
	}

	void LegacySpriteDefinition::LoadXml(const std::string &working_directory, const ticpp::Document& document, clan::VirtualDirectory &dir)
	{
		m_WorkingDirectory = working_directory;

		ticpp::Element *root = document.FirstChildElement();
		if (root == nullptr)
			FSN_EXCEPT(FileTypeException, "Not a valid XML sprite definition.");

		if (root->Value() != "sprite")
			FSN_EXCEPT(FileTypeException, "Not a sprite definition.");

		// Load id
		//m_ID = clan::StringHelp::text_to_int(attribute_text);

		loadImageElements(root);
		loadMoreOptions(root);

		for (ImageArray::iterator it = m_Images.begin(), end = m_Images.end(); it != end; ++it)
		{
			// Load image data if necessary
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
			if (it->image_data.is_null())
				it->image_data = clan::ImageProviderFactory::load(it->filename, dir);
#else
			clan::PixelBuffer image_data = clan::ImageProviderFactory::load(it->filename, dir);
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

	clan::Sprite LegacySpriteDefinition::CreateSprite(clan::GraphicContext &gc)
	{
		clan::Sprite sprite(gc, m_Description);

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

	void LegacySpriteDefinition::SpriteReleased()
	{
#ifdef FSN_SPRITEDEF_STOREIMAGEDATA
		if (--m_Users == 0)
			ClearImageData();
#endif
	}

	void LegacySpriteDefinition::ClearImageData()
	{
		// Drop all refs to frame pixelbuffers
		for (ImageArray::iterator it = m_Images.begin(), end = m_Images.end(); it != end; ++it)
		{
			it->image_data = clan::PixelBuffer();
		}
	}

	void LegacySpriteDefinition::addImage(const LegacySpriteDefinition::Image &image)
	{
		m_ImageFiles.insert(image.filename);
		m_Images.push_back(image);
	}

	void LegacySpriteDefinition::addImage(const std::string &filename)
	{
		std::string expandedPath = SetupPhysFS::parse_path(m_WorkingDirectory, filename);

		Image image;
		image.type = Image::FrameFile;
		image.filename = expandedPath.c_str();

		m_Images.push_back(image);
		m_ImageFiles.insert(expandedPath);
	}

	void LegacySpriteDefinition::addImage(const std::string &filename, int xpos, int ypos, int width, int height, int xarray, int yarray, int array_skipframes, int xspacing, int yspacing)
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

	void LegacySpriteDefinition::addImage(const std::string &filename, int xpos, int ypos, float trans_limit, bool free)
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

	bool LegacySpriteDefinition::exists(const std::string &filename)
	{
		std::string ex = SetupPhysFS::parse_path(m_WorkingDirectory, filename);
		return PHYSFS_exists(ex.c_str()) != 0;
	}

	void LegacySpriteDefinition::loadImageElements(ticpp::Element *root)
	{
		ticpp::Element *element = root->FirstChildElement();
		while (element != nullptr)
		{
			std::string tag_name = element->Value();
			if (tag_name == "image" || tag_name == "image-file")
			{
				if (element->HasAttribute("filesequence"))
				{
					int start_index = 0;
					std::string attribute_text = element->GetAttribute("start_index");
					if (!attribute_text.empty())
						start_index = clan::StringHelp::local8_to_int(attribute_text);

					int skip_index = 1;
					attribute_text = element->GetAttribute("skip_index");
					if (!attribute_text.empty())
						skip_index = clan::StringHelp::text_to_int(attribute_text);

					int leading_zeroes = 0;
					attribute_text = element->GetAttribute("leading_zeroes");
					if (!attribute_text.empty())
						leading_zeroes =  clan::StringHelp::text_to_int(attribute_text);

					std::string prefix = element->GetAttribute("filesequence");
					std::string suffix = std::string(".") +
						clan::StringHelp::text_to_local8( clan::PathHelp::get_extension(std::string(prefix.c_str()), clan::PathHelp::path_type_virtual) ).c_str();
					prefix.erase(prefix.length() - suffix.length(), prefix.length()); //remove the extension

					for (int i = start_index;; i += skip_index)
					{
						std::string file_name = prefix;

						std::string  frame_text = clan::StringHelp::int_to_local8(i);
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
					std::string attribute_text = element->GetAttribute("file");
					if (attribute_text.empty())
						continue;

					std::string image_name(attribute_text);
					
					if (!exists(image_name))
					{
						element = element->NextSiblingElement();
						continue;
					}

					ticpp::Element *cur_child = element->FirstChildElement();
					if(cur_child == nullptr) 
					{
						addImage(image_name);
					}
					else 
					{
						do {
							if(cur_child->Value() == "grid")
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

								std::string attribute_text = cur_child->GetAttribute("size");
								std::vector<std::string> image_size = clan::StringHelp::split_text(std::string(attribute_text), ",");
								if (image_size.size() > 0)
									width = clan::StringHelp::text_to_int(image_size[0]);
								if (image_size.size() > 1)
									height = clan::StringHelp::text_to_int(image_size[1]);

								attribute_text = cur_child->GetAttribute("pos");
								if (!attribute_text.empty())
								{
									std::vector<std::string> image_pos = clan::StringHelp::split_text(std::string(attribute_text), ",");
									if (image_pos.size() > 0)
										xpos = clan::StringHelp::text_to_int(image_pos[0]);
									if (image_pos.size() > 1)
										ypos = clan::StringHelp::text_to_int(image_pos[1]);
								}

								attribute_text = cur_child->GetAttribute("array");
								if (!attribute_text.empty())
								{
									std::vector<std::string> image_array = clan::StringHelp::split_text(std::string(attribute_text), ",");
									if (image_array.size() == 2)
									{
										xarray = clan::StringHelp::text_to_int(image_array[0]);
										yarray = clan::StringHelp::text_to_int(image_array[1]);
									}
									else
									{
										FSN_EXCEPT(FileTypeException,
											"Sprite using image '" + image_name + "' has incorrect array attribute, must be \"X,Y\"!");
									}
								}

								attribute_text = cur_child->GetAttribute("array_skipframes");
								if (!attribute_text.empty())
								{
									array_skipframes = clan::StringHelp::text_to_int( std::string(attribute_text) );
								}

								attribute_text = cur_child->GetAttribute("spacing");
								if (!attribute_text.empty())
								{
									std::vector<std::string> image_spacing = clan::StringHelp::split_text(std::string(attribute_text), ",");
									xspacing = clan::StringHelp::text_to_int(image_spacing[0]);
									yspacing = clan::StringHelp::text_to_int(image_spacing[1]);
								}

								addImage(image_name, xpos, ypos, width, height, xarray, yarray, array_skipframes, xspacing, yspacing);
							}
							else if(cur_child->Value() == "alpha")
							{
								int xpos = 0;
								int ypos = 0;
								float trans_limit = 0.05f;

								std::string attribute_text = cur_child->GetAttribute("pos");
								if (!attribute_text.empty())
								{
									std::vector<std::string> image_pos = clan::StringHelp::split_text(std::string(attribute_text), ",");
									xpos = clan::StringHelp::text_to_int(image_pos[0]);
									ypos = clan::StringHelp::text_to_int(image_pos[1]);
								}

								attribute_text = cur_child->GetAttribute("trans_limit");
								if (!attribute_text.empty())
								{
									trans_limit = clan::StringHelp::text_to_float(std::string(attribute_text));
								}

								if (cur_child->HasAttribute("free"))
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

	clan::Origin readOrigin(std::string attribute_text, clan::Origin default_)
	{
		if (!attribute_text.empty())
		{
			std::string originAttribute = std::string(attribute_text);
			if(originAttribute == "center")
				return clan::origin_center;
			else if(originAttribute == "top_left")
				return clan::origin_top_left;
			else if(originAttribute == "top_center")
				return clan::origin_top_center;
			else if(originAttribute == "top_right")
				return clan::origin_top_right;
			else if(originAttribute == "center_left")
				return clan::origin_center_left;
			else if(originAttribute == "center_right")
				return clan::origin_center_right;
			else if(originAttribute == "bottom_left")
				return clan::origin_bottom_left;
			else if(originAttribute == "bottom_center")
				return clan::origin_bottom_center;
			else if(originAttribute == "bottom_right")
				return clan::origin_bottom_right;
		}

		return default_;
	}

	void LegacySpriteDefinition::loadMoreOptions(ticpp::Element *root)
	{
		// Load play options	
		ticpp::Element *element = root->FirstChildElement();
		while (element != nullptr)
		{
			std::string tag_name = element->Value();

			std::string attribute_text;

			// <color name="string" red="float" green="float" blue="float" alpha="float" />
			if (tag_name == "color")
			{
				attribute_text = element->GetAttribute("name");
				if (!attribute_text.empty())
					m_Colour = clan::Colorf( clan::Color::find_color(attribute_text) );

				attribute_text = element->GetAttribute("red");
				if (!attribute_text.empty())
					m_Colour.r = (float)clan::StringHelp::text_to_float(std::string(attribute_text));

				attribute_text = element->GetAttribute("green");
				if (!attribute_text.empty())
					m_Colour.g = (float)clan::StringHelp::text_to_float(std::string(attribute_text));

				attribute_text = element->GetAttribute("blue");
				if (!attribute_text.empty())
					m_Colour.b = (float)clan::StringHelp::text_to_float(std::string(attribute_text));
				
				attribute_text = element->GetAttribute("alpha");
				if (!attribute_text.empty())
					m_Colour.a = (float)clan::StringHelp::text_to_float(std::string(attribute_text));
			}
			// <animation speed="integer" loop="[yes,no]" pingpong="[yes,no]" direction="[backward,forward]" on_finish="[blank,last_frame,first_frame]"/>
			else if (tag_name == "animation")
			{
				attribute_text = element->GetAttribute("speed");
				if (!attribute_text.empty())
					m_Animation.delay = clan::StringHelp::text_to_int(std::string(attribute_text));
				else
					m_Animation.delay = 60;

				attribute_text = element->GetAttribute("loop");
				if (!attribute_text.empty())
					m_Animation.loop = clan::StringHelp::local8_to_bool(attribute_text);
				else
					m_Animation.loop = true;

				attribute_text = element->GetAttribute("pingpong");
				if (!attribute_text.empty())
					m_Animation.pingpong = clan::StringHelp::local8_to_bool(attribute_text);
				else
					m_Animation.pingpong = false;

				attribute_text = element->GetAttribute("direction");
				if (!attribute_text.empty())
					m_Animation.backward = std::string(attribute_text) == "backward";
				else
					m_Animation.backward = false; // forward

				attribute_text = element->GetAttribute("on_finish");
				bool on_finish_Valid = false;
				if (!attribute_text.empty())
				{
					on_finish_Valid = true;
					std::string on_finish(attribute_text);
					if (on_finish == "first_frame")
						m_Animation.showOnFinish = clan::Sprite::show_first_frame;
					else if(on_finish == "last_frame")
						m_Animation.showOnFinish = clan::Sprite::show_last_frame;
					else
						on_finish_Valid = false;
				}
				if (!on_finish_Valid)
					m_Animation.showOnFinish = clan::Sprite::show_blank;
			}
			// <scale x="float" y="float />
			else if (tag_name == "scale")
			{
				attribute_text = element->GetAttribute("x");
				if (!attribute_text.empty()) m_ScaleX = clan::StringHelp::local8_to_float(attribute_text);
				else m_ScaleX = 1.f;

				attribute_text = element->GetAttribute("y");
				if (!attribute_text.empty()) m_ScaleY = clan::StringHelp::local8_to_float(attribute_text);
				else m_ScaleY = 1.f;
			}
			// <translation origin="string" x="integer" y="integer" />
			else if (tag_name == "translation")
			{
				std::string attribute_text = element->GetAttribute("origin");
				m_OffsetOrigin = readOrigin(attribute_text, clan::origin_top_left);

				attribute_text = element->GetAttribute("x");
				if (!attribute_text.empty())
					m_OffsetX = clan::StringHelp::local8_to_int(attribute_text);
				else m_OffsetX = 0;

				attribute_text = element->GetAttribute("y");
				if (!attribute_text.empty())
					m_OffsetY = clan::StringHelp::local8_to_int(attribute_text);
				else m_OffsetY = 0;

			}
			// <rotation origin="string" x="integer" y="integer" />
			else if (tag_name == "rotation")
			{
				std::string attribute_text = element->GetAttribute("base_angle");
				if (!attribute_text.empty())
					m_BaseAngle = clan::Angle(clan::StringHelp::local8_to_float(attribute_text), clan::angle_degrees);
				
				attribute_text = element->GetAttribute("origin");
				m_RotationOrigin = readOrigin(attribute_text, clan::origin_center);

				attribute_text = element->GetAttribute("x");
				if (!attribute_text.empty())
					m_OffsetX = clan::StringHelp::local8_to_int(attribute_text);
				else m_OffsetX = 0;

				attribute_text = element->GetAttribute("y");
				if (!attribute_text.empty())
					m_RotationPointX = clan::StringHelp::local8_to_int(attribute_text);
				else m_RotationPointY = 0;
			}
			// <frame nr="integer" speed="integer" x="integer" y="integer" />
			else if (tag_name == "frame")
			{
				SpriteFrame frame;
				frame.number = 0;

				std::string attribute_text = element->GetAttribute("number");
				if (!attribute_text.empty())
					frame.number = clan::StringHelp::local8_to_int(attribute_text);

				attribute_text = element->GetAttribute("speed");
				frame.delay = 60;
				if (!attribute_text.empty())
					frame.delay = clan::StringHelp::local8_to_int(attribute_text);

				attribute_text = element->GetAttribute("x");
				frame.offset.x = 0;
				if (!attribute_text.empty())
					frame.offset.x = clan::StringHelp::local8_to_int(attribute_text);

				attribute_text = element->GetAttribute("y");
				frame.offset.y = 0;
				if (!attribute_text.empty())
					frame.offset.y = clan::StringHelp::local8_to_int(attribute_text);

				m_Frames.push_back(frame);
			}

			element = element->NextSiblingElement();
		}
	}

	/*void SpriteDefinition::addGridclippedFrames(
		const clan::Texture &texture, 
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
					throw clan::Exception("add_gridclipped_frames: Outside texture bounds");

				impl->frames.push_back(clan::SpriteDescriptionFrame(texture, clan::Rect(xstart, ystart, xstart + width, ystart + height)));
				xstart += width + xspace;
			}
			ystart += height + yspace;
		}
	}*/

}
