#include "Renderer.h"

#include <SDL3/SDL.h>
#include <iostream>

namespace Speck
{

const float quadVertices[] = {
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

out vec3 vPos;

uniform float uAspect;

void main()
{
  vPos = aPos;
  gl_Position = vec4(aPos.x / uAspect, aPos.y, aPos.z, 1.0);
})";

const char* fragmentShader = R"(
#version 330 core

in vec3 vPos;

out vec4 FragColor;

void main()
{
    vec3 pos = vPos;
    float distance = sqrt(dot(pos, pos));
    if (distance < 0.1)
    {
      FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    } else
    {
      FragColor = vec4(0.5f, 0.0f, 0.0f, 0.0f);
    }
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
  // Clear the screen
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Use the shader
  GLuint uniform = glGetUniformLocation(m_Shader, "uAspect");
  glUniform1f(uniform, (m_Width / m_Height));
  glUseProgram(m_Shader);

  // Display the quad
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Create our index buffer
  glGenBuffers(1, &m_IBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  //// Create our instanced buffer
  //constexpr std::size_t maxInstances = 10000;
  //constexpr std::size_t bytesPerInstance = 8; // for now, we'll just have an x and y pos
  //glGenBuffers(1, &m_InstancedBuffer);
  //glBindBuffer(GL_ARRAY_BUFFER, m_InstancedBuffer);
  //glBufferData(GL_ARRAY_BUFFER, maxInstances * bytesPerInstance, nullptr, GL_DYNAMIC_DRAW);

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