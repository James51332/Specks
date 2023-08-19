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
  void GenerateBuffers();
  void GenerateShaders();

  void DestroyBuffers();
  void DestroyShaders();

private:
  GLuint m_VAO, m_VBO, m_IBO, m_Shader;
  //GLuint m_InstancedBuffer;

  float m_Width, m_Height;
};

}