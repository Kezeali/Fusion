
#include "FusionMessageBuilder.h"

#include <boost/archive/text_iarchive.hpp>

using namespace FusionEngine;

FusionMessage *FusionMessageBuilder::BuildMessage(const ShipState &input)
{
	unsigned char *data[];

	boost::archive::text_oarchive(
}

FusionMessage *FusionMessageBuilder::BuildMessage(const FusionEngine::ProjectileState &input)
{
}

FusionMessage *FusionMessageBuilder::BuildMessage(const Packet &packet)
{
}