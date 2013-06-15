#ifndef H_DLLCONFIG
#define H_DLLCONFIG

#if _MSC_VER > 1000
#define FSN_PLATFORM_WIN32
#endif

#if !defined FSN_STATIC_LIB

#if defined FSN_PLATFORM_WIN32 && defined _WINDLL

#ifdef FSN_DLL_EXPORTS
#define FSN_DLL_API __declspec(dllexport)
#else
#define FSN_DLL_API __declspec(dllimport)
#endif

#else // Not win32 dll
#define FSN_DLL_API// __attribute__((visibility("default")))
#endif

#else // FSN_STATIC_LIB

#define FSN_DLL_API

#endif

#endif
