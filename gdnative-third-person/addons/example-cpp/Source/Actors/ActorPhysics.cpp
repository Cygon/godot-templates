#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "ActorPhysics.h"
#include "../Geometry/Trigonometry.h"
#include "../Support/ComponentReferenceHelper.h"

#include <KinematicBody.hpp>

#include <cassert>

// https://scicomp.stackexchange.com/questions/24998/which-runge-kutta-method-is-more-accurate-dormand-prince-or-cash-karp
// https://github.com/JuliaDiffEq/DifferentialEquations.jl

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Default setting on using the high-quality integrator</summary>
  const bool DefaultUseHighQualityIntegration = false;

  /// <summary>Default path to the kinematic body being controlled</summary>
  const std::string DefaultKinematicBodyPath = "";

  /// <summary>Default gravity vector applied to an actor affected by gravity</summary>
  const godot::Vector3 DefaultGravityVector(0.0f, -9.80665f, 0.0f);

  /// <summary>Default setting for gravity influence on the actor</summary>
  const bool DefaultIsAffectedByGravity = true;

  /// <summary>Default scale with which gravity affects the actor</summary>
  const float DefaultGravityScale = 1.0f;

  /// <summary>Default mass of the actor</summary>
  const float DefaultMass = 85.0f; // Default weight = athletic "hero" human + stuff

  /// <summary>Default step/slope height the character can traverse without jumping</summary>
  const float DefaultMaximumStepHeight = 0.25f;

  /// <summary>Default velocity the actor will start out with</summary>
  const godot::Vector3 DefaultVelocity(0.0f, 0.0f, 0.0f);

  /// <summary>Smallest velocity change the recorded velocity can be updated by</summary>
  const float VelocityEpsilon = 1e-4f;

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

namespace Nuclex { namespace CppExample { namespace Actors {

  // ------------------------------------------------------------------------------------------- //

  ActorPhysics::ActorPhysics() :
    IsAffectedByGravity(DefaultIsAffectedByGravity),
    GravityVector(DefaultGravityVector),
    GravityScale(DefaultGravityScale),
    Mass(DefaultMass),
    MaximumStepHeight(DefaultMaximumStepHeight),
    Velocity(DefaultVelocity),
    UseHighQualityIntegration(DefaultUseHighQualityIntegration),
    KinematicBodyNodePath(DefaultKinematicBodyPath.c_str()),
    queuedForces(godot::Vector3(0.0f, 0.0f, 0.0f)),
    queuedImpulses(godot::Vector3(0.0f, 0.0f, 0.0f)),
    queuedMovement(godot::Vector3(0.0f, 0.0f, 0.0f)),
    midPointVelocity(godot::Vector3(0.0f, 0.0f, 0.0f)),
    stepClimbingBudget(0.0f) {}

  // ------------------------------------------------------------------------------------------- //

  void ActorPhysics::_init() {
    this->IsAffectedByGravity = DefaultIsAffectedByGravity;
    this->GravityVector = DefaultGravityVector;
    this->GravityScale = DefaultGravityScale;
    this->Mass = DefaultMass;
    this->MaximumStepHeight = DefaultMaximumStepHeight;
    this->Velocity = DefaultVelocity;
    this->UseHighQualityIntegration = DefaultUseHighQualityIntegration;
    this->KinematicBodyNodePath = godot::NodePath(DefaultKinematicBodyPath.c_str());
    this->queuedForces = godot::Vector3(0.0f, 0.0f, 0.0f);
    this->queuedImpulses = godot::Vector3(0.0f, 0.0f, 0.0f);
    this->queuedMovement = godot::Vector3(0.0f, 0.0f, 0.0f);
    this->midPointVelocity = godot::Vector3(0.0f, 0.0f, 0.0f);
    this->stepClimbingBudget = 0.0f;
  }

  // ------------------------------------------------------------------------------------------- //

