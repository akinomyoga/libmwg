#include <cstddef>
#include <cstring>
#include <limits>
#include <iomanip>
#include <vector>
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

//-----------------------------------------------------------------------------
// shift

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
mwg_constexpr14 int ntz_impl_shift1f(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  int count = 0;
  for (; (value & 1) == 0; value >>= 1, count++);
  return count;
}
template<typename Unsigned>
mwg_constexpr14 int ntz_impl_shift4f(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  int count = 0;
  for (; (value & 0xF) == 0; value >>= 4, count += 4);
  count += 0x12131210u >> 2 * value & 3;
  return count;
}
template<typename Unsigned>
mwg_constexpr14 int ntz_impl_shift8f(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  int count = 0;
  for (; (value & 0xFF) == 0; value >>= 8, count += 8);
  if ((value & 0xF) == 0) value >>= 4, count += 4;
  count += 0x12131210u >> 2 * value & 3;
  return count;
}

/*
 * value &= -value にすると最下位ビットだけ残して他は消える。
 * この状態ならば判定が簡単になる。
 * シフトしてビット1が残っていればシフトして消えた所にビット1はないと分かる。
 *
 * 残り 8 ビットになった時に以下の方法で 32 bit 整数によるテーブルから値を読み出せる。
 * 残り 4 ビットになった時は1行目は要らない。
 *
 *   if (value >> 4) value >>= 4, count += 4;
 *   count += 0x12131210u >> 2 * value & 3;
 *
 * しかし、現れる組み合わせは 0b0001, 0b0010, 0b0100, 0b1000 しかない。
 * これらは掛け算によってシフトを起こす数であるとも取れる。
 * つまり、シフトの代わりに掛け算をすればより沢山の値を 32 bit 整数に格納できる。
 * 残り 8 ビットになった時に以下のように判定すれば良い。
 *
 *   value *= value;
 *   value *= value;
 *   count += 0x01234567u * value >> 28;
 *
 * 実際に測定してみるとこちらの方が速い様であるのでこちらを採用する。
 *
 * 一方で、残り 4 ビットになった時は以下の式を使えるが、
 * 実際に測定してみると却って遅くなるようなので使わない。
 *
 *   count += (value * value) * 0x001Bu >> 6 & 3;
 *
 */

template<typename Unsigned>
mwg_constexpr14 int ntz_impl_shift4fx(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  value &= -value;
  int count = 0;
  for (; value >> 4; value >>= 4, count += 4);
  count += 0x12131210u >> 2 * value & 3;
  return count;
}
template<typename Unsigned>
mwg_constexpr14 int ntz_impl_shift8fx(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  value &= -value;
  int count = 0;
  for (; value >> 8; value >>= 8, count += 8);
  value *= value;
  value *= value;
  count += 0x01234567u * value >> 28;
  return count;
}

//-----------------------------------------------------------------------------
// floating-point numbers

template<typename Unsigned>
mwg_constexpr14 int ndigits_impl_frexp(Unsigned value) mwg_noexcept {
  /* - Note: frexp は value == 0 に対して *exp = 0 を返すので気にしなくて良い。
   * - NTZ の場合の計測結果がここにある https://srad.jp/~TarZ/journal/481257/
   *   これによると frexp による実装は遅そうだ。
   */
  int ret = 0;
  frexp((double) value, &ret);
  return ret;
}

template<typename Unsigned>
mwg_constexpr14 int ntz_impl_frexp(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  int ret = 0;
  frexp((double) (value & -value), &ret);
  return ret - 1;
}

#if defined(__STDC_IEC_559__) || defined(_MSC_VER) || defined(__CYGWIN__)
// http://www.nminoru.jp/~nminoru/programming/bitcount.html 改変

template<typename Unsigned, typename Float, typename Rep>
int ndigits_impl_float_(Unsigned value) mwg_noexcept {
  static_assert(std::numeric_limits<Unsigned>::digits <= std::numeric_limits<Float>::max_exponent, "integer too big");
  static_assert(std::numeric_limits<Float>::is_iec559, "Float is not a ISO IEC 559 (IEEE 754) floating-point number");
  static_assert(sizeof(Float) == sizeof(Rep), "mismatch in sizes of Float and Rep");
  union {
    Float flt;
    Rep rep;
  } const data = {(Float) value + (Float) 0.5};
  return (std::numeric_limits<Float>::min_exponent - 1) + (data.rep >> (std::numeric_limits<Float>::digits - 1));
}

template<typename Unsigned, typename Float, typename Rep>
int ntz_impl_float_(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  static_assert(std::numeric_limits<Unsigned>::digits <= std::numeric_limits<Float>::max_exponent, "integer too big");
  static_assert(std::numeric_limits<Float>::is_iec559, "Float is not a ISO IEC 559 (IEEE 754) floating-point number");
  static_assert(sizeof(Float) == sizeof(Rep), "mismatch in sizes of Float and Rep");
  union {Float flt; Rep rep;} const data = {value & -value};
  return (std::numeric_limits<Float>::min_exponent - 2) + (data.rep >> (std::numeric_limits<Float>::digits - 1));
}

