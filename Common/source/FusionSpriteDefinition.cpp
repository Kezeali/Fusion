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

	void LoadAnimationResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		FSN_ASSERT(!resource->IsLoaded());

		resource->setLoaded(false);

		SpriteAnimation *data = new SpriteAnimation();
		try
		{
			const auto pathEnd = resource->GetPath().find(":");
			const auto path = resource->GetPath().substr(0, pathEnd);
			const auto animationName = pathEnd != std::string::npos ? resource->GetPath().substr(pathEnd + 1) : "";
			auto dev = fs.open_file(path, clan::File::open_existing, clan::File::access_read);
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

	void UnloadAnimationResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
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

	void LoadSpriteDefinitionResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
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

	void UnloadSpriteDefinitionResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
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
	}

	clan::Sprite SpriteDefinition::CreateSprite(clan::Canvas &gc)
	{
		clan::Sprite sprite(gc);
		if (m_Texture.IsLoaded() && !m_Texture.Get()->is_null())
		{
			if (!m_Animation.IsLoaded())
			{
 				sprite.add_frame(*m_Texture.Get());
			}
			else
			{
				auto& frames = m_Animation->GetFrames();
				sprite.add_frames(*m_Texture.Get(), frames.data(), frames.size());

				auto& frameDelays = m_Animation->GetFrameDelays();
				for (auto it = frameDelays.begin(), end = frameDelays.end(); it != end; ++it)
				{
					sprite.set_frame_delay(it->first, it->second);
				}
			}
		}
		else
		{
			FSN_EXCEPT(InvalidArgumentException, "Tried to generate a sprite using an unloaded texture");
		}
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

}
