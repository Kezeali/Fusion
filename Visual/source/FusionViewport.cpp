#include "PrecompiledHeaders.h"

// Class
#include "FusionViewport.h"

#include "FusionScriptTypeRegistrationUtils.h"

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

	void Viewport_ctor1(float left, float top, float right, float bottom, void* ptr)
	{
		new (ptr) ViewportPtr(new Viewport(CL_Rectf(left, top, right, bottom)));
	}

	void Viewport_ctor2(float left, float top, float right, float bottom, CameraPtr camera, void* ptr)
	{
		new (ptr) ViewportPtr(new Viewport(CL_Rectf(left, top, right, bottom), camera));
	}

	void Viewport_SetCamera(CameraPtr camera, ViewportPtr *viewport)
	{
		(*viewport)->SetCamera(camera);
	}

	CameraPtr Viewport_GetCamera(ViewportPtr *viewport)
	{
		return (*viewport)->GetCamera();
	}

	void Viewport_SetArea(float left, float top, float right, float bottom, ViewportPtr *viewport)
	{
		(*viewport)->SetArea(left, top, right, bottom);
	}

	void Viewport_SetPosition(float left, float top, ViewportPtr *viewport)
	{
		(*viewport)->SetPosition(left, top);
	}

	void Viewport_SetSize(float width, float height, ViewportPtr *viewport)
	{
		(*viewport)->SetSize(width, height);
	}

	void Viewport::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSharedPtrType<Viewport>("Viewport", engine);
		//r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_CONSTRUCT,
		//	"void f(float, float, float, float)",
		//	asFUNCTION(Viewport_ctor1), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		//r = engine->RegisterObjectBehaviour("Viewport", asBEHAVE_FACTORY,
		//	"void f(float, float, float, float, Camera)",
		//	asFUNCTION(Viewport_ctor2), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Viewport",
			"void setArea(float, float, float, float)",
			asFUNCTION(Viewport_SetArea), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setPosition(float, float)",
			asFUNCTION(Viewport_SetPosition), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setSize(float, float)",
			asFUNCTION(Viewport_SetSize), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"void setCamera(Camera)",
			asFUNCTION(Viewport_SetCamera), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Viewport",
			"Camera getCamera()",
			asFUNCTION(Viewport_GetCamera), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

}
