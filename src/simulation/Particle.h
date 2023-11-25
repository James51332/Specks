#pragma once

#include <glm/glm.hpp>

namespace Speck
{

// TODO: Switch to full Verlet integration. 
// It hasn't shown useful to maintain velocity as opposed to previous position
struct Particle
{
  glm::vec2 Position = glm::vec2(0.0f);
  glm::vec2 LastPosition = glm::vec2(0.0f);
  glm::vec2 NetForce = glm::vec2(0.0f);
  
  std::size_t Color = 0;
  
  std::size_t ID = 0; // Particles also cache their index
  std::size_t CellIndex = 0; // Particles cache their cells
};

}
