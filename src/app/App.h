#pragma once

#include "core/App.h"


#include "renderer/Renderer2D.h"
#include "ui/ImGuiRenderer.h"

#include "simulation/System.h"
#include "simulation/ColorMatrix.h"
#include "simulation/ColorForce.h"
#include "simulation/FrictionForce.h"

namespace Speck
{

class Specks : public Vision::App
{
public:
  Specks();
  ~Specks();

  void OnUpdate(float timestep);
  void OnResize();

private:
  void DisplayUI(float timestep);
  
private:
  // Rendering
  Vision::Renderer2D* m_Renderer = nullptr;
  Vision::PerspectiveCamera* m_Camera = nullptr;
  Vision::ImGuiRenderer* m_UIRenderer = nullptr;

  // Particle System
  System* m_System = nullptr;
  bool m_UpdateSystem = false;

  // Forces
  ColorForce m_ColorForce;
  ColorMatrix m_ColorMatrix;
  FrictionForce m_FrictionForce;
};

}
