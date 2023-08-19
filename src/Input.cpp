#include "Input.h"

#include <cstring>

namespace Speck
{

float Input::s_MouseX = 0;
float Input::s_MouseY = 0;
float Input::s_ScrollDeltaX = 0;
float Input::s_ScrollDeltaY = 0;

bool Input::s_MouseButtons[Input::s_NumButtons];
bool Input::s_MouseButtonsLast[Input::s_NumButtons];

bool Input::s_Keys[SDL_NUM_SCANCODES];
bool Input::s_KeysLast[SDL_NUM_SCANCODES];

void Input::Init()
{
  for (size_t i = 0; i < s_NumButtons; i++)
  {
    s_MouseButtons[i] = false;
    s_MouseButtonsLast[i] = false;
  }
  
  for (size_t i = 0; i < SDL_NUM_SCANCODES; i++)
  {
    s_Keys[i] = false;
    s_KeysLast[i] = false;
  }
}

void Input::Update()
{
  // Reset the scroll each frame.
  s_ScrollDeltaX = 0;
  s_ScrollDeltaY = 0;
  
  // Maybe not the safest but it's probably fine
  std::memcpy(s_KeysLast, s_Keys, sizeof(bool) * SDL_NUM_SCANCODES);
  std::memcpy(s_MouseButtonsLast, s_MouseButtons, sizeof(bool) * s_NumButtons);
}

void Input::SetMousePos(float x, float y)
{
  s_MouseX = x;
  s_MouseY = y;
}

void Input::SetScrollDelta(float x, float y)
{
  s_ScrollDeltaX = x;
  s_ScrollDeltaY = y;
}

void Input::SetMouseDown(SDL_MouseID btn)
{
  s_MouseButtons[btn] = true;
}

void Input::SetMouseUp(SDL_MouseID btn)
{
  s_MouseButtons[btn] = false;
}

void Input::SetKeyDown(SDL_Scancode key)
{
  if (key < SDL_NUM_SCANCODES)
    s_Keys[key] = true;
}

void Input::SetKeyUp(SDL_Scancode key)
{
  if (key < SDL_NUM_SCANCODES)
    s_Keys[key] = false;
}

}
