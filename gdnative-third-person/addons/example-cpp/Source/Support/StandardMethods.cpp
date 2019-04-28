#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "StandardMethods.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  // Space for constants. Nothing here yet :-)

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace CppExample { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  bool StandardMethods::TryCallReady(godot::Variant &instance) {
    const static godot::String ReadyMethodName("_ready");
    const static godot::Variant EmptyArguments[1];

    if(instance.has_method(ReadyMethodName)) {
      const godot::Variant *arguments = EmptyArguments;
      instance.call(ReadyMethodName, &arguments, 0);
      return true;
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool StandardMethods::TryCallProcess(
    godot::Variant &instance, float deltaSeconds
  ) {
    const static godot::String ProcessMethodName("_process");

    if(instance.has_method(ProcessMethodName)) {
      const godot::Variant arguments[1] = {
        godot::Variant(deltaSeconds)
      };
      const godot::Variant *argumentsArray = arguments;
      instance.call(ProcessMethodName, &argumentsArray, 1);
      return true;
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool StandardMethods::TryCallPhysicsProcess(
    godot::Variant &instance, float deltaSeconds
  ) {
    const static godot::String PhysicsProcessMethodName("_physics_process");

    if(instance.has_method(PhysicsProcessMethodName)) {
      godot::Variant arguments[1] = {
        godot::Variant(deltaSeconds)
      };
      const godot::Variant *argumentsArray = arguments;
      instance.call(PhysicsProcessMethodName, &argumentsArray, 1);
      return true;
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Support
