#include "Renderer.h"

#include <SDL.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "Input.h"

namespace Speck
{

constexpr static float quadVertices[] = {
  -1.0f, 1.0f, 0.0f,
  1.0f, 1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  -1.0f, -1.0f, 0.0f
};

constexpr static uint16_t indices[] = {
  0, 1, 2, 0, 2, 3
};

const char* vertexShader = R"(
#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_InstancePosition;

out vec2 v_UV;

uniform mat4 u_ViewProjection;

void main()
{  
  gl_Position = u_ViewProjection * vec4(a_Position.xy + a_InstancePosition, 0.0f, 1.0f);
  v_UV = a_Position.xy;
})";

const char* fragmentShader = R"(
#version 410 core

in vec2 v_UV;

out vec4 FragColor;

void main()
{
  float dist = distance(vec2(0.0f), v_UV);
  float radius = 1.0f;

	float col = step(dist, radius);
	float delta = fwidth(dist);
	float alpha = smoothstep(radius + delta, radius - delta, dist);
	FragColor = vec4(col, col, col, alpha);
})";

struct QuadVertex
{
  glm::vec3 Position;
  glm::vec2 UV;
};

QuadVertex backgroundVertices[] = {
  {{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}},
  {{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}},
  {{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
  {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}
};

const char* quadVertexShader = R"(
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

const char* quadFragmentShader = R"(
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
  DestroyShaders();
}

void Renderer::BeginFrame(Camera* camera)
{
  m_InFrame = true;
  m_Camera = camera;
  
  glUseProgram(m_QuadShader);
  
  GLuint uniform = glGetUniformLocation(m_QuadShader, "u_ViewProjection");
  glUniformMatrix4fv(uniform, 1, GL_FALSE, &m_Camera->GetViewProjectionMatrix()[0][0]);
  
  uniform = glGetUniformLocation(m_QuadShader, "u_Transform");
  glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f));
  glUniformMatrix4fv(uniform, 1, GL_FALSE, &transform[0][0]);
  
  glBindVertexArray(m_QuadVAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIBO);
  glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, nullptr);
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
  glUseProgram(m_Shader);
  GLint uniform = glGetUniformLocation(m_Shader, "u_ViewProjection");
  glUniformMatrix4fv(uniform, 1, GL_FALSE, &m_Camera->GetViewProjectionMatrix()[0][0]);
  
  // Copy the instanced buffer to the instanced vbo
  glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * m_Particles, m_InstancedBuffer);
  
  // Display the instances
  glBindVertexArray(m_ParticleVAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
  glDrawElementsInstanced(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, nullptr, m_Particles);
  
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
  // Create our vertex buffer
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create our index buffer
  glGenBuffers(1, &m_IBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create our instanced vbo
  constexpr std::size_t bytesPerInstance = sizeof(glm::vec2); // for now, we'll just have an x and y pos per instance
  glGenBuffers(1, &m_InstancedVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
  glBufferData(GL_ARRAY_BUFFER, m_MaxParticles * bytesPerInstance, nullptr, GL_DYNAMIC_DRAW);
  
  // Allocate the buffer we write to
  m_InstancedBuffer = new glm::vec2[m_MaxParticles];

  // Create our vertex array
  glGenVertexArrays(1, &m_ParticleVAO);
  glBindVertexArray(m_ParticleVAO);

  // Attach our vbo to our vao and define the vertex layout
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  // Attach our instanced buffer and define the layout and increment
  glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1); // step to the next buffer for each vertex
  
  // Unbind our vao
  glBindVertexArray(0);
  
  // Create our background vertex buffer and index buffer
  glGenBuffers(1, &m_QuadVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(backgroundVertices), backgroundVertices, GL_STATIC_DRAW);
  
  glGenBuffers(1, &m_QuadIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  
  // Create our background vao
  glGenVertexArrays(1, &m_QuadVAO);
  glBindVertexArray(m_QuadVAO);
  
  // Attach our vbo to vao and define the layout
  glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)0);
  glEnableVertexAttribArray(0);
  
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(1);
}

void Renderer::GenerateShaders()
{
  // Build the vertex and fragment shaders for particles
  {
  	GLuint vs, fs;

  	vs = glCreateShader(GL_VERTEX_SHADER);
  	glShaderSource(vs, 1, &vertexShader, nullptr);
  	glCompileShader(vs);

  	fs = glCreateShader(GL_FRAGMENT_SHADER);
  	glShaderSource(fs, 1, &fragmentShader, nullptr);
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
  	m_Shader = glCreateProgram();
  	glAttachShader(m_Shader, vs);
  	glAttachShader(m_Shader, fs);
  	glLinkProgram(m_Shader);

  	// Check for success
  	glGetProgramiv(m_Shader, GL_LINK_STATUS, &success);
  	if (!success) {
  	  glGetProgramInfoLog(m_Shader, 512, NULL, infoLog);
  	  std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  	}

  	// Destroy the shaders we no longer need
  	glDeleteShader(vs);
  	glDeleteShader(fs);
  }
  
  // Now create our quad shader
  {
    GLuint vs, fs;
    
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &quadVertexShader, nullptr);
    glCompileShader(vs);
    
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &quadFragmentShader, nullptr);
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
    m_QuadShader = glCreateProgram();
    glAttachShader(m_QuadShader, vs);
    glAttachShader(m_QuadShader, fs);
    glLinkProgram(m_QuadShader);
    
    // Check for success
    glGetProgramiv(m_QuadShader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(m_QuadShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // Destroy the shaders we no longer need
    glDeleteShader(vs);
    glDeleteShader(fs);
  }
}

void Renderer::DestroyBuffers()
{
  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_InstancedVBO);
  delete[] m_InstancedBuffer;
  glDeleteBuffers(1, &m_IBO);
  glDeleteVertexArrays(1, &m_ParticleVAO);
}

void Renderer::DestroyShaders()
{
  glDeleteProgram(m_Shader);
}

}
