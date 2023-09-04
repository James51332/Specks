#include "ImGuiRenderer.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Speck
{

ImGuiRenderer::ImGuiRenderer(float width, float height, float displayScale)
  : m_Width(width), m_Height(height), m_PixelDensity(displayScale)
{
  // Initialize ImGui
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.BackendRendererName = "Specks Renderer";
  io.BackendPlatformName = "Specks Engine";

  io.DisplaySize = { width, height };
  io.DisplayFramebufferScale = { displayScale, displayScale };

  // Create our rendering resources
  GenerateBuffers();
  GenerateArrays();
  GenerateShaders();
  GenerateTextures();
}

ImGuiRenderer::~ImGuiRenderer()
{
  // Shutdown ImGui
  ImGui::DestroyContext();

  // Destroy our rendering resources 
  DestroyBuffers();
  DestroyArrays();
  DestroyShaders();
  DestroyTextures();
}

void ImGuiRenderer::Begin()
{
  ImGui::NewFrame();
}

void ImGuiRenderer::End()
{
  ImGui::Render();
  ImDrawData *drawData = ImGui::GetDrawData();

  // Don't render if the window is minimized
  int32_t fbWidth = static_cast<int32_t>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
  int32_t fbHeight = static_cast<int32_t>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
  if (fbWidth <= 0 || fbHeight <= 0)
    return;

  // We can bind our shader here because it's the same for the entire program
  m_Shader->Use();

  // Setup our projection matrix
  float L = drawData->DisplayPos.x;
  float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
  float T = drawData->DisplayPos.y;
  float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
  glm::mat4 projection = glm::ortho(L, R, B, T);
  m_Shader->UploadUniformMat4(&projection[0][0], "u_ViewProjection");

  // Calculate our clip rect offset and scale
  glm::vec2 clipOff = { drawData->DisplayPos.x, drawData->DisplayPos.y };
  glm::vec2 clipScale = { drawData->FramebufferScale.x, drawData->FramebufferScale.y };
  glEnable(GL_SCISSOR_TEST);

  for (std::size_t i = 0; i < drawData->CmdListsCount; i++)
  {
    const ImDrawList* cmdList = drawData->CmdLists[i];

    // Ensure our buffers are big enough for the draw data
    if (cmdList->VtxBuffer.Size > m_MaxVertices)
    {
      m_MaxVertices *= 2;
      m_VBO->Resize(m_MaxVertices * sizeof(ImDrawVert));
    }

    if (cmdList->IdxBuffer.Size > m_MaxIndices)
    {
      m_MaxIndices *= 2;
      m_IBO->Resize(m_MaxIndices * sizeof(ImDrawIdx));
    }

    // Copy the draw data into our buffers
    m_VBO->SetData(cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
    m_IBO->SetData(cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

    // Bind our vertex array and buffers
    m_VertexArray->Bind();
    m_IBO->Bind();

    // Iterate over each command in the list
    for (std::size_t j = 0; j < cmdList->CmdBuffer.Size; j++)
    {
      const ImDrawCmd* command = &cmdList->CmdBuffer[j];

      // Calculate our scissor rect
      ImVec2 clipMin((command->ClipRect.x - clipOff.x) * clipScale.x, (command->ClipRect.y - clipOff.y) * clipScale.y);
      ImVec2 clipMax((command->ClipRect.z - clipOff.x) * clipScale.x, (command->ClipRect.w - clipOff.y) * clipScale.y);

      // Clamp to viewport as setScissorRect() won't accept values that are off bounds
      if (clipMin.x < 0.0f) { clipMin.x = 0.0f; }
      if (clipMin.y < 0.0f) { clipMin.y = 0.0f; }
      if (clipMax.x > fbWidth) { clipMax.x = (float)fbWidth; }
      if (clipMax.y > fbHeight) { clipMax.y = (float)fbHeight; }
      if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) continue;
      if (command->ElemCount == 0) continue;

      glScissor(clipMin.x, fbHeight - clipMax.y, (clipMax.x - clipMin.x), (clipMax.y - clipMin.y));

      if (command->UserCallback)
      {
        command->UserCallback(cmdList, command);
      } 
      else
      {
        glActiveTexture(GL_TEXTURE0);
        if (command->TextureId == 0)
          glBindTexture(GL_TEXTURE0, m_FontTexture);
        else
          glBindTexture(GL_TEXTURE0, (GLuint)(intptr_t)command->TextureId);
        
        glDrawElements(GL_TRIANGLES, command->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<void*>(command->IdxOffset * sizeof(ImDrawIdx)));
      }
    }
  }

  // Disable Scissoring
  glDisable(GL_SCISSOR_TEST);
}

void ImGuiRenderer::Resize(float width, float height)
{
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = { width, height };

  m_Width = width * m_PixelDensity;
  m_Height = height * m_PixelDensity;
}

void ImGuiRenderer::GenerateBuffers()
{
  BufferDesc vboDesc;
  vboDesc.Type = GL_ARRAY_BUFFER;
  vboDesc.Usage = GL_DYNAMIC_DRAW;
  vboDesc.Size = sizeof(ImDrawVert) * m_MaxVertices;
  vboDesc.Data = nullptr;
  vboDesc.Layout = { // ImDrawVert layout
    { ShaderDataType::Float2, "a_Position"},
    { ShaderDataType::Float2, "a_UV"},
    { ShaderDataType::UByte4, "a_Color", true }
  };
  
  m_VBO = new Buffer(vboDesc);

  BufferDesc iboDesc;
  iboDesc.Type = GL_ELEMENT_ARRAY_BUFFER;
  iboDesc.Usage = GL_DYNAMIC_DRAW;
  iboDesc.Size = sizeof(ImDrawIdx) * m_MaxIndices;
  iboDesc.Data = nullptr;

  m_IBO = new Buffer(iboDesc);
}

void ImGuiRenderer::GenerateArrays()
{
  m_VertexArray = new VertexArray();
  m_VertexArray->AttachBuffer(m_VBO);
}

static const char* vertexShader = R"(
#version 410 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec4 a_Color;

uniform mat4 u_ViewProjection;

out vec4 v_FragColor;
out vec2 v_FragUV;

void main()
{
  v_FragColor = a_Color;
  v_FragUV = a_UV;
  gl_Position = u_ViewProjection * vec4(a_Position, 0.0f, 1.0f);
})";

static const char* fragmentShader = R"(
#version 410 core

in vec4 v_FragColor;
in vec2 v_FragUV;

uniform sampler2D u_Texture;

layout (location = 0) out vec4 f_FragColor;

void main()
{
  f_FragColor = v_FragColor * texture(u_Texture, v_FragUV.st);
})";

void ImGuiRenderer::GenerateShaders()
{
  m_Shader = new Shader(vertexShader, fragmentShader);
}

void ImGuiRenderer::GenerateTextures()
{
  ImGuiIO& io = ImGui::GetIO();

  uint8_t* pixels;
  int32_t width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  glGenTextures(1, &m_FontTexture);
  glBindTexture(GL_TEXTURE_2D, m_FontTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void ImGuiRenderer::DestroyBuffers()
{
  delete m_VBO;
  delete m_IBO;
}

void ImGuiRenderer::DestroyArrays()
{
  delete m_VertexArray;
}

void ImGuiRenderer::DestroyShaders()
{
  delete m_Shader;
}

void ImGuiRenderer::DestroyTextures()
{
  glDeleteTextures(1, &m_FontTexture);
}

}