#ifndef BASE1GFLOAT_HPP_c9b25b3fcb
#define BASE1GFLOAT_HPP_c9b25b3fcb

#include <vector>
#include <stdexcept>

namespace bernd {

using std::vector;
using std::runtime_error;

class Base1GNumber {
  private:
    vector<uint32_t> packedDigits;

    bool negative;
    bool inf;
    bool nan;

    static const unsigned INITIAL_POSITION = 4;

  public:
    static const unsigned LENGTH = 12; // TODO make appropriate

    Base1GNumber()
        : packedDigits(LENGTH), negative(false), inf(false), nan(false) {
      packedDigits[INITIAL_POSITION] = 1;
    }

    Base1GNumber(vector<uint32_t> p, bool negative = false, bool inf = false, bool nan = false) {
      if (p.size() == LENGTH) {
        packedDigits = p;
      } else {
        throw runtime_error("Vector must have correct length");
      }

      this->negative = negative;
      this->inf = inf;
      this->nan = nan;
    }

    void setNegative(bool n) { negative = n; }

    Base1GNumber& div2() {
      bool remainder = false;
      for (unsigned index = 0; index < LENGTH; index++) {
        uint32_t result = remainder ? 500000000 : 0;
        uint32_t currentDigit = packedDigits[index];
        remainder = currentDigit & 1;
        result += currentDigit >> 1;
        packedDigits[index] = result;
      }

      return *this;
    }

    Base1GNumber& operator+=(const Base1GNumber& o) {
      /*char carry = 0;
      for (int index = LENGTH; index >- 0; --index) {
        char sumDigit = digits[index] + o.digits[index] + carry;
        if (sumDigit < 10) {
          digits[index] = sumDigit;
          carry = 0;
        } else {
          digits[index] = sumDigit - 10;
          carry = 1;
        }
      }

      return *this;
      */
      uint32_t carry = false;
      for (unsigned index = LENGTH - 1; index >= 0; index--) {
        uint32_t sum = packedDigits[index] + o.packedDigits[index] + carry;
        if (sum < 1000000000) {
          packedDigits[index] = sum;
          carry = 0;
        } else {
          packedDigits[index] = sum - 1000000000;
          carry = 1;
        }
      }

      return *this;
    }

    friend std::ostream& operator<<(std::ostream& str, const Base1GNumber& n) {
      if (n.nan) {
        str << "nan";
        return str;
      }
      if (n.negative) str << "-";
      if (n.inf) {
        str << "inf";
        return str;
      }
      str << std::endl;
      for (vector<uint32_t>::const_iterator it = n.packedDigits.begin(); it != n.packedDigits.end();
          ++it) {
        str << std::setfill('0') << std::setw(9) << *it << " ";
      }

      return str;
    }
};

}
#endif
