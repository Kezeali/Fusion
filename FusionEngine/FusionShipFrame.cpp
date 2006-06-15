
#include "FusionShipFrame.h"

using namespace FusionEngine;

template <class Archive>
FusionShipFrame::serialize(Archive &ar, const unsigned int ver)
{
	// Serialize members declared in FusionMessage
	ar & boost::serialization::base_object<FusionMessage>(*this);

	ar & time & input & state;
}