template<typename Unsigned>
int ndigits_impl_double(Unsigned value) mwg_noexcept {
  return ndigits_impl_float_<Unsigned, double, std11::uint64_t>(value);
}
template<typename Unsigned>
int ndigits_impl_float(Unsigned value) mwg_noexcept {
  return ndigits_impl_float_<Unsigned, float, std11::uint32_t>(value);
}
template<typename Unsigned>
int ntz_impl_double(Unsigned value) mwg_noexcept {
  return ntz_impl_float_<Unsigned, double, std11::uint64_t>(value);
}
template<typename Unsigned>
int ntz_impl_float(Unsigned value) mwg_noexcept {
  return ntz_impl_float_<Unsigned, float, std11::uint32_t>(value);
}

#endif

//-----------------------------------------------------------------------------
// binary section

template<typename Unsigned>
mwg_constexpr14 int ndigits_impl_bsec(Unsigned value) mwg_noexcept {
  if (value == 0) return 0;
  int digits = std::numeric_limits<Unsigned>::digits;
  int count = 1;
  while (digits > 4) {
    int const modexp = digits / 2;

    /* 以下のようにすると何故か遅い。
     *
     * if (Unsigned const reduced = value >> modexp) value = reduced, count += modexp;
     */
    if (value >> modexp) value >>= modexp, count += modexp;

    digits -= modexp;
  }
  count += 0xFFFFAA50u >> 2 * value & 3;
  return count;
}

/* NTZ 二分法
 *
 * shift による計測で最初に value &= -value にした方が速い様なので、
 * ここではそれを採用する。と思ったがそうとも限らない様だ。
 * 結局両方で実装する事にする。
 *
 * bsec1x
 *   こちらも shift の時と同様に
 *   最後の 1 ビットになるまで二分法を続けるよりも、
 *   最後の 4 ビットになったところでシフトして値を取り出すよりも、
 *   最後の 8 ビットになった所で掛け算にした方が速い。
 */
template<typename Unsigned>
mwg_constexpr14 int ntz_impl_bsec1(Unsigned value) mwg_noexcept {
  static mwg_constexpr_const int digits = std::numeric_limits<Unsigned>::digits;
  if (value == 0) return digits;
  int width = digits;
  int count = 0;
  while (width > 4) {
    int const modexp = width / 2;
    if ((value << digits - modexp) == 0) value >>= modexp, count += modexp;
    width -= modexp;
  }
  count += 0x12131210u >> 2 * value & 3;
  return count;
}
template<typename Unsigned>
mwg_constexpr14 int ntz_impl_bsec1x(Unsigned value) mwg_noexcept {
  if (value == 0) return std::numeric_limits<Unsigned>::digits;
  int digits = std::numeric_limits<Unsigned>::digits;
  value &= -value;
  int count = 0;
  while (digits > 8) {
    int const modexp = digits / 2;
    if (value >> modexp) value >>= modexp, count += modexp;
    digits -= modexp;
  }
  value *= value;
  value *= value;
  count += 0x01234567u * value >> 28;
  return count;
}

