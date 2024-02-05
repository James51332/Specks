#include "app/App.h"

int main()
{
  Speck::Specks* app = new Speck::Specks();
  app->Run();
  delete app;
}
