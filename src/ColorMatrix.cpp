#include "ColorMatrix.h"

namespace Speck
{

ColorMatrix::ColorMatrix(int numColors)
  : m_Colors(numColors, glm::vec4(1.0f))
{
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
}

}