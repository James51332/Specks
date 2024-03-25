#pragma once

#include <glm/glm.hpp>

namespace Speck
{

struct Particle
{
  glm::vec2 Position = glm::vec2(0.0f);
  glm::vec2 LastPosition = glm::vec2(0.0f);
  glm::vec2 NetForce = glm::vec2(0.0f); // Calculated relative to the timestep.
  
  std::size_t Color = 0;
  
  std::size_t ID = 0; // Particles also cache their index
  std::size_t CellIndex = 0; // Particles cache their cells
};

/// A cell stores a list of indices of particle to allow for reduction of unneeded physics calculations.
struct Cell
{
  std::vector<std::size_t> Particles;
};

}
