  #ifndef Header_FusionEngine_ResourceLoader
#define Header_FusionEngine_ResourceLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

/// Fusion
#include "FusionResource.h"
#include "FusionResourcePointer.h"

#include "FusionShipResourceBundle.h"
#include "FusionLevelResourceBundle.h"
#include "FusionWeaponResourceBundle.h"

/// RakNet
#include <RakNet/Bitstream.h>

namespace FusionEngine
{

	////! Map of ship res. names to ship resources
	//typedef std::map<std::string, ShipResource*> ShipResourceMap;
	////! \see ShipResourceMap
	//typedef std::map<std::string, WeaponResource*> WeaponResourceMap;

	class PackageLoadException : public Error
	{
	public:
		PackageLoadException(const std::string& message)
			: Exception(Exception::ExceptionType::PACKLOAD, message)
		{}
	};

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
	 *
	 * \todo Impliment PhysFS <br>
	 * DONE: create InputSourceProvider_PhysFS (based on CL_InputSourceProvider_File) <br>
	 * DONE: create InputSource_PhysFile (based on CL_InputSource_File) <br>
	 * <br>
	 * Pass *physFSProvider = new InputSourceProvider_PhysFS("Resource Package");
	 * to the generic package parser method <br>
	 * <br>
	 * In the generic package parser: <br>
	 * Search for .xml files in the archive (make a method in ResourceLoader
	 * that calls PHYSFS_enumerateFiles and returns a string vector) <br>
	 * Read the xml files to find out what type of resources they define <br>
	 * Call the specific resource parser for that type of resource <br>
	 * Use CL_Surface("physfs filename" , physFSProvider);
	 */
	class ResourceManager : public Singleton<ResourceLoader>
	{
	public:
		typedef std::map<ResourceTag, Resource> ResourceMap;

	public:
		//! Constructor
		ResourceManager() {}
		//! Destructor
		~ResourceManager() { ClearAll(); }

	public:
		//! Returns a list of packages found after the Ships path
		static StringVector GetInstalledShips();
		//! Returns a list of packages found after the Levels path
		static StringVector GetInstalledLevels();
		//! Returns a list of packages found after the Weapons path
		static StringVector GetInstalledWeapons();

		//! Deletes all loaded resources
		void DeleteResources();

		//! Runs garbage collection
		void DisposeUnusedResources();

		//! Clears the resource manager
		/*!
		 * Clears all resources.
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
		 * \retval FileNotFound
		 * If the file isn't found.
		 * \retval UnknownType
		 * If the package is of unknown (invalid) type.
		 */
		//PackageType GetPackageType(const std::string &name);

		//! Gets a verification bitstream for the given package.
		/*!
		 * Gets a bitstream which can be sent to a client (assuming the client
		 * is in the Loading state) to make the client verify the package.
		 *
		 * \param[out] stream
		 * The stream to write to.
		 * \param[in] name
		 * The file to verify.
		 */
		//void GetVerification(RakNet::BitStream *stream, const std::string &name);

		/*!
		 * \brief
		 * Executed client-side when a VerifyPackage packet is received.
		 *
		 * Verifies the existance and crc of a package.
		 *
		 * \param[in] stream
		 * The VerifyPackage bitstream sent from the server.
		 */
		//bool VerifyPackage(RakNet::BitStream *stream);

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
		bool LoadLevel(const std::string &name);
		bool LoadWeapons(StringVector names);

		//! Loads the given level only if it has been verified.
		bool LoadLevelVerified(const std::string &name);
		/*!
		 * \brief
		 * Loads all packages previously verified.
		 *
		 * The ResourceLoader class stores a list of packages verified with VerifyPackage,
		 * VerifyShip, VerifyLevel and VerifyWeapon. This function iterates through that list,
		 * loading all resources. Using this method is recomended over directly calling
		 * LoadShips, and LoadWeapons. LoadLevelVerified should be called for levels.
		 * <br>
		 * If this fails, you should call ClearAll to destroy any invalid data.
		 *
		 * \remarks
		 * This method still has a return value (is failable) because even though the packages
		 * will have been verified as consistant with the server, the server may have bad
		 * packages installed!
		 */
		bool LoadVerified();

		//! Clears the verified packages lists
		void ResetVerified();

		//! Returns a ResourcePointer to the given Resource (of type T)
		template<typename T>
		ResourcePointer<T> GetResource(const ResourceTag& tag);

		/*!
		 * \brief
		 * Returns maps to all loaded ships.
		 */
		//ShipResourceMap GetLoadedShips();
		//LevelResourceBundle* GetLoadedLevel();
		//WeaponResourceMap GetLoadedWeapons();

