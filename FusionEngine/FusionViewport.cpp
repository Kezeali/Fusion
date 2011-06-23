#include "FusionStableHeaders.h"

// Class
#include "FusionViewport.h"


namespace FusionEngine
{

	Viewport::Viewport()
		: m_Area(0.f, 0.f, 1.f, 1.f)
	{}

	Viewport::Viewport(const CL_Rectf &area)
		: m_Area(area)
	{
	}

	Viewport::Viewport(const CL_Rectf &area, const CameraPtr &camera)
		: m_Area(area),
		m_Camera(camera)
	{
	}

	void Viewport::SetArea(const CL_Rectf &area)
	{
		m_Area = area;
	}

	void Viewport::SetArea(float left, float top, float right, float bottom)
	{
		m_Area.left = left;
		m_Area.right = right;
		m_Area.top = top;
		m_Area.bottom = bottom;
	}

	void Viewport::SetPosition(float left, float top)
	{
		m_Area.left = left;
		m_Area.top = top;
	}

	void Viewport::SetSize(float width, float height)
	{
		m_Area.set_width(width);
		m_Area.set_height(height);
	}

	const CL_Rectf &Viewport::GetArea() const
	{
		return m_Area;
	}
	CL_Pointf Viewport::GetPosition() const
	{
		return m_Area.get_top_left();
	}
	CL_Sizef Viewport::GetSize() const
	{
		return m_Area.get_size();
	}

	void Viewport::SetCamera(const CameraPtr &camera)
	{
		m_Camera = camera;
	}
	const CameraPtr &Viewport::GetCamera() const
	{
		return m_Camera;
	}

	Viewport* Viewport_Factory()
	{
		return new Viewport();
	}

	Viewport* Viewport_Factory(float left, float top, float right, float bottom)
	{
		return new Viewport(CL_Rectf(left, top, right, bottom));
	}

	Viewport* Viewport_Factory(float left, float top, float right, float bottom, Camera* camera)
	{
		return new Viewport(CL_Rectf(left, top, right, bottom), camera);
	}

	void Viewport_SetCamera(Camera *camera, Viewport *viewport)
	{
		viewport->SetCamera( CameraPtr(camera) );
		camera->release();
	}

	void Viewport::Register(asIScriptEngine *engine)
	{
		int r;
		RefCounted::RegisterType<Viewport>(engine, "Viewport");
		r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY,
			"Viewport@ f()",
			asFUNCTIONPR(Viewport_Factory, (void), Viewport*), asCALL_CDECL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY,
			"Viewport@ f(float, float, float, float)",
			asFUNCTIONPR(Viewport_Factory, (float, float, float, float), Viewport*), asCALL_CDECL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY,
			"Viewport@ f(float, float, float, float, Camera@)",
			asFUNCTIONPR(Viewport_Factory, (float, float, float, float, Camera*), Viewport*), asCALL_CDECL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Viewport",
			"void setArea(float, float, float, float)",
			asMETHOD(Viewport, SetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setPosition(float, float)",
			asMETHOD(Viewport, SetPosition), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setSize(float, float)",
			asMETHOD(Viewport, SetSize), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setCamera(Camera)",
			asFUNCTION(Viewport_SetCamera), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"Camera getCamera()",
			asMETHOD(Viewport, GetCamera), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

}
