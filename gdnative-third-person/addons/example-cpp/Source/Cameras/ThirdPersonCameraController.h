#ifndef NUCLEX_CPPEXAMPLE_CAMERAS_THIRDPERSONCAMERACONTROLLER_H
#define NUCLEX_CPPEXAMPLE_CAMERAS_THIRDPERSONCAMERACONTROLLER_H

#include "../Config.h"
#include "CameraController.h"

namespace godot {
  class Input;
  class InputEventMouseMotion;
}

namespace Nuclex { namespace CppExample { namespace Cameras {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets the player orbit a camera around a target</summary>
  /// <remarks>
  ///   <para>
  ///     This type of camera is pretty popular with role-playing games where
  ///     the player controls a character that is visible on the screen.
  ///   </para>
  /// </remarks>
  class ThirdPersonCameraController : public CameraController {
    GODOT_CLASS(ThirdPersonCameraController, CameraController)

    /// <summary>Initializes a new third-person camera controller component</summary>
    public: NUCLEX_CPPEXAMPLE_API ThirdPersonCameraController();
    /// <summary>Releases all resources before the instance is destroyed</summary>
    public: NUCLEX_CPPEXAMPLE_API virtual ~ThirdPersonCameraController() {}

    /// <summary>Called to initialize or reset the camera controller attributes</summary>
    public: void _init();
    /// <summary>Called when the node becomes part of the scene tree</summary>
    protected: void enter_tree();
    /// <summary>Called when the node leaves the scene tree again</summary>
    protected: void exit_tree();
    /// <summary>Called once per rendered frame to update scene nodes</summary>
    /// <param name="deltaSeconds">Amount of time passed since the previous update</param>
    protected: virtual void process(float deltaSeconds) override;
    /// <summary>Reports any input device actions to the camera</summary>
    /// <param name="inputEvent">Description of the input made by the player</param>
    private: void input(const godot::InputEvent *inputEvent);

    /// <summary>Rotates the camera around its pivot in response to the mouse</summary>
    /// <param name="mouseMotionEvent">Description of the movement of the mouse</param>
    private: void processMouseMotion(const godot::InputEventMouseMotion &mouseMotionEvent);

    /// <summary>Rotates the camera node according to the player's mouse input</summary>
    /// <param name="cameraNode">Camera node that will be rotated</param>
    /// <param name="mouseMotionEvent">Mouse movement performed by the player</param>
    private: void rotateCameraNode(
      godot::Spatial &cameraNode, const godot::InputEventMouseMotion &mouseMotionEvent
    );

    /// <summary>Translates the camera node into a position that orbits the target</summary>
    /// <param name="cameraNode">Camera that will be moved into an orbiting position</param>
    /// <param name="targetNpde">Node that will be orbited by the camera</param>
    private: void moveToOrbitPosition(
      godot::Spatial &cameraNode, godot::Spatial &targetNode
    );

    /// <summary>Calculates the relative yaw and pitch rotation from a mouse movement<summary>
    /// <param name="mouseMotionEvent">How far the mouse has moved</param>
    /// <returns>The rotation on the pitch (X) and yaw (Y) axes</returns>
    private: godot::Vector2 getYawAndPitchFromMouseMotion(
      const godot::InputEventMouseMotion &mouseMotionEvent
    ) const;

    /// <summary>Performs a raycast along a line segment</summary>
    /// <param name="from">Position at which the ray will begin</param>
    /// <param name="to">Position at which the ray will end</param>
    /// <returns>A dictionary describing the first obstace that has been hit</returns>
    private: godot::Dictionary raycast(const godot::Vector3 &from, godot::Vector3 &to);

    /// <summary>Registers the methods callable from within Godot to the engine</summary>
    public: static void _register_methods();

    /// <summary>Returns the active Godot input manager</summary>
    /// <returns>The active godot input manager</returns>
    private: godot::Input *getInputManager() const;

    /// <summary>Offset from the camera's target to the pivot point it is orbiting</summary>
    /// <remarks>
    ///   Characters are usually designed so that their center point is at the floor,
    ///   level with their feet. The third-person camera should of course orbit around
    ///   the chest of the character, not their shoes, so this offset can adjust for that.
    /// </remarks>
    public: godot::Vector3 Offset;

    /// <summary>How much turning the mouse wheel adjusts the camera's distance</summary>
    public: float MouseWheelZoomSensitivity;
    /// <summary>Shortest distance the camera controller can be adjusted to</summary>
    public: float MinimumDistance;
    /// <summary>Longest distance the camera controller can be adjusted to</summary>
    public: float MaximumDistance;

    /// <summary>Amount of rotation the mouse will perform for one mickey's movement</summary>
    /// <remarks>
    ///   A mickey is the smallest movement a mouse can detect and report (even in 2019,
    ///   mice don't tell the OS about their DPI resolution). This value controls mouse
    ///   sensitivity by changing the number of degrees rotated per mouse movement.
    /// </remarks>
    public: godot::Vector2 RotationDegreesPerMickey;

    /// <summary>Collision mask for things that would block the camera's view</summary>
    public: std::int64_t ViewBlockingMask;

    /// <summary>Godot's input manager</summary>
    private: mutable godot::Input *inputManager;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Cameras

#endif // NUCLEX_CPPEXAMPLE_CAMERAS_THIRDPERSONCAMERACONTROLLER_H
