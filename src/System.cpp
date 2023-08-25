#include "System.h"

#include <cmath>
#include <glm/gtc/random.hpp>

namespace Speck
{

System::System(std::size_t numParticles, float size)
	: m_Size(size)
{
  m_Particles.reserve(numParticles);
  for (std::size_t i = 0; i < numParticles; i++)
  {
    Particle p;

    float x = glm::linearRand(-size, size);
    float y = glm::linearRand(-size, size);
    p.Position = { x, y };
    
    float speed = glm::linearRand(3.0f, 6.0f);
    p.Velocity = glm::circularRand(speed);

    p.NetForce = { 0.0f, 0.0f };

    m_Particles.push_back(p);
  }
}

void System::Update(float timestep)
{
  CalculateForces();

  // Update Position (Euler Integration-TODO: Verlet Integration?)
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];
    particle.Velocity += particle.NetForce * timestep;
    particle.Position += particle.Velocity * timestep;
  }

  BoundPositions();
}

// TODO: Move to a separate class to maintain a matrix of attraction and repulsion.
constexpr float repulsionThreshold = 3.0f;
constexpr float repulsionStrength = 10.0f;

static const glm::vec2& forceFunction(const Particle& particle, const Particle& other)
{
  glm::vec2 delta = particle.Position - other.Position;
  float distance = glm::length(delta);

  if (distance <= repulsionThreshold)
  {
    // The force should be the repulsion strength at 0.0f, and 0.0f at the threshold.
    // Therefore. The slope of the line is strength / threshold. May optimize in future.
    return glm::normalize(delta) * (repulsionStrength / repulsionThreshold) - repulsionThreshold;
  }

  return { 0.0f, 0.0f };
}

void System::CalculateForces()
{
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];
    particle.NetForce = { 0.0f, 0.0f };

    for (std::size_t j = 1; j < m_Particles.size(); j++)
    {
      if (i == j) continue;
      Particle& other = m_Particles[j];
      particle.NetForce += forceFunction(particle, other);
    }
  }
}

void System::BoundPositions()
{
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    glm::vec2& position = m_Particles[i].Position;
    if (position.x > m_Size) position.x -= 2.0f * m_Size;
    if (position.x < -m_Size) position.x += 2.0f * m_Size;
    if (position.y > m_Size) position.y -= 2.0f * m_Size;
    if (position.y < -m_Size) position.y += 2.0f * m_Size;
  }
}

}
