#pragma once

#include <SDL.h>

namespace Speck
{

/// <summary>
/// State manager that stores key and mouse state to be quiered.
/// </summary>
class Input
{
  friend class App;
public:
  static float GetMouseX() { return s_MouseX; }
  static float GetMouseY() { return s_MouseY; }
  
  static float GetScrollX() { return s_ScrollDeltaX; }
  static float GetScrollY() { return s_ScrollDeltaY; }
  
  static bool MousePress(SDL_MouseID btn) { return s_MouseButtons[btn] && !s_MouseButtonsLast[btn]; }
  static bool MouseDown(SDL_MouseID btn) { return s_MouseButtons[btn]; }
  static bool MouseRelease(SDL_MouseID btn) { return !s_MouseButtons[btn] && s_MouseButtonsLast[btn]; }
  
  static bool KeyPress(SDL_Scancode key) { return s_Keys[key] && !s_KeysLast[key]; }
  static bool KeyDown(SDL_Scancode key) { return s_Keys[key]; }
  static bool KeyRelease(SDL_Scancode key) { return !s_Keys[key] && s_KeysLast[key]; }
  
private:
  static void Init();
  static void Update();
  
  // Helper functions called by window to update state.
  static void SetMousePos(float x, float y);
  static void SetScrollDelta(float x, float y);
  static void SetMouseDown(SDL_MouseID btn);
  static void SetMouseUp(SDL_MouseID btn);
  
  static void SetKeyDown(SDL_Scancode key);
  static void SetKeyUp(SDL_Scancode key);
  
private:
  static float s_MouseX, s_MouseY;
  static float s_ScrollDeltaX, s_ScrollDeltaY;
  
  constexpr static size_t s_NumButtons = 8;
  static bool s_MouseButtons[s_NumButtons];
  static bool s_MouseButtonsLast[s_NumButtons];
  
  static bool s_Keys[SDL_NUM_SCANCODES];
  static bool s_KeysLast[SDL_NUM_SCANCODES];
};

}
