#ifndef NUCLEX_CPPEXAMPLE_ACTORS_ACTORPHYSICS_H
#define NUCLEX_CPPEXAMPLE_ACTORS_ACTORPHYSICS_H

#include "../Config.h"

#include <Godot.hpp>
#include <Node.hpp>

namespace godot {
  class KinematicBody;
}

namespace Nuclex { namespace CppExample { namespace Actors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Controls an actor's movement in the game world</summary>
  class ActorPhysics : public godot::Node {
    GODOT_CLASS(ActorPhysics, godot::Node)

    /// <summary>Initializes a new actor physics component</summary>
    public: NUCLEX_CPPEXAMPLE_API ActorPhysics();

    /// <summary>Called to initialize or reset the actor physics attributes</summary>
    public: NUCLEX_CPPEXAMPLE_API void _init();

    /// <summary>Called each physics update to update the simulation</summary>
    /// <param name="deltaSeconds">Time that has passed since the previous update</param>
    public: NUCLEX_CPPEXAMPLE_API void _physics_process(float deltaSeconds);

    /// <summary>Queues a direct movement for the actor</summary>
    /// <param name="movement">Movement that will be queued</param>
    /// <remarks>
    ///   This will bypass acceleration/deceleration and attempt to move the actor
    ///   directly by the specified amount during the next physics update. It is
    ///   useful if you want to combine physics with animation-driven root motion.
    /// </remarks>
    public: NUCLEX_CPPEXAMPLE_API void QueueMovement(godot::Vector3 movement) {
      this->queuedMovement += movement;
    }

    /// <summary>Queues a force to affect an actor's velocity</summary>
    /// <param name="force">Force that will be queued for the actor's velocity</param>
    public: NUCLEX_CPPEXAMPLE_API void QueueForce(godot::Vector3 force) {
      this->queuedForces += force;
    }

    /// <summary>Queues an impulse to affect an actor's velocity</summary>
    /// <param name="impulse">Impulse that will be queued for the actor's velocity</param>
    public: NUCLEX_CPPEXAMPLE_API void QueueImpulse(godot::Vector3 impulse) {
      this->queuedImpulses += impulse;
    }

    /// <summary>Applies the force of gravity to the actor</summary>
    /// <param name="gravity">Gravity direction and strength</param>
    public: NUCLEX_CPPEXAMPLE_API void ApplyGravity(godot::Vector3 gravity) {

      // Times mass b/c w/o friction, a feather falls as fast as a lead weight!
      QueueForce(gravity * this->Mass * this->GravityScale);

    }

    /// <summary>Registers the methods callable from within Godot to the engine</summary>
    public: static void _register_methods();

    /// <summary>
    ///   Moves the actor by the specified amount (unless blocked by colliders)
    /// </summary>
    /// <param name="kinematicBody">Kinematic body that will be moved</param>
    /// <param name="translation">Amount by which the actor will be moved</param>
    /// <param name="deltaSeconds">Time elapsed since the previous physics update</param>
    /// <returns>The actual movement performed by the actor</returns>
    /// <remarks>
    ///   If the actor hits a wall, the reported velocity will change.
    /// </remarks>
    private: godot::Vector3 moveActor(
      godot::KinematicBody *kinematicBody, const godot::Vector3 &translation, float deltaSeconds
    );

    /// <summary>
    ///   Recharge the step climb budget relative from the actor's horizontal movement
    /// </summary>
    /// <param name="translation">Amount the actor has moved</param>
    /// <remarks>
    ///   Realistically, this budget would also recover by time: an actor could walk
    ///   up very steep stairs by moving only upwards, but the character controller
    ///   performs horizontal movement in full and only then adjusts height based on
    ///   obstacles, so it's either this or unlimited stair steepness.
    /// </remarks>
    private: void rechargeStepClimbBudget(const godot::Vector3 &translation);

    /// <summary>Updates the recorded velocity of the actor</summary>
    /// <param name="newVelocity">New velocity the actor will report</param>
    /// <param name="updateHorizontalVelocity">
    ///   Whether the horizontal velocity should be update (if no collisions happen
    ///   it's often a good idea to keep the horizontal velocity untouched)
    /// </param>
    private: void updateVelocity(
      const godot::Vector3 &newVelocity, bool updateHorizontalVelocity
    );

    /// <summary>Integrates acceleration and velocity using the Midpoint method</summary>
    /// <param name="deltaTime">Time by which the simulation will be advanced</param>
    /// <returns>The translation by which the actor should be moved</returns>
    private: godot::Vector3 integrateViaMidpointMethod(float deltaSeconds);

