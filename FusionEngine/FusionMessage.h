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

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace FusionEngine
{

	/*!
	 * \brief High level network messaging.
	 */
	class FusionMessage
	{
		friend class boost::serialization::access;

	public:
		//! Constructor.
		FusionMessage(const std::string &type);
		//! virtual Destructor
		virtual ~FusionMessage();

	public:
		//! Returns the serialised form of this message.
		virtual std::string &Serialize() const;
		//! Creates a message instance from its serialised form.
		virtual static FusionMessage *Deserialize(const std::string &serialized);

	private:
		/*!
		 * \brief
		 * The specific type of message.
		 *
		 * eg. if it were in the gameplay channel, it could be a 'player frame' type, 'change 
		 * state' type, etc.
		 *
		 * \remarks
		 * This should only be considered slightly more specific than the channel, and more
		 * specifics of the message should be controlled by the implementation. For example,
		 * the 'change state' type could have another member, m_State holding the acutal
		 * state-name to change to.
		 */
		std::string m_Type;

		template <class Archive>
		void serialize(Archive &ar, const unsigned int ver);
	};

}

#endif