namespace bsec {
  // CRTP にすればもっと簡単になるのでは?
  template<std::size_t shift, typename next_t>
  struct ndigits_impl_bsec2_eval {
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      return value >> shift? next_t::eval(value >> shift, accumulator + shift): next_t::eval(value, accumulator);
    }
  };
  template<std::size_t MaxDigits, bool = MaxDigits <= 4>
  struct ndigits_impl_bsec2_:
    ndigits_impl_bsec2_eval<MaxDigits / 2, ndigits_impl_bsec2_<MaxDigits - MaxDigits / 2> > {};
  template<std::size_t MaxDigits>
  struct ndigits_impl_bsec2_<MaxDigits, true> {
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      return accumulator + (0xFFFFAA50u >> 2 * value & 3);
    }
  };
  template<typename Unsigned>
  mwg_constexpr int ndigits_impl_bsec2(Unsigned value) mwg_noexcept {
    return value? bsec::ndigits_impl_bsec2_<std::numeric_limits<Unsigned>::digits>::eval(value, 1): 0;
  }

  template<std::size_t MaxDigits, bool = MaxDigits <= 4>
  struct ndigits_impl_bsec3_:
    ndigits_impl_bsec2_eval<MaxDigits / 2, ndigits_impl_bsec3_<MaxDigits - MaxDigits / 2> > {};
  static mwg_constexpr_const int bsec3_table[] = {-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};
  template<std::size_t MaxDigits>
  struct ndigits_impl_bsec3_<MaxDigits, true> {
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      return accumulator + bsec3_table[value];
    }
  };
  template<typename Unsigned>
  mwg_constexpr int ndigits_impl_bsec3(Unsigned value) mwg_noexcept {
    /* bsec2 の様に即値のシフトによるテーブルにしても、bsec3 の様に配列を参照しても速度は変わらない様だ。 */
    return value? ndigits_impl_bsec3_<std::numeric_limits<Unsigned>::digits>::eval(value, 1): 0;
  }

  template<std::size_t width, typename last_t, bool = (width > last_t::max_width)>
  struct impl_shift {
    static mwg_constexpr_const std::size_t shift = width / 2;
    typedef impl_shift<width - shift, last_t> next_t;
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      return value >> shift? next_t::eval(value >> shift, accumulator + shift): next_t::eval(value, accumulator);
    }
  };
  template<std::size_t width, typename last_t>
  struct impl_shift<width, last_t, false>: last_t {};

  template<std::size_t width, typename last_t, bool = (width > last_t::max_width)>
  struct impl_shift_zero {
    static mwg_constexpr_const std::size_t shift = width / 2;
    typedef impl_shift_zero<width - shift, last_t> next_t;
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      return value << (std::numeric_limits<Unsigned>::digits - shift)?
        next_t::eval(value, accumulator):
        next_t::eval(value >> shift, accumulator + shift);
    }
  };
  template<std::size_t width, typename last_t>
  struct impl_shift_zero<width, last_t, false>: last_t {};

  // ntz_bsec2
  struct ntz_impl_bsec2_last {
    static mwg_constexpr_const std::size_t max_width = 4;
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      return accumulator + (0x12131210u >> 2 * (value & 0xF) & 3);
    }
  };

  template<typename Unsigned>
  mwg_constexpr int ntz_impl_bsec2(Unsigned value) mwg_noexcept {
    return value? impl_shift_zero<std::numeric_limits<Unsigned>::digits, ntz_impl_bsec2_last>::eval(value, 0):
      std::numeric_limits<Unsigned>::digits;
  }

  // ntz_bsec3
  static mwg_constexpr_const int ntz_bsec3_table[] = {-1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
  struct ntz_impl_bsec3_last {
    static mwg_constexpr_const std::size_t max_width = 4;
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      return accumulator + ntz_bsec3_table[value & 0xF];
    }
  };
  template<typename Unsigned>
  mwg_constexpr int ntz_impl_bsec3(Unsigned value) mwg_noexcept {
    return value? impl_shift_zero<std::numeric_limits<Unsigned>::digits, ntz_impl_bsec3_last>::eval(value, 0):
      std::numeric_limits<Unsigned>::digits;
  }

  // ntz_bsec2x
  struct ntz_bsec2x_last {
    static mwg_constexpr_const std::size_t max_width = 8;
    template<typename Unsigned>
    static mwg_constexpr int eval(Unsigned value, int accumulator) mwg_noexcept {
      value *= value;
      value *= value;
      return accumulator + (0x01234567u * value >> 28);
    }
  };
  template<typename Unsigned>
  mwg_constexpr int ntz_impl_bsec2x(Unsigned value) mwg_noexcept {
    return value?
      impl_shift<std::numeric_limits<Unsigned>::digits, ntz_bsec2x_last>::eval(value & -value, 0):
      std::numeric_limits<Unsigned>::digits;
  }
}
using bsec::ndigits_impl_bsec2;
using bsec::ndigits_impl_bsec3;
using bsec::ntz_impl_bsec2;
using bsec::ntz_impl_bsec3;
using bsec::ntz_impl_bsec2x;

//-----------------------------------------------------------------------------
// intrinsic functions

namespace util {
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
}

/* bclz/bctz/bpopcount/bffs
 *
 * Note: [50168 - __builtin_ctz() and intrinsics __bsr(), __bsf() generate suboptimal code on x86_64](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=50168)
 * に __bsr()/__bsf() が存在しそうなことが書かれているが実際に検索してみるとこのページにしか見つからない。
 * これは報告した人が勝手に変なことを書いただけなのだと思われる。
 */
#ifdef __GNUC__
inline mwg_constexpr int ndigits_impl_bclz(unsigned value) mwg_noexcept {
  return value? std::numeric_limits<unsigned>::digits - __builtin_clz(value): 0;
}
inline mwg_constexpr int ndigits_impl_bclz(unsigned long value) mwg_noexcept {
  return value? std::numeric_limits<unsigned long>::digits - __builtin_clzl(value): 0;
}
inline mwg_constexpr int ndigits_impl_bclz(unsigned long long value) mwg_noexcept {
  return value? std::numeric_limits<unsigned long long>::digits - __builtin_clzll(value): 0;
}

inline mwg_constexpr int ndigits_impl_bctz(unsigned value) mwg_noexcept {
  return value? 1 + __builtin_ctz(util::highest_bit(value)): 0;
}
inline mwg_constexpr int ndigits_impl_bctz(unsigned long value) mwg_noexcept {
  return value? 1 + __builtin_ctzl(util::highest_bit(value)): 0;
}
inline mwg_constexpr int ndigits_impl_bctz(unsigned long long value) mwg_noexcept {
  return value? 1 + __builtin_ctzll(util::highest_bit(value)): 0;
}

inline mwg_constexpr int ndigits_impl_bpopcount(unsigned int value) mwg_noexcept {
  return __builtin_popcount(util::sup_pow2m1(value));
}
inline mwg_constexpr int ndigits_impl_bpopcount(unsigned long value) mwg_noexcept {
  return __builtin_popcountl(util::sup_pow2m1(value));
}
inline mwg_constexpr int ndigits_impl_bpopcount(unsigned long long value) mwg_noexcept {
  return __builtin_popcountll(util::sup_pow2m1(value));
}

inline mwg_constexpr int ndigits_impl_bffs(unsigned value) mwg_noexcept {
  return __builtin_ffs(util::highest_bit(value));
}
inline mwg_constexpr int ndigits_impl_bffs(unsigned long value) mwg_noexcept {
  return __builtin_ffsl(util::highest_bit(value));
}
inline mwg_constexpr int ndigits_impl_bffs(unsigned long long value) mwg_noexcept {
  return __builtin_ffsll(util::highest_bit(value));
}

