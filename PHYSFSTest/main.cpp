#include "..\FusionEngine\Common.h"

#include "..\FusionEngine\FusionPhysFS.h"

#include "..\FusionEngine\PhysFS.h"

//using namespace FusionEngine;

class PhysFSTest : public CL_ClanApplication
{
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
			const PHYSFS_ArchiveInfo **i;
			for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; i++)
			{
				std::cout << "Supported archive: " << (*i)->extension << " which is "
					<< (*i)->description << "." << std::endl;
			}

			// Configure physFS for this app
			SetupPhysFS::configure("Fusion Project Team", "Test", "ZIP");
			SetupPhysFS::add_subdirectory("Data/", "ZIP", true);


			// Display the current search path
			char **it ;
			for (it = PHYSFS_getSearchPath(); *it != NULL; it++)
			{
				std::cout << (*it) << std::endl;
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
