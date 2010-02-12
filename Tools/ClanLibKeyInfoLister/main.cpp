#define Header_FusionEngine_Hashable
#include "../../FusionEngine/FusionCommon.h"

#ifdef _WIN32
#include "../../FusionEngine/FusionXInputController.h"
#endif

#include <boost/functional/hash.hpp>

static const int s_DevKeyboard = 0;
static const int s_DevGamepad = 1;
static const int s_DevGamepad_Axis = 2;
static const int s_DevMouse = 3;
static const int s_DevMouse_Pointer = 4;
static const int s_DevMouse_Axis = 5;
static const int s_DevXInput = 10;
static const int s_DevXInput_Axis = 11;

static const char* s_DevKeyboardStr = "keyboard";
static const char* s_DevGamepadStr = "gamepad";
static const char* s_DevGamepad_AxisStr = "gamepad-axis";
static const char* s_DevMouseStr = "mouse";
static const char* s_DevMouse_PointerStr = "mouse-pointer";
static const char* s_DevMouse_AxisStr = "mouse-axis";
static const char* s_DevXInputStr = "xinput";
static const char* s_DevXInput_AxisStr = "xinput-axis";

static inline int DeviceNameToID(const std::string& device)
{
	if (device == s_DevKeyboardStr)
		return s_DevKeyboard;

	else if (device == s_DevGamepadStr)
		return s_DevGamepad;
	else if (device == s_DevGamepad_AxisStr)
		return s_DevGamepad_Axis;

	else if (device == s_DevMouseStr)
		return s_DevMouse;
	else if (device == s_DevMouse_PointerStr)
		return s_DevMouse_Pointer;
	else if (device == s_DevMouse_AxisStr)
		return s_DevMouse_Axis;

	else if (device == s_DevXInputStr)
		return s_DevXInput;
	else if (device == s_DevXInput_AxisStr)
		return s_DevXInput_Axis;
}

static inline const std::string& DeviceIDToName(int device)
{
	if (device == s_DevKeyboard)
		return s_DevKeyboardStr;

	else if (device == s_DevGamepad)
		return s_DevGamepadStr;
	else if (device == s_DevGamepad_Axis)
		return s_DevGamepad_AxisStr;

	else if (device == s_DevMouse)
		return s_DevMouseStr;
	else if (device == s_DevMouse_Pointer)
		return s_DevMouse_PointerStr;
	else if (device == s_DevMouse_Axis)
		return s_DevMouse_AxisStr;

	else if (device == s_DevXInput)
		return s_DevXInputStr;
	else if (device == s_DevXInput_Axis)
		return s_DevXInput_AxisStr;
}

static const int s_DeviceIndexAny = 255;

struct BindingKey
{
	int device;
	int index;
	int code;

	BindingKey()
		: device(0),
		index(0),
		code(0)
	{
	}

	BindingKey(int device_type, int index, int code)
		: device(device_type),
		index(index),
		code(code)
	{
	}

	BindingKey(const std::string& device_type_name, int index, int code)
		: device(DeviceNameToID(device_type_name)),
		index(index),
		code(code)
	{
	}
};

static bool operator== (BindingKey const& l, BindingKey const& r)
{
	return l.device == r.device && l.index == r.index && l.code == r.code;
}

static bool operator< (BindingKey const& l, BindingKey const& r)
{
	if (l.device < r.device)
		return true;
	if (r.device < l.device) // i.e. Only continue to compare 'index' if l.device and r.device are equal
		return false;

	if (l.index < r.index)
		return true;
	if (r.index < l.index)
		return false;

	if (l.code < r.code)
		return true;
	if (r.code < l.code)
		return false;

	// All elements are equal
	return false;
}

namespace std { namespace tr1
{
	template <>
	struct hash<BindingKey> : public unary_function<BindingKey, size_t>
	{
		size_t operator()(const BindingKey& key) const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, key.device);
			boost::hash_combine(seed, key.index);
			boost::hash_combine(seed, key.code);

			return seed;
		}
	};
}}


class KeyInfoGenerator
{
protected:
	std::string filename;
	CL_InputContext ic;
	CL_SlotContainer slots;

	class KeyInfo
	{
	public:
		KeyInfo()
			: m_Name(""),
			m_Device(""),
			m_Code("0"),
			m_Description("")
		{
		}

		KeyInfo(std::string name, std::string device, int code, std::string description)
			: m_Name(name),
			m_Device(device),
			m_Code(CL_StringHelp::int_to_local8(code)),
			m_Description(description)
		{
		}

