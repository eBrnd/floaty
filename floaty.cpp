#include <iomanip>
#include <iostream>
#include <vector>

#include "base1gnumber.hpp"

int main(int argc, char** args)
{
  std::vector<uint32_t> digits(bernd::Base1GNumber::LENGTH);
  digits[6] = 23456;
  digits[7] = 123456789;
  digits[8] = 987654321;
  bernd::Base1GNumber n(digits);
  std::cout << n << std::endl;
  n.div2();
  std::cout << n << std::endl;
  n += n;
  std::cout << n << std::endl;

  std::cout << bernd::Base1GNumber(1234567890.f) << std::endl;
  std::cout << bernd::Base1GNumber(1.234567888f) << std::endl;
  std::cout << bernd::Base1GNumber(1.45).round(1) << std::endl;
  std::cout << bernd::Base1GNumber(1.45).format() << std::endl;
  std::cout << bernd::Base1GNumber(0.4567890).format() << std::endl;

  std::cout << bernd::Base1GNumber(1.45).round(1) << std::endl;
  std::cout << bernd::Base1GNumber(1.45).format(6,'f') << std::endl;
  std::cout << bernd::Base1GNumber(0.4567890).format(6, 'f') << std::endl;

  return 0;
}