  void ActorPhysics::_physics_process(float deltaSeconds) {
    godot::KinematicBody *kinematicBody = getKinematicBody();
    if(kinematicBody == nullptr) {
      godot::Godot::print("ERROR: ActorPhysics component could not find its KinematicBody node");
      return;
    }

    // Auto-apply gravity if enabled
    if(this->IsAffectedByGravity) {
      ApplyGravity(this->GravityVector);
    }

    // Determine the translation the actor should attempt this physics frame
    // according to its velocity, acceleration and forces.
    godot::Vector3 translation;
    if(this->UseHighQualityIntegration) {
      translation = integrateViaRungeKutta4Method(deltaSeconds);
    } else {
      translation = integrateViaMidpointMethod(deltaSeconds);
    }

    // Now do the movement. This requires special tricks because the CharacterController
    // component has several issues.
    godot::Vector3 reportedVelocity;
    {
      reportedVelocity = moveActor(kinematicBody, translation, deltaSeconds);

      // As the actor travels horizontally, recharge the step climb budget
      // by the amount the character controller's slope limit would allow
      // the character to climb vertically.
      rechargeStepClimbBudget(translation);
    }

#if defined(DEBUG_ACTORPHYSICS_VELOCITY)
    using Nuclex::CppExample::Support::lexical_cast;
    godot::String applied = lexical_cast<godot::String>(this->Velocity.y);
    godot::String reported = lexical_cast<godot::String>(reportedVelocity.y);
    godot::Godot::print("Applied Vy " + applied + " | Measured Vy " + reported);
#endif

    // We have the velocity we want (forces + gravity) and the velocity at which we
    // actually move (due to collisions etc.). Make the velocity at which we actually
    // moved the recorded velocity so we don't run up huge impulses pushing into a wall.
    //
    // This is filtered so that small errors will not accumulate, like when moving at
    // a speed of 5.0 up a slope and the movement logic says that the character only
    // moved 4.99 units, getting slower every cycle.
    bool updateHorizontalVelocity = true;
    updateVelocity(reportedVelocity, updateHorizontalVelocity);

  }

  // ------------------------------------------------------------------------------------------- //

