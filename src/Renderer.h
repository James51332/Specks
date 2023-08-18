#pragma once

#include <glad/glad.h>

namespace Speck
{

class Renderer
{
public:
  Renderer(float width, float height);
  ~Renderer();

  void Render();

  void Resize(float width, float height);

private:
  GLuint m_VAO, m_VBO, m_Shader;

  float m_Width, m_Height;
};

}