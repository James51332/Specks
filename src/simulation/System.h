#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Particle.h"
#include "ColorMatrix.h"

namespace Speck
{

/// A cell stores a list of indices of particle to allow for reduction of unneeded physics calculations.
struct Cell
{
  std::vector<std::size_t> Particles;
};

/// A system keeps tracks of all of the particles in the scene.
class System
{
public:
  System(std::size_t numParticles = 10000, std::size_t numColors = 1, float size = 100.0f);
  
  void Update(const ColorMatrix& matrix, float timestep);
  
  const std::vector<Particle>& GetParticles() const { return m_Particles; }
  float GetBoundingBoxSize() const { return m_Size; }
  float GetInteractionRadius() const { return m_InteractionRadius; }

  float SetInteractionRadius(float radius = 40.0f) { m_InteractionRadius = radius; AllocateCells(); }

private:
  void AllocateCells();
  void PartitionsParticles();
  void CalculateForces(const ColorMatrix& matrix);
  void UpdatePositions(float timestep);
  void BoundPositions();
  
  glm::vec2 ForceFunction(const Particle& particle, const Particle& other, const ColorMatrix& matrix);
  
private:
  std::vector<Particle> m_Particles;
  
  // Cell system to reduce physics misses, the size of a cell is as close to the
  // interaction radius as possible, so we only check neighboring cells for physics.
  std::vector<Cell> m_Cells;
  float m_CellSize;
  std::size_t m_CellsAcross;

  // Constants the define the parameters of the simulation
  float m_FrictionStrength = 2.0f;
  float m_InteractionRadius = 40.0f;
  float m_RepulsionRadius = 0.3f;
  
  // Size of the bounding box at which point particles will wrap around.
  // Goes from -m_Size to m_Size on both x and y axes.
  float m_Size;
};

}
