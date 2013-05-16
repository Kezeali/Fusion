/*
*  Copyright (c) 2013 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionEASTLScalableAllocator.h"

#include <EABase/eabase.h>

#include <tbb/scalable_allocator.h>

//#define ENABLE_ALIGNMENT

#pragma warning(push)
#pragma warning(disable:4996) 
int Vsnprintf8(char8_t* pDestination, size_t n, const char8_t* pFormat, va_list arguments)
{
#ifdef _MSC_VER
	return _vsnprintf(pDestination, n, pFormat, arguments);
#else
	return vsnprintf(pDestination, n, pFormat, arguments);
#endif
}
#pragma warning(pop)

// New & delete operators used by the standard EASTL allocator
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
#ifdef ENABLE_ALIGNMENT
	return scalable_aligned_malloc(size, 8);
#else
	return scalable_malloc(size);
#endif
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
#ifdef ENABLE_ALIGNMENT
	return scalable_aligned_malloc(size, alignment);
#else
	FSN_ASSERT(alignment <= 8);
	return scalable_malloc(size);
#endif
}

void __cdecl operator delete[](void* pMemory)	
{
#ifdef ENABLE_ALIGNMENT
	scalable_aligned_free(pMemory);
#else
	scalable_free(pMemory);
#endif
}