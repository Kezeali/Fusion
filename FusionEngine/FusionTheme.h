#ifndef Header_FusionEngine_Theme
#define Header_FusionEngine_Theme

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Fusion
#include "FusionSingleton.h"

namespace FusionEngine
{

	//! Resource tree for 
	static const std::string g_Viewport1p_Tree = "gui/viewport1p";
	static const std::string g_Viewport1p_1 = g_Viewport1p_Tree + "/1";

	static const std::string g_Viewport1p_Tree = "gui/viewport1p";
	static const std::string g_Viewport1p_1 = g_Viewport1p_Tree + "/1";
	/*!
	 * \brief
	 * Loads and stores resources for gui elements
	 *
	 * May be merged with ResourceManager at some point...
	 *
	 * \todo
	 * Load message strings into theme
	 */
	class Theme : public Singleton<Theme>
	{
	public:
		//! Constructor
		Theme() { }
		//! Destructor
		~Theme() { Unload(); /*cleans up*/ }

	public:
		//! Map of surfaces
		typedef std::map<std::string, CL_Surface*> SurfaceMap;
		//! Map of sound buffers
		typedef std::map<std::string, CL_SoundBuffer*> SoundBufferMap;

		
		/*!
		 * \brief
		 * Loads the given theme file.
		 *
		 * \returns
		 * False if any files are not found.
		 */
		bool Load(const std::string &filename);

		/*!
		 * \brief
		 * Clears and deletes all loaded resources.
		 */
		void Unload();

		/*!
		 * \brief
		 * Loads the given section of a theme file
		 *
		 * \retval false
		 * If any files are not found, or the tree isn't explicitly defined.
		 */
		bool LoadTree(const std::string &name);

		/*!
		 * \brief
		 * Unloads the resources in the given branch.
		 */
		void UnloadTree(const std::string &name);

		//! Returns a resource of type T
		template<typename T>
		T* GetResource(const std::string& tag);
		/*!
		 * \brief
		 * Gets the image associated with the given tag.
		 * \returns
		 * A pointer to the requested image, or NULL if no such tag exists.
		 */
		//CL_Surface *GetImage(const std::string &tag);
		/*!
		 * \brief
		 * Gets the sound associated with the given tag.
		 * \returns
		 * A pointer to the requested sound, or NULL if no such tag exists.
		 */
		//CL_SoundBuffer *GetSound(const std::string &tag);

	protected:
		bool loadImages(

		/*!
		 * \brief
		 * Returns the pixel the given percentage from the left of the window.
		 */
		static int Theme::percentToXPoint(int percent)
		{
			return CL_Display::get_width() * percent * 0.01;
		}

		/*!
		 * \brief
		 * Returns the pixel the given percentage from the top of the window.
		 */
		static int Theme::percentToYPoint(int percent)
		{
			return CL_Display::get_height() * percent * 0.01;
		}

	private:

		//! Loaded gfx
		SurfaceMap m_Images;
		//! Loaded sounds
		SoundBufferMap m_Sounds;
	};

}

#endif
