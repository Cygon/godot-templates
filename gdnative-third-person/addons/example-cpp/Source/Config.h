#pragma region Copyright
/*
CppExample c++ Game Code Library
Copyright (C) 2008-2019 Premium Hentai Development
*/
#pragma endregion // Copyright

#ifndef NUCLEX_CPPEXAMPLE_CONFIG_H
#define NUCLEX_CPPEXAMPLE_CONFIG_H

// --------------------------------------------------------------------------------------------- //

// Platform recognition
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
  #define NUCLEX_CPPEXAMPLE_WINRT 1
#elif defined(WIN32) || defined(_WIN32)
  #define NUCLEX_CPPEXAMPLE_WIN32 1
#else
  #define NUCLEX_CPPEXAMPLE_LINUX 1
#endif

// --------------------------------------------------------------------------------------------- //

// C++ language features
#if defined(_MSC_VER) && (_MSC_VER >= 1900) // Visual Studio 2015 has the C++14 features we use
  #define NUCLEX_CPPEXAMPLE_CXX14 1
#elif defined(__clang__) && defined(__cplusplus) && (__cplusplus >= 201402)
  #define NUCLEX_CPPEXAMPLE_CXX14 1
#elif (defined(__GNUC__) || defined(__GNUG__)) && defined(__cplusplus) && (__cplusplus >= 201402)
  #define NUCLEX_CPPEXAMPLE_CXX14 1
#else
  #error The Nuclex.CppExample.Native library requires a C++14 compiler
#endif

// --------------------------------------------------------------------------------------------- //

// Endianness detection
#if defined(_MSC_VER) // MSVC is always little endian, including Windows on ARM
  #define NUCLEX_CPPEXAMPLE_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) // GCC
  #define NUCLEX_CPPEXAMPLE_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) // GCC
  #define NUCLEX_CPPEXAMPLE_BIG_ENDIAN 1
#else
  #error Could not determine whether platform is big or little endian
#endif

// --------------------------------------------------------------------------------------------- //

// Standard library features
//#define HAVE_ATOI 1
//#define HAVE_ITOA 1
//#define HAVE_ULTOA 1

// --------------------------------------------------------------------------------------------- //

// Decides whether symbols are imported from a dll (client app) or exported to
// a dll (Nuclex.Storage.Native library). The NUCLEX_CPPEXAMPLE_SOURCE symbol is defined by
// all source files of the library, so you don't have to worry about a thing.
#if defined(_MSC_VER)

  #if defined(NUCLEX_CPPEXAMPLE_STATICLIB)
    #define NUCLEX_CPPEXAMPLE_API
  #else
    #if defined(NUCLEX_CPPEXAMPLE_SOURCE)
      // If we are building the DLL, export the symbols tagged like this
      #define NUCLEX_CPPEXAMPLE_API __declspec(dllexport)
    #else
      // If we are consuming the DLL, import the symbols tagged like this
      #define NUCLEX_CPPEXAMPLE_API __declspec(dllimport)
    #endif
  #endif

#elif defined(__GNUC__)

  #if defined(NUCLEX_CPPEXAMPLE_STATICLIB)
    #define NUCLEX_CPPEXAMPLE_API
  #else
    #if defined(NUCLEX_CPPEXAMPLE_SOURCE)
      #define NUCLEX_CPPEXAMPLE_API __attribute__ ((visibility ("default")))
    #else
      // If you use -fvisibility=hidden in GCC, exception handling and RTTI would break 
      // if visibility wasn't set during export _and_ import because GCC would immediately
      // forget all type infos encountered. See http://gcc.gnu.org/wiki/Visibility
      #define NUCLEX_CPPEXAMPLE_API __attribute__ ((visibility ("default")))
    #endif
  #endif

#else

  #error Unknown compiler, please implement shared library macros for your system

#endif

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_CPPEXAMPLE_CONFIG_H
