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

/// RakNet
#include <RakNet/Bitstream.h>

namespace FusionEngine
{

	class ResourceManagerException : public Exception
	{
	public:
		ResourceManagerException(const std::string& message)
			: Exception(Exception::INTERNAL_ERROR, message)
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
	 * \todo Load exception error messages from XML
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
	 *
	 * \sa Resource | ResourcePointer | ResourceLoader
	 */
	class ResourceManager : public Singleton<ResourceManager>
	{
	public:
		//! Maps ResourceTag keys to Resource ptrs
		typedef std::map<ResourceTag, Resource*> ResourceMap;
		//! Maps Resource types to ResourceLoader objects
		typedef std::map<const char *, ResourceLoader*> ResourceLoaderMap;

	public:
		//! Constructor
		ResourceManager(char **argv);
		//! Destructor
		~ResourceManager();

	public:
		//! Configures the resource manager
		void Configure();
		//! Checks the filesystem for packages and returns the names of all found
		StringVector ListAvailablePackages();
		//! Checks the filesystem for packages and returns the /filenames/ of all found
		StringVector ListAvailablePackageFiles();

		//! Runs garbage collection
		void DisposeUnusedResources();

		//! Deletes all loaded resources
		void DeleteResources();

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
		PackageType GetPackageType(const std::string &name);

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

		//! Returns a collection of tokens representing the given wildcard expression
		StringVector TokeniseExpression(const std::string &expression);

		//! Compares a string to a wildcard expression
		bool CheckAgainstExpression(const std::string &str, const std::string &expression);
		//! Compares a string to a wildcard expression
		bool CheckAgainstExpressionWithOptions(const std::string &str, StringVector expressionTokens);
		//! Compares a string to a wildcard expression
		bool CheckAgainstExpression(const std::string &str, StringVector expressionTokens);

		//! Lists filenames matching the given expression
		StringVector Find(const std::string &expression, bool case_sensitive = true, bool recursive = false);

		//! Lists filenames matching the given expression, below the given path
		/*!
		 * If depth is set >1 and recursive is false, this method will go 'depth' folders deep in the
		 * filesystem below the Search Path. If recursive is true, it will ignore 'depth' and go as 
		 * deep as possible.
		 */
		StringVector Find(const std::string &path, const std::string &expression, int depth, bool case_sensitive = true, bool recursive = true);

		//! Finds and opens the given package
		/*!
		 * Checks for the given package and opens it if it is found.
		 *
		 * \returns null if no package with the given name is found
		 */
		TiXmlDocument* OpenPackage(const std::string &name);

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
		 * Loads the given package
		 */
		bool LoadPackage(const std::string &name);

		/*!
		 * \brief
		 * Loads the listed packages
		 */
		bool LoadPackages(StringVector names);

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

	private:
		bool m_PhysFSConfigured;

		//! A list of packages which passed verification
		StringVector m_VerifiedPackages;

		//! Resources
		ResourceMap m_Resources;

		ResourceLoaderMap m_ResourceLoaders;

	protected:
		RNode createResourceNode(TiXmlElement* xmlNode);
		/*!
		 * \brief
		 * Returns the pixel the given percentage from the left of the window.
		 */
		int percentToAbsX(int percent)
		{
			return CL_Display::get_width() * percent * 0.01;
		}

		/*!
		 * \brief
		 * Returns the pixel the given percentage from the top of the window.
		 */
		int percentToAbsY(int percent)
		{
			return CL_Display::get_height() * percent * 0.01;
		}

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

#endif
