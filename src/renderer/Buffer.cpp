#include "Buffer.h"

#include <SDL.h>

namespace Speck
{

Buffer::Buffer(const BufferDesc& desc)
  : m_Desc(desc)
{
  glGenBuffers(1, &m_Object);
  glBindBuffer(desc.Type, m_Object);
  glBufferData(desc.Type, desc.Size, desc.Data, desc.Usage);
}

Buffer::~Buffer()
{
  glDeleteBuffers(1, &m_Object);
}

void Buffer::SetData(void* data, std::size_t size)
{
  SDL_assert(size <= m_Desc.Size);

  glBindBuffer(m_Desc.Type, m_Object);
  glBufferSubData(m_Desc.Type, 0, size, data);
}

void Buffer::Resize(std::size_t size)
{
  if (size < m_Desc.Size) return; // Don't worry about shrinking

  m_Desc.Size = size;
  glBindBuffer(m_Desc.Type, m_Object);
  glBufferData(m_Desc.Type, m_Desc.Size, nullptr, m_Desc.Usage);
}

void Buffer::Bind()
{
  glBindBuffer(m_Desc.Type, m_Object);
}

}