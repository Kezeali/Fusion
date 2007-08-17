
/// Class
#include "FusionResourceManager.h"

/// Fusion
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionInputSourceProvider_PhysFS.h"
#include "FusionInputSource_PhysFS.h"

namespace FusionEngine
{

	ResourceManager::ResourceManager(char **argv)
		: m_PhysFSConfigured(false)
	{
		SetupPhysFS::init(argv[0]);
	}

	ResourceManager::~ResourceManager()
	{
		ClearAll();

		SetupPhysFS::deinit();
	}

	// Look, nice formatting :D
	///////////
	/// Public:
	void ResourceManager::Configure()
	{
		// A package (xml file) will contain all the ResourceManager config info
		//  and will be placed in the working directory of the game. This file will list all
		//  paths to be added to the search path, as well as the config info.
		SetupPhysFS::configure("Case Software", "Fusion", "ZIP");
		SetupPhysFS::add_subdirectory("Packages/", "ZIP", true);

		m_PhysFSConfigured = true;
	}

	StringVector ResourceManager::ListInstalledPackageFiles()
	{
		StringVector list;

		// Native (OS) filesystem (removed, should only be used if PhysFS is not present)
		//! \todo Check if physfs is available and use native directory scanner if not
		//CL_DirectoryScanner scanner;
		//if (scanner.scan(PackagesPath, "*.xml"))
		//{
		//	while (scanner.next())
		//	{
		//		list.push_back(scanner.get_name());
		//	}
		//}

		// List all files (remember, physfs will also find files in the native filesystem)
		char **files = PHYSFS_enumerateFiles("");
		if (files == NULL)
			throw ResourceManagerException(Exception::GetString("ResourceManager::ListInstalledPackageFiles()", "/strings/exceptions/ResourceManager/t0001"));
		else
		{
			int file_count;
			char **i;
			for (i = files, file_count = 0; *i != NULL; i++, file_count++)
				list.push_back(std::string(*i));

			PHYSFS_freeList(files);
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
		ResourceMap::iterator it = m_Resources.iterator();
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

	StringVector ResourceManager::TokeniseExpression(const std::string &expression)
	{
		StringVector expressionTokens;
		StringVector expressionTokens1, expressionTokens2;

		size_t mid = expression.find("*");

		// If another marker is found
		if (mid != std::string::npos)
		{
			// Split the expression into sub-expressions around the marker (tokens)
			expressionTokens1 = TokeniseExpression( expression.substr(0, mid) );
			expressionTokens2 = TokeniseExpression( expression.substr(mid+1) );

			expressionTokens.resize(expressionTokens1.size() + expressionTokens2.size());

			std::copy(expressionTokens1.begin(), expressionTokens1.end(), expressionTokens.begin());
			std::copy(expressionTokens2.begin(), expressionTokens2.end(), expressionTokens.begin()+expressionTokens1.size());
		}
		// If no markers were found, add this sub-expression (token)
		else if (!expression.empty())
			expressionTokens.push_back(expression);

		return expressionTokens;
	}

	bool ResourceManager::CheckAgainstExpression(const std::string &str, const std::string &expression)
	{
		CheckAgainstExpression(str, TokeniseExpression(expression));
	}

	bool ResourceManager::CheckAgainstExpressionWithOptions(const std::string &str, StringVector expressionTokens)
	{
		size_t strPos = 0;
		bool optionalSection = false, optionFound = false;
		StringVector::iterator it = expressionTokens.begin();
		for (; it != expressionTokens.end(); ++it)
		{
			// Detect options
			if ((*it) == "[")
			{
				optionalSection = true;
				optionFound = false;
				continue; // We don't need to check for the [!
			}
			if ((*it) == "]")
			{
				if (optionFound)
					return false;

				optionalSection = false;
				continue; // We don't need to check for the ]!
			}

			// Skip all options till the end of the current section (after an option has been found)
			if (optionalSection && optionFound)
				continue;

			// We search from the last found token (all tokens must exist /in the correct order/ for the string to match)
			strPos = str.find(*it, strPos);
			if (!optionalSection && strPos == std::string::npos)
				return false;

			else if (strPos != std::string::npos)
				optionFound = true;

		}
		return true;
	}

	bool ResourceManager::CheckAgainstExpression(const std::string &str, StringVector expressionTokens)
	{
		size_t strPos = 0;
		StringVector::iterator it = expressionTokens.begin();
		for (; it != expressionTokens.end(); ++it)
		{
			// We search from the last found token (all tokens must exist /in the correct order/ for the string to match)
			strPos = str.find(*it, strPos);
			if (strPos == std::string::npos)
				return false;

		}
		return true;
	}

	StringVector ResourceManager::Find(const std::string &expression, bool case_sensitive, bool recursive)
	{
		return Find("", expression, case_sensitive, 0, case_sensitive, recursive);
	}

	StringVector ResourceManager::Find(const std::string &path, const std::string &expression, int depth, bool case_sensitive, bool recursive)
	{
		StringVector list;

		StringVector expressionTokens;
		if (case_sensitive)
			expressionTokens = TokeniseExpression(expression);
		else
			expressionTokens = TokeniseExpression(fe_newupper(expression));

		if (m_PhysFSConfigured)
		{
			char **files = PHYSFS_enumerateFiles(path.c_str());
			if (files != NULL)
			{
				int file_count;
				char **i;
				for (i = files, file_count = 0; *i != NULL; i++, file_count++)
				{
					// If recursive is set, search within sub-folders
					if ((recursive || depth--) && PHYSFS_isDirectory(*i))
						Find(std::string(*i), expression, depth, case_sensitive, recursive);

					std::string file(*i);
					// Do the relevant check for case (in)sensitive searches
					if ((case_sensitive && CheckAgainstExpression(file, expressionTokens)) ||
						(!case_sensitive && CheckAgainstExpression(fe_newupper(file), expressionTokens)))
						list.push_back(file);
				}

				PHYSFS_freeList(files);
			}
		}

		return list;
	}

	TiXmlDocument* ResourceManager::OpenPackage(const std::string &name)
	{
		if (m_PhysFSConfigured)
		{
			Find("*.xml");
		}
	}

	bool ResourceManager::LoadPackage(const std::string &name)
	{
		TiXmlDocument* doc = OpenPackage(name);


		return true;
	}

	bool ResourceManager::LoadPackages(StringVector names)
	{
		StringVector::iterator it;
		for (it = names.begin(); it != names.end(); ++it)
		{
			if (!LoadPackage(*it))
				return false;
		}

		return true;
	}

	bool ResourceManager::LoadVerified()
	{
		return LoadPackages(m_VerifiedPackages);
	}

	void ResourceManager::ResetVerified()
	{
		m_VerifiedPackages.clear();
	}

	template<typename T>
	ResourcePointer<T> ResourceManager::GetResource(const ResourceTag &tag)
	{
	}

	////////////
	/// Private:
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
