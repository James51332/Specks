#include "Renderer.h"

#include <SDL3/SDL.h>
#include <iostream>

namespace Speck
{

const float triangle[] = {
  -0.5f, 0.5f, 0.0f,
  0.5f, 0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  -0.5f, -0.5f, 0.0f
};

const uint32_t indices[] = {
  0, 1, 2, 0, 2, 3
};

const char* vertexShader = R"(
#version 330 core

layout (location = 0) in vec3 aPos;

void main()
{
  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
})";

const char* fragmentShader = R"(
#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})";

Renderer::Renderer(float width, float height)
  : m_Width(width), m_Height(height)
{
  // Load OpenGL function pointers
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  // Init OpenGL objects
  GenerateBuffers();
  GenerateShaders();

  // Resize the viewport
  Resize(width, height);
}

Renderer::~Renderer()
{
  // Destroy rendering objects
  DestroyBuffers();
  DestroyShaders();
}

void Renderer::Render()
{
  glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(m_Shader);
  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
  glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, nullptr);
}

void Renderer::Resize(float width, float height)
{
  m_Width = width;
  m_Height = height;
  glViewport(0, 0, width, height);
}

void Renderer::GenerateBuffers()
{
  // Create our vertex buffer
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create our index buffer
  glGenBuffers(1, &m_IBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create our vertex array
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  // Attach our vbo to our vao
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Define the vertex layout
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
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
  glDeleteBuffers(1, &m_IBO);
  glDeleteVertexArrays(1, &m_VAO);
}

void Renderer::DestroyShaders()
{
  glDeleteProgram(m_Shader);
}

}