/*
  Copyright (c) 2006-2007 Fusion Project Team

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

		
	File Author(s):

		Elliot Hayward
*/

#include "FusionShipState.h"

#include <RakNet/NetworkTypes.h>
#include <RakNet/BitStream.h>


namespace FusionEngine
{

	int ShipState::Save(char* buffer) const
	{
		RakNet::BitStream out_stream;

		// PlayerID
		out_stream.Write(PID);
		// Pos
		out_stream.Write(position.x);
		out_stream.Write(position.y);
		// Vel
		out_stream.Write(velocity.x);
		out_stream.Write(velocity.y);
		// Rotation / RotVel
		out_stream.Write(rotation);
		out_stream.Write(rotationalVelocity);
		// Active weapons
		out_stream.Write(current_primary);
		out_stream.Write(current_secondary);
		out_stream.Write(current_bomb);
		// Available components
		out_stream.Write(engines);
		out_stream.Write(weapons);

		return out_stream.GetData();
	}

	ShipState ShipState::Load(const char* data, int length)
	{
		ShipState state;

		RakNet::BitStream stream(data, length, false);

		stream.Read(state.PID);

		stream.Read(state.position.x);
		stream.Read(state.position.y);

		stream.Write(state.velocity.x);
		stream.Write(state.velocity.y);

		stream.Write(state.rotation);
		stream.Write(state.rotationalVelocity);

		stream.Write(state.current_primary);
		stream.Write(state.current_secondary);
		stream.Write(state.current_bomb);

		stream.Write(state.engines);
		stream.Write(state.weapons);

		return state;

	}

}