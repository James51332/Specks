#include "Renderer.h"

#include <SDL.h>
#include <iostream>

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

out vec2 v_Pos;

uniform mat4 u_ViewProjection;

void main()
{  
  gl_Position = u_ViewProjection * vec4(a_Position.xy + a_InstancePosition, 0.0f, 1.0f);
  v_Pos = a_Position.xy;
})";

const char* fragmentShader = R"(
#version 410 core

in vec2 v_Pos;

out vec4 FragColor;

void main()
{
  float distance2 = dot(v_Pos, v_Pos);
  float radius2 = 1.0f; // (r = 1)

  if (distance2 < radius2)
  {
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  }
  else
  {
    FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  }
})";

Renderer::Renderer(float width, float height)
  : m_Width(width), m_Height(height), m_Camera(width, height, 25.0f)
{
  // Load OpenGL function pointers
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  // Init OpenGL objects
  GenerateBuffers();
  GenerateShaders();

  // Resize the viewport (no need to use Resize() because we've already done everything else it does)
  glViewport(0, 0, width, height);
  
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

void Renderer::UpdateCamera()
{
  // Update the camera's position and rotation based on user input
  m_Camera.Update();
}

void Renderer::BeginFrame()
{
  m_InFrame = true;
  m_Particles = 0;
}

void Renderer::DrawParticle(float x, float y)
{
  if (m_Particles == m_MaxParticles)
    Flush();
  
  m_InstancedBuffer[m_Particles] = glm::vec2(x, y);
  m_Particles++;
}

void Renderer::EndFrame()
{
  Flush();
  m_InFrame = false;
}

void Renderer::Flush()
{
  // Upload the camera matrix and use the shader program
  GLint uniform = glGetUniformLocation(m_Shader, "u_ViewProjection");
  glUniformMatrix4fv(uniform, 1, GL_FALSE, &m_Camera.GetViewProjectionMatrix()[0][0]);
  glUseProgram(m_Shader);
  
  // Copy the instanced buffer to the instanced vbo
  glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * m_Particles, m_InstancedBuffer);
  
  // Display the instances
  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
  glDrawElementsInstanced(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, nullptr, m_Particles);
  
  // Reset the particle count
  m_Particles = 0;
}

void Renderer::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;

  m_Camera.SetWindowSize(m_Width, m_Height);

  glViewport(0, 0, width, height);
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
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  // Attach our vbo to our vao and define the vertex layout
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  // Attach our instanced buffer and define the layout and increment
  glBindBuffer(GL_ARRAY_BUFFER, m_InstancedVBO);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1); // step to the next buffer for each vertex
}

void Renderer::GenerateShaders()
{
  // Build the vertex and fragment shaders
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

void Renderer::DestroyBuffers()
{
  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_InstancedVBO);
  delete[] m_InstancedBuffer;
  glDeleteBuffers(1, &m_IBO);
  glDeleteVertexArrays(1, &m_VAO);
}

void Renderer::DestroyShaders()
{
  glDeleteProgram(m_Shader);
}

}