inline mwg_constexpr int ntz_impl_bctz(unsigned value) mwg_noexcept {
  return value? __builtin_ctz(value): std::numeric_limits<unsigned>::digits;
}
inline mwg_constexpr int ntz_impl_bctz(unsigned long value) mwg_noexcept {
  return value? __builtin_ctzl(value): std::numeric_limits<unsigned long>::digits;
}
inline mwg_constexpr int ntz_impl_bctz(unsigned long long value) mwg_noexcept {
  return value? __builtin_ctzll(value): std::numeric_limits<unsigned long long>::digits;
}
inline mwg_constexpr int ntz_impl_bpopcount(unsigned int value) mwg_noexcept {
  return __builtin_popcount((value & -value) - 1);
}
inline mwg_constexpr int ntz_impl_bpopcount(unsigned long value) mwg_noexcept {
  return __builtin_popcountl((value & -value) - 1);
}
inline mwg_constexpr int ntz_impl_bpopcount(unsigned long long value) mwg_noexcept {
  return __builtin_popcountll((value & -value) - 1);
}

#elif defined(_MSC_VER)
# include <intrin.h>
inline int ndigits_impl_bclz(unsigned short value) mwg_noexcept {
  return std::numeric_limits<unsigned short>::digits - __lzcnt16(value);
}
inline int ndigits_impl_bclz(unsigned int value) mwg_noexcept {
  return std::numeric_limits<unsigned int>::digits - __lzcnt(value);
}
# ifdef _M_X64
inline int ndigits_impl_bclz(unsigned __int64 value) mwg_noexcept {
  return std::numeric_limits<unsigned __int64>::digits - __lzcnt64(value);
}
# endif

inline int ndigits_impl_bpopcount(unsigned short value) mwg_noexcept {
  return __popcnt16(util::sup_pow2m1(value));
}
inline int ndigits_impl_bpopcount(unsigned int value) mwg_noexcept {
  return __popcnt(util::sup_pow2m1(value));
}
# ifdef _M_X64
inline int ndigits_impl_bpopcount(unsigned __int64 value) mwg_noexcept {
  return __popcnt64(util::sup_pow2m1(value));
}
# endif

inline int ntz_impl_bpopcount(unsigned short value) mwg_noexcept {
  return __popcnt16((value & -value) - 1);
}
inline int ntz_impl_bpopcount(unsigned int value) mwg_noexcept {
  return __popcnt((value & -value) - 1);
}
# ifdef _M_X64
inline int ntz_impl_bpopcount(unsigned __int64 value) mwg_noexcept {
  return __popcnt64((value & -value) - 1);
}
# endif
#endif

// ilzcnt/tzcnt

/* Note: [x86_64でpopcnt / tzcnt / lzcntする【ビット演算テクニック Advent Calendar 2016 5日目】 - Qiita](http://qiita.com/ocxtal/items/01c46b15cb1f2e656887)
 */
#if defined(_MSC_VER) || defined(__GNUC__) && defined(__CYGWIN__) || MWGCONF_ICC_VER && defined(LAGUERRE)
# define intrin_lzcnt_defined
# ifdef __GNUC__
#  include <x86intrin.h>
# elif defined(_MSC_VER)
#  include <intrin.h>
#  include <ammintrin.h>
#  include <immintrin.h>
# endif
inline int ndigits_impl_ilzcnt(std11::uint32_t value) mwg_noexcept {
  return std::numeric_limits<std11::uint32_t>::digits - _lzcnt_u32(value);
}
inline int ndigits_impl_itzcnt(std11::uint32_t value) mwg_noexcept {
  return _tzcnt_u32(~util::sup_pow2m1(value));
}
inline int ntz_impl_itzcnt(std11::uint32_t value) mwg_noexcept {
  return _tzcnt_u32(value);
}
# if defined(_M_X64) || defined(__x86_64)
inline int ndigits_impl_ilzcnt(std11::uint64_t value) mwg_noexcept {
  return std::numeric_limits<std11::uint64_t>::digits - _lzcnt_u64(value);
}
inline int ndigits_impl_itzcnt(std11::uint64_t value) mwg_noexcept {
  return _tzcnt_u64(~util::sup_pow2m1(value));
}
inline int ntz_impl_itzcnt(std11::uint64_t value) mwg_noexcept {
  return _tzcnt_u64(value);
}
# endif
#endif

// ibsr ibsf

#if defined(_MSC_VER) || defined(__GNUC__) && defined(__CYGWIN__)
# define intrin_bsr_defined
# include <intrin.h>
# include <immintrin.h>
/* MSVC では _BitScanReverse, _BitScanReverse64 が使える。
 *   https://msdn.microsoft.com/ja-jp/library/fbxyd7zd.aspx
 *   http://blog.jiubao.org/2015/01/gcc-bitscanforward-bitscanreverse-msvc.html
 *
 * 調べると一般に使える様だ。と思ったが Linux では使えなかった。
 *   https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=BitScan
 */
