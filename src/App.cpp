#include "App.h"

#include "Input.h"

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

  // Run Loop
  m_Running = true;
  while (m_Running)
  {
    PollEvents();

    m_Renderer->Update();
    
    m_Renderer->Render();
    SDL_GL_SwapWindow(m_Window);
  }

  // Shutdown
  Shutdown();
}

void App::Init(float w, float h)
{
  // Create the window for the app
  m_Window = SDL_CreateWindow(m_Name.c_str(), w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  // Get our OpenGL surface to draw on
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  
  m_Context = SDL_GL_CreateContext(m_Window);
  SDL_GL_MakeCurrent(m_Window, m_Context);
  
  // Disable VSync
  SDL_GL_SetSwapInterval(0);
  
  // Initialize the renderer
  m_Renderer = new Renderer(w, h);
  
  // Initialize the input manager
  Input::Init();
}

void App::Shutdown()
{  
  // Destroy the app's resources
  delete m_Renderer;
  
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
    switch (event.type)
    {
      case SDL_EVENT_WINDOW_RESIZED:
      {
        m_Renderer->Resize(event.window.data1, event.window.data2);
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

}
