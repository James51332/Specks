#pragma once

#include "core/App.h"

#include "ui/ImGuiRenderer.h"

#include "ParticleRenderer.h"
#include "simulation/System.h"
#include "simulation/ColorMatrix.h"


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
  ParticleRenderer* m_Renderer = nullptr;
  Vision::OrthoCamera* m_Camera = nullptr;
  Vision::ImGuiRenderer* m_UIRenderer = nullptr;

  System* m_System = nullptr;
  bool m_UpdateSystem = false;
  ColorMatrix m_ColorMatrix;
};

}
