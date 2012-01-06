#include "PrecompiledHeaders.h"

#include <ClanLib/application.h>

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		return 0;
	}
};

CL_ClanApplication app(&EntryPoint::main);
