#ifndef NEWFLOATREPRESENTATION_HPP_5329a47d1d88
#define NEWFLOATREPRESENTATION_HPP_5329a47d1d88

#include <stdexcept>
#include <cmath>

namespace bernd {

template <typename F>
struct Length {};

template<>
struct Length<float> {
  static constexpr unsigned l = 80;
  static constexpr unsigned e0 = 5;
};

template <typename F>
class NewFloatRepresentation {
  public:
    typedef uint32_t limb_t;
    static constexpr unsigned nLimbs = (Length<F>::l + 8) / 9;

  private:
    limb_t limbs[nLimbs];
    int exponent;

    bool negative;
    int fpclass;

    unsigned sticky;

    static const unsigned BASE = 1000000000;

  public:
    // TODO default constructor initialize to zero?

    NewFloatRepresentation(F f)
        : limbs{}, negative(f < 0.f), fpclass(std::fpclassify(f))
    {
      if (fpclass == FP_INFINITE || fpclass == FP_NAN) return;

      unsigned limbIndex = Length<F>::e0;

      float s = std::fabs(frexp(f, &exponent));

      s *= 1<<29; // pre-scale with 2^29 to extract as many bits as possible in the first iteration
      exponent -= 29; // store prescaling in exponent
      limbs[limbIndex] = static_cast<limb_t>(s);

      while (s) {
        s -= limbs[limbIndex];
        s *= 1<<29;
        mult2(29);

        limbIndex++;
        limbs[limbIndex] = static_cast<limb_t>(s);
      }

      // scale to exponent=0
      if (exponent < 0)
        div2(-exponent);
      else if (exponent > 0)
        mult2(exponent);

    }

    void mult2(unsigned expn) {
      exponent -= expn;

      while (expn) {
        unsigned nextponent = 0;

        if (expn > 29) { // do multiple iterations if exponent is too large
          nextponent = expn - 29;
          expn = 29;
        }

        for (unsigned i = 0; i < nLimbs; i++) {
          uint64_t limb = limbs[i];
          limb *= 1<<expn;

          limbs[i] = limb % BASE;

          if (i != 0) {
            limbs[i-1] += limb / BASE; // carry for next limb

            // bubble carry through
            for (unsigned j = i - 1; j >= 1; j--) {
              limbs[j-1] += limbs[j] / BASE;
              limbs[j] %= BASE;
            }

            if (limbs[0] > BASE)
              throw std::runtime_error("Overflow.");
          }

          if (i == 0 && limb / BASE)
            throw std::runtime_error("Overflow.");
        }

        expn = nextponent;
      }
    }

    void div2(unsigned expn) {
      exponent += expn;

      while (expn) {
        unsigned nextponent = 0;

        if (expn > 29) { // do multiple iterations if exponent is too large
          nextponent = expn - 29;
          expn = 29;
        }

        limb_t remainder = 0;
        for (unsigned i = 0; i < nLimbs; i++) {
          uint64_t limb = limbs[i];
          limb += (static_cast<uint64_t>(remainder) * BASE);
          limbs[i] = limb / (1 << expn);
          remainder = limb % (1 << expn);
        }

        expn = nextponent;
      }
    }

    const limb_t* const getLimbs() const { return &limbs[0]; }

    friend std::ostream& operator<<(std::ostream& str, const NewFloatRepresentation<F>& n) {
      if (n.fpclass == FP_NAN) {
        str << "nan";
        return str;
      }
      if (n.negative) str << "-";
      if (n.fpclass == FP_INFINITE) {
        str << "inf";
        return str;
      }

      for (unsigned i = 0; i < n.nLimbs; i++) {
        const NewFloatRepresentation::limb_t* limbPtr = n.getLimbs() + i;
        str << std::setfill('0') << std::setw(9) << *limbPtr << " ";
      }

      return str;
    }

};

}

#endif
