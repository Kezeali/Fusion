/*
  Copyright (c) 2006 Elliot Hayward

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

#ifndef Header_FusionEngine_FusionMessage
#define Header_FusionEngine_FusionMessage

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "../RakNet/Bitstream.h"

namespace FusionEngine
{

	/*!
	 * \brief High level network messaging.
	 */
	class FusionMessage
	{
	public:
		//! Constructor.
		FusionMessage(int type);
		//! virtual Destructor
		virtual ~FusionMessage();

	public:

	private:
		//! Does all the work so I don't have to
		BitStream m_DataStream;

		/*!
		 * \brief
		 * The specific type of message.
		 *
		 * eg. if it were in the gameplay channel, it could be a 'player frame' type, 'change 
		 * state' type, etc.
		 *
		 * \remarks
		 * The following remark is [depreciated]; it only applied when messages were
		 * serialised objects.
		 * <br>
		 * This should only be considered slightly more specific than the channel, and more
		 * specifics of the message should be controlled by the implementation. For example,
		 * the 'change state' type could have another member, m_State holding the acutal
		 * state-name to change to.
		 */
		int m_Type;

	};

}

#endif