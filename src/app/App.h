#pragma once

#include <string>
#include <SDL.h>

#include "renderer/Renderer.h"
#include "simulation/System.h"
#include "simulation/ColorMatrix.h"
#include "ui/ImGuiRenderer.h"

namespace Speck
{

class App
{
public:
  App(const std::string& name);
  ~App();

  void Stop();
  void Run();

private:
  void Init(int w = 1280, int h = 720);
  void Shutdown();

  void PollEvents();
  
private:
  bool m_Running = false;
  std::string m_Name;

  Renderer* m_Renderer = nullptr;
  ImGuiRenderer* m_UIRenderer = nullptr;
  Camera* m_Camera = nullptr;
  System* m_System = nullptr;
  ColorMatrix m_ColorMatrix;

  SDL_Window* m_Window = nullptr;
  SDL_GLContext m_Context = nullptr;
};

}
