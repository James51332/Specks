#include "App.h"

namespace Speck
{

App::App(const std::string& name)
{
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) == -1)
  {
    SDL_Log("Failed to initialize SDL");
  }
  
  // Create the window for the app
  m_Window = SDL_CreateWindow(name.c_str(), 500, 500, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  
  // Get our OpenGL surface to draw on
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  m_Context = SDL_GL_CreateContext(m_Window);
}

App::~App()
{
  // Destroy the app's resources
  SDL_GL_DeleteContext(m_Context);
  SDL_DestroyWindow(m_Window);
  m_Window = nullptr;

  // Shutdown SDL
  SDL_Quit();
}

void App::Stop()
{
  m_Running = false;
}

void App::Run()
{
  m_Running = true;
  while (m_Running)
  {
    SDL_Event event;
    SDL_PollEvent(&event);

    if (event.type == SDL_EVENT_QUIT)
      Stop();
  }
}

}