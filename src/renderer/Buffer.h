#pragma once

#include <cstddef>
#include <glad/glad.h>

namespace Speck
{

struct BufferDesc
{
  GLenum Type;
  GLenum Usage;
  std::size_t Size;
  void* Data;
};

class Buffer
{
public:
  Buffer(const BufferDesc& desc);
  ~Buffer();

  void SetData(void* data, std::size_t size);

  void Bind();

private:
  GLuint m_Object;
  BufferDesc m_Desc;
};

}