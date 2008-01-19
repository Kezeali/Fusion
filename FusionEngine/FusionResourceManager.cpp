
/// Class
#include "FusionResourceManager.h"

#include "FusionResourceManagerScriptFunctions.h"

/// Fusion
#include "FusionConsole.h"

#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionInputSourceProvider_PhysFS.h"
#include "FusionInputSource_PhysFS.h"

#include "FusionImageLoader.h"
#include "FusionAudioLoader.h"

#include "FusionScriptingEngine.h"
#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	ResourceManager::ResourceManager(char **argv)
		: m_PhysFSConfigured(false)
	{
		int ok = SetupPhysFS::init(argv[0]);
		assert(ok);

		Configure();
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
		SetupPhysFS::configure("CardboardBox Software", "Fusion", "ZIP");
		if (!SetupPhysFS::add_subdirectory("Packages/", "ZIP", true))
			SendToConsole("Default resource path could not be located");

		m_PhysFSConfigured = true;

		AddDefaultLoaders();
	}

	StringVector ResourceManager::ListFiles()
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
		if (files != NULL)
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
		// not yet implemented
#ifdef RESMAN_USEGARBAGELIST
		ResourceGarbageList::iterator it = m_Garbage.begin();
		for (; it != m_Garbage.end(); ++it)
		{
			Resource res = m_Resources.find(*it);
			if (res != m_Resources.end())
				m_Resources.erase(res);
		}
		m_Garbage.clear();
