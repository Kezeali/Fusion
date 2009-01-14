/*
  Copyright (c) 2006-2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward
*/

#ifndef Header_FusionEngine_ResourceManager
#define Header_FusionEngine_ResourceManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

/// Fusion
#include "FusionResource.h"
#include "FusionResourcePointer.h"
#include "FusionResourceLoader.h"

/// RakNet
#include <RakNet/Bitstream.h>

#include <boost/shared_ptr.hpp>

namespace FusionEngine
{

	//class ResourceManagerException : public Exception
	//{
	//public:
	//	ResourceManagerException(const std::string& message)
	//		: Exception(Exception::INTERNAL_ERROR, message)
	//	{}
	//};

	/*!
	 * \brief
	 * Returned by GetPackageType().
	 */
	//class PackageType
	//{
	//public:
	//	enum Type { Ship, Level, Weapon, FileNotFound, UnknownType };
	//};

	/*!
	 * \brief
	 * Loads and stores resources for gameplay.
	 *
	 * \todo Add XMLloader to default loaders
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
		// this is now defined in ResourcePointer:
		//typedef boost::shared_ptr<Resource> ResourceSpt;

		//! ResourceLoader pointer
		typedef std::tr1::shared_ptr<ResourceLoader> ResourceLoaderSpt;
		//! Maps ResourceTag keys to Resource ptrs
		typedef std::tr1::unordered_map<ResourceTag, ResourceSpt> ResourceMap;
		//! Maps Resource types to ResourceLoader objects
		typedef std::tr1::unordered_map<std::string, ResourceLoaderSpt> ResourceLoaderMap;

	public:
		//! Constructor
		ResourceManager(char *arg0);
		//! Constructor - gets equivilant of arg0 from CL_System::get_exe_path()
		ResourceManager();
		//! Destructor
		~ResourceManager();

	public:
		//! Configures the resource manager
		void Configure();
		////! Checks the filesystem for packages and returns the names of all found
		//StringVector ListAvailablePackages();
		//! Checks the filesystem and returns the filenames of all found
		StringVector ListFiles();

		//! Unloads resource which aren't currently held by any ResourcePointer objects
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

		//! Returns a collection of tokens representing the given wildcard expression
		StringVector TokenisePattern(const std::string &expression);

		//! Compares a string to a wildcard string
		bool CheckAgainstPattern(const std::string &str, const std::string &expression);
		//! Compares a string to a wildcard string
		bool CheckAgainstPattern(const std::string &str, StringVector expressionTokens);

		//! Returns the first matching string
		std::string FindFirst(const std::string &expression, bool recursive = false, bool case_sensitive = true);
		std::string FindFirst(const std::string &path, const std::string &expression, int depth, bool recursive = true, bool case_sensitive = true);

		//! Lists filenames matching the given expression
		StringVector Find(const std::string &expression, bool recursive = false, bool case_sensitive = true);

		//! Lists filenames matching the given expression, below the given path
		/*!
		 * If depth is set >1 and recursive is false, this method will go 'depth' folders deep in the
		 * filesystem below the Search Path. If recursive is true, it will ignore 'depth' and go as 
		 * deep as possible.
		 */
		StringVector Find(const std::string &path, const std::string &expression, int depth, bool recursive = true, bool case_sensitive = true);

		////! Finds and opens the given package
		///*!
		// * Checks for the given package and opens it if it is found.
		// *
		// * \returns null if no package with the given name is found
		// */
		//TiXmlDocument* OpenPackage(const std::string &name);

		///*!
		// * \brief
		// * Executed client-side when a VerifyPackage packet is received.
		// *
		// * Verifies the existance and crc of a package.
		// *
		// * \param[in] stream
		// * The VerifyPackage bitstream sent from the server.
		// */
		////bool VerifyPackage(RakNet::BitStream *stream);

		///*!
		// * \brief
		// * Loads the given package
		// */
		//bool LoadPackage(const std::string &name);

		///*!
		// * \brief
		// * Loads the listed packages
		// */
		//bool LoadPackages(StringVector names);

		///*!
		// * \brief
		// * Loads all packages previously verified.
		// *
		// * The ResourceLoader class stores a list of packages verified with VerifyPackage,
		// * VerifyShip, VerifyLevel and VerifyWeapon. This function iterates through that list,
		// * loading all resources. Using this method is recomended over directly calling
		// * LoadShips, and LoadWeapons. LoadLevelVerified should be called for levels.
		// * <br>
		// * If this fails, you should call ClearAll to destroy any invalid data.
		// *
		// * \remarks
		// * This method can fail, because even though the packages
		// * will have been verified as consistant with the server, the server may have bad
		// * packages installed!
		// */
		//bool LoadVerified();

		////! Clears the verified packages lists
		//void ResetVerified();

		void AddDefaultLoaders();

		//! Assigns the given resource loader plugin to its relavant resource type
		void AddResourceLoader(ResourceLoaderSpt loader);

