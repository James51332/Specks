#include "System.h"

#include <cmath>
#include <glm/gtc/random.hpp>

namespace Speck
{

System::System(std::size_t numParticles, float size)
	: m_Size(size), m_NumParticles(numParticles)
{
  m_Particles.reserve(numParticles);
  m_ParticleVelocities.reserve(numParticles);
  
  std::size_t rowSize = 10;
  for (std::size_t i = 0; i < numParticles; i++)
  {
    std::size_t x = i % rowSize;
    std::size_t y = (i - x) / rowSize;
    m_Particles.push_back({ 3.0f * x, 3.0f * y });
    m_ParticleVelocities.push_back(glm::circularRand(glm::linearRand(3.0f, 6.0f)));
  }
  
}

void System::Update(float timestep)
{
  for (std::size_t i = 0; i < m_NumParticles; i++)
  {
    glm::vec2& particle = m_Particles[i];
    
    // Update Position (Euler Integration)
    particle += m_ParticleVelocities[i] * timestep;
    
    // Bound Position
    if (particle.x > m_Size) particle.x -= 2.0f * m_Size;
    if (particle.x < -m_Size) particle.x += 2.0f * m_Size;
    if (particle.y > m_Size) particle.y -= 2.0f * m_Size;
    if (particle.y < -m_Size) particle.y += 2.0f * m_Size;
  }
}

}
