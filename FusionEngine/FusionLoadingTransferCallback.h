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

#ifndef Header_FusionEngine_LoadingTransferCallback
#define Header_FusionEngine_LoadingTransferCallback

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <RakNet/FileListTransfer.h>
#include <RakNet/FileListTransferCBInterface.h>

namespace FusionEngine
{

	//! Updates the loading GUI when a file is synced
	/*!
	 * \remarks Perhaps anything that uses this callback should just be callbacks themselves
	 *  (i.e. implement the interface themselves) assuming only one object needs to use the callback
	 *  at a time.
	 */
	class LoadingTransferCallback : public FileListTransferCBInterface
	{
	public:
		void OnFile(
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
			// Pass this event to any listeners
			if (setCount > 0)
				SigOnFile(fileIndex/setCount, std::string(filename));
		}

		CL_Signal_v2<unsigned int, std::string> SigOnFile;
	};

}

#endif
