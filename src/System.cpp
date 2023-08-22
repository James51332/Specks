#include "System.h"

#include <cmath>

namespace Speck
{

System::System(std::size_t numParticles)
{
  m_Particles.reserve(numParticles);
  std::size_t rowSize = 20;
  for (std::size_t i = 0; i < numParticles; i++)
  {
    std::size_t x = i % rowSize;
    std::size_t y = (i - x) / rowSize;
    m_Particles.push_back({ 3.0f * x, 3.0f * y });
  }
}

void System::Update()
{
  // TODO: Particle Interactions
}

}
