#include "Renderer.h"

#include <SDL3/SDL.h>
#include <iostream>

namespace Speck
{

const float triangle[] = {
  0.0f, 0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  -0.5f, -0.5f, 0.0f
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
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  // Create our vertex buffer
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create our vertex array
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);
  
  // Attach our vbo to our vao
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Define the vertex layout
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Build the shader
  GLuint vs, fs;
  
  vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertexShader, nullptr);
  glCompileShader(vs);

  int success;
  char infoLog[512];
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vs, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragmentShader, nullptr);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fs, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  m_Shader = glCreateProgram();
  glAttachShader(m_Shader, vs);
  glAttachShader(m_Shader, fs);
  glLinkProgram(m_Shader);

  glGetProgramiv(m_Shader, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(m_Shader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  // Resize the viewport
  Resize(width, height);
}

Renderer::~Renderer()
{
}

void Renderer::Render()
{
  glClearColor(0.5f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(m_Shader);
  glBindVertexArray(m_VAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::Resize(float width, float height)
{
  glViewport(0, 0, width, height);
}

}