
/// Class
#include "FusionResourceLoader.h"

/// Boost [depreciated]
//#include <boost/crc.hpp>

/// Fusion
#include "FusionShipResource.h"

#include "FusionPaths.h"

using namespace FusionEngine;

// Look, nice formatting :D
///////////
/// Public:
StringVector ResourceLoader::GetInstalledShips()
{
	StringVector list;

	CL_DirectoryScanner scanner;
	if (scanner.scan(ShipsPath, "*.zip"))
	{
		while (scanner.next())
		{
			list.push_back(scanner.get_name());
		}
	}

	return list;
}

StringVector ResourceLoader::GetInstalledLevels()
{
	StringVector list;

	CL_DirectoryScanner scanner;
	if (scanner.scan(LevelsPath, "*.zip"))
	{
		while (scanner.next())
		{
			list.push_back(scanner.get_name());
		}
	}

	return list;
}

StringVector ResourceLoader::GetInstalledWeapons()
{
	StringVector list;

	CL_DirectoryScanner scanner;
	if (scanner.scan(WeaponsPath, "*.zip"))
	{
		while (scanner.next())
		{
			list.push_back(scanner.get_name());
		}
	}

	return list;
}

// Returns false if any ships aren't found.
bool ResourceLoader::LoadShips(StringVector names)
{
    ShipResourceMap ships;

    StringVector::iterator it;
    for (it = names.begin(); it != names.end(); ++it)
    {
			ShipResource *ship = parseShipDefinition(*it);

			if (ship != NULL)
				ships[*it] = ship;
			else
				return false;
    }

    m_ShipResources = ships;

	return true;
}

ResourceLoader::ShipResourceMap ResourceLoader::GetLoadedShips()
{
	return m_ShipResources;
}

////////////
/// Private:
ShipResource* ResourceLoader::parseShipDefinition(const std::string &filename)
{
	// The return object.
	ShipResource *res = NULL;

	// Open the archive
	CL_Zip_Archive arc(filename);

	// Load the xml definition file from the package
	CL_DomDocument doc;
	doc.load(arc.open_source(filename), true, true);

	if (!verifyShipDocument(&doc))
		return NULL;

	
	// Build a resource list
	PackageResources resourceList = parseResources(&doc, &arc);

	// Get the root element (document level element)
	CL_DomElement root = doc.get_document_element();

	// Get the ship tag-line and description
	std::string tag(root.get_attribute("tag", "x"));
	std::string desc(root.get_attribute("desc", "No description available"));

	// Walk through each node to gather remaining information
	CL_DomNode cNode = root.get_first_child();
	CL_DomElement cElement;

	while (cNode.get_node_type() != CL_DomNode::NULL_NODE)
	{
		cElement = cNode.to_element();
		if (cElement.get_node_type() == CL_DomNode::NULL_NODE)
		{
			std::string image(cElement.get_attribute("image"));

			std::string tag_name = cElement.get_tag_name();
			if (tag_name == "body")
			{
				res->Images.Body = resourceList.Images[image];
			}

			if (tag_name == "leftEngine")
			{
				CL_Point point = getPoint(&cElement);

				res->Positions.LeftEngine = point;
				res->Images.Engine = resourceList.Images[image];
			}

			if (tag_name == "rightEngine")
			{
				CL_Point point = getPoint(&cElement);

				res->Positions.RightEngine = point;
				res->Images.Engine = resourceList.Images[image];
			}

			if (tag_name == "primaryWeapon")
			{
				CL_Point point = getPoint(&cElement);

				res->Positions.PrimaryWeapon = point;
				res->Images.PrimaryWeapon = resourceList.Images[image];
			}

			if (tag_name == "secondaryWeapon")
			{
				CL_Point point = getPoint(&cElement);

				res->Positions.SecondaryWeapon = point;
				res->Images.SecondaryWeapon = resourceList.Images[image];
			}
		}
		// Get the next node
		cNode = cNode.get_next_sibling();
	}

	return res;
}

