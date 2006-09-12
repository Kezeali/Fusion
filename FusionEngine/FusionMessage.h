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