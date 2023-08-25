#pragma once

#include <vector>
#include <glad/glad.h>

#include "Camera.h"

namespace Speck
{

class Renderer
{
public:
  Renderer(float width, float height, float displayScale = 1.0f);
  ~Renderer();
  
  void BeginFrame(Camera* camera, float systemBoundsSize);
  void EndFrame();
  
  void DrawParticle(float x, float y);
  void DrawParticles(const std::vector<glm::vec2>& particles);

  void Resize(float width, float height);

private:
  void Flush();
  
  void GenerateBuffers();
  void GenerateArrays();
  void GenerateShaders();

  void DestroyBuffers();
  void DestroyArrays();
  void DestroyShaders();

private:
  // General Rendering Data
  GLuint m_QuadVBO, m_QuadIBO;
  GLuint m_ParticleVAO, m_BackgroundVAO;
  GLuint m_ParticleShader, m_BackgroundShader;
  
  // Particle Instacning Data
  GLuint m_InstancedVBO;
  glm::vec2* m_InstancedBuffer;
  std::size_t m_Particles = 0, m_MaxParticles = 10000;
  
  // General Rendering Data
  bool m_InFrame = false;
  Camera* m_Camera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}