inline int ndigits_impl_ibsr(unsigned long value) mwg_noexcept {
  if (value == 0) return 0;
  unsigned long ret;
  _BitScanReverse(&ret, value);
  return (int) ret + 1;
}
inline int ndigits_impl_ibsf(unsigned long value) mwg_noexcept {
  if (value == 0) return 0;
  unsigned long ret;
  _BitScanForward(&ret, util::highest_bit(value));
  return (int) ret + 1;
}
# if defined(_M_X64) || defined(__x86_64)
inline int ndigits_impl_ibsr(unsigned __int64 value) mwg_noexcept {
  if (value == 0) return 0;
  unsigned long ret;
  _BitScanReverse64(&ret, value);
  return (int) ret + 1;
}
inline int ndigits_impl_ibsf(unsigned __int64 value) mwg_noexcept {
  if (value == 0) return 0;
  unsigned long ret;
  _BitScanForward64(&ret, util::highest_bit(value));
  return (int) ret + 1;
}
# endif
#endif

#if defined(_MSC_VER) || defined(__GNUC__) && defined(__CYGWIN__) || MWGCONF_ICC_VER
# define intrin_popcnt_defined
# include <immintrin.h>
inline int ndigits_impl_ipopcnt(std11::uint32_t value) mwg_noexcept {
  return _popcnt32(util::sup_pow2m1(value));
}
# if defined(_M_X64) || defined(__x86_64)
inline int ndigits_impl_ipopcnt(std11::uint64_t value) mwg_noexcept {
  return _popcnt64(util::sup_pow2m1(value));
}
# endif

inline int ntz_impl_ipopcnt(std11::uint32_t value) mwg_noexcept {
  return _popcnt32((value & -value) - 1);
}
# if defined(_M_X64) || defined(__x86_64)
inline int ntz_impl_ipopcnt(std11::uint64_t value) mwg_noexcept {
  return _popcnt64((value & -value) - 1);
}
# endif
#endif

// asmbsr

#ifdef __GNUC__
// macros from https://sourceforge.net/p/predef/wiki/Architectures/
# if defined(__i386) || defined(__x86_64)
template<typename U>
inline int ndigits_impl_asmbsr_(U value) mwg_noexcept {
  __asm__ ("\
    test %0,%0 \n\
    je 1f      \n\
    bsr %0,%0  \n\
    add $1,%0  \n\
  1:" : "+r" (value));
  return (int) value;
}

inline int ndigits_impl_asmbsr(std11::uint32_t value) mwg_noexcept {
  return ndigits_impl_asmbsr_<std11::uint32_t>(value);
}

inline int ndigits_impl_asmbsr(std11::uint64_t value) mwg_noexcept {
#  if defined(__i386)
  std11::uint32_t h = value >> 32, l = value;
  __asm__ ("\
    test %0,%0 \n\
    je 1f      \n\
    bsr %0,%0  \n\
    add $33,%0 \n\
    jmp 2f     \n\
  1:           \n\
    mov %1,%0  \n\
    test %0,%0 \n\
    je 2f      \n\
    bsr %0,%0  \n\
    add $1,%0  \n\
  2:" : "+r" (h) : "r" (l));
  return (int) h;
#  else
  return ndigits_impl_asmbsr_<std11::uint64_t>(value);
#  endif
}
# endif
#else
inline int ndigits_impl_asmbsr(std11::uint32_t value) mwg_noexcept {
  __asm {
    mov  eax, value
    test eax, eax
    je label1
    bsr eax, eax
    add eax, 1
  label1:
  }
}
inline int ndigits_impl_asmbsr(std11::uint64_t value) mwg_noexcept {
# ifdef _M_X64
  __asm {
    mov  rax, value
    test rax, rax
    je label1
    bsr rax, rax
    add rax, 1
  label1:
  }
# else
  std11::uint32_t h = value >> 32, l = value;
  __asm {
    mov eax, h
    test eax, eax
    je label1
    bsr eax, eax
    add eax, 33
    jmp label2
  label1:
    mov eax, l
    test eax, eax
    je label2
    bsr eax, eax
    add eax, 1
  label2:
  }
# endif
}
#endif

//-----------------------------------------------------------------------------
// intrinsic functions

