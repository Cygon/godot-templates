#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "LexicalCast.h"

#include <cstdlib>
#include <string>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Maximum length of a floating point number converted to string</summary>
#if defined(NUCLEX_CPPEXAMPLE_WINDOWS)
  const std::size_t MaximumFloatCharacters = _CVTBUFSIZE; // Microsoft-specific
#else
  const std::size_t MaximumFloatCharacters = 32;
#endif

  /// <summary>Maximum length of a 64 bit integer platform</summary>
  const std::size_t MaximumIntCharacters = 21;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace CppExample { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  template<> godot::String lexical_cast<>(const float &from) {
    // Ordinarily gcvt() gets the job done, but Posix 2008 removed it :-(
    char characters[MaximumFloatCharacters];
#if defined(NUCLEX_CPPEXAMPLE_WINDOWS)
    return godot::String(::_gcvt(from, 0, characters));
#else
    ::snprintf(characters, sizeof(characters), "%f", static_cast<double>(from));
    return godot::String(characters);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const godot::String &from) {
    godot::CharString asciiString = from.ascii();
    return std::atof(asciiString.get_data());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> godot::String lexical_cast<>(const double &from) {
    char characters[MaximumFloatCharacters];
#if defined(NUCLEX_CPPEXAMPLE_WINDOWS)
    return godot::String(::_gcvt(from, 0, characters));
#else
    ::snprintf(characters, sizeof(characters), "%f", static_cast<double>(from));
    return godot::String(characters);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const godot::String &from) {
    godot::CharString asciiString = from.ascii();
    std::string stdString(
      asciiString.get_data(), static_cast<std::size_t>(asciiString.length())
    );
    return std::stod(stdString);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> godot::String lexical_cast<>(const int &from) {
    char characters[MaximumIntCharacters];
#if defined(NUCLEX_CPPEXAMPLE_WINDOWS)
    return godot::String(::itoa(from, characters, 10));
#else
    ::snprintf(characters, sizeof(characters), "%d", static_cast<int>(from));
    return godot::String(characters);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> int lexical_cast<>(const godot::String &from) {
    godot::CharString asciiString = from.ascii();
    return std::atoi(asciiString.get_data());
  }

  // ------------------------------------------------------------------------------------------- //

  template<> godot::String lexical_cast<>(const unsigned long &from) {
    char characters[MaximumIntCharacters];
#if defined(NUCLEX_CPPEXAMPLE_WINDOWS)
    return godot::String(::ultoa(from, characters, 10));
#else
    ::snprintf(characters, sizeof(characters), "%d", static_cast<int>(from));
    return godot::String(characters);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const godot::String &from) {
    if(from.length() == 4) {
      static const char lowerChars[] = { 't', 'r', 'u', 'e' };
      static const char upperChars[] = { 'T', 'R', 'U', 'E' };

      for(std::size_t index = 0; index < 4; ++index) {
        if((from[index] != lowerChars[index]) && (from[index] != upperChars[index])) {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  template<> godot::String lexical_cast<>(const bool &from) {
    static const godot::String trueString("true");
    static const godot::String falseString("false");

    if(from) {
      return trueString;
    } else {
      return falseString;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Support
