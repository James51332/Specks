#include "System.h"

#include <glm/gtc/random.hpp>
#include <thread>

namespace Speck
{

System::System(std::size_t numParticles, std::size_t numColors, float size)
	: m_Size(size)
{
  AllocateCells();
  
  // Particles
  m_Particles.reserve(numParticles);
  for (std::size_t i = 0; i < numParticles; i++)
  {
    Particle p;
    p.ID = i;

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
  PartitionsParticles();
  CalculateForces(matrix);
  UpdatePositions(timestep);
  BoundPositions();
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

void System::CalculateForces(const ColorMatrix& matrix)
{
  // Thread Job Function (We only write to our specified particle[i].netforce and never read it, so there's no need for locks)
  auto jobFunc = [&](std::size_t start, std::size_t end)
  {
    for (std::size_t i = start; i <= end; i++)
    {
      Particle& particle = m_Particles[i];
      particle.NetForce = - particle.Velocity * m_FrictionStrength;
      
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
  if (m_Particles.size() < 100)
  {
    jobFunc(0, m_Particles.size() - 1);
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

      workers[i] = std::thread(jobFunc, start, end);
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
  // Update Position (We use a modified version of velocity verlet to integrate)
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];
    particle.Position += (particle.Velocity + particle.NetForce * timestep) * timestep;
    particle.Velocity += particle.NetForce * timestep;
  }
}

void System::BoundPositions()
{
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    glm::vec2& position = m_Particles[i].Position;

    // We force them to move to edge because velocity can get out of hand when paused for long time.
    if (position.x > m_Size) position.x = -m_Size;
    if (position.x < -m_Size) position.x = m_Size;
    if (position.y > m_Size) position.y = -m_Size;
    if (position.y < -m_Size) position.y = m_Size;
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
