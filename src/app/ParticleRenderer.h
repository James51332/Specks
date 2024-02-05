#pragma once

#include <vector>
#include <glad/glad.h>

#include "renderer/Camera.h"
#include "renderer/Shader.h"
#include "renderer/Buffer.h"
#include "renderer/VertexArray.h"

#include "simulation/Particle.h"
#include "simulation/ColorMatrix.h"

namespace Speck
{

class ParticleRenderer
{
public:
  ParticleRenderer(float width, float height, float displayScale = 1.0f);
  ~ParticleRenderer();
  
  void BeginFrame(Vision::OrthoCamera* camera, float boundingBoxSize);
  void EndFrame();
  
  void DrawParticle(const Particle& particle, const ColorMatrix& matrix);
  void DrawParticles(const std::vector<Particle>& particles, const ColorMatrix& matrix);

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
  Vision::VertexArray *m_ParticleVAO, *m_BackgroundVAO;
  Vision::Buffer *m_QuadVBO, *m_QuadIBO;
  Vision::Shader *m_ParticleShader, *m_BackgroundShader;
  
  // Particle Instancing Data
  struct InstancedVertex
  {
    glm::vec2 Position;
    glm::vec4 Color;
  };
  
  Vision::Buffer* m_InstancedVBO;
  InstancedVertex* m_InstancedBuffer;
  std::size_t m_Particles = 0, m_MaxParticles = 10000;
  
  // General Rendering Data
  bool m_InFrame = false;
  Vision::OrthoCamera* m_Camera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}
