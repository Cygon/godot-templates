#ifndef NUCLEX_CPPEXAMPLE_ACTORS_CHARACTERCONTROLLER_H
#define NUCLEX_CPPEXAMPLE_ACTORS_CHARACTERCONTROLLER_H

#include "../Config.h"

#include <Godot.hpp>
#include <Node.hpp>

namespace Nuclex { namespace CppExample { namespace Actors {
  class ActorPhysics;
}}}

namespace godot {
  class Input;
}

namespace Nuclex { namespace CppExample { namespace Actors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>All-in-one character controller that moves a humanoid around the scene</summary>
  class CharacterController : public godot::Node {
    GODOT_CLASS(CharacterController, godot::Node)

    /// <summary>Initializes a new character controller component</summary>
    public: NUCLEX_CPPEXAMPLE_API CharacterController();
    /// <summary>Releases all resources before the instance is destroyed</summary>
    public: NUCLEX_CPPEXAMPLE_API ~CharacterController() {}

    /// <summary>Called to initialize or reset the character controller attributes</summary>
    public: NUCLEX_CPPEXAMPLE_API void _init();

    /// <summary>Registers the methods callable from within Godot to the engine</summary>
    public: static void _register_methods();

    /// <summary>Makes the character jump if it's able to</summary>
    /// <returns>True if the character jumped, false if it was unable to jump</returns>
    public: NUCLEX_CPPEXAMPLE_API bool Jump();

    /// <summary>Called each physics update to update the simulation</summary>
    /// <param name="deltaSeconds">Time that has passed since the previous update</param>
    private: NUCLEX_CPPEXAMPLE_API void physics_process(float deltaSeconds);

    /// <summary>Handles the movement of the character while grounded</summary>
    /// <param name="deltaSeconds">Time that has passed since the previous update</param>
    private: void handleGroundMovement(float deltaSeconds);

    /// <summary>Handles the movement of the character while in the air</summary>
    /// <param name="deltaSeconds">Time that has passed since the previous update</param>
    private: void handleAirMovement(float deltaSeconds);

    /// <summary>Handles the horizontal movement of the character while grounded</summary>
    /// <param name="deltaSeconds">Time that has passed since the previous update</param>
    /// <param name="controlFactor">How much control the player can exert</param>
    private: void handleHorizontalMovement(float deltaSeconds, float controlFactor);

    /// <summary>Accelerates the character towards the specified horizontal velocity</summary>
    /// <param name="targetVelocity">Velocity the character will try to achieve</param>
    /// <param name="acceleration">How fast the character is allowed to accelerate</param>
    /// <param name="deltaSeconds">Time that has passed since the previous update</param>
    /// <remarks>
    ///   Uses standard physics, except that it's Godot units instead of meters
    ///   (but most likely, 1 Godot unit is a meter in your game!). So the target
    ///   velocity is in <em>units per second</em> and the acceleration is in
    ///   <em>units per second per second</em> aka <em>units per second squared</em>
    /// </remarks>
    private: void accelerateToVelocity(
      const godot::Vector2 &targetVelocity, float acceleration, float deltaSeconds
    );

    /// <summary>Returns the active Godot input manager</summary>
    /// <returns>The active godot input manager</returns>
    private: godot::Input *getInputManager() const;

    /// <summary>Looks up the ActorPhysics component simulating character's physics</summary>
    /// <returns>The ActorPhysics component used by the character</returns>
    private: ActorPhysics *getActorPhysics() const;

    /// <summary>How high the character can jump in Godot units</summary>
    public: float JumpHeight;

    /// <summary>How fast the character can run in Godot units per second</summary>
    public: float RunningSpeed;

    /// <summary>How long the character will take to achieve its maximum speed</summary>
    /// <remarks>
    ///   This option allows you to balance between tight controls and a more realistic
    ///   feel where the character's momentum prevents instant running or stopping.
    /// </remarks>
    public: float SecondsToFullSpeed;

    /// <summary>How much control the player has over the character in the air</summary>
    public: float AirControlFactor;

    /// <summary>Number of jumps the character can do after touching ground</summary>
    /// <remarks>
    ///   0 means the character can't jump at all (DooM guy), 1 means the character can
    ///   jump off the ground. 2 and up allow the character to jump in the air
    ///   (2 = double-jump). If the character walks off a cliff, entering the fall
    ///   counts as 1 jump.
    /// </remarks>
    public: int MaximumJumpCount;

    /// <summary>Path to the ActorPhysics node for the character</summary>
    /// <remarks>
    ///   <para>
    ///     This is how you access other nodes in Godot. Since nodes aren't reference-counted,
    ///     storing an actual pointer is dangerous (you wouldn't know when the node behind
    ///     the pointer gets deleted). So you need to re-query the node with get_node()
    ///     each time you want to access it.
    ///   </para>
    ///   <para>
    ///     In the future, there will hopefully be a is_instance_id_valid() method.
    ///     You can already obtain a node's unique (and never reused) instance id via
    ///     the godot::Node::get_instance_id() method.
    ///   </para>
    /// </remarks>
    public: godot::NodePath ActorPhysicsNodePath;

    /// <summary>States the character can be in</summary>
    private: enum class State {

      /// <summary>Character is grounded and can walk and jump</summary>
      Ground,

      /// <summary>Character is in the air after falling or jumping</summary>
      Air

    };

    /// <summary>State the character is currently in</summary>
    private: State currentState;

    /// <summary>Remaining number of jumps before having to touch ground again</summary>
    private: int remainingJumpCount;

    /// <summary>Godot's input manager</summary>
    private: mutable godot::Input *inputManager;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Actors

#endif // NUCLEX_CPPEXAMPLE_ACTORS_CHARACTERCONTROLLER_H
