#ifndef NUCLEX_CPPEXAMPLE_CAMERAS_CAMERACONTROLLER_H
#define NUCLEX_CPPEXAMPLE_CAMERAS_CAMERACONTROLLER_H

#include "../Config.h"

#include <Godot.hpp>
#include <Camera.hpp>

namespace godot {
  class Spatial;
}

namespace Nuclex { namespace CppExample { namespace Cameras {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages the positioning and properties of a camera in the scene</summary>
  /// <remarks>
  ///   This serves as a base class for more advanced camera controllers. Using
  ///   this base class you can change the target followed by the camera without having
  ///   to hardcode the type of camera controller you're expected there to be.
  /// </remarks>
  class CameraController : public godot::Node {
    GODOT_CLASS(CameraController, godot::Node)

    /// <summary>Initializes a new camera controller component</summary>
    public: NUCLEX_CPPEXAMPLE_API CameraController();
    /// <summary>Releases all resources before the instance is destroyed</summary>
    public: NUCLEX_CPPEXAMPLE_API virtual ~CameraController() {}

    /// <summary>Called to initialize or reset the camera controller attributes</summary>
    public: NUCLEX_CPPEXAMPLE_API void _init();

    /// <summary>Registers the methods callable from within Godot to the engine</summary>
    public: NUCLEX_CPPEXAMPLE_API static void _register_methods();

    /// <summary>Called once per rendered frame to update scene nodes</summary>
    /// <param name="deltaSeconds">Amount of time passed since the previous update</param>
    protected: virtual void process(float deltaSeconds);

    /// <summary>Retrieves the camera node this component is attached to</summary>
    /// <returns>The camera node for this component or a nullptr</returns>
    protected: godot::Camera *GetCameraNode() const;

    /// <summary>Retrieves the node at which the camera is currently looking</summary>
    /// <returns>The node the camera is currently looking at or a nullptr</returns>
    protected: godot::Spatial *GetTargetNode() const;

    /// <summary>Path from this component to the camera node (can this node,too!)</summary>
    public: godot::NodePath CameraNodePath;

    /// <summary>How far the camera is faded in (0.0 = all black, 1.0 = all visible)</summary>
    public: float FadeLevel;

    /// <summary>Target the camera is currently tracking, can be empty</summary>
    public: godot::NodePath TargetNodePath;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Cameras

#endif // NUCLEX_CPPEXAMPLE_CAMERAS_CAMERACONTROLLER_H
