/*
  Copyright (c) 2006 Fusion Project Team

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

#ifndef Header_FusionEngine_Error
#define Header_FusionEngine_Error

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	//! Fusion Exception class.
	/*!
	 * Used primarily by Colsole, Logger, and StateManager.  This is only used
	 * for fatal errors, where FusionGame will have to show something to the
	 * user.
	 *
	 * \remarks
	 * This is the message thrown to FusionGame when an unexpected error arises,
	 * which is usually when a state update returns false.
	 */
	class Error
	{
	public:
		//! Types of errors
		enum ErrorType
		{
			//! None / trivial
			NONE = 0,
			//! An internal error of unspecified (but critical) type
			INTERNAL_ERROR,
			//! Disconnected for some (unusual) reason
			UNEXPECTED_DISCONNECT,
			//! You can't connect to this server
			BANNED,
			//! A package couldn't be sync'ed
			PACKSYNC,
			//! A package is damaged
			PACKVERIFY,
			//! A package couldn't be loaded (out of memory?)
			PACKLOAD,
			//! A file couldn't be found
			FILE_NOT_FOUND,
			//! Logger error (obviously these won't be logged!)
			LOGFILE_ERROR
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
		/*!
		 * \remarks
		 * MCS - I can't call this GetMessage because of a stupid windows macro 
		 * which redefines that as GetMessageW!
		 */
		const std::string& GetError() const;

	protected:
		//! Stores the error type
		ErrorType m_Type;
		//! Stores the message
		std::string m_Message;

	};

}

#endif
