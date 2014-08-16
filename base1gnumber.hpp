#ifndef BASE1GFLOAT_HPP_c9b25b3fcb
#define BASE1GFLOAT_HPP_c9b25b3fcb

#include <cmath>
#include <stdexcept>
#include <vector>

namespace bernd {

using std::runtime_error;
using std::string;
using std::vector;

class Base1GNumber {
  private:
    typedef uint32_t digit_t;
    vector<digit_t> packedDigits;

    bool negative;
    bool inf;
    bool nan;

    static const unsigned INITIAL_POSITION = 4;
    static const unsigned BASE = 1000000000;

  public:
    static const unsigned LENGTH = 12; // TODO make appropriate

    Base1GNumber()
        : packedDigits(LENGTH), negative(false), inf(false), nan(false) {
      packedDigits[INITIAL_POSITION] = 1;
    }

    Base1GNumber(vector<digit_t> p, bool negative = false, bool inf = false, bool nan = false)
        : negative(negative), inf(inf), nan(nan) {
      if (p.size() == LENGTH) {
        packedDigits = p;
      } else {
        throw runtime_error("Vector must have correct length");
      }
    }

    Base1GNumber(float f) : packedDigits(LENGTH), negative(f < 0.f), inf(isinf(f)), nan(isnan(f)) {
      if (inf || nan) return;

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

    void fillDigitPackString(string& s, digit_t d) {
      s.replace(0, 9, 9, '0');
      int index = 9;
      while (d && index) {
        s[--index] = (d % 10) + '0';
        d /= 10;
      }
    }

    string format(int decimals = 6, char convspec = 'g') {
      if (convspec != 'g' && convspec != 'f')
        throw runtime_error("Invalid conversion specifier");

      if (nan) return "nan";
      string res(negative ? "-" : "");
      if (inf) {
        res += "inf";
        return res;
      }

      round(decimals); // TODO this changes our number (that's why format can't be const). That's probably not good, but otherwise we'd need a copy.

      bool in_leading_zeros = true; // Flag to store whether we're processing leading zeros. Will be set to false once we reach 'actual' digits. Neede for suppressing leading zeros.
      bool before_decimal_point = true; // Are we processing digits before or after the decimal point. Needed for setting the '0.' prefix on numbers n : -1 < n < 1.
      unsigned digits_count = 0; // How many digits are processed. Needed foer setting the decimal point.
      string d(9, '0'); // Buffer for converting a digitPack to 9 decimal digits.
      bool only_zeros_left = false; // Flag will be set once we have only trailing zeros left to process. Needed to suppress trailing zeros.

      for (vector<digit_t>::const_iterator digit_it = packedDigits.begin();
          digit_it != packedDigits.end(); ++digit_it) {
        fillDigitPackString(d, *digit_it);

        for (string::const_iterator dd_it = d.begin(); dd_it != d.end(); ++dd_it) {
          // in here, we actually iterate over all decimal digits of our number.

          digits_count++;
          if (before_decimal_point && (digits_count > (INITIAL_POSITION + 1) * 9)) {
            before_decimal_point = false;
            if (in_leading_zeros) {
              res += "0.";
              in_leading_zeros = false;
            } else {
              res += ".";
            }
          }

          if (!(in_leading_zeros && *dd_it == '0')) { // trim leading zeros
            in_leading_zeros = false;
            res += *dd_it;
          }

          if (convspec == 'g') { // TODO in printf, g automatically selects from e and f whichever is shorter, additinally trimming trailing zeros
            // trim any trailing zeros
            // check if current digitPack is zero
            only_zeros_left = true;
            for (string::const_iterator check_it = dd_it + 1;
                check_it != d.end() && only_zeros_left; ++check_it) {
              if (*check_it != '0')
                only_zeros_left = false;
            }
            // check if all remaining digit packs are zero
            for (vector<digit_t>::const_iterator check_it = digit_it + 1;
                check_it != packedDigits.end() && only_zeros_left; ++check_it) {
              if (*check_it != 0)
                only_zeros_left = false;
            }
          } else { // if (convspec == 'f')
            // trim only digits that were rounded off
            if (digits_count >= ((INITIAL_POSITION + 1) * 9) + decimals)
              only_zeros_left = true;
          }

          if (only_zeros_left) break;
        }
        if (only_zeros_left) break;
      }

      return res;
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
      for (vector<digit_t>::const_iterator it = n.packedDigits.begin(); it != n.packedDigits.end();
          ++it) {
        str << std::setfill('0') << std::setw(9) << *it << " ";
      }

      return str;
    }

};

}
#endif
