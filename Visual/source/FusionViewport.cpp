#include "PrecompiledHeaders.h"

// Class
#include "FusionViewport.h"

#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	Viewport::Viewport()
		: m_Area(0.f, 0.f, 1.f, 1.f)
	{}

	Viewport::Viewport(const clan::Rectf &area)
		: m_Area(area)
	{
	}

	Viewport::Viewport(const clan::Rectf &area, const CameraPtr &camera)
		: m_Area(area),
		m_Camera(camera)
	{
	}

	void Viewport::SetArea(const clan::Rectf &area)
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

	const clan::Rectf &Viewport::GetArea() const
	{
		return m_Area;
	}
	clan::Pointf Viewport::GetPosition() const
	{
		return m_Area.get_top_left();
	}
	clan::Sizef Viewport::GetSize() const
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
		new (ptr) ViewportPtr(new Viewport(clan::Rectf(left, top, right, bottom)));
	}

	void Viewport_ctor2(float left, float top, float right, float bottom, CameraPtr camera, void* ptr)
	{
		new (ptr) ViewportPtr(new Viewport(clan::Rectf(left, top, right, bottom), camera));
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

	void Viewport::CalculateScreenArea(const clan::GraphicContext& gc, clan::Rect &area, bool apply_camera_offset)
	{
		const clan::Rectf &proportions = GetArea();

		area.left = (int)floor(proportions.left * gc.get_width());
		area.top = (int)floor(proportions.top * gc.get_height());
		area.right = (int)ceil(proportions.right * gc.get_width());
		area.bottom = (int)ceil(proportions.bottom * gc.get_height());

		if (apply_camera_offset)
		{
			const CameraPtr &camera = GetCamera();
			if (!camera)
				FSN_EXCEPT(ExCode::InvalidArgument, "Cannot apply camera offset if the viewport has no camera associated with it");

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			clan::Vec2i viewportOffset =
				camera->GetPosition() - clan::Vec2f::calc_origin(clan::origin_center, clan::Sizef((float)area.get_width(), (float)area.get_height()));

			area.translate(viewportOffset);
		}
	}

	void Viewport::CalculateScreenArea(const clan::GraphicContext& gc, clan::Rectf &area, bool apply_camera_offset)
	{
		const clan::Rectf &proportions = GetArea();

		area.left = proportions.left * gc.get_width();
		area.top = proportions.top * gc.get_height();
		area.right = proportions.right * gc.get_width();
		area.bottom = proportions.bottom * gc.get_height();

		if (apply_camera_offset)
		{
			const CameraPtr &camera = GetCamera();
			if (!camera)
				FSN_EXCEPT(ExCode::InvalidArgument, "Cannot apply camera offset if the viewport has no camera associated with it");

			auto camZoom = (1.f / camera->GetZoom());
			const clan::Sizef zoomedSize(area.get_width() * camZoom, area.get_height() * camZoom);

			// Viewport offset is the top-left of the viewport in the game-world,
			//  i.e. camera_offset - viewport_size * camera_origin
			clan::Vec2f viewportOffset =
				camera->GetPosition() - clan::Vec2f::calc_origin( clan::origin_center, zoomedSize);

			area.left = viewportOffset.x;
			area.top = viewportOffset.y;
			area.set_size(zoomedSize);
		}
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
