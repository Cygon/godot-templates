#ifndef NUCLEX_CPPEXAMPLE_CAMERAS_DEBUGFLYCAMERA_H
#define NUCLEX_CPPEXAMPLE_CAMERAS_DEBUGFLYCAMERA_H

#include "../Config.h"

#include <Godot.hpp>
#include <Camera.hpp>

namespace godot {
  class Input;
  class InputEventMouseMotion;
}

namespace Nuclex { namespace CppExample { namespace Cameras {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets the user drive a camera around in the scene</summary>
  /// <remarks>
  ///   This is mostly a debugging and tech demo aid that lets you move the camera
  ///   around in the scene rather than just having it sit statically in one place.
  ///   It can be used to inspect a map or export a project to quickly show off.
  /// </remarks>
  class DebugFlyCamera : public godot::Camera {
    GODOT_CLASS(DebugFlyCamera, godot::Camera)

    /// <summary>Initializes a new debug fly camera</summary>
    public: NUCLEX_CPPEXAMPLE_API DebugFlyCamera();
    /// <summary>Releases all resources before the instance is destroyed</summary>
    public: NUCLEX_CPPEXAMPLE_API virtual ~DebugFlyCamera() {}

    /// <summary>Called to initialize or reset the camera's attributes</summary>
    public: NUCLEX_CPPEXAMPLE_API void _init();
    /// <summary>Called when the node becomes part of the scene tree</summary>
    protected: NUCLEX_CPPEXAMPLE_API void enter_tree();
    /// <summary>Called when the node leaves the scene tree again</summary>
    protected: NUCLEX_CPPEXAMPLE_API void exit_tree();
    /// <summary>Called once per rendered frame to update scene nodes</summary>
    /// <param name="deltaSeconds">Amount of time passed since the previous update</param>
    protected: NUCLEX_CPPEXAMPLE_API void process(float deltaSeconds);
    /// <summary>Reports any input device actions to the camera</summary>
    /// <param name="inputEvent">Description of the input made by the player</param>
    protected: NUCLEX_CPPEXAMPLE_API void input(const godot::InputEvent *inputEvent);

    /// <summary>Registers the methods callable from within Godot to the engine</summary>
    public: NUCLEX_CPPEXAMPLE_API static void _register_methods();

    /// <summary>Reads the user's current movement inputs</summary>
    /// <returns>The user's current movement inputs as a 3D vector</returns>
    private: godot::Vector3 readMovementInputs() const;

    /// <summary>Rotates the camera matching the reported mouse motion</summary>
    /// <param name="mouseMotionEvent">Description of the motion performed by the mouse</param>
    private: void processMouseMotion(const godot::InputEventMouseMotion &mouseMotionEvent);

    /// <summary>Limits the position of the node to its boundary region</summary>
    private: void limitPositionToBoundaries();

    /// <summary>Returns the active Godot input manager</summary>
    /// <returns>The active godot input manager</returns>
    private: godot::Input *getInputManager() const;

    /// <summary>Boundaries of the area the camera can fly around in</summary>
    public: godot::AABB Boundaries;
    /// <summary>How fast the camera moves around in Godot units per second</summary>
    public: float MovementUnitsPerSecond;
    /// <summary>How fast the camera rotates in relation to mouse movements</summary>
    /// <remarks>
    ///   A mickey is the smallest position change registered by the mouse hardware,
    ///   for example a 3000 DPI mouse would move by one mickey each 1/3000th inch.
    /// </remarks>
    public: float RotationDegreesPerMickey;
    /// <summary>Whether to terminate the game when the escape key is pressed</summary>
    /// <remarks>
    ///   Small convenience hack if this component is used for its intended purpose,
    ///   inspecting levels and publishing tech demos.
    /// </remarks>
    public: bool QuitViaEscapeKey;

    /// <summary>Godot's input manager</summary>
    private: mutable godot::Input *inputManager;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Cameras

#endif // NUCLEX_CPPEXAMPLE_CAMERAS_DEBUGFLYCAMERA_H
