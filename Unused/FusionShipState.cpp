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

	ShipState::ShipState(const char* data, int length)
	{
		load(data, length);
	}

	int ShipState::Save(char* buffer) const
	{
		RakNet::BitStream out_stream;

		// SystemAddress
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

		int bufLen = out_stream.GetNumberOfBytesUsed();

		// Allocate the ptr and copy data
		buffer = malloc(bufLen);
		memcpy(buffer, out_stream.GetData(), bufLen);

		return bufLen;
	}

	void ShipState::load(const char* data, int length)
	{
		RakNet::BitStream stream(data, length, false);

		stream.Read(PID);

		stream.Read(position.x);
		stream.Read(position.y);

		stream.Read(velocity.x);
		stream.Read(velocity.y);

		stream.Read(rotation);
		stream.Read(rotationalVelocity);

		stream.Read(current_primary);
		stream.Read(current_secondary);
		stream.Read(current_bomb);

		stream.Read(engines);
		stream.Read(weapons);

	}

}