#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace bernd {

using std::runtime_error;
using std::signbit;
using std::string;
using std::vector;

class DecimalNumber {
  private:
    vector<char> digits;
    bool negative;

    static const unsigned INITIAL_POSITION = 40;
    static const unsigned LENGTH = 242; // TODO make appropriate


    void round(unsigned destIndex) {
      char carry = 0;

      if (digits[destIndex + 1] >= 5) {
        digits[destIndex]++;
        if (digits[destIndex] >= 10) {
          digits[destIndex] -= 10;
          carry = 1;
        } else {
          carry = 0;
        }
      }

      // zero the rest of the vector
      digits.resize(destIndex + 1);
      digits.resize(LENGTH);
      destIndex--;

      while (carry && destIndex > 0) {
        digits[destIndex] += carry;
        carry = digits[destIndex] >= 10;
        if (digits[destIndex] >= 10) {
          carry = 1;
          digits[destIndex] -= 10;
        } else {
          carry = 0;
        }
        destIndex--;
      }

      if (destIndex == 0 && carry)
        throw runtime_error("overflow on rounding"); // TODO well we don't really need this later when we made sure we have enough digits.
    }

  public:
    DecimalNumber() : digits(LENGTH), negative(false) {
      digits[0] = 1;
    }

    DecimalNumber(const std::vector<char>& d, bool negative = false) {
      if (d.size() == LENGTH) {
        digits = d;
      } else {
        throw runtime_error("Vector must have correct length");
      }
      this->negative = negative;
    }

    void setNegative(bool n) { negative = n; }

    DecimalNumber& div2() {
      vector<char> result(LENGTH);
      for (unsigned index = 0; index < LENGTH; index++) {
        const unsigned currentDigit = take2(index);
        const unsigned nextDigit = currentDigit / 2;
        result[index] = nextDigit;
        put2(index, currentDigit - 2 * nextDigit);
      }
      digits.swap(result);

      return *this;
    }

    unsigned take2(unsigned index) {
      const char digit0 = (index == 0) ? 0 : digits[index - 1];
      const char digit1 = (index >= LENGTH) ? 0 : digits[index];

      return digit1 + 10 * digit0;
    }

    void put2(unsigned index, unsigned digit) {
      if (digit < 10) {
        if (index != 0) {
          digits[index - 1] = 0;
          digits[index] = digit;
        } else {
          digits[index] = digit;
        }
      } else {
        // will only happen if index > 0, thus no extra check
        digits[index - 1] = 1;
        digits[index] = digit;
      }
    }

    friend std::ostream& operator<<(std::ostream& str, const DecimalNumber& n) {
      if (n.negative) str << "-";
      for (vector<char>::const_iterator it = n.digits.begin(); it != n.digits.end(); ++it) {
        str << (char)(*it + '0');
      }
      return str;
    }

    friend DecimalNumber operator+(const DecimalNumber& a, const DecimalNumber& b) { vector<char> result(LENGTH);
      char carry = 0;
      for (int index = LENGTH; index >= 0; --index) {
        char sumDigit = a.digits[index] + b.digits[index] + carry;
        if (sumDigit < 10) {
          result[index] = sumDigit;
          carry = 0;
        } else {
          result[index] = sumDigit - 10;
          carry = 1;
        }
      }

      return DecimalNumber(result, a.negative); // just propagate the negative flag, that's enough for a+a.
    }

    DecimalNumber& operator+=(const DecimalNumber& o) {
      char carry = 0;
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
    }

    std::string format(int decimals = 6) {
      round(decimals); // TODO this changes our number. Is that okay?
      vector<char>::const_iterator d = digits.begin();

      // trim leading 0's
      while (d != digits.end() && *d == 0) d++;

      if (d == digits.end())
        return "0." + string(decimals, '0');

      int distance_from_decimal_point = d - digits.begin() - INITIAL_POSITION - 1;

      string res(negative ? "-" : "");
      bool before_decimal_point = true;
      if (distance_from_decimal_point >= 0) { // print some leading zeros
        before_decimal_point = false;
        res += "0.";
        while (distance_from_decimal_point) {
          res.push_back('0');
          distance_from_decimal_point--;
          if (!decimals--) return res;
        }
      }

      // now the funny part...
      while (d != digits.end() && decimals) {
        if (!before_decimal_point)
          decimals--;

        res.push_back(*d + '0');
        if (d - digits.begin() == INITIAL_POSITION) {
          res.push_back('.');
          before_decimal_point = false;
        }
        d++;

      }

      return res;
    }

