#include <iostream>

#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

int main()
{
  SDL_Window* window = SDL_CreateWindow("OpenGL Test", 500, 500, SDL_WINDOW_OPENGL);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  
  SDL_GLContext context = SDL_GL_CreateContext(window);
  
  while (true)
  {
    SDL_Event event;
    SDL_PollEvent(&event);
    
    if (event.type == SDL_EVENT_QUIT)
      break;
  }
  
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  
  SDL_Quit();
}