    /// <summary>Integrates acceleration and velocity using the Runge-Kutta 4 method</summary>
    /// <param name="deltaTime">Time by which the simulation will be advanced</param>
    /// <returns>The translation by which the actor should be moved</returns>
    private: godot::Vector3 integrateViaRungeKutta4Method(float deltaSeconds);

    /// <summary>Looks up the kinematic body that this component is controlling</summary>
    /// <returns>The kinematic body controlled by this component</returns>
    private: godot::KinematicBody *getKinematicBody() const;

    /// <summary>Whether the actor is being affected by gravity</summary>
    /// <remarks>
    ///   If set, the gravity from Unity's physics settings is applied as a force
    ///   automatically before each update. Only set this to false if you do fancy
    ///   things with gravity.
    /// </remarks>
    public: bool IsAffectedByGravity;
    /// <summary>Direction and strength of gravity for the actor</summary>
    /// <remarks>
    ///   This defaults to earth-equivalent gravity in the metric system. The length
    ///   of the vector is the strength of gravity. You can update this attribute,
    ///   for example through Godot's Area node (it has a special event that is triggered
    ///   when a PhysicsBody such as the KinematicBody required by the ActorPhysics
    ///   component enters or leaves).
    /// </remarks>
    public: godot::Vector3 GravityVector;
    /// <summary>How much the actor is affected by gravity</summary>
    /// <remarks>
    ///   You may want to increase this for fast platformers since a realistic amount
    ///   of gravity makes for very boring movements when combined with unrealistic
    ///   jump heights (of multiple times the actor's own body height).
    /// </remarks>
    public: float GravityScale;
    /// <summary>Mass of the actor</summary>
    /// <remarks>
    ///   This should include the equipment carried by the actor.
    ///   <list>
    ///     <item>
    ///       <term>Human</term>
    ///       <description>75 kilograms</description>
    ///     </item>
    ///     <item>
    ///       <term>Dog</term>
    ///       <description>35 kilograms</description>
    ///     </item>
    ///     <item>
    ///       <term>Horse</term>
    ///       <description>450 kilograms</description>
    ///     </item>
    ///     <item>
    ///       <term>Car</term>
    ///       <description>1500 kilograms</description>
    ///     </item>
    ///   </list>
    /// </remarks>
    public: float Mass;
    /// <summary>Maximum step height the character can traverse without jumping</summary>
    /// <remarks>
    ///   The character controller used by the actor physics component moves the actor
    ///   the full horizontal distance desired and adjusts the height as needed
    ///   (up to this height). The actor will move on top of the step without gaining
    ///   any upward velocity.
    /// </remarks>
    public: float MaximumStepHeight;
    /// <summary>Current velocity of the actor</summary>
    public: godot::Vector3 Velocity;
    /// <summary>Whether a high quality integrator should be used for physics</summary>
    /// <remarks>
    ///   If this is disabled, midpoint integration will be used (which is already beyond
    ///   the effort applied by most games to integrate forces). For a better match with
    ///   physics engines (or if you have really sensible control systems), you can opt
    ///   into using higher order integrator that is more computationally demanding.
    /// </remarks>
    public: bool UseHighQualityIntegration;

    /// <summary>Path to the kinematic body this component is controlling</summary>
    public: godot::NodePath KinematicBodyNodePath;

    /// <summary>Force that has been queued for the actor's velocity</summary>
    private: godot::Vector3 queuedForces;
    /// <summary>Impulses that have been queued for the actor's velocity</summary>
    private: godot::Vector3 queuedImpulses;
    /// <summary>Movements that have been queued for the actor</summary>
    private: godot::Vector3 queuedMovement;

    /// <summary>Stores half of the acceleration from the last physics update</summary>
    /// <remarks>
    ///   This is used when doing Euler integration using the Midpoint Method,
    ///   where half of the acceleration is integrated into velocity before updating
    ///   the actor's position and half of the acceleration is integrated after.
    /// </remarks>
    private: godot::Vector3 midPointVelocity; // acceleration * deltaSeconds

    /// <summary>Remaining height of a step the character will be able to traverse</summary>
    /// <remarks>
    ///   <para>
    ///     Careful: the step climbing budget is negative! 0.0 means the budget is full,
    ///     and it is exhausted at -MaximumStepHeight.
    ///   </para>
    ///   <para>
    ///     The step climbing budget allows characters to move across vertical steps up to
    ///     a certain height. However, this might accidentally allow the character to scale
    ///     steep walls if the step is traced anew each physics frame. In order to prevent
    ///     this, a climbing budget is exhausted each time a step is climbed. It is recharged
    ///     by horizontal movement.
    ///   </para>
    /// </remarks>
    private: float stepClimbingBudget;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Actors

#endif // NUCLEX_CPPEXAMPLE_ACTORS_ACTORPHYSICS_H