    DecimalNumber(const float f) : digits(LENGTH) {
      unsigned i = INITIAL_POSITION;
      int exp;

      float s = fabs(frexp(f, &exp)) * 8.f; // Pre-scale as to not lose significant bits on the first multiplication with 10
      while (s != 0.f && i < LENGTH) {
        char integerPart = static_cast<char>(s);
        digits[i++] = integerPart;
        s -= integerPart;
        s *= 10.f;
      }

      negative = (f < 0.f);
      div2();div2();div2(); // compensate for prescaling

      if (exp > 0)
        while (exp--)
          operator+=(*this);
      else if (exp < 0)
        while (exp++)
          div2();
    }

    DecimalNumber& round(const int digits) {
      const unsigned digit_index = INITIAL_POSITION + digits;
      if (digit_index == LENGTH - 1)
        return *this;
      else if (digit_index > LENGTH - 1) // TODO decide if we even have to catch this or just do nothing if the number's too big/small
        throw runtime_error("Not enough digits to round");

      round(static_cast<unsigned>(digit_index));

      return *this;
    }
};

}

int main(int argc, char** args) {
  bernd::DecimalNumber f(10.9876f);
  std::cout << f << std::endl;
  std::cout << "format" << std::endl;
  std::cout << f.format() << std::endl;
  std::cout << bernd::DecimalNumber(1.0).format() << std::endl;
  std::cout << bernd::DecimalNumber(0.0).format() << std::endl;
  std::cout << bernd::DecimalNumber(100000.f).format() << std::endl;
  std::cout << bernd::DecimalNumber(0.112233).format() << std::endl;
  std::cout << bernd::DecimalNumber(0.000123456f).format() << std::endl;
  std::cout << std::endl << std::endl;
  std::cout << bernd::DecimalNumber(-1.0).format() << std::endl;
  std::cout << bernd::DecimalNumber(-0.0).format() << std::endl;
  std::cout << bernd::DecimalNumber(-100000.f).format() << std::endl;
  std::cout << bernd::DecimalNumber(-0.112233).format() << std::endl;
  std::cout << bernd::DecimalNumber(-0.000123456f).format() << std::endl;
  bernd::DecimalNumber d(0.112233);
  std::cout << "rounded: " << d.round(7).format(10) << std::endl;
  std::cout << bernd::DecimalNumber(0.9999999999f).round(2).format() << std::endl;
  std::cout << bernd::DecimalNumber(0.444444444444445).format() << std::endl;
  std::cout << bernd::DecimalNumber(999999999999).round(-3).format() << std::endl;

  std::cout << std::fixed << std::setprecision(200);
  std::cout << bernd::DecimalNumber(std::numeric_limits<float>::max()).format(200) << std::endl;
  std::cout << std::numeric_limits<float>::max() << std::endl;
  std::cout << bernd::DecimalNumber(std::numeric_limits<float>::min()).format(200) << std::endl;
  std::cout << std::numeric_limits<float>::min() << std::endl;
  std::cout << bernd::DecimalNumber(std::numeric_limits<float>::lowest()).format(200) << std::endl;
  std::cout << std::numeric_limits<float>::lowest() << std::endl;
  std::cout << bernd::DecimalNumber(std::numeric_limits<float>::epsilon()).format(200) << std::endl;
  std::cout << std::numeric_limits<float>::epsilon() << std::endl;
  std::cout << bernd::DecimalNumber(std::numeric_limits<float>::denorm_min()).format(200) << std::endl;
  std::cout << std::numeric_limits<float>::denorm_min() << std::endl;
}
