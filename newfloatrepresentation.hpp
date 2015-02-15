#ifndef NEWFLOATREPRESENTATION_HPP_5329a47d1d88
#define NEWFLOATREPRESENTATION_HPP_5329a47d1d88

#include <stdexcept>
#include <cmath>

namespace bernd {

template <typename F>
struct Length {};

template<>
struct Length<float> {
  static constexpr unsigned l = 60;
  static constexpr unsigned e0 = 5;
};

template <typename F>
class NewFloatRepresentation {
  public:
    typedef uint32_t limb_t;
    static constexpr unsigned nLimbs = (Length<F>::l + 8) / 9;

  private:
    limb_t limbs[nLimbs];

    bool negative;
    bool inf;
    bool nan;

    unsigned sticky;

    static const unsigned BASE = 1000000000;

  public:
    // TODO default constructor initialize to zero?

    NewFloatRepresentation(F f)
        : negative(f < 0.f), inf(isinf(f)), nan(isnan(f))
    {
      for (unsigned i = 0; i < nLimbs; i++)
        limbs[i] = 0;

      if (inf || nan) return;

      unsigned limbIndex = Length<F>::e0;
      int exp;

      float s = std::fabs(frexp(f, &exp));

      s *= 1<<29;
      limbs[limbIndex] = static_cast<limb_t>(s);

      while (s) {
        s -= limbs[limbIndex];
        s *= 1<<29;
        shift29();

        limbIndex++;
        limbs[limbIndex] = static_cast<limb_t>(s);
      }

    }

    void shift29() {

      for (unsigned i = 0; i < nLimbs; i++) {
        uint64_t limb = limbs[i];
        limb *= 1<<29;

        limbs[i] = limb % BASE;

        if (i != 0)
          limbs[i-1] += limb / BASE;

        if (i == 0 && limb / BASE)
          throw std::runtime_error("Overflow.");
      }
    }

    const limb_t* const getLimbs() const { return &limbs[0]; }

    friend std::ostream& operator<<(std::ostream& str, const NewFloatRepresentation<F>& n) {
      if (n.nan) {
        str << "nan";
        return str;
      }
      if (n.negative) str << "-";
      if (n.inf) {
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
