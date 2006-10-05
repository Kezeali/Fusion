
#include "FusionShipState.h"

using namespace FusionEngine;

template <class Archive>
void ShipState::serialize(Archive &ar, unsigned int ver)
{
	ar & position & velocity & facing & spin;
	ar & mass;
	ar & current_primary & current_secondary & current_bomb;
	ar & engines & weapons;
}