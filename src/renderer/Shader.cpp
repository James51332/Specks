#include "Shader.h"

#include <iostream>

namespace Speck
{

Shader::Shader(const char* vertex, const char* fragment)
{
  // Create two shaders with our given shader source code
  GLuint vs, fs;
  {
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex, nullptr);
    glCompileShader(vs);

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment, nullptr);
    glCompileShader(fs);
  }

  // Check for compilation success of these two shader
  constexpr static std::size_t bufferSize = 512;
  int success;
  char infoLog[bufferSize];
  {
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(vs, bufferSize, NULL, infoLog);
      std::cout << "Failed to compile vertex shader:" << std::endl;
      std::cout << infoLog << std::endl;
    }

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(fs, bufferSize, NULL, infoLog);
      std::cout << "Failed to compile fragment shader:" << std::endl;
      std::cout << infoLog << std::endl;
    }
  }

  // Combine the shaders into a program and link it
  m_ShaderProgram = glCreateProgram();
  {
    glAttachShader(m_ShaderProgram, vs);
    glAttachShader(m_ShaderProgram, fs);
    glLinkProgram(m_ShaderProgram);

    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(m_ShaderProgram, 512, NULL, infoLog);
      std::cout << "Failed to link shader program:" << std::endl;
      std::cout << infoLog << std::endl;
    }
  }

  // Delete our shaders now that we have linked
  {
    glDeleteShader(vs);
    glDeleteShader(fs);
  }
}

Shader::~Shader()
{
  glDeleteProgram(m_ShaderProgram);
}

void Shader::Use()
{
  glUseProgram(m_ShaderProgram);
}

void Shader::UploadUniformFloat(const float value, const char* name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniform1f(location, value);
}

void Shader::UploadUniformFloat4(const float* value, const char *name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniform4fv(location, 1, value);
}

void Shader::UploadUniformMat4(const float* value, const char *name)
{
  GLint location = glGetUniformLocation(m_ShaderProgram, name);
  glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

}