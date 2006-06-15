#ifndef Header_FusionPrereq
#define Header_FusionPrereq

#if _MSC_VER > 1000
#pragma once
#endif

// Thanks to the Ogre (ogre3d.org) devs for the following setup.

namespace FusionEngine
{

/* Initial platform/compiler-related stuff to set.
*/
#define FUSION_PLATFORM_WIN32 1
#define FUSION_PLATFORM_LINUX 2
#define FUSION_PLATFORM_APPLE 3

#define FUSION_COMPILER_MSVC 1
#define FUSION_COMPILER_GNUC 2
#define FUSION_COMPILER_BORL 3

#define FUSION_ENDIAN_LITTLE 1
#define FUSION_ENDIAN_BIG 2

#define FUSION_ARCHITECTURE_32 1
#define FUSION_ARCHITECTURE_64 2

/* Finds the compiler type and version.
*/
#if defined( _MSC_VER )
#   define FUSION_COMPILER FUSION_COMPILER_MSVC
#   define FUSION_COMP_VER _MSC_VER

#elif defined( __GNUC__ )
#   define FUSION_COMPILER FUSION_COMPILER_GNUC
#   define FUSION_COMP_VER (((__GNUC__)*100) + \
        (__GNUC_MINOR__*10) + \
        __GNUC_PATCHLEVEL__)

#elif defined( __BORLANDC__ )
#   define FUSION_COMPILER FUSION_COMPILER_BORL
#   define FUSION_COMP_VER __BCPLUSPLUS__

#else
#   pragma error "No known compiler. Abort! Abort!"

#endif

		#if FUSION_COMPILER == FUSION_COMPILER_GNUC && FUSION_COMP_VER >= 310
    #   define HashMap ::__gnu_cxx::hash_map
    #else
    #   if FUSION_COMPILER == FUSION_COMPILER_MSVC
    #       if FUSION_COMP_VER > 1300 && !defined(_STLP_MSVC)
    #           define HashMap ::stdext::hash_map
    #       else
    #           define HashMap ::std::hash_map
    #       endif
    #   else
    #       define HashMap ::std::hash_map
    #   endif
    #endif

}

#endif