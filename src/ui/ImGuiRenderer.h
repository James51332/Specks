#include <imgui.h>

#include <glad/glad.h>

#include "renderer/Buffer.h"
#include "renderer/Shader.h"
#include "renderer/VertexArray.h"

namespace Speck
{

class ImGuiRenderer
{
public:
  ImGuiRenderer(float width, float height, float displayScale = 1.0f);
  ~ImGuiRenderer();

  void Begin();
  void End();

  void Resize(float width, float height);

private:
  void GenerateBuffers();
  void GenerateArrays();
  void GenerateShaders();
  void GenerateTextures();

  void DestroyBuffers();
  void DestroyArrays();
  void DestroyShaders();
  void DestroyTextures();

private:
  Buffer *m_VBO, *m_IBO;
  VertexArray *m_VertexArray;
  Shader *m_Shader;
  GLuint m_FontTexture;

  std::size_t m_MaxVertices = 1000000; // 5MB GPU buffer
  std::size_t m_MaxIndices = 1000000;

  float m_PixelDensity = 1.0f;
  float m_Width, m_Height;
};

}