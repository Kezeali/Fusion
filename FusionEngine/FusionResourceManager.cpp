
/// Class
#include "FusionResourceLoader.h"

/// Fusion
#include "FusionPaths.h"

namespace FusionEngine
{

	// Look, nice formatting :D
	///////////
	/// Public:
	StringVector ResourceManager::GetInstalledShips()
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

	StringVector ResourceManager::GetInstalledLevels()
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

	StringVector ResourceManager::GetInstalledWeapons()
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

	void ResourceManager::DisposeUnusedResources()
	{
#ifdef RESMAN_USEGARBAGELIST
		ResourceGarbageList::iterator it = m_ResourceGarbage.begin();
		for (; it != m_ResourceGarbage.end(); ++it)
		{
			Resource res = m_Resources.find(*it);
			if (res != m_Resources.end())
				m_Resources.erase(res);
		}
		m_ResourceGarbage.clear();
#else
		ResourceMap::iterator it = m
#endif
	}

	void ResourceManager::DeleteResources()
	{
		m_Resources.clear();
	}

	void ResourceManager::ClearAll()
	{
		ResetVerified();

		DeleteResources();
	}

	// Returns false if a ship isn't found.
	bool ResourceManager::LoadShips(StringVector names)
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

		//std::copy(ships.begin(), ships.end(), m_ShipResources.end());
		m_ShipResources = ships;

