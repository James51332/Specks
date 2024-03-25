#pragma once

#include <glm/vec2.hpp>

#include "ForceApplicator.h"
#include "ColorMatrix.h"

namespace Speck
{

class ColorForce : public ForceApplicator
{
public:
  ColorForce() {}
  ~ColorForce() = default;

  glm::vec2 ForceFunction(const Particle &particle, const Particle &other, System* system, const ColorMatrix& matrix);
  void ApplyForces(System* system, const ColorMatrix& matrix, float timestep);

  void SetMultiThreaded(bool multithreaded = true) { m_Multithreaded = multithreaded; }
  bool IsMultiThreaded() const { return m_Multithreaded; }

private:
  float m_RepulsionRadius = 0.3f;
  bool m_Multithreaded = true;
};
  
}