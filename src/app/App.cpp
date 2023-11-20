#include "App.h"

#include <iostream>
#include <glad/glad.h>
#include <imgui.h>
#include <chrono>
#include <glm/gtc/random.hpp>

#include "app/Input.h"
#include "ui/UIInput.h"

#include "ui/Settings.h"

namespace Speck
{

App::App(const std::string& name)
  : m_Name(name)
{
  // Initialize SDL (this should last the entire lifetime of program, not just the run cycle.)
  if (SDL_Init(SDL_INIT_VIDEO) == -1)
    SDL_Log("Failed to initialize SDL");
}

App::~App()
{
  // Shutdown SDL
  SDL_Quit();
}

void App::Stop()
{
  m_Running = false;
}

void App::Run()
{
  // Initialization code
  Init();
  auto lastTime = std::chrono::high_resolution_clock::now();

  // Run Loop
  m_Running = true;
  while (m_Running)
  {
    // Poll User Input
    PollEvents();
    
    // Calculate timestep since last frame (ms to s)
    auto curTime = std::chrono::high_resolution_clock::now();
    float timestepMilliseconds = std::chrono::duration<float, std::chrono::milliseconds::period>(curTime - lastTime).count();
    auto timestep = timestepMilliseconds * 0.001f;
    lastTime = curTime;

    // Update simulation
    if (Input::KeyPress(SDL_SCANCODE_SPACE)) m_UpdateSystem = !m_UpdateSystem;
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

    SDL_GL_SwapWindow(m_Window);
  }

  // Shutdown
  Shutdown();
}

void App::Init(int w, int h)
{
  // Create the window for the app
  m_Window = SDL_CreateWindow(m_Name.c_str(), w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

  // Get our OpenGL surface to draw on
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  
  m_Context = SDL_GL_CreateContext(m_Window);
  SDL_GL_MakeCurrent(m_Window, m_Context);
  
  // Disable VSync
  SDL_GL_SetSwapInterval(1);
  
  // Initialize the renderer
  float displayScale = SDL_GetWindowDisplayScale(m_Window);
  m_Renderer = new Renderer(static_cast<float>(w), static_cast<float>(h), displayScale);
  m_UIRenderer = new ImGuiRenderer(static_cast<float>(w), static_cast<float>(h), displayScale);
  m_Camera = new Camera(static_cast<float>(w), static_cast<float>(h), 75.0f);
  
  // Initialize the input manager
  Input::Init();
  
  // Setup the particle system
  m_System = new System(100, 5, 50.0f);
  m_ColorMatrix = ColorMatrix(5);
  m_ColorMatrix.SetColor(0, {1.0f, 1.0f, 0.0f, 1.0f});
  m_ColorMatrix.SetColor(1, {0.0f, 1.0f, 1.0f, 1.0f});
  m_ColorMatrix.SetColor(2, {1.0f, 0.0f, 1.0f, 1.0f});
  m_ColorMatrix.SetColor(3, {0.5f, 1.0f, 0.8f, 1.0f});
  m_ColorMatrix.SetColor(4, {0.8f, 0.2f, 0.5f, 1.0f});
}

void App::Shutdown()
{
  // Destroy the app's resources
  delete m_Camera;
  delete m_System;
  delete m_Renderer;
  delete m_UIRenderer;
  
  SDL_GL_DeleteContext(m_Context);
  m_Context = nullptr;
  
  SDL_DestroyWindow(m_Window);
  m_Window = nullptr;
}

void App::PollEvents()
{
  // Set the input manager to a new frame
  Input::Update();
  
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    // We let our UI library block events to the app if it uses them
    // This won't interfere with app events, but it still may be more 
    // logical to process app events before passing to the UI library.
    bool processed = UI::ProcessEvent(&event);
    if (processed) continue;

    switch (event.type)
    {
      case SDL_EVENT_WINDOW_RESIZED:
      {
        float width = static_cast<float>(event.window.data1);
        float height = static_cast<float>(event.window.data2);
        m_Camera->SetWindowSize(width, height);
        m_Renderer->Resize(width, height);
        m_UIRenderer->Resize(width, height);
        break;
      }
      case SDL_EVENT_QUIT:
      {
        Stop();
        break;
      }
      case SDL_EVENT_KEY_DOWN:
      {
        Input::SetKeyDown(event.key.keysym.scancode);
        break;
      }
      case SDL_EVENT_KEY_UP:
      {
        Input::SetKeyUp(event.key.keysym.scancode);
        break;
      }
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      {
        Input::SetMouseDown(event.button.button);
        break;
      }
      case SDL_EVENT_MOUSE_BUTTON_UP:
      {
        Input::SetMouseUp(event.button.button);
        break;
      }
      case SDL_EVENT_MOUSE_MOTION:
      {
        Input::SetMousePos(event.motion.x, event.motion.y);
        break;
      }
      case SDL_EVENT_MOUSE_WHEEL:
      {
        Input::SetScrollDelta(event.wheel.x, event.wheel.y);
        break;
      }
      default: break;
    }
  }
}

void App::DisplayUI(float timestep)
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

      //if (ImGui::Button("Make Symmetric", &symmetric))
      {

      }
    }

    // Simulation Settings UI
    ImGui::SeparatorText("Simulation");
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12); // Ensure labels fit in window
    {
      if (ImGui::Button("Play/Pause (Space)")) m_UpdateSystem = !m_UpdateSystem;

      float interactionRadius = m_System->GetInteractionRadius();
      if (ImGui::SliderFloat("Interaction Radius", &interactionRadius, 5.0f, 50.0f, "%.1f"))
        m_System->SetInteractionRadius(interactionRadius);

      float boundingSize = m_System->GetBoundingBoxSize();
      if (ImGui::SliderFloat("Simulation Size", &boundingSize, interactionRadius, 500.0f, "%.1f"))
        m_System->SetBoundingBoxSize(boundingSize);

      int numParticles = static_cast<int>(m_System->GetParticles().size());
      if (ImGui::InputInt("Number of Particles", &numParticles))
        m_System->SetNumParticles(static_cast<std::size_t>(numParticles), m_ColorMatrix.GetNumColors());
    }
    ImGui::PopItemWidth();

    // Debug Info
    ImGui::SeparatorText("Debug Info");
    {
      ImGui::Text("Frame Time: %.2fms", timestep * 1000.0f);
    }
  }
  ImGui::End();
  m_UIRenderer->End();
}

}