bool ResourceLoader::verifyShipDocument(CL_DomDocument *document)
{
	CL_DomElement root = document->get_document_element();

	// Make sure the xml is a ship definition!
	if (root.get_tag_name() != "FusionShip")
		return false;

	CL_DomNode cNode = root.get_first_child();
	CL_DomElement cElement;

	while (cNode.get_node_type() != CL_DomNode::NULL_NODE)
	{
		if (cNode.is_element())
		{
			// Convert the node to an element
			cElement = cNode.to_element();

			std::string elem_name = cElement.get_node_name();

			///////////////////
			// Resource Elements
			if (elem_name == "resources")
			{
				// Check if there are any resources listed here
				if (cElement.has_child_nodes())
				{
					CL_DomNodeList cResList = cElement.get_child_nodes();

					for (int i = 0; i < cResList.get_length(); i++)
					{
						CL_DomNode item = cResList.item(i);

						// Escape if the current node isn't an element
						//  (very unlikely... but we wan't to be failsafe :P)
						if (item.is_element() == false)
							continue;
						CL_DomElement cRes = item.to_element();

						std::string res_type = cRes.get_node_name();

						////////////////////////////
						// Image and Sound Resources
						if (res_type == "image" || res_type == "sound")
						{
							if (!(cRes.has_attribute("name") & cRes.has_attribute("file")))
								return false;
						}
					}
				}
			}

			////////////////////////
			// Body graphic settings
			if (elem_name == "body")
			{
				if (!cElement.has_attribute("image"))
					return false;
			}

			/////////////////////////
			// Other graphic settings
			if (elem_name == "leftEngine" || elem_name == "rightEngine"
				|| elem_name == "primaryWeapon" || elem_name == "secondaryWeapon")
			{
				if (!cElement.has_attribute("image"))
					return false;
				if (!cElement.has_attribute("x"))
					return false;
				if (!cElement.has_attribute("y"))
					return false;
			}

			/////////////////////
			// Physical settings
			if (elem_name == "physics")
			{
				if (!cElement.has_attribute("mass"))
					return false;
				if (!cElement.has_attribute("engineforce"))
					return false;
				if (!cElement.has_attribute("rotationvelocity"))
					return false;
			}

			////////////////////
			// Gameplay settings
			if (elem_name == "gameplay")
			{
				if (!cElement.has_attribute("health"))
					return false;
			}
		} // cNode.is_element
		// Get the next node
		cNode = cNode.get_next_sibling();
	}
}

ResourceLoader::PackageResources ResourceLoader::parseResources(CL_DomDocument *document, CL_Zip_Archive *arc)
{
	SurfaceMap sf_list;
	SoundBufferMap snd_list;

	std::vector<CL_Zip_FileEntry> arcFiles = arc->get_file_list();

	CL_DomNodeList resourceNodes =
		document->get_elements_by_tag_name("resources");

	for (int i=0; i<resourceNodes.get_length(); i++)
	{
		CL_DomNode item = resourceNodes.item(i);

		// Escape if the current node isn't an element
		//  (very unlikely... but we wan't to be failsafe :P)
		if (item.is_element() == false)
			continue;
		CL_DomElement cRes = item.to_element();

		// Images
		CL_DomNodeList images = cRes.get_elements_by_tag_name("image");
		for (int i=0; i<images.get_length(); i++)
		{
			CL_DomElement image = images.item(i).to_element();

			std::string name(image.get_attribute("name"));
			std::string file(image.get_attribute("file"));

			// Umm, what exactly was this line supposed to do? Why would sf_list contain
			//  "name" anyway?! I must have been really out-of-it when I worte this i guess :P
			//if (sf_list.find("name") != sf_list.end())
				//return PackageResources();

			// Makes sure the archive contains the listed file, and load it to mem
			if (checkInList(file, arcFiles))
				sf_list[name] = new CL_Surface(CL_ProviderFactory::load(arc->open_source(file)));
		}

		// Sounds
		CL_DomNodeList sounds = cRes.get_elements_by_tag_name("sound");
		for (int i=0; i<sounds.get_length(); i++)
		{
			CL_DomElement sound = sounds.item(i).to_element();

			std::string name(sound.get_attribute("name"));
			std::string file(sound.get_attribute("file"));

			if (snd_list.find("name") != snd_list.end())
				return NULL;

			// same as in Images above
			if (checkInList(file, arcFiles))
				snd_list[name] = new CL_SoundBuffer(CL_ProviderFactory::load(arc->open_source(file)));
		}
	}

	// Compile the collected resources into an easy to digest package
	PackageResources res;
	res.Images = sf_list;
	res.Sounds = snd_list;

	return res;
}

CL_Point ResourceLoader::getPoint(const CL_DomElement *element);
{
	// Return a zero point if the data is incomplete
	if (!(element.has_attribute("x") & element.has_attribute("y")))
		return CL_Point(0, 0);

	int x = CL_String::to_int(element.get_attribute("x"));
	int y = CL_String::to_int(element.get_attribute("y"));

	return CL_Point(x, y);
}

bool ResourceLoader::checkInList(const std::string &filename, std::vector<CL_Zip_FileEntry> filelist)
{
	std::vector<CL_Zip_FileEntry>::iterator it;
	for (it = filelist.begin(); it != filelist.end(); ++it)
	{
		if (filename == (*it).get_filename())
			return true;
	}

	return false;
}
