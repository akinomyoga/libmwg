#include <cstddef>
#include <limits>
#include <iomanip>
#include <mwg/defs.h>
#include <mwg/except.h>
#include <mwg/std/cstdint>
#include <mwg/std/cmath>
#include "measure.h"

// ndigits()
//
// 桁数を数える。
//
// - http://www.nminoru.jp/~nminoru/programming/bitcount.html に NLZ がある。
//   どうも条件分岐を使うと却って遅いようだ。
//   IEEE 754 を仮定しないのだとしたら最速は NTZ に渡す方法のようだ。
//
// - http://qiita.com/kazatsuyu/items/38203287c19890a2b7c6
//   どうも NTZ に関する Qiita の記事があったようなと思ったらあった。
//   と思ったら NLZ についても書いてあった。
//

namespace std11 = mwg::stdm;

template<typename Unsigned>
mwg_constexpr14 int ndigits_impl_shift(Unsigned value) mwg_noexcept {
  int count = 0;
  for (; value; value >>= 1, count++);
  return count;
}

template<typename Unsigned>
mwg_constexpr14 int ndigits_impl_shift4(Unsigned value) mwg_noexcept {
  if (value == 0) return 0;
  mwg_constexpr_const std11::uint32_t table = 0xFFFFAA50;
  int count = 1;
  for (; value >> 4; value >>= 4, count += 4);
  count += table >> 2 * value & 3;
  return count;
}
template<typename Unsigned>
mwg_constexpr14 int ndigits_impl_shift8(Unsigned value) mwg_noexcept {
  if (value == 0) return 0;
  mwg_constexpr_const std11::uint32_t table = 0xFFFFAA50;
  int count = 1;
  for (; value >> 8; value >>= 8, count += 8);
  if (value >> 4) value >>= 4, count += 4;
  count += table >> 2 * value & 3;
  return count;
}

template<typename Unsigned>
mwg_constexpr14 int ndigits_impl_frexp(Unsigned value) mwg_noexcept {
  // - Note: frexp は value == 0 に対して *exp = 0 を返すので気にしなくて良い。
  // - NTZ の場合の計測結果がここにある https://srad.jp/~TarZ/journal/481257/
  //   これによると frexp による実装は遅そうだ。
  int ret;
  frexp((double) value, &ret);
  return ret;
}

#ifdef __STDC_IEC_559__
// http://www.nminoru.jp/~nminoru/programming/bitcount.html 改変

template<typename Unsigned>
int ndigits_impl_double(Unsigned value) mwg_noexcept {
  static_assert(std::numeric_limits<Unsigned>::digits <= 1024, "integer too big");
  union {
    double dbl;
    std11::uint64_t rep;
  } const data = {(double) value + 0.5};
  return (data.rep >> 52) - 1022;
}

template<typename Unsigned>
int ndigits_impl_float(Unsigned value) mwg_noexcept {
  static_assert(std::numeric_limits<Unsigned>::digits <= 128, "integer too big");
  union {
    float dbl;
    std11::uint32_t rep;
  } const data = {(float) value + 0.5};
  return (data.rep >> 23) - 126;
}

#endif

template<typename Unsigned>
mwg_constexpr14 int ndigits_impl_bsec(Unsigned value) mwg_noexcept {
  int digits = std::numeric_limits<Unsigned>::digits;
  if (value == 0) return 0;
  int count = 1;
  while (digits > 4) {
    int const modexp = digits / 2;
    // if (Unsigned const reduced = value >> modexp) value = reduced, count += modexp; // 何故か遅い
    if (value >> modexp) value >>=modexp, count += modexp;
    digits -= modexp;
  }
  count += 0xFFFFAA50u >> 2 * value & 3;
  return count;
}

namespace bsec {
  template<std::size_t MaxDigits, bool = MaxDigits <= 4>
  struct ndigits_impl_bsec2_ {
    static const std::size_t modexp = MaxDigits / 2;
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) {
      typedef ndigits_impl_bsec2_<MaxDigits - modexp> half_t;
      return value >> modexp? half_t::eval(value >> modexp, accumulator + modexp): half_t::eval(value, accumulator);
    }
  };
  template<std::size_t MaxDigits>
  struct ndigits_impl_bsec2_<MaxDigits, true> {
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) {
      return accumulator + (0xFFFFAA50u >> 2 * value & 3);
    }
  };
}
template<typename Unsigned>
mwg_constexpr int ndigits_impl_bsec2(Unsigned value) {
  return value? bsec::ndigits_impl_bsec2_<std::numeric_limits<Unsigned>::digits>::eval(value, 1): 0;
}

