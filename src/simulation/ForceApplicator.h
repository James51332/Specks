#pragma once

#include <vector>

#include "Particle.h"

namespace Speck
{

class System;

// A force applicator is anything that can do work to the system. 
// This may be the applied force from the color matrix, or something
// altogether different, such as gravity.
class ForceApplicator
{
public:
  virtual ~ForceApplicator() = default;

  virtual void ApplyForces(System* system) {}
};

}