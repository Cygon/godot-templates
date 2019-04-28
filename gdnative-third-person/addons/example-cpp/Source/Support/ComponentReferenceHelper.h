#ifndef NUCLEX_CPPEXAMPLE_SUPPORT_COMPONENTREFERENCEHELPER_H
#define NUCLEX_CPPEXAMPLE_SUPPORT_COMPONENTREFERENCEHELPER_H

#include "../Config.h"

#include <Godot.hpp>

#include <cassert>

namespace Nuclex { namespace CppExample { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides helper methods for dealing with references between components</summary>
  /// <remarks>
  ///   <para>
  ///     Engines based on a Entity/Component Architecture usually allow many
  ///     components to be attached to an entity and looked up by interface.
  ///     This then becomes the main way of adding functionality to things.
  ///   </para>
  ///   <para>
  ///     In Godot, there's only nodes. Each node can have exactly one attached
  ///     script which adds functionality. This is at odds with the established
  ///     Entity/Component Architecture because cross-talk between nodes can no
  ///     longer discover functionality by looked for a component or interface.
  ///   </para>
  ///   <para>
  ///     One approach to this is to completely rely on Godot's dynamic typing:
  ///     if a bullet impacts an actor, just check if the actor node has a method
  ///     <code>damage()</code> and call it, if present. That requires setting up
  ///     a big central delegating script at the root of each each actor (ugly).
  ///   </para>
  ///   <para>
  ///     The envisioned approach here is to emulate an Entity/Component Architecture
  ///     by putting components in a <tt>Components</tt> folder:
  ///     <code>
  ///       * Player
  ///         # Mesh
  ///         0 CollisionShape
  ///         o Components
  ///           o ActorController
  ///           o ActorPresenter
  ///           o ActorPhysics
  ///           o Abilities
  ///           o Health
  ///     </code>
  ///   </para>
  ///   <para>
  ///     The methods in this helper class assume that the above node layout is
  ///     used and make it easy to look up other components.
  ///   </para>
  /// </remarks>
  class ComponentReferenceHelper {

    /// <summary>Looks for a parent component from another component's place<summary>
    /// <typeparam name="ComponentType">Type of component that will be searched</typeparam>
    /// <param name="base">Component that is looking for a parent component</param>
    /// <param name="nodePath">Expected path of the parent node</param>
    /// <returns>The parent component, if found</returns>
    public: template<typename ComponentType>
    static ComponentType *FindParentComponent(
      const godot::Node *base,
      const godot::NodePath &nodePath = godot::NodePath()
    ) {
      assert((base != nullptr) && "Base must be a valid Godot scene node");

      ComponentType *component;

      // If no path is specified, search upwards in the hierarchy
      if(nodePath.is_empty()) {

        const godot::Node *current = base;
        do {
          component = godot::Object::cast_to<ComponentType>(current);
          if(component != nullptr) {
            return component;
          }

          current = current->get_parent();
        } while(current != nullptr);

      } else {

        const godot::Node *target = base->get_node_or_null(nodePath);
        if(target != nullptr) {
          return godot::Object::cast_to<ComponentType>(target);
        }

      }

      return nullptr;
    }

    /// <summary>Looks for a component from within another component's place<summary>
    /// <typeparam name="ComponentType">Type of component that will be searched</typeparam>
    /// <param name="base">Component that is looking for a sibling component</param>
    /// <param name="nodePath">Expected path of the sibling node</param>
    /// <returns>The sibling component, if found</returns>
    public: template<typename ComponentType>
    static ComponentType *FindSiblingComponent(
      const godot::Node *base,
      const godot::NodePath &nodePath = godot::NodePath()
    ) {
      assert((base != nullptr) && "Base must be a valid Godot scene node");

      ComponentType *component;

      // If no path is specified, search upwards in the hierarchy
      if(nodePath.is_empty()) {
          
        const godot::Node *parent = base->get_parent();

        std::int64_t childCount = parent->get_child_count();
        for(std::int64_t index = 0; index < childCount; ++index) {
          const godot::Node *child = parent->get_child(index);
          component = godot::Object::cast_to<ComponentType>(child);
          if(component != nullptr) {
            return component;
          }
        }

      } else {

        const godot::Node *target = base->get_node_or_null(nodePath);
        if(target != nullptr) {
          return godot::Object::cast_to<ComponentType>(target);
        }

      }

      return nullptr;
    }

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Support

#endif // NUCLEX_CPPEXAMPLE_SUPPORT_COMPONENTREFERENCEHELPER_H
