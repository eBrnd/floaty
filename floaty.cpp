#include <iomanip>
#include <iostream>
#include <vector>

#include "newfloatrepresentation.hpp"

int main(int argc, char** args)
{
  float f = 1.f;
  bernd::NewFloatRepresentation<float> n(f);

  std::cout << n << std::endl;

  float g = 6.135e33;
  bernd::NewFloatRepresentation<float> m(g);
  std::cout << m << std::endl;

  float h = 1.3f;
  bernd::NewFloatRepresentation<float> o(h);
  std::cout << o << std::endl;

  return 0;
}
