#ifndef BASE1GFLOAT_HPP_c9b25b3fcb
#define BASE1GFLOAT_HPP_c9b25b3fcb

#include <vector>
#include <stdexcept>

namespace bernd {

using std::vector;
using std::runtime_error;

class Base1GNumber {
  private:
    typedef uint32_t digit_t;
    vector<digit_t> packedDigits;

    bool negative;
    bool inf;
    bool nan;

    static const unsigned INITIAL_POSITION = 4;
    static const unsigned LENGTH = 12; // TODO make appropriate
    static const unsigned BASE = 1000000000;

  public:

    Base1GNumber()
        : packedDigits(LENGTH), negative(false), inf(false), nan(false) {
      packedDigits[INITIAL_POSITION] = 1;
    }

    Base1GNumber(vector<digit_t> p, bool negative = false, bool inf = false, bool nan = false) {
      if (p.size() == LENGTH) {
        packedDigits = p;
      } else {
        throw runtime_error("Vector must have correct length");
      }

      this->negative = negative;
      this->inf = inf;
      this->nan = nan;
    }

    Base1GNumber(float f) : packedDigits(LENGTH) {
      if ((nan = isnan(f)) == true) return;
      if ((inf = isinf(f)) == true) return;
      negative = f < 0.f;

      unsigned i = INITIAL_POSITION;
      int exp;

      float s = fabs(frexp(f, &exp)) * 8.f; // Pre-scale as to not lose siginficant bits on the first multiplication with 1000000000
      while (s != 0.f && i < LENGTH) {
        digit_t integerPart = static_cast<digit_t>(s);
        packedDigits[i++] = integerPart;
        s -= integerPart;
        s *= BASE;
      }

      div2();div2();div2(); // compensate for prescaling

      if (exp > 0)
        while (exp--)
          operator+=(*this);
      else if (exp < 0)
        while (exp++)
          div2();
    }

    Base1GNumber& div2() {
      bool remainder = false;
      for (unsigned index = 0; index < LENGTH; index++) {
        digit_t result = remainder ? (BASE / 2) : 0;
        digit_t currentDigit = packedDigits[index];
        remainder = currentDigit & 1;
        result += currentDigit >> 1;
        packedDigits[index] = result;
      }

      return *this;
    }

    digit_t getDigit(unsigned index) const {
      // split in packedDigit index and digit index
      unsigned subIndex = 8 - index % 9;
      index /= 9;

      digit_t packedDigit = packedDigits[index];
      while (subIndex--)
        packedDigit /= 10;

      return packedDigit % 10;
    }

    Base1GNumber& round(int decimals) {
      decimals += (INITIAL_POSITION + 1) * 9 - 1;
      unsigned subIndex = 8 - decimals % 9;
      int index = decimals / 9;

      digit_t& packedDigit = packedDigits[index];
      digit_t carry = 0;

      if (getDigit(decimals + 1) >= 5) {
        for (unsigned i = 0; i < subIndex; i++)
          packedDigit /= 10;

        packedDigit++;

        for (unsigned i = 0; i < subIndex; i++)
          packedDigit *= 10;

        if (packedDigit >= BASE) {
          carry = 1;
          packedDigit -= BASE;
        }

        while (carry && index-- > 0) {
          digit_t& packedDigit = packedDigits[index];
          if (packedDigit++ > BASE) {
            carry = 1;
            packedDigit -= BASE;
          } else {
            carry = 0;
          }
        }
      } else {
        for (unsigned i = 0; i < subIndex; i++)
          packedDigit /= 10;
        for (unsigned i = 0; i < subIndex; i++)
          packedDigit *= 10;
      }

      // zero everythig to the right
      packedDigits.resize(decimals / 9 + 1);
      packedDigits.resize(LENGTH);

      return *this;
    }

    Base1GNumber& operator+=(const Base1GNumber& o) {
      digit_t carry = 0;
      for (int index = LENGTH - 1; index >= 0; index--) {
        digit_t sum = packedDigits[index] + o.packedDigits[index] + carry;
        if (sum < BASE) {
          packedDigits[index] = sum;
          carry = 0;
        } else {
          packedDigits[index] = sum - BASE;
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
      for (vector<digit_t>::const_iterator it = n.packedDigits.begin(); it != n.packedDigits.end();
          ++it) {
        str << std::setfill('0') << std::setw(9) << *it << " ";
      }

      return str;
    }

};

}
#endif
