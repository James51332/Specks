#include "App.h"

#include <iostream>

int main()
{
  Speck::App* app = new Speck::App("Specks");
  app->Run();
  delete app;
}
