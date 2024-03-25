#pragma once

#include "ForceApplicator.h"

namespace Speck
{

class FrictionForce : public ForceApplicator
{
public:
  void ApplyForces(System* system, float timestep);
};

}