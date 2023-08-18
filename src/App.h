#pragma once

#include <string>

#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

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
  bool m_Running = false;
  SDL_Window* m_Window;
  SDL_GLContext m_Context;
};

}