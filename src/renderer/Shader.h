#pragma once

#include <glad/glad.h>

namespace Speck
{

class Shader
{
public:
  Shader(const char* vertex, const char* fragment);
  ~Shader();

  void Use();

  void UploadUniformFloat(const float, const char* name);
  void UploadUniformFloat4(const float*, const char *name);
  void UploadUniformMat4(const float*, const char *name);

private:
  GLuint m_ShaderProgram;
};

}