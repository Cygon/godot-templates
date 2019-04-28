#ifndef NUCLEX_CPPEXAMPLE_ACTORS_CHARACTERPRESENTER_H
#define NUCLEX_CPPEXAMPLE_ACTORS_CHARACTERPRESENTER_H

#include "../Config.h"

#include <Godot.hpp>
#include <Node.hpp>

namespace Nuclex { namespace CppExample { namespace Actors {
  class ActorPhysics;
}}}

namespace godot {
  class AnimationTree;
}

namespace Nuclex { namespace CppExample { namespace Actors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Updates the animation state of a character to match its actions</summary>
  class CharacterPresenter : public godot::Node {
    GODOT_CLASS(CharacterPresenter, godot::Node)

    /// <summary>Registers the methods callable from within Godot to the engine</summary>
    public: static void _register_methods();

    /// <summary>Initializes a new character presenter component</summary>
    public: NUCLEX_CPPEXAMPLE_API CharacterPresenter();
    /// <summary>Releases all resources before the instance is destroyed</summary>
    public: NUCLEX_CPPEXAMPLE_API ~CharacterPresenter() {}

    /// <summary>Called to initialize or reset the character controller attributes</summary>
    public: NUCLEX_CPPEXAMPLE_API void _init();

    /// <summary>Called each rendered frame to update the visual state</summary>
    /// <param name="deltaSeconds">Time that has passed since the previous frame</param>
    private: NUCLEX_CPPEXAMPLE_API void process(float deltaSeconds);

    /// <summary>Looks up the ActorPhysics component simulating character's physics</summary>
    /// <returns>The ActorPhysics component used by the character</returns>
    private: ActorPhysics *getActorPhysics() const;

    /// <summary>Looks up the AnimationTree component animating the character</summary>
    /// <returns>The AnimationTree component playing character's animations</returns>
    private: godot::AnimationTree *getAnimationTree() const;

    /// <summary>Path to the ActorPhysics node for the character</summary>
    public: godot::NodePath ActorPhysicsNodePath;

    /// <summary>Path to the AnimationTree node for the character</summary>
    public: godot::NodePath AnimationTreeNodePath;

    /// <summary>Velocity at which the running animation will play at 100%</summary>
    public: float MaximumVelocity;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Actors

#endif // NUCLEX_CPPEXAMPLE_ACTORS_CHARACTERPRESENTER_H
