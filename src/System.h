#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Speck
{

/// A system keeps tracks of all of the particles in the scene.
class System
{
public:
  System(std::size_t numParticles = 10000);
  
  void Update();
  
  const std::vector<glm::vec2>& GetParticlePositions() const { return m_Particles; }
  
private:
  std::vector<glm::vec2> m_Particles;
};

}
