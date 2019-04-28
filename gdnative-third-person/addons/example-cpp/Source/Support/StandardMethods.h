#ifndef NUCLEX_CPPEXAMPLE_SUPPORT_STANDARDMETHODS_H
#define NUCLEX_CPPEXAMPLE_SUPPORT_STANDARDMETHODS_H

#include "../Config.h"

#include <Godot.hpp>

namespace Nuclex { namespace CppExample { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helpers to call standard methods commonly implemented in Godot classes</summary>
  /// <remarks>
  ///   <para>
  ///     The entire type system for game logic in Godot is tailored to dynamic typing,
  ///     thus, even from C++, if we want to do things right, we have to call methods
  ///     that are not guaranteed through a base class by name.
  ///   </para>
  ///   <para>
  ///     Godot's type system does allow for inheritance, though, so if you have specific
  ///     methods that belong together, you can define an interface / base class and then
  ///     call them with type safety (all details will be dealt with by godot-cpp).
  ///   </para>
  /// </remarks>
  class StandardMethods {

    /// <summary>Calls the _ready() method if it is implemented by an object</summary>
    /// <param name="instance">Object on which the _ready() method will be called</param>
    /// <returns>True if the object implemented a _ready() method, false otherwise</returns>
    public: static bool TryCallReady(godot::Variant &instance);

    /// <summary>Calls the _process() method if it is implemented by an object</summary>
    /// <param name="instance">Object on which the _process() method will be called</param>
    /// <param name="deltaSeconds">
    ///   Amount of passed time that will be reported to the method
    /// </param>
    /// <returns>True if the object implemented a _process() method, false otherwise</returns>
    public: static bool TryCallProcess(
      godot::Variant &instance, float deltaSeconds
    );

    /// <summary>Calls the _physics_process() method if it is implemented by an object</summary>
    /// <param name="instance">
    ///   Object on which the _physics_process() method will be called
    /// </param>
    /// <param name="deltaSeconds">
    ///   Amount of passed time that will be reported to the method
    /// </param>
    /// <returns>
    ///   True if the object implemented a _physics_process() method, false otherwise
    /// </returns>
    public: static bool TryCallPhysicsProcess(
      godot::Variant &instance, float deltaSeconds
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Support

#endif // NUCLEX_CPPEXAMPLE_SUPPORT_STANDARDMETHODS_H