		// String mapped to this key (used in config files, internally, etc.)
		std::string m_Name;
		// Control device
		std::string m_Device;
		// Scancode, button ID - whatever the device uses to identify this input
		std::string m_Code;
		// Shown in the UI
		std::string m_Description;
	};


	static BindingKey DevCodePair(const std::string& device, int code)
	{
		return BindingKey(device, s_DeviceIndexAny, code);
	}

	typedef std::map<BindingKey, KeyInfo> KeyInfoMap;

	KeyInfoMap keyinfos;

	std::vector<FusionEngine::XInputController> xInputControllers;
	//std::vector<CL_InputDevice> xInputDevices;

public:
	KeyInfoGenerator(CL_InputContext ic)
		: filename("keyinfo.xml"),
		ic(ic)
	{
		// Grab XInput controllers
		for (int i = 0; i < XUSER_MAX_COUNT; i++)
		{
			FusionEngine::XInputController controller(i);
			// Connect to event signals
			//CL_InputDevice inputDevice( new FusionEngine::InputDeviceProvider_XInput(i) );
			//slots.connect(controller.sig_axis_move, this, &KeyInfoGenerator::onXInputAxis);
			slots.connect(controller.sig_key_up, this, &KeyInfoGenerator::onXInputUp);
			//xInputDevices.push_back( inputDevice );
			//ic.add_joystick( inputDevice );
			xInputControllers.push_back( controller );
		}

		if (ic.get_keyboard_count() > 0)
		{
			for (int i = 0; i < ic.get_keyboard_count(); i++)
			{
				//slots.connect(ic.get_keyboard(i).sig_key_down(), this, &KeyInfoGenerator::onKeyDown);
				slots.connect(ic.get_keyboard(i).sig_key_up(), this, &KeyInfoGenerator::onKeyUp);
			}
		}
		if (ic.get_mouse_count() > 0)
		{
			for (int i = 0; i < ic.get_mouse_count(); i++)
			{
				//slots.connect(ic.get_mouse(i).sig_axis_move(), this, &KeyInfoGenerator::onMouseAxis);
				//slots.connect(ic.get_mouse(i).sig_ball_move(), this, &KeyInfoGenerator::onMouseBall);
				//slots.connect(ic.get_mouse(i).sig_key_down(), this, &KeyInfoGenerator::onMouseDown);
				slots.connect(ic.get_mouse(i).sig_key_up(), this, &KeyInfoGenerator::onMouseUp);
				//slots.connect(ic.get_mouse(i).sig_pointer_move(), this, &KeyInfoGenerator::onMouseMove);
			}
		}
		if (ic.get_joystick_count() > 0)
		{
			for (int i = 0; i < ic.get_joystick_count(); i++)
			{
				CL_Console::write_line(L"Gamepad %1: %2", i, ic.get_joystick(i).get_device_name());
				slots.connect(ic.get_joystick(i).sig_axis_move(), this, &KeyInfoGenerator::onGamepadAxis);
				//slots.connect(ic.get_joystick(i).sig_key_down(), this, &KeyInfoGenerator::onGamepadDown);
				slots.connect(ic.get_joystick(i).sig_key_up(), this, &KeyInfoGenerator::onGamepadUp);
			}
		}
	}

	void load()
	{
		try
		{
			ticpp::Document* doc = new ticpp::Document();
			doc->LoadFile(filename);
			ticpp::Element* elem = doc->FirstChildElement();

			if (elem->Value() != "keyinfo")
			{
				CL_Console::write_line("keyinfo file is empty or invalid");
				return;
			}

			ticpp::Iterator< ticpp::Element > child( "key" );
			for ( child = child.begin( elem ); child != child.end(); child++ )
			{
				KeyInfo kinfo;
				kinfo.m_Name = child->GetAttribute("shortname");
				kinfo.m_Device = child->GetAttribute("device");
				kinfo.m_Code = child->GetAttributeOrDefault("id", "0");
				kinfo.m_Description = child->GetAttributeOrDefault("name", kinfo.m_Name);

				//if (kinfo.m_Name.empty() || kinfo.m_Device.empty())
				//	CL_Console::write_line("keyinfo file is invalid");

				int code = CL_StringHelp::local8_to_int(kinfo.m_Code.c_str());
				keyinfos[DevCodePair(kinfo.m_Device, code)] = kinfo;
			}
		}
		catch (ticpp::Exception &ex)
		{
		}

		CL_Console::write_line(L"Loaded %1 keys from %2", keyinfos.size(), filename.c_str());
	}

