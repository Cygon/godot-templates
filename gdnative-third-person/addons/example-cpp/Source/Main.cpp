#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "Main.h"

#include "Actors/ActorPhysics.h"
#include "Actors/CharacterController.h"
#include "Actors/CharacterPresenter.h"
#include "Cameras/DebugFlyCamera.h"
#include "Geometry/Trigonometry.h"

// --------------------------------------------------------------------------------------------- //

extern "C" void NUCLEX_CPPEXAMPLE_API godot_gdnative_init(godot_gdnative_init_options *o) {
  godot::Godot::gdnative_init(o);
}

// --------------------------------------------------------------------------------------------- //

extern "C" void NUCLEX_CPPEXAMPLE_API godot_gdnative_terminate(
  godot_gdnative_terminate_options *o
) {
  godot::Godot::gdnative_terminate(o);
}

// --------------------------------------------------------------------------------------------- //

extern "C" void NUCLEX_CPPEXAMPLE_API godot_nativescript_init(void *handle) {
  godot::Godot::nativescript_init(handle);

  godot::register_class<Nuclex::CppExample::Actors::ActorPhysics>();
  godot::register_class<Nuclex::CppExample::Actors::CharacterController>();
  godot::register_class<Nuclex::CppExample::Actors::CharacterPresenter>();
  godot::register_class<Nuclex::CppExample::Cameras::DebugFlyCamera>();
  godot::register_class<Nuclex::CppExample::Geometry::Trigonometry>();
}

// --------------------------------------------------------------------------------------------- //
