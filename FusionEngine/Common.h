#ifndef Header_FusionCommon
#define Header_FusionCommon

#if _MSC_VER > 1000
#pragma once
#endif

#if defined (_MSC_VER) || defined (__APPLE_CC__)

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

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <array>

// ClanLib
//#if defined (_WIN32) && !defined (_DLL)
//#define HAD_TO_DEF_DLL
//#define _DLL
//#endif
#define WIN32_LEAN_AND_MEAN // If this isn't defined something called 'interface' (keyword or identifier)
//  causes problems (at least that seems to be what's happening.)
#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>
#include <ClanLib/regexp.h>
// Remove _DLL define
//#ifdef HAD_TO_DEF_DLL
//#undef _DLL
//#undef HAD_TO_DEF_DLL
//#endif

// Box2d (Physics)
#include <Box2D/Box2D.h>

// Boost
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/lexical_cast.hpp>

// XML
#define USE_TINYXML
//! XML version to write to the xml declaration of new files
#define XML_STANDARD "1.0"

#ifdef USE_XERCES
#include <xercesc/util/PlatformUtils.hpp>
#include <xqilla/xqilla-simple.hpp>

#elif defined(USE_TINYXML)
//#define USE_TINYXPATH
#ifdef USE_TINYXPATH
#include "../tinyxml/xpath_static.h"
#include "../tinyxml/ticpp.h"
#else
#include "../tinyxml/ticpp.h"
#endif
#endif

// AngelScript
#if defined _WIN32 && !defined ANGELSCRIPT_DLL_LIBRARY_IMPORT
#define ANGELSCRIPT_DLL_LIBRARY_IMPORT
#endif
#include <angelscript.h>

// Fusion
#include "FusionCommon.h"

// Errors/Exceptions
#include "FusionAssert.h"
#include "FusionException.h"
#include "FusionExceptionFactory.h"

#include "FusionVector2.h"

#include "FusionSlotContainer.h"

#include "FusionHashable.h"

#include "FusionStringFormatting.h"


#endif

#endif