namespace bsec {
  template<std::size_t MaxDigits, bool = MaxDigits <= 4>
  struct ndigits_impl_bsec3_: ndigits_impl_bsec2_<MaxDigits> {};
  static mwg_constexpr_const int bsec3_table[] = {-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};
  template<std::size_t MaxDigits>
  struct ndigits_impl_bsec3_<MaxDigits, true> {
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) {
      return accumulator + bsec3_table[value];
    }
  };
}
template<typename Unsigned>
mwg_constexpr int ndigits_impl_bsec3(Unsigned value) {
  // bsec2 の様に即値のシフトによるテーブルにしても、
  // bsec3 の様に配列を参照しても速度は変わらない様だ。
  return value? bsec::ndigits_impl_bsec3_<std::numeric_limits<Unsigned>::digits>::eval(value, 1): 0;
}

#ifdef __GNUC__
inline mwg_constexpr int ndigits_impl_builtin(unsigned value) {
  return value? std::numeric_limits<unsigned>::digits - __builtin_clz(value): 0;
}
inline mwg_constexpr int ndigits_impl_builtin(unsigned long value) {
  return value? std::numeric_limits<unsigned long>::digits - __builtin_clzl(value): 0;
}
inline mwg_constexpr int ndigits_impl_builtin(unsigned long long value) {
  return value? std::numeric_limits<unsigned long long>::digits - __builtin_clzll(value): 0;
}
#elif defined(_MSC_VER)
inline mwg_constexpr int ndigits_impl_builtin(unsigned long value) {
  if (value == 0) return 0;
  unsigned long ret;
  _BitScanReverse(&ret, value);
  return (int) ret + 1;
}
inline mwg_constexpr int ndigits_impl_builtin(unsigned __int64 value) {
  if (value == 0) return 0;
  unsigned long ret;
  _BitScanReverse64(&ret, value);
  return (int) ret + 1;
}

// Note: MSVC では _BitScanReverse, _BitScanReverse64 が使える。
// https://msdn.microsoft.com/ja-jp/library/fbxyd7zd.aspx
// http://blog.jiubao.org/2015/01/gcc-bitscanforward-bitscanreverse-msvc.html
#endif

namespace kazatsuyu {
  template<int n>
  struct highest_bit {
    template<typename T>
    static inline mwg_constexpr T get(T val) mwg_noexcept {
      return highest_bit<n/2>::get(static_cast<T>(val | (val >> n)));
    }
  };
  template<>
  struct highest_bit<1> {
    template<typename T>
    static inline mwg_constexpr T get_2(T val) mwg_noexcept { return static_cast<T>(val ^ (val >> 1)); }
    template<typename T>
    static inline mwg_constexpr T get(T val) mwg_noexcept { return get_2(static_cast<T>(val | (val >> 1))); }
  };
  template<typename T>
  inline mwg_constexpr T get_highest_bit(T val) mwg_noexcept { return highest_bit<sizeof(T)*4>::get(val); }

  template<int> struct ntz_traits {};
  template<>
  struct ntz_traits<8> {
    typedef std11::uint64_t type;
    static mwg_constexpr_const int shift = 57;
    static mwg_constexpr_const type magic = 0x03F0A933ADCBD8D1ULL;
    static const int nlz_table[127];
  };

  const int ntz_traits<8>::nlz_table[127] = {
    64, 63, -1, 62, -1, 51, -1, 61,  3, -1, 50, -1, -1, 10, -1, 60,
    2, -1, -1, 42, -1, 49, -1, 21, -1, 39,  9, -1, -1, 35, -1, 59,
    1, -1,  5, -1, 44, -1, 41, -1, -1, 46, 48, -1, -1, 30, -1, 20,
    -1, 13, -1, 38,  8, -1, -1, 28, -1, 25, 34, -1, -1, 18, -1, 58,
    0, -1, 52, -1,  4, -1, 11, -1, -1, 43, -1, 22, 40, -1, 36, -1,
    -1,  6, 45, -1, 47, -1, 31, -1, 14, -1, -1, 29, 26, -1, 19, -1,
    -1, 53, -1, 12, -1, 23, -1, 37,  7, -1, -1, 32, 15, -1, 27, -1,
    54, -1, 24, -1, -1, 33, 16, -1, 55, -1, -1, 17, 56, -1, 57,
  };