	void save()
	{
		TiXmlDocument *doc = new TiXmlDocument(filename);
		
		// Decl
		TiXmlDeclaration *decl = new TiXmlDeclaration( XML_STANDARD, "utf-8", "" );
		doc->LinkEndChild( decl ); 

		// Root
		TiXmlElement* root = new TiXmlElement("keyinfo");
		doc->LinkEndChild( root );

		// Save KeyInfo objects
		for (KeyInfoMap::iterator it = keyinfos.begin(), end = keyinfos.end();
			it != end; ++it)
		{
			const KeyInfo &keyInfo = it->second;

			// Create the key bind
			TiXmlElement* keyElement = new TiXmlElement("key");

			keyElement->SetAttribute("shortname", keyInfo.m_Name);
			keyElement->SetAttribute("device", keyInfo.m_Device);
			keyElement->SetAttribute("id", keyInfo.m_Code);
			keyElement->SetAttribute("name", keyInfo.m_Description);

			root->LinkEndChild(keyElement);
		}

		doc->SaveFile();
	}

	void poll()
	{
		for (std::vector<FusionEngine::XInputController>::iterator it = xInputControllers.begin(),
			end = xInputControllers.end(); it != end; ++it)
		{
			it->Poll();
		}
	}

	static inline CL_String string2clantext(const std::string &str)
	{
		return CL_StringHelp::local8_to_text(str.c_str());
	}

	void onKeyUp(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyInfoMap::iterator _where = keyinfos.find(DevCodePair("keyboard", event.id));
		if (_where == keyinfos.end())
		{
			CL_Console::write_line(L"KeyInfo:\n\tshortname: " + event.str + 
				"\n\tdevice: keyboard \n\tcode: " + CL_StringHelp::int_to_text(event.id) + 
				"\n\tdescription: " + event.device.get_key_name(event.id)
				);

			std::string shortname(CL_StringHelp::text_to_local8(event.str));
			std::string desc(CL_StringHelp::text_to_local8(event.device.get_key_name(event.id)));
			keyinfos[DevCodePair("keyboard", event.id)] = KeyInfo(shortname, "keyboard", event.id, desc);
		}
		else
		{
			CL_Console::write_line(cl_format(
				L"The key pressed has been listed before. Database contains the following info:\n" \
				L"KeyInfo:\n\tshortname: %1" \
				L"\n\tdevice: keyboard \n\tcode: %2" \
				L"\n\tdescription: %3",
				string2clantext(_where->second.m_Name),
				string2clantext(_where->second.m_Code),
				string2clantext(_where->second.m_Description)
				));
		}
	}

	void onMouseUp(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyInfoMap::iterator _where = keyinfos.find(DevCodePair("mouse", event.id));
		if (_where == keyinfos.end())
		{
			CL_Console::write_line(L"KeyInfo:\n\tshortname: \n\tdevice: mouse \n\tcode: " +
				CL_StringHelp::int_to_text(event.id) + 
				"\n\tdescription: " + event.device.get_key_name(event.id)
				);

			std::string desc(CL_StringHelp::text_to_local8(event.device.get_key_name(event.id)));
			keyinfos[DevCodePair("mouse", event.id)] = KeyInfo("", "mouse", event.id, desc);
		}
		else
		{
			CL_Console::write_line(cl_format(
				L"The key pressed has been listed before. Database contains the following info:\n" \
				L"KeyInfo:\n\tshortname: %1" \
				L"\n\tdevice: keyboard \n\tcode: %2" \
				L"\n\tdescription: %3",
				string2clantext(_where->second.m_Name),
				string2clantext(_where->second.m_Code),
				string2clantext(_where->second.m_Description)
				));
		}
	}

	void onGamepadAxis(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyInfoMap::iterator _where = keyinfos.find(DevCodePair("gamepad-axis", event.id));
		if (_where == keyinfos.end())
		{
			CL_Console::write_line(L"KeyInfo:\n\tshortname: \n\tdevice: gamepad-axis \n\tcode: " +
				CL_StringHelp::int_to_text(event.id) + 
				"\n\tdescription: "
				);

			keyinfos[DevCodePair("gamepad-axis", event.id)] = KeyInfo("", "gamepad-axis", event.id, "");
		}
		else
		{
			CL_Console::write_line(cl_format(
				L"The key pressed has been listed before. Database contains the following info:\n" \
				L"KeyInfo:\n\tshortname: %1" \
				L"\n\tdevice: keyboard \n\tcode: %2" \
				L"\n\tdescription: %3",
				string2clantext(_where->second.m_Name),
				string2clantext(_where->second.m_Code),
				string2clantext(_where->second.m_Description)
				));
		}
	}

