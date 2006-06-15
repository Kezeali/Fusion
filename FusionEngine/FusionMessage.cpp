
#include "FusionMessage.h"

using namespace FusionEngine;

std::string &FusionMessage::Serialize() const
{
	std::stringstream output;

	{
		boost::archive::text_oarchive archive(output);

		// Here boost::archive will call serialize(archive, 0) from this object.
		archive << *this;
	}

	return output;
}

template <class Archive>
void FusionMessage::serialize(Archive &ar, const unsigned int ver)
{
	ar & m_Type;
}