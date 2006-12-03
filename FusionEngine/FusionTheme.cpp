
/// Class
#include "FusionTheme.h"

/// Fusion
#include "FusionPaths.h"

namespace FusionEngine
{

	///////////
	/// Public:
	bool Theme::Load(const std::string &name)
	{
		// Get the full path to the given theme
		std::string full_path = FusionEngine::ThemePath + name;

		// Initialise ClanLib XML stuff
		CL_DomDocument doc;
		doc.load(new CL_InputSource_File(full_path), true, true);

		CL_DomElement root_node = doc.get_document_element();

		// Get a list of nodes with the tag "image"
		CL_DomNodeList image_nodes = root_node.get_elements_by_tag_name("image");

		// Walk through each node to gather information
		for (int i=0; i < image_nodes.get_length(); i++)
		{
			CL_DomElement element = image_nodes.item(i).to_element();

			std::string name(element.get_attribute("name"));
			std::string file(element.get_attribute("file"));

			m_Images[name] = new CL_Surface(file);
		}


		// Get a list of nodes with the tag "sound"
		CL_DomNodeList sound_nodes = root_node.get_elements_by_tag_name("sound");

		// Walk through each node to gather information
		for (int i=0; i < sound_nodes.get_length(); i++)
		{
			CL_DomElement element = sound_nodes.item(i).to_element();

			std::string name(element.get_attribute("name"));
			std::string file(element.get_attribute("file"));

			m_Sounds[name] = new CL_SoundBuffer(file);
		}

		return true;
	}

	void Theme::Unload()
	{
		// Delete resources
		//  Images
		{
			SurfaceMap::iterator it = m_Images.begin();
			for (; it != m_Images.end(); ++it)
			{
				delete it->second;
			}
		}

		m_Images.clear();

		//  Sounds
		{
			SoundBufferMap::iterator it = m_Sounds.begin();
			for (; it != m_Sounds.end(); ++it)
			{
				delete it->second;
			}
		}

		m_Sounds.clear();
	}

	CL_Surface *Theme::GetImage(const std::string &tag)
	{
		SurfaceMap::iterator it = m_Images.find(tag);
		// Don't try to get non-existant items
		if (it == m_Images.end())
			return 0;

		return it->second;
	}

	CL_SoundBuffer *Theme::GetSound(const std::string &tag)
	{
		SoundBufferMap::iterator it = m_Sounds.find(tag);
		// Don't try to get non-existant items
		if (it == m_Sounds.end())
			return 0;

		return it->second;
	}

}