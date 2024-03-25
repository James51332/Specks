#include "ColorForce.h"

#include <thread>

#include "System.h"

namespace Speck
{

glm::vec2 ColorForce::ForceFunction(const Particle& particle, const Particle& other, System* system, const ColorMatrix& matrix)
{
  float systemSize = system->GetBoundingBoxSize();
  float systemInteractionRadius = system->GetInteractionRadius();

  // Get the direction towards other particle, accounting for boundary wrapping.
  glm::vec2 delta = other.Position - particle.Position;
  if (delta.x > systemSize) delta.x -= 2.0f  * systemSize;
  if (delta.x < -systemSize) delta.x += 2.0f * systemSize;
  if (delta.y > systemSize) delta.y -= 2.0f  * systemSize;
  if (delta.y < -systemSize) delta.y += 2.0f * systemSize;
  
  float distance = glm::length(delta);
  
  // An interesting consequence of non-inverse-square law repulsion 
  // is that it minimizes potential energy to create pockets instead of uniform particles.
  // We may want a more physically accurate simulation in the future, that accounts for the
  // total energy in the system.
  if (distance <= m_RepulsionRadius * systemInteractionRadius)
  {
    float forceStrength = (distance / m_RepulsionRadius - systemInteractionRadius);
    glm::vec2 dir = delta / distance;
    return forceStrength * dir;
  }
  else if (distance <= systemInteractionRadius)
  {
    float forceStrength = systemInteractionRadius - glm::abs((2.0f * distance - systemInteractionRadius - m_RepulsionRadius * systemInteractionRadius) / (1.0f - m_RepulsionRadius));
    forceStrength *= matrix.GetAttractionScale(particle.Color, other.Color);
    glm::vec2 dir = delta / distance;
    
    return forceStrength * dir;
  }
  else
  {
    return { 0.0f, 0.0f };
  }
}

void ColorForce::ApplyForces(System* system, const ColorMatrix& matrix, float timestep)
{
  std::vector<Particle>& particles = system->GetParticles();
  std::size_t cellsAcross = system->GetCellsAcross();
  std::vector<Cell> &cells = system->GetCells();

  // Thread Job Function (We only write to our specified particle[i].netforce and never read it, so there's no need for locks)
  auto jobFunc = [&](std::size_t start, std::size_t end, float timestep)
  {
    for (std::size_t i = start; i <= end; i++)
    {
      Particle &particle = particles[i];

      // Create a list of neighboring cells
      constexpr std::size_t numNeighbors = 9;
      std::size_t neighbors[numNeighbors];
      {
        // Find the x and y of our current cell
        int32_t cell = particle.CellIndex;
        int32_t cellX = particle.CellIndex % cellsAcross;
        int32_t cellY = particle.CellIndex / cellsAcross; // integer division

        // Find the value we add to the cell's index to move in a certain direction, accounting for wrapping
        int32_t ld = (cellX != 0) ? -1 : (cellsAcross - 1);                                                                   // if on left, add the size of grid
        int32_t rd = (cellX != cellsAcross - 1) ? 1 : -static_cast<int32_t>(cellsAcross - 1);                               // if on right, subtract size of grid
        int32_t ud = (cellY != 0) ? -cellsAcross : (cellsAcross * (cellsAcross - 1));                                     // if on top, add size of grid
        int32_t dd = (cellY != cellsAcross - 1) ? cellsAcross : -static_cast<int32_t>(cellsAcross * (cellsAcross - 1)); // if on bottom, subtract size of grid
        // Create our list by moving our cell's index in all of these directions
        neighbors[0] = particle.CellIndex + ld + ud;
        neighbors[1] = particle.CellIndex + ud;
        neighbors[2] = particle.CellIndex + rd + ud;
        neighbors[3] = particle.CellIndex + ld;
        neighbors[4] = particle.CellIndex;
        neighbors[5] = particle.CellIndex + rd;
        neighbors[6] = particle.CellIndex + ld + dd;
        neighbors[7] = particle.CellIndex + dd;
        neighbors[8] = particle.CellIndex + rd + dd;
      }

      // Iterate over these neighbors
      for (std::size_t neighbor = 0; neighbor < numNeighbors; neighbor++)
      {
        int32_t cellIndex = neighbors[neighbor];
        Cell &cell = cells[cellIndex];

        for (std::size_t j = 0; j < cell.Particles.size(); j++)
        {
          std::size_t otherID = cell.Particles[j];
          if (particle.ID == otherID)
            continue;
          Particle &other = particles[otherID];

          particle.NetForce += ForceFunction(particle, other, system, matrix) * timestep;
        }
      }
    }
  };

  // If we have less than 100 particles, the overhead isn't needed, and it's hard to distrubute particles anyways
  if (particles.size() == 0)
  {
    return;
  }
  else if (particles.size() < 100 || !m_Multithreaded)
  {
    jobFunc(0, particles.size() - 1, timestep);
  }
  else
  {
    // Spawn our job threads
    constexpr static std::size_t numWorkers = 16;
    std::size_t particlesPerWorker = particles.size() / numWorkers + 1; // integer division, add 1 (cover all)

    std::thread workers[numWorkers];
    for (std::size_t i = 0; i < numWorkers; i++)
    {
      std::size_t start = i * particlesPerWorker;
      std::size_t end = start + (particlesPerWorker - 1);
      end = (end >= particles.size()) ? particles.size() - 1 : end; // cap end at last particle.

      workers[i] = std::thread(jobFunc, start, end, timestep);
    }

    // Wait for our workers
    for (std::size_t i = 0; i < numWorkers; i++)
    {
      workers[i].join();
    }
  }
}

}