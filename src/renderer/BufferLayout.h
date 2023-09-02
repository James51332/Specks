#pragma once

#include <string>
#include <vector>
#include <initializer_list>

#include <glad/glad.h>

namespace Speck 
{

// We can add more to this in the future
enum class ShaderDataType
{
  Float,
  Float2,
  Float3,
  Float4,
};

inline float ShaderDataTypeSize(ShaderDataType type)
{
  switch (type)
  {
    case ShaderDataType::Float:
      return 4;
    case ShaderDataType::Float2:
      return 4 * 2;
    case ShaderDataType::Float3:
      return 4 * 3;
    case ShaderDataType::Float4:
      return 4 * 4;
    default:
      return 0;
  }
}

inline GLint ShaderDataTypeCount(ShaderDataType type)
{
  switch (type)
  {
    case ShaderDataType::Float:
      return 1;
    case ShaderDataType::Float2:
      return 2;
    case ShaderDataType::Float3:
      return 3;
    case ShaderDataType::Float4:
      return 4;
    default:
      return 0;
  }
}

struct BufferElement
{
  ShaderDataType Type;
  std::string Name;
  bool Normalized;

  // If non zero, vertex data steps after each n instances
  std::size_t InstanceDivisor;

  std::size_t Offset;
  std::size_t Size;

  BufferElement(ShaderDataType type, const std::string &name, bool normalized = false, std::size_t instanceDivisor = 0)
      : Type(type), Name(name), Normalized(normalized), Offset(0), Size(ShaderDataTypeSize(type)), InstanceDivisor(instanceDivisor) {}
};

struct BufferLayout
{
  std::vector<BufferElement> Elements;
  std::size_t Stride = 0;

  void CalculateOffsetsAndStride()
  {
    std::size_t offset = 0;
    for (auto &element : Elements)
    {
      element.Offset = offset;
      offset += element.Size;

      Stride += element.Size;
    }
  }

  BufferLayout(std::initializer_list<BufferElement> elements)
      : Elements(elements)
  {
    CalculateOffsetsAndStride();
  }

  BufferLayout() = default;
};

}