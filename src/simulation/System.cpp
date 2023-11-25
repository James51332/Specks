#include "System.h"

#include <glm/gtc/random.hpp>
#include <thread>
#include <SDL.h>

namespace Speck
{

System::System(std::size_t numParticles, std::size_t numColors, float size)
	: m_Size(size)
{
  AllocateCells();
  AllocateParticles(numParticles, numColors);
}

void System::Update(const ColorMatrix& matrix, float timestep)
{
  PartitionsParticles();
  CalculateForces(matrix, timestep); // Timestep needed for velocity-based friction
  UpdatePositions(timestep);
  BoundPositions();
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

void System::CalculateForces(const ColorMatrix& matrix, float timestep)
{
  // Thread Job Function (We only write to our specified particle[i].netforce and never read it, so there's no need for locks)
  auto jobFunc = [&](std::size_t start, std::size_t end, float timestep)
  {
    for (std::size_t i = start; i <= end; i++)
    {
      Particle& particle = m_Particles[i];

      // Friction
      glm::vec2 velocity = (particle.Position - particle.LastPosition) / timestep;
      particle.NetForce = -velocity * m_FrictionStrength;

      // Create a list of neighboring cells
      constexpr std::size_t numNeighbors = 9;
      std::size_t neighbors[numNeighbors];
      {
        // Find the x and y of our current cell
        int32_t cell = particle.CellIndex;
        int32_t cellX = particle.CellIndex % m_CellsAcross;
        int32_t cellY = particle.CellIndex / m_CellsAcross; // integer division
        
        // Find the value we add to the cell's index to move in a certain direction, accounting for wrapping
        int32_t ld = (cellX != 0) ? -1 : (m_CellsAcross - 1); // if on left, add the size of grid
        int32_t rd = (cellX != m_CellsAcross - 1) ? 1 : -static_cast<int32_t>(m_CellsAcross - 1); // if on right, subtract size of grid
        int32_t ud = (cellY != 0) ? -m_CellsAcross : (m_CellsAcross * (m_CellsAcross - 1)); // if on top, add size of grid
        int32_t dd = (cellY != m_CellsAcross - 1) ? m_CellsAcross : -static_cast<int32_t>(m_CellsAcross * (m_CellsAcross - 1)); // if on bottom, subtract size of grid
        
        // Create our list by moving our cell's index in all of these directions
        neighbors[0] = particle.CellIndex + ld + ud;
        neighbors[1] = particle.CellIndex      + ud;
        neighbors[2] = particle.CellIndex + rd + ud;
        neighbors[3] = particle.CellIndex + ld     ;
        neighbors[4] = particle.CellIndex          ;
        neighbors[5] = particle.CellIndex + rd     ;
        neighbors[6] = particle.CellIndex + ld + dd;
        neighbors[7] = particle.CellIndex      + dd;
        neighbors[8] = particle.CellIndex + rd + dd;
      }
      
      // Iterate over these neighbors
      for (std::size_t neighbor = 0; neighbor < numNeighbors; neighbor++)
      {
        int32_t cellIndex = neighbors[neighbor];
        Cell& cell = m_Cells[cellIndex];
        
        for (std::size_t j = 0; j < cell.Particles.size(); j++)
        {
          std::size_t otherID = cell.Particles[j];
          if (particle.ID == otherID) continue;
          Particle& other = m_Particles[otherID];
          
          particle.NetForce += ForceFunction(particle, other, matrix);
        }
      }
    }
  };
  
  // If we have less than 100 particles, the overhead isn't needed, and it's hard to distrubute particles anyways
  if (m_Particles.size() == 0)
  {
    return;
  }
  else if (m_Particles.size() < 100 || !m_ThreadedDispatch)
  {
    jobFunc(0, m_Particles.size() - 1, timestep);
  }
  else
  {
    // Spawn our job threads
    constexpr static std::size_t numWorkers = 16;
    std::size_t particlesPerWorker = m_Particles.size() / numWorkers + 1; // integer division, add 1 (cover all)

    std::thread workers[numWorkers];
    for (std::size_t i = 0; i < numWorkers; i++)
    {
      std::size_t start = i * particlesPerWorker;
      std::size_t end = start + (particlesPerWorker - 1);
      end = (end >= m_Particles.size()) ? m_Particles.size() - 1 : end; // cap end at last particle.

      workers[i] = std::thread(jobFunc, start, end, timestep);
    }

    // Wait for our workers
    for (std::size_t i = 0; i < numWorkers; i++)
    {
      workers[i].join();
    }
  }
}

void System::UpdatePositions(float timestep)
{
  // Update Position
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];

    glm::vec2 acceleration = particle.NetForce; // All particles have equal mass right now
    glm::vec2 newPos = 2.0f * particle.Position - particle.LastPosition + acceleration * timestep * timestep; // Verlet integration

    particle.LastPosition = particle.Position;
    particle.Position = newPos;
  }
}

void System::BoundPositions()
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

glm::vec2 System::ForceFunction(const Particle& particle, const Particle& other, const ColorMatrix& matrix)
{
  // Get the direction towards other particle, accounting for boundary wrapping.
  glm::vec2 delta = other.Position - particle.Position;
  if (delta.x > m_Size) delta.x -= 2.0f * m_Size;
  if (delta.x < -m_Size) delta.x += 2.0f * m_Size;
  if (delta.y > m_Size) delta.y -= 2.0f * m_Size;
  if (delta.y < -m_Size) delta.y += 2.0f * m_Size;
  
  float distance = glm::length(delta);
  
  // An interesting consequence of non-inverse-square law repulsion 
  // is that it minimizes potential energy to create pockets instead of uniform particles.
  // We may want a more physically accurate simulation in the future, that accounts for the
  // total energy in the system.
  if (distance <= m_RepulsionRadius * m_InteractionRadius)
  {
    float forceStrength = (distance / m_RepulsionRadius - m_InteractionRadius);
    glm::vec2 dir = delta / distance;
    return forceStrength * dir;
  }
  else if (distance <= m_InteractionRadius)
  {
    float forceStrength = m_InteractionRadius - glm::abs((2.0f * distance - m_InteractionRadius - m_RepulsionRadius * m_InteractionRadius) / (1.0f - m_RepulsionRadius));
    forceStrength *= matrix.GetAttractionScale(particle.Color, other.Color);
    glm::vec2 dir = delta / distance;
    
    return forceStrength * dir;
  }
  else
  {
    return { 0.0f, 0.0f };
  }
}

}
