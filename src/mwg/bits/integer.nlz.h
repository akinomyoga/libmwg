// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BITS_INTEGER_NLZ_H
#define MWG_BITS_INTEGER_NLZ_H
#include <mwg/std/type_traits>
namespace mwg {
namespace mpl {

  namespace integral_ndigits_detail {

    template<
      typename UIntType, UIntType value, int accumulator = 1, std::size_t width = std::numeric_limits<UIntType>::digits,
      std::size_t shift = (width + 1) / 2>
    struct for_nonzero_unsigned: for_nonzero_unsigned<
      UIntType,
      value >> shift? value >> shift: value,
      value >> shift? accumulator + shift: accumulator,
      width - shift> {};
    template<typename UIntType, UIntType value, int accumulator, std::size_t shift>
    struct for_nonzero_unsigned<UIntType, value, accumulator, 0, shift>: stdm::integral_constant<int, accumulator> {};

    template<typename UIntType, UIntType value, bool = value != 0>
    struct for_unsigned: for_nonzero_unsigned<UIntType, value> {};
    template<typename UIntType, UIntType value>
    struct for_unsigned<UIntType, value, false>: stdm::integral_constant<int, 0> {};

    template<typename Integer, Integer value, typename UIntType = typename stdm::make_unsigned<Integer>::type>
    struct for_signed: for_unsigned<UIntType, (value < 0? -(UIntType) value: (UIntType) value)> {};

    template<typename Integer, Integer value, typename = void> struct impl {};

    template<typename Integer, Integer value>
    struct impl<Integer, value, typename stdm::enable_if<stdm::is_signed<Integer>::value>::type>: for_signed<Integer, value> {};
    template<typename Integer, Integer value>
    struct impl<Integer, value, typename stdm::enable_if<stdm::is_unsigned<Integer>::value>::type>: for_unsigned<Integer, value> {};
  }

  template<typename Integer, Integer value>
  struct integral_ndigits: integral_ndigits_detail::impl<Integer, value> {};

}
}
#include <limits>
#include <mwg/defs.h>
#include <mwg/std/cstdint>
namespace mwg {
namespace integer {

#ifdef __STDC_IEC_559__
  // http://www.nminoru.jp/~nminoru/programming/bitcount.html 改変
  namespace ndigits_detail {
    template<typename Unsigned, typename Float, typename Rep>
    int ndigits_(Unsigned value) mwg_noexcept {
      static_assert(std::numeric_limits<Unsigned>::digits <= std::numeric_limits<Float>::max_exponent, "integer too big");
      static_assert(std::numeric_limits<Float>::is_iec559, "Float is not a ISO IEC 559 (IEEE 754) floating-point number");
      static_assert(sizeof(Float) == sizeof(Rep), "mismatch in sizes of Float and Rep");
      union {Float flt; Rep rep;} const data = {(Float) value + (Float) 0.5};
      return (std::numeric_limits<Float>::min_exponent - 1) + (data.rep >> std::numeric_limits<Float>::digits - 1);
    }
  }

  template<typename Unsigned>
  int ndigits(Unsigned value) mwg_noexcept {
    if (sizeof(void*) == 8)
      return ndigits_detail::ndigits_<Unsigned, double, stdm::uint64_t>(value);
    else
      return ndigits_detail::ndigits_<Unsigned, float, stdm::uint32_t>(value);
  }

#elif defined(__GNUC__)
  inline mwg_constexpr int ndigits(unsigned value) {
    return value? std::numeric_limits<unsigned>::digits - __builtin_clz(value): 0;
  }
  inline mwg_constexpr int ndigits(unsigned long value) {
    return value? std::numeric_limits<unsigned long>::digits - __builtin_clzl(value): 0;
  }
  inline mwg_constexpr int ndigits(unsigned long long value) {
    return value? std::numeric_limits<unsigned long long>::digits - __builtin_clzll(value): 0;
  }

#else
  // http://d.hatena.ne.jp/siokoshou/20090704#p1
  // http://qiita.com/kazatsuyu/items/38203287c19890a2b7c6
  namespace ndigits_detail {
    template<std::size_t Shift>
    struct sup_pow2m1_ {
      static mwg_constexpr_const std::size_t shift = Shift / 2;
      template<typename Unsigned>
      static mwg_constexpr Unsigned get(Unsigned value) mwg_noexcept {
        return sup_pow2m1_<Shift - shift>::get(value | value >> shift);
      }
    };
    template<>
    struct sup_pow2m1_<1> {
      template<typename Unsigned>
      static mwg_constexpr Unsigned get(Unsigned value) mwg_noexcept {return value;}
    };
    template<> struct sup_pow2m1_<0> {};
    template<typename Unsigned>
    mwg_constexpr Unsigned sup_pow2m1(Unsigned value) mwg_noexcept {
      return sup_pow2m1_<std::numeric_limits<Unsigned>::digits>::get(value);
    };

