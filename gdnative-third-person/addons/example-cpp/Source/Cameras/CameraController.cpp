#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "CameraController.h"
#include "../Geometry/Trigonometry.h"

#include <Spatial.hpp>
#include <Camera.hpp>

#include <cassert>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Default path to the camera node, can be the controller's node itself</summary>
  const std::string DefaultCameraNodePath = std::string("../..");
  /// <summary>Default level of fading when viewing from this camera, 1.0 = none</summary>
  const float DefaultFadeLevel = 1.0f;
  /// <summary>Default path to the target node tracked by the camera</summary>
  const std::string DefaultTargetNodePath = std::string();

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace CppExample { namespace Cameras {

  // ------------------------------------------------------------------------------------------- //

  CameraController::CameraController() :
    CameraNodePath(DefaultCameraNodePath.c_str()),
    FadeLevel(DefaultFadeLevel),
    TargetNodePath(DefaultTargetNodePath.c_str()) {}

  // ------------------------------------------------------------------------------------------- //

  void CameraController::_init() {
    this->CameraNodePath = godot::NodePath(DefaultCameraNodePath.c_str());
    this->FadeLevel = DefaultFadeLevel;
    this->TargetNodePath = godot::NodePath(DefaultTargetNodePath.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

  void CameraController::_register_methods() {
    godot::register_method("_process", &CameraController::process);

    godot::register_property<CameraController, godot::NodePath>(
      "camera_node_path",
      &CameraController::CameraNodePath, godot::NodePath(DefaultCameraNodePath.c_str())
    );

    godot::register_property<CameraController, float>(
      "fade_level",
      &CameraController::FadeLevel, DefaultFadeLevel
    );

    godot::register_property<CameraController, godot::NodePath>(
      "target_node_path",
      &CameraController::TargetNodePath, godot::NodePath(DefaultTargetNodePath.c_str())
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void CameraController::process(float deltaSeconds) {}

  // ------------------------------------------------------------------------------------------- //

  godot::Camera *CameraController::GetCameraNode() const {
    if(this->CameraNodePath.is_empty()) {
      return nullptr;
    }

    godot::Node *cameraNode = get_node(this->CameraNodePath);
    if(cameraNode == nullptr) {
      return nullptr;
    }

    return godot::Object::cast_to<godot::Camera>(cameraNode);
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Spatial *CameraController::GetTargetNode() const {
    if(this->TargetNodePath.is_empty()) {
      return nullptr;
    }

    godot::Node *targetNode = get_node(this->TargetNodePath);
    if(targetNode == nullptr) {
      return nullptr;
    }

    return godot::Object::cast_to<godot::Spatial>(targetNode);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Cameras
