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
  System(std::size_t numParticles = 1000, std::size_t numColors = 1, float size = 100.0f);
  
  void Update(const ColorMatrix& matrix, float timestep);
  
  const std::vector<Particle>& GetParticles() const { return m_Particles; }
  void SetNumParticles(std::size_t numParticles = 1000, std::size_t numColors = 1) { AllocateParticles(numParticles, numColors); }
  
  float GetBoundingBoxSize() const { return m_Size; }
  void SetBoundingBoxSize(float size) 
  { 
    m_Size = size; 
    BoundPositions(); 
    AllocateCells();
  }

  float GetInteractionRadius() const { return m_InteractionRadius; }
  void SetInteractionRadius(float radius = 40.0f) { m_InteractionRadius = radius; AllocateCells(); }
  
  bool IsMultiThreaded() const { return m_ThreadedDispatch; }
  void SetMultiThreaded(bool threaded = true) { m_ThreadedDispatch = threaded; }

private:
  void AllocateParticles(std::size_t numParticles, std::size_t numColors);
  void AllocateCells();
  void PartitionsParticles();
  void CalculateForces(const ColorMatrix& matrix, float timestep);
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

  // ----- Engine Details -----
  bool m_ThreadedDispatch = true;
};

}
