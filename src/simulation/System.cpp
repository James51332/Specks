#include "System.h"

#include <glm/gtc/random.hpp>
#include <SDL.h>

namespace Speck
{

System::System(std::size_t numParticles, std::size_t numColors, float size)
	: m_Size(size)
{
  AllocateCells();
  AllocateParticles(numParticles, numColors);
}

void System::AllocateParticles(std::size_t numParticles, std::size_t numColors)
{
  // If we are removing particles
  std::size_t currentParticles = m_Particles.size();
  if (currentParticles >= numParticles)
  {
    m_Particles.resize(numParticles);
    return;
  }

  // Allocate space for our particles
  m_Particles.reserve(numParticles);

  // Iteratively create our particles
  for (std::size_t i = currentParticles; i < numParticles; i++)
  {
    Particle p;
    p.ID = i;

    // Uniform Random Distribution
    float x = glm::linearRand(-m_Size, m_Size);
    float y = glm::linearRand(-m_Size, m_Size);
    p.Position = {x, y};
    p.LastPosition = p.Position;

    p.NetForce = {0.0f, 0.0f};

    p.Color = i % numColors;

    m_Particles.push_back(p);
  }
}

void System::AllocateCells()
{
  // Cells (as close to interaction radius as possible, without being less)
  m_CellsAcross = static_cast<std::size_t>(2.0f * m_Size / m_InteractionRadius); // truncate, so our cells are slightly bigger than needed
  m_CellSize = (2.0f * m_Size) / static_cast<float>(m_CellsAcross);
  m_Cells.resize(m_CellsAcross * m_CellsAcross);
}

void System::PartitionsParticles()
{
  // Clear all cells
  for (std::size_t i = 0; i < m_Cells.size(); i++)
  {
    m_Cells[i].Particles.clear();
  }
  
  // Emplace all particles into cells
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];
    std::size_t cellX = static_cast<std::size_t>((particle.Position.x + m_Size) / m_CellSize);
    std::size_t cellY = static_cast<std::size_t>((m_Size - particle.Position.y) / m_CellSize);
    
    // Due to rounding, we have to ensure that in rare cases, we don't index out of bound
    if (cellX == m_CellsAcross) cellX--;
    if (cellY == m_CellsAcross) cellY--;
    std::size_t cell = cellY * m_CellsAcross + cellX;
    
    m_Cells[cell].Particles.push_back(i);
    particle.CellIndex = cell; // particles cache their cell's index as well.
  }
}

void System::UpdatePositions(float timestep)
{
  // Update Position
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];

    glm::vec2 acceleration = particle.NetForce; // All particles have equal mass right now
    glm::vec2 newPos = 2.0f * particle.Position - particle.LastPosition + acceleration * timestep; // Verlet integration

    particle.LastPosition = particle.Position;
    particle.Position = newPos;
  }
}

void System::WrapPositions()
{
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    glm::vec2& position = m_Particles[i].Position;
    glm::vec2& lastPosition = m_Particles[i].LastPosition;
    glm::vec2 delta = position - lastPosition;

    // We force them to move to edge because velocity can get out of hand when paused for long time.
    bool update = false;
    if (position.x > m_Size) 
    {
      position.x = -m_Size;
      update = true;
    }
    else if (position.x < -m_Size) 
    {
      position.x = m_Size;
      update = true;
    }
    if (position.y > m_Size) 
    {
      position.y = -m_Size;
      update = true;
    }
    else if (position.y < -m_Size) 
    {
      position.y = m_Size;
      update = true;
    }   

    if (update)
    {
      lastPosition = position - delta; // Maintain velocity
    }
  }
}

void System::ClampPositions(float dampening)
{
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    glm::vec2 &position = m_Particles[i].Position;
    glm::vec2 &lastPosition = m_Particles[i].LastPosition;
    glm::vec2 delta = position - lastPosition;

    // We force them to move to edge because velocity can get out of hand when paused for long time.
    bool update = false;
    if (position.x > m_Size)
    {
      position.x = m_Size;
      update = true;
    }
    else if (position.x < -m_Size)
    {
      position.x = -m_Size;
      update = true;
    }
    if (position.y > m_Size)
    {
      position.y = m_Size;
      update = true;
    }
    else if (position.y < -m_Size)
    {
      position.y = -m_Size;
      update = true;
    }

    if (update)
    {
      lastPosition = position + delta * dampening; // Maintain velocity
    }
  }
}

void System::ZeroForces()
{
  for (std::size_t i = 0; i < m_Particles.size(); ++i)
  {
    m_Particles[i].NetForce = {0.0f, 0.0f};
  }
}

}
