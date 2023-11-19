#include "ColorMatrix.h"

#include <glm/gtc/random.hpp>

namespace Speck
{

ColorMatrix::ColorMatrix(int numColors)
  : m_Colors(numColors, glm::vec4(1.0f)), m_AttractionScales(numColors * numColors, 0.0f)
{
  // Set the seed to the time so that the generator doesn't produce the same results
  srand(time(0));
  
  // Generate random values for the color matrix
  for (std::size_t i = 0; i < numColors; i++)
  {
    for (std::size_t j = 0; j < numColors; j++)
    {
      m_AttractionScales[i * numColors + j] = glm::clamp(glm::gaussRand(0.0f, 0.5f), -1.0f, 1.0f);
    }
  }
}

void ColorMatrix::SetColor(std::size_t colorIndex, const glm::vec4& color)
{
  assert(colorIndex >= 0 && colorIndex < m_Colors.size());
  m_Colors[colorIndex] = color;
}

const glm::vec4& ColorMatrix::GetColor(std::size_t colorIndex) const 
{
  assert(colorIndex >= 0 && colorIndex < m_Colors.size());
  return m_Colors[colorIndex];
}

void ColorMatrix::SetAttractionScale(std::size_t primary, std::size_t other, float scale)
{
  std::size_t index = primary * m_Colors.size() + other;
  assert(index < m_Colors.size() * m_Colors.size());
  m_AttractionScales[index] = scale;
}

float ColorMatrix::GetAttractionScale(std::size_t primary, std::size_t other) const
{
  std::size_t index = primary * m_Colors.size() + other;
  assert(index < m_Colors.size() * m_Colors.size());
  return m_AttractionScales[index];
}

}
