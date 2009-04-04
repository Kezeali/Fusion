#include "../FusionEngine/Common.h"

#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

#include "../FusionEngine/FusionResourceManager.h"
#include "../FusionEngine/FusionImageLoader.h"
#include "../FusionEngine/FusionAudioLoader.h"
#include "../FusionEngine/FusionXMLLoader.h"

#include "../FusionEngine/FusionVirtualFileSource_PhysFS.h"

using namespace FusionEngine;

class ResourceTest 
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		CL_SetupCore setup_core;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		CL_SetupSound setup_sound;
		CL_SetupVorbis setup_vorbis;

		CL_ConsoleWindow console("ResourceManager Test");
		//console.redirect_stdio();

		CL_DisplayWindow display("ResourceManager Test: Display", 640, 480);

		Logger* logger = 0;
		ConsoleStdOutWriter* cout = 0;

		try
		{
			new Console;
			//cout = new ConsoleStdOutWriter(con);
			//cout->Activate();
			logger = new Logger(true);

			// Setup resource manager
			ResourceManager* resMan = new ResourceManager();
			resMan->Configure();

			CL_GraphicContext gc = display.get_gc();
			CL_InputContext ic = display.get_ic();
			CL_InputDevice keyboard = ic.get_keyboard();

			// Load Resources
			resMan->PreloadResource("IMAGE", L"body.png");
			ResourcePointer<CL_PixelBuffer> bodyResource = resMan->GetResource<CL_PixelBuffer>(L"body.png");
			CL_Texture bodyTexture;
			if (bodyResource.IsValid())
			{
				bodyTexture = CL_Texture(gc, bodyResource.Get()->get_width(), bodyResource.Get()->get_height());
				bodyTexture.set_image(*bodyResource.Get());
			}

			// Test multithreaded loading
			resMan->PreloadResource_Background("IMAGE", L"FusionLogo.png", 1);
			ResourcePointer<CL_PixelBuffer> logo = resMan->GetResource<CL_PixelBuffer>(L"FusionLogo.png");
			CL_Texture logoTexture;

			CL_SoundOutput sound_output(44100);

			//ResourcePointer<CL_SoundBuffer> beep = resMan->GetResource<CL_SoundBuffer>("beep.wav");
			//ResourcePointer<CL_SoundBuffer> explode = resMan->GetResource<CL_SoundBuffer>("explode.wav", "AUDIO:STREAM");
			//ResourcePointer<CL_SoundBuffer> gunshot = resMan->GetResource<CL_SoundBuffer>("gun.wav");
			//resMan->PreloadResource("AUDIO:STREAM", "cheer1.ogg", "cheer_stream");
			//ResourcePointer<CL_SoundBuffer> cheer = resMan->GetResource<CL_SoundBuffer>("cheer_stream");

			resMan->PreloadResource_Background("AUDIO:STREAM", L"ErcaMusSeg01-0.ogg");
			ResourcePointer<CL_SoundBuffer> music = resMan->GetResource<CL_SoundBuffer>(L"ErcaMusSeg01-0.ogg");

			resMan->PreloadResource_Background("AUDIO", L"SOS-150.ogg");
			ResourcePointer<CL_SoundBuffer> bigAudio = resMan->GetResource<CL_SoundBuffer>(L"SOS-150.ogg");


			resMan->AddResourceLoader("XML", LoadXml, UnloadXml);
			TiXmlNode* node = 0;
			resMan->PreloadResource("XML", L"test.xml");
			ResourcePointer<TiXmlDocument> xmlDoc = resMan->GetResource<TiXmlDocument>(L"test.xml");
			node = xmlDoc->RootElement()->FirstChild();
			if (node != NULL)
				SendToConsole(node->ToElement()->GetText());
			node = node->NextSibling("Element");
			if (node != NULL)
				SendToConsole(node->ToElement()->GetText());

			//// Try to load things that don't exist / with invalid parameters
			//try { resMan->GetResource<CL_Surface>("not_a_file.png"); }
			//catch (FileSystemException& e) {SendToConsole(e.ToString());}
			//try { resMan->GetResource<CL_Surface>("a.png", "PICTURE"); }
			//catch (FileSystemException& e) {SendToConsole(e.ToString());}
			//try { resMan->GetResource<int>("b.png"); }
			//catch (FileSystemException& e) {SendToConsole(e.ToString());}

			//// Load a file normally (to make sure sound output actually works)
			//CL_SoundBuffer b1("beep.wav");
			//b1.play(false, &sound_output);

			bool resetting = false;
			bool notPlaying = true;

			unsigned int lastframe = CL_System::get_time();
			unsigned int split = 0;
			// Loop thing
			while (!keyboard.get_keycode(CL_KEY_ESCAPE))
			{
				gc.clear(CL_Colorf(0.2f, 0.7f, 1.0f));

				split = CL_System::get_time() - lastframe;
				lastframe = CL_System::get_time();

				// Start music if it has finished loading
				if (music.IsValid() && notPlaying)
				{
					music->play(true);
					notPlaying = false;
				}
				else if (notPlaying)
					CL_Console::write_line(CL_StringHelp::int_to_text((int)lastframe) + L" Music not loaded");

				// update / draw
				if (bodyResource.IsValid())
				{
					gc.set_texture(0, bodyTexture);
					CL_Draw::texture(gc, CL_Rectf(bodyTexture.get_size()));
				}
				if (logo.IsValid())
				{
					if (logoTexture.is_null())
					{
						logoTexture = CL_Texture(gc, logo.Get()->get_width(), logo.Get()->get_height());
						logoTexture.set_image(*logo.Get());
					}

					gc.set_texture(0, logoTexture);
					CL_Draw::texture(gc, CL_Rectf(CL_Point(70, 100), logoTexture.get_size()));
				}
				else
					CL_Console::write_line(L"Texture not loaded");

				//if (CL_Keyboard::get_keycode('B'))
				//	beep->play();
				//if (CL_Keyboard::get_keycode('E'))
				//	explode->play();
				//if (CL_Keyboard::get_keycode('C'))
				//	cheer->play();
				//if (CL_Keyboard::get_keycode('G'))
				//	gunshot->play();

				if (!resetting && keyboard.get_keycode('D'))
				{
					CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
					CL_VirtualDirectoryListing vdirlisting = vdir.get_directory_listing();

					while (vdirlisting.next())
					{
						std::wstring outputLine = L"";

						if (vdirlisting.is_directory())
							outputLine += L"[dir]\t";
						else
							outputLine += L"\t";

						outputLine += vdirlisting.get_filename() + L" ";

						if (vdirlisting.is_readable())
							outputLine += L"r";
						if (vdirlisting.is_writable())
							outputLine += L"w";

						CL_Console::write_line(outputLine.c_str());
						SendToConsole(outputLine);
					}

					resetting = true;
				}

				if (!resetting && keyboard.get_keycode('S'))
				{
					resMan->StartBackgroundPreloadThread();

					resetting = true;
				}

				if (!resetting && keyboard.get_keycode('L') && bigAudio.IsValid())
				{
					CL_Console::write_line(L"Music started");
					music->stop();
					bigAudio->play();
					
					resetting = true;
				}
				if (!bigAudio.IsValid())
					CL_Console::write_line(CL_StringHelp::int_to_text((int)lastframe) + L" Big audio not loaded");

				if (resetting && !keyboard.get_keycode('L') && !keyboard.get_keycode('S') && !keyboard.get_keycode('D'))
				{
					resetting = false;
				}

				//if (CL_Keyboard::get_keycode(CL_KEY_BACKSPACE) != resetting && (resetting = !resetting))
				//{
				//	if (logo.IsValid())
				//	{
				//		logo.Release();
				//	}
				//	else
				//	{
				//		logo = resMan->GetResource<CL_Surface>("FusionLogo.png");
				//	}
				//	resMan->DisposeUnusedResources();
				//}

				if (CL_DisplayMessageQueue::has_messages())
					CL_DisplayMessageQueue::process();

				display.flip();
				CL_System::sleep(4);
			}

			//delete stateman;
			//delete conGUI;
		}
		catch (CL_Exception& e)
		{
			// Something bad must have happened
			std::wcout << e.message.c_str() << std::endl;
			console.display_close_message();
		}

		if (logger != 0)
			delete logger;
		if (cout != 0)
			delete cout;
		delete Console::getSingletonPtr();

		// Zero!
		return 0;
	}
};

CL_ClanApplication app(&ResourceTest::main);
