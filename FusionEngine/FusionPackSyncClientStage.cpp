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

#include "FusionPackSyncClientStage.h"

namespace FusionEngine
{

	float PackSyncClientStage::Update(unsigned int split)
	{
		switch (m_Stage)
		{
		case PSCS_QUERY:
			// Ask the server where the fileserver is...
			m_Stage = PSCS_CONNECT;
			break;
		case PSCS_CONNECT:
			// Wait for a reply with the fileserver address...
			// Connect to the given address a reply is received...
			m_Stage = PSCS_SYNC;
			case 

	void PackSyncClientStage::OnFile(
		unsigned fileIndex,
		char *filename,
		char *fileData,
		unsigned compressedTransmissionLength,
		unsigned finalDataLength,
		unsigned short setID,
		unsigned setCount,	
		unsigned setTotalCompressedTransmissionLength,
		unsigned setTotalFinalLength,
		unsigned char context)
	{
		if (setCount > 0)
		{
			m_Progress = fileIndex/setCount;
		}
		
	}

}