// http://d.hatena.ne.jp/siokoshou/20090704#p1
// http://qiita.com/kazatsuyu/items/38203287c19890a2b7c6
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

  template<> struct ntz_traits<4> {
    typedef std11::uint32_t type;
    static mwg_constexpr_const int shift = 26;
    static mwg_constexpr_const type magic = 0x07C56E99U;
    static const int ntz_table[63];
    static const int nlz_table[63];
  };
  const int ntz_traits<4>::ntz_table[63] = {
    32,  0, -1,  1, -1, 10, -1,  2, 29, -1, 11, -1, 25, -1, -1,  3,
    30, -1, -1, 23, -1, 12, 14, -1, -1, 26, -1, 16, -1, 19, -1,  4,
    31, -1,  9, -1, 28, -1, 24, -1, -1, 22, -1, 13, -1, 15, 18, -1,
    -1,  8, 27, -1, 21, -1, -1, 17,  7, -1, 20, -1,  6, -1,  5
  };
  const int ntz_traits<4>::nlz_table[63] = {
    32, 31, -1, 30, -1, 21, -1, 29,  2, -1, 20, -1,  6, -1, -1, 28,
     1, -1, -1,  8, -1, 19, 17, -1, -1,  5, -1, 15, -1, 12, -1, 27,
     0, -1, 22, -1,  3, -1,  7, -1, -1,  9, -1, 18, -1, 16, 13, -1,
    -1, 23,  4, -1, 10, -1, -1, 14, 24, -1, 11, -1, 25, -1, 26
  };

  template<> struct ntz_traits<8> {
    typedef std11::uint64_t type;
    static mwg_constexpr_const int shift = 57;
    static mwg_constexpr_const type magic = 0x03F0A933ADCBD8D1ULL;
    static const int ntz_table[127];
    static const int nlz_table[127];
  };
  const int ntz_traits<8>::ntz_table[127] = {
    64,  0, -1,  1, -1, 12, -1,  2, 60, -1, 13, -1, -1, 53, -1,  3,
    61, -1, -1, 21, -1, 14, -1, 42, -1, 24, 54, -1, -1, 28, -1,  4,
    62, -1, 58, -1, 19, -1, 22, -1, -1, 17, 15, -1, -1, 33, -1, 43,
    -1, 50, -1, 25, 55, -1, -1, 35, -1, 38, 29, -1, -1, 45, -1,  5,
    63, -1, 11, -1, 59, -1, 52, -1, -1, 20, -1, 41, 23, -1, 27, -1,
    -1, 57, 18, -1, 16, -1, 32, -1, 49, -1, -1, 34, 37, -1, 44, -1,
    -1, 10, -1, 51, -1, 40, -1, 26, 56, -1, -1, 31, 48, -1, 36, -1,
     9, -1, 39, -1, -1, 30, 47, -1,  8, -1, -1, 46,  7, -1,  6,
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

  // unsigned型のNTZ。例の黒魔術
  template<typename T>
  inline typename std11::enable_if<std11::is_unsigned<T>::value, int>::type
  ntz(T val) mwg_noexcept {
    typedef ntz_traits<sizeof(T)> tr;
    typedef typename tr::type type;
    return tr::ntz_table[static_cast<type>(tr::magic*static_cast<type>(val&-val))>>tr::shift];
  }
  // unsigned型のNLZ
  template<typename T>
  inline typename std11::enable_if<std11::is_unsigned<T>::value, int>::type
  nlz(T val) mwg_noexcept {
    typedef ntz_traits<sizeof(T)> tr;
    typedef typename tr::type type;
    return tr::nlz_table[static_cast<type>(tr::magic*get_highest_bit(val))>>tr::shift];
  }

  int ndigits_impl_kazatsuyu(std11::uint32_t value) {return 32 - kazatsuyu::nlz(value);}
  int ndigits_impl_kazatsuyu(std11::uint64_t value) {return 64 - kazatsuyu::nlz(value);}
  int nlz_impl_kazatsuyu(std11::uint32_t value) {return kazatsuyu::nlz(value);}
  int nlz_impl_kazatsuyu(std11::uint64_t value) {return kazatsuyu::nlz(value);}
  int ntz_impl_kazatsuyu(std11::uint32_t value) {return kazatsuyu::ntz(value);}
  int ntz_impl_kazatsuyu(std11::uint64_t value) {return kazatsuyu::ntz(value);}
}
using kazatsuyu::ndigits_impl_kazatsuyu;
using kazatsuyu::ntz_impl_kazatsuyu;
using kazatsuyu::nlz_impl_kazatsuyu;

//TODO 上とくっつける。
namespace kazatsuyu {
  // http://qiita.com/kazatsuyu/items/38203287c19890a2b7c6
  static mwg_constexpr_const std11::uint32_t magic32 = 0x07C56E99U;
  static mwg_constexpr_const char nd_table32[63] = {
     0,  1, -1,  2, -1, 11, -1,  3, 30, -1, 12, -1, 26, -1, -1,  4,
    31, -1, -1, 24, -1, 13, 15, -1, -1, 27, -1, 17, -1, 20, -1,  5,
     0, -1, 10, -1, 29, -1, 25, -1, -1, 23, -1, 14, -1, 16, 19, -1,
    -1,  9, 28, -1, 22, -1, -1, 18,  8, -1, 21, -1,  7, -1,  6,
  };
  static mwg_constexpr_const std11::uint64_t magic64 = 0x03F0A933ADCBD8D1;
  static mwg_constexpr_const char nd_table64[127] = {
     0,  1, -1,  2, -1, 13, -1,  3, 61, -1, 14, -1, -1, 54, -1,  4,
    62, -1, -1, 22, -1, 15, -1, 43, -1, 25, 55, -1, -1, 29, -1,  5,
    63, -1, 59, -1, 20, -1, 23, -1, -1, 18, 16, -1, -1, 34, -1, 44,
    -1, 51, -1, 26, 56, -1, -1, 36, -1, 39, 30, -1, -1, 46, -1,  6,
    64, -1, 12, -1, 60, -1, 53, -1, -1, 21, -1, 42, 24, -1, 28, -1,
    -1, 58, 19, -1, 17, -1, 33, -1, 50, -1, -1, 35, 38, -1, 45, -1,
    -1, 11, -1, 52, -1, 41, -1, 27, 57, -1, -1, 32, 49, -1, 37, -1,
    10, -1, 40, -1, -1, 31, 48, -1,  9, -1, -1, 47,  8, -1,  7,
  };
  inline mwg_constexpr int ndigits_impl_debruijn(std11::uint32_t value) mwg_noexcept {
    return nd_table32[magic32 * util::highest_bit(value) >> ntz_traits<4>::shift];
  }
  inline mwg_constexpr int ndigits_impl_debruijn(std11::uint64_t value) mwg_noexcept {
    return nd_table64[magic64 * util::highest_bit(value) >> ntz_traits<8>::shift];
  }
}
using kazatsuyu::ndigits_impl_debruijn;

