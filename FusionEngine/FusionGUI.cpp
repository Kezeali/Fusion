
#include "FusionCommon.h"

#include "FusionGUI.h"

//#include "FusionCEGUIPhysFSResourceProvider.h"
#include "FusionConsole.h"
#include "FusionLogger.h"

//#include "FusionResourceManager.h"

//#include <CEGUI/CEGUITinyXMLParser.h>
//#include <CEGUI/CEGUIDefaultResourceProvider.h>

namespace FusionEngine
{

	class RocketSystem : public Rocket::Core::SystemInterface
	{
	public:
		RocketSystem() {}
	public:
		virtual float GetElapsedTime();
		virtual bool LogMessage(EMP::Core::Log::Type type, const EMP::Core::String& message);
	};

	float RocketSystem::GetElapsedTime()
	{
		return (float)CL_System::get_time() / 1000.f;
	}

	bool RocketSystem::LogMessage(EMP::Core::Log::Type type, const EMP::Core::String& message)
	{
		LogSeverity logLevel = LOG_NORMAL;
		Console::MessageType mtype = Console::MTWARNING;
		if (type == EMP::Core::Log::LT_ERROR || type == EMP::Core::Log::LT_ASSERT)
		{
			logLevel = LOG_CRITICAL;
			mtype = Console::MTERROR;
		}
		else if (type == EMP::Core::Log::LT_INFO)
		{
			logLevel = LOG_TRIVIAL;
			mtype = Console::MTNORMAL;
		}
		Logger::getSingleton().Add(std::string(message.CString()), "rocket_log", logLevel);
		SendToConsole(std::string(message.CString()), mtype);

		return true;
	}

	struct GeometryVertex
	{
		CL_Vec2f position;
		CL_Vec2f tex_coord;
	};

	struct RocketCL_Texture
	{
		RocketCL_Texture(CL_Texture tex) : texture(tex) {}
		CL_Texture texture;
	};

	struct GeometryData
	{
		int num_verticies;
		CL_VertexArrayBuffer vertex_buffer;
		RocketCL_Texture* texture;
	};

	class RocketRenderer : public Rocket::Core::RenderInterface
	{
	public:
		RocketRenderer() {}
		RocketRenderer(CL_GraphicContext gc);

		typedef std::tr1::unordered_map<CL_String, CL_Texture> TextureMap;
		typedef std::list<CL_VertexArrayBuffer> GeometryMap;
	public:
		//! Called by Rocket when it wants to render geometry that it does not wish to optimise.
		virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const EMP::Core::Vector2f& translation);

