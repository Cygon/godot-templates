#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "CharacterController.h"
#include "../Support/ComponentReferenceHelper.h"
#include "ActorPhysics.h"

#include <Input.hpp>

#include <cassert>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>How high the character can jump in Godot units</summary>
  const float DefaultJumpHeight = 0.5f;

  /// <summary>How fast the character can run in Godot units per second</summary>
  const float DefaultRunningSpeed = 2.5f;

  /// <summary>How long the character will take to achieve its maximum speed</summary>
  const float DefaultSecondsToFullSpeed = 0.15f;

  /// <summary>How much control the player has over the character in the air</summary>
  const float DefaultAirControlFactor = 1.0f;

  /// <summary>Number of jumps the character can do after touching ground</summary>
  const int DefaultMaximumJumpCount = 1;

  /// <summary>Maximum height of steps the character can walk up</summary>
  const float DefaultMaximumStepHeight = 0.35f;

  /// <summary>Default path to the ActorPhysics node</summary>
  const std::string DefaultActorPhysicsNodePath = "../ActorPhysics";

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace CppExample { namespace Actors {

  // ------------------------------------------------------------------------------------------- //

  CharacterController::CharacterController() :
    JumpHeight(DefaultJumpHeight),
    RunningSpeed(DefaultRunningSpeed),
    SecondsToFullSpeed(DefaultSecondsToFullSpeed),
    AirControlFactor(DefaultAirControlFactor),
    MaximumJumpCount(DefaultMaximumJumpCount),
    ActorPhysicsNodePath(DefaultActorPhysicsNodePath.c_str()),
    currentState(State::Ground),
    remainingJumpCount(DefaultMaximumJumpCount) {}

  // ------------------------------------------------------------------------------------------- //

  void CharacterController::_init() {
    this->JumpHeight = DefaultJumpHeight;
    this->RunningSpeed = DefaultRunningSpeed;
    this->SecondsToFullSpeed = DefaultSecondsToFullSpeed;
    this->AirControlFactor = DefaultAirControlFactor;
    this->MaximumJumpCount = DefaultMaximumJumpCount;
    this->ActorPhysicsNodePath = godot::NodePath(DefaultActorPhysicsNodePath.c_str());
    this->currentState = State::Ground;
    this->remainingJumpCount = DefaultMaximumJumpCount;
  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterController::_register_methods() {
    godot::register_property<CharacterController, godot::NodePath>(
      "actor_physics_node_path",
      &CharacterController::ActorPhysicsNodePath,
      godot::NodePath(DefaultActorPhysicsNodePath.c_str())
    );

    godot::register_method("_physics_process", &CharacterController::physics_process);
    godot::register_method("jump", &CharacterController::Jump);

    godot::register_property<CharacterController, float>(
      "jump_height",
      &CharacterController::JumpHeight, DefaultJumpHeight,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "0.1,9.9,0.1"
    );

    godot::register_property<CharacterController, float>(
      "running_speed",
      &CharacterController::RunningSpeed, DefaultRunningSpeed,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "0.1,9.9,0.1"
    );

    godot::register_property<CharacterController, float>(
      "seconds_to_full_speed",
      &CharacterController::SecondsToFullSpeed, DefaultSecondsToFullSpeed,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "0.05,1.0,0.05"
    );

    godot::register_property<CharacterController, float>(
      "air_control_factor",
      &CharacterController::AirControlFactor, DefaultAirControlFactor,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "0.0,1.0,0.1"
    );

    godot::register_property<CharacterController, int>(
      "maximum_jump_count",
      &CharacterController::MaximumJumpCount, DefaultMaximumJumpCount,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "0,4,1"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  bool CharacterController::Jump() {
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterController::physics_process(float deltaSeconds) {
    switch(this->currentState) {
      case State::Ground: { handleGroundMovement(deltaSeconds); break; }
      case State::Air: { handleAirMovement(deltaSeconds); break; }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterController::handleGroundMovement(float deltaSeconds) {
    handleHorizontalMovement(deltaSeconds, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterController::handleAirMovement(float deltaSeconds) {
    handleHorizontalMovement(deltaSeconds, this->AirControlFactor);

  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterController::handleHorizontalMovement(float deltaSeconds, float controlFactor) {
    godot::Input *inputManager = getInputManager();
    if(inputManager == nullptr) {
      godot::Godot::print("ERROR: CharacterController could not find the Godot InputManager");
      return;
    }

    // Player controls and strength for movement on the horizontal plane
    godot::Vector2 controls(
      (
        inputManager->get_action_strength("move_right") -
        inputManager->get_action_strength("move_left")
      ),
      (
        inputManager->get_action_strength("move_backward") -
        inputManager->get_action_strength("move_forward")
      )
    );

    // Turn this into an absolute target velocity the character will aim to achieve
    godot::Vector2 targetVelocity = controls * this->RunningSpeed;

    // Calculate how fast the character will achieve this velocity
    float acceleration = this->RunningSpeed / this->SecondsToFullSpeed;
    acceleration *= controlFactor;

    accelerateToVelocity(targetVelocity, acceleration, deltaSeconds);
  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterController::accelerateToVelocity(
    const godot::Vector2 &targetVelocity, float acceleration, float deltaSeconds
  ) {
    ActorPhysics *actorPhysics = getActorPhysics();
    if(actorPhysics == nullptr) {
      godot::Godot::print("ERROR: CharacterController could not find its ActorPhysics");
      return;
    }

    // Get the current velocity on the X/Z plane (horizontal movement)
    godot::Vector2 currentVelocity = godot::Vector2(
      actorPhysics->Velocity.x, actorPhysics->Velocity.z
    );

    // Calculate force that would be required to achieve target velocity instantly
    // and limit it to the maximum acceleration specified by the caller
    godot::Vector2 force = (targetVelocity - currentVelocity) / deltaSeconds;
    force = force.clamped(acceleration);
    force *= actorPhysics->Mass;

    // Apply the calculated force to the actor
    actorPhysics->QueueForce(godot::Vector3(force.x, 0.0f, force.y));
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Input *CharacterController::getInputManager() const {
    if(this->inputManager == nullptr) {
      this->inputManager = godot::Input::get_singleton();

      // Stil no input manager? That's an error.
      if(this->inputManager == nullptr) {
        godot::Godot::print("ERROR: CharacterController could not access Godot's input manager");
        assert(this->inputManager != nullptr);
      }
    }

    return this->inputManager;
  }

  // ------------------------------------------------------------------------------------------- //

  ActorPhysics *CharacterController::getActorPhysics() const {
    return Support::ComponentReferenceHelper::FindSiblingComponent<ActorPhysics>(
      this, this->ActorPhysicsNodePath
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Actors
