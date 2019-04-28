#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "CharacterPresenter.h"
#include "../Support/ComponentReferenceHelper.h"
#include "ActorPhysics.h"

#include <AnimationTree.hpp>

#include <cassert>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Default path to the ActorPhysics node</summary>
  const std::string DefaultActorPhysicsNodePath = "../ActorPhysics";

  /// <summary>Default path to the AnimationTree node</summary>
  const std::string DefaultAnimationTreeNodePath = "../AnimationTree";

  /// <summary>Default maximum velocity at which the running animation will play</summary>
  const float DefaultMaximumVelocity = 2.5f;

  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace CppExample { namespace Actors {

  // ------------------------------------------------------------------------------------------- //

  CharacterPresenter::CharacterPresenter() :
    ActorPhysicsNodePath(DefaultActorPhysicsNodePath.c_str()),
    AnimationTreeNodePath(DefaultAnimationTreeNodePath.c_str()),
    MaximumVelocity(DefaultMaximumVelocity) {}

  // ------------------------------------------------------------------------------------------- //

  void CharacterPresenter::_init() {
    this->ActorPhysicsNodePath = godot::NodePath(DefaultActorPhysicsNodePath.c_str());
    this->AnimationTreeNodePath = godot::NodePath(DefaultAnimationTreeNodePath.c_str());
    this->MaximumVelocity = DefaultMaximumVelocity;
  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterPresenter::process(float deltaSeconds) {
    godot::AnimationTree *animationTree = getAnimationTree();
    if(animationTree == nullptr) {
      godot::Godot::print("ERROR: CharacterPresenter could not find its AnimationTree");
      return;
    }

    ActorPhysics *actorPhysics = getActorPhysics();
    if(actorPhysics == nullptr) {
      godot::Godot::print("ERROR: CharacterPresenter could not find its ActorPhysics");
      return;
    }

    // Assumption: the animation tree is a 2D blend tree and nothing more.
    // If you have more complex animations, this is what you need to update.
    godot::Vector2 horizontal_movement(
      actorPhysics->Velocity.x, -actorPhysics->Velocity.z
    );
    horizontal_movement /= this->MaximumVelocity;

    // Assign the movement speed to the 2D blend tree
    animationTree->set("parameters/blend_position", horizontal_movement);
  }

  // ------------------------------------------------------------------------------------------- //

  void CharacterPresenter::_register_methods() {
    godot::register_property<CharacterPresenter, godot::NodePath>(
      "actor_physics_node_path",                                  
      &CharacterPresenter::ActorPhysicsNodePath,
      godot::NodePath(DefaultActorPhysicsNodePath.c_str())
    );

    godot::register_property<CharacterPresenter, godot::NodePath>(
      "animation_tree_node_path",
      &CharacterPresenter::AnimationTreeNodePath,
      godot::NodePath(DefaultAnimationTreeNodePath.c_str())
    );

    godot::register_method("_process", &CharacterPresenter::process);
  }

  // ------------------------------------------------------------------------------------------- //

  ActorPhysics *CharacterPresenter::getActorPhysics() const {
    return Support::ComponentReferenceHelper::FindSiblingComponent<ActorPhysics>(
      this, this->ActorPhysicsNodePath
    );
  }

  // ------------------------------------------------------------------------------------------- //

  godot::AnimationTree *CharacterPresenter::getAnimationTree() const {
    return Support::ComponentReferenceHelper::FindSiblingComponent<godot::AnimationTree>(
      this, this->AnimationTreeNodePath
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Actors
