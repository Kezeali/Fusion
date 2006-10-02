#ifndef Header_FusionEngine_ResourceLoader
#define Header_FusionEngine_ResourceLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "Common.h"

#include <boost/crc.hpp>

#include "FusionShipResource.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Returned by GetPackageType().
	 */
	class PackageType
	{
	public:
		enum Type { Ship, Level, Weapon, FileNotFound, UnknownType };
	};

	/*!
	 * \brief
	 * Loads and stores resources for gameplay.
	 */
	class ResourceLoader
	{
	public:
		//! Constructor
		ResourceLoader() {}
		//! Destructor
		~ResourceLoader() { ClearAll(); }

	public:
		//! Map of surfaces
		typedef std::map<std::string, CL_Surface*> SurfaceMap;
		//! Map of sound buffers
		typedef std::map<std::string, CL_SoundBuffer*> SoundBufferMap;

		//! Map of ships
		typedef std::map<std::string, ShipResource*> ShipResourceMap;
		//typedef std::map<std::string, WeaponResource*> WeaponResourcePtrMap;

		//! The relative path to Ship packages, including trailing slash.
		static const std::string ShipsPath;
		//! The relative path to Level packages, including trailing slash.
		static const std::string LevelsPath;
		//! The relative path th Weapon packages, including trailing slash.
		static const std::string WeaponsPath;

		//! Self exp.
		static StringVector GetInstalledShips();
		//! Self exp.
		static StringVector GetInstalledLevels();
		//! Self exp.
		static StringVector GetInstalledWeapons();

		/*!
		 * \brief
		 * Clears and deletes all loaded resources.
		 */
		void ClearAll();

		/*!
		 * \brief
		 * Deduces the type of resource stored in the given package.
		 *
		 * \param name
		 * The filename of the package.
		 *
		 * \returns
		 * The PackageType of the package.
		 * If the file isn't found it returns FileNotFound.
		 * If the package is of unknown (invalid) type, it returns UnkownType.
		 */
		PackageType GetPackageType(const std::string &name);

		/*!
		 * \brief
		 * Verifies the existance and crc of a package.
		 *
		 * \param name
		 * The filename of the package.
		 *
		 * \param crc
		 * The crc that should be valid with the package.
		 *
		 * \param path
		 * The path the package should reside in.
		 */
		bool VerifyPackage(const std::string &name, const boost::crc_32_type &crc, const std::string &path);
		/*!
		 * \brief
		 * Verifies the existance and crc of a ship package.
		 *
		 * Shorthand for
		 *  <code>VerifyPackage(name, crc, "Ships/");</code>
		 * where Ships/ is the local ship package directory.
		 */
		bool VerifyShip(const std::string &name, const boost::crc_32_type &crc);
		/*!
		 * \brief
		 * Verifies the existance and crc of a level package.
		 *
		 * Shorthand for
		 *  <code>VerifyPackage(name, crc, "Levels/");</code>
		 * where Levels/ is the local level package directory.
		 */
		bool VerifyLevel(const std::string &name, const boost::crc_32_type &crc);
		/*!
		 * \brief
		 * Verifies the existance and crc of a weapon package.
		 *
		 * Shorthand for
		 *  <code>VerifyPackage(name, crc, "Weapons/");</code>
		 * where Weapons/ is the local weapons package directory.
		 */
		bool VerifyWeapon(const std::string &name, const boost::crc_32_type &crc);

		/*!
		 * \brief
		 * Forces the resource loader to load the defined ships.
		 *
		 * Calls parseShipDefinition on each of the packages named in the vector.
		 *
		 * \remarks
		 * It is recomended that LoadVerified be used instead, as it will load only previously
		 * verified packages (that is, packages with the correct filename and crc.)
		 */
		bool LoadShips(StringVector names);
		// bool LoadLevel(const std::string &name);
		// bool LoadWeapons(StringVector names);

		/*!
		 * \brief
		 * Loads all packages previously verified.
		 *
		 * The ResourceLoader class stores a list of packages verified with VerifyPackage,
		 * VerifyShip, VerifyLevel and VerifyWeapon. This function iterates through that list,
		 * loading all resources. Using this method is recomended over directly calling
		 * LoadShips, LoadLevel, and LoadWeapons.
		 *
		 * \remarks
		 * This method still has a return value (is failable) because even though the packages
		 * will have been verified as consistant with the server, the server may have bad
		 * packages installed!
		 */
		bool LoadVerified();

		/*!
		 * \brief
		 * Checks a loaded document for validity as a ship definiiton.
		 */
		ShipResourceMap GetLoadedShips();
		// LevelResource* GetLoadedLevel();
		// WeaponResourcePtrMap GetLoadedWeapons();

	private:

		//! Encapsulates resource maps of various types.
		struct PackageResources
		{
			SurfaceMap Images;
			SoundBufferMap Sounds;
		};

		ShipResourceMap m_ShipResources;
		// LevelResource* m_LevelResource;
		// WeaponResourcePtrMap m_WeaponResources;

		ShipResource* parseShipDefinition(const std::string &filename);
		//LevelResource* parseShipDefinition(const std::string &filename);
		//WeaponResource* parseShipDefinition(const std::string &filename);

		/*!
		 * \brief
		 * Checks a loaded document for validity as a ship definiiton.
		 */
		bool verifyShipDocument(const CL_DomDocument *document);
		/*!
		 * \brief
		 * Checks a loaded document for validity as a level definiiton.
		 */
		//bool verifyLevelDocument(const CL_DomDocument *document);
		/*!
		 * \brief
		 * Checks a loaded document for validity as a weapon definiiton.
		 */
		//bool verifyWeaponDocument(const CL_DomDocument *document);

		/*!
		 * \brief
		 * Reads a list of resources from a package definition.
		 *
		 * As all package documents use the same format for listing resources, only the one
		 * method is needed.
		 *
		 * \param document
		 * A loaded definition document.
		 *
		 * \returns
		 * A PackageResources object containing all resources.
		 */
		PackageResources parseResources(const CL_DomDocument *document, const CL_Zip_Archive *arc);

		/*!
		 * \brief
		 * Gets a point form the 'x' and 'y' attributes of an element.
		 *
		 * \returns
		 * The CL_Point built from the x and y attributes.
		 * If the given element has no x/y attribute, the point (0, 0) is returned.
		 */
		CL_Point getPoint(const CL_DomElement *element);

		/*!
		 * \brief
		 * Checks the passed list for the specified file.
		 *
		 * Used for checking if a resource specified in a definition actually exists in the
		 * package!
		 */
		bool checkInList(
			const std::string &filename,
			StringVector filelist
			);
	};

}

#endif
