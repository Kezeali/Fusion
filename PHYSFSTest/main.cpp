#include "..\FusionEngine\Common.h"

#include "..\FusionEngine\FusionPhysFS.h"
#include "..\FusionEngine\FusionConsole.h"
#include "..\FusionEngine\FusionConsoleStdOutWriter.h"
#include "..\FusionEngine\FusionLogger.h"

#include "..\FusionEngine\PhysFS.h"

using namespace FusionEngine;

class PhysFSTest : public CL_ClanApplication
{

	bool m_PhysFSConfigured;
	
	StringVector TokeniseExpression(const std::string &expression)
	{
		StringVector expressionTokens;
		StringVector expressionTokens1, expressionTokens2;

		size_t mid = expression.find("*");

		// If more tokens are found
		if (mid != std::string::npos)
		{
			expressionTokens1 = TokeniseExpression( expression.substr(0, mid) );
			expressionTokens2 = TokeniseExpression( expression.substr(mid+1) );

			expressionTokens.resize(expressionTokens1.size() + expressionTokens2.size());

			std::copy(expressionTokens1.begin(), expressionTokens1.end(), expressionTokens.begin());
			std::copy(expressionTokens2.begin(), expressionTokens2.end(), expressionTokens.begin()+expressionTokens1.size());


			//std::string token1 = expression.substr(0, mid);
			//std::string token2 = expression.substr(mid+1);

			//if (!token1.empty())
			//	expressionTokens.push_back( token1 );
			//if (!token2.empty())
			//	expressionTokens.push_back( token2 );
		}
		else if (!expression.empty())
			expressionTokens.push_back(expression);

		return expressionTokens;
	}

	bool CheckAgainstExpression(const std::string &str, const std::string &expression)
	{
		CheckAgainstExpression(str, TokeniseExpression(expression));
	}

	bool CheckAgainstExpressionWithOptions(const std::string &str, StringVector expressionTokens)
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

	bool CheckAgainstExpression(const std::string &str, StringVector expressionTokens)
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

	// This the the purest form of the Find method
	StringVector Find(const std::string &expression)
	{
		StringVector list;

		StringVector expressionTokens = TokeniseExpression(expression);

		if (m_PhysFSConfigured)
		{
			char **files = PHYSFS_enumerateFiles("");
			if (files != NULL)
			{
				int file_count;
				char **i;
				for (i = files, file_count = 0; *i != NULL; i++, file_count++)
				{
					std::string file(*i);
					if (CheckAgainstExpression(file, expressionTokens))
						list.push_back(file);
				}

				PHYSFS_freeList(files);
			}
		}

		return list;
	}

	// This has been replaced with Find(<ex>, false) in ResourceManager
	StringVector FindCaseless(const std::string &expression)
	{
		StringVector list;

		StringVector expressionTokens = TokeniseExpression(fe_newupper(expression));

		if (m_PhysFSConfigured)
		{
			char **files = PHYSFS_enumerateFiles("");
			if (files != NULL)
			{
				int file_count;
				char **i;
				for (i = files, file_count = 0; *i != NULL; i++, file_count++)
				{
					std::string file(*i);
					if (CheckAgainstExpression(fe_newupper(file), expressionTokens))
						list.push_back(file);
				}

				PHYSFS_freeList(files);
			}
		}

		return list;
	}

	virtual int main(int argc, char **argv)
	{
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		SetupPhysFS physfs_setup(argv[0]);

		CL_ConsoleWindow console("PhysFS Test");
		console.redirect_stdio();

		CL_DisplayWindow display("PhysFS Test: Display", 640, 200);

		try
		{
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

			Exception* trivial = 
				new Exception(Exception::TRIVIAL, "An error is about to happen");
			Exception* err = 
				new Exception(Exception::INTERNAL_ERROR, "Just kidding :)");

			logger->SetUseDating(true);
			logger->BeginLog("another log", false);
			// Add a normal message
			logger->Add("hello", "another log");
			// Add an error
			logger->Add(err, "another log");
			logger->EndLog("another log");
			logger->RemoveAndDestroyLog("another log");

			// Send a warning to the console
			SendToConsole(trivial);
			// Send an error to the console
			SendToConsole(err);

			// List filetypes
			logger->Add("PhysFS File Types:", "console");

			const PHYSFS_ArchiveInfo **i;
			for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; i++)
			{
				//std::cout << "Supported archive: " << (*i)->extension << " which is "
				//	<< (*i)->description << "." << std::endl;

				//FusionEngine::Console::getSingletonPtr()->Add((*i)->description);
				SendToConsole((*i)->description);
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
			char **it ;
			for (it = PHYSFS_getSearchPath(); *it != NULL; it++)
			{
				SendToConsole((*it));
			}

			SendToConsole("Searching for '*.xml':");
			StringVector xmlFiles = FindCaseless("*.xml");
			for (int i = 0; i < xmlFiles.size(); i++)
			{
				SendToConsole("\t" + xmlFiles[i]);
			}


			// Get an instance of the custom inputsource
			InputSourceProvider_PhysFS phys_provider("");

			// Initialise some stuff...
			// .. The PhysFS way
			CL_PNGProvider phys_png("PhysFSBody.png", &phys_provider);
			CL_Surface surface(phys_png);

			// ... The standard way
			CL_PNGProvider png("Body.png");
			CL_Surface surface2(png);
			// ... And finally the shorthand version of the standard way
			CL_Surface surface3("Body.png");


			delete logger;
			delete Console::getSingletonPtr();

			// Draw!
			while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
			{
				display.get_gc()->clear(CL_Color(180, 220, 255));

				surface.draw(10, 20);
				surface2.draw(70, 20);
				surface3.draw(106, 20);

				display.flip();
				CL_System::keep_alive(4);
			}
		}
		catch (CL_Error e)
		{
			// O noes!
			std::cout << e.message << std::endl;
			console.wait_for_key();
		}

		// It's so zen
		return 0;
	}
} app;
