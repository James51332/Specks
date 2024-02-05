#include "ParticleRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Speck
{

struct QuadVertex
{
  glm::vec3 Position;
  glm::vec2 UV;
};

constexpr static QuadVertex quadVertices[] = {
  {{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}},
  {{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}},
  {{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
  {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}
};

constexpr static uint16_t quadIndices[] = {
  0, 1, 2, 0, 2, 3
};

const char* particleVertex = R"(
#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec2 a_InstancePosition;
layout (location = 3) in vec4 a_InstanceColor;

out vec2 v_UVNorm;
out vec4 v_Color;

uniform mat4 u_ViewProjection;

void main()
{  
  gl_Position = u_ViewProjection * vec4(a_Position.xy + a_InstancePosition, 0.0f, 1.0f);
  v_UVNorm = a_UV * 2.0f - 1.0f;
  v_Color = a_InstanceColor;
})";

const char* particleFragment = R"(
#version 410 core

in vec2 v_UVNorm;
in vec4 v_Color;

out vec4 FragColor;

void main()
{
  float dist = distance(vec2(0.0f), v_UVNorm);
  float radius = 1.0f;

	float delta = fwidth(dist);
	float alpha = smoothstep(radius + delta, radius - delta, dist);
	FragColor = vec4(v_Color.xyz, v_Color.w * alpha);
})";

const char* backgroundVertex = R"(
#version 410 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_UV;

out vec2 v_UV;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	v_UV = a_UV;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Pos, 1.0);
})";

const char* backgroundFragment = R"(
#version 410 core

in vec2 v_UV;

out vec4 FragColor;

void main()
{
	FragColor = vec4(0.1, 0.1, 0.1, 1.0);
})";

ParticleRenderer::ParticleRenderer(float width, float height, float displayScale)
  : m_Width(width), m_Height(height), m_PixelDensity(displayScale)
{
  // Init OpenGL objects
  GenerateBuffers();
  GenerateArrays();
  GenerateShaders();

  // Resize the viewport (no need to use Resize() because we've already done everything else it does)
  glViewport(0, 0, static_cast<GLsizei>(width * displayScale), static_cast<GLsizei>(height * displayScale));
  
  // Enable Blending and Depth Testing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

ParticleRenderer::~ParticleRenderer()
{
  // Destroy rendering objects
  DestroyBuffers();
  DestroyArrays();
  DestroyShaders();
}

void ParticleRenderer::BeginFrame(Vision::OrthoCamera* camera, float systemBoundSize)
{
  m_InFrame = true;
  m_Camera = camera;
  
  m_BackgroundShader->Use();
  m_BackgroundShader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");

  glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(systemBoundSize, systemBoundSize, 1.0f));
  m_BackgroundShader->UploadUniformMat4(&transform[0][0], "u_Transform");
  
  m_BackgroundVAO->Bind();
  m_QuadIBO->Bind();
  glDrawElements(GL_TRIANGLES, sizeof(quadIndices) / sizeof(quadIndices[0]), GL_UNSIGNED_SHORT, nullptr);
}

void ParticleRenderer::DrawParticle(const Particle& particle, const ColorMatrix& matrix)
{
  if (m_Particles == m_MaxParticles)
    Flush();
  
  m_InstancedBuffer[m_Particles] = { particle.Position, matrix.GetColor(particle.Color) };
  m_Particles++;
}

void ParticleRenderer::DrawParticles(const std::vector<Particle>& particles, const ColorMatrix& matrix)
{
  std::size_t num = particles.size();
  if (m_Particles + num >= m_MaxParticles)
    Flush();
  
  if (num <= m_MaxParticles)
  {
    for (std::size_t i = 0; i < num; i++)
      m_InstancedBuffer[m_Particles + i] = { particles[i].Position, matrix.GetColor(particles[i].Color) };
    m_Particles += num;
  } else
  {
    // The buffer will be flushed if we reach this path.
    std::size_t current = 0;
    while (current < num)
    {
      std::size_t left = num - current;
      std::size_t batchSize = m_MaxParticles <= left ? m_MaxParticles : left;

      for (std::size_t i = 0; i < batchSize; i++)
        m_InstancedBuffer[m_Particles + i] = { particles[i + current].Position, matrix.GetColor(particles[i + current].Color) };
     
      Flush();
      current += batchSize;
    }
  }
}

