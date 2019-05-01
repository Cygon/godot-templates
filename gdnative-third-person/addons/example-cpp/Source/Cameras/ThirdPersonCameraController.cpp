#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "ThirdPersonCameraController.h"
#include "../Geometry/Trigonometry.h"

#include <Input.hpp>
#include <GlobalConstants.hpp>
#include <InputEventMouseMotion.hpp>
#include <InputEventKey.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>
#include <World.hpp>
#include <Spatial.hpp>
#include <Camera.hpp>

#include <cassert>
#include <gen/PhysicsServer.hpp>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Default path to the camera node controlled by this component</summary>
  const std::string DefaultCameraNodePath = "../..";

  /// <summary>Default offset of the camera's actual pivot to the target node</summary>
  const godot::Vector3 DefaultOffset = godot::Vector3(0.0f, 1.5f, 0.0f);

  /// <summary>How much the mouse wheel zooms in or out when turned one notch</summary>
  const float DefaultMouseWheelZoomSensitivity = 1.0f;

  /// <summary>Default for the shortest distance the camera can have from the target</summary>
  const float DefaultMinimumDistance = 1.25f;

  /// <summary>Default for the longest distance the camera can have from the target</summary>
  const float DefaultMaximumDistance = 10.0f;

  /// <summary>Default camera rotation amount per mouse movement</summary>
  const godot::Vector2 DefaultRotationDegreesPerMickey = godot::Vector2(0.5f, 0.5f);

  /// <summary>Default collision mask for things that block the camera's view</summary>
  const std::int64_t DefaultViewBlockingMask = 2147483647;

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

  ThirdPersonCameraController::ThirdPersonCameraController() :
    Offset(DefaultOffset),
    MouseWheelZoomSensitivity(DefaultMouseWheelZoomSensitivity),
    MinimumDistance(DefaultMinimumDistance),
    MaximumDistance(DefaultMaximumDistance),
    RotationDegreesPerMickey(DefaultRotationDegreesPerMickey),
    ViewBlockingMask(DefaultViewBlockingMask) {}

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::_init() {
    CameraController::_init();

    this->Offset = DefaultOffset;
    this->MouseWheelZoomSensitivity = DefaultMouseWheelZoomSensitivity;
    this->MinimumDistance = DefaultMinimumDistance;
    this->MaximumDistance = DefaultMaximumDistance;
    this->RotationDegreesPerMickey = DefaultRotationDegreesPerMickey;
    this->ViewBlockingMask = DefaultViewBlockingMask;
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::_register_methods() {

    // Would be cool to just include the methods from the base class like this,
    // but Godot-CPP is not designed to allow this. Duplication ahoy!
    //CameraController::_register_methods();

    godot::register_method("_process", &ThirdPersonCameraController::process);
    godot::register_method("_input", &ThirdPersonCameraController::input);
    godot::register_method("_enter_tree", &ThirdPersonCameraController::enter_tree);
    godot::register_method("_exit_tree", &ThirdPersonCameraController::exit_tree);

    godot::register_property<ThirdPersonCameraController, godot::NodePath>(
      "camera_node_path",
      &CameraController::CameraNodePath, godot::NodePath("../..")
    );
    godot::register_property<ThirdPersonCameraController, float>(
      "fade_level",
      &CameraController::FadeLevel, 1.0f
    );
    godot::register_property<ThirdPersonCameraController, godot::NodePath>(
      "target_node_path",
      &CameraController::TargetNodePath, godot::NodePath()
    );

    godot::register_property<ThirdPersonCameraController, godot::NodePath>(
      "camera_node_path",
      &ThirdPersonCameraController::CameraNodePath, godot::NodePath(DefaultCameraNodePath.c_str())
    );
    godot::register_property<ThirdPersonCameraController, godot::Vector3>(
      "offset",
      &ThirdPersonCameraController::Offset, DefaultOffset
    );
    godot::register_property<ThirdPersonCameraController, float>(
      "mouse_wheel_zoom_sensitivity",
      &ThirdPersonCameraController::MouseWheelZoomSensitivity, DefaultMouseWheelZoomSensitivity
    );
    godot::register_property<ThirdPersonCameraController, float>(
      "minimum_distance",
      &ThirdPersonCameraController::MinimumDistance, DefaultMinimumDistance
    );
    godot::register_property<ThirdPersonCameraController, float>(
      "maximum_distance",
      &ThirdPersonCameraController::MaximumDistance, DefaultMaximumDistance
    );
    godot::register_property<ThirdPersonCameraController, godot::Vector2>(
      "rotation_degrees_per_mickey",
      &ThirdPersonCameraController::RotationDegreesPerMickey, DefaultRotationDegreesPerMickey
    );
    godot::register_property<ThirdPersonCameraController, std::int64_t >(
      "view_blocking_mask",
      &ThirdPersonCameraController::ViewBlockingMask, DefaultViewBlockingMask
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::enter_tree() {
    godot::Input *inputManager = getInputManager();
    if(inputManager == nullptr) {
      return;
    }
    //godot::Godot::print("DebugFlyCamera: capturing mouse cursor");
    //inputManager->set_mouse_mode(godot::Input::MOUSE_MODE_CAPTURED);
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::exit_tree() {
    godot::Input *inputManager = getInputManager();
    if(inputManager == nullptr) {
      return;
    }
    inputManager->set_mouse_mode(godot::Input::MOUSE_MODE_VISIBLE);
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::process(float deltaSeconds) {
    CameraController::process(deltaSeconds);

    godot::Spatial *cameraNode = GetCameraNode();
    if(cameraNode == nullptr) {
      return;
    }
    // If there's no target, rotating in place is all the camera will do
    godot::Spatial *targetNode = GetTargetNode();
    if(targetNode == nullptr) {
      return;
    }

    moveToOrbitPosition(*cameraNode, *targetNode);


#if defined(REPRODUCE_GODOT_SEGFAULT)
    godot::Node *cameraAsNode = get_node(this->CameraNodePath);
    godot::String class_name = cameraAsNode->get_class();
    godot::Godot::print(class_name);
    //char *temp_leaked_c_string = class_name.alloc_c_string();
    godot::Spatial *cameraAsSpatial = cast_to<godot::Spatial>(cameraAsNode);
    godot::Ref<godot::World> world = cameraAsSpatial->get_world();

    godot::PhysicsDirectSpaceState *spaceState = world->get_direct_space_state();
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::input(const godot::InputEvent *inputEvent) {
    godot::InputEventMouseMotion *mouseMotionEvent = (
      godot::Object::cast_to<godot::InputEventMouseMotion>(inputEvent)
    );
    if(mouseMotionEvent != nullptr) {
      processMouseMotion(*mouseMotionEvent);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::processMouseMotion(
    const godot::InputEventMouseMotion &mouseMotionEvent
  ) {
    godot::Spatial *cameraNode = GetCameraNode();
    if(cameraNode == nullptr) {
      return;
    }

    // First, rotate the camera in place. We'll figure out the orbiting translation
    // in the next step because the target node could have moved or be obscured at
    // our current distance
    rotateCameraNode(*cameraNode, mouseMotionEvent);

    // If there's no target, rotating in place is all the camera will do
    godot::Spatial *targetNode = GetTargetNode();
    if(targetNode == nullptr) {
      return;
    }

    // Put the camera into its orbiting position around the target
    moveToOrbitPosition(*cameraNode, *targetNode);
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::rotateCameraNode(
    godot::Spatial &cameraNode,
    const godot::InputEventMouseMotion &mouseMotionEvent
  ) {

    // Calculate the amount of rotation the mouse movement should cause
    godot::Vector2 relativeRotation = getYawAndPitchFromMouseMotion(mouseMotionEvent);

    // Use 179 degrees as the limit for looking up/down because at 180+ degrees
    // the Euler yaw direction would change (meaning if you looked further
    // down, the camera would rotate 180 degrees suddenly and repeatedly as
    // the player is unlikely to stop the mouse movement exactly at that point.
    const float MaximumVerticalAngle = (
      Geometry::Trigonometry::HalfPI - Geometry::Trigonometry::RadiansPerDegree
    );

    // Obtain the current angle of the target node (we don't save a copy of the angle
    // ourselves because there's no good reason to - this way we can even handle if
    // the camera is moved by other parts of the game code)
    godot::Vector3 eulerAngles = cameraNode.get_rotation();

    eulerAngles.y = std::fmod(eulerAngles.y - relativeRotation.x, Geometry::Trigonometry::Tau);
    eulerAngles.x = clamp(
      eulerAngles.x - relativeRotation.y, -MaximumVerticalAngle, +MaximumVerticalAngle
    );

    cameraNode.set_rotation(eulerAngles);

  }

  // ------------------------------------------------------------------------------------------- //

  godot::Vector2 ThirdPersonCameraController::getYawAndPitchFromMouseMotion(
    const godot::InputEventMouseMotion &mouseMotionEvent
  ) const {
    godot::Vector2 relativeRotation = mouseMotionEvent.get_relative();

    relativeRotation.x *= RotationDegreesPerMickey.x;
    relativeRotation.y *= RotationDegreesPerMickey.y;
    relativeRotation.x *= Geometry::Trigonometry::RadiansPerDegree;
    relativeRotation.y *= Geometry::Trigonometry::RadiansPerDegree;

    return relativeRotation;
  }

  // ------------------------------------------------------------------------------------------- //

  void ThirdPersonCameraController::moveToOrbitPosition(
    godot::Spatial &cameraNode, godot::Spatial &targetNode
  ) {
    godot::Vector3 targetPosition = targetNode.get_global_transform().get_origin();

    //cameraNode.set_translation(targetPosition + this->Offset);
    godot::Transform cameraTransform = cameraNode.get_transform();
    cameraTransform.origin = targetPosition + this->Offset;
    cameraTransform.translate(0.0f, 0.0f, 3.0f);
    cameraNode.set_transform(cameraTransform);


    // This seems to be broken in Godot 3.1, 3.1.1
    // I opened an issue here: https://github.com/godotengine/godot/issues/28574
#if defined(GODOT_HAS_FIXED_THE_SEGFAULT)
    godot::Vector3 observerPosition = targetPosition - godot::Vector3(0.0f, 0.0f, 3.0f);

    godot::Dictionary collisionInfo = raycast(targetPosition, observerPosition);
    /* dictionary contents
    {
       position: Vector2 # point in world space for collision
       normal: Vector2 # normal in world space for collision
       collider: Object # Object collided or null (if unassociated)
       collider_id: ObjectID # Object it collided against
       rid: RID # RID it collided against
       shape: int # shape index of collider
       metadata: Variant() # metadata of collider
    }
    */

    //if(!collisionInfo.empty()) {}

    godot::Transform cameraTransform = cameraNode.get_transform();
    const godot::Basis &cameraBasis = cameraTransform.get_basis();

    godot::Vector3 orbitingOffset = cameraBasis.x * godot::Vector3(5.0f, 0.0f, 0.0f);

    cameraNode.set_translation(targetPosition);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Dictionary ThirdPersonCameraController::raycast(
    const godot::Vector3 &from, godot::Vector3 &to
  ) {
    godot::Viewport *viewport = get_viewport();
    if(viewport == nullptr) {
      godot::Godot::print("ERROR: Could not obtain Viewport the scene is rendered through");
      return godot::Dictionary();
    }

    godot::Ref<godot::World> world = viewport->get_world();
    if(world.is_null()) {
      godot::Godot::print("ERROR: Could not obtain physics World from current Viewport");
      return godot::Dictionary();
    }


#if GODOT_3_1_CRASH_FIXED // Godot 3.1 crashes in space_get_direct_state()
    godot::RID spaceId = world->get_space();
    godot::PhysicsDirectSpaceState *state = (
      godot::PhysicsServer::get_singleton()->space_get_direct_state(spaceId)
    );
#elif GODOT_3_1_OTHER_CRASH_FIXED // Godot 3.1 crashes in get_direct_space_state()
    godot::PhysicsDirectSpaceState *state = world->get_direct_space_state();
    if(state == nullptr) {
      godot::Godot::print("ERROR: Could not obtain physics space state from current World");
      return;
    }
#else // Ugly workaround calling methods dynamically as if by GDScript...

    // If the get_direct_space_state() method exists, call it by name
    const static godot::String GetDirectSpaceStateMethodName = "get_direct_space_state";
    if(world->has_method(GetDirectSpaceStateMethodName)) {
      godot::Variant spaceStateAsVariant = world->call(
        GetDirectSpaceStateMethodName
      );

      // This also causes the crash we're trying to avoid :-(
      //godot::Object *spaceState = godot::Object::___get_from_variant(spaceStateAsVariant);

      // And now, whatever we've got doesn't have an intersect_ray() method. WTF?

      // We've got the space state now, try to call the intersect_ray() method
      const static godot::String IntersectRayMethodName = "intersect_ray";
      if(spaceStateAsVariant.has_method(IntersectRayMethodName)) {
        godot::Godot::print("intersect_ray() exists");

        //godot::Array arguments = godot::Array::make(
        //  IntersectRayMethodName, from, to, godot::Array(), this->ViewBlockingMask, true, false
        //);

        godot::Variant intersectRayArguments[6];
        intersectRayArguments[0] = godot::Variant(from);
        intersectRayArguments[1] = godot::Variant(to);
        intersectRayArguments[2] = godot::Variant(godot::Array());
        intersectRayArguments[3] = godot::Variant(this->ViewBlockingMask);
        intersectRayArguments[4] = godot::Variant(true);
        intersectRayArguments[5] = godot::Variant(false);

        const godot::Variant *arguments = intersectRayArguments;
        godot::Variant dictionaryAsVariant = spaceStateAsVariant.call(
          IntersectRayMethodName, &arguments, 6
        );

        godot::Godot::print("Success!");
        return dictionaryAsVariant;
      }
    }

    // If this point is reached, nothing we tried worked :-(
    return godot::Dictionary();
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Input *ThirdPersonCameraController::getInputManager() const {
    if(this->inputManager == nullptr) {
      this->inputManager = godot::Input::get_singleton();

      // Stil no input manager? That's an error.
      if(this->inputManager == nullptr) {
        godot::Godot::print(
          "ERROR: ThirdPersonCameraController could not access Godot's input manager"
        );
        assert(this->inputManager != nullptr);
      }
    }

    return this->inputManager;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Cameras