    template<typename Unsigned>
    mwg_constexpr Unsigned inf_pow2_(Unsigned value) mwg_noexcept {return value ^ value >> 1;}
    template<typename Unsigned>
    mwg_constexpr Unsigned inf_pow2(Unsigned value) mwg_noexcept {return inf_pow2_(sup_pow2m1(value));}

    // use inverse Burrows-Wheeler transform (see https://en.wikipedia.org/wiki/De_Bruijn_sequence)
    template<typename U, int ndigit, int imax = ndigit - 1, U visited = 0, U result = 0, int i = -1, int pos = -1, bool = i != pos>
    struct magic: magic<U, ndigit, imax, visited | 1ull << pos, result << 1 | (pos < (imax + 1) / 2? 0: 1), i, pos * 2 % imax> {};
    template<typename U, int ndigit, int imax, U visited, U result, int i, int pos>
    struct magic<U, ndigit, imax, visited, result, i, pos, false>: magic<U, ndigit, imax, visited, result, i + 1, i + 1, (visited & 1ull << i + 1) == 0> {};
    template<typename U, int ndigit, int imax, U visited, U result>
    struct magic<U, ndigit, imax, visited, result, imax, imax, true>: stdm::integral_constant<U, result << 1 | 1> {};

    template<typename Unsigned, int nbits>
    struct table {
      static mwg_constexpr_const int ndigit = 1 << nbits;
      static mwg_constexpr_const int nshift = ndigit - nbits  - 1;
      static mwg_constexpr_const int ntable = (1 << nbits + 1) - 1;
      static mwg_constexpr_const Unsigned sequence = magic<Unsigned, ndigit>::value;

      int nd_table[ntable];
      int nlz_table[ntable];
      int ntz_table[ntable];
      table() {
        nd_table[0] = 0;
        ntz_table[0] = ndigit;
        nlz_table[0] = ndigit;
        for (int i = 0; i < ndigit; i++) {
          Unsigned const bit = (Unsigned) 1 << i;
          nd_table[Unsigned(sequence * bit) >> nshift] = i + 1;
          ntz_table[Unsigned(sequence * bit) >> nshift] = i;
          nlz_table[Unsigned(sequence * bit) >> nshift] = ndigit - i - 1;
        }
      }

      mwg_constexpr int nd(Unsigned value) const mwg_noexcept {
        return nd_table[Unsigned(sequence * inf_pow2(value)) >> nshift];
      }
      mwg_constexpr int nlz(Unsigned value) const mwg_noexcept {
        return nlz_table[Unsigned(sequence * inf_pow2(value)) >> nshift];
      }
      mwg_constexpr int ntz(Unsigned value) const mwg_noexcept {
        return ntz_table[Unsigned(sequence * (value & -value)) >> nshift];
      }
    };

    static table<stdm::uint32_t, 5> impl32;
    static table<stdm::uint64_t, 6> impl64;
  }

  inline int ndigits(stdm::uint32_t value) mwg_noexcept {return ndigits_detail::impl32.nd(value);}
  inline int ndigits(stdm::uint64_t value) mwg_noexcept {return ndigits_detail::impl64.nd(value);}

#endif

}
}
#endif
