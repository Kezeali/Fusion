#ifndef H_FusionStableHeaders
#define H_FusionStableHeaders

#if _MSC_VER > 1000
#pragma once
#endif

#if defined (_MSC_VER) || defined (__APPLE_CC__)

#include <angelscript.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

#include <Box2D/Box2D.h>

#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>

#include <tbb/concurrent_queue.h>
#include <tbb/scalable_allocator.h>
#include <tbb/task.h>

#include "FusionAssert.h"
#include "FusionCommon.h"
#include "FusionException.h"
#include "FusionExceptionFactory.h"
#include "FusionHashable.h"
#include "FusionSerialisationHelper.h"
#include "FusionSlotContainer.h"
#include "FusionStdHeaders.h"
#include "FusionStringFormatting.h"
#include "FusionVector2.h"
#include "FusionVectorTypes.h"
#include "FusionXML.h"

#endif

#endif