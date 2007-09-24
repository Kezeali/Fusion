/*
  Copyright (c) 2007 Fusion Project Team
	Copyright (c) 1997-2005 The ClanLib Team

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
		
		
	File Author(s):

		Magnus Norddahl (Inspiration in ClanLib)

		Elliot Hayward (This implementation)

*/


#ifndef Header_Fusion_SlotContainer
#define Header_Fusion_SlotContainer

#if _MSC_VER > 1000
#pragma once
#endif

#include "Common.h"

namespace FusionEngine
{

	//! Fusion SlotContainer
	/*!
	 * Essentually the same as CL_SlotContainer, but with added features
	 * which make it much more useable
	 */
	class SlotContainer
	{
	public:
		//! Connect a slot to a CL_Signal_v0 signal.
		template<class SigClass>
		void connect(SigClass &sig, void (*func)())
		{
			m_Slots.push_back(sig.connect(func));
		}

		//! Connect a slot to a CL_Signal_v0 signal.
		template<class SigClass, class Class>
		void connect(SigClass &sig, Class *self, void(Class::*func)())
		{
			m_Slots.push_back(sig.connect(self, func));
		}

		//! Connect a slot to a CL_Signal_v0 signal with an user data parameter passed along.
		template<class SigClass, class Class, class UserData>
		void connect(SigClass &sig, Class *self, void(Class::*func)(UserData), UserData user_data)
		{
			m_Slots.push_back(sig.connect(self, func, user_data));
		}

		//! Connect a slot to a CL_Signal_v1 signal.
		template<class SigClass, class Class, class Param1>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1))
		{
			m_Slots.push_back(sig.connect(self, func));
		}

		//! Connect a slot to a CL_Signal_v1 signal with an user data parameter passed along.
		template<class SigClass, class Class, class Param1, class UserData>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, UserData), UserData user_data)
		{
			m_Slots.push_back(sig.connect(self, func, user_data));
		}

		//! Connect a slot to a CL_Signal_v2 signal.
		template<class SigClass, class Class, class Param1, class Param2>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2))
		{
			m_Slots.push_back(sig.connect(self, func));
		}

		//! Connect a slot to a CL_Signal_v2 signal with an user data parameter passed along.
		template<class SigClass, class Class, class Param1, class Param2, class UserData>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2, UserData), UserData user_data)
		{
			m_Slots.push_back(sig.connect(self, func, user_data));
		}

		//! Connect a slot to a CL_Signal_v3 signal.
		template<class SigClass, class Class, class Param1, class Param2, class Param3>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2, Param3))
		{
			m_Slots.push_back(sig.connect(self, func));
		}

		//! Connect a slot to a CL_Signal_v3 signal with an user data parameter passed along.
		template<class SigClass, class Class, class Param1, class Param2, class Param3, class UserData>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2, Param3, UserData), UserData user_data)
		{
			m_Slots.push_back(sig.connect(self, func, user_data));
		}

		//! Connect a slot to a CL_Signal_v4 signal.
		template<class SigClass, class Class, class Param1, class Param2, class Param3, class Param4>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2, Param3, Param4))
		{
			m_Slots.push_back(sig.connect(self, func));
		}

		//! Connect a slot to a CL_Signal_v4 signal with an user data parameter passed along.
		template<class SigClass, class Class, class Param1, class Param2, class Param3, class Param4, class UserData>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2, Param3, Param4, UserData user_data), UserData user_data)
		{
			m_Slots.push_back(sig.connect(self, func, user_data));
		}

		//! Connect a slot to a CL_Signal_v5 signal.
		template<class SigClass, class Class, class Param1, class Param2, class Param3, class Param4, class Param5>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2, Param3, Param4, Param5))
		{
			m_Slots.push_back(sig.connect(self, func));
		}

		//! Connect a slot to a CL_Signal_v5 signal with an user data parameter passed along.
		template<class SigClass, class Class, class Param1, class Param2, class Param3, class Param4, class Param5, class UserData>
		void connect(SigClass &sig, Class *self, void(Class::*func)(Param1, Param2, Param3, Param4, Param5, UserData user_data), UserData user_data)
		{
			m_Slots.push_back(sig.connect(self, func, user_data));
		}

		//! Connect a functor slot to a signal.
		template<class SigClass, class Functor>
		void connect_functor(SigClass &sig, const Functor &functor)
		{
			m_Slots.push_back(sig.connect_functor(functor));
		}

		//! Disconnects all the slots
		void disconnect_all()
		{
			m_Slots.clear();
		}

	private:
		std::list<CL_Slot> m_Slots;
	};

}

#endif
