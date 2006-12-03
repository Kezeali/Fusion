#ifndef Header_FusionEngine_ResourceLoader
#define Header_FusionEngine_ResourceLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionSingleton.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Loads and stores resources for gameplay.
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
		 * Gets the image associated with the given tag.
		 * \returns
		 * A pointer to the requested image, or NULL if no such tag exists.
		 */
		CL_Surface *GetImage(const std::string &tag);
		/*!
		 * \brief
		 * Gets the sound associated with the given tag.
		 * \returns
		 * A pointer to the requested sound, or NULL if no such tag exists.
		 */
		CL_SoundBuffer *GetSound(const std::string &tag);

	private:

		//! Loaded gfx
		SurfaceMap m_Images;
		//! Loaded sounds
		SoundBufferMap m_Sounds;
	};

}

#endif
