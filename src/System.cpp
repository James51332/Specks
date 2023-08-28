#include "System.h"

#include <cmath>
#include <glm/gtc/random.hpp>

namespace Speck
{

System::System(std::size_t numParticles, std::size_t numColors, float size)
	: m_Size(size)
{
  // Cells (as close to interaction radius as possible, without being less)
  m_CellsAcross = static_cast<std::size_t>(2.0f * size / m_InteractionRadius); // truncate, so our cells are slightly bigger than needed
  m_CellSize = (2.0f * size) / static_cast<float>(m_CellsAcross);
  m_Cells.resize(m_CellsAcross * m_CellsAcross);
  
  // Particles
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
  PartitionsParticles();
  CalculateForces(matrix);
  UpdatePositions(timestep);
  BoundPositions();
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
  for (std::size_t i = 0; i < m_Particles.size(); i++)
  {
    Particle& particle = m_Particles[i];
    particle.NetForce = - particle.Velocity * m_FrictionStrength;

    // Create a list of neighboring cells
    constexpr std::size_t numNeighbors = 9;
    static std::size_t neighbors[numNeighbors] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
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
        if (i == otherID) continue;
        Particle& other = m_Particles[otherID];
        
        particle.NetForce += ForceFunction(particle, other, matrix);
      }
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
    if (position.x > m_Size) position.x -= 2.0f * m_Size;
    if (position.x < -m_Size) position.x += 2.0f * m_Size;
    if (position.y > m_Size) position.y -= 2.0f * m_Size;
    if (position.y < -m_Size) position.y += 2.0f * m_Size;
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
  glm::vec2 direction = delta / distance;
  float relDistance = distance / m_InteractionRadius;
  
  float relForceStrength = 0.0f;
  if (relDistance <= m_RepulsionRadius)
  {
    relForceStrength = (relDistance / m_RepulsionRadius - 1);
  }
  else if (relDistance <= 1.0f)
  {
    relForceStrength = 1.0f - glm::abs(2.0f * relDistance - 1.0f - m_RepulsionRadius) / (1.0f - m_RepulsionRadius);
    relForceStrength *= matrix.GetAttractionScale(particle.Color, other.Color);
  }
  
  return (m_InteractionRadius * relForceStrength) * direction;
}


}
