#pragma once

#include <vector>
#include <glad/glad.h>

#include "Camera.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"

#include "simulation/Particle.h"
#include "simulation/ColorMatrix.h"

namespace Speck
{

class Renderer
{
public:
  Renderer(float width, float height, float displayScale = 1.0f);
  ~Renderer();
  
  void BeginFrame(Camera* camera, float boundingBoxSize);
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
  VertexArray *m_ParticleVAO, *m_BackgroundVAO;
  Buffer *m_QuadVBO, *m_QuadIBO;
  Shader *m_ParticleShader, *m_BackgroundShader;
  
  // Particle Instancing Data
  struct InstancedVertex
  {
    glm::vec2 Position;
    glm::vec4 Color;
  };
  
  Buffer* m_InstancedVBO;
  InstancedVertex* m_InstancedBuffer;
  std::size_t m_Particles = 0, m_MaxParticles = 10000;
  
  // General Rendering Data
  bool m_InFrame = false;
  Camera* m_Camera = nullptr;
  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}
