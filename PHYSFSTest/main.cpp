#include "..\FusionEngine\FusionCommon.h"

#include "..\FusionEngine\FusionPhysFS.h"
#include "..\FusionEngine\FusionConsole.h"
#include "..\FusionEngine\FusionConsoleStdOutWriter.h"
#include "..\FusionEngine\FusionLogger.h"

#include "..\FusionEngine\PhysFS.h"

using namespace FusionEngine;

class PhysFSTest
{

	bool m_PhysFSConfigured;


	// This the the purest form of the Find method
	StringVector Find(const std::string &expression)
	{
		StringVector list;

		if (m_PhysFSConfigured)
		{
			CL_RegExp regExp(expression.c_str());

			char **files = PHYSFS_enumerateFiles("");
			if (files != NULL)
			{
				int file_count;
				char **i;
				for (i = files, file_count = 0; *i != NULL; i++, file_count++)
				{
					std::string file(*i);
					if (regExp.search(file.c_str(), file.length()).is_match())
						list.push_back(file);
				}

				PHYSFS_freeList(files);
			}
		}

		return list;
	}

	typedef std::map<std::string, std::string> VarMap;

	//void insertVarMapIntoDOM(ticpp::Element &parent, VarMap vars)
	//{
	//	for (VarMap::iterator it = vars.begin(), end = vars.end(); it != end; ++it)
	//	{
	//		//TiXmlElement *var = new TiXmlElement("var");
	//		ticpp::Element var("var");
	//		var.SetAttribute("name", it->first.c_str());
	//		var.SetAttribute("value", it->second.c_str());
	//		parent.LinkEndChild( &var );
	//	}
	//}

	//void testOutput()
	//{
	//	//TiXmlDocument doc;
	//	ticpp::Document doc;

	//	// Decl
	//	//TiXmlDeclaration* decl = new TiXmlDeclaration( XML_STANDARD, "", "" );  
	//	ticpp::Declaration decl( XML_STANDARD, "", "" );
	//	doc.LinkEndChild( &decl ); 

	//	// Root
	//	//TiXmlElement * root = new TiXmlElement("clientoptions");  
	//	ticpp::Element root("clientoptions");
	//	doc.LinkEndChild( &root );

	//	{
	//		VarMap vars;
	//		vars["fullscreen"] = "false";

	//		insertVarMapIntoDOM(root, vars);
	//	}

	//	{
	//		int plNum = 4;
	//		VarMap vars[5];
	//		vars[0]["name"] = "Player";
	//		vars[0]["hud"] = "true";
	//		vars[1]["name"] = "Sam";
	//		vars[1]["hud"] = "false";
	//		vars[2]["name"] = "Lara";
	//		vars[2]["hud"] = "true";
	//		vars[3]["name"] = "Bill";
	//		vars[3]["hud"] = "true";
	//		vars[4]["name"] = "Phil";
	//		vars[4]["hud"] = "false";

	//		for (int i = 0; i <= plNum; ++i)
	//		{
	//			//TiXmlElement* player = new TiXmlElement( "playeroptions" );  
	//			ticpp::Element player("playeroptions");
	//			root.LinkEndChild( &player ); 

	//			std::string playerAttribute;
	//			if (i == 0)
	//				playerAttribute = "default";
	//			else
	//				playerAttribute = CL_String::from_int(i);
	//			player.SetAttribute("player", playerAttribute.c_str());

	//			insertVarMapIntoDOM(player, vars[i]);
	//		}
	//	}

	//	// Save to a file
	//	std::string filename("test.xml");

	//	doc.SaveFile(filename);
	//}

public:
	virtual int main(const std::vector<CL_String> &args)
	{
		CL_SetupCore core_setup;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		SetupPhysFS physfs_setup;

		CL_ConsoleWindow console("PhysFS Test");
		//console.redirect_stdio();

		CL_DisplayWindow display("PhysFS Test: Display", 480, 200);

		try
		{
			CL_GraphicContext gc = display.get_gc();

			using namespace FusionEngine;
			new Console;
			ConsoleStdOutWriter* cout = new ConsoleStdOutWriter();
			cout->Activate();
			Logger* logger = new Logger(true);

			// List version info
			PHYSFS_Version compiled;
			PHYSFS_Version linked;
			PHYSFS_VERSION(&compiled);
			PHYSFS_getLinkedVersion(&linked);

			printf("Compiled against PhysFS version %d.%d.%d.\n",
				compiled.major, compiled.minor, compiled.patch);
			printf("Linked against PhysFS version %d.%d.%d.\n",
				linked.major, linked.minor, linked.patch);

			// List filetypes
			SendToConsole("PhysFS File Types:");

			const PHYSFS_ArchiveInfo **i;
			for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; i++)
			{
				//std::cout << "Supported archive: " << (*i)->extension << " which is "
				//	<< (*i)->description << "." << std::endl;

				//FusionEngine::Console::getSingletonPtr()->Add((*i)->description);
				SendToConsole("\t" + std::string((*i)->description));
			}

			//{
			//	std::ofstream file("error.log", std::ios::app|std::ios::out);
			//	file << "test";
			//}

			// Configure physFS for this app
			SetupPhysFS::configure("Fusion Project Team", "Test", "ZIP");
			SetupPhysFS::add_subdirectory("Data/", "ZIP", true);

			///////////////////
			m_PhysFSConfigured = true;
			///////////////////


			// Display the current search path
			SendToConsole("PHYSFS search path:");
			char **it ;
			for (it = PHYSFS_getSearchPath(); *it != NULL; it++)
			{
				SendToConsole("\t" + std::string(*it));
			}

			SendToConsole("Testing PHYSFS Find - Searching for '*body*':");
			StringVector xmlFiles = Find("*body*");
			for (int i = 0; i < xmlFiles.size(); i++)
			{
				SendToConsole("\t" + xmlFiles[i]);
			}


			// Set up the PhysFS / clanlib virtual file system
			CL_VirtualFileSystem vfs_physfs(new VirtualFileSource_PhysFS());

			CL_VirtualDirectory vdir(vfs_physfs, "");

			// Initialise some stuff...
			// .. The PhysFS way
			CL_Texture surface(gc, "PhysFSBody.png", vdir);

			// ... The standard way
			CL_Texture surface2(gc, "Body.png");

			//testOutput();



			delete logger;
			delete Console::getSingletonPtr();

			CL_InputDevice keyboard = display.get_ic().get_keyboard();

			// Draw!
			while (!keyboard.get_keycode(CL_KEY_ESCAPE))
			{
				gc.clear(CL_Colorf(0.51f, 0.84f, 0.9f));

				gc.set_texture(0, surface);
				CL_Draw::texture(gc, CL_Rectf(CL_Pointf(10, 20), surface.get_size()));

				gc.set_texture(0, surface2);
				CL_Draw::texture(gc, CL_Rectf(CL_Pointf(70, 20), surface2.get_size()));

				if (CL_DisplayMessageQueue::has_messages())
					CL_DisplayMessageQueue::process();

				display.flip();
			}
		}
		catch (CL_Exception& e)
		{
			// O noes!
			CL_Console::write_line( e.message );
			console.wait_for_key();
		}

		// It's so zen
		return 0;
	}
};

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		PhysFSTest app;
		return app.main(args);
	}
};

CL_ClanApplication app(&EntryPoint::main);