namespace de_bruijn {
  // use inverse Burrows-Wheeler transform (see https://en.wikipedia.org/wiki/De_Bruijn_sequence)
  template<typename U, int ndigit, int imax = ndigit - 1, U visited = 0, U result = 0, int i = -1, int pos = -1, bool = i != pos>
  struct magic: magic<U, ndigit, imax, visited | 1ull << pos, result << 1 | (pos < (imax + 1) / 2? 0: 1), i, pos * 2 % imax> {};
  template<typename U, int ndigit, int imax, U visited, U result, int i, int pos>
  struct magic<U, ndigit, imax, visited, result, i, pos, false>: magic<U, ndigit, imax, visited, result, i + 1, i + 1, (visited & 1ull << (i + 1)) == 0> {};
  template<typename U, int ndigit, int imax, U visited, U result>
  struct magic<U, ndigit, imax, visited, result, imax, imax, true>: std11::integral_constant<U, result << 1 | 1> {};

  mwg_constexpr14 std11::uint64_t generate_magic(int nbits) {
    int const len = 1 << nbits;

    std11::uint64_t used = 0;
    std11::uint64_t result = 0;
    for (int start = 0; start < len - 1; start++) {
      if (used & 1ull << start) continue;
      int pos = start;
      do {
        used |= 1 << pos;
        result = result << 1 | (pos >= len / 2);
        pos = pos * 2 % (len - 1);
      } while (pos != start);
    }
    result = result << 1 | 1;
    return result;
  }

  void check() {
    mwg_check((magic<std11::uint64_t, (1 << 2)>::value == generate_magic(2)));
    mwg_check((magic<std11::uint64_t, (1 << 3)>::value == generate_magic(3)));
    mwg_check((magic<std11::uint64_t, (1 << 4)>::value == generate_magic(4)));
    mwg_check((magic<std11::uint64_t, (1 << 5)>::value == generate_magic(5)));
    mwg_check((magic<std11::uint64_t, (1 << 6)>::value == generate_magic(6)));
  }

  void debug() {
    std11::uint64_t magic = de_bruijn::magic<std11::uint64_t, 64>::value;
    int nbits = 6;
    int len = 1 << nbits;
    std::cout << "magic = " << std::hex << magic << std::dec << std::endl;
    for (int i = 0; i < len; i++) {
      std11::uint64_t const value = magic << i >> (len - nbits); // | magic >> len - nbits >> len - i;
      std::cout << std::setw(2) << value << " ";
      for (int j = nbits; --j >= 0; )
        std::cout << (value >> j & 1);
      std::cout << std::endl;
    }
  }

  template<typename Unsigned, int nbits>
  struct table {
    static mwg_constexpr_const int ndigit = 1 << nbits;
    static mwg_constexpr_const int nshift = ndigit - nbits  - 1;
    static mwg_constexpr_const int ntable = (1 << (nbits + 1)) - 1;
    static mwg_constexpr_const Unsigned sequence = de_bruijn::magic<Unsigned, ndigit>::value;

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
      return nd_table[Unsigned(sequence * util::highest_bit(value)) >> nshift];
    }
    mwg_constexpr int nlz(Unsigned value) const mwg_noexcept {
      return nlz_table[Unsigned(sequence * util::highest_bit(value)) >> nshift];
    }
    mwg_constexpr int ntz(Unsigned value) const mwg_noexcept {
      return ntz_table[Unsigned(sequence * (value & -value)) >> nshift];
    }
  };

  // 中で宣言したら遅くなったので。
  static table<std11::uint32_t, 5> impl32;
  static table<std11::uint64_t, 6> impl64;

  inline int ndigits_impl_debruijn2(std11::uint32_t value) mwg_noexcept {return impl32.nd(value);}
  inline int ndigits_impl_debruijn2(std11::uint64_t value) mwg_noexcept {return impl64.nd(value);}
  inline int ntz_impl_debruijn2(std11::uint32_t value) mwg_noexcept {return impl32.ntz(value);}
  inline int ntz_impl_debruijn2(std11::uint64_t value) mwg_noexcept {return impl64.ntz(value);}
  inline int nlz_impl_debruijn2(std11::uint32_t value) mwg_noexcept {return impl32.nlz(value);}
  inline int nlz_impl_debruijn2(std11::uint64_t value) mwg_noexcept {return impl64.nlz(value);}
}
using de_bruijn::ndigits_impl_debruijn2;
using de_bruijn::ntz_impl_debruijn2;
using de_bruijn::nlz_impl_debruijn2;

//-----------------------------------------------------------------------------

volatile int a = 0;