void ParticleRenderer::EndFrame()
{
  Flush();
  
  m_InFrame = false;
  m_Camera = nullptr;
}

void ParticleRenderer::Flush()
{
  // Upload the camera matrix and use the shader program
  m_ParticleShader->Use();
  m_ParticleShader->UploadUniformMat4(&m_Camera->GetViewProjectionMatrix()[0][0], "u_ViewProjection");
  
  // Copy the instanced buffers to the instanced vbos
  m_InstancedVBO->SetData(m_InstancedBuffer, sizeof(InstancedVertex) * m_Particles);
  
  // Display the instances
  m_ParticleVAO->Bind();
  m_QuadIBO->Bind();
  glDrawElementsInstanced(GL_TRIANGLES, sizeof(quadIndices) / sizeof(quadIndices[0]), GL_UNSIGNED_SHORT, nullptr, m_Particles);
  
  // Reset the particle count
  m_Particles = 0;
}

void ParticleRenderer::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  glViewport(0, 0, static_cast<GLsizei>(width * m_PixelDensity), static_cast<GLsizei>(height * m_PixelDensity));
}

void ParticleRenderer::GenerateBuffers()
{
  // Create our quad vertex buffer and index buffer
  {
    Vision::BufferDesc vboDesc;
    vboDesc.Type = GL_ARRAY_BUFFER;
    vboDesc.Usage = GL_STATIC_DRAW;
    vboDesc.Size = sizeof(quadVertices);
    vboDesc.Data = (void*)quadVertices;
    vboDesc.Layout = {
      {Vision::ShaderDataType::Float3, "a_Position"},
      {Vision::ShaderDataType::Float2, "a_UV"}
    };

    m_QuadVBO = new Vision::Buffer(vboDesc);

    Vision::BufferDesc iboDesc;
    iboDesc.Type = GL_ELEMENT_ARRAY_BUFFER;
    iboDesc.Usage = GL_STATIC_DRAW;
    iboDesc.Size = sizeof(quadIndices);
    iboDesc.Data = (void*)quadIndices;

    m_QuadIBO = new Vision::Buffer(iboDesc);
  }

  // Create our particle instance vertex buffers
  {
    Vision::BufferDesc instancedDesc;
    instancedDesc.Type = GL_ARRAY_BUFFER;
    instancedDesc.Usage = GL_DYNAMIC_DRAW;
    instancedDesc.Size = m_MaxParticles * sizeof(InstancedVertex);
    instancedDesc.Data = nullptr;
    instancedDesc.Layout = {
        {Vision::ShaderDataType::Float2, "a_InstancedPosition", false, 1},
        {Vision::ShaderDataType::Float4, "a_InstancedColor", false, 1}
    };

    m_InstancedVBO = new Vision::Buffer(instancedDesc);
  	
  	// Allocate the cpu buffer that we'll write to and copy from
    m_InstancedBuffer = new InstancedVertex[m_MaxParticles];
	}
}

void ParticleRenderer::GenerateArrays()
{
  // Create our particle vertex array and attach buffers
  m_ParticleVAO = new Vision::VertexArray();
  m_ParticleVAO->AttachBuffer(m_QuadVBO);
  m_ParticleVAO->AttachBuffer(m_InstancedVBO);
  
  // Create our background vertex array
  m_BackgroundVAO = new Vision::VertexArray();
  m_BackgroundVAO->AttachBuffer(m_QuadVBO);
}

void ParticleRenderer::GenerateShaders()
{
  m_ParticleShader = new Vision::Shader(particleVertex, particleFragment);
  m_BackgroundShader = new Vision::Shader(backgroundVertex, backgroundFragment);
}

void ParticleRenderer::DestroyBuffers()
{
  delete m_QuadVBO;
  delete m_QuadIBO;
  delete m_InstancedVBO;
  delete[] m_InstancedBuffer;
}

void ParticleRenderer::DestroyArrays()
{
  delete m_ParticleVAO;
  delete m_BackgroundVAO;
}

void ParticleRenderer::DestroyShaders()
{
  delete m_ParticleShader;
  delete m_BackgroundShader;
}

}