		//! Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
		virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture);
		//! Called by Rocket when it wants to render application-compiled geometry.
		virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const EMP::Core::Vector2f& translation);
		//! Called by Rocket when it wants to release application-compiled geometry.
		virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry);

		//! Called by Rocket when it wants to enable or disable scissoring to clip content.
		virtual void EnableScissorRegion(bool enable);
		//! Called by Rocket when it wants to change the scissor region.
		virtual void SetScissorRegion(int x, int y, int width, int height);

		//! Called by Rocket when a texture is required by the library.
		virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, EMP::Core::Vector2i& texture_dimensions, const EMP::Core::String& source, const EMP::Core::String& source_path);
		//! Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
		virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const EMP::Core::byte* source, const EMP::Core::Vector2i& source_dimensions);
		//! Called by Rocket when a loaded texture is no longer required.
		virtual void ReleaseTexture(Rocket::Core::TextureHandle texture);
	protected:
		CL_GraphicContext m_gc;

		int m_Scissor_left;
		int m_Scissor_top;
		int m_Scissor_right;
		int m_Scissor_bottom;
		//TextureMap m_Textures;
		//GeometryMap m_Geometry;;
	};

	RocketRenderer::RocketRenderer(CL_GraphicContext gc)
		: m_gc(gc)
	{
	}

	void RocketRenderer::RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const EMP::Core::Vector2f& translation)
	{
		SendToConsole(L"Rocket requested uncompiled render; ignored", Console::MTWARNING);
	}

	Rocket::Core::CompiledGeometryHandle RocketRenderer::CompileGeometry(Rocket::Core::Vertex *vertices, int num_vertices, int *indices, int num_indices, Rocket::Core::TextureHandle texture)
	{
		using namespace Rocket;

		CL_VertexArrayBuffer buffer(m_gc, num_indices * sizeof(GeometryVertex));

		buffer.lock(cl_access_write_only);
		GeometryVertex* buffer_data = (GeometryVertex*) buffer.get_data();

		for (int i = 0; i < num_indices; i++)
		{
			int vertex_index = indices[i];
			buffer_data[i].position.x = vertices[vertex_index].position.x;
			buffer_data[i].position.y = vertices[vertex_index].position.y;

			buffer_data[i].tex_coord.x = vertices[vertex_index].tex_coord.x;
			buffer_data[i].tex_coord.y = vertices[vertex_index].tex_coord.y;
		}

		buffer.unlock();

		GeometryData* data = new GeometryData;
		data->num_verticies = num_indices;
		data->vertex_buffer = buffer;
		data->texture = (RocketCL_Texture*)texture;

		//m_Geometry.push_back(buffer);
		return (Core::CompiledGeometryHandle*)data;
	}

	void RocketRenderer::RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const EMP::Core::Vector2f& translation)
	{
		using namespace Rocket;

		GeometryData* data = (GeometryData*)geometry;
		CL_VertexArrayBuffer vertex_buffer = data->vertex_buffer;

		m_gc.set_map_mode(cl_map_2d_upper_left);
		m_gc.set_texture(0, data->texture->texture);

		CL_PrimitivesArray prim_array(m_gc);
		prim_array.set_positions(vertex_buffer, data->num_verticies, cl_type_float, &static_cast<GeometryVertex*>(0)->position, sizeof(GeometryVertex));
		prim_array.set_tex_coords(0, vertex_buffer, data->num_verticies, cl_type_float, &static_cast<GeometryVertex*>(0)->tex_coord, sizeof(GeometryVertex));

		m_gc.draw_primitives(cl_triangles, data->num_verticies, prim_array);
	}

	void RocketRenderer::ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
	{
		delete (GeometryData*)geometry;
	}

	// Called by Rocket when it wants to enable or disable scissoring to clip content.
	void RocketRenderer::EnableScissorRegion(bool enable)
	{
		if (!enable)
			m_gc.set_cliprect(CL_Rect(0, 0, m_gc.get_width(), m_gc.get_height()));
		else
			m_gc.set_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));
	}

	// Called by Rocket when it wants to change the scissor region.
	void RocketRenderer::SetScissorRegion(int x, int y, int width, int height)
	{
		m_Scissor_left = x;
		m_Scissor_top = y;
		m_Scissor_right = x + width;
		m_Scissor_bottom = y + height;

		m_gc.set_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));
	}

	bool RocketRenderer::LoadTexture(Rocket::Core::TextureHandle& texture_handle, EMP::Core::Vector2i& texture_dimensions, const EMP::Core::String& source, const EMP::Core::String& source_path)
	{
		CL_PixelBuffer image = CL_ImageProviderFactory::load( CL_String((source_path + source).CString()) );
		if (image.is_null())
			return false;

		CL_Texture texture(m_gc, cl_texture_2d);
		texture.set_image(image);

		if (texture.is_null())
			return false;

		texture_dimensions.x = texture.get_width();
		texture_dimensions.y = texture.get_height();

		texture_handle = new RocketCL_Texture(texture);
		return true;
	}

	// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
	bool RocketRenderer::GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const EMP::Core::byte* source, const EMP::Core::Vector2i& source_dimensions)
	{
		static int texture_id = 1;

		int pitch = source_dimensions.x * 4;
		CL_PixelBuffer image(source_dimensions.x, source_dimensions.y, pitch, CL_PixelFormat::abgr8888, (void*)source);

		CL_Texture texture(m_gc, cl_texture_2d);
		texture.set_image(image);

		if (texture.is_null())
			return false;

		texture_handle = new RocketCL_Texture(texture);
		return true;
	}

	// Called by Rocket when a loaded texture is no longer required.
	void RocketRenderer::ReleaseTexture(Rocket::Core::TextureHandle texture)
	{
		delete ((RocketCL_Texture*)texture);
	}


	GUI::GUI()
		: FusionState(false), // GUI is non-blockin by default
		m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000)
	{
	}

	GUI::GUI(CL_DisplayWindow window)
		: FusionState(false), /* GUI is non-blockin by default */
		m_Modifiers(NOMOD),
		m_MouseShowPeriod(1000),
		m_ShowMouseTimer(1000),
		m_Display(window)
	{
		// Just in case, I guess? (re-initialized in GUI::Initialise() after the CEGUI renderer has been set up)
		//m_GLState = CL_OpenGLState(window->get_gc());
	}

	GUI::~GUI()
	{
		CleanUp();
	}

	void GUI::Configure(const std::string& fname)
	{
		//ResourcePointer<TiXmlDocument> cfgResource = ResourceManager::getSingleton().GetResource("CEGUIConfig.xml", "XML");

		//if (cfgResource.IsValid())
		//{
		//	TiXmlDocument* cfgDoc = cfgResource.GetDataPtr();
		//	
		//	TinyXPath::S_xpath_string(cfgDoc->RootElement(), "/ceguiconfig/paths/datafiles");
		//}
	}

	bool GUI::Initialise()
	{
		//CL_Display::get_current_window()->hide_cursor();
		using namespace Rocket;

		Rocket::Core::Initialise();

		CL_GraphicContext gc = m_Display.get_gc();

		m_Context = Rocket::Core::CreateContext("default", EMP::Core::Vector2i(gc.get_width(), gc.get_width()));
		
		m_Document = m_Context->LoadDocument("../../assets/demo.rml");
		if (m_Document != NULL)
			m_Document->Show();


		CL_InputContext ic = m_Display.get_ic();

		// Mouse Events
		m_Slots.connect(ic.get_mouse().sig_key_down(), this, &GUI::onMouseDown);
		m_Slots.connect(ic.get_mouse().sig_key_up(), this, &GUI::onMouseUp);
		m_Slots.connect(ic.get_mouse().sig_pointer_move(), this, &GUI::onMouseMove);
		// KBD events
		m_Slots.connect(ic.get_keyboard().sig_key_down(), this, &GUI::onKeyDown);
		m_Slots.connect(ic.get_keyboard().sig_key_up(), this, &GUI::onKeyUp);

		return true;
	}

	bool GUI::Update(unsigned int split)
	{
		m_Context->Update();

		// Hide the cursor if the timeout has been reached
		if ( m_ShowMouseTimer <= 0 )
		{
			m_Context->ShowMouseCursor(false);
		}
		else
		{
			m_ShowMouseTimer -= split;
		}

		return true;
	}

	void GUI::Draw()
	{
		m_Context->Render();
	}

	void GUI::CleanUp()
	{
		m_Context->RemoveReference();
		Rocket::Core::Shutdown();

		m_Display.show_cursor();
	}

	bool GUI::AddWindow(const std::string& window)
	{
		using namespace Rocket;

		//if (WindowManager::getSingleton().isWindowPresent(window))
		//	return false;

		//System::getSingleton().getGUISheet()->addChildWindow(window);
		
		return true;
	}

	bool GUI::RemoveWindow(const std::string& window)
	{
		using namespace Rocket;

		//System::getSingleton().getGUISheet()->removeChildWindow(window);

		return true;
	}

	void GUI::SetMouseShowPeriod(unsigned int period)
	{
		m_MouseShowPeriod = period;
	}


	void GUI::onMouseDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		switch(ev.id)
		{
		case CL_MOUSE_LEFT:
			m_Context->ProcessMouseButtonDown(0, modifier);
			break;
		case CL_MOUSE_RIGHT:
			m_Context->ProcessMouseButtonDown(1, modifier);
			break;
		case CL_MOUSE_MIDDLE:
			m_Context->ProcessMouseButtonDown(2, modifier);
			break;
		case CL_MOUSE_XBUTTON1:
			m_Context->ProcessMouseButtonDown(3, modifier);
			break;
		case CL_MOUSE_XBUTTON2:
			m_Context->ProcessMouseButtonDown(4, modifier);
			break;
		}

	}

	void GUI::onMouseUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		switch(ev.id)
		{
		case CL_MOUSE_LEFT:
			m_Context->ProcessMouseButtonUp(0, modifier);
			break;
		case CL_MOUSE_RIGHT:
			m_Context->ProcessMouseButtonUp(1, modifier);
			break;
		case CL_MOUSE_MIDDLE:
			m_Context->ProcessMouseButtonUp(2, modifier);
			break;
		case CL_MOUSE_XBUTTON1:
			m_Context->ProcessMouseButtonUp(3, modifier);
			break;
		case CL_MOUSE_XBUTTON2:
			m_Context->ProcessMouseButtonUp(4, modifier);
			break;
		case CL_MOUSE_WHEEL_UP:
			m_Context->ProcessMouseWheel(1, modifier);
			break;
		case CL_MOUSE_WHEEL_DOWN:
			m_Context->ProcessMouseWheel(-1, modifier);
			break;
		}

	}

	void GUI::onMouseMove(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		m_Context->ProcessMouseMove(ev.mouse_pos.x, ev.mouse_pos.y, modifier);

		if (m_ShowMouseTimer <= 0)
		{
			m_ShowMouseTimer = m_MouseShowPeriod;
			m_Context->ShowMouseCursor(true);
		}
	}

	void GUI::onKeyDown(const CL_InputEvent &ev, const CL_InputState &state)
	{
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		// Grab characters
		if (!ev.str.empty())
		{
			std::string str(ev.str.begin(), ev.str.end());
			m_Context->ProcessTextInput( Rocket::Core::String(str.c_str()) );
			//const wchar_t* c_str = ev.str.c_str();
			// Inject all the characters given
			//for (int c = 0; c < ev.str.length(); c++)
			//	m_Context->ProcessTextInput( EMP::Core::word(c_str[c]) );
		}

		m_Context->ProcessKeyDown(CLKeyToRocketKeyIdent(ev.id), modifier);
	}

	void GUI::onKeyUp(const CL_InputEvent &ev, const CL_InputState &state)
	{
		//if (key.id == CL_KEY_SHIFT)
		//	m_Modifiers ^= SHIFT;
		int modifier = 0;
		if (ev.alt)
			modifier |= Rocket::Core::Input::KM_ALT;
		if (ev.ctrl)
			modifier |= Rocket::Core::Input::KM_CTRL;
		if (ev.shift)
			modifier |= Rocket::Core::Input::KM_SHIFT;

		m_Context->ProcessKeyUp(CLKeyToRocketKeyIdent(ev.id), modifier);
	}

}
