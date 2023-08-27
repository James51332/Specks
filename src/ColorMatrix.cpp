#include "ColorMatrix.h"

#include <glm/gtc/random.hpp>

namespace Speck
{

ColorMatrix::ColorMatrix(int numColors)
  : m_Colors(numColors, glm::vec4(1.0f)), m_AttractionScales(25, 0.0f)
{
  for (std::size_t i = 0; i < numColors; i++)
  {
    for (std::size_t j = 0; j < numColors; j++)
    {
      float attraction = 0.0f;
      if (i == j) attraction = 1.0f;
      else if ((i + 1) % numColors == j) attraction = 0.2f;
      else attraction = 0.0f;

      m_AttractionScales[i * numColors + j] = attraction; // glm::gaussRand(0.0f, 1.0f);
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