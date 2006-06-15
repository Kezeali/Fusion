#ifndef Header_FusionEngine_DestructableImage
#define Header_FusionEngine_DestructableImage

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{
	// Foward declaration
	class DamageMap;

	/*!
	 * \brief
	 * Provides a surface and methods to modify it.
	 *
	 * \remarks
	 * The ApplyDamageMap method is for efficianly syncronising the damage over
	 * a network. A DamageMap can be easyly syncronised in every FusionShipFrame.
	 * This also allows the History system to backtrack without loosing damage data.
	 */
	class DestructableImage
	{
	public:
		DestructableImage(const std::string &filename);
		DestructableImage(const CL_Surface &copy);
		DestructableImage(const CL_PixelBuffer &provider);

		CL_Surface *GetImage();

		void ResetImage();

		void ApplyDamageMap(const DamageMap &map);

		void RemovePixel(int x, int y);
		void RemovePixel(const CL_Point &point);
		void RemovePixelsLine(int x, int y, int x2, int y2);
		void RemovePixelsLine(const CL_Point &point, const CL_Point &point2);
		void RemovePixelsRect(int x, int y, int width, int height);
		void RemovePixelsRect(const CL_Rect &rect);
		void RemovePixelsCircle(int x, int y, int diameter);

	private:
		CL_Surface *Image;
		CL_PixelBuffer *OriginalImage;

		// All the image modification is done on this opengl canvas
		CL_Canvas *m_Canvas;
	};

	//! Allows data about image deformation to be transferred efficiantly over a network.
	class DamageMap
	{
	public:
		void Set(const CL_PixelBuffer &damagemap);
		Cl_PixelBuffer &Get() const;
	private:
		CL_PixelBuffer *DamageMap;
	};

}

#endif
