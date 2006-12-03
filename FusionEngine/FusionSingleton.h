/*
  Copyright (c) 2006 FusionTeam

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef Header_FusionEngine_Singleton
#define Header_FusionEngine_Singleton

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	/*!
	* \brief
	* Template class for creating singleton classes.
	*/
	template<typename T> class Singleton
	{
	public:
		//! Constructor
		Singleton( void )
		{
			assert( !ms_Singleton );
			ms_Singleton = static_cast<T*>(this);
		}
		//! Destructor
		~Singleton( void )
		{
			assert( ms_Singleton );
			ms_Singleton = 0;
		}

	public:
		//! Returns the static instance of singleton
		static T& getSingleton( void )
		{
			assert( ms_Singleton );
			return ( *ms_Singleton );
		}
		//! Returns a pointer to the static instance of singleton
		static T* getSingletonPtr( void )
		{
			return ( ms_Singleton );
		}

	protected:
		//! The instance
		static T *ms_Singleton;

	};

}

#endif