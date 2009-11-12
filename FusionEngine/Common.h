#ifndef Header_FusionCommon
#define Header_FusionCommon

#if _MSC_VER > 1000
#pragma once
#endif

//#include <cassert>

// STL
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <deque>
#include <algorithm>
#include <numeric>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

// STL TR1
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <array>

// Add _DLL define
#ifndef _DLL
#define HAD_TO_DEF_DLL
#define _DLL
#endif
// ClanLib
#define WIN32_LEAN_AND_MEAN // If this isn't defined the 'interface' keyword causes problems (at least that seems to be what's happening.)
#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>
#include <ClanLib/regexp.h>
// Remove _DLL define
#ifdef HAD_TO_DEF_DLL
#undef _DLL
#endif

#endif
