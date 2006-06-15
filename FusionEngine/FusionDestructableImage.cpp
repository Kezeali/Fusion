
#include "FusionEngineCommon.h"

/// STL

/// Fusion

/// Class
#include "FusionDestructableImage.h"

DestructableImage::DestructableImage(std::string &filename);
{
	Image = new CL_Surface(filename);
	OriginalImage = new CL_PixelBuffer(Image->get_pixeldata());

	m_Canvas = new CL_Canvas(*Image);
}

DestructableImage::DestructableImage(const CL_Surface &copy)
{
	Image = new CL_Surface(image);
	OriginalImage = new CL_PixelBuffer(Image->get_pixeldata());

	m_Canvas = new CL_Canvas(*Image);
}

DestructableImage::DestructableImage(CL_PixelBuffer &provider);
{
	Image = new CL_Surface(provider);
	OriginalImage = new CL_PixelBuffer(Image->get_pixeldata());

	m_Canvas = new CL_Canvas(*Image);
}

DestructableImage::~DestructableImage();
{
	delete m_Image;
	delete m_OriginalImage;
	delete m_Canvas;
}

CL_Surface *DestructableImage::GetImage()
{
	return m_Image;
}

void DestructableImage::ResetImage()
{
	m_Canvas->set_pixeldata(*OriginalImage);
	m_Canvas->sync_surface();
}

void DestructableImage::RemovePixel(int x, int y)
{
	m_Canvas->get_gc().draw_pixel(x, y, CL_Color(0, 0, 0, 0));
}

void DestructableImage::RemovePixel(const CL_Point &point)
{
	m_Canvas->get_gc().draw_pixel(point.x, point.y, CL_Color(0, 0, 0, 0));
}

void DestructableImage::RemovePixelsLine(int x, int y, int x2, int y2)
{
	m_Canvas->get_gc().draw_line(x, y, x2, y2, CL_Color(0, 0, 0, 0));
}

void DestructableImage::RemovePixelsLine(const CL_Point &point, const CL_Point &point2)
{
	m_Canvas->get_gc().draw_line(
		point.x, point.y,
		point2.x, point2.y,
		CL_Color(0, 0, 0, 0)
	);
}

void DestructableImage::RemovePixelsRect(int left, int top, int right, int bottom)
{
	CL_Rect rect = CL_Rect(left, top, right, bottom);
	m_Canvas->get_gc().fill_rect(rect, CL_Color(0, 0, 0, 0));
}

void DestructableImage::RemovePixelsRect(const CL_Rect &rect)
{
	m_Canvas->get_gc().fill_rect(rect, CL_Color(0, 0, 0, 0));
}

void DestructableImage::RemovePixelsCircle(int x, int y, int diameter)
{
	// Just remove a rect for now.
	CL_Rect rect = CL_Rect(
		x - diameter / 2,
		y - diameter / 2,
		x + diameter / 2,
		y + diameter / 2
	);
	RemovePixelsRect(rect);
}