  // unsigned型のNLZ
  template<typename T>
  inline mwg_constexpr typename std11::enable_if<std11::is_unsigned<T>::value, int>::type
  nlz(T val) mwg_noexcept {
    typedef ntz_traits<sizeof(T)> tr;
    typedef typename tr::type type;
    return tr::nlz_table[static_cast<type>(tr::magic*get_highest_bit(val))>>tr::shift];
  }

  mwg_constexpr int ndigits_impl_kazatsuyu(std11::uint64_t value) {
    // http://qiita.com/kazatsuyu/items/38203287c19890a2b7c6
    return 64 - kazatsuyu::nlz(value);
  }
}
using kazatsuyu::ndigits_impl_kazatsuyu;

namespace debruijn {
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
  mwg_constexpr Unsigned highest_bit_impl2(Unsigned value) mwg_noexcept {
    return value ^ value >> 1;
  }
  template<typename Unsigned>
  mwg_constexpr Unsigned highest_bit(Unsigned value) mwg_noexcept {
    return highest_bit_impl2(sup_pow2m1(value));
  }
  inline int ndigits_impl_debruijn(std11::uint64_t value) mwg_noexcept {
    // http://qiita.com/kazatsuyu/items/38203287c19890a2b7c6
    static mwg_constexpr_const std11::uint64_t magic = 0x03F0A933ADCBD8D1;
    static mwg_constexpr_const char table[127] = {
       0,  1, -1,  2, -1, 13, -1,  3, 61, -1, 14, -1, -1, 54, -1,  4,
      62, -1, -1, 22, -1, 15, -1, 43, -1, 25, 55, -1, -1, 29, -1,  5,
      63, -1, 59, -1, 20, -1, 23, -1, -1, 18, 16, -1, -1, 34, -1, 44,
      -1, 51, -1, 26, 56, -1, -1, 36, -1, 39, 30, -1, -1, 46, -1,  6,
      64, -1, 12, -1, 60, -1, 53, -1, -1, 21, -1, 42, 24, -1, 28, -1,
      -1, 58, 19, -1, 17, -1, 33, -1, 50, -1, -1, 35, 38, -1, 45, -1,
      -1, 11, -1, 52, -1, 41, -1, 27, 57, -1, -1, 32, 49, -1, 37, -1,
      10, -1, 40, -1, -1, 31, 48, -1,  9, -1, -1, 47,  8, -1,  7,
    };
    return table[magic * highest_bit(value) >> 63 - 6];
  }
}
using debruijn::ndigits_impl_debruijn;

namespace de_bruijn {

  // use inverse Burrows-Wheeler transform (see https://en.wikipedia.org/wiki/De_Bruijn_sequence)
  template<int ndigit, int imax = ndigit - 1, std11::uint64_t used = 0, std11::uint64_t result = 0, int i = -1, int pos = -1, bool = i != pos>
  struct magic: magic<ndigit, imax, used | 1ull << pos, result << 1 | (pos < (imax + 1) / 2? 0: 1), i, pos * 2 % imax> {};
  template<int ndigit, int imax, std11::uint64_t used, std11::uint64_t result, int i, int pos>
  struct magic<ndigit, imax, used, result, i, pos, false>: magic<ndigit, imax, used, result, i + 1, i + 1, (used & 1ull << i + 1) == 0> {};
  template<int ndigit, int imax, std11::uint64_t used, std11::uint64_t result>
  struct magic<ndigit, imax, used, result, imax, imax, true>: std11::integral_constant<std11::uint64_t, result << 1 | 1> {};

  mwg_constexpr std11::uint64_t generate_magic(int nbits) {
    int const len = 1 << nbits;

    std11::uint64_t used = 0;
    std11::uint64_t result = 0;
    for (int start = 0; start < len - 1; start++) {
      if (used & 1ull << start) continue;
      int pos = start;
      do {
        used |= 1 << pos;
        result = result << 1 | pos >= len / 2;
        pos = pos * 2 % (len - 1);
      } while (pos != start);
    }
    result = result << 1 | 1;
    return result;
  }

  void check() {
    mwg_check(magic<(1 << 2)>::value== generate_magic(2));
    mwg_check(magic<(1 << 3)>::value== generate_magic(3));
    mwg_check(magic<(1 << 4)>::value== generate_magic(4));
    mwg_check(magic<(1 << 5)>::value== generate_magic(5));
    mwg_check(magic<(1 << 6)>::value== generate_magic(6));
  }

