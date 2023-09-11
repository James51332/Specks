#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Speck
{

class ColorMatrix
{
public:
  ColorMatrix(int numColors = 0);

  std::size_t GetNumColors() const { return m_Colors.size(); }

  void SetColor(std::size_t colorIndex, const glm::vec4& color);
  const glm::vec4& GetColor(std::size_t colorIndex) const;

  void SetAttractionScale(std::size_t primary, std::size_t other, float scale);
  float GetAttractionScale(std::size_t primary, std::size_t other) const;

private:
  std::vector<glm::vec4> m_Colors;
  
  // Each attraction factor is accessed via index = primary * numColors + other
  std::vector<float> m_AttractionScales;
};

}