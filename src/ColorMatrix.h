#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Speck
{

class ColorMatrix
{
public:
  ColorMatrix(int numColors);

  std::size_t GetNumColors() const { return m_Colors.size(); }

  void SetColor(std::size_t colorIndex, const glm::vec4& color);
  const glm::vec4& GetColor(std::size_t colorIndex) const;

  // TODO: Implement the matrix
  void SetAttractionScale(std::size_t primary, std::size_t other, float scale);
  float GetAttractionScale(std::size_t primary, std::size_t other) const { return primary == other ? 1.0f : -1.0f; }

private:
  std::vector<glm::vec4> m_Colors;
};

}