#include "FrictionForce.h"

#include <glm/vec2.hpp>

#include "System.h"

namespace Speck
{
  
void FrictionForce::ApplyForces(System* system, float timestep)
{
  std::vector<Particle>& particles = system->GetParticles();
  for (std::size_t i = 0; i < particles.size(); ++i)
  {
    Particle& particle = particles[i];
    
    glm::vec2 velocityStep = (particle.Position - particle.LastPosition); // This is already in terms of the timestep
    particle.NetForce += -velocityStep * 0.5f;
  }
}

}