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

#ifndef Header_FusionEngine_ErrorTypes
#define Header_FusionEngine_ErrorTypes

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{

	//! Message passed to FusionGame when an unexpected quit event arises
	/*!
	 * The primary "quit event" is a state update returning false.
	 */
	class Error
	{
	public:
		//! Types of errors that can cause a quit event
		enum ErrorType
		{
			NONE,
			UNEXPECTEDDISCONNECT,
			BANNED
		};
	public:
		//! Basic constructor
		Error();
		//! Constructor +type +message
		Error(ErrorType type, const std::string &message);

	public:
		//! Retrieves the type (of the error which caused the unexpected quit.)
		ErrorType GetType() const;
		//! Retrieves the human readable message.
		const std::string &GetMessage() const;

	protected:
		//! Stores the error type
		ErrorType m_Type;
		//! Stores the message
		std::string m_Message;

	};

}

#endif
