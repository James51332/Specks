#include "Renderer.h"

#include <SDL.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "Input.h"

namespace Speck
{

struct QuadVertex
{
  glm::vec3 Position;
  glm::vec2 UV;
};

struct InstanceVertex
{
  glm::vec2 Position;
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

out vec2 v_UVNorm;

uniform mat4 u_ViewProjection;

void main()
{  
  gl_Position = u_ViewProjection * vec4(a_Position.xy + a_InstancePosition, 0.0f, 1.0f);
  v_UVNorm = a_UV * 2.0f - 1.0f;
})";

const char* particleFragment = R"(
#version 410 core

in vec2 v_UVNorm;

out vec4 FragColor;

void main()
{
  float dist = distance(vec2(0.0f), v_UVNorm);
  float radius = 1.0f;

	float col = 1.0f;
	float delta = fwidth(dist);
	float alpha = smoothstep(radius + delta, radius - delta, dist);
	FragColor = vec4(col, col, col, alpha);
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

Renderer::Renderer(float width, float height, float displayScale)
  : m_Width(width), m_Height(height), m_PixelDensity(displayScale)
{
  // Load OpenGL function pointers
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  // Init OpenGL objects
  GenerateBuffers();
  GenerateArrays();
  GenerateShaders();

  // Resize the viewport (no need to use Resize() because we've already done everything else it does)
  glViewport(0, 0, width * displayScale, height * displayScale);
  
  // Enable Blending and Depth Testing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer::~Renderer()
{
  // Destroy rendering objects
  DestroyBuffers();
  DestroyArrays();
  DestroyShaders();
}

void Renderer::BeginFrame(Camera* camera)
{
  m_InFrame = true;
  m_Camera = camera;
  
  glUseProgram(m_BackgroundShader);
  GLuint uniform = glGetUniformLocation(m_BackgroundShader, "u_ViewProjection");
  glUniformMatrix4fv(uniform, 1, GL_FALSE, &m_Camera->GetViewProjectionMatrix()[0][0]);
  
  uniform = glGetUniformLocation(m_BackgroundShader, "u_Transform");
  glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f));
  glUniformMatrix4fv(uniform, 1, GL_FALSE, &transform[0][0]);
  
  glBindVertexArray(m_BackgroundVAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIBO);
  glDrawElements(GL_TRIANGLES, sizeof(quadIndices) / sizeof(quadIndices[0]), GL_UNSIGNED_SHORT, nullptr);
}

void Renderer::DrawParticle(float x, float y)
{
  if (m_Particles == m_MaxParticles)
    Flush();
  
  m_InstancedBuffer[m_Particles] = glm::vec2(x, y);
  m_Particles++;
}

void Renderer::DrawParticles(const std::vector<glm::vec2>& particles)
{
  std::size_t num = particles.size();
  if (m_Particles + num >= m_MaxParticles)
    Flush();
  
  if (num <= m_MaxParticles)
  {
    std::memcpy(m_InstancedBuffer + m_Particles, particles.data(), sizeof(glm::vec2) * num);
    m_Particles += num;
  } else
  {
    // The buffer will be flushed if we reach this path.
    std::size_t batchSize = 10000; // Fit as many as we can in the batch
    std::memcpy(m_InstancedBuffer , particles.data(), sizeof(glm::vec2) * 10000);
    num -= batchSize;
    Flush();
  }
}

void Renderer::EndFrame()
{
  Flush();
  
  m_InFrame = false;
  m_Camera = nullptr;
}

void Renderer::Flush()
{
  // Upload the camera matrix and use the shader program
  glUseProgram(m_ParticleShader);
  GLint uniform = glGetUniformLocation(m_ParticleShader, "u_ViewProjection");
  glUniformMatrix4fv(uniform, 1, GL_FALSE, &m_Camera->GetViewProjectionMatrix()[0][0]);
  
  // Copy the instanced buffer to the instanced vbo
  glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * m_Particles, m_InstancedBuffer);
  
  // Display the instances
  glBindVertexArray(m_ParticleVAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIBO);
  glDrawElementsInstanced(GL_TRIANGLES, sizeof(quadIndices) / sizeof(quadIndices[0]), GL_UNSIGNED_SHORT, nullptr, m_Particles);
  
  // Reset the particle count
  m_Particles = 0;
}

void Renderer::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  glViewport(0, 0, width * m_PixelDensity, height * m_PixelDensity);
}