void measure_nd() {
  static const std::size_t nmeasure = 100000;
  //if (libmwg::scope_stopwatch sw = libmwg::scope_stopwatch::set_base())
  if (libmwg::scope_stopwatch sw = "base")
    for(std::size_t i = 0; i < nmeasure; i++) a = (int) i;

#define measure_impl(Name) do { \
    for(std::size_t i = 0; i < nmeasure; i++) \
      mwg_check(ndigits_impl_shift(i) == ndigits_impl_##Name(i), "i=%d result=%d (%d)", i, ndigits_impl_##Name(i), ndigits_impl_shift(i)); \
    if (libmwg::scope_stopwatch sw = #Name) \
      for(std::size_t i = 0; i < nmeasure; i++) a = ndigits_impl_##Name(i); \
  } while (0)

  measure_impl(shift);
  measure_impl(shift4);
  measure_impl(shift8);
  measure_impl(bsec);
  measure_impl(bsec2);
  measure_impl(bsec3);
  measure_impl(kazatsuyu);
  measure_impl(debruijn);
  measure_impl(debruijn2);
  measure_impl(frexp);
#if defined(__STDC_IEC_559__) || defined(_MSC_VER) || defined(__CYGWIN__)
  measure_impl(double);
  measure_impl(float);
#endif
#if defined(__GNUC__) || defined(_MSC_VER)
  measure_impl(bclz);
  measure_impl(bpopcount);
#endif
#ifdef __GNUC__
  measure_impl(bctz);
  measure_impl(bffs);
#endif
#ifdef intrin_lzcnt_defined
  measure_impl(ilzcnt);
  measure_impl(itzcnt);
#endif
#ifdef intrin_bsr_defined
  measure_impl(ibsr);
  measure_impl(ibsf);
#endif
#ifdef intrin_popcnt_defined
  measure_impl(ipopcnt);
#endif
#if defined(__GNUC__) && (defined(__i386) || defined(__x86_64)) || defined(_MSC_VER)
  measure_impl(asmbsr);
#endif
#undef measure_impl
}

#ifdef intrin_lzcnt_defined
# define measure_list_itzcnt  measure_impl(itzcnt)
#else
# define measure_list_itzcnt
#endif
#if defined(__GNUC__) || defined(_MSC_VER)
# define measure_list_bpopcount measure_impl(bpopcount)
#else
# define measure_list_bpopcount
#endif
#ifdef __GNUC__
# define measure_list_bctz measure_impl(bctz)
#else
# define measure_list_bctz
#endif
#ifdef intrin_popcnt_defined
# define measure_list_ipopcnt measure_impl(ipopcnt)
#else
# define measure_list_ipopcnt
#endif

#define measure_list \
  measure_impl(shift1f)                         \
  measure_impl(shift4f)                         \
  measure_impl(shift8f)                         \
  measure_impl(shift4fx)                        \
  measure_impl(shift8fx)                        \
  measure_impl(bsec1)                           \
  measure_impl(bsec2)                           \
  measure_impl(bsec3)                           \
  measure_impl(bsec1x)                          \
  measure_impl(bsec2x)                          \
  measure_impl(frexp)                           \
  measure_impl(double)                          \
  measure_impl(float)                           \
  measure_impl(kazatsuyu)                       \
  measure_impl(debruijn2)                       \
  measure_list_bctz                             \
  measure_list_itzcnt                           \
  measure_list_bpopcount                        \
  measure_list_ipopcnt

void measure_ntz_base(std::size_t const nloop) {
  libmwg::scope_stopwatch sw = "base";
  for(std::size_t i = 0; i < nloop; i++) a = (int) i;
}

#define measure_impl(Name) \
void check_ntz_##Name(std::size_t const nloop) { \
  for(std::size_t i = 0; i < nloop; i++) \
    mwg_check(ntz_impl_shift1f(i) == ntz_impl_##Name(i), "i=%d result=%d (%d)", i, ntz_impl_##Name(i), ntz_impl_shift1f(i)); \
} \
void measure_ntz_##Name(std::size_t const nloop) { \
  libmwg::scope_stopwatch sw = #Name; \
  for(std::size_t i = 0; i < nloop; i++) a = ntz_impl_##Name(i); \
}
measure_list
#undef measure_impl

void measure_ntz() {
  static const std::size_t nloop = 100000;

  std::vector<void(*)(std::size_t)> list;

  list.push_back(&measure_ntz_base);
#define measure_impl(Name) check_ntz_##Name(nloop); list.push_back(&measure_ntz_##Name);
  measure_list;
#undef measure_impl

  for (std::vector<void(*)(std::size_t)>::const_iterator i = list.begin(); i != list.end(); ++i) (*i)(nloop);
}

int main(int argc, char** argv) {
  if (argc >= 2){
    int const nmeasure = 2 < argc? atoi(argv[2]): 1;
    if (std::strcmp(argv[1], "ntz") == 0) {
      for (int i = 0; i < nmeasure; i++) measure_ntz();
      return 0;
    } else if (std::strcmp(argv[1], "nd") == 0) {
      for (int i = 0; i < nmeasure; i++) measure_nd();
      return 0;
    }
  }

  std::cout << "sizeof(std::size_t) = " << sizeof(std::size_t) << std::endl;
  //de_bruijn::debug();
  //for (int i = 0; i < 1000; i++) measure_nd();
  return 0;
}
