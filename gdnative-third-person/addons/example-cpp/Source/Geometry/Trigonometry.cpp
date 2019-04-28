#define NUCLEX_CPPEXAMPLE_SOURCE 1

#include "Trigonometry.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

#if CAN_BIND_PROPERTIES_FROM_ANOTHER_CLASS
  class Exports : Nuclex::CppExample::Geometry::Trigonometry {

    public: float GetHalfPI() const {
      return Nuclex::CppExample::Geometry::Trigonometry::HalfPI;
    }
    public: float GetPI() const {
      return Nuclex::CppExample::Geometry::Trigonometry::PI;
    }
    public: float GetTau() const {
      return Nuclex::CppExample::Geometry::Trigonometry::Tau;
    }
    public: float GetDegreesPerRadian() const {
      return Nuclex::CppExample::Geometry::Trigonometry::DegreesPerRadian;
    }
    public: float GetRadiansPerDegree() const {
      return Nuclex::CppExample::Geometry::Trigonometry::RadiansPerDegree;
    }
    public: float DegreesFromRadians(float radians) {
      return radians * Nuclex::CppExample::Geometry::Trigonometry::DegreesPerRadian;
    }
    public: float RadiansFromDegrees(float degrees) {
      return degrees * Nuclex::CppExample::Geometry::Trigonometry::RadiansPerDegree;
    }

  };
#endif
  
  // ------------------------------------------------------------------------------------------- //

}

namespace Nuclex { namespace CppExample { namespace Geometry {

  // ------------------------------------------------------------------------------------------- //

  void Trigonometry::_register_methods() {
    godot::register_property<Trigonometry, float>(
      "half_pi",
      &Trigonometry::reportErrorWhenSettingConstant, &Trigonometry::getHalfPI,
      Trigonometry::HalfPI
    );
    godot::register_property<Trigonometry, float>(
      "pi",
      &Trigonometry::reportErrorWhenSettingConstant, &Trigonometry::getPI,
      Trigonometry::PI
    );
    godot::register_property<Trigonometry, float>(
      "tau",
      &Trigonometry::reportErrorWhenSettingConstant, &Trigonometry::getTau,
      Trigonometry::Tau
    );
    godot::register_property<Trigonometry, float>(
      "radians_per_degree",
      &Trigonometry::reportErrorWhenSettingConstant, &Trigonometry::getRadiansPerDegree,
      Trigonometry::RadiansPerDegree
    );
    godot::register_property<Trigonometry, float>(
      "degrees_per_radian",
      &Trigonometry::reportErrorWhenSettingConstant, &Trigonometry::getDegreesPerRadian,
      Trigonometry::DegreesPerRadian
    );
  }

  // ------------------------------------------------------------------------------------------- //

  const float Trigonometry::HalfPI = (
    1.57079632679489661923132169163975144209858469968755291048747229615390820314310449931401741267f
  );

  // ------------------------------------------------------------------------------------------- //

  const float Trigonometry::PI = (
    3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534f
  );

  // -------------------------------------------------------------------------------------------- //

  const float Trigonometry::Tau = (                                 -
    6.28318530717958647692528676655900576839433879875021164194988918461563281257241799725606965068f
  );

  // ------------------------------------------------------------------------------------------- //

  const float Trigonometry::RadiansPerDegree = (
    0.01745329251994329576923690768488612713442871888541725456097191440171009114603449443682241570f
  );

  // ------------------------------------------------------------------------------------------- //

  const float Trigonometry::DegreesPerRadian = (
    57.2957795130823208767981548141051703324054724665643215491602438612028471483215526324409689959f
  );

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::CppExample::Geometry
