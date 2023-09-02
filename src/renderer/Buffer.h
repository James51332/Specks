#pragma once

#include <cstddef>
#include <glad/glad.h>

#include "BufferLayout.h"

namespace Speck
{

struct BufferDesc
{
  GLenum Type;
  GLenum Usage;
  std::size_t Size;
  void* Data;
  BufferLayout Layout; // Only needed for vertex buffers
};

class Buffer
{
public:
  Buffer(const BufferDesc& desc);
  ~Buffer();

  void SetData(void* data, std::size_t size);

  void Bind();

  const BufferLayout& GetLayout() const { return m_Desc.Layout; }

private:
  GLuint m_Object;
  BufferDesc m_Desc;
};

}