#include "System.h"

#include <cmath>
#include <glm/gtc/random.hpp>

namespace Speck
{

System::System(std::size_t numParticles, float size)
	: m_NumParticles(numParticles), m_Size(size)
{
  m_Positions.reserve(numParticles);
  m_Velocities.reserve(numParticles);
  m_NetForces.reserve(numParticles);

  for (std::size_t i = 0; i < numParticles; i++)
  {
    float x = glm::linearRand(-size, size);
    float y = glm::linearRand(-size, size);
    
    m_Positions.push_back({ x, y });
    m_Velocities.push_back(glm::circularRand(glm::linearRand(3.0f, 6.0f)));
    m_NetForces.push_back({ 0.0f, 0.0f });
  }
}

void System::Update(float timestep)
{
  CalculateForces();

  // Update Position (Euler Integration-TODO: Verlet Integration?)
  for (std::size_t i = 0; i < m_NumParticles; i++)
  {
    m_Velocities[i] += m_NetForces[i] * timestep;
    m_Positions[i] += m_Velocities[i] * timestep;
  }

  BoundPositions();
}

// TODO: Move to a separate class to maintain a matrix of attraction and repulsion.
constexpr float repulsionThreshold = 3.0f;
constexpr float repulsionStrength = 10.0f;

static const glm::vec2& forceFunction(const glm::vec2& particle, const glm::vec2& other)
{
  glm::vec2 delta = particle - other;
  float distance = glm::distance(particle, other);

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
  for (std::size_t i = 0; i < m_NumParticles; i++)
  {
    glm::vec2& particle = m_Positions[i];
    glm::vec2& force = m_NetForces[i];
    force = { 0.0f, 0.0f };

    for (std::size_t j = 1; j < m_NumParticles; j++)
    {
      if (i == j) continue;
      glm::vec2& other = m_Positions[j];
      force += forceFunction(particle, other);
    }
  }
}

void System::BoundPositions()
{
  for (std::size_t i = 0; i < m_NumParticles; i++)
  {
    glm::vec2& particle = m_Positions[i];
    if (particle.x > m_Size) particle.x -= 2.0f * m_Size;
    if (particle.x < -m_Size) particle.x += 2.0f * m_Size;
    if (particle.y > m_Size) particle.y -= 2.0f * m_Size;
    if (particle.y < -m_Size) particle.y += 2.0f * m_Size;
  }
}

}