		return true;
	}

	bool ResourceManager::LoadLevel(const std::string &name)
	{
		LevelResource *level = parseLevelDefinition(name);

		if (level != NULL)
			m_LevelResource = level;
		else
			return false;

		return true;
	}

	bool ResourceManager::LoadWeapons(StringVector names)
	{
		WeaponResourceMap weapons;

		StringVector::iterator it;
		for (it = names.begin(); it != names.end(); ++it)
		{
			WeaponResource *weapon = parseWeaponDefinition(*it);

			if (weapon != NULL)
				weapons[*it] = weapon;
			else
				return false;
		}

		m_WeaponResources = weapons;

		return true;
	}

	bool ResourceManager::LoadLevelVerified(const std::string &name)
	{
		// Make sure the requested level has been verified
		StringVector::iterator it = m_VerifiedLevels.begin();
		for (; it != m_VerifiedLevels.end(); ++it)
		{
			// If the requested level is found in the verified list, try to load it
			if ((*it) == name)
				return LoadLevel(name);
		}

		return false;
	}

	bool ResourceManager::LoadVerified()
	{
		return (
			LoadShips(m_VerifiedShips) &
			LoadWeapons(m_VerifiedWeapons)
			);
	}

	void ResourceManager::ResetVerified()
	{
		m_VerifiedShips.clear();
		m_VerifiedWeapons.clear();
		m_VerifiedLevels.clear();
	}

	ShipResourceMap ResourceManager::GetLoadedShips()
	{
		return m_ShipResources;
	}

	LevelResource *ResourceManager::GetLoadedLevel()
	{
		cl_assert(m_LevelResource != NULL);
		return m_LevelResource;
	}

	WeaponResourceMap ResourceManager::GetLoadedWeapons()
	{
		return m_WeaponResources;
	}

	ShipResource* ResourceManager::GetShipResource(const std::string &name) const
	{
		throw Error(Error::NOT_IMPLEMENTED, "Not Implemented");
		return NULL;
	}

	WeaponResource* ResourceManager::GetWeaponResource(const std::string &name) const
	{
		throw Error(Error::NOT_IMPLEMENTED, "Not Implemented");
		return NULL;
	}

	////////////
	/// Private:
	ShipResource* ResourceManager::parseShipDefinition(CL_DomDocument *doc)
	{
		// The return object.
		ShipResource *res = NULL;

		// Load the xml definition file from the package
		doc.load(new CL_InputSource_File(def_file), true, true);

		if (!verifyShipDocument(&doc))
			return NULL;


		// Build a resource list
		PackageResources resourceList = parseResources(&doc, arc);

		// Get the root element (document level element)
		CL_DomElement root = doc.get_document_element();

		// Get the ship tag-line and description
		std::string tag(root.get_attribute("tag", "x"));
		std::string desc(root.get_attribute("desc", "No description available"));

		// Walk through each node to gather remaining information
		CL_DomNode currentNode = root.get_first_child();
		CL_DomElement currentElement;

		// while there is another node (NULL_NODE is only returned
		//  when we reach the eof)
		while (cNode.get_node_type() != CL_DomNode::NULL_NODE)
		{
			currentElement = cNode.to_element();

			// if the node was successfully converted into an element ...
			if (currentElement.get_node_type() == CL_DomNode::NULL_NODE)
			{
				std::string tag_name = currentElement.get_tag_name();

				if (tag_name == "body")
				{
					res->Images.Body = resourceList.Images[image];
				}

				if (tag_name == "leftEngine")
				{
					res->Positions.LeftEngine = getPoint(&currentElement);
					res->Images.LeftEngine    = getImage(&currentElement);
				}

				if (tag_name == "rightEngine")
				{

					res->Positions.RightEngine = getPoint(&currentElement);
					res->Images.RightEngine    = getImage(&currentElement);
				}

				if (tag_name == "primaryWeapon")
				{
					res->Positions.PrimaryWeapon = getPoint(&currentElement);
					res->Images.PrimaryWeapon    = getImage(&currentElement);
				}

				if (tag_name == "secondaryWeapon")
				{
					res->Positions.SecondaryWeapon = getPoint(&currentElement);
					res->Images.SecondaryWeapon    = getImage(&currentElement);
				}
			} // End if (currentElement.get_node_type() == CL_DomNode::NULL_NODE)

			// Get the next node
			cNode = cNode.get_next_sibling();
		}

		return res;
	}

	bool ResourceManager::verifyShipDocument(CL_DomDocument *document)
	{
		CL_DomElement root = document->get_document_element();

		// Make sure the xml is a ship definition. (even though this
		//  should have been checked for this method to be called.)
		if (root.get_tag_name() != "FusionShip")
			return false;

		// Check that there is a tag
		if (!root.has_attribute("tag"))
			return false;


		int countBody =0, countLeft=0, countRight=0,
			countPrimary =0, countSecondary =0,
			countPhysics =0, countHealth =0;

		// Get the first node
		CL_DomNode cNode = root.get_first_child();
		while (cNode.get_node_type() != CL_DomNode::NULL_NODE)
		{
			if (cNode.is_element())
			{
				// Convert the node to an element
				CL_DomElement cElement = cNode.to_element();

				std::string elem_name = cElement.get_node_name();

				////////////////////////////////
				// Name and description elements
				if (elem_name == "name" && ++countName > 1)
						return false;

				else if (elem_name == "description" && ++countDescription > 1)
						return false;

				// Health
				else if (elem_name == "health" && ++countHealth > 1)
						return false;
	

				////////////////////////
				// Body graphic settings
				else if (elem_name == "body")
				{
					if (++countBody > 1)
						return false;

					if (cElement.get_elements_by_tag_name("image").get_length() != 1)
						return false;
				}

				/////////////////////////
				// Other graphic settings
				else if (elem_name == "leftEngine" || elem_name == "rightEngine"
					|| elem_name == "primaryWeapon" || elem_name == "secondaryWeapon")
				{

					int countElemImage =0, countElemX =0, countElemY =0;

					// Count each subnode type
					CL_DomNode subNode = root.get_first_child();
					while (subNode.get_node_type() != CL_DomNode::NULL_NODE)
					{
						if (subNode.is_element())
						{
							elem_name = subNode.get_node_name();

							if (elem_name == "image" && ++countElemImage > 1)
								return false;

							if (elem_name == "x" && ++countElemX > 1)
								return false;

							if (elem_name == "y" && ++countElemY > 1)
								return false;

						} // End if (subNode.is_element())

						subNode = subNode.get_next_sibling();
					} // End while (subNode.get_node_type() != CL_DomNode::NULL_NODE)

					if (countElemImage == 0 && countElemX == 0 && countElemY == 0)
						return false;

				} // End if (elem_name == "leftEngine || ...)

				/////////////////////
				// Physical settings
				else if (elem_name == "physics")
				{

					int countElemMass =0, countElemForce =0, countElemRotation =0;

					// Count each subnode type
					CL_DomNode subNode = root.get_first_child();
					while (subNode.get_node_type() != CL_DomNode::NULL_NODE)
					{
						if (subNode.is_element())
						{
							elem_name = subNode.get_node_name();

							if (elem_name == "mass" && ++countElemMass > 1)
								return false;

							if (elem_name == "engineforce" && ++countElemForce > 1)
								return false;

							if (elem_name == "rotationvelocity" && ++countElemRotation > 1)
								return false;

						} // End if (subNode.is_element())

						subNode = subNode.get_next_sibling();
					} // End while (subNode.get_node_type() != CL_DomNode::NULL_NODE)

					if (countElemMass == 0 && countElemForce == 0 && countElemRotation == 0)
						return false;

				} // End if (elem_name == "physics")

			} // End if (cNode.is_element())

			// Get the next node itteration
			cNode = cNode.get_next_sibling();
		} // End while (cNode.get_node_type() != CL_DomNode::NULL_NODE)


		// We've finished reading the file, so these should be exactly 1, we
		//  don't need to check < 2 because that is checked during the loop
		if (countBody == 0 && 
			countLeft == 0 && countRight == 0 && 
			countPrimary == 0 && countSecondary == 0 &&
			countPhysics == 0 && countHealth == 0)
			return false;

		// Finally, if all tests passed, return true
		return true;
	}

	ResourceManager::PackageResources ResourceManager::parseResources(CL_DomDocument *document, Archive *arc)
	{
		SurfaceMap sf_list;
		SoundBufferMap snd_list;

		//std::vector<CL_Zip_FileEntry> arcFiles = arc->get_file_list();
		StringVector arcFiles = arc->GetFileList();

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

				// [removed] Umm, what exactly was this line supposed to do? Why would
				//  sf_list contain \"name\" anyway?! Must have been really out-of-it when
				//  I coded this I guess :P
				//if (sf_list.find("name") != sf_list.end())
				//return PackageResources();

				// Makes sure the archive contains the listed file, and load it to mem
				if (checkInList(file, arcFiles))
				{
					// Uses Archive::GetFile() to get the path to the file
					sf_list[name] = new CL_Surface(arc->GetFile(file));

					//sf_list[name] = new CL_Surface(CL_PNGProvider(file, phys_source));
				}
			}

			// Sounds
			CL_DomNodeList sounds = cRes.get_elements_by_tag_name("sound");
			for (int i=0; i<sounds.get_length(); i++)
			{
				CL_DomElement sound = sounds.item(i).to_element();

				std::string name(sound.get_attribute("name"));
				std::string file(sound.get_attribute("file"));

				// same as in Images above
				if (checkInList(file, arcFiles))
				{
					snd_list[name] = new CL_SoundBuffer(arc->GetFile(file));
				}
			}
		}

		// Compile the collected resources into an easy to digest package
		PackageResources res;
		res.Images = sf_list;
		res.Sounds = snd_list;

		return res;
	}

	CL_Point ResourceManager::getPoint(const CL_DomElement *element)
	{
		// Return a zero point if the data is incomplete
		if (!(element->has_attribute("x") & element->has_attribute("y")))
			return CL_Point(0, 0);

		int x = CL_String::to_int(element->get_attribute("x"));
		int y = CL_String::to_int(element->get_attribute("y"));

		return CL_Point(x, y);
	}

	bool ResourceManager::checkInList(const std::string &filename, std::vector<std::string> filelist)
	{
		std::vector<std::string>::iterator it;
		for (it = filelist.begin(); it != filelist.end(); ++it)
		{
			if (filename == (*it))
				return true;
		}

		return false;
	}

}
