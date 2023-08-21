#pragma once

#include <glad/glad.h>

#include "Camera.h"

namespace Speck
{

class Renderer
{
public:
  Renderer(float width, float height);
  ~Renderer();

  void UpdateCamera();
  
  void BeginFrame();
  void DrawParticle(float x, float y);
  void EndFrame();

  void Resize(float width, float height);

private:
  void Flush();
  
  void GenerateBuffers();
  void GenerateShaders();

  void DestroyBuffers();
  void DestroyShaders();

private:
  GLuint m_VAO, m_VBO, m_IBO, m_Shader;
  
  GLuint m_InstancedVBO;
  glm::vec2* m_InstancedBuffer;
  std::size_t m_Particles = 0, m_MaxParticles = 10000;
  bool m_InFrame = false;
  
  Camera m_Camera;

  float m_Width, m_Height;
};

}
