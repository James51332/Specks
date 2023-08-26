#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Particle.h"
#include "ColorMatrix.h"

namespace Speck
{

/// A system keeps tracks of all of the particles in the scene.
class System
{
public:
  System(std::size_t numParticles = 10000, std::size_t numColors = 1, float size = 100.0f);
  
  void Update(const ColorMatrix& matrix, float timestep);
  
  const std::vector<Particle>& GetParticles() const { return m_Particles; }
  float GetBoundingBoxSize() const { return m_Size; }
  
private:
  void CalculateForces(const ColorMatrix& matrix);
  void BoundPositions();

private:
  std::vector<Particle> m_Particles;

  // Size of the bounding box at which point particles will wrap around.
  // Goes from -m_Size to m_Size on both x and y axes.
  float m_Size;
};

}
