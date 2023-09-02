#include "VertexArray.h"

namespace Speck
{

VertexArray::VertexArray()
{
  glGenVertexArrays(1, &m_Object);
}

VertexArray::~VertexArray()
{
  glDeleteVertexArrays(1, &m_Object);
}

void VertexArray::Bind()
{
  glBindVertexArray(m_Object);
}

void VertexArray::AttachBuffer(Buffer* buffer)
{
  const BufferLayout& layout = buffer->GetLayout();
  
  glBindVertexArray(m_Object);
  buffer->Bind();

  for (auto& element : layout.Elements)
  {
    glVertexAttribPointer(m_CurrentAttrib, 
                          ShaderDataTypeCount(element.Type), 
                          GL_FLOAT, 
                          element.Normalized, 
                          layout.Stride, 
                          (void*)element.Offset);
    glVertexAttribDivisor(m_CurrentAttrib, element.InstanceDivisor);
    glEnableVertexAttribArray(m_CurrentAttrib);

    m_CurrentAttrib++;
  }
}

}

