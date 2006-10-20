
#include "FusionArchive.h"

#include "../zlib/contrib/minizip/unzip.h"

using namespace FusionEngine;

Archive::Archive()
{
}

Archive::Archive(std::string filename)
{
	Open(filename);
}

void Archive::Open(std::string filename)
{
	//nothing;
}

void Archive::Decompress()
{
	//nothing;
}

StringVector Archive::GetFileList()
{
	return ;
}

void Archive::DeleteTemps()
{

}