	void onGamepadUp(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyInfoMap::iterator _where = keyinfos.find(DevCodePair("gamepad", event.id));
		if (_where == keyinfos.end())
		{
			CL_Console::write_line(L"KeyInfo:\n\tshortname: " + event.str + 
				"\n\tdevice: gamepad" + 
				"\n\tcode: " + CL_StringHelp::int_to_text(event.id) + 
				"\n\tdescription: " + event.device.get_key_name(event.id)
				);

			std::string desc(CL_StringHelp::text_to_local8(event.device.get_key_name(event.id)));
			keyinfos[DevCodePair("gamepad", event.id)] = KeyInfo("", "gamepad", event.id, desc);
		}
		else
		{
			CL_Console::write_line(cl_format(
				L"The key pressed has been listed before. Database contains the following info:\n" \
				L"KeyInfo:\n\tshortname: %1" \
				L"\n\tdevice: keyboard \n\tcode: %2" \
				L"\n\tdescription: %3",
				string2clantext(_where->second.m_Name),
				string2clantext(_where->second.m_Code),
				string2clantext(_where->second.m_Description)
				));
		}
	}

	void onXInputAxis(const FusionEngine::XInputEvent &event)
	{
		KeyInfoMap::iterator _where = keyinfos.find(DevCodePair("xinput-axis", event.id));
		if (_where == keyinfos.end())
		{
			CL_Console::write_line(L"KeyInfo:\n\tshortname: \n\tdevice: xinput-axis \n\tcode: " +
				CL_StringHelp::int_to_text(event.id) + 
				"\n\tdescription: "
				);

			keyinfos[DevCodePair("xinput-axis", event.id)] = KeyInfo("", "xinput-axis", event.id, "");
		}
		else
		{
			CL_Console::write_line(cl_format(
				L"The key pressed has been listed before. Database contains the following info:\n" \
				L"KeyInfo:\n\tshortname: %1" \
				L"\n\tdevice: keyboard \n\tcode: %2" \
				L"\n\tdescription: %3",
				string2clantext(_where->second.m_Name),
				string2clantext(_where->second.m_Code),
				string2clantext(_where->second.m_Description)
				));
		}
	}

	void onXInputUp(const FusionEngine::XInputEvent &event)
	{
		KeyInfoMap::iterator _where = keyinfos.find(DevCodePair("xinput", event.id));
		if (_where == keyinfos.end())
		{
			CL_Console::write_line(L"KeyInfo:\n\tshortname: \n\tdevice: xinput \n\tcode: " +
				CL_StringHelp::int_to_text(event.id) + 
				L"\n\tdescription: " + event.controller->GetKeyName(event.id)
				);

			std::string desc(CL_StringHelp::text_to_local8(event.controller->GetKeyName(event.id)));
			keyinfos[DevCodePair("xinput", event.id)] = KeyInfo("", "xinput", event.id, desc);
		}
		else
		{
			CL_Console::write_line(cl_format(
				L"The key pressed has been listed before. Database contains the following info:\n" \
				L"KeyInfo:\n\tshortname: %1" \
				L"\n\tdevice: keyboard \n\tcode: %2" \
				L"\n\tdescription: %3",
				string2clantext(_where->second.m_Name),
				string2clantext(_where->second.m_Code),
				string2clantext(_where->second.m_Description)
				));
		}
	}
};

class App
{
private:
	bool working;

public:
	int main(const std::vector<CL_String> &args)
	{
		CL_SetupCore core_setup;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_ConsoleWindow console("KeyInfo Generator: Console");

		CL_DisplayWindow display("KeyInfo Generator: Input Area", 640, 480);

		KeyInfoGenerator gen(display.get_ic());
		// Attempt to load existing data
		gen.load();

		working = true;
		CL_Slot saveSlot = display.get_ic().get_mouse().sig_key_up().connect(this, &App::onSaveClick);
		CL_Slot closeSlot = display.sig_window_close().connect(this, &App::onClose);

		CL_Console::write_line(L"Click the blue box to save");

		CL_InputContext ic = display.get_ic();
		while (working)
		{
			display.get_gc().clear(CL_Colorf::white);
			CL_Draw::box(display.get_gc(), 620, 470, 640, 480, CL_Colorf::aliceblue);
			display.flip();

			// Make sure all input events are fired
			//ic.poll(false);

			// Fire XInput events
			gen.poll();

			if (CL_DisplayMessageQueue::has_messages())
				CL_DisplayMessageQueue::process();

			CL_System::sleep(10);
		}

		gen.save();

		return 0;
	}

	void onClose()
	{
		working = false;
	}

	void onSaveClick(const CL_InputEvent &event, const CL_InputState &state)
	{
		if (event.mouse_pos.x > 620 && event.mouse_pos.y > 470)
			working = false;
	}
};

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		return App().main(args);
	}
};

CL_ClanApplication app(&EntryPoint::main);