		////! Gets one loaded ship by tag
		//ShipResourceBundle* GetShipResource(const std::string& name) const;
		////! Gets one loaded weapon by tag
		//WeaponResourceBundle* GetWeaponResource(const std::string& name) const;

	private:

		//! Encapsulates resource maps of various types.
		//struct PackageResources
		//{
		//	//! Image files mapped to tags
		//	SurfaceMap Images;
		//	//! Sound files mapped to tags
		//	SoundBufferMap Sounds;
		//};

		//SurfaceMap m_Images;
		//SoundBufferMap m_Sounds;

		//! A list of ship packages which passed verification
		StringVector m_VerifiedShips;
		//! A list of level packages which passed verification
		/*!
		 * This list all verified level packages, so LoadLevelVerified can ensure any
		 * particular level package has been verified before attempting to load them.
		 */
		StringVector m_VerifiedLevels;
		//! A list of weapon packages which passed verification
		StringVector m_VerifiedWeapons;

		//! Resource bundles
		ResourceBundleMap m_ResourceBundles;

	protected:
		//! Loads a ship
		ShipResource* parseShipDefinition(const std::string &filename);
		//! Loads a level
		LevelResource* parseLevelDefinition(const std::string &filename);
		//! Loads a weapon
		WeaponResource* parseWeaponDefinition(const std::string &filename);

		//! Parses the element for engine data
		ShipEngineElement* parseShipEngineElement(CL_DomElement *element);
		//! Parses the element for weapon data
		ShipWeaponElement* parseShipWeaponElement(CL_DomElement *element);

		//! Creates the projectile to be used for a ships special weapon
		ProjectileResource* buildProjectileFromEngineElement(CL_DomElement *element);

		/*!
		 * \brief
		 * Checks a loaded document for validity as a ship definiiton.
		 *
		 * Reads the document to confirm that there is exactly one of each
		 * non-optional element.
		 */
		bool verifyShipDocument(CL_DomDocument *document);
		/*!
		 * \brief
		 * Checks a loaded document for validity as a level definiiton.
		 */
		bool verifyLevelDocument(CL_DomDocument *document);
		/*!
		 * \brief
		 * Checks a loaded document for validity as a weapon definiiton.
		 */
		bool verifyWeaponDocument(CL_DomDocument *document);

		/*!
		 * \brief
		 * [depreciated]
		 * Reads a list of resources from a package definition.
		 *
		 * As all package documents use the same format for listing resources, only the one
		 * method is needed.
		 *
		 * \param[in] document
		 * A loaded definition document.
		 *
		 * \returns
		 * A PackageResources object containing all resources.
		 */
		PackageResources parseResources(CL_DomDocument *document, Archive *arc);

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
		 * Returns a surface of the image element given.
		 *
		 * If the given image has already been loaded by the resource loader,
		 * the same pointer will be returned.
		 */
		CL_Surface* getImage(const CL_DomElement *element);

		/*!
		 * \brief
		 * Returns a soundbuffer for the sound element given.
		 *
		 * If the given sound has already been loaded by the resource loader,
		 * the same pointer will be returned.
		 */
		CL_SoundBuffer* getSound(const CL_DomElement *element);

		/*!
		 * \brief
		 * Checks the given list for the specified file.
		 *
		 * Used for checking if a resource specified in a definition actually exists in the
		 * package!
		 *
		 * \param[in] filename The name of the file to be checked
		 * \param[in] filelist The list to search for the given filename
		 */
		bool checkInList(const std::string &filename, StringVector filelist);
	};

}

		//bool VerifyPackage(const std::string &name, const boost::crc_32_type &crc, const std::string &path);
		/*!
		 * \brief
		 * Verifies the existance and crc of a ship package.
		 *
		 * Shorthand for
		 *  <code>VerifyPackage(name, crc, "Ships/");</code>
		 * where Ships/ is the local ship package directory.
		 */
		//bool VerifyShip(const std::string &name, const boost::crc_32_type &crc);
		/*!
		 * \brief
		 * Verifies the existance and crc of a level package.
		 *
		 * Shorthand for
		 *  <code>VerifyPackage(name, crc, "Levels/");</code>
		 * where Levels/ is the local level package directory.
		 */
		//bool VerifyLevel(const std::string &name, const boost::crc_32_type &crc);
		/*!
		 * \brief
		 * Verifies the existance and crc of a weapon package.
		 *
		 * Shorthand for
		 *  <code>VerifyPackage(name, crc, "Weapons/");</code>
		 * where Weapons/ is the local weapons package directory.
		 */
		//bool VerifyWeapon(const std::string &name, const boost::crc_32_type &crc);

#endif
