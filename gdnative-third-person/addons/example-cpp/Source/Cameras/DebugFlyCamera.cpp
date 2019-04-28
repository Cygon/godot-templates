#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "DebugFlyCamera.h"
#include "../Geometry/Trigonometry.h"

#include <Input.hpp>
#include <GlobalConstants.hpp>
#include <InputEventMouseMotion.hpp>
#include <InputEventKey.hpp>
#include <SceneTree.hpp>

#include <cassert>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Default bounding box the debug fly camera will be limited to</summary>
  const godot::AABB DefaultBoundingBox(
    godot::Vector3(-5.0f, -5.0f, -5.0f),
    godot::Vector3(10.0f, 10.0f, 10.0f)
  );

  /// <summary>Default number units a new debug fly camera will move per second</summary>
  const float DefaultMovementUnitsPerSecond = 3.0f;

  /// <summary>Default degrees a new debug fly camera will rotate with mouse movements</summary>
  const float DefaultRotationDegreesPerMickey = 0.1f;

  /// <summary>Default setting for the quit via escape option</summary>
  const bool DefaultQuitViaEscapeKey = false;

  /// <summary>Clamps the provided value into the range min .. max</summary>
  /// <param name="value">Value that will be clamped into the specified range</param>
  /// <param name="min">Minimum value that will be returned</param>
  /// <param name="max">Maximum value that will be returned</param>
  /// <returns>The provided input value clamped to the specified range</returns>
  float clamp(float value, float min, float max) {
    if(value < min) {
      return min;
    }
    if(value >= max) {
      return max;
    }
    return value;
  }

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace CppExample { namespace Cameras {

  // ------------------------------------------------------------------------------------------- //

  DebugFlyCamera::DebugFlyCamera() :
    Boundaries(DefaultBoundingBox),
    MovementUnitsPerSecond(DefaultMovementUnitsPerSecond),
    RotationDegreesPerMickey(DefaultRotationDegreesPerMickey),
    QuitViaEscapeKey(DefaultQuitViaEscapeKey),
    inputManager(nullptr) {}

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::_init() {
    this->Boundaries = DefaultBoundingBox;
    this->MovementUnitsPerSecond = DefaultMovementUnitsPerSecond;
    this->RotationDegreesPerMickey = DefaultRotationDegreesPerMickey;
    this->QuitViaEscapeKey = DefaultQuitViaEscapeKey;
  }

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::enter_tree() {
    godot::Input *inputManager = getInputManager();
    if(inputManager == nullptr) {
      return;
    }
    //godot::Godot::print("DebugFlyCamera: capturing mouse cursor");
    inputManager->set_mouse_mode(godot::Input::MOUSE_MODE_CAPTURED);
  }

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::exit_tree() {
    godot::Input *inputManager = getInputManager();
    if(inputManager == nullptr) {
      return;
    }
    inputManager->set_mouse_mode(godot::Input::MOUSE_MODE_VISIBLE);
  }

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::process(float deltaSeconds) {
    godot::Vector3 movementInputs = readMovementInputs();
    movementInputs *= (this->MovementUnitsPerSecond * deltaSeconds);
    translate(movementInputs);
    limitPositionToBoundaries();
  }

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::input(const godot::InputEvent *inputEvent) {
    godot::InputEventMouseMotion *mouseMotionEvent = (
      godot::Object::cast_to<godot::InputEventMouseMotion>(inputEvent)
    );
    if(mouseMotionEvent != nullptr) {
      processMouseMotion(*mouseMotionEvent);
    }

    // If the option is enabled, check if the Escape key was pressed and quit if so
    if(this->QuitViaEscapeKey) {
      godot::InputEventKey *keyEvent = (
        godot::Object::cast_to<godot::InputEventKey>(inputEvent)
      );
      if(keyEvent != nullptr) {
        if(keyEvent->is_pressed()) {
          if(keyEvent->get_scancode() == godot::GlobalConstants::KEY_ESCAPE) {
            get_tree()->quit();
          }
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::_register_methods() {
    godot::register_property<DebugFlyCamera, godot::AABB>(
      "boundaries",
      &DebugFlyCamera::Boundaries, DefaultBoundingBox
    );
    godot::register_property<DebugFlyCamera, float>(
      "movement_units_per_second",
      &DebugFlyCamera::MovementUnitsPerSecond, DefaultMovementUnitsPerSecond
    );
    godot::register_property<DebugFlyCamera, float>(
      "rotation_degrees_per_mickey",
      &DebugFlyCamera::RotationDegreesPerMickey, DefaultRotationDegreesPerMickey
    );
    godot::register_property<DebugFlyCamera, bool>(
      "quit_via_escape_key",
      &DebugFlyCamera::QuitViaEscapeKey, DefaultQuitViaEscapeKey
    );

    godot::register_method("_process", &DebugFlyCamera::process);
    godot::register_method("_input", &DebugFlyCamera::input);
    godot::register_method("_enter_tree", &DebugFlyCamera::enter_tree);
    godot::register_method("_exit_tree", &DebugFlyCamera::exit_tree);
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Vector3 DebugFlyCamera::readMovementInputs() const {
    godot::Vector3 movementInputs(0.0f, 0.0f, 0.0f);

    const godot::Input *inputManager = getInputManager();
    if(inputManager == nullptr) {
      return godot::Vector3(0.0f, 0.0f, 0.0f);
    }

    if(inputManager->is_key_pressed(godot::GlobalConstants::KEY_A)) {
      movementInputs.x -= 1.0f;
    }
    if(inputManager->is_key_pressed(godot::GlobalConstants::KEY_D)) {
      movementInputs.x += 1.0f;
    }
    if(inputManager->is_key_pressed(godot::GlobalConstants::KEY_Q)) {
      movementInputs.y -= 1.0f;
    }
    if(inputManager->is_key_pressed(godot::GlobalConstants::KEY_E)) {
      movementInputs.y += 1.0f;
    }
    if(inputManager->is_key_pressed(godot::GlobalConstants::KEY_W)) {
      movementInputs.z -= 1.0f;
    }
    if(inputManager->is_key_pressed(godot::GlobalConstants::KEY_S)) {
      movementInputs.z += 1.0f;
    }

    return movementInputs;
  }

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::processMouseMotion(const godot::InputEventMouseMotion &mouseMotionEvent) {

    // Calculate the amount of rotation we need to perform
    godot::Vector2 relativeMotion = mouseMotionEvent.get_relative();
    relativeMotion.x *= this->RotationDegreesPerMickey;
    relativeMotion.y *= this->RotationDegreesPerMickey;
    relativeMotion.x *= Geometry::Trigonometry::RadiansPerDegree;
    relativeMotion.y *= Geometry::Trigonometry::RadiansPerDegree;

    // Apply the rotation to the node
    {
      godot::Vector3 eulerAngles = get_rotation();

      eulerAngles.y = std::fmod(eulerAngles.y - relativeMotion.x, Geometry::Trigonometry::Tau);
      eulerAngles.x = clamp(
        eulerAngles.x - relativeMotion.y,
        -Geometry::Trigonometry::HalfPI + Geometry::Trigonometry::RadiansPerDegree,
        +Geometry::Trigonometry::HalfPI - Geometry::Trigonometry::RadiansPerDegree
      );

      set_rotation(eulerAngles);
    }

  }

  // ------------------------------------------------------------------------------------------- //

  void DebugFlyCamera::limitPositionToBoundaries() {
    godot::Transform transform = get_transform();

    {
      godot::Vector3 boundsMin = this->Boundaries.position;
      godot::Vector3 boundsMax = boundsMin + this->Boundaries.size;

      transform.origin.x = clamp(transform.origin.x, boundsMin.x, boundsMax.x);
      transform.origin.y = clamp(transform.origin.y, boundsMin.y, boundsMax.y);
      transform.origin.z = clamp(transform.origin.z, boundsMin.z, boundsMax.z);
    }

    set_transform(transform);
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Input *DebugFlyCamera::getInputManager() const {
    if(this->inputManager == nullptr) {
      this->inputManager = godot::Input::get_singleton();

      // Stil no input manager? That's an error.
      if(this->inputManager == nullptr) {
        godot::Godot::print("ERROR: DebugFlyCamera could not access Godot's input manager");
        assert(this->inputManager != nullptr);
      }
    }

    return this->inputManager;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Cameras
