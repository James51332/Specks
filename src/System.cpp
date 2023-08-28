#include "System.h"

#include <cmath>
#include <glm/gtc/random.hpp>

namespace Speck
{

System::System(std::size_t numParticles, std::size_t numColors, float size)
	: m_Size(size)
{
  m_Particles.reserve(numParticles);
  for (std::size_t i = 0; i < numParticles; i++)
  {
    Particle p;

    float x = glm::linearRand(-size, size);
    float y = glm::linearRand(-size, size);
    p.Position = { x, y };
    
    float speed = 15.0f;
    p.Velocity = glm::circularRand(speed);

    p.NetForce = { 0.0f, 0.0f };

    p.Color = i % numColors;

    m_Particles.push_back(p);
  }
}

void System::Update(const ColorMatrix& matrix, float timestep)
{
  CalculateForces(matrix);

  // Update Position (We use a modified version of velocity verlet to integrate)
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];
    particle.Position += (particle.Velocity + particle.NetForce * timestep) * timestep;
    particle.Velocity += particle.NetForce * timestep;
  }

  BoundPositions();
}

// Constants the define the parameters of the simulation
constexpr float frictionStrength = 2.0f;
constexpr static float interactionRadius = 30.0f;
constexpr static float repulsionRadius = 0.3f;

static const glm::vec2& forceFunction(const Particle& particle, const Particle& other, const ColorMatrix& matrix, float size)
{
  // Get the direction towards other particle, accounting for boundary wrapping.
  glm::vec2 delta = other.Position - particle.Position;
  if (delta.x > size) delta.x -= 2.0f * size;
  if (delta.x < -size) delta.x += 2.0f * size;
  if (delta.y > size) delta.y -= 2.0f * size;
  if (delta.y < -size) delta.y += 2.0f * size;

  glm::vec2 direction = glm::normalize(delta);
  float relDistance = glm::length(delta) / interactionRadius;

  float relForceStrength = 0.0f;
  if (relDistance <= repulsionRadius)
  {
    relForceStrength = (relDistance / repulsionRadius - 1);
  } 
  else if (relDistance <= 1.0f)
  {
    relForceStrength = 1.0f - glm::abs(2.0f * relDistance - 1.0f - repulsionRadius) / (1.0f - repulsionRadius);
    relForceStrength *= matrix.GetAttractionScale(particle.Color, other.Color);
  }

  return interactionRadius * direction * relForceStrength;
}

void System::CalculateForces(const ColorMatrix& matrix)
{
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];
    particle.NetForce = - particle.Velocity * frictionStrength;

    for (std::size_t j = 0; j < m_Particles.size(); j++)
    {
      if (i == j) continue;
      Particle& other = m_Particles[j];
      particle.NetForce += forceFunction(particle, other, matrix, m_Size);
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