		//! Loads a resource, gives it a tag
		void TagResource(const std::string& type, const std::string& path, const ResourceTag& tag);

		//! Loads a resource
		/*!
		 * \remarks
		 * Though this does the same as TagResource, it must be named differently so that it can be
		 * used directly (through a THISCALL) by AScript, which can't deal with overloaded member fn.s
		 */
		void PreloadResource(const std::string& type, const std::string& path);

		//! Returns a ResourcePointer to the given Resource (of type T)
		//template<typename T>
		//ResourcePointer<T> GetResource(const ResourceTag& tag);

		template<typename T>
		ResourcePointer<T> GetResource(const ResourceTag &tag)
		{
			PreloadResource(GetResourceType<T>(), tag);

			ResourceSpt sptRes = m_Resources[tag];
			if (!sptRes->IsValid())
			{
				InputSourceProvider_PhysFS provider("");
				m_ResourceLoaders[sptRes->GetType()]->ReloadResource(sptRes.get(), &provider);
			}

			return ResourcePointer<T>(sptRes);
		}

		template<typename T>
		ResourcePointer<T> GetResource(const ResourceTag &tag, const std::string& type)
		{
			PreloadResource(type, tag);

			ResourceSpt sptRes = m_Resources[tag];
			if (!sptRes->IsValid())
			{
				InputSourceProvider_PhysFS provider("");
				m_ResourceLoaders[sptRes->GetType()]->ReloadResource(sptRes.get(), &provider);
			}

			return ResourcePointer<T>(sptRes);
		}

		//! Optimised version of GetResource
		template<typename T>
		void GetResource(ResourcePointer<T>& out, const ResourceTag &tag)
		{
			PreloadResource(GetResourceType<T>(), tag);

			ResourceSpt sptRes = m_Resources[tag];
			if (!sptRes->IsValid())
			{
				InputSourceProvider_PhysFS provider("");
				m_ResourceLoaders[sptRes->GetType()]->ReloadResource(sptRes.get(), &provider);
			}

			out = ResourcePointer<T>(sptRes);
		}

		// How many characters from the beginning before a and b diverge
		std::string::size_type quickCompare(const std::string &a, const std::string &b)
		{
			const char* a_cstr = a.c_str();
			const char* b_cstr = b.c_str();
			std::string::size_type length = fe_min(a.length(), b.length());

			for (std::string::size_type i = 0; i < length; i++)
			{
				if (a_cstr[i] != b_cstr[i]) return i;
			}
		}

		template<typename T>
		ResourcePointer<T> OpenOrCreateResource(const ResourceTag &path)
		{
			std::string match = FindFirst(path);
			if (match.empty()) // No match
			{
				std::string writePath(PHYSFS_getWriteDir());
				// Whether the given 'path' is an existing, absolute path
				if (PHYSFS_isDirectory(CL_String::get_path(path).c_str()))
				{
					// If the given path is existing and absolute, make sure it is within the PhysFS write folder
					if (quickCompare(path, writePath) >= writePath.length())
					{
						FSN_EXCEPT(ExCode::IO, "ResourceManager:OpenOrCreateResource",
							"Can't create '" + path + "' as it is not within the write path (" + writePath + ")");
					}
				}
				else
				{
					// Convert the relative path to an absolute path within the write folder
					writePath = writePath + path;

					std::string folder(CL_String::get_path(writePath));
					if (!PHYSFS_isDirectory(folder.c_str()))
						FSN_EXCEPT(ExCode::IO, "ResourceManager:OpenOrCreateResource",
						"Can't create '" + writePath + "' as '" + folder + "' does not exist");
				}
				// Create the file
				std::ofstream resourceFile;
				resourceFile.open(writePath.c_str(), std::ios::out|std::ios::binary);
				resourceFile.close();

				match = writePath;
			}
			
			return GetResource<T>(match);
		}

		void RegisterScriptElements(ScriptingEngine* manager);

	private:
		bool m_PhysFSConfigured;

		//! A list of packages which passed verification
		StringVector m_VerifiedPackages;

		//! Resources
		ResourceMap m_Resources;

		//! Garbage
		ResourceMap m_Garbage;

		ResourceLoaderMap m_ResourceLoaders;

	protected:
		void registerXMLType(asIScriptEngine* engine);
		void registerImageType(asIScriptEngine* engine);
		void registerSoundType(asIScriptEngine* engine);

		//RNode createResourceNode(TiXmlElement* xmlNode);
		/*!
		 * \brief
		 * Returns the pixel the given percentage from the left of the window.
		 */
		int percentToAbsX(int percent)
		{
			return (int)(CL_Display::get_width() * percent * 0.01);
		}

		/*!
		 * \brief
		 * Returns the pixel the given percentage from the top of the window.
		 */
		int percentToAbsY(int percent)
		{
			return (int)(CL_Display::get_height() * percent * 0.01);
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
