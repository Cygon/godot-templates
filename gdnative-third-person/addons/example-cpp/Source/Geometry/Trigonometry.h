#ifndef NUCLEX_CPPEXAMPLE_GEOMETRY_TRIGONOMETRY_H
#define NUCLEX_CPPEXAMPLE_GEOMETRY_TRIGONOMETRY_H

#include "../Config.h"

#include <Godot.hpp>

namespace Nuclex { namespace CppExample { namespace Geometry {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides trigonometric constants and helper methods</summary>
  class Trigonometry : public godot::Reference {
    GODOT_CLASS(Trigonometry, godot::Reference)

    /// <summary>Initializes a new extremes container</summary>
    public: NUCLEX_CPPEXAMPLE_API Trigonometry() {}

    /// <summary>Called to initialize or reset the camera's attributes</summary>
    public: NUCLEX_CPPEXAMPLE_API void _init() {}

    /// <summary>Registers the methods callable from within Godot to the engine</summary>
    public: NUCLEX_CPPEXAMPLE_API static void _register_methods();

    /// <summary>Converts from degree to radian</summary>
    /// <param name="degrees">Angle in degrees that will be converted</param>
    /// <returns>The equivalent radian angle</returns>
    public: NUCLEX_CPPEXAMPLE_API static const float RadiansFromDegrees(float degrees) {
      return degrees * RadiansPerDegree;
    }

    /// <summary>Converts from radian to degree</summary>
    /// <param name="phi">Radian angle that will be converted</param>
    /// <returns>The equivalent angle in degrees</returns>
    public: NUCLEX_CPPEXAMPLE_API static const float DegreesFromRadians(float radians) {
      return radians * DegreesPerRadian;
    }

    /// <summary>Complains if someone attempts to change one of the constants</summary>
    /// <param name="scalar">Value that will not be assigned to the constant</param>
    private: void reportErrorWhenSettingConstant(float scalar) {
      godot::Godot::print("ERROR: Constant is read-only and cannot be changed");
    }

    /// <summary>Getter that returns half the value of PI</summary>
    private: float getHalfPI() const { return HalfPI; }
    /// <summary>Getter that returns the circle constant PI</summary>
    private: float getPI() const { return PI; }
    /// <summary>Getter that returns twice the value of PI</summary>
    private: float getTau() const { return Tau; }

    /// <summary>Getter that returns the number of radians per old-style degree</summary>
    private: float getDegreesPerRadian() const { return DegreesPerRadian; }
    /// <summary>Gette that returns the number of old-style degrees per radian</summary>
    private: float getRadiansPerDegree() const { return RadiansPerDegree; }

    /// <summary>Converts an angle in radians into old-style degrees</summary>
    /// <param name="radians">Angle in radians</param>
    /// <returns>The same angle represented in old-style degrees</summary>
    private: float getDegreesFromRadians(float radians) { return radians * DegreesPerRadian; }

    /// <summary>Converts an angle in old-style degrees into radians</summary>
    /// <param name="degrees">Angle in old-style degrees</param>
    /// <returns>The same angle represented in radians</summary>
    private: float getRadiansFromDegrees(float degrees) { return degrees * RadiansPerDegree; }

    /// <summary>Half the value of PI</summary>
    public: static const float HalfPI;
    /// <summary>The circle constant PI</summary>
    public: static const float PI;
    /// <summary>Twice the value of PI</summary>
    public: static const float Tau;

    /// <summary>Radians per old-style degree</summary>
    /// <remarks>
    ///   Multiply by this to convert from degree to radian
    /// </remarks>
    public: static const float RadiansPerDegree;
    /// <summary>Old-style degrees per radian</summary>
    /// <remarks>
    ///   Multiply by this to convert from radian to degree
    /// </remarks>
    public: static const float DegreesPerRadian;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Geometry

#endif // NUCLEX_CPPEXAMPLE_GEOMETRY_TRIGONOMETRY_H
