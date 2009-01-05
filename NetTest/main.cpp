#include "../FusionEngine/Common.h"

//#define BOOST_CB_DISABLE_DEBUG // Allows overwritten CB iterators to remain valid
//#include <boost/circular_buffer.hpp>

#include "App.h"

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		CL_SetupCore setup_core;
		CL_SetupDisplay setup_disp;
		CL_SetupGL setup_gl;

		CL_ConsoleWindow console(L"Net Test");

		try
		{
			CL_DisplayWindow display("Net Test: Display", 640, 480);

			bool retVal;
			{
				TestApp testApp;
				retVal = testApp.run(args, display);
			}

			display.get_gc().set_font(CL_Font());
			return retVal;
		}
		catch(CL_Exception& exception)
		{
			CL_Console::write_line(exception.message);
			console.display_close_message();
			return 1;
		}
	}
};

CL_ClanApplication app(&EntryPoint::main);

