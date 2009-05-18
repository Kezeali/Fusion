#include "..\FusionEngine\FusionCommon.h"

#include "..\FusionEngine\FusionPhysFS.h"
#include "..\FusionEngine\FusionConsole.h"
#include "..\FusionEngine\FusionConsoleStdOutWriter.h"
#include "..\FusionEngine\FusionLogger.h"

#include "..\FusionEngine\PhysFS.h"

#include <boost/bind.hpp>

using namespace FusionEngine;

static std::string helloworld(const StringVector &args)
{
	std::string command = "";
	for (StringVector::const_iterator it = args.begin(), end = args.end(); it != end; ++it)
		command += *it + "|";
	return "Hello World. Love from <" + command + ">";
}

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
				char **i;
				for (i = files; *i != NULL; i++)
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

			// Configure physFS for this app
			SetupPhysFS::configure("Pom", "Test", "ZIP");
			SetupPhysFS::add_subdirectory("Data/", "ZIP", true);
			///////////////////
			m_PhysFSConfigured = true;
			///////////////////

			// Create console singleton
			new Console;
			// Create console writer
			ConsoleStdOutWriter cout;
			cout.Enable();
			// Create logger singleton
			Logger* logger = new Logger();

			//
			// Test log writing
			logger->OpenLog("console_test", LOG_NORMAL);
			logger->Add("First Message", "console_test", LOG_CRITICAL);
			logger->SetLogingToConsole("console_test", true);
			logger->Add("Second Message", "console_test", LOG_CRITICAL);
			logger->Add("Third (trivial) Message", "console_test", LOG_TRIVIAL);
			logger->Add("Fourth Message (yes, third should be missing)", "console_test", LOG_CRITICAL);
			logger->Add("Fifth Message", "console_test", LOG_NORMAL);
			SendToConsole(L"Intermission");
			logger->Add("Sixth Message", "console_test", LOG_CRITICAL);

			//
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

			//
			// Display the current search path
			SendToConsole("PHYSFS search path:");
			char **it ;
			for (it = PHYSFS_getSearchPath(); *it != NULL; it++)
			{
				SendToConsole("\t" + std::string(*it));
			}

			SendToConsole("Testing PHYSFS Find - Searching for '.Body.':");
			StringVector xmlFiles = Find(".Body.");
			for (unsigned int i = 0; i < xmlFiles.size(); i++)
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


			CL_InputDevice keyboard = display.get_ic().get_keyboard();

			CL_InputDevice mouse = display.get_ic().get_mouse();

			m_AxisEventMessage = cl_text("Axis: no events received");
			m_BallEventMessage = cl_text("Ball: no events received");
			m_PointEventMessage = cl_text("Pointer: no events received");

			CL_SlotContainer eventSlotContainer;
			eventSlotContainer.connect(mouse.sig_axis_move(), this, &PhysFSTest::onInputEvent);
			eventSlotContainer.connect(mouse.sig_ball_move(), this, &PhysFSTest::onInputEvent);
			eventSlotContainer.connect(mouse.sig_pointer_move(), this, &PhysFSTest::onInputEvent);

			// Set up the console interpreter
			Console *con = Console::getSingletonPtr();
			con->BindCommand("test", boost::bind(&helloworld, _1));
			eventSlotContainer.connect(keyboard.sig_key_up(), this, &PhysFSTest::onKeyUp);
			CL_Console::write("> ");

			CL_FontDescription desc;
			desc.set_typeface_name("verdana");
			desc.set_height(14);
			CL_Font font(gc, desc);

			gc.set_font(font);

			// Draw!
			while (!keyboard.get_keycode(CL_KEY_ESCAPE))
			{
				gc.clear(CL_Colorf(0.51f, 0.84f, 0.9f));

				gc.set_texture(0, surface);
				CL_Draw::texture(gc, CL_Rectf(CL_Pointf(10, 20), surface.get_size()));

				gc.set_texture(0, surface2);
				CL_Draw::texture(gc, CL_Rectf(CL_Pointf(50, 20), surface2.get_size()));

				if (CL_DisplayMessageQueue::has_messages())
					CL_DisplayMessageQueue::process();

				gc.draw_text(80, 100, m_ConsoleLine, CL_Colorf::darkgray);

				gc.draw_text(80, 16, m_AxisEventMessage, CL_Colorf::black);
				gc.draw_text(80, 34, m_BallEventMessage);
				gc.draw_text(80, 52, m_PointEventMessage, CL_Colorf::black);

				display.flip();
			}

			delete logger;
			delete Console::getSingletonPtr();
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

	CL_String m_ConsoleLine;

	void onKeyUp(const CL_InputEvent &event, const CL_InputState &state)
	{
		if (event.id != VK_RETURN)
		{
			m_ConsoleLine += event.str;
			CL_Console::write(event.str);
		}
		else
		{
			Console* console = Console::getSingletonPtr();
			if (console != NULL)
			{
				CL_Console::write("\n ");
				console->Interpret(CL_StringHelp::text_to_utf8(m_ConsoleLine));
			}
			CL_Console::write("\n> ");
			m_ConsoleLine.clear();
		}
	}

	CL_String m_AxisEventMessage;
	CL_String m_BallEventMessage;
	CL_String m_PointEventMessage;

	void onInputEvent(const CL_InputEvent &event, const CL_InputState &state)
	{
		if (event.type == CL_InputEvent::axis_moved)
		{
			m_AxisEventMessage = cl_format("Axis: id: %1  value: %2", event.id, event.axis_pos);
		}
		else if (event.type == CL_InputEvent::ball_moved)
		{
			m_BallEventMessage = cl_format("Ball: id: %1  value: %2", event.id, event.axis_pos);
		}
		else if (event.type == CL_InputEvent::pointer_moved)
		{
			m_PointEventMessage = cl_format("Pointer: id: %1  x:%2  y:%3", event.id, event.mouse_pos.x, event.mouse_pos.y);
		}
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
