#ifndef Header_FusionStableHeaders
#define Header_FusionStableHeaders

#if _MSC_VER > 1000
#pragma once
#endif

#if defined (_MSC_VER) || defined (__APPLE_CC__)

#include <angelscript.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <Box2D/Box2D.h>

#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>
#include <ClanLib/regexp.h>
#ifdef GetObject
#undef GetObject
#endif

#include "FusionAssert.h"
#include "FusionCommon.h"
#include "FusionException.h"
#include "FusionExceptionFactory.h"
#include "FusionHashable.h"
#include "FusionSlotContainer.h"
#include "FusionStdHeaders.h"
#include "FusionStringFormatting.h"
#include "FusionVector2.h"
#include "FusionXML.h"

#endif

#endif