  void debug() {
    std11::uint64_t magic = de_bruijn::magic<64>::value;
    int nbits = 6;
    int len = 1 << nbits;
    std::cout << "magic = " << std::hex << magic << std::dec << std::endl;
    for (int i = 0; i < len; i++) {
      std11::uint64_t const value = magic << i >> len - nbits; // | magic >> len - nbits >> len - i;
      std::cout << std::setw(2) << value << " ";
      for (int j = nbits; --j >= 0; )
        std::cout << (value >> j & 1);
      std::cout << std::endl;
    }
  }

  template<int nbits>
  struct table {
    static mwg_constexpr_const int ndigit = 1 << nbits;
    static mwg_constexpr_const int nshift = ndigit - nbits  - 1;
    static mwg_constexpr_const int ntable = ndigit * 2 - 1;
    static mwg_constexpr_const std11::uint64_t magic = de_bruijn::magic<ndigit>::value;

    int nd_table[ntable];
    int nlz_table[ntable];
    int ntz_table[ntable];
    table() {
      nd_table[0] = 0;
      ntz_table[0] = ndigit;
      nlz_table[0] = ndigit;
      for (int i = 0; i < ndigit; i++) {
        std11::uint64_t const value = 1ull << i;
        nd_table[magic * value >> nshift] = i + 1;
        ntz_table[magic * value >> nshift] = i;
        nlz_table[magic * value >> nshift] = ndigit - i - 1;
      }
    }

    template<typename Unsigned>
    mwg_constexpr int nd(Unsigned value) const mwg_noexcept {
      return nd_table[magic * debruijn::highest_bit(value) >> nshift];
    }
    template<typename Unsigned>
    mwg_constexpr int nlz(Unsigned value) const mwg_noexcept {
      return nlz_table[magic * debruijn::highest_bit(value) >> nshift];
    }
  };

  static table<6> impl64;
  inline int ndigits_impl_debruijn2(std11::uint64_t value) mwg_noexcept {return impl64.nd(value);}
}
using de_bruijn::ndigits_impl_debruijn2;

volatile int a = 0;
void measure() {
  static const std::size_t nmeasure = 100000;
  //if (libmwg::scope_stopwatch sw = libmwg::scope_stopwatch::set_base())
  if (libmwg::scope_stopwatch sw = "base")
    for(std::size_t i = 0; i < nmeasure; i++) a = i;

#define measure_impl(Name) do { \
    for(std::size_t i = 0; i < nmeasure; i++) \
      mwg_check(ndigits_impl_shift(i) == ndigits_impl_##Name(i), "i=%d result=%d (%d)", i, ndigits_impl_##Name(i), ndigits_impl_shift(i)); \
    if (libmwg::scope_stopwatch sw = #Name) \
      for(std::size_t i = 0; i < nmeasure; i++) a += ndigits_impl_##Name(i); \
  } while (0)

  measure_impl(shift);
  measure_impl(shift4);
  measure_impl(shift8);
  measure_impl(frexp);
  measure_impl(bsec);
  measure_impl(bsec2);
  measure_impl(bsec3);
#ifdef __STDC_IEC_559__
  measure_impl(double);
  measure_impl(float);
#endif
#if defined(__GNUC__) || defined(_MSC_VER)
  measure_impl(builtin);
#endif
  measure_impl(kazatsuyu);
  measure_impl(debruijn);
  measure_impl(debruijn2);
}

int main() {
  //de_bruijn::debug();
  measure();
  // static mwg_constexpr_const int table[127] = {
  //   64, 63, -1, 62, -1, 51, -1, 61,  3, -1, 50, -1, -1, 10, -1, 60,
  //    2, -1, -1, 42, -1, 49, -1, 21, -1, 39,  9, -1, -1, 35, -1, 59,
  //    1, -1,  5, -1, 44, -1, 41, -1, -1, 46, 48, -1, -1, 30, -1, 20,
  //   -1, 13, -1, 38,  8, -1, -1, 28, -1, 25, 34, -1, -1, 18, -1, 58,
  //    0, -1, 52, -1,  4, -1, 11, -1, -1, 43, -1, 22, 40, -1, 36, -1,
  //   -1,  6, 45, -1, 47, -1, 31, -1, 14, -1, -1, 29, 26, -1, 19, -1,
  //   -1, 53, -1, 12, -1, 23, -1, 37,  7, -1, -1, 32, 15, -1, 27, -1,
  //   54, -1, 24, -1, -1, 33, 16, -1, 55, -1, -1, 17, 56, -1, 57,
  // };
  // for (int i = 0; i < 127; i++) {
  //   std::printf(" %3d,", table[i] == -1? -1: 64 - table[i]);
  //   if ((i + 1) % 16 == 0) std::putchar('\n');
  // }

  return 0;
}
