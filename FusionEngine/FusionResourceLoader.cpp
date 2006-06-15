
#include "FusionEngineCommon.h"

/// Boost
#include <boost/crc.hpp>

/// Fusion
#include "FusionShipResource.h"
#include "FusionDestructableImage.h"

/// Class
#include "FusionResourceLoader.h"

using namespace FusionEngine;

/// Public:

std::vector<std::string> ResourceLoader::GetInstalledShips()
{
	std::vector<std::string> list;

	CL_DirectoryScanner scanner;
	if (scanner.scan("Ships/", "*.zip"))
	{
		while (scanner.next())
		{
			list.push_back(scanner.get_name());
		}
	}

	return list;
}

std::vector<std::string> ResourceLoader::GetInstalledLevels()
{
	std::vector<std::string> list;

	CL_DirectoryScanner scanner;
	if (scanner.scan("Levels/", "*.zip"))
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
	if (scanner.scan("Weapons/", "*.zip"))
	{
		while (scanner.next())
		{
			list.push_back(scanner.get_name());
		}
	}

	return list;
}

c

// Returns false if any ships aren't found.
bool ResourceLoader::LoadShips(std::vector<std::string> names)
{
    std::map<std::string, ShipResource*> ships;

    std::vector<std::string>::iterator it;
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

std::map<std::string, ShipResource*> ResourceLoader::GetLoadedShips()
{
	return m_ShipResources;
}

/// Private:

ShipResource* ResourceLoader::parseShipDefinition(const std::string &filename)
{
	// The return object.
	ShipResource *res = NULL;

	// Open the archive and list its contents
	CL_Zip_Archive arc(filename);
	std::vector<CL_Zip_FileEntry> arcFiles = arc.get_file_list();

	// Load the xml definition file from the package
	CL_DomDocument doc;
	doc.load(arc.open_source(filename), true, true);

	if (!verifyShipDocument(doc))
		return NULL;

	// Build a resource list
	std::map<std::string, CL_Surface> resourceList parseResources(cElement);

	// Get the root element (document level element)
	CL_DomElement root = doc.get_document_element();

	// Get the ship tag-line and description
	std::string tag(root.get_attribute("tag", "x"));
	std::string desc(root.get_attribute("desc", "No description available"));

	// Walk through each node to gather remaining information
	CL_DomNode cNode = root.get_first_child();
	CL_DomElement cElement = NULL;

	while (cNode != NULL)
	{
		if ((cElement = cNode.to_element()) != NULL)
		{
			std::string image(cElement.get_attribute("image"));

			switch (cElement.get_tag_name())
			{
			case "body":
				res->images.Body =
					FusionDestructableImage(resourceList[image]);
				break;

			case "leftEngine":
				CL_Point point = getPoint(cElement);

				res->positions.SecondaryWeapon point;
				res->images.LeftEngine =
					new CL_Surface(resourceList[image]);
				break;

			case "rightEngine":
				CL_Point point = getPoint(cElement);

				res->positions.SecondaryWeapon point;
				res->images.RightEngine =
					new CL_Surface(resourceList[image]);
				break;

			case "primaryWeapon":
				CL_Point point = getPoint(cElement);

				res->positions.SecondaryWeapon point;
				res->images.PrimaryWeapon =
					new CL_Surface(resourceList[image]);
				break;

			case "secondaryWeapon":
				CL_Point point = getPoint(cElement);

				res->positions.SecondaryWeapon point;
				res->images.SecondaryWeapon =
					new CL_Surface(resourceList[image]);
				break;
			}
		}
	} do (current = cNode.get_next_sibling());

	return res;
}

bool ResourceLoader::verifyShipDocument(const CL_DomDocument *document)
{
	CL_DomElement root = doc.get_document_element();

	// Make sure the xml is a ship definition!
	if (root.get_tag_name != "FusionShip")
		return false;

	CL_DomNode cNode = root.get_first_child();
	CL_DomElement cElement = NULL;

	while (cNode != NULL)
	{
		if (cNode.is_element())
		{
			switch (cNode.get_node_name())
			{
			case "resources":
				if (!(cNode.has_attribute("name") & image.has_attribute("file")))
					return false;
				break;

			case "body":
			case "leftEngine":
			case "rightEngine":
			case "primaryWeapon":
			case "secondaryWeapon":
				if (!cElement.has_attribute("image"))
					return false;
				break;
			} // switch
		} // cNode.is_element
	} do (current = cNode.get_next_sibling());
}

std::map<std::string, CL_Surface> parseResources(const CL_DomDocument *document)
{
	std::map<std::string, CL_Surface> list;

	CL_DomNodeList resourceNodes =
		document->get_document_element().get_elements_by_tag_name("resources");

	for (int i=0; i<resourceNodes.get_length(); i++)
	{
		CL_DomNodeList images = element.get_elements_by_tag_name("image");

		for (int i=0; i<images.get_length(); i++)
		{
			CL_DomElement image = images.item(i).to_element();

			std::string name(image.get_attribute("name"));
			std::string file(image.get_attribute("file"));

			if (list.find("name") != list.end())
				return NULL;

			if (checkInList(file, availableFiles))
				list[name] = CL_Surface(arc.open_source(file));
		}
	}

	return list;
}

CL_Point ResourceLoader::getPoint(const CL_DomElement *element);
{
	if (!(element.has_attribute("x") & element.has_attribute("y")))
		return CL_Point(0, 0);

	int x = CL_String::to_int(element.get_attribute("x"));
	int y = CL_String::to_int(element.get_attribute("y"));

	return CL_Point(x, y);
}

bool checkInList(const std::string &filename, std::vector<std::string> filelist)
{
	std::vector<std::string>::iterator it;
	for (it = filelist.begin(); it != filelist.end(); ++it)
	{
		if (filename == it)
			return true;
	}

	return false;
}
