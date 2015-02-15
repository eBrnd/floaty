#include <iomanip>
#include <iostream>
#include <vector>

#include "newfloatrepresentation.hpp"

int main(int argc, char** args)
{
  float f = 1.f;
  bernd::NewFloatRepresentation<float> n(f);

  std::cout << n << std::endl;

  return 0;
}