  void ActorPhysics::_register_methods() {
    godot::register_property<ActorPhysics, godot::NodePath>(
      "kinematic_body_node_path",
      &ActorPhysics::KinematicBodyNodePath,
      godot::NodePath(DefaultKinematicBodyPath.c_str())
    );
    godot::register_property<ActorPhysics, bool>(
      "is_affected_by_gravity",
      &ActorPhysics::IsAffectedByGravity, DefaultIsAffectedByGravity
    );
    godot::register_property<ActorPhysics, godot::Vector3>(
      "gravity_vector",
      &ActorPhysics::GravityVector, DefaultGravityVector
    );
    godot::register_property<ActorPhysics, float>(
      "gravity_scale",
      &ActorPhysics::GravityScale, DefaultGravityScale,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "0.1,9.9,0.1"
    );
    godot::register_property<ActorPhysics, float>(
      "mass",
      &ActorPhysics::Mass, DefaultMass,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "1,999,10"
    );
    godot::register_property<ActorPhysics, float>(
      "maximum_step_height",
      &ActorPhysics::MaximumStepHeight, DefaultMaximumStepHeight,
      GODOT_METHOD_RPC_MODE_DISABLED,
      GODOT_PROPERTY_USAGE_DEFAULT,
      GODOT_PROPERTY_HINT_RANGE, "0.05,0.95,0.05"
    );
    godot::register_property<ActorPhysics, godot::Vector3>(
      "velocity",
      &ActorPhysics::Velocity, DefaultVelocity
    );
    godot::register_property<ActorPhysics, bool>(
      "use_high_quality_integrator",
      &ActorPhysics::UseHighQualityIntegration, DefaultUseHighQualityIntegration
    );

    godot::register_method("_physics_process", &ActorPhysics::_physics_process);

    godot::register_method("queue_movement", &ActorPhysics::QueueMovement);
    godot::register_method("queue_force", &ActorPhysics::QueueForce);
    godot::register_method("queue_impulse", &ActorPhysics::QueueImpulse);
    godot::register_method("apply_gravity", &ActorPhysics::ApplyGravity);
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Vector3 ActorPhysics::moveActor(
    godot::KinematicBody *kinematicBody, const godot::Vector3 &translation, float deltaSeconds
  ) {
#if defined(GODOT_HAS_SLOPE_ISSUE)

    godot::Vector3 horizontalTranslation(translation.x, 0.0f, translation.z);
    godot::Vector3 verticalTranslation(0.0f, translation.y, 0.0f);

    if(translation.y >= 0.0f) {
      godot::Vector3 combinedVelocity = this->kinematicBody->move_and_slide(
        verticalTranslation / deltaSeconds, this->GravityVector
      );
      combinedVelocity += this->kinematicBody->move_and_slide(
        horizontalTranslation / deltaSeconds, this->GravityVector
      );
    }
    assert(!"Not implemented, delete this unless really required");

#else

    // The move_and_slide() method is "helpfully" multiplying velocity by deltaSeconds
    // for us, which is exactly that we don't want.
    //
    // Presently, the method is documented to return the remaining movement, but it
    // actually returns the performed movement. It's the docs that are wrong, most likely.
    godot::Vector3 actualMovement = kinematicBody->move_and_slide(
      translation / deltaSeconds, this->GravityVector
    );

#if defined(DEBUG_ACTORPHYSICS_MOVEMENT)
    using Nuclex::CppExample::Support::lexical_cast;
    godot::String attempted = lexical_cast<godot::String>(translation.y / deltaSeconds);
    godot::String remainder = lexical_cast<godot::String>(remainingMovement.y);
    godot::Godot::print("Attempted Dy " + attempted + " | Remaining Dy " + remainder);
#endif

    return actualMovement; // translation - remainingMovement;

#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void ActorPhysics::rechargeStepClimbBudget(const godot::Vector3 &translation) {

    // Determine the distance moved horizontally and vertically relative to the gravity vector
    float horizontalMovement, verticalMovement;
    {
      float distance = translation.length(); // total movement distance
      godot::Vector3 direction = translation / distance; // normalized movement direction

      // Calculate the angle relative to the floor plane (defined by the gravity vector).
      // Positive angles mean the actor is climbing, negative angles mean descending.
      float directionDotGravity = this->GravityVector.normalized().dot(direction);
      float angleToFloorPlane = Geometry::Trigonometry::HalfPI - std::acos(directionDotGravity);

      verticalMovement = std::sin(angleToFloorPlane) * distance;
      horizontalMovement = std::cos(angleToFloorPlane) * distance;
    }

    // Recharge or deplete the step climbing budget depending on the actor's horizontal
    // movement relative to its vertical movement
    float balance = horizontalMovement - verticalMovement;
    this->stepClimbingBudget = clamp(
      this->stepClimbingBudget + balance, -this->MaximumStepHeight, 0.0f
    );

  }

  // ------------------------------------------------------------------------------------------- //

  void ActorPhysics::updateVelocity(
    const godot::Vector3 &newVelocity, bool updateHorizontalVelocity
  ) {

    // Update the velocity only if it has changed more than the epsilon value.
    // Since the velocity is scaled by delta time, then descaled and scaled again,
    // blindly updating the recorded velocity would over time accumulate floating
    // point inaccuracies.
    if(this->Velocity.distance_squared_to(newVelocity) > VelocityEpsilon) {
      if(updateHorizontalVelocity) {
        this->Velocity = newVelocity;
      } else {
        this->Velocity.y = newVelocity.y;
      }
    }

    // If the velocity has been set to zero on any axis, apply this in any
    // case (since otherwise, a tiny drift might never be cleared from our
    // velocity vector, very slowly moving the actor around against an obstacle).
    {
      if(updateHorizontalVelocity) {
        if(std::abs(newVelocity.x) < VelocityEpsilon) {
          this->Velocity.x = 0.0f;
        }
        if(std::abs(newVelocity.z) < VelocityEpsilon) {
          this->Velocity.z = 0.0f;
        }
      }
      if(std::abs(newVelocity.y) < VelocityEpsilon) {
        this->Velocity.y = 0.0f;
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  godot::Vector3 ActorPhysics::integrateViaMidpointMethod(float deltaSeconds) {
    godot::Vector3 translation(0.0f, 0.0f, 0.0f);
    {
      // Apply the other half of the acceleration from the previous update cycle.
      // This is delayed into now in order to integrate velocity at the midpoint.
      this->Velocity += this->midPointVelocity;

      // Add impulses
      godot::Vector3 impulses = this->queuedImpulses / this->Mass;
      //this.acceleration += impulses / delta; // turn impulse into force over 1 delta tick
      this->Velocity += impulses; // add impulses directly, they have no time

      // Calculate the new acceleration
      godot::Vector3 acceleration = this->queuedForces / this->Mass;

      // Halve the acceleration so that one half can be applied now, the other half at
      // the beginning of the next update cycle (remembered in this->midPointVelocity)
      acceleration /= 2.0f;
      this->midPointVelocity = acceleration * deltaSeconds;

      // Apply the first half of the force scaled by time
      this->Velocity += this->midPointVelocity;

      // Integrate into position
      translation = this->Velocity * deltaSeconds;

      // Finally, queued movements (root motion, etc.) go directly into translation
      translation += this->queuedMovement;
    }

    // Reset the queued influences
    this->queuedForces = godot::Vector3(0.0f, 0.0f, 0.0f);
    this->queuedImpulses = godot::Vector3(0.0f, 0.0f, 0.0f);
    this->queuedMovement = godot::Vector3(0.0f, 0.0f, 0.0f);

    return translation;
  }

  // ------------------------------------------------------------------------------------------- //

  godot::Vector3 ActorPhysics::integrateViaRungeKutta4Method(float deltaSeconds) {
    godot::Godot::print("ERROR: Runge-Kutta4 integration not implemented yet!");
    return integrateViaMidpointMethod(deltaSeconds);
  }

  // ------------------------------------------------------------------------------------------- //

  godot::KinematicBody *ActorPhysics::getKinematicBody() const {
    return Support::ComponentReferenceHelper::FindParentComponent<godot::KinematicBody>(
      this, this->KinematicBodyNodePath
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Actors
