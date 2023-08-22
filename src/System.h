#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Speck
{

/// A system keeps tracks of all of the particles in the scene.
class System
{
public:
  System(std::size_t numParticles = 10000, float size = 50.0f);
  
  void Update(float timestep);
  
  const std::vector<glm::vec2>& GetParticlePositions() const { return m_Particles; }
  float GetBoundingBoxSize() const { return m_Size; }
  
private:
  // We are gonna keep each of these seperate so it's easy
  // to copy render data to the gpu. We can test other configurations
  // in the future, but this seems easy.
  std::size_t m_NumParticles;
  std::vector<glm::vec2> m_Particles;
  std::vector<glm::vec2> m_ParticleVelocities;
  
  // Size of the bounding box at which point particles will wrap around.
  // Goes from -m_Size to m_Size on both x and y axes.
  float m_Size;
};

}