#else

		ResourceMap::iterator it = m_Resources.begin();
		ResourceMap::iterator end = m_Resources.end();
		for (; it != end; ++it)
		{
			ResourceSpt& res = (*it).second;

#if defined(FSN_RESOURCEPOINTER_USE_WEAKPTR)

			if ( !res->IsReferenced() )
			{

#else

			if (res.unique())
			{

#endif
				m_ResourceLoaders[res->GetType()]->UnloadResource(res.get());
				//it = m_Resources.erase(it);
			}
		}

#endif
	}

	void ResourceManager::DeleteResources()
	{
		m_Resources.clear();
	}

	void ResourceManager::ClearAll()
	{
		//ResetVerified();

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
		return CheckAgainstExpression(str, TokeniseExpression(expression));
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
		return Find("", expression, 0, case_sensitive, recursive);
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

	void ResourceManager::AddDefaultLoaders()
	{
		AddResourceLoader(new ImageLoader());
		AddResourceLoader(new AudioLoader());
		AddResourceLoader(new AudioStreamLoader());
	}

	void ResourceManager::AddResourceLoader(ResourceLoader* loader)
	{
		m_ResourceLoaders[loader->GetType()] = ResourceLoaderSpt(loader);
	}

	void ResourceManager::PretagResource(const std::string& type, const std::string& path, const ResourceTag& tag)
	{
		ResourceMap::iterator existing = m_Resources.find(tag);
		if (existing == m_Resources.end())
		{
			ResourceLoaderMap::iterator loader = m_ResourceLoaders.find(type);
			if (loader == m_ResourceLoaders.end())
			{
				FSN_EXCEPT(ExCode::FileType, "ResourceManager::PreloadResource", "Attempted to load unknown resource type '" + type + "'");
			}

			ResourceContainer* res;
			//try
			//{
				res = loader->second->LoadResource(tag, path, NULL);
			//}
			//catch (CL_Error&)
			//{
			//	FSN_EXCEPT(ExCode::IO, "ResourceManager::PreloadResource", "'" + path + "' could not be loaded");
			//}

			m_Resources[tag] = ResourceSpt(res);
		}
	}

	void ResourceManager::PreloadResource(const std::string& type, const std::string& path)
	{
		PretagResource(type, path, path);
	}

	void ResourceManager::RegisterScriptElements(ScriptingEngine* manager)
	{
		asIScriptEngine* engine = manager->GetEnginePtr();
		int r;
		r = engine->RegisterObjectType("ResourceManager", sizeof(ResourceManager), asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
		r = engine->RegisterObjectMethod("ResourceManager", "void Cache(string &in, string &in)", asMETHOD(ResourceManager, PreloadResource), asCALL_THISCALL); assert( r >= 0 );

		// TODO: put all this type registration stuff into another header, then implement a 'plugin' type system
		//  where new script types can be passed in a simmilar way to ResourceLoaders
		registerXMLType(engine);
		registerImageType(engine);
		registerSoundType(engine);

		r = engine->RegisterObjectMethod("ResourceManager", "Image GetImage(string &in)", asFUNCTION(ResourceManager_GetImage), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("ResourceManager", "Sound GetSound(string &in)", asFUNCTION(ResourceManager_GetSound), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("ResourceManager", "XmlDocument GetXML(string &in)", asFUNCTION(ResourceManager_GetXml), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("ResourceManager", "string GetText(string &in)", asFUNCTION(ResourceManager_GetText), asCALL_CDECL_OBJLAST); assert( r >= 0 );


		r = engine->RegisterGlobalProperty("ResourceManager resource_manager", this);
		r = engine->RegisterGlobalProperty("ResourceManager file", this);
	}

	//template<typename T>
	//ResourcePointer<T> ResourceManager::GetResource(const ResourceTag &tag)
	//{
	//	PreloadResource(GetResourceType(T).c_str(), tag);

	//	Resource& res = (*m_Resources[tag]);
	//	if (!res.IsValid())
	//	{
	//		m_ResourceLoaders[res.GetType()]->ReloadResource(res);
	//	}

	//	return ResourcePointer<T>(res);
	//}

	////////////
	/// Private:
	void ResourceManager::registerXMLType(asIScriptEngine* engine)
	{
		int r;

		RegisterResourcePointer<TiXmlDocument>("XmlDocument", engine);

		RegisterTypePOD<TiXmlNode>("XmlNode", engine);

		r = engine->RegisterObjectMethod("XmlDocument",
			"string find(string& in)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_String, (std::string&), std::string),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"string xpath_string(string& in)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_String, (std::string&), std::string),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"double xpath_double(string& in)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_Double, (std::string&), double),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"float xpath_float(string& in)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_Float, (std::string&), float),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"int xpath_int(string& in)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_Int, (std::string&), int),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"bool xpath_int(string& in)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_Int, (std::string&), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");

		r = engine->RegisterObjectMethod("XmlDocument",
			"bool xpath(string &in, string &out)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedString, (std::string&, std::string&), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"bool xpath(string &in, double &out)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedDouble, (std::string&, double&), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"bool xpath(string &in, float &out)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedFloat, (std::string&, float&), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"bool xpath(string &in, int &out)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_Int, (std::string&, int&), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"bool xpath(string &in, uint &out)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedUInt, (std::string&, unsigned int&), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");
		r = engine->RegisterObjectMethod("XmlDocument",
			"bool xpath(string &in, bool &out)",
			asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedBool, (std::string&, bool&), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register a method");

		//r = engine->RegisterObjectMethod("XmlDocument",
		//	"string& get_root_name()",
		//	asFUNCTIONPR(XML_GetRootName, (std::string&), std::string&),
		//	asCALL_CDECL_OBJFIRST);

		//r = engine->RegisterObjectMethod("XmlDocument",
		//	"XmlNode get_root()",
		//	asFUNCTIONPR(XML_GetRoot, (void), TiXmlNode),
		//	asCALL_CDECL_OBJFIRST);

		//r = engine->RegisterObjectMethod("XmlDocument",
		//	"XmlNode xpath_node(string &in)",
		//	asFUNCTIONPR(XML_GetRoot, (void), TiXmlNode),
		//	asCALL_CDECL_OBJFIRST);
		//r = engine->RegisterObjectMethod("XmlDocument",
		//	"string& find(XmlNode &in, string& in)",
		//	asFUNCTIONPR(XML_ExecuteXPathExpr_String, (std::string&), std::string&),
		//	asCALL_CDECL_OBJFIRST);
		// /document/element will get the element name
		// /document/element/ will get the text child (if one exists)


		//r = engine->RegisterObjectMethod("XmlNode",
		//	"string& get_text()",
		//	asFUNCTIONPR(XMLNode_GetText, (void), std::string&),
		//	asCALL_CDECL_OBJFIRST);
		//assert(r >= 0 && "Failed to register get_text()");

		//r = engine->RegisterObjectMethod("XmlNode",
		//	"string& get_attribute(string &in)",
		//	asFUNCTIONPR(TiXmlNode, (std::string&), std::string&),
		//	asCALL_CDECL_OBJFIRST);
		//assert(r >= 0 && "Failed to register get_text()");
	}

	void ResourceManager::registerImageType(asIScriptEngine* engine)
	{
		RegisterResourcePointer<CL_Surface>("Image", engine);

		int r;
		// draw(x, y)
		r = engine->RegisterObjectMethod("Image",
			"void draw(float, float)",
			asFUNCTIONPR(Image_Draw, (float, float), void),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register draw()");
		// draw(x, y, angle)
		r = engine->RegisterObjectMethod("Image",
			"void draw(float, float, float)",
			asFUNCTIONPR(Image_Draw_Angle, (float, float, float), void),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register draw()");
	}

	void ResourceManager::registerSoundType(asIScriptEngine* engine)
	{
		RegisterType<CL_SoundBuffer_Session>("SoundSession", engine);

		int r;
		r = engine->RegisterObjectMethod("SoundSession",
			"void play()",
			asMETHOD(CL_SoundBuffer_Session, play),
			asCALL_THISCALL);
		assert(r >= 0 && "Failed to register play()");
		r = engine->RegisterObjectMethod("SoundSession",
			"void stop()",
			asMETHOD(CL_SoundBuffer_Session, stop),
			asCALL_THISCALL);
		assert(r >= 0 && "Failed to register stop()");
		r = engine->RegisterObjectMethod("SoundSession",
			"bool is_playing()",
			asMETHOD(CL_SoundBuffer_Session, is_playing),
			asCALL_THISCALL);
		assert(r >= 0 && "Failed to register is_playing()");


		RegisterResourcePointer<CL_SoundBuffer>("Sound", engine);

		r = engine->RegisterObjectMethod("Sound",
			"SoundSession prepare(bool)",
			asFUNCTIONPR(Sound_Prepare, (bool), CL_SoundBuffer_Session),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register prepare()");
		r = engine->RegisterObjectMethod("Sound",
			"SoundSession play(bool)",
			asFUNCTIONPR(Sound_Play, (bool), CL_SoundBuffer_Session),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register play()");
		r = engine->RegisterObjectMethod("Sound",
			"void stop()",
			asFUNCTIONPR(Sound_Stop, (void), void),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register stop()");
		r = engine->RegisterObjectMethod("Sound",
			"bool is_playing()",
			asFUNCTIONPR(Sound_IsPlaying, (void), bool),
			asCALL_CDECL_OBJFIRST);
		assert(r >= 0 && "Failed to register is_playing()");
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


	//TiXmlDocument* ResourceManager::OpenPackage(const std::string &name)
	//{
	//	if (m_PhysFSConfigured)
	//	{
	//		Find("*.xml");
	//	}
	//}

	//bool ResourceManager::LoadPackage(const std::string &name)
	//{
	//	TiXmlDocument* doc = OpenPackage(name);

	//	TiXmlNode* root = doc->RootElement();
	//	m_RootRNode.AddChildNode(createResourceNode(root));

	//	// The root node is simply called "" (nothing), thus the paths for finding
	//	//  resources always start with "/Fusion/", the initial "/" signifying the 
	//	//  trailing slash after the nameless root node ;)

	//	return true;
	//}

	//bool ResourceManager::LoadPackages(StringVector names)
	//{
	//	StringVector::iterator it;
	//	for (it = names.begin(); it != names.end(); ++it)
	//	{
	//		if (!LoadPackage(*it))
	//			return false;
	//	}

	//	return true;
	//}

	//bool ResourceManager::LoadVerified()
	//{
	//	return LoadPackages(m_VerifiedPackages);
	//}

	//void ResourceManager::ResetVerified()
	//{
	//	m_VerifiedPackages.clear();
	//}


	//RNode ResourceManager::createResourceNode(TiXmlElement* xmlNode)
	//{
	//	RNode* node;

	//	TiXmlAttribute* typeAttr = xmlNode->Attribute("type");
	//	if (typeAttr != NULL)
	//		loadResource(typeAttr->Value(), xmlNode->FirstChild()->Value());

	//	TiXmlElement *child;
	//	for (child = xmlNode->FirstChildElement(); child; child = child->NextSiblingElement())
	//	{
	//		node.AddChildNode( createResourceNode(child) );
	//	}

	//	return node;
	//}

	//void ResourceManager::loadResource(const char* type, const char* text)
	//{
	//}

	//CL_Point ResourceManager::getPoint(const CL_DomElement *element)
	//{
	//	// Return a zero point if the data is incomplete
	//	if (!(element->has_attribute("x") & element->has_attribute("y")))
	//		return CL_Point(0, 0);

	//	int x = CL_String::to_int(element->get_attribute("x"));
	//	int y = CL_String::to_int(element->get_attribute("y"));

	//	return CL_Point(x, y);
	//}


}
