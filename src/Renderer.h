#pragma once

#include <vector>
#include <glad/glad.h>

#include "Camera.h"

namespace Speck
{

class Renderer
{
public:
  Renderer(float width, float height);
  ~Renderer();
  
  void BeginFrame(Camera* camera);
  void EndFrame();
  
  void DrawParticle(float x, float y);
  void DrawParticles(const std::vector<glm::vec2>& particles);

  void Resize(float width, float height);

private:
  void Flush();
  
  void GenerateBuffers();
  void GenerateShaders();

  void DestroyBuffers();
  void DestroyShaders();

private:
  // Particle Rendering Data
  GLuint m_VAO, m_VBO, m_IBO, m_Shader;
  GLuint m_InstancedVBO;
  glm::vec2* m_InstancedBuffer;
  std::size_t m_Particles = 0, m_MaxParticles = 10000;
  
  // General Rendering Data
  bool m_InFrame = false;
  Camera* m_Camera = nullptr;
  float m_Width, m_Height;
};

}
