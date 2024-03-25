#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Particle.h"
#include "ColorMatrix.h"

namespace Speck
{

/// A system keeps tracks of all of the particles in the scene.
class System
{
  friend class ForceApplicator;
public:
  System(std::size_t numParticles = 1000, std::size_t numColors = 1, float size = 100.0f);

  void UpdatePositions(float timestep);
  void WrapPositions();                        // wrap particles around the edge
  void ClampPositions(float dampening = 0.7f); // bounce particles off edge (w/ speed *= dampening)

  void ZeroForces(); // reset all forces acting on particles.

  std::vector<Particle>& GetParticles() { return m_Particles; }
  const std::vector<Particle>& GetParticles() const { return m_Particles; }
  void SetNumParticles(std::size_t numParticles = 1000, std::size_t numColors = 1) { AllocateParticles(numParticles, numColors); }
  
  float GetBoundingBoxSize() const { return m_Size; }
  void SetBoundingBoxSize(float size) 
  { 
    m_Size = size; 
    WrapPositions(); 
    AllocateCells();
  }

  // Particle Partitions
  void AllocateParticles(std::size_t numParticles, std::size_t numColors);
  void AllocateCells();
  void PartitionsParticles();

  std::size_t GetCellsAcross() const { return m_CellsAcross; }
  std::vector<Cell>& GetCells() { return m_Cells; }

  float GetInteractionRadius() const { return m_InteractionRadius; }
  void SetInteractionRadius(float radius = 40.0f) { m_InteractionRadius = radius; AllocateCells(); }
private:
  // Cell system to reduce physics misses, the size of a cell is as close to the
  // interaction radius as possible, so we only check neighboring cells for physics.
  std::vector<Cell> m_Cells;
  float m_CellSize;
  std::size_t m_CellsAcross;

  // Constants the define the parameters of the simulation
  float m_InteractionRadius = 40.0f;
  float m_FrictionStrength = 2.0f;

private:
  std::vector<Particle> m_Particles;

  // Size of the bounding box at which point particles will wrap around.
  // Goes from -m_Size to m_Size on both x and y axes.
  float m_Size;
};

}
