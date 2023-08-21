#pragma once

#include <string>
#include <SDL.h>

#include "Renderer.h"
#include "System.h"

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
  void Init(float w = 800, float h = 600);
  void Shutdown();

  void PollEvents();
  
private:
  bool m_Running = false;
  std::string m_Name;

  Renderer* m_Renderer = nullptr;
  System* m_System = nullptr;

  SDL_Window* m_Window = nullptr;
  SDL_GLContext m_Context = nullptr;
};

}
