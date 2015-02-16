#include <iomanip>
#include <iostream>
#include <vector>

#include "newfloatrepresentation.hpp"

int main(int argc, char** args)
{
  float f = 1.f;
  bernd::NewFloatRepresentation<float> n(f);

  std::cout << n << std::endl;

  float g = 1.5e33;
  bernd::NewFloatRepresentation<float> m(g);
  std::cout << m << std::endl;

  return 0;
}
