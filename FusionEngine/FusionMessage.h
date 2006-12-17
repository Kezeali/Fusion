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

#ifndef Header_FusionEngine_FusionMessage
#define Header_FusionEngine_FusionMessage

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	/*!
	 * \brief High level network messaging.
	 */
	class FusionMessage
	{
	public:
		//! Basic Constructor. Don't use
		FusionMessage();
		//! Constructor
		FusionMessage(unsigned char *message, unsigned int length);
		//! Constructor. +channel +type +playerID
		FusionMessage(unsigned char channel, unsigned char type, PlayerID client);
		//! Constructor. +channel +type +playerID +message +length
		FusionMessage(unsigned char channel, unsigned char type, PlayerID client, unsigned char *message, unsigned int length);
		//! virtual Destructor
		virtual ~FusionMessage();

	public:
		//! Sets the message data
		/*!
		 * This will completely overwrite the data stored in the message
		 */
		void Write(unsigned char *data);
		//! Returns the message data
		unsigned char *Read() const;

		//! Reads the message data with the header removed
		/*!
		 * \param[out] buffer The buffer into which the data should be read.
		 */
		void ReadWithoutHeader(unsigned char *buffer) const;

		//! Sets the length of the data
		void SetLength(unsigned int length);
		//! Returns the length of the data
		unsigned int GetLength() const;

		//! Read PlayerID
		PlayerID GetPlayerInd() const;

	private:
		//! The client from whence it came
		/*!
		 * \remarks
		 * Remember: this property is server-side only; if a client wants to
		 * know what ship/object to apply a message to it looks at the ID stored
		 * in the received data.
		 */
		PlayerID m_ClientID;

		//! Data length
		unsigned int m_Length;
		//! The message serialised
		unsigned char *m_Message;

	};

}

#endif