void Renderer::GenerateBuffers()
{
  // Create our quad vertex buffer and index buffer
  {
  	glGenBuffers(1, &m_QuadVBO);
  	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
  	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  	glBindBuffer(GL_ARRAY_BUFFER, 0);

  	glGenBuffers(1, &m_QuadIBO);
  	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIBO);
  	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
  	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  // Create our particle instance vertex buffer
  {
  	glGenBuffers(1, &m_InstancedVBO);
  	glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
  	glBufferData(GL_ARRAY_BUFFER, m_MaxParticles * sizeof(InstanceVertex), nullptr, GL_DYNAMIC_DRAW);
  	glBindBuffer(GL_ARRAY_BUFFER, 0);
  	
  	// Allocate the cpu buffer that we'll write to and copy from
  	m_InstancedBuffer = new glm::vec2[m_MaxParticles];
	}
}

void Renderer::GenerateArrays()
{
  // Create our particle vertex array
  {
    glGenVertexArrays(1, &m_ParticleVAO);
    glBindVertexArray(m_ParticleVAO);
    
    // Attach our vbo to our vao and define the vertex layout
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);
    
    // Attach our instanced buffer and define the layout and increment
    glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceVertex), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    
    // Unbind our vao
    glBindVertexArray(0);
  }
  
  // Create our background vertex array
  {
    glGenVertexArrays(1, &m_BackgroundVAO);
    glBindVertexArray(m_BackgroundVAO);
    
    // Attach our vbo to vao and define the layout
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);
    
    // Unbind
    glBindVertexArray(0);
  }
}

void Renderer::GenerateShaders()
{
  // Build the particle shader program
  {
    GLuint vs, fs;
  	vs = glCreateShader(GL_VERTEX_SHADER);
  	glShaderSource(vs, 1, &particleVertex, nullptr);
  	glCompileShader(vs);

  	fs = glCreateShader(GL_FRAGMENT_SHADER);
  	glShaderSource(fs, 1, &particleFragment, nullptr);
  	glCompileShader(fs);

  	// Check for success
  	int success;
  	char infoLog[512];
  	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  	if (!success)
  	{
  	  glGetShaderInfoLog(vs, 512, NULL, infoLog);
  	  std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  	}

  	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  	if (!success)
  	{
  	  glGetShaderInfoLog(fs, 512, NULL, infoLog);
  	  std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  	}

  	// Combine the shaders into a program and link it
  	m_ParticleShader = glCreateProgram();
  	glAttachShader(m_ParticleShader, vs);
  	glAttachShader(m_ParticleShader, fs);
  	glLinkProgram(m_ParticleShader);

  	// Check for success
  	glGetProgramiv(m_ParticleShader, GL_LINK_STATUS, &success);
  	if (!success) {
  	  glGetProgramInfoLog(m_ParticleShader, 512, NULL, infoLog);
  	  std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  	}

  	// Destroy the shaders we no longer need
  	glDeleteShader(vs);
  	glDeleteShader(fs);
  }
  
  // Now create our background shader
  {
    GLuint vs, fs;
    
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &backgroundVertex, nullptr);
    glCompileShader(vs);
    
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &backgroundFragment, nullptr);
    glCompileShader(fs);
    
    // Check for success
    int success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(vs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(fs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Combine the shaders into a program and link it
    m_BackgroundShader = glCreateProgram();
    glAttachShader(m_BackgroundShader, vs);
    glAttachShader(m_BackgroundShader, fs);
    glLinkProgram(m_BackgroundShader);
    
    // Check for success
    glGetProgramiv(m_BackgroundShader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(m_BackgroundShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // Destroy the shaders we no longer need
    glDeleteShader(vs);
    glDeleteShader(fs);
  }
}

void Renderer::DestroyBuffers()
{
  glDeleteBuffers(1, &m_QuadVBO);
  glDeleteBuffers(1, &m_QuadIBO);
  
  glDeleteBuffers(1, &m_InstancedVBO);
  delete[] m_InstancedBuffer;
}

void Renderer::DestroyArrays()
{
  glDeleteVertexArrays(1, &m_ParticleVAO);
  glDeleteVertexArrays(1, &m_BackgroundVAO);
}

void Renderer::DestroyShaders()
{
  glDeleteProgram(m_ParticleShader);
  glDeleteProgram(m_BackgroundShader);
}

}
