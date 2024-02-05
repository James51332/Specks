#include "App.h"

#include <imgui.h>
#include <glm/gtc/random.hpp>

#include "core/Input.h"
#include "ui/Settings.h"

namespace Speck
{

Specks::Specks()
{
  // Initialize the renderer
  m_Renderer = new ::Speck::ParticleRenderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
  m_UIRenderer = new Vision::ImGuiRenderer(m_DisplayWidth, m_DisplayHeight, m_DisplayScale);
  m_Camera = new Vision::OrthoCamera(m_DisplayWidth, m_DisplayHeight, 70.0f);

  // Setup the particle system
  m_System = new System(500, 5, 100.0f);
  m_ColorMatrix = ColorMatrix(5);
  m_ColorMatrix.SetColor(0, {1.0f, 1.0f, 0.0f, 1.0f});
  m_ColorMatrix.SetColor(1, {0.0f, 1.0f, 1.0f, 1.0f});
  m_ColorMatrix.SetColor(2, {1.0f, 0.0f, 1.0f, 1.0f});
  m_ColorMatrix.SetColor(3, {0.5f, 1.0f, 0.8f, 1.0f});
  m_ColorMatrix.SetColor(4, {0.8f, 0.2f, 0.5f, 1.0f});
}

Specks::~Specks()
{
  // Destroy the app's resources
  delete m_Camera;
  delete m_System;
  delete m_Renderer;
  delete m_UIRenderer;
}

void Specks::OnUpdate(float timestep)
{
  // Update simulation
  if (Vision::Input::KeyPress(SDL_SCANCODE_SPACE)) m_UpdateSystem = !m_UpdateSystem;
  if (m_UpdateSystem)
  	m_System->Update(m_ColorMatrix, timestep);
  
  // Update the camera system
  m_Camera->Update(timestep);
  
  // Clear the screen
  glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Render our particles
  m_Renderer->BeginFrame(m_Camera, m_System->GetBoundingBoxSize());
  m_Renderer->DrawParticles(m_System->GetParticles(), m_ColorMatrix);
  m_Renderer->EndFrame();
  
  DisplayUI(timestep);
}

void Specks::OnResize()
{
  m_Camera->SetWindowSize(m_DisplayWidth, m_DisplayHeight);
  m_Renderer->Resize(m_DisplayWidth, m_DisplayHeight);
  m_UIRenderer->Resize(m_DisplayWidth, m_DisplayHeight);
}

void Specks::DisplayUI(float timestep)
{
  m_UIRenderer->Begin();
  ImGui::Begin("Settings");
  {
    // Color Matrix UI
    ImGui::SeparatorText("Color Matrix");
    {
      UI::DisplayColorMatrix(m_ColorMatrix);

      ImGui::SameLine();
      if (ImGui::Button("Randomize"))
      {
        std::size_t numColors = m_ColorMatrix.GetNumColors();
        for (std::size_t i = 0; i < numColors; i++)
        {
          for (std::size_t j = 0; j < numColors; j++)
          {
            m_ColorMatrix.SetAttractionScale(i, j, glm::linearRand(-1.0f, 1.0f));
          }
        }
      }
    }

    // Simulation Settings UI
    ImGui::SeparatorText("Simulation");
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12); // Ensure labels fit in window
    {
      if (ImGui::Button("Play/Pause (Space)")) m_UpdateSystem = !m_UpdateSystem;

      float interactionRadius = m_System->GetInteractionRadius();
      float boundingSize = m_System->GetBoundingBoxSize();
      int numParticles = static_cast<int>(m_System->GetParticles().size());

      if (ImGui::SliderFloat("Interaction Radius", &interactionRadius, 5.0f, boundingSize / 2.0f, "%.1f"))
        m_System->SetInteractionRadius(interactionRadius);
      if (ImGui::SliderFloat("Simulation Size", &boundingSize, interactionRadius, 500.0f, "%.1f"))
        m_System->SetBoundingBoxSize(boundingSize);
      if (ImGui::InputInt("Number of Particles", &numParticles))
        m_System->SetNumParticles(static_cast<std::size_t>(numParticles), m_ColorMatrix.GetNumColors());
    }
    ImGui::PopItemWidth();

    // Debug Info
    ImGui::SeparatorText("Debug Info");
    {
      ImGui::Text("Frame Time: %.2fms", timestep * 1000.0f);
    }

    // Engine Settings
    ImGui::SeparatorText("Engine");
    {
      bool threaded = m_System->IsMultiThreaded();
      if (ImGui::Checkbox("Multithreaded", &threaded))
        m_System->SetMultiThreaded(threaded);
    }
  }
  ImGui::End();
  m_UIRenderer->End();
}

}
