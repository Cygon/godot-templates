#ifndef NUCLEX_CPPEXAMPLE_SUPPORT_LEXICALCAST_H
#define NUCLEX_CPPEXAMPLE_SUPPORT_LEXICALCAST_H

#include "../Config.h"

#include <Godot.hpp>
#include <cassert>

namespace Nuclex { namespace CppExample { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts between a string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  /// <remarks>
  ///   <para>
  ///     A lexical cast between a numeric type and a string type that interprets
  ///     the contents of the string type as a written-out number. The terminology
  ///     comes from Boost.
  ///   </para>
  ///   <para>
  ///     This version pushes responsibility for any localization issues onto
  ///     the C++ standard library. It is intended for display and user entry,
  ///     not for serialization and network transmission (you could use setlocale
  ///     to change that, but you really shouldn't, it would affect the whole program).
  ///   </para>
  /// </remarks>
  template<typename TTarget, typename TSource>
  inline TTarget lexical_cast(const TSource &from) {
    assert(!"Unsupported pair of types for lexical_cast");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a floating point value into a string</summary>
  /// <param name="from">Floating point value that will be converted</param>
  /// <returns>A string containing the printed floating point value</returns>
  template<> godot::String lexical_cast<>(const float &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> float lexical_cast<>(const godot::String &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a double precision floating point value into a string</summary>
  /// <param name="from">Double precision Floating point value that will be converted</param>
  /// <returns>A string containing the printed double precision floating point value</returns>
  template<> godot::String lexical_cast<>(const double &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> double lexical_cast<>(const godot::String &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an integer value into a string</summary>
  /// <param name="from">Integer value that will be converted</param>
  /// <returns>A string containing the printed integer value</returns>
  template<> godot::String lexical_cast<>(const int &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an integer value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The integer value parsed from the specified string</returns>
  template<> int lexical_cast<>(const godot::String &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an unsigned long value into a string</summary>
  /// <param name="from">Unsigned long value that will be converted</param>
  /// <returns>A string containing the printed unsigned long value</returns>
  template<> godot::String lexical_cast<>(const unsigned long &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a boolean value into a string</summary>
  /// <param name="from">Boolean value that will be converted</param>
  /// <returns>A string containing the printed boolean value</returns>
  template<> godot::String lexical_cast<>(const bool &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a boolean value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The boolean value parsed from the specified string</returns>
  template<> bool lexical_cast<>(const godot::String &from);

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Support

#endif // NUCLEX_CPPEXAMPLE_SUPPORT_LEXICALCAST_H
